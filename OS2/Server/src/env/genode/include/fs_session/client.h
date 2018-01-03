#ifndef __FS_SESSION_CLIENT_H__
#define __FS_SESSION_CLIENT_H__

#include <fs_session/fs_session.h>
#include <base/rpc_client.h>

namespace OS2::Fs { struct Session_client; }

struct OS2::Fs::Session_client : Genode::Rpc_client<Session>
{
	Session_client(Genode::Capability<Session> cap)
	: Genode::Rpc_client<Session>(cap) {  }

	long get_drivemap(ULONG *map)
	{
		return call<Rpc_get_drivemap>(map);
	}

	APIRET dos_Read(HFILE hFile,
                        char *pBuf,
                        ULONG *count)
	{
		return call<Rpc_dos_Read>(hFile, pBuf, count);
	}

	APIRET dos_Write(HFILE hFile,
                         char *pBuf,
                         ULONG *count)
	{
		return call<Rpc_dos_Write>(hFile, pBuf, count);
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

	APIRET dos_OpenL(PSZ pszFileName,
                         HFILE *phFile,
                         ULONG *pulAction,
                         LONGLONG cbFile,
                         ULONG ulAttribute,
                         ULONG fsOpenFlags,
                         ULONG fsOpenMode) //,
                         //ULONG dummy) //char *peaop2) // EAOP2 *peaop2
	{
		return call<Rpc_dos_OpenL>(pszFileName,
		                           phFile, pulAction, cbFile,
		                           ulAttribute, fsOpenFlags,
		                           fsOpenMode); //, dummy); //peaop2);
	}

	APIRET dos_DupHandle(HFILE hFile,
                             HFILE *phFile2)
	{
		return call<Rpc_dos_DupHandle>(hFile, phFile2);
	}

	APIRET dos_Delete(PSZ pszFileName)
	{
		return call<Rpc_dos_Delete>(pszFileName);
	}

	APIRET dos_ForceDelete(PSZ pszFileName)
	{
		return call<Rpc_dos_ForceDelete>(pszFileName);
	}

	APIRET dos_DeleteDir(PSZ pszDirName)
	{
		return call<Rpc_dos_DeleteDir>(pszDirName);
	}

	APIRET dos_CreateDir(PSZ pszDirName,
	                     EAOP2 *peaop2)
	{
		return call<Rpc_dos_CreateDir>(pszDirName, peaop2);
	}

	APIRET dos_FindFirst(PSZ pszFileSpec,
                             HDIR *phDir,
                             ULONG flAttribute,
                             char *pFindBuf,
                             ULONG *cbBuf,
                             ULONG *pcFileNames,
                             ULONG ulInfoLevel)
	{
		return call<Rpc_dos_FindFirst>(pszFileSpec, phDir,
		                               flAttribute, pFindBuf, cbBuf,
		                               pcFileNames, ulInfoLevel);
	}

	APIRET dos_FindNext(HDIR hDir,
                            char *pFindBuf,
                            ULONG *cbBuf,
                            ULONG *pcFileNames)
	{
		return call<Rpc_dos_FindNext>(hDir, pFindBuf,
		                              cbBuf, pcFileNames);
	}

	APIRET dos_FindClose(HDIR hDir)
	{
		return call<Rpc_dos_FindClose>(hDir);
	}

	APIRET dos_SetFHState(HFILE hFile,
                              ULONG ulMode)
	{
		return call<Rpc_dos_SetFHState>(hFile, ulMode);
	}

	APIRET dos_QueryFileInfo(HFILE hFile,
                                 ULONG ulInfoLevel,
                                 char *pInfoBuf,
                                 ULONG *cbBuf)
	{
		return call<Rpc_dos_QueryFileInfo>(hFile, ulInfoLevel,
		                                   pInfoBuf, cbBuf);
	}

	APIRET dos_QueryPathInfo(PSZ pszPathName,
                                 ULONG ulInfoLevel,
                                 char *pInfoBuf,
                                 ULONG *cbBuf)
	{
		return call<Rpc_dos_QueryPathInfo>(pszPathName, ulInfoLevel,
		                                   pInfoBuf, cbBuf);
	}

	APIRET dos_SetFileSizeL(HFILE hFile,
                                LONGLONG cbSize)
	{
		return call<Rpc_dos_SetFileSizeL>(hFile, cbSize);
	}

	APIRET dos_SetFileInfo(HFILE hFile,
                               ULONG ulInfoLevel,
                               char *pInfoBuf,
                               ULONG *cbBuf)
	{
		return call<Rpc_dos_SetFileInfo>(hFile, ulInfoLevel,
		                                 pInfoBuf, cbBuf);
	}

	APIRET dos_SetPathInfo(PSZ pszPathName,
                               ULONG ulInfoLevel,
                               char *pInfoBuf,
                               ULONG *cbBuf,
                               ULONG flOptions)
	{
		return call<Rpc_dos_SetPathInfo>(pszPathName, ulInfoLevel,
		                                 pInfoBuf, cbBuf, flOptions);
	}
};

#endif /* __FS_SESSION_CLIENT_H__ */
