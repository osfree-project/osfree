/*    
	GetOpenFileName.c	1.45 GetOpenFileName Common Dialog
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
//#include "Win_User.h"
//#include "windowsx.h"
#include "commdlg.h"
#include "memory.h"

#include "dlgs.h"
#include "string.h"
#include "sys/types.h"
#include "sys/stat.h"
#include "dos.h"
#include "mfs_config.h"
#include "System.h"
#include "CommdlgRC.h"
#include "WinConfig.h"
#include "Log.h"
#include "porting.h"

#define WGOFNERR_NULLPOINTER            -1
#define WGOFNERR_STRUCTSIZE             -2
#define WGOFNERR_NULLWINDOW             -3
#define WGOFNERR_NULLHANDLE             -4
#define WGOFNERR_NULLTEMPLATE           -5
#define WGOFNERR_NULLFUNCTION           -6
#define WGOFNERR_GLOBALALLOC            -7
#define WGOFNERR_GLOBALLOCK             -8
#define WGOFNERR_SETPROP                -9
#define WGOFNERR_FINDRESOURCE           -10
#define WGOFNERR_LOADRESOURCE           -11
#define WGOFNERR_MAKEPROCINSTANCE       -12
#define WGOFNERR_BUFFERTOOSMALL         -13
#define WGOFNERR_CBGETCURSEL            -14
#define WGOFNERR_LBGETCURSEL            -15
#define WGOFNERR_LBGETTEXT              -16
#define WGOFNERR_GETCURRENTDIRECTORY    -17
#define WGOFNERR_INVALIDFILENAME        -18
#define WGOFNERR_DIRECTORYDOESNOTEXIST  -19
#define WGOFNERR_FILEDOESNOTEXIST       -20
#define WGOFNERR_READONLY               -21
#define WGOFNERR_GETSELCOUNT            -22
#define WGOFNERR_GETSELITEMS            -23
#define WGOFNERR_GETITEMDATA            -24
#define WGOFNERR_GETCURSEL              -25
#define WGOFNERR_GETLBTEXT              -26
#define WGOFNERR_GETCOUNT               -27

#define WGOFN_OPEN              1
#define WGOFN_SAVE              2

#define WGOFNPROP_OPENFILENAME  1
#define WGOFNPROP_INITIALDIR    2

#define WGOFNCLOSEDDIR          0x0001
#define WGOFNOPENDIR            0x0002
#define WGOFNCURRDIR            0x0004
#define WGOFNSELECTABLEDIR      0x0008

#ifdef WINOS
#define MAX_FILELENGTH          256
#else
#define MAX_FILELENGTH          512
#endif

#define WGOFNEditFileName       edt1
#define WGOFNListFiles          lst1
#define WGOFNTextDirectory      stc1
#define WGOFNListDirectories    lst2
#define WGOFNComboFilters       cmb1
#define WGOFNComboDrives        cmb2
#define WGOFNButtonHelp         pshHelp
#define WGOFNCheckReadOnly      chx1

#define WGOFN_WM_HELP           1
#define WGOFN_WM_FILEOK         2
#define WGOFN_WM_LBSELCHANGE    3
#define WGOFN_WM_SHAREVIOLATION 4
#define WGOFNWM_HELP            WGOFNGetWindowMessage( WGOFN_WM_HELP )
#define WGOFNWM_FILEOK          WGOFNGetWindowMessage( WGOFN_WM_FILEOK )
#define WGOFNWM_LBSELCHANGE     WGOFNGetWindowMessage( WGOFN_WM_LBSELCHANGE )
#define WGOFNWM_SHAREVIOLATION  WGOFNGetWindowMessage( WGOFN_WM_SHAREVIOLATION )

#define WGOFNCharSet_PathSeparators             WCharSet_PathSeparators
#define WGOFNCharSet_ExtentionSeparators        WCharSet_ExtentionSeparators
#define WGOFNCharSet_CurrentDirectory           WCharSet_CurrentDirectory
#define WGOFNCharSet_DriveSeparator             WCharSet_DriveSeparator
#define WGOFNCharSet_WildCards                  WCharSet_WildCards
#define WGOFNChar_PathSeparator                 WChar_PathSeparator
#define WGOFNChar_SecondaryPathSeparator        WChar_SecondaryPathSeparator
#ifndef WINOS
#define _stat                                   stat
#endif

#define WGOFN_PATHDRIVE                         0
#define WGOFN_FLOPPYDRIVE                       1
#define WGOFN_HARDDRIVE                         2
#define WGOFN_CDDRIVE                           3
#define WGOFN_NETWORKDRIVE                      4
#define WGOFN_RAMDRIVE                          5
#define WGOFN_OPENDIR                           6
#define WGOFN_CURRDIR                           7
#define WGOFN_DIR                               8
#define WGOFN_NUMBITMAPS                        9

typedef struct WGOFN_BITMAP_TAG
{
	WORD                          BitmapID; 
  COLORREF                      WindowColor;
	COLORREF                      HighlightColor;
	HBITMAP                       hBitmap;
	HBITMAP                       hHighlightBitmap;
} WGOFN_BITMAP;

typedef struct WGOFN_BITMAPS_TAG
{
	WGOFN_BITMAP                  Bitmaps [ WGOFN_NUMBITMAPS ];
} WGOFN_BITMAPS;

static WGOFN_BITMAPS            WGOFNBitmaps =
{
	{ 
		{ IDB_MAPPEDDRIVE, 0, 0, 0, 0 },
		{ IDB_FLOPPYDRIVE, 0, 0, 0, 0 },
		{ IDB_HARDDRIVE, 0, 0, 0, 0 },
		{ IDB_CDROMDRIVE, 0, 0, 0, 0 },
		{ IDB_NETWORKDRIVE, 0, 0, 0, 0 },
		{ IDB_RAMDRIVE, 0, 0, 0, 0 },
		{ IDB_OPENDIR, 0, 0, 0, 0 },
		{ IDB_OPENCURRDIR, 0, 0, 0, 0 },
		{ IDB_DIR, 0, 0, 0, 0 } 
	}
};

static char*                    WGOFN_OPENFILENAME_PROPERTY = "WGOFN";
static char*                    WGOFN_INITIALDIR_PROPERTY   = "WGOFNDIR";
static UINT                     WGOFNWMHelp                 = ( UINT )NULL;
static UINT                     WGOFNWMFileOK               = ( UINT )NULL;
static UINT                     WGOFNWM_LBSelChange         = ( UINT )NULL;
static UINT                     WGOFNWM_ShareViolation      = ( UINT )NULL;

#define WGOFNGetInstance        GetInstance
#define WGOFNSetExtendedError   WCDSetExtendedError

//int WINAPI GetFileTitle(LPCSTR , LPSTR , UINT );
HINSTANCE GetInstance();
DWORD WCDSetExtendedError ( DWORD );

//#ifdef WINOS
#include <dos.h>
LPSTR WINAPI strpbrkr (
	LPSTR                   lpSource,
	LPSTR                   lpSet )
	
{

	LPSTR                   lpstrpbrkr = NULL;
	
	while ( lpSource = _fstrpbrk ( ++lpSource, lpSet ) )
		lpstrpbrkr = lpSource;
	
	return ( lpstrpbrkr );
	
}
//#endif
/*============================================================================*/
short WINAPI WGOFNIsADirectory ( 
	LPSTR                   lpPath ) 
	
{

	struct stat             Status;
	
	
	if ( MFS_STAT ( lpPath, &Status ) == -1 )
		return ( FALSE );
	else
		return ( Status.st_mode & S_IFDIR );

}
/*============================================================================*/
LPSTR WINAPI WGOFNGetPropertyName ( 
	WORD                    Property )
	
{

	switch ( Property )
	{
		default:
			return ( NULL );
			
		case WGOFNPROP_OPENFILENAME:
			return ( WGOFN_OPENFILENAME_PROPERTY );
			
		case WGOFNPROP_INITIALDIR:
			return ( WGOFN_INITIALDIR_PROPERTY );
			
	}

}
/*============================================================================*/
LPOPENFILENAME WINAPI WGOFNLockOpenFileName (
	HWND                    hWnd )
  
{

	HGLOBAL                 hOpenFile = ( HGLOBAL )NULL;
	LPOPENFILENAME FAR*     lplpOpenFileName = ( LPOPENFILENAME FAR* )NULL;

  
	if ( ! ( hOpenFile = GetProp ( hWnd, WGOFNGetPropertyName ( WGOFNPROP_OPENFILENAME ) ) ) )
		return ( ( LPOPENFILENAME ) NULL );
	else
	if ( ! ( lplpOpenFileName = ( LPOPENFILENAME FAR* ) GlobalLock ( hOpenFile ) ) )
		return ( ( LPOPENFILENAME ) NULL );
	else
		return ( *lplpOpenFileName );
  
}
/*============================================================================*/
void WINAPI WGOFNUnlockOpenFileName (
	HWND                    hWnd )
  
{

	HGLOBAL                 hOpenFile = ( HGLOBAL )NULL;

  
	if ((hOpenFile = GetProp ( hWnd, WGOFNGetPropertyName ( WGOFNPROP_OPENFILENAME ) ) ) )
		GlobalUnlock ( hOpenFile );
  
}

//LPSTR strpbrkr(LPSTR, LPSTR);

void WINAPI WGOFNSplitPathAndFile ( 
	LPSTR                   PathAndFile, 
	LPSTR                   Path, 
	LPSTR                   File )
	
{
 
	LPSTR                   lpFile = (LPSTR)strpbrkr ( PathAndFile, 
							     WGOFNCharSet_PathSeparators );
	
	if ( ! lpFile ) 
		_fstrcpy ( File, PathAndFile );
	else
	{
		_fstrncpy ( Path, PathAndFile, lpFile - PathAndFile + 1 );
		_fstrcpy ( File, lpFile + 1 );
	}

}       
/*============================================================================*/
static void WINAPI CenterWindowInScreen (
	HWND                    hWnd )
	
{

	RECT                    Rect;
	WORD                    Height, Width;
	WORD                    X, Y;


	GetWindowRect ( hWnd, &Rect );
	Height = Rect.bottom - Rect.top;
	Width  = Rect.right - Rect.left;
	X      = MulDiv ( ( ( WORD )GetSystemMetrics ( SM_CXSCREEN ) - Width ), 1, 2 );
	Y      = MulDiv ( ( ( WORD )GetSystemMetrics ( SM_CYSCREEN ) - Height ), 1, 2 );
	MoveWindow ( hWnd, X, Y, Width, Height, FALSE );

}
/*============================================================================*/
short WINAPI WGOFNFileExists (
	LPSTR                   lpFileName )
	
{

	OFSTRUCT                OfStruct;
	HFILE                   hFile;
			
			
	_fmemset ( &OfStruct, 0, sizeof ( OfStruct ) );                
	hFile = OpenFile ( lpFileName, &OfStruct, OF_EXIST );
	
	return ( hFile == HFILE_ERROR ? FALSE : TRUE );
	
}
/*============================================================================*/
short WINAPI WGOFNGetFileStatus (
	LPSTR                   lpPathAndFileName,
	struct stat*            lpFileStatus )
{

	return ( MFS_STAT ( lpPathAndFileName, lpFileStatus ) );
	
}
/*============================================================================*/
UINT WINAPI WGOFNRegisteredMessage ( 
	UINT FAR*               lpAtom,
	LPSTR                   lpString )
	
{

	if ( ! *lpAtom )
		*lpAtom = RegisterWindowMessage ( lpString );
		
	return ( *lpAtom );

}
/*============================================================================*/
UINT WINAPI WGOFNGetWindowMessage (
	WORD                    WindowMessage )

{

	switch ( WindowMessage )
	{
		default:
			return ( 0 );
			
		case WGOFN_WM_HELP:
			return ( WGOFNRegisteredMessage ( &WGOFNWMHelp, HELPMSGSTRING ) );

		case WGOFN_WM_FILEOK:
			return ( WGOFNRegisteredMessage ( &WGOFNWMFileOK, FILEOKSTRING ) );
			
		case WGOFN_WM_LBSELCHANGE:
			return ( WGOFNRegisteredMessage ( &WGOFNWM_LBSelChange, LBSELCHSTRING ) );
			
		case WGOFN_WM_SHAREVIOLATION:
			return ( WGOFNRegisteredMessage ( &WGOFNWM_ShareViolation, SHAREVISTRING ) );
			
	}
	
}

/*============================================================================*/
short WINAPI WGOFNCheckParameters (
	LPOPENFILENAME		lpOpenFileName,
	WORD              Style )
  
{

	short			ErrorCode = 0;
  
  
	if ( ! lpOpenFileName )
		ErrorCode = WGOFNERR_NULLPOINTER;
	else
/*----- This test does not work in binary mode so screw it
	if ( lpOpenFileName->lStructSize != sizeof ( OPENFILENAME ) )
		ErrorCode = WGOFNERR_STRUCTSIZE;
	else
-----*/
	if ( ( lpOpenFileName->Flags & OFN_SHOWHELP ) && 
	     ( ! lpOpenFileName->hwndOwner ) )
		ErrorCode = WGOFNERR_NULLWINDOW;
	else                                              
	if ( ( lpOpenFileName->Flags & OFN_ENABLETEMPLATEHANDLE ) && 
	     ( ! lpOpenFileName->hInstance ) )
		ErrorCode = WGOFNERR_NULLHANDLE;
	else
	if ( ( ! ( lpOpenFileName->Flags & OFN_ENABLETEMPLATEHANDLE ) ) && 
	     ( lpOpenFileName->Flags & OFN_ENABLETEMPLATE ) && 
	     ( ! lpOpenFileName->lpTemplateName ) )
		ErrorCode = WGOFNERR_NULLTEMPLATE;
	else
	if ( ( lpOpenFileName->Flags & OFN_ENABLEHOOK ) && 
	     ( ! lpOpenFileName->lpfnHook ) )
		ErrorCode = WGOFNERR_NULLFUNCTION;
	else
	{
		if ( lpOpenFileName->Flags & OFN_CREATEPROMPT )
			lpOpenFileName->Flags |= OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
		if ( ( Style == WGOFN_OPEN ) && ( lpOpenFileName->Flags & OFN_OVERWRITEPROMPT ) )
			lpOpenFileName->Flags &= ~OFN_OVERWRITEPROMPT;
	}
	  
	return ( ErrorCode );
  
}
/*============================================================================*/
short WINAPI WGOFNSetTextDirectory (
	HWND			hWnd, 
	LPSTR			lpPath )
	
{

	char                    DisplayPath [ MAX_FILELENGTH ];
	char                    WorkingPath [ MAX_FILELENGTH ];
	LPSTR                   lpPartialPath = ( LPSTR )NULL;
	LPSTR                   lpPartialDisplayPath = ( LPSTR )NULL;
	RECT                    DisplayRect;
	WORD                    DisplayWidth;
	HWND                    hTextWnd = GetDlgItem ( hWnd, 
	                                                WGOFNTextDirectory );
	HWND                    hDirListWnd = GetDlgItem ( hWnd, 
	                                                   WGOFNListDirectories );
	HWND                    hDriveComboWnd = GetDlgItem ( hWnd,
	                                                      WGOFNComboDrives );
	HDC                     hDC      = ( HDC )NULL;
	DWORD                   TextExtent;
	DWORD                   DirCount;
	WORD                    DirIndex;
	WORD                    MaxDirCount;
	DWORD                   ReturnValue;
	DWORD                   DriveItemData = 0;
	WORD                    DriveSelection;
	char                    DrivePath [ MAX_FILELENGTH ];
	LPSTR                   lpDrivePath;
	short                   ErrorCode;
	

	memset ( DisplayPath, 0, sizeof ( DisplayPath ) );
	memset ( WorkingPath, 0, sizeof ( WorkingPath ) );
	memset ( DrivePath, 0, sizeof ( DrivePath ) );
	DriveSelection = MFS_GETDRIVE ();
	if ( ( ReturnValue = SendMessage ( hDriveComboWnd, CB_GETCURSEL, 
	                                   0, 0 ) ) == (DWORD)CB_ERR )
		ErrorCode = WGOFNERR_GETCURSEL;
	else
	{
		DriveSelection = ( WORD ) ReturnValue;
		if ( ( ReturnValue = SendMessage ( hDriveComboWnd, CB_GETLBTEXT,
		                                   DriveSelection, 
		                                   ( LPARAM )DrivePath ) ) == (DWORD)CB_ERR )
			ErrorCode = WGOFNERR_GETLBTEXT;
		else
		if ( ( DriveItemData = SendMessage ( hDriveComboWnd, CB_GETITEMDATA,
		                                     DriveSelection, 0 ) ) == CB_ERR )
			ErrorCode = WGOFNERR_GETITEMDATA;
	}
	MFS_GETCWD ( 0, DisplayPath, sizeof ( DisplayPath ) );
	if ( lpPath )
	{
		if ((lpDrivePath = _fstrpbrk ( lpPath, WGOFNCharSet_DriveSeparator ) ) )
		{
			DriveSelection = ( WORD )( *( lpDrivePath - 1 ) - 'A' ) + 1;
			MFS_GETDRIVEMAP ( DriveSelection, WorkingPath, sizeof ( WorkingPath ) );
			wsprintf ( DisplayPath, "%c: %s", *( lpDrivePath - 1 ), WorkingPath );
			DriveSelection = ( WORD ) SendMessage ( hDriveComboWnd, 
			                                        CB_FINDSTRINGEXACT,
			                                        0, ( LPARAM ) DisplayPath );
			SendMessage ( hDriveComboWnd, CB_SETCURSEL, DriveSelection, 0 );
		}
		MFS_CHDIR ( lpPath );
		MFS_GETCWD ( 0, DisplayPath, sizeof ( DisplayPath ) );
	}
	DlgDirList ( hWnd, DisplayPath, WGOFNListDirectories, ( int ) NULL,
	             DDL_DIRECTORY | DDL_EXCLUSIVE );
	if ( ( ReturnValue = SendMessage ( hDirListWnd, LB_FINDSTRINGEXACT, 0, ( LPARAM )( "[..]" ) ) ) != LB_ERR )
		SendMessage ( hDirListWnd, LB_DELETESTRING, ( WPARAM ) ReturnValue, 0 );
	if ( ( DirCount = SendMessage ( hDirListWnd, LB_GETCOUNT, 0, 0 ) ) != CB_ERR )
		for ( DirIndex = 0; DirIndex < ( WORD ) DirCount; DirIndex++ )
		{
			SendMessage ( hDirListWnd, LB_GETTEXT, DirIndex, ( LPARAM )WorkingPath );
			if ( lstrlen ( WorkingPath ) )
				WorkingPath [ lstrlen ( WorkingPath ) - 1 ] = '\0';
			SendMessage ( hDirListWnd, LB_DELETESTRING, DirIndex, 0 );
			SendMessage ( hDirListWnd, LB_INSERTSTRING, DirIndex, 
			              ( LPARAM )WorkingPath + 1 );
		}
	if ( ( DirCount = SendMessage ( hDirListWnd, LB_GETCOUNT, 0, 0 ) ) != CB_ERR )
		for ( DirIndex = 0; DirIndex < ( WORD ) DirCount; DirIndex++ )
			SendMessage ( hDirListWnd, LB_SETITEMDATA, DirIndex, -2 );
	SetWindowText ( hTextWnd, DisplayPath );

	MaxDirCount = 0;
	lstrcpy ( WorkingPath, DisplayPath );
	if ( DriveItemData )
	{
		SendMessage ( hDirListWnd, LB_INSERTSTRING, 0, 
		              ( LPARAM )( DrivePath + 3 ) );
		MaxDirCount   = 1;
		lpPartialPath = WorkingPath + lstrlen ( DrivePath ) - 3;
	}
	else
		lpPartialPath = _fstrpbrk ( WorkingPath, WGOFNCharSet_PathSeparators );

	if ( ( *lpPartialPath ==  WChar_PathSeparator ) || ( *lpPartialPath == WChar_SecondaryPathSeparator ) )
	  lpPartialPath++;

	while ((lpPartialPath = _fstrpbrk ( lpPartialDisplayPath = lpPartialPath,
	                                   WGOFNCharSet_PathSeparators ) ))
	{
		*lpPartialPath = '\0';
		lpPartialPath++;
		SendMessage ( hDirListWnd, LB_INSERTSTRING, ( WPARAM )MaxDirCount, 
                 ( LPARAM )( lpPartialDisplayPath ) );
		SendMessage ( hDirListWnd, LB_SETITEMDATA, ( WPARAM )MaxDirCount, 
		              MaxDirCount );
		MaxDirCount++;
	}
	if ( ( ! lpPartialDisplayPath ) || ( ! *lpPartialDisplayPath ) )
		MaxDirCount--;
	else
		DirCount = ( WORD ) SendMessage ( hDirListWnd, 
		                                  LB_INSERTSTRING, 
		                                  ( WPARAM )MaxDirCount,
		                                  ( LPARAM )( lpPartialDisplayPath ) );
	SendMessage ( hDirListWnd, LB_SETITEMDATA, ( WPARAM ) MaxDirCount,
	              MAKELONG ( MaxDirCount, 1 ) );
	/*--- the following two lines make sure the current dir is visible ---*/
	SendMessage ( hDirListWnd, LB_SETCURSEL, ( WPARAM )MaxDirCount, 0 );
	SendMessage ( hDirListWnd, LB_SETCURSEL, -1, 0 );
	MaxDirCount++;

	/*--- Set the item data for closed directories ---*/
	if ( ( DirCount = SendMessage ( hDirListWnd, LB_GETCOUNT, 0, 0 ) ) != CB_ERR )
		for ( DirIndex = 0; DirIndex < ( WORD ) DirCount; DirIndex++ )
			if ( SendMessage ( hDirListWnd, LB_GETITEMDATA, DirIndex, 0 ) == -2 )
				SendMessage ( hDirListWnd, LB_SETITEMDATA, DirIndex, 
				              MAKELONG ( MaxDirCount, 2 ) );

	if ((hDC = GetDC ( hTextWnd ) ))
	{
		GetClientRect ( hTextWnd, &DisplayRect );
		DisplayWidth = DisplayRect.right - DisplayRect.left;
		lstrcpy ( WorkingPath, DisplayPath );
		do
		{
			TextExtent = GetTextExtent ( hDC, WorkingPath, 
		                                     lstrlen ( WorkingPath ) );
			if ( LOWORD ( TextExtent ) > DisplayWidth )
			{
				if ( lpPartialPath )
					lpPartialPath = ( LPSTR )_fstrpbrk ( lpPartialPath, 
					                                     WGOFNCharSet_PathSeparators );
				else
				{
					lpPartialPath = ( LPSTR )_fstrpbrk ( WorkingPath, 
					                                     WGOFNCharSet_PathSeparators );
					if ((lpPartialDisplayPath = ( LPSTR )_fstrpbrk ( DisplayPath, 
					                                                 WGOFNCharSet_PathSeparators ) ))
					{
						lpPartialDisplayPath++;
						*lpPartialDisplayPath++ = '.';
						*lpPartialDisplayPath++ = '.';
						*lpPartialDisplayPath++ = '.';
					}
				}
				if ( lpPartialPath )
				{
					lpPartialPath++;
					lstrcpy ( lpPartialDisplayPath, lpPartialPath );
				}
			}
		} while ( ( lpPartialPath ) && ( LOWORD ( TextExtent ) > DisplayWidth ) );
		SetWindowText ( GetDlgItem ( hWnd, WGOFNTextDirectory ), DisplayPath );
	}

	if ( hDC )
		ReleaseDC ( hTextWnd, hDC );
				             
	return ( 0 );

}
/*============================================================================*/
short WINAPI WGOFNGetCurrentFilter (
	HWND			hWnd,
	LPOPENFILENAME		lpOpenFileName,
	LPSTR			lpTargetFilter )

{

	LPSTR			lpFilter;

	short			ErrorCode = 0;
	LRESULT			ReturnCode;
                                           
	
	if ((lpFilter = ( LPSTR )lpOpenFileName->lpstrFilter ))
		if ( ( ReturnCode = SendMessage ( GetDlgItem ( hWnd, WGOFNComboFilters ), 
		                                  CB_GETCURSEL, ( WPARAM )NULL, 
		                                  ( LPARAM )NULL ) ) == CB_ERR )
			ErrorCode = WGOFNERR_CBGETCURSEL;
		else
		{
			while ( ReturnCode )
			{
				lpFilter += lstrlen ( lpFilter ) + 1;
				if ( *lpFilter )
					lpFilter += lstrlen ( lpFilter ) + 1;
				ReturnCode--;
			}
			lpFilter += lstrlen ( lpFilter ) + 1;
			lstrcpy ( lpTargetFilter, lpFilter );
		}
		
	return ( ErrorCode );


}
/*============================================================================*/
short WINAPI WGOFNSetEditFileName (
	HWND                    hWnd, 
	LPSTR                   lpFileName,
	BOOL                    UpdateFileList )
	
{

	char                    Path [ MAX_FILELENGTH ];
	char                    File [ MAX_FILELENGTH ];
	char                    PathAndFile [ MAX_FILELENGTH ];
	char                    Filter [ MAX_FILELENGTH ];
	LPOPENFILENAME          lpOpenFileName = WGOFNLockOpenFileName ( hWnd );
	
	short                   ErrorCode = 0;
	
	
	_fmemset ( &Path, 0, sizeof ( Path ) );
	_fmemset ( &File, 0, sizeof ( File ) );
	_fmemset ( &PathAndFile, 0, sizeof ( PathAndFile ) );

	if ( lpFileName )
		SetWindowText ( GetDlgItem ( hWnd, WGOFNEditFileName ), lpFileName );

	if ( ! GetWindowText ( GetDlgItem ( hWnd, WGOFNEditFileName ), 
	                       PathAndFile, sizeof ( PathAndFile ) ) )
		ErrorCode = WGOFNERR_BUFFERTOOSMALL;
	else        	                       
	if ( WGOFNIsADirectory ( PathAndFile ) )
		lstrcpy ( Path, PathAndFile );
	else 
		WGOFNSplitPathAndFile ( PathAndFile, Path, File );
	if ( Path [ 0 ] )
		WGOFNSetTextDirectory ( hWnd, Path );
	SetWindowText ( GetDlgItem ( hWnd, WGOFNEditFileName ), File );
	if ( UpdateFileList )
	{
		if ( ! lpFileName )
			SendMessage ( GetDlgItem ( hWnd, WGOFNListFiles ), 
			              LB_RESETCONTENT, 
			              ( WPARAM )NULL, ( LPARAM )NULL );

		if ( ( ! ErrorCode ) && ( lpOpenFileName ) )
		{
			if ( strpbrk ( PathAndFile, WGOFNCharSet_WildCards ) )
				lstrcpy ( Filter, PathAndFile );
			else
				ErrorCode = WGOFNGetCurrentFilter ( hWnd, 
		                                        lpOpenFileName, 
		                                        Filter );
			if ( ! ErrorCode )
				if ( IsDlgButtonChecked ( hWnd, WGOFNCheckReadOnly ) )
					DlgDirList ( hWnd, Filter, WGOFNListFiles, 
					             WGOFNTextDirectory,
					             DDL_READONLY | DDL_EXCLUSIVE );
				else
					DlgDirList ( hWnd, Filter, WGOFNListFiles, 
					             WGOFNTextDirectory, 
					             DDL_READWRITE | DDL_EXCLUSIVE );
		}
	}

	if ( lpOpenFileName )
		WGOFNUnlockOpenFileName ( hWnd );

	return ( ErrorCode );

}
/*============================================================================*/
short WINAPI WGOFNRemoveItems ( 
	HWND              hComboWnd,
	DWORD             ItemData )

{

	LRESULT                      ReturnValue;
	WORD                         Item;

	short                        ErrorCode = 0;


	if ( ( ReturnValue = SendMessage ( hComboWnd, CB_GETCOUNT, 0, 
	                                   0 ) ) == CB_ERR )
		ErrorCode = WGOFNERR_GETSELCOUNT;
	else
	{
		for ( Item = ( WORD ) ReturnValue; Item; --Item )
		{
			if ( ( ReturnValue = SendMessage ( hComboWnd, CB_GETITEMDATA, 
			                                   Item - 1, 0 ) ) == CB_ERR )
				ErrorCode = WGOFNERR_GETITEMDATA;
			else
			if ( ( ( ItemData ) && ( ReturnValue ) ) ||
					 ( ( ! ItemData ) && ( ! ReturnValue ) ) )
				SendMessage ( hComboWnd, CB_DELETESTRING, Item - 1, 0 );
		}
		if ( ( ReturnValue = SendMessage ( hComboWnd, CB_GETCURSEL, 0, 
		                                   0 ) ) == CB_ERR )
		{
		}
	}
	return ( ErrorCode );

}
/*============================================================================*/
HBITMAP WINAPI WGOFNCopyBitmap (
	HDC                     hDC,
	HBITMAP                 hBitmap )

{
	
	HBITMAP			hOldBitmap;
	HBITMAP                 hCopyBitmap = 0, hOldCopyBitmap;
	HDC                     hCopyDC = CreateCompatibleDC ( hDC );
	HDC                     hMemDC = CreateCompatibleDC ( hDC );
	BITMAP                  Bitmap;

	GetObject ( hBitmap, sizeof ( BITMAP ), &Bitmap );
	hCopyBitmap = CreateBitmapIndirect ( &Bitmap );
	hOldCopyBitmap = SelectObject ( hCopyDC, hCopyBitmap );
	hOldBitmap = SelectObject ( hMemDC, hBitmap );
	BitBlt ( hCopyDC, 0, 0, Bitmap.bmWidth, Bitmap.bmHeight, hMemDC, 0, 0, SRCCOPY );
	SelectObject ( hCopyDC, hOldCopyBitmap );
	SelectObject ( hMemDC, hOldBitmap );
	DeleteDC ( hCopyDC );
	DeleteDC ( hMemDC );

	return ( hCopyBitmap );

}

/*============================================================================*/
short WINAPI WGOFNGetBitmap (
	HDC                    hDC,
	WORD                   BitmapID,
	COLORREF               ColorBackground,
	COLORREF               ColorHighlight,
	HBITMAP FAR*           lphBitmap,
	HBITMAP FAR*           lphHBitmap )

{
	HDC                    hMemDC = CreateCompatibleDC ( hDC );
	HDC                    hHighDC = CreateCompatibleDC ( hDC );
	HDC                    hMaskDC = CreateCompatibleDC ( hDC );
	HBITMAP                hMaskBitmap, hOldMaskBitmap;
	BITMAP                 Bitmap;
	HBITMAP                hBitmap = 0, hOldBitmap;
	HBITMAP                hHBitmap = 0, hOldHBitmap;
	HBITMAP                hResBitmap = 0;
	HBRUSH		       hMemBrush, hOldMemBrush;
	HBRUSH		       hHighBrush, hOldHighBrush;

	if ((hResBitmap = LoadBitmap ( GetInstance (), MAKEINTRESOURCE ( BitmapID ) ) ))
	{
		hBitmap = WGOFNCopyBitmap ( hDC, hResBitmap );
		hHBitmap = WGOFNCopyBitmap ( hDC, hResBitmap );
		GetObject ( hBitmap, sizeof ( BITMAP ), &Bitmap );
		hOldBitmap = SelectObject ( hMemDC, hBitmap );
		hOldHBitmap = SelectObject ( hHighDC, hHBitmap );
		if (( hMaskBitmap = CreateBitmap ( Bitmap.bmWidth, Bitmap.bmHeight, 1, 1, 0 ) ))
		{
			hOldMaskBitmap = SelectObject ( hMaskDC, hMaskBitmap );
			SetBkColor ( hMemDC, RGB ( 255, 255, 255 ) );
			SetBkColor ( hHighDC, RGB ( 255, 255, 255 ) );
			/* initialize mask bitmap */
			if (!BitBlt ( hMaskDC, 0, 0,
				Bitmap.bmWidth, Bitmap.bmHeight,
				hMemDC, 0, 0, SRCCOPY ))
			{
				ERRSTR((LF_ERROR, "WGOFNGetBitmap:"
					" BitBlt SRCCOPY failure!\n"));
			}
			if (!BitBlt ( hMaskDC, 0, 0,
				Bitmap.bmWidth, Bitmap.bmHeight,
				(HDC) 0, 0, 0, DSTINVERT ))
			{
				ERRSTR((LF_ERROR, "WGOFNGetBitmap:"
					" BitBlt DSTINVERT failure!\n"));
			}
#if 1
			/* initialize background image bitmap
			 *
			 * ROP calculation
			 * ---------------
			 * P  S  D  Result
			 * 0  0  0  0 (P)
			 * 0  0  1  0 (P)
			 * 0  1  0  0 (D)
			 * 0  1  1  1 (D)
			 * 1  0  0  1 (P)
			 * 1  0  1  1 (P)
			 * 1  1  0  0 (D)
			 * 1  1  1  1 (D)
			 *
			 * If source (mask) bit = 0, dest = pattern (bg color).
			 * If source (mask) bit = 1, dest = dest (unchanged).
			 */
			hMemBrush = CreateSolidBrush(ColorBackground);
			hOldMemBrush = SelectObject(hMemDC, hMemBrush);
			if (!BitBlt ( hMemDC, 0, 0,
				Bitmap.bmWidth, Bitmap.bmHeight,
				hMaskDC, 0, 0, 0x00b8074aL ))
			{
				ERRSTR((LF_ERROR, "WGOFNGetBitmap:"
					" BitBlt init bg failure!\n"));
			}
			SelectObject(hMemDC, hOldMemBrush);
			DeleteObject(hMemBrush);
			/* initialize highlight image bitmap
			 *
			 * ROP calculation
			 * ---------------
			 * P  S  D  Result
			 * 0  0  0  0 (P)
			 * 0  0  1  0 (P)
			 * 0  1  0  0 (D)
			 * 0  1  1  1 (D)
			 * 1  0  0  1 (P)
			 * 1  0  1  1 (P)
			 * 1  1  0  0 (D)
			 * 1  1  1  1 (D)
			 *
			 * If source (mask) bit = 0, dest = pattern (hi color).
			 * If source (mask) bit = 1, dest = dest (unchanged).
			 */
			hHighBrush = CreateSolidBrush(ColorHighlight);
			hOldHighBrush = SelectObject(hHighDC, hHighBrush);
			if (!BitBlt ( hHighDC, 0, 0,
				Bitmap.bmWidth, Bitmap.bmHeight,
				hMaskDC, 0, 0, 0x00b8074aL ))
			{
				ERRSTR((LF_ERROR, "WGOFNGetBitmap:"
					" BitBlt init hi failure!\n"));
			}
			SelectObject(hHighDC, hOldHighBrush);
			DeleteObject(hHighBrush);
#else
			for ( x = 0; x < Bitmap.bmWidth; x++ )
				for ( y = 0; y < Bitmap.bmHeight; y++ )
				{
					if ( GetPixel ( hMaskDC, x, y ) == RGB( 0, 0, 0 ) )
					{
						SetPixel ( hMemDC, x, y, ColorBackground );
						SetPixel ( hHighDC, x, y, ColorHighlight );
					}
				}
#endif
			SelectObject ( hMaskDC, hOldMaskBitmap );
			DeleteObject ( hMaskBitmap );
		}
		SelectObject( hMemDC, hOldBitmap);
		SelectObject( hHighDC, hOldHBitmap);
	}

	if ( hResBitmap )
		DeleteObject ( hResBitmap );

	DeleteDC( hMemDC );
	DeleteDC( hHighDC );
	DeleteDC( hMaskDC );

	*lphBitmap  = hBitmap;
	*lphHBitmap = hHBitmap;

	return ( 0 );

}
/*============================================================================*/
short WINAPI WGOFNInitControls (
	HWND                     hWnd,
	LPOPENFILENAME           lpOpenFileName )

{

	char                     TwinString [ 64 ];
	char                     Path [ MAX_FILELENGTH ];

	MFS_GETCWD ( 0, Path, sizeof ( Path ) );
	DlgDirListComboBox ( hWnd, Path, WGOFNComboDrives, ( int )NULL, 
	                     DDL_DRIVES | DDL_EXCLUSIVE );
	GetTwinString ( WCP_PATHSASDRIVES, TwinString, sizeof ( TwinString ) );
	if ( ! stricmp ( TwinString, "no" ) )
	{
		WGOFNRemoveItems ( GetDlgItem ( hWnd, WGOFNComboDrives ), FALSE );
	}
	GetTwinString ( WCP_DOSDRIVES, TwinString, sizeof ( TwinString ) );
	if ( ! stricmp ( TwinString, "no" ) )
	{
		WGOFNRemoveItems ( GetDlgItem ( hWnd, WGOFNComboDrives ), TRUE );
	}
	if ( ( lpOpenFileName->lpstrFile ) && ( *lpOpenFileName->lpstrFile ) )
		WGOFNSetEditFileName ( hWnd, lpOpenFileName->lpstrFile,
		                       TRUE );
	if ( ( lpOpenFileName->lpstrInitialDir ) && ( *lpOpenFileName->lpstrInitialDir ) &&
	     ( WGOFNIsADirectory ( ( LPSTR )lpOpenFileName->lpstrInitialDir ) ) )
		WGOFNSetTextDirectory ( hWnd, ( LPSTR )lpOpenFileName->lpstrInitialDir );
	else
		WGOFNSetTextDirectory ( hWnd, ( LPSTR )NULL );
	WGOFNSetEditFileName ( hWnd, NULL, TRUE );

	return ( 0 );

}
/*============================================================================*/
short WINAPI WGFONInitDialog (
	HWND			hWnd,
	LPOPENFILENAME		lpOpenFileName )
    
{

	HGLOBAL			hOpenFile     = ( HGLOBAL )NULL;
	LPOPENFILENAME FAR*	lplpOpenFile  = ( LPOPENFILENAME FAR* )NULL;
	LPSTR			lpFilter;
	LPSTR			lpFilterToUse = NULL;
	WORD			FilterCount;
	HWND			hFilterWnd    = GetDlgItem ( hWnd, WGOFNComboFilters );
	char			Path [ MAX_FILELENGTH ];
	HGLOBAL			hInitDir = (HGLOBAL)NULL;
	LPSTR			lpInitDir = NULL;
  
	short			ErrorCode  = 0;
  

	CenterWindowInScreen ( hWnd );
	if ( ! ( hOpenFile = GlobalAlloc ( GHND, sizeof ( LPOPENFILENAME ) ) ) )
		ErrorCode = WGOFNERR_GLOBALALLOC;
	else
	if ( ! ( lplpOpenFile = ( LPOPENFILENAME FAR* )GlobalLock ( hOpenFile ) ) )
		ErrorCode = WGOFNERR_GLOBALLOCK;
	else
	if ( ! SetProp ( hWnd, WGOFNGetPropertyName ( WGOFNPROP_OPENFILENAME ), hOpenFile ) )
		ErrorCode = WGOFNERR_SETPROP;
	else
	{
		*lplpOpenFile = lpOpenFileName;
		if ( lpOpenFileName->Flags & OFN_HIDEREADONLY )
			ShowWindow ( GetDlgItem ( hWnd, WGOFNCheckReadOnly ), SW_HIDE );
		if ( lpOpenFileName->Flags & OFN_READONLY )
			CheckDlgButton ( hWnd, WGOFNCheckReadOnly, TRUE );
		ShowWindow ( GetDlgItem ( hWnd, WGOFNButtonHelp ), 
		             lpOpenFileName->Flags & OFN_SHOWHELP ? SW_SHOW : SW_HIDE );
		MFS_GETCWD ( 0, Path, sizeof ( Path ) );
		WGOFNInitControls ( hWnd, lpOpenFileName );
		if ((lpFilter = ( LPSTR )lpOpenFileName->lpstrFilter ))
		{
			FilterCount = 0;
			while ( *lpFilter )
			{
				FilterCount++;
				SendMessage ( hFilterWnd, CB_ADDSTRING, 
				              ( WPARAM )NULL, ( LPARAM )lpFilter );
				lpFilter += lstrlen ( lpFilter ) + 1;
				if ( ( ! lpFilterToUse ) && 
				     ( ( ! lpOpenFileName->nFilterIndex ) || 
				       ( FilterCount == lpOpenFileName->nFilterIndex ) ) )
					lpFilterToUse = lpFilter;
				if ( *lpFilter )
					lpFilter += lstrlen ( lpFilter ) + 1;
			}
			if ( lpOpenFileName->nFilterIndex )
				SendMessage ( hFilterWnd, CB_SETCURSEL, 
				              ( WPARAM )( lpOpenFileName->nFilterIndex - 1 ), 
				              ( LPARAM ) NULL );
			else
				SendMessage ( hFilterWnd, CB_SETCURSEL, 
				              ( WPARAM ) NULL,
				              ( LPARAM ) NULL );
			if ( lpFilterToUse )
				WGOFNSetEditFileName ( hWnd, lpFilterToUse, TRUE );
		}
		if ( ( lpOpenFileName->lpstrFile ) && ( *lpOpenFileName->lpstrFile ) )
			WGOFNSetEditFileName ( hWnd, lpOpenFileName->lpstrFile, TRUE );
		if ( lpOpenFileName->lpstrTitle )
			SetWindowText ( hWnd, lpOpenFileName->lpstrTitle );
		if ( lpOpenFileName->Flags & OFN_NOCHANGEDIR )
		{
			if ( ! ( hInitDir = GlobalAlloc ( GHND, lstrlen ( Path ) + 1 ) ) )
				ErrorCode = WGOFNERR_GLOBALALLOC;
			else
			if ( ! ( lpInitDir = ( LPSTR ) GlobalLock ( hInitDir ) ) )
				ErrorCode = WGOFNERR_GLOBALLOCK;
			else
			{
				lstrcpy ( lpInitDir, Path );
				if ( ! SetProp ( hWnd, WGOFNGetPropertyName ( WGOFNPROP_INITIALDIR ), hInitDir ) )
					ErrorCode = WGOFNERR_SETPROP;
			}
		}
	}

	if ( lplpOpenFile )
		GlobalUnlock ( hOpenFile );
		
	if ( lpInitDir )
		GlobalUnlock ( hInitDir );

	if ( ( ErrorCode ) && ( hOpenFile ) )
		GlobalFree ( hOpenFile );
		
	if ( ( ErrorCode ) && ( hInitDir ) )
		GlobalFree ( hInitDir );
    
	return ( ErrorCode );
  
}
/*============================================================================*/
short WINAPI WGFONEndDialog (
	HWND                            hWnd )
  
{

	HGLOBAL                         hProp = ( HGLOBAL )NULL;
	LPSTR                           lpInitDir = ( LPSTR )NULL;
  
	short                           ErrorCode = 0;
  
  
	if ((hProp = GetProp ( hWnd, WGOFNGetPropertyName ( WGOFNPROP_OPENFILENAME ) ) ) )
	{
		GlobalFree ( hProp );
		RemoveProp ( hWnd, WGOFNGetPropertyName ( WGOFNPROP_OPENFILENAME ) );
	}
	
	if ((hProp = GetProp ( hWnd, WGOFNGetPropertyName ( WGOFNPROP_INITIALDIR ) ) ) )
	{
		if ( ! ( lpInitDir = ( LPSTR ) GlobalLock ( hProp ) ) )
			ErrorCode = WGOFNERR_GLOBALLOCK;
		else
		{
			MFS_CHDIR ( lpInitDir );
			GlobalUnlock ( hProp );
		}
		GlobalFree ( hProp );
		RemoveProp ( hWnd, WGOFNGetPropertyName ( WGOFNPROP_INITIALDIR ) );
	}
   
	return ( ErrorCode );
  
}
/*============================================================================*/
short WINAPI WGOFNComboFiltersSelectionChange ( 
	HWND			hWnd, 
	HWND			hControlWnd,
	LPOPENFILENAME		lpOpenFileName )

{

	char			Filter [ MAX_FILELENGTH ];

	short			ErrorCode = 0;
                                           
	
	if ( ! ( ErrorCode = WGOFNGetCurrentFilter ( hWnd, lpOpenFileName, 
	                                             Filter ) ) )
		WGOFNSetEditFileName ( hWnd, Filter, TRUE );
		
	return ( ErrorCode );

}	
/*============================================================================*/
short WINAPI WGOFNCheckOK ( 
	HWND			hWnd,
	LPOPENFILENAME		lpOpenFileName )

{

	char			FileName [ MAX_FILELENGTH ];
	char			Path [ MAX_FILELENGTH ];
	char			PathAndFileName [ MAX_FILELENGTH ];
	char			Title [ MAX_FILELENGTH ];
	char			Message [ MAX_FILELENGTH ];
	LPSTR			lpExt;
	WORD			PathLength;
	WORD			FileLength;
	struct stat		FileStatus;
	
	short			ErrorCode = 0;
	

	GetWindowText ( GetDlgItem ( hWnd, WGOFNEditFileName ), FileName, 
	                sizeof ( FileName ) );
	if ( strpbrk ( FileName, WGOFNCharSet_WildCards ) )
	{
		WGOFNSetEditFileName ( hWnd, NULL, TRUE );
		return ( TRUE );
	}

	if ((FileLength = lstrlen ( FileName ) ))
		if ( _fstrspn ( FileName + FileLength - 1, WGOFNCharSet_PathSeparators ) )
			FileName [ FileLength - 1 ] = '\0';
			
	if ( WGOFNIsADirectory ( FileName ) )
	{
		WGOFNSetTextDirectory ( hWnd, FileName );
		WGOFNComboFiltersSelectionChange ( hWnd, 
		                                   GetDlgItem ( hWnd, WGOFNComboFilters ), 
		                                   lpOpenFileName );
		return ( TRUE );
	}

	if ( ( lpOpenFileName->lpstrDefExt ) && 
	     ( ! ( lpOpenFileName->Flags & OFN_NOVALIDATE ) ) )
	{
		if ( ! strpbrk ( FileName, WGOFNCharSet_ExtentionSeparators ) )
		{
			lstrcat ( FileName, WGOFNCharSet_ExtentionSeparators );
			lstrcat ( FileName, lpOpenFileName->lpstrDefExt );
		}
		if ((lpExt = ( LPSTR )strpbrkr ( FileName, 
		                                 WGOFNCharSet_ExtentionSeparators ) ))
			lpOpenFileName->Flags |= ( ( lstrcmp ( lpExt + 1, 
			                                       lpOpenFileName->lpstrDefExt ) ) ? 
			                         OFN_EXTENSIONDIFFERENT : 0 );
	}
 
	MFS_GETCWD ( 0, Path, sizeof ( Path ) );
	PathLength = lstrlen ( Path );
	FileLength = lstrlen ( FileName );
	if ( ( ( PathLength ) && 
	       ( Path [ PathLength - 1 ] != WGOFNChar_PathSeparator ) && 
	       ( Path [ PathLength - 1 ] != WGOFNChar_PathSeparator ) ) &&
	     ( ( FileLength ) && 
	       ( FileName [ FileLength - 1 ] != WGOFNChar_SecondaryPathSeparator ) && 
	       ( FileName [ FileLength - 1 ] != WGOFNChar_SecondaryPathSeparator ) ) )
	{
		Path [ lstrlen ( Path ) + 1 ] = '\0';		       
		Path [ lstrlen ( Path ) ]     = WGOFNChar_PathSeparator;
	}
	wsprintf ( ( LPSTR ) PathAndFileName, "%s%s", ( LPSTR ) Path, 
	           ( LPSTR ) FileName );

	lpOpenFileName->nFileExtension = 0;
	lpOpenFileName->nFileOffset = 0;

	if ( lpOpenFileName->lpstrFile ) {
		if ( ( DWORD )lstrlen ( PathAndFileName ) + 1 > lpOpenFileName->nMaxFile )
		{
			ErrorCode = WGOFNERR_BUFFERTOOSMALL;
			if ( lpOpenFileName->nMaxFile >= sizeof ( DWORD ) )
				*( ( LPDWORD )( lpOpenFileName->lpstrFile ) ) = ( DWORD )( lstrlen ( PathAndFileName ) + 1 );
		}
		else
			lstrcpy ( lpOpenFileName->lpstrFile, PathAndFileName );
	}

	if ( lpOpenFileName->lpstrFileTitle ) {
		if ( ( DWORD )lstrlen ( FileName ) + 1 > lpOpenFileName->nMaxFileTitle )
			ErrorCode = WGOFNERR_BUFFERTOOSMALL;
		else
			lstrcpy ( lpOpenFileName->lpstrFileTitle, FileName );
	}

	if ((lpExt = ( LPSTR )strpbrkr ( PathAndFileName, 
	                                 WGOFNCharSet_ExtentionSeparators ) ))
		lpOpenFileName->nFileExtension = ( lpExt - ( LPSTR )PathAndFileName + 1);

	if ((lpExt = ( LPSTR )strpbrkr ( PathAndFileName, WGOFNCharSet_PathSeparators ) ))
		lpOpenFileName->nFileOffset = ( lpExt - ( LPSTR )PathAndFileName + 1);
	else
		lpOpenFileName->nFileOffset = (WORD)-1;
	
	if ( ( ! ErrorCode ) && ( ! ( lpOpenFileName->Flags & OFN_NOVALIDATE ) ) ) {
		if ( ! ( ErrorCode = GetFileTitle ( PathAndFileName, FileName, sizeof ( FileName ) ) ) )
		{
			GetWindowText ( hWnd, Title, sizeof ( Title ) );
			if ( lpOpenFileName->Flags & OFN_PATHMUSTEXIST )
				if ( ! WGOFNIsADirectory ( Path ) )
				{
					ErrorCode = WGOFNERR_DIRECTORYDOESNOTEXIST;
					wsprintf ( Message, 
					           "%s\nPath does not exist.\n\nPlease verify that the correct path is given.\n", 
					           ( LPSTR ) PathAndFileName );
					MessageBox ( hWnd, Message, Title, MB_OK | MB_ICONEXCLAMATION );
				}
			if ( ( ! ErrorCode ) && ( lpOpenFileName->Flags & OFN_FILEMUSTEXIST ) )
				if ( ! WGOFNFileExists ( PathAndFileName ) )
					if ( lpOpenFileName->Flags & OFN_CREATEPROMPT )
					{
						wsprintf ( Message, 
						           "%s\nThis file does not exist.\n\nCreate the file?\n",
						           ( LPSTR ) PathAndFileName );
						switch ( MessageBox ( hWnd, Message, Title, MB_YESNO | MB_ICONQUESTION ) )
						{
							default:
							case IDNO:
								return ( TRUE );
							
							case IDYES:
								break;
						}
					}
					else
					{
						wsprintf ( Message, 
						           "%s\nCannot find this file.\n\nPlease verify that the correct path and filename are given.\n",
						           ( LPSTR ) PathAndFileName );
						MessageBox ( hWnd, Message, Title, MB_OK | MB_ICONEXCLAMATION );
						ErrorCode = WGOFNERR_FILEDOESNOTEXIST;
					}
			if ( ( ! ErrorCode ) && ( lpOpenFileName->Flags & OFN_NOREADONLYRETURN ) &&
			     ( ! ( ErrorCode = WGOFNGetFileStatus ( PathAndFileName, &FileStatus ) ) ) )
			{
				ErrorCode = 0;
				if ( ( ! ( FileStatus.st_mode & S_IWRITE ) ) || ( ! ( FileStatus.st_mode & S_IEXEC ) ) )
				{
					ErrorCode = WGOFNERR_READONLY;
					wsprintf ( Message,
					           "%s\nThis file exists and is read-only.\nUse a different filename.",
					           ( LPSTR )PathAndFileName );
					MessageBox ( hWnd, Message, Title, MB_OK | MB_ICONEXCLAMATION );
				}
			}
			else
				ErrorCode = 0;
			if ( ( ! ErrorCode ) && ( lpOpenFileName->Flags & OFN_OVERWRITEPROMPT ) )
			{
				if ( WGOFNFileExists ( PathAndFileName ) )
				{
					wsprintf ( Message,
					           "%s\nThis file already exists.\n\nReplace existing file?",
					           ( LPSTR ) PathAndFileName );
					switch ( MessageBox ( hWnd, Message, Title, MB_YESNO | MB_ICONQUESTION ) )
					{
						default:
						case IDNO:
							return ( TRUE );
							
						case IDYES:
							break;
					}
				}
			}
		}
		else {
			if ( ErrorCode >= 0 )
				ErrorCode = WGOFNERR_BUFFERTOOSMALL;
			else
				ErrorCode = WGOFNERR_INVALIDFILENAME;
		}
	}

	return ( ErrorCode );
	
}
/*============================================================================*/
short WINAPI WGOFNEditFileNameLoseFocus ( 
	HWND			hWnd, 
	HWND			hControlWnd,
	LPOPENFILENAME		lpOpenFileName )
	
{
	
	short			ErrorCode = 0;
	
	
	ErrorCode = WGOFNSetEditFileName ( hWnd, NULL, 
		(BOOL)SendMessage(hControlWnd, EM_GETMODIFY, 0, 0) );
	
	return ( ErrorCode );
	
}
/*============================================================================*/
short WINAPI WGOFNListFilesDoubleClick ( 
	HWND			hWnd,
	HWND			hControlWnd,
	LPOPENFILENAME		lpOpenFileName )
	
{

	char			FileName [ MAX_FILELENGTH ]; 
	short			ErrorCode = 0;
	LRESULT			ReturnCode;
	
	
	if ( ( ReturnCode = SendMessage ( hControlWnd, LB_GETCURSEL, ( WPARAM )NULL, 
	                                  ( LPARAM )NULL ) ) == LB_ERR )
		ErrorCode = WGOFNERR_LBGETCURSEL;
	else
	if ( ( ReturnCode = SendMessage ( hControlWnd, LB_GETTEXT, ( WPARAM )ReturnCode, 
	                                  ( LPARAM )( ( LPVOID ) FileName ) ) ) == LB_ERR )
		ErrorCode = WGOFNERR_LBGETTEXT;
	else
	{
		SetWindowText ( GetDlgItem ( hWnd, WGOFNEditFileName ), FileName );
		if ( ! WGOFNEditFileNameLoseFocus ( hWnd, GetDlgItem ( hWnd, WGOFNEditFileName ), lpOpenFileName ) )
			PostMessage ( hWnd, WM_COMMAND,
				GET_WM_COMMAND_MPS(IDOK,
					GetDlgItem ( hWnd, IDOK ),
					BN_CLICKED ) );
	}
	
	return ( ErrorCode );

}	
/*============================================================================*/
short WINAPI WGOFNListFilesSelectionChange ( 
	HWND			hWnd,
	HWND			hControlWnd,
	LPOPENFILENAME		lpOpenFileName )
	
{

	char			FileName [ MAX_FILELENGTH ]; 
	char			SelectedFile [ MAX_FILELENGTH ];
	short			ErrorCode = 0;
	LRESULT			ReturnCode;
	HGLOBAL			hSelections = (HGLOBAL)NULL;
	LPWORD			lpSelections = NULL;
	WORD			Selection;
	WORD			NumSelections;
	

	if ( ( ReturnCode = SendMessage ( hControlWnd, LB_GETSELCOUNT, ( WPARAM )NULL, 
	                                  ( LPARAM )NULL ) ) == LB_ERR )
	{	                                  
		if ( ( ReturnCode = SendMessage ( hControlWnd, LB_GETCURSEL, ( WPARAM )NULL, 
		                                  ( LPARAM )NULL ) ) == LB_ERR )
			ErrorCode = WGOFNERR_LBGETCURSEL;
		else
		if ( ( ReturnCode = SendMessage ( hControlWnd, LB_GETTEXT, ( WPARAM )ReturnCode, 
		                                  ( LPARAM )( ( LPVOID ) FileName ) ) ) == LB_ERR )
			ErrorCode = WGOFNERR_LBGETTEXT;
	}
	else
	if ( ReturnCode > 1 ) 
	{	/*--- There are multiple selected files ---*/
		NumSelections = ( WORD )ReturnCode;
		if ( ! ( hSelections = GlobalAlloc ( GHND, NumSelections * sizeof ( WORD ) ) ) )
			ErrorCode = WGOFNERR_GLOBALALLOC;
		else
		if ( ! ( lpSelections = ( LPWORD ) GlobalLock ( hSelections ) ) )
			ErrorCode = WGOFNERR_GLOBALLOCK;
		else
		if ( ( ReturnCode = SendMessage ( hControlWnd, LB_GETSELITEMS, 
		                                  ( WPARAM )NumSelections, 
		                                  ( LPARAM )lpSelections ) ) == LB_ERR )
			ErrorCode = WGOFNERR_GETSELITEMS;
		else
		for ( Selection = 0, FileName [ 0 ] = '\0'; 
		      ( ! ErrorCode ) && ( Selection < NumSelections ); 
		      Selection++, lpSelections++ )
		{
			if ( ( ReturnCode = SendMessage ( hControlWnd, LB_GETTEXT, 
			                                  ( WPARAM )*lpSelections, 
			                                  ( LPARAM )( ( LPSTR ) SelectedFile ) ) ) == LB_ERR )
				ErrorCode = WGOFNERR_LBGETTEXT;
			else
			if ( lstrlen ( FileName ) + lstrlen ( SelectedFile ) + 2 > sizeof ( FileName ) )
				ErrorCode = WGOFNERR_BUFFERTOOSMALL;
			else
			{
				if ( FileName [ 0 ] )
					lstrcat ( FileName, " " );
				lstrcat ( FileName, SelectedFile );
			}
		}
		
	}

	if ( ! ErrorCode )
		WGOFNSetEditFileName ( hWnd, FileName, FALSE );
		
	if ( lpSelections )
		GlobalUnlock ( hSelections );
		
	if ( hSelections )
		GlobalFree ( hSelections );
	
	return ( ErrorCode );

}	
/*============================================================================*/
short WINAPI WGOFNComboDrivesSelectionChange ( 
	HWND			hWnd,
	HWND			hControlWnd,
	LPOPENFILENAME		lpOpenFileName )

{

	char                    CurrentDirectory [ MAX_FILELENGTH ];
	char                    NewDirectory [ MAX_FILELENGTH ];
	short                   ErrorCode = 0;
	WORD                    CurrentSelection = 0;
	LRESULT                 ReturnCode;
	WORD                    CurrentDrive;
	WORD                    Drive;
	WORD                    NumDrives;
	
	
	if ((ReturnCode = SendMessage(hControlWnd, CB_GETCURSEL, (WPARAM)NULL, 
	                                  ( LPARAM )NULL ) ) == CB_ERR )
	{
		if ( ! ( CurrentDrive = MFS_GETDRIVE () ) )
			CurrentDrive = MFS_GETROOTDRIVE ();
		if ( ( ReturnCode = SendMessage ( hControlWnd, CB_GETCOUNT, 0, 0 ) ) == CB_ERR )
			ErrorCode = WGOFNERR_GETCOUNT;
		else
		for ( Drive = 0, NumDrives = ( WORD )ReturnCode, ReturnCode = 0;
		      ( Drive < NumDrives ) && 
		        ( ReturnCode != CB_ERR ) && 
		        ( ReturnCode != CurrentDrive );
		      Drive++ )
			if ( ( ReturnCode = SendMessage ( hControlWnd, CB_GETITEMDATA, 
			                                   Drive, 0 ) ) == CurrentDrive )
				SendMessage ( hControlWnd, CB_SETCURSEL, CurrentSelection = Drive, 0 );	
	}
	else
		CurrentSelection = ( WORD ) ReturnCode;
	if ( ( ReturnCode = SendMessage ( hControlWnd, CB_GETITEMDATA, 
	                                  CurrentSelection, 
	                                  ( LPARAM )NULL ) ) == LB_ERR )
		ErrorCode = WGOFNERR_LBGETCURSEL;
	else
	if ( ReturnCode )
	{
		MFS_SETDRIVE ( ReturnCode );
		MFS_GETCWD ( 0, CurrentDirectory, sizeof ( CurrentDirectory ) );
		wsprintf ( NewDirectory, "%c:%s", 
		           ( char )( ReturnCode - 1 ) + 'A', 
		           CurrentDirectory );
		MFS_CHDIR ( NewDirectory );
#ifdef TWIN32
		switch ( GetDriveType16 ( ReturnCode - 1 ) )
#else
		switch ( GetDriveType ( ReturnCode - 1 ) )
#endif
		{
			default:
				WGOFNSetTextDirectory ( hWnd, NewDirectory );
				break;

			case DRIVE_REMOVABLE:
				WGOFNSetTextDirectory ( hWnd, ( LPSTR )NULL );
				break;
		}
		WGOFNSetEditFileName ( hWnd, ( LPSTR ) NULL, TRUE );
	}
	else
	{
		SendMessage ( hControlWnd, CB_GETLBTEXT, CurrentSelection,
	              	( LPARAM )CurrentDirectory );
		MFS_CHDIR ( CurrentDirectory );
		WGOFNSetTextDirectory ( hWnd, CurrentDirectory );
		WGOFNSetEditFileName ( hWnd, ( LPSTR ) NULL, TRUE );
	}

	return ( ErrorCode );

}
/*============================================================================*/
short WINAPI WGOFNListDirectoriesDoubleClick ( 
	HWND                    hWnd,
	HWND                    hControlWnd,
	LPOPENFILENAME          lpOpenFileName )
	
{

	char                    DirectoryName [ MAX_FILELENGTH ]; 
	char                    CurrentDirectory [ MAX_FILELENGTH ];
	char                    Path [ MAX_FILELENGTH ];
	char                    DrivePath [ MAX_FILELENGTH ];
	HWND                    hDriveWnd = GetDlgItem ( hWnd, WGOFNComboDrives );
	short                   ErrorCode = 0;
	WORD                    SelectedItem;
	WORD                    SelectedDrive;
	WORD                    Level;
	WORD                    Drives;
	WORD                    RootDrive;
	DWORD                   DriveItemData;
	DWORD                   DirItemData;
	LPSTR                   lpDir;
	LRESULT                 ReturnCode;
	
	
	if ( ( ReturnCode = SendMessage ( hControlWnd, LB_GETCURSEL, ( WPARAM )NULL, 
	                                  ( LPARAM )NULL ) ) == LB_ERR )
		ErrorCode = WGOFNERR_LBGETCURSEL;
	else
	if ( ( ReturnCode = SendMessage ( hControlWnd, LB_GETTEXT, 
	                                  SelectedItem = ( WORD )ReturnCode, 
	                                  ( LPARAM )( DirectoryName ) ) ) == LB_ERR )
		ErrorCode = WGOFNERR_LBGETTEXT;
	else
	if ( ( DirItemData = SendMessage ( hControlWnd, LB_GETITEMDATA,
	                                   SelectedItem, 0 ) ) == LB_ERR )
		ErrorCode = WGOFNERR_GETITEMDATA;
	else
	if ( ( ReturnCode = SendMessage ( hDriveWnd, CB_GETCURSEL, ( WPARAM )NULL,
	                                  ( LPARAM )NULL ) ) == CB_ERR )
		ErrorCode = WGOFNERR_GETCURSEL;
	else
	if ( ( ReturnCode = SendMessage ( hDriveWnd, CB_GETLBTEXT,
	                                  SelectedDrive = ( WORD )ReturnCode,
	                                  ( LPARAM )( DrivePath ) ) ) == CB_ERR )
		ErrorCode = WGOFNERR_GETLBTEXT;
	else
	if ( ( DriveItemData = SendMessage ( hDriveWnd, CB_GETITEMDATA,
	                                     SelectedDrive, 
	                                     ( LPARAM )NULL ) ) == CB_ERR )
		ErrorCode = WGOFNERR_GETITEMDATA;
	else
	{
		memset ( CurrentDirectory, 0, sizeof ( CurrentDirectory ) );
		if ( DriveItemData )
			wsprintf ( CurrentDirectory, "%c:", ( char )( DriveItemData - 1 ) + 'A' );
		for ( Level = DriveItemData ? 1 : 0 ; Level < LOWORD ( DirItemData ); 
		      Level++ )
		{
			SendMessage ( hControlWnd, LB_GETTEXT, Level, ( LPARAM )Path );
			CurrentDirectory [ lstrlen ( CurrentDirectory ) ] = WChar_PathSeparator;
			lstrcat ( CurrentDirectory, Path );
		}
		CurrentDirectory [ lstrlen ( CurrentDirectory ) ] = WChar_PathSeparator;
		lstrcat ( CurrentDirectory, DirectoryName );
		MFS_CHDIR ( CurrentDirectory );
		MFS_GETCWD ( 0, DirectoryName, sizeof ( DirectoryName ) );
		if ( ( ( DriveItemData ) && 
		       ( strncmp ( DrivePath + 3, DirectoryName, strlen ( DrivePath ) - 3 ) ) ) ||
		     ( ( !DriveItemData ) &&
		       ( strncmp ( DrivePath, DirectoryName, strlen ( DrivePath ) ) ) ) )
		{
			if ((lpDir = ( LPSTR ) strpbrkr ( CurrentDirectory, 
			                                  WCharSet_PathSeparators ) ))
				lpDir [ 1 ] = '\0';
			MFS_CHDIR ( CurrentDirectory );
			RootDrive = MFS_GETROOTDRIVE();
			wsprintf ( CurrentDirectory, "%c%c%s", 
			           ( char )( RootDrive - 1 ) + 'A', 
			           *WCharSet_DriveSeparator, DirectoryName );
			MFS_SETDRIVE ( RootDrive );
			if ( ( ReturnCode = SendMessage ( hDriveWnd, CB_GETCOUNT, 0, 0 ) ) != CB_ERR )
			{
				for ( Level = 0, Drives = ( WORD ) ReturnCode, ReturnCode = 0; 
				      ( Level < Drives ) && ( ( WORD )ReturnCode != RootDrive );
				      Level++ )
				{
					ReturnCode = SendMessage ( hDriveWnd, CB_GETITEMDATA, Level, 0 );
					if ( ( WORD ) ReturnCode == RootDrive )
					{
						SendMessage ( hDriveWnd, CB_SETCURSEL, Level, 0 );
						WGOFNComboDrivesSelectionChange ( hWnd, hDriveWnd, lpOpenFileName );
					}
				}
			}
			MFS_CHDIR ( CurrentDirectory );
		}
		WGOFNSetTextDirectory ( hWnd, ( LPSTR ) NULL );
		WGOFNSetEditFileName ( hWnd, ( LPSTR ) NULL, TRUE );
	}
	
	return ( ErrorCode );

}	
/*============================================================================*/
short WINAPI WGFONMeasureItem ( 
	HWND                    hWnd,
	WORD			ControlID,
	LPMEASUREITEMSTRUCT     lpMeasure )

{

	RECT			Rect;
	/*HWND			hControlWnd = GetDlgItem ( hWnd, ControlID );*/

	short                   ErrorCode = 0;

	GetClientRect ( GetDlgItem ( hWnd, ControlID ), &Rect );
	lpMeasure->itemWidth = Rect.right - Rect.left;
	lpMeasure->itemHeight = min ( Rect.bottom - Rect.top, 15 );
	ErrorCode = TRUE;

	return ( ErrorCode );

}
/*============================================================================*/
void WINAPI WGOFNDrawBitmap ( 
	HDC                     hDC,
	WORD                    BitmapIndex,
  WORD                    X,
  WORD                    Y,
	COLORREF                ColorBackground )
 
{

	WGOFN_BITMAP FAR        *lpBitmap = &( WGOFNBitmaps.Bitmaps [ BitmapIndex ] );
	HDC                     hMemDC   = CreateCompatibleDC ( hDC );
	HBITMAP                 hBitmap  = ( HBITMAP )NULL;
	HBITMAP			hOldBitmap;
	BITMAP                  Bitmap;
	COLORREF                BackgroundColor = GetSysColor ( COLOR_WINDOW );
	COLORREF                HighlightColor = GetSysColor ( COLOR_HIGHLIGHT );

	
	if ( ( ColorBackground == lpBitmap->WindowColor ) && ( lpBitmap->hBitmap ) )
		hBitmap = lpBitmap->hBitmap;
	else
	if ( ( ColorBackground == lpBitmap->HighlightColor ) && ( lpBitmap->hHighlightBitmap ) )
		hBitmap = lpBitmap->hHighlightBitmap;
	else
	{
		if ( lpBitmap->hBitmap )
			DeleteObject ( lpBitmap->hBitmap );
		if ( lpBitmap->hHighlightBitmap )
			DeleteObject ( lpBitmap->hHighlightBitmap );
		WGOFNGetBitmap ( hDC, lpBitmap->BitmapID,
				BackgroundColor, HighlightColor,
		                &( lpBitmap->hBitmap ),
				&( lpBitmap->hHighlightBitmap ) );
		lpBitmap->WindowColor    = BackgroundColor;
		lpBitmap->HighlightColor = HighlightColor;
		if ( ColorBackground == BackgroundColor )
			hBitmap = lpBitmap->hBitmap;
		else if ( ColorBackground == HighlightColor )
			hBitmap = lpBitmap->hHighlightBitmap;
	}

	if ( hBitmap )
	{
		GetObject ( hBitmap, sizeof ( BITMAP ), &Bitmap );
		hOldBitmap = SelectObject ( hMemDC, hBitmap );
		BitBlt ( hDC, X, Y, Bitmap.bmWidth, Bitmap.bmHeight, 
		         hMemDC, 0, 0, SRCCOPY );
		SelectObject( hMemDC, hOldBitmap );
	}

	if ( hMemDC )
		DeleteDC ( hMemDC );

}
/*============================================================================*/
#if 1
short WINAPI WGFONDrawItem ( 
	HWND                    hWnd, 
	WORD                    ControlID,
	LPDRAWITEMSTRUCT        lpDrawItem )

{

	static WORD             ShowDriveLetters = 0;
	HBITMAP                 hBitmap = 0;
	DWORD                   ItemData;
	HBRUSH                  hBackgroundBrush = ( HBRUSH )NULL;
	COLORREF                BackgroundColor;
	COLORREF                TextColor;
	RECT                    Rect;
	char                    DriveName [ MAX_FILELENGTH ];
	char                    TwinString [ 64 ];

	short                   ErrorCode = 0;

	
	if ( ( ! ( lpDrawItem->itemAction & ODA_DRAWENTIRE ) ) &&
	     ( ! ( lpDrawItem->itemAction & ODA_SELECT ) ) )
	/*--- Change of focus no need to redraw ---*/
		return ( ErrorCode );

	if ( lpDrawItem->itemState & ODS_SELECTED )
	{
		BackgroundColor = GetSysColor ( COLOR_HIGHLIGHT );
		TextColor       = GetSysColor ( COLOR_HIGHLIGHTTEXT ); 
	}
	else
	{
		BackgroundColor = GetSysColor ( COLOR_WINDOW );
		TextColor       = GetSysColor ( COLOR_WINDOWTEXT );
	}
	if ((hBackgroundBrush = CreateSolidBrush ( BackgroundColor ) ))
	{
		FillRect ( lpDrawItem->hDC, &lpDrawItem->rcItem, hBackgroundBrush );
		DeleteObject ( hBackgroundBrush );
		hBackgroundBrush = ( HBRUSH )NULL;
	}
	SetBkMode ( lpDrawItem->hDC, TRANSPARENT );
	SetBkColor ( lpDrawItem->hDC, BackgroundColor );
	SetTextColor ( lpDrawItem->hDC, TextColor );
	switch ( ControlID )
	{
		default:
			ErrorCode = -1;
			break;

		case WGOFNListFiles:
			SendMessage ( lpDrawItem->hwndItem, LB_GETTEXT, 
			              lpDrawItem->itemID,
			              ( LPARAM )DriveName );
			TextOut ( lpDrawItem->hDC, lpDrawItem->rcItem.left + 2, 
			          lpDrawItem->rcItem.top, DriveName, 
			          lstrlen ( DriveName ) );
			break;
		
		case WGOFNListDirectories:
			ItemData = SendMessage ( lpDrawItem->hwndItem, LB_GETITEMDATA,
			                         lpDrawItem->itemID, 0 );
			CopyRect ( &Rect, &lpDrawItem->rcItem );
			Rect.left += 5 * ( LOWORD ( ItemData ) ) + 2;
			SendMessage ( lpDrawItem->hwndItem, LB_GETTEXT, lpDrawItem->itemID,
			              ( LPARAM )( DriveName ) );
			{
				WORD itmData = HIWORD ( ItemData );
				WORD bmpIndx = ( itmData == 0 ) ? WGOFN_OPENDIR :
					       ( itmData == 1 ) ? WGOFN_CURRDIR :
					       WGOFN_DIR; 
				WGOFNDrawBitmap ( lpDrawItem->hDC, bmpIndx,
					          Rect.left + 2, lpDrawItem->rcItem.top,
					          BackgroundColor );
				TextOut ( lpDrawItem->hDC, Rect.left + 20, 
					          lpDrawItem->rcItem.top, DriveName, 
					          lstrlen ( DriveName ) );
			}
			break;

		case WGOFNComboDrives:
			if ( ! ShowDriveLetters )
			{
				GetTwinString ( WCP_DRIVELETTERS, TwinString, sizeof ( TwinString ) );
				ShowDriveLetters = stricmp ( TwinString, "yes" ) ? 2 : 1;
			}
			ItemData = SendMessage ( lpDrawItem->hwndItem, CB_GETITEMDATA, 
			                         lpDrawItem->itemID, 0 );
			if ( ItemData )
			{
#ifdef TWIN32
				switch ( GetDriveType16 ( ItemData - 1 ) )
#else
				switch ( GetDriveType ( ItemData - 1 ) )
#endif
				{
					default:
						hBitmap = ( HBITMAP )NULL;
						break;
		
					case DRIVE_REMOVABLE:
						WGOFNDrawBitmap ( lpDrawItem->hDC, WGOFN_FLOPPYDRIVE,
						                  lpDrawItem->rcItem.left + 2, 
						                  lpDrawItem->rcItem.top,
					                    BackgroundColor );
						break;

					case DRIVE_FIXED:
						WGOFNDrawBitmap ( lpDrawItem->hDC, WGOFN_HARDDRIVE,
						                  lpDrawItem->rcItem.left + 2, 
						                  lpDrawItem->rcItem.top,
						                  BackgroundColor );
						break;

					case DRIVE_REMOTE:
						WGOFNDrawBitmap ( lpDrawItem->hDC, WGOFN_NETWORKDRIVE,
						                  lpDrawItem->rcItem.left + 2, 
						                  lpDrawItem->rcItem.top,
						                  BackgroundColor );
						break;
				}
			}
			else
				WGOFNDrawBitmap ( lpDrawItem->hDC, WGOFN_PATHDRIVE, 
				                  lpDrawItem->rcItem.left + 2, lpDrawItem->rcItem.top,
				                  BackgroundColor );
			SendMessage ( lpDrawItem->hwndItem, CB_GETLBTEXT, lpDrawItem->itemID,
			              ( LPARAM )( DriveName ) );
			switch ( ShowDriveLetters )
			{
				default:
				case 1:
					TextOut ( lpDrawItem->hDC, lpDrawItem->rcItem.left + 20, 
					          lpDrawItem->rcItem.top, DriveName, 
					          lstrlen ( DriveName ) );
					break;

				case 2:
					if ( ItemData )
						TextOut ( lpDrawItem->hDC, lpDrawItem->rcItem.left + 20, 
						          lpDrawItem->rcItem.top, DriveName + 3, 
						          lstrlen ( DriveName + 3 ) );
					else
						TextOut ( lpDrawItem->hDC, lpDrawItem->rcItem.left + 20, 
						          lpDrawItem->rcItem.top, DriveName, 
						          lstrlen ( DriveName ) );
					break;

			}
	}

	return ( ErrorCode );

}
#else
short WINAPI WGFONDrawItem ( 
	HWND                    hWnd, 
	WORD                    ControlID,
	LPDRAWITEMSTRUCT        lpDrawItem )

{

	static HBITMAP          hPathDrive    = ( HBITMAP ) NULL;
	static HBITMAP          hFloppyDrive  = ( HBITMAP ) NULL;
	static HBITMAP          hHardDrive    = ( HBITMAP ) NULL;
	static HBITMAP          hNetworkDrive = ( HBITMAP ) NULL;
	static HBITMAP          hCDDrive      = ( HBITMAP ) NULL;
	static HBITMAP          hDirectory    = ( HBITMAP ) NULL;
	static HBITMAP          hCurrentDir   = ( HBITMAP ) NULL;
	static HBITMAP          hOpenDir      = ( HBITMAP ) NULL;
	static WORD             ShowDriveLetters = 0;
	HBITMAP                 hBitmap = 0;
	DWORD                   ItemData;
	HBRUSH                  hBackgroundBrush = ( HBRUSH )NULL;
	COLORREF                BackgroundColor;
	COLORREF                TextColor;
	RECT                    Rect;
	char                    DriveName [ MAX_FILELENGTH ];
	char                    TwinString [ 64 ];

	short                   ErrorCode = 0;

	
	if ( ( ! ( lpDrawItem->itemAction & ODA_DRAWENTIRE ) ) &&
	     ( ! ( lpDrawItem->itemAction & ODA_SELECT ) ) )
	/*--- Change of focus no need to redraw ---*/
		return ( ErrorCode );

	if ( lpDrawItem->itemState & ODS_SELECTED )
	{
		BackgroundColor = GetSysColor ( COLOR_HIGHLIGHT );
		TextColor       = GetSysColor ( COLOR_HIGHLIGHTTEXT ); 
	}
	else
	{
		BackgroundColor = GetSysColor ( COLOR_WINDOW );
		TextColor       = GetSysColor ( COLOR_WINDOWTEXT );
	}
	if ( hBackgroundBrush = CreateSolidBrush ( BackgroundColor ) )
	{
		FillRect ( lpDrawItem->hDC, &lpDrawItem->rcItem, hBackgroundBrush );
		DeleteObject ( hBackgroundBrush );
		hBackgroundBrush = ( HBRUSH )NULL;
	}
	SetBkMode ( lpDrawItem->hDC, TRANSPARENT );
	SetBkColor ( lpDrawItem->hDC, BackgroundColor );
	SetTextColor ( lpDrawItem->hDC, TextColor );
	switch ( ControlID )
	{
		default:
			ErrorCode = -1;
			break;

		case WGOFNListFiles:
			SendMessage ( lpDrawItem->hwndItem, LB_GETTEXT, 
			              lpDrawItem->itemID,
			              ( LPARAM )DriveName );
			TextOut ( lpDrawItem->hDC, lpDrawItem->rcItem.left + 2, 
			          lpDrawItem->rcItem.top, DriveName, 
			          lstrlen ( DriveName ) );
			break;
		
		case WGOFNListDirectories:
			ItemData = SendMessage ( lpDrawItem->hwndItem, LB_GETITEMDATA,
			                         lpDrawItem->itemID, 0 );
			CopyRect ( &Rect, &lpDrawItem->rcItem );
			Rect.left += 5 * ( LOWORD ( ItemData ) ) + 2;
			SendMessage ( lpDrawItem->hwndItem, LB_GETTEXT, lpDrawItem->itemID,
			              ( LPARAM )( DriveName ) );
      switch ( HIWORD ( ItemData ) )
			{
				case 0:
					WGOFNDrawBitmap ( lpDrawItem->hDC, WGOFN_OPENDIR,
					                  Rect.left + 2, lpDrawItem->rcItem.top,
					                  BackgroundColor );
					TextOut ( lpDrawItem->hDC, Rect.left + 20, 
					          lpDrawItem->rcItem.top, DriveName, 
					          lstrlen ( DriveName ) );
					break;

				case 1:
					WGOFNDrawBitmap ( lpDrawItem->hDC, WGOFN_CURRDIR,
					                  Rect.left + 2, lpDrawItem->rcItem.top,
					                  BackgroundColor );
					TextOut ( lpDrawItem->hDC, Rect.left + 20, 
					          lpDrawItem->rcItem.top, DriveName, 
					          lstrlen ( DriveName ) );
					break;

				case 2:
					WGOFNDrawBitmap ( lpDrawItem->hDC, WGOFN_DIR, 
					                  Rect.left + 2, lpDrawItem->rcItem.top,
					                  BackgroundColor );
					TextOut ( lpDrawItem->hDC, Rect.left + 20, 
					          lpDrawItem->rcItem.top, DriveName, 
					          lstrlen ( DriveName ) );
					break;

			}
			break;

		case WGOFNComboDrives:
			if ( ! ShowDriveLetters )
			{
				GetTwinString ( WCP_DRIVELETTERS, TwinString, sizeof ( TwinString ) );
				ShowDriveLetters = stricmp ( TwinString, "yes" ) ? 2 : 1;
			}
			ItemData = SendMessage ( lpDrawItem->hwndItem, CB_GETITEMDATA, 
			                         lpDrawItem->itemID, 0 );
			if ( ItemData )
			{
#ifdef TWIN32
				switch ( GetDriveType16 ( ItemData - 1 ) )
#else
				switch ( GetDriveType ( ItemData - 1 ) )
#endif
				{
					default:
						hBitmap = ( HBITMAP )NULL;
						break;
		
					case DRIVE_REMOVABLE:
						WGOFNDrawBitmap ( lpDrawItem->hDC, WGOFN_FLOPPYDRIVE,
						                  lpDrawItem->rcItem.left + 2, 
						                  lpDrawItem->rcItem.top,
					                    BackgroundColor );
						break;

					case DRIVE_FIXED:
						WGOFNDrawBitmap ( lpDrawItem->hDC, WGOFN_HARDDRIVE,
						                  lpDrawItem->rcItem.left + 2, 
						                  lpDrawItem->rcItem.top,
						                  BackgroundColor );
						break;

					case DRIVE_REMOTE:
						WGOFNDrawBitmap ( lpDrawItem->hDC, WGOFN_NETWORKDRIVE,
						                  lpDrawItem->rcItem.left + 2, 
						                  lpDrawItem->rcItem.top,
						                  BackgroundColor );
						break;
				}
			}
			else
				WGOFNDrawBitmap ( lpDrawItem->hDC, WGOFN_PATHDRIVE, 
				                  lpDrawItem->rcItem.left + 2, lpDrawItem->rcItem.top,
				                  BackgroundColor );
			SendMessage ( lpDrawItem->hwndItem, CB_GETLBTEXT, lpDrawItem->itemID,
			              ( LPARAM )( DriveName ) );
			switch ( ShowDriveLetters )
			{
				default:
				case 1:
					TextOut ( lpDrawItem->hDC, lpDrawItem->rcItem.left + 20, 
					          lpDrawItem->rcItem.top, DriveName, 
					          lstrlen ( DriveName ) );
					break;

				case 2:
					if ( ItemData )
						TextOut ( lpDrawItem->hDC, lpDrawItem->rcItem.left + 20, 
						          lpDrawItem->rcItem.top, DriveName + 3, 
						          lstrlen ( DriveName + 3 ) );
					else
						TextOut ( lpDrawItem->hDC, lpDrawItem->rcItem.left + 20, 
						          lpDrawItem->rcItem.top, DriveName, 
						          lstrlen ( DriveName ) );
					break;

			}
	}

	return ( ErrorCode );

}
#endif
/*============================================================================*/
short WINAPI WGFONCompareItem ( 
	HWND                    hWnd, 
	WORD                    ControlID,
	LPCOMPAREITEMSTRUCT     lpCompareItem )

{

	short                   ErrorCode = 0;
	LRESULT                 ReturnedValue1;
	LRESULT                 ReturnedValue2;
	short                   Value1;
	short                   Value2;
	char                    String1 [ MAX_FILELENGTH ];
	char                    String2 [ MAX_FILELENGTH ];


	if ( ControlID == WGOFNListDirectories )
	{
		ReturnedValue1 = SendMessage ( lpCompareItem->hwndItem, CB_GETITEMDATA, 
		                               lpCompareItem->itemID1, 0 );
		ReturnedValue2 = SendMessage ( lpCompareItem->hwndItem, CB_GETITEMDATA,
		                               lpCompareItem->itemID2, 0 );
		if ( ( ReturnedValue1 != CB_ERR ) && 
		     ( ReturnedValue2 != CB_ERR ) )
		{
			Value1 = ( short )LOWORD ( ReturnedValue1 );
			Value2 = ( short )LOWORD ( ReturnedValue2 );
			if ( Value1 < Value2 )
				ErrorCode = -1;
			else
			if ( Value1 > Value2 )
				ErrorCode = 1;
		}
	}

	if ( ! ErrorCode )
	{
		SendMessage ( lpCompareItem->hwndItem, CB_GETLBTEXT, 
		              lpCompareItem->itemID1, ( LPARAM )( String1 ) );
		SendMessage ( lpCompareItem->hwndItem, CB_GETLBTEXT, 
		              lpCompareItem->itemID2, ( LPARAM )( String2 ) );
		ErrorCode = lstrcmp ( String1, String2 );
		if ( ErrorCode < 0 )
			ErrorCode = -1;
		else
		if ( ErrorCode > 0 )
			ErrorCode = 1;
	}

	return ( ErrorCode );

}
/*============================================================================*/
short WINAPI WGOFNControlCommand (
	HWND                    hWnd, 
	HWND                    hControlWnd,
	WORD                    ControlID,
	WORD                    Notification,
	LPOPENFILENAME          lpOpenFileName )

{

	short                   ErrorCode = 0;

	
	switch ( ControlID )
	{
		case WGOFNEditFileName:
			switch ( Notification )
			{
				default:
					break;
					
				case EN_KILLFOCUS:
					ErrorCode = WGOFNEditFileNameLoseFocus ( hWnd, 
										 hControlWnd,
										 lpOpenFileName );
					break;
			}
			break;
			
		case WGOFNListFiles:
			switch ( Notification )
			{
				default:
					break;
						   
				case LBN_DBLCLK:
					ErrorCode = WGOFNListFilesDoubleClick ( hWnd,
										hControlWnd,
										lpOpenFileName );
					break;
											   
				case LBN_SELCHANGE:
					if ( ! ( ErrorCode = WGOFNListFilesSelectionChange ( hWnd,
											     hControlWnd,
											     lpOpenFileName ) ) )
#if 0
						SendMessage ( hWnd, WGOFNWM_LBSELCHANGE, ControlID, 
							      MAKELONG ( ( WORD ) SendMessage ( hWnd, CB_GETCURSEL, 
									 ( WPARAM )NULL, ( LPARAM )NULL ), 
									 CD_LBSELCHANGE ) );
#else
					;
#endif
					break;
			}
			break;
			
		case WGOFNTextDirectory:
			break;
			
		case WGOFNListDirectories:
			switch ( Notification )
			{
				default:
					break;
					
				case LBN_DBLCLK:
					ErrorCode = WGOFNListDirectoriesDoubleClick ( hWnd,
										      hControlWnd,
										      lpOpenFileName );
					break;
			}
			break;
			
		case WGOFNComboFilters:
			switch ( Notification )
			{
				default:
					break;
					
				case CBN_SELCHANGE:
					if ( ! ( ErrorCode = WGOFNComboFiltersSelectionChange ( hWnd, 
												hControlWnd,
												lpOpenFileName ) ) )
#if 0
						SendMessage ( hWnd, WGOFNWM_LBSELCHANGE, ControlID, 
							      MAKELONG ( ( WORD ) SendMessage ( hWnd, CB_GETCURSEL, 
									 ( WPARAM )NULL, ( LPARAM )NULL ), 
									 CD_LBSELCHANGE ) );
#else
					;
#endif
					break;
			}
			break;
			
		case WGOFNComboDrives:
			switch ( Notification )
			{
				default:
					break;
				
				case CBN_SELCHANGE:
					if ( ! ( ErrorCode = WGOFNComboDrivesSelectionChange ( hWnd,
											       hControlWnd,
											       lpOpenFileName ) ) )
#if 0
						SendMessage ( hWnd, WGOFNWM_LBSELCHANGE, ControlID, 
							      MAKELONG ( ( WORD ) SendMessage ( hWnd, CB_GETCURSEL, 
									 ( WPARAM )NULL, ( LPARAM )NULL ), 
									 CD_LBSELCHANGE ) );
#else
						;
#endif
					break;
			}
			break;
			
		case WGOFNButtonHelp:
			switch ( Notification )
			{
				default:
					break;
				
				case BN_CLICKED:
					SendMessage ( GetParent ( hWnd ), 
					              WGOFNGetWindowMessage ( WGOFN_WM_HELP ), 
					              0, 0 ); 
					break;
			}
			break;
			
		case WGOFNCheckReadOnly:
			break;
	}
	
	return ( ErrorCode );

}
/*============================================================================*/
BOOL CALLBACK WGOFNDialogProc (
	HWND                    hWnd,
	UINT                    Message,
	WPARAM                  wParam,
	LPARAM                  lParam )
    
{

	LPOPENFILENAME          lpOpenFileName = WGOFNLockOpenFileName ( hWnd );
	LRESULT			lResult        = 0;

	
	if ( ( lpOpenFileName ) && ( lpOpenFileName->Flags & OFN_ENABLEHOOK ) )
	{
		lResult = ( *lpOpenFileName->lpfnHook )( hWnd, Message, wParam, lParam );
	}
	if ( ( lResult ) && ( Message != WM_INITDIALOG ) )
		return ( TRUE );

	lResult = TRUE;

	switch ( Message )
	{
		default:
			lResult = FALSE;
			break;
      
		case WM_INITDIALOG:
			lpOpenFileName = ( LPOPENFILENAME ) lParam;
			if ( (lpOpenFileName->Flags & OFN_ENABLEHOOK) &&
				lpOpenFileName->lpfnHook )
				( *lpOpenFileName->lpfnHook )( hWnd, Message, 
							       wParam, lParam );
			if ( WGFONInitDialog ( hWnd, ( LPOPENFILENAME ) lParam ) )
				PostMessage ( hWnd, WM_CLOSE, ( WPARAM )NULL, 
					      ( LPARAM ) NULL );
			/* not necessary
			else
				WGOFNComboDrivesSelectionChange ( hWnd, GetDlgItem ( hWnd, WGOFNComboDrives ), ( LPOPENFILENAME ) lParam );*/
			lpOpenFileName = ( LPOPENFILENAME )NULL;
			break;
      
		case WM_CLOSE:
			WGFONEndDialog ( hWnd );
			EndDialog ( hWnd, wParam );
			break;

		case WM_MEASUREITEM:
			WGFONMeasureItem ( hWnd, wParam, ( LPMEASUREITEMSTRUCT ) lParam );
			return ( TRUE );
			break;

		case WM_DRAWITEM:
			WGFONDrawItem ( hWnd, wParam, ( LPDRAWITEMSTRUCT ) lParam );
			break;

		case WM_COMPAREITEM:
			lResult = WGFONCompareItem ( hWnd, wParam, 
			                             ( LPCOMPAREITEMSTRUCT ) lParam );
			break;
      
#if 0
		case WM_ERASEBKGND:
			if ( ( hDC = GetDC ( hWnd ) ) &&
			     ( hBrush = ( HBRUSH ) SendMessage ( hWnd, 
			                                         GET_WM_CTLCOLOR_MSG ( CTLCOLOR_DLG ),
			                                         GET_WM_CTLCOLOR_MPS ( hDC, hWnd, CTLCOLOR_DLG ) ) ) )
			{
				GetClientRect ( hWnd, &Rect );
				FillRect ( ( HDC )wParam, &Rect, hBrush ); 
			}
			if ( hDC ) 
				ReleaseDC ( hWnd, hDC );
			break; 
#endif

		case WM_WININICHANGE:
			if ( ! lstrcmp ( ( LPSTR )lParam, "xdos" ) )
				WGOFNInitControls ( hWnd, lpOpenFileName );
			break;

		case WM_COMMAND:
			switch ( GET_WM_COMMAND_ID(wParam, lParam) )
			{
				default:
					lResult = FALSE;
					break;
	    
				case WGOFNEditFileName:
				case WGOFNListFiles:
				case WGOFNTextDirectory:
				case WGOFNListDirectories:
				case WGOFNComboFilters:
				case WGOFNComboDrives:
				case WGOFNButtonHelp:
				case WGOFNCheckReadOnly:
					WGOFNControlCommand ( hWnd, 
					    GET_WM_COMMAND_HWND(wParam, lParam),
					    GET_WM_COMMAND_ID(wParam, lParam),
					    GET_WM_COMMAND_CMD(wParam, lParam),
					    lpOpenFileName );
					break;

				case IDOK:
					if ( WGOFNCheckOK ( hWnd, lpOpenFileName ) != 0 )
						break;
/* modified: hung 01/23/97 - MFC CFileDialog would subclass and intercept this registered
                             window message, look up its message mapping table, find nothing and
			     ASSERT */
#if 0
					if ( SendMessage ( hWnd, WGOFNWM_FILEOK, 0, ( LPARAM )( lpOpenFileName ) ) )
						break;
#else
					if ( ( lpOpenFileName ) && ( lpOpenFileName->Flags & OFN_ENABLEHOOK ) )
						lResult = ( *lpOpenFileName->lpfnHook )( hWnd, Message, wParam, lParam );
#endif
				case IDCANCEL:
					PostMessage ( hWnd, WM_CLOSE,
					    GET_WM_COMMAND_ID(wParam,lParam),
					    0 );
				break;
			}
			break;
	}
	
	if ( lpOpenFileName )
		WGOFNUnlockOpenFileName ( hWnd );
  
	if ( lResult )
		SetWindowLong ( hWnd, DWL_MSGRESULT, lResult ); 

	return ( lResult );
  
}
/*============================================================================*/
BOOL WINAPI WGOFNGetFileName (
	LPOPENFILENAME          lpOpenFileName,
	WORD                    Style )

{

	FARPROC                 lpfnDialogProc  = ( FARPROC )NULL;
	HRSRC                   hResource       = ( HRSRC )NULL;
	HGLOBAL                 hStandardDialog = ( HGLOBAL )NULL;
	LPDLGTEMPLATE		lpTmp		= ( LPDLGTEMPLATE )NULL;
	HGLOBAL                 hUseDialogBox = 0;

	short                   ErrorCode   = 0;
	BOOL                    ReturnValue = TRUE;

  
	if ((ErrorCode = WGOFNCheckParameters ( lpOpenFileName, Style ) ))
	/*--- Check to see that the elements of lpOpenFileName make sense ---*/
		ReturnValue = FALSE;
	else
	{
		if ( lpOpenFileName->Flags & OFN_ENABLETEMPLATEHANDLE )
		/*--- if the user has supplied a template use it; note the template must
			exist because the parameters were already checked ---*/
			hUseDialogBox = lpOpenFileName->hInstance;
		else
		if ( ! ( hResource = FindResource ( WGOFNGetInstance (), 
						    ( LPSTR )MAKEINTRESOURCE ( lpOpenFileName->Flags & OFN_ALLOWMULTISELECT ? 
								      IDD_OPENFILE2 : IDD_OPENFILE ),
						    RT_DIALOG ) ) )
			ErrorCode = WGOFNERR_FINDRESOURCE;
		else
		if ( ! ( hUseDialogBox = hStandardDialog = LoadResource ( WGOFNGetInstance (), 
									  hResource ) ) )
			ErrorCode = WGOFNERR_LOADRESOURCE;
		else
			lpTmp = ( LPDLGTEMPLATE )LockResource ( hUseDialogBox );
		
		if ( ! ( lpfnDialogProc = MakeProcInstance ( ( FARPROC )WGOFNDialogProc, WGOFNGetInstance () ) ) )
			ErrorCode = WGOFNERR_MAKEPROCINSTANCE;
			
		if ( ( ! ErrorCode ) && ( lpTmp ) )
#ifdef TWIN32
			ReturnValue = DialogBoxIndirectParam ( WGOFNGetInstance (), 
							       lpTmp, 
							       lpOpenFileName->hwndOwner, 
							       lpfnDialogProc,
							       ( LPARAM ) lpOpenFileName );
#else
			ReturnValue = DialogBoxIndirectParam ( WGOFNGetInstance (), 
							       hUseDialogBox, 
							       lpOpenFileName->hwndOwner, 
							       lpfnDialogProc,
							       ( LPARAM ) lpOpenFileName );
#endif
		if ( hStandardDialog )
		/*--- if the resource was loaded then free it ---*/
			FreeResource ( hStandardDialog );
			
		if ( lpfnDialogProc )
		/*--- Free the procinstance ---*/
			FreeProcInstance ( lpfnDialogProc );
	}
  
	WGOFNSetExtendedError ( ErrorCode );
  
	return ( ReturnValue == IDOK );                         

}
/*============================================================================*/
BOOL WINAPI WGetOpenFileName (
	LPOPENFILENAME          lpOpenFileName )

{

	return ( WGOFNGetFileName ( lpOpenFileName, WGOFN_OPEN ) );

}
/*============================================================================*/
BOOL WINAPI WGetSaveFileName (
	LPOPENFILENAME          lpOpenFileName )

{

	return ( WGOFNGetFileName ( lpOpenFileName, WGOFN_SAVE ) );

}
/*============================================================================*/
short WGOFNInitDialog ( void )

{

	return ( 0 );

}
/*============================================================================*/
short WGOFNTerminateDialog ( void )

{

	WORD                       Bitmap;


	for ( Bitmap = 0; Bitmap < WGOFN_NUMBITMAPS; Bitmap++ )
	{
		if ( WGOFNBitmaps.Bitmaps [ Bitmap ].hBitmap )
		{
			DeleteObject ( WGOFNBitmaps.Bitmaps [ Bitmap ].hBitmap );
			WGOFNBitmaps.Bitmaps [ Bitmap ].hBitmap = 0;
		}
		if ( WGOFNBitmaps.Bitmaps [ Bitmap ].hHighlightBitmap )
		{
			DeleteObject ( WGOFNBitmaps.Bitmaps [ Bitmap ].hHighlightBitmap );
			WGOFNBitmaps.Bitmaps [ Bitmap ].hHighlightBitmap = 0;
		}
	}

	return ( 0 );

}

