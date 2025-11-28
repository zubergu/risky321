#include "kspinlock.h"
#include "system.h"

void
kspinlock_init(struct kspinlock *lock, char *name)
{
    lock->isLocked = 0;
    lock->name = name;
}


void
kspinlock_acquire(struct kspinlock *lock)
{
    while(__sync_lock_test_and_set(&lock->isLocked, 1) != 0)
    {
        continue;
    }

    __sync_synchronize();
}


void
kspinlock_release(struct kspinlock *lock)
{
    __sync_synchronize();
    __sync_lock_release(&lock->isLocked);
}


uint8_t
kspinlock_holding(struct kspinlock *lock)
{
    return lock->isLocked;
}