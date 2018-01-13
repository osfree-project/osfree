/* threading interface */

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/thread.h>

/* l4env includes */
#include <l4/sys/types.h>

BOOL ThreadEqual(l4_os3_thread_t one, l4_os3_thread_t two)
{
    return l4_thread_equal(one.thread, two.thread);
}

BOOL TaskEqual(l4_os3_thread_t one, l4_os3_thread_t two)
{
    return (one.thread.id.task == two.thread.id.task);
}
