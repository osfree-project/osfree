/***************************************************************************************
**
**
**                              TWINVIEW.INI Functions
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
#define BOGUS_DATA  "-----"  

#define INI_FILENAME     "twinview.ini"

#define INI_ASSOC_SECTION  "Associations"

#define INI_GENERAL_SECTION  "TwinView"

#define INI_LOAD_ITEM  "Load"
#define MAX_LOAD_LINE  500

#define CMD_TOKEN_SEP ';'
#define CMD_INTRA_TOKEN_SEP ' '


/*************************************
*
*  This function looks in the 
*  TWINVIEW.INI file for the name of
*  a library that has been associated
*  with szFileName's extension in the
*  [Associations] section. 
*
**************************************/
BOOL __far __pascal GetTVINILibAssoc( char __far * szFileName, char __far * szLibNameBuffer )
{
  char szTVINIFilePath[_MAX_PATH];
  char szFileExt[_MAX_EXT];
  int nReturn;
  
  /* Get TWINVIEW.INI file's path. */
  if( ! GetTVINIFilePath( szTVINIFilePath ) ) return FALSE;

  /* 
  ** Get the extension from the
  ** name of the file.
  */
  if( ! GetFilesExtension( szFileName, szFileExt ) ) return FALSE;

  /* 
  ** Get an association from the [Associations] section. 
  */
  GetPrivateProfileString( (LPSTR) INI_ASSOC_SECTION, (LPSTR) szFileExt, (LPSTR) BOGUS_DATA, szLibNameBuffer, _MAX_FNAME, (LPSTR) szTVINIFilePath );

  /* See if we found something. */
  nReturn = _fstricmp( szLibNameBuffer, BOGUS_DATA );

  /* Found nothing - default "bogus" lib. name returned. */
  if( nReturn == 0 ) return FALSE;
  
  /* Success. */
  return TRUE; 
}


/*************************************
*
*  This function looks in the 
*  TWINVIEW.INI file for the name of
*  a library that has been associated
*  with szFileName's extension in the
*  [Associations] section. 
*
**************************************/
BOOL __far __pascal GetTVINILoadLine( char __far * szBuffer )
{

  char szTVINIFilePath[_MAX_PATH];
  int nReturn;


  /* Get TWINVIEW.INI file's path. */
  if( ! GetTVINIFilePath( szTVINIFilePath ) ) return FALSE;

  /* 
  ** Get an association from the [Associations] section. 
  */
  GetPrivateProfileString( (LPSTR) INI_GENERAL_SECTION, (LPSTR) INI_LOAD_ITEM, (LPSTR) BOGUS_DATA, (LPSTR) szBuffer, MAX_LOAD_LINE, (LPSTR) szTVINIFilePath );

  /* See if we found something. */
  nReturn = _fstricmp( szBuffer, BOGUS_DATA );

  /* Found nothing - default "bogus" name returned or NULL returned. */
  if( nReturn == 0 || *szBuffer == NULL_CHAR ) return FALSE;
  
  /* Success. */
  return TRUE; 
}



/*************************************
*
*  Enumerates the command-line and
*  sends the results to a function
*  that will open a new window for
*  each command-line entry in the line. 
*
**************************************/
void __far __pascal __export ProcessTVCmdLine( HWND hOwnerWnd, LPSTR szCommandLine )
{
  /* Open the TWINVIEW.INI Load= line windows. */
  EnumCmdLine( (TVCMDLINEENUMPROC) OpenCmdLineWindows,
               szCommandLine,
               (LPARAM) (HWND __far * ) &hOwnerWnd );
}


/*************************************
*
*  This function retrieves the load= 
*  line in the TWINVIEW.INI file. 
*
*  It then enumerates the line and
*  sends the results to a function
*  that will open a new window for
*  each command-line entry in the line. 
*
**************************************/
BOOL __far __pascal __export ProcessLoadLine( HWND hOwnerWnd )
{
  char szTVINILoadLine[MAX_LOAD_LINE];
  BOOL bReturn;


  /* Get load= data. */
  bReturn = GetTVINILoadLine( szTVINILoadLine );
  
  /* Found nothing - default "bogus" name returned. */
  if( bReturn == FALSE ) return FALSE;
  
  /* Open the TWINVIEW.INI Load= line windows. */
  EnumCmdLine( (TVCMDLINEENUMPROC) OpenCmdLineWindows,
               (LPSTR) szTVINILoadLine,
               (LPARAM) (HWND __far * ) &hOwnerWnd );

  /* Success. */
  return TRUE;
}


BOOL CALLBACK OpenCmdLineWindows( LPSTR szFilePath, LPSTR szArgs, LPARAM lParam )
{
  TVOPENWNDSTRUCT TVWndOpenStruct; 
  HWND __far * fphWnd;

  
  /* Get lParam data. */
  fphWnd = (HWND __far * ) lParam;
  
  /* Initialize structure. */
  TVWndOpenStruct.hOwner = *fphWnd;
  _fstrcpy( TVWndOpenStruct.szArguments, szArgs );
  _fstrcpy( TVWndOpenStruct.szFilePath, szFilePath );
  TVWndOpenStruct.lParam = 0;

  /* Open the window. */ 
  OpenTVFile( *fphWnd, (FPTVOPENWNDSTRUCT) &TVWndOpenStruct, TV_ALLOWOPENFILE );

  return TRUE;
}




/*************************************
*
*  This function enumerates items
*  in a command-line.
*
*  Format of a command-line is:
*
*  filename fileargs<;filename fileargs>
*
**************************************/
void __far __pascal EnumCmdLine
( 
  TVCMDLINEENUMPROC EnumCmdLineProc, 
  LPSTR szCmdLine,
  LPARAM lParam 
)
{
  char __far * szCommandField;
  char __far * szNextCommandField;
  
  char __far * szArgs;
  
  BOOL bReturn;


  /* Get first command field. */
  szCommandField = szCmdLine;
  GetNextToken( &szCommandField, &szNextCommandField, CMD_TOKEN_SEP );

  /* For each token - pass it on to callback proc. */
  while( szCommandField != NULL )
  { 
    /* Get place between filename and args. */
    GetNextToken( &szCommandField, &szArgs, CMD_INTRA_TOKEN_SEP );

    /* If no space, then no args. */
    if( szArgs != NULL )
    {
      /* Call call-back proc with vaild args. */
      bReturn = (*EnumCmdLineProc) ( szCommandField, szArgs, lParam );
    }

    /* No args, show state, position. */
    else
    {
      /* Call call-back proc. */
      bReturn = (*EnumCmdLineProc) ( szCommandField, (LPSTR) "\0", lParam );
    }    
    
    /* Stop enumerating? */
    if( bReturn == FALSE ) break;
    
    /* Get next command field. */
    if( szNextCommandField != NULL )
    {
      szCommandField = szNextCommandField;
      GetNextToken( &szCommandField, &szNextCommandField, CMD_TOKEN_SEP );
    }
    else szCommandField = NULL;
  }
}



/*************************************
*
*  This function looks for a chDelimiter
*  delimter after szCurrentPos position in
*  the string.
*
*  If the delimiter is found then the
*  string is split at the poistion where the
*  delimiter is discovered.
*
*  Blank spaces after the delimiter are 
*  skipped.
*
*  If there is a vaild character after the
*  delimiter (other than a space), the
*  position of the character is saved in
*  szNextPos.  Otherwise szNextPos is assigned
*  a value of NULL;
*
**************************************/
char __far * GetNextToken( char __far ** szCurrentPos, char __far ** szNextPos, char chDelimiter )
{
  char __far * szDelimiter;
  
  /* Look for next delimiter. */
  szDelimiter = _fstrchr( *szCurrentPos, chDelimiter ); 

  /* Found delimiter. */
  if( szDelimiter != NULL )
  {
    /* Get the beginning of the arg. info. */
    *szNextPos = AnsiNext( szDelimiter );

    /* Separate strings. */
    *szDelimiter = NULL_CHAR;

    /* Skip whitespace. */
    while( **szNextPos == ' ' )
    {
      /* Move to next char. */
      *szNextPos = AnsiNext( *szNextPos );
    }

    /* 
    ** If the next token starts with a NULL_CHAR act 
    ** as if there is no next token. 
    */
    if( **szNextPos == NULL_CHAR ) *szNextPos = NULL;
  }

  /* No delimer found - no next token. */
  else *szNextPos = NULL;

  /* Return pointer to beginning of current token. */
  return *szCurrentPos;
}   




/*************************************
*
*  This function makes the complete
*  path name for the TWINVIEW.INI
*  file and puts it in szTVINIFilePath. 
*
**************************************/
BOOL __far __pascal GetTVINIFilePath( char __far * szTVINIFilePath )
{
  char __far * szLastSlash;
  
  /* Get complete path for DLL. */
  GetModuleFileName( hDLLInst, szTVINIFilePath, _MAX_PATH );
  
  /* If didn't get a path for DLL. */
  if( szTVINIFilePath[0] == NULL_CHAR ) return FALSE;
   
  /* Get the last backslash in path. */
  szLastSlash = _fstrrchr( szTVINIFilePath, DIR_CHAR_SEPARATOR ); 

  /* If didn't get last backslash in path. */
  if( szLastSlash == NULL ) return FALSE;

  /* Go to char. after backslash. */
  szLastSlash++;

  /* Chop off DLL's name from path. */
  *szLastSlash = NULL_CHAR;

  /* Add on the name of the INI file. */
  _fstrcat( szTVINIFilePath, INI_FILENAME );

  /* Success. */
  return TRUE;
}


