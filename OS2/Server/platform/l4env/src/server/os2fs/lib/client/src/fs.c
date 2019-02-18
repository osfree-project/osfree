/* os2fs client-side RPC API (l4env platform) */

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/io.h>
#include <os3/types.h>
#include <os3/thread.h>
#include <os3/fs.h>

/* l4env includes */
#include <l4/names/libnames.h>

/* os2fs RPC includes */
#include <os2fs-client.h>

static l4_threadid_t fs;

APIRET FSClientInit(l4_os3_thread_t *thread)
{
    if (! names_waitfor_name("os2fs", &fs, 30000))
    {
        io_log("os2fs not found at the name server!\n");
        return ERROR_FILE_NOT_FOUND;
    }

    thread->thread = fs;

    return NO_ERROR;
}

APIRET FSClientDone(void)
{
    return NO_ERROR;
}

long FSClientGetDriveMap(ULONG *map)
{
    CORBA_Environment env = dice_default_environment;
    long ret;

    ret = os2fs_get_drivemap_call(&fs, map, &env);
    return ret;
}

APIRET FSClientRead(HFILE hFile,
                    char *pBuf,
                    ULONG cbRead,
                    ULONG *pcbReadActual)
{
    CORBA_Environment env = dice_default_environment;
    APIRET rc;

    *pcbReadActual = cbRead;
    rc = os2fs_dos_Read_call(&fs, hFile, &pBuf, pcbReadActual, &env);
    return rc;
}

APIRET FSClientWrite(HFILE hFile,
                     char *pBuf,
                     ULONG cbWrite,
                     ULONG *pcbWrittenActual)
{
    CORBA_Environment env = dice_default_environment;
    APIRET rc;

    *pcbWrittenActual = cbWrite;
    rc = os2fs_dos_Write_call(&fs, hFile, pBuf, pcbWrittenActual, &env);
    return rc;
}

APIRET FSClientResetBuffer(HFILE hFile)
{
    CORBA_Environment env = dice_default_environment;
    APIRET rc;

    rc = os2fs_dos_ResetBuffer_call(&fs, hFile, &env);
    return rc;
}

APIRET FSClientSetFilePtrL(HFILE hFile,
                           LONGLONG ib,
                           ULONG method,
                           ULONGLONG *ibActual)
{
    CORBA_Environment env = dice_default_environment;
    APIRET rc;

    rc = os2fs_dos_SetFilePtrL_call(&fs, hFile, ib,
                                    method, ibActual, &env);
    return rc;
}

APIRET FSClientClose(HFILE hFile)
{
    CORBA_Environment env = dice_default_environment;
    APIRET rc;

    rc = os2fs_dos_Close_call(&fs, hFile, &env);
    return rc;
}

APIRET FSClientQueryHType(HFILE hFile,
                          ULONG *pType,
                          ULONG *pAttr)
{
    CORBA_Environment env = dice_default_environment;
    APIRET rc;

    rc = os2fs_dos_QueryHType_call(&fs, hFile,
                                   pType, pAttr, &env);
    return rc;
}

APIRET FSClientOpenL(PSZ pszFileName,
                     HFILE *phFile,
                     ULONG *pcbAction,
                     LONGLONG cbSize,
                     ULONG ulAttribute,
                     ULONG fsOpenFlags,
                     ULONG fsOpenMode,
                     EAOP2 *peaop2)
{
    CORBA_Environment env = dice_default_environment;
    EAOP2 eaop2;
    APIRET rc;

    memset(&eaop2, 0, sizeof(EAOP2));

    if (! peaop2)
    {
        peaop2 = &eaop2;
    }

    rc = os2fs_dos_OpenL_call(&fs, pszFileName, phFile, pcbAction,
                              cbSize, ulAttribute, fsOpenFlags,
                              fsOpenMode, peaop2, &env);
    return rc;
}

APIRET FSClientDupHandle(HFILE hFile,
                         HFILE *phFile2)
{
    CORBA_Environment env = dice_default_environment;
    APIRET rc;

    rc = os2fs_dos_DupHandle_call(&fs, hFile, phFile2, &env);
    return rc;
}

APIRET FSClientDelete(PSZ pszFileName)
{
    CORBA_Environment env = dice_default_environment;
    APIRET rc;

    rc = os2fs_dos_Delete_call(&fs, pszFileName, &env);
    return rc;
}

APIRET FSClientForceDelete(PSZ pszFileName)
{
    CORBA_Environment env = dice_default_environment;
    APIRET rc;

    rc = os2fs_dos_ForceDelete_call(&fs, pszFileName, &env);
    return rc;
}

APIRET FSClientDeleteDir(PSZ pszDirName)
{
    CORBA_Environment env = dice_default_environment;
    APIRET rc;

    rc = os2fs_dos_DeleteDir_call(&fs, pszDirName, &env);
    return rc;
}

APIRET FSClientCreateDir(PSZ pszDirName,
                         EAOP2 *peaop2)
{
    CORBA_Environment env = dice_default_environment;
    APIRET rc;

    rc = os2fs_dos_CreateDir_call(&fs, pszDirName, peaop2, &env);
    return rc;
}

APIRET FSClientFindFirst(PSZ pszFileSpec,
                         HDIR *phDir,
                         ULONG ulAttribute,
                         char *pFindBuf,
                         ULONG *cbBuf,
                         ULONG *pcFileNames,
                         ULONG ulInfoLevel)
{
    CORBA_Environment env = dice_default_environment;
    APIRET rc;

    rc = os2fs_dos_FindFirst_call(&fs, pszFileSpec, phDir,
                                  ulAttribute, &pFindBuf, cbBuf,
                                  pcFileNames, ulInfoLevel, &env);
    io_log("FSClientFindFirst: rc=%lx\n", rc);
    return rc;
}

APIRET FSClientFindNext(HDIR hDir,
                        char *pFindBuf,
                        ULONG *cbBuf,
                        ULONG *pcFileNames)
{
    CORBA_Environment env = dice_default_environment;
    APIRET rc;

    rc = os2fs_dos_FindNext_call(&fs, hDir,
                                 &pFindBuf, cbBuf,
                                 pcFileNames, &env);
    return rc;
}

APIRET FSClientFindClose(HDIR hDir)
{
    CORBA_Environment env = dice_default_environment;
    APIRET rc;

    rc = os2fs_dos_FindClose_call(&fs, hDir, &env);
    return rc;
}

APIRET FSClientQueryFHState(HFILE hFile,
                            ULONG *pulMode)
{
    CORBA_Environment env = dice_default_environment;
    APIRET rc;

    rc = os2fs_dos_QueryFHState_call(&fs, hFile, pulMode, &env);
    return rc;
}

APIRET FSClientSetFHState(HFILE hFile,
                          ULONG ulMode)
{
    CORBA_Environment env = dice_default_environment;
    APIRET rc;

    rc = os2fs_dos_SetFHState_call(&fs, hFile, ulMode, &env);
    return rc;
}

APIRET FSClientQueryFileInfo(HFILE hFile,
                             ULONG ulInfoLevel,
                             char *pInfoBuf,
                             ULONG *cbInfoBuf)
{
    CORBA_Environment env = dice_default_environment;
    APIRET rc;

    rc = os2fs_dos_QueryFileInfo_call(&fs, hFile, ulInfoLevel,
                                      &pInfoBuf, cbInfoBuf, &env);
    return rc;
}

APIRET FSClientQueryPathInfo(PSZ pszPathName,
                             ULONG ulInfoLevel,
                             char *pInfoBuf,
                             ULONG *cbInfoBuf)
{
    CORBA_Environment env = dice_default_environment;
    APIRET rc;

    rc = os2fs_dos_QueryPathInfo_call(&fs, pszPathName, ulInfoLevel,
                                      &pInfoBuf, cbInfoBuf, &env);
    return rc;
}

APIRET FSClientSetFileSizeL(HFILE hFile,
                            LONGLONG cbSize)
{
    CORBA_Environment env = dice_default_environment;
    APIRET rc;

    rc = os2fs_dos_SetFileSizeL_call(&fs, hFile, cbSize, &env);
    return rc;
}

APIRET FSClientSetFileInfo(HFILE hFile,
                           ULONG ulInfoLevel,
                           char *pInfoBuf,
                           ULONG *cbInfoBuf)
{
    CORBA_Environment env = dice_default_environment;
    APIRET rc;

    rc = os2fs_dos_SetFileInfo_call(&fs, hFile, ulInfoLevel,
                                    &pInfoBuf, cbInfoBuf, &env);
    return rc;
}

APIRET FSClientSetPathInfo(PSZ pszPathName,
                           ULONG ulInfoLevel,
                           char *pInfoBuf,
                           ULONG *cbInfoBuf,
                           ULONG flOptions)
{
    CORBA_Environment env = dice_default_environment;
    APIRET rc;

    rc = os2fs_dos_SetPathInfo_call(&fs, pszPathName, ulInfoLevel,
                                    &pInfoBuf, cbInfoBuf, flOptions, &env);
    return rc;
}
