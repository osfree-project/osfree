/*    
	WinHelp.c	1.17
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
/*************************************
*
*  System Includes
*
**************************************/


#include <string.h>
#include <stdlib.h>
#include "windows.h"


/*************************************
*
*  Private Defines
*
**************************************/
#include "WinHelp.h"

/*
** Error message strings.
*/
#define IDS_HELP_ERROR    "Help System Error"
#define IDS_OPEN_LIBRARY  "Cannot open the help library."
#define IDS_BAD_HELP_MSG  "Unsupported help message encounter."
#define IDS_MEMORY_ERR    "Cannot allocate enough memory for the operation."
#define IDS_WINHELPROG    "twinview.exe"
                  

                  
/*************************************
*
*  Prototypes of private functions
*
**************************************/

static BOOL WinHelpEx( HWND hHelpWnd, HWND hAppWnd, LPCSTR lpszHelpFile, UINT fuCommand, DWORD dwData );

static BOOL CreateKeyData( HWND hErrorWnd, GLOBALHANDLE * hHelpParams, LPCSTR szPath, LPSTR szKeyWord );
static void DestroyKeyData( GLOBALHANDLE hHelpParams );

static BOOL CreateCommandData( HWND hErrorWnd, GLOBALHANDLE * hHelpParams, LPCSTR szPath, LPSTR szMacro );
static void DestroyCommandData( GLOBALHANDLE hHelpParams );

static BOOL CreatePartialKeyData( HWND hErrorWnd, GLOBALHANDLE * hHelpParams, LPCSTR szPath, LPSTR szKeyWord );
static void DestroyPartialKeyData( GLOBALHANDLE hHelpParams );

static BOOL CreateMultiKeyData( HWND hErrorWnd, GLOBALHANDLE * hHelpParams, LPCSTR szPath, MULTIKEYHELP * MultiKeyIn );
static void DestroyMultiKeyData( GLOBALHANDLE hHelpParams );

static BOOL CreateContextPopupData( HWND hErrorWnd, GLOBALHANDLE * hHelpParams, LPCSTR szPath, unsigned long Context );
static void DestroyContextPopupData( GLOBALHANDLE hHelpParams );

static BOOL CreateSetContentsData( HWND hErrorWnd, GLOBALHANDLE * hHelpParams, LPCSTR szPath, unsigned long Context );
static void DestroySetContentsData( GLOBALHANDLE hHelpParams );

static BOOL CreateContextData( HWND hErrorWnd, GLOBALHANDLE * hHelpParams, LPCSTR szPath, unsigned long Context );
static void DestroyContextData( GLOBALHANDLE hHelpParams );

static BOOL CreateHelpOnHelpData( HWND hErrorWnd, GLOBALHANDLE * hHelpParams, LPCSTR szPath );
static void DestroyHelpOnHelpData( GLOBALHANDLE hHelpParams );

static BOOL CreateForceFileData( HWND hErrorWnd, GLOBALHANDLE * hHelpParams, LPCSTR szPath );
static void DestroyForceFileData( GLOBALHANDLE hHelpParams );

static BOOL CreateContentsData( HWND hErrorWnd, GLOBALHANDLE * hHelpParams, LPCSTR szPath );
static void DestroyContentsData( GLOBALHANDLE hHelpParams );

static BOOL CreateQuitData( HWND hErrorWnd, GLOBALHANDLE * hHelpParams );
static void DestroyQuitData( GLOBALHANDLE hHelpParams );

static BOOL  CreateSetWinPosData( HWND hErrorWnd, GLOBALHANDLE * hHelpParams, LPCSTR szPath, HELPWININFO * HelpWinInfoInPtr );
static void DestroySetWinPosData( GLOBALHANDLE hHelpParams );

static BOOL GlobalAllocMem( HWND hWnd, HGLOBAL * hGlobal, DWORD dwSize  );


/*************************************
*
*  Returns:
*
*     FALSE - Failure.
*     TRUE - Success. 
*
**************************************/
BOOL 
WinHelp( HWND hWnd, LPCSTR lpszHelpFile, UINT fuCommand, DWORD dwData )
{
#define SECTION    "windows"        /* INI section. */
#define ENTRY      "HelpProgram"    /* INI entry. */
#define NOT_FOUND  "!NOTFOUND!"     /* INI Entry not found. */
#define BUFF_SIZE  _MAX_FNAME + 5   /* Size of buffer to hold help program name. */
  
  HWND hHelpWnd;                   /* Handle to the main help window. */
  BOOL bReturn;                    /* Return value from WinHelpEx() function. */
  char szCmdLine[BUFF_SIZE];  /* Command line when calling help program. */
  

  /* Find the help message redirector window. */
  hHelpWnd = FindWindow( MS_WINHELP_CLASS, NULL );

  if( (hHelpWnd == (HWND) NULL) && fuCommand == HELP_QUIT ) {
	return TRUE;
  }

  /* 
  ** If window does not exist, launch TWINVIEW and
  ** tell it to open the help message redirector 
  ** window.
  */
  if( (hHelpWnd == (HWND) NULL) && fuCommand != HELP_QUIT )
  {
    /* 
    ** Make command line. 
    */
    /* Get program name from WIN.INI. */
    GetProfileString( SECTION, ENTRY, NOT_FOUND, szCmdLine, BUFF_SIZE );

    /* No entry for program name. */
    if( _fstrcmp( szCmdLine, NOT_FOUND ) == 0 ) 
    {
      /* Use standard help program. */
      _fstrcpy( szCmdLine, IDS_WINHELPROG );
    }
    
    /* Use standard help program. */
    _fstrcat( szCmdLine, " -x" );
    
    /* Run help program. */
    WinExec( szCmdLine, SW_HIDE );

    /* Try to find the help message redirector window again. */
    hHelpWnd = FindWindow( MS_WINHELP_CLASS, NULL );

    /* Did we find it? */
    if( hHelpWnd == (HWND) NULL )
    {
      /* Error. */
      return( FALSE );
    }
  }
  

  /* Build and send message data to the help message redirector window. */
  bReturn = WinHelpEx( hHelpWnd, hWnd, lpszHelpFile, fuCommand, dwData );

  
  /* Return message building success. */
  return( bReturn );
}


/****************************************************
*
*  WinHelpEx()
*
*  Creates the message data that will be sent
*  to a MS_WINHELP window.  
*
*  Sends the help message redirector window. 
*
*  Returns:
*
*     FALSE - Failure.
*     TRUE - Success.
*
*****************************************************/
static int WinHelpEx( HWND hHelpWnd, HWND hAppWnd, LPCSTR lpszHelpFile, UINT fuCommand, DWORD dwData )
{
  GLOBALHANDLE hHelpParams;      /* Handle to a help message LPARAM data structure. */
  static UINT  WM_WINHELP = 0;   /* Registered window help message for API. */


  /* Register the MS_WINHELP message. */
  if( WM_WINHELP == 0 )
  {
    /* Register special help messages. */
    WM_WINHELP  = RegisterWindowMessage( MSWIN_HELP_MSG_STRING );
  }

  /* Process the message. */
  switch( fuCommand ) 
  {
    case HELP_QUIT:      
    {
      /* Allocate the  event's HELPPARAMS data. */
      if( ! CreateQuitData( hAppWnd, &hHelpParams ) ) 
      {
        /* Failure. */
        return( FALSE );
      }
  
      /* Send the help viewer the message. */
      SendMessage( hHelpWnd, WM_WINHELP, (WPARAM) hAppWnd, (LPARAM) hHelpParams );
          
      /* Free the  event's HELPPARAMS data. */
      DestroyQuitData( hHelpParams );
  
      /* Success. */
      return( TRUE );
    }


    case HELP_CONTEXT:      
    {
      /* Allocate the  event's HELPPARAMS data. */
      if( ! CreateContextData( hAppWnd, &hHelpParams, lpszHelpFile, (unsigned long) dwData ) ) 
      {
        /* Failure. */
        return( FALSE );
      }
  
      /* Send the help viewer the message. */
      SendMessage( hHelpWnd, WM_WINHELP, (WPARAM) hAppWnd, (LPARAM) hHelpParams );
          
      /* Free the  event's HELPPARAMS data. */
      DestroyContextData( hHelpParams );
  
      /* Success. */
      return( TRUE );
    }


    case HELP_CONTEXTPOPUP: 
    {
      /* Allocate the  event's HELPPARAMS data. */
      if( ! CreateContextPopupData( hAppWnd, &hHelpParams, lpszHelpFile, (unsigned long) dwData ) )
      {
        /* Failure. */
        return( FALSE );
      }
  
      /* Send the help viewer the message. */
      SendMessage( hHelpWnd, WM_WINHELP, (WPARAM) hAppWnd, (LPARAM) hHelpParams );
          
      /* Free the  event's HELPPARAMS data. */
      DestroyContextPopupData( hHelpParams );
  
      /* Success. */
      return( TRUE );
    }


    case HELP_CONTENTS:     
    {
      /* Allocate the  event's HELPPARAMS data. */
      if( ! CreateContentsData( hAppWnd, &hHelpParams, lpszHelpFile ) )
      {
        /* Failure. */
        return( FALSE );
      }
  
      /* Send the help viewer the message. */
      SendMessage( hHelpWnd, WM_WINHELP, (WPARAM) hAppWnd, (LPARAM) hHelpParams );
          
      /* Free the  event's HELPPARAMS data. */
      DestroyContentsData( hHelpParams );
  
      /* Success. */
      return( TRUE );
    }


    case HELP_SETCONTENTS:  
    {
      /* Allocate the  event's HELPPARAMS data. */
      if( ! CreateSetContentsData( hAppWnd, &hHelpParams, lpszHelpFile, (unsigned long) dwData ) ) 
      {
        /* Failure. */
        return( FALSE );
      }
  
      /* Send the help viewer the message. */
      SendMessage( hHelpWnd, WM_WINHELP, (WPARAM) hAppWnd, (LPARAM) hHelpParams );
          
      /* Free the  event's HELPPARAMS data. */
      DestroySetContentsData( hHelpParams );
  
      /* Success. */
      return( TRUE );
    }


    case HELP_KEY:          
    {
      /* Allocate the  event's HELPPARAMS data. */
      if( ! CreateKeyData( hAppWnd, &hHelpParams, lpszHelpFile, (LPSTR) dwData ) ) 
      {
        /* Failure. */
        return( FALSE );
      }
  
      /* Send the help viewer the message. */
      SendMessage( hHelpWnd, WM_WINHELP, (WPARAM) hAppWnd, (LPARAM) hHelpParams );
          
      /* Free the  event's HELPPARAMS data. */
      DestroyKeyData( hHelpParams );
  
      /* Success. */
      return( TRUE );
    }


    case HELP_PARTIALKEY:   
    {
      /* Allocate the  event's HELPPARAMS data. */
      if( ! CreatePartialKeyData( hAppWnd, &hHelpParams, lpszHelpFile, (LPSTR) dwData ) ) 
      {
        /* Failure. */
        return( FALSE );
      }
  
      /* Send the help viewer the message. */
      SendMessage( hHelpWnd, WM_WINHELP, (WPARAM) hAppWnd, (LPARAM) hHelpParams );
          
      /* Free the  event's HELPPARAMS data. */
      DestroyPartialKeyData( hHelpParams );
  
      /* Success. */
      return( TRUE );
    }


    case HELP_COMMAND:      
    {
      /* Allocate the  event's HELPPARAMS data. */
      if( ! CreateCommandData( hAppWnd, &hHelpParams, lpszHelpFile, (LPSTR) dwData ) ) 
      {
        /* Failure. */
        return( FALSE );
      }
  
      /* Send the help viewer the message. */
      SendMessage( hHelpWnd, WM_WINHELP, (WPARAM) hAppWnd, (LPARAM) hHelpParams );
          
      /* Free the  event's HELPPARAMS data. */
      DestroyCommandData( hHelpParams );
  
      /* Success. */
      return( TRUE );
    }


    case HELP_FORCEFILE:    
    {
      /* Allocate the  event's HELPPARAMS data. */
      if( ! CreateForceFileData( hAppWnd, &hHelpParams, lpszHelpFile ) ) 
      {
        /* Failure. */
        return( FALSE );
      }
  
      /* Send the help viewer the message. */
      SendMessage( hHelpWnd, WM_WINHELP, (WPARAM) hAppWnd, (LPARAM) hHelpParams );
          
      /* Free the  event's HELPPARAMS data. */
      DestroyForceFileData( hHelpParams );
  
      /* Success. */
      return( TRUE );
    }


    case HELP_HELPONHELP:   
    {
      /* Allocate the  event's HELPPARAMS data. */
      if( ! CreateHelpOnHelpData( hAppWnd, &hHelpParams, lpszHelpFile ) )  
      {
        /* Failure. */
        return( FALSE );
      }
  
      /* Send the help viewer the message. */
      SendMessage( hHelpWnd, WM_WINHELP, (WPARAM) hAppWnd, (LPARAM) hHelpParams );
          
      /* Free the  event's HELPPARAMS data. */
      DestroyHelpOnHelpData( hHelpParams );
  
      /* Success. */
      return( TRUE );
    }

    case HELP_MULTIKEY:     
    {
      /* Allocate the  event's HELPPARAMS data. */
      if( ! CreateMultiKeyData( hAppWnd, &hHelpParams, lpszHelpFile, (MULTIKEYHELP *) dwData ) ) 
      {
        /* Failure. */
        return( FALSE );
      }
  
      /* Send the help viewer the message. */
      SendMessage( hHelpWnd, WM_WINHELP, (WPARAM) hAppWnd, (LPARAM) hHelpParams );
          
      /* Free the  event's HELPPARAMS data. */
      DestroyMultiKeyData( hHelpParams );
  
      /* Success. */
      return( TRUE );
    }


    /* Unsupported WinHelp Commands. */
    case HELP_SETWINPOS:    
    {
      /* Allocate the  event's HELPPARAMS data. */
      if( ! CreateSetWinPosData( hAppWnd, &hHelpParams, lpszHelpFile, (HELPWININFO *) dwData ) ) 
      {
        /* Failure. */
        return( FALSE );
      }
  
      /* Send the help viewer the message. */
      SendMessage( hHelpWnd, WM_WINHELP, (WPARAM) hAppWnd, (LPARAM) hHelpParams );
          
      /* Free the  event's HELPPARAMS data. */
      DestroySetWinPosData( hHelpParams );
  
      /* Success. */
      return( TRUE );
    }

  }

  /* If we get down to here then the command is not recognized. */ 
  MessageBox( hAppWnd, (LPSTR) IDS_BAD_HELP_MSG, (LPSTR) IDS_HELP_ERROR, MB_ICONHAND | MB_OK );

  /* Failure. */
  return FALSE;
}






/*************************************
*
*   Builds the HELPPARAMS data for the HELP_KEY event.
*
**************************************/
static 
int CreateKeyData( HWND hErrorWnd, GLOBALHANDLE * hHelpParams, LPCSTR szPath, LPSTR szKeyWord )
{
  FPHELPPARAMS fpHelpParams;


  /* Verify that path data was supplied. */
  if( szPath == NULL ) return FALSE;
  if( _fstrlen( szPath ) == 0 ) return FALSE; 
  
  /* Verify that path data was supplied. */
  if( szKeyWord == NULL ) return FALSE;
  if( _fstrlen( szKeyWord ) == 0 )  return FALSE; 
  
  
  /* 
  ** Allocate help parameters structure. 
  ** Null char. of Path string accounted for
  ** by Path[1] in structure.
  */
  if( !GlobalAllocMem( hErrorWnd, hHelpParams, sizeof( HELPPARAMS ) + _fstrlen( szPath ) + (_fstrlen( szKeyWord ) + 1)) )
  {
    /* Failure. */
    return FALSE;
  }
  fpHelpParams = (FPHELPPARAMS) GlobalLock( *hHelpParams ); 

  /* Fill help parameters. */
  fpHelpParams->Size = sizeof( HELPPARAMS ) + _fstrlen( szPath ) + (_fstrlen( szKeyWord ) + 1);
  fpHelpParams->Message = HELP_KEY;
  fpHelpParams->Context = 0;
  fpHelpParams->Unknown = 0;
  fpHelpParams->PathOffset = 0x10;
  _fstrcpy( fpHelpParams->Path, szPath ); 
  fpHelpParams->ExtraDataOffset = fpHelpParams->PathOffset + _fstrlen( szPath ) + 1;
  _fstrcpy( (char * ) fpHelpParams + fpHelpParams->ExtraDataOffset, szKeyWord );
  
          
  /* Unlock the structure's handle. */
  GlobalUnlock( *hHelpParams );

  /* Success. */
  return( TRUE );
}

/*************************************
*
*   Frees the HELPPARAMS data for the HELP_KEY event.
*
**************************************/
static void 
DestroyKeyData( GLOBALHANDLE hHelpParams )
{
  GlobalFree( hHelpParams );
}


/*************************************
*
*   Builds the HELPPARAMS data for the HELP_COMMAND event.
*
**************************************/
static BOOL 
CreateCommandData( HWND hErrorWnd, GLOBALHANDLE * hHelpParams, LPCSTR szPath, LPSTR szMacro )
{
  FPHELPPARAMS fpHelpParams;

#ifdef LATER
  /* Verify that path data was supplied. */
  if( szPath == NULL ) return FALSE;
  if( _fstrlen( szPath ) == 0 ) return FALSE; 
#endif
  
  /* Verify that path data was supplied. */
  if( szMacro == NULL ) return FALSE;
  if( _fstrlen( szMacro ) == 0 )  return FALSE; 
  
  
  /* 
  ** Allocate help parameters structure. 
  ** Null char. of Path string accounted for
  ** by Path[1] in structure.
  */
  if( !GlobalAllocMem(hErrorWnd, hHelpParams, sizeof( HELPPARAMS ) + _fstrlen( szPath ) + (_fstrlen( szMacro ) + 1)) )
  {
    /* Failure. */
    return FALSE;
  }
  fpHelpParams = (FPHELPPARAMS) GlobalLock( *hHelpParams ); 

  /* Fill help parameters. */
  fpHelpParams->Size = sizeof( HELPPARAMS ) + _fstrlen( szPath ) + (_fstrlen( szMacro ) + 1);
  fpHelpParams->Message = HELP_COMMAND;
  fpHelpParams->Context = 0;
  fpHelpParams->Unknown = 0;
  fpHelpParams->PathOffset = 0x10;

  if ( szPath && *szPath )
  {
    _fstrcpy( fpHelpParams->Path, szPath ); 
  }

  fpHelpParams->ExtraDataOffset = fpHelpParams->PathOffset + _fstrlen( szPath ) + 1;
  _fstrcpy( (char * ) fpHelpParams + fpHelpParams->ExtraDataOffset, szMacro );
  
          
  /* Unlock the structure's handle. */
  GlobalUnlock( *hHelpParams );

  /* Success. */
  return( TRUE );
}


/*************************************
*
*   Frees the HELPPARAMS data for the HELP_COMMAND event.
*
**************************************/
static void 
DestroyCommandData( GLOBALHANDLE hHelpParams )
{
  GlobalFree( hHelpParams );
}


/*************************************
*
*   Builds the HELPPARAMS data for the HELP_PARTIALKEY event.
*
**************************************/
static BOOL 
CreatePartialKeyData( HWND hErrorWnd, GLOBALHANDLE * hHelpParams, LPCSTR szPath, LPSTR szKeyWord )
{
  FPHELPPARAMS fpHelpParams;


  /* Verify that path data was supplied. */
  if( szPath == NULL ) return FALSE;
  if( _fstrlen( szPath ) == 0 ) return FALSE; 
  
  /* 
  ** Allocate help parameters structure. 
  ** Null char. of Path string accounted for
  ** by Path[1] in structure.
  */
  if( !GlobalAllocMem(hErrorWnd, hHelpParams, sizeof( HELPPARAMS ) + _fstrlen( szPath ) + (_fstrlen( szKeyWord ) + 1)) )
  {
    /* Failure. */
    return FALSE;
  }
  fpHelpParams = (FPHELPPARAMS) GlobalLock( *hHelpParams ); 

  /* Fill help parameters. */
  fpHelpParams->Size = sizeof( HELPPARAMS ) + _fstrlen( szPath ) + (_fstrlen( szKeyWord ) + 1);
  fpHelpParams->Message = HELP_PARTIALKEY;
  fpHelpParams->Context = 0;
  fpHelpParams->Unknown = 0;
  fpHelpParams->PathOffset = 0x10;
  _fstrcpy( fpHelpParams->Path, szPath ); 
  fpHelpParams->ExtraDataOffset = fpHelpParams->PathOffset + _fstrlen( szPath ) + 1;
  _fstrcpy( (char * ) fpHelpParams + fpHelpParams->ExtraDataOffset, szKeyWord );
  
          
  /* Unlock the structure's handle. */
  GlobalUnlock( *hHelpParams );

  /* Success. */
  return( TRUE );
}


/*************************************
*
*   Frees the HELPPARAMS data for the HELP_PARTIALKEY event.
*
**************************************/
static void 
DestroyPartialKeyData( GLOBALHANDLE hHelpParams )
{
  GlobalFree( hHelpParams );
}

/*************************************
*
*   Builds the HELPPARAMS data for the HELP_MULTIKEY event.
*
**************************************/
static BOOL CreateMultiKeyData( HWND hErrorWnd, GLOBALHANDLE * hHelpParams, LPCSTR szPath, MULTIKEYHELP * MultiKeyIn )
{
  FPHELPPARAMS fpHelpParams;
  WORD  wPhraseLength;
  MULTIKEYHELP * MultiKeyPtr;
                        
                        
  /* Verify that path data was supplied. */
  if( szPath == NULL ) return FALSE;
  if( _fstrlen( szPath ) == 0 ) return FALSE; 
  
  /* Get the length of the multikey's phrase. */
  wPhraseLength = _fstrlen( (LPSTR)MultiKeyIn->szKeyphrase );
  
  /* 
  ** Allocate help parameters structure. 
  **
  ** Null char. of Path string accounted for
  ** by Path[1] in HELPPARAMS structure.
  **
  ** Null char. of MultiKeyIn->szKeyphrase string accounted for
  ** by szKeyphrase[1] in MULTIKEYHELP structure.
  */
  if( !GlobalAllocMem(hErrorWnd, hHelpParams, sizeof( HELPPARAMS ) + _fstrlen( szPath ) + sizeof( MULTIKEYHELP ) + wPhraseLength ) )
  {
    /* Failure. */
    return FALSE;
  }
  fpHelpParams = (FPHELPPARAMS) GlobalLock( *hHelpParams ); 

  /* Fill help parameters. */
  fpHelpParams->Size = sizeof( HELPPARAMS ) + _fstrlen( szPath ) + sizeof( MULTIKEYHELP ) + wPhraseLength;
  fpHelpParams->Message = HELP_MULTIKEY;
  fpHelpParams->Context = 0;
  fpHelpParams->Unknown = 0;
  fpHelpParams->PathOffset = 0x10;
  _fstrcpy( fpHelpParams->Path, szPath ); 
  fpHelpParams->ExtraDataOffset = fpHelpParams->PathOffset + _fstrlen( szPath ) + 1;

  MultiKeyPtr = ( MULTIKEYHELP * ) ( ( BYTE * ) fpHelpParams + fpHelpParams->ExtraDataOffset );

  /* Save Multikey data. */
  MultiKeyPtr->mkSize      = sizeof( MULTIKEYHELP ) + wPhraseLength;
  MultiKeyPtr->mkKeylist   = MultiKeyIn->mkKeylist;
  _fstrcpy( (LPSTR)MultiKeyPtr->szKeyphrase, (LPSTR)MultiKeyIn->szKeyphrase );

  /* Unlock the structure's handle. */
  GlobalUnlock( *hHelpParams );

  /* Success. */
  return( TRUE );
}


/*************************************
*
*   Frees the HELPPARAMS data for the HELP_MULTIKEY event.
*
**************************************/
static void DestroyMultiKeyData( GLOBALHANDLE hHelpParams )
{
  GlobalFree( hHelpParams );
}


/*************************************
*
*   Builds the HELPPARAMS data for the HELP_CONTEXTPOPUP event.
*
**************************************/
static BOOL 
CreateContextPopupData( HWND hErrorWnd, GLOBALHANDLE * hHelpParams, LPCSTR szPath, unsigned long Context )
{
  FPHELPPARAMS fpHelpParams;


  /* Verify that path data was supplied. */
  if( szPath == NULL ) return FALSE;
  if( _fstrlen( szPath ) == 0 ) return FALSE; 
  
  /* Verify that vaild context data was supplied. */
  if( Context < 1 ) return FALSE;
  
  /* 
  ** Allocate help parameters structure. 
  ** Null char. of Path string accounted for
  ** by Path[1] in structure.
  */
  if( !GlobalAllocMem(hErrorWnd, hHelpParams, sizeof( HELPPARAMS ) + _fstrlen( szPath )) )
  {
    /* Failure. */
    return FALSE;
  }
  fpHelpParams = (FPHELPPARAMS) GlobalLock( *hHelpParams ); 

  /* Fill help parameters. */
  fpHelpParams->Size = sizeof( HELPPARAMS ) + _fstrlen( szPath );
  fpHelpParams->Message = HELP_CONTEXTPOPUP;
  fpHelpParams->Context = Context;
  fpHelpParams->Unknown = 0;
  fpHelpParams->PathOffset = 0x10;
  fpHelpParams->ExtraDataOffset = 0;
  _fstrcpy( fpHelpParams->Path, szPath ); 
          
  /* Unlock the structure's handle. */
  GlobalUnlock( *hHelpParams );

  /* Success. */
  return( TRUE );
}


/*************************************
*
*   Frees the HELPPARAMS data for the HELP_CONTEXTPOPUP event.
*
**************************************/
static void 
DestroyContextPopupData( GLOBALHANDLE hHelpParams )
{
  GlobalFree( hHelpParams );
}


/*************************************
*
*   Builds the HELPPARAMS data for the HELP_SETCONTENTS event.
*
**************************************/
static BOOL CreateSetContentsData( HWND hErrorWnd, GLOBALHANDLE * hHelpParams, LPCSTR szPath, unsigned long Context )
{
  FPHELPPARAMS fpHelpParams;


  /* Verify that path data was supplied. */
  if( szPath == NULL ) return FALSE;
  if( _fstrlen( szPath ) == 0 ) return FALSE; 
  
  /* Verify that vaild context data was supplied. */
  if( Context < 1 ) return FALSE;
  
  /* 
  ** Allocate help parameters structure. 
  ** Null char. of Path string accounted for
  ** by Path[1] in structure.
  */
  if( !GlobalAllocMem(hErrorWnd, hHelpParams, sizeof( HELPPARAMS ) + _fstrlen( szPath )) )
  {
    /* Failure. */
    return FALSE;
  }
  fpHelpParams = (FPHELPPARAMS) GlobalLock( *hHelpParams ); 

  /* Fill help parameters. */
  fpHelpParams->Size = sizeof( HELPPARAMS ) + _fstrlen( szPath );
  fpHelpParams->Message = HELP_SETCONTENTS;
  fpHelpParams->Context = Context;
  fpHelpParams->Unknown = 0;
  fpHelpParams->PathOffset = 0x10;
  fpHelpParams->ExtraDataOffset = 0;
  _fstrcpy( fpHelpParams->Path, szPath ); 
          
  /* Unlock the structure's handle. */
  GlobalUnlock( *hHelpParams );

  /* Success. */
  return( TRUE );
}


/*************************************
*
*   Frees the HELPPARAMS data for the HELP_SETCONTENTS event.
*
**************************************/
static void DestroySetContentsData( GLOBALHANDLE hHelpParams )
{
  GlobalFree( hHelpParams );
}


/*************************************
*
*   Builds the HELPPARAMS data for the HELP_CONTEXT event.
*
**************************************/
static BOOL CreateContextData( HWND hErrorWnd, GLOBALHANDLE * hHelpParams, LPCSTR szPath, unsigned long Context )
{
  FPHELPPARAMS fpHelpParams;


  /* Verify that path data was supplied. */
  if( szPath == NULL ) return FALSE;
  if( _fstrlen( szPath ) == 0 ) return FALSE; 
  
  /* Verify that vaild context data was supplied. */
  if( Context < 1 ) return FALSE;
  
  /* 
  ** Allocate help parameters structure. 
  ** Null char. of Path string accounted for
  ** by Path[1] in structure.
  */
  if( !GlobalAllocMem(hErrorWnd, hHelpParams, sizeof( HELPPARAMS ) + _fstrlen( szPath )) )
  {
    /* Failure. */
    return FALSE;
  }
  fpHelpParams = (FPHELPPARAMS) GlobalLock( *hHelpParams ); 

  /* Fill help parameters. */
  fpHelpParams->Size = sizeof( HELPPARAMS ) + _fstrlen( szPath );
  fpHelpParams->Message = HELP_CONTEXT;
  fpHelpParams->Context = Context;
  fpHelpParams->Unknown = 0;
  fpHelpParams->PathOffset = 0x10;
  fpHelpParams->ExtraDataOffset = 0;
  _fstrcpy( fpHelpParams->Path, szPath ); 
          
  /* Unlock the structure's handle. */
  GlobalUnlock( *hHelpParams );

  /* Success. */
  return( TRUE );
}


/*************************************
*
*   Frees the HELPPARAMS data for the HELP_CONTEXT event.
*
**************************************/
static void DestroyContextData( GLOBALHANDLE hHelpParams )
{
  GlobalFree( hHelpParams );
}


/*************************************
*
*   Builds the HELPPARAMS data for the HELP_HELPONHELP event.
*
**************************************/
static BOOL CreateHelpOnHelpData( HWND hErrorWnd, GLOBALHANDLE * hHelpParams, LPCSTR szPath )
{
  FPHELPPARAMS fpHelpParams;

  /* 
  ** Allocate help parameters structure. 
  ** Null char. of Path string accounted for
  ** by Path[1] in structure.
  */
  if( !GlobalAllocMem(hErrorWnd, hHelpParams, sizeof( HELPPARAMS ) ) )
  {
    /* Failure. */
    return FALSE;
  }
  fpHelpParams = (FPHELPPARAMS) GlobalLock( *hHelpParams ); 

  /* Fill help parameters. */
  fpHelpParams->Size = sizeof( HELPPARAMS );
  fpHelpParams->Message = HELP_HELPONHELP;
  fpHelpParams->Context = 0;
  fpHelpParams->Unknown = 0;
  fpHelpParams->PathOffset = 0x10;
  fpHelpParams->ExtraDataOffset = 0;
  fpHelpParams->Path[0] = '\0';
          
  /* Unlock the structure's handle. */
  GlobalUnlock( *hHelpParams );

  /* Success. */
  return( TRUE );
}



/*************************************
*
*   Frees the HELPPARAMS data for the HELP_HELPONHELP event.
*
**************************************/
static void DestroyHelpOnHelpData( GLOBALHANDLE hHelpParams )
{
  GlobalFree( hHelpParams );
}


/*************************************
*
*   Builds the HELPPARAMS data for the HELP_FORCEFILE event.
*
**************************************/
static BOOL CreateForceFileData( HWND hErrorWnd, GLOBALHANDLE * hHelpParams, LPCSTR szPath )
{
  FPHELPPARAMS fpHelpParams;

  /* Verify that path data was supplied. */
  if( szPath == NULL ) return FALSE;
  if( _fstrlen( szPath ) == 0 ) return FALSE; 
  
  /* 
  ** Allocate help parameters structure. 
  ** Null char. of Path string accounted for
  ** by Path[1] in structure.
  */
  if( !GlobalAllocMem(hErrorWnd, hHelpParams, sizeof( HELPPARAMS ) + _fstrlen( szPath )) )
  {
    /* Failure. */
    return FALSE;
  }
  fpHelpParams = (FPHELPPARAMS) GlobalLock( *hHelpParams ); 

  /* Fill help parameters. */
  fpHelpParams->Size = sizeof( HELPPARAMS ) + _fstrlen( szPath );
  fpHelpParams->Message = HELP_FORCEFILE;
  fpHelpParams->Context = 0;
  fpHelpParams->Unknown = 0;
  fpHelpParams->PathOffset = 0x10;
  fpHelpParams->ExtraDataOffset = 0;
  _fstrcpy( fpHelpParams->Path, szPath ); 
          
  /* Unlock the structure's handle. */
  GlobalUnlock( *hHelpParams );

  /* Success. */
  return( TRUE );
}



/*************************************
*
*   Frees the HELPPARAMS data for the HELP_FORCEFILE event.
*
**************************************/
static void DestroyForceFileData( GLOBALHANDLE hHelpParams )
{
  GlobalFree( hHelpParams );
}


/*************************************
*
*   Builds the HELPPARAMS data for the HELP_CONTENTS event.
*
**************************************/
static BOOL CreateContentsData( HWND hErrorWnd, GLOBALHANDLE * hHelpParams, LPCSTR szPath )
{
  FPHELPPARAMS fpHelpParams;


  /* Verify that path data was supplied. */
  if( szPath == NULL ) return FALSE;
  if( _fstrlen( szPath ) == 0 ) return FALSE; 
  
  /* 
  ** Allocate help parameters structure. 
  ** Null char. of Path string accounted for
  ** by Path[1] in structure.
  */
  if( !GlobalAllocMem(hErrorWnd, hHelpParams, sizeof( HELPPARAMS ) + _fstrlen( szPath )) )
  {
    /* Failure. */
    return FALSE;
  }
  fpHelpParams = (FPHELPPARAMS) GlobalLock( *hHelpParams ); 

  /* Fill help parameters. */
  fpHelpParams->Size = sizeof( HELPPARAMS ) + _fstrlen( szPath );
  fpHelpParams->Message = HELP_CONTENTS;
  fpHelpParams->Context = 0;
  fpHelpParams->Unknown = 0;
  fpHelpParams->PathOffset = 0x10;
  fpHelpParams->ExtraDataOffset = 0;
  _fstrcpy( fpHelpParams->Path, szPath ); 
          
  /* Unlock the structure's handle. */
  GlobalUnlock( *hHelpParams );

  /* Success. */
  return( TRUE );
}



/*************************************
*
*   Frees the HELPPARAMS data for the HELP_CONTENTS event.
*
**************************************/
static void DestroyContentsData( GLOBALHANDLE hHelpParams )
{
  GlobalFree( hHelpParams );
}


/*************************************
*
*   Builds the HELPPARAMS data for the HELP_QUIT event.
*
**************************************/
static BOOL CreateQuitData( HWND hErrorWnd, GLOBALHANDLE * hHelpParams )
{
  FPHELPPARAMS fpHelpParams;

  /* 
  ** Allocate help parameters structure. 
  */
  if( !GlobalAllocMem(hErrorWnd, hHelpParams, sizeof( HELPPARAMS )) ) 
  {
    /* Failure. */
    return FALSE;
  }
  fpHelpParams = (FPHELPPARAMS) GlobalLock( *hHelpParams ); 

  /* Fill help parameters. */
  fpHelpParams->Size = sizeof( HELPPARAMS );
  fpHelpParams->Message = HELP_QUIT;
  fpHelpParams->Context = 0;
  fpHelpParams->Unknown = 0;
  fpHelpParams->PathOffset = 0;
  fpHelpParams->ExtraDataOffset = 0;
  fpHelpParams->Path[0] = '\0';
          
  /* Unlock the structure's handle. */
  GlobalUnlock( *hHelpParams );

  /* Success. */
  return( TRUE );
}



/*************************************
*
*   Frees the HELPPARAMS data for the HELP_QUIT event.
*
**************************************/
static void DestroyQuitData( GLOBALHANDLE hHelpParams )
{
  GlobalFree( hHelpParams );
}



/*************************************
*
*   Builds the HELPPARAMS data for the HELP_SETWINPOS event.
*
**************************************/
static BOOL CreateSetWinPosData( HWND hErrorWnd, GLOBALHANDLE * hHelpParams, LPCSTR szPath, HELPWININFO * HelpWinInfoInPtr )
{
  FPHELPPARAMS fpHelpParams;
  HELPWININFO * HelpWinInfoPtr;
  

  /* Verify that path data was supplied. */
  if( szPath == NULL ) return FALSE;
  if( _fstrlen( szPath ) == 0 ) return FALSE; 
  
  /* Verify that HELPWININFO data was supplied. */
  if( HelpWinInfoInPtr == NULL ) return FALSE;
  
  /* 
  ** Allocate help parameters structure. 
  **
  ** Null char. of Path string accounted for
  ** by Path[1] in structure.
  **
  ** Null char. for HelpWinInfo->rcchMember
  ** acounted for in HELPWININFO structure.
  */
  if( !GlobalAllocMem(hErrorWnd, hHelpParams, sizeof( HELPPARAMS ) +
		_fstrlen( szPath ) + sizeof(HELPWININFO) +
		_fstrlen( HelpWinInfoInPtr->rgchMember ) ) )
  {
    /* Failure. */
    return FALSE;
  }
  fpHelpParams = (FPHELPPARAMS) GlobalLock( *hHelpParams ); 


  /* Assign HELPPARAMS values. */

  fpHelpParams->Size = sizeof( HELPPARAMS ) + _fstrlen( szPath ) 
                       + sizeof(HELPWININFO) + _fstrlen(HelpWinInfoInPtr->rgchMember);
  fpHelpParams->Message = HELP_SETWINPOS;
  fpHelpParams->Context = 0;
  fpHelpParams->Unknown = 0;
  fpHelpParams->PathOffset = 0x10;
  _fstrcpy( fpHelpParams->Path, szPath ); 
  fpHelpParams->ExtraDataOffset = fpHelpParams->PathOffset + _fstrlen( szPath ) + 1;


  /* Get HELPWININFO buffer. */
  HelpWinInfoPtr = (  HELPWININFO * ) ( (BYTE * ) fpHelpParams + fpHelpParams->ExtraDataOffset );
  
  /* Store values. */
  HelpWinInfoPtr->wStructSize = sizeof(HELPWININFO) + _fstrlen(HelpWinInfoInPtr->rgchMember);
  HelpWinInfoPtr->x = HelpWinInfoInPtr->x;
  HelpWinInfoPtr->y = HelpWinInfoInPtr->y;
  HelpWinInfoPtr->dx = HelpWinInfoInPtr->dx;
  HelpWinInfoPtr->dy = HelpWinInfoInPtr->dy;
  HelpWinInfoPtr->wMax = HelpWinInfoInPtr->wMax;
  _fstrcpy( HelpWinInfoPtr->rgchMember, HelpWinInfoInPtr->rgchMember );
          
  /* Unlock the structure's handle. */
  GlobalUnlock( *hHelpParams );

  /* Success. */
  return( TRUE );
}

static void DestroySetWinPosData( GLOBALHANDLE hHelpParams )
{
  GlobalFree( hHelpParams );
}

/*************************************
*
*  Allocate global memory of a certain size and trap error.
*
**************************************/
static BOOL GlobalAllocMem( HWND hWnd, HGLOBAL * hGlobal, DWORD dwSize  )
{
  /* Allocate memory for menu list. */
  *hGlobal = GlobalAlloc ( GHND | GMEM_SHARE, dwSize ) ;
                              
  /* Error retrieving memory. */
  if ( *hGlobal == (HGLOBAL) NULL ) {
  
    /* If we get down to here then the command is not recognized. */ 
    MessageBox( hWnd, (LPSTR) IDS_MEMORY_ERR, (LPSTR) IDS_HELP_ERROR, MB_ICONHAND | MB_OK );
    return FALSE;
  }

  /* Return it. */
  return TRUE;
}
