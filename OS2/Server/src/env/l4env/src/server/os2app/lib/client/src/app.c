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

APIRET AppClientGetLoadError(ULONG  ulClient,
                             PBYTE  szLoadError,
                             PULONG pcbLoadError)
{
    CORBA_Environment env = dice_default_environment;
    l4_threadid_t os2app = *(l4_threadid_t *)ulClient;
    APIRET rc;

    os2app_app_GetLoadError_call(&os2app, &szLoadError,
                                 pcbLoadError, &rc, &env);
    return rc;
}

APIRET AppClientTerminate(ULONG ulClient)
{
    CORBA_Environment env = dice_default_environment;
    l4_threadid_t os2app = *(l4_threadid_t *)ulClient;

    os2app_app_Terminate_call(&os2app, &env);
    return 0;
}

APIRET AppClientAreaAdd(ULONG ulClient,
                        PVOID pAddr,
                        ULONG ulSize,
                        ULONG ulFlags)
{
    CORBA_Environment env = dice_default_environment;
    l4_threadid_t os2app = *(l4_threadid_t *)ulClient;
    l4_addr_t addr = (l4_addr_t)pAddr;
    l4_size_t size = (l4_size_t)ulSize;
    l4_uint32_t flags = (l4_uint32_t)ulFlags;
    long   ret;

    ret = os2app_app_AddArea_call(&os2app, addr, size, flags, &env);
    return (APIRET)ret;
}

APIRET AppClientDataspaceAttach(ULONG              ulClient,
                                PVOID              pAddr,
                                l4_os3_dataspace_t ds,
                                ULONG              ulRights)
{
    CORBA_Environment env = dice_default_environment;
    l4_threadid_t os2app = *(l4_threadid_t *)ulClient;
    l4_addr_t addr = (l4_addr_t)pAddr;
    l4dm_dataspace_t *ds2 = (l4dm_dataspace_t *)ds;
    l4_uint32_t rights = (l4_uint32_t)ulRights;
    long ret;

    ret = os2app_app_AttachDataspace_call(&os2app, addr, ds2, rights, &env);
    return (APIRET)ret;
}

APIRET AppClientDataspaceRelease(ULONG ulClient,
                                 l4_os3_dataspace_t ds)
{
    CORBA_Environment env = dice_default_environment;
    l4_threadid_t os2app = *(l4_threadid_t *)ulClient;
    l4dm_dataspace_t *ds2 = (l4dm_dataspace_t *)ds;
    long ret;

    ret = os2app_app_ReleaseDataspace_call(&os2app, ds2, &env);
    return (APIRET)ret;
}
