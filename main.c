#include <stdio.h>
#include <stddef.h>
#include <assert.h>

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
    HOLE='o',
    PEG='*',
    INVALID='-'
};

enum {
    NROWS=7,
    NCOLS=7
};

enum {
    DIR_UP=0,
    DIR_DOWN,
    DIR_LEFT,
    DIR_RIGHT
};

const char * dir_strs[] = { "up", "down", "left", "right" };

char grid_full[NROWS][NCOLS] = {
    "--***--",
    "--***--",
    "*******",
    "***o***",
    "*******",
    "--***--",
    "--***--"
};

char grid_cross[NROWS][NCOLS] = {
    "--ooo--",
    "--o*o--",
    "oo***oo",
    "ooo*ooo",
    "ooo*ooo",
    "--ooo--",
    "--ooo--"
};

char (*grid)[NCOLS] = grid_full;
char * grid_1d = (char *) grid_full;

int verbose = 0;

/* NOTE: stack pointers point to empty slot */
enum { MAX_STACK_DEPTH = 1000 };
size_t avail_sp = 0;
move_t avail_move_stack[MAX_STACK_DEPTH];
#if 0
int moves_taken = 0;
#endif

void print_moves(move_t * moves, size_t n)
{
    size_t i;
    for (i=0; i<n; i++) {
        size_t dir = moves[i].dir;
        printf("%d %d %s\n", moves[i].row, moves[i].col, dir_strs[dir]);
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
            printf("%d %d %s\n", mp->row, mp->col, dir_strs[dir]);
    }
}

void print_top_move()
{
    move_t *mp = &avail_move_stack[avail_sp-1];
    size_t dir = mp->dir;
    printf("%d %d %s\n", mp->row, mp->col, dir_strs[dir]);
}

void print_grid()
{
    int r, c;
    for (r=0; r<NROWS; r++) {
        for (c=0; c<NCOLS; c++) {
            putchar(grid[r][c]);
        }
        putchar('\n');
    }
    putchar('\n');
}

/* NOTE: could be made faster by keeping track of the number of moves made
 * and subtracting this from the initial number of pegs */
int check_win()
{
#if 0
    return (INIT_NUM_PEGS - moves_taken == 1) ? 1 : 0;
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
    avail_move_stack[avail_sp].row = r;
    avail_move_stack[avail_sp].col = c;
    avail_move_stack[avail_sp].dir = dir;
    avail_move_stack[avail_sp].taken = 0;
    avail_sp++;
}

int get_moves()
{
    int n = 0;
    int r, c;
/* if there are more pegs than holes, it's more efficient to start with holes and look for pegs */
#if 0
    int num_pegs = INIT_NUM_PEGS - moves_taken;
    int num_holes = INIT_NUM_HOLES + moves_taken;
    if (num_pegs < num_holes) {
#else
    if (1) {
#endif
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
        /* TODO: this code doesn't work for some reason? */
        for (r=0; r<NROWS; r++) {
            for (c=0; c<NCOLS; c++) {
                int is_hole = (grid[r][c] == HOLE) ? 1 : 0;
                if (!is_hole)
                    continue;
                /* NOTE: move directions are inverse when looking at holes */
                if ((RIGHT_ONE(r,c) == PEG) && (RIGHT_TWO(r,c) == PEG)) {
                    push_move(r, c+2, DIR_LEFT);
                    n++;
                }
                if ((LEFT_ONE(r,c) == PEG) && (LEFT_TWO(r,c) == PEG)) {
                    push_move(r, c-2, DIR_RIGHT);
                    n++;
                }
                if ((DOWN_ONE(r,c) == PEG) && (DOWN_TWO(r,c) == PEG)) {
                    push_move(r+2, c, DIR_UP);
                    n++;
                }
                if ((UP_ONE(r,c) == PEG) && (UP_TWO(r,c) == PEG)) {
                    push_move(r-2, c, DIR_DOWN);
                    n++;
                }
            }
        }
    }
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
#if 0
    moves_taken++;
#endif
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
#if 0
    moves_taken--;
#endif
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
 */
void solve()
{
    int bail = 0;
    while (1) {
        int n_avail_moves;
        assert(bail++ < 10000000);
        if (check_win()) {
            return;
        }
        n_avail_moves = get_moves();
        if ((n_avail_moves > 0) && win_still_possible()) {
            verbose && (print_top_move(),1);
            do_top_move();
            verbose && (print_grid(),1);
        } else {
            do {
                verbose && (printf("undoing: "),1);
                verbose && (print_top_move(),1);
                undo_top_move();
                verbose && (print_grid(),1);
                pop();
            } while (top_move_tried());
            verbose && (print_top_move(),1);
            do_top_move();
            verbose && (print_grid(),1);
        }
    }
}

int main()
{
    solve();
    print_taken_moves();
    return 0;
}
