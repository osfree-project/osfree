#ifndef __OS3_SEMAPHORE_H__
#define __OS3_SEMAPHORE_H__

#ifdef __cplusplus
  extern "C" {
#endif

#include <platform/semaphore.h>

void SemaphoreUp(l4_os3_semaphore_t *sem);

void SemaphoreDown(l4_os3_semaphore_t *sem);

#ifdef __cplusplus
  }
#endif

#endif /* __OS3_SEMAPHORE_H__ */
