#ifndef __GENODE_PLATFORM_SEMAPHORE_H__
#define __GENODE_PLATFORM_SEMAPHORE_H__

#ifdef __cplusplus
  extern "C" {
#endif

#define SEMAPHORE_INIT(a) NULL

typedef void *l4_os3_semaphore_t;
typedef void *l4_os3_lock_t;

#ifdef __cplusplus
  }
#endif

#endif /* __GENODE_PLATFORM_SEMAPHORE_H__ */
