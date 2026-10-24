#include "vine_game.h"

#include "../gng_bool.h"
#include "../gng_platform.h"
#include "../gng_random.h"
#include "../gng_sprites.h"
#include "../gng_virtual_input.h"

vine_game *vg;
mem_arena *scratchMemory;

scratch_mem_marker pushScratchMemory (void) {
    scratch_mem_marker result = {};
    result.marker = scratchMemory->current;
    return result;
}

void popScratchMemory (scratch_mem_marker marker) {
    scratchMemory->current = marker.marker;
}

void setDifficulty (vine_game_difficulty difficulty) {
    switch (difficulty) {
        case VINE_GAME_DIFFICULTY_EASY: {
            vg->fieldWidth = 9;
            vg->fieldHeight = 9;
            vg->numBombs = 10;
        } break;
        case VINE_GAME_DIFFICULTY_MEDIUM: {
            vg->fieldWidth = 16;
            vg->fieldHeight = 16;
            vg->numBombs = 40;
        } break;
        case VINE_GAME_DIFFICULTY_HARD: {
            vg->fieldWidth = 30;
            vg->fieldHeight = 16;
            vg->numBombs = 99;
        } break;
    }
}

void initVineGame (vine_game *vineGame, mem_arena *memory) {
    vg = vineGame;
    vg->isInitialized = true;
}

b32 cellIsNextToStartingCell (mine_cell *cell, mine_cell *startingCell) {
    i32 rowDiff = startingCell->row - cell->row;
    i32 colDiff = startingCell->col - cell->col;
    return (rowDiff >= -1 && rowDiff <= 1) && (colDiff >= -1 && colDiff <= 1);
}

void placeBombs (mine_cell *firstCell) {
    for (i32 i = 0; i < vg->fieldHeight; ++i) {
        for (i32 j = 0; j < vg->fieldWidth; ++j) {
            i32 index = i * vg->fieldWidth + j;
            mine_cell *cell = &vg->cells[index];
            cell->row = i;
            cell->col = j;

            if (index < vg->numBombs && ! cellIsNextToStartingCell(cell, firstCell)) {
                cell->hasBomb = true;
            }
        }
    }

    // shuffle bombs
    i32 numCells = vg->fieldWidth * vg->fieldHeight;
    for (i32 i = numCells - 1; i >= 0; --i) {
        mine_cell *cell = &vg->cells[i];

        if (cellIsNextToStartingCell(cell, firstCell)) {
            continue;
        }

        i32 randomIndex = randomU32() % (numCells);
        mine_cell *randomCell = &vg->cells[randomIndex];

        if (cellIsNextToStartingCell(randomCell, firstCell)) {
            continue;
        }
    
        b32 temp = cell->hasBomb;
        cell->hasBomb = randomCell->hasBomb;
        randomCell->hasBomb = temp;
    }

    for (i32 cellIndex = 0; cellIndex < vg->fieldWidth * vg->fieldHeight; ++cellIndex) {
        mine_cell *cell = &vg->cells[cellIndex];
        cell->numAdjBombs = 0;

        for (i32 i = -1; i <= 1; ++i) {
            for (i32 j = -1; j <= 1; ++j) {
                i32 row = cell->row + i;
                i32 col = cell->col + j;

                if ((row == cell->row && col == cell->col )|| 
                    row < 0 || row >= vg->fieldHeight ||
                    col < 0 || col >= vg->fieldWidth) 
                {
                    continue;
                }

                mine_cell *neighbor = &vg->cells[row * vg->fieldWidth + col];
                if (neighbor->hasBomb) {
                    cell->numAdjBombs++;
                }
            }
        }
    }
}

void startGameWithDifficulty (vine_game_difficulty difficulty, mem_arena *memory) {
    setDifficulty(difficulty);

    vg->cells = allocMemory(memory, sizeof(mine_cell) * vg->fieldWidth * vg->fieldHeight);


    // add powerups (will be trimmers in this game)
    //for (int i = numCellsLessLastRows - 1; i >= 0; --i) {

    //    mine_cell *cell = &game->cells[i];
    //    if (!cell->hasBomb) {
    //        float roll = randomFloat();

    //        float sWeight = 0.01f;
    //        float wWeight = 0.01f;
    //        float lWeight = 0.01f;
    //        float healthWeight = 0.02f;

    //        if (roll < sWeight) {
    //            cell->hasSPower = true;
    //        }
    //        else if (roll < sWeight + wWeight) {
    //            cell->hasWPower = true;
    //        }
    //        else if (roll < sWeight + wWeight + lWeight) {
    //            cell->hasLPower = true;
    //        }
    //        else if (roll < sWeight + wWeight + lWeight + healthWeight) {
    //            cell->hasExtraHealth = true;
    //        }

    //    }
    //    int randomIndex = randomUint() % (numCellsLessLastRows);
    //    mine_cell *randomCell = &game->cells[randomIndex];
    //}

}

void openAllNearbyZeroValueCells (mine_cell *startCell, mem_arena *tempMemory, b32 awardScore) {
    scratch_mem_marker m = pushScratchMemory();

    mine_cell_ptr_list cellsToOpen = mine_cell_ptr_listInit(tempMemory, vg->fieldWidth * vg->fieldHeight);
    mine_cell_ptr_listPush(&cellsToOpen, startCell);

    for (int cellIndex = 0; cellIndex < vg->fieldWidth * vg->fieldHeight; ++cellIndex) {
        mine_cell *cell = &vg->cells[cellIndex];
        cell->seen = false;
    }

    while (cellsToOpen.numValues > 0) {
        mine_cell *currentCell = cellsToOpen.values[0];
        currentCell->open = true;
        if (awardScore) {
            vg->score += 10;
        }
        mine_cell_ptr_listSplice(&cellsToOpen, 0);

        if (currentCell->numAdjBombs == 0) {
            // TODO(ebuchholz): condense all these -1 to 1 searches somehow
            for (int i = -1; i <= 1; ++i) {
                for (int j = -1; j <= 1; ++j) {
                    int row = currentCell->row + i;
                    int col = currentCell->col + j;

                    if ((row == currentCell->row && col == currentCell->col )|| 
                        row < 0 || row >= vg->fieldHeight ||
                        col < 0 || col >= vg->fieldWidth) 
                    {
                        continue;
                    }

                    mine_cell *neighbor = &vg->cells[row * vg->fieldWidth + col];
                    if (!neighbor->seen) {
                        neighbor->seen = true;
                        mine_cell_ptr_listPush(&cellsToOpen, neighbor);
                    }
                }
            }
        }
    }

    popScratchMemory(m);
}

void updateGameplay (mem_arena *memory, mem_arena *tempMemory, game_input *input) {
    // TODO(vg): pan screen

    mat3x3 gameTransform = spriteManPeekMatrix();
    mat3x3 invGameTransform = mat3x3Inv(gameTransform);
    vec3 localPointerPos = (vec3){ (f32)input->pointerX, (f32)input->pointerY, 1.0f };
    localPointerPos = vec3MatrixMul(invGameTransform, localPointerPos);

    i32 mouseRow = (i32)(localPointerPos.y / CELL_DIM);
    i32 mouseCol = (i32)(localPointerPos.x / CELL_DIM);

    b32 gameOver = false;

    if (mouseRow >= 0 && mouseRow < vg->fieldHeight && mouseCol >= 0 && mouseCol < vg->fieldWidth) {
        if (input->pointerJustDown) {
            mine_cell *cell = &vg->cells[mouseRow * vg->fieldWidth + mouseCol];
            if (!cell->open) {
                if (!cell->flagged) {
                    cell->open = true;
                    if (!vg->firstCellWasOpened) {
                        vg->firstCellWasOpened = true;
                        cell->hasBomb = false;
                        placeBombs(cell);
                        openAllNearbyZeroValueCells(cell, tempMemory, false);
                    }
                    else {
                        // Game over!
                        if (cell->hasBomb) {
                            gameOver = true;
                            //playSound("wormhole_revealed", gameSounds, assets);
                        }
                        else {
                            //playSound("click", gameSounds, assets);
                            vg->score += 10;
                            if (cell->numAdjBombs == 0) {
                                openAllNearbyZeroValueCells(cell, tempMemory, true);
                            }
                        }
                    }
                }
            }
            else {
                // trimmers
                if (cell->hasTrimmers) {
                    // TODO(vg)
                }
                else if (!cell->hasBomb && cell->numAdjBombs > 0) {
                    i32 numFlaggedNeighbors = 0;
                    for (i32 i = -1; i <= 1; ++i) {
                        for (i32 j = -1; j <= 1; ++j) {
                            i32 row = cell->row + i;
                            i32 col = cell->col + j;

                            if ((row == cell->row && col == cell->col )|| 
                                row < 0 || row >= vg->fieldHeight ||
                                col < 0 || col >= vg->fieldWidth) 
                            {
                                continue;
                            }

                            mine_cell *neighbor = &vg->cells[row * vg->fieldWidth + col];
                            if (neighbor->flagged) {
                                numFlaggedNeighbors++;
                            }
                        }
                    }
                    if (numFlaggedNeighbors == cell->numAdjBombs) {
                        //playSound("click", gameSounds, assets);

                        for (i32 i = -1; i <= 1; ++i) {
                            for (i32 j = -1; j <= 1; ++j) {
                                i32 row = cell->row + i;
                                i32 col = cell->col + j;

                                if ((row == cell->row && col == cell->col )|| 
                                    row < 0 || row >= vg->fieldHeight ||
                                    col < 0 || col >= vg->fieldWidth) 
                                {
                                    continue;
                                }

                                mine_cell *neighbor = &vg->cells[row * vg->fieldWidth + col];
                                if (!neighbor->open && !neighbor->flagged) {
                                    neighbor->open = true;
                                    vg->score += 10;

                                    if (neighbor->hasBomb) {
                                        // Game Over! TODO(vg)
                                        gameOver = true;
                                    }
                                    else if (neighbor->numAdjBombs == 0) {
                                        openAllNearbyZeroValueCells(neighbor, tempMemory, true);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        // TODO(vg) right click
        else if (input->aKey.justPressed) {
            mine_cell *cell = &vg->cells[mouseRow * vg->fieldWidth + mouseCol];
            if (!cell->open) {
                cell->flagged = !cell->flagged;
                //playSound("click", gameSounds, assets);
            }
        }
    }

    if (gameOver) {
        // TODO(vg) game over state
        vg->state = VINE_GAME_STATE_TITLE_SCREEN;
    }
}

void updateVineGame (vine_game *bg, game_input *input, 
                       virtual_input *vInput, f32 dt, plat_api platAPI, mem_arena *memory,
                       mem_arena *sm) 
{
    scratchMemory = sm;

    //mat3x3 gameTransform = spriteManPeekMatrix();
    //mat3x3 invGameTransform = mat3x3Inv(gameTransform);
    //vec3 localPointerPos = (vec3){ (f32)input->pointerX, (f32)input->pointerY, 1.0f };
    //localPointerPos = vec3MatrixMul(invGameTransform, localPointerPos);

    switch (vg->state) {
        case VINE_GAME_STATE_TITLE_SCREEN: {
            if (input->pointerJustDown) {
                vg->state = VINE_GAME_STATE_GAMEPLAY;
                startGameWithDifficulty(VINE_GAME_DIFFICULTY_HARD, memory);
                //playSound("start_sound", gameSounds, assets);
            }
        } break;
        case VINE_GAME_STATE_GAMEPLAY: {
            updateGameplay(memory, sm, input);
        } break;
        case VINE_GAME_STATE_GAME_OVER_SCREEN: {
        } break;
        case VINE_GAME_STATE_WIN_SCREEN: {
        } break;
    }
    vg->gameTime += dt;

}

f32 textWidth (sprite_text *text) {
    f32 width = 0;
    char *cursor = text->text;
    while (*cursor != 0) {
        bitmap_font_letter_coord c = spriteMan->fontLetterCoords[*cursor];
        width += c.advance;
        cursor++;
    }
    return width;
}

void centerText (sprite_text *text) {
    f32 width = textWidth(text);
    f32 offset = -((f32)width / 2.0f);
    text->x += offset;
    text->y -= 4.0f;
}

void drawVineGameGameplay (vine_game *vg, plat_api platAPI) {
    // TODO(vg) panning

    for (int i = 0; i < vg->fieldHeight; ++i) {
        for (int j = 0; j < vg->fieldWidth; ++j) {
            mine_cell *cell = &vg->cells[i * vg->fieldWidth + j];
            cell->row = i;
            cell->col = j;

            if (!cell->open) {
                sprite cellSprite = defaultSprite();
                cellSprite.atlasKey = "game_atlas";
                cellSprite.frameKey = "closed_cell";
                cellSprite.pos.x = (float)(cell->col * CELL_DIM);
                cellSprite.pos.y = (float)(cell->row * CELL_DIM);

                spriteManAddSprite(cellSprite);

                if (cell->flagged) {
                    cellSprite.frameKey = "flag";
                    spriteManAddSprite(cellSprite);
                }
            }
            else {
                sprite cellSprite = defaultSprite();
                cellSprite.atlasKey = "game_atlas";
                cellSprite.frameKey = "open_cell";
                cellSprite.pos.x = (float)(cell->col * CELL_DIM);
                cellSprite.pos.y = (float)(cell->row * CELL_DIM);

                if (cell->hasBomb) {
                    cellSprite.frameKey = "bomb";
                    spriteManAddSprite(cellSprite);
                }
                else {
                    if (cell->numAdjBombs > 0) {
                        cellSprite.frameKey = tempStringFromI32(cell->numAdjBombs);
                        spriteManAddSprite(cellSprite);
                    }
                    // TODO(vg) trimmers
                    //if (cell->hasSPower) {
                    //    sprite pickupSprite = Sprite();
                    //    pickupSprite.pos.x = (float)(cell->col * CELL_DIM);
                    //    pickupSprite.pos.y = (float)(cell->row * CELL_DIM) + shiftAmount;
                    //    addSprite(pickupSprite, assets, "game_atlas", "s_power_grid", spriteList);
                    //}
                }
            }
        }
    }
}

void drawVineGame (vine_game *vg, plat_api platAPI) {
    switch (vg->state) {
        case VINE_GAME_STATE_TITLE_SCREEN: {
            sprite_text labelText = {
                .text = "vinesweeper",
                .fontKey = "font",
                .x = 320.0f,
                .y = 160.0f
            };
            centerText(&labelText);
            spriteManAddText(labelText);

            {
                sprite_text labelText = {
                    .text = "Click to continue",
                    .fontKey = "font",
                    .x = 320.0f,
                    .y = 200.0f
                };
                centerText(&labelText);
                spriteManAddText(labelText);
            }
        } break;
        case VINE_GAME_STATE_GAMEPLAY: {
            drawVineGameGameplay(vg, platAPI);
        } break;
        case VINE_GAME_STATE_GAME_OVER_SCREEN: {

        } break;
        case VINE_GAME_STATE_WIN_SCREEN: {

        } break;
    }
}
