/*    
	dos_findfile.c	1.12
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
#include "windows.h"
#include "dos.h"
#include "io.h"
#include <time.h>
#include <errno.h>
#include <string.h>

#include "mfs_config.h"
#include "mfs_core.h"
#include "Log.h"

int _findclose(long);
extern	void	CopyDTAToC(LPBYTE, struct find_t *);

	/* library based dta for use with mfs calls */
static unsigned char lib_dta[256];

unsigned int
_dos_findfirst(char *filename, unsigned attrib, struct find_t *fileinfo)
{
	DWORD p1, p2, p3, p4;
	DWORD tretval;

	if (fileinfo == NULL)
	{
		errno = ENOENT;
		return MFS_FAILURE;
	}

		/* prep for mfs call */
	p1 = (DWORD)filename;
	p2 = (DWORD)attrib;
	p3 = (DWORD)&lib_dta;
	p4 = (DWORD)NULL;

	tretval = mfs_findfile(p1, p2, p3, p4);

	CopyDTAToC((LPBYTE)&lib_dta, fileinfo);

	return(tretval);
}


unsigned
_dos_findnext(struct find_t *fileinfo)
{
	DWORD p1, p2, p3, p4;
	DWORD tretval;

	if (fileinfo == NULL)
	{
		errno = ENOENT;
		return MFS_FAILURE;
	}

		/* prep for mfs call */
	p1 = (DWORD)NULL;
	p2 = (DWORD)fileinfo->attrib;
	p3 = (DWORD)&lib_dta;
	p4 = (DWORD)NULL;

	tretval = mfs_findfile(p1, p2, p3, p4);

	CopyDTAToC((LPBYTE)&lib_dta, fileinfo);

	return(tretval);
}

/*
 *	_findfirst(), _findnext(), findclose() are based on File32.c
 */

typedef struct find_file_info {
	int dirp;
	int in_use;
	char szMatch[255];
	char szPath[255];
} FIND_FILE_INFO;

static FIND_FILE_INFO *pFindFileInfo = NULL;

static int nMaxFindFileInfo = 0;


/*
 *	_findfirst(...) - prepare file handle and also return
 *		the first reacheable entry in the directory.
 *
 */
long
_findfirst(const char *filespec, struct _finddata_t *fileinfo)
{
	int hFFF, found = 0;
	char szPath[256], *offset;
	BOOL endFiles;

	/* Create a new FIND_FILE_INFO struct, if necessary */
	if (!pFindFileInfo)
	{
		pFindFileInfo = (FIND_FILE_INFO *)WinMalloc(sizeof(FIND_FILE_INFO));
		pFindFileInfo->in_use = 0;
		nMaxFindFileInfo = 1;
	}

	for (hFFF = 0; (hFFF < nMaxFindFileInfo) && !found; hFFF++)
	{
		if (!pFindFileInfo[hFFF].in_use)
		{
			found = 1;
			break;
		}
	}

	if (!found)
		pFindFileInfo = (FIND_FILE_INFO *)WinRealloc((char *)pFindFileInfo, ++nMaxFindFileInfo * sizeof(FIND_FILE_INFO));

	pFindFileInfo[hFFF].in_use = 1;
	pFindFileInfo[hFFF].szPath[0] = '\0';

	/* Try to find a path and a filename in the passed filename */
	strcpy(szPath, filespec);
	offset = (char *)strrchr(szPath, '\\');
	if (!offset)
	offset = (char *)strrchr(szPath, '/');
	if (!offset)
	{
		MFS_GETCWD(0, szPath, sizeof(szPath));
                offset = szPath + strlen(szPath);
                strcat(szPath, "/");
                strcat(szPath, filespec);
                *(offset++) = 0;
	}
	else
	{
		*offset++ = '\0';
		strcpy(pFindFileInfo[hFFF].szPath, szPath);
	}
	strcpy(pFindFileInfo[hFFF].szMatch, offset);

	/* Now we can open the directory */
	pFindFileInfo[hFFF].dirp = MFS_OPENDIR(szPath);

	if (!pFindFileInfo[hFFF].dirp)
	{
		pFindFileInfo[hFFF].in_use = 0;
		return INVALID_HANDLE_VALUE;
	}

	do
	{
		endFiles = _findnext(hFFF, fileinfo);
	} while ( endFiles && 
		(!strcmp(fileinfo->name,".") || !strcmp(fileinfo->name,"..")) );

	if ( !endFiles )
		return hFFF;

	_findclose(hFFF);
	return INVALID_HANDLE_VALUE;
}



int
_findnext(long handle, struct _finddata_t *fileinfo)
{
	char *pNameRead;
	char FileName[256];
/*	HANDLE hFile;	*/

	/* Is the handle valid? */
	if (handle > nMaxFindFileInfo)
		return FALSE;
	if (!pFindFileInfo[handle].in_use)
		return FALSE;

	/* Try to find a file */
	while ((pNameRead = (char *)MFS_READDIR(pFindFileInfo[handle].dirp)))
		if (mfs_regexp((DWORD)pNameRead, (DWORD)pFindFileInfo[handle].szMatch, 0L, 0L)) 
		{
			struct stat sbuf; 	 

			if (pFindFileInfo[handle].szPath[0] != '\0')
			{
				strcpy(FileName, pFindFileInfo[handle].szPath);
				strcat(FileName, "/");
				strcat(FileName, pNameRead);
			}
			else
				strcpy(FileName, pNameRead);
			fileinfo->attrib = GetFileAttributes(FileName);
			/* matched, now check file attributes */
			mfs_stat((DWORD)FileName, (DWORD)&sbuf, 0L, 0L);
			fileinfo->time_create = sbuf.st_ctime;
			fileinfo->time_access = sbuf.st_atime;
			fileinfo->time_write = sbuf.st_mtime;
			fileinfo->size = sbuf.st_size;
			strncpy(fileinfo->name, FileName, sizeof(fileinfo->name));
			return 0;
		} 
	_findclose(handle);
	return -1;
}


int
_findclose(long handle)
{
	/* Is the handle valid? */
	if (handle > nMaxFindFileInfo)
		return FALSE;
	if (!pFindFileInfo[handle].in_use)
		return FALSE;

	MFS_CLOSEDIR(pFindFileInfo[handle].dirp);
	pFindFileInfo[handle].in_use = 0;

	return TRUE;
}

