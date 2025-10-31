#ifndef _KTRAP_H
#define _KTRAP_H

#include "types.h"


struct trap_frame
{
    uint32_t kernel_satp;   /* unused? we map all kernel to user space */
    uint32_t kernel_sp;     /* process's top of kernel stack pointer */
    uint32_t kernel_trap;   /* initialized with address of usertrap */
    uint32_t epc;           /* saved user program counter while trap happened*/
    uint32_t kernel_hartid; /* unused for now, just on hart */
    uint32_t ra;
    uint32_t sp;
    uint32_t gp;
    uint32_t tp;
    uint32_t t0;
    uint32_t t1;
    uint32_t t2;
    uint32_t s0;
    uint32_t s1;
    uint32_t a0;
    uint32_t a1;
    uint32_t a2;
    uint32_t a3;
    uint32_t a4;
    uint32_t a5;
    uint32_t a6;
    uint32_t a7;
    uint32_t s2;
    uint32_t s3;
    uint32_t s4;
    uint32_t s5;
    uint32_t s6;
    uint32_t s7;
    uint32_t s8;
    uint32_t s9;
    uint32_t s10;
    uint32_t s11;
    uint32_t t3;
    uint32_t t4;
    uint32_t t5;
    uint32_t t6;
} __attribute__((packed));

void ktrap_init();
void ktrap_from_user_handler(void);
void ktrap_to_user(void);
void ktrap_handler(struct trap_frame *frame);
void kdump_trapframe(struct trap_frame *frame);


#endif