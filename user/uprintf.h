#ifndef _UPRINTF_H
#define _UPRINTF_H

#include "../kernel/types.h"
#include "usyscall.h"

int uprintf(const char *format, ...);

#endif