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
    addi sp, sp, (-4) * 31  # total of 31 registers (4 bytes per register) will be saved
    sw ra,  ( 5 * 4)(sp)
    sw sp,  ( 6 * 4)(sp)
    sw gp,  ( 7 * 4)(sp)
    sw tp,  ( 8 * 4)(sp)
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
    lw ra, (4 *  0)(sp)
    lw sp, (4 *  1)(sp)
    lw gp, (4 *  2)(sp)
    # TP will contain HARTID, on multiprocessor machines it would write wrong ID if process changed CPU lw tp, (4 *  3)(sp)
    lw t0, (4 *  4)(sp)
    lw t1, (4 *  5)(sp)
    lw t2, (4 *  6)(sp)
    lw t3, (4 *  7)(sp)
    lw t4, (4 *  8)(sp)
    lw t5, (4 *  9)(sp)
    lw t6, (4 * 10)(sp)
    lw a0, (4 * 11)(sp)
    lw a1, (4 * 12)(sp)
    lw a2, (4 * 13)(sp)
    lw a3, (4 * 14)(sp)
    lw a4, (4 * 15)(sp)
    lw a5, (4 * 16)(sp)
    lw a6, (4 * 17)(sp)
    lw a7, (4 * 18)(sp)
    lw s0, (4 * 19)(sp)
    lw s1, (4 * 20)(sp)
    lw s2, (4 * 21)(sp)
    lw s3, (4 * 22)(sp)
    lw s4, (4 * 23)(sp)
    lw s5, (4 * 24)(sp)
    lw s6, (4 * 25)(sp)
    lw s7, (4 * 26)(sp)
    lw s8, (4 * 27)(sp)
    lw s9, (4 * 28)(sp)
    lw s10,(4 * 29)(sp)
    lw s11,(4 * 30)(sp)

    # decrease stack
    addi sp, sp, 4 * 31

    # and continue to wherever sepc(supervisor exception PC), points to
    # Originally exactly address that caused exception, but has to be increased +4 to point for next instruction
    # or there would be a infinite loop, execution would jump to instruction that caused exception in the first place

    sret
