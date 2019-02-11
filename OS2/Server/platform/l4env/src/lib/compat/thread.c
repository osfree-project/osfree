/* threading interface */

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/thread.h>

/* l4env includes */
#include <l4/sys/types.h>
#include <l4/util/util.h>
#include <l4/thread/thread.h>
#include <l4/generic_ts/generic_ts.h>

BOOL ThreadEqual(l4_os3_thread_t one, l4_os3_thread_t two)
{
    return l4_thread_equal(one.thread, two.thread);
}

BOOL TaskEqual(l4_os3_thread_t one, l4_os3_thread_t two)
{
    return (one.thread.id.task == two.thread.id.task);
}

void ThreadSleep(unsigned long ms)
{
    l4_sleep(ms);
}

void ThreadExit(void)
{
    l4thread_exit();
}

void TaskExit(int result)
{
    l4ts_exit();
}

l4_os3_thread_t ThreadCreate(ThreadFunc fn, void *data, ULONG flags)
{
    l4thread_t t;
    l4_os3_thread_t thread;

    t = l4thread_create((l4thread_fn_t)fn, data, flags);

    thread.thread = l4thread_l4_id(t);
    return thread;
}

l4_os3_thread_t ThreadCreateLong(ThreadFunc fn, void *data, ULONG flags,
                                 const char *name, ULONG stacksize)
{
    l4_os3_thread_t thread;
    l4thread_t rc;

    if ( (rc = l4thread_create_long(L4THREAD_INVALID_ID, fn, name,
                       L4THREAD_INVALID_SP, stacksize, L4THREAD_DEFAULT_PRIO,
                       &data, flags)) < 0)
    {
        return INVALID_THREAD;
    }

    thread.thread = l4thread_l4_id(rc);
    return thread;
}

void ThreadKill(l4_os3_thread_t native)
{
    l4thread_shutdown(l4thread_id(native.thread));
}

void ThreadSuspend(l4_os3_thread_t native)
{
}

void ThreadResume(l4_os3_thread_t native)
{
}

void ThreadWait(l4_os3_thread_t native)
{
}
