.equ VA_MAX, 0xFFFFFFFF
.equ VA_TRAPFRAME, (VA_MAX - 4096 + 1)

.section .text

# entry point for traps that occured while executing code
# in user mode




.align 4
.globl uservec

uservec:
    # SP   is still from user process
    # SATP is still from user process
    # MODE is Supervisor, trap was automagically transferred
    # from user mode to machine mode,
    # but we delegated all traps to supervisor mode, yeah.


    # all general purpose registers need to be saved
    # process didn't do any saving,
    # jump here is not a function call but
    # immediate hardware interrupt of process
    # This means we can't change any user process register
    # or it might destroy processe's execution
    # when kernel finishes trap handling and jumps to user process again

    # let's backup a0 in sscratch
    # so we can use a0 without destroying user's value and this code and restore it when saving
    # a0 for process in trap_frame

    csrw sscratch, a0


    # load address of current process's page allocated
    # for trap frame
    # every process gets one while being created
    # that trap frame gets mapped
    # to the same Virtual Address in every process
    # they all have different physical addresses, just the same VA
    # We use individual process's paging table here
    # so it still reads correct physical address

    li a0, VA_TRAPFRAME

    # save all the user registers in processes trap_frame
    # while using a0 as pointer, a0 will be saved too,
    # it was previously saved temporarily in sscratch

    sw ra,  ( 5 * 4)(a0)
    sw sp,  ( 6 * 4)(a0)
    sw gp,  ( 7 * 4)(a0)
    sw tp,  ( 8 * 4)(a0)
    sw t0,  ( 9 * 4)(a0)
    sw t1,  (10 * 4)(a0)
    sw t2,  (11 * 4)(a0)
    sw s0,  (12 * 4)(a0)
    sw s1,  (13 * 4)(a0)
    # space for a0 at index 14
    sw a1,  (15 * 4)(a0)
    sw a2,  (16 * 4)(a0)
    sw a3,  (17 * 4)(a0)
    sw a4,  (18 * 4)(a0)
    sw a5,  (19 * 4)(a0)
    sw a6,  (20 * 4)(a0)
    sw a7,  (21 * 4)(a0)
    sw s2,  (22 * 4)(a0)
    sw s3,  (23 * 4)(a0)
    sw s4,  (24 * 4)(a0)
    sw s5,  (25 * 4)(a0)
    sw s6,  (26 * 4)(a0)
    sw s7,  (27 * 4)(a0)
    sw s8,  (28 * 4)(a0)
    sw s9,  (29 * 4)(a0)
    sw s10, (30 * 4)(a0)
    sw s11, (31 * 4)(a0)
    sw t3,  (32 * 4)(a0)
    sw t4,  (33 * 4)(a0)
    sw t5,  (34 * 4)(a0)
    sw t6,  (35 * 4)(a0)


    # now save user's a0 that is in sscratch
    # we saved all other user registers so we can use them
    # they will be restored just fine from trap_frame
    # to we use a0 to still hold trapframe address
    # and we will use t0 to get user's a0 from sscratch safely

    csrr t0, sscratch
    sw   t0, (14 * 4)(a0)


    # initialize kernel stack pointer from current process's trapframe
    # it was put there during process creation
    
    # !! from now on, everything will happen on interrupted process's 
    # !! kernel stack!!
    lw sp, (1 * 4)(a0)
    

    # ignore hartid for now, we only use one hart anyway
    
    # no need to change page table from user to kernel
    # we mapped all our kernel address space into every process
    # so 

    # get address of function that is actual trap handler
    # ktrap_from_user_handler() in ktrap.c
    # for traps that come from user mode
    lw t0, (2 * 4)(a0)

    # and jump to that kernel, address is in t0
    # kernel's stack is in use so that handler will work
    # on kernel stack !
    jr t0




# enter_user_process(struct pagetable *)
# Will be called in 2 cases:
#  1) first jump into freashly created user process, from scheduler
#  2) return from actual trap handler
# It will be always treated like return from trap handler,
# just trapframe will be set up while initializing brand new user process in the 1)

.align 4
.globl enter_user_process

enter_user_process:
    # process's pagetable address is in a0(first argument to function)
    # put it in satp
    # From now on we still execute kernel code but with process's page table

    sfence.vma
    csrw satp, a0
    sfence.vma

    # reuse a0, load process's trapframe
    li a0, VA_TRAPFRAME

    # and now restore all user process registers from trapframe
    # except a0, we still use that as a pointer to actual trapframe
    lw ra,  ( 5 * 4)(a0)
    lw sp,  ( 6 * 4)(a0)
    lw gp,  ( 7 * 4)(a0)
    lw tp,  ( 8 * 4)(a0)
    lw t0,  ( 9 * 4)(a0)
    lw t1,  (10 * 4)(a0)
    lw t2,  (11 * 4)(a0)
    lw s0,  (12 * 4)(a0)
    lw s1,  (13 * 4)(a0)
    # space for a0 at index 14
    lw a1,  (15 * 4)(a0)
    lw a2,  (16 * 4)(a0)
    lw a3,  (17 * 4)(a0)
    lw a4,  (18 * 4)(a0)
    lw a5,  (19 * 4)(a0)
    lw a6,  (20 * 4)(a0)
    lw a7,  (21 * 4)(a0)
    lw s2,  (22 * 4)(a0)
    lw s3,  (23 * 4)(a0)
    lw s4,  (24 * 4)(a0)
    lw s5,  (25 * 4)(a0)
    lw s6,  (26 * 4)(a0)
    lw s7,  (27 * 4)(a0)
    lw s8,  (28 * 4)(a0)
    lw s9,  (29 * 4)(a0)
    lw s10, (30 * 4)(a0)
    lw s11, (31 * 4)(a0)
    lw t3,  (32 * 4)(a0)
    lw t4,  (33 * 4)(a0)
    lw t5,  (34 * 4)(a0)
    lw t6,  (35 * 4)(a0)

	# restore user a0
    # we don't need trapframe address in a0 anymore after that
    lw a0, (14 *4)(a0)


    # jump to user process at address currently in sepc
    # This also switches from Supervisor to User Mode, because of value set in SSTATUS:SPP

    sret
