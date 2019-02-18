#ifndef __L4ENV_PLATFORM_LOCK_H__
#define __L4ENV_PLATFORM_LOCK_H__

#ifdef __cplusplus
  extern "C" {
#endif

#include <l4/lock/lock.h>

//#define LOCK_LOCKED(n)    L4LOCK_LOCKED(n)
//#define LOCK_UNLOCKED  L4LOCK_UNLOCKED

typedef l4lock_t l4_os3_lock_t;

#ifdef __cplusplus
  }
#endif

#endif /* __L4ENV_PLATFORM_LOCK_H__ */
