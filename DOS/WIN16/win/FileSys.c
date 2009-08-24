/*    
	FileSys.c	2.33
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

 */


#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

#include "windows.h"
#include "windowsx.h"

#include "kerndef.h"
#include "Log.h"
#include "Resources.h"
#include "Module.h"
#include "mfs_config.h"
#include "mfs_fileio.h"
#include "compat.h"
#include "WinConfig.h"
#include "FileSys.h"

extern char *strpbrkr(char *,char *);
extern int  GetCompatibilityFlags(int);

/*
 * we will support a system drive array, which shows the mapping
 * between drive numbers (letters) and unix directories and/or
 * devices, ala a: b:
 * 
 * we may have default mapping so that h: is the home directory,
 * other mappings are possible, eg. m: is /usr/spool/mail.
 *
 * for now, just return DRIVE_FIXED, later we will replace with
 * DRIVE_REMOVEABLE	for floppy drives using the DOS file package
 * DRIVE_REMOTE		for NFS file systems...
 */

#define DRIVE_C	'C'

UINT WINAPI
#ifdef TWIN32
GetDriveType16(int nDrive)
#else
GetDriveType(int nDrive)
#endif
{
	UINT	drvtype;

	/*   API accepts 0 as A:, etc, while MFS thinks 0 is current drive
	**   and A: starts from 1
	*/
	drvtype = (UINT)MFS_DRIVETYPE(nDrive+1);

	APISTR((LF_API,"GetDriveType(drive:%x) returns %x\n",nDrive,drvtype));
	return drvtype;
}

/*
 * If the environment variable WINDIR is set, then set WindowsDirectory
 * to that. Read system.ini to get WindowsSystemDirectory, defaulting to
 * WindowsDirectory appended with "system".
 */
 
char *WindowsSystemDirectory;
char *WindowsDir;

void
InitFileSystem()
{
    int  len; 
    char windir[_MAX_PATH];

    /* get the windows directory */
    WindowsDir = GetTwinString(WCP_WINDOWS,windir,_MAX_PATH);

    /* Remove the terminating slash if it exists */
    len = strlen(windir);
    if (windir[len-1] == '/')
        windir[len-1] = '\0';

    /* allocate a buffer to hold the directory string */
    len  = strlen(windir);    
    WindowsDir = WinMalloc(len+1);
    strcpy(WindowsDir, windir);

    /* get the system directory by appending /system */
    WindowsSystemDirectory = WinMalloc(len + 8);
    strcpy(WindowsSystemDirectory, WindowsDir);
    strcat(WindowsSystemDirectory, "/system");
    logstr(LF_SYSTEM,"Windows Directory:\t%s\n",WindowsDir);	
    logstr(LF_SYSTEM,"System Directory:\t%s\n",WindowsSystemDirectory);	
}
	
UINT WINAPI
GetSystemDirectory(LPSTR lpBuffer, UINT nSize)
{
	int	nLength,nWSDLength;

	if (lpBuffer == NULL)
	    return 0;

	if (WindowsSystemDirectory == NULL) {
	    lpBuffer[0] = '\0';
	    return 0;
	}

	nWSDLength = strlen(WindowsSystemDirectory)+1;
	nLength = min((int)nSize,(int)nWSDLength);
	strncpy(lpBuffer,WindowsSystemDirectory,nLength);

	APISTR((LF_API,"GetSystemDirectory() returns %s\n",lpBuffer));
	return nWSDLength;
}

UINT  WINAPI
GetWindowsDirectory(LPSTR lpBuffer, UINT nSize)
{
	int nLength,nWDLength;

	if (lpBuffer == NULL)
	    return 0;

	if (WindowsDir == NULL) {
	    lpBuffer[0] = '\0';
	    return 0;
	}

	nWDLength = strlen(WindowsDir);
	nLength = min((int)nSize, (int)nWDLength+1);
	strncpy(lpBuffer, WindowsDir, nLength);

	APISTR((LF_API,"GetWindowsDirectory() returns %s\n",lpBuffer));
	return (nSize <= nWDLength) ? nWDLength+1 : nLength-1;
}

BYTE WINAPI
GetTempDrive(char cDriveLetter)
{
	return DRIVE_C;
}

/*
 * build a temporary filename
 * if unique == 0
 *	use system time to create unique name,
 * else
 *	use unique to generate specific name
 * return the specific unique key used.
 */

int WINAPI
#ifdef TWIN32
GetTempFileName16(BYTE cDriveLetter, LPCSTR lpPrefix,
		UINT unique, LPSTR lpTempFile)
#else
GetTempFileName(BYTE cDriveLetter, LPCSTR lpPrefix,
		UINT unique, LPSTR lpTempFile)
#endif
{
	char szNativePath[256];
	char szNativePath1[256];
    LPSTR lpszPath = szNativePath;
    HFILE hFile;
    UINT uUnique = unique;
    OFSTRUCT of;
    int Len;
	
    cDriveLetter = cDriveLetter & TF_FORCEDRIVE ?
		cDriveLetter & ~TF_FORCEDRIVE:DRIVE_C;

	GetTwinString(WCP_TEMP, szNativePath1, sizeof(szNativePath1));
	MFS_DOSPATH(szNativePath, szNativePath1);


    if ( *(lpszPath+1) == ':' ) {
	cDriveLetter = *lpszPath;
	lpszPath += 2;
    } 

    if ( *lpszPath == '\\' || *lpszPath == '/' )
	lpszPath++;

    Len = strlen(lpszPath);
    if ( lpszPath[Len-1] == '\\' || lpszPath[Len-1] == '/' )
	lpszPath[Len-1] = '\0';

    if (uUnique == 0)
	uUnique = GetCurrentTime();

    sprintf(lpTempFile,"%c:\\%s\\%c%.3s%4.4x%s",
	cDriveLetter,lpszPath,'~',lpPrefix,uUnique,".TMP");

    APISTR((LF_API,
	"GetTempFileName(drive:%x,prefix:%s,unique=%x,tempfile=%s)\n",
	cDriveLetter,lpPrefix,uUnique,lpTempFile));

    if (unique)
	return uUnique;

    while ((hFile = OpenFile(lpTempFile,&of,OF_READWRITE|OF_EXIST)) !=
			HFILE_ERROR) {
	sprintf(lpTempFile,"%c:\\%s\\%c%.3s%4.4x%s",
	    cDriveLetter,lpszPath,'~',lpPrefix,++uUnique,".TMP");
	APISTR((LF_API,"GetTempFileName: trying %s...\n",lpTempFile));
    }

    if ((hFile = OpenFile(lpTempFile,&of,OF_CREATE)) == HFILE_ERROR)
	return 0;
    else {
	_lclose(hFile);
	return uUnique;
    }
}

/*
 * we should interface to system here, and get
 * available amount upto the system limit.
 */
UINT WINAPI
SetHandleCount(UINT nCount)
{
	return nCount;
}


UINT WINAPI
SetErrorMode(UINT fuErrorMode)
{
	static UINT fuCurrentMode = 0;
	UINT fuTemp;

	fuTemp = fuCurrentMode;
	fuCurrentMode = fuErrorMode;
	APISTR((LF_API,"SetErrorMode(mode:%x) returns %x\n",
		fuErrorMode,fuTemp));
	return fuTemp;
}

/************************************************************************/

#define MIN_RES_FD 0x100
#define MAKE_RES_FD(d) ((d) + MIN_RES_FD)
#define GET_INDEX(d) ((d) - MIN_RES_FD)
#define IS_RES_FD(d) (((d) >= MIN_RES_FD) ? 1 : 0)

#define NFILEHANDLES	128

struct {
	HANDLE hInst;
	HANDLE rsrc_handle;
	long file_ptr;
} rsrc_file_data[NFILEHANDLES];

int WINAPI
AccessResource(HINSTANCE hInst, HRSRC hRsrc) 
{ 
#ifdef LATER
	int fd;

	for(fd=0;fd < NFILEHANDLES && rsrc_file_data[fd].rsrc_handle != 0;fd++);

	if(fd == NFILEHANDLES)
		return -1;

	rsrc_file_data[fd].hInst = hInst;
	rsrc_file_data[fd].rsrc_handle = hRsrc;
	rsrc_file_data[fd].file_ptr = 0;
	fd = MAKE_RES_FD(fd);
	return fd;
#endif
	/* mem_xxx routines are disconnected */
	/* read AccessResource is in binary layer... */
	return -1;
}

#ifdef LATER
static int
mem_read(int fd, char *buffer, int nread)
{
	HRSRC hRsrc;
	HINSTANCE hInst;
	HANDLE h;
	int index;
	char *p;

	index = GET_INDEX(fd);
	hRsrc = rsrc_file_data[index].rsrc_handle;
	hInst = rsrc_file_data[index].hInst;
	h = LoadResource(hInst, hRsrc);
	p = (LPSTR)LockResource(h) + rsrc_file_data[index].file_ptr;
	memcpy(buffer, p, nread);
	rsrc_file_data[index].file_ptr += nread;
	UnlockResource(h);
	return nread;
}

static long
mem_seek(int fd, long offset, int origin)
{
	int index;

	index = GET_INDEX(fd);
	switch (origin) {
		case SEEK_SET:		/* from beginning */
			rsrc_file_data[index].file_ptr = offset;
			break;
		case SEEK_CUR:		/* from current position */
			rsrc_file_data[index].file_ptr += offset;
			break;
		case SEEK_END:		/* from end */
		default:
			break;
	}
	return rsrc_file_data[index].file_ptr;
}

static int
mem_close(int fd)
{
	rsrc_file_data[GET_INDEX(fd)].rsrc_handle = 0;
	return 0;
}
#endif

/************************************************************************/

#define OF_OPENMASK	(OF_READ|OF_READWRITE|OF_WRITE|OF_CREATE)
#define OF_FILEMASK	(OF_DELETE|OF_PARSE)
#define OF_MASK		(OF_OPENMASK|OF_FILEMASK)

int
_lopen(LPCSTR lpfname,int wFunction)
{
	int fd;
	int nFunction;
   	
	/* Don't assume a 1:1 relationship between OF_* modes and O_* modes */
	/* Here we translate the read/write permission bits (which had better */
	/* be the low 2 bits.  If not, we're in trouble.  Other bits are  */
	/* passed through unchanged */
	
	nFunction = wFunction & 3;
   	
	switch (wFunction & 3) {
		case OF_READ:
			nFunction |= O_RDONLY;
   			break;
   		case OF_READWRITE:
   			nFunction |= O_RDWR;
   			break;
   		case OF_WRITE:
   			nFunction |= O_WRONLY;
   			break;
   		default:
   			ERRSTR((LF_ERROR, "_lopen: bad file open mode %x\n", wFunction));
   			return HFILE_ERROR;
   	}

	fd = MFS_OPEN((char*)lpfname, nFunction);
	if (fd < 0)
		 return HFILE_ERROR;
	return fd;
}

int
_lclose(int fd)
{
	if (fd == (HFILE)-1)
		return -1;
	return MFS_CLOSE(fd);
}

int
_lcreat(LPCSTR lpfname,int fileattr)
{
    int fd;
    int openmask = O_CREAT|O_TRUNC;

    openmask |= O_RDWR;			/* READONLY case not supported */
    fd = MFS_CREATE((char*)lpfname, openmask);
    if (fd < 0)
	return HFILE_ERROR;
    return fd;
}

LONG
_llseek(int fd, long offset, int origin)
{
    long position;
    if (fd == (HFILE)-1)
	return HFILE_ERROR;
    position = HFILE_ERROR;
    MFS_SEEK(fd, offset, origin, &position);
    return position;
}

long
_hread(HFILE fd,void _huge* hpvbuffer, long count)
{
    if (fd == (HFILE)-1)
	return HFILE_ERROR;
    return MFS_READ(fd, hpvbuffer, count);
}

long
_hwrite(HFILE fd,const void _huge* hpvbuffer, long count)
{
    return MFS_WRITE(fd, (char*)hpvbuffer, count);
}

UINT 
_lread(HFILE fd,LPVOID buffer,UINT count)
{
    return _hread(fd,(void _huge*) buffer,(long) count);
}

UINT 
_lwrite(HFILE fd,const VOID *buffer,UINT count)
{
    return MFS_WRITE(fd, (char*)buffer, count);
}

int
_ldelete(LPCSTR lpfname)	
{
    int fd;

    fd = MFS_DELETE((char*)lpfname);
    if (fd < 0)
	return HFILE_ERROR;
    return fd;
}

/***************************************************************************/

/*
 * open the file given by lpszFileName
 * search in all the right places...
 * put in the 'resulting' name in buf
 * 
 * Search order is...
 * 1. The current directory and/or a fully qualified name.
 * 2. The Windows directory.
 * 3. The Windows system directory.
 * 4. The directory containing the executable file for hTask.
 * 5. The directories listed in the PATH env variable.
 * 
 * bailout w/ failure...
 */

static int
appendslash(char *path) 
{
	char *slash;
	if((slash = strrchr(path,'/'))) {
		slash++;
		if(*slash)
			return 1;
		return 0;
	}
	return 1;
}

/* 
 *	OpenFile
 *	OpenFile is the primary interface to the filesystem.
 *	There are three types of operations that can be performed.
 *		1) open/create a file	OF_READ,OF_READWRITE,OF_WRITE,OF_CREATE
 *		2) delete a file	OF_DELETE
 *		3) parse a filename	OF_PARSE
 *
 *	For case 1 and 2, you can pass a flag OF_REOPEN to use the
 *	data in an OFSTRUCT to reopen the file instead of using 
 *	the specified filename.
 *
 *	For case 1, you can pass a flag, OF_EXIST which will close the
 *	file after it has been opened/created.
 *
 *	For case 1 and 2, except for OF_CREATE, you can use the OF_PROMPT
 *	flag, with the optional OF_CANCEL flag to prompt the user for
 *	a filename.
 *	
 *	For OF_READ only, you can use the OF_VERIFY flag to see if the
 *	file data/time stamp has changed.
 *
 *	issues:
 *		must special case devices, LPTx: COMx:
 */

HFILE
OpenEngine(LPCSTR lpszFile,LPOFSTRUCT lpofs,UINT wFunction)
{
    char szFullPath[_MAX_PATH];
    int mode, nLen;
    LPSTR lpTemp,lpszFileName = (LPSTR)lpszFile;
    HFILE hFile;
    char *path,*next,*PATH;

    /* set the open mode from the lowest two bits, this is for _lopen  */
    mode  = wFunction & 0x3;

    /* PHASE 1: use current directory and/or fully qualified name  */
    /* note:    this will also resolve any leading <DRIVE>: prefix */
    /*          the <DRIVE>: prefix can resolve to either a mapped */
    /*  	directory or a physical device (floppy)	 	   */

#ifdef LATER
	handle case of OF_REOPEN
	does this open the file given by the lpofs struct?
        if it fails, do we restart the whole thing?
#endif

    MFS_ALTPATH(szFullPath, lpszFileName);

    hFile = _lopen(szFullPath, mode);

    /* if it worked, then we are done... */
    if (hFile != HFILE_ERROR) {
	nLen = min((int)strlen(szFullPath),_MAX_PATH-1);
	strncpy(lpofs->szPathName,szFullPath,nLen);
	lpofs->szPathName[nLen] = '\0';
	return hFile;
    }

    /* if it was fully qualified, and we are NOT to search then quit */
    /* LATER do we want basname here?  we may have foo/bar           */
    /*       in which case do we want just the bar or foo/bar?       */
    if ((*lpszFileName == '/') || (*(lpszFileName+1) == ':') ||
		(*lpszFileName == '\\')) {
	if ((wFunction & OF_SEARCH) == 0)
	    return hFile;
    }

    /* note: if lpTemp then we do have strlen, so check if its the last */
    if ((lpTemp = (LPSTR)strpbrkr(szFullPath,"/:\\")))
	lpszFileName = lpTemp+1;

    if (strlen(lpszFileName) == 0)
	return hFile;

    /* PHASE 2: use windows directory */
    GetWindowsDirectory(lpofs->szPathName, 255);
    if (appendslash(lpofs->szPathName))
    	strcat(lpofs->szPathName,"/");
    strcat(lpofs->szPathName,lpszFileName);

    hFile = _lopen(lpofs->szPathName,mode);
    if (hFile != HFILE_ERROR) {
	return hFile;
    }

    /* PHASE 3: use system directory */
    GetSystemDirectory(lpofs->szPathName, 255);
    if (appendslash(lpofs->szPathName))
    	strcat(lpofs->szPathName,"/");
    strcat(lpofs->szPathName,lpszFileName);

    hFile = _lopen(lpofs->szPathName,mode);
    if (hFile != HFILE_ERROR) {
	return hFile;
    }

    /* PHASE 4: use openpath to get subcomponent */
    path = GetTwinString(WCP_OPENPATH,0,0);

    if(path == 0)
	return hFile;

    PATH = (char *) WinStrdup(path);
    /* walk the path... */
    
    for(next = PATH;next && *next;next = path) {

	/* get the next element and terminate it */
	path = strchr(next,':');
	if(path) {
		*path = 0;		/* end the current element */
		path++;			/* point to the nextone    */
	}

	/* build the current name... */
	strcpy(lpofs->szPathName,next);
        if(appendslash(lpofs->szPathName))
    		strcat(lpofs->szPathName,"/");
	strcat(lpofs->szPathName,lpszFileName);

	hFile = _lopen(lpofs->szPathName,mode);
	if (hFile != HFILE_ERROR) {
		WinFree(PATH);
		return hFile;
	}
    }

    WinFree(PATH);
    
    return hFile;
}

