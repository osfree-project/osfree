#ifndef __OS2FS_API_API_H__
#define __OS2FS_API_API_H__

#ifdef __cplusplus
  extern "C" {
#endif

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal; */
#include <os3/rm.h>

APIRET FSRead(HFILE hFile,
              PBYTE pBuf,
              ULONG *count);

APIRET FSWrite(HFILE hFile,
               PBYTE pBuf,
               ULONG *count);

APIRET FSSetFilePtrL(HFILE hFile,
                     LONGLONG ib,
                     ULONG method,
                     ULONGLONG *ibActual);

APIRET FSClose(HFILE hFile);

APIRET FSQueryHType(HFILE hFile,
                    ULONG *pType,
                    ULONG *pAttr);

APIRET FSOpenL(PSZ pszFileName,
               HFILE *phFile,
               ULONG *pulAction,
               LONGLONG cbFile,
               ULONG ulAttribute,
               ULONG fsOpenFlags,
               ULONG fsOpenMode,
               EAOP2 *peaop2);

APIRET FSDupHandle(HFILE hFile,
                   HFILE *phFile2);

APIRET FSDelete(PSZ pszFileName);

APIRET FSForceDelete(PSZ pszFileName);

APIRET FSDeleteDir(PSZ pszDirName);

APIRET FSCreateDir(PSZ pszDirName,
                   EAOP2 *peaop2);

APIRET FSFindFirst(PSZ pszFileSpec,
                   HDIR *phDir,
                   ULONG flAttribute,
                   char **pFindBuf,
                   ULONG *cbBuf,
                   ULONG *pcFileNames,
                   ULONG ulInfoLevel);

APIRET FSFindNext(HDIR hDir,
                  char **pFindBuf,
                  ULONG *cbBuf,
                  ULONG *pcFileNames);

APIRET FSFindClose(HDIR hDir);

APIRET FSQueryFHState(HFILE hFile,
                      ULONG *pMode);

APIRET FSSetFHState(HFILE hFile,
                    ULONG mode);

APIRET FSQueryFileInfo(HFILE hFile,
                       ULONG ulInfoLevel,
                       char **pInfo,
                       ULONG *cbInfoBuf);

APIRET FSQueryPathInfo(PSZ pszPathName,
                       ULONG ulInfoLevel,
                       char **pInfo,
                       ULONG *cbInfoBuf);

APIRET FSResetBuffer(HFILE hFile);

APIRET FSSetFileSizeL(HFILE hFile,
                      LONGLONG cbSize);

APIRET FSSetFileInfo(HFILE hFile,
                     ULONG ulInfoLevel,
                     char **pInfoBuf,
                     ULONG *cbInfoBuf);

APIRET FSSetPathInfo(PSZ pszPathName,
                     ULONG ulInfoLevel,
                     char **pInfoBuf,
                     ULONG *cbInfoBuf,
                     ULONG flOptions);

APIRET FSGetDriveMap(ULONG *map);

APIRET FSFileProv(PSZ fname,
                  l4_os3_cap_idx_t *dm,
                  ULONG flags,
                  l4_os3_dataspace_t *ds,
                  ULONG *size);

#ifdef __cplusplus
  }
#endif

#endif /* __OS2FS_API_API_H__ */
