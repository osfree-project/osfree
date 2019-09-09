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
**                                  Memory Functions
**
**
***************************************************************************************/
/*************************************
*
*  System Includes
*
**************************************/
#include <string.h>


/*************************************
*
*  Private Includes
*
**************************************/
#include "winmem.h"
#include "msgbox.h"
#include "twhlprc.h"
#include "globals.h"


/***********************************
**
**  Private functions
**
***********************************/


/*************************************
*
*  Allocate global memory and trap error.
*
**************************************/
BOOL __far __pascal GlobalAllocMem( HWND hWnd, HGLOBAL __far * hGlobal, DWORD dwSize  )
{
  /* Allocate memory for menu list. */
  *hGlobal = GlobalAlloc ( GHND | GMEM_SHARE, dwSize ) ;
                              
  /* Error retrieving memory. */
  if ( *hGlobal == NULL ) 
  {
    /* If we get down to here then the command is not recognized. */ 
    MsgBox( GetLibInst(), hWnd, IDS_ERRORTITLE, IDS_MEMORYALLOC, MB_ICONHAND | MB_OK );
    return FALSE;
  }

  /* Return it. */
  return TRUE;
}


/*************************************
*
*  Reallocate global memory and trap error.
*
**************************************/
BOOL __far __pascal GlobalReAllocMem( HWND hWnd, HGLOBAL __far * hGlobal, DWORD dwNewSize  )
{
  /* Allocate memory for menu list. */
  *hGlobal = GlobalReAlloc( *hGlobal, dwNewSize, GHND | GMEM_SHARE ) ;
                              
  /* Error retrieving memory. */
  if ( *hGlobal == NULL ) 
  {
    /* Allocation error. */ 
    MsgBox( GetLibInst(), hWnd, IDS_ERRORTITLE, IDS_MEMORYALLOC, MB_ICONHAND | MB_OK );
    return FALSE;
  }

  /* Return it. */
  return TRUE;
}


/*****************************************
**
**  Allocates a global memory buffer and 
**  copies string's data into the buffer.
**
*****************************************/
HGLOBAL __far __pascal CopyString( HWND hErrorWnd, char __far * fpString )
{
  short int nBufferSize;
  
  HGLOBAL   hNewString;
  char __far * fpNewString;
  
                   
  /* Get size of filepath. Add 1 for NULL. */
  nBufferSize = _fstrlen( fpString ) + sizeof( char );

  /* Allocate memory for filepath. */
  if( !GlobalAllocMem( hErrorWnd, &hNewString, nBufferSize ) )
  {
    /* Failure. */
    return( NULL );
  }
  
  /* Lock the new string. */
  fpNewString = ( char __far * ) GlobalLock( hNewString ); 

  /* Save path data in the record. */  
  _fstrcpy( fpNewString, fpString );

  /* Unlock the new string. */
  GlobalUnlock( hNewString ); 

  /* Return new string. */
  return( hNewString );
}


/****************************** Local Heap Functions ******************************/

#define MEM_LIST_ADD   10       /* Default number of pages and increment size. */
#define SYS_RESERVED   16       /* Space reserved in a page by LocalInit(). */
#define PAGE_SIZE      25600    /* Initial page memory size for each page is 25K. 
                                   LocalAlloc() will expand the page's size to 64K
                                   if needed. */

/***************************************************
*
* Creates a MEMORYLIST structure that contains a 
* list of memory pages. 
*
****************************************************/
BOOL AllocMemoryList( HGLOBAL __far * hMemoryList )
{
  FPMEMORYLIST fpMemoryList;
  
  /*
  ** Allocate the MEMORYLIST structure. 
  */
  *hMemoryList =  GlobalAlloc( GHND, sizeof(MEMORYLIST) );
  if( *hMemoryList == NULL )
  {
    /* Failure. */
    return( FALSE );
  }

  /* Lock MEMORYLIST structure. */
  fpMemoryList = ( FPMEMORYLIST ) GlobalLock( *hMemoryList );
  
  /* Init MEMORYLIST structure. */
  
  /*
  ** Allocate the memory page list. 
  */
  fpMemoryList->wUsedPages = 0;
  fpMemoryList->wNumPages = MEM_LIST_ADD;
  fpMemoryList->hPageList = GlobalAlloc( GHND, fpMemoryList->wNumPages * sizeof(MEMORYPAGE) );
  if( fpMemoryList->hPageList == NULL )
  {
    /* Free MEMORYLIST structure. */
    GlobalUnlock( *hMemoryList  );
    GlobalFree( *hMemoryList );
    
    /* Failure. */
    return( FALSE );
  }

  /* Lock memory page list. */
  fpMemoryList->TopPagePtr = (FPMEMORYPAGE) GlobalLock( fpMemoryList->hPageList );
  

  /* Unlock MEMORYLIST structure. */
  GlobalUnlock( *hMemoryList  );

  /* Success. */
  return( TRUE );
}


/***************************************************
*
* Adds more pages to the memory page list in a 
* MEMORYLIST structure. 
*
****************************************************/
BOOL ReAllocMemList( FPMEMORYLIST fpMemoryList )
{
  /* Unlock the memory page list. */
  GlobalUnlock( fpMemoryList->hPageList );

  /*
  ** ReAllocate the memory page list. 
  */
  fpMemoryList->wNumPages = fpMemoryList->wNumPages + MEM_LIST_ADD;

  fpMemoryList->hPageList = GlobalReAlloc( fpMemoryList->hPageList, 
                            fpMemoryList->wNumPages * sizeof(MEMORYPAGE),
                            GMEM_MOVEABLE | GMEM_ZEROINIT );
  if( fpMemoryList->hPageList == NULL )
  {
    /* Failure. */
    return( FALSE );
  }

  /* Lock page list. */
  fpMemoryList->TopPagePtr = (FPMEMORYPAGE) GlobalLock( fpMemoryList->hPageList );
  
  /* Success. */
  return( TRUE );
}


/***************************************************
*
* Free a MEMORYLIST structure that contains a 
* list of memory pages. 
*
****************************************************/
void FreeMemoryList( HGLOBAL __far * hMemoryList )
{
  FPMEMORYLIST fpMemoryList;
  FPMEMORYPAGE fpPage;
  WORD wCounter;
  
  
  /* Lock MEMORYLIST structure. */
  fpMemoryList = ( FPMEMORYLIST ) GlobalLock( *hMemoryList );
  
  /*
  ** Free each page being used in the memory page list. 
  */
  fpPage = fpMemoryList->TopPagePtr;
  for( wCounter = 0; wCounter < fpMemoryList->wUsedPages; wCounter++ )
  {
    /* Unlock page. */
    GlobalUnlock( fpPage->hGlobal );
    
    /* Free page. */
    GlobalFree( fpPage->hGlobal );
  }

  /* Unlock MEMORYLIST structure. */
  GlobalUnlock( *hMemoryList  );
  
  /* Free MEMORYLIST structure. */
  GlobalFree( *hMemoryList  );
  *hMemoryList = NULL;
}


/***************************************************
*
* Allocates another page of memory.  If all pages
* in the list are used, more pages are added to the
* list. 
*
****************************************************/
FPMEMORYPAGE AllocNewMemoryPage( FPMEMORYLIST fpMemoryList )
{
  FPMEMORYPAGE fpPage;

  /* If there is no unused page? */
  if( fpMemoryList->wUsedPages == fpMemoryList->wNumPages )
  {
    /* Allocate more usused pages. */
    if( ! ReAllocMemList( fpMemoryList ) )
    {
      return FALSE;
    }
  }                 

  /* Goto next unused page. */
  fpPage = fpMemoryList->TopPagePtr + fpMemoryList->wUsedPages;
  
  /* Allocate page's space. */
  fpPage->hGlobal = GlobalAlloc( GPTR, PAGE_SIZE );
  if( fpPage->hGlobal == NULL )
  {
    /* Failure. */
    return( NULL );
  }

  /* Lock page. */
  fpPage->GlobalPtr = (void __far *) GlobalLock( fpPage->hGlobal );

  /* Get page's segment. */
  fpPage->Segment = (UINT) SELECTOROF( fpPage->GlobalPtr );

  /* Initialize local heap. */
  LocalInit( fpPage->Segment, SYS_RESERVED, PAGE_SIZE - 1 ); 
  
  /* One more used page in the memory pages list. */
  fpMemoryList->wUsedPages = fpMemoryList->wUsedPages + 1;
  
  /* Return page. */
  return( fpPage );
}


/***************************************************
*
* Allocates memory inside of a page of memory. 
*
****************************************************/
LPVOID MyAlloc( FPMEMORYLIST fpMemoryList, WORD wSize )
{
  FPMEMORYPAGE fpPage;
  WORD wCounter;
  UINT Segment;
  HANDLE hMem;
  
  
  /*
  ** For each page being used in the memory page list,
  ** find one with the space available. 
  */
  hMem = NULL;
  fpPage = fpMemoryList->TopPagePtr;
  for( wCounter = 0; wCounter < fpMemoryList->wUsedPages; wCounter++ )
  {
    /* Can have pointer indirection in __asm code so... */
    Segment = fpPage->Segment;

    /* Use global alloc'd page's segment. */
    __asm
    {
      push    ds                ; Save the DS.
      mov     ds, Segment       ; Get segment to allocate within.
    }
  
    /* Allocate the memory in the page. */
    hMem = LocalAlloc( LMEM_FIXED, wSize ); 
  
    /* Restore DS. */
    __asm
    {
      pop     ds                  ; Restore the DS.
    }
  
    /* Got memory? */
    if( hMem != NULL ) 
    {
      return( (LPVOID) MAKELONG( hMem, fpPage->Segment ) );
    }
  }

  /* 
  ** Didn't find a page with free space. 
  ** Use a new memory page.
  */

  /* Allocate a new page. */
  fpPage = AllocNewMemoryPage( fpMemoryList );

  /* Error? */
  if( fpPage == NULL )
  {
    return( NULL );
  }

  /* Can have pointer indirection in __asm code so... */
  Segment = fpPage->Segment;

  /* Use global alloc'd page's segment. */
  __asm
  {
    push  ds                ; Save the DS.
    mov   ds, Segment       ; Get segment to allocate within.
  }
  
  /* Allocate the memory in the page. */
  hMem = LocalAlloc( LMEM_FIXED, wSize ); 
  
  /* Restore DS. */
  __asm
  {
    pop   ds                  ; Restore the DS.
  }

  /* Error? */
  if( hMem == NULL )
  {
    return NULL;
  }

  /* Success. */
  return( (LPVOID) MAKELONG( hMem, fpPage->Segment ) );
} 
