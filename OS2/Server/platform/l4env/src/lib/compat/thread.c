/* threading interface */

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/thread.h>

/* l4env includes */
#include <l4/sys/types.h>
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

void ThreadExit(void)
{
    l4thread_exit();
}

void TaskExit(void)
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
