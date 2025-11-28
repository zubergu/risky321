#include "plic.h"
#include "types.h"
#include "uart.h"
//#include "virtio.h"

void
PlicInit(void)
{
    // for every irq line coming from external device to PLIC we need to set up some (any non-zero) priority
    // priority for single irq takes 4 bytes, there is an array of prioritis starting at PLIC_PRIORITY address
    *(uint32_t*)(PLIC_PRIORITY + UART0_IRQ*4) = 1;
}

void
PlicInitHart(void)
{
    // we have just one HART = 0 in this version

    // for every HART there are two contexts, first for M-mode, second for S-mode
    // HART0 has contexts 0, 1. Context0 is for M, Context1 is for privilege S.
    // for every HART Context M will be even (0, 2, 4, ...) and Context S will be odd (1, 3, 5, ...)
    // We want PLIC to generate interrupts with privilege S, so we set UP odd context
    *((uint32_t*)(PLIC_ENABLE_S(0))) = (1 << UART0_IRQ);

    // set S-mode priority threshold for HART0, context 1 to 0
    // Priority threshold is set per context
    // PLIC will mask all interrupts of priority <= threshold for that Context
    // we set uart priority to 1, now we set up threshold to 0 so it will not be masked
    *((uint32_t*)(PLIC_PRIORITY_THR_S(0))) = 0;
}

int
PlicClaim(void)
{
    int irq = *((uint32_t*)(PLIC_CLAIM_S(0)));

    return irq;
}

void
PlicComplete(int irq)
{
    *((uint32_t*)(PLIC_CLAIM_S(0))) = irq;
}