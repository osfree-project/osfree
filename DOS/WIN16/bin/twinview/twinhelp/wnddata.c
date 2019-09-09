/***********************************************************************

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

***********************************************************************/
/***********************************
**
**  System Includes
**
***********************************/
#include <windows.h>


/***********************************
**
**  App. Includes
**
***********************************/
#include "wnddata.h"
#include "fontbuff.h"
#include "winmem.h"
#include "macroeng.h"
#include "filebuff.h"
#include "accel.h"


/* 
** Main window's window data structure. 
*/
typedef struct tagMAINWNDDATA
{
  HTOPICDATA hTopicData;       /* Topic data for main topic window. */
  HWND hQuitWnd;               /* Handle to the parent window. */
  HWND hTopicWnd;              /* Main topic window handle. - Used by Secondary Wnd & Main Wnd. */
  HWND hButtonBarWnd;          /* Button bar window handle. */
  HWND hHistoryWnd;            /* History window handle. */
  HGLOBAL hBackBtnData;        /* Handle to the "Back" button data. */
  HGLOBAL hHelpFileBuffer;     /* Handle to buffer of help file data. */
  long int dwNextTopicCharOffset; /* Browse button next topic. */
  long int dwPrevTopicCharOffset; /* Browse button previous topic. */
  HGLOBAL hSecWndList;         /* Active secondary window list. */
  HMACROENGINE hMacroEngine;   /* Macro engine instance handle. */
  BOOL bSearchDlgDisplayed;    /* Is the Search dialog already being displayed. */ 
  HWND hDataWnd;               /* Handle to the window that keeps the data. - Used by Secondary Wnd. Only. */
  HACCELSYS hAccelSys;         /* Handle to accelerator support system. */
}
MAINWNDDATA;
typedef MAINWNDDATA __far * FPMAINWNDDATA;


/* 
** Child window's window data structure. 
*/
typedef struct tagCHILDWNDDATA
{
  HWND        hDataWnd;          /* Handle to the window that keeps the data. */
  HWND        hParentWnd;        /* Handle of the window's parent window. */
  HWND        hNoScrollWnd;      /* Handle to the non-scrollable topic window. */
  HGLOBAL     hDisplayInfo;      /* Display info. for window. */
  HFONTBUFFER hFontBuffer;       /* Handle to the window's font buffer. */
  RECT        HotSpotPosRec;     /* Screen position of a hotspot. */
  BOOL        bAmPainting;       /* Am I painting the window currently. */
  RECT        ClientPosRect;     /* Position of window in the parent's client window. */
}
CHILDWNDDATA;
typedef CHILDWNDDATA __far * FPCHILDWNDDATA;


/* 
** Shared window EXTRABYTES structure. 
*/
typedef union tagWNDDATA
{
  CHILDWNDDATA ChildWndData;
  MAINWNDDATA MainWndData;
}
WNDDATA;
typedef WNDDATA __far * FPWNDDATA;
typedef HGLOBAL HWNDDATA;



DWORD __far __pascal WndData( HWND hWnd, WNDDATAMSG Msg, DWORD dwParam, void __far * fpDataBuffer )
{
  RECT __far * fpRect;
  FPWNDDATA fpWndData;
  HWNDDATA hWndData;
  DWORD dwReturn;
  
  switch( Msg )
  {
    /******************* CHILDWNDDATA structure messages.  *******************/


    /* Am I painting the window currently. */
    case WDM_SETAMPAINTING: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      fpWndData->ChildWndData.bAmPainting = ( BOOL ) dwParam;
      GlobalUnlock( hWndData );
      
      return 0;
    }

    case WDM_GETAMPAINTING: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      dwReturn = (DWORD) fpWndData->ChildWndData.bAmPainting;
      GlobalUnlock( hWndData );
      return( dwReturn );
    }


    /*  The screen position of a hotspot. */
    case WDM_SETHOTSPOTPOS: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      fpRect = ( RECT __far * ) fpDataBuffer;
      fpWndData->ChildWndData.HotSpotPosRec = *fpRect;
      GlobalUnlock( hWndData );
      
      return 0;
    }

    case WDM_GETHOTSPOTPOS: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      fpRect = ( RECT __far * ) fpDataBuffer;
      *fpRect = fpWndData->ChildWndData.HotSpotPosRec;
      GlobalUnlock( hWndData );
      return( dwReturn );
    }


    /*  The display data for window. */
    case WDM_SETDISPLAYINFO: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      fpWndData->ChildWndData.hDisplayInfo = ( HGLOBAL ) dwParam;
      GlobalUnlock( hWndData );
      
      return 0;
    }

    case WDM_GETDISPLAYINFO: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      dwReturn = (DWORD) fpWndData->ChildWndData.hDisplayInfo;
      GlobalUnlock( hWndData );
      return( dwReturn );
    }


    /* Handle to the window that keeps the data. */
    case WDM_SETDATAHWND: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      fpWndData->ChildWndData.hDataWnd = ( HWND ) dwParam;
      GlobalUnlock( hWndData );
      return 0;
    }
    case WDM_GETDATAHWND: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      dwReturn = (DWORD) fpWndData->ChildWndData.hDataWnd;
      GlobalUnlock( hWndData );
      return( dwReturn );
    }


    /* Handle of the window's parent window. */
    case WDM_SETPARENTHWND: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      fpWndData->ChildWndData.hParentWnd = ( HWND ) dwParam;
      GlobalUnlock( hWndData );
      return 0;
    }
    case WDM_GETPARENTHWND: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      dwReturn = (DWORD) fpWndData->ChildWndData.hParentWnd;
      GlobalUnlock( hWndData );
      return( dwReturn );
    }


    /* Handle to the non-scrollable topic window. */
    case WDM_SETNOSCROLLHWND: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      fpWndData->ChildWndData.hNoScrollWnd = ( HWND ) dwParam;
      GlobalUnlock( hWndData );
      return 0;
    }
    case WDM_GETNOSCROLLHWND: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      dwReturn = (DWORD) fpWndData->ChildWndData.hNoScrollWnd;
      GlobalUnlock( hWndData );
      return( dwReturn );
    }


    /* Position of window in client area. */
    case WDM_SETCLIENTPOS: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      fpRect = ( RECT __far * ) fpDataBuffer;
      fpWndData->ChildWndData.ClientPosRect = *fpRect;
      GlobalUnlock( hWndData );
      return 0;
    }
    case WDM_GETCLIENTPOS: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      fpRect = ( RECT __far * ) fpDataBuffer;
      *fpRect = fpWndData->ChildWndData.ClientPosRect;
      GlobalUnlock( hWndData );
      return( dwReturn );
    }


    /******************* MAINWNDDATA structure messages.  *******************/
    
    /* Handle to accelerator support system. */
    case WDM_SETACCELSYS: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      fpWndData->MainWndData.hAccelSys = ( HACCELSYS ) dwParam;
      GlobalUnlock( hWndData );
      return 0;
    }
    case WDM_GETACCELSYS: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      dwReturn = (DWORD) fpWndData->MainWndData.hAccelSys;
      GlobalUnlock( hWndData );
      return( dwReturn );
    }


    /* Handle to the window that keeps the data. */
    case WDM_SETMAINDATAHWND: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      fpWndData->MainWndData.hDataWnd = ( HWND ) dwParam;
      GlobalUnlock( hWndData );
      return 0;
    }
    case WDM_GETMAINDATAHWND: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      dwReturn = (DWORD) fpWndData->MainWndData.hDataWnd;
      GlobalUnlock( hWndData );
      return( dwReturn );
    }


    /* Is the Search dialog already being displayed. */
    case WDM_SETSEARCHDLGDISPLAYED: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      fpWndData->MainWndData.bSearchDlgDisplayed = ( BOOL ) dwParam;
      GlobalUnlock( hWndData );
      return 0;
    }
    case WDM_GETSEARCHDLGDISPLAYED: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      dwReturn = (DWORD) fpWndData->MainWndData.bSearchDlgDisplayed;
      GlobalUnlock( hWndData );
      return( dwReturn );
    }


    /* Topic data for main topic window. */
    case WDM_SETMAINTOPICDATA: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      fpWndData->MainWndData.hTopicData = ( HTOPICDATA ) dwParam;
      GlobalUnlock( hWndData );
      return 0;
    }
    case WDM_GETMAINTOPICDATA: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      dwReturn = (DWORD) fpWndData->MainWndData.hTopicData;
      GlobalUnlock( hWndData );
      return( dwReturn );
    }


    /* Active secondary window list. */
    case WDM_SETSECWNDDATA: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      fpWndData->MainWndData.hSecWndList = ( HGLOBAL ) dwParam;
      GlobalUnlock( hWndData );
      return 0;
    }
    case WDM_GETSECWNDDATA: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      dwReturn = (DWORD) fpWndData->MainWndData.hSecWndList;
      GlobalUnlock( hWndData );
      return( dwReturn );
    }


    /* Handle to the "Back" button data. */
    case WDM_SETBACKDATA: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      fpWndData->MainWndData.hBackBtnData = ( HGLOBAL ) dwParam;
      GlobalUnlock( hWndData );
      return 0;
    }
    case WDM_GETBACKDATA: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      dwReturn = (DWORD) fpWndData->MainWndData.hBackBtnData;
      GlobalUnlock( hWndData );
      return( dwReturn );
    }


    /* Browse next topic offset. */
    case WDM_SETBROWSENEXTDATA: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      fpWndData->MainWndData.dwNextTopicCharOffset = ( long int ) dwParam;
      GlobalUnlock( hWndData );
      return 0;
    }
    case WDM_GETBROWSENEXTDATA: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      dwReturn = ( long int ) fpWndData->MainWndData.dwNextTopicCharOffset;
      GlobalUnlock( hWndData );
      return( dwReturn );
    }


    /* Browse prev. topic offset. */
    case WDM_SETBROWSEPREVDATA: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      fpWndData->MainWndData.dwPrevTopicCharOffset = ( long int ) dwParam;
      GlobalUnlock( hWndData );
      return 0;
    }
    case WDM_GETBROWSEPREVDATA: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      dwReturn = ( long int ) fpWndData->MainWndData.dwPrevTopicCharOffset;
      GlobalUnlock( hWndData );
      return( dwReturn );
    }


    /* Help file/topic information buffer. */
    case WDM_SETFILEBUFFER: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      fpWndData->MainWndData.hHelpFileBuffer = ( HGLOBAL ) dwParam;
      GlobalUnlock( hWndData );
      return 0;
    }
    case WDM_GETFILEBUFFER: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      dwReturn = (DWORD) fpWndData->MainWndData.hHelpFileBuffer;
      GlobalUnlock( hWndData );
      return( dwReturn );
    }

    /* History window's HWND. */
    case WDM_SETHISTORYHWND: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      fpWndData->MainWndData.hHistoryWnd = ( HWND ) dwParam;
      GlobalUnlock( hWndData );
      return 0;
    }
    case WDM_GETHISTORYHWND: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      dwReturn = (DWORD) fpWndData->MainWndData.hHistoryWnd;
      GlobalUnlock( hWndData );
      return( dwReturn );
    }


    /*  Main topic window's HWND. */
    case WDM_SETMAINTOPICHWND: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      fpWndData->MainWndData.hTopicWnd = ( HWND ) dwParam;
      GlobalUnlock( hWndData );
      return 0;
    }
    case WDM_GETMAINTOPICHWND: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      dwReturn = (DWORD) fpWndData->MainWndData.hTopicWnd;
      GlobalUnlock( hWndData );
      return( dwReturn );
    }

    /*  Button bar window's HWND. */
    case WDM_SETBUTTONBARHWND: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      fpWndData->MainWndData.hButtonBarWnd = ( HWND ) dwParam;
      GlobalUnlock( hWndData );
      return 0;
    }
    case WDM_GETBUTTONBARHWND: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      dwReturn = (DWORD) fpWndData->MainWndData.hButtonBarWnd;
      GlobalUnlock( hWndData );
      return( dwReturn );
    }


    /*  Window to notify when quitting. */
    case WDM_SETQUITHWND: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      fpWndData->MainWndData.hQuitWnd = ( HWND ) dwParam;
      GlobalUnlock( hWndData );
      return 0;
    }
    case WDM_GETQUITHWND: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      dwReturn = (DWORD) fpWndData->MainWndData.hQuitWnd;
      GlobalUnlock( hWndData );
      return( dwReturn );
    }


    /*  Macro engine instance handle. */
    case WDM_SETMACROENGINE: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      fpWndData->MainWndData.hMacroEngine = ( HMACROENGINE ) dwParam;
      GlobalUnlock( hWndData );
      return 0;
    }
    case WDM_GETMACROENGINE: 
    {
      hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
      fpWndData = ( FPWNDDATA ) GlobalLock( hWndData );
      dwReturn = (DWORD) fpWndData->MainWndData.hMacroEngine;
      GlobalUnlock( hWndData );
      return( dwReturn );
    }
  }
  
  /* If message doesn't return a value - return 0. */
  return 0;
}


/* 
** Allocate memory space for the child window's data. 
*/
BOOL __far __pascal AllocChildWndData( HWND hWnd )
{
  HWNDDATA hWndData;
  
  /* Allocate memory. */
  if( ! GlobalAllocMem( hWnd, &hWndData, sizeof( WNDDATA ) ) ) return FALSE;
  
  /* Save handle in the window's extrabyte. */
  SetWindowWord( hWnd, 0, (WORD) hWndData );

  /* Success. */
  return( TRUE );
}



/* 
** Free memory space for the child window's data. 
*/
void __far __pascal FreeChildWndData( HWND hWnd )
{
  HWNDDATA hWndData;

  
  /* get window data handle. */
  hWndData = (HWNDDATA) GetWindowWord( hWnd, 0 );
  
  /* Free memory. */
  GlobalFree( hWndData );
  
  /* Save handle in the window's extrabyte. */
  SetWindowWord( hWnd, 0, 0 );
}



