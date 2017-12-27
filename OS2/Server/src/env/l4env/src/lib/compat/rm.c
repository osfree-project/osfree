/* Region mapper interface */

/* osFree Internal */
#include <os3/rm.h>

/* l4env includes */
#include <l4/l4rm/l4rm.h>

long RegAreaReserveInArea(unsigned long size,
                          unsigned long flags,
                          void          **addr,
                          unsigned long *area)
{
    return l4rm_area_reserve_in_area(size, flags, (l4_addr_t *)addr, (l4_uint32_t *)area);
}

long RegAreaRelease(unsigned long area)
{
    return l4rm_area_release((l4_uint32_t)area);
}

long RegAttach(void               **start,
               unsigned long      size,
               unsigned long      flags,
               l4_os3_dataspace_t ds,
               void               *offset,
               unsigned char      align)
{
    return l4rm_attach((l4dm_dataspace_t *)ds, (l4_size_t)size,
                       (l4_addr_t)offset, flags, start);
}

long RegAttachToRegion(void               **start,
                       unsigned long      size,
                       unsigned long      flags,
                       l4_os3_dataspace_t ds,
                       void               *offset,
                       unsigned char      align)
{
    return l4rm_attach_to_region((l4dm_dataspace_t *)ds, *start, size,
                                 (l4_addr_t)offset, flags);
}

long RegAreaAttachToRegion(void               **start,
                           unsigned long      size,
                           unsigned long      area,
                           unsigned long      flags,
                           l4_os3_dataspace_t ds,
                           void               *offset,
                           unsigned char      align)
{
    return l4rm_area_attach_to_region((l4dm_dataspace_t *)ds, area, *start, size,
                                      (l4_addr_t)offset, flags);
}

long RegAttachDataspaceToArea(l4_os3_dataspace_t ds,
                              unsigned long      area,
                              unsigned long      rights,
                              void               *addr)
{
    return attach_ds_area(ds, area, rights, (l4_addr_t)addr);
}

long RegDetach(void *addr)
{
    l4rm_detach(addr);
    return 0;
}

long RegLookupRegion(void               *addr,
                     void               **addr_new,
                     unsigned long      *size,
                     l4_os3_dataspace_t *ds)
{
    l4_offs_t     offset;
    l4_threadid_t pager;
    return l4rm_lookup_region(addr, (l4_addr_t *)addr_new, (l4_size_t *)size,
                              *ds, &offset, &pager);
}
