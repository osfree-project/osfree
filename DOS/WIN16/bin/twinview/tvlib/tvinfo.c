/***************************************************************************************
**
**
**                              TwinView Info Functions
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
*  Globals
*
**************************************/
HMENU  hTVMenu = 0;
HWND   hTVWnd = 0;



void __far __pascal __export SetTVMenuHandle( HMENU hMenu )
{
  hTVMenu = hMenu;
}


HMENU __far __pascal __export GetTVMenuHandle( void )
{
  return( hTVMenu );
}

void __far __pascal __export SetTVWindowHandle( HWND hWnd )
{
  hTVWnd = hWnd;
}


HWND __far __pascal __export GetTVWindowHandle( void )
{
  return( hTVWnd );
}


BOOL __far __pascal __export CanTwinViewClose( void )
{
  /* See if all child windows can closed. */
  return( CloseAllTVWindows() );
}

