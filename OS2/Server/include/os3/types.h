#ifndef __OS3_TYPES_H__
#define __OS3_TYPES_H__

#ifdef __cplusplus
  extern "C" {
#endif

#ifdef L4API_l4v2

/* l4env includes */
#include <l4/sys/types.h>

typedef l4_threadid_t l4_os3_cap_idx_t;

#elif defined(L4API_l4f)

#include <l4/sys/types.h>

typedef l4_cap_idx_t l4_os3_cap_idx_t;

#elif 1 // Genode @todo use more correct define

typedef void *l4_os3_cap_idx_t;

#else
#error "port me!"
#endif

#ifdef __cplusplus
  }
#endif

#endif
