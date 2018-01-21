/* dataspace interface */

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/dataspace.h>
#include <os3/rm.h>
#include <os3/err.h>
#include <os3/io.h>

/* Genode includes */
#include <dataspace/capability.h>
#include <dataspace/client.h>

/* libc includes */
#include <stdlib.h>

/* local includes */
#include "genode_env.h"

extern "C"
long DataspaceAlloc(l4_os3_dataspace_t *ds, ULONG flags,
                    l4_os3_cap_idx_t dm, ULONG size)
{
    Genode::Env &_env = genode_env();
    Genode::Dataspace_capability _ds;

    try
    {
        _ds = _env.ram().alloc(size);
    }
    catch (...)
    {
        return ERROR_NOT_ENOUGH_MEMORY;
    }

    *ds = (l4_os3_dataspace_t)&_ds;
    return NO_ERROR;
}

extern "C"
long DataspaceFree(l4_os3_dataspace_t ds)
{
    Genode::Env &_env = genode_env();
    Genode::Dataspace_capability _ds;

    if (! ds)
    {
        return ERROR_INVALID_PARAMETER;
    }

    _ds = *(Genode::Dataspace_capability *)ds;

    if (! _ds.valid())
    {
        return ERROR_INVALID_PARAMETER;
    }

    _env.ram().free(Genode::static_cap_cast<Genode::Ram_dataspace>(_ds));
    return NO_ERROR;
}

extern "C"
long DataspaceGetSize(l4_os3_dataspace_t ds, ULONG *size)
{
    Genode::Dataspace_capability _ds;

    if (! ds || ! size)
    {
        return ERROR_INVALID_PARAMETER;
    }

    _ds = *(Genode::Dataspace_capability *)ds;

    if (! _ds.valid())
    {
        return ERROR_INVALID_PARAMETER;
    }

    *size = Genode::Dataspace_client(_ds).size();
    return NO_ERROR;
}

extern "C"
long DataspaceShare(l4_os3_dataspace_t ds,
                    l4_os3_cap_idx_t   client_id,
                    unsigned long      rights)
{
    /* nothing to do on Genode, because dataspace is shared
       automagically, when delegating the capability */
    return 0;
}

extern "C"
long attach_ds(l4_os3_dataspace_t ds, unsigned long flags, void **addr)
//long attach_ds(l4os3_ds_t *ds, unsigned long flags, void **addr)
{
    //int error;
    ULONG size;
    APIRET rc;

    rc = DataspaceGetSize(ds, &size);

    //if ((size = l4os3_ds_size(ds)) < 0)
    if (rc)
    {
      io_log("Error %u getting size of dataspace\n", rc);
      return rc;
    }

    //if ((error = l4rm_attach(ds, size, 0, flags, (void **)addr)))
    //if ((error = l4os3_rm_attach((void **)addr, size, flags, *ds, 0, 0)))
    if ((rc = RegAttach((void **)addr, size, flags, ds, 0, 0)))
    {
      io_log("Error %u attaching dataspace\n", rc);
      return rc;
    }

  return 0;
}


/** attach dataspace to our address space. (concrete address) */
extern "C"
long attach_ds_reg(l4_os3_dataspace_t ds, unsigned long flags, void *addr)
{
    //int error;
    ULONG size;
    void *a = addr;
    APIRET rc;

    rc = DataspaceGetSize(ds, &size);

    /* get dataspace size */
    //if ((error = l4dm_mem_size(&ds, &size)))
    if (rc)
    {
      io_log("Error %u getting size of dataspace\n", rc);
      return rc;
    }

    /* attach it to a given region */
    //if ((error = l4rm_attach_to_region(&ds, (void *)a, size, 0, flags)))
    if ( (rc = RegAttachToRegion((void **)&a, size, flags, ds, 0, 0)) )
    {
      io_log("Error %u attaching dataspace\n", rc);
      return rc;
    }

  return 0;
}

/** attach dataspace to our address space. (concrete address) */
extern "C"
long attach_ds_area(l4_os3_dataspace_t ds, unsigned long long area,
                    unsigned long flags, void *addr)
{
    //int error;
    ULONG size;
    void *a = addr;
    APIRET rc;

    rc = DataspaceGetSize(ds, &size);

    /* get dataspace size */
    //if ((error = l4dm_mem_size(&ds, &size)))
    if (rc)
    {
      io_log("Error %u getting size of dataspace\n", rc);
      return rc;
    }

    /* attach it to a given region */
    //if ( (error = l4rm_area_attach_to_region(&ds, area,
    //                   (void *)a, size, 0, flags)) )
    if ( (rc = RegAreaAttachToRegion((void **)&a, size, area,
                                      flags, ds, 0, 0)) )
    {
      io_log("Error %u attaching dataspace\n", rc);
      return rc;
    }

  return 0;
}
