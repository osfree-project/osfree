#ifndef __OS3_DATASPACE_H__
#define __OS3_DATASPACE_H__

#ifdef __cplusplus
  extern "C" {
#endif

/* osFree internal */
#include <os3/types.h>

#define L4DM_READ    0x00000001
#define L4DM_WRITE   0x00000002
#define L4DM_RO      (L4DM_READ)
#define L4DM_RW      (L4DM_READ | L4DM_WRITE)
#define L4RM_MAP     0x01000000

#if defined(__l4env__)

/* l4env includes */
#include <l4/env/env.h>
#include <l4/dm_generic/types.h>
#include <l4/dm_mem/dm_mem.h>

typedef l4dm_dataspace_t *l4_os3_dataspace_t;
typedef l4dm_dataspace_t l4os3_ds_t;

long l4os3_ds_allocate(l4os3_ds_t *ds, l4_addr_t offset, l4_size_t size);

#elif defined(__l4re__)

/* l4re includes */
#include <l4/re/c/dataspace.h>

typedef l4re_ds_t *l4_os3_dataspace_t;

#elif defined (__genode__)

typedef void *l4_os3_dataspace_t;

#else
#error "Not implemented!"
#endif

long DataspaceAlloc(l4_os3_dataspace_t *ds, unsigned long flags,
                    l4_os3_cap_idx_t dm, unsigned long size);
long DataspaceFree(l4_os3_dataspace_t ds);
long DataspaceGetSize(l4_os3_dataspace_t ds, unsigned long *size);
long DataspaceShare(l4_os3_dataspace_t ds, void *client_id, unsigned long rights);

long attach_ds(l4_os3_dataspace_t ds, unsigned long flags, void **addr);

long attach_ds_reg(l4_os3_dataspace_t ds, unsigned long flags, void *addr);
long attach_ds_area(l4_os3_dataspace_t ds, unsigned long long area, unsigned long flags, void *addr);

#ifdef __cplusplus
  }
#endif

#endif
