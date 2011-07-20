/*  file/directory API implementations
 *  (on the server side)
 */
 
#include <l4/os3/gcc_os2def.h>
#include <l4/os3/MountReg.h>

#include <sys/types.h>
#include <sys/time.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <glob.h>
#include <time.h>
#include <utime.h>

#include <l4/log/l4log.h>

#include <dice/dice.h>

#include "os2fs-server.h"

extern l4_threadid_t os2srv;

typedef struct
{
  HFILE hfile;
  char id[4];
  ULONG openmode;
  char filename[256];
} filehandle_t;

// glob_t 
static glob_t thehdir;
// counter
static int ctr = 0;

int pathconv(char **converted, char *fname);

APIRET DICE_CV
os2fs_dos_Read_component(CORBA_Object _dice_corba_obj,
                             HFILE hFile, char **pBuffer,
                             ULONG *count,
                             short *dice_reply,
                             CORBA_Server_Environment *_dice_corba_env)
{
  int  c;
  int  nread = 0;
  int  total = 1;
  filehandle_t *h;
  HFILE handle;
  
  if (hFile != 0)
  {
    h = (filehandle_t *)hFile;
    handle = h->hfile;    
  }
  else
    handle = hFile;    

  nread = read(handle, (char *)*pBuffer, *count);
  if (nread == -1)
  {
    LOG("read() error, errno=%d", errno);
    switch (errno)
    {
      // @todo: more accurate error handling
      default:
        return 232; //ERROR_NO_DATA
    }
  }
  *count = nread;


  return 0; // NO_ERROR
}


APIRET DICE_CV
os2fs_dos_Write_component(CORBA_Object _dice_corba_obj,
                              HFILE handle, const char *pBuffer,
                              ULONG *count,
                              short *dice_reply,
	 		      CORBA_Server_Environment *_dice_corba_env)
{
  char *s;
  int  nwritten;
  filehandle_t *h;
  HFILE hdl;
  
  if (handle != 0 && handle != 1 && handle != 2)
  {
    h = (filehandle_t *)handle;
    hdl = h->hfile;    
  }
  else
    hdl = handle;    
  
  LOG("entered");
  nwritten = write(hdl, pBuffer, *count);
  LOG("in the middle");
  if (nwritten == -1)
  {
    LOG("write() error, errno=%d", errno);
    switch (errno)
    {
      // @todo: more accurate error handling
      default:
        return 232; //ERROR_NO_DATA
    }
  }

  LOG("nwritten=%u", nwritten);
  *count = nwritten;
  LOG("exited");
  return 0/*NO_ERROR*/;
}


#define FILE_BEGIN   0
#define FILE_CURRENT 1
#define FILE_END     2

APIRET DICE_CV
os2fs_dos_SetFilePtrL_component (CORBA_Object _dice_corba_obj,
                                    HFILE handle /* in */,
                                    long long ib /* in */,
                                    ULONG method /* in */,
                                    long long *ibActual /* out */,
                                    CORBA_Server_Environment *_dice_corba_env)
{
  long ret;
  long long pos;
  long long len;
  struct stat stat;
  filehandle_t *h;
  HFILE hdl;
  
  if (handle != 0 && handle != 1 && handle != 2)
  {
    h = (filehandle_t *)handle;
    hdl = h->hfile;    
  }
  else
    hdl = handle;    
  
  ret = fstat(hdl, &stat);

  if (ret == -1)
    return 6; /* ERROR_INVALID_HANDLE */

  /* get length */
  len = stat.st_size;
  /* get current position */
  pos = lseek(hdl, 0L, SEEK_CUR);

  switch (method)
  {
  case FILE_BEGIN:
    pos = ib;
    break;
  case FILE_CURRENT:
    pos += ib;
    break;
  case FILE_END:
    pos = len + ib;
    break;
  default:
    pos = ib;
  }

  if (pos < 0)
    return 131; /* ERROR_NEGATIVE_SEEK */

  *ibActual = pos;
    
  ret = lseek(hdl, pos, SEEK_SET);

  if (ret == -1)
    return 132; /* ERROR_SEEK_ON_DEVICE */

  return 0; /* NO_ERROR */
}

APIRET DICE_CV
os2fs_dos_Close_component (CORBA_Object _dice_corba_obj,
                               HFILE handle /* in */,
                               CORBA_Server_Environment *_dice_corba_env)
{
  int ret;
  filehandle_t *h;
  HFILE hdl;
  
  if (handle != 0 && handle != 1 && handle != 2)
  {
    h = (filehandle_t *)handle;
    hdl = h->hfile;    
  }
  else
    hdl = handle;    
  
  ret = close(hdl);
  
  if (ret == - 1)
  {
    if (errno == EBADF)
      return 6; /* ERROR_INVALID_HANDLE */
      
    return 5; /* ERROR_ACCESS_DENIED */
  }
  
  return 0; /* NO_ERROR */
}


APIRET DICE_CV
os2fs_dos_QueryHType_component (CORBA_Object _dice_corba_obj,
                                    HFILE handle /* in */,
                                    ULONG *pType /* out */,
                                    ULONG *pAttr /* out */,
                                    CORBA_Server_Environment *_dice_corba_env)
{
  int ret;
  unsigned short m;
  struct stat stat;
  filehandle_t *h;
  HFILE hdl;
  
  if (handle != 0 && handle != 1 && handle != 2)
  {
    h = (filehandle_t *)handle;
    hdl = h->hfile;    
  }
  else
    hdl = handle;    
  
  if (isatty(hdl))
  {
    *pType = 1; // character device
    *pAttr = 0;
    return 0;
  }
  
  ret = fstat(hdl, &stat);

  if (ret == -1)
    return 6; /* ERROR_INVALID_HANDLE */

  m = stat.st_mode;

  if (S_ISREG(m))
  {
    *pType = 0; // disk file
    *pAttr = 0;
    return 0;
  }

  if (S_ISCHR(m))
  {
    *pType = 1; // character device
    *pAttr = 0;
    return 0;
  }

  if (S_ISFIFO(m))
  {
    *pType = 2; // pipe
    *pAttr = 0;
    return 0;
  }
  
  return 0; /* NO_ERROR */
}

/* pulAction */
#define FILE_EXISTED   1
#define FILE_CREATED   2
#define FILE_TRUNCATED 3

/* ulAttribute */
#define FILE_ARCHIVED  0x00000020
#define FILE_DIRECTORY 0x00000010
#define FILE_SYSTEM    0x00000004
#define FILE_HIDDEN    0x00000002
#define FILE_READONLY  0x00000001
#define FILE_NORMAL    0x00000000

/* fsOpenFlags */
#define OPEN_ACTION_FAIL_IF_EXISTS     0x00000000
#define OPEN_ACTION_OPEN_IF_EXISTS     0x00000001
#define OPEN_ACTION_REPLACE_IF_EXISTS  0x00000010

#define OPEN_ACTION_FAIL_IF_NEW        0x00000000
#define OPEN_ACTION_CREATE_IF_NEW      0x00010000

/* fsOpenMode */
#define OPEN_ACCESS_READONLY           0x00000000
#define OPEN_ACCESS_WRITEONLY          0x00000001
#define OPEN_ACCESS_READWRITE          0x00000002

#define OPEN_SHARE_DENYREADWRITE       0x00000010
#define OPEN_SHARE_DENYWRITE           0x00000020
#define OPEN_SHARE_DENYREAD            0x00000030
#define OPEN_SHARE_DENYNONE            0x00000040

#define OPEN_FLAGS_NOINHERIT           0x00000080

#define OPEN_FLAGS_NO_LOCALITY         0x00000000
#define OPEN_FLAGS_SEQUENTIAL          0x00000100
#define OPEN_FLAGS_RANDOM              0x00000200
#define OPEN_FLAGS_RANDOMSEQUENTIAL    0x00000300

#define OPEN_FLAGS_NO_CACHE            0x00001000

#define OPEN_FLAGS_FAIL_ON_ERROR       0x00002000

#define OPEN_FLAGS_WRITE_THROUGH       0x00004000

#define OPEN_FLAGS_DASD                0x00008000

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
  filehandle_t *h;
  struct stat st;
  char file_existed = 0;
  char *newfilename;
  int mode = 0; //O_BINARY;
  int handle;

  /* ignore DASD opens for now; also, OPEN_FLAGS_FAIL_ON_ERROR
     always for now (before Hard error handling not implemented) */
  if (fsOpenMode & OPEN_FLAGS_DASD)
    return 5; /* ERROR_ACCESS_DENIED */

  if (fsOpenMode & OPEN_ACCESS_READONLY)
    mode |= O_RDONLY;

  if (fsOpenMode & OPEN_ACCESS_WRITEONLY)
    mode |= O_WRONLY;

  if (fsOpenMode & OPEN_ACCESS_READWRITE)
    mode |= O_RDWR;

  //if (fsOpenMode & OPEN_FLAGS_NOINHERIT)
  //    mode |= O_NOINHERIT;

  if (fsOpenMode & OPEN_SHARE_DENYREAD)
    mode |= O_RDWR | O_EXCL;

  if (fsOpenMode & OPEN_SHARE_DENYWRITE)
    mode |= O_WRONLY | O_EXCL;

  if (fsOpenMode & OPEN_SHARE_DENYREADWRITE)
    mode |= O_RDONLY | O_EXCL;

  if (fsOpenMode & OPEN_SHARE_DENYNONE)
    mode &= ~O_EXCL;

  if (fsOpenFlags & OPEN_ACTION_CREATE_IF_NEW)
    mode |= O_CREAT;

  if (fsOpenFlags & OPEN_ACTION_REPLACE_IF_EXISTS)
    mode |= O_TRUNC;

  /* convert OS/2-style pathname to PN-style pathname */
  if (pathconv(&newfilename, pszFileName))
    return 3; /* ERROR_PATH_NOT_FOUND */

  if (strlen(newfilename) + 1 > 256)
    return ERROR_FILENAME_EXCED_RANGE;

  if (!stat(newfilename, &st))
    file_existed = 1;

  handle = open(newfilename, mode);

  if (handle == -1)
  {
    if (errno == EACCES)
      return 5; /* ERROR_ACCESS_DENIED */

    if (errno == EMFILE)
      return 4; /* ERROR_TOO_MANY_OPEN_FILES */
      
    if (errno == ENOENT)
      return 2; /* ERROR_FILE_NOT_FOUND */
  }

  if (fsOpenFlags & OPEN_ACTION_FAIL_IF_EXISTS)
  {
    close(handle);
    return 110; /* ERROR_OPEN_FAILED */
  }

  if (fsOpenFlags & OPEN_ACTION_OPEN_IF_EXISTS)
  {
    if (file_existed && (mode & ~O_TRUNC))
      *pulAction = FILE_EXISTED;
    else if (file_existed && (mode & O_TRUNC))
      *pulAction = FILE_TRUNCATED;
    else if (!file_existed)
      *pulAction = FILE_CREATED;
  }
  
  h = (filehandle_t *)malloc(sizeof(filehandle_t));
  h->openmode = fsOpenMode;
  h->hfile = handle;

  strcpy(h->filename, newfilename);

  *phFile = (HFILE)h;

  
  return 0; /* NO_ERROR */
}

APIRET DICE_CV
os2fs_dos_DupHandle_component (CORBA_Object _dice_corba_obj,
                               HFILE hFile /* in */,
                               HFILE *phFile2 /* in, out */,
                               CORBA_Server_Environment *_dice_corba_env)
{
  struct stat st;
  HFILE handle, hdl;
  APIRET rc;
  filehandle_t *h, *h2;
  
  if (hFile != 0 && hFile != 1 && hFile != 2)
  {
    h = (filehandle_t *)hFile;
    hdl = h->hfile;    
  }
  else
    hdl = hFile;   
  
  if (*phFile2 == -1)
  {
    handle = dup(hdl);
  
    if (handle == -1)
    {
      if (errno == EBADF)
        return 6; /* ERROR_INVALID_HANDLE */
	
      if (errno == EMFILE)
        return 4; /* ERROR_TOO_MANY_OPEN_FILES */
    }
  }
#if 0 // for now dup2 seems to be not implemented
  else
  {
    rc = dup2(hdl, handle);

    if (rc == -1)
    {
      if (errno == EBADF)
      {
        rc = fstat(hFile, &st);
	
	if (rc == -1)
	  return 6;   /* ERROR_INVALID_HANDLE */
	else
	  return 114; /* ERROR_INVALID_TARGET_HANDLE */
      }
      else if (errno == EMFILE)
        return 4; /* ERROR_TOO_MANY_OPEN_FILES */
    }
  }
#else
  return 114; /* ERROR_INVALID_TARGET_HANDLE */
#endif
  h2 = (filehandle_t *)malloc(sizeof(filehandle_t));
  h2->openmode = h->openmode;
  h2->hfile = handle;
  *phFile2 = (HFILE)h2;
  
  return 0; /* NO_ERROR */
}

APIRET DICE_CV
os2fs_dos_Delete_component (CORBA_Object _dice_corba_obj,
                            const char* pszFileName /* in */,
                            CORBA_Server_Environment *_dice_corba_env)
{
  /* For now, it is identical to DosForceDelete,
     no SET DELDIR support yet */

  if (unlink(pszFileName))
  {
    if (errno == ENAMETOOLONG)
      return 206; /* ERROR_FILENAME_EXCED_RANGE */

    if (errno == ENOTDIR)
      return 3; /* ERROR_PATH_NOT_FOUND */
      
    if (errno == EISDIR)
      return 87; /* ERROR_INVALID_PARAMETER */
      
    if (errno == ENOENT)
      return 2; /* ERROR_FILE_NOT_FOUND */
      
    if (errno == EACCES)
      return 5; /* ERROR_ACCESS_DENIED */
      
    if (errno == EPERM)
      return 32; /* ERROR_SHARING_VIOLATION */
   }
   
   return 0; /* NO_ERROR */
}


APIRET DICE_CV
os2fs_dos_ForceDelete_component (CORBA_Object _dice_corba_obj,
                                 const char* pszFileName /* in */,
                                 CORBA_Server_Environment *_dice_corba_env)
{
  if (unlink(pszFileName))
  {
    if (errno == ENAMETOOLONG)
      return 206; /* ERROR_FILENAME_EXCED_RANGE */

    if (errno == ENOTDIR)
      return 3; /* ERROR_PATH_NOT_FOUND */
      
    if (errno == EISDIR)
      return 87; /* ERROR_INVALID_PARAMETER */
      
    if (errno == ENOENT)
      return 2; /* ERROR_FILE_NOT_FOUND */
      
    if (errno == EACCES)
      return 5; /* ERROR_ACCESS_DENIED */
      
    if (errno == EPERM)
      return 32; /* ERROR_SHARING_VIOLATION */
   }
   
   return 0; /* NO_ERROR */
}


APIRET DICE_CV
os2fs_dos_DeleteDir_component (CORBA_Object _dice_corba_obj,
                               const char* pszDirName /* in */,
                               CORBA_Server_Environment *_dice_corba_env)
{
  if (rmdir(pszDirName) == -1)
  {
    if (errno == ENAMETOOLONG)
      return 206; /* ERROR_FILENAME_EXCED_RANGE */
      
    if (errno == ENOTDIR)
      return 3; /* ERROR_PATH_NOT_FOUND */
      
    if (errno == ENOENT)
      return 2; /* ERROR_FILE_NOT_FOUND */
      
    if (errno == EACCES)
      return 5; /* ERROR_ACCESS_DENIED */
      
    if (errno == EPERM)
      return 32; /* ERROR_SHARING_VIOLATION */
    
    if (errno == EBUSY)
      return 16; /* ERROR_CURRENT_DIRECTORY */
      
    if (errno)
      return 5; /* ERROR_ACCESS_DENIED */
  }
  
  return 0; /* NO_ERROR */
}


APIRET DICE_CV
os2fs_dos_CreateDir_component (CORBA_Object _dice_corba_obj,
                               const char* pszDirName /* in */,
                               const EAOP2 *peaop2 /* in */,
                               CORBA_Server_Environment *_dice_corba_env)
{
  EAOP2 eaop2;
  
  if (peaop2 == NULL)
    peaop2 = &eaop2;
  
  if (mkdir(pszDirName, 0) == -1)
  {
    if (errno == ENAMETOOLONG)
      return 206; /* ERROR_FILENAME_EXCED_RANGE */
      
    if (errno == ENOTDIR)
      return 3; /* ERROR_PATH_NOT_FOUND */
    
    if (errno == EACCES)
      return 5; /* ERROR_ACCESS_DENIED */
      
    if (errno == EPERM)
      return 32; /* ERROR_SHARING_VIOLATION */

    if (errno)
      return 5; /* ERROR_ACCESS_DENIED */
  }
  
  return 0; /* NO_ERROR */
}

/* The next three functions were borrowed from os2linux
 * added by valerius, Jul 13, 2011  
 */
/*-- C -----------------------------------------------------------------------*/
/*                                                                            */
/* Module:      filemgr.c                                                     */
/*                                                                            */
/* Description: This file includes the code to support the file manager.      */
/*                                                                            */
/* Copyright (C) IBM Corporation 2003. All Rights Reserved.                   */
/* Copyright (C) W. David Ashley 2004-2010. All Rights Reserved.              */
/*                                                                            */
/* This program and the accompanying materials are made available under       */
/* the terms of the Common Public License v1.0 which accompanies this         */
/* distribution. A copy is also available at the following address:           */
/* http://www.ibm.com/developerworks/oss/CPLv1.0.htm                          */
/*                                                                            */
/* Redistribution and use in source and binary forms, with or                 */
/* without modification, are permitted provided that the following            */
/* conditions are met:                                                        */
/*                                                                            */
/* Redistributions of source code must retain the above copyright             */
/* notice, this list of conditions and the following disclaimer.              */
/* Redistributions in binary form must reproduce the above copyright          */
/* notice, this list of conditions and the following disclaimer in            */
/* the documentation and/or other materials provided with the distribution.   */
/*                                                                            */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS        */
/* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT          */
/* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS          */
/* FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   */
/* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,      */
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,        */
/* OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY     */
/* OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING    */
/* NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS         */
/* SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.               */
/*                                                                            */
/*----------------------------------------------------------------------------*/


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
  CORBA_Environment env = dice_default_environment;
  int   len = 0;
  glob_t *hdir;
  struct stat statbuf;
  struct tm tt;
  char *s, *t, *p;
  char *fname;
  int  rc, i;

  // No support for EA's for the 1st time
  if (ulInfolevel != FIL_STANDARD)
  {
    LOG("ulInfolevel=%d", ulInfolevel);
    return ERROR_INVALID_PARAMETER;
  }
    
  if (*cbBuf == 0)
    return ERROR_BUFFER_OVERFLOW;
    
  if (*pcFileNames * sizeof(FILEFINDBUF3) > *cbBuf)
    return ERROR_BUFFER_OVERFLOW;

  /* convert the filespec to PN format */
  s = strdup(pszFileSpec);
  //LOG("s=%s", s);
  rc = pathconv(&t, s);
  //DosNameConversion(s);
  LOG("rc=%u", rc);
  LOG("t=%s", t);
  
  if (*phDir == HDIR_SYSTEM)
    hdir = &thehdir;
  else if (*phDir == HDIR_CREATE)
    hdir = malloc(sizeof(glob_t));
  else
  {
    LOG("*phDir=%d", *phDir);
    return ERROR_INVALID_PARAMETER;
  }
    
  /* perform the search */    
  hdir->gl_offs = 0;
  rc = glob(t, 0, NULL, hdir);
  free(t);

  LOG("rc=%x", rc);
  
  if (rc)
  {
    if (*phDir == HDIR_CREATE)
      free(hdir);
      
    switch (rc)
    {
    case GLOB_ABORTED:
      return ERROR_NO_MORE_SEARCH_HANDLES;
    case GLOB_NOSPACE:
      return ERROR_NOT_ENOUGH_MEMORY;
    default:
      return ERROR_FILE_NOT_FOUND;
    }
  }
  
  /* fill the FindBuf */
  for (i = 0; i < *pcFileNames; i++, *pFindBuf += sizeof(FILEFINDBUF3))
  {
    if (i == hdir->gl_pathc)
      break;
    
    if (i == *pcFileNames)
      ((PFILEFINDBUF3)*pFindBuf)->oNextEntryOffset = 0;
    else
      ((PFILEFINDBUF3)*pFindBuf)->oNextEntryOffset = sizeof(FILEFINDBUF3);

    fname = hdir->gl_pathv[i];
    // search for the last slash
    for (p = fname + strlen(fname); p > fname && *p != '/'; p--) ;
    if (*p == '/') p++;
    
    ((PFILEFINDBUF3)*pFindBuf)->cchName = strlen(p);
    strcpy(((PFILEFINDBUF3)*pFindBuf)->achName, p);
    
    LOG("filename=%s", hdir->gl_pathv[i]);
    
    /* get file info by stat'ing it */
    rc = stat(hdir->gl_pathv[i], &statbuf);
    localtime_r(&statbuf.st_ctime, &tt);
    
    ((PFILEFINDBUF3)*pFindBuf)->fdateCreation.day =
        (UINT)tt.tm_mday;
    ((PFILEFINDBUF3)*pFindBuf)->fdateCreation.month =
        (UINT)tt.tm_mon + 1;
    ((PFILEFINDBUF3)*pFindBuf)->fdateCreation.year =
        (UINT)tt.tm_year - 80;
    ((PFILEFINDBUF3)*pFindBuf)->ftimeCreation.twosecs =
        (USHORT)tt.tm_sec / 2;
    ((PFILEFINDBUF3)*pFindBuf)->ftimeCreation.minutes =
        (USHORT)tt.tm_min;
    ((PFILEFINDBUF3)*pFindBuf)->ftimeCreation.hours =
        (USHORT)tt.tm_hour;

    localtime_r(&statbuf.st_atime, &tt);

    ((PFILEFINDBUF3)*pFindBuf)->fdateLastAccess.day =
        (UINT)tt.tm_mday;
    ((PFILEFINDBUF3)*pFindBuf)->fdateLastAccess.month =
        (UINT)tt.tm_mon + 1;
    ((PFILEFINDBUF3)*pFindBuf)->fdateLastAccess.year =
        (UINT)tt.tm_year - 80;
    ((PFILEFINDBUF3)*pFindBuf)->ftimeLastAccess.twosecs =
        (USHORT)tt.tm_sec / 2;
    ((PFILEFINDBUF3)*pFindBuf)->ftimeLastAccess.minutes =
        (USHORT)tt.tm_min;
    ((PFILEFINDBUF3)*pFindBuf)->ftimeLastAccess.hours =
        (USHORT)tt.tm_hour;

    localtime_r(&statbuf.st_mtime, &tt);

    ((PFILEFINDBUF3)*pFindBuf)->fdateLastWrite.day =
        (UINT)tt.tm_mday;
    ((PFILEFINDBUF3)*pFindBuf)->fdateLastWrite.month =
        (UINT)tt.tm_mon + 1;
    ((PFILEFINDBUF3)*pFindBuf)->fdateLastWrite.year =
        (UINT)tt.tm_year - 80;
    ((PFILEFINDBUF3)*pFindBuf)->ftimeLastWrite.twosecs =
        (USHORT)tt.tm_sec / 2;
    ((PFILEFINDBUF3)*pFindBuf)->ftimeLastWrite.minutes =
        (USHORT)tt.tm_min;
    ((PFILEFINDBUF3)*pFindBuf)->ftimeLastWrite.hours =
        (USHORT)tt.tm_hour;

    ((PFILEFINDBUF3)*pFindBuf)->cbFile = (ULONG)statbuf.st_size;

    ((PFILEFINDBUF3)*pFindBuf)->cbFileAlloc =
        (ULONG)(statbuf.st_blksize * statbuf.st_blocks);

    if (S_ISDIR(statbuf.st_mode)) {
        ((PFILEFINDBUF3)*pFindBuf)->attrFile = FILE_DIRECTORY;
    }
    else {
        ((PFILEFINDBUF3)*pFindBuf)->attrFile = (ULONG)0;
    }
  }
  
  ctr = i;
  *pcFileNames = (ULONG)i;

  if (*phDir == HDIR_CREATE)
    *phDir = (HDIR)hdir;

  *cbBuf = *pcFileNames * sizeof(FILEFINDBUF3); // vs
  *pFindBuf -= *cbBuf; // vs

  //if (i == hdir->gl_pathc)
  //    return ERROR_NO_MORE_FILES;

  return NO_ERROR;
}


APIRET DICE_CV
os2fs_dos_FindNext_component (CORBA_Object _dice_corba_obj,
                              HDIR  hDir /* in */,
                              char  **pFindBuf /* in, out */,
                              ULONG *cbBuf /* in, out */,
                              ULONG *pcFileNames /* in, out */,
                              CORBA_Server_Environment *_dice_corba_env)
{
  glob_t *hdir;
  struct stat statbuf;
  struct tm tt;
  char *p, *fname;
  int  rc, i, j;

  /* check args */
  if (*cbBuf == 0)
      return ERROR_BUFFER_OVERFLOW;

  if (*pcFileNames * sizeof(FILEFINDBUF3) > *cbBuf)
      return ERROR_BUFFER_OVERFLOW;

  /* set HDIR (glob_t) */
  if (hDir == HDIR_SYSTEM)
    hdir = &thehdir;
  else
    hdir = (glob_t *)hDir;

  if (ctr == hdir->gl_pathc)
      return ERROR_NO_MORE_FILES;

  /* build FILEFINDBUF3 entries */
  for (i = ctr, j = 0; j < *pcFileNames;
       i++, j++, *pFindBuf += sizeof(FILEFINDBUF3)) 
  {
    if (i == hdir->gl_pathc)
      break;

    if (j == *pcFileNames - 1)
        ((PFILEFINDBUF3)*pFindBuf)->oNextEntryOffset = 0;
    else
        ((PFILEFINDBUF3)*pFindBuf)->oNextEntryOffset = sizeof(FILEFINDBUF3);

    fname = hdir->gl_pathv[i];
    // search for the last slash
    for (p = fname + strlen(fname); p > fname && *p != '/'; p--) ;
    if (*p == '/') p++;
	    
    ((PFILEFINDBUF3)*pFindBuf)->cchName = strlen(p);
    strcpy(((PFILEFINDBUF3)*pFindBuf)->achName, p);

    LOG("filename=%s", hdir->gl_pathv[i]);

    /* stat the file to get it's  info */
    rc = stat(hdir->gl_pathv[i], &statbuf);

    localtime_r(&statbuf.st_ctime, &tt);
    
    ((PFILEFINDBUF3)*pFindBuf)->fdateCreation.day =
        (UINT)tt.tm_mday;
    ((PFILEFINDBUF3)*pFindBuf)->fdateCreation.month =
        (UINT)tt.tm_mon + 1;
    ((PFILEFINDBUF3)*pFindBuf)->fdateCreation.year =
        (UINT)tt.tm_year - 80;
    ((PFILEFINDBUF3)*pFindBuf)->ftimeCreation.twosecs =
        (USHORT)tt.tm_sec / 2;
    ((PFILEFINDBUF3)*pFindBuf)->ftimeCreation.minutes =
        (USHORT)tt.tm_min;
    ((PFILEFINDBUF3)*pFindBuf)->ftimeCreation.hours =
	(USHORT)tt.tm_hour;
        
    localtime_r(&statbuf.st_atime, &tt);
    
    ((PFILEFINDBUF3)*pFindBuf)->fdateLastAccess.day =
        (UINT)tt.tm_mday;
    ((PFILEFINDBUF3)*pFindBuf)->fdateLastAccess.month =
        (UINT)tt.tm_mon + 1;
    ((PFILEFINDBUF3)*pFindBuf)->fdateLastAccess.year =
        (UINT)tt.tm_year - 80;
    ((PFILEFINDBUF3)*pFindBuf)->ftimeLastAccess.twosecs =
        (USHORT)tt.tm_sec / 2;
    ((PFILEFINDBUF3)*pFindBuf)->ftimeLastAccess.minutes =
        (USHORT)tt.tm_min;
    ((PFILEFINDBUF3)*pFindBuf)->ftimeLastAccess.hours =
        (USHORT)tt.tm_hour;
        
    localtime_r(&statbuf.st_mtime, &tt);
    
    ((PFILEFINDBUF3)*pFindBuf)->fdateLastWrite.day =
        (UINT)tt.tm_mday;
    ((PFILEFINDBUF3)*pFindBuf)->fdateLastWrite.month =
        (UINT)tt.tm_mon + 1;
    ((PFILEFINDBUF3)*pFindBuf)->fdateLastWrite.year =
        (UINT)tt.tm_year - 80;
    ((PFILEFINDBUF3)*pFindBuf)->ftimeLastWrite.twosecs =
        (USHORT)tt.tm_sec / 2;
    ((PFILEFINDBUF3)*pFindBuf)->ftimeLastWrite.minutes =
        (USHORT)tt.tm_min;
    ((PFILEFINDBUF3)*pFindBuf)->ftimeLastWrite.hours =
        (USHORT)tt.tm_hour;
    ((PFILEFINDBUF3)*pFindBuf)->cbFile = (ULONG)statbuf.st_size;
        ((PFILEFINDBUF3)*pFindBuf)->cbFileAlloc =
        (ULONG)(statbuf.st_blksize * statbuf.st_blocks);

    if (S_ISDIR(statbuf.st_mode))
        ((PFILEFINDBUF3)*pFindBuf)->attrFile = FILE_DIRECTORY;
    else
        ((PFILEFINDBUF3)*pFindBuf)->attrFile = (ULONG)0;
  }

  ctr = i;

  *pcFileNames = (ULONG)j;

  *cbBuf = *pcFileNames * sizeof(FILEFINDBUF3); // vs
  *pFindBuf -= *cbBuf; // vs

  //if (i == hdir->gl_pathc)
  //    return ERROR_NO_MORE_FILES;

  return NO_ERROR;
}


APIRET DICE_CV
os2fs_dos_FindClose_component (CORBA_Object _dice_corba_obj,
                               HDIR hDir /* in */,
                               CORBA_Server_Environment *_dice_corba_env)
{
  glob_t *hdir;
  
  LOG("0");
  /* set HDIR (glob_t) */
  if (hDir == HDIR_SYSTEM)
    hdir = &thehdir;
  else if (hDir != HDIR_CREATE)
    hdir = (glob_t *)hDir;

  LOG("1");

  if (hdir != HDIR_CREATE)
  {
    /* free the memory */
    globfree(hdir);

    LOG("2");
    if (hdir != &thehdir)
      free(hdir);
  }
  LOG("3");

  return NO_ERROR;
}


APIRET DICE_CV
os2fs_dos_QueryFHState_component(CORBA_Object _dice_corba_obj,
                                 HFILE hFile, 
		                 PULONG pMode,
		                 CORBA_Server_Environment *_dice_corba_env)
{
  filehandle_t *h;

  if (hFile == 0 || hFile == 1 || hFile == 2) // stdin/stdout/stderr
    *pMode = OPEN_ACCESS_READWRITE | OPEN_SHARE_DENYNONE;
  else
  {
    h = (filehandle_t *)hFile;
    *pMode = h->openmode;
  }
  
  return NO_ERROR;
}


APIRET DICE_CV
os2fs_dos_SetFHState_component (CORBA_Object _dice_corba_obj,
                                HFILE hFile /* in */,
                                ULONG pMode /* in */,
                                CORBA_Server_Environment *_dice_corba_env)
{
  filehandle_t *h;
  int mode;
  int rc;
  
  if (hFile != 0 && hFile != 1 && hFile != 2) // stdin/stdout/stderr
  {
    h = (filehandle_t *)hFile;
    mode = h->openmode;
    
    if (pMode & OPEN_FLAGS_WRITE_THROUGH)
      mode |= O_SYNC;

    if (pMode & OPEN_ACTION_REPLACE_IF_EXISTS)
      mode |= O_TRUNC;
      
    if (pMode & OPEN_ACCESS_READONLY)
      mode |= O_RDONLY;
    else if (pMode & OPEN_ACCESS_WRITEONLY)
      mode |= O_WRONLY;
    else
      mode |= O_RDWR;
  
    rc = fcntl(h->hfile, F_SETFL, mode);

    if (rc)
    {
      switch (rc)
      {
      case EBADF:
        return ERROR_INVALID_HANDLE;
      default:
        return ERROR_INVALID_PARAMETER;
      }
    }
    else
      h->openmode = pMode;
  }

  return NO_ERROR;
}


APIRET DICE_CV
os2fs_dos_QueryFileInfo_component (CORBA_Object _dice_corba_obj,
                                   HFILE hf /* in */,
                                   ULONG ulInfoLevel /* in */,
                                   char **pInfo /* in, out */,
                                   ULONG *cbInfoBuf /* in, out */,
                                   CORBA_Server_Environment *_dice_corba_env)
{
    filehandle_t *pfh;
    struct stat buf;
    struct tm brokentime;
    int rc;

    /* check args */
    if (ulInfoLevel != FIL_STANDARD)
        return ERROR_INVALID_LEVEL;

    if (*cbInfoBuf < sizeof(FILESTATUS3))
        return ERROR_BUFFER_OVERFLOW;

    /* get the file status */
    pfh = (filehandle_t *)hf;
    rc = fstat(pfh->hfile, &buf);
    if (rc == -1) 
    {
        switch (errno)
	{
        case EBADF:
          return ERROR_INVALID_HANDLE;
        default:
          return ERROR_ACCESS_DENIED;
        }
    }
    
    localtime_r(&buf.st_ctime, &brokentime);
    
    ((PFILESTATUS3)*pInfo)->fdateCreation.day = (UINT)brokentime.tm_mday;
    ((PFILESTATUS3)*pInfo)->fdateCreation.month = (UINT)brokentime.tm_mon + 1;
    ((PFILESTATUS3)*pInfo)->fdateCreation.year = (UINT)brokentime.tm_year - 80;
    ((PFILESTATUS3)*pInfo)->ftimeCreation.twosecs = (USHORT)brokentime.tm_sec / 2;
    ((PFILESTATUS3)*pInfo)->ftimeCreation.minutes = (USHORT)brokentime.tm_min;
    ((PFILESTATUS3)*pInfo)->ftimeCreation.hours = (USHORT)brokentime.tm_hour;
    
    localtime_r(&buf.st_atime, &brokentime);
    
    ((PFILESTATUS3)*pInfo)->fdateLastAccess.day = (UINT)brokentime.tm_mday;
    ((PFILESTATUS3)*pInfo)->fdateLastAccess.month = (UINT)brokentime.tm_mon + 1;
    ((PFILESTATUS3)*pInfo)->fdateLastAccess.year = (UINT)brokentime.tm_year - 80;
    ((PFILESTATUS3)*pInfo)->ftimeLastAccess.twosecs = (USHORT)brokentime.tm_sec / 2;
    ((PFILESTATUS3)*pInfo)->ftimeLastAccess.minutes = (USHORT)brokentime.tm_min;
    ((PFILESTATUS3)*pInfo)->ftimeLastAccess.hours = (USHORT)brokentime.tm_hour;
    
    localtime_r(&buf.st_mtime, &brokentime);
    
    ((PFILESTATUS3)*pInfo)->fdateLastWrite.day = (UINT)brokentime.tm_mday;
    ((PFILESTATUS3)*pInfo)->fdateLastWrite.month = (UINT)brokentime.tm_mon + 1;
    ((PFILESTATUS3)*pInfo)->fdateLastWrite.year = (UINT)brokentime.tm_year - 80;
    ((PFILESTATUS3)*pInfo)->ftimeLastWrite.twosecs = (USHORT)brokentime.tm_sec / 2;
    ((PFILESTATUS3)*pInfo)->ftimeLastWrite.minutes = (USHORT)brokentime.tm_min;
    ((PFILESTATUS3)*pInfo)->ftimeLastWrite.hours = (USHORT)brokentime.tm_hour;
    ((PFILESTATUS3)*pInfo)->cbFile = (ULONG)buf.st_size;
    ((PFILESTATUS3)*pInfo)->cbFileAlloc = (ULONG)(buf.st_blksize * buf.st_blocks);

    if (S_ISDIR(buf.st_mode))
        ((PFILESTATUS3)*pInfo)->attrFile = FILE_DIRECTORY;
    else
        ((PFILESTATUS3)*pInfo)->attrFile = (ULONG)0;

    return NO_ERROR;
}


APIRET DICE_CV
os2fs_dos_QueryPathInfo_component (CORBA_Object _dice_corba_obj,
                                   const char* pszPathName /* in */,
                                   ULONG ulInfoLevel /* in */,
                                   char **pInfo /* in, out */,
                                   ULONG *cbInfoBuf /* in, out */,
                                   CORBA_Server_Environment *_dice_corba_env)
{
    PSZ pszNewName;
    struct stat buf;
    struct tm brokentime;
    int rc;

    /* check args */
    if (*cbInfoBuf < sizeof(FILESTATUS3))
      return ERROR_BUFFER_OVERFLOW;

    if (ulInfoLevel == FIL_STANDARD)
    {
        /* convert the file names */
        pszNewName = alloca(strlen(pszPathName) + 1);

        if (pszNewName == NULL)
          return ERROR_SHARING_BUFFER_EXCEEDED;

        //strcpy(pszNewName, pszPathName);
	//DosNameConversion(pszNewName, "\\", "/", FALSE);
	pathconv(&pszNewName, pszPathName);
        
	/* get the file status */
        rc = stat(pszNewName, &buf);

        if (rc == -1)
	{
            switch (errno)
	    {
            case EBADF:
              return ERROR_INVALID_HANDLE;
            default:
              return ERROR_ACCESS_DENIED;
            }
        }
        
	localtime_r(&buf.st_ctime, &brokentime);
        
	((PFILESTATUS3)*pInfo)->fdateCreation.day = (UINT)brokentime.tm_mday;
        ((PFILESTATUS3)*pInfo)->fdateCreation.month =
            (UINT)brokentime.tm_mon + 1;
        ((PFILESTATUS3)*pInfo)->fdateCreation.year =
            (UINT)brokentime.tm_year + 1900;
        ((PFILESTATUS3)*pInfo)->ftimeCreation.twosecs =
            (USHORT)brokentime.tm_sec / 2;
        ((PFILESTATUS3)*pInfo)->ftimeCreation.minutes =
            (USHORT)brokentime.tm_min;
        ((PFILESTATUS3)*pInfo)->ftimeCreation.hours =
            (USHORT)brokentime.tm_hour;
        
	localtime_r(&buf.st_atime, &brokentime);
        
	((PFILESTATUS3)*pInfo)->fdateLastAccess.day = (UINT)brokentime.tm_mday;
        ((PFILESTATUS3)*pInfo)->fdateLastAccess.month =
            (UINT)brokentime.tm_mon + 1;
        ((PFILESTATUS3)*pInfo)->fdateLastAccess.year =
            (UINT)brokentime.tm_year + 1900;
        ((PFILESTATUS3)*pInfo)->ftimeLastAccess.twosecs =
            (USHORT)brokentime.tm_sec / 2;
        ((PFILESTATUS3)*pInfo)->ftimeLastAccess.minutes =
            (USHORT)brokentime.tm_min;
        ((PFILESTATUS3)*pInfo)->ftimeLastAccess.hours =
            (USHORT)brokentime.tm_hour;
        
	localtime_r(&buf.st_mtime, &brokentime);
        
	((PFILESTATUS3)*pInfo)->fdateLastWrite.day = (UINT)brokentime.tm_mday;
        ((PFILESTATUS3)*pInfo)->fdateLastWrite.month =
            (UINT)brokentime.tm_mon + 1;
        ((PFILESTATUS3)*pInfo)->fdateLastWrite.year =
            (UINT)brokentime.tm_year + 1900;
        ((PFILESTATUS3)*pInfo)->ftimeLastWrite.twosecs =
            (USHORT)brokentime.tm_sec / 2;
        ((PFILESTATUS3)*pInfo)->ftimeLastWrite.minutes =
            (USHORT)brokentime.tm_min;
        ((PFILESTATUS3)*pInfo)->ftimeLastWrite.hours =
            (USHORT)brokentime.tm_hour;
        ((PFILESTATUS3)*pInfo)->cbFile = (ULONG)buf.st_size;
        ((PFILESTATUS3)*pInfo)->cbFileAlloc =
            (ULONG)(buf.st_blksize * buf.st_blocks);

        if (S_ISDIR(buf.st_mode))
            ((PFILESTATUS3)*pInfo)->attrFile = FILE_DIRECTORY;
        else
            ((PFILESTATUS3)*pInfo)->attrFile = (ULONG)0;
    }
    else
        return ERROR_INVALID_LEVEL;

    return NO_ERROR;
}


APIRET DICE_CV
os2fs_dos_ResetBuffer_component (CORBA_Object _dice_corba_obj,
                                 HFILE hFile /* in */,
                                 CORBA_Server_Environment *_dice_corba_env)
{
#if 0
  filehandle_t *pfh;
  int rc;

  /* set the file size */
  pfh = (filehandle_t *)hFile;

  //if (strcmp(pfh->id, "FIL") == 0)
  {
    rc = fsync(pfh->hfile);
    
    if (rc == -1) {
      switch (errno)
      {
      case EBADF:
        return ERROR_INVALID_HANDLE;
      case EINVAL:
      case EIO:
        return ERROR_ACCESS_DENIED;
      default:
        return ERROR_INVALID_PARAMETER;
      }
    }
  }
  /* there is no need to flush a pipe */

  return NO_ERROR;
#else
  return 0;
#endif
}


APIRET DICE_CV
os2fs_dos_SetFileSizeL_component (CORBA_Object _dice_corba_obj,
                                  HFILE hFile /* in */,
                                  long long cbSize /* in */,
                                  CORBA_Server_Environment *_dice_corba_env)
{
#if 0
  filehandle_t *pfh;
  int rc;

  /* set the file size */
  pfh = (filehandle_t *)hFile;

  rc = ftruncate(pfh->hfile, cbSize);

  if (rc == -1)
  {
    switch (errno)
    {
    case EBADF:
      return ERROR_INVALID_HANDLE;
    case EINVAL:
      return ERROR_ACCESS_DENIED;
    default:
      return ERROR_INVALID_PARAMETER;
    }
  }

  return NO_ERROR;
#else
  return 0;
#endif
}

/* -------- End os2linux -----------
 */

#if 0

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
  filehandle_t *pfh;
  struct tm brokentime;
  struct utimbuf tb;
  struct timeval tv[2];
  struct stat buf;
  ULONG mode;
  int rc;

  pfh = (filehandle_t *)hf;
  mode = pfh->openmode;
  
  if (!(mode & OPEN_ACCESS_READWRITE))
    return ERROR_ACCESS_DENIED;
    
  if (!(mode & OPEN_SHARE_DENYREADWRITE))
    return ERROR_ACCESS_DENIED;
    
  if (ulInfoLevel != FIL_STANDARD)
    return ERROR_INVALID_LEVEL;
    
  /* check args */
  if (*cbInfoBuf < sizeof(FILESTATUS3))
    return ERROR_BUFFER_OVERFLOW;

  rc = fstat(pfh->hfile, &buf);
  
  if (rc)
    return ERROR_INVALID_HANDLE;

  localtime_r(buf.st_atime, &brokentime);
    
  if (((PFILESTATUS3)*pInfoBuf)->fdateLastAccess.day)
    brokentime.tm_mday = ((PFILESTATUS3)*pInfoBuf)->fdateLastAccess.day;

  if (((PFILESTATUS3)*pInfoBuf)->fdateLastAccess.month)
    brokentime.tm_mon = ((PFILESTATUS3)*pInfoBuf)->fdateLastAccess.month - 1;

  if (((PFILESTATUS3)*pInfoBuf)->fdateLastAccess.year)
    brokentime.tm_year = ((PFILESTATUS3)*pInfoBuf)->fdateLastAccess.year - 1900;
  
  if (((PFILESTATUS3)*pInfoBuf)->ftimeLastAccess.twosecs)
    brokentime.tm_sec = ((PFILESTATUS3)*pInfoBuf)->ftimeLastAccess.twosecs * 2;

  if (((PFILESTATUS3)*pInfoBuf)->ftimeLastAccess.minutes)
    brokentime.tm_min = ((PFILESTATUS3)*pInfoBuf)->ftimeLastAccess.minutes;

  if (((PFILESTATUS3)*pInfoBuf)->ftimeLastAccess.hours)
    brokentime.tm_hour = ((PFILESTATUS3)*pInfoBuf)->ftimeLastAccess.hours;

  tv[0].tv_sec  = timelocal(&brokentime);
  tv[0].tv_usec = 0;

  localtime_r(buf.st_mtime, &brokentime);
    
  if (((PFILESTATUS3)*pInfoBuf)->fdateLastWrite.day)
    brokentime.tm_mday = ((PFILESTATUS3)*pInfoBuf)->fdateLastWrite.day;

  if (((PFILESTATUS3)*pInfoBuf)->fdateLastWrite.month)
    brokentime.tm_mon = ((PFILESTATUS3)*pInfoBuf)->fdateLastWrite.month - 1;

  if (((PFILESTATUS3)*pInfoBuf)->fdateLastWrite.year)
    brokentime.tm_year = ((PFILESTATUS3)*pInfoBuf)->fdateLastWrite.year - 1900;
  
  if (((PFILESTATUS3)*pInfoBuf)->ftimeLastWrite.twosecs)
    brokentime.tm_sec = ((PFILESTATUS3)*pInfoBuf)->ftimeLastWrite.twosecs * 2;

  if (((PFILESTATUS3)*pInfoBuf)->ftimeLastWrite.minutes)
    brokentime.tm_min = ((PFILESTATUS3)*pInfoBuf)->ftimeLastWrite.minutes;

  if (((PFILESTATUS3)*pInfoBuf)->ftimeLastWrite.hours)
    brokentime.tm_hour = ((PFILESTATUS3)*pInfoBuf)->ftimeLastWrite.hours;

  tv[1].tv_sec  = timelocal(&brokentime);
  tv[1].tv_usec = 0;
  
  if (hf != 0 && hf != 1 && hf != 2)
  {
    rc = utimes(pfh->filename, tv);
    
    if (rc)
    {
      switch (rc)
      {
      case ENOENT:
        return ERROR_FILE_NOT_FOUND;
      default:
        return ERROR_ACCESS_DENIED;
      }
    }
  }
  
  return NO_ERROR;
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
}

#endif
