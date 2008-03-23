/*  
	WLists.h 	Common Controls Implementation source - helper file
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

The maintainer of the Willows TWIN Libraries may be reached (Email) 
at the address twin@willows.com	

*/

#ifdef _WINDOWS
#include <windows.h>
#else
#include "WCommCtrl.h"
#endif
#include "WLists.h"

#define WL_MINSIZE		1


/*----- Create a List --------------------------------------------------------*/
int WINAPI
WLCreate
(
	WLIST*                    lpList,
	UINT                      ElementSize
)

{

	int                       ErrorCode = 0;


	if ( ! lpList )
		ErrorCode = WLERR_NULLHANDLE;
	else
	if ( ! ElementSize )
		ErrorCode = WLERR_NULLELEMENTSIZE;
	else
	{
		memset ( lpList, 0, sizeof ( WLIST ) );
		/*--- Allocate one byte so that the realloc works ---*/
		if ( ! ( lpList->hData = GlobalAlloc ( GHND, WL_MINSIZE ) ) )
			ErrorCode = WLERR_GLOBALALLOC;
		else
		{
			lpList->ElementSize    = ElementSize;
			lpList->AllocationSize = 1;
		}
	}

	return ( ErrorCode );
	
}

/*----- Allocate space large enough to house certain amount of elements -----*/
int WINAPI
WLSetSize
(
	WLIST*                    lpList,
	int                       iElements
)
{

	HGLOBAL                   hNewData  = ( HGLOBAL )NULL;
	int                       ErrorCode = 0;

	if ( iElements <= lpList->AllocatedElements )
		return WLERR_NULLELEMENTSIZE;
	if ( ! ( hNewData = GlobalReAlloc ( lpList->hData,
					    iElements * lpList->ElementSize,
					    GHND ) ) )
		ErrorCode = WLERR_GLOBALREALLOC;
	else
	{
		lpList->hData = hNewData;
		lpList->AllocatedElements = iElements;
	}
	return ErrorCode;
}

int WINAPI
WLInsertEx
(
	WLIST*                    lpList,
	UINT                      IndexBefore,
	LPVOID                    lpElementData
)

{

	if ( IndexBefore > lpList->ElementCount )
		IndexBefore = lpList->ElementCount;

	return WLInsert ( lpList, IndexBefore, lpElementData ) == 0 ? 
	       IndexBefore : -1;
}

/*----- Insert an element into a list ---------------------------------------*/
/* To allow for nested call, the check for LockCount is disabled */
int WINAPI
WLInsert
(
	WLIST*                    lpList,
	UINT                      IndexBefore,
	LPVOID                    lpElementData
)

{

	HGLOBAL                   hNewData  = ( HGLOBAL )NULL;
	LPBYTE                    lpSource  = ( LPBYTE )NULL;
	LPBYTE                    lpTarget  = ( LPBYTE )NULL;

	int                       ErrorCode = 0;


	if ( ( ! lpList ) || ( ! lpElementData ) )
		ErrorCode = WLERR_NULLHANDLE;
	else
#if 0
	if ( lpList->LockCount )
		ErrorCode = WLERR_LOCKED;
	else
	if ( IndexBefore > lpList->ElementCount )
		ErrorCode = WLERR_OUTOFBOUNDS;
	else
#endif
	{
		if ( IndexBefore > lpList->ElementCount )
			IndexBefore = lpList->ElementCount;
		/*--- If all of the memory is being used, allocate more ---*/
		if ( lpList->ElementCount == lpList->AllocatedElements )
		{
			/*--- If there are more elements than we increment by
			      increase the allocation amount ---*/
			if ( lpList->ElementCount >= lpList->AllocationSize )
				lpList->AllocationSize *= 2;
			if ( ! ( hNewData = GlobalReAlloc ( lpList->hData, 
			                                    ( lpList->ElementCount + 
			                                      lpList->AllocationSize ) * 
			                                    lpList->ElementSize,
			                                    GHND ) ) )
				ErrorCode = WLERR_GLOBALREALLOC;
			else
			{
				lpList->hData             = hNewData;
				lpList->AllocatedElements += lpList->AllocationSize;
			}
		}
		if ( ! ErrorCode )
		{
			if ( ! ( lpList->lpData = GlobalLock ( lpList->hData ) ) )
				ErrorCode = WLERR_GLOBALLOCK;
			else
			{
				lpSource = ( LPBYTE )lpList->lpData + ( IndexBefore * lpList->ElementSize );
				/*--- Move the memory from the begining of the
				      indexed element to the begining of the 
				      next element ---*/
				if ( lpList->ElementCount >= IndexBefore + 1 )
				{
					lpTarget = lpSource + lpList->ElementSize;
					if ( IndexBefore )
					{
						memmove ( lpTarget, lpSource,
						          ( lpList->ElementCount - IndexBefore ) * lpList->ElementSize );
					}
					else
					{
						memmove ( lpTarget, lpSource,
						          lpList->ElementCount * lpList->ElementSize );
					}
				}
				/*--- Copy the new element to is place ---*/
				memcpy ( lpSource, ( LPBYTE )lpElementData, lpList->ElementSize );
				lpList->ElementCount++;
			}

			if ( lpList->lpData )
			{
				GlobalUnlock ( lpList->hData );
				lpList->lpData = ( LPVOID )NULL;
			}
		}
	}

	return ( ErrorCode );

}
/*----- Delete an element from a list ----------------------------------------*/
int WINAPI
WLDelete
(
	WLIST*                    lpList,
	UINT                      Index
)

{
	int                       ErrorCode = 0;


	if ( ! lpList ) 
		ErrorCode = WLERR_NULLHANDLE;
	else
	if ( lpList->LockCount )
		ErrorCode = WLERR_LOCKED;
	else
	if ( Index >= lpList->ElementCount )
		ErrorCode = WLERR_OUTOFBOUNDS;
	else
	{
		if ( ! ( lpList->lpData = GlobalLock ( lpList->hData ) ) )
			ErrorCode = WLERR_GLOBALLOCK;
		else
		{
			/*--- All of the elements following the one to be deleted to the
			      position of the element being deleted ---*/
			memmove ( ( LPBYTE )lpList->lpData + ( Index * lpList->ElementSize ),
				( LPBYTE )lpList->lpData + ( ( Index + 1 ) * lpList->ElementSize ),
			        ( lpList->ElementCount - Index - 1 ) * lpList->ElementSize );
			lpList->ElementCount--;
		}
		if ( lpList->lpData )
		{
			GlobalUnlock ( lpList->hData );
			lpList->lpData = ( LPVOID )NULL;
		}
	}

	return ( ErrorCode );

}
/*----- Lock the list and get a pointer to the data --------------------------*/
int WINAPI
WLLock
(
	WLIST*                    lpList,
	LPVOID*                   lplpData
)

{

	int                       ErrorCode = 0;


	if ( ( ! lpList ) || ( ! lplpData ) || ( ! lpList->hData ) ) 
		ErrorCode = WLERR_NULLHANDLE;
	else
	if ( (*lplpData = lpList->lpData ))
		lpList->LockCount++;
	else
	if ( ! ( *lplpData = lpList->lpData = GlobalLock ( lpList->hData ) ) )
		ErrorCode = WLERR_GLOBALLOCK;
	else
		lpList->LockCount++;

	return ( ErrorCode );

}
/*----- UnLock the list and set a pointer to the data to NULL ----------------*/
int WINAPI
WLUnlock
(
	WLIST*                    lpList,
	LPVOID*                   lplpData
)

{

	int                       ErrorCode = 0;


	if ( ( ! lpList ) || ( ! lpList->hData ) ) 
		ErrorCode = WLERR_NULLHANDLE;
	else
	if ( ! lpList->lpData )
		ErrorCode = WLERR_NOTLOCKED;
	else
	{
		if ( ! ( --( lpList->LockCount ) ) )
		{
			GlobalUnlock ( lpList->hData );
			lpList->lpData = ( LPVOID ) NULL;
			if ( lplpData )
				*lplpData = ( LPVOID )NULL;
		}
	}

	return ( ErrorCode );

}
/*----- Destory the list even if it's locked ---------------------------------*/
int WINAPI
WLDestroy
(
	WLIST*                   lpList
)

{

	int                       ErrorCode = 0;


	if ( ! lpList ) 
		ErrorCode = WLERR_NULLHANDLE;
	else
	if ( ! lpList->hData )
		ErrorCode = WLWARN_NULLHANDLE;
	else
	{
		if ( lpList->lpData )
		{
			ErrorCode = WLWARN_LOCKED;
			GlobalUnlock ( lpList->hData );
		}
		GlobalFree ( lpList->hData );
		memset ( lpList, 0, sizeof ( WLIST ) );
	}

	return ( ErrorCode );

}
/*----- Get the number of elements in the list -------------------------------*/
int WINAPI
WLCount
(	
	WLIST                     *lpList,
	UINT                      *lpCount
)

{

	int                       ErrorCode	= 0;


	if ( ! lpList )
		ErrorCode = WLERR_NULLHANDLE;
	else
		*lpCount = lpList->ElementCount;

	return ( ErrorCode );

}
