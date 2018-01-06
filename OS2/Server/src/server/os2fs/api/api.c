/*  file/directory API implementations
 *  (on the server side)
 */

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/MountReg.h>
#include <os3/kal.h>
#include <os3/io.h>

/* libc includes */
#include <sys/types.h>
#include <sys/time.h>
#include <stdlib.h>
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
#include <ctype.h>

/* local includes */
#include "api.h"

//extern l4_threadid_t os2srv;

extern struct FSRouter fsrouter;

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
    ULONG ctr;
    ULONG attr;
    ULONG ulInfoLevel;
} filefindstruc_t;

static filefindstruc_t thehdir;

void DosNameConversion(char *pszName);

void setdrivemap(ULONG *map);

int pathconv(char **converted, char *fname);
int cdir(char **dir, char *component);

int pathconv(char **converted, char *fname)
{
    struct I_Fs_srv *fsrv;
    char drv;
    char *directory;
    char *name;
    char *newfilename;
    char *newdirectory;

    drv = tolower(get_drv(fname));

    if(drv == '\0')
    {
        return 1;
    }

    directory = get_directory(fname);

    if (! directory)
    {
        return 1;
    }

    name = get_name(fname);

    DosNameConversion(directory);
    DosNameConversion(name);

    // @todo release memory allocated here by malloc
    if (drv >= 'c' && drv <= 'z')
    {
        fsrv = FSRouter_route(&fsrouter, drv);

        newdirectory=malloc(strlen(fsrv->mountpoint)+
                            strlen(directory)+1);

        strcpy(newdirectory, fsrv->mountpoint);
        newdirectory=strcat(newdirectory, directory);
    }
    else
    {
        newdirectory = malloc(strlen(directory) + 1);
        strcpy(newdirectory, directory);
    }

    newfilename=malloc(strlen(newdirectory)+
                       strlen(name)+1);

    strcpy(newfilename, newdirectory);
    newfilename=strcat(newfilename, name);
    *converted = newfilename;

    return 0;
}

APIRET FSRead(HFILE hFile,
              char *pBuffer,
              ULONG *count)

{
    ssize_t nread = 0;

    nread = read(hFile, pBuffer, *count);

    if (nread == -1)
    {
        io_log("read() error, errno=%d\n", errno);
        switch (errno)
        {
        // @todo: more accurate error handling
        default:
            return ERROR_NO_DATA;
        }
    }
    *count = (ULONG)nread;

    return NO_ERROR;
}


APIRET FSWrite(HFILE hFile,
               PBYTE pBuffer,
               ULONG *count)
{
    ssize_t nwritten;

    nwritten = write(hFile, pBuffer, *count);

    if (nwritten == -1)
    {
        io_log("write() error, errno=%d\n", errno);
        switch (errno)
        {
        // @todo: more accurate error handling
        default:
            return ERROR_NO_DATA;
        }
    }

    *count = (ULONG)nwritten;

    return NO_ERROR;
}

APIRET FSSetFilePtrL(HFILE handle,
                     long long ib,
                     ULONG method,
                     unsigned long long *ibActual)
{
    long ret;
    long long pos;
    long long len;
    struct stat stat;

    ret = fstat(handle, &stat);

    if (ret == -1)
        return ERROR_INVALID_HANDLE;

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
        return ERROR_NEGATIVE_SEEK;

    ret = lseek(handle, pos, SEEK_SET);

    if (ret == -1)
        return ERROR_SEEK_ON_DEVICE;

    *ibActual = (unsigned long long)pos;

    return NO_ERROR;
}

APIRET FSClose(HFILE hFile)
{
    int ret;

    ret = close(hFile);

    if (ret == - 1)
    {
        if (errno == EBADF)
            return ERROR_INVALID_HANDLE;

        return ERROR_ACCESS_DENIED;
    }

    return NO_ERROR;
}


APIRET FSQueryHType(HFILE handle,
                    ULONG *pType,
                    ULONG *pAttr)
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
        return ERROR_INVALID_HANDLE;

    m = stat.st_mode;

    if (S_ISREG(m))
    {
        *pType = 0; // disk file
        *pAttr = handle;
        return NO_ERROR;
    }

    if (S_ISCHR(m))
    {
        *pType = 1; // character device
        *pAttr = 0;
        return NO_ERROR;
    }

    if (S_ISFIFO(m))
    {
        *pType = 2; // pipe
        *pAttr = 0;
        return NO_ERROR;
    }

    return NO_ERROR;
}

APIRET FSOpenL(PSZ pszFileName,
               HFILE *phFile,
               ULONG *pulAction,
               LONGLONG cbFile,
               ULONG ulAttribute,
               ULONG fsOpenFlags,
               ULONG fsOpenMode,
               EAOP2 *peaop2)
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
        return ERROR_ACCESS_DENIED;

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
        return ERROR_PATH_NOT_FOUND;

    if (strlen(newfilename) + 1 > 256)
        return ERROR_FILENAME_EXCED_RANGE;

    if (! stat(newfilename, &st))
        file_existed = 1;

    handle = open(newfilename, mode);

    if (handle == -1)
    {
        if (errno == EMFILE)
            return ERROR_TOO_MANY_OPEN_FILES;

        if (errno == ENOENT)
            return ERROR_FILE_NOT_FOUND;

        return ERROR_ACCESS_DENIED;
    }

    if (fsOpenFlags & OPEN_ACTION_FAIL_IF_EXISTS)
    {
        close(handle);
        return ERROR_OPEN_FAILED;
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

    // @todo release this structure in FSClose
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

    return NO_ERROR;
}

APIRET FSDupHandle(HFILE hFile,
                   HFILE *phFile2)
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
                return ERROR_INVALID_HANDLE;

            if (errno == EMFILE)
                return ERROR_TOO_MANY_OPEN_FILES;
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
                    return ERROR_INVALID_HANDLE;
                else
                    return ERROR_INVALID_TARGET_HANDLE;
            }
            else if (errno == EMFILE)
                return ERROR_TOO_MANY_OPEN_FILES;
        }
    }
#else
    return ERROR_INVALID_TARGET_HANDLE;
#endif
    h2 = (filehandle_t *)malloc(sizeof(filehandle_t));
    h2->openmode = h->openmode;
    h2->hfile = handle;
    h2->next = NULL;

    p->next = h2;

    *phFile2 = (HFILE)h2;

    return NO_ERROR;
}

APIRET FSDelete(PSZ pszFileName)
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
            return ERROR_FILENAME_EXCED_RANGE;

        if (errno == ENOTDIR)
            return ERROR_PATH_NOT_FOUND;

        if (errno == EISDIR)
            return ERROR_INVALID_PARAMETER;

        if (errno == ENOENT)
            return ERROR_FILE_NOT_FOUND;

        if (errno == EACCES)
            return ERROR_ACCESS_DENIED;

        if (errno == EPERM)
            return ERROR_SHARING_VIOLATION;
    }

    return NO_ERROR;
}


APIRET FSForceDelete(PSZ pszFileName)
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
            return ERROR_FILENAME_EXCED_RANGE;

        if (errno == ENOTDIR)
            return ERROR_PATH_NOT_FOUND;

        if (errno == EISDIR)
            return ERROR_INVALID_PARAMETER;

        if (errno == ENOENT)
            return ERROR_FILE_NOT_FOUND;

        if (errno == EACCES)
            return ERROR_ACCESS_DENIED;

        if (errno == EPERM)
            return ERROR_SHARING_VIOLATION;
     }

     return NO_ERROR;
}


APIRET FSDeleteDir(PSZ pszDirName)
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
            return ERROR_FILENAME_EXCED_RANGE;

        if (errno == ENOTDIR)
            return ERROR_PATH_NOT_FOUND;

        if (errno == ENOENT)
            return ERROR_FILE_NOT_FOUND;

        if (errno == EACCES)
            return ERROR_ACCESS_DENIED;

        if (errno == EPERM)
            return ERROR_SHARING_VIOLATION;

        if (errno == EBUSY)
            return ERROR_CURRENT_DIRECTORY;

        if (errno)
            return ERROR_ACCESS_DENIED;
    }

    return NO_ERROR;
}


APIRET FSCreateDir(PSZ pszDirName,
                   EAOP2 *peaop2)
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
            return ERROR_FILENAME_EXCED_RANGE;

        if (errno == ENOTDIR)
            return ERROR_PATH_NOT_FOUND;

        if (errno == EACCES)
            return ERROR_ACCESS_DENIED;

        if (errno == EPERM)
            return ERROR_SHARING_VIOLATION;

        if (errno)
            return ERROR_ACCESS_DENIED;
    }

    return NO_ERROR;
}


APIRET FSFindFirst(PSZ pszFileSpec,
                   HDIR  *phDir,
                   ULONG flAttribute,
                   char  **pFindBuf,
                   ULONG *cbBuf,
                   ULONG *pcFileNames,
                   ULONG ulInfoLevel)
{
    filefindstruc_t *hdir;
    struct stat statbuf;
    struct tm tt;
    char *t, *p;
    char *fname;
    int  rc, i, j;
    char fn[CCHMAXPATH];

    strcpy(fn, pszFileSpec);

    for (i = 0; fn[i]; i++)
    {
      if (fn[i] == '/')
          fn[i] = '\\';
    }

    io_log("fn=%s\n", fn);

    if (*phDir == HDIR_SYSTEM)
        hdir = &thehdir;
    else if (*phDir == HDIR_CREATE)
        hdir = (filefindstruc_t *)malloc(sizeof(filefindstruc_t));
    else
        return ERROR_INVALID_HANDLE;

    hdir->ulInfoLevel = ulInfoLevel;
    hdir->attr        = flAttribute;
    hdir->ctr         = 0;

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

    switch (ulInfoLevel)
    {
    case FIL_STANDARD:
        {
            PFILEFINDBUF3 info = (PFILEFINDBUF3)*pFindBuf;

            // check for buffer overflow, and whether it is <= 64k
            if (*cbBuf == 0 || *cbBuf > 0x10000 ||
                *pcFileNames * sizeof(FILEFINDBUF3) > *cbBuf)
                return ERROR_BUFFER_OVERFLOW;

            for (i = 0, j = 0;
                 j < *pcFileNames && i < hdir->g.gl_pathc;
                 i++)
            {
                if (j + 1 == *pcFileNames)
                    info->oNextEntryOffset = 0;
                else
                    info->oNextEntryOffset = sizeof(FILEFINDBUF3);

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

                info->fdateCreation.year  = tt.tm_year - 80;
                info->fdateCreation.month = tt.tm_mon  + 1;
                info->fdateCreation.day   = tt.tm_mday;

                info->ftimeCreation.hours   = tt.tm_hour;
                info->ftimeCreation.minutes = tt.tm_min;
                info->ftimeCreation.twosecs = tt.tm_sec / 2;

                localtime_r(&statbuf.st_atime, &tt);

                info->fdateLastAccess.year  = tt.tm_year - 80;
                info->fdateLastAccess.month = tt.tm_mon  + 1;
                info->fdateLastAccess.day   = tt.tm_mday;

                info->ftimeLastAccess.hours   = tt.tm_hour;
                info->ftimeLastAccess.minutes = tt.tm_min;
                info->ftimeLastAccess.twosecs = tt.tm_sec / 2;

                localtime_r(&statbuf.st_mtime, &tt);

                info->fdateLastWrite.year  = tt.tm_year - 80;
                info->fdateLastWrite.month = tt.tm_mon  + 1;
                info->fdateLastWrite.day   = tt.tm_mday;

                info->ftimeLastWrite.hours   = tt.tm_hour;
                info->ftimeLastWrite.minutes = tt.tm_min;
                info->ftimeLastWrite.twosecs = tt.tm_sec / 2;

                info->attrFile = 0;

                if (statbuf.st_mode & S_IREADONLY)
                    info->attrFile |= FILE_READONLY;

                if (statbuf.st_mode & S_IHIDDEN)
                    info->attrFile |= FILE_HIDDEN;

                if (statbuf.st_mode & S_ISYSTEM)
                    info->attrFile |= FILE_SYSTEM;

                if (statbuf.st_mode & S_IDIRECTORY)
                    info->attrFile |= FILE_DIRECTORY;

                if (statbuf.st_mode & S_IARCHIVED)
                    info->attrFile |= FILE_ARCHIVED;

                // search for the last slash
                for (p = fname + strlen(fname); p > fname && *p != '/'; p--) ;
                if (*p == '/') p++;

                info->cchName = strlen(p) + 1;
                strcpy(info->achName, p);

                info->cbFile = statbuf.st_size;
                info->cbFileAlloc = statbuf.st_blksize * statbuf.st_blocks;

                *pFindBuf += sizeof(FILEFINDBUF3);
                j++;
            }
        }

        *cbBuf       = j * sizeof(FILEFINDBUF3);
        break;

    case FIL_STANDARDL:
        {
            PFILEFINDBUF3L info = (PFILEFINDBUF3L)*pFindBuf;

            // check for buffer overflow, and whether it is <= 64k
            if (*cbBuf == 0 || *cbBuf > 0x10000 ||
                *pcFileNames * sizeof(FILEFINDBUF3L) > *cbBuf)
                return ERROR_BUFFER_OVERFLOW;

            for (i = 0, j = 0;
                 j < *pcFileNames && i < hdir->g.gl_pathc;
                 i++)
            {
                if (j + 1 == *pcFileNames)
                    info->oNextEntryOffset = 0;
                else
                    info->oNextEntryOffset = sizeof(FILEFINDBUF3L);

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

                info->fdateCreation.year  = tt.tm_year - 80;
                info->fdateCreation.month = tt.tm_mon  + 1;
                info->fdateCreation.day   = tt.tm_mday;

                info->ftimeCreation.hours   = tt.tm_hour;
                info->ftimeCreation.minutes = tt.tm_min;
                info->ftimeCreation.twosecs = tt.tm_sec / 2;

                localtime_r(&statbuf.st_atime, &tt);

                info->fdateLastAccess.year  = tt.tm_year - 80;
                info->fdateLastAccess.month = tt.tm_mon  + 1;
                info->fdateLastAccess.day   = tt.tm_mday;

                info->ftimeLastAccess.hours   = tt.tm_hour;
                info->ftimeLastAccess.minutes = tt.tm_min;
                info->ftimeLastAccess.twosecs = tt.tm_sec / 2;

                localtime_r(&statbuf.st_mtime, &tt);

                info->fdateLastWrite.year  = tt.tm_year - 80;
                info->fdateLastWrite.month = tt.tm_mon  + 1;
                info->fdateLastWrite.day   = tt.tm_mday;

                info->ftimeLastWrite.hours   = tt.tm_hour;
                info->ftimeLastWrite.minutes = tt.tm_min;
                info->ftimeLastWrite.twosecs = tt.tm_sec / 2;

                info->attrFile = 0;

                if (statbuf.st_mode & S_IREADONLY)
                    info->attrFile |= FILE_READONLY;

                if (statbuf.st_mode & S_IHIDDEN)
                    info->attrFile |= FILE_HIDDEN;

                if (statbuf.st_mode & S_ISYSTEM)
                    info->attrFile |= FILE_SYSTEM;

                if (statbuf.st_mode & S_IDIRECTORY)
                    info->attrFile |= FILE_DIRECTORY;

                if (statbuf.st_mode & S_IARCHIVED)
                    info->attrFile |= FILE_ARCHIVED;

                // search for the last slash
                for (p = fname + strlen(fname); p > fname && *p != '/'; p--) ;
                if (*p == '/') p++;

                info->cchName = strlen(p) + 1;
                strcpy(info->achName, p);

                info->cbFile = statbuf.st_size;
                info->cbFileAlloc = statbuf.st_blksize * statbuf.st_blocks;

                *pFindBuf += sizeof(FILEFINDBUF3L);
                j++;
            }
        }

        *cbBuf = j * sizeof(FILEFINDBUF3L);
        break;

    default:
        return ERROR_INVALID_LEVEL;
    }

    *pFindBuf    -= *cbBuf;
    *pcFileNames = (ULONG)j;
    hdir->ctr    = j;

    if (hdir == &thehdir)
        *phDir = HDIR_SYSTEM;
    else
        *phDir = (HDIR)hdir;


    return NO_ERROR;
}


APIRET FSFindNext(HDIR  hDir,
                  char  **pFindBuf,
                  ULONG *cbBuf,
                  ULONG *pcFileNames)
{
    filefindstruc_t *hdir;
    struct stat statbuf;
    struct tm tt;
    char *p, *fname;
    int  rc, i, j, k;

    hdir = (filefindstruc_t *)(hDir);

    if (hDir == HDIR_SYSTEM)
        hdir = &thehdir;

    if (hdir->ctr == hdir->g.gl_pathc)
        return ERROR_NO_MORE_FILES;

    switch (hdir->ulInfoLevel)
    {
    case FIL_STANDARD:
        {
            PFILEFINDBUF3 info = (PFILEFINDBUF3)*pFindBuf;

            // check for buffer overflow, and whether it is <= 64k
            if (*cbBuf == 0 || *cbBuf > 0x10000 ||
                *pcFileNames * sizeof(FILEFINDBUF3) > *cbBuf)
                return ERROR_BUFFER_OVERFLOW;

            for (i = hdir->ctr, j = 0, k = 0;
                 k < *pcFileNames && i < hdir->g.gl_pathc;
                 i++, j++)
            {
                if (k + 1 == *pcFileNames)
                    info->oNextEntryOffset = 0;
                else
                    info->oNextEntryOffset = sizeof(FILEFINDBUF3);

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

                info->fdateCreation.year  = tt.tm_year - 80;
                info->fdateCreation.month = tt.tm_mon  + 1;
                info->fdateCreation.day   = tt.tm_mday;

                info->ftimeCreation.hours   = tt.tm_hour;
                info->ftimeCreation.minutes = tt.tm_min;
                info->ftimeCreation.twosecs = tt.tm_sec / 2;

                localtime_r(&statbuf.st_atime, &tt);

                info->fdateLastAccess.year  = tt.tm_year - 80;
                info->fdateLastAccess.month = tt.tm_mon  + 1;
                info->fdateLastAccess.day   = tt.tm_mday;

                info->ftimeLastAccess.hours   = tt.tm_hour;
                info->ftimeLastAccess.minutes = tt.tm_min;
                info->ftimeLastAccess.twosecs = tt.tm_sec / 2;

                localtime_r(&statbuf.st_mtime, &tt);

                info->fdateLastWrite.year  = tt.tm_year - 80;
                info->fdateLastWrite.month = tt.tm_mon  + 1;
                info->fdateLastWrite.day   = tt.tm_mday;

                info->ftimeLastWrite.hours   = tt.tm_hour;
                info->ftimeLastWrite.minutes = tt.tm_min;
                info->ftimeLastWrite.twosecs = tt.tm_sec / 2;

                info->attrFile = 0;

                if (statbuf.st_mode & S_IREADONLY)
                    info->attrFile |= FILE_READONLY;

                if (statbuf.st_mode & S_IHIDDEN)
                    info->attrFile |= FILE_HIDDEN;

                if (statbuf.st_mode & S_ISYSTEM)
                    info->attrFile |= FILE_SYSTEM;

                if (statbuf.st_mode & S_IDIRECTORY)
                    info->attrFile |= FILE_DIRECTORY;

                if (statbuf.st_mode & S_IARCHIVED)
                    info->attrFile |= FILE_ARCHIVED;

                // search for the last slash
                for (p = fname + strlen(fname); p > fname && *p != '/'; p--) ;
                if (*p == '/') p++;

                info->cchName = strlen(p) + 1;
                strcpy(info->achName, p);

                info->cbFile = statbuf.st_size;
                info->cbFileAlloc = statbuf.st_blksize * statbuf.st_blocks;

                *pFindBuf += sizeof(FILEFINDBUF3);
                k++;
            }
        }

        *cbBuf = k * sizeof(FILEFINDBUF3);
        break;

    case FIL_STANDARDL:
        {
            PFILEFINDBUF3L info = (PFILEFINDBUF3L)*pFindBuf;

            // check for buffer overflow, and whether it is <= 64k
            if (*cbBuf == 0 || *cbBuf > 0x10000 ||
                *pcFileNames * sizeof(FILEFINDBUF3L) > *cbBuf)
                return ERROR_BUFFER_OVERFLOW;

            for (i = hdir->ctr, j = 0, k = 0;
                 k < *pcFileNames && i < hdir->g.gl_pathc;
                 i++, j++)
            {
                if (k + 1 == *pcFileNames)
                    info->oNextEntryOffset = 0;
                else
                    info->oNextEntryOffset = sizeof(FILEFINDBUF3L);

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

                info->fdateCreation.year  = tt.tm_year - 80;
                info->fdateCreation.month = tt.tm_mon  + 1;
                info->fdateCreation.day   = tt.tm_mday;

                info->ftimeCreation.hours   = tt.tm_hour;
                info->ftimeCreation.minutes = tt.tm_min;
                info->ftimeCreation.twosecs = tt.tm_sec / 2;

                localtime_r(&statbuf.st_atime, &tt);

                info->fdateLastAccess.year  = tt.tm_year - 80;
                info->fdateLastAccess.month = tt.tm_mon  + 1;
                info->fdateLastAccess.day   = tt.tm_mday;

                info->ftimeLastAccess.hours   = tt.tm_hour;
                info->ftimeLastAccess.minutes = tt.tm_min;
                info->ftimeLastAccess.twosecs = tt.tm_sec / 2;

                localtime_r(&statbuf.st_mtime, &tt);

                info->fdateLastWrite.year  = tt.tm_year - 80;
                info->fdateLastWrite.month = tt.tm_mon  + 1;
                info->fdateLastWrite.day   = tt.tm_mday;

                info->ftimeLastWrite.hours   = tt.tm_hour;
                info->ftimeLastWrite.minutes = tt.tm_min;
                info->ftimeLastWrite.twosecs = tt.tm_sec / 2;

                info->attrFile = 0;

                if (statbuf.st_mode & S_IREADONLY)
                    info->attrFile |= FILE_READONLY;

                if (statbuf.st_mode & S_IHIDDEN)
                    info->attrFile |= FILE_HIDDEN;

                if (statbuf.st_mode & S_ISYSTEM)
                    info->attrFile |= FILE_SYSTEM;

                if (statbuf.st_mode & S_IDIRECTORY)
                    info->attrFile |= FILE_DIRECTORY;

                if (statbuf.st_mode & S_IARCHIVED)
                    info->attrFile |= FILE_ARCHIVED;

                // search for the last slash
                for (p = fname + strlen(fname); p > fname && *p != '/'; p--) ;
                if (*p == '/') p++;

                info->cchName = strlen(p) + 1;
                strcpy(info->achName, p);

                info->cbFile = statbuf.st_size;
                info->cbFileAlloc = statbuf.st_blksize * statbuf.st_blocks;

                *pFindBuf += sizeof(FILEFINDBUF3L);
                k++;
            }
        }

        *cbBuf = k * sizeof(FILEFINDBUF3);
        break;

    default:
        return ERROR_INVALID_LEVEL;
    }

    *pFindBuf    -= *cbBuf;
    *pcFileNames = (ULONG)k;
    hdir->ctr    = i;

    //if (hdir == &thehdir)
    //    hDir = HDIR_SYSTEM;
    //else
    //    hDir = (HDIR)hdir;


    return NO_ERROR;
}


APIRET FSFindClose(HDIR hDir)
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


APIRET FSQueryFHState(HFILE hFile,
                      PULONG pMode)
{
    filehandle_t *h;

    // stdin/stdout/stderr
    if (hFile == 0 || hFile == 1 || hFile == 2)
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


APIRET FSSetFHState(HFILE hFile,
                    ULONG ulMode)
{
    filehandle_t *h;
    int mode;
    int rc;

    // not stdin/stdout/stderr
    if (hFile != 0 && hFile != 1 && hFile != 2)
    {
        for (h = fileroot; h->hfile != hFile; h = h->next) ;

        if (h == NULL)
            return ERROR_INVALID_HANDLE;

        mode = h->openmode;

        if (ulMode & OPEN_FLAGS_WRITE_THROUGH)
            mode |= O_SYNC;

        if (ulMode & OPEN_ACTION_REPLACE_IF_EXISTS)
            mode |= O_TRUNC;

        if (ulMode & OPEN_ACCESS_READONLY)
            mode |= O_RDONLY;
        else if (ulMode & OPEN_ACCESS_WRITEONLY)
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


APIRET FSQueryFileInfo(HFILE hf,
                       ULONG ulInfoLevel,
                       char **pInfo,
                       ULONG *cbInfoBuf)
{
    struct stat buf;
    struct tm brokentime;
    int rc;

    switch (ulInfoLevel)
    {
    case FIL_STANDARD:
        {
            PFILESTATUS3 info = (PFILESTATUS3)*pInfo;

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

            info->fdateCreation.day = (UINT)brokentime.tm_mday;
            info->fdateCreation.month = (UINT)brokentime.tm_mon + 1;
            info->fdateCreation.year = (UINT)brokentime.tm_year - 80;
            info->ftimeCreation.twosecs = (USHORT)brokentime.tm_sec / 2;
            info->ftimeCreation.minutes = (USHORT)brokentime.tm_min;
            info->ftimeCreation.hours = (USHORT)brokentime.tm_hour;

            localtime_r(&buf.st_atime, &brokentime);

            info->fdateLastAccess.day = (UINT)brokentime.tm_mday;
            info->fdateLastAccess.month = (UINT)brokentime.tm_mon + 1;
            info->fdateLastAccess.year = (UINT)brokentime.tm_year - 80;
            info->ftimeLastAccess.twosecs = (USHORT)brokentime.tm_sec / 2;
            info->ftimeLastAccess.minutes = (USHORT)brokentime.tm_min;
            info->ftimeLastAccess.hours = (USHORT)brokentime.tm_hour;

            localtime_r(&buf.st_mtime, &brokentime);

            info->fdateLastWrite.day = (UINT)brokentime.tm_mday;
            info->fdateLastWrite.month = (UINT)brokentime.tm_mon + 1;
            info->fdateLastWrite.year = (UINT)brokentime.tm_year - 80;
            info->ftimeLastWrite.twosecs = (USHORT)brokentime.tm_sec / 2;
            info->ftimeLastWrite.minutes = (USHORT)brokentime.tm_min;
            info->ftimeLastWrite.hours = (USHORT)brokentime.tm_hour;
            info->cbFile = (ULONG)buf.st_size;
            info->cbFileAlloc = (ULONG)(buf.st_blksize * buf.st_blocks);

            if (S_ISDIR(buf.st_mode))
                info->attrFile = FILE_DIRECTORY;
            else
                info->attrFile = (ULONG)0;
        }
        break;

    case FIL_STANDARDL:
        {
            PFILESTATUS3L info = (PFILESTATUS3L)*pInfo;

            if (*cbInfoBuf < sizeof(FILESTATUS3L))
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

            info->fdateCreation.day = (UINT)brokentime.tm_mday;
            info->fdateCreation.month = (UINT)brokentime.tm_mon + 1;
            info->fdateCreation.year = (UINT)brokentime.tm_year - 80;
            info->ftimeCreation.twosecs = (USHORT)brokentime.tm_sec / 2;
            info->ftimeCreation.minutes = (USHORT)brokentime.tm_min;
            info->ftimeCreation.hours = (USHORT)brokentime.tm_hour;

            localtime_r(&buf.st_atime, &brokentime);

            info->fdateLastAccess.day = (UINT)brokentime.tm_mday;
            info->fdateLastAccess.month = (UINT)brokentime.tm_mon + 1;
            info->fdateLastAccess.year = (UINT)brokentime.tm_year - 80;
            info->ftimeLastAccess.twosecs = (USHORT)brokentime.tm_sec / 2;
            info->ftimeLastAccess.minutes = (USHORT)brokentime.tm_min;
            info->ftimeLastAccess.hours = (USHORT)brokentime.tm_hour;

            localtime_r(&buf.st_mtime, &brokentime);

            info->fdateLastWrite.day = (UINT)brokentime.tm_mday;
            info->fdateLastWrite.month = (UINT)brokentime.tm_mon + 1;
            info->fdateLastWrite.year = (UINT)brokentime.tm_year - 80;
            info->ftimeLastWrite.twosecs = (USHORT)brokentime.tm_sec / 2;
            info->ftimeLastWrite.minutes = (USHORT)brokentime.tm_min;
            info->ftimeLastWrite.hours = (USHORT)brokentime.tm_hour;
            info->cbFile = (ULONGLONG)buf.st_size;
            info->cbFileAlloc = (ULONGLONG)(buf.st_blksize * buf.st_blocks);

            if (S_ISDIR(buf.st_mode))
                info->attrFile = FILE_DIRECTORY;
            else
                info->attrFile = (ULONG)0;
        }
        break;

    default:
        return ERROR_INVALID_LEVEL;
    }

    return NO_ERROR;
}


APIRET FSQueryPathInfo(PSZ pszPathName,
                       ULONG ulInfoLevel,
                       char **pInfo,
                       ULONG *cbInfoBuf)
{
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

    switch (ulInfoLevel)
    {
    case FIL_STANDARD:
        {
            PFILESTATUS3 info = (PFILESTATUS3)*pInfo;

            /* convert the file names */
            //pszNewName = alloca(strlen(fn) + 1);

            //if (pszNewName == NULL)
            //  return ERROR_SHARING_BUFFER_EXCEEDED;

            /* check args */
            if (*cbInfoBuf < sizeof(FILESTATUS3))
                return ERROR_BUFFER_OVERFLOW;

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

            info->fdateCreation.day = (UINT)brokentime.tm_mday;
            info->fdateCreation.month = (UINT)brokentime.tm_mon + 1;
            info->fdateCreation.year = (UINT)brokentime.tm_year + 1900;
            info->ftimeCreation.twosecs = (USHORT)brokentime.tm_sec / 2;
            info->ftimeCreation.minutes = (USHORT)brokentime.tm_min;
            info->ftimeCreation.hours = (USHORT)brokentime.tm_hour;

            localtime_r(&buf.st_atime, &brokentime);

            info->fdateLastAccess.day = (UINT)brokentime.tm_mday;
            info->fdateLastAccess.month = (UINT)brokentime.tm_mon + 1;
            info->fdateLastAccess.year = (UINT)brokentime.tm_year + 1900;
            info->ftimeLastAccess.twosecs = (USHORT)brokentime.tm_sec / 2;
            info->ftimeLastAccess.minutes = (USHORT)brokentime.tm_min;
            info->ftimeLastAccess.hours = (USHORT)brokentime.tm_hour;

            localtime_r(&buf.st_mtime, &brokentime);

            info->fdateLastWrite.day = (UINT)brokentime.tm_mday;
            info->fdateLastWrite.month = (UINT)brokentime.tm_mon + 1;
            info->fdateLastWrite.year = (UINT)brokentime.tm_year + 1900;
            info->ftimeLastWrite.twosecs = (USHORT)brokentime.tm_sec / 2;
            info->ftimeLastWrite.minutes = (USHORT)brokentime.tm_min;
            info->ftimeLastWrite.hours = (USHORT)brokentime.tm_hour;
            info->cbFile = (ULONG)buf.st_size;
            info->cbFileAlloc = (ULONG)(buf.st_blksize * buf.st_blocks);

            if (S_ISDIR(buf.st_mode))
                info->attrFile = FILE_DIRECTORY;
            else
                info->attrFile = (ULONG)0;
        }
        break;

        case FIL_STANDARDL:
        {
            PFILESTATUS3L info = (PFILESTATUS3L)*pInfo;

            /* convert the file names */
            //pszNewName = alloca(strlen(fn) + 1);

            //if (pszNewName == NULL)
            //  return ERROR_SHARING_BUFFER_EXCEEDED;

            /* check args */
            if (*cbInfoBuf < sizeof(FILESTATUS3))
                return ERROR_BUFFER_OVERFLOW;

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

            info->fdateCreation.day = (UINT)brokentime.tm_mday;
            info->fdateCreation.month = (UINT)brokentime.tm_mon + 1;
            info->fdateCreation.year = (UINT)brokentime.tm_year + 1900;
            info->ftimeCreation.twosecs = (USHORT)brokentime.tm_sec / 2;
            info->ftimeCreation.minutes = (USHORT)brokentime.tm_min;
            info->ftimeCreation.hours = (USHORT)brokentime.tm_hour;

            localtime_r(&buf.st_atime, &brokentime);

            info->fdateLastAccess.day = (UINT)brokentime.tm_mday;
            info->fdateLastAccess.month = (UINT)brokentime.tm_mon + 1;
            info->fdateLastAccess.year = (UINT)brokentime.tm_year + 1900;
            info->ftimeLastAccess.twosecs = (USHORT)brokentime.tm_sec / 2;
            info->ftimeLastAccess.minutes = (USHORT)brokentime.tm_min;
            info->ftimeLastAccess.hours = (USHORT)brokentime.tm_hour;

            localtime_r(&buf.st_mtime, &brokentime);

            info->fdateLastWrite.day = (UINT)brokentime.tm_mday;
            info->fdateLastWrite.month = (UINT)brokentime.tm_mon + 1;
            info->fdateLastWrite.year = (UINT)brokentime.tm_year + 1900;
            info->ftimeLastWrite.twosecs = (USHORT)brokentime.tm_sec / 2;
            info->ftimeLastWrite.minutes = (USHORT)brokentime.tm_min;
            info->ftimeLastWrite.hours = (USHORT)brokentime.tm_hour;
            info->cbFile = (ULONGLONG)buf.st_size;
            info->cbFileAlloc = (ULONGLONG)(buf.st_blksize * buf.st_blocks);

            if (S_ISDIR(buf.st_mode))
                info->attrFile = FILE_DIRECTORY;
            else
                info->attrFile = (ULONG)0;
        }
        break;

    case FIL_QUERYFULLNAME:
        {
            if (fn[1] == ':')
            {
                strcpy(fname, fn);
                len = strlen(fn);
            }
            else
            {
                setdrivemap(&map);
                len = sizeof(fname);
                ////rc = os2server_dos_QueryCurrentDir_call(&os2srv, 0, map, (char **)&fname, &len, &env); // !!!!!

                ////if (rc)
                    ////return rc;

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
            {
                if (*r == '/')
                    *r = '\\';
            }

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
        }
        break;

    default:
        return ERROR_INVALID_LEVEL;
    }

    return NO_ERROR;
}


APIRET FSResetBuffer(HFILE hFile)
{
#if 0
    int rc;

    //if (strcmp(pfh->id, "FIL") == 0)
    {
        rc = fsync(hFile);

        if (rc == -1)
        {
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
    return NO_ERROR;
#endif
}


APIRET FSSetFileSizeL(HFILE hFile,
                      LONGLONG cbSize)
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
    return NO_ERROR;
#endif
}


/* 1) no EA support yet :(
 * 2) only times are changed
 */
APIRET FSSetFileInfo(HFILE hf,
                     ULONG ulInfoLevel,
                     char **pInfoBuf,
                     ULONG *cbInfoBuf)
{
#if 0
    filehandle_t *h;
    struct tm brokentime;
    struct utimbuf tb;
    struct timeval tv[2];
    struct stat buf;
    ULONG mode;
    int rc;

    for (h = fileroot; h->hfile != hf; h = h->next) ;

    if (h == NULL)
        return ERROR_INVALID_HANDLE;

    mode = h->openmode;

    if (! (mode & OPEN_ACCESS_READWRITE) )
        return ERROR_ACCESS_DENIED;

    if (! (mode & OPEN_SHARE_DENYREADWRITE) )
        return ERROR_ACCESS_DENIED;

    switch (ulInfoLevel)
    {
        case FIL_STANDARD:
        {
            PFILESTATUS3 info = (PFILESTATUS3)*pInfoBuf;

            /* check args */
            if (*cbInfoBuf < sizeof(FILESTATUS3))
                return ERROR_BUFFER_OVERFLOW;

            rc = fstat(hf, &buf);

            if (rc)
                return ERROR_INVALID_HANDLE;

            localtime_r(buf.st_atime, &brokentime);

            if (info->fdateLastAccess.day)
                brokentime.tm_mday = info->fdateLastAccess.day;

            if (info->fdateLastAccess.month)
                brokentime.tm_mon = info->fdateLastAccess.month - 1;

            if (info->fdateLastAccess.year)
                brokentime.tm_year = info->fdateLastAccess.year - 1900;

            if (info->ftimeLastAccess.twosecs)
                brokentime.tm_sec = info->ftimeLastAccess.twosecs * 2;

            if (info->ftimeLastAccess.minutes)
                brokentime.tm_min = info->ftimeLastAccess.minutes;

            if (info->ftimeLastAccess.hours)
                brokentime.tm_hour = info->ftimeLastAccess.hours;

            tv[0].tv_sec  = timelocal(&brokentime);
            tv[0].tv_usec = 0;

            localtime_r(buf.st_mtime, &brokentime);

            if (info->fdateLastWrite.day)
                brokentime.tm_mday = info->fdateLastWrite.day;

            if (info->fdateLastWrite.month)
                brokentime.tm_mon = info->fdateLastWrite.month - 1;

            if (info->fdateLastWrite.year)
                brokentime.tm_year = info->fdateLastWrite.year - 1900;

            if (info->ftimeLastWrite.twosecs)
                brokentime.tm_sec = info->ftimeLastWrite.twosecs * 2;

            if (info->ftimeLastWrite.minutes)
                brokentime.tm_min = info->ftimeLastWrite.minutes;

            if (info->ftimeLastWrite.hours)
                brokentime.tm_hour = info->ftimeLastWrite.hours;

            tv[1].tv_sec  = timelocal(&brokentime);
            tv[1].tv_usec = 0;
        }
        break;

        case FIL_STANDARDL:
        {
            PFILESTATUS3L info = (PFILESTATUS3L)*pInfoBuf;

            /* check args */
            if (*cbInfoBuf < sizeof(FILESTATUS3L))
                return ERROR_BUFFER_OVERFLOW;

            rc = fstat(hf, &buf);

            if (rc)
                return ERROR_INVALID_HANDLE;

            localtime_r(buf.st_atime, &brokentime);

            if (info->fdateLastAccess.day)
                brokentime.tm_mday = info->fdateLastAccess.day;

            if (info->fdateLastAccess.month)
                brokentime.tm_mon = info->fdateLastAccess.month - 1;

            if (info->fdateLastAccess.year)
                brokentime.tm_year = info->fdateLastAccess.year - 1900;

            if (info->ftimeLastAccess.twosecs)
                brokentime.tm_sec = info->ftimeLastAccess.twosecs * 2;

            if (info->ftimeLastAccess.minutes)
                brokentime.tm_min = info->ftimeLastAccess.minutes;

            if (info->ftimeLastAccess.hours)
                brokentime.tm_hour = info->ftimeLastAccess.hours;

            tv[0].tv_sec  = timelocal(&brokentime);
            tv[0].tv_usec = 0;

            localtime_r(buf.st_mtime, &brokentime);

            if (info->fdateLastWrite.day)
                brokentime.tm_mday = info->fdateLastWrite.day;

            if (info->fdateLastWrite.month)
                brokentime.tm_mon = info->fdateLastWrite.month - 1;

            if (info->fdateLastWrite.year)
                brokentime.tm_year = info->fdateLastWrite.year - 1900;

            if (info->ftimeLastWrite.twosecs)
                brokentime.tm_sec = info->ftimeLastWrite.twosecs * 2;

            if (info->ftimeLastWrite.minutes)
                brokentime.tm_min = info->ftimeLastWrite.minutes;

            if (info->ftimeLastWrite.hours)
                brokentime.tm_hour = info->ftimeLastWrite.hours;

            tv[1].tv_sec  = timelocal(&brokentime);
            tv[1].tv_usec = 0;
        }
        break;

        default:
            return ERROR_INVALID_LEVEL;
    }

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
APIRET FSSetPathInfo(PSZ pszPathName,
                     ULONG ulInfoLevel,
                     char **pInfoBuf,
                     ULONG *cbInfoBuf,
                     ULONG flOptions)
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

    if (! strcmp(component, ".."))
    {
        if (**dir)
        {
            p = *dir + strlen(*dir);

            // find last backslash position
            while (p >= *dir && *p != '\\') p--;
            if (p < *dir) p++;
            *p = '\0';
        }

        return NO_ERROR;
    }

    if (! strcmp(component, "."))
        return NO_ERROR;

    if (*component != '\\')
    {
        if (**dir)
            strcat(*dir, "\\");

        strcat(*dir, component);
    }
    else
        strcpy(*dir, component);

    return NO_ERROR;
}
