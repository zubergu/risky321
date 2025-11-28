#include "tty.h"
#include "uart.h"

static uint8_t ttyBuffer[TTY_BUFFER_SIZE];
uint8_t isEmpty = TRUE;
uint8_t isFull = FALSE;

int r = 0;
int w = 0;
int insert = 0;

int
TtyBufferInsert(uint8_t c)
{
    if(isFull)
    {
        return FALSE;
    }

    if(c == '\r')
    {
        c = '\n';
    }

    if(c == BACKSPACE)
    {
        int prev = (insert + TTY_BUFFER_SIZE - 1) % TTY_BUFFER_SIZE;
        if(!isEmpty && ttyBuffer[prev] != '\n')
        {
            insert = prev;
            UartSendBlocking('\b');
            UartSendBlocking(' ');
            UartSendBlocking('\b');
            if(insert == r)
            {
                isEmpty = TRUE;
            }
        }
        
        return TRUE;
    }
    

    ttyBuffer[insert] = c;
    insert++;
    insert = insert % TTY_BUFFER_SIZE;
    isEmpty = FALSE;


    if(c == '\n')
    {
        w = insert;
    }

    if(insert == r)
    {
        isFull = TRUE;
    }

    /* character was inserted, echo back to console through UART */

    UartSendBlocking(c);
    return TRUE;
}

int
TtyWrite(uint8_t *src, uint32_t n)
{
    int i = 0;
    while(i < n)
    {
        UartSendBlocking(src[i]);
        i++;
    }
    return i;
}

int
TtyRead(uint8_t  *dest, uint32_t n)
{
    int i = 0;

    if(isEmpty)
    {
        return 0;
    }

    while((i < n) && (!isEmpty) && (r != w))
    {
        isFull = FALSE;
        
        dest[i] = ttyBuffer[r];
        i++;
        r++;
        r %= TTY_BUFFER_SIZE;
        
        if(dest[i-1] == '\n')
        {
            break;
        }
    }

    if(r == insert)
    {
        isEmpty = TRUE;
    }

    return i;
}