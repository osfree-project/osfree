/*
 *
 */

#if defined(L4API_l4v2)

#include <os3/dataspace.h>

int
l4os3_rm_attach(void **start, unsigned long size, unsigned long flags,
                l4os3_ds_t mem, l4_addr_t offs,
                unsigned char align)
{
  return l4rm_attach(&mem, size, offs, flags, start);
}

#elif defined(L4API_l4f)

int
l4os3_rm_attach(void **start, unsigned long size, unsigned long flags,
                l4os3_ds_t mem, l4_addr_t offs,
                unsigned char align)
{
  return l4re_rm_attach(start, size, flags, mem, offs, align);
}

#else
#error "Not implemented!"
#endif
