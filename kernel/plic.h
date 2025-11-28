#ifndef _PLIC_H
#define _PLIC_H

#define PLIC 0x0C000000L
#define PLIC_PRIORITY              (PLIC + 0x0)
#define PLIC_PENDING               (PLIC + 0x1000)

// 0x100 - there are so many bytes in array for enable bits per whole HART (2 Contexts)
// BASE + 0x2080 is beginning address of Context1 (HART0 S-mode)
// we add (hart * 0x100) to set pointer at context1, 3, 5, 7 etc.
#define PLIC_ENABLE_S(hart)        (PLIC + 0x2080 + (hart)*0x100)
#define PLIC_PRIORITY_THR_S(hart)  (PLIC + 0x201000 + (hart)*0x2000)
#define PLIC_CLAIM_S(hart)         (PLIC + 0x201004 + (hart)*0x2000)

void
PlicInit(void);

void
PlicInitHart(void);

int
PlicClaim(void);

void
PlicComplete(int irq);

#endif