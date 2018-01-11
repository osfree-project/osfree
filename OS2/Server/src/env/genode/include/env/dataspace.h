#ifndef __GENODE_ENV_DATASPACE_H__
#define __GENODE_ENV_DATASPACE_H__

#ifdef __cplusplus
  extern "C" {
#endif

/* osFree internal */
#include <os3/types.h>

#define INVALID_DATASPACE {0, 0, 0, 0}

#define DEFAULT_DSM NIL_THREAD

#ifdef __cplusplus
  }
#endif

#endif /* __GENODE_ENV_DATASPACE_H__ */
