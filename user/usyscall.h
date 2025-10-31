#ifndef _USYSCALL_H
#define _USYSCALL_H

#include "../kernel/types.h"

extern uint32_t usys_fork();
extern uint32_t usys_open(char *path, uint32_t mode);
extern uint32_t usys_read(int fd, uint8_t *buf, uint32_t size);
extern void usys_exit();
extern void usys_write(int fd, uint8_t *data, uint32_t size);
extern uint32_t usys_getpid();
extern uint32_t usys_kill(uint32_t pid);
extern uint32_t usys_exec(char *path, char *argv[]);
extern uint32_t usys_close(uint32_t fd);
extern uint32_t usys_sleep();
extern void * usys_sbrk();

#endif