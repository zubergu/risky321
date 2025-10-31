#ifndef _KLIBC_H
#define _KLIBC_H

#include "types.h"

void *memcpy(void *dest, const void *src, size_t size);

void *memmove(void *dest, const void *src, size_t n);

void *memset(void *dest, int val, size_t n);

int   memcmp(const void *s1, const void *s2, size_t n);

int strcmp(const char * s1, const char *s2);

#define offsetof(type, member)  __builtin_offsetof (type, member)

#endif