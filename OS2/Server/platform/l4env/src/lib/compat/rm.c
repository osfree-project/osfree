/* Region mapper interface */

/*
l4rm_detach
l4rm_attach
l4rm_attach_to_region
//l4rm_direct_area_setup_region
l4rm_area_release
l4rm_area_release_addr
l4rm_area_reserve
l4rm_area_reserve_region
//l4rm_area_reserve_in_area
l4rm_area_reserve_region_in_area
l4rm_area_attach_to_region
l4rm_area_attach
l4rm_lookup
//l4rm_lookup_region
//l4rm_show_region_list
 */

/* osFree Internal */
#include <os3/rm.h>

/* l4env includes */
#include <l4/l4rm/l4rm.h>

long RegDetach(void *addr)
{
    l4rm_detach(addr);
    return 0;
}

long RegAttach(void               **start,
               unsigned long      size,
               unsigned long      flags,
               l4_os3_dataspace_t ds,
               void               *offset,
               unsigned char      align)
{
    return l4rm_attach(&ds.ds, (l4_size_t)size,
                       (l4_addr_t)offset, flags, start);
}

long RegAttachToRegion(void               **start,
                       unsigned long      size,
                       unsigned long      flags,
                       l4_os3_dataspace_t ds,
                       void               *offset,
                       unsigned char      align)
{
    return l4rm_attach_to_region(&ds.ds, *start, size,
                                 (l4_addr_t)offset, flags);
}

long RegAreaRelease(unsigned long long area)
{
    return l4rm_area_release((l4_uint32_t)area);
}

long RegAreaReserveInArea(unsigned long size,
                          unsigned long flags,
                          void          **addr,
                          unsigned long long *area)
{
    return l4rm_area_reserve_in_area(size, flags, (l4_addr_t *)addr, (l4_uint32_t *)area);
}

long RegAreaAttachToRegion(void               **start,
                           unsigned long      size,
                           unsigned long long area,
                           unsigned long      flags,
                           l4_os3_dataspace_t ds,
                           void               *offset,
                           unsigned char      align)
{
    return l4rm_area_attach_to_region(&ds.ds, (unsigned long)area, *start, size,
                                      (l4_addr_t)offset, flags);
}

long RegAttachDataspaceToArea(l4_os3_dataspace_t ds,
                              unsigned long long area,
                              unsigned long      rights,
                              void               *addr)
{
    return attach_ds_area(ds, area, rights, (l4_addr_t)addr);
}

long RegLookupRegion(void               *addr,
                     void               **addr_new,
                     unsigned long      *size,
                     unsigned long      *offset,
                     l4_os3_dataspace_t *ds)
{
    l4_threadid_t pager;
    long ret = l4rm_lookup_region(addr, (l4_addr_t *)addr_new, (l4_size_t *)size,
                                  &ds->ds, (l4_offs_t *)&offset, &pager);
    return ret;
}
