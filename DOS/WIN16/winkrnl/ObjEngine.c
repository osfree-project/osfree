/*    
	ObjEngine.c	2.27
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
/*
*
*
*	An object has a OBJHEAD, which contains:
*		hObj		the HANDLE for this object
*		wObjSignature	a double word of type
*				eg. MAKESIGNATURE(OT_GDI,OT_BRUSH)
*
*	The following fields are also present, and are defined by the
*	object owner.
*		wFlags		object flags, tentatively
*				selected,
*				unrealized,
*				deleted
*		wRefCount	usage count how many people are using it
*				(ie. is it currently locked, by how many)
*		lpObjNext	linked list to additional objects
*
********************************************************************/

#include <stdio.h>
#include <string.h>

#include "windows.h"

#include "ObjEngine.h"
#include "GdiObjects.h"
#include "Kernel.h"
#include "WinDefs.h"
#include "GdiDC.h"
#include "GdiText.h"
#include "Listbox.h"
#include "Scroll.h"
#include "Module.h"
#include "Log.h"
#include "compat.h"
#include "WinMalloc.h"

#ifdef DEBUG
extern void debuggerbreak();
#endif

static OBJTAG **lplpArray = 0;
static LPOBJTAG FreeHead = 0;
static LPOBJTAG FreeTail = 0;
static int FreePoolSize = 0;
    
LPOBJHEAD GetHandleErr(int,LPOBJHEAD,HANDLE,WORD);

/*
 *	GetHandleErr
 *	used primarily for debug purposes
 *	set breakpoint at GetHandleErr, and check errcode
 *	fatal errors will not return
 */

LPOBJHEAD
GetHandleErr(int errcode,LPOBJHEAD lpObjHead,HANDLE hObj,WORD wSignature)
{
	int	fatal;
	int  	errflag;
	int	warnflag;
	int	ctlflag;
	extern int GetCompatibilityFlags(int);

	static char	*fmterr = "***ERROR*** Object Manager:%s: %s %x\n";
	static char	*fmtwarn = "***WARN*** Object Manager:%s: %s %x\n";

	/* not necessarily fatal */
	fatal = 0;

	/* what type of control do we want */
	ctlflag = GetCompatibilityFlags(0);

	errflag = LF_ERROR;
	warnflag = LF_WARNING;

	switch(errcode) {
	   case 1:
		ERRSTR((errflag, fmterr,"Internal Error","Table overflow",0));
		fatal = -1;
		break;
	   case 2:
		ERRSTR((errflag, fmterr,"Get Handle","NULL category or type",2));
		break;
	   case 3:
	        ERRSTR((errflag, fmterr,"Get Handle","Unknown object type",wSignature));
		break;

	   case 4:	/* bogus handle 	*/
	   case 5:	/* bad table index    	*/
	   case 6:	/* bad table offset	*/
	   case 7:	/* NULL pointer		*/
		if (hObj) {
		    if (wSignature == MLWIN &&
			(hObj == HWND_BOTTOM || hObj == HWND_TOPMOST ||
			 hObj == HWND_NOTOPMOST || hObj == HWND_TOP))
		       break;
		    ERRSTR((errflag, fmterr,"Get Object","Bad Handle",hObj));
		}
		else {
		    ERRSTR((warnflag,fmtwarn,"Get Object","NULL Handle",0));
		}
		break;
	   case 8:	/* bad category match   */
	   case 9:	/* bad type match   	*/
	        LOGSTR((LF_LOG, fmterr,"Get Object","Suspicious Handle",hObj));
		break;

	   case 10:	/* bad category match   */
	   case 11:	/* bad type match   	*/
	        ERRSTR((errflag, fmterr,"Lock/Unlock Handle","Bad Handle",hObj));
		break;

	   case 12:	/* bad table index   	*/
	   case 13:	/* bad table offset     */
	   case 14:	/* bad category match	*/
	   case 15:	/* bad type match	*/
	        ERRSTR((errflag, fmterr,"Free Object","Bad Handle",hObj));
		break;

	   case 16:
	   default:
	        ERRSTR((errflag,fmterr,"Internal Error","Bad Function",hObj));
		fatal = -1;
		break;
	}
#ifdef DEBUG
	if (ctlflag & WD_HANDLERR) {
	        ERRSTR((errflag,fmterr,"Internal Error","Bad Function",hObj));
		debuggerbreak();
	}
#endif

	if (fatal) {

		/* fatal exit, does not return */
		FatalAppExit(fatal,"Object Manager Fatal Error\n");	
	}

	return lpObjHead;
}
static BOOL AllocateMoreObjects()
{
    static int NextToAllocate = -1;
    LPOBJTAG NewListTail;
    int Index1;

    if (lplpArray == 0) 
    {
	lplpArray = (LPOBJTAG *)WINMALLOC(sizeof(LPOBJTAG) * 256,5,0);
	memset((LPSTR)lplpArray, '\0', sizeof(LPOBJTAG) * 256);
    }

    if (++NextToAllocate == 256) 
	return FALSE;

    lplpArray[NextToAllocate] = (LPOBJTAG)WINMALLOC(sizeof(OBJTAG) * 256,5,0);
    memset((LPSTR)lplpArray[NextToAllocate], '\0', sizeof(OBJTAG)*256);
    for (Index1 = 0; Index1 < 256; Index1++) 
    {
	lplpArray[NextToAllocate][Index1].lpObjInfo = 
	    (LPSTR)&lplpArray[NextToAllocate][Index1+1];
	lplpArray[NextToAllocate][Index1].hObj =
	    (HANDLE)((((NextToAllocate & 0xff) << 8) + (Index1 & 0xff)) << 1);
    }

    NewListTail = &lplpArray[NextToAllocate][255];
    
    NewListTail->hObj = (HANDLE)((((NextToAllocate & 0xff) << 8) + 255) << 1);

    if (!FreeHead)
    {
	NewListTail->lpObjInfo = 0;
	FreeHead = lplpArray[NextToAllocate];
	FreeTail = NewListTail;
	FreePoolSize = 256;
    }
    else
    {
	NewListTail->lpObjInfo = (LPSTR) FreeHead;
	FreeHead = lplpArray[NextToAllocate];
	FreePoolSize += 256;
    }

    return TRUE;
}

LPOBJHEAD
HandleObj(int Func,WORD wSignature,LPVOID lpv)
{
    LPOBJTAG lpTemp;
    LPOBJHEAD lpObjHead;
    LPSTR lpObjTemp,lpString;
    int Index, Index1;
    int Size;
    WORD Type, Category;
    HANDLE hObj;

    Type = LOBYTE(wSignature);
    Category = HIBYTE(wSignature);

    switch(Func) {
	case HM_GETHND:		/* Get new object handle and pointer */
	    if (FreePoolSize < 100) {
		if (!AllocateMoreObjects() && !FreeHead)
		    return (LPOBJHEAD)GetHandleErr(1,NULL,0,wSignature);
	    }
	    if (!Category || !Type)
		return (LPOBJHEAD)GetHandleErr(2,NULL,0,wSignature);
	    switch (wSignature) {
		case MLDC:
		    Size = sizeof(DCINFO);
		    break;
		case MLICON:
		    Size = sizeof(TWIN_ICONINFO);
		    break;
		case MLBITMAP:
		    Size = sizeof(IMAGEINFO);
		    break;
		case MLHANDLE:
		case MLREGION:
		    Size = sizeof(HANDLEINFO);
		    break;
		case MLMODULE:
		    Size = sizeof(MODULEINFO);
		    break;
		case MLTASK:
		    Size = sizeof(TASKINFO);
		    break;
		case MLBRUSH:
		    Size = sizeof(BRUSHINFO);
		    break;
		case MLPEN:
		    Size = sizeof(PENINFO);
		    break;
		case MLFONT:
		    Size = sizeof(FONTINFO);
		    break;
		case MLPALETTE:
		    Size = sizeof(PALETTEINFO);
		    break;
		case MLWIN:
		    Size = sizeof(WININFO);
		    break;
		case MLPOS:
		    Size = sizeof(MULTIWINPOS);
		    break;
		case MLLBOX:
		    Size = sizeof(LISTBOXINFO);
		    break;
		case MLSBAR:
		    Size = sizeof(TWINSCROLLINFO);
		    break;
		case MLMETAFILE:
		    Size = sizeof(METAFILEINFO);
		    break;
		case MLCURSOR:
		    Size = sizeof(CURSORINFO);
		    break;
		default:
		    return (LPOBJHEAD)GetHandleErr(3,NULL,0,wSignature);
	    }

	    lpObjTemp = WINMALLOC(Size,0,0);
            memset(lpObjTemp, 0, Size);
	    lpTemp = FreeHead;
	    FreeHead = (LPOBJTAG)(FreeHead->lpObjInfo);
	    FreePoolSize--;
	    if (!FreeHead)
	    {
		FreeTail = NULL;
		FreePoolSize = 0;
	    }
	    
	    lpTemp->lpObjInfo = lpObjTemp;
	    lpTemp->wFlags |= OE_BUSY;
	    lpTemp->hObj = (HANDLE)((DWORD)lpTemp->hObj | OM_MASK);
	    ((LPOBJHEAD)lpObjTemp)->hObj = lpTemp->hObj;
	    ((LPOBJHEAD)lpObjTemp)->wObjSignature = MAKESIGNATURE(Category,Type);
	    *((LPHANDLE)lpv) = lpTemp->hObj;

	    WINMALLOCINFO(WMI_TAG, lpObjTemp, 3, MAKELONG(lpTemp->hObj,wSignature));

#ifdef TWIN_THREAD_SAFE
	    ((LPOBJHEAD)lpObjTemp)->wRefCount++;
#endif /* TWIN_THREAD_SAFE */

	    return((LPOBJHEAD)lpObjTemp);

	case HM_CHECKHND:	/* Check handle validity */
	    if (!((DWORD)(hObj = (HANDLE)(DWORD)lpv) & OM_MASK))
		return (LPOBJHEAD)FALSE;

	    Index = HIBYTE((WORD)((DWORD)hObj & ~OM_MASK) >> 1);
	    Index1 = LOBYTE((WORD)((DWORD)hObj & ~OM_MASK) >> 1);
	    if (!lplpArray[Index])
		return (LPOBJHEAD)FALSE;
	    if (!(lplpArray[Index][Index1].wFlags & OE_BUSY))
		return (LPOBJHEAD)FALSE;

	    if (!(lpObjTemp = lplpArray[Index][Index1].lpObjInfo))
			return (LPOBJHEAD)FALSE;

	    if (Category &&
		(HIBYTE(((LPOBJHEAD)lpObjTemp)->wObjSignature) != Category))
		return (LPOBJHEAD)FALSE;

	    if (Type &&
		(LOBYTE(((LPOBJHEAD)lpObjTemp)->wObjSignature) != Type))
		return (LPOBJHEAD)FALSE;

#ifdef TWIN_THREAD_SAFE
	    ((LPOBJHEAD)lpObjTemp)->wRefCount++;
#endif /* TWIN_THREAD_SAFE */

	    return (LPOBJHEAD)lpObjTemp;

	case HM_GETOBJ:		/* Return object ptr with a given handle */
	    /* if OM_MASK is not set -- bogus handle!!! */
	    if (!((DWORD)(hObj = (HANDLE)(DWORD)lpv) & OM_MASK))
		return (LPOBJHEAD)GetHandleErr(4,NULL,hObj,wSignature);

	    Index = HIBYTE((WORD)((DWORD)hObj & ~OM_MASK) >> 1);
	    Index1 = LOBYTE((WORD)((DWORD)hObj & ~OM_MASK) >> 1);

	    if (!lplpArray[Index])
		return (LPOBJHEAD)GetHandleErr(5,NULL,hObj,wSignature);
	    if (!(lplpArray[Index][Index1].wFlags & OE_BUSY))
		return (LPOBJHEAD)GetHandleErr(6,NULL,hObj,wSignature);

	    if (!(lpObjTemp = lplpArray[Index][Index1].lpObjInfo))
			return (LPOBJHEAD)GetHandleErr(7,NULL,hObj,wSignature);

	    if (Category &&
		(HIBYTE(((LPOBJHEAD)lpObjTemp)->wObjSignature) != Category))
		return (LPOBJHEAD)GetHandleErr(8,NULL,hObj,wSignature);

	    if (Type &&
		(LOBYTE(((LPOBJHEAD)lpObjTemp)->wObjSignature) != Type))
		return (LPOBJHEAD)GetHandleErr(9,NULL,hObj,wSignature);

#ifdef TWIN_THREAD_SAFE
	    ((LPOBJHEAD)lpObjTemp)->wRefCount++;
#endif /* TWIN_THREAD_SAFE */

	    return (LPOBJHEAD)lpObjTemp;

	case HM_LOCKHND:
	case HM_UNLOCKHND:
	    hObj = (HANDLE)(DWORD)lpv;
	    Index = HIBYTE((WORD)((DWORD)hObj & ~OM_MASK) >> 1);
	    if (!lplpArray[Index])
		return (LPOBJHEAD)GetHandleErr(10,NULL,hObj,wSignature);
	    Index1 = LOBYTE((WORD)((DWORD)hObj & ~OM_MASK) >> 1);
	    if (!(lplpArray[Index][Index1].wFlags & OE_BUSY))
		return (LPOBJHEAD)GetHandleErr(11,NULL,hObj,wSignature);

	    lpObjTemp = lplpArray[Index][Index1].lpObjInfo;
	    if (lpObjTemp) {
		int nCount = (int)(short)((LPOBJHEAD)lpObjTemp)->wRefCount;

		if(Func == HM_LOCKHND && nCount >= 0)
	    		((LPOBJHEAD)lpObjTemp)->wRefCount++;
		else
	    		if(Func == HM_UNLOCKHND && nCount > 0)
	    			((LPOBJHEAD)lpObjTemp)->wRefCount--;
	    }
	    return (LPOBJHEAD)lpObjTemp;

	case HM_FREEHND:	/* add object ptr to the free list */
	    hObj = (HANDLE)(DWORD)lpv;
	    Index = HIBYTE((WORD)((DWORD)hObj & ~OM_MASK) >> 1);
	    if (!lplpArray[Index])
		return (LPOBJHEAD)GetHandleErr(12,NULL,hObj,wSignature);
	    Index1 = LOBYTE((WORD)((DWORD)hObj & ~OM_MASK) >> 1);

	    lpObjTemp = lplpArray[Index][Index1].lpObjInfo;
	    if (lpObjTemp) {
		/* do not free if category or type does not match */
		if (Category &&
		    (HIBYTE(((LPOBJHEAD)lpObjTemp)->wObjSignature) != Category)) {
		    return (LPOBJHEAD)GetHandleErr(13,NULL,hObj,wSignature);
		}

		if (Type &&
		    (LOBYTE(((LPOBJHEAD)lpObjTemp)->wObjSignature) != Type)) {
		    return (LPOBJHEAD)GetHandleErr(14,NULL,hObj,wSignature);
		}

		/* do not free if locked */
	    	if(((LPOBJHEAD)lpObjTemp)->wRefCount)
		        return (LPOBJHEAD) (unsigned int)hObj;
		

	/* we do not need to clean up here, so I'll be removing this case
	   after checking every object type */
		switch (wSignature) {
		    case MLWIN:
			if (((LPWININFO)lpObjTemp)->lpWindowName)
			    WinFree(((LPWININFO)lpObjTemp)->lpWindowName);
			if (((LPWININFO)lpObjTemp)->lpPropertyList)
			    WinFree((LPVOID)
				(((LPWININFO)lpObjTemp)->lpPropertyList));
			if (((LPWININFO)lpObjTemp)->lpWndExtra)
			    WinFree(((LPWININFO)lpObjTemp)->lpWndExtra);
			break;
		    case MLPOS:
			if (((LPMULTIWINPOS)lpObjTemp)->lpWinPosInfo)
			    WinFree((LPSTR)
				(((LPMULTIWINPOS)lpObjTemp)->lpWinPosInfo));
			break;
		}
		WinFree(lplpArray[Index][Index1].lpObjInfo);
	    }
	    lplpArray[Index][Index1].lpObjInfo = (LPSTR) 0;
	    lplpArray[Index][Index1].wFlags &= ~OE_BUSY;
	    if (FreeTail)
	    {
		FreeTail->lpObjInfo = (LPSTR) &lplpArray[Index][Index1];
		FreeTail = (LPOBJTAG) FreeTail->lpObjInfo;
		FreePoolSize++;
	    }
	    else
	    {
		FreeTail = (LPOBJTAG) ((LPSTR) &lplpArray[Index][Index1]);
		FreeHead = FreeTail;
		FreePoolSize = 1;
	    }
	    return (LPOBJHEAD)(-1);

	case HM_DUMPTABLE:

	  if(lplpArray == 0)
	  	return (LPOBJHEAD)1;

          Size = 0;
	  for (Index = 0; Index < 256 && lplpArray[Index]; Index++)
	    for (Index1 = 0; Index1 < 256; Index1++) {
	      if (!(lplpArray[Index][Index1].wFlags & OE_BUSY))
		continue;
	      lpObjHead = (LPOBJHEAD)lplpArray[Index][Index1].lpObjInfo;
	      if ((Category &&
		   (HIBYTE(lpObjHead->wObjSignature) != Category)) ||
	          (Type &&
		  (LOBYTE(lpObjHead->wObjSignature) != Type)))
		continue;
	      switch (lpObjHead->wObjSignature) {
		case MLDC:
		    lpString = "DC";
		    break;
		case MLICON:
		    lpString = "ICON";
		    break;
		case MLBITMAP:
		    lpString = "BITMAP";
		    break;
		case MLHANDLE:
		    lpString = "HANDLE";
		    break;
		case MLREGION:
		    lpString = "REGION";
		    break;
		case MLMODULE:
		    lpString = "MODULE";
		    break;
		case MLTASK:
		    lpString = "TASK";
		    break;
		case MLBRUSH:
		    lpString = "BRUSH";
		    break;
		case MLPEN:
		    lpString = "PEN";
		    break;
		case MLFONT:
		    lpString = "FONT";
		    break;
		case MLPALETTE:
		    lpString = "PALETTE";
		    break;
		case MLWIN:
		    lpString = "WINDOW";
		    break;
		case MLPOS:
		    lpString = "WINPOS";
		    break;
		case MLLBOX:
		    lpString = "LBINFO";
		    break;
		case MLSBAR:
		    lpString = "SBINFO";
		    break;
		case MLMETAFILE:
		    lpString = "METAFILE";
		    break;
		case MLCURSOR:
		    lpString = "CURSOR";
		    break;
		default:
		    lpString = "UNKNOWN OBJECT TYPE";
	      }
	      LOGSTR((LF_LOG,"%3d OBJECT %s handle %x lpObjInfo %x\n",
		Size,lpString,lpObjHead->hObj,lpObjHead));
	    }
	  LOGSTR((LF_LOG,"END OF DUMP\n"));
	  return (LPOBJHEAD)1;
	default:
	    break;
    }
    return (LPOBJHEAD)GetHandleErr(16,NULL,(HANDLE)Func,wSignature);
}


DWORD
TWIN_CheckObject (int theptr)
{
	LOGSTR((LF_LOG,"TWIN_CheckObject: %x\n",theptr));
	return 0;
}
