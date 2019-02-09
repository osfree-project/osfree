#ifndef __FS_SESSION_CLIENT_H__
#define __FS_SESSION_CLIENT_H__

#include <fs_session/fs_session.h>
#include <base/rpc_client.h>

namespace OS2::Fs { struct Session_client; }

struct OS2::Fs::Session_client : Genode::Rpc_client<Session>
{
private:
	Genode::Env &env;

public:
	// constructor
	Session_client(Genode::Env &env,
                       Genode::Capability<Session> cap)
	: Genode::Rpc_client<Session>(cap), env(env) {  }

	long get_drivemap(ULONG *map)
	{
		return call<Rpc_get_drivemap>(map);
	}

	APIRET dos_Read(HFILE hFile,
                        Genode::Ram_dataspace_capability &ds,
                        ULONG *count)
	{
		return call<Rpc_dos_Read>(hFile, ds, count);
	}

	APIRET dos_Write(HFILE hFile,
                         Genode::Ram_dataspace_capability &ds,
                         ULONG *count)
	{
		return call<Rpc_dos_Write>(hFile, ds, count);
	}

	APIRET dos_ResetBuffer(HFILE hFile)
	{
		return call<Rpc_dos_ResetBuffer>(hFile);
	}

	APIRET dos_SetFilePtrL(HFILE hFile,
                               LONGLONG ib,
                               ULONG method,
                               ULONGLONG *ibActual)
	{
		return call<Rpc_dos_SetFilePtrL>(hFile, ib, method, ibActual);
	}

	APIRET dos_Close(HFILE hFile)
	{
		return call<Rpc_dos_Close>(hFile);
	}

	APIRET dos_QueryHType(HFILE hFile,
                              ULONG *pType,
                              ULONG *pAttr)
	{
		return call<Rpc_dos_QueryHType>(hFile, pType, pAttr);
	}

	APIRET dos_OpenL(Pathname &fName,
                         HFILE *phFile,
                         ULONG *pulAction,
                         LONGLONG cbFile,
                         ULONG ulAttribute,
                         ULONG fsOpenFlags,
                         ULONG fsOpenMode)
                         //EAOP2 *peaop2)
	{
		return call<Rpc_dos_OpenL>(fName,
		                           phFile, pulAction, cbFile,
		                           ulAttribute, fsOpenFlags,
		                           fsOpenMode); //, peaop2);
	}

	APIRET dos_DupHandle(HFILE hFile,
                             HFILE *phFile2)
	{
		return call<Rpc_dos_DupHandle>(hFile, phFile2);
	}

	APIRET dos_Delete(Pathname &fName)
	{
		return call<Rpc_dos_Delete>(fName);
	}

	APIRET dos_ForceDelete(Pathname &fName)
	{
		return call<Rpc_dos_ForceDelete>(fName);
	}

	APIRET dos_DeleteDir(Pathname &fName)
	{
		return call<Rpc_dos_DeleteDir>(fName);
	}

	APIRET dos_CreateDir(Pathname &fName,
	                     Genode::Ram_dataspace_capability &ds)
	{
		return call<Rpc_dos_CreateDir>(fName, ds);
	}

	APIRET dos_FindFirst(Pathname &pName,
                             HDIR *phDir,
                             ULONG flAttribute,
                             Genode::Ram_dataspace_capability &ds,
                             ULONG *pcFileNames,
                             ULONG ulInfoLevel)
	{
		return call<Rpc_dos_FindFirst>(pName, phDir,
		                               flAttribute, ds,
		                               pcFileNames, ulInfoLevel);
	}

	APIRET dos_FindNext(HDIR hDir,
                            Genode::Ram_dataspace_capability &ds,
                            ULONG *pcFileNames)
	{
		return call<Rpc_dos_FindNext>(hDir, ds, pcFileNames);
	}

	APIRET dos_FindClose(HDIR hDir)
	{
		return call<Rpc_dos_FindClose>(hDir);
	}

	APIRET dos_QueryFHState(HFILE hFile,
                                ULONG *pulMode)
	{
		return call<Rpc_dos_QueryFHState>(hFile, pulMode);
	}

	APIRET dos_SetFHState(HFILE hFile,
                              ULONG ulMode)
	{
		return call<Rpc_dos_SetFHState>(hFile, ulMode);
	}

	APIRET dos_QueryFileInfo(HFILE hFile,
                                 ULONG ulInfoLevel,
                                 Genode::Ram_dataspace_capability &ds)
	{
		return call<Rpc_dos_QueryFileInfo>(hFile, ulInfoLevel, ds);
	}

	APIRET dos_QueryPathInfo(Pathname &pName,
                                 ULONG ulInfoLevel,
                                 Genode::Ram_dataspace_capability &ds)
	{
		return call<Rpc_dos_QueryPathInfo>(pName, ulInfoLevel, ds);
	}

	APIRET dos_SetFileSizeL(HFILE hFile,
                                LONGLONG cbSize)
	{
		return call<Rpc_dos_SetFileSizeL>(hFile, cbSize);
	}

	APIRET dos_SetFileInfo(HFILE hFile,
                               ULONG ulInfoLevel,
                               Genode::Ram_dataspace_capability &ds)
	{
		return call<Rpc_dos_SetFileInfo>(hFile, ulInfoLevel,
		                                 ds);
	}

	APIRET dos_SetPathInfo(Pathname &pName,
                               ULONG ulInfoLevel,
                               Genode::Ram_dataspace_capability &ds,
                               ULONG flOptions)
	{
		return call<Rpc_dos_SetPathInfo>(pName, ulInfoLevel,
		                                 ds, flOptions);
	}
};

#endif /* __FS_SESSION_CLIENT_H__ */
