#ifndef _SYSTEM_H
#define _SYSTEM_H

#include "types.h"

#define KernelPanic(message) kpanic(__FILE_NAME__, __LINE__, (message))

void kpanic(char *filename, size_t line, char *message);
void delay(void);

#endif