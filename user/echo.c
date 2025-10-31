#include "ulibc.h"
#include "uprintf.h"
#include "umalloc.h"

uint32_t my_var = 0xCAFEBABE;

int main(int argc, char **argv)
{
    uprintf("I'm an echo %x!\n", my_var);
    uprintf("argc = %d\n", argc);
    
    for(int i = 0; i < argc; i++)
    {
        uprintf("%s ", argv[i]);
    }

    uprintf("\n");
    
    return 0;
}