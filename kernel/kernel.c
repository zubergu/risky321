#include "uart.h"
#include "kprintf.h"
#include "klibc.h"
#include "system.h"
#include "csr.h"
#include "kmem.h"
#include "kvmem.h"
#include "ktrap.h"
#include "kproc.h"
#include "virtio.h"
#include "filesystem.h"
#include "plic.h"

struct process * shell_proc;
//struct process * shell_proc2;

extern struct filesystem_file filesystem_files[MAX_FS_FILES];


/* entry point for kernel in supervisor mode */
void
kernel_main()
{
    /* Give supervisor mode access to memory at addresses paged for User Mode */
    uint32_t sstatus = READ_CSR("sstatus");
    WRITE_CSR("sstatus", sstatus | SSTATUS_SUM);

    /* initialize UART for I/O */
	uart_init();

    /* initialize kernel dynamic memory allocator */
    kmem_init();

    /* initialize kernel virtual addressing */
    kvmem_init_kernel();

    /* turn on kernel memory paging */
    kvmem_init_hart();

    /* initialize kernel trap handling */
    ktrap_init();

    /* initialize PLIC for external interrupts from devices */
    PlicInit();
    PlicInitHart();

    /* initialize process control blocks */
    kproc_init();

    /* initialize VIRTIO-BLK */
    virtio_blk_init();

    /* initialize filesystem */
    filesystem_init();

    //for(;;)
    //{
        //kprintf("Stuck at s-mode\n");
    //    delay();
    //}

    shell_proc =  kproc_create((uint32_t *)(&filesystem_files[2].data[0]));
    //shell_proc2 = kproc_create((uint32_t *)(&files[2].data[0]), files[2].size);
    /* call scheduler, that infinitely loops through processes to switch to on the CPU */
    kproc_scheduler();

    
    /* test the trap handler with unimp instruction that causes illegal instruction exception */
    __asm__ __volatile__("unimp");  /* should cause exception in CPU */

    /* should never happen, kernel_main shouldn't return ever */
    kpanic(__FILE__, __LINE__, "Kernel main wanted to return?");
}
