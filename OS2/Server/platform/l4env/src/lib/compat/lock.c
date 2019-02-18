/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/lock.h>

#include <l4/lock/lock.h>

/* libc includes */
#include <stdlib.h>

APIRET LockInit(l4_os3_lock_t **lock, ULONG n)
{
    *lock = (l4_os3_lock_t *)malloc(sizeof(l4_os3_lock_t));

    if (! *lock)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    **lock = L4LOCK_UNLOCKED;

    return NO_ERROR;
}

void LockDone(l4_os3_lock_t *lock)
{
    free(lock);
}

void LockLock(l4_os3_lock_t *lock)
{
    l4lock_lock((l4lock_t *)lock);
}

void LockUnlock(l4_os3_lock_t *lock)
{
    l4lock_unlock((l4lock_t *)lock);
}
