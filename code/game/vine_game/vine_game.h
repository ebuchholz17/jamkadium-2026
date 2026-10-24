#ifndef VINE_GAME_H
#define VINE_GAME_H

#include "../gng_types.h"

#define CELL_DIM 12

typedef struct scratch_mem_marker {
    void *marker;
} scratch_mem_marker;

typedef struct mine_cell {
    int row;
    int col;
    b32 open;
    b32 hasBomb;
    b32 flagged;
    int numAdjBombs;

    b32 seen;
    b32 hasTrimmers;
} mine_cell;

typedef mine_cell *mine_cell_ptr;
#define LIST_TYPE mine_cell_ptr
#include "../list.h"

typedef enum {
    VINE_GAME_STATE_TITLE_SCREEN,
    VINE_GAME_STATE_GAMEPLAY,
    VINE_GAME_STATE_GAME_OVER_SCREEN,
    VINE_GAME_STATE_WIN_SCREEN
} vine_game_state;

typedef enum {
    VINE_GAME_DIFFICULTY_EASY,
    VINE_GAME_DIFFICULTY_MEDIUM,
    VINE_GAME_DIFFICULTY_HARD
} vine_game_difficulty;

typedef struct vine_game {
    b32 isInitialized;
    b32 shouldReset;
    f32 gameTime;

    mine_cell *cells;
    i32 fieldWidth;
    i32 fieldHeight;
    i32 numBombs;

    vine_game_state state;

    b32 firstCellWasOpened;
    i32 score;
} vine_game;

#endif
