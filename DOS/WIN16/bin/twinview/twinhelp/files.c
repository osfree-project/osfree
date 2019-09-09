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
**                              General File Functions
**
**
***************************************************************************************/

/*************************************
*
*  System Includes
*
**************************************/
#include <dos.h>
#include <direct.h>                                      
#include <string.h>
#include <stdlib.h>


/***********************************
**
**  App. Includes
**
***********************************/
#include "files.h"


/*************************************
*
*  Private Defines
*
**************************************/

/* EnumFiles() callback function. */
typedef BOOL ( CALLBACK * FILESENUMPROC )( char __far * szFilePath, LPARAM lParam );

/* Directory separator token. */
#define DIR_STRING_SEPARATOR "\\"
#define DIR_CHAR_SEPARATOR   '\\'
                                            
#define AUTOEXEC_PATH_DELIMITER  ';'
#define EXTENSION_CHAR_SEPARATOR '.'


/*************************************
*
*  Private Functions
*
**************************************/
static BOOL __far __pascal EnumFilesProc( char __far * szFilePath, LPARAM lParam );
static BOOL __far __pascal EnumFiles( char __far * szWildName, FILESENUMPROC EnumFilesProc, LPARAM lParam, BOOL Yield );
static BOOL __far __pascal FindFile( char __far * szDir, char __far * szWildName, struct _find_t __far * FileInfo );



/***************************************************
*
* If a path is given, just use it.
*
* If a path is not given, locate the file
* in the current directory, WINDOWS, WINDOWS\SYSTEM,
* and PATH= directories.
*
* Either way, copy the file's path to the szPathBuffer.
*
****************************************************/
BOOL __far __pascal MakeHelpFilePath( char __far * szHelpFileName, char __far * szPathBuffer )
{
  char __far * szPathSep;      /* A path separator token ( e.g. '\' ). */
  
  
  /*
  ** First, we'll see if the file's path
  ** is a path.  If it contains a path
  ** separator token ( e.g. '\') then we
  ** consider it a path.
  */
  
  /* See the path separator in path. */
  szPathSep = _fstrrchr( szHelpFileName, DIR_CHAR_SEPARATOR ); 

  /* Found a separator in path. */
  if( szPathSep != NULL )
  {
    /* Copy the path into the buffer. */
    _fstrcpy( szPathBuffer, szHelpFileName );
  
    /* Success. */
    return( TRUE );
  }
  
  
  /*
  ** Next, we'll see if we can the specified file.
  ** If we find it, we'll copy its name into the
  ** buffer that was passed in.
  */
  
  /* Clear the szPathBuffer. */
  *szPathBuffer = '\0';
  
  /* Try to find the file and get its full path. */
  EnumFiles( szHelpFileName, EnumFilesProc, (LPARAM) szPathBuffer, FALSE );

  /* Didn't find file. */
  if( *szPathBuffer == '\0' ) return FALSE;


  /* Success. */
  return( TRUE );
}


/*************************************
*
*  EnumFiles() callback function
*  we have defined to receive a 
*  found file's full path.
*
**************************************/
static BOOL __far __pascal EnumFilesProc( char __far * szFilePath, LPARAM lParam )
{
  char __far * szSavePath;


  /* Get lParam data structure. */
  szSavePath = (char __far *) lParam;
  
  /* Save full path. */
  _fstrcpy( szSavePath, szFilePath );

  /* Stop enumerating. */
  return FALSE;
}


/*************************************
*
*  Search for a file in the following places:
*  - Current dir.
*  - /WINDOWS 
*  - /WINDOWS/SYSTEM
*  - All directories listed in the system's $PATH$ variable.
*
*  Call EnumFilesProc() if a matching file is found.
*
**************************************/
static BOOL __far __pascal EnumFiles( char __far * szWildName, FILESENUMPROC EnumFilesProc, LPARAM lParam, BOOL Yield )
{
  char szWinDir[_MAX_PATH];
  char szWinSysDir[_MAX_PATH];
  char szCurrentDir[_MAX_PATH];

  char szDir[_MAX_PATH];
  char szFilePath[_MAX_PATH];
  char szPathCopyBuffer[_MAX_PATH];

  char __far * szPathCopy;
  char __far * szPath;
  char __far * szNextPath;

  struct _find_t FileInfo;

  int nPhase, nResult;

  BOOL bCallBack;
  
  MSG Msg;              // Storage for messages pulled from windows message
                        // queue.   
                        

  /* Each phase equates to a different search path. */
    
  for( nPhase = 1; nPhase < 5; nPhase ++ )
  {
    /* If we should let other applications execute. */
    if( Yield )
    {
      while ( PeekMessage( &Msg, NULL, 0, 0, PM_REMOVE ) )
      {
        TranslateMessage( &Msg );
        DispatchMessage( &Msg );
      }
    }
    
    switch( nPhase )
    {
      /* PHASE 1: use current directory.  */
      case 1: 
        getcwd( szDir, _MAX_PATH );
        _fstrcpy( szCurrentDir, szDir );
        break;

      /* PHASE 2: use windows directory */
      case 2: 
        GetWindowsDirectory( szDir, _MAX_PATH);
        _fstrcpy( szWinDir, szDir );
        break;

      /* PHASE 3: use windows\system directory */
      case 3: 
        GetSystemDirectory( szDir, _MAX_PATH);
        _fstrcpy( szWinSysDir, szDir );
        break;
    } 


    /* For Phases #1 - #3. */
    if( nPhase < 4 )
    {
      if( FindFile( szDir, szWildName, &FileInfo ) )
      {
        _fstrcpy( szFilePath, szDir );
        _fstrcat( szFilePath, DIR_STRING_SEPARATOR );
        _fstrcat( szFilePath, FileInfo.name );
        
        /* Call EnumFilesProc() callback function. */
        bCallBack = ( *EnumFilesProc )( szFilePath, lParam ); 
        
        /* Does callback tell us to quit. */
        if( !bCallBack ) return TRUE;
        
        while( FindFile( NULL, NULL, &FileInfo ) )
        {
          _fstrcpy( szFilePath, szDir );
          _fstrcat( szFilePath, DIR_STRING_SEPARATOR );
          _fstrcat( szFilePath, FileInfo.name );
          
          /* Call EnumFilesProc() callback function. */
          bCallBack = ( *EnumFilesProc )( szFilePath, lParam ); 
          
          /* Does callback tell us to quit. */
          if( !bCallBack ) return TRUE;
        } 
      }
    }


    /* For Phase #4 - Use PATH. */
    else
    {
      /* Is there a WIN_PATH set? */
      szPath = getenv("WIN_PATH");
      
      /* If no WIN_PATH set, use standard PATH variable. */
      if( szPath == NULL ) szPath = getenv( "PATH" );
    
      /* If we have some path to search. */
      if( szPath != 0)
      {
        /* Make a copy of the path so that we can change with the copy. */
        lstrcpy( (LPSTR) szPathCopyBuffer, (LPSTR) szPath );
        szPathCopy = szPathCopyBuffer;
        
        /* Walk the path... */
        for( szNextPath = szPathCopy; szNextPath && *szNextPath; szNextPath = szPathCopy) 
        {
          /* If we should let other applications execute. */
          if( Yield )
          {
            while ( PeekMessage( &Msg, NULL, 0, 0, PM_REMOVE ) )
            {
              TranslateMessage( &Msg );
              DispatchMessage( &Msg );
            }
          }
          
          /* Get the next element in the path and terminate it */
          szPathCopy = strchr( szNextPath, AUTOEXEC_PATH_DELIMITER );
          if( szPathCopy )
          {
            *szPathCopy = 0;  /* End the current part. */
            szPathCopy++;     /* Point to the next one. */
          }
    
          /* If it's a directory that we've already visited - skip search. */
          nResult = _fstricmp( szNextPath, szWinDir );   /* \Windows dir. */
          if( nResult == 0 ) continue;
          nResult = _fstricmp( szNextPath, szWinSysDir );   /* \Windows\System dir. */
          if( nResult == 0 ) continue;
          nResult = _fstricmp( szNextPath, szCurrentDir );   /* Original dir. */
          if( nResult == 0 ) continue;

          if( FindFile( szNextPath, szWildName, &FileInfo ) )
          {
            _fstrcpy( szFilePath, szNextPath );
            _fstrcat( szFilePath, DIR_STRING_SEPARATOR );
            _fstrcat( szFilePath, FileInfo.name );

            /* Call EnumFilesProc() callback function. */
            bCallBack = ( *EnumFilesProc )( szFilePath, lParam ); 
            
            /* Does callback tell us to quit. */
            if( !bCallBack ) return TRUE;
        
            while( FindFile( NULL, NULL, &FileInfo ) )
            {
              _fstrcpy( szFilePath, szNextPath );
              _fstrcat( szFilePath, DIR_STRING_SEPARATOR );
              _fstrcat( szFilePath, FileInfo.name );
          
              /* Call EnumFilesProc() callback function. */
              bCallBack = ( *EnumFilesProc )( szFilePath, lParam ); 
              
              /* Does callback tell us to quit. */
              if( !bCallBack ) return TRUE;
            } 
          }
        }
      }
    }
  } /* for loop. */
  
  
  /* Return success. */
  return( TRUE );
}


/*************************************
*
*  Search for a file in the given directory.
*
**************************************/
static BOOL __far __pascal FindFile( char __far * szDir, char __far * szWildName, struct _find_t __far * FileInfo )
{
  char szFullPath[_MAX_PATH];

  /* First time through. */
  if( szDir != NULL )
  {
    /* Build full path for search directory and wild name of file. */
    _fstrcpy( szFullPath, szDir );
    _fstrcat( szFullPath, DIR_STRING_SEPARATOR );
    _fstrcat( szFullPath, szWildName );
      
    if( !_dos_findfirst( szFullPath, _A_NORMAL, FileInfo ) ) return TRUE;
  }

  /* No first time through. */
  else
  {
    if( !_dos_findnext( FileInfo ) ) return TRUE;
  }
  
  /* Failure. */
  return FALSE;
}



