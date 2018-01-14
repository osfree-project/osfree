#ifndef __OS3_DATASPACE_H__
#define __OS3_DATASPACE_H__

#ifdef __cplusplus
  extern "C" {
#endif

/* osFree internal */
#include <os3/types.h>
#include <os3/thread.h>

#define L4DM_READ    0x00000001
#define L4DM_WRITE   0x00000002
#define L4DM_RO      (L4DM_READ)
#define L4DM_RW      (L4DM_READ | L4DM_WRITE)
#define L4RM_MAP     0x01000000

typedef l4_os3_cap_idx_t l4_os3_dataspace_t;

#include <platform/dataspace.h>

long DataspaceAlloc(l4_os3_dataspace_t *ds, unsigned long flags,
                    l4_os3_cap_idx_t dm, unsigned long size);
long DataspaceFree(l4_os3_dataspace_t ds);
long DataspaceGetSize(l4_os3_dataspace_t ds, unsigned long *size);
long DataspaceShare(l4_os3_dataspace_t ds, l4_os3_thread_t client_id, unsigned long rights);

long attach_ds(l4_os3_dataspace_t ds, unsigned long flags, void **addr);

long attach_ds_reg(l4_os3_dataspace_t ds, unsigned long flags, void *addr);
long attach_ds_area(l4_os3_dataspace_t ds, unsigned long long area, unsigned long flags, void *addr);

#ifdef __cplusplus
  }
#endif

#endif
