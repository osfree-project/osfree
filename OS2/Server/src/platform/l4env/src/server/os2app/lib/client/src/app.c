/* os2app client-side API */

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/dataspace.h>
#include <os3/app.h>

/* l4env includes */
#include <l4/sys/types.h>

/* os2app RPC */
#include <l4/os2app/os2app-client.h>

APIRET AppClientGetLoadError(l4_os3_cap_idx_t client_id,
                             PBYTE  szLoadError,
                             PULONG pcbLoadError)
{
    CORBA_Environment env = dice_default_environment;
    l4_threadid_t os2app = client_id.thread;
    APIRET rc;

    os2app_app_GetLoadError_call(&os2app, &szLoadError,
                                 pcbLoadError, &rc, &env);
    return rc;
}

APIRET AppClientTerminate(l4_os3_cap_idx_t client_id)
{
    CORBA_Environment env = dice_default_environment;
    l4_threadid_t os2app = client_id.thread;

    os2app_app_Terminate_call(&os2app, &env);
    return 0;
}

APIRET AppClientAreaAdd(l4_os3_cap_idx_t client_id,
                        PVOID pAddr,
                        ULONG ulSize,
                        ULONG ulFlags)
{
    CORBA_Environment env = dice_default_environment;
    l4_threadid_t os2app = client_id.thread;
    l4_addr_t addr = (l4_addr_t)pAddr;
    l4_size_t size = (l4_size_t)ulSize;
    l4_uint32_t flags = (l4_uint32_t)ulFlags;
    long   ret;

    ret = os2app_app_AddArea_call(&os2app, addr, size, flags, &env);
    return (APIRET)ret;
}

APIRET AppClientDataspaceAttach(l4_os3_cap_idx_t   client_id,
                                PVOID              pAddr,
                                l4_os3_dataspace_t ds,
                                ULONG              ulRights)
{
    CORBA_Environment env = dice_default_environment;
    l4_threadid_t os2app = client_id.thread;
    l4_addr_t addr = (l4_addr_t)pAddr;
    l4_uint32_t rights = (l4_uint32_t)ulRights;
    long ret;

    ret = os2app_app_AttachDataspace_call(&os2app, addr, &ds.ds, rights, &env);
    return (APIRET)ret;
}

APIRET AppClientDataspaceRelease(l4_os3_cap_idx_t   client_id,
                                 l4_os3_dataspace_t ds)
{
    CORBA_Environment env = dice_default_environment;
    l4_threadid_t os2app = client_id.thread;
    long ret;

    ret = os2app_app_ReleaseDataspace_call(&os2app, &ds.ds, &env);
    return (APIRET)ret;
}
