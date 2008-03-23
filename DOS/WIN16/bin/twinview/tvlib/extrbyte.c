/* 
/***************************************************************************************
**
**
**                          Child Window Extrabyte Access Functions
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



HACCEL __far __pascal __export GetTVAccel( HWND hWnd )
{
  /* Error checking. */
  if( !IsWindow( hWnd ) ) return 0;

  /* 
  ** Get the child's accelerator handle from the dataspace
  ** in the child window's extrabytes. 
  */
  return( (HACCEL) GetWindowWord( hWnd, TVL_GWW_HACCEL ) );
}


void __far __pascal __export RegisterTVAccel( HWND hWnd, HACCEL hAccel )
{
  /* Error checking. */
  if( !IsWindow( hWnd ) ) return;

  /* 
  ** Save the child's accelerator handle in our own dataspace
  ** in the child window's extrabytes. 
  */
  SetWindowWord( hWnd, TVL_GWW_HACCEL, (WORD) hAccel );
}


void __far __pascal SetTVEntryCount( HWND hWnd, WORD wValue )
{
  /* Error checking. */
  if( !IsWindow( hWnd ) ) return;

  /* 
  ** Set the current count. 
  */
  SetWindowWord( hWnd, TVL_GWW_ENTRIES, 0 );
}


void __far __pascal IncTVEntryCount( HWND hWnd )
{
  WORD wCount;
  
  /* Error checking. */
  if( !IsWindow( hWnd ) ) return;

  /* 
  ** Get the current count. 
  */
  wCount = GetWindowWord( hWnd, TVL_GWW_ENTRIES );

  /* 
  ** Set the current count. 
  */
  SetWindowWord( hWnd, TVL_GWW_ENTRIES, wCount+1 );
}


void __far __pascal DecTVEntryCount( HWND hWnd )
{
  WORD wCount;
  
  /* Error checking. */
  if( !IsWindow( hWnd ) ) return;

  /* 
  ** Get the current count. 
  */
  wCount = GetWindowWord( hWnd, TVL_GWW_ENTRIES );

  /* 
  ** Set the current count. 
  */
  if( wCount != 0 )
  {
    SetWindowWord( hWnd, TVL_GWW_ENTRIES, wCount-1 );
  }
}


WORD __far __pascal GetTVEntryCount( HWND hWnd )
{
  /* Error checking. */
  if( !IsWindow( hWnd ) ) return 0;

  /* 
  ** Get the current count. 
  */
  return( GetWindowWord( hWnd, TVL_GWW_ENTRIES ) );
}


/*************************************
*
*  Sets child window subclassing.
*
**************************************/
void __far __pascal SetChildWindowFilter( HWND hWnd )
{
  WORD wReturn;
  WNDPROC ChildWndProc;
  
  /* Error checking. */
  if( !IsWindow( hWnd ) ) return;

  /* Replace the child's WndProc() with our own. */
  ChildWndProc = (WNDPROC) GetWindowLong( hWnd, GWL_WNDPROC );
  SetWindowLong( hWnd, GWL_WNDPROC, (LONG) TVFilterWndProc );
  
  /* Save the child's WndProc() in our own dataspace in the child window's extrabytes. */
  wReturn = SetWindowLong( hWnd, TVL_GWW_WNDPROC, (LONG) ChildWndProc );
}


/*************************************
*
*  Removes child window subclassing.
*
**************************************/
void __far __pascal RemoveChildWindowFilter( HWND hWnd )
{
  WNDPROC ChildWndProc;
  
  /* Error checking. */
  if( !IsWindow( hWnd ) ) return;

  /* Get the child's WndProc() from our own dataspace in the child window's extrabytes. */
  ChildWndProc = (WNDPROC) GetWindowLong( hWnd, TVL_GWW_WNDPROC );

  /* Set the child's WndProc() to its original WndProc(). */
  SetWindowLong( hWnd, GWL_WNDPROC, (LONG) ChildWndProc );

  /* Clear the WndProc() storage dataspace in the child window's extrabytes. */
  SetWindowLong( hWnd, TVL_GWW_WNDPROC, (LONG) 0 );
}


