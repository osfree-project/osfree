/*    
	Clipboard.c	2.23
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

#include <string.h>
#include "windows.h"
#include "GdiObjects.h"
#include "Clipboard.h"
#include "WinDefs.h"
#include "Driver.h"
#include "Log.h"

BOOL TWIN_InternalClipboard(DWORD, DWORD);

/* all clipboard data is contained here */
static DRVCLIPINFO drvci;
static CLIPTABLE   cb = { 0, 0, 0, 0, 0, &drvci };

/***************************************************************************/
BOOL TWIN_InternalClipboard(DWORD clipchange, DWORD cliptime)
{
    register  i;
    HWND      hWnd;

    switch(clipchange)
    {
    case 0L:
       hWnd = GetClipboardOwner();
       if (IsWindow(hWnd))
          SendMessage(hWnd, WM_DESTROYCLIPBOARD, 0, 0L);
       cb.lpDrvData->ClipboardTime = cliptime;
       break;

    case 1L:
       cb.lpDrvData->CutBufferTime = cb.hwndOpen ? 0 : cliptime;
       break;
 
    case 2L:
       for (i = 0;   i < cb.ViewerCount;   i++)
           {
           if (cb.ViewerList[i])
              SendMessage(cb.ViewerList[i], WM_DRAWCLIPBOARD, 0, 0L);
           }
       break;
    } 
    return 1;
}


/***************************************************************************/
BOOL WINAPI
OpenClipboard(HWND hWnd)
{
    BOOL  ynSuccess; 
    DWORD win = WIN_GETDRVDATA(TWIN_GetTopLevelFrame(hWnd));

    APISTR((LF_APICALL,"OpenClipboard(HWND=%x)\n", hWnd));

    if (cb.hwndOwner) 
       {
       APISTR((LF_APIFAIL,"OpenClipboard: returning BOOL FALSE\n"));
       return FALSE;
       }

    if (!win || !hWnd || !IsWindow(hWnd)) 
        {
        APISTR((LF_APIFAIL,"OpenClipboard: returning BOOL FALSE\n"));
	return FALSE;
        }

    cb.hwndOwner = cb.hwndOpen = hWnd;
    ynSuccess = (BOOL)DRVCALL_IPC(IPCH_OPENCLIPBRD, 0L, win, cb.lpDrvData);

    if (!ynSuccess)
       cb.hwndOwner = cb.hwndOpen = 0;

    APISTR((LF_APIRET,"OpenClipboard: returning BOOL %d\n",ynSuccess));
    return ynSuccess;   
}

/****************************************************************************/
BOOL WINAPI
CloseClipboard(void)
{
    APISTR((LF_APICALL,"CloseClipboard()\n"));
    /*
    **   Release all X related stuff first...
    */
    DRVCALL_IPC(IPCH_CLOSECLIPBRD, 0L, 0L, cb.lpDrvData);
    cb.hwndOwner = 0;
    cb.hwndOpen  = 0;
    APISTR((LF_APIRET,"CloseClipboard: return BOOL TRUE.\n"));
    return TRUE;
 
}

/****************************************************************************/
BOOL WINAPI
EmptyClipboard(void)
{
    HWND   hWnd;
    BOOL   rc = 0;

    APISTR((LF_APICALL,"EmptyClipboard()\n"));
    /*
    **  Do we have it open? 
    **  we could read the format to verify it is us...      
    */
    if (cb.hwndOpen) 
       {
       hWnd = GetClipboardOwner();
       if (IsWindow(hWnd))
          SendMessage(hWnd, WM_DESTROYCLIPBOARD, 0, 0L);
       rc = (BOOL)DRVCALL_IPC(IPCH_EMPTYCLIPBRD, 0L, 0L, cb.lpDrvData);
       }
    APISTR((LF_APIRET,"EmptyClipboard: return BOOL %d.\n",rc));
    return 0;
}

/****************************************************************************/
HWND WINAPI
GetOpenClipboardWindow(void)
{
    APISTR((LF_APICALL,"GetOpenClipboardWindow()\n"));
    APISTR((LF_APIRET,"GetOpenClipboardWindow: returning HWND %x\n",
			cb.hwndOpen));
    return cb.hwndOpen;
}

/****************************************************************************/
HWND WINAPI
GetClipboardOwner(void)
{
    APISTR((LF_APICALL,"GetOpenClipboardWindow()\n"));
    APISTR((LF_APIRET,"GetOpenClipboardWindow: returning HWND %x\n",
			cb.hwndOwner));
    return cb.hwndOwner;
}

/****************************************************************************/
HWND WINAPI
GetClipboardViewer(void)
{
        register  i;

        APISTR((LF_APICALL,"GetClipboardViewer()\n"));
        if (cb.ViewerList) 
           {
           for (i = 0;   i < cb.ViewerCount;   i++)
                if (cb.ViewerList[i]) {
        	  APISTR((LF_APIRET,"GetClipboardViewer: returns HWND %x\n",
			cb.ViewerList[i]));
                  return cb.ViewerList[i];
	 	}
           }
        APISTR((LF_APIRET,"GetClipboardViewer: returns HWND 0\n"));
        return (HWND)NULL;
}

/****************************************************************************/
HWND WINAPI
SetClipboardViewer(HWND hWnd)
{
        register  i;
        APISTR((LF_APICALL,"SetClipboardViewer(HWND=%x)\n",hWnd));
 
        /* try to insert into a list if we have one */
        if (cb.ViewerList) 
           {
           for (i = 0;   i < cb.ViewerCount;   i++) 
               {
               if (cb.ViewerList[i])
                  continue;
                /* we have a slot */
                cb.ViewerList[i] = hWnd;
        	APISTR((LF_APIRET,"SetClipboardViewer: %x\n",hWnd));
                return 0;
               }
           }
        /* the remainder of these are guaranteed to be at end of list */
        if (cb.ViewerList == 0) 
           {
           /* no list so allocate one */
           cb.ViewerList = (HWND *) WinMalloc(sizeof(HWND));
           } 
        else {
             /* expand the current list */
             cb.ViewerList = (HWND *) WinRealloc((char *)cb.ViewerList,
                                                 (cb.ViewerCount+1)*sizeof(HWND));
             }
        /* fill in the new slot */
        cb.ViewerList[cb.ViewerCount++] = hWnd;

        APISTR((LF_APIRET,"SetClipboardViewer: return HWND NULL\n"));
        return (HWND)NULL;
}

/****************************************************************************/
HANDLE WINAPI
GetClipboardData(UINT uiFormat)
{
    GETSETDATA gd;
    HANDLE     hMem   = (HANDLE)0;
    LPSTR      lpData = NULL;

    APISTR((LF_APICALL,"GetClipboardData(format=%x)\n", uiFormat ));

    gd.uiFormat = uiFormat;
    gd.dwSize   = 0L;
    gd.lpMem    = NULL;

    if (DRVCALL_IPC(IPCH_GETCLIPBRDDATA, 0, &gd, cb.lpDrvData) == 0L) {
       APISTR((LF_APIFAIL,"GetClipboardData: returning HANDLE 0\n"));
       return 0;
    }

    if (uiFormat == CF_BITMAP)
       {
       HBITMAP hbmp;
       LPIMAGEINFO lpii;
 
       if (cb.hbmpClip)
          DeleteObject(cb.hbmpClip);

       lpii = CREATEHBITMAP(hbmp);
       hMem = cb.hbmpClip = hbmp;
       memcpy(gd.lpMem, (LPSTR)lpii, sizeof(OBJHEAD));
       memcpy((LPSTR)lpii, gd.lpMem, sizeof(IMAGEINFO));
       RELEASEBITMAPINFO(lpii);
       }
    else {           
         hMem = GlobalAlloc(GHND, gd.dwSize);
         if (hMem)
         {
              lpData = GlobalLock(hMem); 
              memcpy(lpData, gd.lpMem, gd.dwSize);
              GlobalUnlock(hMem);
              
         }
    }
    WinFree(gd.lpMem);
    APISTR((LF_APIRET,"GetClipboardData: returning HANDLE %x\n",hMem));
    return hMem;
}

/****************************************************************************/
HANDLE WINAPI
SetClipboardData(UINT uiFormat, HANDLE hData)
{
    register     i;
    GETSETDATA   sd;

    if (cb.hwndOpen)
       {
       /*   If hData is NULL, we send WM_RENDERFORMAT to clipboard owner...
       */
       if (!hData)
          SendMessage(cb.hwndOwner, WM_RENDERFORMAT, uiFormat, 0L);
       else {
            sd.uiFormat = uiFormat;
            if (sd.uiFormat != CF_BITMAP)
               {
               sd.dwSize = GlobalSize(hData);
               sd.lpMem = GlobalLock(hData);
		
 	       /* for text, cut the length to the ascii string length */	
	       if(sd.uiFormat == CF_TEXT) 
	           sd.dwSize = strlen(sd.lpMem);

               DRVCALL_IPC(IPCH_SETCLIPBRDDATA, 0, &sd, cb.lpDrvData);
               GlobalUnlock(hData);
               }
            else {
                 LPIMAGEINFO lpim;

                 lpim = GETBITMAPINFO(hData);
                 if (!lpim)
                     return 0;
                 sd.lpMem = (LPSTR)lpim->lpDrvData;
                 DRVCALL_IPC(IPCH_SETCLIPBRDDATA, 0, &sd, cb.lpDrvData);
		 RELEASEBITMAPINFO(lpim);
                 }
            /*
            **   Force a reload of formats and data...
            */
            for (i = 0;   i < cb.ViewerCount;   i++)
                {
                if (cb.ViewerList[i])
                   SendMessage(cb.ViewerList[i], WM_DRAWCLIPBOARD, 0, 0L);
                }
            return hData;
            }
      }
    return 0;
}

/****************************************************************************/
BOOL WINAPI
IsClipboardFormatAvailable(UINT uiFormat)
{
        BOOL ynIs = DRVCALL_IPC(IPCH_ISFORMATAVAIL, uiFormat, 0L, 0L);

        APISTR((LF_APICALL,"IsClipboardFormatAvailable(UINT=%x)\n",uiFormat));

        APISTR((LF_APIRET,"IsClipboardFormatAvailable: returns BOOL %d\n",ynIs));
        return ynIs;
}

/****************************************************************************/
int WINAPI
GetPriorityClipboardFormat(UINT *lpuiPriorityList, int cEntries)
{
        int nitems;
 
        APISTR((LF_APICALL,"GetPriorityClipboardFormat(UINT *=%p,int=%x)\n",
		lpuiPriorityList,cEntries));
 
        nitems = CountClipboardFormats();
        if (nitems == 0) {
           APISTR((LF_APIRET,"GetPriorityClipboardFormat: returns int 0\n"));
           return 0;
	}

        APISTR((LF_APIRET,"GetPriorityClipboardFormat: returns int -1\n"));
        return -1;
}


/****************************************************************************/
int WINAPI
CountClipboardFormats(void)
{
	int rc;
        APISTR((LF_APICALL,"CountClipboardFormats()\n"));
        rc = (int)DRVCALL_IPC(IPCH_LOADFORMATS, 0L, 0L, cb.lpDrvData);
        APISTR((LF_APIRET,"CountClipboardFormats: returns int %d\n",rc));
	return rc;
}

/****************************************************************************/
UINT WINAPI
EnumClipboardFormats(UINT uiFormat)
{
	UINT rc;
        APISTR((LF_APICALL,"EnumClipboardFormats(UINT=%x)\n", uiFormat));
        rc = cb.hwndOpen ? (UINT)DRVCALL_IPC(IPCH_ENUMFORMATS, uiFormat, 0L, 0L) : 0;
        APISTR((LF_APIRET,"EnumClipboardFormats: returns UINT %x\n", rc));
	return rc;
}

/****************************************************************************/
int WINAPI
GetClipboardFormatName(UINT uFormat, LPSTR lpszFormatName, int cbMax)
{
    int len;

    APISTR((LF_APICALL,"GetClipboardFormatName(UINT=%x,LPSTR=%s,int=%d)\n",
	uFormat,lpszFormatName,cbMax));

    if (uFormat < 0xCC00) {
       APISTR((LF_APIRET,"GetClipboardFormatName: returns int %d\n",0));
       return 0; /* predefined formats don't have names */
    }

    len = GlobalGetAtomName(uFormat,lpszFormatName,cbMax);

    APISTR((LF_APIRET,"GetClipboardFormatName: returns int %d\n",len));
    return len;
}

/****************************************************************************/
BOOL WINAPI
ChangeClipboardChain(HWND hWnd, HWND hWndNext)
{
	register  i;
        APISTR((LF_APICALL,"ChangeClipboardViewer(HWND=%x,HWND=%x)\n",
		hWnd,hWndNext));
 
        if (cb.ViewerList) 
           {
           for (i = 0;   i < cb.ViewerCount;   i++) 
               {
               if (cb.ViewerList[i] == hWnd) 
                  {
                  cb.ViewerList[i] = 0;
        	  APISTR((LF_APIRET,"ChangeClipboardViewer: returns BOOL %d\n",
			TRUE));
                  return TRUE;
                  }
               }
           }
        APISTR((LF_APIRET,"ChangeClipboardViewer: returns BOOL FALSE\n"));
        return FALSE;
}

/****************************************************************************/
UINT WINAPI
RegisterClipboardFormat(LPCSTR lpszFormat)
{
    UINT uiAtom;

    APISTR((LF_APICALL,"RegisterClipboardFormat(LPCSTR=%s)\n",lpszFormat));
    uiAtom = GlobalAddAtom(lpszFormat);
    /*call driver to add new format to FormatData property under unique ID */

    DRVCALL_IPC(IPCH_REGISTERFORMAT, 0L, uiAtom, lpszFormat);
    APISTR((LF_APIRET,"RegisterClipboard: return UINT %x.\n",uiAtom));
    return uiAtom;
}

