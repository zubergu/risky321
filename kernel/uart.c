#include "types.h"
#include "uart.h"

void uart_init()
{
    UART_IER = UART_IER_RX_ENABLE & UART_IER_TX_ENABLE;
}

/* put character into UART transmission register */
uint8_t uart_putc_block(uint8_t ch)
{
    // waint until transmission line is idle
    while((UART_LSR & UART_LSR_TX_IDLE) == 0)
    {
        continue;
    }

    UART_THR = ch;

    return 0;
}

uint8_t uart_putc_nonblock(uint8_t ch)
{
    if((UART_LSR & UART_LSR_TX_IDLE) == 0)
    {
        return 0xFF;
    }

    UART_THR = ch;
    return 0;
}

uint8_t uart_getc_block()
{
    // wait until receive register is ready to read
    /* had to turn off gcc optimization because this loop was optimized out */
    while((UART_LSR & UART_LSR_RX_READY) == 0)
    {
        continue;
    }

    return UART_RHR;
}

uint8_t uart_getc_nonblock()
{
    if((UART_LSR & UART_LSR_RX_READY) == 0)
    {
        return 0xFF;
    }

    return UART_RHR;
}
