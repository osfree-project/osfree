#ifndef __OS3_DATASPACE_H__
#define __OS3_DATASPACE_H__

#include <l4/env/env.h>
#include <l4/dm_mem/dm_mem.h>

typedef l4dm_dataspace_t *l4re_ds_t;

L4_CV L4_INLINE long
l4re_ds_allocate(const l4re_ds_t ds, l4_addr_t offset, l4_size_t size);

L4_CV L4_INLINE long
l4re_ds_size(const l4re_ds_t ds);

// implementation

L4_CV L4_INLINE long
l4re_ds_allocate(const l4re_ds_t ds, l4_addr_t offset, l4_size_t size)
{
  return l4dm_mem_open(l4env_get_default_dsm(), size, offset, 0, "", ds);
}

L4_CV L4_INLINE long
l4re_ds_size(const l4re_ds_t ds)
{
  l4_size_t size;
  int error;

  if ((error = l4dm_mem_size(ds, &size)))
    return error;
  else
    return size;
}

#endif
