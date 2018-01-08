/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* Genode includes */
#include <base/allocator.h>
#include <fs_session/connection.h>

/* local includes */
#include "genode_env.h"

OS2::Fs::Connection *fs;

Genode::Allocator *_alloc = NULL;
Genode::Env *_env_ptr = NULL;

extern "C"
void FSClientInit(void)
{
    Genode::Allocator &alloc = genode_alloc();
    Genode::Env &env = genode_env();

    fs = new (alloc) OS2::Fs::Connection(env);
}

extern "C"
void FSClientDone(void)
{
    Genode::Allocator &alloc = genode_alloc();

    destroy(alloc, fs);
}

extern "C"
long FSClientGetDriveMap(ULONG *map)
{
    return fs->get_drivemap(map);
}

extern "C"
APIRET FSClientRead(HFILE hFile,
                    char *pBuf,
                    ULONG cbRead,
                    ULONG *pcbReadActual)
{
    Genode::Env &env = genode_env();
    Genode::Ram_dataspace_capability ds;
    char *addr;
    APIRET rc;

    ds = env.ram().alloc(cbRead);
    addr = env.rm().attach(ds);
    *pcbReadActual = cbRead;

    rc = fs->dos_Read(hFile, ds, pcbReadActual);

    Genode::memcpy(pBuf, addr, *pcbReadActual);
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
    char *addr;
    APIRET rc;

    ds = env.ram().alloc(cbWrite);
    addr = env.rm().attach(ds);
    *pcbWrittenActual = cbWrite;
    Genode::memcpy(addr, pBuf, cbWrite);

    rc = fs->dos_Write(hFile, ds, pcbWrittenActual);

    env.rm().detach(addr);
    env.ram().free(ds);
    return rc;
}

extern "C"
APIRET FSClientResetBuffer(HFILE hFile)
{
    return fs->dos_ResetBuffer(hFile);
}

extern "C"
APIRET FSClientSetFilePtrL(HFILE hFile,
                           LONGLONG ib,
                           ULONG method,
                           ULONGLONG *ibActual)
{
    return fs->dos_SetFilePtrL(hFile, ib, method, ibActual);
}

extern "C"
APIRET FSClientClose(HFILE hFile)
{
    return fs->dos_Close(hFile);
}

extern "C"
APIRET FSClientQueryHType(HFILE hFile,
                          ULONG *pType,
                          ULONG *pAttr)
{
    return fs->dos_QueryHType(hFile, pType, pAttr);
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
    OS2::Fs::Session::Pathname fName(pszFileName);

    return fs->dos_OpenL(fName,
                         phFile,
                         pulAction,
                         cbFile,
                         ulAttribute,
                         fsOpenFlags,
                         fsOpenMode);
}

extern "C"
APIRET FSClientDupHandle(HFILE hFile,
                         HFILE *phFile2)
{
    return fs->dos_DupHandle(hFile, phFile2);
}

extern "C"
APIRET FSClientDelete(PSZ pszFileName)
{
    OS2::Fs::Session::Pathname fName(pszFileName);
    return fs->dos_Delete(fName);
}

extern "C"
APIRET FSClientForceDelete(PSZ pszFileName)
{
    OS2::Fs::Session::Pathname fName(pszFileName);
    return fs->dos_ForceDelete(fName);
}

extern "C"
APIRET FSClientDeleteDir(PSZ pszDirName)
{
    OS2::Fs::Session::Pathname fName(pszDirName);
    return fs->dos_DeleteDir(fName);
}

extern "C"
APIRET FSClientCreateDir(PSZ pszDirName,
                         EAOP2 *peaop2)
{
    OS2::Fs::Session::Pathname fName(pszDirName);
    Genode::Env &env = genode_env();
    Genode::Ram_dataspace_capability ds;
    char *addr;
    APIRET rc;

    ds = env.ram().alloc(sizeof(EAOP2));
    addr = env.rm().attach(ds);

    rc = fs->dos_CreateDir(fName, ds);

    Genode::memcpy(peaop2, addr, sizeof(EAOP2));
    env.rm().detach(addr);
    env.ram().free(ds);
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
    OS2::Fs::Session::Pathname fName(pszFileSpec);
    Genode::Env &env = genode_env();
    Genode::Ram_dataspace_capability ds;
    char *addr;
    APIRET rc;

    ds = env.ram().alloc(*cbBuf);
    addr = env.rm().attach(ds);

    rc = fs->dos_FindFirst(fName, phDir, ulAttribute,
                           ds, pcFileNames, ulInfoLevel);

    Genode::memcpy(pFindBuf, addr, *cbBuf);
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
    char *addr;
    APIRET rc;

    ds = env.ram().alloc(*cbBuf);
    addr = env.rm().attach(ds);

    rc = fs->dos_FindNext(hDir, ds, pcFileNames);

    Genode::memcpy(pFindBuf, addr, *cbBuf);
    env.rm().detach(addr);
    env.ram().free(ds);
    return rc;
}

extern "C"
APIRET FSClientFindClose(HDIR hDir)
{
    return fs->dos_FindClose(hDir);
}

extern "C"
APIRET FSClientSetFHState(HFILE hFile,
                          ULONG ulMode)
{
    return fs->dos_SetFHState(hFile, ulMode);
}

extern "C"
APIRET FSClientQueryFileInfo(HFILE hFile,
                             ULONG ulInfoLevel,
                             char *pInfoBuf,
                             ULONG *cbInfoBuf)
{
    Genode::Env &env = genode_env();
    Genode::Ram_dataspace_capability ds;
    char *addr;
    APIRET rc;

    ds = env.ram().alloc(*cbInfoBuf);
    addr = env.rm().attach(ds);

    rc = fs->dos_QueryFileInfo(hFile, ulInfoLevel, ds);

    Genode::memcpy(pInfoBuf, addr, *cbInfoBuf);
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
    OS2::Fs::Session::Pathname pName(pszPathName);
    Genode::Env &env = genode_env();
    Genode::Ram_dataspace_capability ds;
    char *addr;
    APIRET rc;

    ds = env.ram().alloc(*cbInfoBuf);
    addr = env.rm().attach(ds);

    rc = fs->dos_QueryPathInfo(pName, ulInfoLevel, ds);

    Genode::memcpy(pInfoBuf, addr, *cbInfoBuf);
    env.rm().detach(addr);
    env.ram().free(ds);
    return rc;
}

extern "C"
APIRET FSClientSetFileSizeL(HFILE hFile,
                            LONGLONG cbSize)
{
    return fs->dos_SetFileSizeL(hFile, cbSize);
}

extern "C"
APIRET FSClientSetFileInfo(HFILE hFile,
                           ULONG ulInfoLevel,
                           char *pInfoBuf,
                           ULONG *cbInfoBuf)
{
    Genode::Env &env = genode_env();
    Genode::Ram_dataspace_capability ds;
    char *addr;
    APIRET rc;

    ds = env.ram().alloc(*cbInfoBuf);
    addr = env.rm().attach(ds);
    Genode::memcpy(addr, pInfoBuf, *cbInfoBuf);

    rc = fs->dos_SetFileInfo(hFile, ulInfoLevel, ds);

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
    OS2::Fs::Session::Pathname pName(pszPathName);
    Genode::Env &env = genode_env();
    Genode::Ram_dataspace_capability ds;
    char *addr;
    APIRET rc;

    ds = env.ram().alloc(*cbInfoBuf);
    addr = env.rm().attach(ds);
    Genode::memcpy(addr, pInfoBuf, *cbInfoBuf);

    rc = fs->dos_SetPathInfo(pName, ulInfoLevel, ds, flOptions);

    env.rm().detach(addr);
    env.ram().free(ds);
    return rc;
}
