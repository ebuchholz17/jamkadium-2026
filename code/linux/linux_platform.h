#ifndef LINUX_PLATFORM_H   
#define LINUX_PLATFORM_H

#include "../game/gng_types.h"

#define LINUX_FILES_TO_LOAD_MAX 1000

typedef struct linux_file_to_load {
    char *id;
    char *path;
} linux_file_to_load;


#endif
