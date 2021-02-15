#!/usr/bin/env python3

import random
import sys

DIR_RIGHT = 0
DIR_LEFT = 1
DIR_UP = 2
DIR_DOWN = 3

dir_strs = [
    "right",
    "left",
    "up",
    "down"
]

MAX_TRIES = 100000
MAX_MOVES = 10000

PEG = "o"
HOLE= "."
INVALID = " "

max_fanout = 0
max_stack_sp =0

grid_english = [
    list('         '),
    list('   ooo   '),
    list('   ooo   '),
    list(' ooooooo '),
    list(' ooo.ooo '),
    list(' ooooooo '),
    list('   ooo   '),
    list('   ooo   '),
    list('         ')
]

grid_test = [
    list('         '),
    list('   ooo   '),
    list('   ooo   '),
    list(' .oooooo '),
    list(' .oooooo '),
    list(' .oo.ooo '),
    list('   ooo   '),
    list('   ...   '),
    list('         ')
]

grid_cross = [
    list('         '),
    list('   ...   '),
    list('   .o.   '),
    list(' ..ooo.. '),
    list(' ...o... '),
    list(' ...o... '),
    list('   ...   '),
    list('   ...   '),
    list('         ')
]

grid_init = grid_test

NROWS = len(grid_init)
NCOLS = len(grid_init[0])

grid = None
npegs = None
nholes = None

avail_move_stack = list()

def right_one(grid, r, c):
    try:
        x = grid[r][c+1]
    except IndexError:
        return INVALID
    return x

def right_two(grid, r, c):
    try:
        x = grid[r][c+2]
    except IndexError:
        return INVALID
    return x

def left_one(grid, r, c):
    try:
        x = grid[r][c-1]
    except IndexError:
        return INVALID
    return x

def left_two(grid, r, c):
    try:
        x = grid[r][c-2]
    except IndexError:
        return INVALID
    return x

def up_one(grid, r, c):
    try:
        x = grid[r-1][c]
    except IndexError:
        return INVALID
    return x

def up_two(grid, r, c):
    try:
        x = grid[r-2][c]
    except IndexError:
        return INVALID
    return x

def down_one(grid, r, c):
    try:
        x = grid[r+1][c]
    except IndexError:
        return INVALID
    return x

def down_two(grid, r, c):
    try:
        x = grid[r+2][c]
    except IndexError:
        return INVALID
    return x

def print_grid(g):
    print("\n".join(["".join(row) for row in g]))

def print_taken_moves():
    for move in avail_move_stack:
        d = move['dir']
        if move['taken']:
            print("%d %d %s" % (move['row'], move['col'], dir_strs[d]))

def num_pegs(grid):
    n = 0
    for row in grid:
        for c in row:
            if c == PEG:
                n += 1
    return n

def num_holes(grid):
    n = 0
    for row in grid:
        for c in row:
            if c == HOLE:
                n += 1
    return n

def reset():
    global grid
    global npegs
    global nholes
    global avail_move_stack
    avail_move_stack = list()
    grid = list()
    for row in grid_init:
        grid.append(row[:])
    npegs = num_pegs(grid)
    nholes = num_holes(grid)

def check_win():
    npegs = 0
    for row in grid:
        for c in row:
            if c == PEG:
                npegs += 1
                if npegs > 1:
                    return False
    return True

def push_move(r, c, d):
    global max_stack_sp
    avail_move_stack.append({'row': r, 'col': c, 'dir': d, 'taken': False})
    if len(avail_move_stack) > max_stack_sp:
        max_stack_sp = len(avail_move_stack)

def pop_move():
    avail_move_stack.pop()

def shuffle_new_moves(n):
    ms = avail_move_stack
    for i in range (0, n):
        idx = random.randint(i, n-1)
        ms[-n+i], ms[-n+idx] = ms[-n+idx], ms[-n+i]

def get_moves():
    global max_fanout
    n=0
    for r in range(0, NROWS):
        for c in range(0, NCOLS):
            if grid[r][c] != PEG:
                continue
            if ((right_one(grid,r,c) == PEG) and (right_two(grid,r,c) == HOLE)):
                push_move(r, c, DIR_RIGHT)
                n += 1
            if ((left_one(grid,r,c) == PEG) and (left_two(grid,r,c) == HOLE)):
                push_move(r, c, DIR_LEFT)
                n += 1
            if ((down_one(grid,r,c) == PEG) and (down_two(grid,r,c) == HOLE)):
                push_move(r, c, DIR_DOWN)
                n += 1
            if ((up_one(grid,r,c) == PEG) and (up_two(grid,r,c) == HOLE)):
                push_move(r, c, DIR_UP)
                n += 1
    shuffle_new_moves(n)
    if n > max_fanout:
        max_fanout = n
    return n

def undo_top_move():
    global npegs
    global nholes
    top_move = avail_move_stack[-1]
    r = top_move['row']
    c = top_move['col']
    d = top_move['dir']
    grid[r][c] = PEG
    if d == DIR_UP:
        grid[r-1][c] = PEG
        grid[r-2][c] = HOLE
    elif d == DIR_DOWN:
        grid[r+1][c] = PEG
        grid[r+2][c] = HOLE
    elif d == DIR_LEFT:
        grid[r][c-1] = PEG
        grid[r][c-2] = HOLE
    elif d == DIR_RIGHT:
        grid[r][c+1] = PEG
        grid[r][c+2] = HOLE
    npegs += 1
    nholes -= 1

def do_top_move():
    global npegs
    global nholes
    top_move = avail_move_stack[-1]
    r = top_move['row']
    c = top_move['col']
    d = top_move['dir']
    grid[r][c] = HOLE
    if d == DIR_UP:
        grid[r-1][c] = HOLE
        grid[r-2][c] = PEG
    elif d == DIR_DOWN:
        grid[r+1][c] = HOLE
        grid[r+2][c] = PEG
    elif d == DIR_LEFT:
        grid[r][c-1] = HOLE
        grid[r][c-2] = PEG
    elif d == DIR_RIGHT:
        grid[r][c+1] = HOLE
        grid[r][c+2] = PEG
    top_move['taken'] = True
    npegs -= 1
    nholes += 1

def top_move_tried():
    return avail_move_stack[-1]['taken']

def solve():
    for nresets in range(0, MAX_TRIES):
        reset()
        if nresets % 10 == 0:
            sys.stdout.write('.')
            sys.stdout.flush()
        for nmoves_tried in range(0, MAX_MOVES):
            if check_win():
                return
            get_moves()
            while (top_move_tried()):
                undo_top_move()
                pop_move()
            do_top_move()
    assert False, "no solution found"

if __name__ == "__main__":
    init_npegs = num_pegs(grid_init)
    solve()
    sys.stdout.write('\n')
    print_grid(grid)
    print("max fanout: %d" % max_fanout)
    print("max stack usage: %d" % max_stack_sp)
    print_taken_moves()
    taken_moves = [move for move in avail_move_stack if move['taken']]
    assert len(taken_moves) == init_npegs-1
