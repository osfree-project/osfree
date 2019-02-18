#ifndef __OS3_FS_H__
#define __OS3_FS_H__

#ifdef __cplusplus
  extern "C" {
#endif

/* osFree internal */
#include <os3/thread.h>

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

APIRET FSClientInit(l4_os3_thread_t *thread);

APIRET FSClientDone(void);

long FSClientGetDriveMap(ULONG *map);

APIRET FSClientRead(HFILE hFile,
                    char *pBuf,
                    ULONG cbRead,
                    ULONG *pcbReadActual);

APIRET FSClientWrite(HFILE hFile,
                     char *pBuf,
                     ULONG cbWrite,
                     ULONG *pcbWrittenActual);

APIRET FSClientResetBuffer(HFILE hFile);

APIRET FSClientSetFilePtrL(HFILE hFile,
                           LONGLONG ib,
                           ULONG method,
                           ULONGLONG *ibActual);

APIRET FSClientClose(HFILE hFile);

APIRET FSClientQueryHType(HFILE hFile,
                          ULONG *pType,
                          ULONG *pAttr);

APIRET FSClientOpenL(PSZ pszFileName,
                     HFILE *phFile,
                     ULONG *pulAction,
                     LONGLONG cbSize,
                     ULONG ulAttribute,
                     ULONG fsOpenFlags,
                     ULONG fsOpenMode,
                     EAOP2 *peaop2);

APIRET FSClientDupHandle(HFILE hFile,
                         HFILE *phFile);

APIRET FSClientDelete(PSZ pszFileName);

APIRET FSClientForceDelete(PSZ pszFileName);

APIRET FSClientDeleteDir(PSZ pszDirName);

APIRET FSClientCreateDir(PSZ pszDirName,
                         EAOP2 *peaop2);

APIRET FSClientFindFirst(PSZ pszPathSpec,
                         HDIR *phDir,
                         ULONG ulAttribute,
                         char *pFindBuf,
                         ULONG *cbBuf,
                         ULONG *pcFileNames,
                         ULONG ulInfoLevel);

APIRET FSClientFindNext(HDIR hDir,
                        char *pFindBuf,
                        ULONG *cbBuf,
                        ULONG *pcFileNames);

APIRET FSClientFindClose(HDIR hDir);

APIRET FSClientQueryFHState(HFILE hFile,
                            ULONG *pulMode);

APIRET FSClientSetFHState(HFILE hFile,
                          ULONG ulMode);

APIRET FSClientQueryFileInfo(HFILE hFile,
                             ULONG ulInfoLevel,
                             char *pInfoBuf,
                             ULONG *cbInfoBuf);

APIRET FSClientQueryPathInfo(PSZ pszPathName,
                             ULONG ulInfoLevel,
                             char *pInfoBuf,
                             ULONG *cbInfoBuf);

APIRET FSClientSetFileSizeL(HFILE hFile,
                            LONGLONG cbSize);

APIRET FSClientSetFileInfo(HFILE hFile,
                           ULONG ulInfoLevel,
                           char *pInfoBuf,
                           ULONG *cbInfoBuf);

APIRET FSClientSetPathInfo(PSZ pszPathName,
                           ULONG ulInfoLevel,
                           char *pInfoBuf,
                           ULONG *cbInfoBuf,
                           ULONG flOptions);

#ifdef __cplusplus
  }
#endif

#endif /* __OS3_FS_H__ */
