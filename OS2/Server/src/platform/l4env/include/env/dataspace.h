#ifndef __L4ENV_ENV_DATASPACE_H__
#define __L4ENV_ENV_DATASPACE_H__

#ifdef __cplusplus
  extern "C" {
#endif

/* l4env includes */
#include <l4/env/env.h>
#include <l4/dm_generic/types.h>
#include <l4/dm_mem/dm_mem.h>

#define INVALID_DATASPACE ((l4_os3_dataspace_t){.ds=(l4dm_dataspace_t)(L4DM_INVALID_DATASPACE)})

#define DEFAULT_DSM ((l4_os3_cap_idx_t){(l4_threadid_t)L4DM_DEFAULT_DSM})

typedef l4dm_dataspace_t l4os3_ds_t;

long l4os3_ds_allocate(l4os3_ds_t *ds, l4_addr_t offset, l4_size_t size);

#ifdef __cplusplus
  }
#endif

#endif /* __L4ENV_ENV_DATASPACE_H__ */
