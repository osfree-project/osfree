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

#if defined(L4API_l4v2)

/* l4env includes */
#include <l4/sys/types.h>
#include <l4/env/env.h>
#include <l4/dm_mem/dm_mem.h>

long
DataspaceAlloc(l4_os3_dataspace_t *ds, void *offset, unsigned long size)
{
  l4dm_dataspace_t *temp_ds = (l4dm_dataspace_t *)malloc(sizeof(l4dm_dataspace_t));
  long ret = l4dm_mem_open(l4env_get_default_dsm(), size, 0, 0, "", temp_ds);
  *ds = temp_ds;
  return ret;
}

long DataspaceFree(l4_os3_dataspace_t ds)
{
  long ret = l4dm_close((l4dm_dataspace_t *)ds);
  free(ds);
  return ret;
}

long DataspaceGetSize(l4_os3_dataspace_t ds)
{
  l4_size_t size;
  int error;

  if ((error = l4dm_mem_size((l4dm_dataspace_t *)ds, &size)))
    return error;
  else
    return size;
}

long DataspaceShare(l4_os3_dataspace_t ds,
                    void               *client_id,
                    unsigned long      rights)
{
  APIRET rc = NO_ERROR;
  long   ret;

  if ( (ret = l4dm_share(ds, *(l4_threadid_t *)client_id, rights)) < 0 )
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

#elif defined(L4API_l4f)

// implementation

long
DataspaceAlloc(l4_os3_dataspace_t *ds, void *offset, unsigned long size)
{
  l4re_ds_t *temp_ds = (l4re_ds_t *)malloc(sizeof(l4re_ds_t));
  long ret = l4re_ds_allocate(temp_ds, offset, size);
  *ds = temp_ds;
  return ret;
}

long DataspaceFree(l4_os3_dataspace_t ds)
{
  return 0; // not implemented!
}

long DataspaceGetSize(l4_os3_dataspace_t ds)
{
  return l4re_ds_size(*(l4re_ds_t *)ds);
}

#else
#error "Not implemented!"
#endif

long attach_ds(l4_os3_dataspace_t ds, unsigned long flags, void **addr)
{
  int error;
  l4_size_t size;

  //if ((size = l4os3_ds_size(ds)) < 0)
  if ((size = DataspaceGetSize(ds)) < 0)
    {
      io_log("Error %d (%s) getting size of dataspace\n",
	  error, l4os3_errtostr(error));
      return error;
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
  int error;
  l4_size_t size;
  l4_addr_t a = (l4_addr_t)addr;

  /* get dataspace size */
  //if ((error = l4dm_mem_size(&ds, &size)))
  if ((size = DataspaceGetSize(ds)) < 0)
    {
      io_log("Error %d (%s) getting size of dataspace\n",
	  error, l4os3_errtostr(error));
      return error;
    }

  /* attach it to a given region */
  //if ((error = l4rm_attach_to_region(&ds, (void *)a, size, 0, flags)))
  if ( (error = RegAttachToRegion((void **)&a, size, flags, ds, 0, 0)) < 0 )
    {
      io_log("Error %d (%s) attaching dataspace\n",
	  error, l4os3_errtostr(error));
      return error;
    }

  return 0;
}

/** attach dataspace to our address space. (concrete address) */
long attach_ds_area(l4_os3_dataspace_t ds, unsigned long area, unsigned long flags, void *addr)
{
  int error;
  l4_size_t size;
  l4_addr_t a = (l4_addr_t)addr;

  /* get dataspace size */
  //if ((error = l4dm_mem_size(&ds, &size)))
  if ((size = DataspaceGetSize(ds)) < 0)
    {
      io_log("Error %d (%s) getting size of dataspace\n",
	  error, l4os3_errtostr(error));
      return error;
    }

  /* attach it to a given region */
  //if ( (error = l4rm_area_attach_to_region(&ds, area,
    //                   (void *)a, size, 0, flags)) )
  if ( (error = RegAreaAttachToRegion((void **)&a, size, area,
                                      flags, ds, 0, 0)) < 0 )
    {
      io_log("Error %d (%s) attaching dataspace\n",
	  error, l4os3_errtostr(error));
      return error;
    }

  return 0;
}
