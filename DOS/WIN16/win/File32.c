/*    
	File32.c	1.24
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

#include <fcntl.h>

#include "windows.h"
#include "Log.h"
#include "mfs_config.h"
#include "kerndef.h"
#include "WinConfig.h"
#include <sys/stat.h>
#include <string.h>

/***********************************************************************/
HANDLE 
CreateFile(LPCTSTR lpszName, DWORD fdwAccess, DWORD fdwShare,
           LPSECURITY_ATTRIBUTES lpsa, DWORD fdwCreate, 
           DWORD fdwAttr, HANDLE hTemplate)
{
  BOOL fExists = FALSE;
  BOOL fIsFile = FALSE;
  HFILE hFile  = 0;
  int nAttr = 0;
  unsigned attribute;
  
  APISTR((LF_APICALL, 
"CreateFile(LPCSTR=%s,DWORD=%lX,DWORD=%lX,LPSECURITY_ATTRIBUTES=%lX,DWORD=%lX,DWORD=%lX,HANDLE=%lX)\n", 
	  lpszName, fdwAccess, fdwShare, lpsa, fdwCreate, fdwAttr, hTemplate));
  
  fExists = (MFS_ACCESS(0, lpszName, &attribute) == 0);
  fIsFile = !(attribute & _A_SUBDIR);
  
  if (fExists && !fIsFile) {
      APISTR((LF_APIFAIL,
	"CreateFile: returns HANDLE %x\n",INVALID_HANDLE_VALUE));
      return INVALID_HANDLE_VALUE;
  }
  
  if (fdwCreate == CREATE_NEW && fExists) {
      APISTR((LF_APIFAIL,
	"CreateFile: returns HANDLE %x\n",INVALID_HANDLE_VALUE));
      return INVALID_HANDLE_VALUE;
  }
  
  if ((fdwCreate == TRUNCATE_EXISTING || fdwCreate == OPEN_EXISTING) && !fExists) {
      APISTR((LF_APIFAIL,
	"CreateFile: returns HANDLE %x\n",INVALID_HANDLE_VALUE));
      return INVALID_HANDLE_VALUE;
  }
  
/* modified: hung 01/23/97 - MFC might call CreateFile with fdwAccess set to GENERIC_READ | GENERIC_WRITE */

#if 0
  switch (fdwAccess)
    {
    case GENERIC_READ:
      nAttr = O_RDONLY;
      break;

    case GENERIC_WRITE:
      nAttr = O_WRONLY;
      break;

    case GENERIC_ALL:
      nAttr = O_RDWR;
      break;
    }
#endif
  nAttr = ( (fdwAccess & GENERIC_ALL) ||
	    (fdwAccess & (GENERIC_READ | GENERIC_WRITE)) ) ? O_RDWR :
          (fdwAccess & GENERIC_WRITE) ? O_WRONLY : O_RDONLY; 

  switch (fdwCreate)
    {
    case CREATE_NEW:
    case CREATE_ALWAYS:
    case TRUNCATE_EXISTING:
      nAttr |= O_CREAT | O_TRUNC;
      hFile = MFS_CREATE(lpszName, nAttr);
      break;
      
    case OPEN_ALWAYS:
    case OPEN_EXISTING:
      hFile = MFS_OPEN(lpszName, nAttr);
      break;
    }
  
  if (hFile < 0) {
      APISTR((LF_APIFAIL,
	"CreateFile: returns HANDLE %x\n",INVALID_HANDLE_VALUE));
      return INVALID_HANDLE_VALUE;
  }
  
  APISTR((LF_APIRET,
	"CreateFile: returns HANDLE %x\n",hFile));
  return (HANDLE)hFile;
}


/***********************************************************************/
BOOL 
DeleteFile(LPTSTR lpszName)
{
   return ( MFS_DELETE(lpszName) != MFS_FAILURE );
}

/***********************************************************************/
BOOL 
CopyFile(LPTSTR lpExisting, LPTSTR lpNew, BOOL fFail)
{
   return ( MFS_COPYFILE(lpExisting, lpNew, fFail) != MFS_FAILURE );
}


/***********************************************************************/
BOOL
MoveFile(LPCTSTR lpExisting, LPCTSTR lpNew)
{
   if (MFS_RENAME(lpExisting, lpNew, MOVEFILE_REPLACE_EXISTING) != MFS_FAILURE )
      return TRUE; /* We're done (and happy) */
  
   /* Can't just rename? Try to copy */
   if (MFS_COPYFILE(lpExisting, lpNew, FALSE) != MFS_FAILURE) {
      MFS_DELETE(lpExisting);
      return TRUE;
   }

  return FALSE;
}


/***********************************************************************/
BOOL
MoveFileEx(LPCTSTR lpExisting, LPCTSTR lpNew, DWORD dwFlags)
{
   if (dwFlags & MOVEFILE_DELAY_UNTIL_REBOOT)
      return FALSE; /* We don't support delayed deletes/moves */
  
   if ( MFS_RENAME(lpExisting, lpNew, dwFlags) != MFS_FAILURE )
      return TRUE;

   if (dwFlags & MOVEFILE_COPY_ALLOWED)
      if ( MFS_COPYFILE(lpExisting, lpNew, (dwFlags & MOVEFILE_REPLACE_EXISTING) ? TRUE : FALSE) != MFS_FAILURE)
         {
 	 MFS_DELETE(lpExisting);
	 return TRUE;
         }

   return FALSE;
}

/***********************************************************************/
BOOL
CloseHandle(HANDLE hObject)
{
   return (MFS_CLOSE(hObject) != MFS_FAILURE);
}

/***********************************************************************/
BOOL
ReadFile(HANDLE hFile, LPVOID lpBuffer, DWORD nBytesToRead,
         LPDWORD lpBytesRead, LPOVERLAPPED lpOverlapped)
{
   DWORD count;
  
   if (lpBytesRead)
      *lpBytesRead = 0;
   if (lpOverlapped)
      MFS_SEEK(hFile, lpOverlapped->Offset, SEEK_SET, NULL);
   if ((count = MFS_READ(hFile, lpBuffer, nBytesToRead)) == MFS_FAILURE)
      return FALSE;
   if (lpBytesRead)
      *lpBytesRead = count;
    return TRUE;
}


/***********************************************************************/
BOOL
ReadFileEx(HANDLE hFile, LPVOID lpBuffer, DWORD nBytesToRead,
           LPOVERLAPPED lpOverlapped, 
           LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
  DWORD count;
  
  if (lpOverlapped)
    MFS_SEEK(hFile, lpOverlapped->Offset, SEEK_SET, NULL);

  count = MFS_READ(hFile, lpBuffer, nBytesToRead);
  lpCompletionRoutine(0 /* error code */, count, lpOverlapped);
  return count != MFS_FAILURE;
}


/***********************************************************************/
BOOL
WriteFile(HANDLE hFile, LPCVOID lpBuffer, DWORD nBytesToWrite,
          LPDWORD lpBytesWritten, LPOVERLAPPED lpOverlapped)
{
   DWORD count;
  
   if (lpOverlapped)
      MFS_SEEK(hFile, lpOverlapped->Offset, SEEK_SET, NULL);
   if (lpBytesWritten)
      *lpBytesWritten = 0;
   if ((count = MFS_WRITE(hFile, lpBuffer, nBytesToWrite)) == MFS_FAILURE)
      return FALSE;
   if (lpBytesWritten)
      *lpBytesWritten = count;
   return TRUE;

}

/***********************************************************************/
BOOL
WriteFileEx(HANDLE hFile, LPCVOID lpBuffer, DWORD nBytesToWrite,
            LPOVERLAPPED lpOverlapped,
            LPOVERLAPPED_COMPLETION_ROUTINE lpCompletionRoutine)
{
   DWORD count;
  
   if (lpOverlapped)
      MFS_SEEK(hFile, lpOverlapped->Offset, SEEK_SET, NULL);
   count = MFS_WRITE(hFile, lpBuffer, nBytesToWrite);
   lpCompletionRoutine(0 /* error code */, count, lpOverlapped);
   return count != MFS_FAILURE;
}


/***********************************************************************/
DWORD
SetFilePointer(HANDLE hFile, LONG lDistance, PLONG lpDistance,
               DWORD dwMoveMethod)
{
   long offset;
   int mode;
  
   if (dwMoveMethod == FILE_BEGIN)
      mode = SEEK_SET;
   if (dwMoveMethod == FILE_CURRENT)
      mode = SEEK_CUR;
   if (dwMoveMethod == FILE_END)
      mode = SEEK_END;
  
   if (MFS_SEEK(hFile, lDistance, dwMoveMethod, &offset) == MFS_FAILURE )
      return MFS_FAILURE;

   return offset;
}


/***********************************************************************/
BOOL
SetEndOfFile(HANDLE hFile)
{
  return MFS_WRITE(hFile, NULL, 0) != MFS_FAILURE;
}


/***********************************************************************/
BOOL
FlushFileBuffers(HANDLE hFile)
{
  return MFS_FLUSH(hFile) != MFS_FAILURE;
}

/***********************************************************************/
BOOL
LockFile(HANDLE hFile, DWORD dwOffsetLow, DWORD dwOffsetHigh,
         DWORD nBytesToLockLow, DWORD nBytesToLockHigh)
{
   return 1;
}


/***********************************************************************/
BOOL
UnlockFile(HANDLE hFile, DWORD dwOffsetLow, DWORD dwOffsetHigh,
           DWORD nBytesToUnlockLow, DWORD nBytesToUnlockHigh)
{
   return 1;
}


/***********************************************************************/
BOOL
UnlockFileEx(HANDLE hFile, DWORD dwReserved,
             DWORD nBytesToUnlockLow, DWORD nBytesToUnlockHigh,
             LPOVERLAPPED lpOverlapped)
{
   return 1;
}

/*
 * Some thoughts on the Find...File functions:
 *
 * -- Empirical evidence has shown that Win95 does not match wildcards
 *        in the path (i. e. trying to find *\* returns garbage rather
 *        than every file on the disk; only \* will work as a start).
 *        It therefore suffices to split off the mask from the pathname,
 *        open the specified directory (possibly .) and use MFS_REGEXP
 *        to try to match the filename.
 */

struct FIND_FILE_INFO
{
  int dirp;
  int in_use;
  char szMatch[255];
  char szPath[255];
} *pFindFileInfo = NULL;

int nMaxFindFileInfo = 0;

/***********************************************************************/
HANDLE
FindFirstFile(LPCTSTR lpszSearchFile, LPWIN32_FIND_DATA lpffd)
{
  int hIndex, found = 0;
  char szPath[256], *offset;
  BOOL fOkay;

  /* Create a new FIND_FILE_INFO struct, if necessary */
  if (!pFindFileInfo)
    {
      pFindFileInfo = (struct FIND_FILE_INFO *)WinMalloc(sizeof(struct FIND_FILE_INFO));
      pFindFileInfo->in_use = 0;
      nMaxFindFileInfo = 1;
    }
  
  for (hIndex = 0; (hIndex < nMaxFindFileInfo) && !found; hIndex++)
    if (!pFindFileInfo[hIndex].in_use) {
      found = 1;
      break;
    }

  if (!found)
    pFindFileInfo = (struct FIND_FILE_INFO *)WinRealloc((char *)pFindFileInfo, ++nMaxFindFileInfo * sizeof(struct FIND_FILE_INFO));

  pFindFileInfo[hIndex].in_use = 1;
  pFindFileInfo[hIndex].szPath[0] = '\0';

  /* Try to find a path and a filename in the passed filename */
  strcpy(szPath, lpszSearchFile);
  offset = strrchr(szPath, '\\');
  if (!offset)
      offset = strrchr(szPath, '/');
  if (!offset)
    {
      MFS_GETCWD(0, szPath, sizeof(szPath));
      offset = (char *)lpszSearchFile;
    }
  else {
    *offset++ = '\0';
    strcpy(pFindFileInfo[hIndex].szPath, szPath);
  }
  strcpy(pFindFileInfo[hIndex].szMatch, offset);

  /* Now we can open the directory */
  pFindFileInfo[hIndex].dirp = MFS_OPENDIR(szPath);

  if (!pFindFileInfo[hIndex].dirp)
    {
      pFindFileInfo[hIndex].in_use = 0;
      return INVALID_HANDLE_VALUE;
    }

  do {
    fOkay = FindNextFile(hIndex, lpffd);
  } while ( fOkay && 
	(!strcmp(lpffd->cFileName,".") || !strcmp(lpffd->cFileName,"..")) );

  if ( fOkay )
    return hIndex;

  FindClose(hIndex);
  return INVALID_HANDLE_VALUE;

}

/***********************************************************************/
BOOL
FindNextFile(HANDLE hFindFile, LPWIN32_FIND_DATA lpffd)
{
  char *pNameRead;
  char FileName[256];
/*  HANDLE hFile;	*/
  TWIN_FILEINFO FileInfo;

  /* Is the handle valid? */
  if (hFindFile > nMaxFindFileInfo)
    return FALSE;
  if (!pFindFileInfo[hFindFile].in_use)
    return FALSE;

  /* Try to find a file */
  while ((pNameRead = (char *)MFS_READDIR(pFindFileInfo[hFindFile].dirp)))
    if (MFS_REGEXP(pNameRead, pFindFileInfo[hFindFile].szMatch))
      { /* We found one! */
        if (pFindFileInfo[hFindFile].szPath[0] != '\0')
        {
	  strcpy(FileName, pFindFileInfo[hFindFile].szPath);
	  strcat(FileName, "/");
	  strcat(FileName, pNameRead);
        }
        else
          strcpy(FileName, pNameRead);

	if (!MFS_FILEINFO(FileName, 0, &FileInfo))
	  return FALSE;

	/*
	 *  We have the internal (TWIN) structure, so now copy
	 *  the fields we need.
	 */
	lpffd->dwFileAttributes = FileInfo.dwFileAttributes;
	lpffd->ftCreationTime = FileInfo.ftCreationTime;
	lpffd->ftLastAccessTime = FileInfo.ftLastAccessTime;
	lpffd->ftLastWriteTime = FileInfo.ftLastWriteTime;
	lpffd->nFileSizeHigh = FileInfo.nFileSizeHigh;
	lpffd->nFileSizeLow = FileInfo.nFileSizeLow;

	/*
	 *  Set the filename.  The altname we do nothing with just yet.
	 *  Make sure reserved fields are zero.
	 */
	strcpy(lpffd->cFileName, pNameRead);
        strcpy(lpffd->cAlternateFileName, "");
	lpffd->dwReserved0 = 0;
	lpffd->dwReserved1 = 0;
	return TRUE;
      }
    
  return FALSE;
}


/***********************************************************************/
BOOL
FindClose(HANDLE hFindFile)
{
  /* Is the handle valid? */
  if (hFindFile > nMaxFindFileInfo)
    return FALSE;
  if (!pFindFileInfo[hFindFile].in_use)
    return FALSE;

  MFS_CLOSEDIR(pFindFileInfo[hFindFile].dirp);
  pFindFileInfo[hFindFile].in_use = 0;

  return TRUE;
}

/***********************************************************************/
#ifdef TWIN32
UINT
GetTempFileName(LPCTSTR lpszPath, LPCTSTR lpszPrefix, 
                UINT uUnique, LPTSTR lpszTempFile)
{
  HFILE hFile;
  UINT unique = uUnique;
  
  if (!lpszPath)
    return 0;

  if (uUnique == 0)
    uUnique = GetCurrentTime();
  
  sprintf(lpszTempFile,"%s\\%c%.3s%4.4x%s",
	  lpszPath,'~',lpszPrefix,uUnique,".TMP");
  
  if (unique)
    return uUnique;
  
  while ((hFile = CreateFile(lpszTempFile, GENERIC_WRITE, 0, NULL,
			     CREATE_NEW, FILE_ATTRIBUTE_TEMPORARY,
			     0)) == INVALID_HANDLE_VALUE)
    {
      sprintf(lpszTempFile,"%s\\%c%.3s%4.4x%s",
	      lpszPath,'~',lpszPrefix,++uUnique,".TMP");
    }

  CloseHandle(hFile);
  return uUnique;
}
#endif /* TWIN32 */

/***********************************************************************/
DWORD
GetTempPath(DWORD cchBuffer, LPTSTR lpszTempFile)
{
  char szNativePath[256];

  GetTwinString(WCP_TEMP, szNativePath, sizeof(szNativePath));
  MFS_DOSPATH(lpszTempFile, szNativePath);

  return 1L;
}

/***********************************************************************/
DWORD
GetFileAttributes(LPCTSTR lpszFile)
{
   unsigned int attr;
   DWORD dwReturn;

   if (MFS_ACCESS(0, lpszFile, &attr) == MFS_FAILURE)
      return MFS_FAILURE;

   dwReturn = 0;
   if (attr & _A_SUBDIR)
      dwReturn |= FILE_ATTRIBUTE_DIRECTORY;
   if (attr & _A_RDONLY)
      dwReturn |= FILE_ATTRIBUTE_READONLY;
   if (attr & _A_HIDDEN)
      dwReturn |= FILE_ATTRIBUTE_HIDDEN;
   if (attr & _A_SYSTEM)
      dwReturn |= FILE_ATTRIBUTE_SYSTEM;
   if (attr & _A_ARCH)
      dwReturn |= FILE_ATTRIBUTE_ARCHIVE;

   if (!dwReturn)
      dwReturn = FILE_ATTRIBUTE_NORMAL;
   return dwReturn;
}

/***********************************************************************/
BOOL
SetFileAttributes(LPCTSTR lpFileName, DWORD dwAttributes)
{
  unsigned int attr;

  attr = 0;
  if (dwAttributes & FILE_ATTRIBUTE_DIRECTORY)
    attr |= _A_SUBDIR;
  if (dwAttributes & FILE_ATTRIBUTE_READONLY)
    attr |= _A_RDONLY;
  if (dwAttributes & FILE_ATTRIBUTE_HIDDEN)
    attr |= _A_HIDDEN;
  if (dwAttributes & FILE_ATTRIBUTE_SYSTEM)
    attr |= _A_SYSTEM;
  if (dwAttributes & FILE_ATTRIBUTE_ARCHIVE)
    attr |= _A_ARCH;

  return (MFS_ACCESS(1, lpFileName, &attr) != MFS_FAILURE);

}


/***********************************************************************/
DWORD
GetFileType(HANDLE hFile)
{
  LPMFSFILE pInfo;

  if (!(pInfo = ((LPMFSFILE)(MFS_FINFO(hFile)))))
    return FALSE;
      
  switch (pInfo->type)
    {
    case MSF_FILE:
      return FILE_TYPE_DISK;

    case MSF_DEVICE:
      return FILE_TYPE_CHAR;
    }

  return FILE_TYPE_UNKNOWN;
}

/***********************************************************************/
DWORD
GetFileSize(HANDLE hFile, LPDWORD lpdwFileSizeHigh)
{
   LPMFSFILE pInfo;
   if (lpdwFileSizeHigh)
     *lpdwFileSizeHigh = 0;

   pInfo = (LPMFSFILE)(MFS_FINFO(hFile));
   if (!pInfo)
     return -1;
   return pInfo->fsize;
}

/***********************************************************************/
DWORD
GetFullPathName(LPCTSTR lpszFile, DWORD cchPath, 
                LPTSTR lpszPath, LPTSTR *ppszFilePart)
{
   DWORD Length;

   if (MFS_GETCWD(0, lpszPath, cchPath) == MFS_FAILURE)
	return (DWORD)0;

   lpszPath[cchPath-1] = '\0';	/* in case of overflow */

   Length = strlen(lpszPath) + strlen(lpszFile) + 2;

   if ( Length > cchPath )
       return Length;

   strcat(lpszPath, "\\");

   *ppszFilePart = lpszPath + strlen(lpszPath);

   strcat(lpszPath, lpszFile);

   return (DWORD)strlen(lpszPath);
}


/***********************************************************************/
BOOL
CreateDirectory(LPTSTR lpszPath, LPSECURITY_ATTRIBUTES lpsa)
{
   return (MFS_MKDIR(lpszPath, 0755) != MFS_FAILURE);
}


/***********************************************************************/
BOOL
CreateDirectoryEx(LPCTSTR lpTemplate, LPCTSTR lpNew, LPSECURITY_ATTRIBUTES lpsa)
{
  int attr;

  if (MFS_ACCESS(0, lpTemplate, &attr) == MFS_FAILURE)
     return FALSE; /* Can't get permissions of template dir */

  return (MFS_MKDIR(lpNew, attr & 07777) != MFS_FAILURE);
}

/***********************************************************************/
BOOL
RemoveDirectory(LPCTSTR lpPath)
{
   return (MFS_RMDIR(lpPath) != MFS_FAILURE) ;
}


/***********************************************************************/
DWORD
GetCurrentDirectory(DWORD cchCurDir, LPTSTR lpszCurDir)
{
   if (MFS_GETCWD(0, lpszCurDir, cchCurDir) == MFS_FAILURE)
	return (DWORD)0;

   lpszCurDir[cchCurDir-1] = '\0';	/* in case of overflow */

   return (DWORD)strlen(lpszCurDir);
}


/***********************************************************************/
BOOL
SetCurrentDirectory(LPCTSTR lpPath)
{
   return ( MFS_CHDIR(lpPath) != MFS_FAILURE );
}


/***********************************************************************/
BOOL
GetBinaryType(LPCTSTR lpAppName, LPDWORD lpBinType)
{
   return 1;
}


/***********************************************************************/
HANDLE
FindFirstChangeNotification(LPTSTR lpszPath, BOOL fWatchSubTree, DWORD fdwFilter)
{
   return (HANDLE)0;
}


/***********************************************************************/
BOOL
FindNextChangeNotification(HANDLE hChange)
{
   return 1;
}


/***********************************************************************/
BOOL
FindCloseChangeNotification(HANDLE hChange)
{
   return 1;
}


/***********************************************************************/
BOOL
GetVolumeInformation(LPCTSTR lpRootPath, LPTSTR lpVolumeName,
                     DWORD nVolumeNameSize, LPDWORD lpSerialNo,
                     LPDWORD lpMaxLength, LPDWORD lpFlags,
                     LPTSTR lpFileSysName, DWORD nFileSysNameSize)
{
  /* We can only support some of these things... */
   if (lpVolumeName)
      if (MFS_GETDRIVEMAP(MFS_FILEDRIVE(lpRootPath), lpVolumeName, nVolumeNameSize) == MFS_FAILURE)
         return FALSE;
   if (lpMaxLength)
      *lpMaxLength = 255;
   if (lpFlags)
      *lpFlags = FS_CASE_IS_PRESERVED | FS_CASE_SENSITIVE; /* For Un*x */
   if (lpFileSysName)
      lstrcpy(lpFileSysName, "XDOS");

  return TRUE;
}


/***********************************************************************/
BOOL
GetDiskFreeSpace(LPCTSTR lpszRootPath, LPDWORD lpSectorsPerCluster,
                 LPDWORD lpBytesPerSector, LPDWORD lpFreeClusters,
                 LPDWORD lpClusters)
{
  int drive;
  DISKFREE diskfree;

  drive = MFS_FILEDRIVE(lpszRootPath);
  if (MFS_DISKFREE(drive, &diskfree) == MFS_FAILURE)
    return FALSE;

  *lpSectorsPerCluster = diskfree.sectors_per_cluster;
  *lpBytesPerSector = diskfree.sector_size;
  *lpFreeClusters = diskfree.free_clusters;
  *lpClusters = diskfree.total_clusters;

  return TRUE;
}


/***********************************************************************/
#ifdef TWIN32
UINT
GetDriveType(LPTSTR lpszRootPath)
{
   return MFS_DRIVETYPE(MFS_FILEDRIVE(lpszRootPath));
}
#endif /* TWIN32 */

/***********************************************************************/
DWORD
GetLogicalDrives(void)
{
#ifdef LATER
  DWORD dwMask = 0;
  int i;

  for (i = 31; i >= 0; i--, dwMask <<= 1)
    if (MFS_DRIVETYPE(i))
      dwMask |= 0x1;

  return dwMask;
#endif
  return 1L;
}


/***********************************************************************/
DWORD
GetLogicalDriveStrings(DWORD cchBuffer, LPTSTR lpszBuffer)
{
  int i, ok = TRUE, count = 0, written = 0;
  char szBuffer[16];

  for (i = 0; i < 32; i++)
    if (MFS_DRIVETYPE(i))
      {
	count++;
	if (written + 4 > cchBuffer)
	  {
	    ok = FALSE;
	    continue;
	  }
	wsprintf(szBuffer, "%c:\\", i + 'A');
	strcpy(lpszBuffer + written, szBuffer);
	written += 4;
      }

  if (written != cchBuffer)
    *(lpszBuffer + written) = '\0';
  else
    ok = FALSE;

  if (!ok)
      return count * 4 + 1;
  return written;
}

/***********************************************************************/
BOOL
SetFileTime(HANDLE hFile, LPFILETIME lpftCreation, LPFILETIME lpftAccess,
	    LPFILETIME lpftWrite)
{
   WORD wDOSDate, wDOSTime;
   unsigned int uiDOSDate, uiDOSTime;
  
  /* Like DOS, the only thing we know about is modify date/time... */
   if (!lpftWrite)
      return 0;
  
   FileTimeToDosDateTime(lpftWrite, &wDOSDate, &wDOSTime);
   uiDOSDate = (unsigned int)wDOSDate;
   uiDOSTime = (unsigned int)wDOSTime;
   return ( MFS_TOUCH(hFile, 1, &uiDOSTime, &uiDOSDate) != MFS_FAILURE );


}

/***********************************************************************/
BOOL	WINAPI
GetFileTime(HANDLE hFile, LPFILETIME lpftCreation, LPFILETIME lpftAccess,
	    LPFILETIME lpftWrite)
{
    TWIN_FILEINFO FileInfo;

    if (!MFS_FILEINFO(0, hFile, &FileInfo))
	return (FALSE);

    if (lpftCreation)
	*lpftCreation = FileInfo.ftCreationTime;
    if (lpftAccess)
	*lpftAccess = FileInfo.ftLastAccessTime;
    if (lpftWrite)
	*lpftWrite = FileInfo.ftLastWriteTime;

    return (TRUE);
}

/* (WIN32) Search Path ***************************************************** */

DWORD	WINAPI
SearchPath(LPCTSTR lpszPath, LPCTSTR lpszFile, LPCTSTR lpszExtension,
	DWORD dwBufferSize, LPTSTR lpszBuffer, LPTSTR *plpszFilePath)
{
	return (0);
}
