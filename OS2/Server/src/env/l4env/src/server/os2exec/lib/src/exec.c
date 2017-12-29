/* os2exec client-side RPC API */

/* osFree internal */
#include <os3/io.h>
#include <os3/types.h>
#include <os3/exec.h>

/* l4env includes */
#include <l4/names/libnames.h>

/* os2exec RPC includes */
#include <l4/os2exec/os2exec-client.h>

/* os2exec client interface cap */
extern l4_os3_cap_idx_t execsrv;

APIRET ExcClientInit(void)
{
    if (! names_waitfor_name("os2exec", &execsrv, 30000) )
    {
        io_log("os2exec not found on the name server!\n");
        return ERROR_FILE_NOT_FOUND; // @todo more appropriate error code
    }

    return NO_ERROR;
}

APIRET ExcClientOpen(PSZ                pszFileName,
                     ULONG              ulFlags,
                     PBYTE              pbLoadError,
                     PULONG             pcbLoadError,
                     PHMODULE           phmod)
{
    CORBA_Environment env = dice_default_environment;
    l4dm_dataspace_t ds;
    long ret;

    ret = os2exec_open_call(&execsrv, (char *)pszFileName, &ds,
                            ulFlags, (char **)&pbLoadError, pcbLoadError, phmod, &env);
    return (APIRET)ret;
}

APIRET ExcClientLoad(HMODULE          hmod,
                     PBYTE            pbLoadError,
                     PULONG           pcbLoadError,
                     os2exec_module_t *s)
{
    CORBA_Environment env = dice_default_environment;
    long ret;

    ret = os2exec_load_call(&execsrv, hmod, (char **)&pbLoadError,
                            pcbLoadError, s, &env);
    return (APIRET)ret;
}

APIRET ExcClientShare(HMODULE hmod)
{
    CORBA_Environment env = dice_default_environment;
    long ret;

    ret = os2exec_share_call(&execsrv, hmod, &env);
    return (APIRET)ret;
}

APIRET ExcClientGetImp(HMODULE hmod,
                       ULONG *index,
                       HMODULE *imp_hmod)
{
    CORBA_Environment env = dice_default_environment;
    long ret;

    ret = os2exec_getimp_call(&execsrv, hmod, index, imp_hmod, &env);
    return (APIRET)ret;
}

APIRET ExcClientGetSect(HMODULE hmod,
                        ULONG *index,
                        l4exec_section_t *sect)
{
    CORBA_Environment env = dice_default_environment;
    long ret;

    ret = os2exec_getsect_call(&execsrv, hmod, index, sect, &env);
    return (APIRET)ret;
}

APIRET ExcClientQueryProcAddr(HMODULE hmod,
                              ULONG ordinal,
                              PSZ pszModname,
                              void **ppfn)
{
    CORBA_Environment env = dice_default_environment;
    long ret;

    ret = os2exec_query_procaddr_call(&execsrv, hmod, ordinal,
                                      pszModname, (l4_addr_t *)ppfn, &env);
    return (APIRET)ret;
}

APIRET ExcClientQueryModuleHandle(PSZ pszModname,
                                  HMODULE *phmod)
{
    CORBA_Environment env = dice_default_environment;
    long ret;

    ret = os2exec_query_modhandle_call(&execsrv, pszModname, phmod, &env);
    return (APIRET)ret;
}

APIRET ExcClientQueryModuleName(HMODULE hmod,
                                ULONG cbBuf,
                                PBYTE pBuf)
{
    CORBA_Environment env = dice_default_environment;
    long ret;

    ret = os2exec_query_modname_call(&execsrv, hmod, cbBuf, &pBuf, &env);
    return (APIRET)ret;
}

APIRET ExcClientAllocSharedMem(ULONG size,
                               PSZ pszName,
                               ULONG rights,
                               void **addr,
                               ULONG *area)
{
    CORBA_Environment env = dice_default_environment;
    long ret;

    ret = os2exec_alloc_sharemem_call(&execsrv, size, pszName,
                                      rights, (l4_addr_t *)addr, (l4_uint32_t *)area, &env);
    return (APIRET)ret;
}

APIRET ExcClientMapDataspace(void *addr,
                             ULONG rights,
                             l4_os3_dataspace_t ds)
{
    CORBA_Environment env = dice_default_environment;
    long ret;

    ret = os2exec_map_dataspace_call(&execsrv, (l4_addr_t)addr, (l4_uint32_t)rights,
                                     (l4dm_dataspace_t *)ds, &env);
    return (APIRET)ret;
}

APIRET ExcClientUnmapDataspace(void *addr,
                               l4_os3_dataspace_t ds)
{
    CORBA_Environment env = dice_default_environment;
    long ret;

    ret = os2exec_unmap_dataspace_call(&execsrv, (l4_addr_t)addr, (l4dm_dataspace_t *)ds, &env);
    return (APIRET)ret;
}

APIRET ExcClientGetDataspace(void **addr,
                             ULONG *size,
                             l4_os3_dataspace_t *ds)
{
    CORBA_Environment env = dice_default_environment;
    long ret;

    ret = os2exec_get_dataspace_call(&execsrv, (l4_addr_t *)addr,
                                     (l4_size_t *)size, (l4dm_dataspace_t *)*ds, &env);
    return (APIRET)ret;
}

APIRET ExcClientGetSharedMem(void *pb,
                             void **addr,
                             ULONG *size,
                             l4_os3_cap_idx_t *owner)
{
    CORBA_Environment env = dice_default_environment;
    long ret;

    ret = os2exec_get_sharemem_call(&execsrv, (l4_addr_t)pb,
                                    (l4_addr_t *)addr, (l4_size_t *)size,
                                    (l4_threadid_t *)owner, &env);
    return (APIRET)ret;
}

APIRET ExcClientGetNamedSharedMem(PSZ pszName,
                                  void **addr,
                                  ULONG *size,
                                  l4_os3_cap_idx_t *owner)
{
    CORBA_Environment env = dice_default_environment;
    long ret;

    ret = os2exec_get_namedsharemem_call(&execsrv, pszName, (l4_addr_t *)addr,
                                         (l4_size_t *)size, (l4_threadid_t *)owner, &env);
    return (APIRET)ret;
}

APIRET ExcClientIncrementSharedMemRefcnt(void *addr)
{
    CORBA_Environment env = dice_default_environment;
    long ret;

    ret = os2exec_increment_sharemem_refcnt_call(&execsrv, (l4_addr_t)addr, &env);
    return (APIRET)ret;
}

APIRET ExcClientReleaseSharedMem(void *addr,
                                 ULONG *count)
{
    CORBA_Environment env = dice_default_environment;
    long ret;

    ret = os2exec_release_sharemem_call(&execsrv, (l4_addr_t)addr,
                                        (l4_uint32_t *)count, &env);
    return (APIRET)ret;
}
