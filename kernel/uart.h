#ifndef _UART_H
#define _UART_H

#include "types.h"

#define UART0 0x10000000
#define UART_RHR (*(uint8_t *) (UART0 + 0x00)) // Receive Holding Register
#define UART_THR (*(uint8_t *) (UART0 + 0x00)) // Transmit Holding Register
#define UART_IER (*(uint8_t *) (UART0 + 0x01)) // Interrupt Enable Register
#define UART_ISR (*(uint8_t *) (UART0 + 0x02)) // Interrupt Status Register
#define UART_LCR (*(uint8_t *) (UART0 + 0x03)) // Line Control Register
#define UART_LSR (*(uint8_t *) (UART0 + 0x05)) // Line Status Register

#define UART_LSR_RX_READY (1 << 0)             // Ready to read from RHR
#define UART_LSR_TX_IDLE  (1 << 5)             // THR ready to accept data to send

#define UART_IER_RX_ENABLE (1 << 0)
#define UART_IER_TX_ENABLE (1 << 1)

void uart_init();
uint8_t uart_putc_nonblock(uint8_t ch);
uint8_t uart_getc_nonblock();
uint8_t uart_putc_block(uint8_t ch);
uint8_t uart_getc_block();

#endif