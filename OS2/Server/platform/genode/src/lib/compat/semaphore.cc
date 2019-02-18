/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/semaphore.h>

/* Genode includes */
#include <base/allocator.h>
#include <base/semaphore.h>

/* local includes */
#include <genode_env.h>

extern "C" APIRET
SemaphoreInit(l4_os3_semaphore_t **sem, ULONG n)
{
    Genode::Allocator &alloc = genode_alloc();

    try
    {
        **sem = new (alloc) Genode::Semaphore(n);
    }
    catch (...)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    return NO_ERROR;
}

extern "C" void
SemaphoreDone(l4_os3_semaphore_t *sem)
{
    Genode::Allocator &alloc = genode_alloc();

    destroy(alloc, (Genode::Semaphore *)(*sem));
}

extern "C" void
SemaphoreUp(l4_os3_semaphore_t *sem)
{
    ((Genode::Semaphore *)(*sem))->up();
}

extern "C" void
SemaphoreDown(l4_os3_semaphore_t *sem)
{
    ((Genode::Semaphore *)(*sem))->down();
}
