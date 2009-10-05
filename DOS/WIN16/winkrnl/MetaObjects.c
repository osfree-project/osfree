/*    
	MetaObjects.c	2.17
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

#include "GdiObjects.h"
#include "Kernel.h"
#include "GdiText.h"
#include "Log.h"
#include "Meta.h"

/* When object is selected into given DC, its handle is looked up
   in objTable. If handle is not found, <*> the object is created,
   appropriate record about it is written in metafile, and its
   handle along with logical structure are stored in objTable.
   If handled exists in objTable, we compare logical properties
   of the given handle and the handle found in objTable. If
   comparison is positive, both handles refer to the same object
   (and we just pick up the handle without creating object).
   In case of negative comparison the object whose handle stored
   in objTable was deleted (we reflect this fact in metafile and
   delete handle from objTable), and a new object was created whith 
   the same handle (we perform step <*>).
*/ 

int
TWIN_GetObjInfo(HGDIOBJ hObj,
		METACREATEOBJPROC *lpfnCreateObj,
		LPVOID *lpLogObj)
{
    LPOBJHEAD	lpObjInfo;
    LPBRUSHINFO lpBrushInfo;
    int		iSize = 0;

    lpObjInfo = GETGDIINFO(hObj);
    switch ( GET_OBJECT_TYPE(lpObjInfo) ) {
	case OT_BRUSH:
	    lpBrushInfo = (LPBRUSHINFO)lpObjInfo;
	    iSize = sizeof(LOGBRUSH);
	    if ( lpfnCreateObj )
		*lpfnCreateObj = &TWIN_mf_CreateBrushIndirect;
	    if ( lpLogObj )
		*lpLogObj = (LPVOID)(&lpBrushInfo->lpBrush);
	    break;
	case OT_PEN:
	    iSize = sizeof(LOGPEN);
	    if ( lpfnCreateObj )
		*lpfnCreateObj = &TWIN_mf_CreatePenIndirect;
	    if ( lpLogObj )
		*lpLogObj = (LPVOID)(&((LPPENINFO)lpObjInfo)->lpPen);
	    break;
	case OT_FONT:
	    iSize = sizeof(LOGFONT);
	    if ( lpfnCreateObj )
		*lpfnCreateObj = &TWIN_mf_CreateFontIndirect;
	    if ( lpLogObj )
		*lpLogObj = (LPVOID)(&((LPFONTINFO)lpObjInfo)->LogFont);
	    break;
	case OT_PALETTE:
	    iSize = sizeof(PALETTEINFO);
	    if ( lpfnCreateObj )
		*lpfnCreateObj = &TWIN_mf_CreatePalette;
	    if ( lpLogObj )
		*lpLogObj = (LPVOID)lpObjInfo;
	    break;
	default:
	    break;
    }
    RELEASEGDIINFO(lpObjInfo);
    return iSize;
}

/* -------------------------------------------------------------------- */
/* Object table management						*/
/* -------------------------------------------------------------------- */
int
TWIN_ControlObjTable(LPINTERNALMETAFILE lpIntMeta, int nCmd, HGDIOBJ hObj)
{
    LPVOID		lpObj;
    int		iSize, rc = 0;

    switch ( nCmd ) {
	case INIT_OBJ_TBL:
	    for ( rc = 0; rc < HANDLE_TABLE_SIZE; rc++)
		lpIntMeta->lpObjTable[rc].hObj = 0;
	    break;

	case DELETE_OBJ_TBL:
		break;

	case GET_FREE_SLOT:
	    for ( rc = 0; rc < HANDLE_TABLE_SIZE; rc++ )
		if ( !lpIntMeta->lpObjTable[rc].hObj ) {
		    if ((rc+1) > lpIntMeta->header.mtNoObjects)
			lpIntMeta->header.mtNoObjects = rc+1;
		    return rc;
		}
	    rc = NOT_FOUND;
	    break;

	case FIND_OBJ:
	    for ( rc = 0; rc < HANDLE_TABLE_SIZE; rc++ )
		if ( lpIntMeta->lpObjTable[rc].hObj == hObj )
			return rc; /* return current rc */
	    rc = NOT_FOUND;
	    break;

	case FREE_OBJ:
	    if ( (rc = TWIN_ControlObjTable(lpIntMeta,FIND_OBJ, hObj)) == -1 )
		rc = -1;
	    else
		lpIntMeta->lpObjTable[rc].hObj = 0;
	    break;

	case STORE_OBJ:
	    rc = TWIN_ControlObjTable(lpIntMeta,GET_FREE_SLOT, 0);
	    lpIntMeta->lpObjTable[rc].hObj = hObj;
	    iSize = TWIN_GetObjInfo(hObj, 0, &lpObj);
	    memcpy(&lpIntMeta->lpObjTable[rc].obj, lpObj, iSize);
	    break;

	case CHECK_HANDLE:
	    /* Returns slot number, if the same object      */
	    /* exists; NOT_FOUND, if handle not found; and  */
	    /* NEED_TO_DELETE, if handle exists, but refers */
	    /* to another object.                           */
	    if ((rc = TWIN_ControlObjTable(lpIntMeta, FIND_OBJ,hObj)) ==
			NOT_FOUND)
		break;
	    iSize = TWIN_GetObjInfo(hObj, 0, &lpObj);
	    if (memcmp(lpObj, &lpIntMeta->lpObjTable[rc].obj, iSize) == 0)
		break;
	    else
		rc = NEED_TO_DELETE;
	    break;

	default:
	    break;
    }
    return rc;
}

/* -------------------------------------------------------------------- */
/* Handle table management						*/
/* -------------------------------------------------------------------- */

void
TWIN_InitHandleTable(LPHANDLETABLE lpht, int nSize)
{
    memset((char*)lpht, 0, nSize);
}

int
TWIN_MetaFindFreeSlot(LPHANDLETABLE lpht, int table_size)
{
	int i;
	HANDLE *htable = lpht->objectHandle;

	for ( i = 0; i < table_size; i++ )
		if ( !htable[i] )
			return i;
	return -1;			/* No free slots */
}
