/*    
	System.c	2.41
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

* This file contains system-related API functions 
*
*	todo: 	InquireSystem stubbed out
*	 	equipment flag is not set
*		fpu flags not configurable	
*		getnumtasks not set correctly
*		locksegment/unlocksegment stubbed out
*		should allow version to be configured	
*		winflags should be configurable
*		output debug should be re-directable, currently both log/stderr
*		get free space should be configurable or actual
*   
********************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <stdarg.h>

#include "windows.h"
#include "windowsx.h"
#include "kerndef.h"
#include "Log.h"

#include "mfs_fileio.h"
#include "mfs_config.h"
#include "WinConfig.h"
#include "System.h"

unsigned _dos_getfileattr(char *, unsigned short *);
extern LPSTR strpbrkr(LPCSTR,char *);
DWORD mfs_regexp(char * , char * , DWORD , DWORD );

extern char **environ;

/* internal routines */

#define PUBLIC
#define PRIVATE	static

#define DOSMINOR	0
#define DOSMAJOR	5
#define	DOSVERSION()	(DOSMAJOR<<8|DOSMINOR)
#define WINMINOR	10
#define WINMAJOR	3
#define	WINVERSION()	(WINMINOR<<8|WINMAJOR)

DWORD
InquireSystem(WORD wFlag, WORD wDriveNumber, BOOL bOptionalEnable)
{
    APISTR((LF_APISTUB,"InquireSystem(WORD=%x,WORD=%x,BOOL=%d)\n",
	wFlag,wDriveNumber,bOptionalEnable));
    return 0L;
}

WORD
Get80x87SaveSize(void)
{
    APISTR((LF_APISTUB,"Get80x87SaveSize()\n"));
#ifndef	NOFPU87
    return 0x5e;
#else
    return 0;
#endif
}

LPSTR
GetDOSEnvironment(void)
{
    APISTR((LF_APISTUB,"GetDOSEnvironment()\n"));
#ifndef	NETWARE
    return *environ;
#else
    return 0;
#endif
}

int     WINAPI 
GetInstanceData(HINSTANCE hInst, BYTE* lpData, int nCnt)
{
    APISTR((LF_APICALL,"GetInstanceData(HINSTANCE=%x,BYTE *=%x,int=%x)\n",
	hInst,lpData,nCnt));
    APISTR((LF_APIRET,"GetInstanceData: returns int 0\n"));
    return 0;
}

HGLOBAL
LockSegment(UINT uSegment)
{
    APISTR((LF_APISTUB,"LockSegment(segment=%x)\n",uSegment));
    return 0;
}

void
UnlockSegment(UINT uSegment)
{
    APISTR((LF_APISTUB,"UnlockSegment(segment=%x)\n",uSegment));
}

DWORD	FAR PASCAL 
GetVersion(void)
{
	return MAKELONG(WINVERSION(),DOSVERSION());
}

BOOL	WINAPI
GetVersionEx(LPOSVERSIONINFO lpOSVersionInfo)
{
	APISTR((LF_API, "GetVersionEx:"
		" (API) lpOSVersionInfo %p\n",
		lpOSVersionInfo));

	if (!lpOSVersionInfo)
		return (FALSE);
	lpOSVersionInfo->dwMajorVersion = 4;
	lpOSVersionInfo->dwMinorVersion = 0;
	lpOSVersionInfo->dwBuildNumber = 0x04000033;
	lpOSVersionInfo->dwPlatformId = VER_PLATFORM_WIN32_WINDOWS;
	strcpy(lpOSVersionInfo->szCSDVersion, "Willows Toolkit 2.0");

	return (TRUE);
}

DWORD FAR PASCAL 
GetWinFlags(void)
{
	return (LONG) (
#ifndef	X386
		WF_CPU286|WF_STANDARD|
#else
		WF_CPU386|WF_ENHANCED|
#endif
		WF_PMODE
#ifndef	NOFPU87
		|WF_80x87
#endif
		);
}

/**************************************************************************/

void
OutputDebugString(LPCSTR lpOutput)
{
	logstr(LF_DEBUG,lpOutput);
}

#ifdef LATER
/***************************************************
 *
 *	flags:
 *		DBF_TRACE
 *		DBF_WARNING
 *		DBF_ERROR
 *		DBF_FATAL
 *
 ***************************************************/
void    FAR _cdecl 
DebugOutput(UINT flags,LPCSTR * lpsz, ...)
{
	char	buf[512];
	va_list args;

	va_start(args, lpsz);

	vsprintf(buf,lpsz, args);
	va_end(args);
	OutputDebugString(buf);
	return strlen(buf);
}
#endif

void
LogError(UINT uerr, void FAR *lpvInfo)
{
	char output[512];
	wsprintf(output,"err=%d [0x%x] @%x\n",
		uerr,uerr,lpvInfo);
	logstr(LF_ERROR,output);
}

void
LogParamError(UINT uerr,FARPROC lpfn,void FAR *lpvParam)
{
	char output[512];
	wsprintf(output,"LogParamError: err=%d [0x%x] @%x %x\n",
		uerr,uerr,lpfn,lpvParam);
	logstr(LF_ERROR,output);
}

BOOL
GetWinDebugInfo(WINDEBUGINFO FAR *lpwdi,UINT flags)
{
    APISTR((LF_APISTUB,"GetWinDebugInfo(WINDEBUGINFO *%x,UINT=%x)\n",
	lpwdi,flags));
    return FALSE;
}

BOOL
SetWinDebugInfo(const WINDEBUGINFO FAR *lpwdi)
{
    APISTR((LF_APISTUB,"SetWinDebugInfo(WINDEBUG *%x)\n",lpwdi));
    return FALSE;
}

LONG
GetSystemDebugState(void)
{
    LONG rc;
#ifdef DEBUG
    rc = 1;
#else
    rc = 0;
#endif
    APISTR((LF_APICALL,"GetSystemDebugState()\n"));
    APISTR((LF_APIRET,"GetSystemDebugState: returns LONG %d\n",rc));
    return rc;
}

int
SetDirString(HWND hWnd,LPSTR lpSourceTarget )
{
	char 	String[256];
	LPSTR	lpString, lpSubString, lpDir;
	int	Offset;
	RECT 	Rect;
	HDC	hDC;
	int	Width,Length;

	if(hWnd) {
		GetClientRect(hWnd,&Rect);
		Width = Rect.right - Rect.left;

		hDC = GetDC(hWnd);
		Length = LOWORD(GetTextExtent(hDC,lpSourceTarget,strlen(lpSourceTarget)));

/*---------------- Commented out by tdd to move the ... to the middle
		while (len > width) {
			lp = strchr(lp+1,'/');
			wsprintf(lpstring,"...%s",lp);	
			len = LOWORD(GetTextExtent(hDC,lpstr,strlen(lpstring)));
		}
-----------------*/
		if ( Length > Width )
		{
			memset ( String, 0, sizeof ( String ) );
			lpString = ( char* )& ( String [ 0 ] );
			lpDir    = lpSourceTarget;
			lstrcpy ( lpString, lpSourceTarget ); 
			if ( ( Offset = _fstrcspn ( lpString + 1, "/\\" ) ) &&
				( *( lpString = lpString + Offset + 2 ) ) )
			{
				*lpString++ = '.';
				*lpString++ = '.';
				*lpString++ = '.';
			}
			if ( ( Offset = _fstrcspn ( lpDir + 1, "/\\" ) ) &&
				( *( lpSubString = lpSourceTarget + Offset + 1 ) ) )
			{
				while ( ( *lpSubString ) && ( Length > Width ) )
				{
					if ( ( Offset = _fstrcspn ( lpSubString + 1, "/\\" ) ) &&
						( *( lpSubString = lpSubString + Offset + 1 ) ) )
					{
						sprintf ( lpString, "%s", lpSubString );
						Length = LOWORD ( GetTextExtent ( hDC, String, strlen ( String ) ) );
						lpSubString++;
					}
				}
				lstrcpy ( lpSourceTarget, String );
			}
		  
		}
		ReleaseDC(hWnd,hDC);

#ifdef LATER
	we need to GETFONT for the edit control
#endif
		
    		SendMessage(hWnd, WM_SETTEXT, 0,(LONG)lpSourceTarget);
	}
	return 0;
}

void ListDir (
	HWND                    hListWnd,
	HWND                    hTextWnd,
	UINT                    Flags,
	LPSTR                   lpBuffer )

{

	int             Dir;
	char            CurrentPath [ 255 ];
	char*           pCurrentObject;
	struct stat     Status;
	BOOL            MatchPattern = strpbrk ( lpBuffer, "*?" ) ? TRUE : FALSE;
	WORD            FileAttributes;
	WORD            Drive;
	WORD            CurrentDrive;
	WORD            Index;
	char            CurrentWorkingDirectory [ 255 ];
	char            DriveMap [ 255 ];
	int             ErrorCode;


	MFS_GETCWD ( 0, CurrentPath, sizeof ( CurrentPath ) );
	lstrcpy ( CurrentWorkingDirectory, CurrentPath );
	if ( hTextWnd )
#ifdef WINOS
		SetWindowText ( hTextWnd, CurrentPath );
#else   
		SetDirString ( hTextWnd, CurrentPath );
#endif


	if ( ( hListWnd ) && 
	     ( Dir = MFS_OPENDIR ( CurrentWorkingDirectory ) ) )
	{
		SendMessage ( hListWnd, WM_SETREDRAW, 0, 0L );
		while ((pCurrentObject = ( char* )MFS_READDIR ( Dir ) ))
		{
			if ( ( MatchPattern ) && 
			     ( ! mfs_regexp ( pCurrentObject, lpBuffer ,0,0) ) )
				continue; /*--- There is a pattern and it doesn't
						match so try the next item ---*/
			if ( MFS_STAT ( ( LPSTR )pCurrentObject, &Status ) == -1 )
				continue;

			if ( S_ISDIR(Status.st_mode) )
			{ /*--- This is a directory ---*/
				if ( ( Flags & DDL_DIRECTORY ) || 
				     ( ! ( Flags & DDL_EXCLUSIVE ) ) )
					if ( strcmp ( pCurrentObject, "." ) )
					{
						/*--- Supposed to add directories
						      and the directory is not
						      the current (".") 
						      directory ---*/
						wsprintf ( CurrentPath, "[%s]",
							   pCurrentObject );
						SendMessage ( hListWnd, 
							      LB_ADDSTRING, 0,
							      ( LPARAM ) CurrentPath );
						continue;
					}
			}
			if ( S_ISREG(Status.st_mode) )
			{ /*--- This is a regular file ---*/
				_dos_getfileattr ( pCurrentObject, &FileAttributes );
				if ( ( Flags & DDL_HIDDEN ) &&
				     ( Flags & DDL_EXCLUSIVE ) &&
				     ( ! ( FileAttributes & _A_HIDDEN ) ) )
					/*--- if we're looking for hidden files
					      and the file is not hidden continue
					      with the next file ---*/
					continue;
				else
				if ( ( FileAttributes & _A_HIDDEN ) &&
				     ( ! ( Flags & DDL_HIDDEN ) ) )
					/*--- if we're not looking for hidden
					      files and this is a hidden file
					      continue with the next file ---*/
					continue;

				if ( ( Flags & DDL_SYSTEM ) &&
				     ( Flags & DDL_EXCLUSIVE ) &&
				     ( ! ( FileAttributes & _A_SYSTEM ) ) )
					/*--- if we're looking for system files
					      and the file is not a system file
					      continue with the next file ---*/
					continue;
				else
				if ( ( FileAttributes & _A_SYSTEM ) &&
				     ( ! ( Flags & DDL_SYSTEM ) ) )
					/*--- if we're not looking for system
					      files and this is a system file
					      continue with the next file ---*/
					continue;

				if ( ( Flags & DDL_READONLY ) &&
				     ( Flags & DDL_EXCLUSIVE ) &&
				     ( ! ( FileAttributes & _A_RDONLY ) ) )
					/*--- if we're looking for read-only files
					      and the file is not a read-only
					      file, continue with the next
					      file ---*/
					continue;

				if ( ( Flags & DDL_ARCHIVE ) &&
				     ( Flags & DDL_EXCLUSIVE ) &&
				     ( ! ( FileAttributes & _A_ARCH ) ) )
					/*--- if we're looking for archive files
					      and this is not an archive file
					      then continue with the next file ---*/
					continue;
				else
				if ( ( FileAttributes & _A_ARCH ) &&
				     ( ! ( Flags & DDL_ARCHIVE ) ) )
					/*--- if we're not looking for archive
					      file and this is an archive file
					      continue with the next file ---*/
					continue;

				if ( ( Flags & DDL_DRIVES ) &&
				     ( Flags & DDL_EXCLUSIVE ) )
					/*--- if we're looking for drives
					      then forget this ---*/
					break;
	
				if ( ( Flags & DDL_DIRECTORY ) &&
				     ( Flags & DDL_EXCLUSIVE ) )
					/*--- Files are not needed ---*/
					continue;

				/*--- The file should be included so add
				      it to the list box ---*/
				SendMessage ( hListWnd, LB_ADDSTRING, 0, 
					      ( LPARAM ) pCurrentObject );
			}
		}       
		SendMessage ( hListWnd, WM_SETREDRAW, 1, 0L );
		InvalidateRect ( hListWnd, NULL, FALSE );
		MFS_CLOSEDIR ( Dir );
	}

	if ( ( hListWnd ) && ( Flags & DDL_DRIVES ) )
	{
		GetTwinString ( WCP_OPENPATH, CurrentPath, sizeof ( CurrentPath ) );
		pCurrentObject = CurrentPath;
		while ( *pCurrentObject )
		{
			Index = 0;
			while ( ( *pCurrentObject ) && ( *pCurrentObject != ':' ) )
				DriveMap [ Index++ ] = *pCurrentObject++;
			if ( *pCurrentObject )
				pCurrentObject++;
			DriveMap [ Index ] = '\0';
			if ( Index )
				SendMessage ( hListWnd, LB_ADDSTRING, 0,
				              ( LPARAM )( DriveMap ) );
		}
		Drive     = 0;
		CurrentDrive = MFS_GETDRIVE ();
		for ( Drive = 0; Drive <= 30; Drive++ )
		{
#ifdef TWIN32
			switch ( ErrorCode = GetDriveType16( Drive ) )
#else
			switch ( ErrorCode = GetDriveType( Drive ) )
#endif
			{
				default:
				case 0:
					continue;
					
				case DRIVE_REMOVABLE:
					wsprintf ( CurrentPath, "%c: %c", ( char )( 'a' + Drive ), WChar_PathSeparator );
					break;

				case DRIVE_FIXED:
					MFS_SETDRIVE ( Drive + 1 );
					wsprintf ( CurrentPath, "%c", WChar_PathSeparator );
					MFS_CHDIR ( CurrentPath );
					wsprintf ( CurrentPath, "%s", ( LPSTR )WCharSet_WildCards );
					CurrentPath [ 1 ] = '\0';
					MFS_GETDRIVEMAP ( Drive + 1, DriveMap, sizeof ( DriveMap ) );
					wsprintf ( CurrentPath, "%c: %s", 
						   ( char )( 'a' + Drive ), ( LPSTR )DriveMap );
					break;
					
				case DRIVE_REMOTE:
#ifdef WINOS
					if ( mfs_isDriveRemoveableMedia ( Drive + 1 ) )
						wsprintf ( CurrentPath, "%c:", ( char )( 'a' + Drive ) );
					else
#endif 
						wsprintf ( CurrentPath, "%c:", ( char )( 'a' + Drive ) );
					break;
			}
			if ( ( ErrorCode = ( int )SendMessage ( hListWnd, LB_ADDSTRING, 0, 
				                             ( LPARAM ) ( ( LPSTR ) CurrentPath ) ) ) != ( int ) LB_ERR )
				SendMessage ( hListWnd, LB_SETITEMDATA, ErrorCode, Drive + 1 );
			if ( Drive == CurrentDrive - 1 )
				SendMessage ( hListWnd, LB_SETCURSEL, ErrorCode, ( LPARAM )NULL );
		}
		MFS_SETDRIVE ( CurrentDrive );
		MFS_CHDIR ( CurrentWorkingDirectory );
	}

}

int WINAPI
DlgDirListComboBox(HWND hDlg,LPSTR pattern,int listctl,int textctl,UINT attr)
{
    char	 result[144];
    HWND	 hEdit;

    MFS_GETCWD(0, result, 144);
    
    hEdit = GetDlgItem(hDlg,textctl);
    SetDirString(hEdit,result);

    SendDlgItemMessage(hDlg,listctl,CB_RESETCONTENT,0,0);
    SendDlgItemMessage(hDlg,listctl,CB_DIR,attr,(LONG) pattern);

    return strlen(pattern);
}

int WINAPI
DlgDirList(HWND hDlg,LPSTR pattern,int listctl,int textctl,UINT attr)
{
    HWND 	 hList;
    HWND	 hEdit;


    if(textctl)
    	 hEdit = GetDlgItem(hDlg,textctl);
    else hEdit = 0;

    SendDlgItemMessage(hDlg,listctl,LB_RESETCONTENT,0,0);
    hList = GetDlgItem(hDlg,listctl);
    ListDir(hList,hEdit,attr,pattern); 
    
    
    return strlen(pattern);
}

/**************************************************************************/

DWORD
GetFreeSpace(UINT fuFlags)
{
	return 16*1024*1024;
}

/* these are undocumented APIs */

WORD CreateSystemTimer(int, FARPROC);
WORD KillSystemTimer(WORD);
void EnableSystemTimers(void);
void DisableSystemTimers(void);

WORD
CreateSystemTimer(int nRate, FARPROC lpCallBack)
{
    APISTR((LF_APISTUB,
	"CreateSystemTimer(int=%x,FARPROC=%x)\n",
	nRate,lpCallBack));
    return 1;
}

WORD
KillSystemTimer(WORD wTimer)
{
    APISTR((LF_APISTUB,"KillSystemTimer(WORD=%x)\n",wTimer));
    return wTimer;
}

void
EnableSystemTimers(void)
{
    APISTR((LF_APISTUB,"EnableSystemTimers()\n"));
}

void
DisableSystemTimers(void)
{
    APISTR((LF_APISTUB,"DisableSystemTimers()\n"));
}

BOOL
IsROMModule(LPCSTR lpszModuleName, HANDLE h)
{
    LOGSTR((LF_LOG,"IsROMModule: mod %s handle %x\n",
	(HIWORD(lpszModuleName))?lpszModuleName:"ATOM",h));
    return FALSE;
}

BOOL
IsROMFile(LPCSTR lpszFileName, HANDLE h)
{
    LOGSTR((LF_LOG,"IsROMFile: file %s handle %x\n",
	(HIWORD(lpszFileName))?lpszFileName:"ATOM",h));
    return FALSE;
}

BOOL WINAPI
DlgDirSelectEx(HWND hDlg,LPSTR lpstring,int nIDListBox,int flag)
{
	HWND hWnd;
	int nItem;
	char selection[256];
	char *lp;

	hWnd = GetDlgItem(hDlg, nIDListBox);
	if(hWnd == 0)
		return 0;
	nItem = SendMessage(hWnd, flag ? CB_GETCURSEL : LB_GETCURSEL, 0, 0);
	SendMessage(hWnd, flag?CB_GETLBTEXT:LB_GETTEXT, nItem, (LPARAM)selection);
	if ( selection[0] == '[' ) {
		nItem = 1;
		lp = (char *)strchr(selection, ']');	
		if ( lp ) 
			*lp = 0;
	} 
	else  
		nItem = 0;
	strcpy(lpstring,&selection[nItem]);

	return nItem;
}


BOOL WINAPI
DlgDirSelect(HWND hDlg,LPSTR lpstring,int nIDListBox)
{
	return DlgDirSelectEx(hDlg,lpstring,nIDListBox,0);
}


int WINAPI
GetFileTitle(LPCSTR lpszFile, LPSTR lpszTitle, UINT cnBuf)
{
	int     nLen, i;
	LPSTR	lpszFileTitle;

        /* Note: a space IS a valid on unix and the mac. */
	if ( !lpszFile  ||  !(nLen = strlen(lpszFile))  ||
		strpbrk(lpszFile, "*[]")  ||  *(lpszFile+nLen-1) == ':'  ||
		*(lpszFile+nLen-1) == '\\'  ||  *(lpszFile+nLen-1) == '/' )

		return -1;		/* Not valid file name */

	for ( i = 0; i < nLen; i++ ) {
		if ( !isprint(*(lpszFile+i)) )
			return -1;	/* Not valid file name */
	}	/* Don't remove {}: won't work on SGI */

	if ((lpszFileTitle = (LPSTR) strpbrkr(lpszFile, "/\\") ))
		nLen -= (++lpszFileTitle - lpszFile);
	else
		lpszFileTitle = (LPSTR)lpszFile;

	/* At this point nLen=strlen(lpszTitle) */
	if ( cnBuf < (UINT)nLen + 1 )
		return nLen;		/* Specified buffer is too small */

	strcpy(lpszTitle, lpszFileTitle);
	return 0;
}

BOOL WINAPI
DlgDirSelectComboBox(HWND hDlg,LPSTR lpstring,int nIDListBox)
{
	return DlgDirSelectEx(hDlg,lpstring,nIDListBox,1);
}

