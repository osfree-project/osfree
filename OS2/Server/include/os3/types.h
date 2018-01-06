#ifndef __OS3_TYPES_H__
#define __OS3_TYPES_H__

#ifdef __cplusplus
  extern "C" {
#endif

#if defined(__l4env__)

/* l4env includes */
#include <l4/sys/types.h>

typedef l4_threadid_t l4_os3_cap_idx_t;

#elif defined(__l4re__)

#include <l4/sys/types.h>

typedef l4_cap_idx_t l4_os3_cap_idx_t;

#elif defined(__genode__)

typedef void *l4_os3_cap_idx_t;

#else
#error "port me!"
#endif

#ifdef __cplusplus
  }
#endif

#endif
