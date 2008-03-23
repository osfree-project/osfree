/*    
	DdeML.c	2.7
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


#include <stdio.h>
#include <string.h>

#include "windows.h"
#include "windowsx.h"
#include "dde.h"
#include "ddeml.h"

#include "kerndef.h"
#include "DdeMLDefs.h"
#include "DPMI.h"
#include "Endian.h"
#include "KrnAtoms.h"
#include "Log.h"



static BOOL RegisterDdeMLClasses(void);
static BOOL IsValidInstanceID(DWORD);
static BOOL IsValidDdeMLHandle(DDEML_HANDLE,UINT);
static LRESULT DdeMLHandleCreate(HWND,CREATESTRUCT *);
static LRESULT DdeMLConvListHandleCreate(HWND,CREATESTRUCT *);
static LRESULT DdeMLClientHandleCreate(HWND,CREATESTRUCT *);
static LRESULT DdeMLServerHandleCreate(HWND,CREATESTRUCT *);
static void DdeSetLastError(LPDDEMLINSTANCE, UINT);
static HWND DdeEstablishConversation(LPDDEMLINSTANCE,HSZ,HSZ,CONVCONTEXT *);
static LPDDEMLCONV DdeMakeNewConv(HWND,CONVCONTEXT *);
static BOOL ConnectEnumProc(HWND,LPARAM);
static UINT DdeMLClientHandleInitACK(HWND,LPDDEMLCONV,WPARAM,LPARAM);
static LRESULT DdeMLSubFrameHandleInit(LPDDEMLINSTANCE, HWND, HWND, LPARAM);

/* **** DLL registration group **** */

UINT WINAPI
DdeInitialize(LPDWORD lpidInst, PFNCALLBACK pfnCallback, DWORD afCmd,
		DWORD uRes)
{
#define	PROFILEBUFLEN	16

    int nShutdownTimeo;
    char ProfileBuf[PROFILEBUFLEN];
    int newIndex;
    LPDDEMLINSTANCE lpInst;
    DWORD dwFlags;

    if (uRes) {
	return DMLERR_INVALIDPARAMETER;
    }

    if (afCmd & APPCLASS_MONITOR) {
	FEATURE_NOT_SUPPORTED("DDEML","Monitor applications not supported");
	return DMLERR_DLL_USAGE;
    }

    if (*lpidInst) {
	if (IsValidInstanceID(*lpidInst)) {
	    newIndex = (int)LOWORD(*lpidInst);
	    dwFlags = (DdeMLInstances[newIndex].dwFlags ^ afCmd) & 0x00C00F00;
	    DdeMLInstances[newIndex].dwFlags = dwFlags ^ afCmd;
	    return DMLERR_NO_ERROR;
	}
	else {
	    return DMLERR_INVALIDPARAMETER;
	}
    }

    if (!DdeMLInitialized) {
	GetProfileString(DdeMLSection,
		ShutdownTimeout_String,
		ShutdownTimeout_Default,
		ProfileBuf,
		PROFILEBUFLEN);
	ShutdownTimeout = (sscanf(ProfileBuf,"%d",&nShutdownTimeo) > 0)?
		nShutdownTimeo:SHUTDOWNTIMEOUT_DEFAULT;

	GetProfileString(DdeMLSection,
		ShutdownRetryTimeout_String,
		ShutdownRetryTimeout_Default,
		ProfileBuf,
		PROFILEBUFLEN);
	ShutdownRetryTimeout = (sscanf(ProfileBuf,"%d",&nShutdownTimeo) > 0)?
		nShutdownTimeo:SHUTDOWNRETRYTIMEOUT_DEFAULT;
	if (!RegisterDdeMLClasses())
	    return DMLERR_SYS_ERROR;
	else
	    DdeMLInitialized = TRUE;
    }

    /* find empty slot in instance table */
    for (lpInst = &DdeMLInstances[0], newIndex = 0;
	 lpInst->lpfnCallback && newIndex < DDEML_MAX_INSTANCES;
	 lpInst++,newIndex++) ;
    if (newIndex == DDEML_MAX_INSTANCES) {
	return DMLERR_LOW_MEMORY;
    }
    *lpidInst = MAKELONG((WORD)newIndex,DDEML_INSTID_MAGIC);

    lpInst->uiIndex = (UINT)newIndex;
    lpInst->dwFlags = afCmd | APPCMD_FILTERINITS;
    lpInst->hTask = GetCurrentTask();
    lpInst->lpfnCallback = pfnCallback;
    if ((lpInst->hWndDdeML = CreateWindow(DdeMLClassName,
		"",
		WS_OVERLAPPED,
		0,0,0,0,
		(HWND)0,
		(HMENU)0,
		(HINSTANCE)0,
		(LPVOID)lpInst)) == (HWND)0) {
	DdeUninitialize(*lpidInst);
	*lpidInst = 0L;
	return DMLERR_SYS_ERROR;
    }

    if (afCmd & APPCLASS_MONITOR) {
	FEATURE_NOT_SUPPORTED("DDEML","Monitor applications not supported");
#ifdef	LATER
	additional logic here
#endif
	return DMLERR_DLL_USAGE;
    }

    if (afCmd & APPCMD_CLIENTONLY)
	return DMLERR_NO_ERROR;

    if ((lpInst->hWndDdeML = CreateWindow(FrameClassName,
		"",
		WS_POPUP,
		0,0,0,0,
		(HWND)0,
		(HMENU)0,
		(HINSTANCE)0,
		(LPVOID)lpInst)) == (HWND)0) {
	DdeUninitialize(*lpidInst);
	*lpidInst = 0L;
	return DMLERR_SYS_ERROR;
    }

    return DMLERR_NO_ERROR;
}

BOOL WINAPI
DdeUninitialize(DWORD idInst)
{
    return TRUE;
}

#ifdef 0
static HDDEDATA
DdeDoCallback(LPDDEMLINSTANCE lpInst, LPDDEMLCONV lpConv,
		UINT uType, UINT uFmt, HSZ hsz1, HSZ hsz2,
		HDDEDATA hDDEData, DWORD dwData1, DWORD dwData2)
{
    UINT uiIndex = (uType & 0xff) >> XTYP_SHIFT;

    if (DdeTransactionFailMasks[uiIndex] & lpInst->dwFlags)
	return (HDDEDATA)0;

    return (HDDEDATA)0;	/* to satisfy the compiler for the time being */
}
#endif

/* **** end of group **** */

/* **** conversation enumeration group **** */

HCONVLIST WINAPI
DdeConnectList(DWORD idInst, HSZ hszService, HSZ hszTopic,
		HCONVLIST hConvList, CONVCONTEXT *lpCC)
{
    return (HCONVLIST)0;
}

HCONV WINAPI
DdeQueryNextServer(HCONVLIST hConvList, HCONV hConvPrev)
{
    LPDDEMLCONV lpConvList,lpPrevConv;
    LPDDEMLINSTANCE lpInst;

    if ((lpConvList = (LPDDEMLCONV)hConvList)) {
	if (!IsValidDdeMLHandle((DDEML_HANDLE)hConvPrev,DDEML_HCONVLIST_MAGIC))
	    return (HCONV)0;
    }
    lpInst = &DdeMLInstances[LOWORD(lpConvList->idInst)];
    lpInst->LastError = DMLERR_NO_ERROR;

    if ((lpPrevConv = (LPDDEMLCONV)hConvPrev)) {
	if (!IsValidDdeMLHandle((DDEML_HANDLE)hConvPrev,DDEML_HCONV_MAGIC)) {
	    DdeSetLastError(lpInst,DMLERR_INVALIDPARAMETER);
	    return (HCONV)0;
	}
	if (lpPrevConv->lpNextConv) {
	    if (lpPrevConv->lpNextConv->hWndConv &&
		(lpConvList->hWndConv ==
			GetParent(lpPrevConv->lpNextConv->hWndConv)))
		return (HCONV)lpPrevConv->lpNextConv;
	    else
		return (HCONV)0;
	}
	else
	    return (HCONV)0;
    }
    else
	return (HCONV)lpConvList->lpNextConv;
}

BOOL WINAPI
DdeDisconnectList(HCONVLIST hConvList)
{
    return (HCONV)0;
}

/* **** end of group **** */

/* **** conversation control group **** */

HCONV WINAPI
DdeConnect(DWORD idInst, HSZ hszService, HSZ hszTopic, CONVCONTEXT *lpCC)
{
    LPDDEMLINSTANCE lpInst;
    HWND hWndConv;

    if (!IsValidInstanceID(idInst))
	return (HCONV)0;

    lpInst = &DdeMLInstances[LOWORD(idInst)];
    lpInst->LastError = DMLERR_NO_ERROR;

    if (lpCC && lpCC->cb != sizeof(CONVCONTEXT)) {
	DdeSetLastError(lpInst, DMLERR_INVALIDPARAMETER);
	return (HCONV)0;
    }

    if (!(hWndConv = DdeEstablishConversation(lpInst,
			hszService,hszTopic,lpCC))) {
	DdeSetLastError(lpInst,DMLERR_NO_CONV_ESTABLISHED);
	return (HCONV)0;
    }

    if (!IsWindow(hWndConv))
	return (HCONV)0;
    else
	return (HCONV)GetWindowLong(hWndConv,DDEMLHCONV_INDEX);
}

static HWND
DdeEstablishConversation(LPDDEMLINSTANCE lpInst,
		HSZ hszService, HSZ hszTopic, CONVCONTEXT *lpCC)
{
    HWND hWndConv;
    LPDDEMLCONV lpConv;

    if (!(hWndConv = CreateWindow(ClientClassName,"",WS_CHILD,
		0,0,0,0,
		lpInst->hWndDdeML,
		(HMENU)0,(HINSTANCE)0,
		(LPVOID)lpCC)))
	return (HWND)0;

    lpConv = (LPDDEMLCONV)GetWindowLong(hWndConv,DDEMLHCONV_INDEX);
    lpConv->idInst = MAKELONG(lpInst->uiIndex,DDEML_INSTID_MAGIC);

    lpConv->ConvInfo.cb = sizeof(CONVINFO);
    lpConv->ConvInfo.hszServiceReq = hszService;
    lpConv->ConvInfo.hszTopic = hszTopic;
    lpConv->ConvInfo.wConvst = XST_INIT1;

    EnumWindows((WNDENUMPROC)ConnectEnumProc,(LPARAM)lpConv);
    if (lpConv->ConvInfo.wConvst != XST_INIT1) {
	lpConv->ConvInfo.wConvst = XST_CONNECTED;
	lpConv->ConvInfo.wStatus |= ST_CONNECTED;
	return hWndConv;
    }
    else {
	DestroyWindow(hWndConv);
	return (HWND)0;
    }
}

static LPDDEMLCONV
DdeMakeNewConv(HWND hWndConv, CONVCONTEXT *lpCC)
{
    LPDDEMLCONV lpConv;

    if (!(lpConv = (LPDDEMLCONV)WinMalloc(sizeof(DDEMLCONV))))
	return (LPDDEMLCONV)0;

    memset((LPSTR)lpConv,'\0',sizeof(DDEMLCONV));
    lpConv->ConvInfo.ConvCtxt = *lpCC;
    lpConv->uMagic = DDEML_HCONV_MAGIC;
    lpConv->hWndConv = hWndConv;
    return lpConv;
}

static BOOL
ConnectEnumProc(HWND hWnd, LPARAM lParam)
{
    LPDDEMLCONV lpConv = (LPDDEMLCONV)lParam;

    SendMessage(hWnd,WM_DDE_INITIATE,(WPARAM)lpConv->hWndConv,
	MAKELPARAM(LOWORD(lpConv->ConvInfo.hszServiceReq),
		   LOWORD(lpConv->ConvInfo.hszTopic)));
    if (lpConv->ConvInfo.wStatus & ST_INLIST ||
	lpConv->ConvInfo.wConvst == XST_INIT1)
	return TRUE;
    else
	return FALSE;
}

BOOL WINAPI
DdeDisconnect(HCONV hConv)
{
    return FALSE;
}

HCONV WINAPI
DdeReconnect(HCONV hConv)
{
    return FALSE;
}

UINT WINAPI
DdeQueryConvInfo(HCONV hConv, DWORD idTransaction, CONVINFO *lpConvInfo)
{
    return (UINT)0;
}

BOOL WINAPI
DdeSetUserHandle(HCONV hConv, DWORD idTransaction, DWORD hUser)
{
    return FALSE;
}

BOOL WINAPI
DdeAbandonTransaction(DWORD idInst, HCONV hConv, DWORD idTransaction)
{
    return FALSE;
}

/* **** end of group **** */

/* **** app server interface group **** */

BOOL WINAPI
DdePostAdvise(DWORD idInst, HSZ hszTopic, HSZ hszItem)
{
    return FALSE;
}

BOOL WINAPI
DdeEnableCallback(DWORD idInst, HCONV hConv, UINT uCmd)
{
    return FALSE;
}

HDDEDATA WINAPI
DdeNameService(DWORD idInst, HSZ hszService, HSZ hszReserved, UINT afCmd)
{
    LPDDEMLINSTANCE lpInst;
    LPDDEMLSERVICE lpService,lpNewService;

    if (hszReserved || !IsValidInstanceID(idInst))
	return (HDDEDATA)0;
    lpInst = &DdeMLInstances[LOWORD(idInst)];
    lpInst->LastError = DMLERR_NO_ERROR;

    if (afCmd & DNS_FILTERON)
	lpInst->dwFlags |= APPCMD_FILTERINITS;
    if (afCmd & DNS_FILTEROFF)
	lpInst->dwFlags &= ~APPCMD_FILTERINITS;

    if (!(afCmd & (DNS_REGISTER|DNS_UNREGISTER)))
	return (HDDEDATA)0;

    if (lpInst->dwFlags & APPCMD_CLIENTONLY) {
	DdeSetLastError(lpInst,DMLERR_DLL_USAGE);
	return (HDDEDATA)0;
    }

    lpService = lpInst->lpService;
    if (!hszService) {
	if (afCmd & DNS_REGISTER) {
	    DdeSetLastError(lpInst,DMLERR_INVALIDPARAMETER);
	    return (HDDEDATA)0;
	}
	/* to be continued... */
    }

    if (afCmd & DNS_REGISTER) {
	lpNewService = (LPDDEMLSERVICE)WinMalloc(sizeof(DDEMLSERVICE));
	lpNewService->lpNextService = (LPDDEMLSERVICE)0;
	lpNewService->hszService = hszService;
	if (lpService)
	    lpService->lpNextService = lpNewService;
	else
	    lpService = lpInst->lpService = lpNewService;
	GlobalLockAtom((ATOM)LOWORD(hszService));
	/* to be continued...
	   send XTYP_REGISTER transactions to all clients */
    }
    else {
	/* to be continued... */
    }
    return (HDDEDATA)1;
}

/* **** end of group **** */

/* **** app client interface group **** */

HDDEDATA WINAPI
DdeClientTransaction(void *lpData, DWORD cbData, HCONV hConv,
		HSZ hszItem, UINT uFmt, UINT uType, DWORD uTimeOut,
		DWORD *lpdwResult)
{
    return (HDDEDATA)0;
}

/* **** end of group **** */

/* **** data transfer group **** */

HDDEDATA WINAPI
DdeCreateDataHandle(DWORD idInst, void *lpSrcBuf, DWORD cbInitData,
		DWORD offSrcBuf, HSZ hszItem, UINT uFmt, UINT afCmd)
{
    return (HDDEDATA)0;
}

HDDEDATA WINAPI
DdeAddData(HDDEDATA hData, void *lpSrcBuf, DWORD cbAddData, DWORD offObj)
{
    return (HDDEDATA)0;
}

DWORD WINAPI
DdeGetData(HDDEDATA hData, void *lpDest, DWORD cbMax, DWORD offSrc)
{
    return (DWORD)0;
}

LPBYTE WINAPI
DdeAccessData(HDDEDATA hData, LPDWORD lpcbData)
{
    LPDDEMLINSTANCE lpInst;
    LPDDEMLDATA lpDDEMLData;

    if (!(lpDDEMLData = (LPDDEMLDATA)hData) ||
	!(lpInst = lpDDEMLData->lpInst))
	return (LPBYTE)0;

    if (!lpDDEMLData->hGlobal) {
	DdeSetLastError(lpInst, DMLERR_INVALIDPARAMETER);
	return (LPBYTE)0;
    }

    lpInst->LastError = DMLERR_NO_ERROR;

    if (lpcbData)
	*lpcbData = GlobalSize(lpDDEMLData->hGlobal);

    return (LPBYTE)GlobalLock(lpDDEMLData->hGlobal);
}

BOOL WINAPI
DdeUnaccessData(HDDEDATA hData)
{
    LPDDEMLINSTANCE lpInst;
    LPDDEMLDATA lpDDEMLData;

    if (!(lpDDEMLData = (LPDDEMLDATA)hData) ||
	!(lpInst = lpDDEMLData->lpInst))
	return FALSE;

    if (!lpDDEMLData->hGlobal) {
	DdeSetLastError(lpInst, DMLERR_INVALIDPARAMETER);
	return FALSE;
    }

    lpInst->LastError = DMLERR_NO_ERROR;
    GlobalUnlock(lpDDEMLData->hGlobal);
    return TRUE;
}

BOOL WINAPI
DdeFreeDataHandle(HDDEDATA hData)
{
    LPDDEMLINSTANCE lpInst;
    LPDDEMLDATA lpDDEMLData;

    if (!(lpDDEMLData = (LPDDEMLDATA)hData) ||
	!(lpInst = lpDDEMLData->lpInst))
	return FALSE;

    if (!lpDDEMLData->hGlobal) {
	DdeSetLastError(lpInst, DMLERR_INVALIDPARAMETER);
	return FALSE;
    }

    lpInst->LastError = DMLERR_NO_ERROR;

    while (GlobalUnlock(lpDDEMLData->hGlobal));

    GlobalFree(lpDDEMLData->hGlobal);
    WinFree((LPSTR)lpDDEMLData);
    return TRUE;
}

UINT WINAPI
DdeGetLastError(DWORD idInst)
{
    UINT uiError = DMLERR_DLL_NOT_INITIALIZED;
    int index;

    if (idInst) {
	if (!IsValidInstanceID(idInst))
	    uiError = DMLERR_INVALIDPARAMETER;
	else {
	    index = (int)LOWORD(idInst);
	    uiError = DdeMLInstances[index].LastError;
	    DdeMLInstances[index].LastError = 0;
	}
    }

    return uiError;
}

HSZ WINAPI
DdeCreateStringHandle(DWORD idInst, LPCSTR lpszString, int codepage)
{
    LPDDEMLINSTANCE lpInst;
    ATOM atmString;
    WORD index;

    if (!IsValidInstanceID(idInst))
	return (HSZ)0;
    index = LOWORD(idInst);
    lpInst = &DdeMLInstances[index];
    lpInst->LastError = DMLERR_NO_ERROR;

    if (!lpszString || strlen(lpszString) == 0)
	return (HSZ)0;

    if (codepage != 0 && codepage != CP_WINANSI &&
	codepage != GetKBCodePage()) {
	DdeSetLastError(lpInst, DMLERR_INVALIDPARAMETER);
	return (HSZ)0;
    }

#ifdef	LATER
	add some monitor stuff
#endif

    if (!(atmString = GlobalAddAtom(lpszString))) {
	DdeSetLastError(lpInst, DMLERR_LOW_MEMORY);
	return (HSZ)0;
    }

    return (HSZ)MAKELONG(atmString,index);
}

DWORD WINAPI
DdeQueryString(DWORD idInst, HSZ hsz, LPSTR lpsz, DWORD cchMax, int codepage)
{
    LPDDEMLINSTANCE lpInst;
    WORD index;
    char buf[256];

    if (!IsValidInstanceID(idInst))
	return 0L;

    index = LOWORD(idInst);
    lpInst = &DdeMLInstances[index];
    lpInst->LastError = DMLERR_NO_ERROR;

    if (codepage != 0 && codepage != CP_WINANSI &&
	codepage != GetKBCodePage()) {
	DdeSetLastError(lpInst, DMLERR_INVALIDPARAMETER);
	return 0L;
    }

    if (!hsz) {
	if (lpsz) *lpsz = 0;
	return 0L;
    }
    else {
	buf[0] = 0;
	GlobalGetAtomName((ATOM)LOWORD(hsz),buf,255);
	if (lpsz) {
	    strncpy(lpsz,buf,min(cchMax-1,255));
	    return (UINT)strlen(lpsz);
	}
	else
	    return strlen(buf);
    }
}

BOOL WINAPI
DdeFreeStringHandle(DWORD idInst, HSZ hsz)
{
    return FALSE;
}

BOOL WINAPI
DdeKeepStringHandle(DWORD idInst, HSZ hsz)
{
    LPDDEMLINSTANCE lpInst;
    ATOM atom;
    WORD index;

    if (!IsValidInstanceID(idInst))
	return FALSE;
    index = LOWORD(idInst);
    lpInst = &DdeMLInstances[index];
    lpInst->LastError = DMLERR_NO_ERROR;

    if (index != HIWORD(hsz)) {
	DdeSetLastError(lpInst, DMLERR_INVALIDPARAMETER);
	return FALSE;
    }
    else
	atom = (ATOM)LOWORD(hsz);

#ifdef	LATER
	add some monitor stuff
#endif

    return (GlobalLockAtom(atom) != (UINT)-1)?TRUE:FALSE;
}

int WINAPI
DdeCmpStringHandles(HSZ hsz1, HSZ hsz2)
{
    return 0;
}

/* **** end of group **** */

/* static routines */

static void
DdeSetLastError(LPDDEMLINSTANCE lpInst, UINT uError)
{
    if (uError != DMLERR_NO_ERROR) {
#ifdef	LATER
	add monitor stuff here
#endif
    }
    lpInst->LastError = uError;
}

static BOOL
RegisterDdeMLClasses(void)
{
    DDEMLWNDCLASS *lpDdeMLWndClass;

    for (lpDdeMLWndClass = &DdeMLClasses[0];
	 lpDdeMLWndClass->WndClass.lpszClassName;
	 lpDdeMLWndClass++)
	if (!(lpDdeMLWndClass->atmClass =
		RegisterClass(&lpDdeMLWndClass->WndClass)))
	    return FALSE;
    return TRUE;
}

static BOOL
IsValidInstanceID(DWORD dwID)
{
/* instance id has DDEML_INSTID_MAGIC in HIWORD and index in the
   DdeMLInstances table in LOWORD
*/
    int index;

    if (HIWORD(dwID) != DDEML_INSTID_MAGIC ||
	(index = (int)LOWORD(dwID)) >= DDEML_MAX_INSTANCES ||
	DdeMLInstances[index].lpfnCallback == 0)
	return FALSE;
    else
	return TRUE;
}

static BOOL
IsValidDdeMLHandle(DDEML_HANDLE hHandle,UINT uMagic)
{
/*	HCONV is a pointer to LPDDEMLCONV,
		which has DDEML_HCONV_MAGIC in it
	HCONVLIST is a pointer to LPDDEMLCONV,
		which has DDEML_HCONVLIST_MAGIC in it
*/
    LPDDEMLCONV lpConv;

    if (!(lpConv = (LPDDEMLCONV)hHandle))
	return FALSE;

    if (lpConv->uMagic != uMagic || !IsWindow(lpConv->hWndConv)) {
	if (IsValidInstanceID(lpConv->idInst))
	    DdeSetLastError(&DdeMLInstances[LOWORD(lpConv->idInst)],
			DMLERR_INVALIDPARAMETER);
	return FALSE;
    }
    else
	return TRUE;
}

/* window procedures */

static LRESULT
ClientWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LPDDEMLCONV lpConv;

    lpConv = (LPDDEMLCONV)GetWindowLong(hWnd,DDEMLHCONV_INDEX);

    switch (uMsg) {
	case WM_CREATE:
	    return HANDLE_WM_CREATE(hWnd,wParam,lParam,
			DdeMLClientHandleCreate);

	case WM_DDE_ACK:
	    if (!lpConv)
		return 0;
	    if (lpConv->ConvInfo.wConvst == XST_INIT1 ||
		lpConv->ConvInfo.wConvst == XST_INIT2) {
		DdeMLClientHandleInitACK(hWnd,lpConv,wParam,lParam);
		return (LRESULT)1;
	    }
	    else {
#ifdef	LATER
	This is a non-init WM_DDE_ACK
	to be continued...
#endif
		return (LRESULT)0;
	    }
	default:
	    return DefWindowProc(hWnd,uMsg,wParam,lParam);
    }
}

static LRESULT
DdeMLClientHandleCreate(HWND hWnd, CREATESTRUCT *lpCreateStruct)
{
    CONVCONTEXT *lpCC;
    LPDDEMLCONV lpConv;

    if ((lpCC = (CONVCONTEXT *)lpCreateStruct->lpCreateParams)) {
	if ((lpConv = DdeMakeNewConv(hWnd,lpCC))) {
	    SetWindowLong(hWnd,DDEMLHCONV_INDEX,(LONG)lpConv);
	    return (LRESULT)1;
	}
    }
    return (LRESULT)-1;
}

static UINT
DdeMLClientHandleInitACK(HWND hWnd, LPDDEMLCONV lpConv,
			WPARAM wParam, LPARAM lParam)
{
    HWND hWndPosting;
    ATOM atmApp, atmTopic;

    if (lpConv->ConvInfo.wConvst == XST_INIT2)
	return (UINT)1;

    if (lpConv->ConvInfo.wConvst != XST_INIT1) {
#ifdef	LATER
	status is not XST_INIT1 or XST_INIT2
	to be continued...
#endif
    }
    lpConv->ConvInfo.wConvst = XST_INIT2;
#ifdef	LATER
	insert some monitor stuff here...
#endif

    hWndPosting = (HWND)wParam;
    atmApp = LOWORD(lParam);
    atmTopic = HIWORD(lParam);
    if ((ATOM)GetClassWord(hWndPosting,GCW_ATOM) ==
	DdeMLClasses[SERVERATOM_INDEX].atmClass) {
	lpConv->ConvInfo.wStatus |= ST_ISLOCAL;
	lpConv->ConvInfo.hConvPartner = (IsWindow(hWndPosting))?
	    (HCONV)GetWindowLong(hWndPosting,DDEMLHCONV_INDEX):(HCONV)0;
    }
    else {
	/* somebody with message-based DDE is talking to us */
	lpConv->ConvInfo.hConvPartner = (HCONV)MAKELONG(hWndPosting,0);
	GlobalLockAtom(atmApp);
	GlobalLockAtom(atmTopic);
    }

    lpConv->ConvInfo.hszSvcPartner = (HSZ)atmApp;
    lpConv->ConvInfo.hszTopic = (HSZ)atmTopic;

    /* to be figured out... */

    GlobalLockAtom((ATOM)LOWORD(lpConv->ConvInfo.hszServiceReq));

    return (UINT)1;
}

static LRESULT
ServerWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
	case WM_CREATE:
	    return HANDLE_WM_CREATE(hWnd,wParam,lParam,
		DdeMLServerHandleCreate);

	default:
	    return DefWindowProc(hWnd,uMsg,wParam,lParam);
    }
}

static LRESULT
DdeMLServerHandleCreate(HWND hWnd, CREATESTRUCT *lpCreateStruct)
{
    LPDDEMLCONV lpConv;
    LPDDEMLINSTANCE lpInst;
    DWORD idInst;

    if (IsValidInstanceID(idInst = (DWORD)lpCreateStruct->lpCreateParams)) {
	lpInst = &DdeMLInstances[LOWORD(idInst)];
	if (!(lpConv = (LPDDEMLCONV)WinMalloc(sizeof(DDEMLCONV)))) {
	    DdeSetLastError(lpInst,DMLERR_MEMORY_ERROR);
	    return (LRESULT)-1;
	}
	memset((LPSTR)lpConv,'\0',sizeof(DDEMLCONV));
	lpConv->uMagic = DDEML_HCONV_MAGIC;
	lpConv->idInst = idInst;
	lpConv->hWndConv = hWnd;
	SetWindowLong(hWnd,DDEMLHCONV_INDEX,(LONG)lpConv);
	return (LRESULT)1;
    }
    return (LRESULT)-1;
}

static LRESULT
DdeMLWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
	case WM_CREATE:
	    return HANDLE_WM_CREATE(hWnd,wParam,lParam,DdeMLHandleCreate);

	default:
	    return DefWindowProc(hWnd,uMsg,wParam,lParam);
    }
}

static LRESULT
DdeMLHandleCreate(HWND hWnd, CREATESTRUCT *lpCreateStruct)
{
    LPDDEMLINSTANCE lpInst;

    if ((lpInst = (LPDDEMLINSTANCE)lpCreateStruct->lpCreateParams)) {
	SetWindowLong(hWnd,DDEMLINSTID_INDEX,(LONG)lpInst->uiIndex);
	return (LRESULT)1;
    }
    else
	return (LRESULT)-1;
}

static LRESULT
ConvListWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg) {
	case WM_CREATE:
	    return HANDLE_WM_CREATE(hWnd,wParam,lParam,
			DdeMLConvListHandleCreate);

	default:
	    return DefWindowProc(hWnd,uMsg,wParam,lParam);
    }
}

static LRESULT
DdeMLConvListHandleCreate(HWND hWnd, CREATESTRUCT *lpCreateStruct)
{
    LPDDEMLCONV lpConv;

    if ((lpConv = (LPDDEMLCONV)lpCreateStruct->lpCreateParams)) {
	SetWindowLong(hWnd,DDEMLHCONV_INDEX,(LONG)lpConv);
	return (LRESULT)1;
    }
    else
	return (LRESULT)-1;
}

static LRESULT
MonWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    return (LRESULT)0;
}

static LRESULT
SubFrameWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    DWORD idInst;
    LPDDEMLINSTANCE lpInst;

    switch (uMsg) {
	case WM_CREATE:
	    return HANDLE_WM_CREATE(hWnd,wParam,lParam,DdeMLHandleCreate);

	case WM_DDE_INITIATE:
	    if (!IsValidInstanceID(
			idInst = GetWindowLong(hWnd,DDEMLINSTID_INDEX)))
		return (LRESULT)0;
	    lpInst = &DdeMLInstances[LOWORD(idInst)];
	    return DdeMLSubFrameHandleInit(lpInst,hWnd,(HWND)wParam,lParam);

	default:
	    return DefWindowProc(hWnd,uMsg,wParam,lParam);
    }
}

static LRESULT
DdeMLSubFrameHandleInit(LPDDEMLINSTANCE lpInst, HWND hWnd, HWND hWndPosting,
			LPARAM lParam)
{
    BOOL bDdeMLConnection,bSameInstance,bWildConnect;
    LPDDEMLCONV lpConv;
    HSZ hszApp,hszTopic;

    if (lpInst->dwFlags & CBF_FAIL_CONNECTIONS)
	return (LRESULT)0;

    if (lpInst->dwFlags & APPCMD_FILTERINITS) {
	/* to be figured out... */
    }

    if (!lpInst->lpService)
	return (LRESULT)0;

    bDdeMLConnection =  ((ATOM)GetClassWord(hWndPosting,GCW_ATOM) ==
	DdeMLClasses[CLIENTATOM_INDEX].atmClass)?TRUE:FALSE;

    if (bDdeMLConnection) {
	lpConv = (LPDDEMLCONV)GetWindowLong(hWndPosting,DDEMLHCONV_INDEX);
	bSameInstance = ((UINT)LOWORD(lpConv->idInst) == lpInst->uiIndex)?
				TRUE:FALSE;
	if (bSameInstance && (lpInst->dwFlags & CBF_FAIL_SELFCONNECTIONS))
	    return (LRESULT)0;

    }

    if ((hszApp = (HSZ)LOWORD(lParam))) {
	if (lpInst->dwFlags & APPCMD_FILTERINITS) {
	    /* to be figured out... */
	}
    }

    hszTopic = (HSZ)HIWORD(lParam);

    bWildConnect = (!hszApp || !hszTopic)?TRUE:FALSE;
    /*
    now send XTYP_CONNECT or XTYP_WILDCONNECT callback
    by calling DoCallback
    */
    /* to be continued... */
   return (LRESULT) 0; /* satisfies the compiler while breathlessly waiting for the sequel */
}

void
IT_DDEINIT (ENV *envp, LONGPROC f)
{
    DWORD dwID, retcode, dwProc;
    LPBYTE lpStruct;

    lpStruct = (LPBYTE)GetAddress(GETWORD(SP+18),GETWORD(SP+16));
    dwID = GETDWORD(lpStruct);
    dwProc = GETDWORD(SP+12);
#ifdef	LATER
	make it normal thunk
#endif
    retcode = (f)(&dwID, dwProc, GETDWORD(SP+8), GETDWORD(SP+4));
    if (retcode == DMLERR_NO_ERROR)
	PUTDWORD(lpStruct,dwID); 
    envp->reg.sp += 2*DWORD_86 + 2*LP_86 + RET_86;
    envp->reg.ax = LOWORD(retcode);
    envp->reg.dx = HIWORD(retcode);
}

