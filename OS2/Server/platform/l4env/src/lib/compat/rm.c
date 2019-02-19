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
#include <l4/env/errno.h>

long RegDetach(void *addr)
{
    l4rm_detach(addr);
    return 0;
}

long RegAttach(void               **start,
               unsigned long      size,
               unsigned long      flags,
               l4_os3_dataspace_t ds,
               unsigned long      offset,
               unsigned char      align)
{
    ULONG rights = 0;
    int rc = NO_ERROR;

    if (flags & DATASPACE_READ)
        rights |= L4DM_READ;
    if (flags & DATASPACE_WRITE)
        rights |= L4DM_WRITE;

    rc = l4rm_attach(&ds.ds, size, offset, rights, start);

    if (rc < 0)
    {
        switch (-rc)
        {
            case L4_EINVAL:
                rc = ERROR_INVALID_DATASPACE;
                break;

            case L4_EUSED:
                rc = ERROR_ALREADY_USED;
                break;

            case L4_ENOMEM:
                rc = ERROR_NOT_ENOUGH_MEMORY;
                break;

            case L4_ENOMAP:
                rc = ERROR_NO_MAPPING;
                break;

            case L4_EIPC:
                rc = ERROR_IPC;
        }
    }

    return rc;
}

long RegAttachToRegion(void               **start,
                       unsigned long      size,
                       unsigned long      flags,
                       l4_os3_dataspace_t ds,
                       unsigned long      offset,
                       unsigned char      align)
{
    ULONG rights = 0;
    int rc = NO_ERROR;

    if (flags & DATASPACE_READ)
        rights |= L4DM_READ;
    if (flags & DATASPACE_WRITE)
        rights |= L4DM_WRITE;

    rc = l4rm_attach_to_region(&ds.ds, *start, size,
                               offset, rights);

    if (rc < 0)
    {
        switch (-rc)
        {
            case L4_EINVAL:
                rc = ERROR_INVALID_DATASPACE;
                break;

            case L4_EUSED:
                rc = ERROR_ALREADY_USED;
                break;

            case L4_ENOMEM:
                rc = ERROR_NOT_ENOUGH_MEMORY;
                break;

            case L4_ENOMAP:
                rc = ERROR_NO_MAPPING;
                break;

            case L4_EIPC:
                rc = ERROR_IPC;
        }
    }

    return rc;
}

long RegAreaRelease(unsigned long long area)
{
    return l4rm_area_release((l4_uint32_t)area);
    //return NO_ERROR;
}

long RegAreaReleaseAddr(void *addr)
{
    int rc = NO_ERROR;

    return l4rm_area_release_addr(addr);

    if (rc < 0)
    {
        rc = ERROR_INVALID_ADDRESS;
    }

    return rc;
}

long RegAreaReserveInArea(unsigned long size,
                          unsigned long flags,
                          void          **addr,
                          unsigned long long *area)
{
    ULONG rights = 0;
    long rc = NO_ERROR;

    if (flags & DATASPACE_READ)
        rights |= L4DM_READ;
    if (flags & DATASPACE_WRITE)
        rights |= L4DM_WRITE;

    rc = l4rm_area_reserve_in_area(size, rights, (l4_addr_t *)addr, (l4_uint32_t *)area);

    if (rc < 0)
    {
      switch (-rc)
      {
        case L4_ENOMEM:
        case L4_ENOTFOUND:
          rc = ERROR_NOT_ENOUGH_MEMORY;
        default:
          rc = ERROR_ACCESS_DENIED;
      }
    }

    return rc;
}

long RegAreaReserveRegionInArea(unsigned long size,
                                unsigned long flags,
                                void          *addr,
                                unsigned long long *area)
{
    return RegAreaReserveInArea(size, flags, &addr, area);
}

long RegAreaReserve(unsigned long size,
                          unsigned long flags,
                          void          **addr,
                          unsigned long long *area)
{
    unsigned long long a = 0;
    *area = a;
    return RegAreaReserveInArea(size, flags, addr, area);
}

long RegAreaAttach(void               **addr,
                   unsigned long      size,
                   unsigned long long area,
                   unsigned long      flags,
                   l4_os3_dataspace_t ds,
                   unsigned long      offset,
                   unsigned char      align)
{
    ULONG rights = 0;
    int rc = NO_ERROR;

    if (flags & DATASPACE_READ)
        rights |= L4DM_READ;
    if (flags & DATASPACE_WRITE)
        rights |= L4DM_WRITE;

    rc = l4rm_area_attach(&ds.ds, (unsigned long)area, size, offset, rights, addr);

    if (rc < 0)
    {
        switch (-rc)
        {
            case L4_EINVAL:
                rc = ERROR_INVALID_DATASPACE;
                break;

            case L4_EUSED:
                rc = ERROR_ALREADY_USED;
                break;

            case L4_ENOMEM:
                rc = ERROR_NOT_ENOUGH_MEMORY;
                break;

            case L4_ENOMAP:
                rc = ERROR_NO_MAPPING;
                break;

            case L4_EIPC:
                rc = ERROR_IPC;
        }
    }

    return rc;
}

long RegAreaAttachToRegion(void               *start,
                           unsigned long      size,
                           unsigned long long area,
                           unsigned long      flags,
                           l4_os3_dataspace_t ds,
                           unsigned long      offset,
                           unsigned char      align)
{
    ULONG rights = 0;
    int rc = NO_ERROR;

    if (flags & DATASPACE_READ)
        rights |= L4DM_READ;
    if (flags & DATASPACE_WRITE)
        rights |= L4DM_WRITE;

    rc = l4rm_area_attach_to_region(&ds.ds, (unsigned long)area, start, size,
                                    offset, rights);

    if (rc < 0)
    {
        switch (-rc)
        {
            case L4_EINVAL:
                rc = ERROR_INVALID_DATASPACE;
                break;

            case L4_EUSED:
                rc = ERROR_ALREADY_USED;
                break;

            case L4_ENOMEM:
                rc = ERROR_NOT_ENOUGH_MEMORY;
                break;

            case L4_ENOMAP:
                rc = ERROR_NO_MAPPING;
                break;

            case L4_EIPC:
                rc = ERROR_IPC;
        }
    }

    return rc;
}

long RegAttachDataspaceToArea(l4_os3_dataspace_t ds,
                              unsigned long long area,
                              unsigned long      flags,
                              void               *addr)
{
    ULONG rights = 0;

    if (flags & DATASPACE_READ)
        rights |= L4DM_READ;
    if (flags & DATASPACE_WRITE)
        rights |= L4DM_WRITE;

    return attach_ds_area(ds, area, rights, (l4_addr_t)addr);
}


long RegLookupRegion(void               *addr,
                     void               **addr_new,
                     unsigned long      *size,
                     unsigned long      *offset,
                     l4_os3_dataspace_t *ds)
{
    l4_threadid_t pager;
    long rc = l4rm_lookup_region(addr, (l4_addr_t *)addr_new, (l4_size_t *)size,
                                  &ds->ds, (l4_offs_t *)&offset, &pager);

    switch (rc)
    {
       case L4RM_REGION_RESERVED:
         rc = REG_RESERVED;
         break;

       case L4RM_REGION_FREE:
         rc = REG_FREE;
         break;

       case L4RM_REGION_DATASPACE:
         rc = REG_DATASPACE;
         break;

       case -L4_ENOTFOUND:
         rc = -ERROR_FILE_NOT_FOUND;
         break;

       case -L4_EUSED:
         rc = -ERROR_ALREADY_USED;
         break;

       default:
         rc = REG_FREE;
    }

    return rc;
}

void RegDumpRegions(void)
{
  l4rm_show_region_list();
}

long attach_ds(l4_os3_dataspace_t ds, unsigned long flags, void **addr)
//long attach_ds(l4os3_ds_t *ds, unsigned long flags, void **addr)
{
  int error;
  l4_size_t size;
  ULONG rights = 0;
  APIRET rc;

  if (flags & DATASPACE_READ)
    rights |= L4DM_READ;
  if (flags & DATASPACE_WRITE)
    rights |= L4DM_WRITE;

  rc = DataspaceGetSize(ds, (unsigned long *)&size);

  //if ((size = l4os3_ds_size(ds)) < 0)
  if (rc)
    {
      //io_log("Error %d (%s) getting size of dataspace\n",
	  //error, l4os3_errtostr(error));
      return rc;
    }

    //if ((error = l4rm_attach(ds, size, 0, flags, (void **)addr)))
    //if ((error = l4os3_rm_attach((void **)addr, size, flags, *ds, 0, 0)))
    if ((error = RegAttach((void **)addr, size, rights, ds, 0, 0)))
    {
      io_log("Error %d (%s) attaching dataspace\n",
	  error, l4os3_errtostr(error));
      return error;
    }

  return 0;
}

/** attach dataspace to our address space. (concrete address) */
long attach_ds_reg(l4_os3_dataspace_t ds, unsigned long flags, void *addr)
{
  //int error;
  l4_size_t size;
  l4_addr_t a = (l4_addr_t)addr;
  ULONG rights = 0;
  APIRET rc;

  if (flags & DATASPACE_READ)
    rights |= L4DM_READ;
  if (flags & DATASPACE_WRITE)
    rights |= L4DM_WRITE;

  rc = DataspaceGetSize(ds, (unsigned long *)&size);

  /* get dataspace size */
  //if ((error = l4dm_mem_size(&ds, &size)))
  if (rc)
    {
      //io_log("Error %d (%s) getting size of dataspace\n",
	//  error, l4os3_errtostr(error));
      io_log("Error %u getting size of dataspace\n", rc);
      return rc;
    }

  /* attach it to a given region */
  //if ((error = l4rm_attach_to_region(&ds, (void *)a, size, 0, flags)))
  if ( (rc = RegAttachToRegion((void **)&a, size, rights, ds, 0, 0)) )
    {
      io_log("Error %d (%s) attaching dataspace\n", rc);
      return rc;
    }

  return 0;
}

/** attach dataspace to our address space. (concrete address) */
long attach_ds_area(l4_os3_dataspace_t ds, unsigned long long area, unsigned long flags, void *addr)
{
  int error;
  l4_size_t size;
  ULONG rights = 0;
  APIRET rc;

  if (flags & DATASPACE_READ)
    rights |= L4DM_READ;
  if (flags & DATASPACE_WRITE)
    rights |= L4DM_WRITE;

  rc = DataspaceGetSize(ds, (unsigned long *)&size);

  /* get dataspace size */
  //if ((error = l4dm_mem_size(&ds, &size)))
  if (rc)
    {
      //io_log("Error %d (%s) getting size of dataspace\n",
      //  error, l4os3_errtostr(error));
      io_log("Error %u getting size of dataspace\n", rc);
      return rc;
    }

  /* attach it to a given region */
  //if ( (error = l4rm_area_attach_to_region(&ds, area,
    //                   (void *)a, size, 0, rights)) )
  if ( (rc = RegAreaAttachToRegion(addr, size, area,
                                   flags, ds, 0, 0)) )
    {
      io_log("Error %d (%s) attaching dataspace\n", rc);
      return rc;
    }

  return 0;
}
