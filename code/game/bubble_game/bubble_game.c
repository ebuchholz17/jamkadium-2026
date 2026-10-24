#include "bubble_game.h"
#include "../gng_sprites.h"

// https://opensource.apple.com/source/Libm/Libm-315/Source/Intel/atan.c
//#include "atan.c"

bubble_game *bubbleGame;
mem_arena *scratchMemory;

// https://stackoverflow.com/questions/3581528/how-is-the-square-root-function-implemented
//f32 squareRoot (f32 n){
//  // Max and min are used to take into account numbers less than 1
//    f32 lo = 1 < n ? 1 : n;
//    f32 hi = 1 > n ? 1 : n;
//    f32 mid;
//
//  // Update the bounds to be off the target by a factor of 10
//  while(100 * lo * lo < n) lo *= 10;
//  while(0.01 * hi * hi > n) hi *= 0.1;
//
//  for(int i = 0 ; i < 100 ; i++){
//      mid = (lo+hi)/2;
//      if(mid*mid == n) return mid;
//      if(mid*mid > n) hi = mid;
//      else lo = mid;
//  }
//  return mid;
//}

// https://stackoverflow.com/questions/35183929/approximating-atan-without-a-math-library
//f32 arctangent(f32 x)
//{
//    if (x > 1.0f) {
//        x = 1.0f / x;
//    }
//    if (x < -1.0f) {
//        x = 1.0f / x;
//    }
//    f32 result =x - (x*x*x)/3 + (x*x*x*x*x)/5 - (x*x*x*x*x*x*x)/7;
//    if (result > 1.0f) {
//        return 1.0f - result;
//    }
//    if (result < -1.0f) {
//        return -1.0f - result;
//    }
//    return result;
//}

f32 arctangent2(f32 y, f32 x) {
    // NOTE broken on purpose to work in vinesweeper
    return 0.0f;
    //if (x > 0.0f) {
    //    return arctangent(y / x);
    //}
    //else if (x < 0.0f && y >= 0.0f) {
    //    return arctangent(y / x) + PI;
    //}
    //else if (x < 0.0f && y < 0.0f) {
    //    return arctangent(y / x) - PI;
    //}
    //else if (x == 0.0f && y > 0.0f) {
    //    return PI / 2.0f;
    //}
    //else if (x == 0.0f && y < 0.0f) {
    //    return -PI / 2.0f;
    //}
    //else {
    //    return 0.0f;
    //}
}

ScratchMemMarker pushScratchMemory (void) {
    ScratchMemMarker result = {};
    result.marker = scratchMemory->current;
    return result;
}

void popScratchMemory (ScratchMemMarker marker) {
    scratchMemory->current = marker.marker;
}

b32 rowIsOffset (i32 row) {
    return row % 2 != bubbleGame->offsetIndex;
}

FloatingText *spawnFloatingText (void) {
    FloatingText *result = 0;
    for (i32 i = 1; i < bubbleGame->floatingTexts.numValues; i++) {
        FloatingText *floatingText = &bubbleGame->floatingTexts.values[i];
        if (!floatingText->active) {
            result = floatingText;
        }
    }

    if (!result) {
        FloatingText b = {};
        b.id.val = bubbleGame->floatingTexts.numValues;
        FloatingText_listPush(&bubbleGame->floatingTexts, b);
        result = &bubbleGame->floatingTexts.values[bubbleGame->floatingTexts.numValues - 1];
    }

    // reset bubble
    EntityID id = result->id;
    *result = (FloatingText){};
    result->id = id;

    result->active = true;

    return result;
}

void removeFloatingText (FloatingText *floatingText) {
    floatingText->active = false;
    floatingText->id.gen++;
}

Bubble *spawnBubble (void) {
    Bubble *result = 0;
    for (i32 i = 1; i < bubbleGame->bubbles.numValues; i++) {
        Bubble *bubble = &bubbleGame->bubbles.values[i];
        if (!bubble->active) {
            result = bubble;
        }
    }

    if (!result) {
        Bubble b = {};
        b.id.val = bubbleGame->bubbles.numValues;
        Bubble_listPush(&bubbleGame->bubbles, b);
        result = &bubbleGame->bubbles.values[bubbleGame->bubbles.numValues - 1];
    }

    // reset bubble
    EntityID id = result->id;
    *result = (Bubble){};
    result->id = id;

    result->active = true;

    return result;
}

void removeBubble (Bubble *bubble) {
    bubble->active = false;
    bubble->id.gen++;
}

BubbleParticles *spawnBubbleParticles (void) {
    BubbleParticles *result = 0;
    for (i32 i = 1; i < bubbleGame->particles.numValues; i++) {
        BubbleParticles *particle = &bubbleGame->particles.values[i];
        if (!particle->active) {
            result = particle;
        }
    }

    if (!result) {
        BubbleParticles b = {};
        b.id.val = bubbleGame->particles.numValues;
        BubbleParticles_listPush(&bubbleGame->particles, b);
        result = &bubbleGame->particles.values[bubbleGame->particles.numValues - 1];
    }

    // reset bubble
    EntityID id = result->id;
    *result = (BubbleParticles){};
    result->id = id;

    result->active = true;

    return result;
}

void removeBubbleParticles (BubbleParticles *particles) {
    particles->active = false;
    particles->id.gen++;
}

Bubble *getBubbleByID (EntityID id) {
    if (id.val == 0) {
        return 0;
    }

    if (id.val >= 0 && id.val < bubbleGame->bubbles.numValues) {
        Bubble *result = &bubbleGame->bubbles.values[id.val];
        if (result->id.gen == id.gen) {
            return result;
        }
    }
    return 0;
}

Projectile *spawnProjectile (void) {
    Projectile *result = 0;
    for (i32 i = 1; i < bubbleGame->projectiles.numValues; i++) {
        Projectile *projectile = &bubbleGame->projectiles.values[i];
        if (!projectile->active) {
            result = projectile;
        }
    }

    if (!result) {
        Projectile b = {};
        b.id.val = bubbleGame->projectiles.numValues;
        Projectile_listPush(&bubbleGame->projectiles, b);
        result = &bubbleGame->projectiles.values[bubbleGame->projectiles.numValues - 1];
    }

    // reset bubble
    EntityID id = result->id;
    *result = (Projectile){};
    result->id = id;

    result->active = true;

    return result;
}

void removeProjectile (Projectile *projectile) {
    projectile->active = false;
    projectile->id.gen++;
}

BubbleColor randomColor (void) {
    return (BUBBLE_COLOR_NONE + 1) + (randomU32() % (BUBBLE_COLOR_COUNT - (BUBBLE_COLOR_NONE + 1)));
}

vec2 gridCellRowColToPos (i32 row, i32 col) {
    if (!rowIsOffset(row)) {
        return (vec2){ 
            .x = GRID_CELL_WIDTH * col,
            .y = GRID_CELL_HEIGHT * row
        };
    }
    else {
        return (vec2){ 
            .x = (GRID_CELL_WIDTH / 2) + GRID_CELL_WIDTH * col,
            .y = GRID_CELL_HEIGHT * row
        };
    }
}

GridCoord posToGridCoord (vec2 pos) {
    GridCoord result = {};

    result.row = (i32)(pos.y / (f32)GRID_CELL_HEIGHT);
    f32 posX = !rowIsOffset(result.row) ? pos.x : pos.x - (GRID_CELL_WIDTH / 2.0f);
    result.col = (i32)(posX / (f32)GRID_CELL_WIDTH);

    return result;
}

i32 colsForRow (i32 row) {
    if (!rowIsOffset(row)) {
        return GRID_NUM_COLS;
    }
    else {
        return GRID_NUM_COLS - 1;
    }
}

Bubble *spawnBubbleInGridAtRowCol (i32 row, i32 col) {
    GridSlot *slot = &bubbleGame->grid[row * GRID_NUM_COLS + col];

    Bubble *bubble = spawnBubble();
    bubble->color = randomColor();
    bubble->pos = gridCellRowColToPos(row, col);
    bubble->pos = vec2Add(bubble->pos, (vec2){ .x = BUBBLE_RADIUS, .y = BUBBLE_RADIUS });
    bubble->row = row;
    bubble->col = col;

    slot->id = bubble->id;

    return bubble;
}

void spawnBubbleRow (i32 row) {
    if (row < GRID_NUM_ROWS - 1) {

        for (i32 j = 0; j < colsForRow(row); j++) { 
            Bubble *bubble = spawnBubbleInGridAtRowCol(row, j);

            i32 belowStartCol = j - 1;
            if (rowIsOffset(row)) {
                belowStartCol = j;
            }

            f32 random = randomF32();
            if (random < 0.01f) {
                bubble->color = BUBBLE_COLOR_NONE;
                bubble->isDiamond = true;
            }
            if (random < 0.41f) {
                // nothing
            }
            else {
                GridSlot *leftSlot = 0;
                GridSlot *rightSlot = 0;

                if (belowStartCol >= 0) {
                    leftSlot = &bubbleGame->grid[(row + 1) * GRID_NUM_COLS + belowStartCol];
                }
                if ((belowStartCol + 1) < colsForRow(row + 1)) {
                    rightSlot = &bubbleGame->grid[(row + 1) * GRID_NUM_COLS + (belowStartCol + 1)];
                }

                Bubble *belowBubble = 0;
                if (leftSlot && rightSlot) {
                    if (randomF32() < 0.5f) {
                        belowBubble = getBubbleByID(leftSlot->id);
                    }
                    else {
                        belowBubble = getBubbleByID(rightSlot->id);
                    }
                }
                else if (leftSlot) {
                    belowBubble = getBubbleByID(leftSlot->id);
                }
                else if (rightSlot) {
                    belowBubble = getBubbleByID(rightSlot->id);
                }

                if (belowBubble && belowBubble->active && !belowBubble->isDiamond) {
                    bubble->color = belowBubble->color;
                }
            }
        }
    }
    else {
        for (i32 j = 0; j < colsForRow(row); j++) { 
            spawnBubbleInGridAtRowCol(row, j);
        }
    }
}

void populateGrid (void) {
    for (i32 i = 0; i < GRID_NUM_ROWS; i++) { 
        for (i32 j = 0; j < colsForRow(i); j++) { 
            GridSlot *slot = &bubbleGame->grid[i * GRID_NUM_COLS + j];
            slot->row = i;
            slot->col = j;
        }
    }

    for (i32 i = 6; i >= 0; i--) { 
        spawnBubbleRow(i);
    }
}

void loadBubblesIntoQueue (void) {
    while (bubbleGame->bubbleQueue.numValues < bubbleGame->bubbleQueue.capacity) {
        BubbleQueueItem item = {};
        item.type = BUBBLE_TYPE_NORMAL;
        item.color = randomColor();
        BubbleQueueItem_listPush(&bubbleGame->bubbleQueue, item);
    }
}

void initBubbleGame (bubble_game *bg, mem_arena *memory) {
    
    bubbleGame = bg;
    bg->isInitialized = true;
    bubbleGame->bubbles = Bubble_listInit(memory, MAX_NUM_BUBBLES);

    // Put bubble in the 0 slot, not to be used
    Bubble b = {};
    b.id.val = bubbleGame->bubbles.numValues;
    Bubble_listPush(&bubbleGame->bubbles, b);

    populateGrid();

    bubbleGame->bubbleQueue = BubbleQueueItem_listInit(memory, BUBBLE_QUEUE_LENGTH);
    loadBubblesIntoQueue();

    bubbleGame->bubblesTilDrop = LAUNCHED_BUBBLES_BEFORE_DROP;
    bubbleGame->dropTimer = BUBBLES_DROP_TIMER;

    bubbleGame->enemies = Enemy_listInit(memory, MAX_NUM_ENEMIES);

    // Put enemy in the 0 slot, not to be used
    Enemy e = {};
    e.id.val = bubbleGame->enemies.numValues;
    Enemy_listPush(&bubbleGame->enemies, e);

    bubbleGame->enemySpawnTimer = 0.0f;

    bubbleGame->projectiles = Projectile_listInit(memory, MAX_NUM_PROJECTILES);

    bubbleGame->buttons = Button_listInit(memory, 10);

    bubbleGame->floatingTexts = FloatingText_listInit(memory, 500);
    bubbleGame->gold = 150;

    bubbleGame->lives = MAX_LIVES;

    bubbleGame->waves = EnemyWave_listInit(memory, 20);
    EnemyWave_listPush(&bubbleGame->waves, (EnemyWave){
        .numEnemies = 10,
        .enemyProbs = {
            1.0f,
            0.0f,
            0.0f,
            0.0f
        },
        .bossType = ENEMY_TYPE_GOBLIN,
        .bossSize = 5,
        .spawnTime = 10.0f
    });
    EnemyWave_listPush(&bubbleGame->waves, (EnemyWave){
        .numEnemies = 10,
        .enemyProbs = {
            0.5f,
            0.5f,
            0.0f,
            0.0f
        },
        .bossType = ENEMY_TYPE_GARGOYLE,
        .bossSize = 5,
        .spawnTime = 9.0f
    });
    EnemyWave_listPush(&bubbleGame->waves, (EnemyWave){
        .numEnemies = 10,
        .enemyProbs = {
            0.65f,
            0.25f,
            0.1f,
            0.0f
        },
        .bossType = ENEMY_TYPE_RAT,
        .bossSize = 5,
        .spawnTime = 9.0f
    });
    EnemyWave_listPush(&bubbleGame->waves, (EnemyWave){
        .numEnemies = 20,
        .enemyProbs = {
            0.5f,
            0.20f,
            0.20f,
            0.1f
        },
        .bossType = ENEMY_TYPE_BLACK_KNIGHT,
        .bossSize = 20,
        .spawnTime = 8.0f
    });
    EnemyWave_listPush(&bubbleGame->waves, (EnemyWave){
        .numEnemies = 15,
        .enemyProbs = {
            1.0f,
            0.0f,
            0.0f,
            0.0f
        },
        .bossType = ENEMY_TYPE_GOBLIN,
        .bossSize = 10,
        .spawnTime = 7.0f
    });
    EnemyWave_listPush(&bubbleGame->waves, (EnemyWave){
        .numEnemies = 15,
        .enemyProbs = {
            0.5f,
            0.5f,
            0.0f,
            0.0f
        },
        .bossType = ENEMY_TYPE_GARGOYLE,
        .bossSize = 10,
        .spawnTime = 7.0f
    });
    EnemyWave_listPush(&bubbleGame->waves, (EnemyWave){
        .numEnemies = 15,
        .enemyProbs = {
            0.65f,
            0.25f,
            0.1f,
            0.0f
        },
        .bossType = ENEMY_TYPE_RAT,
        .bossSize = 10,
        .spawnTime = 7.0f
    });
    EnemyWave_listPush(&bubbleGame->waves, (EnemyWave){
        .numEnemies = 40,
        .enemyProbs = {
            0.5f,
            0.20f,
            0.20f,
            0.1f
        },
        .bossType = ENEMY_TYPE_BLACK_KNIGHT,
        .bossSize = 30,
        .spawnTime = 6.0f
    });
    bubbleGame->waveIndex = 0;
    bubbleGame->waveIndex = 0;

    bubbleGame->particles = BubbleParticles_listInit(memory, MAX_NUM_BUBBLES);
}

void getNearbyGridSlots (i32 row, i32 col, GridSlot **nearbySlots, i32 *slotCount) {
    i32 aboveRow = row - 1;
    i32 belowRow = row + 1;
    i32 aboveStartCol = col - 1;
    i32 belowStartCol = col - 1;

    if (rowIsOffset(row)) {
        aboveStartCol = col;
        belowStartCol = col;
    }

    i32 numSlots = 0;

    if (aboveRow >= 0) {
        for (i32 j = aboveStartCol; j <= aboveStartCol + 1; j++) {
            if (j >= 0 && j < colsForRow(aboveRow)) {
                GridSlot *slot = &bubbleGame->grid[aboveRow * GRID_NUM_COLS + j];
                nearbySlots[numSlots++] = slot;
            }
        }
    }

    for (i32 j = col - 1; j <= col + 1; j++) {
        if (j >= 0 && j < colsForRow(row)) {
            GridSlot *slot = &bubbleGame->grid[row * GRID_NUM_COLS + j];
            nearbySlots[numSlots++] = slot;
        }
    }

    if (belowRow < GRID_NUM_ROWS) {
        for (i32 j = belowStartCol; j <= belowStartCol + 1; j++) {
            if (j >= 0 && j < colsForRow(belowRow)) {
                GridSlot *slot = &bubbleGame->grid[belowRow * GRID_NUM_COLS + j];
                nearbySlots[numSlots++] = slot;
            }
        }
    }

    *slotCount = numSlots;
}

void getNearbyBubblesInGrid (i32 row, i32 col, Bubble **nearbyBubbles, i32 *bubbleCount) {
    i32 aboveRow = row - 1;
    i32 belowRow = row + 1;
    i32 aboveStartCol = col - 1;
    i32 belowStartCol = col - 1;

    if (rowIsOffset(row)) {
        aboveStartCol = col;
        belowStartCol = col;
    }

    i32 numBubbles = 0;

    if (aboveRow >= 0) {
        for (i32 j = aboveStartCol; j <= aboveStartCol + 1; j++) {
            if (j >= 0 && j < colsForRow(aboveRow)) {
                GridSlot *slot = &bubbleGame->grid[aboveRow * GRID_NUM_COLS + j];
                if (slot->id.val != 0) {
                    Bubble *bubble = getBubbleByID(slot->id);
                    if (bubble) {
                        nearbyBubbles[numBubbles++] = bubble;
                    }
                }
            }
        }
    }

    for (i32 j = col - 1; j <= col + 1; j++) {
        if (j >= 0 && j < colsForRow(row)) {
            GridSlot *slot = &bubbleGame->grid[row * GRID_NUM_COLS + j];
            if (slot->id.val != 0) {
                Bubble *bubble = getBubbleByID(slot->id);
                if (bubble) {
                    nearbyBubbles[numBubbles++] = bubble;
                }
            }
        }
    }

    if (belowRow < GRID_NUM_ROWS) {
        for (i32 j = belowStartCol; j <= belowStartCol + 1; j++) {
            if (j >= 0 && j < colsForRow(belowRow)) {
                GridSlot *slot = &bubbleGame->grid[belowRow * GRID_NUM_COLS + j];
                if (slot->id.val != 0) {
                    Bubble *bubble = getBubbleByID(slot->id);
                    if (bubble) {
                        nearbyBubbles[numBubbles++] = bubble;
                    }
                }
            }
        }
    }

    *bubbleCount = numBubbles;
}

b32 bubblesAreTouching (Bubble *a, Bubble *b) {
    f32 aRadius = a->flying ? BUBBLE_FLYING_RADIUS : BUBBLE_RADIUS;
    f32 bRadius = b->flying ? BUBBLE_FLYING_RADIUS : BUBBLE_RADIUS;

    f32 dist = vec2Length(vec2Subtract(a->pos, b->pos));
    if (dist < aRadius + bRadius) {
        return true;
    }
    return false;
}

void insertBubbleIntoGridAtRowCol (Bubble *bubble, i32 row, i32 col) {
    bubble->flying = false;
    GridSlot *slot = &bubbleGame->grid[row * GRID_NUM_COLS + col];
    slot->id = bubble->id;
    vec2 pos = gridCellRowColToPos(row, col);
    bubble->pos = pos;
    bubble->pos = vec2Add(bubble->pos, (vec2){ .x = BUBBLE_RADIUS, .y = BUBBLE_RADIUS });
    bubble->vel = (vec2){};
    bubble->row = row;
    bubble->col = col;
}

Bubble *tryGetBubbleAtRowCol (i32 row, i32 col) {
    GridSlot *slot = &bubbleGame->grid[row * GRID_NUM_COLS + col];
    if (slot->id.val != 0) {
        Bubble *bubble = getBubbleByID(slot->id);
        return bubble;
    }
    return 0;
}

void unmarkBubbles (void) {
    for (i32 i = 0; i < bubbleGame->bubbles.numValues; i++) {
        Bubble *bubble = &bubbleGame->bubbles.values[i];
        if (bubble->active) {
            bubble->marked = false;
        }
    }
}

void unmarkSlots (void) {
    for (i32 i = 0; i < GRID_NUM_ROWS; i++) { 
        for (i32 j = 0; j < colsForRow(i); j++) { 
            GridSlot *slot = &bubbleGame->grid[i * GRID_NUM_COLS + j];
            slot->marked = false;
        }
    }
}

void checkForBubblesThatShouldFall (void) {
    ScratchMemMarker m = pushScratchMemory();

    unmarkBubbles();

    BubblePtr_list bubblesToCheck = BubblePtr_listInit(scratchMemory, MAX_NUM_BUBBLES);
    for (i32 j = 0; j < colsForRow(0); j++) {
        Bubble *bubble = tryGetBubbleAtRowCol(0, j);
        if (bubble) {
            bubble->marked = true;
            BubblePtr_listPush(&bubblesToCheck, bubble);
        }
    }

    i32 index = 0;
    while (index < bubblesToCheck.numValues) {
        Bubble *bubble = bubblesToCheck.values[index++];

        i32 numNeighbors = 0;
        Bubble *neighborBubbles[7]; 
        getNearbyBubblesInGrid(bubble->row, bubble->col, neighborBubbles, &numNeighbors);

        for (i32 i = 0; i < numNeighbors; i++) {
            Bubble *neighbor = neighborBubbles[i];
            if (neighbor != bubble && !neighbor->marked) {
                neighbor->marked = true;
                BubblePtr_listPush(&bubblesToCheck, neighbor);
            }
        }
    }

    // cause unmarked bubbles to start falling
    for (i32 i = 0; i < GRID_NUM_ROWS; i++) { 
        for (i32 j = 0; j < colsForRow(i); j++) { 
            Bubble *bubble = tryGetBubbleAtRowCol(i, j);
            if (bubble && !bubble->marked) {
                bubble->falling = true;

                GridSlot *slot = &bubbleGame->grid[bubble->row * GRID_NUM_COLS + bubble->col];
                slot->id = (EntityID){};
            }
        }
    }

    popScratchMemory(m);
}

void burstBubble (Bubble *bubble) {
    if (bubble->type == BUBBLE_TYPE_NORMAL) {
        bubbleGame->bubblesBurst = true;
        BubbleParticles *particles = spawnBubbleParticles();
        particles->pos = bubble->pos;
        particles->color = bubble->color;
        for (i32 i = 0; i < 5; i++) {
            particles->angles[i] = randomF32();
        }
    }
}

void checkForMatchesAtRowCol (i32 row, i32 col) {
    Bubble *bubble = tryGetBubbleAtRowCol(row, col);
    if (bubble) {

        if (bubble->color == BUBBLE_COLOR_NONE) {
            return;
        }

        ScratchMemMarker m = pushScratchMemory();

        for (i32 i = 0; i < bubbleGame->bubbles.numValues; i++) {
            Bubble *bubble = &bubbleGame->bubbles.values[i];
            if (bubble->active) {
                bubble->marked = false;
            }
        }

        BubblePtr_list bubblesToCheck = BubblePtr_listInit(scratchMemory, MAX_NUM_BUBBLES);
        bubble->marked = true;
        BubblePtr_listPush(&bubblesToCheck, bubble);

        i32 index = 0;
        while (index < bubblesToCheck.numValues) {
            Bubble *bubbleToCheck = bubblesToCheck.values[index++];

            i32 numNeighbors = 0;
            Bubble *neighborBubbles[7]; 
            getNearbyBubblesInGrid(bubbleToCheck->row, bubbleToCheck->col, neighborBubbles, &numNeighbors);

            for (i32 i = 0; i < numNeighbors; i++) {
                Bubble *neighbor = neighborBubbles[i];
                if (neighbor != bubbleToCheck && !neighbor->marked && neighbor->color == bubbleToCheck->color) {
                    neighbor->marked = true;
                    BubblePtr_listPush(&bubblesToCheck, neighbor);
                }
            }
        }

        if (bubblesToCheck.numValues >= 3) {
            vec2 pos = {};
            for (i32 i = 0; i < bubblesToCheck.numValues; i++) {
                Bubble *bubbleToCheck = bubblesToCheck.values[i];
                pos.x = bubbleToCheck->pos.x;
                if (bubbleToCheck->pos.y > pos.y) {
                    pos.y = bubbleToCheck->pos.y;
                }

                GridSlot *slot = &bubbleGame->grid[bubbleToCheck->row * GRID_NUM_COLS + bubbleToCheck->col];
                slot->id = (EntityID){};
                removeBubble(bubbleToCheck);

                burstBubble(bubbleToCheck);
            }
            i32 gold = bubblesToCheck.numValues * POPPED_BUBBLE_GOLD;
            bubbleGame->gold += gold;

            FloatingText *ft = spawnFloatingText();
            if (ft) {
                ft->amount = gold;
                ft->icon = "gold";
                ft->startPos = pos;
            }
        }

        popScratchMemory(m);
    }
}

Enemy *spawnEnemy (void) {
    Enemy *result = 0;
    for (i32 i = 1; i < bubbleGame->enemies.numValues; i++) {
        Enemy *enemy = &bubbleGame->enemies.values[i];
        if (!enemy->active) {
            result = enemy;
        }
    }

    if (!result) {
        Enemy e = {};
        e.id.val = bubbleGame->enemies.numValues;
        Enemy_listPush(&bubbleGame->enemies, e);
        result = &bubbleGame->enemies.values[bubbleGame->enemies.numValues - 1];
    }

    // reset enemy
    EntityID id = result->id;
    *result = (Enemy){};
    result->id = id;

    result->active = true;

    return result;
}

void removeEnemy (Enemy *enemy) {
    enemy->active = false;
    enemy->id.gen++;
}

Enemy *getEnemyByID (EntityID id) {
    if (id.val == 0) {
        return 0;
    }

    if (id.val >= 0 && id.val < bubbleGame->enemies.numValues) {
        Enemy *result = &bubbleGame->enemies.values[id.val];
        if (result->id.gen == id.gen) {
            return result;
        }
    }
    return 0;
}

void setUnitProps (Bubble *bubble) {
    switch (bubble->unitType) {
        case UNIT_TYPE_ARCHER: {
            bubble->range = 128.0f;
            bubble->cooldownDuration = 2.0f;
            bubble->projectileSpeed = 128.0f;
        } break;
        case UNIT_TYPE_KNIGHT: {
            bubble->range = 64.0f;
            bubble->cooldownDuration = 1.0f;
            bubble->projectileSpeed = 128.0f;
        } break;
        case UNIT_TYPE_CANNON: {
            bubble->range = 256.0f;
            bubble->cooldownDuration = 4.0f;
            bubble->projectileSpeed = 96.0f;
        } break;
        case UNIT_TYPE_FARMER: {
            bubble->range = 0.0f;
            bubble->cooldownDuration = 5.0f;
            bubble->projectileSpeed = 1.0f;
        } break;
    }
}

f32 damageByUpgradeType (UpgradeType ug) {
    switch (ug) {
        case UPGRADE_TYPE_ARROW_ATK: {
            return 8.0f + bubbleGame->upgrades[UPGRADE_TYPE_ARROW_ATK] * 2.0f;
        } break;
        case UPGRADE_TYPE_SLASH_ATK: {
            return 12.0f + bubbleGame->upgrades[UPGRADE_TYPE_SLASH_ATK] * 4.0f;
        } break;
        case UPGRADE_TYPE_CANNONBALL_ATK: {
            return 20.0f + bubbleGame->upgrades[UPGRADE_TYPE_ARROW_ATK] * 5.0f;
        } break;
        case UPGRADE_TYPE_FARMING: {
            return 0;
        } break;
    }
    return 0.0f;
}

void assignEnemyProps (Enemy *e) {
    switch (e->type) {
        case ENEMY_TYPE_GOBLIN: {
            e->hitPoints = 80.0f;
            e->baseHitPoints = e->hitPoints;
            e->maxHitPoints = e->hitPoints;
            e->radius = 7.0f;
            e->baseRadius = e->radius;
            e->scale = 1.0f;
            e->damage = 1;
            e->speed = 10.0f;
            e->weakness = UPGRADE_TYPE_NONE;
            e->resists = UPGRADE_TYPE_NONE;
        } break;
        case ENEMY_TYPE_GARGOYLE: {
            e->hitPoints = 100.0f;
            e->baseHitPoints = e->hitPoints;
            e->maxHitPoints = e->hitPoints;
            e->radius = 7.0f;
            e->baseRadius = e->radius;
            e->scale = 1.0f;
            e->damage = 1;
            e->speed = 10.0f;
            e->weakness = UPGRADE_TYPE_ARROW_ATK;
            e->resists = UPGRADE_TYPE_NONE;
        } break;
        case ENEMY_TYPE_RAT: {
            e->hitPoints = 30.0f;
            e->baseHitPoints = e->hitPoints;
            e->maxHitPoints = e->hitPoints;
            e->radius = 7.0f;
            e->baseRadius = e->radius;
            e->scale = 1.0f;
            e->damage = 1;
            e->speed = 25.0f;
            e->weakness = UPGRADE_TYPE_NONE;
            e->resists = UPGRADE_TYPE_NONE;
        } break;
        case ENEMY_TYPE_BLACK_KNIGHT: {
            e->hitPoints = 150.0f;
            e->baseHitPoints = e->hitPoints;
            e->maxHitPoints = e->hitPoints;
            e->radius = 9.0f;
            e->baseRadius = e->radius;
            e->scale = 1.0f;
            e->damage = 2;
            e->speed = 5.0f;
            e->weakness = UPGRADE_TYPE_CANNONBALL_ATK;
            e->resists = UPGRADE_TYPE_ARROW_ATK;
        } break;
    }
}

EnemyType getEnemyTypeByProbabilities (f32 *enemyProbs) {
    f32 random = randomF32();
    f32 prob = 0.0f;
    for (i32 i = 0; i < ENEMY_TYPE_COUNT; i++) {
        prob += enemyProbs[i];
        if (random < prob) {
            return i;
        }
    }
    return 0;
}

b32 enemyTouchingSlot (Enemy *enemy, GridSlot *slot) {
    vec2 slotPos = gridCellRowColToPos(slot->row, slot->col);
    slotPos = vec2Add(slotPos, (vec2){ .x = GRID_CELL_WIDTH / 2.0f, .y = GRID_CELL_HEIGHT / 2.0f });
    f32 dist = vec2Length(vec2Subtract(enemy->pos, slotPos));
    if (dist < enemy->radius + BUBBLE_RADIUS_SUCK) {
        return true;
    }
    return false;
}

void enemyTrySuckingBubble (Enemy *enemy) {
    GridCoord enemyCoord = posToGridCoord(enemy->pos);

    ScratchMemMarker m = pushScratchMemory();

    GridSlotPtr_list slotsToCheck = GridSlotPtr_listInit(scratchMemory, GRID_NUM_COLS * GRID_NUM_ROWS);

    if (enemyCoord.row >= 0 && enemyCoord.row < GRID_NUM_ROWS &&
        enemyCoord.col >= 0 && enemyCoord.col < GRID_NUM_COLS) 
    {
        GridSlot *firstSlot = &bubbleGame->grid[enemyCoord.row * GRID_NUM_COLS + enemyCoord.col];
        if (!firstSlot ->marked){
            firstSlot->marked = true;
            GridSlotPtr_listPush(&slotsToCheck, firstSlot);
        }
    }

    i32 index = 0;
    while (index < slotsToCheck.numValues) {
        GridSlot *slot = slotsToCheck.values[index++];
        if (enemyTouchingSlot(enemy, slot)) {
            i32 numSlots = 0;
            GridSlot *nearbySlots[7];
            getNearbyGridSlots(slot->row, slot->col, nearbySlots, &numSlots);

            for (i32 i = 0; i < numSlots; i++) {
                GridSlot *neighbor = nearbySlots[i];
                if (neighbor && !neighbor->marked) {
                    neighbor->marked = true;
                    GridSlotPtr_listPush(&slotsToCheck, neighbor);
                }
            }
        }
    }

    for (i32 i = 0; i < slotsToCheck.numValues; i++) {
        GridSlot *slot = slotsToCheck.values[i];
        if (enemyTouchingSlot(enemy, slot)) {
            Bubble *bubble = getBubbleByID(slot->id);
            if (bubble) {
                if (enemy->size >= ENEMY_MAX_SIZE) {
                    burstBubble(bubble);
                    removeBubble(bubble);
                }
                else {
                    slot->id = (EntityID){};
                    enemy->state = ENEMY_STATE_SUCKING_IN_BUBBLE;
                    enemy->suckingBubbleID = bubble->id;
                    break;
                }
            }
        }
    }

    popScratchMemory(m);
}

void updateEnemySuckingBubble (Enemy *enemy, f32 dt) {
    Bubble *suckedBubble = getBubbleByID(enemy->suckingBubbleID);
    if (suckedBubble) {
        f32 suckSpeed = 64.0f * dt;
        vec2 toEnemy = vec2Subtract(enemy->pos, suckedBubble->pos);
        f32 toEnemyLength = vec2Length(toEnemy);

        vec2 toEnemyN = vec2Normalize(toEnemy);
        vec2 bubbleVec = vec2ScalarMul(suckSpeed, toEnemyN);

        f32 bubbleVecLength = vec2Length(bubbleVec);
        if (bubbleVecLength > toEnemyLength) {
            bubbleVec = toEnemy;
        }

        suckedBubble->pos = vec2Add(suckedBubble->pos, bubbleVec);
        toEnemy = vec2Subtract(enemy->pos, suckedBubble->pos);
        toEnemyLength = vec2Length(toEnemy);
        if (toEnemyLength < 2.0f) {
            removeBubble(suckedBubble);
            enemy->size++;
            enemy->t = 0.0f;
            enemy->state = ENEMY_STATE_GROWING;
            soundManPlaySound("sfx_enemy_grow", false);
        }
    }
    else {
        enemy->state = ENEMY_STATE_WALKING;
    }
}

void setEnemyPropsForScale (Enemy *enemy) {
    enemy->radius = enemy->baseRadius * enemy->scale;
    f32 percentHitpoints = (f32)enemy->hitPoints / (f32)enemy->maxHitPoints;
    enemy->maxHitPoints = enemy->baseHitPoints * enemy->scale;
    enemy->hitPoints = (i32)((f32)enemy->maxHitPoints * percentHitpoints);
    enemy->damage = 1 + enemy->size / 3;
}

void updateEnemyGrowing (Enemy *enemy, f32 dt) {
    b32 done = false;
    enemy->t += dt;
    if (enemy->t >= ENEMY_GROW_TIME) {
        enemy->t = ENEMY_GROW_TIME;
        done = true;
    }

    f32 t = enemy->t / ENEMY_GROW_TIME;
    f32 startScale = 1.0f + (enemy->size - 1) * 0.2f;
    f32 endScale = 1.0f + (enemy->size) * 0.2f;
    enemy->scale = (1.0f - t) * startScale + t * endScale;

    if (done) {
        enemy->state = ENEMY_STATE_WALKING;
        setEnemyPropsForScale(enemy);
    }
}

void updateEnemyAttackingBase (Enemy *enemy, f32 dt) {
    b32 done = false;
    enemy->t += dt;
    if (enemy->t >= ENEMY_GROW_TIME) {
        enemy->t = ENEMY_GROW_TIME;
        done = true;
    }

    f32 t = enemy->t / ENEMY_GROW_TIME;
    vec2 startPos = enemy->startPos;
    vec2 endPos = (vec2){ .x = 408.0f, .y = 160.0f };
    enemy->pos = vec2Add(vec2ScalarMul((1.0f - t), startPos), 
                         vec2ScalarMul(t, endPos));
    if (done) {
        removeEnemy(enemy);
        bubbleGame->lives -= enemy->damage;
        soundManPlaySound("sfx_house_destroyed", false);
        if (bubbleGame->lives <= 0) {
            bubbleGame->gameOver = true;
        }
    }
}

void assignProjectileProps (Projectile *p, Bubble *b) {
    p->speed = b->projectileSpeed;
    p->damage = b->projectileDamage;
    switch (b->unitType) {
        case UNIT_TYPE_ARCHER: {
            p->frame = "arrow";
            p->upgradeType = UPGRADE_TYPE_ARROW_ATK;
        } break;
        case UNIT_TYPE_KNIGHT: {
            p->frame = "slash";
            p->upgradeType = UPGRADE_TYPE_SLASH_ATK;
        } break;
        case UNIT_TYPE_CANNON: {
            p->frame = "cannonball";
            p->upgradeType = UPGRADE_TYPE_CANNONBALL_ATK;
        } break;
        case UNIT_TYPE_FARMER: {
            p->frame = "arrow";
            p->upgradeType = UPGRADE_TYPE_ARROW_ATK;
        } break;
    }
}

b32 doButton (Button b, vec3 pointerPos, b32 pointerDown, b32 pointerJustDown) {
    if (pointerPos.x >= b.pos.x && pointerPos.x < b.pos.x + b.dims.x &&
        pointerPos.y >= b.pos.y && pointerPos.y < b.pos.y + b.dims.y)
    {
        b.isOver = true;
        if (pointerDown) {
            b.isDown = true;
        }
    }

    Button_listPush(&bubbleGame->buttons, b);

    return b.isDown && pointerJustDown;
}

i32 costForUnit (UnitType unitType) {
    switch (unitType) {
    case UNIT_TYPE_ARCHER: 
       return 75;
    case UNIT_TYPE_KNIGHT:
       return 75;
    case UNIT_TYPE_CANNON: 
       return 75;
    case UNIT_TYPE_FARMER: 
       return 75;
    }
    return 0;
}

void doUnitButton (b32 *boughtSomething, vec2 buttonPos, UnitType unitType, char *icon, vec3 localPointerPos, b32 pointerDown, b32 pointerJustDown) {
    i32 unitCost = costForUnit(unitType);

    if (doButton((Button){ 
            .upFrame = "button_up", 
            .overFrame = "button_over", 
            .downFrame = "button_down", 
            .icon = icon,
            .label = tempStringFromI32(unitCost),
            .labelIcon = "gold",
            .pos = buttonPos,
            .dims = (vec2){ .x = 96.0f, .y = 32.0f}
        },
        localPointerPos, pointerDown, pointerJustDown))
    {
        *boughtSomething = true;

        if (bubbleGame->gold >= unitCost) {
            bubbleGame->gold -= unitCost;

            if (bubbleGame->bubbleQueue.numValues == BUBBLE_QUEUE_LENGTH) {
                BubbleQueueItem_listSplice(&bubbleGame->bubbleQueue, BUBBLE_QUEUE_LENGTH - 1);
            }

            BubbleQueueItem item = {
                .type = BUBBLE_TYPE_UNIT,
                .unitType = unitType
            };
            BubbleQueueItem_listInsert(&bubbleGame->bubbleQueue, item, 0);

            soundManPlaySound("sfx_bought_unit", false);
        }
    }
}

void doUpgradeUI (b32 *boughtSomething, vec2 buttonPos, UpgradeType upgradeType, char *icon, vec3 localPointerPos, b32 pointerDown, b32 pointerJustDown) {
    if (doButton((Button){ 
            .upFrame = "button_up_small", 
            .overFrame = "button_over_small", 
            .downFrame = "button_down_small", 
            .icon = 0,
            .label = tempStringFromI32(1),
            .labelIcon = "diamond",
            .labelOffset = (vec2){ .x = -48.0f, .y = 0.0f },
            .pos = vec2Add((vec2){ .x = 48.0f, .y = 0.0f }, buttonPos),
            .dims = (vec2){ .x = 96.0f, .y = 32.0f}
        },
        localPointerPos, pointerDown, pointerJustDown))
    {
        *boughtSomething = true;

        if (bubbleGame->diamonds >= 1) {
            bubbleGame->diamonds -= 1;
            bubbleGame->upgrades[upgradeType]++;

            soundManPlaySound("sfx_bought_unit", false);
        }
    }
}

void splashDamageNearbyEnemies (Projectile *p) {
    for (i32 enemyIndex = 0; enemyIndex < bubbleGame->enemies.numValues; enemyIndex++) {
        Enemy *enemy = &bubbleGame->enemies.values[enemyIndex];
        if (enemy->active) {
            vec2 projToEnemy = vec2Subtract(enemy->pos, p->pos);
            f32 distToEnemy = vec2Length(projToEnemy);
            if (distToEnemy <= 36.0f) {

                f32 damage = damageByUpgradeType(p->upgradeType);
                damage /= 5.0f;
                if (enemy->resists == p->upgradeType) {
                    damage /= 2.0f;
                }
                if (enemy->weakness == p->upgradeType) {
                    damage *= 2.0f;
                }
                enemy->hitPoints -= damage;

                FloatingText *ft = spawnFloatingText();
                if (ft) {
                    ft->amount = (i32)damage;
                    ft->icon = 0;
                    ft->startPos = enemy->pos;
                }

                if (enemy->hitPoints <= 0.0f) {
                    Bubble *suckedBubble = getBubbleByID(enemy->suckingBubbleID);
                    if (suckedBubble) {
                        suckedBubble->falling = true;
                    }
                    enemy->state = ENEMY_STATE_DYING;
                    enemy->t = 0.0f;
                soundManPlaySound("sfx_enemy_killed", false);
                }
            }
        }
    }
}

void updateDyingEnemy (Enemy *enemy, f32 dt) {
    enemy->t += dt;
    if (enemy->t >= 0.5f) {
        removeEnemy(enemy);
    }
}

void sortEnemies (EnemyPtr_list *sortedEnemies) {
    for (int i = 0; i < sortedEnemies->numValues; ++i) {
        for (int j = i; j > 0; --j) {
            Enemy *first = sortedEnemies->values[j-1];
            Enemy *second = sortedEnemies->values[j];
            if (second->pos.x > first->pos.x) {
                sortedEnemies->values[j-1] = second;
                sortedEnemies->values[j] = first;
            }
        }
    }
}


void updateBubbleGame (bubble_game *bg, game_input *input, 
                       virtual_input *vInput, f32 dt, plat_api platAPI, mem_arena *memory,
                       mem_arena *sm) 
{
    scratchMemory = sm;

        bubbleGame->bubblesBurst = false;

    if (bubbleGame->state == GAME_STATE_TITLE) {
        if (input->pointerJustDown) {
            bubbleGame->state = GAME_STATE_NORMAL;
        }
        return;
    }

    if (bubbleGame->gameOver || bubbleGame->win) {
        bubbleGame->gameEndTimer += dt;
        if (bubbleGame->gameEndTimer > 1.0f && input->pointerJustDown) {
            bubbleGame->reset = true;
        }
    }


    mat3x3 gameTransform = spriteManPeekMatrix();
    mat3x3 invGameTransform = mat3x3Inv(gameTransform);
    vec3 localPointerPos = (vec3){ (f32)input->pointerX, (f32)input->pointerY, 1.0f };
    localPointerPos = vec3MatrixMul(invGameTransform, localPointerPos);

    // store
    bubbleGame->buttons.numValues = 0;
    b32 boughtSomething = false;

    doUnitButton(&boughtSomething, (vec2){ .x = 416.0f, .y = 224.0f}, UNIT_TYPE_ARCHER, 
                 "archer_0", localPointerPos, input->pointerDown, input->pointerJustDown);
    doUnitButton(&boughtSomething, (vec2){ .x = 528.0f, .y = 224.0f}, UNIT_TYPE_KNIGHT, 
                 "knight_0", localPointerPos, input->pointerDown, input->pointerJustDown);
    doUnitButton(&boughtSomething, (vec2){ .x = 416.0f, .y = 272.0f}, UNIT_TYPE_CANNON, 
                 "cannon_0", localPointerPos, input->pointerDown, input->pointerJustDown);
    doUnitButton(&boughtSomething, (vec2){ .x = 528.0f, .y = 272.0f}, UNIT_TYPE_FARMER, 
                 "farmer_0", localPointerPos, input->pointerDown, input->pointerJustDown);

    doUpgradeUI(&boughtSomething, (vec2){ .x = 416.0f, .y = 48.0f}, UPGRADE_TYPE_ARROW_ATK, 
                 "arrow", localPointerPos, input->pointerDown, input->pointerJustDown);
    doUpgradeUI(&boughtSomething, (vec2){ .x = 528.0f, .y = 48.0f}, UPGRADE_TYPE_SLASH_ATK, 
                 "slash", localPointerPos, input->pointerDown, input->pointerJustDown);
    doUpgradeUI(&boughtSomething, (vec2){ .x = 416.0f, .y = 96.0f}, UPGRADE_TYPE_CANNONBALL_ATK, 
                 "cannonball", localPointerPos, input->pointerDown, input->pointerJustDown);
    doUpgradeUI(&boughtSomething, (vec2){ .x = 528.0f, .y = 96.0f}, UPGRADE_TYPE_FARMING,
                 "pitchfork", localPointerPos, input->pointerDown, input->pointerJustDown);

    if (bubbleGame->win) {
        return;
    }

    bubbleGame->gameTime += dt;

    spriteManPushTransform((sprite_transform){
            .pos = (vec2){ .x = 16.0f, .y = 16.0f },
            .scale = 1.0f
    });

    gameTransform = spriteManPeekMatrix();
    invGameTransform = mat3x3Inv(gameTransform);
    localPointerPos = (vec3){ (f32)input->pointerX, (f32)input->pointerY, 1.0f };
    localPointerPos = vec3MatrixMul(invGameTransform, localPointerPos);

    bubbleGame->launcherTarget.x = localPointerPos.x;
    bubbleGame->launcherTarget.y = localPointerPos.y;


    spriteManPopMatrix();

    // launch bubbles
    if (input->pointerJustDown && !boughtSomething) {
        vec2 vel = (vec2){ bubbleGame->launcherTarget.x - LAUNCHER_POS_X, bubbleGame->launcherTarget.y - LAUNCHER_POS_Y} ;
        if (vel.y < 0) {
            Bubble *bubble = spawnBubble();
            
            BubbleQueueItem queueItem =  BubbleQueueItem_listSplice(&bubbleGame->bubbleQueue, 0);
            bubble->type = queueItem.type;
            if (bubble->type == BUBBLE_TYPE_NORMAL) {
                bubble->color = queueItem.color;
            }
            else if (bubble->type == BUBBLE_TYPE_UNIT) {
                bubble->unitType = queueItem.unitType;
                setUnitProps(bubble);
            }
            loadBubblesIntoQueue();

            bubble->pos = (vec2){ .x = LAUNCHER_POS_X, .y = LAUNCHER_POS_Y };
            bubble->flying = true;
            vel = vec2Normalize(vel);
            vel = vec2ScalarMul(BUBBLE_LAUNCH_SPEED, vel);

            bubble->vel = vel;

            bubbleGame->bubblesTilDrop--;

            soundManPlaySound("sfx_bubble_launch", false);
        }
    }

    // updated launched bubbles
    for (i32 i = 0; i < bubbleGame->bubbles.numValues; i++) {
        Bubble *bubble = &bubbleGame->bubbles.values[i];
        if (bubble->active && bubble->flying) {
            vec2 posOffset = vec2ScalarMul(dt, bubble->vel);
            bubble->pos = vec2Add(bubble->pos, posOffset);

            f32 leftEdge = bubble->pos.x - BUBBLE_RADIUS;
            f32 rightEdge = bubble->pos.x + BUBBLE_RADIUS;
            f32 topEdge = bubble->pos.y - BUBBLE_RADIUS;
            f32 bottomEdge = bubble->pos.y + BUBBLE_RADIUS;

            // bounce
            if (bubble->vel.x < 0 && leftEdge < 0) {
                bubble->pos.x = BUBBLE_RADIUS;
                bubble->vel.x = -bubble->vel.x;
            }
            if (bubble->vel.x > 0 && rightEdge >= BUBBLE_ZONE_WIDTH) {
                bubble->pos.x = BUBBLE_ZONE_WIDTH - BUBBLE_RADIUS;
                bubble->vel.x = -bubble->vel.x;
            }

            GridCoord gridCoord = posToGridCoord(bubble->pos);
            if (gridCoord.col >= 0 && gridCoord.col < colsForRow(gridCoord.row) &&
                gridCoord.row >= 0 && gridCoord.row < GRID_NUM_ROWS)
            {
                i32 bubbleCount;
                Bubble *nearbyBubbles[7];
                getNearbyBubblesInGrid(gridCoord.row, gridCoord.col, nearbyBubbles, &bubbleCount);
                for (i32 i = 0; i < bubbleCount; i++) {
                    Bubble *nearbyBubble = nearbyBubbles[i];
                    if (bubblesAreTouching(bubble, nearbyBubble)) {
                        if (nearbyBubble->row == gridCoord.row && nearbyBubble->col == gridCoord.col) {
                            removeBubble(bubble);
                        }
                        else {
                            insertBubbleIntoGridAtRowCol(bubble, gridCoord.row, gridCoord.col);
                            checkForMatchesAtRowCol(gridCoord.row, gridCoord.col);
                            soundManPlaySound("sfx_bubble_land", false);
                        }

                        break;
                    }
                }
            }
        }
    }

    bubbleGame->dropTimer -= dt;
    if (bubbleGame->bubblesTilDrop <= 0 || bubbleGame->dropTimer <= 0) {
        bubbleGame->dropTimer = BUBBLES_DROP_TIMER;
        bubbleGame->bubblesTilDrop = LAUNCHED_BUBBLES_BEFORE_DROP;


        for (i32 j = 0; j < GRID_NUM_COLS; j++) { 
            i32 row = GRID_NUM_ROWS - 1;

            Bubble *bubble = tryGetBubbleAtRowCol(row, j);
            if (bubble) {
                removeBubble(bubble);
            }
        }

        for (i32 i = GRID_NUM_ROWS - 1; i >= 1; i--) {
            for (i32 j = 0; j < GRID_NUM_COLS; j++) { 
                GridSlot *slot = &bubbleGame->grid[i * GRID_NUM_COLS + j];
                GridSlot *prevSlot = &bubbleGame->grid[(i - 1) * GRID_NUM_COLS + j];
                slot->id = prevSlot->id;

                Bubble *bubble = tryGetBubbleAtRowCol(i, j);
                if (bubble) {
                    bubble->row = i;
                    bubble->col = j;
                }
            }
        }

        bubbleGame->offsetIndex = (bubbleGame->offsetIndex + 1) % 2;

        spawnBubbleRow(0);

        for (i32 i = 0; i < GRID_NUM_ROWS; i++) {
            for (i32 j = 0; j < colsForRow(i); j++) { 
                Bubble *bubble = tryGetBubbleAtRowCol(i, j);
                if (bubble) {
                    bubble->pos = gridCellRowColToPos(i, j);
                    bubble->pos = vec2Add(bubble->pos, (vec2){ .x = BUBBLE_RADIUS, .y = BUBBLE_RADIUS });
                }
            }
        }
    }

    checkForBubblesThatShouldFall();

    // Update falling bubbles
    for (i32 i = 0; i < bubbleGame->bubbles.numValues; i++) {
        Bubble *bubble = &bubbleGame->bubbles.values[i];
        f32 gravity = 640.0f;
        f32 maxFallSpeed = 320.0f;
        if (bubble->active && bubble->falling) {
            bubble->vel.y += gravity * dt;
            if (bubble->vel.y > maxFallSpeed) {
                bubble->vel.y = maxFallSpeed;
            }
            bubble->pos.y += bubble->vel.y * dt;
        }
    }

    // remove out-of-bounds bubbles
    for (i32 i = 0; i < bubbleGame->bubbles.numValues; i++) {
        Bubble *bubble = &bubbleGame->bubbles.values[i];
        if (bubble->active) {
            b32 shouldRemove = false;
            b32 earnGold = false;
            if (bubble->pos.x < -BUBBLE_RADIUS) { shouldRemove = true; }
            if (bubble->pos.x >= BUBBLE_ZONE_WIDTH + BUBBLE_RADIUS) { shouldRemove = true; }
            if (bubble->pos.y < -BUBBLE_RADIUS) { shouldRemove = true; }
            if (bubble->pos.y >= BUBBLE_ZONE_HEIGHT + BUBBLE_RADIUS) { 
                shouldRemove = true; 
                earnGold = true;
            }

            if (earnGold) {
                if (bubble->isDiamond) {
                    FloatingText *ft = spawnFloatingText();
                    if (ft) {
                        ft->amount = 1;
                        ft->icon = "diamond";
                        ft->startPos = bubble->pos;
                    }

                    bubbleGame->diamonds++;
                }
                else {
                    i32 gold = DROPPED_BUBBLE_GOLD;
                    if (bubble->type == BUBBLE_TYPE_UNIT) {
                        gold = costForUnit(bubble->unitType) / 2;
                    }
                    bubbleGame->gold += gold;

                    FloatingText *ft = spawnFloatingText();
                    if (ft) {
                        ft->amount = gold;
                        ft->icon = "gold";
                        ft->startPos = bubble->pos;
                    }
                }
            }

            if (shouldRemove) {
                removeBubble(bubble);
            }
        }
    }

    // Enemies
    // Spawn enemies
    EnemyWave *wave = &bubbleGame->waves.values[bubbleGame->waveIndex];
    bubbleGame->enemySpawnTimer += dt;
    if (bubbleGame->enemySpawnTimer >= wave->spawnTime) {
        if (wave->numEnemies > 0) {
            wave->numEnemies--;
            bubbleGame->enemySpawnTimer = 0.0f;
            Enemy *enemy = spawnEnemy();
            enemy->pos.x = -16.0f;
            enemy->pos.y = ENEMY_SPAWN_Y - (ENEMY_SPAWN_RANGE / 2.0f) + randomF32() * ENEMY_SPAWN_RANGE;
            enemy->type = getEnemyTypeByProbabilities(wave->enemyProbs);
            assignEnemyProps(enemy);
        }
        else if (!wave->spawnedBoss) {
            bubbleGame->enemySpawnTimer = 0.0f;

            Enemy *enemy = spawnEnemy();
            enemy->pos.x = 0.0f;
            enemy->pos.y = ENEMY_SPAWN_Y - (ENEMY_SPAWN_RANGE / 2.0f) + randomF32() * ENEMY_SPAWN_RANGE;
            enemy->type = wave->bossType;
            assignEnemyProps(enemy);

            enemy->size = wave->bossSize;
            enemy->scale = 1.0f + (enemy->size) * 0.2f;
            setEnemyPropsForScale(enemy);

            wave->spawnedBoss = true;
        }
    }

    for (i32 i = 0; i < bubbleGame->enemies.numValues; i++) {
        Enemy *enemy = &bubbleGame->enemies.values[i];
        if (enemy->active) {

            f32 walkSlowFactor = 1.0f / enemy->scale * (enemy->speed / 10.0f);
            enemy->walkTimer += dt * walkSlowFactor;
            if (enemy->walkTimer >= 0.5f) {
                enemy->walkTimer = 0.0f;
                enemy->walkFrame = (enemy->walkFrame + 1) % 2;
            }

            switch (enemy->state) {
                case ENEMY_STATE_WALKING: {
                    f32 enemySpeed = enemy->speed;
                    enemy->pos.x += enemySpeed * dt;

                    if (enemy->pos.x >= BASE_START_X) {
                        enemy->t = 0.0f;
                        enemy->startPos = enemy->pos;
                        enemy->state = ENEMY_STATE_ATTACKING_BASE;
                    }
                    else {
                        // TODO more efficient
                        unmarkSlots();
                        enemyTrySuckingBubble(enemy);
                    }
                } break;
                case ENEMY_STATE_SUCKING_IN_BUBBLE: {
                    updateEnemySuckingBubble(enemy, dt);
                } break;
                case ENEMY_STATE_GROWING: {
                    updateEnemyGrowing(enemy, dt);
                } break;
                case ENEMY_STATE_ATTACKING_BASE: {
                    updateEnemyAttackingBase(enemy, dt);
                } break;
                case ENEMY_STATE_DYING: {
                    updateDyingEnemy(enemy, dt);
                } break;
            }
        }
    }

    ScratchMemMarker m = pushScratchMemory();

    EnemyPtr_list sortedEnemies = EnemyPtr_listInit(scratchMemory, bubbleGame->enemies.numValues);
    for (i32 enemyIndex = 0; enemyIndex < bubbleGame->enemies.numValues; enemyIndex++) {
        Enemy *enemy = &bubbleGame->enemies.values[enemyIndex];
        if (enemy->active && enemy->state != ENEMY_STATE_DYING) {
            EnemyPtr_listPush(&sortedEnemies, enemy);
        }
    }
    sortEnemies(&sortedEnemies);

    // Units
    for (i32 bubbleIndex = 0; bubbleIndex < bubbleGame->bubbles.numValues; bubbleIndex++) {
        Bubble *bubble = &bubbleGame->bubbles.values[bubbleIndex];
        if (bubble->active && bubble->type == BUBBLE_TYPE_UNIT && bubble->unitType != UNIT_TYPE_FARMER) {
            bubble->cooldownTimer -= dt;
            if (bubble->cooldownTimer <= 0) {
                // TODO sort enemies?
                Enemy *closestEnemy = 0;
                f32 maxDist = 99999.0f;

                for (i32 enemyIndex = 0; enemyIndex < sortedEnemies.numValues; enemyIndex++) {
                    Enemy *enemy = sortedEnemies.values[enemyIndex];
                    if (enemy->active) {
                        vec2 bubbleToEnemy = vec2Subtract(enemy->pos, bubble->pos);
                        if (bubbleToEnemy.x - enemy->radius > bubble->range) {
                            continue;
                        }
                        if (bubbleToEnemy.x + enemy->radius < -bubble->range) {
                            break;
                        }
                        f32 distToEnemy = vec2Length(bubbleToEnemy);
                        if (distToEnemy < maxDist) {
                            closestEnemy = enemy;
                            maxDist = distToEnemy;
                        }
                    }
                }

                if (closestEnemy) {
                    Enemy *enemy = closestEnemy;
                    vec2 bubbleToEnemy = vec2Subtract(enemy->pos, bubble->pos);
                    f32 distToEnemy = vec2Length(bubbleToEnemy);

                    if (distToEnemy <= bubble->range + enemy->radius) {
                        bubble->cooldownTimer = bubble->cooldownDuration;

                        Projectile *p = spawnProjectile();
                        p->target = enemy->id;
                        p->pos = bubble->pos;

                        assignProjectileProps(p, bubble);

                        switch (p->upgradeType) {
                            case UPGRADE_TYPE_ARROW_ATK: {
                                                             soundManPlaySound("sfx_arrow", false);

                                                         } break;
                            case UPGRADE_TYPE_SLASH_ATK: {
                                                             soundManPlaySound("sfx_slash", false);

                                                         } break;
                            case UPGRADE_TYPE_CANNONBALL_ATK: {
                                                             soundManPlaySound("sfx_cannon", false);

                                                         } break;

                        }
                        break;
                    }
                }
            }
        }
        if (bubble->active && bubble->type == BUBBLE_TYPE_UNIT && bubble->unitType == UNIT_TYPE_FARMER) {
            bubble->cooldownTimer -= dt;
            if (bubble->cooldownTimer <= 0) {
                bubble->cooldownTimer = bubble->cooldownDuration;

                i32 goldAmt = 5 + bubbleGame->upgrades[UPGRADE_TYPE_FARMING] * 1;
                bubbleGame->gold += goldAmt;

                FloatingText *ft = spawnFloatingText();
                if (ft) {
                    ft->amount = goldAmt;
                    ft->icon = "gold";
                    ft->startPos = bubble->pos;
                }
            }
        }
    }

    // remove sorted enemy list
    popScratchMemory(m);

    // Projectiles
    for (i32 projectileIndex = 0; projectileIndex < bubbleGame->projectiles.numValues; projectileIndex++) {
        Projectile *projectile = &bubbleGame->projectiles.values[projectileIndex];
        if (projectile->active) {
            Enemy *enemy = getEnemyByID(projectile->target);
            if (enemy) {
                vec2 toEnemy = vec2Subtract(enemy->pos, projectile->pos);
                f32 toEnemyLength = vec2Length(toEnemy);
                if (toEnemyLength < 1.5f) {

                    f32 damage = damageByUpgradeType(projectile->upgradeType);
                    if (enemy->resists == projectile->upgradeType) {
                        damage /= 2.0f;
                    }
                    if (enemy->weakness == projectile->upgradeType) {
                        damage *= 2.0f;
                    }
                    enemy->hitPoints -= damage;

                    if (projectile->upgradeType == UPGRADE_TYPE_CANNONBALL_ATK) {
                        splashDamageNearbyEnemies(projectile);
                    }

                    soundManPlaySound("sfx_enemy_hit", false);

                    FloatingText *ft = spawnFloatingText();
                    if (ft) {
                        ft->amount = (i32)damage;
                        ft->icon = 0;
                        ft->startPos = enemy->pos;
                    }

                    if (enemy->hitPoints <= 0.0f) {
                        Bubble *suckedBubble = getBubbleByID(enemy->suckingBubbleID);
                        if (suckedBubble) {
                            suckedBubble->falling = true;
                        }
                        enemy->state = ENEMY_STATE_DYING;
                        enemy->t = 0.0f;
                soundManPlaySound("sfx_enemy_killed", false);
                    }

                    removeProjectile(projectile);
                }
                else {
                    toEnemy = vec2Normalize(toEnemy);
                    vec2 vel = vec2ScalarMul(projectile->speed, toEnemy);
                    vel = vec2ScalarMul(dt, vel);
                    projectile->pos = vec2Add(projectile->pos, vel);
                    vec2 toEnemyNormalized = vec2Normalize(toEnemy);
                    projectile->angle = arctangent2(toEnemyNormalized.y, toEnemyNormalized.x);
                    projectile->angle /= (2.0f * PI);
                }
            }
            else {
                removeProjectile(projectile);
            }
        }
    }

    // Floating texts 
    for (i32 i = 0; i < bubbleGame->floatingTexts.numValues; i++) {
        FloatingText *ft = &bubbleGame->floatingTexts.values[i];
        if (ft && ft->active) {
            ft->t += dt;
            f32 riseOffset = 16.0f;
            if (ft->t <= FLOATING_TEXT_RISE_DURATION) {
                f32 t = ft->t / FLOATING_TEXT_RISE_DURATION;
                vec2 pos = ft->startPos;
                pos.y = pos.y + t * (-riseOffset);
                ft->pos = pos;
            }
            else if (ft->t <= FLOATING_TEXT_TOTAL_DURATION) {

            }
            else {
                removeFloatingText(ft);
            }
        }
    }

    i32 numActiveEnemies = 0;
    for (i32 i = 0; i < bubbleGame->enemies.numValues; i++) {
        Enemy *enemy = &bubbleGame->enemies.values[i];
        if (enemy->active) {
            numActiveEnemies++;
        }
    }
    if (numActiveEnemies == 0 && wave->spawnedBoss) {
        bubbleGame->waveIndex++;
        bubbleGame->enemySpawnTimer = 0.0f;
        if (bubbleGame->waveIndex >= bubbleGame->waves.numValues) {
            bubbleGame->win = true;
        }
    }

    for (i32 i = 0; i < bubbleGame->particles.numValues; i++) {
        BubbleParticles *particles = &bubbleGame->particles.values[i];
        if (particles->active) {
            particles->t += dt;
            if (particles->t >= PARTICLE_BURST_TIME) {
                removeBubbleParticles(particles);
            }
        }
    }

    i32 lowestRow = 0;
    for (i32 i = 0; i < GRID_NUM_ROWS; i++) {
        for (i32 j = 0; j < GRID_NUM_COLS; j++) { 
            Bubble *bubble = tryGetBubbleAtRowCol(i, j);
            if (bubble) {
                lowestRow = i;
            }
        }
    }

    if (lowestRow > 18) {
        bubbleGame->tooLowDanger = true;
        bubbleGame->tooLowDangerTimer += dt;
        if (bubbleGame->tooLowDangerTimer >= 0.75f) {
            bubbleGame->tooLowDangerTimer = 0.0f;
            bubbleGame->lineRed = !bubbleGame->lineRed;
        }
    }
    else {
        bubbleGame->tooLowDanger = true;
        bubbleGame->tooLowDangerTimer = 0.0f;
        bubbleGame->lineRed = false;
    }

    if (lowestRow > 21) {
        bubbleGame->gameOver = true;
    }
    if (bubbleGame->bubblesBurst) {
        soundManPlaySound("sfx_pop", false);
    }
}

char *bubbleColorToFrameName (BubbleColor color) {
    switch (color) {
    case BUBBLE_COLOR_RED: 
        return "bubble_red";
    case BUBBLE_COLOR_YELLOW: 
        return "bubble_yellow";
    case BUBBLE_COLOR_GREEN: 
        return "bubble_green";
    case BUBBLE_COLOR_BLUE: 
        return "bubble_blue";
    case BUBBLE_COLOR_PURPLE: 
        return "bubble_purple";
    }
    return "bubble_red";
}

void setBubbleSpriteFrameKey (sprite *s, BubbleType type, BubbleColor color, UnitType unitType, b32 isDiamond) {
    s->frameKey = "bubble_red";

    if (isDiamond) {
        s->frameKey = "diamond";
    }
    else if (type == BUBBLE_TYPE_NORMAL) {
        s->frameKey = bubbleColorToFrameName(color);
    }
    else if (type == BUBBLE_TYPE_UNIT) {
        switch (unitType) {
            case UNIT_TYPE_ARCHER: {
                s->frameKey = "archer_0";
            } break;
            case UNIT_TYPE_KNIGHT: {
                s->frameKey = "knight_0";
            } break;
            case UNIT_TYPE_CANNON: {
                s->frameKey = "cannon_0";
            } break;
            case UNIT_TYPE_FARMER: {
                s->frameKey = "farmer_0";
            } break;
        }
    }
}

void setEnemySpriteFrameKey (sprite *s, EnemyType type, i32 frame) {
    switch (type) {
        case ENEMY_TYPE_GOBLIN:{
            if (frame == 0) {
                s->frameKey = "goblin_0";
            }
            else {
                s->frameKey = "goblin_1";
            }
        } break;
        case ENEMY_TYPE_GARGOYLE:{
            if (frame == 0) {
                s->frameKey = "gargoyle_0";
            }
            else {
                s->frameKey = "gargoyle_1";
            }
        } break;
        case ENEMY_TYPE_RAT:{
            if (frame == 0) {
                s->frameKey = "rat_0";
            }
            else {
                s->frameKey = "rat_1";
            }
        } break;
        case ENEMY_TYPE_BLACK_KNIGHT:{
            if (frame == 0) {
                s->frameKey = "black_knight_0";
            }
            else {
                s->frameKey = "black_knight_1";
            }
        } break;
    }
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

void drawBubbleGame (bubble_game *bg, plat_api platAPI) {
    if (bubbleGame->state == GAME_STATE_TITLE) {
        sprite_text labelText = {
            .text = "Battle Bobble",
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

        return;
    }

    {
        sprite s = defaultSprite();
        s.pos = (vec2) { .x = 0.0f, .y = 0.0f };
        s.textureKey = "background";
        spriteManAddSprite(s);
    }

    i32 numDestroyedHouses = MAX_LIVES - bubbleGame->lives;
    for (i32 i = 0; i < numDestroyedHouses && i < MAX_LIVES; i++) {
            sprite s = defaultSprite();
            i32 col = i / 2;
            i32 row = i % 2;
            s.pos = (vec2){ .x = 422.0f, .y = 173.0f };
            s.pos = vec2Add(s.pos, (vec2){ .x = (f32)col * 32.0f, .y = (f32)row * 25.0f });
            s.atlasKey = "atlas";
            s.frameKey = "house_destroyed";
            s.anchor = (vec2){ .x = 0.5f, .y = 1.0f };
            spriteManAddSprite(s);
    }

    for (i32 i = numDestroyedHouses; i < MAX_LIVES; i++) {
            sprite s = defaultSprite();
            i32 col = i / 2;
            i32 row = i % 2;
            s.pos = (vec2){ .x = 422.0f, .y = 173.0f };
            s.pos = vec2Add(s.pos, (vec2){ .x = (f32)col * 32.0f, .y = (f32)row * 25.0f });
            s.atlasKey = "atlas";
            s.frameKey = "house";
            s.anchor = (vec2){ .x = 0.5f, .y = 1.0f };
            spriteManAddSprite(s);
    }


    spriteManPushTransform((sprite_transform){
            .pos = (vec2){ .x = 16.0f, .y = 16.0f },
            .scale = 1.0f
    });

    for (i32 i = 0; i < bubbleGame->enemies.numValues; i++) {
        Enemy *enemy = &bubbleGame->enemies.values[i];
        if (enemy->active) {
            sprite s = defaultSprite();
            s.pos = enemy->pos;
            s.atlasKey = "atlas";
            setEnemySpriteFrameKey(&s, enemy->type, enemy->walkFrame);
            s.anchor = (vec2){ .x = 0.5f, .y = 0.5f };
            s.scale = enemy->scale;

            if (enemy->state == ENEMY_STATE_DYING) {
                s.tint = 0xff0000;
                f32 t = enemy->t / 0.5f;
                f32 deathScale = (1.0f - t) * 1.0f;
                s.scale *= deathScale;
            }

            spriteManAddSprite(s);
        }
    }

    for (i32 i = 0; i < bubbleGame->bubbles.numValues; i++) {
        Bubble *bubble = &bubbleGame->bubbles.values[i];
        if (bubble->active) {
            sprite s = defaultSprite();
            s.pos = bubble->pos;
            s.atlasKey = "atlas";
            setBubbleSpriteFrameKey(&s, bubble->type, bubble->color, bubble->unitType, bubble->isDiamond);
            s.anchor = (vec2){ .x = 0.5f, .y = 0.5f };
            spriteManAddSprite(s);
        }
    }

    vec2 launcherAngle = (vec2){ .y = bubbleGame->launcherTarget.y - LAUNCHER_POS_Y, .x = bubbleGame->launcherTarget.x - LAUNCHER_POS_X };
    launcherAngle = vec2Normalize(launcherAngle);
    f32 angle = arctangent2(launcherAngle.y, launcherAngle.x);
    {
        sprite s = defaultSprite();
        s.pos = (vec2){ .x = LAUNCHER_POS_X, .y = LAUNCHER_POS_Y };
        s.rotation = angle / (2.0f * PI);
        s.atlasKey = "atlas";
        s.frameKey = "launcher";
        s.anchor = (vec2){ .x = 0.5f, .y = 0.5f };
        spriteManAddSprite(s);
    }

    for (i32 i = 0; i < bubbleGame->bubbleQueue.numValues; i++) {
        sprite s = defaultSprite();
        s.pos = (vec2){ .x = LAUNCHER_POS_X + 24.0f + i * 16.0f, .y = LAUNCHER_POS_Y + 8.0f };
        s.atlasKey = "atlas";
        BubbleQueueItem item = bubbleGame->bubbleQueue.values[i];
        setBubbleSpriteFrameKey(&s, item.type, item.color, item.unitType, false);
        s.anchor = (vec2){ .x = 0.5f, .y = 0.5f };
        spriteManAddSprite(s);
    }


    for (i32 i = 0; i < bubbleGame->projectiles.numValues; i++) {
        Projectile *projectile = &bubbleGame->projectiles.values[i];
        if (projectile->active) {
            sprite s = defaultSprite();
            s.pos = projectile->pos;
            s.atlasKey = "atlas";
            s.frameKey = projectile->frame;
            s.rotation = projectile->angle;
            s.anchor = (vec2){ .x = 0.5f, .y = 0.5f };
            spriteManAddSprite(s);
        }
    }

    spriteManPopMatrix();

    for (i32 i = 0; i < bubbleGame->buttons.numValues; i++) {
        Button *b = &bubbleGame->buttons.values[i];
        {
            sprite s = defaultSprite();
            s.pos = b->pos;
            s.atlasKey = "atlas";
            s.frameKey = b->upFrame;
            if (b->isOver) {
                s.frameKey = b->overFrame;
            }
            if (b->isDown) {
                s.frameKey = b->downFrame;
            }
            spriteManAddSprite(s);
        }

        if (b->icon != 0) {
            sprite s = defaultSprite();
            s.pos = b->pos;
            s.pos = vec2Add(s.pos, (vec2){ .x = 24.0f, .y = 16.0f});
            s.atlasKey = "atlas";
            s.frameKey = b->icon;
            s.anchor = (vec2){ .x = 0.5f, .y = 0.5f };
            spriteManAddSprite(s);
        }

        if (b->label != 0) {
            vec2 labelPos = b->pos;
            labelPos = vec2Add(labelPos, (vec2){ .x = 60.0f, .y = 16.0f});
            labelPos = vec2Add(labelPos, b->labelOffset);

            sprite_text labelText = {
                .text = b->label,
                .fontKey = "font",
                .x = labelPos.x,
                .y = labelPos.y
            };
            centerText(&labelText);
            spriteManAddText(labelText);
            if (b->labelIcon != 0) {
                f32 labelWidth = textWidth(&labelText);

                sprite s = defaultSprite();
                s.pos = (vec2){ .x = labelText.x, .y = labelText.y };
                s.pos = vec2Add(s.pos, (vec2){ .x = labelWidth + 10.0f, .y = 4.0f});
                s.atlasKey = "atlas";
                s.frameKey = b->labelIcon;
                s.anchor = (vec2){ .x = 0.5f, .y = 0.5f };
                spriteManAddSprite(s);
            }
        }
    }

    // Gold count
    sprite_text goldText = {
        .text = tempStringFromI32(bubbleGame->gold),
        .fontKey = "font",
        .x = 519.0f,
        .y = 331.0f
    };
    centerText(&goldText);
    spriteManAddText(goldText);

    f32 goldTextWidth = textWidth(&goldText);
    {
        sprite s = defaultSprite();
        s.pos = (vec2) {
            .x = goldText.x + goldTextWidth + 10.0f,
            .y = goldText.y + 4.0f
        };
        s.atlasKey = "atlas";
        s.frameKey = "gold";
        s.anchor = (vec2){ .x = 0.5f, .y = 0.5f };
        spriteManAddSprite(s);
    }

    // Diamond count
    sprite_text diamondText = {
        .text = tempStringFromI32(bubbleGame->diamonds),
        .fontKey = "font",
        .x = 519.0f,
        .y = 20.0f
    };
    centerText(&diamondText);
    spriteManAddText(diamondText);

    f32 diamondTextWidth = textWidth(&diamondText);
    {
        sprite s = defaultSprite();
        s.pos = (vec2) {
            .x = diamondText.x + diamondTextWidth + 10.0f,
            .y = diamondText.y + 4.0f
        };
        s.atlasKey = "atlas";
        s.frameKey = "diamond";
        s.anchor = (vec2){ .x = 0.5f, .y = 0.5f };
        spriteManAddSprite(s);
    }


    {
        sprite s = defaultSprite();
        s.pos = (vec2) { .x = 0.0f, .y = 0.0f };
        s.textureKey = "frame";
        spriteManAddSprite(s);
    }

    // upgrade zone
    {
        sprite s = defaultSprite();
        s.pos = (vec2) { .x = 444.0f, .y = 64.0f };
        s.atlasKey = "atlas";
        s.frameKey = "arrow";
        s.anchor = (vec2){ .x = 0.5f, .y = 0.5f };
        spriteManAddSprite(s);
    }
    {
        char *text = "+";
        char *upgradeCount = tempStringFromI32(bubbleGame->upgrades[UPGRADE_TYPE_ARROW_ATK]);
        sprite_text t = {
            .text = tempStringAppend(text, upgradeCount),
            .fontKey = "font",
            .x = 424.0f,
            .y = 64.0f
        };
        centerText(&t);
        spriteManAddText(t);
    }

    {
        sprite s = defaultSprite();
        s.pos = (vec2) { .x = 556.0f, .y = 64.0f };
        s.atlasKey = "atlas";
        s.frameKey = "slash";
        s.anchor = (vec2){ .x = 0.5f, .y = 0.5f };
        spriteManAddSprite(s);
    }
    {
        char *text = "+";
        char *upgradeCount = tempStringFromI32(bubbleGame->upgrades[UPGRADE_TYPE_SLASH_ATK]);
        sprite_text t = {
            .text = tempStringAppend(text, upgradeCount),
            .fontKey = "font",
            .x = 536.0f,
            .y = 64.0f
        };
        centerText(&t);
        spriteManAddText(t);
    }

    {
        sprite s = defaultSprite();
        s.pos = (vec2) { .x = 444.0f, .y = 112.0f };
        s.atlasKey = "atlas";
        s.frameKey = "cannonball";
        s.anchor = (vec2){ .x = 0.5f, .y = 0.5f };
        spriteManAddSprite(s);
    }
    {
        char *text = "+";
        char *upgradeCount = tempStringFromI32(bubbleGame->upgrades[UPGRADE_TYPE_CANNONBALL_ATK]);
        sprite_text t = {
            .text = tempStringAppend(text, upgradeCount),
            .fontKey = "font",
            .x = 424.0f,
            .y = 112.0f
        };
        centerText(&t);
        spriteManAddText(t);
    }

    {
        sprite s = defaultSprite();
        s.pos = (vec2) { .x = 556.0f, .y = 112.0f };
        s.atlasKey = "atlas";
        s.frameKey = "pitchfork";
        s.anchor = (vec2){ .x = 0.5f, .y = 0.5f };
        spriteManAddSprite(s);
    }
    {
        char *text = "+";
        char *upgradeCount = tempStringFromI32(bubbleGame->upgrades[UPGRADE_TYPE_FARMING]);
        sprite_text t = {
            .text = tempStringAppend(text, upgradeCount),
            .fontKey = "font",
            .x = 536.0f,
            .y = 112.0f
        };
        centerText(&t);
        spriteManAddText(t);
    }


    spriteManPushTransform((sprite_transform){
            .pos = (vec2){ .x = 16.0f, .y = 16.0f },
            .scale = 1.0f
    });

    for (i32 i = 0; i < bubbleGame->floatingTexts.numValues; i++) {
        FloatingText *floatingText = &bubbleGame->floatingTexts.values[i];
        if (floatingText->active) {
            sprite_text labelText = {
                .text = tempStringFromI32(floatingText->amount),
                .fontKey = "font",
                .x = floatingText->pos.x,
                .y = floatingText->pos.y
            };
            centerText(&labelText);
            spriteManAddText(labelText);

            if (floatingText->icon) {
                f32 ftTextWidth = textWidth(&labelText);
                {
                    sprite s = defaultSprite();
                    s.pos = (vec2) {
                        .x = labelText.x + ftTextWidth + 10.0f,
                        .y = labelText.y + 4.0f
                    };
                    s.atlasKey = "atlas";
                    s.frameKey = floatingText->icon;
                    s.anchor = (vec2){ .x = 0.5f, .y = 0.5f };
                    spriteManAddSprite(s);
                }
            }
        }
    }

    for (i32 i = 0; i < bubbleGame->particles.numValues; i++) {
        BubbleParticles *particles = &bubbleGame->particles.values[i];
        if (particles->active) {
            f32 t = particles->t / PARTICLE_BURST_TIME;
            for (i32 pIndex = 0; pIndex < 5; pIndex++) {
                f32 startScale = 0.5f;
                f32 endScale = 0.0f;
                f32 dx = fastCos2PI(particles->angles[pIndex]);
                f32 dy = fastSin2PI(particles->angles[pIndex]);
                vec2 dir = (vec2){ .x = dx, .y = dy };
                dir = vec2ScalarMul(16.0f, dir);
                vec2 endPos = vec2Add(particles->pos, dir);

                sprite s = defaultSprite();
                s.pos = vec2Add(vec2ScalarMul((1.0f - t), particles->pos),
                                vec2ScalarMul(t, endPos));
                s.scale = (1.0f - t) * startScale + t * endScale;
                s.atlasKey = "atlas";
                setBubbleSpriteFrameKey(&s, BUBBLE_TYPE_NORMAL, particles->color, UNIT_TYPE_ARCHER, false);
                s.anchor = (vec2){ .x = 0.5f, .y = 0.5f };
                spriteManAddSprite(s);
            }
        }
    }

    {
        sprite s = defaultSprite();
        s.pos = (vec2) { .x = 1.0f, .y = 295.0f };
        s.atlasKey = "atlas";
        s.frameKey = "dashed_lines";
        if (bubbleGame->lineRed) {
            s.tint = 0xff0000;
        }
        spriteManAddSprite(s);
    }

    spriteManPopMatrix();

    {
        sprite_text labelText = {
            .text = tempStringAppend("wave ", tempStringFromI32(bubbleGame->waveIndex + 1)),
            .fontKey = "font",
            .x = 39.0f,
            .y = 344.0f
        };
        centerText(&labelText);
        spriteManAddText(labelText);
    }

    {

        u32 time = (u32)bubbleGame->gameTime;
        u32 seconds = time % 60;
        u32 minutes = time / 60;
        u32 hours = time / 3600;
        char *timeString = tempStringFromI32((i32)hours);
        timeString = tempStringAppend(timeString, ":");
        if (minutes < 10) {
            timeString = tempStringAppend(timeString, "0");
        }
        timeString = tempStringAppend(timeString, tempStringFromI32((i32)minutes));
        timeString = tempStringAppend(timeString, ":");
        if (seconds < 10) {
            timeString = tempStringAppend(timeString, "0");
        }
        timeString = tempStringAppend(timeString, tempStringFromI32((i32)seconds));

        sprite_text labelText = {
            .text = timeString,
            .fontKey = "font",
            .x = 374.0f,
            .y = 343.0f
        };
        centerText(&labelText);
        spriteManAddText(labelText);
    }

    if (bubbleGame->gameOver) {
        {
            sprite s = defaultSprite();
            s.pos = (vec2) { .x = 320.0f, .y = 180.0f };
            s.atlasKey = "atlas";
            s.frameKey = "message_panel";
                s.anchor = (vec2){ .x = 0.5f, .y = 0.5f };
            spriteManAddSprite(s);
        }

        sprite_text labelText = {
            .text = "Game Over",
            .fontKey = "font",
            .x = 320.0f,
            .y = 180.0f
        };
        centerText(&labelText);
        spriteManAddText(labelText);
    }

    if (bubbleGame->win) {
        {
            sprite s = defaultSprite();
            s.pos = (vec2) { .x = 320.0f, .y = 180.0f };
            s.atlasKey = "atlas";
            s.frameKey = "message_panel";
                s.anchor = (vec2){ .x = 0.5f, .y = 0.5f };
            spriteManAddSprite(s);
        }

        sprite_text labelText = {
            .text = "You Win!",
            .fontKey = "font",
            .x = 320.0f,
            .y = 180.0f
        };
        centerText(&labelText);
        spriteManAddText(labelText);
    }

}
