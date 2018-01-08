#ifndef __FS_SESSION_FS_SESSION_H__
#define __FS_SESSION_FS_SESSION_H__

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

namespace OS2::Fs { struct Session; }

/* Genode includes */
#include <base/rpc.h>
#include <base/rpc_args.h>
#include <base/ram_allocator.h>
#include <session/session.h>
#include <dataspace/capability.h>

struct OS2::Fs::Session : Genode::Session
{
    static const char *service_name() { return "os2fs"; }

    enum { CAP_QUOTA = 2 };

    typedef Genode::Rpc_in_buffer<CCHMAXPATHCOMP> Pathname;

    virtual long get_drivemap(ULONG *map) = 0;

    virtual APIRET dos_Read(HFILE hFile,
                            Genode::Ram_dataspace_capability &ds,
                            ULONG *count) = 0;

    virtual APIRET dos_Write(HFILE hFile,
                             Genode::Ram_dataspace_capability &ds,
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

    virtual APIRET dos_OpenL(Pathname &fName,
                             HFILE *phFile,
                             ULONG *pulAction,
                             LONGLONG cbFile,
                             ULONG ulAttribute,
                             ULONG fsOpenFlags,
                             ULONG fsOpenMode) = 0; //,
                             // EAOP2 *peaop2) = 0;

   virtual APIRET dos_DupHandle(HFILE hFile,
                                HFILE *phFile2) = 0;

   virtual APIRET dos_Delete(Pathname &fName) = 0;

   virtual APIRET dos_ForceDelete(Pathname &fName) = 0;

   virtual APIRET dos_DeleteDir(Pathname &dName) = 0;

   virtual APIRET dos_CreateDir(Pathname &dName,
                                Genode::Ram_dataspace_capability &ds) = 0;

   virtual APIRET dos_FindFirst(Pathname &pName,
                                HDIR *phDir,
                                ULONG flAttribute,
                                Genode::Ram_dataspace_capability &ds,
                                ULONG *pcFileNames,
                                ULONG ulInfoLevel) = 0;

   virtual APIRET dos_FindNext(HDIR hDir,
                               Genode::Ram_dataspace_capability &ds,
                               ULONG *pcFileNames) = 0;

   virtual APIRET dos_FindClose(HDIR hDir) = 0;

   virtual APIRET dos_SetFHState(HFILE hFile,
                                 ULONG ulMode) = 0;

   virtual APIRET dos_QueryFileInfo(HFILE hFile,
                                    ULONG ulInfoLevel,
                                    Genode::Ram_dataspace_capability &ds) = 0;

   virtual APIRET dos_QueryPathInfo(Pathname &pName,
                                    ULONG ulInfoLevel,
                                    Genode::Ram_dataspace_capability &ds) = 0;

   virtual APIRET dos_SetFileSizeL(HFILE hFile,
                                   LONGLONG cbSize) = 0;

   virtual APIRET dos_SetFileInfo(HFILE hFile,
                                  ULONG ulInfoLevel,
                                  Genode::Ram_dataspace_capability &ds) = 0;

   virtual APIRET dos_SetPathInfo(Pathname &pName,
                                  ULONG ulInfoLevel,
                                  Genode::Ram_dataspace_capability &ds,
                                  ULONG flOptions) = 0;

  /* RPC interface */
  GENODE_RPC(Rpc_get_drivemap, long, get_drivemap, ULONG *);
  GENODE_RPC(Rpc_dos_Read, APIRET, dos_Read, HFILE, Genode::Ram_dataspace_capability &, ULONG *);
  GENODE_RPC(Rpc_dos_Write, APIRET, dos_Write, HFILE, Genode::Ram_dataspace_capability &, ULONG *);
  GENODE_RPC(Rpc_dos_ResetBuffer, APIRET, dos_ResetBuffer, HFILE);
  GENODE_RPC(Rpc_dos_SetFilePtrL, APIRET, dos_SetFilePtrL, HFILE, LONGLONG, ULONG, ULONGLONG *);
  GENODE_RPC(Rpc_dos_Close, APIRET, dos_Close, HFILE);
  GENODE_RPC(Rpc_dos_QueryHType, APIRET, dos_QueryHType, HFILE, ULONG *, ULONG *);
  GENODE_RPC(Rpc_dos_OpenL, APIRET, dos_OpenL, Pathname &, HFILE *, ULONG *, LONGLONG, ULONG, ULONG, ULONG); //, ULONG);
  GENODE_RPC(Rpc_dos_DupHandle, APIRET, dos_DupHandle, HFILE, HFILE *);
  GENODE_RPC(Rpc_dos_Delete, APIRET, dos_Delete, Pathname &);
  GENODE_RPC(Rpc_dos_ForceDelete, APIRET, dos_ForceDelete, Pathname &);
  GENODE_RPC(Rpc_dos_DeleteDir, APIRET, dos_DeleteDir, Pathname &);
  GENODE_RPC(Rpc_dos_CreateDir, APIRET, dos_CreateDir, Pathname &, Genode::Ram_dataspace_capability &);
  GENODE_RPC(Rpc_dos_FindFirst, APIRET, dos_FindFirst, Pathname &, HDIR *, ULONG,
    Genode::Ram_dataspace_capability &, ULONG *, ULONG);
  GENODE_RPC(Rpc_dos_FindNext, APIRET, dos_FindNext, HDIR, Genode::Ram_dataspace_capability &, ULONG *);
  GENODE_RPC(Rpc_dos_FindClose, APIRET, dos_FindClose, HDIR);
  GENODE_RPC(Rpc_dos_SetFHState, APIRET, dos_SetFHState, HFILE, ULONG);
  GENODE_RPC(Rpc_dos_QueryFileInfo, APIRET, dos_QueryFileInfo, HFILE, ULONG, Genode::Ram_dataspace_capability &);
  GENODE_RPC(Rpc_dos_QueryPathInfo, APIRET, dos_QueryPathInfo, Pathname &, ULONG, Genode::Ram_dataspace_capability &);
  GENODE_RPC(Rpc_dos_SetFileSizeL, APIRET, dos_SetFileSizeL, HFILE, LONGLONG);
  GENODE_RPC(Rpc_dos_SetFileInfo, APIRET, dos_SetFileInfo, HFILE, ULONG, Genode::Ram_dataspace_capability &);
  GENODE_RPC(Rpc_dos_SetPathInfo, APIRET, dos_SetPathInfo, Pathname &, ULONG, Genode::Ram_dataspace_capability &, ULONG);

  GENODE_RPC_INTERFACE(Rpc_get_drivemap, Rpc_dos_Read, Rpc_dos_Write, Rpc_dos_ResetBuffer,
    Rpc_dos_SetFilePtrL, Rpc_dos_Close, Rpc_dos_QueryHType, Rpc_dos_OpenL, Rpc_dos_DupHandle,
    Rpc_dos_Delete, Rpc_dos_ForceDelete, Rpc_dos_DeleteDir, Rpc_dos_CreateDir, Rpc_dos_FindFirst,
    Rpc_dos_FindNext, Rpc_dos_FindClose, Rpc_dos_SetFHState, Rpc_dos_QueryFileInfo, Rpc_dos_QueryPathInfo,
    Rpc_dos_SetFileSizeL, Rpc_dos_SetFileInfo, Rpc_dos_SetPathInfo);
};

#endif /* __FS_SESSION_FS_SESSION_H__ */
