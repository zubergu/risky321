# entry point for handling TRAPs on S(upervisor)/ Kernel level
# written in plain Assembly just because it looks cleaner than asm in C function

# kernel stack is being used here to store the trap data
# because trap frame might be already used and store initial
# interrupt data


.section .text
.globl kernelvec
.align 4  # all instructions have 32 bit (4 bytes) and should start at addresses being multiple of 4

kernelvec:
    # there is kernel stack already set up to use under sp
    # PUSH caller-saved registers onto the stack before calling the actual trap handler

    # increase stack
    addi sp, sp, (-4) * 35  # 35 register will be saved, first 5 values unused but want to have exact format as trapframe has
    sw ra,  ( 5 * 4)(sp)
    # sw sp,  ( 6 * 4)(sp)
    sw gp,  ( 7 * 4)(sp)
    # sw tp,  ( 8 * 4)(sp)
    sw t0,  ( 9 * 4)(sp)
    sw t1,  (10 * 4)(sp)
    sw t2,  (11 * 4)(sp)
    sw s0,  (12 * 4)(sp)
    sw s1,  (13 * 4)(sp)
    sw a0,  (14 * 4)(sp)
    sw a1,  (15 * 4)(sp)
    sw a2,  (16 * 4)(sp)
    sw a3,  (17 * 4)(sp)
    sw a4,  (18 * 4)(sp)
    sw a5,  (19 * 4)(sp)
    sw a6,  (20 * 4)(sp)
    sw a7,  (21 * 4)(sp)
    sw s2,  (22 * 4)(sp)
    sw s3,  (23 * 4)(sp)
    sw s4,  (24 * 4)(sp)
    sw s5,  (25 * 4)(sp)
    sw s6,  (26 * 4)(sp)
    sw s7,  (27 * 4)(sp)
    sw s8,  (28 * 4)(sp)
    sw s9,  (29 * 4)(sp)
    sw s10, (30 * 4)(sp)
    sw s11, (31 * 4)(sp)
    sw t3,  (32 * 4)(sp)
    sw t4,  (33 * 4)(sp)
    sw t5,  (34 * 4)(sp)
    sw t6,  (35 * 4)(sp)

    call ktrap_from_supervisor_handler

    # POP and restore all previously saved registers off the stack
    lw ra,  ( 5 * 4)(sp)
    # lw sp,  ( 6 * 4)(sp)
    lw gp,  ( 7 * 4)(sp)
    # lw tp,  ( 8 * 4)(sp)
    lw t0,  ( 9 * 4)(sp)
    lw t1,  (10 * 4)(sp)
    lw t2,  (11 * 4)(sp)
    lw s0,  (12 * 4)(sp)
    lw s1,  (13 * 4)(sp)
    lw a0,  (14 * 4)(sp)
    lw a1,  (15 * 4)(sp)
    lw a2,  (16 * 4)(sp)
    lw a3,  (17 * 4)(sp)
    lw a4,  (18 * 4)(sp)
    lw a5,  (19 * 4)(sp)
    lw a6,  (20 * 4)(sp)
    lw a7,  (21 * 4)(sp)
    lw s2,  (22 * 4)(sp)
    lw s3,  (23 * 4)(sp)
    lw s4,  (24 * 4)(sp)
    lw s5,  (25 * 4)(sp)
    lw s6,  (26 * 4)(sp)
    lw s7,  (27 * 4)(sp)
    lw s8,  (28 * 4)(sp)
    lw s9,  (29 * 4)(sp)
    lw s10, (30 * 4)(sp)
    lw s11, (31 * 4)(sp)
    lw t3,  (32 * 4)(sp)
    lw t4,  (33 * 4)(sp)
    lw t5,  (34 * 4)(sp)
    lw t6,  (35 * 4)(sp)

    # decrease stack
    addi sp, sp, 4 * 35

    # and continue to wherever sepc(supervisor exception PC), points to
    # Originally exactly address that caused exception, but has to be increased +4 to point for next instruction
    # or there would be a infinite loop, execution would jump to instruction that caused exception in the first place

    sret
