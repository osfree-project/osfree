/*    
	Commdlg.c	1.6 		Common Dialog Support Routines
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
#include "commdlg.h"

DWORD LastCommonDialogError = 0;

DWORD WCDSetExtendedError (
	DWORD                           NewError )

{

	DWORD                           OldError = LastCommonDialogError;


	LastCommonDialogError = NewError;

	return ( OldError );

}

extern int InternalDlgIndirectParam(HINSTANCE,HGLOBAL,HWND,DLGPROC,LPARAM);

/*********************************************************************/
BOOL
MakeDialogBox(HINSTANCE hInstance, HWND hwndOwner, LPCSTR lpTemplateName, 
		BOOL bEnableTemplate, BOOL bEnableTemplateHandle, 
		LPCSTR lpRsc, LPVOID lpCommDlgStruct, DLGPROC lpfnDlgProc)
{
	HINSTANCE	hInst;
	LPSTR		lpDlg;

	LastCommonDialogError = 0;

	if ( !(hInst = GetModuleHandle("COMMDLG")) )
		hInst = LoadLibrary("COMMDLG");
	
	if ( bEnableTemplateHandle )
		return InternalDlgIndirectParam(hInst, hInstance, hwndOwner,
					lpfnDlgProc, (LPARAM)lpCommDlgStruct);

	if ( bEnableTemplate ) {
		lpDlg = (LPSTR)lpTemplateName;
		hInst = hInstance;
	}
	else
		lpDlg = (LPSTR)lpRsc;

	return DialogBoxParam(hInst, lpDlg, hwndOwner,
			    lpfnDlgProc, (LPARAM)lpCommDlgStruct);
}
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
/******************************************************************************/
DWORD WINAPI CommDlgExtendedError ( void )
{

	return ( LastCommonDialogError );

}

extern BOOL  WGetOpenFileName (LPOPENFILENAME);
extern BOOL  WGetSaveFileName (LPOPENFILENAME);
extern short WGOFNInitDialog ();
extern short WGOFNTerminateDialog ();

BOOL WINAPI GetOpenFileName (
	LPOPENFILENAME                   lpOpenFileName )

{
	
	return ( WGetOpenFileName ( lpOpenFileName ) );

}

BOOL WINAPI GetSaveFileName (
	LPOPENFILENAME                   lpOpenFileName )

{

	return ( WGetSaveFileName ( lpOpenFileName ) );

}

short TWIN_InitCommdlg ( void )

{

	return ( WGOFNInitDialog () );

}

short TWIN_TerminateCommdlg ( void )

{

	return ( WGOFNTerminateDialog () );

}
