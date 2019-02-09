#ifndef __OS3_DATASPACE_H__
#define __OS3_DATASPACE_H__

#ifdef __cplusplus
  extern "C" {
#endif

/* osFree internal */
#include <os3/types.h>
#include <os3/thread.h>

#define DATASPACE_READ    0x00000001
#define DATASPACE_WRITE   0x00000002
#define DATASPACE_EXECUTE 0x00000004
#define DATASPACE_RO      (DATASPACE_READ)
#define DATASPACE_RW      (DATASPACE_READ | DATASPACE_WRITE)
#define DATASPACE_MAP     0x01000000

typedef l4_os3_cap_idx_t l4_os3_dataspace_t;

#include <platform/dataspace.h>

long DataspaceAlloc(l4_os3_dataspace_t *ds, unsigned long flags,
                    l4_os3_cap_idx_t dm, unsigned long size);
long DataspaceFree(l4_os3_dataspace_t ds);
long DataspaceGetSize(l4_os3_dataspace_t ds, unsigned long *size);
long DataspaceShare(l4_os3_dataspace_t ds, l4_os3_thread_t client_id, unsigned long rights);

long DataspaceCopy(l4_os3_dataspace_t ds, ULONG src_offs, ULONG dst_offs,
                   ULONG size, ULONG dst_addr, ULONG dst_size,
                   ULONG flags, l4_os3_dataspace_t *copy);

long attach_ds(l4_os3_dataspace_t ds, unsigned long flags, void **addr);

long attach_ds_reg(l4_os3_dataspace_t ds, unsigned long flags, void *addr);
long attach_ds_area(l4_os3_dataspace_t ds, unsigned long long area, unsigned long flags, void *addr);

#ifdef __cplusplus
  }
#endif

#endif
