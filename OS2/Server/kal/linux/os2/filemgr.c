/*-- C -----------------------------------------------------------------------*/
/*                                                                            */
/* Module:      filemgr.c                                                     */
/*                                                                            */
/* Description: This file includes the code to support the file manager.      */
/*                                                                            */
/* Copyright (C) IBM Corporation 2003. All Rights Reserved.                   */
/* Copyright (C) W. David Ashley 2004, 2005. All Rights Reserved.             */
/*                                                                            */
/* Author(s):                                                                 */
/*      W. David Ashley  <dashley@us.ibm.com>                                 */
/*                                                                            */
/* This software is subject to the terms of the Common Public License v1.0.   */
/* You must accept the terms of this license to use this software.            */
/*                                                                            */
/* This program is distributed in the hope that it will be useful, but        */
/* WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY */
/* or FITNESS FOR A PARTICULAR PURPOSE.                                       */
/*                                                                            */
/*----------------------------------------------------------------------------*/


#ifdef HAVE_CONFIG_H
#include "config.h"
#endif  /* #ifdef HAVE_CONFIG_H

/* now include the OS/2 stuff */
#define INCL_NOPMAPI
#define INCL_DOSFILEMGR
#define INCL_DOSPROCESS
#define INCL_ERRORS
#include "os2.h"

/* include the linux headers and our local stuff */
#include "os2linux.h"


/*============================================================================*/
/* OS/2 APIs for the file manager                                             */
/*============================================================================*/


/*----------------------------------------------------------------------------*/
/* DosOpen                                                                    */
/*    Notes:                                                                  */
/*       - lots of flags are not checked in ulAttribute, fsOpenFlags and      */
/*         fsOpenMode                                                         */
/*       - peaop2 is ignored                                                  */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosOpen(PCSZ pszFileName, PHFILE pHf, PULONG pulAction,
                        ULONG cbFile, ULONG ulAttribute, ULONG fsOpenFlags,
                        ULONG fsOpenMode, PEAOP2 peaop2)
{
    SAVEENV;
    PFILEHANDLESTRUCT pfh;
    int mode;
    int flags = 0;
    PSZ pszNewName;
    PSZ pszTmpName;
    char pipe_prefix[7];
    int i;
    int file_exists = FALSE;
    static char tmpdir[] = "/tmp";

    /* check args */
    if (pszFileName == NULL) {
        RESTOREENV_RETURN(ERROR_INVALID_PARAMETER);
    }
    if (*(pszFileName + 1) == ':') {  /* check for drive specifier */
        RESTOREENV_RETURN(ERROR_INVALID_PARAMETER);
    }

    /* allocate memory for the file handles */
    pfh = malloc(sizeof(FILEHANDLESTRUCT));
    if (pfh == NULL) {
        RESTOREENV_RETURN(ERROR_OPEN_FAILED);
    }

    /* is this a standard file or a named pipe? */
    strncpy(pipe_prefix, pszFileName, 6);
    pipe_prefix[6] = '\0';
    for (i = 0; i < strlen(pipe_prefix); i++) {
        pipe_prefix[i] = toupper(pipe_prefix[i]);
    }
    if (strncmp(pipe_prefix, "\\PIPE\\", 6) == 0) {
        strcpy(pfh->id, "PIP");
    }
    else {
        strcpy(pfh->id, "FIL");
    }

    /* set up flags and mode */
    if (fsOpenFlags & OPEN_ACTION_CREATE_IF_NEW) {
        flags = flags | O_CREAT | O_EXCL;
        if (ulAttribute & FILE_READONLY) {
            mode = S_IRGRP | S_IRUSR | S_IROTH;
        }
        else {
            mode = S_IRWXG | S_IRWXU | S_IROTH;
        }
    }
    else if (fsOpenFlags & OPEN_ACTION_REPLACE_IF_EXISTS) {
        flags = flags | O_TRUNC;
    }
    if (fsOpenMode & OPEN_FLAGS_WRITE_THROUGH) {
        flags = flags | O_SYNC;
    }
    if (fsOpenMode & OPEN_ACCESS_READONLY) {
        flags = flags | O_RDONLY;
    }
    else if (fsOpenMode & OPEN_ACCESS_WRITEONLY) {
        flags = flags | O_WRONLY;
    }
    else {
        flags = flags | O_RDWR;
    }

    /* open the file/pipe */
    if (strcmp(pfh->id, "FIL") == 0) {
        /* convert the file name to Linux format */
        pszNewName = alloca(strlen(pszFileName) + 1);
        if (pszNewName == NULL) {
            RESTOREENV_RETURN(ERROR_OPEN_FAILED);
        }
        strcpy(pszNewName, pszFileName);
        DosNameConversion(pszNewName, "\\", "/", FALSE);
        /* see if the file currently exists */
        if (!access(pszNewName, F_OK)) {
            file_exists = TRUE;
        }
        /* open a standard file */
        pfh->fh = open(pszNewName, flags, mode);
        if (pfh->fh == -1) {
            switch (errno) {
            case EACCES:
            case EROFS:
            case ETXTBSY:
            case ENODEV:
            case ENXIO:
                free(pfh);
                RESTOREENV_RETURN(ERROR_ACCESS_DENIED);
            case EFAULT:
            case EINVAL:
                free(pfh);
                RESTOREENV_RETURN(ERROR_INVALID_ACCESS);
            case ENAMETOOLONG:
                free(pfh);
                RESTOREENV_RETURN(ERROR_FILENAME_EXCED_RANGE);
            case ENOSPC:
                free(pfh);
                RESTOREENV_RETURN(ERROR_DISK_FULL);
            case EMFILE:
            case ENFILE:
                free(pfh);
                RESTOREENV_RETURN(ERROR_TOO_MANY_OPEN_FILES);
            case EEXIST:
                *pulAction = FILE_EXISTED;
            default:
                free(pfh);
                RESTOREENV_RETURN(ERROR_OPEN_FAILED);
            }
        }
        /* set up the pulAction return value */
        if (file_exists) {
            if (fsOpenFlags & OPEN_ACTION_REPLACE_IF_EXISTS) {
                *pulAction = FILE_TRUNCATED;
            }
            else {
                *pulAction = FILE_EXISTED;
            }
        }
        else {
            *pulAction = FILE_CREATED;
        }
        /* set the file size if necessary */
        if ((cbFile != 0) && ((fsOpenFlags & OPEN_ACTION_CREATE_IF_NEW) ||
         (fsOpenFlags & OPEN_ACTION_REPLACE_IF_EXISTS))) {
            *pulAction = *pulAction | FILE_TRUNCATED;
            if (ftruncate(pfh->fh, cbFile) == -1) {
                close(pfh->fh);
                free(pfh);
                RESTOREENV_RETURN(ERROR_CANNOT_MAKE);
            }
        }
    }
    else {
        /* convert the pipe name to Linux format so we can find it */
        pszTmpName = alloca(strlen(pszFileName) + 1);
        if (pszTmpName == NULL) {
            RESTOREENV_RETURN(ERROR_OPEN_FAILED);
        }
        pszNewName = alloca(strlen(pszFileName) + strlen(tmpdir) + 1);
        if (pszTmpName == NULL) {
            RESTOREENV_RETURN(ERROR_OPEN_FAILED);
        }
        strcpy(pszTmpName, pszFileName);
        DosNameConversion(pszTmpName, "\\", ".", TRUE);
        if (*pszTmpName == '.') {
            *pszTmpName = '/';
        }
        strcpy(pszNewName, tmpdir);
        strcat(pszNewName, pszTmpName);
        /* open the pipe */
        pfh->fh = open(pszNewName, flags, mode);
        if (pfh->fh == -1) {
            switch (errno) {
            case EACCES:
            case EROFS:
            case ETXTBSY:
            case ENODEV:
            case ENXIO:
                free(pfh);
                RESTOREENV_RETURN(ERROR_ACCESS_DENIED);
            case EFAULT:
            case EINVAL:
                free(pfh);
                RESTOREENV_RETURN(ERROR_INVALID_ACCESS);
            case ENAMETOOLONG:
                free(pfh);
                RESTOREENV_RETURN(ERROR_FILENAME_EXCED_RANGE);
            case ENOSPC:
                free(pfh);
                RESTOREENV_RETURN(ERROR_DISK_FULL);
            case EMFILE:
            case ENFILE:
                free(pfh);
                RESTOREENV_RETURN(ERROR_TOO_MANY_OPEN_FILES);
            case EEXIST:
                *pulAction = FILE_EXISTED;
            default:
                free(pfh);
                RESTOREENV_RETURN(ERROR_OPEN_FAILED);
            }
        }
        *pulAction = FILE_EXISTED;
    }

    pfh->npname = NULL;
    pfh->openmode = fsOpenMode;
    *pHf = (HFILE)pfh;
    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosClose                                                                   */
/*    Notes:                                                                  */
/*       - none                                                               */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosClose(HFILE hFile)
{
    SAVEENV;
    PFILEHANDLESTRUCT pfh;
    int rc;

    /* this will close a file and a named pipe */
    pfh = (PFILEHANDLESTRUCT)hFile;
    rc = close(pfh->fh);
    if (rc) {
        switch (errno) {
        case EBADF:
            RESTOREENV_RETURN(ERROR_INVALID_HANDLE);
        default:
            RESTOREENV_RETURN(ERROR_FILE_NOT_FOUND);
        }
    }

    /* if the pipe was created with DosCreateNPipe then unlink the file */
    if (pfh->npname) {
        unlink(pfh->npname);
        free(pfh->npname);
    }

    free(pfh);

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosRead                                                                    */
/*    Notes:                                                                  */
/*       - none                                                               */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosRead(HFILE hFile, PVOID pBuffer, ULONG cbRead,
                        PULONG pcbActual)
{
    SAVEENV;
    PFILEHANDLESTRUCT pfh;

    /* check args */
    if (cbRead == 0) {
        *pcbActual = 0;
        RESTOREENV_RETURN(NO_ERROR);
    }

    /* read the file/pipe */
    pfh = (PFILEHANDLESTRUCT)hFile;
    *pcbActual = (ULONG)read(pfh->fh, pBuffer, (size_t)cbRead);
    if (*pcbActual == -1) {
        *pcbActual = 0;
        switch (errno) {
        case EFAULT:
        case EPERM:
            RESTOREENV_RETURN(ERROR_ACCESS_DENIED);
        case EAGAIN:
            break;
        case EPIPE:
            RESTOREENV_RETURN(ERROR_PIPE_NOT_CONNECTED);
        default:
            RESTOREENV_RETURN(ERROR_INVALID_HANDLE);
        }
    }

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosWrite                                                                   */
/*    Notes:                                                                  */
/*       - none                                                               */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosWrite(HFILE hFile, CPVOID pBuffer, ULONG cbWrite,
                         PULONG pcbActual)
{
    SAVEENV;
    PFILEHANDLESTRUCT pfh;
    ULONG byteswritten = 0;

    /* check args */
    if (cbWrite == 0) {
        *pcbActual = 0;
        RESTOREENV_RETURN(NO_ERROR);
    }

    /* write the file/pipe */
    pfh = (PFILEHANDLESTRUCT)hFile;
    *pcbActual = 0;
    while (*pcbActual < cbWrite) {
        byteswritten = (ULONG)write(pfh->fh, pBuffer, (size_t)cbWrite);
        if (byteswritten == -1) {
            printf("errno = %d\n", errno);
            *pcbActual = 0;
            switch (errno) {
            case EFAULT:
            case EPERM:
                RESTOREENV_RETURN(ERROR_ACCESS_DENIED);
            case EIO:
            case ENOSPC:
                RESTOREENV_RETURN(ERROR_WRITE_FAULT);
            case EPIPE:
                RESTOREENV_RETURN(ERROR_PIPE_NOT_CONNECTED);
            case EINVAL:
                RESTOREENV_RETURN(ERROR_WRITE_PROTECT);
            default:
                RESTOREENV_RETURN(ERROR_INVALID_HANDLE);
            }
        }
        pBuffer += byteswritten;
        cbWrite -= byteswritten;
        *pcbActual += byteswritten;
    }

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosDelete                                                                  */
/*    Notes:                                                                  */
/*       - none                                                               */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosDelete(PCSZ pszFile)
{
    SAVEENV;
    int rc;
    PSZ pszNewName;

    /* convert the file name to Linux format */
    pszNewName = alloca(strlen(pszFile) + 1);
    if (pszNewName == NULL) {
        RESTOREENV_RETURN(ERROR_NOT_ENOUGH_MEMORY);
    }
    strcpy(pszNewName, pszFile);
    DosNameConversion(pszNewName, "\\", "/", FALSE);

    /* delete the file */
    rc = unlink(pszNewName);
    if (rc) {
        switch (errno) {
        case EISDIR:
        case EFAULT:
        case EACCES:
        case EPERM:
        case EROFS:
            RESTOREENV_RETURN(ERROR_ACCESS_DENIED);
        case ENAMETOOLONG:
        case ELOOP:
            RESTOREENV_RETURN(ERROR_FILENAME_EXCED_RANGE);
        default:
            RESTOREENV_RETURN(ERROR_FILE_NOT_FOUND);
        }
    }

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosForceDelete                                                             */
/*    Notes:                                                                  */
/*       - none                                                               */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosForceDelete(PCSZ pszFile)
{
    return DosDelete(pszFile);
}


/*----------------------------------------------------------------------------*/
/* DosDupHandle                                                               */
/*    Notes:                                                                  */
/*       - none                                                               */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosDupHandle(HFILE hFile, PHFILE pHfile)
{
    SAVEENV;
    PFILEHANDLESTRUCT pfh;

    /* allocate memory for the file handles */
    pfh = malloc(sizeof(FILEHANDLESTRUCT));
    if (pfh == NULL) {
        RESTOREENV_RETURN(ERROR_NOT_ENOUGH_MEMORY);
    }
    strcpy(pfh->id, ((PFILEHANDLESTRUCT)hFile)->id);

    /* dup the handle */
    pfh->fh = dup(((PFILEHANDLESTRUCT)hFile)->fh);
    if (pfh->fh == -1) {
        switch (errno) {
        case EBADF:
            RESTOREENV_RETURN(ERROR_INVALID_HANDLE);
        case EMFILE:
            RESTOREENV_RETURN(ERROR_TOO_MANY_OPEN_FILES);
        default:
            RESTOREENV_RETURN(ERROR_INVALID_TARGET_HANDLE);
        }
    }

    *pHfile = (HFILE)pfh;
    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosQueryFHState                                                            */
/*    Notes:                                                                  */
/*       - none                                                               */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosQueryFHState(HFILE hFile, PULONG pMode)
{
    SAVEENV;
    PFILEHANDLESTRUCT pfh;

    pfh = (PFILEHANDLESTRUCT)hFile;
    *pMode = pfh->openmode;

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosSetFHState                                                              */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosSetFHState(HFILE hFile, ULONG mode)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosQueryHType                                                              */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosQueryHType(HFILE hFile, PULONG pType, PULONG pAttr)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosFindFirst                                                               */
/*    Notes:                                                                  */
/*       - flAttribute is ignored                                             */
/*       - ulInfoLevel must be FIL_STANDARD                                   */
/*       - added the return code ERROR_NOT_ENOUGH_MEMORY                      */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosFindFirst(PCSZ pszFileSpec, PHDIR phdir, ULONG flAttribute,
                             PVOID pfindbuf, ULONG cbBuf, PULONG pcFileNames,
                             ULONG ulInfoLevel)
{
    SAVEENV;
    PSZ pszNewSpec;
    PLINUXHDIRSTRUCT os2hdir;
    glob_t * myhdir;
    int rc;
    int i;
    struct stat statbuf;
    struct tm brokentime;

    /* check args */
    if (ulInfoLevel != FIL_STANDARD) {
        RESTOREENV_RETURN(ERROR_INVALID_PARAMETER);
    }
    if (cbBuf == 0) {
        RESTOREENV_RETURN(ERROR_BUFFER_OVERFLOW);
    }
    if (*pcFileNames * sizeof(FILEFINDBUF3) < cbBuf) {
        RESTOREENV_RETURN(ERROR_BUFFER_OVERFLOW);
    }

    /* convert the file spec to Linux format */
    pszNewSpec = alloca(strlen(pszFileSpec) + 1);
    if (pszNewSpec == NULL) {
        RESTOREENV_RETURN(ERROR_NOT_ENOUGH_MEMORY);
    }
    strcpy(pszNewSpec, pszFileSpec);
    DosNameConversion(pszNewSpec, "\\", "/", FALSE);

    /* set HDIR (glob_t) */
    if (*phdir == HDIR_SYSTEM) {
        os2hdir = &DosLinuxStruct.syshdir;
    }
    else if (*phdir == HDIR_CREATE) {
        os2hdir = malloc(sizeof(LINUXHDIRSTRUCT));
    }
    else {
        RESTOREENV_RETURN(ERROR_INVALID_PARAMETER);
    }
    os2hdir->ctr = 0;
    myhdir = &os2hdir->hdir;

    /* perform the search */
    myhdir->gl_offs = 0;
    rc = glob(pszNewSpec, 0, NULL, myhdir);
    if (rc) {
        if (*phdir == HDIR_CREATE) {
            free(myhdir);
        }
        switch (rc) {
        case GLOB_NOSPACE:
            RESTOREENV_RETURN(ERROR_NOT_ENOUGH_MEMORY);
        case GLOB_ABORTED:
            RESTOREENV_RETURN(ERROR_NO_MORE_SEARCH_HANDLES);
        default:
            RESTOREENV_RETURN(ERROR_FILE_NOT_FOUND);
        }
    }

    /* build FILEFINDBUF3 entries */
    for (i = 0; i < *pcFileNames; i++, pfindbuf += sizeof(FILEFINDBUF3)) {
        if (i == myhdir->gl_pathc) {
            break;
        }
        if (i == *pcFileNames - 1) {
            ((PFILEFINDBUF3)pfindbuf)->oNextEntryOffset = 0;
        }
        else {
            ((PFILEFINDBUF3)pfindbuf)->oNextEntryOffset = sizeof(FILEFINDBUF3);
        }
        ((PFILEFINDBUF3)pfindbuf)->cchName = strlen(myhdir->gl_pathv[i]);
        strcpy(((PFILEFINDBUF3)pfindbuf)->achName, myhdir->gl_pathv[i]);
        /* stat the file to get it's  info */
        rc = stat(myhdir->gl_pathv[i], &statbuf);
        localtime_r(&statbuf.st_ctime, &brokentime);
        ((PFILEFINDBUF3)pfindbuf)->fdateCreation.day =
            (UINT)brokentime.tm_mday;
        ((PFILEFINDBUF3)pfindbuf)->fdateCreation.month =
            (UINT)brokentime.tm_mon + 1;
        ((PFILEFINDBUF3)pfindbuf)->fdateCreation.year =
            (UINT)brokentime.tm_year - 80;
        ((PFILEFINDBUF3)pfindbuf)->ftimeCreation.twosecs =
            (USHORT)brokentime.tm_sec / 2;
        ((PFILEFINDBUF3)pfindbuf)->ftimeCreation.minutes =
            (USHORT)brokentime.tm_min;
        ((PFILEFINDBUF3)pfindbuf)->ftimeCreation.hours =
            (USHORT)brokentime.tm_hour;
        localtime_r(&statbuf.st_atime, &brokentime);
        ((PFILEFINDBUF3)pfindbuf)->fdateLastAccess.day =
            (UINT)brokentime.tm_mday;
        ((PFILEFINDBUF3)pfindbuf)->fdateLastAccess.month =
            (UINT)brokentime.tm_mon + 1;
        ((PFILEFINDBUF3)pfindbuf)->fdateLastAccess.year =
            (UINT)brokentime.tm_year - 80;
        ((PFILEFINDBUF3)pfindbuf)->ftimeLastAccess.twosecs =
            (USHORT)brokentime.tm_sec / 2;
        ((PFILEFINDBUF3)pfindbuf)->ftimeLastAccess.minutes =
            (USHORT)brokentime.tm_min;
        ((PFILEFINDBUF3)pfindbuf)->ftimeLastAccess.hours =
            (USHORT)brokentime.tm_hour;
        localtime_r(&statbuf.st_mtime, &brokentime);
        ((PFILEFINDBUF3)pfindbuf)->fdateLastWrite.day =
            (UINT)brokentime.tm_mday;
        ((PFILEFINDBUF3)pfindbuf)->fdateLastWrite.month =
            (UINT)brokentime.tm_mon + 1;
        ((PFILEFINDBUF3)pfindbuf)->fdateLastWrite.year =
            (UINT)brokentime.tm_year - 80;
        ((PFILEFINDBUF3)pfindbuf)->ftimeLastWrite.twosecs =
            (USHORT)brokentime.tm_sec / 2;
        ((PFILEFINDBUF3)pfindbuf)->ftimeLastWrite.minutes =
            (USHORT)brokentime.tm_min;
        ((PFILEFINDBUF3)pfindbuf)->ftimeLastWrite.hours =
            (USHORT)brokentime.tm_hour;
        ((PFILEFINDBUF3)pfindbuf)->cbFile = (ULONG)statbuf.st_size;
        ((PFILEFINDBUF3)pfindbuf)->cbFileAlloc =
            (ULONG)(statbuf.st_blksize * statbuf.st_blocks);
        if (S_ISDIR(statbuf.st_mode)) {
            ((PFILEFINDBUF3)pfindbuf)->attrFile = FILE_DIRECTORY;
        }
        else {
            ((PFILEFINDBUF3)pfindbuf)->attrFile = (ULONG)0;
        }
    }
    os2hdir->ctr = i;

    *pcFileNames = (ULONG)i;
    if (*phdir == HDIR_CREATE) {
        *phdir = (HDIR)os2hdir;
    }
    if (i == myhdir->gl_pathc) {
        RESTOREENV_RETURN(ERROR_NO_MORE_FILES);
    }
    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosFindNext                                                                */
/*    Notes:                                                                  */
/*       - none                                                               */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosFindNext(HDIR hDir, PVOID pfindbuf, ULONG cbfindbuf,
                            PULONG pcFilenames)
{
    SAVEENV;
    PLINUXHDIRSTRUCT os2hdir;
    glob_t * myhdir;
    int i, j;
    int rc;
    struct stat statbuf;
    struct tm brokentime;

    /* check args */
    if (cbfindbuf == 0) {
        RESTOREENV_RETURN(ERROR_BUFFER_OVERFLOW);
    }
    if (*pcFilenames * sizeof(FILEFINDBUF3) < cbfindbuf) {
        RESTOREENV_RETURN(ERROR_BUFFER_OVERFLOW);
    }

    /* set HDIR (glob_t) */
    if (hDir == HDIR_SYSTEM) {
        os2hdir = &DosLinuxStruct.syshdir;
    }
    else {
        os2hdir = (PLINUXHDIRSTRUCT)hDir;
    }
    myhdir = &os2hdir->hdir;

    /* build FILEFINDBUF3 entries */
    for (i = os2hdir->ctr, j = 0; j < *pcFilenames;
     i++, j++, pfindbuf += sizeof(FILEFINDBUF3)) {
        if (i == myhdir->gl_pathc) {
            break;
        }
        if (j == *pcFilenames - 1) {
            ((PFILEFINDBUF3)pfindbuf)->oNextEntryOffset = 0;
        }
        else {
            ((PFILEFINDBUF3)pfindbuf)->oNextEntryOffset = sizeof(FILEFINDBUF3);
        }
        ((PFILEFINDBUF3)pfindbuf)->cchName = strlen(myhdir->gl_pathv[i]);
        strcpy(((PFILEFINDBUF3)pfindbuf)->achName, myhdir->gl_pathv[i]);
        /* stat the file to get it's  info */
        rc = stat(myhdir->gl_pathv[i], &statbuf);
        localtime_r(&statbuf.st_ctime, &brokentime);
        ((PFILEFINDBUF3)pfindbuf)->fdateCreation.day =
            (UINT)brokentime.tm_mday;
        ((PFILEFINDBUF3)pfindbuf)->fdateCreation.month =
            (UINT)brokentime.tm_mon + 1;
        ((PFILEFINDBUF3)pfindbuf)->fdateCreation.year =
            (UINT)brokentime.tm_year - 80;
        ((PFILEFINDBUF3)pfindbuf)->ftimeCreation.twosecs =
            (USHORT)brokentime.tm_sec / 2;
        ((PFILEFINDBUF3)pfindbuf)->ftimeCreation.minutes =
            (USHORT)brokentime.tm_min;
        ((PFILEFINDBUF3)pfindbuf)->ftimeCreation.hours =
            (USHORT)brokentime.tm_hour;
        localtime_r(&statbuf.st_atime, &brokentime);
        ((PFILEFINDBUF3)pfindbuf)->fdateLastAccess.day =
            (UINT)brokentime.tm_mday;
        ((PFILEFINDBUF3)pfindbuf)->fdateLastAccess.month =
            (UINT)brokentime.tm_mon + 1;
        ((PFILEFINDBUF3)pfindbuf)->fdateLastAccess.year =
            (UINT)brokentime.tm_year - 80;
        ((PFILEFINDBUF3)pfindbuf)->ftimeLastAccess.twosecs =
            (USHORT)brokentime.tm_sec / 2;
        ((PFILEFINDBUF3)pfindbuf)->ftimeLastAccess.minutes =
            (USHORT)brokentime.tm_min;
        ((PFILEFINDBUF3)pfindbuf)->ftimeLastAccess.hours =
            (USHORT)brokentime.tm_hour;
        localtime_r(&statbuf.st_mtime, &brokentime);
        ((PFILEFINDBUF3)pfindbuf)->fdateLastWrite.day =
            (UINT)brokentime.tm_mday;
        ((PFILEFINDBUF3)pfindbuf)->fdateLastWrite.month =
            (UINT)brokentime.tm_mon + 1;
        ((PFILEFINDBUF3)pfindbuf)->fdateLastWrite.year =
            (UINT)brokentime.tm_year - 80;
        ((PFILEFINDBUF3)pfindbuf)->ftimeLastWrite.twosecs =
            (USHORT)brokentime.tm_sec / 2;
        ((PFILEFINDBUF3)pfindbuf)->ftimeLastWrite.minutes =
            (USHORT)brokentime.tm_min;
        ((PFILEFINDBUF3)pfindbuf)->ftimeLastWrite.hours =
            (USHORT)brokentime.tm_hour;
        ((PFILEFINDBUF3)pfindbuf)->cbFile = (ULONG)statbuf.st_size;
        ((PFILEFINDBUF3)pfindbuf)->cbFileAlloc =
            (ULONG)(statbuf.st_blksize * statbuf.st_blocks);
        if (S_ISDIR(statbuf.st_mode)) {
            ((PFILEFINDBUF3)pfindbuf)->attrFile = FILE_DIRECTORY;
        }
        else {
            ((PFILEFINDBUF3)pfindbuf)->attrFile = (ULONG)0;
        }
    }
    os2hdir->ctr = i;

    *pcFilenames = (ULONG)j;
    if (i == myhdir->gl_pathc) {
        RESTOREENV_RETURN(ERROR_NO_MORE_FILES);
    }
    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosFindClose                                                               */
/*    Notes:                                                                  */
/*       - none                                                               */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosFindClose(HDIR hDir)
{
    SAVEENV;
    PLINUXHDIRSTRUCT os2hdir;
    glob_t * myhdir;

    /* set HDIR (glob_t) */
    if (hDir == HDIR_SYSTEM) {
        os2hdir = &DosLinuxStruct.syshdir;
    }
    else {
        os2hdir = (PLINUXHDIRSTRUCT)hDir;
    }
    myhdir = &os2hdir->hdir;

    /* free the memory */
    globfree(myhdir);
    if (os2hdir != &DosLinuxStruct.syshdir) {
        free(os2hdir);
    }

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosFSAttach                                                                */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosFSAttach(PCSZ pszDevice, PCSZ pszFilesystem,
                            __const__ VOID *pData, ULONG cbData, ULONG flag)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosQueryFSAttach                                                           */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosQueryFSAttach(PCSZ pszDeviceName, ULONG ulOrdinal,
                                 ULONG ulFSAInfoLevel, PFSQBUFFER2 pfsqb,
                                 PULONG pcbBuffLength)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosFSCtl                                                                   */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosFSCtl(PVOID pData, ULONG cbData, PULONG pcbData,
                         PVOID pParms, ULONG cbParms, PULONG pcbParms,
                         ULONG function, PCSZ pszRoute, HFILE hFile,
                         ULONG method)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosSetFileSize                                                             */
/*    Notes:                                                                  */
/*       - none                                                               */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosSetFileSize(HFILE hFile, ULONG cbSize)
{
    SAVEENV;
    PFILEHANDLESTRUCT pfh;
    int rc;

    /* set the file size */
    pfh = (PFILEHANDLESTRUCT)hFile;
    rc = ftruncate(pfh->fh, cbSize);
    if (rc == -1) {
        switch (errno) {
        case EBADF:
            RESTOREENV_RETURN(ERROR_INVALID_HANDLE);
        case EINVAL:
            RESTOREENV_RETURN(ERROR_ACCESS_DENIED);
        default:
            RESTOREENV_RETURN(ERROR_INVALID_PARAMETER);
        }
    }

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosResetBuffer                                                             */
/*    Notes:                                                                  */
/*       - none                                                               */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosResetBuffer(HFILE hFile)
{
    SAVEENV;
    PFILEHANDLESTRUCT pfh;
    int rc;

    /* set the file size */
    pfh = (PFILEHANDLESTRUCT)hFile;
    if (strcmp(pfh->id, "FIL") == 0) {
        rc = fsync(pfh->fh);
        if (rc == -1) {
            switch (errno) {
            case EBADF:
                RESTOREENV_RETURN(ERROR_INVALID_HANDLE);
            case EINVAL:
            case EIO:
                RESTOREENV_RETURN(ERROR_ACCESS_DENIED);
            default:
                RESTOREENV_RETURN(ERROR_INVALID_PARAMETER);
            }
        }
    }
    /* there is no need to flush a pipe */

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosSetFilePtr                                                              */
/*    Notes:                                                                  */
/*       - none                                                               */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosSetFilePtr(HFILE hFile, LONG ib, ULONG method,
                              PULONG ibActual)
{
    SAVEENV;
    PFILEHANDLESTRUCT pfh;
    int whence;

    /* set the Linux whence */
    switch (method) {
    case FILE_BEGIN:
        whence = SEEK_SET;
        break;
    case FILE_CURRENT:
        whence = SEEK_CUR;
        break;
    case FILE_END:
        whence = SEEK_END;
        break;
    default:
        RESTOREENV_RETURN(ERROR_INVALID_PARAMETER);
    }

    /* move file pointer */
    pfh = (PFILEHANDLESTRUCT)hFile;
    *ibActual = lseek(pfh->fh, ib, whence);
    if (*ibActual == -1) {
        *ibActual = 0;
        switch (errno) {
        case EBADF:
            RESTOREENV_RETURN(ERROR_INVALID_HANDLE);
        case ESPIPE:
            RESTOREENV_RETURN(ERROR_SEEK_ON_DEVICE);
        default:
            RESTOREENV_RETURN(ERROR_INVALID_FUNCTION);
        }
    }

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosMove                                                                    */
/*    Notes:                                                                  */
/*       - none                                                               */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosMove(PCSZ pszOld, PCSZ pszNew)
{
    SAVEENV;
    PSZ pszOldName;
    PSZ pszNewName;
    int rc;

    /* convert the file names */
    pszOldName = alloca(strlen(pszOld) + 1);
    if (pszOldName == NULL) {
        RESTOREENV_RETURN(ERROR_SHARING_BUFFER_EXCEEDED);
    }
    strcpy(pszOldName, pszOld);
    DosNameConversion(pszOldName, "\\", "/", FALSE);
    pszNewName = alloca(strlen(pszNew) + 1);
    if (pszNewName == NULL) {
        RESTOREENV_RETURN(ERROR_SHARING_BUFFER_EXCEEDED);
    }
    strcpy(pszNewName, pszNew);
    DosNameConversion(pszNewName, "\\", "/", FALSE);

    /* perform the move */
    rc = rename(pszOldName, pszNewName);
    if (rc == -1) {
        switch (errno) {
        case EXDEV:
            RESTOREENV_RETURN(ERROR_NOT_SAME_DEVICE);
        case EEXIST:
        case ENOTEMPTY:
        case EBUSY:
            RESTOREENV_RETURN(ERROR_SHARING_VIOLATION);
        case EINVAL:
            RESTOREENV_RETURN(ERROR_CIRCULARITY_REQUESTED);
        case EMLINK:
        case ENOSPC:
        case ENOMEM:
            RESTOREENV_RETURN(ERROR_SHARING_BUFFER_EXCEEDED);
        case EFAULT:
        case EACCES:
        case EPERM:
        case EROFS:
            RESTOREENV_RETURN(ERROR_ACCESS_DENIED);
        case ENAMETOOLONG:
        case ELOOP:
            RESTOREENV_RETURN(ERROR_FILENAME_EXCED_RANGE);
        default:
            RESTOREENV_RETURN(ERROR_FILE_NOT_FOUND);
        }
    }

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosCopy                                                                    */
/*    Notes:                                                                  */
/*       - added return code ERROR_NOT_ENOUGH_MEMORY                          */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosCopy(PCSZ pszOld, PCSZ pszNew, ULONG option)
{
    SAVEENV;
    PSZ pszOldName, pszNewName;
    int targetflags = O_WRONLY;
    int srcFd, trgFd;
    int mode;
    struct stat statbuf;
    int bytesread;
    char rwbuf[32767];
    struct utimbuf UTimBuf;
    int rc;

    /* convert the file names */
    pszOldName = alloca(strlen(pszOld) + 1);
    if (pszOldName == NULL) {
        RESTOREENV_RETURN(ERROR_NOT_ENOUGH_MEMORY);
    }
    strcpy(pszOldName, pszOld);
    DosNameConversion(pszNewName, "\\", "/", FALSE);
    pszNewName = alloca(strlen(pszNew) + 1);
    if (pszNewName == NULL) {
        RESTOREENV_RETURN(ERROR_NOT_ENOUGH_MEMORY);
    }
    strcpy(pszNewName, pszNew);
    DosNameConversion(pszNewName, "\\", "/", FALSE);

    /* open the source file */
    srcFd = open(pszOldName, O_RDONLY);
    if (srcFd == -1) {
        switch (errno) {
        case ENAMETOOLONG:
            RESTOREENV_RETURN(ERROR_FILENAME_EXCED_RANGE);
        case ENOENT:
            RESTOREENV_RETURN(ERROR_FILE_NOT_FOUND);
        case ENOTDIR:
            RESTOREENV_RETURN(ERROR_PATH_NOT_FOUND);
        case ENOMEM:
        case EMFILE:
        case ENFILE:
            RESTOREENV_RETURN(ERROR_NOT_ENOUGH_MEMORY);
        default:
            RESTOREENV_RETURN(ERROR_ACCESS_DENIED);
        }
    }

    /* open the target file */
    stat(pszNewName, &statbuf);
    mode = statbuf.st_mode & (S_IRWXU | S_IRWXG | S_IROTH);
    if (option & DCPY_APPEND) {
        targetflags |= O_APPEND;
    }
    else {
        targetflags |= O_CREAT;
    }
    if (option & DCPY_EXISTING) {
        targetflags |= O_EXCL;
    }
    trgFd = open(pszNewName, targetflags, mode);
    if (srcFd == -1) {
        switch (errno) {
        case EISDIR:
            close(srcFd);
            RESTOREENV_RETURN(ERROR_DIRECTORY);
        case ENAMETOOLONG:
            close(srcFd);
            RESTOREENV_RETURN(ERROR_FILENAME_EXCED_RANGE);
        case ENOENT:
            close(srcFd);
            RESTOREENV_RETURN(ERROR_FILE_NOT_FOUND);
        case ENOTDIR:
            close(srcFd);
            RESTOREENV_RETURN(ERROR_PATH_NOT_FOUND);
        case ENOMEM:
        case EMFILE:
        case ENFILE:
            close(srcFd);
            RESTOREENV_RETURN(ERROR_NOT_ENOUGH_MEMORY);
        default:
            close(srcFd);
            RESTOREENV_RETURN(ERROR_ACCESS_DENIED);
        }
    }

    /* copy the file */
    lseek(trgFd, 0, SEEK_END);
    bytesread = read(srcFd, rwbuf, sizeof(rwbuf));
    while (bytesread > 0) {
        rc = write(trgFd, rwbuf, bytesread);
        if (rc) {
            switch (errno) {
            case ENOSPC:
                close(srcFd);
                close(trgFd);
                RESTOREENV_RETURN(ERROR_DISK_FULL);
            default:
                close(srcFd);
                close(trgFd);
                RESTOREENV_RETURN(ERROR_ACCESS_DENIED);
            }
        }
        bytesread = read(srcFd, rwbuf, sizeof(rwbuf));
    }
    close(srcFd);
    close(trgFd);

    /* modify the target file access and modification timestamps */
    UTimBuf.actime = statbuf.st_atime;
    UTimBuf.modtime = statbuf.st_mtime;
    utime(pszNewName, &UTimBuf);

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosEditName                                                                */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosEditName(ULONG metalevel, PCSZ pszSource, PCSZ pszEdit,
                            PBYTE pszTarget, ULONG cbTarget)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosCreateDir                                                               */
/*    Notes:                                                                  */
/*       - peaop2 is ignored                                                  */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosCreateDir(PCSZ pszDirName, PEAOP2 peaop2)
{
    SAVEENV;
    PSZ pszNewName;
    int rc;

    /* check args */
    if (peaop2 != NULL) {
        RESTOREENV_RETURN(ERROR_INVALID_EA_NAME);
    }

    /* convert the file names */
    pszNewName = alloca(strlen(pszDirName) + 1);
    if (pszNewName == NULL) {
        RESTOREENV_RETURN(ERROR_SHARING_BUFFER_EXCEEDED);
    }
    strcpy(pszNewName, pszDirName);
    DosNameConversion(pszNewName, "\\", "/", FALSE);

    /* make the directory */
    rc = mkdir(pszNewName, S_IRWXG | S_IRWXU | S_IROTH);
    if (rc == -1) {
        switch (errno) {
        case EEXIST:
            RESTOREENV_RETURN(ERROR_INVALID_PARAMETER);
        case EFAULT:
        case EACCES:
        case ENOSPC:
        case EROFS:
            RESTOREENV_RETURN(ERROR_ACCESS_DENIED);
        case ENAMETOOLONG:
        case ELOOP:
            RESTOREENV_RETURN(ERROR_FILENAME_EXCED_RANGE);
        default:
            RESTOREENV_RETURN(ERROR_PATH_NOT_FOUND);
        }
    }

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosDeleteDir                                                               */
/*    Notes:                                                                  */
/*       - none                                                               */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosDeleteDir(PCSZ pszDir)
{
    SAVEENV;
    PSZ pszNewName;
    int rc;

    /* convert the file names */
    pszNewName = alloca(strlen(pszDir) + 1);
    if (pszNewName == NULL) {
        RESTOREENV_RETURN(ERROR_NOT_ENOUGH_MEMORY);
    }
    strcpy(pszNewName, pszDir);
    DosNameConversion(pszNewName, "\\", "/", FALSE);

    /* remove directory */
   rc = rmdir(pszNewName);
   if (rc == -1) {
       switch (errno) {
       case ENAMETOOLONG:
           RESTOREENV_RETURN(ERROR_FILENAME_EXCED_RANGE);
       case ENOTDIR:
           RESTOREENV_RETURN(ERROR_PATH_NOT_FOUND);
       case ENOMEM:
           RESTOREENV_RETURN(ERROR_NOT_ENOUGH_MEMORY);
       default:
           RESTOREENV_RETURN(ERROR_ACCESS_DENIED);
       }
   }

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosSetDefaultDisk                                                          */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosSetDefaultDisk(ULONG disknum)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosQueryCurrentDisk                                                        */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosQueryCurrentDisk(PULONG pdisknum, PULONG plogical)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosSetCurrentDir                                                           */
/*    Notes:                                                                  */
/*       - none                                                               */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosSetCurrentDir(PCSZ pszDir)
{
    SAVEENV;
    PSZ pszNewName;
    int rc;

    /* convert the file names */
    pszNewName = alloca(strlen(pszDir) + 1);
    if (pszNewName == NULL) {
        RESTOREENV_RETURN(ERROR_SHARING_BUFFER_EXCEEDED);
    }
    strcpy(pszNewName, pszDir);
    DosNameConversion(pszNewName, "\\", "/", FALSE);

    /* make the directory */
    rc = chdir(pszNewName);
    if (rc == -1) {
        switch (errno) {
        case EFAULT:
        case EACCES:
            RESTOREENV_RETURN(ERROR_ACCESS_DENIED);
        case ENAMETOOLONG:
        case ELOOP:
            RESTOREENV_RETURN(ERROR_FILENAME_EXCED_RANGE);
        default:
            RESTOREENV_RETURN(ERROR_PATH_NOT_FOUND);
        }
    }

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosQueryCurrentDir                                                         */
/*    Notes:                                                                  */
/*       - disknum is ignored                                                 */
/*       - some new OS/2 error codes have been added                          */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosQueryCurrentDir(ULONG disknum, PBYTE pBuf, PULONG pcbBuf)
{
    SAVEENV;

    if (getcwd(pBuf, *pcbBuf) == NULL) {
        switch (errno) {
        case EINVAL:
        case ERANGE:
            *pcbBuf = (ULONG)pathconf(".", _PC_PATH_MAX);
            RESTOREENV_RETURN(ERROR_BUFFER_OVERFLOW);
        default:
            RESTOREENV_RETURN(ERROR_ACCESS_DENIED);
        }
    }

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosQueryFSInfo                                                             */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosQueryFSInfo(ULONG disknum, ULONG infolevel, PVOID pBuf,
                               ULONG cbBuf)
{
    /* might use statfs() here? */
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosSetFSInfo                                                               */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosSetFSInfo(ULONG disknum, ULONG infolevel, PVOID pBuf,
                             ULONG cbBuf)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosQueryVerify                                                             */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosQueryVerify(PBOOL32 pBool)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosSetVerify                                                               */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosSetVerify(BOOL32 verify)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosSetMaxFH                                                                */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosSetMaxFH(ULONG cFH)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosSetRelMaxFH                                                             */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosSetRelMaxFH(PLONG pcbReqCount, PULONG pcbCurMaxFH)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosQueryFileInfo                                                           */
/*    Notes:                                                                  */
/*       - ulInfoLevel must be FIL_STANDARD                                   */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosQueryFileInfo(HFILE hf, ULONG ulInfoLevel, PVOID pInfo,
                                 ULONG cbInfoBuf)
{
    SAVEENV;
    PFILEHANDLESTRUCT pfh;
    struct stat buf;
    struct tm brokentime;
    int rc;

    /* check args */
    if (ulInfoLevel != FIL_STANDARD) {
        RESTOREENV_RETURN(ERROR_INVALID_LEVEL);
    }
    if (cbInfoBuf < sizeof(FILESTATUS3)) {
        RESTOREENV_RETURN(ERROR_BUFFER_OVERFLOW);
    }

    /* get the file status */
    pfh = (PFILEHANDLESTRUCT)hf;
    rc = fstat(pfh->fh, &buf);
    if (rc == -1) {
        switch (errno) {
        case EBADF:
            RESTOREENV_RETURN(ERROR_INVALID_HANDLE);
        default:
            RESTOREENV_RETURN(ERROR_ACCESS_DENIED);
        }
    }
    localtime_r(&buf.st_ctime, &brokentime);
    ((PFILESTATUS3)pInfo)->fdateCreation.day = (UINT)brokentime.tm_mday;
    ((PFILESTATUS3)pInfo)->fdateCreation.month = (UINT)brokentime.tm_mon + 1;
    ((PFILESTATUS3)pInfo)->fdateCreation.year = (UINT)brokentime.tm_year - 80;
    ((PFILESTATUS3)pInfo)->ftimeCreation.twosecs = (USHORT)brokentime.tm_sec / 2;
    ((PFILESTATUS3)pInfo)->ftimeCreation.minutes = (USHORT)brokentime.tm_min;
    ((PFILESTATUS3)pInfo)->ftimeCreation.hours = (USHORT)brokentime.tm_hour;
    localtime_r(&buf.st_atime, &brokentime);
    ((PFILESTATUS3)pInfo)->fdateLastAccess.day = (UINT)brokentime.tm_mday;
    ((PFILESTATUS3)pInfo)->fdateLastAccess.month = (UINT)brokentime.tm_mon + 1;
    ((PFILESTATUS3)pInfo)->fdateLastAccess.year = (UINT)brokentime.tm_year - 80;
    ((PFILESTATUS3)pInfo)->ftimeLastAccess.twosecs = (USHORT)brokentime.tm_sec / 2;
    ((PFILESTATUS3)pInfo)->ftimeLastAccess.minutes = (USHORT)brokentime.tm_min;
    ((PFILESTATUS3)pInfo)->ftimeLastAccess.hours = (USHORT)brokentime.tm_hour;
    localtime_r(&buf.st_mtime, &brokentime);
    ((PFILESTATUS3)pInfo)->fdateLastWrite.day = (UINT)brokentime.tm_mday;
    ((PFILESTATUS3)pInfo)->fdateLastWrite.month = (UINT)brokentime.tm_mon + 1;
    ((PFILESTATUS3)pInfo)->fdateLastWrite.year = (UINT)brokentime.tm_year - 80;
    ((PFILESTATUS3)pInfo)->ftimeLastWrite.twosecs = (USHORT)brokentime.tm_sec / 2;
    ((PFILESTATUS3)pInfo)->ftimeLastWrite.minutes = (USHORT)brokentime.tm_min;
    ((PFILESTATUS3)pInfo)->ftimeLastWrite.hours = (USHORT)brokentime.tm_hour;
    ((PFILESTATUS3)pInfo)->cbFile = (ULONG)buf.st_size;
    ((PFILESTATUS3)pInfo)->cbFileAlloc = (ULONG)(buf.st_blksize * buf.st_blocks);
    if (S_ISDIR(buf.st_mode)) {
        ((PFILESTATUS3)pInfo)->attrFile = FILE_DIRECTORY;
    }
    else {
        ((PFILESTATUS3)pInfo)->attrFile = (ULONG)0;
    }

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosSetFileInfo                                                             */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosSetFileInfo(HFILE hf, ULONG ulInfoLevel, PVOID pInfoBuf,
                               ULONG cbInfoBuf)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosQueryPathInfo                                                           */
/*    Notes:                                                                  */
/*       - ulInfoLevel must be FIL_STANDARD                                   */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosQueryPathInfo(PCSZ pszPathName, ULONG ulInfoLevel,
                                 PVOID pInfo, ULONG cbInfoBuf)
{
    SAVEENV;
    PSZ pszNewName;
    struct stat buf;
    struct tm brokentime;
    int rc;

    /* check args */
    if (cbInfoBuf < sizeof(FILESTATUS3)) {
        RESTOREENV_RETURN(ERROR_BUFFER_OVERFLOW);
    }

    if (ulInfoLevel == FIL_STANDARD) {
        /* convert the file names */
        pszNewName = alloca(strlen(pszPathName) + 1);
        if (pszNewName == NULL) {
            RESTOREENV_RETURN(ERROR_SHARING_BUFFER_EXCEEDED);
        }
        strcpy(pszNewName, pszPathName);
        DosNameConversion(pszNewName, "\\", "/", FALSE);
        /* get the file status */
        rc = stat(pszNewName, &buf);
        if (rc == -1) {
            switch (errno) {
            case EBADF:
                RESTOREENV_RETURN(ERROR_INVALID_HANDLE);
            default:
                RESTOREENV_RETURN(ERROR_ACCESS_DENIED);
            }
        }
        localtime_r(&buf.st_ctime, &brokentime);
        ((PFILESTATUS3)pInfo)->fdateCreation.day = (UINT)brokentime.tm_mday;
        ((PFILESTATUS3)pInfo)->fdateCreation.month =
            (UINT)brokentime.tm_mon + 1;
        ((PFILESTATUS3)pInfo)->fdateCreation.year =
            (UINT)brokentime.tm_year + 1900;
        ((PFILESTATUS3)pInfo)->ftimeCreation.twosecs =
            (USHORT)brokentime.tm_sec / 2;
        ((PFILESTATUS3)pInfo)->ftimeCreation.minutes =
            (USHORT)brokentime.tm_min;
        ((PFILESTATUS3)pInfo)->ftimeCreation.hours =
            (USHORT)brokentime.tm_hour;
        localtime_r(&buf.st_atime, &brokentime);
        ((PFILESTATUS3)pInfo)->fdateLastAccess.day = (UINT)brokentime.tm_mday;
        ((PFILESTATUS3)pInfo)->fdateLastAccess.month =
            (UINT)brokentime.tm_mon + 1;
        ((PFILESTATUS3)pInfo)->fdateLastAccess.year =
            (UINT)brokentime.tm_year + 1900;
        ((PFILESTATUS3)pInfo)->ftimeLastAccess.twosecs =
            (USHORT)brokentime.tm_sec / 2;
        ((PFILESTATUS3)pInfo)->ftimeLastAccess.minutes =
            (USHORT)brokentime.tm_min;
        ((PFILESTATUS3)pInfo)->ftimeLastAccess.hours =
            (USHORT)brokentime.tm_hour;
        localtime_r(&buf.st_mtime, &brokentime);
        ((PFILESTATUS3)pInfo)->fdateLastWrite.day = (UINT)brokentime.tm_mday;
        ((PFILESTATUS3)pInfo)->fdateLastWrite.month =
            (UINT)brokentime.tm_mon + 1;
        ((PFILESTATUS3)pInfo)->fdateLastWrite.year =
            (UINT)brokentime.tm_year + 1900;
        ((PFILESTATUS3)pInfo)->ftimeLastWrite.twosecs =
            (USHORT)brokentime.tm_sec / 2;
        ((PFILESTATUS3)pInfo)->ftimeLastWrite.minutes =
            (USHORT)brokentime.tm_min;
        ((PFILESTATUS3)pInfo)->ftimeLastWrite.hours =
            (USHORT)brokentime.tm_hour;
        ((PFILESTATUS3)pInfo)->cbFile = (ULONG)buf.st_size;
        ((PFILESTATUS3)pInfo)->cbFileAlloc =
            (ULONG)(buf.st_blksize * buf.st_blocks);
        if (S_ISDIR(buf.st_mode)) {
            ((PFILESTATUS3)pInfo)->attrFile = FILE_DIRECTORY;
        }
        else {
            ((PFILESTATUS3)pInfo)->attrFile = (ULONG)0;
        }
    }
    else {
        RESTOREENV_RETURN(ERROR_INVALID_LEVEL);
    }

    RESTOREENV_RETURN(NO_ERROR);
}


/*----------------------------------------------------------------------------*/
/* DosSetPathInfo                                                             */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosSetPathInfo(PCSZ pszPathName, ULONG ulInfoLevel,
                               PVOID pInfoBuf, ULONG cbInfoBuf, ULONG flOptions)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosShutdown                                                                */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosShutdown(ULONG ulReserved)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosEnumAttribute                                                           */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosEnumAttribute(ULONG ulRefType, CPVOID pvFile, ULONG ulEntry,
                                 PVOID pvBuf, ULONG cbBuf, PULONG pulCount,
                                 ULONG ulInfoLevel)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosSetFileLocks                                                            */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosSetFileLocks(HFILE hFile, __const__ FILELOCK *pflUnlock,
                                __const__ FILELOCK *pflLock, ULONG timeout,
                                ULONG flags)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosCancelLockRequest                                                       */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosCancelLockRequest(HFILE hFile, __const__ FILELOCK *pflLock)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosProtectEnumAttribute                                                    */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosProtectEnumAttribute(ULONG ulRefType, CPVOID pvFile,
                                        ULONG ulEntry, PVOID pvBuf, ULONG cbBuf,
                                        PULONG pulCount, ULONG ulInfoLevel,
                                        FHLOCK fhFileHandleLockID )
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosProtectSetFileLocks                                                     */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosProtectSetFileLocks(HFILE hFile,
                                       __const__ FILELOCK *pflUnlock,
                                       __const__ FILELOCK *pflLock,
                                       ULONG timeout, ULONG flags,
                                       FHLOCK fhFileHandleLockID)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosProtectOpen                                                             */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosProtectOpen(PCSZ pszFileName, PHFILE phf, PULONG pulAction,
                               ULONG cbFile, ULONG ulAttribute,
                               ULONG fsOpenFlags, ULONG fsOpenMode,
                               PEAOP2 peaop2, PFHLOCK pfhFileHandleLockID)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosProtectClose                                                            */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosProtectClose(HFILE hFile, FHLOCK fhFileHandleLockID)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosProtectRead                                                             */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosProtectRead(HFILE hFile, PVOID pBuffer, ULONG cbRead,
                               PULONG pcbActual, FHLOCK fhFileHandleLockID)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosProtectWrite                                                            */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY  DosProtectWrite(HFILE hFile, CPVOID pBuffer, ULONG cbWrite,
                                 PULONG pcbActual, FHLOCK fhFileHandleLockID)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosProtectQueryFHState                                                     */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosProtectQueryFHState(HFILE hFile, PULONG pMode,
                                       FHLOCK fhFileHandleLockID)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosProtectSetFHState                                                       */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosProtectSetFHState(HFILE hFile, ULONG mode,
                                     FHLOCK fhFileHandleLockID)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosProtectSetFileSize                                                      */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosProtectSetFileSize(HFILE hFile, ULONG cbSize,
                                      FHLOCK fhFileHandleLockID)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosProtectSetFilePtr                                                       */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosProtectSetFilePtr(HFILE hFile, LONG ib, ULONG method,
                                     PULONG ibActual, FHLOCK fhFileHandleLockID)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosProtectQueryFileInfo                                                    */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosProtectQueryFileInfo(HFILE hf, ULONG ulInfoLevel,
                                        PVOID pInfo, ULONG cbInfoBuf,
                                        FHLOCK fhFileHandleLockID)
{
    return ERROR_ACCESS_DENIED;
}


/*----------------------------------------------------------------------------*/
/* DosProtectSetFileInfo                                                      */
/*    Notes:                                                                  */
/*       - this function is not supported and always returns                  */
/*         ERROR_ACCESS_DENIED                                                */
/*----------------------------------------------------------------------------*/

APIRET APIENTRY DosProtectSetFileInfo(HFILE hf, ULONG ulInfoLevel,
                                      PVOID pInfoBuf, ULONG cbInfoBuf,
                                      FHLOCK fhFileHandleLockID)
{
    return ERROR_ACCESS_DENIED;
}

