#include "kmem.h"
#include "system.h"
#include "types.h"
#include "kprintf.h"
#include "klibc.h"
#include "kvmem.h"

extern char __free_ram_begin[]; /* symbol created by linker in kernel.ld */
extern char __free_ram_end[];   /* symbol created by linker in kernel.ld */

static struct kmem_freepage *head = NULL;

/* initialize kernel memory */
void kmem_init(void)
{
    struct kmem_freepage *free_ptr = (struct kmem_freepage *) __free_ram_begin;
    while(free_ptr < (struct kmem_freepage *) __free_ram_end)
    {
        kmem_free((void*) free_ptr);
        free_ptr = (struct kmem_freepage *)((void *) free_ptr + PAGE_SIZE);
    }
}

/* Allocate a single 4096 bytes physical memory page */
void *kmem_alloc(void)
{
    void *page;

    if(head)
    {
        page = (void *)head;
        head = head->next;
        memset(page, 0, PAGE_SIZE);
        return page;
    }

    kpanic(__FILE__, __LINE__, "kernel out of memory!");

    /* should be unreachable */
    return NULL;
    
}

/* Free a single 4096 bytes page pointed by page_ptr */
void kmem_free(void *page_ptr)
{
    struct kmem_freepage *freed = (struct kmem_freepage *)page_ptr;
    freed->next = head;
    head = freed;
}
