/* os2fs client-side RPC API (Genode platform) */

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/thread.h>

/* libc includes */
#include <string.h>

/* Genode includes */
#include <base/allocator.h>
#include <base/attached_dataspace.h>

#include <cpi_session/connection.h>

/* local includes */
#include "genode_env.h"

using namespace OS2::Cpi;

static Connection *fs;

static Sysio *_sysio = NULL;

extern "C"
APIRET FSClientInit(l4_os3_thread_t *thread)
{
    Genode::Allocator &alloc = genode_alloc();
    Genode::Env &env = genode_env();
    Genode::Dataspace_capability _ds;

    try
    {
        fs = new (alloc) OS2::Cpi::Connection(env, "fs");
    }
    catch (...)
    {
        return ERROR_FILE_NOT_FOUND;
    }

    _ds = fs->sysio_dataspace();

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

    *thread = fs;

    return NO_ERROR;
}

extern "C"
APIRET FSClientDone(void)
{
    Genode::Allocator &alloc = genode_alloc();

    destroy(alloc, fs);
    return NO_ERROR;
}

extern "C"
long FSClientGetDriveMap(ULONG *map)
{
    APIRET rc;
    fs->syscall(Session::SYSCALL_FS_GETDRIVEMAP);
    *map = _sysio->fsgetdrivemap.out.map;
    rc = _sysio->fsgetdrivemap.out.rc;
    return rc;
}

extern "C"
APIRET FSClientRead(HFILE hFile,
                    char *pBuf,
                    ULONG cbRead,
                    ULONG *pcbReadActual)
{
    Genode::Env &env = genode_env();
    Genode::Ram_dataspace_capability ds;
    Genode::Untyped_capability cap;
    char *addr;
    APIRET rc;

    ds = env.ram().alloc(cbRead);
    addr = env.rm().attach(ds);
    cap = (Genode::Untyped_capability)ds;
    fs->send_cap(cap, 0);
    _sysio->fsread.in.hFile = hFile;
    _sysio->fsread.in.cbRead = cbRead;
    fs->syscall(Session::SYSCALL_FS_READ);
    *pcbReadActual = _sysio->fsread.out.cbActual;
    rc = _sysio->fsread.out.rc;
    memcpy(pBuf, addr, *pcbReadActual);
    env.rm().detach(addr);
    env.ram().free(ds);
    return rc;
}

extern "C"
APIRET FSClientWrite(HFILE hFile,
                     char *pBuf,
                     ULONG cbWrite,
                     ULONG *pcbWrittenActual)
{
    Genode::Env &env = genode_env();
    Genode::Ram_dataspace_capability ds;
    Genode::Untyped_capability cap;
    char *addr;
    APIRET rc;

    ds = env.ram().alloc(cbWrite);
    addr = env.rm().attach(ds);
    cap = (Genode::Untyped_capability)ds;
    fs->send_cap(cap, 0);
    _sysio->fswrite.in.hFile = hFile;
    _sysio->fswrite.in.cbWrite = cbWrite;
    memcpy(addr, pBuf, cbWrite);
    fs->syscall(Session::SYSCALL_FS_WRITE);
    *pcbWrittenActual = _sysio->fswrite.out.cbActual;
    rc = _sysio->fswrite.out.rc;
    env.rm().detach(addr);
    env.ram().free(ds);
    return rc;
}

extern "C"
APIRET FSClientResetBuffer(HFILE hFile)
{
    APIRET rc;
    _sysio->fsresetbuffer.in.hFile = hFile;
    fs->syscall(Session::SYSCALL_FS_RESETBUFFER);
    rc = _sysio->fsresetbuffer.out.rc;
    return rc;
}

extern "C"
APIRET FSClientSetFilePtrL(HFILE hFile,
                           LONGLONG ib,
                           ULONG method,
                           ULONGLONG *ibActual)
{
    APIRET rc;
    _sysio->fssetfileptrl.in.hFile = hFile;
    _sysio->fssetfileptrl.in.ib = ib;
    _sysio->fssetfileptrl.in.method = method;
    fs->syscall(Session::SYSCALL_FS_SETFILEPTRL);
    *ibActual = _sysio->fssetfileptrl.out.ibActual;
    rc = _sysio->fssetfileptrl.out.rc;
    return rc;
}

extern "C"
APIRET FSClientClose(HFILE hFile)
{
    APIRET rc;
    _sysio->fsclose.in.hFile = hFile;
    fs->syscall(Session::SYSCALL_FS_CLOSE);
    rc = _sysio->fsclose.out.rc;
    return rc;
}

extern "C"
APIRET FSClientQueryHType(HFILE hFile,
                          ULONG *pType,
                          ULONG *pAttr)
{
    APIRET rc;
    _sysio->fsqueryhtype.in.hFile = hFile;
    fs->syscall(Session::SYSCALL_FS_QUERYHTYPE);
    *pType = _sysio->fsqueryhtype.out.type;
    *pAttr = _sysio->fsqueryhtype.out.attr;
    rc = _sysio->fsqueryhtype.out.rc;
    return rc;
}

extern "C"
APIRET FSClientOpenL(PSZ pszFileName,
                     HFILE *phFile,
                     ULONG *pulAction,
                     LONGLONG cbFile,
                     ULONG ulAttribute,
                     ULONG fsOpenFlags,
                     ULONG fsOpenMode,
                     EAOP2 *peaop2)
{
    APIRET rc;
    strcpy(_sysio->fsopenl.in.pszFilename, pszFileName);
    _sysio->fsopenl.in.cbFile = cbFile;
    _sysio->fsopenl.in.ulAttribute = ulAttribute;
    _sysio->fsopenl.in.fsOpenFlags = fsOpenFlags;
    _sysio->fsopenl.in.fsOpenMode = fsOpenMode;
    fs->syscall(Session::SYSCALL_FS_OPENL);
    *phFile = _sysio->fsopenl.out.hFile;
    *pulAction = _sysio->fsopenl.out.ulAction;
    memcpy(peaop2, &_sysio->fsopenl.out.eaop2, sizeof(EAOP2));
    rc = _sysio->fsopenl.out.rc;
    return rc;
}

extern "C"
APIRET FSClientDupHandle(HFILE hFile,
                         HFILE *phFile2)
{
    APIRET rc;
    _sysio->fsduphandle.in.hFile = hFile;
    fs->syscall(Session::SYSCALL_FS_DUPHANDLE);
    *phFile2 = _sysio->fsduphandle.out.hFile2;
    rc = _sysio->fsduphandle.out.rc;
    return rc;
}

extern "C"
APIRET FSClientDelete(PSZ pszFileName)
{
    APIRET rc;
    strcpy(_sysio->fsdelete.in.pszFilename, pszFileName);
    fs->syscall(Session::SYSCALL_FS_DELETE);
    rc = _sysio->fsdelete.out.rc;
    return rc;
}

extern "C"
APIRET FSClientForceDelete(PSZ pszFileName)
{
    APIRET rc;
    strcpy(_sysio->fsforcedelete.in.pszFilename, pszFileName);
    fs->syscall(Session::SYSCALL_FS_FORCEDELETE);
    rc = _sysio->fsforcedelete.out.rc;
    return rc;
}

extern "C"
APIRET FSClientDeleteDir(PSZ pszDirName)
{
    APIRET rc;
    strcpy(_sysio->fsdeletedir.in.pszDirname, pszDirName);
    fs->syscall(Session::SYSCALL_FS_DELETEDIR);
    rc = _sysio->fsdeletedir.out.rc;
    return rc;
}

extern "C"
APIRET FSClientCreateDir(PSZ pszDirName,
                         EAOP2 *peaop2)
{
    APIRET rc;
    strcpy(_sysio->fscreatedir.in.pszDirname, pszDirName);
    memcpy(&_sysio->fscreatedir.in.eaop2, peaop2, sizeof(EAOP2));
    fs->syscall(Session::SYSCALL_FS_CREATEDIR);
    rc = _sysio->fscreatedir.out.rc;
    return rc;
}

extern "C"
APIRET FSClientFindFirst(PSZ pszFileSpec,
                         HDIR *phDir,
                         ULONG ulAttribute,
                         char *pFindBuf,
                         ULONG *cbBuf,
                         ULONG *pcFileNames,
                         ULONG ulInfoLevel)
{
    Genode::Env &env = genode_env();
    Genode::Ram_dataspace_capability ds;
    Genode::Untyped_capability cap;
    char *addr;
    APIRET rc;

    ds = env.ram().alloc(*cbBuf);
    addr = env.rm().attach(ds);
    cap = (Genode::Untyped_capability)ds;
    fs->send_cap(cap, 0);
    strcpy(_sysio->fsfindfirst.in.pszFilespec, pszFileSpec);
    _sysio->fsfindfirst.in.ulAttribute = ulAttribute;
    _sysio->fsfindfirst.in.ulInfoLevel = ulInfoLevel;
    _sysio->fsfindfirst.out.cbBuf = *cbBuf;
    fs->syscall(Session::SYSCALL_FS_FINDFIRST);
    *phDir = _sysio->fsfindfirst.out.hDir;
    *cbBuf = _sysio->fsfindfirst.out.cbBuf;
    *pcFileNames = _sysio->fsfindfirst.out.cFileNames;
    memcpy(pFindBuf, addr, *cbBuf);
    rc = _sysio->fsfindfirst.out.rc;
    env.rm().detach(addr);
    env.ram().free(ds);
    return rc;
}

extern "C"
APIRET FSClientFindNext(HDIR hDir,
                        char *pFindBuf,
                        ULONG *cbBuf,
                        ULONG *pcFileNames)
{
    Genode::Env &env = genode_env();
    Genode::Ram_dataspace_capability ds;
    Genode::Untyped_capability cap;
    char *addr;
    APIRET rc;

    ds = env.ram().alloc(*cbBuf);
    addr = env.rm().attach(ds);
    cap = (Genode::Untyped_capability)ds;
    fs->send_cap(cap, 0);
    _sysio->fsfindnext.in.hDir = hDir;
    _sysio->fsfindnext.out.cbBuf = *cbBuf;
    fs->syscall(Session::SYSCALL_FS_FINDNEXT);
    *cbBuf = _sysio->fsfindnext.out.cbBuf;
    *pcFileNames = _sysio->fsfindnext.out.cFileNames;
    memcpy(pFindBuf, addr, *cbBuf);
    rc = _sysio->fsfindnext.out.rc;
    env.rm().detach(addr);
    env.ram().free(ds);
    return rc;
}

extern "C"
APIRET FSClientFindClose(HDIR hDir)
{
    APIRET rc;
    _sysio->fsfindclose.in.hDir = hDir;
    fs->syscall(Session::SYSCALL_FS_FINDCLOSE);
    rc = _sysio->fsfindclose.out.rc;
    return rc;
}

extern "C"
APIRET FSClientQueryFHState(HFILE hFile,
                            ULONG *pulMode)
{
    APIRET rc;
    _sysio->fsqueryfhstate.in.hFile = hFile;
    fs->syscall(Session::SYSCALL_FS_QUERYFHSTATE);
    *pulMode = _sysio->fsqueryfhstate.out.ulMode;
    rc = _sysio->fsqueryfhstate.out.rc;
    return rc;
}

extern "C"
APIRET FSClientSetFHState(HFILE hFile,
                          ULONG ulMode)
{
    APIRET rc;
    _sysio->fssetfhstate.in.hFile = hFile;
    _sysio->fssetfhstate.in.ulMode = ulMode;
    fs->syscall(Session::SYSCALL_FS_SETFHSTATE);
    rc = _sysio->fssetfhstate.out.rc;
    return rc;
}

extern "C"
APIRET FSClientQueryFileInfo(HFILE hFile,
                             ULONG ulInfoLevel,
                             char *pInfoBuf,
                             ULONG *cbInfoBuf)
{
    Genode::Env &env = genode_env();
    Genode::Ram_dataspace_capability ds;
    Genode::Untyped_capability cap;
    char *addr;
    APIRET rc;

    ds = env.ram().alloc(*cbInfoBuf);
    addr = env.rm().attach(ds);
    cap = (Genode::Untyped_capability)ds;
    fs->send_cap(cap, 0);
    _sysio->fsqueryfileinfo.in.hFile = hFile;
    _sysio->fsqueryfileinfo.in.ulInfoLevel = ulInfoLevel;
    _sysio->fsqueryfileinfo.out.cbInfoBuf = *cbInfoBuf;
    fs->syscall(Session::SYSCALL_FS_QUERYFILEINFO);
    *cbInfoBuf = _sysio->fsqueryfileinfo.out.cbInfoBuf;
    memcpy(pInfoBuf, addr, *cbInfoBuf);
    rc = _sysio->fsqueryfileinfo.out.rc;
    env.rm().detach(addr);
    env.ram().free(ds);
    return rc;
}

extern "C"
APIRET FSClientQueryPathInfo(PSZ pszPathName,
                             ULONG ulInfoLevel,
                             char *pInfoBuf,
                             ULONG *cbInfoBuf)
{
    Genode::Env &env = genode_env();
    Genode::Ram_dataspace_capability ds;
    Genode::Untyped_capability cap;
    char *addr;
    APIRET rc;

    ds = env.ram().alloc(*cbInfoBuf);
    addr = env.rm().attach(ds);
    cap = (Genode::Untyped_capability)ds;
    fs->send_cap(cap, 0);
    strcpy(_sysio->fsquerypathinfo.in.pszPathName, pszPathName);
    _sysio->fsquerypathinfo.in.ulInfoLevel = ulInfoLevel;
    _sysio->fsquerypathinfo.out.cbInfoBuf = *cbInfoBuf;
    fs->syscall(Session::SYSCALL_FS_QUERYPATHINFO);
    *cbInfoBuf = _sysio->fsquerypathinfo.out.cbInfoBuf;
    memcpy(pInfoBuf, addr, *cbInfoBuf);
    rc = _sysio->fsquerypathinfo.out.rc;
    env.rm().detach(addr);
    env.ram().free(ds);
    return rc;
}

extern "C"
APIRET FSClientSetFileSizeL(HFILE hFile,
                            LONGLONG cbSize)
{
    APIRET rc;
    _sysio->fssetfilesizel.in.hFile = hFile;
    _sysio->fssetfilesizel.in.cbSize = cbSize;
    fs->syscall(Session::SYSCALL_FS_SETFILESIZEL);
    rc = _sysio->fssetfilesizel.out.rc;
    return rc;
}

extern "C"
APIRET FSClientSetFileInfo(HFILE hFile,
                           ULONG ulInfoLevel,
                           char *pInfoBuf,
                           ULONG *cbInfoBuf)
{
    Genode::Env &env = genode_env();
    Genode::Ram_dataspace_capability ds;
    Genode::Untyped_capability cap;
    char *addr;
    APIRET rc;

    ds = env.ram().alloc(*cbInfoBuf);
    addr = env.rm().attach(ds);
    cap = (Genode::Untyped_capability)ds;
    fs->send_cap(cap, 0);
    _sysio->fssetfileinfo.in.hFile = hFile;
    _sysio->fssetfileinfo.in.ulInfoLevel = ulInfoLevel;
    _sysio->fssetfileinfo.out.cbInfoBuf = *cbInfoBuf;
    fs->syscall(Session::SYSCALL_FS_SETFILEINFO);
    *cbInfoBuf = _sysio->fssetfileinfo.out.cbInfoBuf;
    memcpy(pInfoBuf, addr, *cbInfoBuf);
    rc = _sysio->fssetfileinfo.out.rc;
    env.rm().detach(addr);
    env.ram().free(ds);
    return rc;
}

extern "C"
APIRET FSClientSetPathInfo(PSZ pszPathName,
                           ULONG ulInfoLevel,
                           char *pInfoBuf,
                           ULONG *cbInfoBuf,
                           ULONG flOptions)
{
    Genode::Env &env = genode_env();
    Genode::Ram_dataspace_capability ds;
    Genode::Untyped_capability cap;
    char *addr;
    APIRET rc;

    ds = env.ram().alloc(*cbInfoBuf);
    addr = env.rm().attach(ds);
    cap = (Genode::Untyped_capability)ds;
    fs->send_cap(cap, 0);
    strcpy(_sysio->fssetpathinfo.in.pszPathName, pszPathName);
    _sysio->fssetpathinfo.in.ulInfoLevel = ulInfoLevel;
    _sysio->fssetpathinfo.out.cbInfoBuf = *cbInfoBuf;
    _sysio->fssetpathinfo.in.flOptions = flOptions;
    fs->syscall(Session::SYSCALL_FS_SETPATHINFO);
    *cbInfoBuf = _sysio->fssetpathinfo.out.cbInfoBuf;
    memcpy(pInfoBuf, addr, *cbInfoBuf);
    rc = _sysio->fssetpathinfo.out.rc;
    env.rm().detach(addr);
    env.ram().free(ds);
    return rc;
}
