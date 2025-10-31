#ifndef _SYSTEM_H
#define _SYSTEM_H

#include "types.h"

void kpanic(char *filename, size_t line, char *message);
void delay(void);

#endif