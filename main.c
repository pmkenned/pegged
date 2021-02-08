/*
 * Peg Solitaire Solver
 * Paul Kennedy <paul.kennedy124@gmail.com>
 * 7 Feb 2021
 *
 * TODO:
 * - use ncurses to render grid being solved
 * - round-robin several stacks
 * - eliminate symmetrical moves
 * - detect impossible to solve configurations
 * - consider multi-threading
 *
 */

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <ncurses.h>

/* used for left-hand-side of assignment */
#define RIGHT_ONE_LVAL(r,c)   grid[r   ][(c)+1]
#define RIGHT_TWO_LVAL(r,c)   grid[r   ][(c)+2]
#define LEFT_ONE_LVAL(r,c)    grid[r   ][(c)-1]
#define LEFT_TWO_LVAL(r,c)    grid[r   ][(c)-2]
#define UP_ONE_LVAL(r,c)      grid[r-1 ][c    ]
#define UP_TWO_LVAL(r,c)      grid[r-2 ][c    ]
#define DOWN_ONE_LVAL(r,c)    grid[r+1 ][c    ]
#define DOWN_TWO_LVAL(r,c)    grid[r+2 ][c    ]

/* used to look up grid locations; evaluate to INVALID if outside bounds */
#define RIGHT_ONE(r,c)  (((c) < NCOLS-1)    ? grid[r][c+1] : INVALID)
#define RIGHT_TWO(r,c)  (((c) < NCOLS-2)    ? grid[r][c+2] : INVALID)
#define LEFT_ONE(r,c)   (((c) > 0)          ? grid[r][c-1] : INVALID)
#define LEFT_TWO(r,c)   (((c) > 1)          ? grid[r][c-2] : INVALID)
#define DOWN_ONE(r,c)   (((r) < NROWS-1)    ? grid[r+1][c] : INVALID)
#define DOWN_TWO(r,c)   (((r) < NROWS-2)    ? grid[r+2][c] : INVALID)
#define UP_ONE(r,c)     (((r) > 0)          ? grid[r-1][c] : INVALID)
#define UP_TWO(r,c)     (((r) > 1)          ? grid[r-2][c] : INVALID)

typedef struct {
    char row;
    char col;
    char dir;
    char taken;
} move_t;

enum {
    HOLE='.',
    PEG='o',
    INVALID=' '
};

enum {
    NROWS=9,
    NCOLS=9
};

enum {
    DIR_UP=0,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
};

#define THOUSAND(n) n ## 000
#define MILLION(n) n ## 000000

enum {
    MAX_TRIES = THOUSAND(1),
    MAX_MOVES = THOUSAND(20)
};

const char * dir_strs[] = { "up", "down", "left", "right" };

#define GRID_FRENCH {\
    "         ",\
    "   ooo   ",\
    "  ooooo  ",\
    " ooo.ooo ",\
    " ooooooo ",\
    " ooooooo ",\
    "  poooo  ",\
    "   ooo   ",\
    "         "\
}

#define GRID_GERMAN {\
    "   ooo   ",\
    "   ooo   ",\
    "   ooo   ",\
    "ooooooooo",\
    "oooo.oooo",\
    "ooooooooo",\
    "   ooo   ",\
    "   ooo   ",\
    "   ooo   "\
}

#define GRID_ENGLISH {\
    "         ",\
    "   ooo   ",\
    "   ooo   ",\
    " ooooooo ",\
    " ooo.ooo ",\
    " ooooooo ",\
    "   ooo   ",\
    "   ooo   ",\
    "         "\
}

#define GRID_TRIANGLE {\
    "    o    ",\
    "   ooo   ",\
    "  ooooo  ",\
    " ooooooo ",\
    "oooo.oooo",\
    " ooooooo ",\
    "  ooooo  ",\
    "   ooo   ",\
    "    o    "\
}

#define GRID_CROSS {\
    "         ",\
    "   ...   ",\
    "   .o.   ",\
    " ..ooo.. ",\
    " ...o... ",\
    " ...o... ",\
    "   ...   ",\
    "   ...   ",\
    "         "\
}

#define CHOSEN_GRID GRID_ENGLISH

char grid_init[NROWS][NCOLS] = CHOSEN_GRID;
char grid[NROWS][NCOLS] = CHOSEN_GRID;
char * grid_1d = (char *) grid;

int verbose = 1;
long seed;
int nmoves_tried=0;
int nresets;

/* NOTE: stack pointers point to empty slot */
enum { MAX_STACK_DEPTH = 1000 };
size_t avail_sp = 0;
move_t avail_move_stack[MAX_STACK_DEPTH];
size_t max_stack_sp;
int max_fanout = 0;
int npegs=1, nholes=0; /* NOTE: set these initially */

void print_moves(move_t * moves, size_t n)
{
    size_t i;
    for (i=0; i<n; i++) {
        size_t dir = moves[i].dir;
        printw("%d %d %s\n", moves[i].row, moves[i].col, dir_strs[dir]);
    }
}

/* print only the moves that are part of the solution */
void print_taken_moves()
{
    size_t i;
    for (i=0; i<avail_sp; i++) {
        move_t *mp = &avail_move_stack[i];
        size_t dir = mp->dir;
        if (avail_move_stack[i].taken)
            printw("%d %d %s\n", mp->row, mp->col, dir_strs[dir]);
    }
}

void print_top_move()
{
    move_t *mp = &avail_move_stack[avail_sp-1];
    size_t dir = mp->dir;
    move(NROWS+2, 0);
    printw("%d %d %s\n", mp->row, mp->col, dir_strs[dir]);
}

void print_grid()
{
    int r, c;
    for (r=0; r<NROWS; r++) {
        for (c=0; c<NCOLS; c++) {
            /*putchar(grid[r][c]);*/
            mvaddch(r+1,c+1,grid[r][c]);
        }
        /*putchar('\n');*/
    }
    /*putchar('\n');*/
}

int num_pegs()
{
    int i, n=0;
    for (i=0; i < NROWS*NCOLS; i++)
        n += (grid_1d[i] == PEG) ? 1 : 0;
    return n;
}

int num_holes()
{
    int i, n=0;
    for (i=0; i < NROWS*NCOLS; i++)
        n += (grid_1d[i] == HOLE) ? 1 : 0;
    return n;
}

int check_win()
{
#if 1
    return (npegs == 1) ? 1 : 0;
#else
    int i;
    int npegs=0;
    for (i=0; i < NROWS*NCOLS; i++) {
        npegs += (grid_1d[i] == PEG) ? 1 : 0;
        if (npegs > 1)
            return 0;
    }
    return 1;
#endif
}

/* TODO: detect when it is impossible to win */
int win_still_possible()
{
    return 1;
}

void push_move(char r, char c, char dir)
{
    assert(avail_sp < MAX_STACK_DEPTH);
    if (avail_sp > max_stack_sp)
        max_stack_sp = avail_sp;
    avail_move_stack[avail_sp].row = r;
    avail_move_stack[avail_sp].col = c;
    avail_move_stack[avail_sp].dir = dir;
    avail_move_stack[avail_sp].taken = 0;
    avail_sp++;
}

void shuffle_new_moves(int n)
{
    int i;
    move_t tmp;
    move_t *m1, *m2;
    for (i=0; i<n-1; i++) {
        int idx = rand() % (n-i);
        m1 = &avail_move_stack[avail_sp-n+i];
        m2 = &avail_move_stack[avail_sp-n+idx];
        memcpy(&tmp, m1, sizeof(move_t));
        memcpy(m1, m2, sizeof(move_t));
        memcpy(m2, &tmp, sizeof(move_t));
    }

}


int get_moves()
{
    int n = 0;
    int r, c;
/* if there are more pegs than holes, it's more efficient to start with holes and look for pegs */
    if (npegs < nholes) {
        for (r=0; r<NROWS; r++) {
            for (c=0; c<NCOLS; c++) {
                int is_peg = (grid[r][c] == PEG) ? 1 : 0;
                if (!is_peg)
                    continue;
                if ((RIGHT_ONE(r,c) == PEG) && (RIGHT_TWO(r,c) == HOLE)) {
                    push_move(r, c, DIR_RIGHT);
                    n++;
                }
                if ((LEFT_ONE(r,c) == PEG) && (LEFT_TWO(r,c) == HOLE)) {
                    push_move(r, c, DIR_LEFT);
                    n++;
                }
                if ((DOWN_ONE(r,c) == PEG) && (DOWN_TWO(r,c) == HOLE)) {
                    push_move(r, c, DIR_DOWN);
                    n++;
                }
                if ((UP_ONE(r,c) == PEG) && (UP_TWO(r,c) == HOLE)) {
                    push_move(r, c, DIR_UP);
                    n++;
                }
            }
        }
    } else {
        for (r=0; r<NROWS; r++) {
            for (c=0; c<NCOLS; c++) {
                int is_hole = (grid[r][c] == HOLE) ? 1 : 0;
                if (!is_hole)
                    continue;
                /* NOTE: move directions are inverse when looking at holes */
                if ((LEFT_ONE(r,c) == PEG) && (LEFT_TWO(r,c) == PEG)) {
                    push_move(r, c-2, DIR_RIGHT);
                    n++;
                }
                if ((DOWN_ONE(r,c) == PEG) && (DOWN_TWO(r,c) == PEG)) {
                    push_move(r+2, c, DIR_UP);
                    n++;
                }
                if ((RIGHT_ONE(r,c) == PEG) && (RIGHT_TWO(r,c) == PEG)) {
                    push_move(r, c+2, DIR_LEFT);
                    n++;
                }
                if ((UP_ONE(r,c) == PEG) && (UP_TWO(r,c) == PEG)) {
                    push_move(r-2, c, DIR_DOWN);
                    n++;
                }
            }
        }
    }
    if (n > max_fanout) {
        max_fanout = n;
    }
#if 1
    shuffle_new_moves(n);
#endif
    return n;
}

/* execute the move at the top of the stack */
void do_top_move()
{
    move_t *mp = &avail_move_stack[avail_sp-1];
    size_t r = mp->row;
    size_t c = mp->col;
    assert(avail_sp > 0);
    grid[r][c] = HOLE;
    switch (mp->dir) {
        case DIR_UP:
            UP_ONE_LVAL(r,c)= HOLE;
            UP_TWO_LVAL(r,c)= PEG;
            break;
        case DIR_DOWN:
            DOWN_ONE_LVAL(r,c)= HOLE;
            DOWN_TWO_LVAL(r,c)= PEG;
            break;
        case DIR_LEFT:
            LEFT_ONE_LVAL(r,c)= HOLE;
            LEFT_TWO_LVAL(r,c)= PEG;
            break;
        case DIR_RIGHT:
            RIGHT_ONE_LVAL(r,c)= HOLE;
            RIGHT_TWO_LVAL(r,c)= PEG;
            break;
        default:
            assert(0);
            break;
    }
    mp->taken = 1;
    npegs--;
    nholes++;
}

/* undo the move at the top of the stack */
void undo_top_move()
{
    move_t *mp = &avail_move_stack[avail_sp-1];
    size_t r = mp->row;
    size_t c = mp->col;
    assert(avail_sp > 0);
    grid[r][c] = PEG;
    switch (mp->dir) {
        case DIR_UP:
            UP_ONE_LVAL(r,c)= PEG;
            UP_TWO_LVAL(r,c)= HOLE;
            break;
        case DIR_DOWN:
            DOWN_ONE_LVAL(r,c)= PEG;
            DOWN_TWO_LVAL(r,c)= HOLE;
            break;
        case DIR_LEFT:
            LEFT_ONE_LVAL(r,c)= PEG;
            LEFT_TWO_LVAL(r,c)= HOLE;
            break;
        case DIR_RIGHT:
            RIGHT_ONE_LVAL(r,c)= PEG;
            RIGHT_TWO_LVAL(r,c)= HOLE;
            break;
        default:
            assert(0);
            break;
    }
    npegs++;
    nholes--;
}

void pop()
{
    avail_sp--;
    assert(avail_sp > 0);
}

int top_move_tried()
{
    assert(avail_sp > 0);
    return avail_move_stack[avail_sp-1].taken;
}

void reset()
{
    memcpy(grid, grid_init, sizeof(grid));
    npegs = num_pegs();
    nholes = num_holes();
    avail_sp = 0;
    nmoves_tried = 0;
}

/* depth-first-search
 *
 * at start and each time a move is taken, identify all the newly available
 * moves and push them onto a stack
 *
 * apply the move at the top of the stack and mark it as taken
 *
 * repeat the above until either solved or no moves are available
 *
 * if no moves are available, undo the most recent move, remove it from the 
 * stack and look at the new top-of-stack move; if it is already marked as 
 * taken, pop again until an untaken move is found and take it
 *
 * repeat until a solution is found
 *
 * this algorithm can be visualized as searching a tree where nodes are game
 * states and edges are moves; when a leaf node is hit (i.e. no moves), back
 * up to the nearest node with untried moves
 *
 * apparently it can sometimes happen that you go down an unlucky path where
 * there either are no solutions, or they take a very long time to find
 * to mitigate this, there is an outer look that resets the board after a
 * certain threshold and tries a different random path
 *
 */
void solve()
{
    for (nresets=0; nresets < MAX_TRIES; nresets++) {
        reset();
        while (nmoves_tried++ < MAX_MOVES) {
            int n_avail_moves;
            if (check_win()) {
                return;
            }
            n_avail_moves = get_moves();
            if ((n_avail_moves > 0) && win_still_possible()) {
                verbose && (print_top_move(),1);
                do_top_move();
                verbose && (print_grid(),1);
                verbose && refresh();
                0 && getch();
            } else {
                do {
                    verbose && (move(NROWS+1,0), printw("undoing: "),1);
                    verbose && (print_top_move(),1);
                    undo_top_move();
                    verbose && (print_grid(),1);
                    pop();
                    verbose && refresh();
                    0 && getch();
                } while (top_move_tried());
                verbose && (move(NROWS+1,0), printw("         "),1);
                verbose && (print_top_move(),1);
                do_top_move();
                verbose && (print_grid(),1);
                verbose && refresh();
                0 && getch();
            }
            verbose && (move(NROWS+3,0), printw("%d      ", nmoves_tried), 1);
            verbose && refresh();
        }
#if 0
        putchar('.');
        fflush(stdout);
        print_grid();
        refresh();
#endif
        printw(".");
        refresh();
    }
    assert("no solution found" == 0);
}

int main(int argc, char * argv[])
{
    initscr();
    seed = (argc > 1) ? atol(argv[1]) : time(NULL);
    printw("seed: %ld\n", seed);
    srand(seed);
    solve();
    move(NROWS+2,1);
    printw("\nmax fanout: %d\n", max_fanout);
    printw("max stack usage: %zu\n", max_stack_sp);
    printw("num resets: %d\n", nresets);
    printw("num moves tried: %d\n", nmoves_tried);
    print_taken_moves();
    refresh();
    getch();
    endwin();
    return 0;
}
