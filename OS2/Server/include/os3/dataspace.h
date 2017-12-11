#ifndef __OS3_DATASPACE_H__
#define __OS3_DATASPACE_H__

#ifdef __cplusplus
  extern "C" {
#endif

/* osFree internal */
#include <os3/types.h>

#if defined(L4API_l4v2)

/* l4env includes */
#include <l4/env/env.h>
#include <l4/dm_generic/types.h>
#include <l4/dm_mem/dm_mem.h>

typedef l4dm_dataspace_t l4os3_ds_t;

long
l4os3_ds_allocate(l4os3_ds_t *ds, l4_addr_t offset, l4_size_t size);

long
l4os3_ds_size(l4os3_ds_t ds);

#elif defined(L4API_l4f)

/* l4re includes */
#include <l4/re/c/dataspace.h>

typedef l4re_ds_t l4os3_ds_t;

long
l4os3_ds_allocate(l4os3_ds_t ds, l4_addr_t offset, l4_size_t size);

long
l4os3_ds_size(l4os3_ds_t ds);

#else
#error "Not implemented!"
#endif

int attach_ds(l4os3_ds_t *ds, l4_uint32_t flags, l4_addr_t *addr);
int attach_ds_reg(l4dm_dataspace_t ds, l4_uint32_t flags, l4_addr_t addr);
int attach_ds_area(l4dm_dataspace_t ds, l4_uint32_t area, l4_uint32_t flags, l4_addr_t addr);

#ifdef __cplusplus
  }
#endif

#endif
