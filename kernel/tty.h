#ifndef _TTY_H
#define _TTY_H

#include "types.h"

#define TTY_BUFFER_SIZE 1024
#define BACKSPACE 127

int
TtyWrite(uint8_t *src, uint32_t n);

int
TtyRead(uint8_t *dest, uint32_t n);

int
TtyBufferInsert(uint8_t c);

#endif