#include "types.h"
#include "uart.h"
#include "tty.h"

void uart_init()
{
    UART_IER = 0x00;

    /* set Line Control Register for setting transmision configuration */
    UART_LCR = UART_LCR_BAUD_LATCH;
    
    /* set LSB & MSB for baud rate 38.4k */
    UART_THR = 0x03;
    UART_IER = 0x00;

    /* set LCR for 8-bit word length */
    UART_LCR = UART_LCR_EIGHT_BITS;

    /* enable and clear FIFO control register */
    UART_FCR = UART_FCR_CLEAR | UART_FCR_ENABLE;

    /* enable receive and send interrups */
    UART_IER = UART_IER_RX_ENABLE | UART_IER_TX_ENABLE;
}

/* put character into UART transmission register */
uint8_t
UartSendBlocking(uint8_t ch)
{
    // waint until transmission line is idle
    while((UART_LSR & UART_LSR_TX_IDLE) == 0)
    {
        continue;
    }

    UART_THR = ch;

    return ch;
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

void
UartInterruptHandle()
{
    __attribute__((unused)) uint32_t intr = UART_ISR;

    /* UART TRANSMISSION LINE IS EMPTY */
    /* might want to handle it in the future
    if(lsr & UART_LSR_TX_IDLE)
    {
        
    }
    */

    /* UART RECEPTION LINE HAS DATA READY TO READ */
    /* while there is still data ready to be received by kernel ...*/
    while(UART_LSR & UART_LSR_RX_READY)
    {
        uint8_t c = UART_RHR;
        TtyBufferInsert(c);
        /* now put this characer into tty buffer */
    }

}