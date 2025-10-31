#ifndef _KVMEM_H
#define _KVMEM_H

#include "types.h"

#define PAGE_SIZE              4096 /* memory page size in bytes */

#define VA_MAX                 0xFFFFFFFF
#define VA_TRAPFRAME           (VA_MAX - PAGE_SIZE + 1)
#define VA_USER_BASE           0xC0000000
#define VA_USER_STACK_INIT     VA_TRAPFRAME    // stack starts towards lower addresses from the same address that trapframe begins 
#define VA_USER_STACK          VA_USER_STACK_INIT - PAGE_SIZE


/* PAGING TABLE ENTRY FLAGS */
#define PTE_V (1L << 0) // PTE VALID BIT
#define PTE_R (1L << 1) // READABLE
#define PTE_W (1L << 2) // WRITABLE
#define PTE_X (1L << 3) // PTE EXECUTABLE
#define PTE_U (1L << 4) // USER MODE CAN ACCESS

void kvmem_init_hart();
void kvmem_init_kernel();
void kvmem_map_page(uint32_t *table, uint32_t va, uint32_t pa, uint32_t flags);
uint32_t * kvmem_copy_pagetable(uint32_t *src_page);
void * kvmem_get_pa(uint32_t *pagetable, uint32_t va);


#endif