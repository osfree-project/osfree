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
/***************************************************************************************
**
**
**                         Memory Allocation Functions
**
**
***************************************************************************************/
#ifndef WINMEM_H
#define WINMEM_H


/*************************************
*
*  System Includes
*
**************************************/
#include <windows.h>


/***********************************
**
**  Shared Defines
**
***********************************/

/* Element list and list info. */
typedef struct tagMEMORYPAGE
{
  HGLOBAL hGlobal;             /* Handle to page's memory space. */
  void __far * GlobalPtr;      /* Locked page's memory space. */
  UINT    Segment;             /* SELECTOROF( GlobalPtr ). */
}
MEMORYPAGE;
typedef MEMORYPAGE __far * FPMEMORYPAGE;

/* Memory list and list info. */
typedef struct tagMEMORYLIST
{
  WORD wNumPages;              /* Number of page's in list (used and  not used). */
  WORD wUsedPages;             /* Number of page's in list that are being used. */
  HGLOBAL hPageList;           /* Handle to the list of pages. */
  FPMEMORYPAGE TopPagePtr;     /* Pointer to the first page in the list. */
}
MEMORYLIST;
typedef MEMORYLIST __far * FPMEMORYLIST;


/*************************************
*
*  Allocate global memory of a certain size and trap error.
*
**************************************/
BOOL __far __pascal GlobalAllocMem( HWND hWnd, HGLOBAL __far * hGlobal, DWORD dwSize  );


/*************************************
*
*  Reallocate global memory of a certain size and trap error.
*
**************************************/
BOOL __far __pascal GlobalReAllocMem( HWND hWnd, HGLOBAL __far * hGlobal, DWORD dwNewSize  );


/*****************************************
**
**  Allocates a buffer and copies string's
**  data into the buffer.
**
*****************************************/
HGLOBAL __far __pascal CopyString( HWND hErrorWnd, char __far * fpString );



/****************************** Local Heap Functions ******************************/

/***************************************************
*
* Creates a MEMORYLIST structure that contains a 
* list of memory pages. 
*
****************************************************/
BOOL AllocMemoryList( HGLOBAL __far * hMemoryList );


/***************************************************
*
* Free a MEMORYLIST structure that contains a 
* list of memory pages. 
*
****************************************************/
void FreeMemoryList( HGLOBAL __far * hMemoryList ); 


/***************************************************
*
* Allocates memory inside of a page of memory. 
*
****************************************************/
LPVOID MyAlloc( FPMEMORYLIST fpMemoryList, WORD wSize );


#endif
