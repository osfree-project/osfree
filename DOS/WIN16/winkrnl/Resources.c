/*    
	Resources.c	2.43
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

#include <stdlib.h>
#include <string.h>

#include "windows.h"
#include "windowsx.h"

#include "Log.h"
#include "kerndef.h"
#include "Resources.h"
#include "Module.h"
#include "Kernel.h"
#include "WImageIO.h"

/* external linkage */
extern void LoadResourceEx(HANDLE,NAMEINFO *,LPMEMORYINFO);
extern HBITMAP CreateDIBitmapEx(LPBITMAPIMAGE,LPNAMEINFO);
extern HICON LoadIconResource(LPNAMEINFO);
extern HCURSOR LoadCursorResource(LPNAMEINFO);
extern HMODULE GetModuleFromInstance(HINSTANCE);
extern LPBYTE ConvertResourceToNat(LPBYTE, WORD);

/* static functions */
static LPMEMORYINFO HandleAlloc(LPSTR,WORD);
static LPSTR HandleLock(HANDLE,UINT);
static BOOL HandleUnlock(HANDLE,WORD);
static DWORD HandleSize(HINSTANCE,HANDLE);

static LPMEMORYINFO GlobalFirst;
static GNOTIFYPROC	  slpNotifyProc;

#define MT_MEMORY	0	/* object is globalalloc data   */
#define MT_NAMEINFO	1	/* object is nameinfo structure */
#define MT_RESOURCE	2	/* object is resource data      */

/*****************************************************************
 *
 *	This file is broken out into six parts
 *
 *	1 static internal memory objects 	HandleXXXX,...
 *	2 binary interface support routines	GlobalHandle32
 *	3 global memory API's			GlobalAlloc,...
 *	4 exported internal resource support 	LoadResourceTable
 *	5 resource dependent API's;		LoadIcon,LoadCursor...
 *	6 resource API's			FindResource,LoadResource,...
 *
 ****************************************************************/

/****************************************************************
 *
 *	Internal Memory Object Routines
 *	used by resources and memory allocation
 *	functions include:
 *		HandleAlloc 	allocate handle and point to data
 *		HandleLock	return data pointer given handle
 *		HandleUnlock	decrement lock count of HandleLock
 *		HandleSize	return the size of the object
 *				note: is dependent on what the object
 *				      points to.
 *
 ****************************************************************/		
#ifdef WINMALLOC_CHECK
void
TWIN_HandleCleanup()
{
        LPMEMORYINFO 	lpMemory,lpnext;
	int i;
	
	for(i=0,lpMemory = GlobalFirst;lpMemory;lpMemory = lpnext) {
		lpnext = lpMemory->lpNext;
		
		if(lpMemory->wType == MT_MEMORY) {
			logstr(LF_LOG,"%d: ptr=%x handle=%x size=%d\n",
				i++,
				lpMemory->lpCore, 
				lpMemory->hMemory,
				lpMemory->dwSize);
			if (lpMemory->hMemory) {
				GlobalUnlock(lpMemory->hMemory);
				GlobalFree(lpMemory->hMemory);
			}
			if (lpMemory->lpCore) {
				WinFree(lpMemory->lpCore);
				lpMemory->lpCore = 0;
				lpMemory->dwSize = 0;
			}
		}
	}
}
#endif

static LPMEMORYINFO
HandleAlloc(LPSTR lpData,WORD wType)
{
        LPMEMORYINFO 	lpMemory;
	HANDLE 		hResData;

	if((lpMemory = CREATEHANDLE(hResData))) {

		/* insert into the list */
		lpMemory->lpNext  = GlobalFirst;
		GlobalFirst = lpMemory;

		/* assign a pointer to the data */
		lpMemory->lpCore = lpData;
		lpMemory->hMemory = hResData;
		lpMemory->wType   = wType;
		lpMemory->wIndex = 0;
		return lpMemory;
	}
	return 0;
}

static LPSTR 
HandleLock(HANDLE hMem,UINT wFlag)
{
    	LPMEMORYINFO lpMemory;

	if((lpMemory  = GETHANDLEINFO(hMem))) {

		/* does handle check */
		if(lpMemory->hMemory != hMem) {
		        RELEASEHANDLEINFO(lpMemory);
			return 0;	
		}

		if(wFlag && lpMemory->lpCore) 
			lpMemory->wRefCount++;
		RELEASEHANDLEINFO(lpMemory);
		return lpMemory->lpCore;
	}
	return 0;
}

static BOOL
HandleUnlock(HANDLE hMem,WORD wFlag)
{
	LPMEMORYINFO 	lpMemory;

	if((lpMemory  = GETHANDLEINFO(hMem))) {

		/* does handle check */
		if(lpMemory->hMemory != hMem) {
		        RELEASEHANDLEINFO(lpMemory);
			return TRUE;	
		}

		if(wFlag && lpMemory->wRefCount) 
			lpMemory->wRefCount--;
		if (lpMemory->wRefCount == 0) {
			RELEASEHANDLEINFO(lpMemory);
			return FALSE;
		}
		RELEASEHANDLEINFO(lpMemory);
		return TRUE;
	}
	return TRUE;
}

static DWORD
HandleSize(HINSTANCE hInst,HANDLE hMem)
{
	LPMEMORYINFO 	lpMemory;
	LPNAMEINFO 	p;
	DWORD		size;
	DWORD		dwResult = 0;

	if((lpMemory  = GETHANDLEINFO(hMem))) {
		switch(lpMemory->wType) {
		case MT_MEMORY:
			dwResult = lpMemory->dwSize;
			break;
		case MT_NAMEINFO:
			p = (LPNAMEINFO) lpMemory->lpCore;
			dwResult = p->rcslength;
			break;
		case MT_RESOURCE:	
			if((size = lpMemory->dwSize) == 0)
			   dwResult = lpMemory->dwBinSize;
			break;
		}
		RELEASEHANDLEINFO(lpMemory);
		return dwResult;
	}
	return(0);
}

#ifdef LATER
	for completeness, we should add 
	HandleFree
	HandleRealloc

#endif

/*********************************************************/
/***** API Binary Interface Routines *********************/
/*********************************************************/

HGLOBAL
GlobalHandle32(LPCVOID lpaddress)
{
	LPMEMORYINFO p;	

	for(p = GlobalFirst;p;p = p->lpNext) {
		if(((DWORD) p->lpCore <= (DWORD) lpaddress) && 
		   (DWORD) lpaddress < (DWORD)(p->lpCore + p->dwSize))
			return p->hMemory;
	}
	return 0;
}


/*********************************************************/
/***** API Memory Routines *******************************/
/*********************************************************/

#ifdef	STRICT
LPVOID
#else
LPSTR
#endif
GlobalLock(HGLOBAL hMem)
{
	return HandleLock(hMem,1);
}

BOOL
GlobalUnlock(HGLOBAL hMem)
{
	return HandleUnlock(hMem,1);
}

HGLOBAL	WINAPI
GlobalHandle(LPCVOID lpGlobalMem)
{
	APISTR((LF_APISTUB, "GlobalHandle(LPCVOID=%p)\n",
		lpGlobalMem));
	return (0);
}

UINT	WINAPI
GlobalPageLock(HGLOBAL hGlobalMem)
{
	APISTR((LF_APISTUB, "GlobalPageLock(HGLOBAL=%p)\n",hGlobalMem));
	return (1);
}

UINT	WINAPI
GlobalPageUnlock(HGLOBAL hGlobalMem)
{
	APISTR((LF_APISTUB, "GlobalPageUnlock(HGLOBAL=%p)\n",hGlobalMem));
	return (1);
}

HGLOBAL
GlobalAlloc(UINT wFlags,DWORD dwBytes)
{
        LPMEMORYINFO 	lpMemory;
	LPSTR	     	lpmem;
	
	dwBytes = (dwBytes+31) & 0xffffffe0;
	if (dwBytes) {
	    lpmem  = (LPSTR) WinMalloc(dwBytes);
	    if(!lpmem)
		return((HGLOBAL)NULL);
	 } else
	    lpmem = 0;

	/* allocate an object using this data */
	if((lpMemory = HandleAlloc(lpmem,MT_MEMORY))) {

		/* initialize the MEMORYINFO block */
		lpMemory->wFlags  = wFlags;
		lpMemory->dwSize  = dwBytes;

		lpMemory->wRefCount = 0;

		/* global memory flags */
		if(wFlags & GMEM_ZEROINIT)
			memset(lpMemory->lpCore,0,dwBytes);
		return lpMemory->hMemory;
	}
	return (HGLOBAL)NULL;
}

HGLOBAL
GlobalReAlloc(HGLOBAL hMem, DWORD dwBytes, UINT uFlags)
{
	LPHANDLEINFO lpMemory; 	
	LPSTR	     lpmem;
	
	dwBytes = (dwBytes+31) & 0xffffffe0;
	if((lpMemory  = GETHANDLEINFO(hMem))) { 
  	    if ((lpMemory->wFlags & (GMEM_DISCARDABLE|GMEM_MOVEABLE))
			== (GMEM_DISCARDABLE|GMEM_MOVEABLE)) {

		if((uFlags == GMEM_MOVEABLE) && (dwBytes == 0)) {

			if(lpMemory->lpCore)
				WinFree(lpMemory->lpCore);

			lpMemory->lpCore = 0;
			lpMemory->wFlags = uFlags;

			RELEASEHANDLEINFO(lpMemory);
			return hMem;
		}
	    }

	    if (!(uFlags & GMEM_MODIFY)) {
		lpmem = lpMemory->lpCore; 
		if (lpmem)
			lpmem = WinRealloc(lpmem,dwBytes); 
		else
			lpmem = WinMalloc(dwBytes);

		lpMemory->lpCore = lpmem;

		if(uFlags & GMEM_ZEROINIT) {
			if(dwBytes > lpMemory->dwSize)
				memset(lpMemory->lpCore+lpMemory->dwSize,0,
					dwBytes-lpMemory->dwSize);
		}
		lpMemory->dwSize = dwBytes;
		lpMemory->wFlags = uFlags;
	    }
	    else {
		if(uFlags & GMEM_DISCARDABLE)
			uFlags |= lpMemory->wFlags;
		lpMemory->wFlags = (uFlags & ~GMEM_MODIFY);
	    }
	    RELEASEHANDLEINFO(lpMemory);
	}
	return hMem;
}

/* this should be HandleFree... */
HANDLE
GlobalFree(HANDLE hMem)
{
	LPMEMORYINFO p;
	LPHANDLEINFO lpMemory; 	

	if((lpMemory = GETHANDLEINFO(hMem))) {

		/* deallocate any allocated memory */
		if(lpMemory->wType == MT_MEMORY && lpMemory->lpCore) {
			WinFree((LPSTR)lpMemory->lpCore);
			lpMemory->lpCore = 0;
		}

		/* unlink the object from the global list */
		for(p = GlobalFirst;p;p = p->lpNext) {
			if((p == GlobalFirst) && (p == lpMemory)) {
				GlobalFirst = lpMemory->lpNext;
				break;
			}
			if(p->lpNext == lpMemory) {
				p->lpNext = lpMemory->lpNext;
				break;
			}
		}
		RELEASEHANDLEINFO(lpMemory);
	}
	else			/* failed to find handle, return parameter */
		return hMem;

	/* now get rid of the handle */
	FREEHANDLE(hMem);

	return 0;
}

DWORD
GlobalSize(HANDLE hMem)
{
	return HandleSize(0,hMem);
}

UINT
GlobalFlags(HANDLE hMem)
{
	UINT		flags;
	LPHANDLEINFO lpMemory; 	

	if ((lpMemory  = GETHANDLEINFO(hMem))) { 
		flags = lpMemory->wFlags;
		flags &= ~GMEM_LOCKCOUNT;
		flags |= lpMemory->wRefCount & GMEM_LOCKCOUNT;

		RELEASEHANDLEINFO(lpMemory);
		return(flags);
	}
	return(0);
}

void
GlobalNotify(GNOTIFYPROC lpNotifyProc)
{
	slpNotifyProc = lpNotifyProc;
}

HGLOBAL
GlobalLRUOldest(HGLOBAL hglb)
{
    APISTR((LF_APISTUB, "GlobalLRUOldest(HGLOBAL=%p)\n",hglb));
    return hglb;
}

HGLOBAL
GlobalLRUNewest(HGLOBAL hglb)
{
    APISTR((LF_APISTUB, "GlobalLRUNewest(HGLOBAL=%p)\n",hglb));
    return hglb;
}

DWORD
GlobalCompact(DWORD dwMinFree)
{
    APISTR((LF_APISTUB, "GlobalCompact(DWORD=%p)\n",dwMinFree));
    return 4*1024*1024;
}

/*********************************************************/
/***** API Resource Dependent Routines *******************/
/*********************************************************/

HACCEL	WINAPI
LoadAccelerators(HINSTANCE hInstance, LPCSTR lpTableName)
{
	HANDLE hResInfo;
	HACCEL rc;

    	APISTR((LF_APICALL,"LoadAccelerators(HINSTANCE=%x,LPCSTR=%x)\n",
		hInstance,lpTableName));

	hResInfo = FindResource(hInstance,lpTableName,RT_ACCELERATOR);

	if(hResInfo == 0) {
    		APISTR((LF_APIFAIL,"LoadAccelerators: returns HACCEL 0\n"));
		return 0;
	}

	rc =  LoadResource(hInstance,hResInfo);
    	APISTR((LF_APIRET,"LoadAccelerators: returns HACCEL %x\n",rc));
	return rc;
}

int	WINAPI
CopyAcceleratorTable(HACCEL hAccelTable, LPACCEL lpAccelTable,
	int nAccelTableSize)
{
     	APISTR((LF_APISTUB, "CopyAcceleratorTable(HACCEL=%x,LPACCEL=%p)\n",
		hAccelTable,lpAccelTable));
	return (0);
}

int	WINAPI
LoadString(HINSTANCE hInstance, UINT wID, LPSTR lpBuffer, int nBufferMax)
{
	HANDLE hResInfo;
	HANDLE hResData;
	STRINGENTRY *lpString;
	int 	count = 0;
	int	nIndex;

    	APISTR((LF_APICALL,"LoadString(HINSTANCE=%x,UINT=%x,LPSTR=%p,int=%x)\n",
		hInstance,wID,lpBuffer,nBufferMax));

	hResInfo = FindResource(hInstance,
				(LPCSTR)(((UINT)(wID & 0x0000fff0)>>4)+1),
				RT_STRING);
	if(hResInfo == 0) {
    		APISTR((LF_APIFAIL,"LoadString: returns int 0\n"));
		return count;
	}

	hResData = LoadResource(hInstance,hResInfo);
	if(hResData == 0) {
    		APISTR((LF_APIFAIL,"LoadString: returns int 0\n"));
		return count;
	}

	lpString = (STRINGENTRY *) HandleLock(hResData,0);

	nIndex = wID & 0xf;
	if (lpString[nIndex]) {
	    count = min(nBufferMax,(int)strlen(lpString[nIndex])+1);
	    strncpy(lpBuffer,lpString[nIndex],count);
	}
	else {
	    count = 1;
	    lpBuffer[0] = '\0';
	}

#if 0
	FreeResource(hResData);
#endif

    	APISTR((LF_APIRET,"LoadString: returns int %d\n",count-1));
	return count-1;
}

#ifdef TWIN32
/*
 * LoadImage() Win32 API
 */
HANDLE 
LoadImage
(
	HINSTANCE			hInstance,
	LPCSTR				pstrImage,
	UINT				ImageType,
	int				cxDesired,
	int				cyDesired,
	UINT				LoadFlags
)

{
	POINT			    	ptSize;
	HBITMAP				hbImage		  = ( HBITMAP )NULL;
	HBITMAP				hbImageMask	  = ( HBITMAP )NULL;
	HDC				hDeskDC		  = GetDC ( GetDesktopWindow () );
	HBITMAP				hBitmap		  = ( HBITMAP )NULL;
	HBITMAP				hbmpMask	  = ( HBITMAP )NULL;
	HDC				hSrcDC		  = ( HDC )NULL;
	HDC				hDstDC		  = ( HDC )NULL;
	HANDLE				hImage	  	  = ( HANDLE )NULL;

	BITMAPINFOHEADER		bi;
	HBITMAP			    	hbmpOld;

	BITMAP			    	bmColor;
	BITMAP			    	Bitmap;
	ICONINFO		    	IconInfo;


	memset ( &bi, 0, sizeof ( BITMAPINFOHEADER ) );
	memset ( &bmColor, 0, sizeof ( BITMAP ) );
	memset ( &IconInfo, 0, sizeof ( ICONINFO ) );

	if ( LoadFlags & LR_LOADFROMFILE )
	{
		if ( ImageType == IMAGE_BITMAP )
			hImage = W_IReadBmpFile ( pstrImage );
		else
		if ( ImageType == IMAGE_ICON )
			hImage = W_IReadIconFile ( pstrImage );
		else
			hImage = W_IReadCursorFile ( pstrImage );

	}
	else
	{
		switch ( ImageType )
		{
			
			case IMAGE_BITMAP   :
				hImage = LoadBitmap ( hInstance, pstrImage ); 
				break;

			case IMAGE_ICON     :
				if ( ! ( hImage = LoadIcon ( hInstance, pstrImage ) ) )
					break;
				if ( GetIconInfo ( hImage, &IconInfo ) )
				{
					hbImage = IconInfo.hbmColor;
					hbImageMask = IconInfo.hbmMask;
				}
				break;
			
			case IMAGE_CURSOR   :
				if ( ! (hImage = LoadCursor ( hInstance, pstrImage ) ) )
					break;
				if ( GetIconInfo (hImage, &IconInfo ) )
				{
					hbImage = IconInfo.hbmColor;
					hbImageMask = IconInfo.hbmMask;
				}
				break;
		}

	}
#if 1
	GetObject ( hbImage, sizeof ( BITMAP ), &Bitmap ); 
	ptSize.x = Bitmap.bmWidth;
	ptSize.y = Bitmap.bmHeight;
	DPtoLP ( hDeskDC, &ptSize, 1 );

	if ( ( hSrcDC = CreateCompatibleDC ( hDeskDC ) ) &&
	     ( hDstDC = CreateCompatibleDC ( hDeskDC ) )
	   ) 
	{
		hBitmap = CreateCompatibleBitmap ( hDeskDC, cxDesired, cyDesired );
		hbmpOld = SelectObject ( hDstDC, hBitmap );
		SelectObject ( hSrcDC, hbImage );
		StretchBlt ( hDstDC , 0, 0, cxDesired, cyDesired, hSrcDC,
			0, 0, ptSize.x, ptSize.y, SRCCOPY );
		SelectObject ( hDstDC, hbmpOld );
		GetObject ( hBitmap, sizeof ( BITMAP ), &Bitmap ); 

		if ( hbImageMask )
		{
			hbmpMask = CreateCompatibleBitmap ( hDeskDC, cxDesired, cyDesired );
			hbmpOld = SelectObject ( hDstDC, hbmpMask );
			SelectObject ( hSrcDC, hbImageMask );
			StretchBlt ( hDstDC, 0, 0, cxDesired, cyDesired, hSrcDC,
				0, 0, ptSize.x, ptSize.y, SRCCOPY );
			SelectObject ( hDstDC, hbmpOld );
		}
	}

	if ( ImageType	== IMAGE_ICON || ImageType == IMAGE_CURSOR )
	{
		IconInfo.hbmColor = hBitmap;
		IconInfo.hbmMask = hbmpMask;
		return CreateIconIndirect ( &IconInfo );
	}

	DeleteObject ( hBitmap );
	DeleteObject ( hbmpMask );
	if ( hDstDC )
		DeleteDC ( hDstDC );
	if ( hSrcDC )
		DeleteDC ( hSrcDC );
	ReleaseDC ( GetDesktopWindow (), hDeskDC );
#endif
	return hImage;

}
#endif

HICON FAR PASCAL
LoadIcon(HINSTANCE hInstance, LPCSTR lpIconName)
{
	HANDLE hResNameInfo;
	HICON hIcon;
	LPNAMEINFO  lpTemplate;
	LPMEMORYINFO    lpMemory = (LPMEMORYINFO)NULL;
	ICONDIRENTRY *picon;
	ICONDIRENTRY *plast;

    	APISTR((LF_APICALL,"LoadIcon(HINSTANCE=%x,LPCSTR==%x)\n",
		hInstance,lpIconName));

	hResNameInfo = FindResource(hInstance, lpIconName, RT_GROUP_ICON);

	if(hResNameInfo == 0) {
    		APISTR((LF_APIFAIL,"LoadIcon: returns HICON 0\n"));
		return 0;
	}

	lpTemplate = (LPNAMEINFO) HandleLock(hResNameInfo,0);

	if(lpTemplate == 0) {
    		APISTR((LF_APIFAIL,"LoadIcon: returns HICON 0\n"));
		return 0;
	}

	if (lpTemplate->hGlobal == 0) {
	    lpMemory = HandleAlloc((LPSTR)lpTemplate->rcsdata,MT_RESOURCE);
	    lpTemplate->hGlobal = lpMemory->hMemory;
	}
	if (lpTemplate->rcsdata == 0)
	    LoadResourceEx(hInstance,lpTemplate,lpMemory);
	    
	/* we are now pointing to the ICONDIRENTRY */
	picon = (ICONDIRENTRY *) lpTemplate->rcsdata;

	for(plast=0;picon->wOrdinalNumber;picon++){
		if(plast && plast->bColorCount > picon->bColorCount)
			continue;
		plast = picon;
	}
	if(plast == 0) {
    		APISTR((LF_APIFAIL,"LoadIcon: returns HICON 0\n"));
		return 0;
	}

	hResNameInfo = FindResource(hInstance, 
		(LPSTR) (unsigned) plast->wOrdinalNumber, RT_ICON);

	if(hResNameInfo == 0) {
    		APISTR((LF_APIFAIL,"LoadIcon: returns HICON 0\n"));
		return 0;
	}

	lpTemplate = (LPNAMEINFO) HandleLock(hResNameInfo,0);

	if(lpTemplate == 0) {
    		APISTR((LF_APIFAIL,"LoadIcon: returns HICON 0\n"));
		return 0;
	}
	if(lpTemplate->hObject) {
    	    	APISTR((LF_APIRET,"LoadIcon: returns HICON %x\n",
			lpTemplate->hObject));
	    	return (HICON)lpTemplate->hObject;
	}

	if (lpTemplate->rcsdata == 0)
	    LoadResourceEx(hInstance,lpTemplate,lpMemory);

	hIcon = LoadIconResource(lpTemplate);
		
	lpTemplate->hObject = (HGDIOBJ)hIcon;

    	APISTR((LF_APIRET,"LoadIcon: returns HICON %x\n",hIcon));
	return hIcon;
}

HCURSOR WINAPI 
LoadCursor(HINSTANCE hInst, LPCSTR lpCursorRsrc)
{
	HCURSOR		hCursor;
	HRSRC		hRsrc;
	LPNAMEINFO  lpTemplate;
	LPMEMORYINFO    lpMemory = (LPMEMORYINFO)NULL;
	CURSORDIRENTRY	*lpcursor;

	APISTR((LF_APICALL, "LoadCursor(HINSTANCE=%x,LPCSTR=%p)\n", 
		hInst, lpCursorRsrc));

	if ( !(hRsrc = FindResource(hInst, lpCursorRsrc, RT_GROUP_CURSOR)) ) {
    		APISTR((LF_APIFAIL,"LoadCursor: returns HCURSOR 0 #1\n"));
		return (HCURSOR)0;
	}

	if (!(lpTemplate = (LPNAMEINFO) HandleLock(hRsrc,0))) {
    		APISTR((LF_APIFAIL,"LoadCursor: returns HCURSOR 0 #2\n"));
		return (HCURSOR)0;
	}

	if (!lpTemplate->hGlobal) {
	    lpMemory = (LPMEMORYINFO)HandleAlloc
			((LPSTR)lpTemplate->rcsdata,MT_RESOURCE);
	    lpTemplate->hGlobal = lpMemory->hMemory;
	}
	if (!lpTemplate->rcsdata)
	    LoadResourceEx(hInst,lpTemplate,lpMemory);

	/* we are now pointing to the CURSORDIRENTRY */
	lpcursor = (CURSORDIRENTRY *)lpTemplate->rcsdata;

	hRsrc = FindResource(hInst,
		(LPSTR)(unsigned)lpcursor->wOrdinalNumber,RT_CURSOR);
	if (!hRsrc) {
    		APISTR((LF_APIFAIL,"LoadCursor: returns HCURSOR 0 #3\n"));
		return (HCURSOR)0;
	}

	if (!(lpTemplate = (LPNAMEINFO) HandleLock(hRsrc,0))) {
    		APISTR((LF_APIFAIL,"LoadCursor: returns HCURSOR 0 #4\n"));
		return (HCURSOR)0;
	}

	if (!lpTemplate->hGlobal) {
	    lpMemory = (LPMEMORYINFO)HandleAlloc
			((LPSTR)lpTemplate->rcsdata,MT_RESOURCE);
	    lpTemplate->hGlobal = lpMemory->hMemory;
	}

	if (lpTemplate->hObject) {
    		APISTR((LF_APIRET,"LoadCursor: returns HCURSOR %x\n",
			lpTemplate->hObject));
	    	return (HCURSOR)lpTemplate->hObject;
	}

	if (lpTemplate->rcsdata == 0)
	    LoadResourceEx(hInst,lpTemplate,lpMemory);

 	hCursor = LoadCursorResource(lpTemplate);

	lpTemplate->hObject = (HGDIOBJ)hCursor;

    	APISTR((LF_APIRET,"LoadCursor: returns HCURSOR %x\n",hCursor));
	return hCursor;
}

HBITMAP
LoadBitmap(HINSTANCE hInstance, LPCSTR lpszBitmap)
{
	HANDLE hResNameInfo;
	LPNAMEINFO  lpTemplate;
	LPMEMORYINFO    lpMemory = (LPMEMORYINFO)NULL;

    	APISTR((LF_APICALL,"LoadBitmap(HINSTANCE=%x,LPCSTR=%p)\n",
		hInstance,lpszBitmap));

	hResNameInfo = FindResource(hInstance, lpszBitmap, RT_BITMAP);

	if(hResNameInfo == 0) {
    		APISTR((LF_APIFAIL,"LoadBitmap: returns HBITMAP %x\n",0));
		return 0;
	}

	lpTemplate = (LPNAMEINFO) HandleLock(hResNameInfo,0);

	if(lpTemplate == 0) {
    		APISTR((LF_APIFAIL,"LoadBitmap: returns HBITMAP %x\n",0));
		return 0;
	}
	if (lpTemplate->hGlobal == 0) {
	    lpMemory = HandleAlloc((LPSTR)lpTemplate->rcsdata,MT_RESOURCE);
	    lpTemplate->hGlobal = lpMemory->hMemory;
	}
	if (lpTemplate->rcsdata == 0)
	    LoadResourceEx(hInstance,lpTemplate,lpMemory);

	if(!lpTemplate->hObject)
	    lpTemplate->hObject = (HGDIOBJ)CreateDIBitmapEx(
			(LPBITMAPIMAGE)lpTemplate->rcsdata,lpTemplate);
	else
	    LOCKGDI(lpTemplate->hObject);

    	APISTR((LF_APIRET,"LoadBitmap: returns HBITMAP %x\n",lpTemplate->hObject));

	return (HBITMAP)lpTemplate->hObject;
}

BOOL
FreeResource(HGLOBAL hResData)
{
	int	ResCount = 0;
	/* Decrement the count on the resdata resource */
    	APISTR((LF_APICALL,"FreeResource(HGLOBAL=%x)\n",hResData));

#ifdef	LATER
	FREEHANDLE(hResData);
	GlobalFree(hResData);
#endif
    	APISTR((LF_APIRET,"FreeResource: returns BOOL %x\n",ResCount));
	return ResCount;
}

#ifdef	STRICT
LPVOID
#else
LPSTR
#endif
LockResource(HANDLE hResData)
{
	return HandleLock(hResData,1);
}

HGLOBAL WINAPI
AllocResource(HINSTANCE hInst, HRSRC hRsrc, DWORD bytes)
{
    	APISTR((LF_APISTUB, "AllocResource(HINSTANCE=%x,HRSRC=%x,DWORD=%x)\n",
		hInst, hRsrc, bytes));
	return((HGLOBAL)NULL);
}

RSRCHDLRPROC WINAPI
SetResourceHandler(HINSTANCE hInst, LPCSTR lpszRsrc, RSRCHDLRPROC proc)
{
    	APISTR((LF_APISTUB, "SetResourceHandler(HINSTANCE=%x,LPCSTR=%p,RSRCHDLRPROC=%x)\n",
		hInst,lpszRsrc,proc));
	return((RSRCHDLRPROC)NULL);
}

/* Some of this code is copied from LoadResource() */
HICON WINAPI CreateIconFromResource(PBYTE presbits, DWORD dwResSize, BOOL fIcon, DWORD dwVer)
{
  LPHANDLEINFO lpHandle;
  LPMEMORYINFO lpMemory;
  LPNAMEINFO pNameInfo;
  LPVOID lpData;

  pNameInfo = (LPNAMEINFO)WinMalloc(sizeof(NAMEINFO));
  lpHandle = HandleAlloc((LPSTR)pNameInfo,MT_NAMEINFO);

  pNameInfo->rcsdata = presbits;
  pNameInfo->rcslength = dwResSize;
  pNameInfo->wType = (WORD)(DWORD)(fIcon ? RT_ICON : RT_CURSOR);
  lpData = (LPVOID)ConvertResourceToNat(pNameInfo->rcsdata,
					pNameInfo->wType);
  lpMemory = HandleAlloc((LPSTR)lpData,MT_RESOURCE);
  lpMemory->wIndex = pNameInfo->wType;
  lpMemory->dwBinSize = pNameInfo->rcslength;
  pNameInfo->hGlobal = lpMemory->hMemory;

  return LoadIconResource(pNameInfo);
}

/*********************************************************/
/***** API Internal Resource Routines ********************/
/*********************************************************/


#define LRT_GET	0
#define LRT_SET 1
#define LRT_PUT 2

typedef struct {
	HINSTANCE	hInstance;
	TYPEINFO	*lpResource;
} INTERNALRESOURCE;

extern TYPEINFO hsmt_resource_USER[];
extern MODULETAB TWIN_ModuleTable[];

TYPEINFO *
LoadResourceTable(int nFunc,HINSTANCE hInst,TYPEINFO *lpResource)
{
	HANDLE hModule;
	LPMODULEINFO modinfo = NULL;

	switch(nFunc) {
	case LRT_SET:
		if(hInst == 0) 
		  lpResource = hsmt_resource_USER;
		else 
		  lpResource = (TYPEINFO*) TWIN_ModuleTable[0].dscr->resource;
		break;

	case LRT_GET:	/* get resource table for given hinstance 	  */
	case LRT_PUT:	/* get/replace resource table for given hinstance */
		if (!hInst) {
		    	hModule = GetModuleHandle("USER");
		} else if (!GETMODULEINFO(hInst)) /* might be hModule already */
		    	hModule = GetModuleFromInstance(hInst);
		else
			hModule = hInst;

		if (!(modinfo = GETMODULEINFO(hModule)))
		    lpResource = (hInst)?(TYPEINFO *)NULL:hsmt_resource_USER;
		else
	            lpResource = modinfo->ResourceTable;

	    	if (modinfo) {
			RELEASEMODULEINFO(modinfo);
	    	}
	    	break;

	default:
	    	break;
	}
	return lpResource;
}

/*********************************************************/
/***** API Resource Routines *****************************/
/*********************************************************/

typedef BOOL (CALLBACK * ENUMRESPROC)(NAMEINFO *, DWORD, LPARAM);

TYPEINFO *
EnumResources(HINSTANCE hInstance,LPCSTR lpType,
		FARPROC lpEnumFunc, LPARAM lParam)
{
	ENUMRESPROC   lpCallBack = 0;
	TYPEINFO *lpResTable;	/* resource table pointer */
  	NAMEINFO *lpResInfo;	/* nameinfo table pointer/resource */
	BOOL	  bByName;
	int       i;
	DWORD	  rval;

	if (HIWORD(lpType) == 0) {
		bByName = FALSE;
	}
	else {
		bByName = TRUE;
	}
	
	for (lpResTable = LoadResourceTable(LRT_GET,hInstance,0);
	     lpResTable && lpResTable->rcstypename;
	     lpResTable++) {
		/* if doing a name check, make sure we have one */
		/* this avoids passing an ordinal to strcasecmp */
		if ( bByName && HIWORD(lpResTable->rcstypename) == 0)
			continue;

		/* compare by ordinal value or matching strings */
		if (!bByName && (lpResTable->rcstypename == lpType))
			break;

		if(bByName && (strcasecmp(lpResTable->rcstypename,lpType) == 0))
			break;
	}

	/* if not found, return NULL */
	if (lpResTable == 0 || lpResTable->rcstypename == 0) {
		return 0;
	}

	if (lpEnumFunc)
		lpCallBack = (ENUMRESPROC)
			MakeProcInstance((FARPROC)lpEnumFunc,hInstance);

	/* search the type, for the specific entry */
	/* note: this could be bByName or by value */
	for (i=0,lpResInfo=lpResTable->rcsinfo;
	    i<(int)(lpResTable->rcscount);
	    i++,lpResInfo++) {
		rval = (DWORD) lpResInfo->rcsitemname;
		if (lpCallBack == 0)
			continue;

		if(HIWORD(rval) == 0)
			rval &= 0x7fff;

	        if (!lpCallBack(lpResInfo, rval, lParam)) {
			break;
		}

	}

	if(lpCallBack)
		FreeProcInstance((FARPROC)lpEnumFunc);
	return lpResTable;
}


/*
 * note:
 *	hiword of both lpname and lptype may be 0
 *	so be prepared to search based on ordinals or by name...
 */
HRSRC
FindResource(HINSTANCE hInstance,LPCSTR lpName,LPCSTR lpType)
{
	TYPEINFO *lpResTable;	/* resource table pointer */
  	NAMEINFO *lpResInfo;	/* nameinfo table pointer/resource */
	LPHANDLEINFO lpHandle; /* pointer to nameinfo table entry */
	HRSRC    hResInfo ;  /* handle of nameinfo table pointer */
	BOOL	  bByName;
	BOOL	  bByOrdinal;
	int       i;
	DWORD	  dwOrdinal = 0;

	APISTR((LF_APICALL,"FindResource(HINSTANCE=%x,LPCSTR=%p,LPCSTR=%p)\n",
		hInstance,lpName,lpType));

	if(HIWORD(lpType) == 0) {
		bByName = FALSE;
	} else {
		bByName = TRUE;
	}
	
	if(HIWORD(lpName) == 0) {
		bByOrdinal = TRUE;
		dwOrdinal = (DWORD)lpName;
	} else {
	    if (lpName[0] == '#') {
	 	dwOrdinal = (DWORD)atol(lpName+1);
		bByOrdinal = TRUE;
	    }
	    else {
		bByOrdinal = FALSE;
	    }
	}
	
	lpResTable = LoadResourceTable(LRT_GET,hInstance,0);

	/* search ResourceTable for Resource Type given by lpType */
	/* note: this could be bByName or by value */
	for(; lpResTable && lpResTable->rcstypename;
	    lpResTable++) {

		/* if doing a name check, make sure we have one */
		/* this avoids passing an ordinal to strcasecmp */
		if ( bByName && HIWORD(lpResTable->rcstypename) == 0)
			continue;

		/* compare by ordinal value or matching strings */
		if(!bByName && (lpResTable->rcstypename == lpType))
			break;

		if(bByName && (strcasecmp(lpResTable->rcstypename,lpType) == 0))
			break;
	}

	/* if not found, return NULL */
	if(lpResTable == 0 || (lpResTable->rcstypename == 0)) {
    		APISTR((LF_APIFAIL,"FindResource: returns HRSRC %x\n",0));
		return 0;
	}

	/* search the type, for the specific entry */
	/* note: this could be bByName or by value */
	for(i=0,lpResInfo=lpResTable->rcsinfo;
	    i<(int)(lpResTable->rcscount);
	    i++,lpResInfo++) {

		if(bByOrdinal) {
		    if(((DWORD)(lpResInfo->rcsitemname) & 0x7fff) == dwOrdinal)
			break;
		} else  {
		    if(HIWORD(lpResInfo->rcsitemname) && 
			    (strcasecmp(lpResInfo->rcsitemname,lpName) == 0))
			break;
		}
	}

	/* if not found return NULL */
	if(i == lpResTable->rcscount) {
    		APISTR((LF_APIFAIL,"FindResource: returns HRSRC %x\n",0));
		return 0;
	}

	/* see if we have already found this resource */
	if(lpResInfo->hRsrc) {
    		APISTR((LF_APIRET,"FindResource: returns HRSRC %x\n",
			lpResInfo->hRsrc));
		return lpResInfo->hRsrc;
	}

	/* allocate a handle to this resource item */
	lpHandle = HandleAlloc((LPSTR)lpResInfo,MT_NAMEINFO);

	if(lpHandle == 0) {
    		APISTR((LF_APIFAIL,"FindResource: returns HRSRC %x\n",0));
		return 0;
	}

	hResInfo = (HRSRC)lpHandle->hMemory;

	/* save the handle for later, don't allocate more than one */
	lpResInfo->hRsrc = hResInfo;

    	APISTR((LF_APIRET,"FindResource: returns HRSRC %x\n",hResInfo));
	return hResInfo;
}

HGLOBAL
LoadResource(HINSTANCE hInstance, HRSRC hResInfo)
{
  	NAMEINFO *lpResInfo;		/* nameinfo table pointer/resource */
        LPMEMORYINFO 	lpMemory = (LPMEMORYINFO)NULL;
	LPVOID lpData;
	
	if((lpResInfo =  (LPNAMEINFO) HandleLock(hResInfo,0)) == 0)
		return 0;

	if (!lpResInfo->hGlobal) {
#ifdef	LATER
	/* the following call does the right thing only for RT_BITMAP.
	   To support other types (RT_DIALOG,RT_MENU), we have to change
	   the corresponding code in the library
	*/
#endif
	    if (lpResInfo->rcsdata)
		lpData = (LPVOID)ConvertResourceToNat(lpResInfo->rcsdata,
					lpResInfo->wType);
	    else
		lpData = (LPVOID)0;
	    lpMemory = HandleAlloc((LPSTR)lpData,MT_RESOURCE);
	    lpMemory->wIndex = lpResInfo->wType;
	    lpMemory->dwBinSize = lpResInfo->rcslength;
	    lpResInfo->hGlobal = lpMemory->hMemory;
	}

	if (lpResInfo->rcsdata == 0) {
	    LoadResourceEx(hInstance,lpResInfo,lpMemory);
	    if (lpResInfo->rcsdata)
		lpMemory->lpCore =
			(LPVOID)ConvertResourceToNat(lpResInfo->rcsdata,
			lpResInfo->wType);
	}

	return lpResInfo->hGlobal;
}

DWORD
SizeofResource(HINSTANCE hinst, HRSRC hrsrc)
{
	return HandleSize(hinst,hrsrc);
}

/* (WIN32) Virtual Memory ************************************************** */

BOOL	WINAPI
VirtualProtect(LPVOID lpAddress, DWORD dwSize,
	DWORD dwAccessFlags, PDWORD pdwOldAccessFlags)
{
	APISTR((LF_API, "VirtualProtect: (API) addr %p size %ld flags %lx\n",
		lpAddress, dwSize, dwAccessFlags));
	return (TRUE);
}

BOOL	WINAPI
VirtualProtectEx(HANDLE hProcess, LPVOID lpAddress, DWORD dwSize,
	DWORD dwAccessFlags, PDWORD pdwOldAccessFlags)
{
	APISTR((LF_API, "VirtualProtectEx: (API) addr %p size %ld flags %lx\n",
		lpAddress, dwSize, dwAccessFlags));
	return (TRUE);
}
