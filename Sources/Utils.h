#ifndef UTILS_H
#define UTILS_H

#ifdef DEBUG_MEM /* Track memory deallocation */

#include <stdlib.h>

void* AllocMem(size_t size);
void FreeMem(void* mem);

#define free(X) FreeMem(X)
#define malloc(X) AllocMem(X)

#endif /* DEBUG_MEM */

void quit(char* msg, int status);

#endif