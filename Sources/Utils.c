#include "Utils.h"
#include "IO_tty.h"
#include "Navig.h"
#include <stdio.h>

#ifdef DEBUG_MEM

#undef malloc
#undef free

static long nb_alloc = 0, nb_free = 0, mem_alloc = 0;
void* AllocMem(size_t size)
{
    mem_alloc += size;
    nb_alloc++;
    return malloc(size);
}

void FreeMem(void* mem)
{
    if (!mem) {
        quit("tried to free a null pointer!", EXIT_FAILURE);
    } else if (nb_free++ == nb_alloc) {
        quit("double free!", EXIT_FAILURE);
    } else {
        free(mem);
    }
}

#define malloc(X) AllocMem(X)
#define free(X) FreeMem(X)

static void PrintMemDebugMsg()
{
    fprintf(stderr,"Mem usage: %ld alloc (%ld bytes), %ld free\n",
        nb_alloc, mem_alloc, nb_free);
}

#endif



/*** Exit the program ***/
void quit(char* msg, int status)
{
    static int pending_quit = 0;

    /* prevent infinite recursion */
    if (!pending_quit) {
        pending_quit = 1;

        set_mode(MODE_CANONICAL);
        PopAGNodes();
    }

    if (msg) {
        fprintf(stderr, "%s%s", msg, "\n");
    }

#ifdef DEBUG_MEM
    PrintMemDebugMsg();
#endif

    exit(status);
}
