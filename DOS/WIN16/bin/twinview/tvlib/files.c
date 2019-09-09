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
#include <windows.h>
#include <dos.h>
#include <direct.h>
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
#define DIR_STRING_SEPARATOR "\\"
#define DIR_CHAR_SEPARATOR   '\\'
#define AUTOEXEC_PATH_DELIMITER ';'
#define EXTENSION_CHAR_SEPARATOR '.'


/***************************************************************************************
**
**
**                              Library File Functions
**
**
***************************************************************************************/

/*************************************
*
* Private data to send to the EnumFilesProc() callback 
* function.  The data structure is sent to the 
* EnumFilesProc() using the lParam parameter.
*
**************************************/
typedef struct tagENUMTVLIBSTRUCT
{
  HWND hWnd;
  TVLIBFILESENUMPROC EnumTVLibFilesProc;
  LPARAM lParam ;
} 
ENUMLIBFILESPRIVDATA;
typedef ENUMLIBFILESPRIVDATA __far * FPENUMLIBFILESPRIVDATA;


/*************************************
*
*  Search for a library file using the EnumFiles() function.
*
*  Call EnumTVLibFilesProc() if a library file is found.
*
**************************************/
BOOL __far __pascal EnumTVLibFiles
( 
  HWND hWnd, 
  char __far * szLibWildName, 
  TVLIBFILESENUMPROC EnumTVLibFilesProc, 
  LPARAM lParam, 
  BOOL Yield
)
{
  ENUMLIBFILESPRIVDATA EnumLibFilesPrivateData;
  
  /* Store the lParam data to send to the EnumFilesProc() callback function. */
  EnumLibFilesPrivateData.hWnd = hWnd;
  EnumLibFilesPrivateData.EnumTVLibFilesProc = EnumTVLibFilesProc;
  EnumLibFilesPrivateData.lParam = lParam;
  
  /* Locate library files using szLibWildName file spec. */
  if( !EnumFiles( szLibWildName, EnumFilesProc, (LPARAM) (FPENUMLIBFILESPRIVDATA) &EnumLibFilesPrivateData, Yield ) ) return 0;

  /* Success. */
  return TRUE;
}


/*************************************
*
*  EnumFiles() callback function
*  we have defined to receive all library files found.
*
*  Gets TVLIBFILEINFO information from the library file and
*  calls the users EnumTVLibFiles() callback function to
*  give it the library's info.
*
**************************************/
BOOL __far __pascal EnumFilesProc( char __far * szLibFilePath, LPARAM lParam )
{
  FPENUMLIBFILESPRIVDATA fpEnumLibFilesPrivateData;
  TVLIBFILESENUMPROC fpfnTVLibFileEnumProc;

  TVLIBFILEINFO TVLibInfo;
  
  TVLIBGETINFOPROC fpfnTVLibGetInfo;

  BOOL bReturn;
  
  HINSTANCE hLibInst;
  
  
  /* Get lParam data structure. */
  fpEnumLibFilesPrivateData = (FPENUMLIBFILESPRIVDATA) lParam;
  
  /* Cast the structure's callback function to a variable. */
  fpfnTVLibFileEnumProc = (TVLIBFILESENUMPROC) fpEnumLibFilesPrivateData->EnumTVLibFilesProc;
   
  /* Load the library. */
  hLibInst = LoadLibrary( szLibFilePath);

  /* Error opening library. */
  if ( hLibInst <= HINSTANCE_ERROR )
  {
    MessageBox( fpEnumLibFilesPrivateData->hWnd, (LPSTR)"LoadLibrary() failed!", (LPSTR)"Library Functions", MB_ICONHAND | MB_OK );
    return FALSE;
  }

  /* Retrieve the address of the control's LibInfo() function. */
  (FARPROC) fpfnTVLibGetInfo = GetProcAddress( hLibInst, MAKEINTRESOURCE(TVLIB_GETINFO_ORD) );
                                            
  /* Verify the GetProcAddress() function returned data. */
  if ( fpfnTVLibGetInfo == NULL)
  {
    FreeLibrary( hLibInst );
    MessageBox(  fpEnumLibFilesPrivateData->hWnd, (LPSTR)"GetProcAddress() failed!", (LPSTR)"Library Functions", MB_ICONHAND | MB_OK );
    return FALSE;
  }

  /* Get the viewer library info. */
  bReturn = (*fpfnTVLibGetInfo)( (FPTVLIBFILEINFO) &TVLibInfo );
  
  /* Free the DLLs library. */
  FreeLibrary( hLibInst );
  
  /* Verify that we got something back. */
  if ( bReturn == FALSE )
  {
    MessageBox( fpEnumLibFilesPrivateData->hWnd, (LPSTR)"LibGetInfo() failed!", (LPSTR)"Library Functions", MB_ICONHAND | MB_OK );
    return FALSE;
  }

  /* Call the EnumTVLibFilesProc() callback function. */
  bReturn = ( *fpfnTVLibFileEnumProc )( szLibFilePath, (FPTVLIBFILEINFO) &TVLibInfo, fpEnumLibFilesPrivateData->lParam );
  
  /* 
  ** If the callback function tell us to stop calling it,
  ** tell the EnumFile() function to stop sending us the
  ** library file names.
  */
  if( bReturn == FALSE ) return FALSE;
  
  /* Tell the EnumFile() function to keep sending us the files. */ 
  return TRUE;
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
BOOL __export __far __pascal EnumFiles( char __far * szWildName, FILESENUMPROC EnumFilesProc, LPARAM lParam, BOOL Yield )
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
                        
  short int LastCharPos;
  
  
  /* Each phase equates to a different search path. */
    
  for( nPhase = 1; nPhase < 5; nPhase ++ )
  {
    /* If we should let other applications execute. */
    if( Yield )
    {
      while ( PeekMessage( &Msg, NULL, NULL, NULL, PM_REMOVE ) )
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
        /* Start with file's path. */
        _fstrcpy( szFilePath, szDir );
        
        /* Is there already a dir. separator? */
        LastCharPos = ( _fstrlen( szFilePath ) / sizeof(char) ) - 1;
        if( szFilePath[LastCharPos] != DIR_CHAR_SEPARATOR )
        {
          _fstrcat( szFilePath, DIR_STRING_SEPARATOR );
        }
  
        /* Use filename. */
        _fstrcat( szFilePath, FileInfo.name );
        
        /* Call EnumFilesProc() callback function. */
        bCallBack = ( *EnumFilesProc )( szFilePath, lParam ); 
        
        /* Does callback tell us to quit. */
        if( !bCallBack ) return TRUE;
        
        /* Any more file? */
        while( FindFile( NULL, NULL, &FileInfo ) )
        {
          /* Start with file's path. */
          _fstrcpy( szFilePath, szDir );
          
          /* Is there already a dir. separator? */
          LastCharPos = ( _fstrlen( szFilePath ) / sizeof(char) ) - 1;
          if( szFilePath[LastCharPos] != DIR_CHAR_SEPARATOR )
          {
            _fstrcat( szFilePath, DIR_STRING_SEPARATOR );
          }
    
          /* Use filename. */
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
            while ( PeekMessage( &Msg, NULL, NULL, NULL, PM_REMOVE ) )
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
            /* Start with file's path. */
            _fstrcpy( szFilePath, szNextPath );
            
            /* Is there already a dir. separator? */
            LastCharPos = ( _fstrlen( szFilePath ) / sizeof(char) ) - 1;
            if( szFilePath[LastCharPos] != DIR_CHAR_SEPARATOR )
            {
              _fstrcat( szFilePath, DIR_STRING_SEPARATOR );
            }
      
            /* Use filename. */
            _fstrcat( szFilePath, FileInfo.name );
        
            /* Call EnumFilesProc() callback function. */
            bCallBack = ( *EnumFilesProc )( szFilePath, lParam ); 
            
            /* Does callback tell us to quit. */
            if( !bCallBack ) return TRUE;
        
            while( FindFile( NULL, NULL, &FileInfo ) )
            {
              /* Start with file's path. */
              _fstrcpy( szFilePath, szNextPath );
              
              /* Is there already a dir. separator? */
              LastCharPos = ( _fstrlen( szFilePath ) / sizeof(char) ) - 1;
              if( szFilePath[LastCharPos] != DIR_CHAR_SEPARATOR )
              {
                _fstrcat( szFilePath, DIR_STRING_SEPARATOR );
              }
        
              /* Use filename. */
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
BOOL __far __pascal FindFile( char __far * szDir, char __far * szWildName, struct _find_t __far * FileInfo )
{
  char szFilesPath[_MAX_PATH];
  short int LastCharPos;

  /* First time through. */
  if( szDir != NULL )
  {
    /* Build full path for search directory and wild name of file. */
    _fstrcpy( szFilesPath, szDir );

    /* Is there already a dir. separator? */
    LastCharPos = ( _fstrlen( szFilesPath ) / sizeof(char) ) - 1;
    if( szFilesPath[LastCharPos] != DIR_CHAR_SEPARATOR )
    {
      _fstrcat( szFilesPath, DIR_STRING_SEPARATOR );
    }

    _fstrcat( szFilesPath, szWildName );
      
    if( !_dos_findfirst( szFilesPath, _A_NORMAL, FileInfo ) ) return TRUE;
  }

  /* No first time through. */
  else
  {
    if( !_dos_findnext( FileInfo ) ) return TRUE;
  }
  
  /* Failure. */
  return FALSE;
}


/*************************************
*
*  Given a file's full path
*  and a simple file name, see if
*  the filenames match.
*
**************************************/
BOOL __far __pascal DoFileNamesMatch( char __far * szLibFilePath, char __far * szFilePath )
{
  char __far * szSeparator;
  char __far * szLibFileName;
  int nReturn;
  
  
  /* Is the supplied szFilePath is a full path. */
  szSeparator = _fstrrchr( szFilePath, DIR_CHAR_SEPARATOR );
  
  /* If we found a DIR_CHAR_SEPARATOR, then compare as full paths. */
  if( szSeparator != NULL )
  {
    /* Compare the full file paths. */
    nReturn = _fstricmp( szLibFilePath, szFilePath ); 

    if( nReturn == 0 ) return TRUE;
    else return FALSE;
  }
  

  /* 
  ** The supplied szFilePath is not a full path, compare it
  ** to the libraries filename.
  */
  else
  {
    /* Point to the filename in the library's full path. */
    szLibFileName = _fstrrchr( szLibFilePath, DIR_CHAR_SEPARATOR );
    
    /* Could not find the last occurence of the DIR_CHAR_SEPARATOR. */ 
    if( szLibFileName == NULL )  return FALSE;
  
    /* Point to the next char - the lib's filename. */
    szLibFileName++;
    
    /* Compare the file names. */
    nReturn = _fstricmp( szLibFileName, szFilePath ); 
  }

  /* We did not find the library we are looking for. */
  if( nReturn != 0 )  return FALSE;
  
  /* Found match. */
  return( TRUE );
}



BOOL __far __pascal GetFilesExtension( char __far * szFilePath, char __far * szExtBuffer )
{
  char __far * szFileExt;
  
  
  /*
  ** Get the szFilePath's extension. 
  */

  /* Get the extension separator in path. */
  szFileExt = _fstrrchr( szFilePath, EXTENSION_CHAR_SEPARATOR ); 

  /* If didn't find the separator in path. */
  if( szFileExt == NULL ) return FALSE;

  /* Move to the character after the extension character. */
  szFileExt++;
  
  /* ** If we are at the end of the name string - no extension. */ 
  if( *szFileExt == NULL_CHAR ) return FALSE;

  /* Copy the extension into the buffer. */
  _fstrcpy( szExtBuffer, szFileExt );

  return TRUE;
}
