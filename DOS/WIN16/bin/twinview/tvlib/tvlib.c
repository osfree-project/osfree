/***************************************************************************************
**
**
**                         DLL Startup and Closing Functions
**
**
***************************************************************************************/


/*************************************
*
*  System Includes
*
**************************************/
#include <windows.h>
#include <string.h>
#include <dos.h>
#include <direct.h>


/*************************************
*
*  Custom Includes
*
**************************************/
#include "tvlibprv.h"


/*************************************
*
*  Private Defines
*
**************************************/
//#define TVLIB_WILD "*.tvl"


/*************************************
*
*  Global Variables
*
**************************************/
HINSTANCE   hDLLInst = 0;


/***************************************************************************
*
*  Procecure Name:
*
*  Purpose: 
*
*  Returns:
*
*  Notes:
*
*  Revisions: Paul Kissel, 9/30/92.
*
*****************************************************************************/
int __far __pascal LibMain( HINSTANCE hInst, WORD wDataSeg, WORD cbHeap, LPSTR lpszCmdLine )
{
  /* Save instance of library. */ 
  hDLLInst = hInst;

  /* Create the library list using the global handle. */
//  if( ! CreateTVLibList( TVLIB_WILD ) ) return 0;

  /* Success. */
  return(1);
}


int __far __pascal _WEP( int nExitType )
{
//  /* Build library list. */
//  DestroyTVLibList();
  
//  /* Delete menu list. */
//  DeleteMenuInfoList();

  /* return 1 = success; 0 = fail */
  return(1);  
}













