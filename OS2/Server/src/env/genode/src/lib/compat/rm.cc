/* Region mapper interface */

/* osFree Internal */
#include <os3/rm.h>

/* Genode includes */

extern "C"
long RegAreaReserveInArea(unsigned long size,
                          unsigned long flags,
                          void          **addr,
                          unsigned long long *area)
{
    return 0;
}

extern "C"
long RegAreaRelease(unsigned long long area)
{
    return 0;
}

extern "C"
long RegAttach(void               **start,
               unsigned long      size,
               unsigned long      flags,
               l4_os3_dataspace_t ds,
               void               *offset,
               unsigned char      align)
{
    return 0;
}

extern "C"
long RegAttachToRegion(void               **start,
                       unsigned long      size,
                       unsigned long      flags,
                       l4_os3_dataspace_t ds,
                       void               *offset,
                       unsigned char      align)
{
    return 0;
}

extern "C"
long RegAreaAttachToRegion(void               **start,
                           unsigned long      size,
                           unsigned long long area,
                           unsigned long      flags,
                           l4_os3_dataspace_t ds,
                           void               *offset,
                           unsigned char      align)
{
    return 0;
}

extern "C"
long RegAttachDataspaceToArea(l4_os3_dataspace_t ds,
                              unsigned long long area,
                              unsigned long      rights,
                              void               *addr)
{
    return attach_ds_area(ds, area, rights, addr);
}

extern "C"
long RegDetach(void *addr)
{
    return 0;
}

extern "C"
long RegLookupRegion(void               *addr,
                     void               **addr_new,
                     unsigned long      *size,
                     l4_os3_dataspace_t *ds)
{
    return 0;
}
