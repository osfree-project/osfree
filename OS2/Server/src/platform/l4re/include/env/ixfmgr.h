#ifndef __l4re_env_ixfmgr_h__
#define __l4re_env_ixfmgr_h__

#ifdef __cplusplus
  extern "C" {
#endif

#include <os3/dataspace.h>

typedef struct
{
  void *addr;
  unsigned long size;
  l4_os3_dataspace_t ds;
} l4exec_section_t;

#ifdef __cplusplus
  }
#endif

#endif /* __l4re_env_ixfmgr_h__ */
