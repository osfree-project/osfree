/*    
	Messages.c	2.61
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
#include <assert.h>
#include <string.h>

#include "windows.h"
#include "windowsx.h"

#include "Messages.h"
#include "kerndef.h"
#include "WinDefs.h"
#include "Resources.h"
#include "Kernel.h"
#include "BinTypes.h"
#include "Log.h"
#include "Hook.h"
#include "Driver.h"

/* internal variables */
static ITSMINFO *lpSendMessageStack = NULL;

BOOL TWIN_InDriverWait = FALSE;

/* imported routines */
extern BOOL TWIN_GetTimerMsg(HWND,HTASK,LPMSG,UINT);
extern DWORD DriverWaitEvent(BOOL);

extern BOOL TWIN_IsLiveTask(HTASK);
extern void TranslateMenuChar(HWND, WPARAM);
extern void WINAPI WSysMenu(HWND);

/* internal routines */
static BOOL AddAppMsgQueue(HTASK, WORD,HWND, UINT, WPARAM, LPARAM);
static BOOL TWIN_DriverMessage(LPMSG , HWND , UINT , UINT ,UINT ,BOOL);
static BOOL MultiPostMsg(HWND, LONG);
static BOOL MultiSendMsg(HWND, LONG);
static BOOL CallGetMessageHooks(LPMSG, UINT);

void TWIN_ReceiveMessage(BOOL yield);

extern ATOM atmGlobalLookup[];

void	TWIN_ResolveAddr(FARPROC , char *);

char   *GetTwinMsgCode(HWND,UINT);

/* API functions */
LRESULT WINAPI
TWIN_CallWindowProc(
#ifdef	STRICT
		WNDPROC lpfnwndproc,
#else
		FARPROC lpfnwndproc,
#endif
		int flg,
		HWND hWnd,UINT message,
		WPARAM w, LPARAM l)
{
	LRESULT	rc;
        char	functionaddress[128];
	char   *p;
	
	if(flg)
		p = "CallWindowProc";
	else
		p = "SendMessage";
	TWIN_ResolveAddr(lpfnwndproc,functionaddress);

	APISTR((LF_APICALL,"%s(HWND=%x,UINT=%x:%s,LPARAM=%x,LPARAM=%x) f()=%s\n",
		p,hWnd,message,
		GetTwinMsgCode(hWnd,message),w,l,
		functionaddress));

	if (!lpfnwndproc) {
		APISTR((LF_APIFAIL,"%s: returns LRESULT %x\n",p,0));
		return 0L;
	}


	if (message == WM_CONVERT  &&  ((WMCONVERT *)l)->targ == NULL)
		((WMCONVERT *)l)->targ = (LONGPROC)lpfnwndproc;

	rc =  lpfnwndproc(hWnd,message,w,l);

	APISTR((LF_APIRET,"%s: returns LRESULT %x\n",p,rc));

	return rc;
}

LRESULT WINAPI
CallWindowProc(
#ifdef	STRICT
		WNDPROC lpfnwndproc,
#else
		FARPROC lpfnwndproc,
#endif
		HWND hWnd,UINT message,
		WPARAM w, LPARAM l)
{
	return TWIN_CallWindowProc(lpfnwndproc,1,hWnd,message,w,l);
}

LONG WINAPI
DispatchMessage(const MSG *lpMsg)
{
    TIMERPROC	lpfnTimerProc;
    LONG	rc = 0L;

    APISTR((LF_APICALL,"DispatchMessage(MSG *%x)\n", lpMsg));

    if((lpMsg->message == WM_TIMER || lpMsg->message == WM_SYSTIMER) && 
	(lpfnTimerProc = (TIMERPROC)lpMsg->lParam)) {

        LOGSTR((LF_MSGRET,"[HWND=%x,UINT=%x:%s,LPARAM=%x,LPARAM=%x]\n",
		lpMsg->hwnd,lpMsg->message,
		GetTwinMsgCode(lpMsg->hwnd, lpMsg->message),
		lpMsg->wParam,lpMsg->lParam));

	lpfnTimerProc(lpMsg->hwnd,lpMsg->message,(UINT)lpMsg->wParam,
			GetTickCount());
        APISTR((LF_APIRET,"DispatchMessage: returns LONG 1L\n"));
	return 1L;
    }

    if (lpMsg->hwnd)
	rc =  SendMessage(lpMsg->hwnd,lpMsg->message,
			   lpMsg->wParam,lpMsg->lParam);
    APISTR((LF_APIRET,"DispatchMessage: returns LONG %d\n",rc));
    return 0L;
}

static	DWORD	LastMessagePos;
static	LONG	LastMessageTime;

#ifdef LATER
	Driver will update position/time values
	Messages will use the current values to fill into a message
#endif

DWORD   WINAPI 
GetMessagePos(void)
{
	DWORD  rc;
    	APISTR((LF_APICALL,"GetMessagePos()\n"));
	rc =  (DWORD) LastMessagePos;
    	APISTR((LF_APIRET,"GetMessagePos: returns DWORD %d\n",rc));
	return rc;
}

LONG    WINAPI 
GetMessageTime(void)
{
	LONG  rc;
    	APISTR((LF_APICALL,"GetMessageTime()\n"));
	rc = LastMessageTime;
    	APISTR((LF_APIRET,"GetMessageTime: returns LONG %d\n",rc));
	return rc;
}

WORD DebugMessage = 0x201;

BOOL WINAPI
GetMessage(LPMSG lpMsg, HWND hWnd, UINT uMin, UINT uMax)
{
    DWORD	pos=0;
    BOOL	rc;

    APISTR((LF_APICALL,"GetMessage(LPMSG=%x,HWND=%x,UINT=%x,UINT=%x)\n",
	lpMsg,hWnd,uMin,uMax));

    TWIN_DriverMessage(lpMsg,hWnd,uMin,uMax,PM_REMOVE,FALSE);
    if(DebugMessage == lpMsg->message)
	pos++;

    if((pos = MAKELONG( lpMsg->pt.x,lpMsg->pt.y)))
	LastMessagePos = pos;	

    if(lpMsg->time)
    	LastMessageTime = lpMsg->time;
    
    LOGSTR((LF_MSGRET,"[HWND=%x,UINT=%x:%s,LPARAM=%x,LPARAM=%x]\n",
	lpMsg->hwnd,lpMsg->message,
	GetTwinMsgCode(lpMsg->hwnd, lpMsg->message),
	lpMsg->wParam,lpMsg->lParam));

    rc = lpMsg->message != WM_QUIT;
    APISTR((LF_APIRET,"GetMessage: returns BOOL %x\n",rc));
    return rc;
}

BOOL WINAPI
PeekMessage(LPMSG lpMsg, HWND hWnd, UINT uMin, UINT uMax,UINT uFlg)
{
    BOOL bRet;
    static MSG msg;
    static BOOL bState;

    APISTR((LF_APICALL,"PeekMessage(LPMSG=%x,HWND=%x,UINT=%x,UINT=%x,UINT=%x)\n",
	lpMsg,hWnd,uMin,uMax,uFlg));

    if ((bRet = TWIN_DriverMessage(lpMsg,hWnd,uMin,uMax,uFlg,TRUE))) {
	if (lpHookList[WH_GETMESSAGE+1]) {
	    lpHookList[WH_GETMESSAGE+1]->lpfnHookProc(0,0,(LPARAM)lpMsg);
	}
    }

    if (bRet) {
	msg = *lpMsg;
	bState = FALSE;
       LOGSTR((LF_MSGRET,"[HWND=%x,UINT=%x:%s,LPARAM=%x,LPARAM=%x]\n",
		lpMsg->hwnd,lpMsg->message,
		GetTwinMsgCode(lpMsg->hwnd, lpMsg->message),
		lpMsg->wParam,lpMsg->lParam));
        APISTR((LF_APIRET,"PeekMessage: returns BOOL %x\n",TRUE));
	return TRUE;
    }
    else {
	if (bState && uMin == WM_MOUSEMOVE && uMax == WM_MOUSEMOVE) {
	    *lpMsg = msg;
	    lpMsg->message = WM_MOUSEMOVE;
	    bState = FALSE;
       LOGSTR((LF_MSGRET,"[HWND=%x,UINT=%x:%s,LPARAM=%x,LPARAM=%x]\n",
		lpMsg->hwnd,lpMsg->message,
		GetTwinMsgCode(lpMsg->hwnd, lpMsg->message),
		lpMsg->wParam,lpMsg->lParam));
            APISTR((LF_APIRET,"PeekMessage: returns BOOL %x\n",TRUE));
	    return TRUE;
	}
	if (msg.message == WM_MOUSEMOVE) {
	    msg.message = WM_ENTERIDLE;
	    bState = TRUE;
	}
	else
	    bState = FALSE;

       LOGSTR((LF_MSGRET,"[HWND=%x,UINT=%x:%s,LPARAM=%x,LPARAM=%x]\n",
		lpMsg->hwnd,lpMsg->message,
		GetTwinMsgCode(lpMsg->hwnd, lpMsg->message),
		lpMsg->wParam,lpMsg->lParam));
        APISTR((LF_APIRET,"PeekMessage: returns BOOL %x\n",FALSE));
	return FALSE;
    }
}

void WINAPI
WaitMessage(void)
{
#ifdef	LATER
	does not see messages already peeked but not removed
#endif
	MSG	msg;
	
        APISTR((LF_APICALL,"WaitMessage()\n"));
        TWIN_DriverMessage(&msg,0,0,0,PM_NOREMOVE,FALSE);
        APISTR((LF_APIRET,"WaitMessage: returns void\n"));
}

static BOOL
CallGetMessageHooks (LPMSG lpMsg, UINT uiFlags)
{
    UINT	ret;
    BOOL	rc;
    UINT	uiHookCode;
    MOUSEHOOKSTRUCT mhs;
    EVENTMSG	em;

    uiHookCode = (uiFlags & PM_REMOVE)?HC_ACTION:HC_NOREMOVE;

    if (lpHookList[WH_GETMESSAGE+1]) {
	lpHookList[WH_GETMESSAGE+1]->lpfnHookProc(uiHookCode,0,(LPARAM)lpMsg);
    }

    ret = IsMouseOrKeyboardMsg(lpMsg->message);

    if (ret && lpHookList[WH_JOURNALRECORD+1]) {
	em.message = lpMsg->message;
	em.time = lpMsg->time;
	if (ret == WM_KEYFIRST) {	/* keyboard event */
	    em.paramL = LOBYTE(lpMsg->wParam);		/* virtual key code */
	    em.paramL |= LOBYTE(HIWORD(lpMsg->lParam));	/* scan code */
	    em.paramH = LOWORD(lpMsg->lParam);		/* repeat count */
	    em.paramH |= HIWORD(lpMsg->lParam) & 0x8000;/* extended key */
	}
	else {				/* mouse event */
	    em.paramL = lpMsg->pt.x;
	    em.paramH = lpMsg->pt.y;
	}
	lpHookList[WH_JOURNALRECORD+1]->lpfnHookProc(HC_ACTION,(WPARAM)0,
				(LPARAM)&em);
    }

    if (lpHookList[WH_KEYBOARD+1] && (ret == WM_KEYFIRST)) {
	rc = lpHookList[WH_KEYBOARD+1]->lpfnHookProc(uiHookCode,
				lpMsg->wParam,lpMsg->lParam);
	if (rc == TRUE)
	    return rc;
    }

    if (lpHookList[WH_MOUSE+1] && (ret == WM_MOUSEFIRST)) {
	mhs.pt = lpMsg->pt;
	mhs.hwnd = lpMsg->hwnd;
	mhs.wHitTestCode = HTCLIENT;
#ifdef LATER
	We should figure out how to access the actual hit code
#endif
	rc = lpHookList[WH_MOUSE+1]->lpfnHookProc(uiHookCode,
				(WPARAM)lpMsg->message,(LPARAM)&mhs);
	if (rc == TRUE)
	    return rc;
    }
    return FALSE;
}

BOOL
TWIN_DriverMessage(LPMSG lpMsg, HWND hWnd,
	      UINT uMin, UINT uMax,UINT uFlg,BOOL bNoWait)
{
    DWORD   dwerv;
    HANDLE  hTask;
    LPQUEUE lpQueue;

    if (hWnd && !IsWindow(hWnd))
	hWnd = 0;

    if (hWnd)
	hTask = GetWindowTask(hWnd);
    else
	hTask = GetCurrentTask();
    lpQueue = QueueGetPtr(hTask);

    /******************************************/
    /* what is this?  it is not called...     */
    /*hFocusTask = GetWindowTask(GetFocus()); */
    /******************************************/

  lpMsg->hwnd = 0;
  labLoop:
    while(1) {
	while (lpSendMessageStack && 
	       lpSendMessageStack->hReceivingTask == GetCurrentTask() &&
	       !lpSendMessageStack->bSendReceived)
	{
	    TWIN_ReceiveMessage(FALSE);
	}

	/* try for a message from application queue */
	if (QueueGetMsg(lpQueue, lpMsg, hWnd, uMin, uMax, uFlg)) {
	    break;
	}

	/* try for a message from system queue */
	if (QueueGetMsg(0,lpMsg,hWnd,uMin,uMax,uFlg)) {
	    break;
	}

	if (uMin <= WM_PAINT && (!uMax || uMax >= WM_PAINT)) {
	    /* finally, check if the window needs a paint message */
	    if(lpQueue->wQueueFlags & QFPAINT) {
	      labTryNext:
		if((lpMsg->hwnd = InternalUpdateWindows())) {
		    if (TestWF(lpMsg->hwnd, WFNCDIRTY)) {
			if (NonEmptyNCRect(lpMsg->hwnd)) {
			    lpMsg->message = WM_NCPAINT;
			    lpMsg->wParam = 0;
			    lpMsg->lParam = 0L;
			    break;
			}
			else {
			    ClearWF(lpMsg->hwnd, WFNCDIRTY);
			}
		    }
		    if (TestWF(lpMsg->hwnd, WFDIRTY)) {
			if (IsIconic(lpMsg->hwnd) && 
			    GetClassIcon(lpMsg->hwnd)) {
			    lpMsg->message = WM_PAINTICON;
			    lpMsg->wParam = 1;
			}
			else {
			    lpMsg->message = WM_PAINT;
			    lpMsg->wParam = 0;
			}
			lpMsg->lParam = 0L;
			break;
		    }
		    else
			goto labTryNext;
		}
		lpQueue->wQueueFlags &= ~QFPAINT;
	    }
	}

	if ((uMin <= WM_TIMER && (!uMax || uMax >= WM_TIMER)) ||
	    (uMin <= WM_SYSTIMER && (!uMax || uMax >= WM_SYSTIMER))) {
	    if(lpQueue->wQueueFlags & QFTIMER) {
		if (TWIN_GetTimerMsg(hWnd,hTask,lpMsg,uFlg)) {
		    break;
		}
	    }
	}

	/* none of the above, so see if system is ready. */
	if (!TWIN_InDriverWait)
	{

	    TWIN_InDriverWait = TRUE;
	    dwerv = DriverWaitEvent(TRUE);
	    TWIN_InDriverWait = FALSE;

	    if (!dwerv && !bNoWait)
	    {
		/*
		 *  The code here used to call ReadyTask(GetCurrentTask())
		 *  before calling InternalYield(), but that results in
		 *  a solid-run condition if more than one task is
		 *  defined, because it results in the other task looking
		 *  like it's ready to run, when all tasks should be waiting
		 *  on a driver event.  So we yield if necessary, but do
		 *  not set ourselves as ready-to-run if we do.  We will
		 *  be run again when we first get a message in our queue.
		 */
#if defined(MAC_TASKING_PATCH)
		ReadyTask(GetCurrentTask());
#endif
		if (!InternalYield())
		{
		    TWIN_InDriverWait = TRUE;
		    (void)DriverWaitEvent(FALSE);
		    TWIN_InDriverWait = FALSE;
		}
	    }
	    else if (!(uFlg & PM_NOYIELD))
		Yield();
	}
	else if (!(uFlg & PM_NOYIELD))
	    InternalYield();

	if(bNoWait)
	    return FALSE;
    }
    if (CallGetMessageHooks(lpMsg,uFlg & PM_REMOVE))
	goto labLoop;
    return TRUE;
}

BOOL WINAPI
PostAppMessage(HTASK hTask,UINT wMsg,WPARAM wParam,LPARAM lParam)
{
	 BOOL rc;
	 APISTR((LF_APICALL,"PostAppMessage(...)\n"));
	 rc = AddAppMsgQueue(hTask,-1,0,wMsg,wParam,lParam);
    	 APISTR((LF_APIRET,"PostAppMessage: returns BOOL %d\n",rc));
	 return rc;
 }

BOOL WINAPI
PostMessage(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
    HTASK hTask;
    BOOL  rc;

    APISTR((LF_APICALL,
	    "PostMessage(HWND=%x,UINT=%x:%s,WPARAM=%x,LPARAM=%x)\n", 
		hWnd,wMsg,GetTwinMsgCode(hWnd,wMsg),wParam,lParam));

    if (hWnd != 0xffff)
	hTask = GetWindowTask(hWnd);
    else
	hTask = GetCurrentTask();

    rc = AddAppMsgQueue(hTask,-1,hWnd,wMsg,wParam,lParam);
    APISTR((LF_APIRET,"PostMessage: returns BOOL %d\n",rc));
    return rc;
}

void WINAPI
PostQuitMessage(int nExitCode)
{
    APISTR((LF_APICALL,"PostQuitMessage(int %x)\n", nExitCode));
    (void) AddAppMsgQueue(GetCurrentTask(),0,0,WM_QUIT,nExitCode,0);
    APISTR((LF_APIRET,"PostQuitMessage: returns void\n"));
}

static BOOL bInSendMessage;

void    WINAPI 
ReplyMessage(LRESULT lParam)
{
    if (lpSendMessageStack && 
	lpSendMessageStack->hReceivingTask == GetCurrentTask())
    {
	lpSendMessageStack->lResult = lParam;
	lpSendMessageStack->bSendCompleted = TRUE;
	DirectedYield(lpSendMessageStack->hSendingTask);
    }
}

BOOL
TWIN_SendMessagePendingForTask(HTASK hTask)
{
    if (lpSendMessageStack && 
	lpSendMessageStack->hReceivingTask == hTask)
    {
	return TRUE;
    }
    else
	return FALSE;
}

BOOL WINAPI
QuerySendMessage(HANDLE hReserved1, HANDLE hReserved2, HANDLE hReserved3,
		 LPMSG lpMessage)
{
    return FALSE;
}

BOOL    WINAPI 
InSendMessage(void)
{
    return bInSendMessage;
}

BOOL TWIN_IsSafeToMessageBox()
{
    if (lpSendMessageStack && 
	lpSendMessageStack->hReceivingTask == GetCurrentTask() &&
	!lpSendMessageStack->bSendReceived)
    {
	return FALSE;
    }
    else
	return TRUE;
}

void TWIN_ReceiveMessage(BOOL yield)
{
    LRESULT lResult;
    MSG *lpMsg;
    
    if (!lpSendMessageStack)
	return;

    if (lpSendMessageStack->hReceivingTask == GetCurrentTask() &&
	!lpSendMessageStack->bSendCompleted &&
	!lpSendMessageStack->bSendReceived)
    {
	ITSMINFO *saved_ptr = lpSendMessageStack;
	int saved_msg = lpSendMessageStack->msg.message;
	
	/* We are being sent a message. */
	lpSendMessageStack->bSendReceived = TRUE;
	lpMsg = &lpSendMessageStack->msg;
	lResult = SendMessage(lpMsg->hwnd, lpMsg->message, lpMsg->wParam,
			      lpMsg->lParam);

	if (saved_ptr == lpSendMessageStack && 
	    saved_msg == lpSendMessageStack->msg.message)
	{
	    ReplyMessage(lResult);
	}
	else if (yield)
	{
	    Yield();
	}
    }
    else if (yield)
    {
	Yield();
    }
}

LRESULT WINAPI
SendMessage(HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam)
{
#ifdef	STRICT
	WNDPROC lpfnWndProc;
#else
	FARPROC lpfnWndProc;
#endif
	DWORD rc;
        BOOL      cInSendMessage;
	MSG	  msg;
	HWND32	  hWnd32;

#ifdef	DEBUG
	memset((LPSTR)&msg,'\0',sizeof(MSG));
#endif

    	if(hWnd == HWND_BROADCAST) {

		msg.message = wMsg;
		msg.wParam  = wParam;
		msg.lParam  = lParam;

		/*send to all top level windows */
		return EnumWindows((WNDENUMPROC)MultiSendMsg,(LONG)&msg);
    	}

	/* 
	 * Is this a valid window? 
	 */
	if (!(hWnd32 = CHECKHWND32(hWnd)))
	    return 0;
	
	/*
	 * Does the window have a valid task?
	 */
	if (GetWindowTask(hWnd) == 0 || 
	    !TWIN_IsLiveTask(GetWindowTask(hWnd))) {
	    RELEASEWININFO(hWnd32);
	    return 0;
	 }

	/* 
	 * Is this window dead? 
	 */
	if (hWnd32->dwWinFlags & WFDEAD) {
	    RELEASEWININFO(hWnd32);
	    return 0;
	}

	/* 
	 * Window is a good destination.  Intertask send? 
	 */
	if (hWnd != 0 && GetWindowTask(hWnd) != GetCurrentTask())
	{
	    ITSMINFO smse;
	    
	    /* save current InSendMessage Flag */
	    cInSendMessage = bInSendMessage;

	    /* we are now in sendmessage */
	    bInSendMessage = TRUE;

	    smse.hSendingTask = GetCurrentTask();
	    smse.hReceivingTask = GetWindowTask(hWnd);
	    smse.msg.hwnd = hWnd;
	    smse.msg.message = wMsg;
	    smse.msg.wParam = wParam;
	    smse.msg.lParam = lParam;
	    smse.bSendReceived = FALSE;
	    smse.bSendCompleted = FALSE;
	    smse.lpPrev = lpSendMessageStack;
	    lpSendMessageStack = &smse;
	    
	    DirectedYield(smse.hReceivingTask);
	    while (!smse.bSendCompleted)
	    {
		TWIN_ReceiveMessage(TRUE);
	    }

	    lpSendMessageStack = smse.lpPrev;

	    /* back to what it was before... */
	    bInSendMessage = cInSendMessage;

	    RELEASEWININFO(hWnd32);
	    return smse.lResult;
	}

    	if ((lpfnWndProc =
#ifdef	STRICT
		(WNDPROC)
#else
		(FARPROC)
#endif
			GetWindowLong(hWnd, GWL_WNDPROC)) == NULL) {
	    	ERRSTR((LF_WARNING,"SendMessage: Null wndproc!\n"));
		RELEASEWININFO(hWnd32);
		return(0L);
    	}

    	/* save current InSendMessage Flag */
    	cInSendMessage = bInSendMessage;

    	/* we are now in sendmessage */
    	bInSendMessage = TRUE;
    	if (lpHookList[WH_CALLWNDPROC+1]) {
		ATOM atmClassName;
		HOOKINFO hki;
	
		if ((atmClassName = GetClassWord(hWnd,GCW_ATOM)) !=
			atmGlobalLookup[LOOKUP_FRAME]) {
	    	hki.hWnd = hWnd;
	    	hki.msg = wMsg;
	    	hki.wParam  = wParam;
	    	hki.lParam  = lParam;

		msg.hwnd = hWnd;
		msg.message = wMsg;
		msg.wParam = wParam;
		msg.lParam = lParam;
	    	lpHookList[WH_CALLWNDPROC+1]->lpfnHookProc(0,0,(LPARAM)&hki);
		}
    	}

    	rc = TWIN_CallWindowProc(lpfnWndProc,0,hWnd, wMsg, wParam, lParam);

	RELEASEWININFO(hWnd32);
    	/* back to what it was before... */
    	bInSendMessage = cInSendMessage;
    	return rc;
}

/*
 * translate the message given the handle to an accelerator table
 * LATER
 *	must handle windows that have sysmenu or mdi windows with menu
 *	from the menu we should get an accelerator, (do we just build one,
 *	or do we walk each menu item.
 * LATER
 *	if the window is minimized, we don't do some processing
 * LATER
 *	does translateaccelerator work on keyup for some things?
 *	like menu?
 */

int WINAPI
TranslateAccelerator(HWND hWnd, HACCEL hAcc, LPMSG lpMsg) 
{
    ACCELENTRY *lpData;
    WORD wEvent;
    int rc,code;
    int keystate;

    /* we must have an accelerator table. */
    if(hAcc == 0)
	return 0;

    /* must be one of keydown or syskey down, we do not do keyup */
    if(lpMsg->message == WM_KEYDOWN || lpMsg->message == WM_SYSKEYDOWN) {
      
       int char_code = DRVCALL_KEYBOARD(PKH_GETCHAR, lpMsg->wParam, 0, 0);

	/* these don't generate accelerators */
        if (lpMsg->wParam == VK_SHIFT)
	   return 0;
	if (lpMsg->wParam == VK_CONTROL)
	   return 0;
	if (lpMsg->wParam == 0x2a)
	   return 0;

	/* lock the handle to the accelerators */
	lpData = (ACCELENTRY *) LockResource(hAcc);

	/* if we have one, then lets try to match it up */
	if(lpData) {

		/* setup initial state of keyboard */
		if (GetKeyState(VK_CONTROL))
		    keystate = 8;
		else 
		    keystate = 0;

		/* what is initial keystate */
		if (GetKeyState(VK_SHIFT))
		    keystate |= 4;

		/* loop through the accelerators */
		while (TRUE) {

			/* what event are we looking for */
			rc   = 0;
			code = 0;
			wEvent = lpMsg->wParam;

			/* vk_key or ascii event */
			if ((lpData->fFlags & 1) == 0) {
			  /* the event we match will be ascii */
			  if (char_code != 0) /* compare with ascii code */
			    wEvent = char_code;
			  if ((keystate & 0x8))  /* control key down */
			    wEvent &= 0x1f;  /* mask off to ctrl range */
			}
			else {   /*  the following applies to VK's only */

			  /* Account for control & shift */
			  rc |= (keystate & 0xc);

			  /* test alt key */
			  if (lpMsg->lParam & 0x20000000)
			      rc |= 0x10;

			  /* Account for acc. needing shift, ctrl, alt. */
			  code |= (lpData->fFlags & 0x1c);
			}

			/* if we matched the event              */
			/* we should have done this first       */
			/* after checking for ascii vs. virtual */
			if (wEvent != lpData->wEvent)
			    rc = -1;

			/* we have a matching key combination   */
			/* and the event matches                */
			if (rc == code) { 

			    /* send the corresponding event */
			    SendMessage(hWnd,WM_COMMAND,
				GET_WM_COMMAND_MPS(lpData->wId,0,1));

			    /* unlock table and return TRUE */
			    UnlockResource(hAcc);
			    return 1;			
			}
		    
		    if (lpData->fFlags & 0x80)
			break;
		    else
			lpData++;
		}
		UnlockResource(hAcc);
	  }
    }

    return 0;
}

BOOL WINAPI
TranslateMDISysAccel(HWND hwndClient, MSG *lpmsg)
{
	APISTR((LF_APISTUB,
	    "TranslateMDISysAccel(HWND=%x, MSG *=%p)\n",hwndClient,lpmsg));
	return 0;
}

BOOL WINAPI
TranslateMessage(const MSG *lpMsg)
{
	MSG msgChar;
    	APISTR((LF_APICALL,"TranslateMessage(MSG *=%p)\n", lpMsg));

	/* post WM_CHAR message, if needed */
	if (lpMsg->message == WM_KEYDOWN || lpMsg->message == WM_KEYUP ||
	    lpMsg->message == WM_SYSKEYDOWN || lpMsg->message == WM_SYSKEYUP) {
	    if (lpMsg->message == WM_KEYDOWN ||
		lpMsg->message == WM_SYSKEYDOWN) {
		if (lpMsg->lParam & 0x20000000L) { /* ALT key pressed */
			switch ( lpMsg->wParam )
			{
				default:
					TranslateMenuChar ( GetTopLevelAncestor ( lpMsg->hwnd ),
					                    lpMsg->wParam );
					break;

				case VK_F4:
					PostMessage ( GetTopLevelAncestor ( lpMsg->hwnd ),
					              WM_SYSCOMMAND, SC_CLOSE, 0 );
					break;

				case VK_F10:
					WSysMenu ( lpMsg->hwnd );
					break;

			}

			msgChar = *lpMsg;

			msgChar.message = WM_SYSCHAR;
			if (0 !=(msgChar.wParam = DRVCALL_KEYBOARD(PKH_GETCHAR,
			    lpMsg->wParam,0,0)))
			    	QueueAddMsg(0,0,&msgChar);
		}
		else {
		    MSG msgChar = *lpMsg;

		    msgChar.message = WM_CHAR;
		    if (0 != (msgChar.wParam = DRVCALL_KEYBOARD(PKH_GETCHAR,
				lpMsg->wParam,0,0)))
			QueueAddMsg(0,0,&msgChar);
		}
	    }
    	    APISTR((LF_APIRET,"TranslateMessage: returns BOOL 1\n"));
	    return TRUE;
	}
	
    	APISTR((LF_APIRET,"TranslateMessage: returns BOOL 0\n"));
	return FALSE;
}

#ifdef	TWIN32
BOOL	WINAPI
#else
void	WINAPI
#endif
MessageBeep(UINT uMessageBeep)
{

#ifdef	TWIN32
	int	nFlags = SND_ASYNC;
	LPSTR	MsgStr = 0;
	BOOL bBeep     = FALSE;
	FARPROC		soundfunc;
	HINSTANCE	hInst;


	APISTR((LF_APICALL, "MessageBeep(UINT=%x)\n", uMessageBeep));

	bBeep = FALSE;

	if (uMessageBeep == MB_OK)
	{
		MsgStr = "SystemDefault";
		nFlags = SND_ASYNC|SND_NODEFAULT;
	}
	else if (uMessageBeep == MB_ICONHAND)
	{
		MsgStr = "SystemHand";
	}
	else if (uMessageBeep == MB_ICONQUESTION)
	{
		MsgStr = "SystemQuestion";
	}
	else if (uMessageBeep == MB_ICONEXCLAMATION)
	{
		MsgStr = "SystemExclamation";
	}
	else if (uMessageBeep == MB_ICONASTERISK)
	{
		MsgStr = "SystemAsterisk";
	}

	//bBeep = sndPlaySound(MsgStr, nFlags);

	hInst = LoadLibrary("mmsystem");

	soundfunc = GetProcAddress(hInst,"sndPlaySound");

	if(soundfunc)
		bBeep = soundfunc( MsgStr, nFlags); 

	if (!bBeep || (uMessageBeep == (UINT) -1))
	{
		(void) DRVCALL_EVENTS(PEH_BEEP, uMessageBeep, 0, 0);
		APISTR((LF_APIRET, "MessageBeep: returns BOOL TRUE\n"));
		return (bBeep);
	}
	return (bBeep);
#else
	(void) DRVCALL_EVENTS(PEH_BEEP, uMessageBeep, 0, 0);
#endif
	APISTR((LF_APIRET, "MessageBeep: returns void\n"));
}

/* Supplemental routines */

static BOOL
AddAppMsgQueue(HTASK hTask,WORD loc,HWND hWnd,UINT wMsg,
				WPARAM wParam,LPARAM lParam)
{
	MSG	msg;

	/* can handle case of hWnd == 0 or hWnd == HWND_BROADCAST */
	/* hWnd == 0		my queue  	 */
	/* hWnd == HWND_BROADCAST	all top windows  */

	/* if driver is blocked (e.g., via select()) waiting for an event,
	 * then Post*Message() won't work, so tell driver to unblock first.
	 */
	DRVCALL_EVENTS(PEH_UNBLOCKSELECTWAIT, 0, 0, 0);

#ifdef	DEBUG
	memset((LPSTR)&msg,'\0',sizeof(MSG));
#endif

	msg.message = wMsg;
	msg.wParam  = wParam;
	msg.lParam  = lParam;

        if(hWnd == HWND_BROADCAST) {
		/*send to all top level windows */
		return EnumWindows((WNDENUMPROC)MultiPostMsg,
			(LONG)(LPMSG) &msg);
        }
	else {
	    	HWND32 hWnd32 = CHECKHWND32(hWnd);

		if (hWnd32 && (hWnd32->dwWinFlags & WFDEAD)) {
		        RELEASEWININFO(hWnd32);
		    	return 0;
		}
	}
	
	msg.hwnd = hWnd;

	RELEASEWININFO(hWnd32);

	return(QueueAddMsg(hTask,loc,&msg));
}

static BOOL
MultiPostMsg(HWND hWnd,LONG lpMsg)
{
	(void) PostMessage(hWnd,((LPMSG)lpMsg)->message,
			((LPMSG)lpMsg)->wParam,
			((LPMSG)lpMsg)->lParam); 
	return TRUE;
}

static BOOL
MultiSendMsg(HWND hWnd,LONG lpMsg)
{
	(void) SendMessage(hWnd,((LPMSG)lpMsg)->message,
			((LPMSG)lpMsg)->wParam,
			((LPMSG)lpMsg)->lParam); 
	return TRUE;
}

BOOL
IsMouseOrKeyboardMsg(UINT uiMsg)
{
    if (uiMsg >= WM_MOUSEFIRST && uiMsg <= WM_MOUSELAST)
	return (BOOL)WM_MOUSEFIRST;

    if (uiMsg >= WM_KEYFIRST && uiMsg <= WM_KEYLAST)
	return (BOOL)WM_KEYFIRST;

    return FALSE;
}

void
TWIN_FlushWindowMessages(HWND hWnd)
{
    LPQUEUEMSG lpQueueMsg, lpQueuePtr, lpQueueNext;
    HANDLE  hTask;
    LPQUEUE lpQueue;

    hTask = GetWindowTask(hWnd);
    lpQueue = QueueGetPtr(hTask);
    if (!lpQueue)
	return;

    for(lpQueueMsg = lpQueue->lpQueueNext; 
	lpQueueMsg; 
	lpQueueMsg = lpQueueNext) 
    {
	lpQueueNext = (LPQUEUEMSG)(lpQueueMsg->lpNextMsg);

	if (lpQueueMsg->Message.hwnd == hWnd)
	{
	    /* is it the first one in the queue */
	    if (lpQueueMsg == lpQueue->lpQueueNext)
		lpQueue->lpQueueNext = lpQueueMsg->lpNextMsg;
	    else 
	    {
		/* find the message before this one */
		for (lpQueuePtr = lpQueue->lpQueueNext;
		     lpQueuePtr->lpNextMsg != lpQueueMsg;
		     lpQueuePtr = lpQueuePtr->lpNextMsg);
		
		/* set the next message of our previous to our next msg */
		lpQueuePtr->lpNextMsg = lpQueueMsg->lpNextMsg;
	    }

	    /* add this message to the free list */
	    lpQueueMsg->lpNextMsg = lpQueue->lpQueueFree;
	    lpQueue->lpQueueFree = lpQueueMsg;
	}
    }
}

