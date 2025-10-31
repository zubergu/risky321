#include "system.h"
#include "kprintf.h"

void kpanic(char *filename, size_t line, char *message)
{
    kprintf("KERNEL PANIC! %s\t%s::%d\n", message, filename, line);
    /* endless loop */
    for(;;);
}

void delay(void)
{
    for(int i = 0; i < 30000000; i++)
    {
        __asm__ __volatile__("nop"); // do nothing
    }
}
