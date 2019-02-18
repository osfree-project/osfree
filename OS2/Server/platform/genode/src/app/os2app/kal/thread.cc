/* osFree internal */
#include <os3/thread.h>

/* Genode includes */
#include <base/thread.h>

/* Get an identifier of a local thread
   (to distinguish between different threads) */
extern "C" l4_os3_thread_t
KalNativeID(void)
{
  Genode::Thread *thread = Genode::Thread::myself();
  l4_os3_thread_t id = (l4_os3_thread_t)thread;
  return id;
}
