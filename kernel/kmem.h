#ifndef _KALLOC_H
#define _KALLOC_H

#include "types.h"

struct kmem_freepage
{
    struct kmem_freepage *next;
};

void  kmem_init(void);
void *kmem_alloc(void);
void  kmem_free(void *page);

#endif