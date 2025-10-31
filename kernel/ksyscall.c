#include "ksyscall.h"
#include "kprintf.h"
#include "kproc.h"
#include "kterminal.h"

uint32_t ksys_fork()
{
    return kproc_fork();
}

uint32_t ksys_read(int fd, uint8_t *buf, uint32_t size)
{
    if(fd == 0)
    {
        return kterminal_readline_block((char *)buf, size);
    }

    if(fd == 1 || fd == 2)
    {
        return 0;
    }

    struct os_file * file_handle = kproc_file_get(fd);

    return filesystem_file_read(file_handle, buf, size);

}

uint32_t ksys_exit()
{
    kproc_free();
    return 0;
}

uint32_t ksys_write(int fd, uint8_t *data, uint32_t size)
{
    if(fd == 1)
    {
        for(uint32_t i = 0; i < size; i++)
        {
            kprintf("%c", (char)(data[i]));
        }
        
        return 0;
    }
    
    /* error */
    return -1;
}

uint32_t ksys_kill(uint32_t pid)
{
    return kproc_kill(pid);
}


uint32_t ksys_open(char *path, uint32_t mode)
{
    return filesystem_file_open(path, mode);
}

uint32_t ksys_close(uint32_t fd)
{
    return filesystem_file_close(fd);
}

uint32_t ksys_exec(char *path, char *argv[])
{
    return kproc_exec(path, argv);
}

uint32_t ksys_sleep()
{
    kproc_yield();
    return 0;
}

uint32_t ksys_sbrk()
{
    return kproc_add_page();
}
