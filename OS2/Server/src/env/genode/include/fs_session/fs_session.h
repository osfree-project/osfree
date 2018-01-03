#ifndef __FS_SESSION_FS_SESSION_H__
#define __FS_SESSION_FS_SESSION_H__

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

namespace OS2::Fs { struct Session; }

/* Genode includes */
#include <base/rpc.h>
#include <base/rpc_args.h>

struct OS2::Fs::Session : Genode::Session
{
    static const char *service_name() { return "os2fs"; }

    enum { CAP_QUOTA = 2 };

    virtual long get_drivemap(ULONG *map) = 0;

    virtual APIRET dos_Read(HFILE hFile,
                            char *pBuf,
                            ULONG *count) = 0;

    virtual APIRET dos_Write(HFILE hFile,
                             char *pBuf,
                             ULONG *count) = 0;

    virtual APIRET dos_ResetBuffer(HFILE hFile) = 0;

    virtual APIRET dos_SetFilePtrL(HFILE hFile,
                                   LONGLONG ib,
                                   ULONG method,
                                   ULONGLONG *ibActual) = 0;

    virtual APIRET dos_Close(HFILE hFile) = 0;

    virtual APIRET dos_QueryHType(HFILE hFile,
                                  ULONG *pType,
                                  ULONG *pAttr) = 0;

    virtual APIRET dos_OpenL(PSZ pszFileName,
                             HFILE *phFile,
                             ULONG *pulAction,
                             LONGLONG cbFile,
                             ULONG ulAttribute,
                             ULONG fsOpenFlags,
                             ULONG fsOpenMode) = 0; //,
                             //ULONG dummy) = 0; // EAOP2 *peaop2

   virtual APIRET dos_DupHandle(HFILE hFile,
                                HFILE *phFile2) = 0;

   virtual APIRET dos_Delete(PSZ pszFileName) = 0;

   virtual APIRET dos_ForceDelete(PSZ pszFileName) = 0;

   virtual APIRET dos_DeleteDir(PSZ pszDirName) = 0;

   virtual APIRET dos_CreateDir(PSZ pszDirName,
                                EAOP2 *peaop2) = 0;

   virtual APIRET dos_FindFirst(PSZ pszFileSpec,
                                HDIR *phDir,
                                ULONG flAttribute,
                                char *pFindBuf,
                                ULONG *cbBuf,
                                ULONG *pcFileNames,
                                ULONG ulInfoLevel) = 0;

   virtual APIRET dos_FindNext(HDIR hDir,
                               char *pFindBuf,
                               ULONG *cbBuf,
                               ULONG *pcFileNames) = 0;

   virtual APIRET dos_FindClose(HDIR hDir) = 0;

   virtual APIRET dos_SetFHState(HFILE hFile,
                                 ULONG ulMode) = 0;

   virtual APIRET dos_QueryFileInfo(HFILE hFile,
                                    ULONG ulInfoLevel,
                                    char *pBuf,
                                    ULONG *cbBuf) = 0;

   virtual APIRET dos_QueryPathInfo(PSZ pszPathName,
                                    ULONG ulInfoLevel,
                                    char *pBuf,
                                    ULONG *cbBuf) = 0;

   virtual APIRET dos_SetFileSizeL(HFILE hFile,
                                   LONGLONG cbSize) = 0;

   virtual APIRET dos_SetFileInfo(HFILE hFile,
                                  ULONG ulInfoLevel,
                                  char *pInfoBuf,
                                  ULONG *cbInfoBuf) = 0;

   virtual APIRET dos_SetPathInfo(PSZ pszPathName,
                                  ULONG ulInfoLevel,
                                  char *pInfoBuf,
                                  ULONG *cbInfoBuf,
                                  ULONG flOptions) = 0;

  /* RPC interface */
  GENODE_RPC(Rpc_get_drivemap, long, get_drivemap, ULONG *);
  GENODE_RPC(Rpc_dos_Read, APIRET, dos_Read, HFILE, char *, ULONG *);
  GENODE_RPC(Rpc_dos_Write, APIRET, dos_Write, HFILE, char *, ULONG *);
  GENODE_RPC(Rpc_dos_ResetBuffer, APIRET, dos_ResetBuffer, HFILE);
  GENODE_RPC(Rpc_dos_SetFilePtrL, APIRET, dos_SetFilePtrL, HFILE, LONGLONG, ULONG, ULONGLONG *);
  GENODE_RPC(Rpc_dos_Close, APIRET, dos_Close, HFILE);
  GENODE_RPC(Rpc_dos_QueryHType, APIRET, dos_QueryHType, HFILE, ULONG *, ULONG *);
  GENODE_RPC(Rpc_dos_OpenL, APIRET, dos_OpenL, PSZ, HFILE *, ULONG *, LONGLONG, ULONG, ULONG, ULONG); //, ULONG);
  GENODE_RPC(Rpc_dos_DupHandle, APIRET, dos_DupHandle, HFILE, HFILE *);
  GENODE_RPC(Rpc_dos_Delete, APIRET, dos_Delete, PSZ);
  GENODE_RPC(Rpc_dos_ForceDelete, APIRET, dos_ForceDelete, PSZ);
  GENODE_RPC(Rpc_dos_DeleteDir, APIRET, dos_DeleteDir, PSZ);
  GENODE_RPC(Rpc_dos_CreateDir, APIRET, dos_CreateDir, PSZ, EAOP2 *);
  GENODE_RPC(Rpc_dos_FindFirst, APIRET, dos_FindFirst, PSZ, HDIR *, ULONG, char *, ULONG *, ULONG *, ULONG);
  GENODE_RPC(Rpc_dos_FindNext, APIRET, dos_FindNext, HDIR, char *, ULONG *, ULONG *);
  GENODE_RPC(Rpc_dos_FindClose, APIRET, dos_FindClose, HDIR);
  GENODE_RPC(Rpc_dos_SetFHState, APIRET, dos_SetFHState, HFILE, ULONG);
  GENODE_RPC(Rpc_dos_QueryFileInfo, APIRET, dos_QueryFileInfo, HFILE, ULONG, char *, ULONG *);
  GENODE_RPC(Rpc_dos_QueryPathInfo, APIRET, dos_QueryPathInfo, PSZ, ULONG, char *, ULONG *);
  GENODE_RPC(Rpc_dos_SetFileSizeL, APIRET, dos_SetFileSizeL, HFILE, LONGLONG);
  GENODE_RPC(Rpc_dos_SetFileInfo, APIRET, dos_SetFileInfo, HFILE, ULONG, char *, ULONG *);
  GENODE_RPC(Rpc_dos_SetPathInfo, APIRET, dos_SetPathInfo, PSZ, ULONG, char *, ULONG *, ULONG);

  GENODE_RPC_INTERFACE(Rpc_get_drivemap, Rpc_dos_Read, Rpc_dos_Write, Rpc_dos_ResetBuffer,
    Rpc_dos_SetFilePtrL, Rpc_dos_Close, Rpc_dos_QueryHType, Rpc_dos_OpenL, Rpc_dos_DupHandle,
    Rpc_dos_Delete, Rpc_dos_ForceDelete, Rpc_dos_DeleteDir, Rpc_dos_CreateDir, Rpc_dos_FindFirst,
    Rpc_dos_FindNext, Rpc_dos_FindClose, Rpc_dos_SetFHState, Rpc_dos_QueryFileInfo, Rpc_dos_QueryPathInfo,
    Rpc_dos_SetFileSizeL, Rpc_dos_SetFileInfo, Rpc_dos_SetPathInfo);
};

#endif /* __FS_SESSION_FS_SESSION_H__ */
