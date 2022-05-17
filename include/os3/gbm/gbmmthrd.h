/**
 * gbmmthrd.h - GBM Multithreading support library
 */

#ifndef _GBMMTHREAD_H_
#define _GBMMTHREAD_H_

#ifdef __cplusplus
  extern "C"
  {
#endif

#include "gbm.h"

/* --------------------------------------------------------- */

/* The thread */
typedef void* GBM_THREAD;

/* The thread function argument */
typedef void* GBM_THREAD_ARG;

/* The thread function to be executed */
typedef void (*GBM_THREAD_FUNC)(GBM_THREAD_ARG);

/**
 * Schedules the specified task in a separate thread.
 *
 * @param function      The function to be executed.
 * @param function_arg  The argument that will be passed to the
 *                      function when the thread starts.
 * @param initStackSize The stack size. Depending on the underlying
 *                      Operating System the may be initial stack size
 *                      of the thread, a fixed size or be ignored.
 * @return The thread id.
 * @retval ==NULL  The task could not be started.
 * @retval !=NULL  The task was successfully started.
 */
GBM_THREAD gbmmthrd_scheduleTask(GBM_THREAD_FUNC function,
                                 GBM_THREAD_ARG  function_arg,
                                 int initStackSize);

/**
 * Waits for the specified thread until it has finished.
 * When the function returns, the thread id is no longer valid.
 *
 * @param thread  The thread to wait for.
 *
 * @note This function has to be called after a successful
 *       call to gbmmthrd_scheduleTask() to wait and cleanup
 *       allocated resources.
 */
void gbmmthrd_finishTask(GBM_THREAD *thread);

/**
 * Returns the number of CPU cores the system provides.
 *
 * @retval >=1
 */
gbm_u32 gbmmthrd_getNumberOfCores(void);

#ifdef __cplusplus
  }  /* extern "C" */
#endif

#endif /* _GBMMTHREAD_H_ */

