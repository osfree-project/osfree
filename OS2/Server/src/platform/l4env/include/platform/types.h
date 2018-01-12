#ifndef __L4ENV_ENV_TYPES_H__
#define __L4ENV_ENV_TYPES_H__

#ifdef __cplusplus
  extern "C" {
#endif

/* l4env includes */
#include <l4/sys/types.h>
#include <l4/dm_generic/types.h>

typedef union
{
    l4_threadid_t     thread;
    l4dm_dataspace_t  ds;
    struct
    {
        unsigned long v[4];
    } raw;
} l4_os3_cap_idx_t;

#ifdef __cplusplus
  }
#endif

#endif /* __L4ENV_ENV_TYPES_H__ */
