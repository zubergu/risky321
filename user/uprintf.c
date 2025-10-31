#include "uprintf.h"
#include <stdarg.h>

char hex_lookup[] = "0123456789ABCDEF";
char percent = '%';

/* support function to print decimal representation of int/uint32_t */
static void print_dec(int val, bool isSigned)
{
    char str[16];
    uint32_t uval = (uint32_t)val;
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
        usys_write(1, (uint8_t *)&str[i], 1);
        i--;
    }

}

/* support function to print hexadecimal representation of uint32_t and pointer(addr_t) */
static void print_hex(uint32_t val)
{
    usys_write(1, (uint8_t *)&"0"[0], 1);
    usys_write(1, (uint8_t *)&"x"[0], 1);
    
    for(int i = 7; i >=0; i--)
    {
        usys_write(1, (uint8_t *)&hex_lookup[(val >> i*4) & 0x0F], 1);
    }
}

/* user side printf version */
int uprintf(const char * format, ...)
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
                    usys_write(1, (uint8_t *)&percent, 1);
                    va_end(arguments);
                    return 0;
                case '%':
                    usys_write(1, (uint8_t *)&percent, 1);
                    break;
                case 'c':
                    {
                        int val = va_arg(arguments, int);
                        usys_write(1, (uint8_t *)&val, 1);
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
                            usys_write(1, (uint8_t *)str, 1);
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
            usys_write(1, (uint8_t *)format, 1);
        }

        /* go to next character in format */
        format++;
    }

    va_end(arguments);
    return 0;
}
