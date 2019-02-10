#ifndef __OS3_SEMAPHORE_H__
#define __OS3_SEMAPHORE_H__

#ifdef __cplusplus
  extern "C" {
#endif

#define  INCL_BASE
#include <os2.h>

#include <platform/semaphore.h>

APIRET SemaphoreInit(l4_os3_semaphore_t **sem, ULONG n);

void SemaphoreDone(l4_os3_semaphore_t *sem);

void SemaphoreUp(l4_os3_semaphore_t *sem);

void SemaphoreDown(l4_os3_semaphore_t *sem);

#ifdef __cplusplus
  }
#endif

#endif /* __OS3_SEMAPHORE_H__ */
