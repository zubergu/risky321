#include "kterminal.h"
#include "uart.h"



uint32_t kterminal_readline_block(char *buf, uint32_t size)
{
    uint32_t bytes = 0;

    while(bytes < size)
    {
        uint8_t next_char = uart_getc_block();
        
        if(next_char == '\r')
        {
            UartSendBlocking('\n');
            buf[bytes] = '\0';
            return bytes;
        }
        else if(next_char == '\b')
        {
            if(bytes > 0)
            {
                /* erase last character from screen */
                UartSendBlocking('\b');
                UartSendBlocking(' ');
                UartSendBlocking('\b');
                /* and move index in input buffer by -1 */
                bytes--;
            }
        }
        else
        {
            buf[bytes] = next_char;
            bytes++;
            /* echo keybord input to screen, without that user wouldn't see what he's typing */
            UartSendBlocking(next_char);
        }

    }

    return bytes;
}