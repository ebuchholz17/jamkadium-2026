#include "vine_game.h"

#include "../gng_bool.h"
#include "../gng_platform.h"
#include "../gng_random.h"
#include "../gng_sprites.h"
#include "../gng_virtual_input.h"

#include "atan.c"

vine_game *vg;
mem_arena *scratchMemory;

f32 arctangent2(f32 y, f32 x) {
    if (x > 0.0f) {
        return arctangent(y / x);
    }
    else if (x < 0.0f && y >= 0.0f) {
        return arctangent(y / x) + PI;
    }
    else if (x < 0.0f && y < 0.0f) {
        return arctangent(y / x) - PI;
    }
    else if (x == 0.0f && y > 0.0f) {
        return PI / 2.0f;
    }
    else if (x == 0.0f && y < 0.0f) {
        return -PI / 2.0f;
    }
    else {
        return 0.0f;
    }
}

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
            vg->numTrimmersInField = 3;
        } break;
        case VINE_GAME_DIFFICULTY_MEDIUM: {
            vg->fieldWidth = 16;
            vg->fieldHeight = 16;
            vg->numBombs = 40;
            vg->numTrimmersInField = 6;
        } break;
        case VINE_GAME_DIFFICULTY_HARD: {
            vg->fieldWidth = 50;
            vg->fieldHeight = 28;
            vg->numBombs = 200;
            vg->numTrimmersInField = 12;
        } break;
    }
}

void initVineGame (vine_game *vineGame, mem_arena *memory) {
    vg = vineGame;
    vg->isInitialized = true;
    vg->vineNodeList = vine_node_listInit(memory, 2000);
    vg->cameraZoom = 1.0f; // TODO(vg) change per difficulty?

    // Put vine in the 0 slot, not to be used
    vine_node v = {};
    v.id.val = vg->vineNodeList.numValues;
    vine_node_listPush(&vg->vineNodeList, v);

    // TODO(vg) make per difficulty?
    vg->vineSeekDelay = 3.5f;
    vg->vineSplitLifeTime = 15;
    vg->numTrimmers = 0;
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

vine_node *spawnNewVine (void) {
    vine_node *result = 0;
    for (i32 i = 1; i < vg->vineNodeList.numValues; i++) {
        vine_node *vine = &vg->vineNodeList.values[i];
        if (!vine->active) {
            result = vine;
        }
    }

    if (!result) {
        vine_node b = {};
        b.id.val = vg->vineNodeList.numValues;
        vine_node_listPush(&vg->vineNodeList, b);
        result = &vg->vineNodeList.values[vg->vineNodeList.numValues - 1];
    }

    // reset bubble
    entity_id id = result->id;
    *result = (vine_node){};
    result->id = id;

    result->active = true;

    return result;
}

vine_node *getVineByID (entity_id id) {
    if (id.val == 0) {
        return 0;
    }

    if (id.val >= 0 && id.val < vg->vineNodeList.numValues) {
        vine_node *result = &vg->vineNodeList.values[id.val];
        if (result->id.gen == id.gen) {
            return result;
        }
    }
    return 0;
}

void destroyVine (vine_node *vine) {
    vine->active = false;
    vine->id.gen++;
}

void sortCells (mine_cell_ptr_list *cells) {
    for (int i = 0; i < cells->numValues; ++i) {
        for (int j = i; j > 0; --j) {
            mine_cell *first = cells->values[j-1];
            mine_cell *second = cells->values[j];
            if (second->sortValue < first->sortValue) {
                cells->values[j-1] = second;
                cells->values[j] = first;
            }
        }
    }
}

// https://stackoverflow.com/questions/3581528/how-is-the-square-root-function-implemented
f32 squareRoot (f32 n){
  // Max and min are used to take into account numbers less than 1
    f32 lo = 1 < n ? 1 : n;
    f32 hi = 1 > n ? 1 : n;
    f32 mid;

  // Update the bounds to be off the target by a factor of 10
  while(100 * lo * lo < n) lo *= 10;
  while(0.01 * hi * hi > n) hi *= 0.1;

  for(int i = 0 ; i < 100 ; i++){
      mid = (lo+hi)/2;
      if(mid*mid == n) return mid;
      if(mid*mid > n) hi = mid;
      else lo = mid;
  }
  return mid;
}



void coordKeyFromRowCol (i32 row, i32 col, char *key) {
    for (i32 i = 0; i < 16; i++) {
        key[i] = 'x';
    }
    stringFromI32Unterminated(row, key, 4);
    stringFromI32Unterminated(col, key + 6, 4);
    key[15] = 0;
}

void pathFindTargetRowColFromVineNode (i32 targetRow, i32 targetCol, vine_node *vine, i32 *outRow, i32 *outCol) {
    *outRow = -1;
    *outCol = -1;

    if (vine->row == targetRow && vine->col == targetCol) {
        *outRow = vine->row;
        *outCol = vine->col;
        return;
    }

    scratch_mem_marker m = pushScratchMemory();

    pathfinding_node startNode = {};
    startNode.coords.row = vine->row;
    startNode.coords.col = vine->col;
    startNode.coords.key = allocMemory(scratchMemory, 16 * sizeof(char));
    coordKeyFromRowCol(startNode.coords.row, startNode.coords.col, startNode.coords.key);

    f32 diffX = (f32)(targetCol - startNode.coords.col);
    f32 diffY = (f32)(targetRow - startNode.coords.row);
    startNode.value = squareRoot(diffX * diffX + diffY * diffY);

    pathfinding_node_list nodes = pathfinding_node_listInit(scratchMemory, vg->fieldWidth * vg->fieldHeight);
    pathfinding_node_listPush(&nodes, startNode);

    cell_coord dummyCoord = {
        .row = -1,
        .col = -1
    };
    dummyCoord.key = allocMemory(scratchMemory, 16 * sizeof(char));
    coordKeyFromRowCol(dummyCoord.row, dummyCoord.col, dummyCoord.key);

    cell_coord_hash_map seenPoints = cell_coord_hash_mapInit(scratchMemory, 4 * vg->fieldWidth * vg->fieldHeight);
    cell_coord_hash_mapStore(&seenPoints, dummyCoord, dummyCoord.key);

    // TODO(ebuchholz): try inserting nodes after processed ones, to avoid splicing?
    while (nodes.numValues > 0) {
        pathfinding_node node = nodes.values[0];
        pathfinding_node_listSplice(&nodes, 0);

        if (node.coords.col == targetCol && node.coords.row == targetRow) {
            cell_coord previousCoords = cell_coord_hash_mapGetVal(&seenPoints, node.coords.key);

            if (previousCoords.col == vine->col && previousCoords.row == vine->row) {
                *outRow = targetRow;
                *outCol = targetCol;
            }
            else {
                // return node after first
                cell_coord secondLastCoords = previousCoords;
                while (previousCoords.col != vine->col || previousCoords.row != vine->row) {
                    secondLastCoords = previousCoords;
                    previousCoords = cell_coord_hash_mapGetVal(&seenPoints, previousCoords.key);
                }
                *outRow = secondLastCoords.row;
                *outCol = secondLastCoords.col;
            }

            break;
        }
        else {
            for (int i = -1; i <= 1; ++i) {
                for (int j = -1; j <= 1; ++j) {
                    if (i == 0 && j == 0) {
                        continue;
                    }
                    if (i != 0 && j != 0) {
                        continue;
                    }

                    cell_coord coords = {};
                    coords.col = node.coords.col + j;
                    coords.row = node.coords.row + i;
                    coords.key = allocMemory(scratchMemory, 16 * sizeof(char));
                    coordKeyFromRowCol(coords.row, coords.col, coords.key);

                    if (coords.col < 0 || coords.col >= vg->fieldWidth ||
                        coords.row < 0 || coords.row >= vg->fieldHeight)
                    {
                        continue;
                    }

                    mine_cell nextMineCell = vg->cells[coords.row * vg->fieldWidth + coords.col];

                    vine_node *existingVine = getVineByID(nextMineCell.vineID);
                    if (!existingVine || !existingVine->active) {
                        cell_coord *existingCoord = cell_coord_hash_mapGetPtr(&seenPoints, coords.key);
                        if (!existingCoord) {
                            pathfinding_node nextNode = {};
                            nextNode.coords.col = coords.col;
                            nextNode.coords.row = coords.row;
                            nextNode.coords.key = allocMemory(scratchMemory, 16 * sizeof(char));
                            coordKeyFromRowCol(nextNode.coords.row, nextNode.coords.col, nextNode.coords.key);
                            diffX = (float)(targetCol - nextNode.coords.col);
                            diffY = (float)(targetRow - nextNode.coords.row);
                            nextNode.value = squareRoot(diffX * diffX + diffY * diffY);

                            // TODO(ebuchholz): log-based insert
                            if (nodes.numValues == 0) {
                                pathfinding_node_listPush(&nodes, nextNode);
                            }
                            else {
                                b32 inserted = false;
                                for (int index = 0; index < nodes.numValues; ++index) {
                                    pathfinding_node *existingNode = &nodes.values[index];
                                    if (nextNode.value < existingNode->value) {
                                        pathfinding_node_listInsert(&nodes, nextNode, index);
                                        inserted = true;
                                        break;
                                    }
                                }

                                if (!inserted) {
                                    pathfinding_node_listPush(&nodes, nextNode);
                                }
                            }
                            cell_coord_hash_mapStore(&seenPoints, node.coords, nextNode.coords.key);
                        }
                    }
                }
            }
        }
    }

    popScratchMemory(m);
}

void getAvailableNeighborCellsToVine (vine_node *vine, mine_cell **neighbors, i32 *outNumNeighbors) {
    i32 numNeighbors = 0;

    if (vine->col > 0) {
        mine_cell *leftCell = &vg->cells[vine->row * vg->fieldWidth + vine->col - 1];
        vine_node *leftVine = getVineByID(leftCell->vineID);
        if (!leftVine || !leftVine->active) {
            neighbors[numNeighbors++] = leftCell;
        }
    }
    if (vine->col < vg->fieldWidth - 1) {
        mine_cell *rightCell = &vg->cells[vine->row * vg->fieldWidth + vine->col + 1];
        vine_node *rightVine = getVineByID(rightCell->vineID);
        if (!rightVine || !rightVine->active) {
            neighbors[numNeighbors++] = rightCell;
        }
    }
    if (vine->row > 0) {
        mine_cell *topCell = &vg->cells[(vine->row - 1) * vg->fieldWidth + vine->col];
        vine_node *topVine = getVineByID(topCell->vineID);
        if (!topVine || !topVine->active) {
            neighbors[numNeighbors++] = topCell;
        }
    }
    if (vine->row < vg->fieldHeight - 1) {
        mine_cell *bottomCell = &vg->cells[(vine->row + 1) * vg->fieldWidth + vine->col];
        vine_node *bottomVine = getVineByID(bottomCell->vineID);
        if (!bottomVine || !bottomVine->active) {
            neighbors[numNeighbors++] = bottomCell;
        }
    }

    for (i32 i = numNeighbors - 1; i >= 0; --i) {
        mine_cell *cell = neighbors[i];

        i32 randomIndex = randomU32() % (numNeighbors);
        mine_cell *randomCell = neighbors[randomIndex];

        neighbors[randomIndex] = cell;
        neighbors[i] = randomCell;
    }

    *outNumNeighbors = numNeighbors;
}

void determineTargetBombForVine (vine_node *vine) {
    scratch_mem_marker m = pushScratchMemory();

    mine_cell_ptr_list cellsToSeek = mine_cell_ptr_listInit(scratchMemory, vg->numBombs);

    switch (vine->bombStrat) {
        case VINE_TARGET_BOMB_STRATEGY_RANDOM_DIR: {

            f32 randomAngle = randomF32() * 2.0f * PI;
            f32 range = (1.0f / 3.0f) * PI;
            vec2 vinePos = {
                .x = vine->col * CELL_DIM,
                .y = vine->row * CELL_DIM
            };
            for (int i = 0; i < vg->fieldHeight; ++i) {
                for (int j = 0; j < vg->fieldWidth; ++j) {
                    mine_cell *cell = &vg->cells[i * vg->fieldWidth + j];
                    vine_node *cellsVine = getVineByID(cell->vineID);
                    if (cell->hasBomb && !cellsVine) {
                        vec2 cellPos = {
                            .x = cell->col * CELL_DIM,
                            .y = cell->row * CELL_DIM
                        };
                        vec2 diff = vec2Subtract(cellPos, vinePos);
                        f32 angle = arctangent2(diff.y, diff.x);
                        if (angle < 0.0f) {
                            angle += 2.0f * PI;
                        }
                        f32 angleDiff = randomAngle - angle;
                        if (angleDiff < 0.0f) {
                            angleDiff = -angleDiff;
                        }
                        f32 maybeShorterAngleDiff = 2.0f * PI - angleDiff;
                        angleDiff = maybeShorterAngleDiff < angleDiff ? maybeShorterAngleDiff : angleDiff;
                        if (angleDiff <= range) {
                            cell->sortValue = vec2Length(diff);
                            mine_cell_ptr_listPush(&cellsToSeek, cell);
                        }
                    }
                }
            }

        } break;
        case VINE_TARGET_BOMB_STRATEGY_ORTHOGONAL_DIR: {

        } break;
        case VINE_TARGET_BOMB_STRATEGY_NEAREST: {
            vec2 vinePos = {
                .x = vine->col * CELL_DIM,
                .y = vine->row * CELL_DIM
            };
            for (int i = 0; i < vg->fieldHeight; ++i) {
                for (int j = 0; j < vg->fieldWidth; ++j) {
                    mine_cell *cell = &vg->cells[i * vg->fieldWidth + j];
                    vine_node *cellsVine = getVineByID(cell->vineID);
                    if (cell->hasBomb && !cellsVine) {
                        vec2 cellPos = {
                            .x = cell->col * CELL_DIM,
                            .y = cell->row * CELL_DIM
                        };
                        vec2 diff = vec2Subtract(cellPos, vinePos);
                        cell->sortValue = vec2Length(diff);
                        mine_cell_ptr_listPush(&cellsToSeek, cell);
                    }
                }
            }

        } break;
        case VINE_TARGET_BOMB_STRATEGY_RANDOM: {

        } break;
    }

    sortCells(&cellsToSeek);

    b32 foundTarget = false;
    for (i32 i = 0; i < cellsToSeek.numValues; i++) {
        mine_cell *targetCell = cellsToSeek.values[i];
        i32 nextRow;
        i32 nextCol;
        pathFindTargetRowColFromVineNode(targetCell->row, targetCell->col, vine, &nextRow, &nextCol);
        if (nextRow == -1 || nextCol == -1) {
            continue;
        }
        vine->targetRow = targetCell->row;
        vine->targetCol = targetCell->col;
        foundTarget = true;
        break;
    }

    if (!foundTarget) {
        mine_cell *neighbors[4];
        i32 numNeighbors = 0;
        getAvailableNeighborCellsToVine(vine, neighbors, &numNeighbors);

        if (numNeighbors > 0) {
            vine->targetRow = neighbors[0]->row;
            vine->targetCol = neighbors[0]->col;
        }
    }


    popScratchMemory(m);
}

void spawnTrimmers (void) {
    i32 numTrimmersPlaced = 0;

    scratch_mem_marker m = pushScratchMemory();

    mine_cell **validCells = allocMemory(scratchMemory, sizeof(mine_cell *) * vg->fieldWidth * vg->fieldHeight);
    i32 numValidCells = 0;

    for (int i = 0; i < vg->fieldHeight; ++i) {
        for (int j = 0; j < vg->fieldWidth; ++j) {
            mine_cell *cell = &vg->cells[i * vg->fieldWidth + j];
            vine_node *vine = getVineByID(cell->vineID);
            b32 hasVine = vine && vine->active;
            if (!cell->hasBomb && cell->numAdjBombs == 0 && !hasVine) {
                validCells[numValidCells++] = cell;
            }
        }
    }

    for (i32 i = numValidCells - 1; i >= 0; --i) {
        mine_cell *cell = validCells[i];

        i32 randomIndex = randomU32() % (numValidCells);
        mine_cell *randomCell = validCells[randomIndex];
    
        validCells[randomIndex] = cell;
        validCells[i] = randomCell;
    }

    for (i32 i = 0; i < vg->numTrimmersInField && i < numValidCells; i++) {
        mine_cell *cell = validCells[i];
        cell->hasTrimmers = true;
    }

    popScratchMemory(m);
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

void openCell (mine_cell *cell) {
    cell->open = true;
    vg->score += 10;

    if (cell->hasBomb) {
        vg->openedCellCausedGameOver = true;
    }
    else if (cell->numAdjBombs == 0) {
        openAllNearbyZeroValueCells(cell, scratchMemory, true);
    }
}

void openCellWithVine (mine_cell *cell, vine_node *vine) {
    if (cell->flagged && cell->hasBomb) {
        // TODO(vg) make flower
    }
    else {
        openCell(cell);
    }
}

void growVine (vine_node *vineNode) {
    i32 nextRow;
    i32 nextCol;
    pathFindTargetRowColFromVineNode(vineNode->targetRow, vineNode->targetCol, vineNode, &nextRow, &nextCol);

    if (nextRow != -1 && nextCol != -1) {
        vineNode->isHead = false;

        mine_cell *cell = &vg->cells[nextRow * vg->fieldWidth + nextCol];

        if (vineNode->lifeTime >= vg->vineSplitLifeTime) {
            mine_cell *neighbors[4];
            i32 numNeighbors = 0;
            getAvailableNeighborCellsToVine(vineNode, neighbors, &numNeighbors);

            mine_cell *cellsToSpawnVine[2];
            i32 numCellsToSpawnVinesIn = 0;
            for (i32 i = 0; i < numNeighbors; i++) {
                mine_cell *neighbor = neighbors[i];
                if (neighbor != cell) {
                    cellsToSpawnVine[numCellsToSpawnVinesIn++] = neighbor;
                }
                if (numCellsToSpawnVinesIn == 2) {
                    break; // probably unnecessary, because the front cell and prev cell are
                           // excluded
                }
            }

            // loosen restriction to not grow forward if couldn't find 2 spots
            if (numCellsToSpawnVinesIn != 2) {
                numCellsToSpawnVinesIn = 0;
                for (i32 i = 0; i < numNeighbors; i++) {
                    mine_cell *neighbor = neighbors[i];
                    cellsToSpawnVine[numCellsToSpawnVinesIn++] = neighbor;
                    if (numCellsToSpawnVinesIn == 2) {
                        break; // probably unnecessary, because the front cell and prev cell are
                               // excluded
                    }
                }
            }

            vineNode->isRoot = true;

            for (i32 i = 0; i < numCellsToSpawnVinesIn; i++) {
                mine_cell *neighbor = cellsToSpawnVine[i];

                vine_node *newVine = spawnNewVine();
                newVine->row = neighbor->row;
                newVine->col = neighbor->col;
                newVine->isHead = true;
                newVine->bombStrat = VINE_TARGET_BOMB_STRATEGY_NEAREST; // TODO(vg) make orthoganal
                                                                        // to prev direction
                newVine->lifeTime = 0;

                neighbor->vineID = newVine->id;

                determineTargetBombForVine(newVine);

                newVine->seekTimer = vg->vineSeekDelay;

                newVine->prev = vineNode->id;
                vine_node *nextVine0 = getVineByID(vineNode->next0);
                if (nextVine0 == 0 || !nextVine0->active) {
                    vineNode->next0 = newVine->id;
                }
                else {
                    vineNode->next1 = newVine->id;
                }

                openCellWithVine(neighbor, newVine);
            }

        }
        else {
            vine_node *newVine = spawnNewVine();
            newVine->row = cell->row;
            newVine->col = cell->col;
            newVine->isHead = true;
            newVine->bombStrat = VINE_TARGET_BOMB_STRATEGY_NEAREST;
            newVine->lifeTime = vineNode->lifeTime + 1;

            cell->vineID = newVine->id;

            if (vineNode->targetRow == newVine->row && vineNode->targetCol == newVine->col) {
                determineTargetBombForVine(newVine);
            }
            else {
                newVine->targetRow = vineNode->targetRow;
                newVine->targetCol = vineNode->targetCol;
            }

            newVine->seekTimer = vg->vineSeekDelay;

            newVine->prev = vineNode->id;
            vineNode->next0 = newVine->id;

            openCellWithVine(cell, newVine);
        }
    }
    else {
        // TODO(vg) better tracking of others vines targets, etc.
        determineTargetBombForVine(vineNode);
    }
}

void startGameWithDifficulty (vine_game_difficulty difficulty, mem_arena *memory) {
    setDifficulty(difficulty);

    vg->cells = allocMemory(memory, sizeof(mine_cell) * vg->fieldWidth * vg->fieldHeight);

    for (int i = 0; i < vg->fieldHeight; ++i) {
        for (int j = 0; j < vg->fieldWidth; ++j) {
            mine_cell *cell = &vg->cells[i * vg->fieldWidth + j];
            cell->row = i;
            cell->col = j;
        }
    }

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


void trimVine (vine_node *vine) {
    if (!vine->isRoot) {
        vine_node *next0 = getVineByID(vine->next0);
        vine_node *next1 = getVineByID(vine->next1);

        destroyVine(vine);

        if (next0 && next0->active) {
            trimVine(next0);
        }
        if (next1 && next1->active) {
            trimVine(next1);
        }
    }
}

void useTrimmersOnVine (vine_node *vine) {
    if (!vine->isRoot) {
        vg->numTrimmers--;

        vine_node *next0 = getVineByID(vine->next0);
        vine_node *next1 = getVineByID(vine->next1);

        if (next0 && next0->active) {
            trimVine(next0);
        }
        if (next1 && next1->active) {
            trimVine(next1);
        }

        vine->seekTimer = vg->vineSeekDelay;
        vine->lifeTime = vg->vineSplitLifeTime;

        growVine(vine);
    }
}


void updateGameplay (mem_arena *memory, mem_arena *tempMemory, game_input *input, f32 dt) {
    if (input->rightArrow.down || input->dKey.down) {
        vg->cameraX += CAMERA_SPEED * dt;
    }
    if (input->leftArrow.down|| input->aKey.down) {
        vg->cameraX -= CAMERA_SPEED * dt;
    }
    if (input->upArrow.down|| input->wKey.down) {
        vg->cameraY -= CAMERA_SPEED * dt;
    }
    if (input->downArrow.down|| input->sKey.down) {
        vg->cameraY += CAMERA_SPEED * dt;
    }

    if (input->pointerJustDown) {
        vg->pointerDownPos = (vec2){
            .x = input->pointerX,
            .y = input->pointerY
        };
    }

    b32 startDragging = false;
    if (input->pointerDown) {
        if (!vg->dragging) {
            vec2 pointerDisp = vec2Subtract((vec2){.x = input->pointerX, .y = input->pointerY }, vg->pointerDownPos);
            if (vec2Length(pointerDisp) > 30.0f) {
                vg->dragging = true;
                vg->dragStartCameraPos = (vec2){
                    .x = vg->cameraX,
                    .y = vg->cameraY
                };
                startDragging = true;
            }
        }
    }

    spriteManPushTransform((sprite_transform){
        .pos = { .x = -vg->cameraX, .y = -vg->cameraY },
        .scale = vg->cameraZoom
    });

    mat3x3 gameTransform = spriteManPeekMatrix();
    mat3x3 invGameTransform = mat3x3Inv(gameTransform);
    vec3 localPointerPos = (vec3){ (f32)input->pointerX, (f32)input->pointerY, 1.0f };
    localPointerPos = vec3MatrixMul(invGameTransform, localPointerPos);

    if (input->pointerJustDown) {
        vg->gameSpacePointerDownPos = (vec2){
            .x = localPointerPos.x,
            .y = localPointerPos.y
        };
    }
    if (startDragging) {
        vg->gameSpacePointerDownPos = (vec2){
            .x = localPointerPos.x,
            .y = localPointerPos.y
        };
    }

    spriteManPopMatrix();

    if (vg->dragging) {
        vec2 gameSpaceDragStart = vg->gameSpacePointerDownPos;
        vec2 gameSpaceDragEnd = (vec2){
            .x = localPointerPos.x,
            .y = localPointerPos.y
        };
        vec2 cameraDisplacement = vec2Subtract(gameSpaceDragEnd, gameSpaceDragStart);
        vg->cameraX -= cameraDisplacement.x;
        vg->cameraY -= cameraDisplacement.y;
    }


    i32 mouseRow = (i32)(localPointerPos.y / CELL_DIM);
    i32 mouseCol = (i32)(localPointerPos.x / CELL_DIM);


    if (!vg->dragging) {
        if (mouseRow >= 0 && mouseRow < vg->fieldHeight && mouseCol >= 0 && mouseCol < vg->fieldWidth) {
            if (input->pointerJustUp) {
                mine_cell *cell = &vg->cells[mouseRow * vg->fieldWidth + mouseCol];
                if (!cell->open) {
                    if (!cell->flagged) {
                        cell->open = true;
                        if (!vg->firstCellWasOpened) {
                            vg->firstCellWasOpened = true;
                            cell->hasBomb = false;
                            placeBombs(cell);

                            vine_node *vine = spawnNewVine();
                            vine->row = cell->row;
                            vine->col = cell->col;
                            vine->isRoot = true;
                            vine->isHead = true;
                            vine->bombStrat = VINE_TARGET_BOMB_STRATEGY_RANDOM_DIR;
                            vine->lifeTime = 0;

                            cell->vineID = vine->id;

                            determineTargetBombForVine(vine);
                            vine->seekTimer = vg->vineSeekDelay;

                            openAllNearbyZeroValueCells(cell, tempMemory, true);
                            spawnTrimmers();
                        }
                        else {
                            openCell(cell);
                        }
                    }
                }
                else {
                    // trimmers
                    if (cell->hasTrimmers) {
                        cell->hasTrimmers = false;
                        vg->numTrimmers++;
                        // TODO(vg) SFX
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
                                        openCell(neighbor);
                                    }
                                }
                            }
                        }
                    }
                }
            }
            // TODO(vg) right click
            else if (input->pointer2JustUp) {

                mine_cell *cell = &vg->cells[mouseRow * vg->fieldWidth + mouseCol];

                b32 cellHasVine = false;
                vine_node *cellsVine = getVineByID(cell->vineID);
                if (cellsVine) {
                    cellHasVine = true;
                }

                if (cellHasVine && vg->numTrimmers > 0) {
                    useTrimmersOnVine(cellsVine);
                    cellsVine = 0;
                }
                else if (!cell->open) {
                    cell->flagged = !cell->flagged;
                    //playSound("click", gameSounds, assets);
                }
            }
        }
    }

    if (!input->pointerDown) {
        vg->dragging = false;
    }

    // grow vines
    for (i32 i = 0; i < vg->vineNodeList.numValues; i++) {
        vine_node *vine = &vg->vineNodeList.values[i];
        if (vine->active && vine->isHead) {
            vine->seekTimer -= dt;
            if (vine->seekTimer <= 0.0f) {
                growVine(vine);
            }
        }
    }

    if (vg->openedCellCausedGameOver) {
        vg->state = VINE_GAME_STATE_GAME_OVER_SCREEN;
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
            if (input->pointerJustUp) {
                vg->state = VINE_GAME_STATE_GAMEPLAY;
                startGameWithDifficulty(VINE_GAME_DIFFICULTY_HARD, memory);
                //playSound("start_sound", gameSounds, assets);
            }
        } break;
        case VINE_GAME_STATE_GAMEPLAY: {
            updateGameplay(memory, sm, input, dt);
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

i32 getVineRelativePositionIndexToNeighbor (vine_node *vine, vine_node *neighbor) {
    i32 xDiff = vine->col - neighbor->col;
    i32 yDiff = vine->row - neighbor->row;

    if (xDiff == 1) {
        return 0;
    }
    else if (xDiff == -1) {
        return 2;
    }
    else if (yDiff == 1) {
        return 1;
    }
    else if (yDiff == -1) {
        return 3;
    }
    return 0;
}

void setVineSpriteFrameAndAngleForRelativePositions (sprite *vineSprite, i32 prevPos, i32 nextPos) {
    switch (prevPos) {
        case 0: {
            switch (nextPos) {
                case 0: {
                } break;
                case 1: {
                    vineSprite->rotation = 0.25f;
                    vineSprite->frameKey = "vine_corner";
                } break;
                case 2: {
                    vineSprite->frameKey = "vine_straight";
                } break;
                case 3: {
                    vineSprite->frameKey = "vine_corner";
                } break;
            }
        } break;
        case 1: {
            switch (nextPos) {
                case 0: {
                    vineSprite->rotation = 0.25f;
                    vineSprite->frameKey = "vine_corner";
                } break;
                case 1: {
                } break;
                case 2: {
                    vineSprite->rotation = 0.5f;
                    vineSprite->frameKey = "vine_corner";
                } break;
                case 3: {
                    vineSprite->rotation = 0.25f;
                    vineSprite->frameKey = "vine_straight";
                } break;
            }
        } break;
        case 2: {
            switch (nextPos) {
                case 0: {
                    vineSprite->rotation = 0.75f;
                    vineSprite->frameKey = "vine_straight";
                } break;
                case 1: {
                    vineSprite->rotation = 0.5f;
                    vineSprite->frameKey = "vine_corner";
                } break;
                case 2: {
                } break;
                case 3: {
                    vineSprite->rotation = 0.75f;
                    vineSprite->frameKey = "vine_corner";
                } break;
            }
        } break;
        case 3: {
            switch (nextPos) {
                case 0: {
                    vineSprite->frameKey = "vine_corner";
                } break;
                case 1: {
                    vineSprite->rotation = 0.25f;
                    vineSprite->frameKey = "vine_straight";
                } break;
                case 2: {
                    vineSprite->rotation = 0.75f;
                    vineSprite->frameKey = "vine_corner";
                } break;
                case 3: {
                } break;
            }
        } break;
    }
}

void drawVineGameGameplay (vine_game *vg, plat_api platAPI) {
    // TODO(vg) panning
    spriteManPushTransform((sprite_transform){
        .pos = { .x = -vg->cameraX, .y = -vg->cameraY },
        .scale = vg->cameraZoom
    });

    // loop cells
    for (int i = 0; i < vg->fieldHeight; ++i) {
        for (int j = 0; j < vg->fieldWidth; ++j) {
            mine_cell *cell = &vg->cells[i * vg->fieldWidth + j];

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
                spriteManAddSprite(cellSprite);

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

            if (cell->hasTrimmers && cell->open) {
                sprite trimmersSprite = defaultSprite();
                trimmersSprite.atlasKey = "game_atlas";
                trimmersSprite.frameKey = "trimmers";
                trimmersSprite.scale = 0.65f;
                trimmersSprite.anchor.x = 0.5f;
                trimmersSprite.anchor.y = 0.5f;
                trimmersSprite.pos.x = (float)(cell->col * CELL_DIM) + CELL_DIM * 0.5f;
                trimmersSprite.pos.y = (float)(cell->row * CELL_DIM) + CELL_DIM * 0.5f;
                spriteManAddSprite(trimmersSprite);
            }
        }
    }

    // trimmers
    for (int i = 0; i < vg->fieldHeight; ++i) {
        for (int j = 0; j < vg->fieldWidth; ++j) {
            mine_cell *cell = &vg->cells[i * vg->fieldWidth + j];

            if (cell->hasTrimmers && cell->open) {
                sprite trimmersSprite = defaultSprite();
                trimmersSprite.atlasKey = "game_atlas";
                trimmersSprite.frameKey = "trimmers";
                trimmersSprite.scale = 1.15f;
                trimmersSprite.anchor.x = 0.5f;
                trimmersSprite.anchor.y = 0.5f;
                trimmersSprite.pos.x = (float)(cell->col * CELL_DIM) + CELL_DIM * 0.5f;
                trimmersSprite.pos.y = (float)(cell->row * CELL_DIM) + CELL_DIM * 0.5f;
                spriteManAddSprite(trimmersSprite);
            }
        }
    }

    for (i32 i = 0; i < vg->vineNodeList.numValues; i++) {
        vine_node *vine = &vg->vineNodeList.values[i];
        if (vine->active) {
            sprite vineSprite = defaultSprite();
            vineSprite.atlasKey = "game_atlas";

            vineSprite.pos.x = (float)(vine->col * CELL_DIM) + CELL_DIM * 0.5f;
            vineSprite.pos.y = (float)(vine->row * CELL_DIM) + CELL_DIM * 0.5f;
            vineSprite.anchor.x = 0.5f;
            vineSprite.anchor.y = 0.5f;

            if (vine->isRoot) {
                vineSprite.frameKey = "vine_root";
            }
            else if (vine->isHead) {
                vineSprite.frameKey = "vine_head_2";
                vine_node *neighbor = getVineByID(vine->prev);
                if (neighbor) {
                    i32 index = getVineRelativePositionIndexToNeighbor(vine, neighbor);
                    switch (index) {
                    case 0:
                        vineSprite.rotation = 0.25f;
                        break;
                    case 1:
                        vineSprite.rotation = 0.5f;
                        break;
                    case 2:
                        vineSprite.rotation = 0.75f;
                        break;
                    case 3:
                        vineSprite.rotation = 0.0f;
                        break;
                    }
                }
            }
            else {
                vine_node *prev = getVineByID(vine->prev);
                vine_node *next = getVineByID(vine->next0);
                if (prev && next) {
                    i32 prevIndex = getVineRelativePositionIndexToNeighbor(vine, prev);
                    i32 nextIndex = getVineRelativePositionIndexToNeighbor(vine, next);
                    setVineSpriteFrameAndAngleForRelativePositions(&vineSprite, prevIndex, nextIndex);
                }
            }
            spriteManAddSprite(vineSprite);

            // debug vine targets
            //vineSprite.tint = 0xff0000;
            //vineSprite.pos.x = (float)(vine->targetCol * CELL_DIM);
            //vineSprite.pos.y = (float)(vine->targetRow * CELL_DIM);
            //spriteManAddSprite(vineSprite);
        }
    }

    spriteManPopMatrix();

    // minimap
    spriteManPushTransform((sprite_transform){
        .pos = { .x = 0, .y = 300.0f },
        .scale = 1.0f / 8.0f
    });

    for (int i = 0; i < vg->fieldHeight; ++i) {
        for (int j = 0; j < vg->fieldWidth; ++j) {
            mine_cell *cell = &vg->cells[i * vg->fieldWidth + j];
            vine_node *vine = getVineByID(cell->vineID);

            sprite boxSprite = defaultSprite();
            boxSprite.textureKey = "minimap_box";
            boxSprite.scale = 1.0f;

            if (vine && vine->active) {
                boxSprite.tint = 0x0000ff;
            }
            else {
                if (!cell->open) {
                    if (cell->flagged) {
                        boxSprite.tint = 0xff0000;
                    }
                    else {
                        boxSprite.tint = 0xffffff;
                    }
                }
                else {
                    boxSprite.tint = 0x000000;
                }
            }

            boxSprite.pos.x = (float)(cell->col * 16.0f);
            boxSprite.pos.y = (float)(cell->row * 16.0f);

            spriteManAddSprite(boxSprite);
        }
    }

    spriteManPopMatrix();

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
            drawVineGameGameplay(vg, platAPI);
            sprite_text labelText = {
                .text = "game over",
                .fontKey = "font",
                .x = 320.0f,
                .y = 160.0f
            };
            centerText(&labelText);
            spriteManAddText(labelText);

        } break;
        case VINE_GAME_STATE_WIN_SCREEN: {

        } break;
    }
}
