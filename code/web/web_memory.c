#include "web_memory.h"
#include "../game/gng_types.h"
#include "web_platform.h"

u32 memCapacity;
u32 tempMemCapacity;

extern u8 __heap_base;
u8 *memStart = &__heap_base;
u8 *memCurrent = &__heap_base;

// grow temp memory from other side of the heap
u8 *tempMemStart = &__heap_base;
u8 *tempMemCurrent = &__heap_base;

WASM_EXPORT void *webAllocMemory (u32 size) {
    u32 align = 8;
    memCurrent = (u8 *)(((u32)memCurrent + align - 1) & ~(align - 1));

    u32 memSize = memCurrent - memStart;
    if (size > memCapacity - memSize) {
        onError("Memory capacity exceeded during webAllocMemory");
    }

    u8 *result = memCurrent;
    memCurrent += size;
    return (void *)result;
}

// per-frame temporary memory
WASM_EXPORT void *webAllocTempMemory (u32 size) {
    u32 align = 8;
    tempMemCurrent = (u8 *)(((u32)tempMemCurrent + align - 1) & ~(align - 1));

    u32 tempMemSize = tempMemCurrent - tempMemStart;

    // don't let temporary memory flood into 'permanent' memory
    if (size > tempMemCapacity - tempMemSize) {
        onError("Temp memory capacity exceeded during webAllocTempMemory");
    }

    u8 *result = tempMemCurrent;
    tempMemCurrent += size;
    return (void *)result;
}

void webFreeTempMemory () {
    tempMemCurrent = tempMemStart;
}

