#ifndef _CSR_H
#define _CSR_H

#include "types.h"

/*      ###  SCAUSE  ###     */

/* EXCEPTION SCAUSE */
#define SCAUSE_IA_MISALIGNED     0
#define SCAUSE_IACCESS_FAULT     1
#define SCAUSE_ILLEGAL_INSTR     2
#define SCAUSE_BREAKPOINT        3
#define SCAUSE_LA_MISALIGNED     4
#define SCAUSE_LACCESS_FAULT     5
#define SCAUSE_UMODE_ECALL       8
#define SCAUSE_SMODE_ECALL       9
#define SCAUSE_INSTR_PAGE_FAULT 12
#define SCAUSE_LOAD_PAGE_FAULT  13

/* INTERRUPT SCAUSE */
#define SCAUSE_SMODE_SOFTWARE_INTR 1
#define SCAUSE_SMODE_TIMER_INTR    5
#define SCAUSE_SMODE_EXTERNAL_INTR 9

/* SCAUSE BITMAP */
#define SCAUSE_INTR_BIT         (1u << 31)
#define SCAUSE_CODE_MASK        0x7FFFFFFF



/* ###  MSTATUS  ### */

/* MASK FOR BIT FIELDS*/
#define MSTATUS_MPP_MASK (0x3 << 11) /* MSTATUS:MPP  */

/* MASKS TO SET BIT FIELDS IN MSTATUS REGISTER */
#define MSTATUS_MPP_S    (0x1 << 11) /* MSTATUS:MPP = S(UPERVISOR) */
#define MSTATUS_SIE      (0x1 << 1)  /* MSTATUS:SIE = 1 */


/* ###   SSTATUS   ### */
#define SSTATUS_SPP_MASK (0x1 << 8) /* SSTATUS:SPP */
#define SSTATUS_SPP_S    (0x1 << 8) /* SSTATUS:SPP = S(upervisor), Trap came from supervisor mode, and not user (0)*/
#define SSTATUS_SPIE     (0x1 << 5) /* SSTATUS:SPIE = 1, Supervisor Previous Interrupts Enabled, status of SIE in supervisor mode !!before!! trap, will be recovered when sret to user mode */
#define SSTATUS_SUM      (1 << 18)  /* SSTATUS:SUM  = 1, Supervisor User Memory Access Enabled */

/* ###  SIE (SUPERVISOR INTERRUPT ENABLE)  ### */

/* MASKS TO SET BIT FIELDS IN SIE REGISTER */
#define SIE_SEIE       (0x1) << 9 /* SEIE: Supervisor External Interrupt Enable */
#define SIE_STIE       (0x1) << 5 /* STIE: Supervisor Timer    Interrupt Enable */
#define SIE_SSIE       (0x1) << 1 /* SSIE: Supervisor Software Interrupt Enable */

/* MASKS TO SET BIT FIELDS IN MIE REGISTER */
#define MIE_STIE       (0x1) << 5 /* STIE: Supervisor Timer  Interrupt Enable */


/* ###  SATP  ### */

/* Supervisor Address Translation and Protection */
#define SATP_SV32 (1u << 31) /* Bit in SATP that indicates format used in created Paging Table */

#define READ_CSR(reg)\
({\
    uint32_t __tmp;\
    __asm__ __volatile__("csrr %0," reg : "=r"(__tmp));\
    __tmp;\
})

#define WRITE_CSR(reg, value)\
do {\
    uint32_t __tmp = (value);\
    __asm__ __volatile__("csrw " reg ", %0" :: "r"(__tmp));\
} while(0)


#endif
