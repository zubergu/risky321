#ifndef _UMALLOC_H
#define _UMALLOC_H

#include "../kernel/types.h"

struct malloc_header
{
    struct malloc_header *prev;
    struct malloc_header *next;
    uint32_t size;
    uint32_t free;
};

void * umalloc(uint32_t bytes);

void ufree(void * chunk);

void * umorecore();

#endif
