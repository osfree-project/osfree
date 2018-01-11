/* dataspace interface */

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/dataspace.h>
#include <os3/rm.h>
#include <os3/err.h>
#include <os3/io.h>

/* libc includes */
#include <stdlib.h>

#if defined(__l4env__)

/* l4env includes */
#include <l4/sys/types.h>
#include <l4/env/env.h>
#include <l4/dm_mem/dm_mem.h>

long
l4os3_ds_allocate(l4os3_ds_t *ds, l4_addr_t offset, l4_size_t size)
{
  return l4dm_mem_open(l4env_get_default_dsm(), size, 0, 0, "", ds);
}

long DataspaceAlloc(l4_os3_dataspace_t *ds, ULONG flags,
                    l4_os3_cap_idx_t dm, ULONG size)
{
  //l4dm_dataspace_t *temp_ds = (l4dm_dataspace_t *)malloc(sizeof(l4dm_dataspace_t));

  if (l4_is_invalid_id(dm.thread))
    dm.thread = l4env_get_default_dsm();

  long ret = l4dm_mem_open(dm.thread, size, 4096, flags, "", &ds->ds);
  //*ds = temp_ds;

  return ret;
}

long DataspaceFree(l4_os3_dataspace_t ds)
{
  long ret = l4dm_close(&ds.ds);
  //free(ds);
  return ret;
}

long DataspaceGetSize(l4_os3_dataspace_t ds, unsigned long *size)
{
  //l4_size_t size;
  int error;

  if ((error = l4dm_mem_size(&ds.ds, (l4_size_t *)size)))
    return ERROR_INVALID_PARAMETER;
  else
    return NO_ERROR;
}

long DataspaceShare(l4_os3_dataspace_t ds,
                    l4_os3_cap_idx_t   client_id,
                    unsigned long      rights)
{
  APIRET rc = NO_ERROR;
  long   ret;

  if ( (ret = l4dm_share(&ds.ds, client_id.thread, rights)) < 0 )
  {
    switch (-ret)
    {
      case L4_EINVAL:
        rc = ERROR_FILE_NOT_FOUND;
        break;

      case L4_EPERM:
        rc = ERROR_ACCESS_DENIED;
        break;

      default:
        rc = ERROR_INVALID_PARAMETER;
    }
  }

  return rc;
}

#elif defined(__l4re__)

// implementation

long
DataspaceAlloc(l4_os3_dataspace_t *ds, void *offset,
               l4_os3_cap_idx_t dm, unsigned long size)
{
  //l4re_ds_t *temp_ds = (l4re_ds_t *)malloc(sizeof(l4re_ds_t));
  long ret = l4re_ds_allocate(ds, offset, size);
  //*ds = temp_ds;
  return ret;
}

long DataspaceFree(l4_os3_dataspace_t ds)
{
  return 0; // not implemented!
}

long DataspaceGetSize(l4_os3_dataspace_t ds, unsigned long *size)
{
  if (! ds || ! size)
      return ERROR_INVALID_PARAMETER;

  *size =  l4re_ds_size((l4re_ds_t)ds);
  return NO_ERROR;
}

#else
#error "Not implemented!"
#endif

long attach_ds(l4_os3_dataspace_t ds, unsigned long flags, void **addr)
//long attach_ds(l4os3_ds_t *ds, unsigned long flags, void **addr)
{
  int error;
  l4_size_t size;
  APIRET rc;

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
    if ((error = RegAttach((void **)addr, size, flags, ds, 0, 0)))
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
  APIRET rc;

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
  if ( (rc = RegAttachToRegion((void **)&a, size, flags, ds, 0, 0)) < 0 )
    {
      io_log("Error %d (%s) attaching dataspace\n", rc);
      return rc;
    }

  return 0;
}

/** attach dataspace to our address space. (concrete address) */
long attach_ds_area(l4_os3_dataspace_t ds, unsigned long long area, unsigned long flags, void *addr)
{
  //int error;
  l4_size_t size;
  l4_addr_t a = (l4_addr_t)addr;
  APIRET rc;

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
    //                   (void *)a, size, 0, flags)) )
  if ( (rc = RegAreaAttachToRegion((void **)&a, size, area,
                                      flags, ds, 0, 0)) < 0 )
    {
      io_log("Error %d (%s) attaching dataspace\n", rc);
      return rc;
    }

  return 0;
}
