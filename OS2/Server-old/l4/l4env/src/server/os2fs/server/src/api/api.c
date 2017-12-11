/*  file/directory API implementations
 *  (on the server side)
 */
 
#include <l4/os3/gcc_os2def.h>
#include <l4/os3/MountReg.h>
#include <l4/os3/kal.h>
#include <l4/os3/io.h>

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
//#include <stdlib.h>

#include <dice/dice.h>

//#include "os2fs-client.h"
#include "os2fs-server.h"
#include <l4/os2fs/os2fs-client.h>
#include <l4/os2srv/os2server-client.h>

extern l4_threadid_t fs;
extern l4_threadid_t os2srv;

typedef struct filehandle filehandle_t;

struct filehandle
{
  HFILE hfile;
  char id[4];
  ULONG openmode;
  char filename[256];
  filehandle_t *next;
};

filehandle_t *fileroot = NULL;

typedef struct
{
  glob_t g;
  int ctr;
  int attr;
} filefindstruc_t;

static filefindstruc_t thehdir;

void setdrivemap(ULONG *map);

int pathconv(char **converted, char *fname);
int cdir(char **dir, char *component);

APIRET DICE_CV
os2fs_dos_Read_component(CORBA_Object _dice_corba_obj,
                             HFILE hFile, char **pBuffer,
                             ULONG *count,
                             short *dice_reply,
                             CORBA_Server_Environment *_dice_corba_env)
{
  //int  c;
  int  nread = 0;
  //int  total = 1;
  
  nread = read(hFile, (char *)*pBuffer, *count);
  if (nread == -1)
  {
    io_log("read() error, errno=%d\n", errno);
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
  //char *s;
  int  nwritten;

  //io_log("entered\n");
  nwritten = write(handle, pBuffer, *count);
  //io_log("in the middle\n");
  if (nwritten == -1)
  {
    io_log("write() error, errno=%d\n", errno);
    switch (errno)
    {
      // @todo: more accurate error handling
      default:
        return 232; //ERROR_NO_DATA
    }
  }

  //io_log("nwritten=%u\n", nwritten);
  *count = nwritten;
  //io_log("exited\n");
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
                                    unsigned long long *ibActual /* out */,
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

  *ibActual = (unsigned long long)pos;

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
    return ERROR_INVALID_PARAMETER;
  }

  if (pos < 0)
    return 131; /* ERROR_NEGATIVE_SEEK */

  ret = lseek(handle, pos, SEEK_SET);

  if (ret == -1)
    return 132; /* ERROR_SEEK_ON_DEVICE */

  *ibActual = (unsigned long long)pos;

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
  
  if (isatty(handle))
  {
    *pType = 1; // character device
    *pAttr = 0;
    return 0;
  }
  
  ret = fstat(handle, &stat);

  if (ret == -1)
    return 6; /* ERROR_INVALID_HANDLE */

  m = stat.st_mode;

  if (S_ISREG(m))
  {
    *pType = 0; // disk file
    *pAttr = handle;
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
//#define FILE_ARCHIVED  0x00000020
//#define FILE_DIRECTORY 0x00000010
//#define FILE_SYSTEM    0x00000004
//#define FILE_HIDDEN    0x00000002
//#define FILE_READONLY  0x00000001
//#define FILE_NORMAL    0x00000000

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
  filehandle_t *h, *p;
  struct stat st;
  char file_existed = 0;
  char *newfilename;
  int mode = 0; //O_BINARY;
  int handle;
  char fn[CCHMAXPATH];
  int i;
 
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

  strcpy(fn, pszFileName);

  for (i = 0; fn[i]; i++)
  {
    if (fn[i] == '/')
      fn[i] = '\\';
  }

  /* convert OS/2-style pathname to PN-style pathname */
  if (pathconv(&newfilename, fn))
    return 3; /* ERROR_PATH_NOT_FOUND */

  if (strlen(newfilename) + 1 > 256)
    return ERROR_FILENAME_EXCED_RANGE;

  if (!stat(newfilename, &st))
    file_existed = 1;

  handle = open(newfilename, mode);

  if (handle == -1)
  {
    if (errno == EMFILE)
    {
      io_log("ERROR_TOO_MANY_OPEN_FILES\n");
      return 4; /* ERROR_TOO_MANY_OPEN_FILES */
    }
      
    if (errno == ENOENT)
    {
      io_log("ERROR_FILE_NOT_FOUND\n");
      return 2; /* ERROR_FILE_NOT_FOUND */
    }

    io_log("ERROR_ACCESS_DENIED\n");
    return 5; /* ERROR_ACCESS_DENIED */
  }

  if (fsOpenFlags & OPEN_ACTION_FAIL_IF_EXISTS)
  {
    close(handle);
    io_log("ERROR_OPEN_FAILED\n");
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
  h->next = NULL;
  strcpy(h->filename, newfilename);

  if (fileroot)
  {
    p = fileroot;

    while (p->next)
      p = p->next;
      
    p->next = h;
  }
  else
    fileroot = h;

  *phFile = (HFILE)handle;

  io_log("NO_ERROR\n");
  return 0; /* NO_ERROR */
}

APIRET DICE_CV
os2fs_dos_DupHandle_component (CORBA_Object _dice_corba_obj,
                               HFILE hFile /* in */,
                               HFILE *phFile2 /* in, out */,
                               CORBA_Server_Environment *_dice_corba_env)
{
  //struct stat st;
  int handle;
  filehandle_t *h, *h2, *p;
  //APIRET rc;
  
  // search for filehandle_t structure, corresponding to hFile
  for (h = fileroot; h->hfile != hFile; h = h->next) ;

  if (h == NULL)
    return ERROR_INVALID_HANDLE;

  // get last list element
  p = fileroot;
  while (p->next)
    p = p->next;
    
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
  }
#if 0 // for now dup2 seems to be not implemented
  else
  {
    rc = dup2(hFile, handle);

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
  h2->next = NULL;

  p->next = h2;
  
  *phFile2 = (HFILE)h2;
  
  return 0; /* NO_ERROR */
}

APIRET DICE_CV
os2fs_dos_Delete_component (CORBA_Object _dice_corba_obj,
                            const char* pszFileName /* in */,
                            CORBA_Server_Environment *_dice_corba_env)
{
  char fn[CCHMAXPATH];
  int  i;
  /* For now, it is identical to DosForceDelete,
     no SET DELDIR support yet */

  strcpy(fn, pszFileName);

  for (i = 0; fn[i]; i++)
  {
    if (fn[i] == '/')
      fn[i] = '\\';
  }

  if (unlink(fn))
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
  char fn[CCHMAXPATH];
  int  i;

  strcpy(fn, pszFileName);

  for (i = 0; fn[i]; i++)
  {
    if (fn[i] == '/')
      fn[i] = '\\';
  }

  if (unlink(fn))
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
  char fn[CCHMAXPATH];
  int  i;
  
  strcpy(fn, pszDirName);
  
  for (i = 0; fn[i]; i++)
  {
    if (fn[i] == '/')
      fn[i] = '\\';
  }

  if (rmdir(fn) == -1)
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
  char fn[CCHMAXPATH];
  int  i;

  strcpy(fn, pszDirName);

  for (i = 0; fn[i]; i++)
  {
    if (fn[i] == '/')
      fn[i] = '\\';
  }
  
  if (peaop2 == NULL)
    peaop2 = &eaop2;
  
  if (mkdir(fn, 0) == -1)
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
  //CORBA_Environment env = dice_default_environment;
  //char path[256];
  //int   len = 0;
  filefindstruc_t *hdir;
  struct stat statbuf;
  struct tm tt;
  char *t, *p;
  char *fname;
  int  rc, i, j;
  //ULONG disk, logical;
  char fn[CCHMAXPATH];

  strcpy(fn, pszFileSpec);

  for (i = 0; fn[i]; i++) 
  {
    if (fn[i] == '/')
      fn[i] = '\\';
  }

  io_log("fn=%s\n", fn);

  // no support for EA's for now
  if (ulInfolevel != FIL_STANDARD)
    return ERROR_INVALID_LEVEL;
    
  // check for buffer overflow, and whether it is <= 64k
  if (*cbBuf == 0 || *cbBuf > 0x10000 || 
      *pcFileNames * sizeof(FILEFINDBUF3) > *cbBuf)
    return ERROR_BUFFER_OVERFLOW;

  if (*phDir == HDIR_SYSTEM)
    hdir = &thehdir;
  else if (*phDir == HDIR_CREATE)
    hdir = (filefindstruc_t *)malloc(sizeof(filefindstruc_t));
  else
    return ERROR_INVALID_HANDLE;

  hdir->attr = flAttribute;
  hdir->ctr  = 0;

  if (strlen(fn) > 255)
    return ERROR_FILENAME_EXCED_RANGE;

  // convert the pathname from OS/2 style to PN one
  if (pathconv(&t, fn))
    return ERROR_PATH_NOT_FOUND;

  // check for filename length overflow
  if (strlen(t) > 255)
    return ERROR_FILENAME_EXCED_RANGE;
    //return ERROR_META_EXPANSION_TOO_LONG;
    
  // perform the search
  glob(t, 0, NULL, &hdir->g);
  
  for (i = 0, j = 0; j < *pcFileNames && i < hdir->g.gl_pathc; i++)
  {
    if (j + 1 == *pcFileNames)
      ((PFILEFINDBUF3)*pFindBuf)->oNextEntryOffset = 0;
    else
      ((PFILEFINDBUF3)*pFindBuf)->oNextEntryOffset = sizeof(FILEFINDBUF3);

    fname = hdir->g.gl_pathv[i];

    if (strlen(fname) > 255)
      return ERROR_META_EXPANSION_TOO_LONG;

    rc = stat(fname, &statbuf);

    if (rc)
    {
      switch (rc)
      {
	 case EACCES:
	   return ERROR_ACCESS_DENIED;
	 case ENAMETOOLONG:
	   return ERROR_FILENAME_EXCED_RANGE;
	 case ENOTDIR:
	   return ERROR_PATH_NOT_FOUND;
         case ENOENT:
	   return ERROR_FILE_NOT_FOUND;
         case ENOMEM:
	   return ERROR_NOT_ENOUGH_MEMORY;
	 default:
	   return ERROR_INVALID_PARAMETER;
      }
    }

    // must have flags
    if ((hdir->attr & MUST_HAVE_ARCHIVED) && 
        (!(hdir->attr & FILE_ARCHIVED) || !(statbuf.st_mode & S_IARCHIVED)) &&
         ((hdir->attr & FILE_ARCHIVED) || (statbuf.st_mode & S_IARCHIVED)))
      continue;

    if ((hdir->attr & MUST_HAVE_DIRECTORY) && 
        (!(hdir->attr & FILE_DIRECTORY) || !(statbuf.st_mode & S_IDIRECTORY)) &&
         ((hdir->attr & FILE_DIRECTORY) || (statbuf.st_mode & S_IDIRECTORY)))
      continue;

    if ((hdir->attr & MUST_HAVE_SYSTEM) && 
        (!(hdir->attr & FILE_SYSTEM) || !(statbuf.st_mode & S_ISYSTEM)) &&
         ((hdir->attr & FILE_SYSTEM) || (statbuf.st_mode & S_ISYSTEM)))
      continue;

    if ((hdir->attr & MUST_HAVE_HIDDEN) && 
        (!(hdir->attr & FILE_HIDDEN) || !(statbuf.st_mode & S_IHIDDEN)) &&
         ((hdir->attr & FILE_HIDDEN) || (statbuf.st_mode & S_IHIDDEN)))
      continue;

    if ((hdir->attr & MUST_HAVE_READONLY) && 
        (!(hdir->attr & FILE_READONLY) || !(statbuf.st_mode & S_IREADONLY)) &&
         ((hdir->attr & FILE_READONLY) || (statbuf.st_mode & S_IREADONLY)))
      continue;

    localtime_r(&statbuf.st_ctime, &tt);
    
    ((PFILEFINDBUF3)*pFindBuf)->fdateCreation.year  = tt.tm_year - 80;
    ((PFILEFINDBUF3)*pFindBuf)->fdateCreation.month = tt.tm_mon  + 1;
    ((PFILEFINDBUF3)*pFindBuf)->fdateCreation.day   = tt.tm_mday;

    ((PFILEFINDBUF3)*pFindBuf)->ftimeCreation.hours   = tt.tm_hour;
    ((PFILEFINDBUF3)*pFindBuf)->ftimeCreation.minutes = tt.tm_min;
    ((PFILEFINDBUF3)*pFindBuf)->ftimeCreation.twosecs = tt.tm_sec / 2;

    localtime_r(&statbuf.st_atime, &tt);

    ((PFILEFINDBUF3)*pFindBuf)->fdateLastAccess.year  = tt.tm_year - 80;
    ((PFILEFINDBUF3)*pFindBuf)->fdateLastAccess.month = tt.tm_mon  + 1;
    ((PFILEFINDBUF3)*pFindBuf)->fdateLastAccess.day   = tt.tm_mday;

    ((PFILEFINDBUF3)*pFindBuf)->ftimeLastAccess.hours   = tt.tm_hour;
    ((PFILEFINDBUF3)*pFindBuf)->ftimeLastAccess.minutes = tt.tm_min;
    ((PFILEFINDBUF3)*pFindBuf)->ftimeLastAccess.twosecs = tt.tm_sec / 2;

    localtime_r(&statbuf.st_mtime, &tt);

    ((PFILEFINDBUF3)*pFindBuf)->fdateLastWrite.year  = tt.tm_year - 80;
    ((PFILEFINDBUF3)*pFindBuf)->fdateLastWrite.month = tt.tm_mon  + 1;
    ((PFILEFINDBUF3)*pFindBuf)->fdateLastWrite.day   = tt.tm_mday;

    ((PFILEFINDBUF3)*pFindBuf)->ftimeLastWrite.hours   = tt.tm_hour;
    ((PFILEFINDBUF3)*pFindBuf)->ftimeLastWrite.minutes = tt.tm_min;
    ((PFILEFINDBUF3)*pFindBuf)->ftimeLastWrite.twosecs = tt.tm_sec / 2;
    
    ((PFILEFINDBUF3)*pFindBuf)->attrFile = 0;
    
    if (statbuf.st_mode & S_IREADONLY)
      ((PFILEFINDBUF3)*pFindBuf)->attrFile |= FILE_READONLY;

    if (statbuf.st_mode & S_IHIDDEN)
      ((PFILEFINDBUF3)*pFindBuf)->attrFile |= FILE_HIDDEN;
    
    if (statbuf.st_mode & S_ISYSTEM)
      ((PFILEFINDBUF3)*pFindBuf)->attrFile |= FILE_SYSTEM;
    
    if (statbuf.st_mode & S_IDIRECTORY)
      ((PFILEFINDBUF3)*pFindBuf)->attrFile |= FILE_DIRECTORY;

    if (statbuf.st_mode & S_IARCHIVED)
      ((PFILEFINDBUF3)*pFindBuf)->attrFile |= FILE_ARCHIVED;
    
    // search for the last slash
    for (p = fname + strlen(fname); p > fname && *p != '/'; p--) ;
    if (*p == '/') p++;

    ((PFILEFINDBUF3)*pFindBuf)->cchName = strlen(p) + 1;
    strcpy(((PFILEFINDBUF3)*pFindBuf)->achName, p);
    
    ((PFILEFINDBUF3)*pFindBuf)->cbFile = statbuf.st_size;
    ((PFILEFINDBUF3)*pFindBuf)->cbFileAlloc = statbuf.st_blksize * statbuf.st_blocks;

    *pFindBuf += sizeof(FILEFINDBUF3);
    j++;
  }
    
  *cbBuf = j * sizeof(FILEFINDBUF3);
  *pFindBuf    -= *cbBuf;
  *pcFileNames = (ULONG)j;
  hdir->ctr = j;

  if (hdir == &thehdir)
    *phDir = HDIR_SYSTEM;
  else
    *phDir = (HDIR)hdir;
  
 
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
  filefindstruc_t *hdir;
  struct stat statbuf;
  struct tm tt;
  char *p, *fname;
  int  rc, i, j, k;

  hdir = (filefindstruc_t *)(hDir);

  if (hDir == HDIR_SYSTEM)
    hdir = &thehdir;

  // check for buffer overflow, and whether it is <= 64k
  if (*cbBuf == 0 || *cbBuf > 0x10000 || 
      *pcFileNames * sizeof(FILEFINDBUF3) > *cbBuf)
    return ERROR_BUFFER_OVERFLOW;

  if (hdir->ctr == hdir->g.gl_pathc)
    return ERROR_NO_MORE_FILES;

  for (i = hdir->ctr, j = 0, k = 0; k < *pcFileNames && i < hdir->g.gl_pathc; i++, j++)
  {
    if (k + 1 == *pcFileNames)
      ((PFILEFINDBUF3)*pFindBuf)->oNextEntryOffset = 0;
    else
      ((PFILEFINDBUF3)*pFindBuf)->oNextEntryOffset = sizeof(FILEFINDBUF3);

    fname = hdir->g.gl_pathv[i];

    if (strlen(fname) > 255)
      return ERROR_META_EXPANSION_TOO_LONG;

    rc = stat(fname, &statbuf);

    if (rc)
    {
      switch (rc)
      {
	 case EACCES:
	   return ERROR_ACCESS_DENIED;
	 case ENAMETOOLONG:
	   return ERROR_FILENAME_EXCED_RANGE;
	 case ENOTDIR:
	   return ERROR_PATH_NOT_FOUND;
         case ENOENT:
	   return ERROR_FILE_NOT_FOUND;
         case ENOMEM:
	   return ERROR_NOT_ENOUGH_MEMORY;
	 default:
	   return ERROR_INVALID_PARAMETER;
      }
    }

    // must have flags
    if ((hdir->attr & MUST_HAVE_ARCHIVED) && 
        (!(hdir->attr & FILE_ARCHIVED) || !(statbuf.st_mode & S_IARCHIVED)) &&
         ((hdir->attr & FILE_ARCHIVED) || (statbuf.st_mode & S_IARCHIVED)))
      continue;

    if ((hdir->attr & MUST_HAVE_DIRECTORY) && 
        (!(hdir->attr & FILE_DIRECTORY) || !(statbuf.st_mode & S_IDIRECTORY)) &&
         ((hdir->attr & FILE_DIRECTORY) || (statbuf.st_mode & S_IDIRECTORY)))
      continue;

    if ((hdir->attr & MUST_HAVE_SYSTEM) && 
        (!(hdir->attr & FILE_SYSTEM) || !(statbuf.st_mode & S_ISYSTEM)) &&
         ((hdir->attr & FILE_SYSTEM) || (statbuf.st_mode & S_ISYSTEM)))
      continue;

    if ((hdir->attr & MUST_HAVE_HIDDEN) && 
        (!(hdir->attr & FILE_HIDDEN) || !(statbuf.st_mode & S_IHIDDEN)) &&
         ((hdir->attr & FILE_HIDDEN) || (statbuf.st_mode & S_IHIDDEN)))
      continue;

    if ((hdir->attr & MUST_HAVE_READONLY) && 
        (!(hdir->attr & FILE_READONLY) || !(statbuf.st_mode & S_IREADONLY)) &&
         ((hdir->attr & FILE_READONLY) || (statbuf.st_mode & S_IREADONLY)))
      continue;

    localtime_r(&statbuf.st_ctime, &tt);
    
    ((PFILEFINDBUF3)*pFindBuf)->fdateCreation.year  = tt.tm_year - 80;
    ((PFILEFINDBUF3)*pFindBuf)->fdateCreation.month = tt.tm_mon  + 1;
    ((PFILEFINDBUF3)*pFindBuf)->fdateCreation.day   = tt.tm_mday;

    ((PFILEFINDBUF3)*pFindBuf)->ftimeCreation.hours   = tt.tm_hour;
    ((PFILEFINDBUF3)*pFindBuf)->ftimeCreation.minutes = tt.tm_min;
    ((PFILEFINDBUF3)*pFindBuf)->ftimeCreation.twosecs = tt.tm_sec / 2;

    localtime_r(&statbuf.st_atime, &tt);

    ((PFILEFINDBUF3)*pFindBuf)->fdateLastAccess.year  = tt.tm_year - 80;
    ((PFILEFINDBUF3)*pFindBuf)->fdateLastAccess.month = tt.tm_mon  + 1;
    ((PFILEFINDBUF3)*pFindBuf)->fdateLastAccess.day   = tt.tm_mday;

    ((PFILEFINDBUF3)*pFindBuf)->ftimeLastAccess.hours   = tt.tm_hour;
    ((PFILEFINDBUF3)*pFindBuf)->ftimeLastAccess.minutes = tt.tm_min;
    ((PFILEFINDBUF3)*pFindBuf)->ftimeLastAccess.twosecs = tt.tm_sec / 2;

    localtime_r(&statbuf.st_mtime, &tt);

    ((PFILEFINDBUF3)*pFindBuf)->fdateLastWrite.year  = tt.tm_year - 80;
    ((PFILEFINDBUF3)*pFindBuf)->fdateLastWrite.month = tt.tm_mon  + 1;
    ((PFILEFINDBUF3)*pFindBuf)->fdateLastWrite.day   = tt.tm_mday;

    ((PFILEFINDBUF3)*pFindBuf)->ftimeLastWrite.hours   = tt.tm_hour;
    ((PFILEFINDBUF3)*pFindBuf)->ftimeLastWrite.minutes = tt.tm_min;
    ((PFILEFINDBUF3)*pFindBuf)->ftimeLastWrite.twosecs = tt.tm_sec / 2;
    
    ((PFILEFINDBUF3)*pFindBuf)->attrFile = 0;
    
    if (statbuf.st_mode & S_IREADONLY)
      ((PFILEFINDBUF3)*pFindBuf)->attrFile |= FILE_READONLY;

    if (statbuf.st_mode & S_IHIDDEN)
      ((PFILEFINDBUF3)*pFindBuf)->attrFile |= FILE_HIDDEN;
    
    if (statbuf.st_mode & S_ISYSTEM)
      ((PFILEFINDBUF3)*pFindBuf)->attrFile |= FILE_SYSTEM;
    
    if (statbuf.st_mode & S_IDIRECTORY)
      ((PFILEFINDBUF3)*pFindBuf)->attrFile |= FILE_DIRECTORY;

    if (statbuf.st_mode & S_IARCHIVED)
      ((PFILEFINDBUF3)*pFindBuf)->attrFile |= FILE_ARCHIVED;

    // search for the last slash
    for (p = fname + strlen(fname); p > fname && *p != '/'; p--) ;
    if (*p == '/') p++;
    
    ((PFILEFINDBUF3)*pFindBuf)->cchName = strlen(p) + 1;
    strcpy(((PFILEFINDBUF3)*pFindBuf)->achName, p);
    
    ((PFILEFINDBUF3)*pFindBuf)->cbFile = statbuf.st_size;
    ((PFILEFINDBUF3)*pFindBuf)->cbFileAlloc = statbuf.st_blksize * statbuf.st_blocks;
      
    *pFindBuf += sizeof(FILEFINDBUF3);
    k++;
  }

  *cbBuf = k * sizeof(FILEFINDBUF3);
  *pFindBuf    -= *cbBuf;
  *pcFileNames = (ULONG)k;
  hdir->ctr = i;

  //if (hdir == &thehdir)
  //    hDir = HDIR_SYSTEM;
  //else
  //    hDir = (HDIR)hdir;


  return NO_ERROR;
}


APIRET DICE_CV
os2fs_dos_FindClose_component (CORBA_Object _dice_corba_obj,
                               HDIR hDir /* in */,
                               CORBA_Server_Environment *_dice_corba_env)
{
  filefindstruc_t *hdir;

  hdir = (filefindstruc_t *)hDir;

  if (hDir == HDIR_SYSTEM)
    hdir = &thehdir;

  globfree(&hdir->g);  
  
  if (hdir != &thehdir)
    free(hdir);

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
    for (h = fileroot; h->hfile != hFile; h = h->next) ;
    
    if (h == NULL)
      return ERROR_INVALID_HANDLE;

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
    for (h = fileroot; h->hfile != hFile; h = h->next) ;
    
    if (h == NULL)
      return ERROR_INVALID_HANDLE;

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
  
    rc = fcntl(hFile, F_SETFL, mode);

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

    h->openmode = mode;
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
    struct stat buf;
    struct tm brokentime;
    int rc;

    /* check args */
    if (ulInfoLevel != FIL_STANDARD)
        return ERROR_INVALID_LEVEL;

    if (*cbInfoBuf < sizeof(FILESTATUS3))
        return ERROR_BUFFER_OVERFLOW;

    /* get the file status */
    rc = fstat(hf, &buf);
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
    CORBA_Environment env = dice_default_environment;
    PSZ pszNewName;
    char fn[CCHMAXPATH];
    char fname[CCHMAXPATH];
    char str[CCHMAXPATH];
    char *p, *q, *r, *s;
    struct stat buf;
    struct tm brokentime;
    ULONG len, map;
    int rc;
    int  i;

    strcpy(fn, pszPathName);

    for (i = 0; fn[i]; i++)
    {
      if (fn[i] == '/')
        fn[i] = '\\';
    }

    /* check args */
    if (*cbInfoBuf < sizeof(FILESTATUS3))
      return ERROR_BUFFER_OVERFLOW;

    switch (ulInfoLevel)
    {
    case FIL_STANDARD:
        /* convert the file names */
        //pszNewName = alloca(strlen(fn) + 1);

        //if (pszNewName == NULL)
        //  return ERROR_SHARING_BUFFER_EXCEEDED;

	if (pathconv(&pszNewName, fn))
          return ERROR_PATH_NOT_FOUND;
    
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
        
	break;
    case FIL_QUERYFULLNAME:
        if (fn[1] == ':')
        {
	  strcpy(fname, fn);
          len = strlen(fn);
        }
	else
	{
          setdrivemap(&map);
          len = sizeof(fname);
          rc = os2server_dos_QueryCurrentDir_call(&os2srv, 0, map, (char **)&fname, &len, &env);
          //rc = kalQueryCurrentDir(0, (char **)&fname, &len);
          fname[len] = '\0';
          strcat(fname, fn); // @todo implement global chars processing
          len += strlen(fn);
	}

        if (*cbInfoBuf <= len)
          return ERROR_BUFFER_OVERFLOW;

        strcpy(*pInfo, fname);
        s = *pInfo;

        if (fname == NULL)
        {
          *s = '\0';
          return NO_ERROR;
        }

        p = q = fname;

        for (r = p; *r; r++)
          if (*r == '/')
            *r = '\\';

        if (!strcmp(fname, "\\"))
        {
          *s = '\0';
          return NO_ERROR;
        }

        do
        {
          p = strstr(p, "\\");

          if (p)
          {
            strncpy(str, q, p - q);
            str[p - q] = '\0';
            p++;
          }
          else
            strcpy(str, q);

          cdir(&s, str);
          q = p;
        }
        while (p);
	break;
    default:
        return ERROR_INVALID_LEVEL;
    }

    return NO_ERROR;
}


APIRET DICE_CV
os2fs_dos_ResetBuffer_component (CORBA_Object _dice_corba_obj,
                                 HFILE hFile /* in */,
                                 CORBA_Server_Environment *_dice_corba_env)
{
#if 0
  int rc;

  //if (strcmp(pfh->id, "FIL") == 0)
  {
    rc = fsync(hfile);
    
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
  int rc;

  rc = ftruncate(hFile, cbSize);

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
  //filehandle_t *h;
  //struct tm brokentime;
  //struct utimbuf tb;
  //struct timeval tv[2];
  //struct stat buf;
  //ULONG mode;
  //int rc;

#if 0
  for (h = fileroot; h->hfile != hf; h = h->next) ;
    
  if (h == NULL)
    return ERROR_INVALID_HANDLE;

  mode = h->openmode;
  
  if (!(mode & OPEN_ACCESS_READWRITE))
    return ERROR_ACCESS_DENIED;
    
  if (!(mode & OPEN_SHARE_DENYREADWRITE))
    return ERROR_ACCESS_DENIED;
    
  if (ulInfoLevel != FIL_STANDARD)
    return ERROR_INVALID_LEVEL;
    
  /* check args */
  if (*cbInfoBuf < sizeof(FILESTATUS3))
    return ERROR_BUFFER_OVERFLOW;

  rc = fstat(hf, &buf);
  
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
    rc = utimes(hf, tv);
    
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
#endif
  
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
  char fn[CCHMAXPATH];
  int  i;

  strcpy(fn, pszPathName);
  
  for (i = 0; fn[i]; i++)
  {
    if (fn[i] == '/')
      fn[i] = '\\';
  }

  return NO_ERROR;
}

/* changes the current directory in '*dir'
   by one path component in 'component' */
int cdir(char **dir, char *component)
{
  char *p;

  if (!strcmp(component, ".."))
  {
    if (**dir)
    {
      p = *dir + strlen(*dir);

      // find last backslash position
      while (p >= *dir && *p != '\\') p--;
      if (p < *dir) p++;
      *p = '\0';
    }
    return 0; /* NO_ERROR */  
  }

  if (!strcmp(component, "."))
    return 0;

  if (*component != '\\')
  {
    if (**dir)
      strcat(*dir, "\\");

    strcat(*dir, component);
  }
  else
    strcpy(*dir, component);

  return 0;
}
