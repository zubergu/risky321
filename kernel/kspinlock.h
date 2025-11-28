#ifndef _KSPINLOCK_H
#define _KSPINLOCK_H

#include "types.h"

struct kspinlock
{
    uint8_t isLocked;

    char *name;
};

#endif
