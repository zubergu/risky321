#include "ktrap.h"
#include "csr.h"
#include "system.h"
#include "kprintf.h"
#include "ksyscall.h"
#include "kproc.h"

/* from kproc.c */
/* variables that hold process information */
extern struct context scheduler_context;
extern struct process *current_user_process;

/* for compiler to be quiet, kernelvec will be linked from assembly in kernelvec.S */
extern void kernelvec();

/* functions from uservec.S */
extern void uservec(void);
extern void enter_user_process(uint32_t pagetable);

void ktrap_init()
{
    /* kernelvec is a subroutine written in assembly in kernelvec.S*/
    WRITE_CSR("stvec", (uint32_t) kernelvec);
}


void ktrap_from_user_handler(void)
{
    /* we came here from uservec, user registers were saved there */
    /* we are in supervisor mode but...*/
    /* ... we use interrupted process's stack pointer AND page table */
    uint32_t scause = READ_CSR("scause");
    //uint32_t stval  = READ_CSR("stval");
    uint32_t sepc   = READ_CSR("sepc");
    uint32_t sstatus = READ_CSR("sstatus");

    uint32_t retval = 0;

    /* switch trap handler from user to kernelvec */
    /* since now we are executing in kernel mode  */

    WRITE_CSR("stvec", (uint32_t)kernelvec);

    current_user_process->trapframe->epc = sepc;

    bool from_kernel  = (sstatus & SSTATUS_SPP_MASK) == SSTATUS_SPP_S;
    bool is_exception = (scause & SCAUSE_INTR_BIT) == 0;
    uint32_t trap_code = (scause & SCAUSE_CODE_MASK);

    if(from_kernel)
    {
        kpanic(__FILE__, __LINE__, "uservec started handling traps from supervisor mode");
    }

    /* till this point interrupts were automatically turned OFF */
    /* by hardware */
    /* to make trap handling interruptible we should turn them on here */

    if(is_exception)
    {
        /* sepc points to exception instruction so we need to manually */
        /* move to the next instruction in user process that caused exception */
        /* we do it here, because if system call handler is for example exec - we would move entry point to new program */
        current_user_process->trapframe->epc += 4;

        switch(trap_code)
        {
            case SCAUSE_UMODE_ECALL:
                switch(current_user_process->trapframe->a7)
                {
                    case SYS_fork:
                        ksys_fork();
                        break;
                    case SYS_read:
                        retval = ksys_read(current_user_process->trapframe->a0, (uint8_t *)current_user_process->trapframe->a1, current_user_process->trapframe->a2);
                        current_user_process->trapframe->a0 = retval;
                        break;
                    case SYS_write:
                        ksys_write(current_user_process->trapframe->a0, (uint8_t *)current_user_process->trapframe->a1, current_user_process->trapframe->a2);
                        break;
                    case SYS_exit:
                        ksys_exit();
                        break;
                    case SYS_getpid:
                        retval = current_user_process->pid;
                        current_user_process->trapframe->a0 = retval;
                        break;
                    case SYS_kill:
                        retval = ksys_kill(current_user_process->trapframe->a0);
                        current_user_process->trapframe->a0 = retval;
                        break;
                    case SYS_open:
                        retval = ksys_open((char *)current_user_process->trapframe->a0, (uint32_t)current_user_process->trapframe->a1);
                        current_user_process->trapframe->a0 = retval;
                        break;
                    case SYS_close:
                        retval = ksys_close(current_user_process->trapframe->a0);
                        current_user_process->trapframe->a0 = retval;
                        break;
                    case SYS_exec:
                        retval = ksys_exec( (char *)current_user_process->trapframe->a0, (char **)current_user_process->trapframe->a1 );
                        break;
                    case SYS_sleep:
                        retval = ksys_sleep();
                        current_user_process->trapframe->a0 = retval;
                        break;
                    case SYS_sbrk:
                        retval = ksys_sbrk();
                        current_user_process->trapframe->a0 = retval;
                        break;
                    default:
                        kpanic(__FILE__, __LINE__, "unhandled umode ecall");
                        break;
                }
                break;
            
            default:
                kpanic(__FILE__, __LINE__, "unhandled exception code");
                break;
        }

    }
    else
    {
        kpanic(__FILE__, __LINE__, "unhandled interrupt in user trap handler");
    }

    ktrap_to_user();
}

/* Called at the end of user trap handler OR at initial start of process */
/* prepare trapframe and all system registers before */
/* */
void ktrap_to_user(void)
{
    uint32_t sstatus = READ_CSR("sstatus");

    /* set trap handler to one designed to handle traps from user mode */
    WRITE_CSR("stvec", (uint32_t)uservec);


    /* set/reset trapframe of current process for next eventual interrupt */
    /* when this process executes back in user mode */
    /* reset stack to initial position */
    current_user_process->trapframe->kernel_sp = current_user_process->kernel_stack_top;
    current_user_process->trapframe->kernel_trap = (uint32_t) ktrap_from_user_handler;

    /* prepare SSTATUS for jump to User Mode */
    /* clear previous privilage bits*/
    sstatus = sstatus & (~SSTATUS_SPP_MASK);
    /* set previous privilage bits to Supervisor and enable interrupts by setting SPIE */
    sstatus = sstatus | SSTATUS_SPIE;
    WRITE_CSR("sstatus", sstatus);

    /* set SEPC to jump to correct place in user process */
    WRITE_CSR("sepc", current_user_process->trapframe->epc);

    /* argument to this function is correct content of satp register */
    /* for user process that is about to be run*/
    enter_user_process(SATP_SV32 | ((uint32_t)current_user_process->pagetable) >> 12);

}

/* handle traps that originate from execution in supervisor mode */
/* we jump here from KERNELVEC in kernelvec.S */
void ktrap_from_supervisor_handler()
{
    
    uint32_t scause = READ_CSR("scause");
    uint32_t stval  = READ_CSR("stval");
    uint32_t sepc   = READ_CSR("sepc");
    uint32_t sstatus = READ_CSR("sstatus");

    bool from_kernel  = (sstatus & SSTATUS_SPP_MASK) == SSTATUS_SPP_S;
    bool is_exception = (scause & SCAUSE_INTR_BIT) == 0;
    
    kprintf("sstatus=%x, scause=%x, stval=%x, sepc=%x\n", sstatus, scause, stval, sepc);


    if(!from_kernel)
    {
        kprintf("sstatus=%x, scause=%x, stval=%x, sepc=%x\n", sstatus, scause, stval, sepc);
        kpanic(__FILE__, __LINE__, "Trap not from kernel mode was handled by kernel trap handler");
    }

    if(is_exception)
    {
        kpanic(__FILE__, __LINE__, "Exception from kernel mode happened");
    }

    kpanic(__FILE__, __LINE__, "Interrupts in kernel are not handled yet");
    
    /* at the end of this function we would simply return to where we were called */
    /* that is kernelvec in kernelvec.S */
    /* where registers from before trap will be restored from the stack */
    /* and jump will be made to address in sepc, also sstatus will be restored */
    /* so unless we change that, we will be switching from kernel to kernel execution */
}

void kdump_trapframe(struct trap_frame *frame)
{
    kprintf("TRAP FRAME:\n");
    kprintf("ra=%x\n", frame->ra);
    kprintf("sp=%x\n", frame->sp);
    kprintf("gp=%x\n", frame->gp);
    kprintf("tp=%x\n", frame->tp);
    kprintf("t0=%x\n", frame->t0);
    kprintf("t1=%x\n", frame->t1);
    kprintf("t2=%x\n", frame->t2);
    kprintf("t3=%x\n", frame->t3);
    kprintf("t4=%x\n", frame->t4);
    kprintf("t5=%x\n", frame->t5);
    kprintf("t6=%x\n", frame->t6);
    kprintf("a0=%x\n", frame->a0);
    kprintf("a1=%x\n", frame->a1);
    kprintf("a2=%x\n", frame->a2);
    kprintf("a3=%x\n", frame->a3);
    kprintf("a4=%x\n", frame->a4);
    kprintf("a5=%x\n", frame->a5);
    kprintf("a6=%x\n", frame->a6);
    kprintf("a7=%x\n", frame->a7);
    kprintf("s0=%x\n", frame->s0);
    kprintf("s1=%x\n", frame->s1);
    kprintf("s2=%x\n", frame->s2);
    kprintf("s3=%x\n", frame->s3);
    kprintf("s4=%x\n", frame->s4);
    kprintf("s5=%x\n", frame->s5);
    kprintf("s6=%x\n", frame->s6);
    kprintf("s7=%x\n", frame->s7);
    kprintf("s8=%x\n", frame->s8);
    kprintf("s9=%x\n", frame->s9);
    kprintf("s10=%x\n", frame->s10);
    kprintf("s11=%x\n", frame->s11);
    kprintf("kernel_sp=%x\n", frame->kernel_sp);
}
