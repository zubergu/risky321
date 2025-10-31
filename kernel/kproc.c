#include "kproc.h"
#include "system.h"
#include "kvmem.h"
#include "kmem.h"
#include "kprintf.h"
#include "klibc.h"
#include "csr.h"
#include "ktrap.h"
#include "elf.h"
#include "filesystem.h"

extern char __free_ram_end[];

/* statically allocated array to hold process control blocks */
struct process proc_array[NUM_PROCESSES];

struct context scheduler_context;
struct process *current_user_process;



void kproc_init()
{
    for(int i = 0; i < NUM_PROCESSES; i++)
    {
        proc_array[i].state = UNUSED;
        proc_array[i].kernel_stack_top = (uint32_t)(&(proc_array[i].kernel_stack[8192]));
        proc_array[i].free_heap_page = VA_USER_BASE;
    }
}


struct process *kproc_create(uint32_t *elf_file)
{

    int i = 0;
    bool found = FALSE;
    for(i = 0; i < NUM_PROCESSES; i++)
    {
        if(proc_array[i].state == UNUSED)
        {
            found = TRUE;
            break;
        }
    }

    if(!found)
    {
        kpanic(__FILE__, __LINE__, "No free PCB for new process");
    }

    proc_array[i].state = RUNNABLE;
    proc_array[i].pid = i + 1;
    proc_array[i].pagetable = kmem_alloc();
    proc_array[i].trapframe = kmem_alloc();

    /* This will make on swtch from scheduler */
    /* first jump to ktrap_to_user function */
    /* that prepares user process to make actual jump to user mode */
    proc_array[i].context.ra = (uint32_t)ktrap_to_user;

    /* on swtch, make transition to kernel stack */
    /* to use it before jumping to user process */
    /* context of process has kernel sp while trapframe has user sp */
    proc_array[i].context.sp = (uint32_t)proc_array[i].kernel_stack_top;

    /* these will be used in uservec.S if trap happens while this process is being executed */
    proc_array[i].trapframe->kernel_sp = proc_array[i].kernel_stack_top;
    proc_array[i].trapframe->kernel_trap = (uint32_t) ktrap_from_user_handler;


    /* map kernel virtual addresses into user address space 1:1 */
    for(char *pa = 0; pa < __free_ram_end; pa += PAGE_SIZE)
    {
        kvmem_map_page(proc_array[i].pagetable, (uint32_t)pa, (uint32_t)pa, PTE_R | PTE_W | PTE_X);
    }


    /* Load ELF file into process memory */

    struct elf_header *eh = (struct elf_header *) elf_file;
    elf_dump_header(eh);

    /* when starting process, it will be treated like return from trap handler */
    /* so epc will be set insise sepc on jumping to user mode */
    proc_array[i].trapframe->epc         = eh->entry;


    for(int i = 0; i < eh->phnum; i++)
    {
        struct elf_program_header *eph = (struct elf_program_header *)((uint32_t)eh + eh->phoff + i*sizeof(struct elf_program_header));
        elf_dump_program_header(eph);

        uint32_t program_start = (uint32_t)elf_file + (uint32_t)eph->offset;

        int full_pages = eph->memsz/PAGE_SIZE;
        int last_page_size = eph->memsz % PAGE_SIZE;

        int page_idx = 0;
        for(page_idx = 0; page_idx < full_pages; page_idx++)
        {
            void *page = kmem_alloc();
            memcpy(page, (void *)(program_start + (page_idx * PAGE_SIZE)), PAGE_SIZE);
            kvmem_map_page(proc_array[i].pagetable, eph->vaddr + (page_idx * PAGE_SIZE), (uint32_t)page, PTE_R | PTE_W | PTE_X | PTE_U);
        }

        /* handle the remainder of the last page, that might be not full */
        /* page_idx is full_pages at the end of loop above, index of last page if ther should be one */
        if(last_page_size > 0)
        {
            void *page = kmem_alloc();
            memcpy(page, (void *)(program_start + page_idx * PAGE_SIZE), last_page_size);
            kvmem_map_page(proc_array[i].pagetable, eph->vaddr + (page_idx * PAGE_SIZE), (uint32_t)page, PTE_R | PTE_W | PTE_X | PTE_U);
        }

    }

    /* every process has 1 page for trapframe mapped to the same virtual address address */
    kvmem_map_page(proc_array[i].pagetable, VA_TRAPFRAME, (uint32_t)proc_array[i].trapframe, PTE_R | PTE_W);

    /* allocate user stack */
    void * user_stack_page = kmem_alloc();
    proc_array[i].user_stack = user_stack_page;
    proc_array[i].trapframe->sp = VA_USER_STACK_INIT;
    kvmem_map_page(proc_array[i].pagetable, VA_USER_STACK, (uint32_t)user_stack_page, PTE_R | PTE_W | PTE_X | PTE_U );

    return &proc_array[i];
}


void kproc_free()
{
    current_user_process->state = UNUSED;

    kmem_free(current_user_process->trapframe);
    kmem_free(current_user_process->pagetable);
    kmem_free(current_user_process->user_stack);

    kproc_yield();
    kpanic(__FILE__, __LINE__, "should never return after process was freed");
}


uint32_t kproc_kill(uint32_t pid)
{
    if(current_user_process->pid == pid)
    {
        kproc_free();
    }

    for(uint32_t i = 0; i < NUM_PROCESSES; i++)
    {
        if( (proc_array[i].state != UNUSED) && (proc_array[i].pid == pid) )
        {
            kmem_free(proc_array[i].trapframe);
            kmem_free(proc_array[i].pagetable);
            kmem_free(proc_array[i].user_stack);
            proc_array[i].state = UNUSED;
        }
    }

    return 0xFFFFFFFF;
}


void kproc_yield()
{
    swtch(&current_user_process->context, &scheduler_context);
}


uint32_t kproc_file_add(struct os_file * opened_file)
{
    for(uint32_t fd = 3; fd < MAX_OPEN_FILES; fd++)
    {
        if(current_user_process->opened_files[fd] == NULL)
        {
            current_user_process->opened_files[fd] = opened_file;
            return fd;
        }
    }

    return -1;
}

uint32_t kproc_file_remove(uint32_t fd)
{
    if(current_user_process->opened_files[fd] == NULL)
    {
        return -1;
    }

    current_user_process->opened_files[fd] = NULL;
    return 0;
}

struct os_file *kproc_file_get(uint32_t fd)
{
    return current_user_process->opened_files[fd];
}

/* return PID of new process */
uint32_t kproc_fork()
{
    int i = 0;
    bool found = FALSE;
    for(i = 0; i < NUM_PROCESSES; i++)
    {
        if(proc_array[i].state == UNUSED)
        {
            found = TRUE;
            break;
        }
    }

    if(!found)
    {
        kpanic(__FILE__, __LINE__, "No free PCB for forked process");
    }

    proc_array[i].state = RUNNABLE;
    proc_array[i].pid = i + 1;
    proc_array[i].pagetable = kvmem_copy_pagetable(current_user_process->pagetable);
    proc_array[i].trapframe = kmem_alloc();

    kvmem_map_page(proc_array[i].pagetable, VA_TRAPFRAME, (uint32_t)proc_array[i].trapframe, PTE_R | PTE_W);


    memcpy(proc_array[i].trapframe, current_user_process->trapframe, PAGE_SIZE);

    proc_array[i].user_stack = kvmem_get_pa(proc_array[i].pagetable, VA_USER_STACK);

    /* This will make on swtch from scheduler */
    /* first jump to ktrap_to_user function */
    /* that prepares user process to make actual jump to user mode */
    proc_array[i].context.ra = (uint32_t)ktrap_to_user;

    /* on swtch, make transition to kernel stack */
    /* to use it before jumping to user process */
    /* context of process has kernel sp while trapframe has user sp */
    proc_array[i].context.sp = (uint32_t)proc_array[i].kernel_stack_top;

    /* these will be used in uservec.S if trap happens while this process is being executed */
    proc_array[i].trapframe->kernel_sp = proc_array[i].kernel_stack_top;
    proc_array[i].trapframe->kernel_trap = (uint32_t) ktrap_from_user_handler;
    
    /* this will return forked process PID to process that initialized fork()*/
    current_user_process->trapframe->a0 = proc_array[i].pid;
    /* and 0 in created process */
    proc_array[i].trapframe->a0 = 0;


    kprintf("Successfully forked %d to %d\n", current_user_process->pid, proc_array[i].pid);
    return proc_array[i].pid;
}


uint32_t kproc_exec(char *path, char **argv)
{
    kprintf("kproc_exec got filepath %s\n", path);
    int argc = 0;
    char **argv_backup = argv;
    while(argv && *argv)
    {
        kprintf("Kernel got argv in exec = %s\n", *argv);
        argv++;
        argc++;
    }

    argv = argv_backup;

    uint8_t *elf_file_ptr = filesystem_get_filedata(path);
    if(elf_file_ptr == NULL)
    {
        return -1;
    }

    current_user_process->state = RUNNABLE;
    current_user_process->pagetable = kmem_alloc();
    current_user_process->trapframe = kmem_alloc();

    /* This will make on swtch from scheduler */
    /* first jump to ktrap_to_user function */
    /* that prepares user process to make actual jump to user mode */
    current_user_process->context.ra = (uint32_t)ktrap_to_user;

    /* on swtch, make transition to kernel stack */
    /* to use it before jumping to user process */
    /* context of process has kernel sp while trapframe has user sp */
    current_user_process->context.sp = (uint32_t)current_user_process->kernel_stack_top;

    /* these will be used in uservec.S if trap happens while this process is being executed */
    current_user_process->trapframe->kernel_sp = current_user_process->kernel_stack_top;
    current_user_process->trapframe->kernel_trap = (uint32_t) ktrap_from_user_handler;


    /* map kernel virtual addresses into user address space 1:1 */
    for(char *pa = 0; pa < __free_ram_end; pa += PAGE_SIZE)
    {
        kvmem_map_page(current_user_process->pagetable, (uint32_t)pa, (uint32_t)pa, PTE_R | PTE_W | PTE_X);
    }

    /* Load ELF file into process memory */

    struct elf_header *eh = (struct elf_header *) elf_file_ptr;
    elf_dump_header(eh);

    /* when starting process, it will be treated like return from trap handler */
    /* so epc will be set insise sepc on jumping to user mode */
    current_user_process->trapframe->epc         = eh->entry;

    for(int i = 0; i < eh->phnum; i++)
    {
        struct elf_program_header *eph = (struct elf_program_header *)((uint32_t)eh + eh->phoff + i*sizeof(struct elf_program_header));
        elf_dump_program_header(eph);

        uint32_t program_start = (uint32_t)elf_file_ptr + (uint32_t)eph->offset;

        int full_pages = eph->memsz/PAGE_SIZE;
        int last_page_size = eph->memsz % PAGE_SIZE;

        int page_idx = 0;
        for(page_idx = 0; page_idx < full_pages; page_idx++)
        {
            void *page = kmem_alloc();
            memcpy(page, (void *)(program_start + (page_idx * PAGE_SIZE)), PAGE_SIZE);
            kvmem_map_page(current_user_process->pagetable, eph->vaddr + (page_idx * PAGE_SIZE), (uint32_t)page, PTE_R | PTE_W | PTE_X | PTE_U);
            current_user_process->free_heap_page += PAGE_SIZE;
        }

        /* handle the remainder of the last page, that might be not full */
        /* page_idx is full_pages at the end of loop above, index of last page if ther should be one */
        if(last_page_size > 0)
        {
            void *page = kmem_alloc();
            memcpy(page, (void *)(program_start + page_idx * PAGE_SIZE), last_page_size);
            kvmem_map_page(current_user_process->pagetable, eph->vaddr + (page_idx * PAGE_SIZE), (uint32_t)page, PTE_R | PTE_W | PTE_X | PTE_U);
            current_user_process->free_heap_page += PAGE_SIZE;
        }

    }

    /* every process has 1 page for trapframe mapped to the same virtual address address */
    kvmem_map_page(current_user_process->pagetable, VA_TRAPFRAME, (uint32_t)current_user_process->trapframe, PTE_R | PTE_W);

    /* allocate user stack */
    void * user_stack_page = kmem_alloc();
    current_user_process->user_stack = user_stack_page;
    current_user_process->trapframe->sp = VA_USER_STACK_INIT;
    kvmem_map_page(current_user_process->pagetable, VA_USER_STACK, (uint32_t)user_stack_page, PTE_R | PTE_W | PTE_X | PTE_U );

    /* write all arguments onto new process user stack and create array of char* on the stack that can be passed as argv to main()*/

    char * tmp_argv[argc];
    char * physical_user_stack = (char*)((uint32_t) user_stack_page + PAGE_SIZE);

    for(int i = 0; i < argc; i++)
    {
        uint32_t arg_len = 0;
        char *arg = argv[i];
        
        while(*arg)
        {
            arg_len++;
            arg++;
        }

        arg_len++; // for '\0'
        physical_user_stack -= arg_len;
        current_user_process->trapframe->sp -= arg_len;
        tmp_argv[i] = current_user_process->trapframe->sp;
        memcpy((void*)physical_user_stack, (void*)argv[i], arg_len);
    }

    physical_user_stack -= argc *sizeof(char *);
    memcpy((void*)physical_user_stack, (void*)tmp_argv, sizeof(tmp_argv));
    current_user_process->trapframe->sp -= (argc * sizeof(char*));

    /* set up arguments to main(a0 = int argc, a1 = char **argv) in process being executed */
    current_user_process->trapframe->a0 = argc;
    current_user_process->trapframe->a1 = current_user_process->trapframe->sp;

    kprintf("EXEC: process next free heap page VA = %x\n", current_user_process->free_heap_page);


    return 0;
}

void kproc_scheduler()
{
    /* infinitely loop and look for next process to run */

    for(;;)
    {
        for(int i = 0; i < NUM_PROCESSES; i++)
        {
            if(proc_array[i].state == RUNNABLE || proc_array[i].state == RUNNING)
            {
                current_user_process = &(proc_array[i]);
                swtch(&scheduler_context, &(proc_array[i].context));
            }
        }

    }
}

uint32_t kproc_add_page()
{
    void * page = kmem_alloc();
    uint32_t retval = current_user_process->free_heap_page;
    kvmem_map_page(current_user_process->pagetable, current_user_process->free_heap_page, page, PTE_R | PTE_W | PTE_U);
    current_user_process->free_heap_page += PAGE_SIZE;
    
    return retval;
}
