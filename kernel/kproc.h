#ifndef _PROC_H
#define _PROC_H

#include "types.h"
#include "ktrap.h"
#include "filesystem.h"

#define NUM_PROCESSES  8

#define MAX_OPEN_FILES 8

struct context
{
    uint32_t ra;
    uint32_t sp;

    /* callee-saved registers */
    uint32_t  s0;
    uint32_t  s1;
    uint32_t  s2;
    uint32_t  s3;
    uint32_t  s4;
    uint32_t  s5;
    uint32_t  s6;
    uint32_t  s7;
    uint32_t  s8;
    uint32_t  s9;
    uint32_t s10;
    uint32_t s11;
};

enum procstate
{
    UNUSED,
    USED,
    SLEEPING,
    RUNNABLE,
    RUNNING,
};

struct process
{
    uint32_t pid;
    enum procstate state;
    struct process *parent;
    struct context context;
    struct trap_frame *trapframe;
    uint32_t *pagetable;
    uint32_t *user_stack;

    uint32_t free_heap_page;

    struct os_file *opened_files[MAX_OPEN_FILES];

    uint32_t kernel_stack_top; /* kernel stack pointer */
    uint8_t  kernel_stack[8192]; /* kernel stack private to single process */
};


extern void swtch(struct context *old, struct context *new);


void kproc_init();

struct process *kproc_create(uint32_t *entry_point);
void kproc_free();

void kproc_scheduler();
void kproc_yield();
uint32_t kproc_kill(uint32_t pid);
uint32_t kproc_file_add(struct os_file * opened_file);
struct os_file *kproc_file_get(uint32_t fd);
uint32_t kproc_file_remove(uint32_t fd);
uint32_t kproc_fork();
uint32_t kproc_exec(char *path, char *argv[]);
uint32_t kproc_add_page();


#endif