
/*  WLists.h	1.3 
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


#ifndef __WLIST_H__
#define __WLIST_H__



typedef struct _WLIST
{
	HGLOBAL                   hData;
	LPVOID                    lpData;
	UINT                      ElementSize;
	UINT                      ElementCount;
	UINT                      AllocatedElements;
	UINT                      AllocationSize;
	UINT                      LockCount;
} WLIST;

#define WBASE_LIST                2000
#define WLERR_NULLHANDLE          ( -WBASE_LIST - 1 )
#define WLERR_NULLELEMENTSIZE     ( -WBASE_LIST - 2 )
#define WLERR_GLOBALALLOC         ( -WBASE_LIST - 3 )
#define WLERR_LOCKED              ( -WBASE_LIST - 4 )
#define WLERR_OUTOFBOUNDS         ( -WBASE_LIST - 5 )
#define WLERR_GLOBALREALLOC       ( -WBASE_LIST - 6 )
#define WLERR_GLOBALLOCK          ( -WBASE_LIST - 7 )
#define WLERR_NOTLOCKED           ( -WBASE_LIST - 8 )

#define WLWARN_NULLHANDLE         ( WBASE_LIST + 1 )
#define WLWARN_LOCKED             ( WBASE_LIST + 2 )


/*----- Create a List --------------------------------------------------------*/
int WINAPI
WLCreate
(
	WLIST*                    lpList,
	UINT                      ElementSize
);
/*----- Insert and element into a list ---------------------------------------*/
int WINAPI
WLInsertEx
(
	WLIST*                    lpList,
	UINT                      IndexBefore,
	LPVOID                    lpElementData
);
/*----- Insert and element into a list ---------------------------------------*/
int WINAPI
WLInsert
(
	WLIST*                    lpList,
	UINT                      IndexBefore,
	LPVOID                    lpElementData
);
/*----- Delete an element from a list ----------------------------------------*/
int WINAPI
WLDelete
(
	WLIST*                    lpList,
	UINT                      Index
);
/*----- Lock the list and get a pointer to the data --------------------------*/
int WINAPI
WLLock
(
	WLIST*                    lpList,
	LPVOID*                   lplpData
);
/*----- UnLock the list and set a pointer to the data to NULL ----------------*/
int WINAPI
WLUnlock
(
	WLIST*                    lpList,
	LPVOID*                   lplpData
);
/*----- Destory the list even if it's locked ---------------------------------*/
int WINAPI
WLDestroy
(
	WLIST*                    lpList
);

/*----- Get the number of elements in the list -------------------------------*/
int WINAPI
WLCount
(	
	WLIST                     *lpList,
	UINT                      *lpCount
);

/*----- Allocate space large enough to house certain amount of elements -----*/
int WINAPI
WLSetSize
(
	WLIST*                    lpList,
	int                       iElements
);
#endif /* __WLIST_H__ */
