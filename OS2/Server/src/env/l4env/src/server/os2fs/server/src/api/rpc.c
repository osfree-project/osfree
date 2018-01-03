/*  file/directory API implementations
 *  (on the server side)
 */

/* osFree internal */
#include <os3/MountReg.h>
#include <os3/globals.h>
#include <os3/io.h>

/* libc includes */
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

/* l4env includes */
#include <l4/dm_mem/dm_mem.h>
#include <l4/dm_generic/consts.h>

/* dice */
#include <dice/dice.h>

/* servers RPC includes */
#include "os2fs-server.h"

/* local includes */
#include "api.h"

void DosNameConversion(char *pszName);

APIRET DICE_CV
os2fs_dos_Read_component(CORBA_Object _dice_corba_obj,
                         HFILE hFile, char **pBuffer,
                         ULONG *count,
                         short *dice_reply,
                         CORBA_Server_Environment *_dice_corba_env)
{
  return FSRead(hFile, *pBuffer, count);
}

APIRET DICE_CV
os2fs_dos_Write_component(CORBA_Object _dice_corba_obj,
                          HFILE handle, const char *pBuffer,
                          ULONG *count,
                          short *dice_reply,
                          CORBA_Server_Environment *_dice_corba_env)
{
  return FSWrite(handle, (PBYTE)pBuffer, count);
}

APIRET DICE_CV
os2fs_dos_SetFilePtrL_component (CORBA_Object _dice_corba_obj,
                                    HFILE handle /* in */,
                                    long long ib /* in */,
                                    ULONG method /* in */,
                                    unsigned long long *ibActual /* out */,
                                    CORBA_Server_Environment *_dice_corba_env)
{
  return FSSetFilePtrL(handle, ib, method, ibActual);
}

APIRET DICE_CV
os2fs_dos_Close_component (CORBA_Object _dice_corba_obj,
                               HFILE handle /* in */,
                               CORBA_Server_Environment *_dice_corba_env)
{
  return FSClose(handle);
}

APIRET DICE_CV
os2fs_dos_QueryHType_component (CORBA_Object _dice_corba_obj,
                                    HFILE handle /* in */,
                                    ULONG *pType /* out */,
                                    ULONG *pAttr /* out */,
                                    CORBA_Server_Environment *_dice_corba_env)
{
  return FSQueryHType(handle, pType, pAttr);
}

APIRET DICE_CV
os2fs_dos_OpenL_component (CORBA_Object _dice_corba_obj,
                           const char* pszFileName /* in */,
                           HFILE *phFile /* out */,
                           ULONG *pulAction /* out */,
                           long long cbFile /* in */,
                           ULONG ulAttribute /* in */,
                           ULONG fsOpenFlags /* in */,
                           ULONG fsOpenMode /* in */,
                           EAOP2 *peaop2 /* out */,
                           CORBA_Server_Environment *_dice_corba_env)
{
  return FSOpenL((PSZ)pszFileName,
                 phFile,
                 pulAction,
                 cbFile,
                 ulAttribute,
                 fsOpenFlags,
                 fsOpenMode,
                 peaop2);
}

APIRET DICE_CV
os2fs_dos_DupHandle_component (CORBA_Object _dice_corba_obj,
                               HFILE hFile /* in */,
                               HFILE *phFile2 /* in, out */,
                               CORBA_Server_Environment *_dice_corba_env)
{
  return FSDupHandle(hFile, phFile2);
}

APIRET DICE_CV
os2fs_dos_Delete_component (CORBA_Object _dice_corba_obj,
                            const char* pszFileName /* in */,
                            CORBA_Server_Environment *_dice_corba_env)
{
   return FSDelete((PSZ)pszFileName);
}


APIRET DICE_CV
os2fs_dos_ForceDelete_component (CORBA_Object _dice_corba_obj,
                                 const char* pszFileName /* in */,
                                 CORBA_Server_Environment *_dice_corba_env)
{
   return FSForceDelete((PSZ)pszFileName);
}

APIRET DICE_CV
os2fs_dos_DeleteDir_component (CORBA_Object _dice_corba_obj,
                               const char* pszDirName /* in */,
                               CORBA_Server_Environment *_dice_corba_env)
{
  return FSDeleteDir((PSZ)pszDirName);
}

APIRET DICE_CV
os2fs_dos_CreateDir_component (CORBA_Object _dice_corba_obj,
                               const char* pszDirName /* in */,
                               const EAOP2 *peaop2 /* in */,
                               CORBA_Server_Environment *_dice_corba_env)
{
  return FSCreateDir((PSZ)pszDirName, peaop2);
}

APIRET DICE_CV
os2fs_dos_FindFirst_component (CORBA_Object _dice_corba_obj,
                               const char* pszFileSpec /* in */,
                               HDIR  *phDir /* out */,
                               ULONG flAttribute /* in */,
                               char  **pFindBuf /* in, out */,
                               ULONG *cbBuf /* in, out */,
                               ULONG *pcFileNames /* in, out */,
                               ULONG ulInfolevel /* in */,
                               CORBA_Server_Environment *_dice_corba_env)
{
  return FSFindFirst((PSZ)pszFileSpec,
                     phDir, flAttribute,
                     pFindBuf, cbBuf, pcFileNames,
                     ulInfolevel);
}

APIRET DICE_CV
os2fs_dos_FindNext_component (CORBA_Object _dice_corba_obj,
                              HDIR  hDir /* in */,
                              char  **pFindBuf /* in, out */,
                              ULONG *cbBuf /* in, out */,
                              ULONG *pcFileNames /* in, out */,
                              CORBA_Server_Environment *_dice_corba_env)
{
  return FSFindNext(hDir, pFindBuf, cbBuf, pcFileNames);
}

APIRET DICE_CV
os2fs_dos_FindClose_component (CORBA_Object _dice_corba_obj,
                               HDIR hDir /* in */,
                               CORBA_Server_Environment *_dice_corba_env)
{
  return FSFindClose(hDir);
}

APIRET DICE_CV
os2fs_dos_QueryFHState_component(CORBA_Object _dice_corba_obj,
                                 HFILE hFile,
                                 PULONG pMode,
                                 CORBA_Server_Environment *_dice_corba_env)
{
  return FSQueryFHState(hFile, pMode);
}

APIRET DICE_CV
os2fs_dos_SetFHState_component (CORBA_Object _dice_corba_obj,
                                HFILE hFile /* in */,
                                ULONG ulMode /* in */,
                                CORBA_Server_Environment *_dice_corba_env)
{
  return FSSetFHState(hFile, ulMode);
}

APIRET DICE_CV
os2fs_dos_QueryFileInfo_component (CORBA_Object _dice_corba_obj,
                                   HFILE hf /* in */,
                                   ULONG ulInfoLevel /* in */,
                                   char **pInfo /* in, out */,
                                   ULONG *cbInfoBuf /* in, out */,
                                   CORBA_Server_Environment *_dice_corba_env)
{
    return FSQueryFileInfo(hf, ulInfoLevel, pInfo, cbInfoBuf);
}

APIRET DICE_CV
os2fs_dos_QueryPathInfo_component (CORBA_Object _dice_corba_obj,
                                   const char* pszPathName /* in */,
                                   ULONG ulInfoLevel /* in */,
                                   char **pInfo /* in, out */,
                                   ULONG *cbInfoBuf /* in, out */,
                                   CORBA_Server_Environment *_dice_corba_env)
{
    return FSQueryPathInfo((PSZ)pszPathName, ulInfoLevel,
                            pInfo, cbInfoBuf);
}

APIRET DICE_CV
os2fs_dos_ResetBuffer_component (CORBA_Object _dice_corba_obj,
                                 HFILE hFile /* in */,
                                 CORBA_Server_Environment *_dice_corba_env)
{
  return FSResetBuffer(hFile);
}

APIRET DICE_CV
os2fs_dos_SetFileSizeL_component (CORBA_Object _dice_corba_obj,
                                  HFILE hFile /* in */,
                                  long long cbSize /* in */,
                                  CORBA_Server_Environment *_dice_corba_env)
{
  return FSSetFileSizeL(hFile, cbSize);
}

/* 1) no EA support yet :( 
 * 2) only times are changed
 */
APIRET DICE_CV
os2fs_dos_SetFileInfo_component (CORBA_Object _dice_corba_obj,
                                 HFILE hf /* in */,
                                 ULONG ulInfoLevel /* in */,
                                 char **pInfoBuf /* in, out */,
                                 ULONG *cbInfoBuf /* in, out */,
                                 CORBA_Server_Environment *_dice_corba_env)
{
  return FSSetFileInfo(hf, ulInfoLevel, pInfoBuf, cbInfoBuf);
}

/* No EA support yet :( */
APIRET DICE_CV
os2fs_dos_SetPathInfo_component (CORBA_Object _dice_corba_obj,
                                 const char* pszPathName /* in */,
                                 ULONG ulInfoLevel /* in */,
                                 char **pInfoBuf /* in, out */,
                                 ULONG *cbInfoBuf /* in, out */,
                                 ULONG flOptions /* in */,
                                 CORBA_Server_Environment *_dice_corba_env)
{
  return FSSetPathInfo((PSZ)pszPathName,
                       ulInfoLevel, pInfoBuf,
                       cbInfoBuf, flOptions);
}

long DICE_CV
os2fs_get_drivemap_component (CORBA_Object _dice_corba_obj,
                              ULONG *map /* out */,
                              CORBA_Server_Environment *_dice_corba_env)
{
  return FSGetDriveMap(map);
}

/*  File provider proxy
 *  (gives a file as a dataspace
 *  by its OS/2-style pathname)
 *  -- is a frontend to another file provider
 *  or l4vfs.
 */

long DICE_CV
l4fprov_file_open_component (CORBA_Object _dice_corba_obj,
                      const char* fname /* in */,
                      const l4_threadid_t *dm /* in */,
                      unsigned long flags /* in */,
                      l4dm_dataspace_t *ds /* out */,
                      l4_size_t *size /* out */,
                      CORBA_Server_Environment *_dice_corba_env)
{
  struct stat stat;
  l4_addr_t addr;
  char *newfilename;
  int  handle;
  int  rc;

  /* convert OS/2 path to PN path */
  io_log("fname=%s\n", fname);
  if (pathconv(&newfilename, (char *)fname))
   return ERROR_FILE_NOT_FOUND;

  io_log("newfilename=%s\n", newfilename);
  handle = open(newfilename, O_RDONLY);

  io_log("000\n");
  if(handle == -1)
      return ERROR_FILE_NOT_FOUND;

  io_log("file opened\n");

  io_log("001\n");
  rc = fstat(handle, &stat);

  /* get length */
  io_log("002\n");
  *size = stat.st_size;

  /* Create a dataspace of a given size */
  io_log("002\n");
  rc = l4dm_mem_open((l4_threadid_t)*dm, *size, 0, L4DM_RW, "fprov dataspace", ds);

  io_log("003\n");
  if (rc < 0)
    return ERROR_NOT_ENOUGH_MEMORY;

  /* attach the created dataspace to our address space */
  io_log("004\n");
  rc = l4rm_attach(ds, *size, 0, L4DM_RW, (void **)&addr);

  io_log("005\n");
  if (rc < 0)
    return 8; /* What to return? */

  io_log("006\n");
  read(handle, (void *)addr, *size);

  io_log("007\n");
  close(handle);

  io_log("008\n");
  l4rm_detach((void *)addr);

  io_log("caller=%x.%x\n", _dice_corba_obj->id.task, _dice_corba_obj->id.lthread);
  rc = l4dm_transfer(ds, *_dice_corba_obj);

  io_log("009\n");
  if (rc < 0)
  {
    io_log("error transferring ds\n");
    l4dm_close(ds);
    return ERROR_ACCESS_DENIED;
  }

  io_log("successful return\n");

  return NO_ERROR;
}
