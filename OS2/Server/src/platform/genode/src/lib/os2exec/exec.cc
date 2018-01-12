/* os2exec client RPC API (Genode platform) */

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* Genode includes */
#include <base/allocator.h>
#include <exec_session/connection.h>

/* local includes */
#include "genode_env.h"

OS2::Exec::Connection *exec;

Genode::Allocator *_alloc = NULL;
Genode::Env *_env_ptr = NULL;

extern "C"
APIRET ExcClientInit(void)
{
    Genode::Allocator &alloc = genode_alloc();
    Genode::Env &env = genode_env();

    try
    {
        exec = new (alloc) OS2::Exec::Connection(env);
    }
    catch (...)
    {
        return ERROR_FILE_NOT_FOUND;
    }

    return NO_ERROR;
}

extern "C"
APIRET ExcClientDone(void)
{
    Genode::Allocator &alloc = genode_alloc();

    destroy(alloc, exec);
    return NO_ERROR;
}

extern "C"
APIRET ExcClientOpen(PSZ pszFileName,
                     ULONG ulFlags,
                     PBYTE pLoadError,
                     ULONG *pcbLoadError,
                     HMODULE *phmod)
{
    OS2::Exec::Session::Pathname fName(pszFileName);
    Genode::Ram_dataspace_capability err_ds;
    Genode::Env &env = genode_env();
    char *addr;
    APIRET rc;

    err_ds = env.ram().alloc(*pcbLoadError);
    addr = env.rm().attach(err_ds);

    rc = exec->open(fName, ulFlags, err_ds, pcbLoadError, phmod);

    Genode::memcpy(pLoadError, addr, *pcbLoadError);
    env.rm().detach(addr);
    env.ram().free(err_ds);
    return rc;
}

extern "C"
APIRET ExcClientLoad(ULONG hmod,
                     PBYTE pLoadError,
                     ULONG *pcbLoadError,
                     os2exec_module_t *s)
{
    Genode::Ram_dataspace_capability err_ds, mod_ds;
    Genode::Env &env = genode_env();
    char *addr, *addr2;
    APIRET rc;

    err_ds = env.ram().alloc(*pcbLoadError);
    addr = env.rm().attach(err_ds);
    mod_ds = env.ram().alloc(sizeof(os2exec_module_t));
    addr2 = env.rm().attach(mod_ds);

    rc = exec->load(hmod, err_ds, pcbLoadError, mod_ds);

    Genode::memcpy(pLoadError, addr, *pcbLoadError);
    Genode::memcpy(s, addr2, sizeof(os2exec_module_t));
    env.rm().detach(addr);
    env.ram().free(err_ds);
    env.rm().detach(addr2);
    env.ram().free(mod_ds);
    return rc;
}

extern "C"
APIRET ExcClientShare(HMODULE hmod)
{
    return exec->share(hmod);
}

extern "C"
APIRET ExcClientGetImp(HMODULE hmod,
                       ULONG *index,
                       HMODULE *imp_hmod)
{
    return exec->getimp(hmod, index, imp_hmod);
}

extern "C"
APIRET ExcClientGetSect(HMODULE hmod,
                        ULONG *index,
                        l4exec_section_t *sect)
{
    Genode::Ram_dataspace_capability sect_ds;
    Genode::Env &env = genode_env();
    char *addr;
    APIRET rc;

    sect_ds = env.ram().alloc(sizeof(l4exec_section_t));
    addr = env.rm().attach(sect_ds);

    rc = exec->getsect(hmod, index, sect_ds);

    Genode::memcpy(sect, addr, sizeof(l4exec_section_t));
    env.rm().detach(addr);
    env.ram().free(sect_ds);
    return rc;
}

extern "C"
APIRET ExcClientQueryProcAddr(HMODULE hmod,
                              ULONG ordinal,
                              PSZ pszModname,
                              void **ppfn)
{
    OS2::Exec::Session::Pathname mName(pszModname);

    return exec->query_procaddr(hmod, ordinal,
                                mName, (ULONGLONG *)ppfn);
}

extern "C"
APIRET ExcClientQueryModuleHandle(PSZ pszModname,
                                  HMODULE *hmod)
{
    OS2::Exec::Session::Pathname mName(pszModname);

    return exec->query_modhandle(mName, hmod);
}

extern "C"
APIRET ExcClientQueryModuleName(HMODULE hmod,
                                ULONG cbName,
                                PBYTE pbName)
{
    Genode::Ram_dataspace_capability ds;
    Genode::Env &env = genode_env();
    char *addr;
    APIRET rc;

    ds = env.ram().alloc(cbName);
    addr = env.rm().attach(ds);

    rc = exec->query_modname(hmod, ds);

    Genode::memcpy(pbName, addr, cbName);
    env.rm().detach(addr);
    env.ram().free(ds);
    return rc;
}

extern "C"
APIRET ExcClientAllocSharedMem(ULONG cbSize,
                               PSZ pszName,
                               ULONG rights,
                               void **addr,
                               ULONGLONG *area)
{
    OS2::Exec::Session::Pathname mName(pszName);

    return exec->alloc_sharemem(cbSize, mName, rights,
                                (ULONGLONG *)addr, area);
}

extern "C"
APIRET ExcClientMapDataspace(void *addr,
                             ULONG rights,
                             l4_os3_dataspace_t ds)
{
    Genode::Ram_dataspace_capability native_ds;
    native_ds = *(Genode::Ram_dataspace_capability *)ds;

    return exec->map_dataspace((ULONGLONG)addr, rights, native_ds);
}

extern "C"
APIRET ExcClientUnmapDataspace(void *addr,
                               l4_os3_dataspace_t ds)
{
    Genode::Ram_dataspace_capability native_ds;
    native_ds = *(Genode::Ram_dataspace_capability *)ds;

    return exec->unmap_dataspace((ULONGLONG)addr, native_ds);
}

extern "C"
APIRET ExcClientGetDataspace(void **addr,
                             ULONG *size,
                             l4_os3_dataspace_t *ds)
{
    return exec->get_dataspace((ULONGLONG *)addr, size,
                               (Genode::Ram_dataspace_capability *)*ds);
}

extern "C"
APIRET ExcClientGetSharedMem(void *pb,
                             void **addr,
                             ULONG *size,
                             l4_os3_cap_idx_t *owner)
{
    return exec->get_sharemem((ULONGLONG)pb, (ULONGLONG *)addr,
                              size, (ULONGLONG *)owner);
}

extern "C"
APIRET ExcClientGetNamedSharedMem(PSZ pszName,
                                  void **addr,
                                  ULONG *size,
                                  l4_os3_cap_idx_t *owner)
{
    OS2::Exec::Session::Pathname mName(pszName);

    return exec->get_named_sharemem(mName, (ULONGLONG *)addr,
                                    size, (ULONGLONG *)owner);
}

extern "C"
APIRET ExcClientIncrementSharedMemRefcnt(void *addr)
{
    return exec->increment_sharemem_refcnt((ULONGLONG)addr);
}

extern "C"
APIRET ExcClientReleaseSharedMem(void *addr,
                                 ULONG *count)
{
    return exec->release_sharemem((ULONGLONG)addr, count);
}
