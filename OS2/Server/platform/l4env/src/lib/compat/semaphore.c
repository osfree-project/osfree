/* osFree internal */
#include <os3/semaphore.h>

void SemaphoreUp(l4_os3_semaphore_t *sem)
{
    l4semaphore_up((l4semaphore_t *)sem);
}

void SemaphoreDown(l4_os3_semaphore_t *sem)
{
    l4semaphore_down((l4semaphore_t *)sem);
}
