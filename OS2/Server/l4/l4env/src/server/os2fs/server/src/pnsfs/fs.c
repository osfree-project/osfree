/*  
 *
 *
 */

#include <fsd.h>

FSRET FSENTRY pns_INIT(char *szParm
                      ULONG DevHelp,
                      ULONG *pMiniFSD)
{
  return NO_ERROR;
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

FSRET FSENTRY pns_OPENCREATE(struct cdfsi *pcdfsi,
                            struct cdfsd *pcdfsd,
                            char *pName,
                            USHORT iCurDirEnd,
                            struct sffsi *psffsi,
                            struct sffsd *psffsd,
                            ULONG ulOpenMode,
                            USHORT usOpenFlag,
                            USHORT *pusAction,
                            USHORT usAttr,
                            char *pcEABuf,
                            USHORT *pfgenflag)
{
  filehandle_t *h, *p;
  struct stat st;
  char file_existed = 0;
  char *newfilename;
  int mode = 0; //O_BINARY;
  int handle;

  /* ignore DASD opens for now; also, OPEN_FLAGS_FAIL_ON_ERROR
     always for now (before Hard error handling not implemented) */
  if (ulOpenMode & OPEN_FLAGS_DASD)
    return ERROR_ACCESS_DENIED;


  if (ulOpenMode & OPEN_ACCESS_READONLY)
    mode |= O_RDONLY;

  if (ulOpenMode & OPEN_ACCESS_WRITEONLY)
    mode |= O_WRONLY;

  if (ulOpenMode & OPEN_ACCESS_READWRITE)
    mode |= O_RDWR;

  //if (ulOpenMode & OPEN_FLAGS_NOINHERIT)
  //    mode |= O_NOINHERIT;

  if (ulOpenMode & OPEN_SHARE_DENYREAD)
    mode |= O_RDWR | O_EXCL;

  if (ulOpenMode & OPEN_SHARE_DENYWRITE)
    mode |= O_WRONLY | O_EXCL;

  if (ulOpenMode & OPEN_SHARE_DENYREADWRITE)
    mode |= O_RDONLY | O_EXCL;

  if (ulOpenMode & OPEN_SHARE_DENYNONE)
    mode &= ~O_EXCL;

  if (usOpenFlag & OPEN_ACTION_CREATE_IF_NEW)
    mode |= O_CREAT;

  if (usOpenFlag & OPEN_ACTION_REPLACE_IF_EXISTS)
    mode |= O_TRUNC;

  /* convert OS/2-style pathname to PN-style pathname */
  if (pathconv(&newfilename, pName))
    return ERROR_PATH_NOT_FOUND;

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
      return ERROR_TOO_MANY_OPEN_FILES;
    }
      
    if (errno == ENOENT)
    {
      io_log("ERROR_FILE_NOT_FOUND\n");
      return ERROR_FILE_NOT_FOUND;
    }

    io_log("ERROR_ACCESS_DENIED\n");
    return ERROR_ACCESS_DENIED;
  }

  if (usOpenFlag & OPEN_ACTION_FAIL_IF_EXISTS)
  {
    close(handle);
    io_log("ERROR_OPEN_FAILED\n");
    return ERROR_OPEN_FAILED;
  }

  if (usOpenFlag & OPEN_ACTION_OPEN_IF_EXISTS)
  {
    if (file_existed && (mode & ~O_TRUNC))
      *pusAction = FILE_EXISTED;
    else if (file_existed && (mode & O_TRUNC))
      *pusAction = FILE_TRUNCATED;
    else if (!file_existed)
      *pusAction = FILE_CREATED;
  }
  
  h = (filehandle_t *)malloc(sizeof(filehandle_t));
  h->openmode = ulOpenMode;
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

  psffsi->sfi_mode = ulOpenMode;
  psffsi->sfi_tstamp = 3; // ST_SCREAT | ST_PCREAT

  localtime_r(&st.st_ctime, &tt);

  ((FDATE)(psffsi->sfi_cdate)).year    = tt.tm_year - 80;
  ((FDATE)(psffsi->sfi_cdate)).month   = tt.tm_mon + 1;
  ((FDATE)(psffsi->sfi_cdate)).day     = tt.tm_mday;

  ((FTIME)(psffsi->sfi_ctime)).hours   = tt.tm_hour;
  ((FTIME)(psffsi->sfi_ctime)).minutes = tt.tm_min;
  ((FTIME)(psffsi->sfi_ctime)).twosecs = tt.tm_sec / 2;

  localtime_r(&st.st_atime, &tt);

  ((FDATE)(psffsi->sfi_adate)).year    = tt.tm_year - 80;
  ((FDATE)(psffsi->sfi_adate)).month   = tt.tm_mon + 1;
  ((FDATE)(psffsi->sfi_adate)).day     = tt.tm_mday;

  ((FTIME)(psffsi->sfi_atime)).hours   = tt.tm_hour;
  ((FTIME)(psffsi->sfi_atime)).minutes = tt.tm_min;
  ((FTIME)(psffsi->sfi_atime)).twosecs = tt.tm_sec / 2;

  localtime_r(&st.st_mtime, &tt);

  ((FDATE)(psffsi->sfi_mdate)).year    = tt.tm_year - 80;
  ((FDATE)(psffsi->sfi_mdate)).month   = tt.tm_mon + 1;
  ((FDATE)(psffsi->sfi_mdate)).day     = tt.tm_mday;

  ((FTIME)(psffsi->sfi_mtime)).hours   = tt.tm_hour;
  ((FTIME)(psffsi->sfi_mtime)).minutes = tt.tm_min;
  ((FTIME)(psffsi->sfi_mtime)).twosecs = tt.tm_sec / 2;

  psffsi->sfi_size = st.st_size;
  psffsi->sfi_position = 0;
  psffsi->sfi_DOSattr  = FILE_ARCHIVED | FILE_READONLY;

  *((ULONG *)psffsd) = (HFILE)handle;

  io_log("NO_ERROR\n");
  return NO_ERROR;
}

FSRET FSENTRY pns_CHGFILEPTR(struct sffsi *psffsi,
                            struct sffsd *psffsd,
                            long offset,
                            USHORT type,
                            USHORT IOflag)
{
  long off;
  
  switch (type)
  {
    case 0: // from the beginning
      off = offset;
      break;
    case 1: // relative to current file pointer
      off = psffsi->sfi_position + offset;
      break;
    case 2: // realtive to the end of file
      off = psffsi->sfi_size + offset;
      break;
    default:
      return ERROR_INVALID_PARAMETER;
  }
  psffsi->sfi_position = off;
    
  
  return NO_ERROR;
}

FSRET FSENTRY pns_CLOSE(USHORT type,
                       USHORT IOflag,
                       struct sffsi *psffsi,
                       struct sffsd *psffsd)
{
  int ret;
  HFILE handle;
  
  handle = (HFILE)*((ULONG *)(psffsd));
  
  ret = close(handle);
  
  if (ret == - 1)
  {
    if (errno == EBADF)
      return 6; /* ERROR_INVALID_HANDLE */
      
    return 5; /* ERROR_ACCESS_DENIED */
  }


  return NO_ERROR;
}

FSRET FSENTRY pns_READ(struct sffsi *psffsi,
                      struct sffsd *psffsd,
                      char *pData,
                      unsigned short *pLen,
                      USHORT IOflag)
{
  int  c;
  int  nread = 0;
  int  total = 1;
  HFILE handle;
  
  handle = (HFILE)*((ULONG *)(psffsd));
  
  nread = read(handle, pData, *pLen);
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
  *pLen = nread;


  return NO_ERROR;
}

FSRET FSENTRY pns_WRITE(struct sffsi *psffsi,
                       struct sffsd *psffsd,
                       char *pData,
                       unsigned short *pLen,
                       USHORT IOflag)
{
  char *s;
  int  nwritten;
  HFILE handle;
  
  handle = (HFILE)*((ULONG *)(psffsd));
  
  io_log("entered\n");
  nwritten = write(handle, pData, *pLen);
  io_log("in the middle\n");
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

  io_log("nwritten=%u\n", nwritten);
  *pLen = nwritten;
  io_log("exited\n");


  return NO_ERROR;
}

FSRET FSENTRY pns_DELETE(struct cdfsi *pcdfsi,
                        struct cdfsd *pcdfsd,
                        char *pFile,
                        USHORT iCurDirEnd)
{

  if (unlink(pFile))
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


  return NO_ERROR;
}

FSRET FSENTRY pns_RMDIR(struct cdfsi *pcdfsi,
                       struct cdfsd *pcdfsd,
                       char *pName,
                       USHORT iCurDirEnd)
{
  if (rmdir(pName) == -1)
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


  return NO_ERROR;
}

FSRET FSENTRY pns_MKDIR(struct cdfsi *pcdfsi,
                       struct cdfsd *pcdfsd,
                       char *pName,
                       USHORT iCurDirEnd,
                       char *pEABuf,
                       USHORT flags)
{
  if (mkdir(pName, 0) == -1)
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


  return NO_ERROR;
}

FSRET FSENTRY pns_CHDIR(USHORT flag,
                       struct cdsfi *pcdfsi,
                       struct cdfsd *pcdfsd,
                       char *pDir,
                       USHORT iCurDirEnd)
{
  return NO_ERROR;
}
