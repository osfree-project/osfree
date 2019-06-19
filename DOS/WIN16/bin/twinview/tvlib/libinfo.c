/*
	libinfo.c
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
/***************************************************************************************
**
**
**                              Library Info Functions
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
#define FILTER_WILD_SEPARATOR ';'



/*************************************
*
*  An library file funtion that recieves
*  library info and inserts it into
*  the library list.
*
**************************************/
BOOL __far __pascal InsertLibInfo
( 
  char __far * szLibFilePath, 
  FPTVLIBFILEINFO fpTmpTVLibFileInfo, 
  LPARAM lParam 
)
{
  FPTVLIBLIST fpTVLibList;         /* Pointer to an allocated library list. */

  FPTVLIBINFO fpTVLibInfo;         /* Pointer to the top of the lib. info list. */

  HTVLIBFILEINFO hTVLibFileInfo;    /* Handle for memory to a lib file info structure. */
  FPTVLIBFILEINFO fpTVLibFileInfo;  /* Pointer to a lib file info structure. */


  /* Lock the TVLIBLIST handle. */
  fpTVLibList = (FPTVLIBLIST) GlobalLock( hTVLibList );
  
  /* First TVLIBINFO structure in array. */
  if( fpTVLibList->hTVLibInfo == NULL )
  {
    if( ! GlobalAllocMem(  NULL, &(fpTVLibList)->hTVLibInfo, sizeof(TVLIBINFO) ) ) 
    {
      GlobalUnlock( hTVLibList );
      return FALSE;
    }
  }

  /* Not first time through - add another TVLIBINFO structure to the array. */
  else
  {
    if( !GlobalReAllocMem( NULL, 
                           &(fpTVLibList)->hTVLibInfo, 
                           sizeof( TVLIBINFO ) * (fpTVLibList->nLibCount + 1) ) )
    {
      GlobalUnlock( hTVLibList );
      return FALSE;
    }
  }

  /* Lock the TVLIBINFO list. */
  fpTVLibInfo = (FPTVLIBINFO) GlobalLock( fpTVLibList->hTVLibInfo ); 

  /* Move to appropriate place in the TVLIBINFO list. */
  fpTVLibInfo = fpTVLibInfo + fpTVLibList->nLibCount;
  
  /* Initialize the TVLIBINFO data. */
  _fstrcpy( fpTVLibInfo->szLibFilePath, szLibFilePath);
  fpTVLibInfo->hInstance = 0;
  fpTVLibInfo->hTVWndInfo = 0;

  /* Allocate a TVLIBFILEINFO structure to store the library's file info.  */
  if( ! GlobalAllocMem(  NULL, &hTVLibFileInfo, sizeof(TVLIBFILEINFO) ) ) 
  {
    GlobalUnlock( fpTVLibList->hTVLibInfo );
    GlobalUnlock( hTVLibList );
    return FALSE;
  }

  /* Lock the new LIBFILEINFO handle. */
  fpTVLibFileInfo = (FPTVLIBFILEINFO) GlobalLock( hTVLibFileInfo ); 

  /* Assign LIBFILEINFO values. */
  _fstrcpy( fpTVLibFileInfo->szFilter, fpTmpTVLibFileInfo->szFilter );
  _fstrcpy( fpTVLibFileInfo->szIconResouce, fpTmpTVLibFileInfo->szIconResouce );
  _fstrcpy( fpTVLibFileInfo->szIconCaption, fpTmpTVLibFileInfo->szIconCaption );
  fpTVLibFileInfo->bAskForFile = fpTmpTVLibFileInfo->bAskForFile;

  /* Unlock the LIBFILEINFO handle. */
  GlobalUnlock( hTVLibFileInfo );

  /* Place the TVLIBFILEINFO handle in the TVLIBINFO structure. */
  fpTVLibInfo->hTVLibFileInfo = hTVLibFileInfo;

  /* Unlock the the TVLIBINFO handle. */
  GlobalUnlock( fpTVLibList->hTVLibInfo );

  /* Increment the library counter. */
  fpTVLibList->nLibCount = fpTVLibList->nLibCount + 1;

  /* Unlock the TVLIBLIST handle. */
  GlobalUnlock( hTVLibList );

  /* Success. */
  return TRUE;
}


/*************************************
*
*  Deletes the data inside of a LIBINFO
*  structure.
*
**************************************/
void __far __pascal DeleteLibInfoData( FPTVLIBINFO fpTVLibInfo )
{
  /* Free the TVLIBFILEINFO handle. */
  GlobalFree( fpTVLibInfo->hTVLibFileInfo );

  /* Free the window information. */
  DeleteAllWndInfo( &(fpTVLibInfo)->hTVWndInfo );
}


/*************************************
*
*  Enumerates the TVLIBINFO structures
*  in a library list.
*
**************************************/
BOOL __far __pascal __export EnumTVLibInfo( TVLIBINFOENUMPROC EnumTVLibInfoProc, LPARAM lParam )
{
  FPTVLIBLIST fpTVLibList;        /* Pointer to the library list. */
  FPTVLIBINFO fpTVLibInfo;        /* Pointer to the top of the lib. info list. */
  int Counter;                    /* Incrementer. */
  BOOL nReturn;                   /* Call-back return value. */
 

  /* Error - Bad list handle. */
  if( hTVLibList == 0 ) return TRUE;

  /* Get the TVLIBLIST handle from the LPARAM. */
  fpTVLibList = (FPTVLIBLIST) GlobalLock( hTVLibList );

  /* Error - Bad list handle. */
  if( fpTVLibList == NULL ) return FALSE;

  /* No lib info in the list. */
  if( fpTVLibList->nLibCount == 0 ) 
  {
    GlobalUnlock( hTVLibList );
    return TRUE;
  }
  
  /* Lock the lib info list. */ 
  fpTVLibInfo = (FPTVLIBINFO) GlobalLock( fpTVLibList->hTVLibInfo );

  /* For each TVLIBINFO structure in the list. */
  for( Counter = 0; Counter < fpTVLibList->nLibCount; Counter++ )
  {
     /* Call call-back function. */
     nReturn = (*EnumTVLibInfoProc)( fpTVLibInfo, lParam );
     
     /* Does the call-back function want us to stop. */
     if( !nReturn ) break;

     /* Move to the next TVLIBINFO structure in the list. */
     fpTVLibInfo++;
  }

  GlobalUnlock( fpTVLibList->hTVLibInfo );
  GlobalUnlock( hTVLibList );

  /* Success. */
  return TRUE;
}

    
/*************************************
*
*  This function looks in the 
*  TWINVIEW.INI file for the name of
*  a library that has been associated
*  with szFileName's extension. 
*
**************************************/
BOOL __far __pascal DoesLibSupportExt( FPTVLIBINFO fpTVLibInfo, char __far * szFilePath )
{
  char szFileExt[_MAX_EXT];
  char szWildExt[_MAX_EXT];
  
  char szFilter[MAX_FILTER];

  char __far * szFilterField;
  char __far * szNextFilterField;

  char __far * szFilterWild;
  char __far * szNextFilterWild;

  int cbString;
  char chDelimiter;

  FPTVLIBFILEINFO fpLibFileInfo;
   
  int nReturn;
  
  
  /* Get the szFilePath's extension. */
  if( !GetFilesExtension( szFilePath, szFileExt ) )return FALSE;


  /* Lock the TVLIBFILEINFO data. */
  fpLibFileInfo = (FPTVLIBFILEINFO) GlobalLock( fpTVLibInfo->hTVLibFileInfo ); 

  /* If the library's file has no filter info. */
  if( fpLibFileInfo->szFilter[0] == NULL_CHAR ) 
  {
    /* Unlock the TVLIBFILEINFO data. */
    GlobalUnlock( fpTVLibInfo->hTVLibFileInfo ); 

    return FALSE;
  }
  
  /* 
  ** Check to see if one of the filters supports
  ** the filename's extension.
  */

  /* Copy the lib. file's filter. */
  _fstrcpy( szFilter, fpLibFileInfo->szFilter );

  /* Get filter's length. */
  cbString = _fstrlen( szFilter); 
  
  /* Retrieve filter's field separator. */
  chDelimiter = szFilter[cbString - 1]; 
  
  /* Make a new end for the string. */
  szFilter[cbString - 1] = NULL_CHAR;

  /* 
  ** Go though filter's info. 
  ** looking for the extension. 
  */

  /* Get description field. */
  szFilterField = szFilter;
  GetNextToken( &szFilterField, &szNextFilterField, chDelimiter );
   
  /* While we find a field in the filter. */
  while( szFilterField )
  {
    /* Get wildcard filenames field. */
    if( szNextFilterField != NULL )
    {
      szFilterField = szNextFilterField;
      GetNextToken( &szFilterField, &szNextFilterField, chDelimiter );
    }
    else szFilterField = NULL;
    
    /* If we got a list of wildcards. */
    if( szFilterField != NULL )
    {
      /* Get first wildcard in the wildcard filenames field. */
      szFilterWild = szFilterField;
      GetNextToken( &szFilterWild, &szNextFilterWild, FILTER_WILD_SEPARATOR );

      /* While we find wildcard filenames in the filename field. */
      while( szFilterWild )
      {
        /* Get the filter's extension. */
        if( !GetFilesExtension( szFilterWild, szWildExt ) )
        {
          /* Unlock the TVLIBFILEINFO data. */
          GlobalUnlock( fpTVLibInfo->hTVLibFileInfo );

          /* Failure. */
          return FALSE;
        }

        /* Compare the file extensions. */
        nReturn = _fstricmp( szWildExt, szFileExt ); 
        
        /* Success - Found a library supporting file extension. */ 
        if( nReturn == 0 )
        {
          /* Unlock the TVLIBFILEINFO data. */
          GlobalUnlock( fpTVLibInfo->hTVLibFileInfo );
          
          /* Success. */
          return TRUE;
        }
        
        /* Get next wildcard in the wildcard filenames field. */
        if( szNextFilterWild != NULL )
        {
          szFilterWild = szNextFilterWild;
          GetNextToken( &szFilterWild, &szNextFilterWild, FILTER_WILD_SEPARATOR );
        }
        else szFilterWild = NULL;
      }
    }
  
    /* Get next description field. */
    if( szNextFilterField != NULL )
    {
      szFilterField = szNextFilterField;
      GetNextToken( &szFilterField, &szNextFilterField, chDelimiter );
    }
    else szFilterField = NULL;
  }
  
  /* Unlock the TVLIBFILEINFO data. */
  GlobalUnlock( fpTVLibInfo->hTVLibFileInfo ); 

  /* Failure. */
  return FALSE; 
}


/*************************************
*  
*  Receives the each library's info.
*
*  If the desired window (passed in the 
*  LPARAM value) is found in the
*  library's window list, the window is 
*  removed from the list.
*
*  Closes the library if it was the library's
*  last window.
*                  
**************************************/
BOOL __far __pascal RemoveChildWindow( FPTVLIBINFO fpTVLibInfo, LPARAM lParam )
{
  HWND __far * hWnd;
  
  
  /* Get LPARAM data. */
  hWnd = (HWND __far *) lParam;
  
  /* If we found and deleted the window. */
  if( DeleteWndInfo( &(fpTVLibInfo)->hTVWndInfo, *hWnd ) )
  {
    /* 
    ** Try to close the library. 
    ** It will only work if no more windows
    ** exist.
    */
    PostCloseLibrary( fpTVLibInfo );
    
    /* Stop enumerating. */
    return FALSE;
  }

  /* Keep enumerating - next library. */
  return TRUE;
}


/*************************************
*
*  Open a library and get the
*  address of the function exported
*  with OrdinalValue.
*                  
**************************************/
BOOL __far __pascal OpenLibrary( HWND hWnd, FPTVLIBINFO fpTVLibInfo  )
{
  /* If not already opened. */
  if( fpTVLibInfo->hInstance == 0 )
  {
    /* Open DLL. */
    fpTVLibInfo->hInstance = LoadLibrary( fpTVLibInfo->szLibFilePath );
                                                
    /* Error opening library. */
    if ( fpTVLibInfo->hInstance <= HINSTANCE_ERROR )
    {
      MessageBox( hWnd, (LPSTR)"LoadLibrary() failed!", (LPSTR)"Library Functions", MB_ICONHAND | MB_OK );
      return FALSE;
    }
  }
  
  /* Success. */
  return TRUE;
}    


/*************************************
*
*  Free a library if there are no windows
*  using the library.
*                  
**************************************/
void __far __pascal CloseLibrary( FPTVLIBINFO fpTVLibInfo  )
{
  /* If already closed. */
  if( fpTVLibInfo->hInstance == 0 ) return;

  /* If library has opened windows using it. */
  if( fpTVLibInfo->hTVWndInfo != 0 ) return;
  
  /* Free the library. */
  FreeLibrary( fpTVLibInfo->hInstance ); 
  
  /* Set library's instance handle back to 0. */
  fpTVLibInfo->hInstance = 0;
}


/*************************************
*
*  Free a library if there are no windows
*  using the library by calling
*  TWINVIEW.EXE and asking it to close it.
*
*  This is used so that a library can complete
*  its work before we close it.
*                  
**************************************/
void __far __pascal PostCloseLibrary( FPTVLIBINFO fpTVLibInfo  )
{
  /* If already closed. */
  if( fpTVLibInfo->hInstance == 0 ) return;

  /* If library has opened windows using it. */
  if( fpTVLibInfo->hTVWndInfo != 0 ) return;
  
  /* Free the library. */
  PostMessage( GetTVWindowHandle(), GetTVMsgValue(), MSGID_CLOSELIB, (LPARAM) fpTVLibInfo->hInstance ); 
  
  /* Set library's instance handle back to 0. */
  fpTVLibInfo->hInstance = 0;
}


HTVWNDINFO __far __pascal GetLibWndInfo( FPTVLIBINFO fpTVLibInfo )
{
  return fpTVLibInfo->hTVWndInfo;
}




