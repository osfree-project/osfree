/* os2exec client-side RPC API (Genode platform) */

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/thread.h>
#include <os3/io.h>

/* libc includes */
#include <string.h>

/* Genode includes */
#include <base/allocator.h>
#include <base/attached_dataspace.h>

#include <cpi_session/connection.h>

/* local includes */
#include <genode_env.h>

using namespace OS2::Cpi;

static Connection *exec;

static Sysio *_sysio = NULL;

extern "C"
APIRET ExcClientInit(l4_os3_thread_t *thread)
{
    Genode::Allocator &alloc = genode_alloc();
    Genode::Env &env = genode_env();
    Genode::Dataspace_capability _ds;

    try
    {
        exec = new (alloc) OS2::Cpi::Connection(env, "exec");
    }
    catch (...)
    {
        return ERROR_FILE_NOT_FOUND;
    }

    _ds = exec->sysio_dataspace();

    if (!_ds.valid())
    {
        return ERROR_INVALID_DATASPACE;
    }

    try
    {
        static Genode::Attached_dataspace _sysio_ds(env.rm(), _ds);
        _sysio = _sysio_ds.local_addr<Sysio>();
    }
    catch (Genode::Region_map::Invalid_dataspace)
    {
        return ERROR_INVALID_DATASPACE;
    }
    catch (Genode::Region_map::Region_conflict)
    {
        return ERROR_NO_MAPPING;
    }

    *thread = exec;

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
    Genode::Ram_dataspace_capability err_ds;
    Genode::Untyped_capability cap;
    Genode::Env &env = genode_env();
    char *addr;
    APIRET rc;

    err_ds = env.ram().alloc(*pcbLoadError);
    addr = env.rm().attach(err_ds);
    cap = (Genode::Untyped_capability)err_ds;
    exec->send_cap(cap, 0);
    strcpy((char *)_sysio->execopen.in.pszName, pszFileName);
    _sysio->execopen.in.cbLoadError = *pcbLoadError;
    _sysio->execopen.in.flags = ulFlags;
    exec->syscall(Session::SYSCALL_EXEC_OPEN);
    memcpy(pLoadError, addr, *pcbLoadError);
    *phmod = _sysio->execopen.out.hmod;
    rc = _sysio->execopen.out.rc;
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
    Genode::Untyped_capability cap;
    Genode::Ram_dataspace_capability err_ds, mod_ds;
    Genode::Env &env = genode_env();
    char *addr, *addr2;
    APIRET rc;

    err_ds = env.ram().alloc(*pcbLoadError);
    addr = env.rm().attach(err_ds);
    cap = (Genode::Untyped_capability)err_ds;
    exec->send_cap(cap, 0);
    mod_ds = env.ram().alloc(sizeof(os2exec_module_t));
    addr2 = env.rm().attach(mod_ds);
    cap = (Genode::Untyped_capability)mod_ds;
    exec->send_cap(cap, 1);
    _sysio->execload.in.hmod = hmod;
    _sysio->execload.in.cbLoadError = *pcbLoadError;
    exec->syscall(Session::SYSCALL_EXEC_LOAD);
    rc = _sysio->execload.out.rc;
    memcpy(pLoadError, addr, *pcbLoadError);
    memcpy(s, addr2, sizeof(os2exec_module_t));
    env.rm().detach(addr);
    env.ram().free(err_ds);
    env.rm().detach(addr2);
    env.ram().free(mod_ds);
    return rc;
}

extern "C"
APIRET ExcClientFree(HMODULE hmod)
{
    APIRET rc;
    _sysio->execfree.in.hmod = hmod;
    exec->syscall(Session::SYSCALL_EXEC_FREE);
    rc = _sysio->execfree.out.rc;
    return rc;
}

extern "C"
APIRET ExcClientShare(HMODULE hmod)
{
    APIRET rc;
    _sysio->execshare.in.hmod = hmod;
    exec->syscall(Session::SYSCALL_EXEC_SHARE);
    rc = _sysio->execshare.out.rc;
    return rc;
}

extern "C"
APIRET ExcClientGetImp(HMODULE hmod,
                       ULONG *index,
                       ULONG *imp_hmod)
{
    APIRET rc;
    _sysio->execgetimp.in.hmod = hmod;
    exec->syscall(Session::SYSCALL_EXEC_GETIMP);
    *index = _sysio->execgetimp.out.index;
    *imp_hmod = _sysio->execgetimp.out.imp_hmod;
    rc = _sysio->execgetimp.out.rc;
    return rc;
}

extern "C"
APIRET ExcClientGetSect(HMODULE hmod,
                        ULONG *index,
                        l4_os3_section_t *sect)
{
    Genode::Untyped_capability cap;
    Genode::Ram_dataspace_capability sect_ds;
    Genode::Env &env = genode_env();
    char *addr;
    APIRET rc;

    sect_ds = env.ram().alloc(sizeof(l4_os3_section_t));
    addr = env.rm().attach(sect_ds);
    cap = (Genode::Untyped_capability)sect_ds;
    exec->send_cap(cap, 0);
    _sysio->execgetsect.in.hmod = hmod;
    exec->syscall(Session::SYSCALL_EXEC_GETSECT);
    *index = _sysio->execgetsect.out.index;
    rc = _sysio->execgetsect.out.rc;
    memcpy(sect, addr, sizeof(l4_os3_section_t));
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
    APIRET rc;
    _sysio->execqueryprocaddr.in.hmod = hmod;
    _sysio->execqueryprocaddr.in.ordinal = ordinal;
    strcpy((char *)_sysio->execqueryprocaddr.in.mName, pszModname);
    exec->syscall(Session::SYSCALL_EXEC_QUERYPROCADDR);
    *ppfn = (void *)_sysio->execqueryprocaddr.out.addr;
    rc = _sysio->execqueryprocaddr.out.rc;
    return rc;
}

extern "C"
APIRET ExcClientQueryModuleHandle(PSZ pszModname,
                                  HMODULE *hmod)
{
    APIRET rc;
    strcpy((char *)_sysio->execquerymodulehandle.in.mName, pszModname);
    exec->syscall(Session::SYSCALL_EXEC_QUERYMODULEHANDLE);
    *hmod = _sysio->execquerymodulehandle.out.hmod;
    rc = _sysio->execquerymodulehandle.out.rc;
    return rc;
}

extern "C"
APIRET ExcClientQueryModuleName(HMODULE hmod,
                                ULONG cbName,
                                PBYTE pbName)
{
    APIRET rc;
    _sysio->execquerymodulename.in.hmod = hmod;
    _sysio->execquerymodulename.in.cbName = cbName;
    exec->syscall(Session::SYSCALL_EXEC_QUERYMODULENAME);
    memcpy(pbName, (char *)_sysio->execquerymodulename.out.pbName, cbName);
    rc = _sysio->execquerymodulename.out.rc;
    return rc;
}

extern "C"
APIRET ExcClientAllocSharedMem(ULONG cbSize,
                               PSZ pszName,
                               ULONG rights,
                               void **addr,
                               ULONGLONG *area)
{
    APIRET rc;
    _sysio->execallocsharedmem.in.cbSize = cbSize;
    strcpy(_sysio->execallocsharedmem.in.pszName, pszName);
    _sysio->execallocsharedmem.in.rights = rights;
    exec->syscall(Session::SYSCALL_EXEC_ALLOCSHAREDMEM);
    *addr = _sysio->execallocsharedmem.out.addr;
    *area = _sysio->execallocsharedmem.out.area;
    rc = _sysio->execallocsharedmem.out.rc;
    return rc;
}

extern "C"
APIRET ExcClientMapDataspace(void *addr,
                             ULONG rights,
                             l4_os3_dataspace_t ds)
{
    Genode::Ram_dataspace_capability _ds;
    Genode::Untyped_capability cap;
    APIRET rc;

    _ds = *(Genode::Ram_dataspace_capability *)ds;
    cap = (Genode::Untyped_capability)_ds;
    exec->send_cap(cap, 0);
    _sysio->execmapdataspace.in.addr = addr;
    _sysio->execmapdataspace.in.rights = rights;
    exec->syscall(Session::SYSCALL_EXEC_MAPDATASPACE);
    rc = _sysio->execmapdataspace.out.rc;
    return rc;
}

extern "C"
APIRET ExcClientUnmapDataspace(void *addr,
                               l4_os3_dataspace_t ds)
{
    Genode::Ram_dataspace_capability _ds;
    Genode::Untyped_capability cap;
    APIRET rc;

    _ds = *(Genode::Ram_dataspace_capability *)ds;
    cap = (Genode::Untyped_capability)_ds;
    exec->send_cap(cap, 0);
    _sysio->execunmapdataspace.in.addr = addr;
    exec->syscall(Session::SYSCALL_EXEC_UNMAPDATASPACE);
    rc = _sysio->execunmapdataspace.out.rc;
    return rc;
}

extern "C"
APIRET ExcClientGetDataspace(void **addr,
                             ULONG *size,
                             l4_os3_dataspace_t *ds)
{
    Genode::Ram_dataspace_capability _ds;
    Genode::Untyped_capability cap;
    APIRET rc;

    exec->syscall(Session::SYSCALL_EXEC_GETDATASPACE);
    *addr = _sysio->execgetdataspace.out.addr;
    *size = _sysio->execgetdataspace.out.size;
    rc = _sysio->execgetdataspace.out.rc;
    cap = exec->get_cap(0);
    _ds = Genode::reinterpret_cap_cast<Genode::Ram_dataspace>(cap);
    *ds = (l4_os3_dataspace_t)&_ds;
    return rc;
}

extern "C"
APIRET ExcClientGetSharedMem(void *pb,
                             void **addr,
                             ULONG *size,
                             PID *owner)
{
    APIRET rc;
    _sysio->execgetsharedmem.in.pb = pb;
    exec->syscall(Session::SYSCALL_EXEC_GETSHAREDMEM);
    *addr = _sysio->execgetsharedmem.out.addr;
    *size = _sysio->execgetsharedmem.out.size;
    *owner = _sysio->execgetsharedmem.out.owner;
    rc = _sysio->execgetsharedmem.out.rc;
    return rc;
}

extern "C"
APIRET ExcClientGetNamedSharedMem(PSZ pszName,
                                  void **addr,
                                  ULONG *size,
                                  PID *owner)
{
    APIRET rc;
    strcpy((char *)_sysio->execgetnamedsharedmem.in.pszName, pszName);
    exec->syscall(Session::SYSCALL_EXEC_GETNAMEDSHAREDMEM);
    *addr = _sysio->execgetnamedsharedmem.out.addr;
    *size = _sysio->execgetnamedsharedmem.out.size;
    *owner = _sysio->execgetnamedsharedmem.out.owner;
    rc = _sysio->execgetnamedsharedmem.out.rc;
    return rc;
}

extern "C"
APIRET ExcClientIncrementSharedMemRefcnt(void *addr)
{
    APIRET rc;
    _sysio->execincrementsharedmemrefcnt.in.addr = addr;
    exec->syscall(Session::SYSCALL_EXEC_INCREMENTSHAREDMEMREFCNT);
    rc = _sysio->execincrementsharedmemrefcnt.out.rc;
    return rc;
}

extern "C"
APIRET ExcClientReleaseSharedMem(void *addr,
                                 ULONG *count)
{
    APIRET rc;
    _sysio->execreleasesharedmem.in.addr = addr;
    exec->syscall(Session::SYSCALL_EXEC_RELEASESHAREDMEM);
    *count = _sysio->execreleasesharedmem.out.count;
    rc = _sysio->execreleasesharedmem.out.rc;
    return rc;
}
