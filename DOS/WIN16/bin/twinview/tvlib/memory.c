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
#include <windows.h>


/*************************************
*
*  Custom Includes
*
**************************************/
#include "tvlibprv.h"



/*************************************
*
*  Allocate global memory of a certain size and trap error.
*
**************************************/
BOOL __far __pascal GlobalAllocMem( HWND hWnd, HGLOBAL __far * hGlobal, DWORD dwSize  )
{
  /* Allocate memory for menu list. */
  *hGlobal = GlobalAlloc ( GHND, dwSize ) ;
                              
  /* Error retrieving memory. */
  if ( *hGlobal == NULL ) 
  {
    MessageBox( hWnd, (LPSTR)"GlobalAlloc() failed!", (LPSTR)"Memory Functions", MB_ICONHAND | MB_OK );
    return FALSE;
  }

  /* Return it. */
  return TRUE;
}


/*************************************
*
*  Reallocate global memory of a certain size and trap error.
*
**************************************/
BOOL __far __pascal GlobalReAllocMem( HWND hWnd, HGLOBAL __far * hGlobal, DWORD dwNewSize  )
{
  /* Allocate memory for menu list. */
  *hGlobal = GlobalReAlloc( *hGlobal, dwNewSize, GHND ) ;
                              
  /* Error retrieving memory. */
  if ( *hGlobal == NULL ) 
  {
    MessageBox( hWnd, (LPSTR)"GlobalAlloc() failed!", (LPSTR)"Memory Functions", MB_ICONHAND | MB_OK );
    return FALSE;
  }

  /* Return it. */
  return TRUE;
}

