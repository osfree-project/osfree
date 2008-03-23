/*
	@(#)DrvIPC.c	1.24
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

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/Xatom.h>
#include "Driver.h"

#include "Clipboard.h"
#include "KrnAtoms.h"
#include "DrvDP.h"		/* PRIVATEDISPLAY */
#include "DrvImages.h"
#include "DrvIPC.h"
#include "Log.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

DWORD DrvIPCInit(LPARAM,LPARAM,LPVOID);
DWORD DrvIPCTab(void);
static DWORD DrvIPCDoNothing(LPARAM,LPARAM,LPVOID);
static BOOL DrvInitIPC(void);
static DWORD DrvInitAtomTable(LPARAM,LPARAM,LPVOID);
static DWORD DrvAtomData(LPARAM,LPARAM,LPVOID);
static DWORD DrvClipbrdClose(LPARAM,LPARAM,LPVOID);
static DWORD DrvClipbrdEmpty(LPARAM,LPARAM,LPVOID);
static DWORD DrvClipbrdGetData(LPARAM,LPARAM,LPVOID);
static DWORD DrvClipbrdOpen(LPARAM,LPARAM,LPVOID);
static DWORD DrvClipbrdSetData(LPARAM,LPARAM,LPVOID);
static DWORD DrvEnumFormats(LPARAM,LPARAM,LPVOID);
static DWORD DrvIsFormatAvailable(LPARAM,LPARAM,LPVOID);
static DWORD DrvLoadFormats(LPARAM,LPARAM,LPVOID);
static DWORD DrvRegisterFormat(LPARAM,LPARAM,LPVOID);

static TWINDRVSUBPROC DrvIPCEntryTab[] = {
	DrvIPCInit,
	DrvIPCDoNothing,
	DrvIPCDoNothing,
	DrvInitAtomTable,
	DrvAtomData,
	DrvClipbrdClose,
	DrvClipbrdEmpty,
	DrvClipbrdGetData,
	DrvClipbrdOpen,
	DrvClipbrdSetData,
	DrvLoadFormats,
	DrvEnumFormats,
	DrvIsFormatAvailable,
	DrvRegisterFormat
};
	
/*
**   How do we talk to library...
*/
extern TWINLIBCALLBACKPROC lpLibCallback;
 
DWORD DrvDestroyImage(LPARAM, LPARAM, LPVOID);
void DrvCopyPixmap(LPDRVIMAGEDATA);
LPBYTE DrvFillImageInfo(LPDRVIMAGEDATA);

static int          nAllFormats = 4;     /* CF_TEXT CF_BITMAP CF_DIB CF_METAFILEPICT so far... */
static LPCLIPENTRY  lpAllFormats = NULL;

/****************************************************************************/
int
InternalClipboard(PRIVATEDISPLAY *dp, Window wid, Atom atm, Time t, int state)
{
        int clipchange = 0;
 
        if (atm == dp->XFormatAtom)
           {
           /*   If our clipboard is emptied, then revert to X
           */
           if (state == PropertyDelete)
              t = 1;
           lpLibCallback(TWINLIBCALLBACK_CLIPBOARD, 0, t, 0); 
           clipchange++;
           }
        if (atm == lpAllFormats[0].atom)
           {
           /*   Clipboard is open - ignore other call
           **   should add if owned by us...
           **   we won't change what someone started
           */
           lpLibCallback(TWINLIBCALLBACK_CLIPBOARD, 1, t, 0); 
           clipchange++;
           }
        if (clipchange)
           lpLibCallback(TWINLIBCALLBACK_CLIPBOARD, 2, 0, 0);
 
        return clipchange;
}

DWORD
DrvIPCInit(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	if (dwParm2)
		DrvInitIPC();
	else
		if (lpAllFormats)
			WinFree((LPSTR)lpAllFormats);
	return 1L;
}
		
/****************************************************************************/
DWORD
DrvIPCTab(void)
{
#if defined(TWIN_RUNTIME_DRVTAB)
        DrvIPCEntryTab[0] = DrvIPCInit;
        DrvIPCEntryTab[1] = DrvIPCDoNothing;
        DrvIPCEntryTab[2] = DrvIPCDoNothing;
        DrvIPCEntryTab[3] = DrvInitAtomTable;
        DrvIPCEntryTab[4] = DrvAtomData;
        DrvIPCEntryTab[5] = DrvClipbrdClose;
        DrvIPCEntryTab[6] = DrvClipbrdEmpty;
        DrvIPCEntryTab[7] = DrvClipbrdGetData;
        DrvIPCEntryTab[8] = DrvClipbrdOpen;
        DrvIPCEntryTab[9] = DrvClipbrdSetData;
        DrvIPCEntryTab[10] = DrvLoadFormats;
        DrvIPCEntryTab[11] = DrvEnumFormats;
        DrvIPCEntryTab[12] = DrvIsFormatAvailable;
        DrvIPCEntryTab[13] = DrvRegisterFormat;
#endif
	return (DWORD)DrvIPCEntryTab;
}

static BOOL
DrvInitIPC(void)
{
    PRIVATEDISPLAY *dp = GETDP();

    /* initialize all atoms */
    dp->wm_protocols     = XInternAtom(dp->display,"WM_PROTOCOLS",True);
    dp->wm_delete_window = XInternAtom(dp->display,"WM_DELETE_WINDOW",True);
    dp->ipc_protocols    = XInternAtom(dp->display,"IPC_PROTOCOLS",True);

    dp->XFormatAtom = XInternAtom(dp->display,"ClipboardFormat",False);
    lpAllFormats = (LPCLIPENTRY)WinMalloc(nAllFormats * sizeof(CLIPENTRY));
    memset((char *)lpAllFormats, '\0', nAllFormats * sizeof(CLIPENTRY));
    lpAllFormats[0].wFormat = CF_TEXT;
    lpAllFormats[0].atom = (DWORD)XInternAtom(dp->display, "CUT_BUFFER0", False);
    lpAllFormats[1].wFormat = CF_BITMAP;
    lpAllFormats[1].atom = (DWORD)XInternAtom(dp->display, "TWIN_CF_BITMAP", False);
    lpAllFormats[2].wFormat = CF_DIB;
    lpAllFormats[2].atom = (DWORD)XInternAtom(dp->display, "TWIN_CF_DIB", False);
    lpAllFormats[3].wFormat = CF_METAFILEPICT;
    lpAllFormats[3].atom = (DWORD)XInternAtom(dp->display, "TWIN_CF_METAFILEPICT", False);
    XChangeProperty(dp->display,
		RootWindow(dp->display,dp->screen),
               	dp->XFormatAtom,XA_STRING,
               	8,PropModeReplace, (LPBYTE)lpAllFormats, 
		nAllFormats * sizeof(CLIPENTRY));
    return TRUE;
}

/****************************************************************************/
static DWORD
DrvClipbrdClose(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
       	LPDRVCLIPINFO lpct = (LPDRVCLIPINFO)lpStruct;
        PRIVATEDISPLAY *dp = GETDP();
 
	if (lpct->winOwner)
	   {
	   XSetSelectionOwner(dp->display, dp->XFormatAtom, 0, 0);
	   lpct->winOwner = 0;
	   }
        return 1L;
}

/****************************************************************************/
static DWORD
DrvClipbrdEmpty(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
        LPDRVCLIPINFO lpct = (LPDRVCLIPINFO)lpStruct;
        register        i;
        LPCLIPENTRY     lpce;
        Atom            actual_type;
        int             actual_format;
        unsigned long   remainder, nitems;
        PRIVATEDISPLAY *dp = GETDP();
 
        XGetWindowProperty(dp->display,
                        RootWindow(dp->display,dp->screen),
                        dp->XFormatAtom,0,8192,FALSE,XA_STRING,
                        &actual_type, &actual_format,&nitems,
                        &remainder, (unsigned char **)&lpce);
        if (lpce != NULL)
           {
           for (i = 0;   i < nitems / sizeof(CLIPENTRY);   i++)
               {
               lpAllFormats[i].offset = lpce[i].offset = 0;
               if (lpce[i].atom && lpce[i].wFormat != CF_TEXT)
                  {
                  lpAllFormats[i].length = lpce[i].length = 0;
                  XDeleteProperty(dp->display,
                                  RootWindow(dp->display,dp->screen),
                                  lpce[i].atom);
                  }
               }
           XChangeProperty(dp->display,
                           RootWindow(dp->display,dp->screen),
                           dp->XFormatAtom,XA_STRING,
                           8,PropModeReplace, (LPBYTE)lpce, nitems);
           XFree((void *)lpce);
           }
           /* this will free any data we have cached */
           /* and set the format count to 0          */
           lpct->ClipboardTime = 1;
           lpct->CutBufferTime = 0;
           return TRUE;
}

/****************************************************************************/
static DWORD
DrvClipbrdGetData(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
        LPGETSETDATA lpgd = (LPGETSETDATA)dwParm2;
        register           i;
	PRIVATEDISPLAY    *dp = GETDP();
        LPCLIPENTRY        lpce, lpceTop;
        LPBYTE             lpdata;
        Atom               actual_type;
        int                actual_format;
        unsigned long      remainder;
        long               DataLength;
        unsigned long      nitems, nFormats;

        XGetWindowProperty(dp->display,
                        RootWindow(dp->display,dp->screen),
                        dp->XFormatAtom,0,8192,FALSE,XA_STRING,
                        &actual_type, &actual_format,&nitems,
                        &remainder, (unsigned char **) &lpceTop);

        nFormats = nitems / sizeof(CLIPENTRY);
        for (i = 0, lpce = lpceTop;   i < nFormats;  i++, lpce++)
           {
           if (lpce->wFormat == lpgd->uiFormat)
              break;
           }

        if (i == nFormats || lpce->wFormat != lpgd->uiFormat)
           {
           XFree((LPSTR)lpceTop);
           return 0L;
           }

        if (lpgd->uiFormat != CF_TEXT)
           	DataLength = lpce->length;
        else 
		DataLength = 8192;

        XGetWindowProperty(dp->display,
                        RootWindow(dp->display,dp->screen),
                        lpce->atom, 0, DataLength,
                        FALSE,XA_STRING,&actual_type,
                        &actual_format,&nitems,&remainder,
                        (unsigned char **)&lpdata);
 
        if (nitems == 0) 
           {
           return (HANDLE) NULL;
           }

        if (lpgd->uiFormat == CF_BITMAP)
           {
           LPDRVIMAGEDATA lpdim = (LPDRVIMAGEDATA)WinMalloc(sizeof(DRVIMAGEDATA));

           memcpy((char *)lpdim, (char *)lpdata, sizeof(DRVIMAGEDATA)); 
           lpgd->lpMem = DrvFillImageInfo(lpdim);
           if (lpgd->lpMem == NULL)
              {
              /*   Remove data atom
              */
              XDeleteProperty(dp->display,
                   RootWindow(dp->display,dp->screen),
                   lpce->atom);
              /* 
              **   Replace the format record 
              */
              lpce->length = lpce->atom = 0;
              lpAllFormats[1].length = lpAllFormats[1].atom = 0; /* it's CF_BITMAP */ 
              XChangeProperty(dp->display,
                           RootWindow(dp->display,dp->screen),
                           dp->XFormatAtom,XA_STRING,
                           8,PropModeReplace, (LPBYTE)lpceTop, sizeof(CLIPENTRY)*nFormats);
              } 
           }
        else {
             lpgd->dwSize = nitems;
             lpgd->lpMem = (LPSTR)WinMalloc(lpgd->dwSize);
             memcpy((char *)lpgd->lpMem, (char *)lpdata, lpgd->dwSize);
             }

        XFree((LPSTR)lpceTop);
        XFree((LPSTR)lpdata);
 
        return (DWORD)lpgd->lpMem;
}

/****************************************************************************/
static DWORD
DrvIsFormatAvailable(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
	DWORD format = (DWORD)dwParm1;
        register           i;
	PRIVATEDISPLAY    *dp = GETDP();
        LPCLIPENTRY        lpce, lpceTop;
        Atom               actual_type;
        int                actual_format;
        unsigned long      nitems;
        unsigned long      remainder;
        BOOL               ynIs = 0;
 
        XGetWindowProperty(dp->display,
                        RootWindow(dp->display,dp->screen),
                        dp->XFormatAtom,0,8192,FALSE,XA_STRING,
                        &actual_type, &actual_format,&nitems,
                        &remainder, (unsigned char **)&lpceTop);
        if (lpceTop != NULL)
           {
           for (i = 0, lpce = lpceTop;  i < nitems / sizeof(CLIPENTRY);  i++, lpce++)
               if (lpce->length && lpce->wFormat == format)
                  {  ynIs = 1;   break;  } 
           XFree((char *)lpceTop);
           }
        return (DWORD)ynIs;
}


/****************************************************************************/
static DWORD 
DrvLoadFormats(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
        register           i;
	PRIVATEDISPLAY    *dp = GETDP();
        LPCLIPENTRY        lpce, lpceTop;
        Atom               actual_type;
        int                actual_format;
        unsigned long      nitems;
        unsigned long      remainder;
        long               nFormats = 0;
 
        nitems = 0;

        XGetWindowProperty(dp->display,
                        RootWindow(dp->display,dp->screen),
                        dp->XFormatAtom,0,8192,FALSE,XA_STRING,
                        &actual_type, &actual_format,&nitems,
                        &remainder, (unsigned char **)&lpceTop);
        if (lpceTop != NULL)
           {
           for (i = 0, lpce = lpceTop;  i < nitems / sizeof(CLIPENTRY);  i++, lpce++)
               if (lpce->length)
                  nFormats++; 
           XFree((char *)lpceTop);
           }
        return (DWORD)nFormats;
}


/****************************************************************************/
static DWORD
DrvClipbrdOpen(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
        DWORD win = (DWORD)dwParm2;
        LPDRVCLIPINFO lpct = (LPDRVCLIPINFO)lpStruct;
	PRIVATEDISPLAY  *dp = GETDP();
	Window           owner;
        BOOL             ynSuccess = 0;
        LPCLIPENTRY      lpce;
        LPBYTE           lpdata;
        Atom             actual_type;
        int              actual_format;
        unsigned long    nbytes, nitems;
        unsigned long    remainder;

        lpct->winOwner = win;
#ifdef SEVERE
	XGrabServer(dp->display);
#endif
	owner = XGetSelectionOwner(dp->display, dp->XFormatAtom);
	if (owner == 0)
	   {
	   XSetSelectionOwner(dp->display, dp->XFormatAtom, (Window)lpct->winOwner, 0);
           owner = XGetSelectionOwner(dp->display, dp->XFormatAtom);
           lpct->winOwner = owner;
	   ynSuccess = 1;
           /*
           **   Get CUT_BUFFER0 length and update format record on X.
           */
           XGetWindowProperty(dp->display,
                        RootWindow(dp->display,dp->screen),
                        lpAllFormats[0].atom,0,8192,FALSE,XA_STRING,
                        &actual_type, &actual_format,&nbytes,
                        &remainder, (unsigned char **)&lpdata);
           if (lpdata != NULL)
              {
              XGetWindowProperty(dp->display,
                        RootWindow(dp->display,dp->screen),
                        dp->XFormatAtom,0,8192,FALSE,XA_STRING,
                        &actual_type, &actual_format,&nitems,
                        &remainder, (unsigned char **)&lpce);
              if (lpce != NULL)
                 {
                 nAllFormats = nitems / sizeof(CLIPENTRY);
                 actual_type = lpAllFormats[0].atom;
                 memcpy((char *)lpAllFormats, (char *)lpce, sizeof(CLIPENTRY) * nAllFormats);
                 lpAllFormats[0].length = nbytes;
                 lpAllFormats[0].atom = actual_type;
#ifdef SEVERE
                 XChangeProperty(dp->display,
                           RootWindow(dp->display,dp->screen),
                           dp->XFormatAtom,XA_STRING,
                           8,PropModeReplace, (LPBYTE)lpAllFormats, sizeof(CLIPENTRY)*nAllFormats);
#endif
                 XFree((char *)lpce);
                 }
              XFree((char *)lpdata);
              }
           }
#ifdef SEVERE
        XUngrabServer(dp->display);
#endif
        return (DWORD)ynSuccess;
}

/****************************************************************************/
static DWORD
DrvClipbrdSetData(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
        LPGETSETDATA lpsd = (LPGETSETDATA)dwParm2;
        register           i;
        PRIVATEDISPLAY    *dp = GETDP();
        LPCLIPENTRY        lpce, lpceTop;
        Atom               actual_type;
        int                actual_format;
        unsigned long      nitems;
        unsigned long      remainder;
        int                DataMode;
        long               nFormats;
 
        if (lpsd->uiFormat != CF_TEXT && lpsd->uiFormat != CF_BITMAP && lpsd->uiFormat != CF_DIB && lpsd->uiFormat != CF_METAFILEPICT && lpsd->uiFormat < 0xCC00)
           return 0L; /* we don't support anything but CF_TEXT, CF_BITMAP and user defined formats */

        XGetWindowProperty(dp->display,
                        RootWindow(dp->display,dp->screen),
                        dp->XFormatAtom,0,8192,FALSE,XA_STRING,
                        &actual_type, &actual_format,&nitems,
                        &remainder, (unsigned char **)&lpceTop);

        nFormats = nitems / sizeof(CLIPENTRY);
        for (i = 0, lpce = lpceTop;  i < nFormats;  i++, lpce++)
            {
            if (lpce->wFormat == lpsd->uiFormat)
               break;
            }
        if (i == nFormats || lpce->wFormat != lpsd->uiFormat)
           {
           return 0;
           }

        if (lpsd->uiFormat == CF_TEXT) 
           DataMode = PropModeReplace;
        else {
             DataMode = PropModeAppend;
             if (lpsd->uiFormat == CF_BITMAP)
                { 
                DRVIMAGEDATA  dim;
                LPDRVIMAGEDATA lpXData;

                lpsd->dwSize = sizeof(DRVIMAGEDATA);
                /*
                **   Delete previous bitmap stuff, if any...
                */
                XGetWindowProperty(dp->display,
                        RootWindow(dp->display,dp->screen),
                        lpce->atom, 0, 8192,
                        FALSE,XA_STRING,&actual_type,
                        &actual_format,&nitems,&remainder,
                        (unsigned char **)&lpXData);
                if (nitems > 0)
                   DrvDestroyImage(dwParm1, dwParm2, (LPVOID)lpXData); 
                XFree((char *)lpXData);
                /*   
                **   lpsd->lpMem points to DRVIMAGEDATA structure, which contains
                **   bitmap info. The only thing we should do before moving it
                **   to X is to fill out Pixmap data...
                */
                memcpy((char *)&dim, (char *)lpsd->lpMem, sizeof(dim));
                DrvCopyPixmap(&dim);
                lpsd->dwSize = sizeof(dim);
                lpsd->lpMem = (LPSTR)&dim;
                }
             }
        /* 
        **   Put the data to appropriate atom on the clipboard
        */
        XChangeProperty(dp->display,
                        RootWindow(dp->display,dp->screen),
                        lpce->atom,XA_STRING,
                        8,DataMode,(LPBYTE)lpsd->lpMem, lpsd->dwSize);
        /*
        **   Now update the format property on X...
        */
        if (lpce->wFormat == lpsd->uiFormat)
           lpce->length  = lpsd->dwSize;
/********
        else {
             lpceTop = (LPCLIPENTRY)WinRealloc(lpceTop, sizeof(CLIPENTRY) * ++nFormats);
             lpAllFormats = (LPCLIPENTRY)WinRealloc(lpAllFormats, sizeof(CLIPENTRY) * nFormats);
             lpce = &lpceTop[nFormats - 1];
             lpce->wFormat = lpsd->uiFormat;
             lpce->length  = lpsd->dwSize;
             lpce->offset  = DataOffset;
             } 
*********/
        XChangeProperty(dp->display,
                           RootWindow(dp->display,dp->screen),
                           dp->XFormatAtom,XA_STRING,
                           8,PropModeReplace, (LPBYTE)lpceTop, sizeof(CLIPENTRY)*nFormats);
        memcpy((char *)lpAllFormats, (char *)lpceTop, sizeof(CLIPENTRY) * nFormats);
        nAllFormats = nFormats;
        XFree((char *)lpceTop); 
        /* ... and reload format and data... */

        return 1L;
}


/****************************************************************************/
static DWORD
DrvEnumFormats(LPARAM dwParm1,LPARAM dwParm2, LPVOID lpStruct)
{
	DWORD format = (DWORD)dwParm1;
        register        i;
        LPCLIPENTRY     lpce;
        Atom            actual_type;
        int             actual_format;
        unsigned long   remainder, nitems;
        PRIVATEDISPLAY *dp = GETDP();
        DWORD           dwFormat = 0L;
	BOOL            ynFound = 0;
	/*
	**   If dwFormat == 0 return the first format available. Otherwise
	**   return next format in list.
	*/

        XGetWindowProperty(dp->display,
                        RootWindow(dp->display,dp->screen),
                        dp->XFormatAtom,0,8192,FALSE,XA_STRING,
                        &actual_type, &actual_format,&nitems,
                        &remainder, (unsigned char **)&lpce);
        if (lpce != NULL)
           {
	   if (format != 0)
              {
              for (i = 0;   i < nitems / sizeof(CLIPENTRY);   i++)
                  {
                  if (ynFound)
                     {
                     dwFormat = (DWORD)lpce[i].wFormat;
                     break;
                     }
                  else if (lpce[i].wFormat == format)
                          ynFound = 1;
                  }
              }
           else dwFormat = lpce->wFormat;
           XFree((char *)lpce);
           }
	
	return dwFormat;
}

/****************************************************************************/
static DWORD DrvRegisterFormat(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
   DWORD format = (DWORD)dwParm2;
   LPSTR name = (LPSTR)lpStruct;
   register    i;
   LPCLIPENTRY lpce;
   PRIVATEDISPLAY *dp = GETDP();
  
   /*   First check whether the given format already exists...
   */
   for (i = 0, lpce = lpAllFormats;   i < nAllFormats;   i++, lpce++)
       {
       if (lpce->wFormat == format)
          return format; /* what about incremening reference count? */
       }
   /*
   **   Format is not found, add it to the end of our table...
   */
   lpAllFormats = (LPCLIPENTRY)WinRealloc((char *)lpAllFormats, ++nAllFormats * sizeof(CLIPENTRY));
   lpce = &lpAllFormats[nAllFormats - 1];
   lpce->wFormat = format;
   lpce->length  = 0;
   lpce->atom = XInternAtom(dp->display, name, False);
   /*
   **   ...and put the updated list back to X
   */
   XChangeProperty(dp->display,
                   RootWindow(dp->display,dp->screen),
                   dp->XFormatAtom,XA_STRING,
                   8,PropModeReplace, (LPBYTE)lpAllFormats, nAllFormats * sizeof(CLIPENTRY));
   return 1L;
}


/****************************************************************************/
static DWORD
DrvInitAtomTable(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
    LPSTR lpTableName = (LPSTR)dwParm1;
    LPSTR lpDataName = (LPSTR)dwParm2;
    LPATOMTABLE lpAtomTable = (LPATOMTABLE)lpStruct;
    LPDRVATOMSTRUCT lpDrvAtomStruct;
    PRIVATEDISPLAY *dp = GETDP();

    if (lpAtomTable == 0)
	return 0L;
    if (lpTableName == 0)
	return 0L;
    if (lpDataName == 0)
	return 0L;

    lpDrvAtomStruct = (LPDRVATOMSTRUCT)WinMalloc(sizeof(DRVATOMSTRUCT));
    memset((LPSTR)lpDrvAtomStruct,'\0',sizeof(DRVATOMSTRUCT));

    lpDrvAtomStruct->TableAtom = XInternAtom(dp->display,lpTableName,False);
    lpDrvAtomStruct->DataAtom = XInternAtom(dp->display,lpDataName,False);

    lpAtomTable->lpDrvData = (LPVOID)lpDrvAtomStruct;

    return 1L;
}

/****************************************************************************/
static DWORD
DrvAtomData(LPARAM dwParm1, LPARAM dwParm2, LPVOID lpStruct)
{
    WORD wFunc = (WORD)dwParm1;
    LPATOMTABLE lpAtomTable = (LPATOMTABLE)lpStruct;
    PRIVATEDISPLAY    *dp;
    Atom	actual_type;
    int  	actual_format;
    unsigned long	remainder;
    int	nSize;
    LPDRVATOMSTRUCT lpDrvAtomStruct;

    if (lpAtomTable == 0)
	return 0L;

    dp = GETDP();

    /* temporary */
    if (!dp->bConnected)
	return 0L;

    if (lpAtomTable->lpDrvData == 0)
	if (!DrvInitAtomTable((LPARAM)"_GlobalTable",(LPARAM)"_GlobalData",
			(LPVOID)lpAtomTable))
	    return 0L;

    lpDrvAtomStruct = (LPDRVATOMSTRUCT)lpAtomTable->lpDrvData;

    if (lpDrvAtomStruct->XTable) {
	XFree((LPSTR)lpDrvAtomStruct->XTable);
	lpDrvAtomStruct->XTable = 0;
    }

    if (lpDrvAtomStruct->XData) {
	XFree(lpDrvAtomStruct->XData);
	lpDrvAtomStruct->XData = 0;
    }

    if (wFunc == GAD_READONLY || wFunc == GAD_READWRITE) {
	XGetWindowProperty(
		dp->display,RootWindow(dp->display,dp->screen),
		lpDrvAtomStruct->TableAtom,
		0,8192,
		False,
		XA_INTEGER,&actual_type,
		&actual_format,
		&lpAtomTable->TableSize,
		&remainder,
		(unsigned char **)&lpDrvAtomStruct->XTable);
		
	lpAtomTable->TableSize /= 4;

	XGetWindowProperty(
		dp->display,RootWindow(dp->display,dp->screen),
		lpDrvAtomStruct->DataAtom,
		0,8192,
		False,
		XA_STRING,&actual_type,
		&actual_format,
		&lpAtomTable->DataSize,
		&remainder,
		(unsigned char **)&lpDrvAtomStruct->XData);

#ifdef	LATER
	if (wFunc == GAD_READONLY) {
	    lpAtomTable->AtomTable = lpDrvAtomStruct->XTable;
	    lpAtomTable->AtomData  = lpDrvAtomStruct->XData;
	    if (lpAtomTable->DataSize == 0) {
		lpAtomTable->AtomTable = 0;
		lpAtomTable->TableSize = 0;
	    }
	}
	else {
#endif
	    if (lpAtomTable->TableSize == 0) {
		lpAtomTable->AtomTable = 0;
		lpAtomTable->AtomData  = 0;
		lpAtomTable->DataSize  = 0;
	    }
	    else if (lpAtomTable->DataSize == 0) {
		lpAtomTable->AtomTable = 0;
		lpAtomTable->TableSize = 0;
		lpAtomTable->AtomData  = 0;
	    }
	    else {
		nSize = lpAtomTable->TableSize*sizeof(ATOMENTRY);
		if (lpAtomTable->AtomTable)
		    lpLibCallback(TWINLIBCALLBACK_MEMORY,
				TLC_MEMFREE, 0, lpAtomTable->AtomTable); 
		lpAtomTable->AtomTable = (LPATOMENTRY)
			lpLibCallback(TWINLIBCALLBACK_MEMORY,
				TLC_MEMALLOC, nSize, 0); 
		memcpy((LPVOID)lpAtomTable->AtomTable,
			(LPVOID)lpDrvAtomStruct->XTable, nSize);
		if (lpAtomTable->AtomData)
		    lpLibCallback(TWINLIBCALLBACK_MEMORY,
				TLC_MEMFREE, 0, lpAtomTable->AtomData); 
		lpAtomTable->AtomData = (LPSTR)
			lpLibCallback(TWINLIBCALLBACK_MEMORY,
				TLC_MEMALLOC, lpAtomTable->DataSize, 0); 
		memcpy(lpAtomTable->AtomData,
			lpDrvAtomStruct->XData,lpAtomTable->DataSize);
	    }
#ifdef	LATER
	}
#endif
	if (lpDrvAtomStruct->XTable)
		XFree((LPSTR)lpDrvAtomStruct->XTable);
	if (lpDrvAtomStruct->XData)
		XFree((LPSTR)lpDrvAtomStruct->XData);

	lpDrvAtomStruct->XTable = 0;
	lpDrvAtomStruct->XData = 0;
    }

    if (wFunc == GAD_UPDATE) {
	if (lpAtomTable->TableSize == 0 || lpAtomTable->DataSize == 0) {
	    XDeleteProperty(dp->display,
		RootWindow(dp->display,dp->screen),
		lpDrvAtomStruct->TableAtom);
	    XDeleteProperty( dp->display,
		RootWindow(dp->display,dp->screen),
		lpDrvAtomStruct->DataAtom);
	} else {
	    XChangeProperty(
		dp->display,RootWindow(dp->display,dp->screen),
		lpDrvAtomStruct->TableAtom,
		XA_INTEGER,
		32,
		PropModeReplace,
		(unsigned char *) lpAtomTable->AtomTable, 
		lpAtomTable->TableSize*4);

	    XChangeProperty(
		dp->display,RootWindow(dp->display,dp->screen),
		lpDrvAtomStruct->DataAtom,
		XA_STRING,
		8,
		PropModeReplace,
		(LPBYTE)(lpAtomTable->AtomData), lpAtomTable->DataSize);
	}
    }

    return 1L;
}

static DWORD
DrvIPCDoNothing(LPARAM dwParam1, LPARAM dwParam2, LPVOID lpStruct)
{
	return 0L;
}

