#include <stdarg.h>

#include "kprintf.h"
#include "types.h"
#include "uart.h"

char hex_lookup[] = "0123456789ABCDEF";

/* support function to print decimal representation of int/uint32_t */
static void print_dec(int val, bool isSigned)
{
    char str[16];
    uint32_t uval = val;
    int i = 0;

    bool sign = (val < 0);

    if(isSigned && sign)
    {
        uval = -val;
    }

    /* calculate digits right to left, and put in array left to right */
    do
    {
        str[i] = hex_lookup[uval % 10]; /* remainder to digit */
        uval = uval / 10;
        i++;
    } while (uval);
    
    if(isSigned && sign)
    {
        str[i] = '-';
        i++;
    }

    i--;

    /* send characters from array to uart in reverse order, right to left*/
    while(i>=0)
    {
        UartSendBlocking(str[i]);
        i--;
    }

}

/* support function to print hexadecimal representation of uint32_t and pointer(addr_t) */
static void print_hex(uint32_t val)
{
    UartSendBlocking('0');
    UartSendBlocking('x');
    
    for(int i = 7; i >=0; i--)
    {
        UartSendBlocking(hex_lookup[(val >> i*4) & 0x0F]);
    }
}

/* kernel side printf version */
int kprintf(const char * format, ...)
{
    va_list arguments;

    va_start(arguments, format);

    while(*format != '\0')
    {
        if(*format == '%')
        {
            format++;

            switch(*format)
            {
                case '\0':
                    UartSendBlocking('%');
                    va_end(arguments);
                    return 0;
                case '%':
                    UartSendBlocking('%');
                    break;
                case 'c':
                    {
                        int val = va_arg(arguments, int);
                        UartSendBlocking(val);
                        break;
                    }
                case 'd':
                    {
                        int val = va_arg(arguments, int);
                        print_dec(val, TRUE);
                        break;
                    }
                case 'u':
                    {
                        uint32_t val = va_arg(arguments, uint32_t);
                        print_dec(val, FALSE);
                        break;
                    }
                case 'x':
                    {
                        uint32_t val = va_arg(arguments, uint32_t);
                        print_hex(val);
                        break;
                    }
                case 'p':
                    {
                        addr_t val = va_arg(arguments, addr_t);
                        print_hex(val);
                        break;
                    }
                case 's':
                    {
                        const char * str = va_arg(arguments, const char *);
                        while(*str)
                        {
                            UartSendBlocking(*str);
                            str++;
                        }
                        break;
                    }
                default:
                    va_end(arguments);
                    return -1;
            }

        }
        else
        {
            UartSendBlocking(*format);
        }

        /* go to next character in format */
        format++;
    }

    va_end(arguments);
    return 0;
}
