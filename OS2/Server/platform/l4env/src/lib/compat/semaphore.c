/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/semaphore.h>

/* libc includes */
#include <stdlib.h>

APIRET SemaphoreInit(l4_os3_semaphore_t **sem, ULONG n)
{
    *sem = (l4_os3_semaphore_t *)malloc(sizeof(l4_os3_semaphore_t));

    if (! *sem)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    **sem = SEMAPHORE_INIT(n);

    return NO_ERROR;
}

void SemaphoreDone(l4_os3_semaphore_t *sem)
{
    free(sem);
}

void SemaphoreUp(l4_os3_semaphore_t *sem)
{
    l4semaphore_up((l4semaphore_t *)sem);
}

void SemaphoreDown(l4_os3_semaphore_t *sem)
{
    l4semaphore_down((l4semaphore_t *)sem);
}
