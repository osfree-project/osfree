/* os2srv client-side RPC API (Genode platform) */

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* Genode includes */
#include <base/allocator.h>
#include <cpi_session/connection.h>

/* local includes */
#include "genode_env.h"

OS2::Cpi::Connection *cpi;

Genode::Allocator *_alloc = NULL;
Genode::Env *_env_ptr = NULL;

extern "C"
APIRET CPClientInit(void)
{
    Genode::Allocator &alloc = genode_alloc();
    Genode::Env &env = genode_env();

    try
    {
        cpi = new (alloc) OS2::Cpi::Connection(env);
    }
    catch (...)
    {
        return ERROR_FILE_NOT_FOUND;
    }

    return NO_ERROR;
}

extern "C"
APIRET CPClientDone(void)
{
    Genode::Allocator &alloc = genode_alloc();

    destroy(alloc, cpi);
    return NO_ERROR;
}
