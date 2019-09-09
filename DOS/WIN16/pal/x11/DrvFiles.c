/********************************************************************

	@(#)DrvFiles.c	1.9 System-specific Files Driver Module (Unix Version)
    	Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.


For more information about the Willows Twin Libraries.

	http://www.willows.com	

To send email to the maintainer of the Willows Twin Libraries.

	mailto:twin@willows.com 

********************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <time.h>
#include <sys/types.h>
#include <Driver.h>
#include <sys/stat.h>

#include "windows.h"
#include "About.h"

#include "DrvCommon.h"

/*
 *  DrvGetFileInfo --- Return information about the given file.
 *
 *	If lpFileName is not NULL, use the filename to lookup
 *      the information.  If lpFileName is NULL, then dwFileHandle
 *      is the handle of an already open file, so use that handle
 *      to get the information.
 */
static DWORD
DrvGetFileInfo(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
    LPSTR lpFileName = (LPSTR)dwParam1;
    DWORD dwFileHandle = (DWORD)dwParam2;
    LPTWIN_FILEINFO lpFileInfo = (LPTWIN_FILEINFO) lpStruct; 
    struct stat st;
    uid_t uid;
    gid_t gid;
    int ret;
    unsigned int mode;

    if (lpFileName != NULL)
      ret = lstat(lpFileName, &st);   /* Make sure we follow symlinks */
    else
	ret = fstat(dwFileHandle, &st);

    if (ret < 0)
    	return FALSE;

    TWIN_DrvUnixTime2Filetime(st.st_ctime, &(lpFileInfo->ftCreationTime));
    TWIN_DrvUnixTime2Filetime(st.st_atime, &(lpFileInfo->ftLastAccessTime));
    TWIN_DrvUnixTime2Filetime(st.st_mtime, &(lpFileInfo->ftLastWriteTime));
    lpFileInfo->nNumberOfLinks = st.st_nlink;
    lpFileInfo->nFileSizeHigh = 0;
    lpFileInfo->nFileSizeLow = st.st_size;
    lpFileInfo->nFileIndexHigh = 0;
    lpFileInfo->nFileIndexLow = st.st_ino;
    lpFileInfo->dwVolumeSerialNumber = st.st_dev;
    lpFileInfo->dwFileAttributes = 0;
    mode = st.st_mode;
    if (S_ISDIR(mode))
	lpFileInfo->dwFileAttributes |= FILE_ATTRIBUTE_DIRECTORY;
    if (S_ISCHR(mode))
	lpFileInfo->dwFileAttributes |= FILE_ATTRIBUTE_SYSTEM;
    if (S_ISBLK(mode))
	lpFileInfo->dwFileAttributes |= FILE_ATTRIBUTE_SYSTEM;

    uid = geteuid();
    gid = getegid();
    if (uid == st.st_uid)
    {
	if (!(mode & S_IWUSR))
	    lpFileInfo->dwFileAttributes |= FILE_ATTRIBUTE_READONLY;
    }
    else if (gid == st.st_gid)
    {
	if (!(mode & S_IWGRP))
	    lpFileInfo->dwFileAttributes |= FILE_ATTRIBUTE_READONLY;
    }
    else
    {
	if (!(mode & S_IWOTH))
	    lpFileInfo->dwFileAttributes |= FILE_ATTRIBUTE_READONLY;
    }

    if (lpFileInfo->dwFileAttributes == 0)
	lpFileInfo->dwFileAttributes = FILE_ATTRIBUTE_NORMAL;

    return TRUE;
}

static DWORD DrvFilesDoNothing(LPARAM, LPARAM, LPVOID);

DWORD DrvFilesTab(void);

static TWINDRVSUBPROC DrvFilesEntryTab[] = {
	DrvFilesDoNothing,
	DrvFilesDoNothing,
	DrvFilesDoNothing,
	DrvGetFileInfo
};

static DWORD
DrvFilesDoNothing(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
	return 1L;
}

DWORD
DrvFilesTab(void)
{
#if defined(TWIN_RUNTIME_DRVTAB)
        DrvFilesEntryTab[0] = DrvFilesDoNothing;
        DrvFilesEntryTab[1] = DrvFilesDoNothing;
        DrvFilesEntryTab[2] = DrvFilesDoNothing;
        DrvFilesEntryTab[3] = DrvGetFileInfo;
#endif
	return (DWORD)DrvFilesEntryTab;
}	
