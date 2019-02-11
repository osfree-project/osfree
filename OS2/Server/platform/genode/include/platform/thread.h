#ifndef __GENODE_ENV_THREAD_H__
#define __GENODE_ENV_THREAD_H__

#ifdef __cplusplus
  extern "C" {
#endif

typedef void * l4_os3_thread_t;

#define INVALID_THREAD NULL

#define THREAD_SYNC  1
#define THREAD_ASYNC 2

#ifdef __cplusplus
  }
#endif

#endif /* __GENODE_ENV_THREAD_H__ */
