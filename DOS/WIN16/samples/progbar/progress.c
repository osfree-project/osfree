/********************************************************************
*    PROGRAM: Progress.c
*
*	@(#)progress.c	1.2
*
*	Copyright (c) 1995-1997, Willows Software Inc.  All rights reserved.
*
********************************************************************/

#include "windows.h"    // includes basic windows functionality
#include "commctrl.h"   // includes the common control header
#include <string.h>
#include "progress.h"
#include "WProgBar.h"

HINSTANCE hInst;
HWND hWndMain;
HWND hWndProgress;
UINT uMin, uMax;

/****************************************************************************
*
*    FUNCTION: WinMain(HANDLE, HANDLE, LPSTR, int)
*
*    PURPOSE: calls initialization function, processes message loop
*
****************************************************************************/

int APIENTRY WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow
	)
{

	MSG msg;                       

	InitCommonControls();
	if (!InitApplication(hInstance))
		return (FALSE);     

	/* Perform initializations that apply to a specific instance */
	if (!InitInstance(hInstance, nCmdShow))
		return (FALSE);

	/* Acquire and dispatch messages until a WM_QUIT message is received. */
	while (GetMessage(&msg,
		NULL,              
		0,                 
		0))                
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg); 
	}

	return (msg.wParam);  
												 
}


/****************************************************************************
*
*    FUNCTION: InitApplication(HANDLE)
*
*    PURPOSE: Initializes window data and registers window class
*
****************************************************************************/

BOOL InitApplication(HANDLE hInstance)       /* current instance             */
{
	WNDCLASS  wcProgress;
	
	/* Fill in window class structure with parameters that describe the       */
	/* main window.                                                           */

	wcProgress.style = 0;                     
	wcProgress.lpfnWndProc = (WNDPROC)MainWndProc; 
	wcProgress.cbClsExtra = 0;              
	wcProgress.cbWndExtra = 0;              
	wcProgress.hInstance = hInstance;       
	wcProgress.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(PROGRESS_ICON));
	wcProgress.hCursor = LoadCursor(NULL, IDC_ARROW);
	wcProgress.hbrBackground = GetStockObject(WHITE_BRUSH); 
	wcProgress.lpszMenuName =  "ProgressMenu";  
	wcProgress.lpszClassName = "ProgressWClass";

	return (RegisterClass(&wcProgress));

}


/****************************************************************************
*
*    FUNCTION:  InitInstance(HANDLE, int)
*
*    PURPOSE:  Saves instance handle and creates main window
*
****************************************************************************/

BOOL InitInstance(
	HANDLE          hInstance,
	int             nCmdShow) 
{

	hInst = hInstance;

	hWndMain = CreateWindow(
		"ProgressWClass",           
		"Progress Sample", 
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL,               
		NULL,               
		hInstance,          
		NULL);

	/* If window could not be created, return "failure" */
	if (!hWndMain)
		return (FALSE);

	/* Make the window visible; update its client area; and return "success" */
	ShowWindow(hWndMain, nCmdShow);
	UpdateWindow(hWndMain); 
	return (TRUE);      

}

/****************************************************************************
*
*    FUNCTION: MainWndProc(HWND, unsigned, WORD, LONG)
*
*    PURPOSE:  Processes messages
*
****************************************************************************/

LONG APIENTRY MainWndProc(
	HWND hWnd,                /* window handle                   */
	UINT message,             /* type of message                 */
	UINT wParam,              /* additional information          */
	LONG lParam)              /* additional information          */
{
	HDC hdc;
	PAINTSTRUCT ps;
	static UINT uCurrent;

	switch (message) 
	{

		case WM_CREATE:
			hWndProgress  = CreateWindow(
				WC_PROGRESSBAR,
			//	PROGRESS_CLASS,
				"Position",
				WS_CHILD | WS_VISIBLE,
				10,100,500,20,
				hWnd,         
				(HMENU)2000,         
				hInst,
				NULL);

            if (hWndProgress == NULL)
			{
				MessageBox (NULL, "Progress Bar not created!", NULL, MB_OK );
				break;
			}


			uMin=0;
			uMax=100;
			SendMessage(hWndProgress, PBM_SETRANGE, 0L, MAKELONG(uMin, uMax));
			SendMessage(hWndProgress, PBM_SETSTEP, 1L, 0L);
			break;

		case WM_PAINT:
		{
			hdc = BeginPaint(hWnd, &ps);
			TextOut(hdc, 20, 10, hello, lstrlen(hello));      
			EndPaint(hWnd, &ps);
		}
		break;
 
		case WM_TIMER:
			if (uCurrent < uMax)
			{
				SendMessage(hWndProgress, PBM_STEPIT,0L,0L);
				uCurrent++;
			}
			else
			{
				SendMessage(hWndProgress, PBM_SETPOS,0L,0L);
				KillTimer(hWnd, ID_TIMER);
				uCurrent = 0;
			}
			break;

		case WM_COMMAND:

			switch( LOWORD( wParam ))
			{

				case IDM_STOP:
					SendMessage(hWndProgress, PBM_SETPOS,0L,0L);
					KillTimer(hWnd, ID_TIMER);
					break;

				case IDM_START:
					uCurrent = uMin;
					SetTimer(hWnd, ID_TIMER, 500, NULL);
					break;

				case IDM_SETRANGE:
					DialogBox(hInst, "SetRange", hWnd, (DLGPROC)Range);
					break;

				case IDM_EXIT:
					PostQuitMessage(0);
					break;

				case IDM_ABOUT:
					DialogBox(hInst, "AboutBox", hWnd, (DLGPROC)About);
					break;

				default:
					return (DefWindowProc(hWnd, message, wParam, lParam));

		  }
		  break;

	case WM_DESTROY:                  /* message: window being destroyed */
		  PostQuitMessage(0);
		  break;

	   default:
		  return (DefWindowProc(hWnd, message, wParam, lParam));
	}
	return (0);
}


/****************************************************************************
*
*    FUNCTION: About(HWND, UINT, UINT, LONG)
*
*    PURPOSE:  Processes messages for "About" dialog box
*
****************************************************************************/

BOOL APIENTRY About(
	HWND hDlg,
	UINT message,
	UINT wParam,
	LONG lParam)
{
	switch (message)
	{
		case WM_INITDIALOG:
			return (TRUE);

		case WM_COMMAND:              
			if (LOWORD(wParam) == IDOK)
			{
				EndDialog(hDlg, TRUE);
				return (TRUE);
			}
			break;
	}
	return (FALSE);   

}


/****************************************************************************
* 
*    FUNCTION: Range(HWND, UINT, UINT, LONG)
*
*    PURPOSE:  Processes messages for "SetRange" dialog box
*
****************************************************************************/

BOOL APIENTRY Range(
	HWND hDlg,
	UINT message,
	UINT wParam,
	LONG lParam)
{
	BOOL bErr;

	switch (message)
	{
		case WM_INITDIALOG:
			return (TRUE);

		case WM_COMMAND:              
			if (LOWORD(wParam) == IDOK)
			{
				uMin = GetDlgItemInt(hDlg, IDE_MIN, &bErr, TRUE);
				uMax = GetDlgItemInt(hDlg, IDE_MAX, &bErr, TRUE);
				SendMessage(hWndProgress, PBM_SETRANGE, 0L, MAKELONG(uMin, uMax));
				EndDialog(hDlg, TRUE);
				return (TRUE);
			}
			break;
	}
	return (FALSE);   

}
