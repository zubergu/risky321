#ifndef _KSYSCALL_H
#define _KSYSCALL_H

#include "types.h"

#define SYS_fork    1  // done
#define SYS_exit    2  // done
#define SYS_wait    3
#define SYS_pipe    4
#define SYS_read    5  // done
#define SYS_kill    6  // done
#define SYS_exec    7  // done, add memory cleanup of old process
#define SYS_fstat   8
#define SYS_chdir   9
#define SYS_dup    10
#define SYS_getpid 11  // done
#define SYS_sbrk   12  // done, allocates only full pages
#define SYS_sleep  13  // implement as yield, to simply give up processor to other processes for now
#define SYS_uptime 14
#define SYS_open   15  // done
#define SYS_write  16  // done for stdout only
#define SYS_mknod  17
#define SYS_unlink 18
#define SYS_link   19
#define SYS_mkdir  20
#define SYS_close  21  // done

uint32_t ksys_read(int fd, uint8_t *buf, uint32_t size);
uint32_t ksys_exit();
uint32_t ksys_open(char *path, uint32_t mode);
uint32_t ksys_write(int fd, uint8_t *data, uint32_t size);
uint32_t ksys_kill(uint32_t pid);
uint32_t ksys_close(uint32_t fd);
uint32_t ksys_fork();
uint32_t ksys_exec(char *path, char *argv[]);
uint32_t ksys_sleep();
uint32_t ksys_sbrk();

#endif
