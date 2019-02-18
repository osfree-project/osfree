#ifndef __OS3_LOCK_H__
#define __OS3_LOCK_H__

#ifdef __cplusplus
  extern "C" {
#endif

#define  INCL_BASE
#include <os2.h>

#include <platform/lock.h>

APIRET LockInit(l4_os3_lock_t **lock, ULONG n);

void LockDone(l4_os3_lock_t *lock);

void LockLock(l4_os3_lock_t *lock);

void LockUnlock(l4_os3_lock_t *lock);

#ifdef __cplusplus
  }
#endif

#endif /* __OS3_LOCK_H__ */
