#ifndef GNG_STRING_H
#define GNG_STRING_H

#include "gng_types.h"

b32 stringEquals (char *a, char *b);
char *tempStringAppend (char *first, char *second) ;
char *tempStringFromI32 (i32 num);
char *stringFromI32 (i32 num, char *buffer, u32 bufferLength);
char *stringFromI32Unterminated (i32 num, char *buffer, u32 bufferLength);

#endif
