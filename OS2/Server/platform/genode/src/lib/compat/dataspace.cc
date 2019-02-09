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
    static Genode::Ram_dataspace_capability _ds;

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
    Genode::Ram_dataspace_capability _ds;

    if (! ds || ! size)
    {
        return ERROR_INVALID_PARAMETER;
    }

    _ds = *(Genode::Ram_dataspace_capability *)ds;

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
long DataspaceCopy(l4_os3_dataspace_t ds, ULONG src_offs, ULONG dst_offs,
                   ULONG size, ULONG dst_addr, ULONG dst_size,
                   ULONG flags, l4_os3_dataspace_t *copy)
{
    l4_os3_dataspace_t temp_ds;
    void *addr1, *addr2;
    int rc;

    // Allocate new dataspace of the same size
    rc = DataspaceAlloc(&temp_ds, flags, NULL, size);

    if (rc)
    {
        return rc;
    }

    rc = RegAttach(&addr1, size, flags, ds, NULL, 0);

    if (rc)
    {
        return rc;
    }

    rc = RegAttach(&addr2, size, flags, temp_ds, NULL, 0);

    if (rc)
    {
        return rc;
    }

    Genode::memcpy(addr2, addr1, size);

    RegDetach(addr2);
    RegDetach(addr1);
    return 0;
}
