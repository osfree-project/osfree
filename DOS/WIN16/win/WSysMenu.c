/*    
	WSysMenu.c	1.24
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

 *   System menu implementation.
 *   Michael Dvorkin  09-JAN-1996  added dynamic drive mapping
 *
 */

#include "windows.h"
#include "commdlg.h"
#include "RscData.h"
#include "mfs_config.h"

#include "windowsx.h"
#include "WinDefs.h"
#include "WinConfig.h"
#include "Log.h"
#include <string.h>

#define WSYSSTR_WILLOWS_SYSTEM		"Willows_System"

static BOOL	WSysInitialized = FALSE;
static HWND	hSysWnd         = 0;
static int	InSysMenu       = 0;
static HMENU    hDLLMenu;
static BOOL	InGetExe        = FALSE;
static HWND	hRealWnd        = 0;
static int      WSysDrivesDlgProc(HWND, UINT, WPARAM, LPARAM);
static int      WSysDriveMapDlgProc(HWND, UINT, WPARAM, LPARAM);
static void     ShowDriveMap(HWND, int);

int charicmp ( char , char );
static void WSysMenuHandler ( HWND , UINT , WPARAM , LPARAM );


int
WSysDLLErr(HWND hWnd, LPSTR lpMsg)
{
	char		MsgStr [ 255 ];

	wsprintf(MsgStr,"Error in commdlg.dll,\n%s\n",lpMsg);
	MessageBox(hWnd, MsgStr, "Willows Software, Inc.", MB_OK);
	return FALSE;
}


/*----------------------------------------------------------------------------*/
short WINAPI
WSysGetExe ( HWND hWnd, LPSTR lpFileName, size_t	FileNameSize )
{
	char		CurrentDirectory [ 255 ];
	char*		ExeFileFilter = "Windows (*.exe)\0*.exe\0";
	OPENFILENAME	OpenFileName;
	FARPROC		opfn;
	HINSTANCE	hInst;
	short		ReturnCode = 0;

	if ( ! InGetExe ) {
		InGetExe = TRUE;
		MFS_GETCWD(0, CurrentDirectory, sizeof(CurrentDirectory));

		memset ( &OpenFileName, 0, sizeof ( OpenFileName ) );
		OpenFileName.lStructSize  = sizeof ( OpenFileName );
		OpenFileName.hwndOwner    = hWnd;
		OpenFileName.lpstrFilter  = ExeFileFilter;
		OpenFileName.nFilterIndex = 1;
		OpenFileName.lpstrFile    = lpFileName;
		OpenFileName.nMaxFile     = FileNameSize;

		/*ReturnCode = GetOpenFileName ( &OpenFileName ); */

		hInst = LoadLibrary("commdlg");
		if(hInst == 0) {
			return WSysDLLErr(hWnd,"Cannot LoadLibrary(commdlg)");
		}
		opfn = GetProcAddress(hInst,"GetOpenFileName");
		if(opfn) {
			ReturnCode = opfn ( &OpenFileName );
			InGetExe = FALSE;
		} else {
			return WSysDLLErr(hWnd,
				"GetProcAddress(GetOpenFileName) failed."); 
		}

	}

	return ( ReturnCode );

}
/*----------------------------------------------------------------------------*/
BOOL CALLBACK
WSysEnumTaskList ( HWND hWnd, LPARAM lParam )
{
	HWND			hListWnd = ( HWND ) lParam;
	int			Item;
	int			ItemCount;
	BOOL			AddToList = TRUE;
	LRESULT			ReturnValue;
	char			WindowTitle [ 255 ];

	ReturnValue = SendMessage ( hListWnd, LB_GETCOUNT, 
		(WPARAM) NULL, (LPARAM) NULL );

	switch ( ReturnValue )
	{
		default:
			ItemCount = ( int ) ReturnValue;
			Item      = 0;
			do
			{
				ReturnValue = SendMessage ( hListWnd, 
					LB_GETITEMDATA, ( WPARAM ) Item, 0 );
				if ( GetWindowInstance ( (HWND) ReturnValue ) == 
				     GetWindowInstance (        hWnd ) )
					AddToList = FALSE;
				else
					Item++;
			} while ( ( AddToList ) && ( Item < ItemCount ) );

			/*--- Fall Through ---*/
		case 0:
			if ( ( AddToList ) && ( IsWindowVisible ( hWnd ) ) )
			{
				GetWindowText ( hWnd, WindowTitle, sizeof ( WindowTitle ) );
				ReturnValue = SendMessage ( hListWnd, LB_ADDSTRING, 0, 
				                            ( LPARAM ) WindowTitle );
				SendMessage ( hListWnd, LB_SETITEMDATA, ( WPARAM ) ReturnValue, hWnd );
			}
			ReturnValue = TRUE;
			break;

		case LB_ERR:
			ReturnValue = FALSE;
			break;
	}
	
	return ( ReturnValue );

}
/*----------------------------------------------------------------------------*/
static short WINAPI
WSysInitTaskList (HWND hWnd )
{

	HWND	hListWnd = GetDlgItem ( hWnd, IDC_TASKLIST );
	
	return ( EnumWindows ( (WNDENUMPROC) WSysEnumTaskList, hListWnd ) );

}

/*----------------------------------------------------------------------------*/
static void WINAPI
WSysSwitchTo ( HWND hWnd, WPARAM Action )
{
	HWND		hSwitchToWnd;
	LRESULT		ReturnValue;
	HWND		hListWnd = GetDlgItem ( hWnd, IDC_TASKLIST );

	switch ( ReturnValue = SendMessage ( hListWnd, LB_GETCURSEL, 0, 0 ) )
	{
		default:
			ReturnValue = SendMessage ( hListWnd, LB_GETITEMDATA, 
			                            ( WPARAM ) ReturnValue, 0 );
			if ((hSwitchToWnd = ( HWND ) ReturnValue ))
				switch ( Action )
				{
					case IDABORT:
						PostMessage ( hSwitchToWnd, WM_CLOSE, 
							(WPARAM) NULL, (LPARAM) NULL );
						break;

					case IDOK:
						if ( IsIconic ( hSwitchToWnd ) )
							ShowWindow ( hSwitchToWnd, SW_RESTORE );
						SetFocus ( hSwitchToWnd );
						break;
				}
			break;

		case LB_ERR:
			break;
	}
}

/*----------------------------------------------------------------------------*/
static int CALLBACK
WSysTaskListDlgProc ( HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam )
{

	switch ( Message )
	{
		default:
			return ( FALSE );

		case WM_INITDIALOG:
			WSysInitTaskList ( hWnd );
			break;

		case WM_CLOSE:
			EndDialog ( hWnd, wParam );
			break;

		case WM_COMMAND:
			switch ( GET_WM_COMMAND_ID(wParam,lParam) )
			{
				default:
					return ( FALSE );
			
				case IDOK:
				case IDCANCEL:
				case IDABORT:
					WSysSwitchTo ( hWnd, GET_WM_COMMAND_ID(wParam,lParam) );
					PostMessage ( hWnd, WM_CLOSE, 
						GET_WM_COMMAND_ID(wParam,lParam), 0 );
					break;

			}
			break;
		}

	return ( TRUE );

}
/*----------------------------------------------------------------------------*/
static short WINAPI
WSysTaskList ( HWND hWnd )
{

	static BOOL	TaskListUp = FALSE;
	short       ErrorCode = 0;


	if ( ! TaskListUp )
	{
		TaskListUp = TRUE;
		ErrorCode = DialogBox ( GetModuleHandle ( "USER" ), "TaskList", hWnd, 
		                        (DLGPROC) WSysTaskListDlgProc );
		TaskListUp = FALSE;
	}

	return ( ErrorCode );

}
/*----------------------------------------------------------------------------*/
void WINAPI
WSysAbout
	( HWND						hWnd )

{
	HICON		hIcon;
	char		String[256];
	FARPROC		shellabout;
	HINSTANCE	hInst;

	/* LATER: Move our icon to RscData.rc instead */
	hIcon = LoadIcon ( 0, MAKEINTRESOURCE ( IDI_WILLOWS ) );

	/* note: this will fail if we are using the REAL shell.dll */
	/*       so if we don't have shellabout, then call         */
	/*       MessageBox instead.                               */
	/* note: we need to do this, rather than a direct call to  */	
	/* 	 shell about function, it is now in a library      */
	/*       It does not have a native to binary hook 	   */
	hInst = LoadLibrary("shell");
	shellabout = GetProcAddress(hInst,"ShellAbout");

	strcpy(String,"http://www.willows.com");
	/*								*/
 	/* Note: this will fail even if the real shell.dll is loaded 	*/
	/*	 because we don't have an OEM table for the shell       */
	/*	 functions.  A binary application can call the function */
	/*	 directly.						*/
	if(shellabout)
		shellabout( hWnd, "Willows Software TWIN", String, hIcon );
	else
		MessageBox(hWnd, "Willows Software TWIN", "Willows Software, Inc.", MB_OK);
	if(hIcon)
		DestroyIcon ( hIcon );

}
/*----------------------------------------------------------------------------*/
static short WINAPI
WSysRun ( HWND hWnd )
{
	char	FileName [ 255 ];
	short	ReturnCode = 0;

	memset ( FileName, 0, sizeof ( FileName ) );
	if ( ( ReturnCode = WSysGetExe ( hWnd, FileName, sizeof ( FileName ) ) ) == 
	     IDOK )
		WinExec ( FileName, 0 );

	return ( ReturnCode );

}
/*----------------------------------------------------------------------------*/
static LRESULT CALLBACK
WSysWindowProc ( HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam )
{

	LRESULT		ReturnValue = 0;

	switch ( Message )
	{
		default:
			ReturnValue = DefWindowProc ( hWnd, Message, wParam, lParam );
			break;

		case WM_COMMAND:
			switch ( GET_WM_COMMAND_ID(wParam,lParam) )
			{
				case SC_RESTORE:
					ShowWindow ( hRealWnd, SW_RESTORE );
					break;
		
				case SC_MINIMIZE:
					ShowWindow ( hRealWnd, SW_MINIMIZE );
					break;

				case SC_MAXIMIZE:
					ShowWindow ( hRealWnd, WS_MAXIMIZE );
					break;

				case SC_CLOSE:
					PostMessage (hRealWnd,WM_CLOSE,(WPARAM) NULL,(LPARAM)NULL);
					break;

				case SC_TASKLIST:
					WSysTaskList ( hWnd );
					break;

			        default:
					WSysMenuHandler( hWnd,Message,
						wParam, lParam );

			}
		}

	return ( ReturnValue );
}

extern CONFIG xdos;
extern LPDEVICE AllocateDrive(LPSTR, LPSTR); /* resides in xdos/mfs_config.c  */

/*----------------------------------------------------------------------------*/
static int 
WSysDrivesDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    int   i, nCur;
    char  sz[256];
    DWORD dwItem;

    switch (msg)
        {
        case WM_INITDIALOG:
           i = 4; /* tabstop position */
           SendDlgItemMessage(hWnd, IDL_DRIVELIST, LB_SETTABSTOPS, 1, (LPARAM)(LPINT)&i);
           ShowDriveMap(hWnd, 0);
           break;

        case WM_CLOSE:
           EndDialog (hWnd, wParam);
           break;

        case WM_COMMAND:
           switch (GET_WM_COMMAND_ID(wParam,lParam))
              {
              case IDL_DRIVELIST:
                 if (HIWORD(lParam) == LBN_SELCHANGE)
                    {
                    nCur = (int)SendDlgItemMessage(hWnd, IDL_DRIVELIST, LB_GETCURSEL, 0, 0L);
                    dwItem = SendDlgItemMessage(hWnd, IDL_DRIVELIST, LB_GETITEMDATA, nCur, 0L);
                    EnableWindow(GetDlgItem(hWnd, IDB_MAP), !dwItem);
                    EnableWindow(GetDlgItem(hWnd, IDB_UNMAP), dwItem);
                    }
                 break;

              case IDOK:
                 /*
                 **   Don't exit if there are no mapped drives left!
                 */
                 for (nCur = 0, i = 1;   i <= 26;   i++)
                     {
                     nCur = (int)(xdos.drivemap[i] != NULL);
                     if (nCur) break;
                     }
                 if (!nCur)
                    {
                    MessageBox(hWnd, "At least one drive should be mapped", "Drive Map", MB_OK | MB_ICONEXCLAMATION | MB_APPLMODAL);
                    break;
                    }
                 /*
                 **   If IDC_MAP checkbox has been checked, then save
                 **   all drive mappings in .twinrc (or whatever our
                 **   startup .INI file is).
                 */
                 if (IsDlgButtonChecked(hWnd, IDC_MAP))
                    {
                    char   szIni[256];

                    strcpy(szIni, GetTwinFilename());
                    for (i = 1;   i <= 26;   i++)
                        {
                        sprintf(sz, "%c", i-1+'A');
                        if (xdos.drivemap[i])
                           WritePrivateProfileString("xdos", sz, xdos.drivemap[i]->name, szIni);
                        else WritePrivateProfileString("xdos", sz, NULL, szIni); /* delete the entry */
                        }
                    }
                 PostMessage(GetParent(hWnd), WM_WININICHANGE, 0L, (LPARAM)(LPSTR)"xdos");
                 EndDialog(hWnd, wParam);
                 break;

              case IDCANCEL:
                 EndDialog(hWnd, wParam);
                 break;

              case IDB_MAP:
                 /* Make sure current drive is unmapped */
                 nCur = (int)SendDlgItemMessage(hWnd, IDL_DRIVELIST, LB_GETCURSEL, 0, 0L);
                 dwItem = SendDlgItemMessage(hWnd, IDL_DRIVELIST, LB_GETITEMDATA, nCur, 0L);
                 if (!dwItem && xdos.drivemap[nCur+1] == NULL)
                    {
		    if (DialogBoxParam(0, "DRIVES_MAP", hWnd, (DLGPROC)WSysDriveMapDlgProc, (LPARAM)(nCur+1)))
                       ShowDriveMap(hWnd, nCur);
                    }
                 break;

              case IDB_UNMAP:
                 /* make sure current drive has been mapped */
                 nCur = (int)SendDlgItemMessage(hWnd, IDL_DRIVELIST, LB_GETCURSEL, 0, 0L);
                 dwItem = SendDlgItemMessage(hWnd, IDL_DRIVELIST, LB_GETITEMDATA, nCur, 0L);
                 if (dwItem && xdos.drivemap[nCur+1])
                    {
                    WinFree((LPVOID)xdos.drivemap[nCur+1]);
                    xdos.drivemap[nCur+1] = NULL;
                    xdos.drivemap[0] = NULL;   /* no current drive yet */
                    if (xdos.currentdrive == nCur+1)
                       {
                       /*   Go find new current drive, first down, then up...
                       */
                       for (i = xdos.currentdrive+1;  i <= 26;  i++)
                           {
                           if (xdos.drivemap[i] != NULL)
                              { 
                              xdos.currentdrive = i;
                              xdos.drivemap[0] = xdos.drivemap[i];
                              break;
                              }
                           }
                       if (xdos.drivemap[0] == NULL)
                          {
                          for (i = xdos.currentdrive-1;  i >= 0;  i--)
                              {
                              if (xdos.drivemap[i] != NULL)
                                 { 
                                 xdos.currentdrive = i;
                                 xdos.drivemap[0] = xdos.drivemap[i];
                                 break;
                                 }
                              }
                          }
                       }
                    ShowDriveMap(hWnd, nCur);
                    }
                 break;
              }
           break;

        default:
           return 0;
        }

    return 1; /* message processed */
}

/*----------------------------------------------------------------------------*/
static int WSysDriveMapDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
       char  szDrive[4];
       char  sz[256];
static int   nDrive;

    switch (msg)
        {
        case WM_INITDIALOG:
           nDrive = lParam-1+'A';
           sprintf(sz, "Map Drive %c: to Path:", nDrive);
           SetWindowText(hWnd, sz); 
           break;

        case WM_CLOSE:
           EndDialog (hWnd, 0); /* no changes... */
           break;

        case WM_COMMAND:
           switch (GET_WM_COMMAND_ID(wParam,lParam))
              {
              case IDOK:
                 GetDlgItemText(hWnd, IDE_MAP, sz, sizeof(sz)-1);
                 sprintf(szDrive, "%c", nDrive);
                 AllocateDrive(szDrive, sz);
                 EndDialog(hWnd, 1); /* tell calling party to reload listbox */
                 break;
                 
              case IDCANCEL:
                 EndDialog(hWnd, 0); /* no changes... */
                 break;
              }
        default:
           return 0;
        }
   return 1;
}


/*----------------------------------------------------------------------------*/
static void ShowDriveMap(HWND hWnd, int nCur)
{
   int   i;
   char  sz[256];
   DWORD dwItem;

   SendDlgItemMessage(hWnd, IDL_DRIVELIST, WM_SETREDRAW, 0, 0L);
   SendDlgItemMessage(hWnd, IDL_DRIVELIST, LB_RESETCONTENT, 0, 0L);
   for (i = 1;   i <= 26;   i++)
       {
       if (xdos.drivemap[i] != NULL)
          {
          sprintf(sz, "%c:\t%s", i-1+'A', xdos.drivemap[i]->name);
          SendDlgItemMessage(hWnd, IDL_DRIVELIST, LB_ADDSTRING, i-1, (LPARAM)(LPSTR)sz);
          SendDlgItemMessage(hWnd, IDL_DRIVELIST, LB_SETITEMDATA, i-1, 1L);
          }
       else {
            sprintf(sz, "%c:\t(unmapped)", i-1+'A');
            SendDlgItemMessage(hWnd, IDL_DRIVELIST, LB_ADDSTRING, i-1, (LPARAM)(LPSTR)sz);
            SendDlgItemMessage(hWnd, IDL_DRIVELIST, LB_SETITEMDATA, i-1, 0L);
            }
       }
   SendDlgItemMessage(hWnd, IDL_DRIVELIST, WM_SETREDRAW, 0, 0L);
   SendDlgItemMessage(hWnd, IDL_DRIVELIST, LB_SETCURSEL, nCur, 0L);
   dwItem = SendDlgItemMessage(hWnd, IDL_DRIVELIST, LB_GETITEMDATA, nCur, 0L);
   EnableWindow(GetDlgItem(hWnd, IDB_MAP), !dwItem);
   EnableWindow(GetDlgItem(hWnd, IDB_UNMAP), dwItem);
}
            
static void
WSysMenuHandler ( HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam )
{
	switch( GET_WM_COMMAND_ID(wParam,lParam) ) {
	case WSYS_ABOUT:
		WSysAbout ( hWnd );
		break;

	case WSYS_CONFIGURE:
		break;

	case WSYS_RUN:
		WSysRun ( hWnd );
		break;

	case WSYS_DRIVES:
		DialogBox(0,"DRIVES_CFG",hWnd,(DLGPROC)WSysDrivesDlgProc);
		break;

	default:
		DefWindowProc(hWnd,Message,wParam,lParam);
		break;
	}
}

static void
WSysMenuUpdate(HMENU hMenu)
{
	static int bInit;

	if(bInit)
		return;
	EnableMenuItem ( hMenu, WSYS_CONFIGURE, MF_BYCOMMAND|MF_ENABLED);
	hDLLMenu = CreateMenu();
	AppendMenu(hDLLMenu, MF_ENABLED|MF_STRING, 0x1000, "Windows 3.1");
	AppendMenu(hDLLMenu, MF_ENABLED|MF_STRING, 0x1001, "Windows 95");
	AppendMenu(hDLLMenu, MF_ENABLED|MF_STRING, 0x1002, "Macintosh");
	AppendMenu(hDLLMenu, MF_ENABLED|MF_STRING, 0x1003, "Motif");

	ModifyMenu(hMenu, WSYS_CONFIGURE,MF_BYCOMMAND|MF_ENABLED|MF_POPUP, hDLLMenu, "Configure...");
	bInit++;
}

/*----------------------------------------------------------------------------*/
void WINAPI
WSysMenu
	( HWND			hWnd )

{
	HMENU			hMenu;
	POINT			CursorPosition;

	if ( ! InSysMenu )
	{
		InSysMenu++;
		hRealWnd = hWnd;

		if ( ! WSysInitialized ) {
			WNDCLASS		WndClass;
			memset ( &WndClass, 0, sizeof ( WndClass ) );

			WndClass.style         = CS_HREDRAW | CS_VREDRAW;	
			WndClass.lpfnWndProc   = WSysWindowProc;
			WndClass.hInstance     = GetModuleHandle ( "USER" );
			WndClass.hCursor       = LoadCursor ( (HINSTANCE) NULL, IDC_ARROW );
			WndClass.hbrBackground = COLOR_BACKGROUND + 1;
			WndClass.lpszClassName = WSYSSTR_WILLOWS_SYSTEM;
			if ( ! RegisterClass ( &WndClass ) )
				return;

			WSysInitialized = TRUE;
		}

		if ( ! hSysWnd )
			if ( ! ( hSysWnd = CreateWindow ( WSYSSTR_WILLOWS_SYSTEM, 
		                                  	WSYSSTR_WILLOWS_SYSTEM,
	                                    	WS_POPUP | WS_CAPTION | WS_SYSMENU, 
		                                  	CW_USEDEFAULT, CW_USEDEFAULT,
	                                    	CW_USEDEFAULT, CW_USEDEFAULT, 
											(HWND)  NULL,
	                                    	(HMENU) NULL, 
											GetModuleHandle ( "USER" ), 
											NULL ) ) )
				return;
	
	
		ShowWindow ( hSysWnd, SW_HIDE );
		UpdateWindow ( hSysWnd );
		if ((hMenu = GetSystemMenu ( hSysWnd, FALSE )))
		{
#ifdef LATER
		    BOOL Iconic;
			Iconic = IsIconic ( hWnd );
			EnableMenuItem ( hMenu, SC_MINIMIZE, 
				MF_BYCOMMAND | ( Iconic ? MF_DISABLED | MF_GRAYED : MF_ENABLED ) );
			EnableMenuItem ( hMenu, SC_RESTORE, 
				MF_BYCOMMAND | ( Iconic ? MF_ENABLED : MF_DISABLED | MF_GRAYED ) );
#endif
			GetCursorPos ( &CursorPosition );

			WSysMenuUpdate(hMenu);

			TrackPopupMenu ( hMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON, 
				CursorPosition.x, CursorPosition.y, 0, hSysWnd, 0 );
		}
		InSysMenu--;
	}
	
	return;
}

int _mfs_regexp ( 
	LPSTR                   lpFileName,
	LPSTR                   lpFilter )

{

	LPSTR                   lpTempFileName = lpFileName;
	LPSTR                   lpTempFilter   = lpFilter;
	char                    TempToken [ 2 ];
	BOOL                    Matched = FALSE;

	
	if ( ( ! lpFileName ) || ( ! *lpFileName ) || 
	     ( ! lpFilter ) || ( ! *lpFilter ) )
		return ( FALSE );

	while ( ( lpTempFilter ) && ( *lpTempFilter ) && ( ! Matched ) )
	{
		memset ( TempToken, 0, sizeof ( TempToken ) );
		switch ( *lpTempFilter )
		{
			default:
				if ( charicmp ( *lpTempFileName, *lpTempFilter ) )
				{
					lpTempFileName = lpFileName;
					if ((lpTempFilter = strpbrk ( lpTempFilter, " ,;" )))
						lpTempFilter++;
				}
				else
				{
					lpTempFilter++;
					lpTempFileName++;
					switch ( *lpTempFilter )
					{
						default:
							break;

						case '\0':
						case ' ':
						case ',':
						case ';':
							if ( ! *lpTempFileName )
								Matched = TRUE;
							break;
					}
				}
				break;

			case '?':
				lpTempFilter++;
				lpTempFileName++;
				break;

			case '*':
				if ( ! ( TempToken [ 0 ] = *( ++lpTempFilter ) ) )
					Matched = TRUE;
				else
				{
					lpTempFilter++;
					while ( ( lpTempFileName = strpbrk ( lpTempFileName, TempToken ) ) &&
					        ( ! Matched ) )
						Matched = _mfs_regexp ( ++lpTempFileName, lpTempFilter );
					if ( ( ! lpTempFileName ) && ( ! Matched ) )
					{
						lpTempFileName = lpFileName;
						if ((lpTempFilter = strpbrk ( lpTempFilter, " ,;" )))
							lpTempFilter++;
					}
				}
				break;

			case '\0':
			case ' ':
			case ',':
			case ';':
				Matched = TRUE;
				break;
		}
	}

	return ( Matched );

}
int charicmp ( char char1, char char2 )
{
	char	Char1 = ( 'a' <= char1 ) && ( char1 <= 'z' ) ? 
	       		char1 - 'a' + 'A' : char1;
	char	Char2 = ( 'a' <= char2 ) && ( char2 <= 'z' ) ? 
	                char2 - 'a' + 'A' : char2;
	return ( Char2 - Char1 );
}
