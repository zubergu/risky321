#ifndef _ULIBC_H
#define _ULIBC_H

#include "../kernel/types.h"

__attribute__((noreturn)) void exit(void);

void *memcpy(void *dest, const void *src, size_t size);

void *memmove(void *dest, const void *src, size_t n);

void *memset(void *dest, int val, size_t n);

int   memcmp(const void *s1, const void *s2, size_t n);

#endif