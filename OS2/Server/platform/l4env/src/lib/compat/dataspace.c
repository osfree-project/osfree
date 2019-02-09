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
#include <l4/dm_phys/dm_phys.h>

long
l4os3_ds_allocate(l4os3_ds_t *ds, l4_addr_t offset, l4_size_t size)
{
  return l4dm_mem_open(l4env_get_default_dsm(), size, 0, 0, "", ds);
}

long DataspaceAlloc(l4_os3_dataspace_t *ds, ULONG flags,
                    l4_os3_cap_idx_t dm, ULONG size)
{
  //l4dm_dataspace_t *temp_ds = (l4dm_dataspace_t *)malloc(sizeof(l4dm_dataspace_t));
  ULONG rights = 0;

  if (flags & DATASPACE_READ)
    rights |= L4DM_READ;
  if (flags & DATASPACE_WRITE)
    rights |= L4DM_WRITE;

  if (l4_is_invalid_id(dm.thread))
    dm.thread = l4env_get_default_dsm();

  long ret = l4dm_mem_open(dm.thread, size, 4096, rights, "", &ds->ds);
  //*ds = temp_ds;

  if (ret < 0)
  {
    return ERROR_NOT_ENOUGH_MEMORY;
  }

  return NO_ERROR;
}

long DataspaceFree(l4_os3_dataspace_t ds)
{
  l4dm_close(&ds.ds);
  //free(ds);
  return NO_ERROR;
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
                    unsigned long      flags)
{
  APIRET rc = NO_ERROR;
  ULONG rights = 0;
  long   ret;

  if (flags & DATASPACE_READ)
    rights |= L4DM_READ;
  if (flags & DATASPACE_WRITE)
    rights |= L4DM_WRITE;

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

long DataspaceCopy(l4_os3_dataspace_t ds, ULONG src_offs, ULONG dst_offs,
                   ULONG size, ULONG dst_addr, ULONG dst_size, 
                   ULONG flags, l4_os3_dataspace_t *copy)
{
    int rc = l4dm_memphys_copy(&ds.ds, src_offs, dst_offs, size, L4DM_MEMPHYS_DEFAULT,
                               dst_addr ? dst_addr : L4DM_MEMPHYS_ANY_ADDR, dst_size,
                               4096, flags, "OS/2 personality dataspace", &copy->ds);

    if (rc < 0)
    {
        switch (rc)
        {
            case L4_EIPC:
                rc = ERROR_IPC;
                break;

            case L4_EINVAL:
                rc = ERROR_INVALID_DATASPACE;
                break;

            case L4_EPERM:
                rc = ERROR_ACCESS_DENIED;
                break;

            case L4_ENOHANDLE:
                rc = ERROR_INVALID_HANDLE;
                break;

            case L4_ENOMEM:
                rc = ERROR_NOT_ENOUGH_MEMORY;
                break;

            default:
                rc = ERROR_GEN_FAILURE;
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
