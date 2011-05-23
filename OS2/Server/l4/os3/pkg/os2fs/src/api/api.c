/*  file/directory API implementations
 *  (on the server side)
 */
 
#include <l4/os3/gcc_os2def.h>

#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <l4/log/l4log.h>

#include <dice/dice.h>

extern l4_threadid_t os2srv;

int pathconv(char **converted, char *fname);

APIRET
os2fs_dos_Read_component(CORBA_Object _dice_corba_obj,
                             HFILE hFile, void **pBuffer,
                             ULONG *pcbRead,
                             short *dice_reply,
                             CORBA_Server_Environment *_dice_corba_env)
{
  int  c;
  int  nread = 0;
  int  total = 1;

  nread = read(hFile, (char *)*pBuffer, *pcbRead);
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
  *pcbRead = nread;


  return 0; // NO_ERROR
}

APIRET
os2fs_dos_Write_component(CORBA_Object _dice_corba_obj,
                              HFILE hFile, PVOID pBuffer,
                              ULONG *pcbWrite,
                              short *dice_reply,
	 		      CORBA_Server_Environment *_dice_corba_env)
{
  char *s;
  int  nwritten;

  LOG("entered");
  nwritten = write(hFile, pBuffer, *pcbWrite);
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
  *pcbWrite = nwritten;
  LOG("exited");
  return 0/*NO_ERROR*/;
}


APIRET DICE_CV
os2fs_dos_ResetBuffer_component (CORBA_Object _dice_corba_obj,
                                     HFILE handle /* in */,
                                     CORBA_Server_Environment *_dice_corba_env)
{
  return 0; /* NO_ERROR */
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
  
  ret = fstat(handle, &stat);

  if (ret == -1)
    return 6; /* ERROR_INVALID_HANDLE */

  /* get length */
  len = stat.st_size;
  /* get current position */
  pos = lseek(handle, 0L, SEEK_CUR);

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
    
  ret = lseek(handle, pos, SEEK_SET);

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
  
  ret = close(handle);
  
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
  
  ret = fstat(handle, &stat);

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
  
  *phFile = handle;

  
  return 0; /* NO_ERROR */
}

APIRET DICE_CV
os2fs_dos_DupHandle_component (CORBA_Object _dice_corba_obj,
                               HFILE hFile /* in */,
                               HFILE *phFile2 /* in, out */,
                               CORBA_Server_Environment *_dice_corba_env)
{
  struct stat st;
  HFILE handle;
  APIRET rc;
  
  if (*phFile2 == -1)
  {
    handle = dup(hFile);
  
    if (handle == -1)
    {
      if (errno == EBADF)
        return 6; /* ERROR_INVALID_HANDLE */
	
      if (errno == EMFILE)
        return 4; /* ERROR_TOO_MANY_OPEN_FILES */
    }
    
    *phFile2 = handle;
  }
#if 0 // for now dup2 seems to be not implemented
  else
  {
    rc = dup2(hFile, *phFile2);

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
