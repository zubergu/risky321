#include "kvmem.h"
#include "kmem.h" /* kmem_alloc */
#include "system.h" /* for panic */
#include "csr.h"
#include "kprintf.h"
#include "klibc.h" /* memcpy */

extern uint8_t __kernel_base[];
extern uint8_t __free_ram_end[];

uint32_t *kernel_pagetable = NULL;


/* turn on memory paging in CPU */
void kvmem_init_hart()
{
    uint32_t value = SATP_SV32 | ((uint32_t)kernel_pagetable >> 12);

    __asm__ __volatile__("sfence.vma");
    WRITE_CSR("satp", value);
    __asm__ __volatile__("sfence.vma");
}

void kvmem_init_kernel()
{
    kernel_pagetable = kmem_alloc();

    for(uint32_t pa = 0; (uint8_t *)pa < __free_ram_end; pa += PAGE_SIZE)
    {
        kvmem_map_page(kernel_pagetable, pa, pa, PTE_R | PTE_W | PTE_X);
    }
}

void kvmem_map_page(uint32_t *table, uint32_t va, uint32_t pa, uint32_t flags)
{
    /*
     * TODO: add checks for page alignment in memory 
    */

    uint32_t vpn1 = (va >> 22) & 0x3FF;  /* first index  */
    uint32_t vpn0 = (va >> 12) & 0x3FF;  /* second index */

    if( (table[vpn1] & PTE_V) == 0 ) /* if entry not valid then allocate 1st level table*/
    {
        table[vpn1] = (uint32_t)kmem_alloc();
        table[vpn1] = (table[vpn1] & 0xFFFFF000); /* remove 12-bit offset */
        table[vpn1] = (table[vpn1] >> 2) | PTE_V; /* move by 2 bits so PPN occupies 22 bits, and set valid flag*/
    }    

    uint32_t *table0 = (uint32_t *)((table[vpn1] & 0xFFFFFC00) << 2); /* remove 10 bit flags and move by 2 bits to make space for offset */
    table0[vpn0] = (pa & 0xFFFFF000); /* remove 12 bit offset*/
    table0[vpn0] = (table0[vpn0] >> 2) | flags | PTE_V; /* store remaining 20 bits on 22 bit space + 10 bit flags*/
}

uint32_t * kvmem_copy_pagetable(uint32_t *src_page)
{
    uint32_t *dest_page = kmem_alloc(); 
    for(int i = 0; i < 1024; i++)
    {
        uint32_t pte = src_page[i];

        if(pte & PTE_V)
        {
            uint32_t *pte_base_addr = (uint32_t *)((pte & 0xFFFFFC00) << 2);

            uint32_t * dest_base_addr = kmem_alloc();
            dest_page[i] = (uint32_t)dest_base_addr;
            dest_page[i] = (dest_page[i] & 0xFFFFF000); /* remove 12-bit offset */
            dest_page[i] = (dest_page[i] >> 2) | PTE_V; /* move by 2 bits so PPN occupies 22 bits, and set valid flag */

            for(int j = 0; j < 1024; j++)
            {
                if(pte_base_addr[j] & PTE_V)
                {
                    dest_base_addr[j] = pte_base_addr[j];
                    /* if valid entry is for user space then allocate new physical page */
                    if(pte_base_addr[j] & PTE_U)
                    {
                        uint32_t new_pp = (uint32_t)kmem_alloc();
                        dest_base_addr[j] = dest_base_addr[j] & 0x000003FF; /* clear previous physical page number */
                        dest_base_addr[j] = dest_base_addr[j] | ((new_pp & 0xFFFFF000) >> 2); /* insert new 22-bit physical page number with with 12-bit offset, 10bit flags*/

                        /* copy actual data from source page to allocated destination page */
                        memcpy((void*)new_pp, (const void *)((pte_base_addr[j] & 0xFFFFFC00) << 2), PAGE_SIZE);
                    }

                }
            }
        }
    }

    return dest_page;
}

void * kvmem_get_pa(uint32_t *pagetable, uint32_t va)
{
    uint32_t vpn1 = (va >> 22) & 0x3FF;  /* first index  */
    uint32_t vpn0 = (va >> 12) & 0x3FF;  /* second index */

    if( pagetable[vpn1] & PTE_V)
    {
        uint32_t *table0 = (uint32_t *)((pagetable[vpn1] & 0xFFFFFC00) << 2); /* remove 10 bit flags and move by 2 bits to make space for offset */
        if(table0[vpn0] & PTE_V)
        {
            return (void *)((table0[vpn0] & 0xFFFFFC00) << 2);
        }
        return (void*)-1;
    }

    return (void*)-1;
}
