/***************************************************************************************
**
**
**                              Open Window Functions
**
**
***************************************************************************************/

/*************************************
*
*  System Includes
*
**************************************/
#include <windows.h>
#include <commdlg.h>
#include <string.h>
#include <direct.h>


/*************************************
*
*  Custom Includes
*
**************************************/
#include "tvlibprv.h"



/*************************************
*
* Private Defines.
*
**************************************/
#define OPER_NOTPROCESSED  0       /* Operation not processed. */
#define OPER_PROCESSED     1       /* Operation processed. */
#define OPER_ERROR        -1       /* Operation not processed due to error. */
#define OPER_CANCEL       -2       /* Operation not processed due to cancellation. */

#define TVLIB_EXT ".TVL"           /* TV library extension to add to a filename. */ 


/*************************************
*
* Private data to send to a EnumLibInfo() callback 
* function from the OpenTVFile() function.
* The data structure is sent via the lParam parameter.
*
**************************************/
typedef struct tagOPENWNDPRIVATEDATA
{
  HWND hWnd;
  FPTVOPENWNDSTRUCT fpTVOpenWndStruct;
  BOOL bAllowOpenFile;
  int nResult;
} 
OPENWNDPRIVATEDATA;
typedef OPENWNDPRIVATEDATA __far * FPOPENWNDPRIVATEDATA;


/*************************************
*
*  Opens a library window.
*
*  Uses two callback functions in attempt
*  to process the fpTVOpenWndStruct->szFilePath
*  as the name of a TwinView library or as a file to load
*  into a TwinView library.
*
**************************************/
BOOL __far __pascal __export OpenTVFile( HWND hWnd, FPTVOPENWNDSTRUCT fpTVOpenWndStruct, BOOL bAllowOpenFile )
{
  OPENWNDPRIVATEDATA OpenWndPrivateData;

  char szLibPath[_MAX_FNAME];
  char szExt[_MAX_EXT];
  char szTmpArgs[MAX_ARG_LENGTH];

  char szBuffer[200];
  
  
  /* Initialize private data. */
  OpenWndPrivateData.hWnd = hWnd;
  OpenWndPrivateData.fpTVOpenWndStruct = fpTVOpenWndStruct;
  OpenWndPrivateData.nResult = OPER_NOTPROCESSED;
  OpenWndPrivateData.bAllowOpenFile = bAllowOpenFile;

  /* If no extension - assume its a library and add a TVL on the end of it. */
  if( ! GetFilesExtension( fpTVOpenWndStruct->szFilePath, szExt ) )
  {
    _fstrcat( fpTVOpenWndStruct->szFilePath, TVLIB_EXT );
  }

  /* 
  **  Try to process the fpTVOpenWndStruct->szFilePath as if it were a library
  **  to open. 
  */                                           

  EnumTVLibInfo( OpenFileAsLibrary, (LPARAM) (FPOPENWNDPRIVATEDATA) &OpenWndPrivateData );

  /* If filename was a library's filename - finished. */
  if( OpenWndPrivateData.nResult == OPER_PROCESSED )  return TRUE;

  /* Error - Abort. */
  if( OpenWndPrivateData.nResult == OPER_ERROR )  return FALSE;

  /* Canceled - Abort. */
  if( OpenWndPrivateData.nResult == OPER_CANCEL )  return FALSE;


  /* 
  **  Try to process the fpTVOpenWndStruct->szFilePath as if it were a file to 
  **  view in a library:                                           
  **
  **    A - First try to find and use the file name's associated library.
  **
  **    B - If that fails, go though every library and use the first one that
  **        can process the extension.
  */

  
  /*************** PHASE A *************/

  /* 
  ** Look in [Associations] section for a library supporting 
  ** the file's extension. 
  */
  if( GetTVINILibAssoc( fpTVOpenWndStruct->szFilePath, szLibPath ) )
  {
    /* 
    ** Got a default library. 
    */
    
    /* Move szFileNameszFilePathto the front of fpTVOpenWndStruct->szArguments. */
    _fstrcpy( szTmpArgs, fpTVOpenWndStruct->szFilePath );
    _fstrcat( szTmpArgs, " " );
    _fstrcat( szTmpArgs, fpTVOpenWndStruct->szArguments );
     
    /* Move szLibPath to fpTVOpenWndStruct->szFilePath. */
    _fstrcpy( fpTVOpenWndStruct->szFilePath, szLibPath );

    /* Assign new arguments string. */
    _fstrcpy( fpTVOpenWndStruct->szArguments, szTmpArgs );

    /* Try to process the fpTVOpenWndStruct->szFilePath as if it were a library to open. */
    EnumTVLibInfo( OpenFileAsLibrary, 
                   (LPARAM) (FPOPENWNDPRIVATEDATA) &OpenWndPrivateData );
  
    /* If filename was a library's filename - finished. */
    if( OpenWndPrivateData.nResult == OPER_PROCESSED )  return TRUE;
  
    /* Error - Abort. */
    if( OpenWndPrivateData.nResult == OPER_ERROR )  return FALSE;
  
    /* Canceled - Abort. */
    if( OpenWndPrivateData.nResult == OPER_CANCEL )  return FALSE;
  }
  
  
  /*************** PHASE B *************/

  /* Try to process the fpTVOpenWndStruct->szFilePath as if it were a library to open. */
  EnumTVLibInfo( OpenFileAsInputFile, 
                 (LPARAM) (FPOPENWNDPRIVATEDATA) &OpenWndPrivateData );

  /* If filename was a library's filename - finished. */
  if( OpenWndPrivateData.nResult == OPER_PROCESSED )  return TRUE;


  /* 
  ** Otherwise an error occured or the file could not be processed. 
  ** Report problem to hWnd and return failure. 
  */
  wsprintf( szBuffer, 
            (LPSTR)"Library Open Operation Failed!\n\nDo not know which viewer to use for file %s.", 
            (LPSTR) _fstrupr(fpTVOpenWndStruct->szFilePath));
  MessageBox( hWnd, (LPSTR)szBuffer, (LPSTR)"Window Functions", MB_ICONHAND | MB_OK );
  return FALSE;

}




/*************************************
*
*  This function will try to find a
*  library to display the file listed
*  in the TVLIBOPENWNDSTRUCT->szFilePath.
*  
*  Return in fpOpenWndPrivateData->nResult:
*    OPER_NOTPROCESSED = Not Processed - No error.
*    OPER_PROCESSED    = Processed.
*    OPER_ERROR        = Not Processed - Error.
*
**************************************/
BOOL __far __pascal OpenFileAsInputFile( FPTVLIBINFO fpTVLibInfo, LPARAM lParam )
{
  FPOPENWNDPRIVATEDATA fpOpenWndPrivateData;
  char szTmpArgs[MAX_ARG_LENGTH];

  
  /* Get lParam data. */
  fpOpenWndPrivateData = (FPOPENWNDPRIVATEDATA) lParam;
  
  /* See if the library support the given filename's extension! */
  if( ! DoesLibSupportExt( fpTVLibInfo, 
                           fpOpenWndPrivateData->fpTVOpenWndStruct->szFilePath ) ) 
  {
    /* Get next lib. info. - keep searching...*/
    return TRUE; 
  }

  /* Move szFilePath to the front of fpTVOpenWndStruct->szArguments. */
  _fstrcpy( szTmpArgs, fpOpenWndPrivateData->fpTVOpenWndStruct->szFilePath );
  _fstrcat( szTmpArgs, " " );
  _fstrcat( szTmpArgs, fpOpenWndPrivateData->fpTVOpenWndStruct->szArguments );
     
  /* Move szLibPath to fpTVOpenWndStruct->szFilePath. */
  _fstrcpy( fpOpenWndPrivateData->fpTVOpenWndStruct->szFilePath, fpTVLibInfo->szLibFilePath );
 
  /* Assign new arguments string. */
  _fstrcpy( fpOpenWndPrivateData->fpTVOpenWndStruct->szArguments, szTmpArgs );

  /* 
  ** Open the library, call its OpenWindow() function, 
  ** and put new window's info in window list. 
  */
  if( ! OpenWindow( fpOpenWndPrivateData->hWnd, fpOpenWndPrivateData->fpTVOpenWndStruct, 
                           fpTVLibInfo ) ) 
  {
    /* Set the flag so that we'll know that we had an error. */
    fpOpenWndPrivateData->nResult = OPER_ERROR;
  }
  
  else
  {
    /* Set the flag so that we'll know that we processed the filename. */
    fpOpenWndPrivateData->nResult = OPER_PROCESSED;
  }

  /* Stop getting more library info. */
  return FALSE;
}




/*************************************
*
*  If a library is passed into this
*  function that matches the name
*  in the TVLIBOPENWNDSTRUCT->szFilePath
*  field - open the library.
*
*  Return in fpOpenWndPrivateData->nResult:
*    OPER_PROCESSED    = Processed.
*    OPER_NOTPROCESSED = Not Processed - No error.
*    OPER_ERROR        = Not Processed - Error.
*    OPER_CANCEL       = Not Processed - Canceled.
*
**************************************/
BOOL __far __pascal OpenFileAsLibrary( FPTVLIBINFO fpTVLibInfo, LPARAM lParam )
{
  FPOPENWNDPRIVATEDATA fpOpenWndPrivateData;
  FPTVLIBFILEINFO fpLibFileInfo;
  char szOpenFileName[ _MAX_FNAME ];


  /* Get lParam data. */
  fpOpenWndPrivateData = (FPOPENWNDPRIVATEDATA) lParam;
  
  /* The name of the library and the given filename are not the same - next! */
  if( ! DoFileNamesMatch( fpTVLibInfo->szLibFilePath, 
                          fpOpenWndPrivateData->fpTVOpenWndStruct->szFilePath ) ) 
  {
    /* Get next lib. info. - keep searching...*/
    return TRUE; 
  }

  /* If no filename or other args. to pass to the library. */
  if( fpOpenWndPrivateData->fpTVOpenWndStruct->szArguments[0] == NULL_CHAR )
  {
    /* Lock the lib. file info. */
    fpLibFileInfo = (FPTVLIBFILEINFO) GlobalLock( fpTVLibInfo->hTVLibFileInfo ); 
    
    /* Does the library need a filename that we should supply? */
    if( fpLibFileInfo->bAskForFile == TRUE && fpOpenWndPrivateData->bAllowOpenFile == TRUE )
    {
      /* Get filename to open. */
      if( !GetInputFileName( fpOpenWndPrivateData->hWnd, szOpenFileName, fpLibFileInfo ) ) 
      {
        /* 
        ** Cancellation during GetInputFileName(). 
        */
        
        /* Unlock fileinfo. */
        GlobalUnlock( fpTVLibInfo->hTVLibFileInfo ); 

        /* Set the flag so that we'll know that we cancelled the operation. */
        fpOpenWndPrivateData->nResult = OPER_CANCEL;

        /* Stop getting more library info. */
        return FALSE;
      }

      /* Copy the filename into the arguments. */
      _fstrcpy( fpOpenWndPrivateData->fpTVOpenWndStruct->szArguments, szOpenFileName );
    }     
    
    /* Unlock the lib. file info. */
    GlobalUnlock( fpTVLibInfo->hTVLibFileInfo ); 
  }

 
  /* 
  ** Open the library, call its OpenWindow() function, 
  ** and put new window's info in window list. 
  */
  if( ! OpenWindow( fpOpenWndPrivateData->hWnd, 
                    fpOpenWndPrivateData->fpTVOpenWndStruct, 
                    fpTVLibInfo ) ) 
  {
    /* Set the flag so that we'll know that we had an error. */
    fpOpenWndPrivateData->nResult = OPER_ERROR;
  }
  
  else
  {
    /* Set the flag so that we'll know that we processed the filename. */
    fpOpenWndPrivateData->nResult = OPER_PROCESSED;
  }

  /* Stop getting more library info. */
  return FALSE;

}



/*************************************
*
*  Let the user select a file to open
*  for a given TwinView library.
*
*  szOpenFileName should be of size
*  _MAX_PATH.
*                  
**************************************/
BOOL __far __pascal GetInputFileName( HWND hWnd, char __far * szOpenFileName, FPTVLIBFILEINFO fpLibFileInfo )
{
  OPENFILENAME ofn;
  char szFilter[MAX_FILTER];
  char szDirName[_MAX_PATH];
  char szFileTitle[_MAX_PATH];
  UINT  i, cbString;
  char  chReplace;    /* string separator for szFilter */

  
  /* Get the current directory and store in szDirName */
  if( _getcwd( szDirName, sizeof(szDirName) ) == NULL  ) return FALSE;

  /* Initialize the filename. */
  szOpenFileName[0] = NULL_CHAR;
  
  /* Setup filter info. */
  szFilter[0] = NULL_CHAR;

  /* If the library's file has filter info. */
  if( fpLibFileInfo->szFilter[0] != NULL_CHAR )
  {
    /* Copy the lib. file's filter. */
    _fstrcpy( szFilter, fpLibFileInfo->szFilter );

    /* Get filter's length. */
    cbString = _fstrlen( szFilter); 
  
    /* Retrieve separator. */
    chReplace = szFilter[cbString - 1]; 
  
    /* Replace separator with NULL character. */
    for (i = 0; szFilter[i] != NULL_CHAR; i++)
    {
      if( szFilter[i] == chReplace ) szFilter[i] = NULL_CHAR;
    }
  }
  else  
  {
    /* Clear filter info. */
    szFilter[0] = NULL_CHAR;
  }


  /* Set all structure members to zero. */
  memset(&ofn, 0, sizeof(OPENFILENAME));
  
  /* Set structure with values. */
  ofn.lStructSize = sizeof(OPENFILENAME);
  ofn.hwndOwner = hWnd;
  ofn.lpstrFilter = szFilter;
  ofn.nFilterIndex = 1;
  ofn.lpstrFile= szOpenFileName;
  ofn.nMaxFile = _MAX_PATH;
  ofn.lpstrFileTitle = szFileTitle;
  ofn.nMaxFileTitle = sizeof(szFileTitle);
  ofn.lpstrInitialDir = szDirName;
  ofn.Flags = OFN_SHOWHELP | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
  
  /* Open dialog box and check for error. */
  if( !GetOpenFileName( &ofn ) ) return FALSE;
 
  /* Success. */
  return TRUE;
}


/*************************************
*
*  1. Opens the library if necessary,
*  2. Call its OpenWindow() function, 
*  3. Substitutes windows WndProc() for our
*     custom WndProc.
*  4. Add window's info to window list.
*
**************************************/
BOOL __far __pascal OpenWindow( HWND hWnd, FPTVOPENWNDSTRUCT fpTVOpenWndStruct, FPTVLIBINFO fpTVLibInfo )
{
  HWND hOpenWnd;
  TVLIBOPENWINDOWPROC fpfnTVLibOpenWindow;
  TVLIBCLOSEWINDOWPROC fpfnTVLibCloseWindow;
    
  
  /* Open library. */
  if( ! OpenLibrary( hWnd, fpTVLibInfo ) )
  {
    /* Failure. */
    return FALSE;
  }

  /* Retrieve the address of the control's TVLibOpenWindow() function. */
  (FARPROC) fpfnTVLibOpenWindow = GetProcAddress( fpTVLibInfo->hInstance, MAKEINTRESOURCE(TVLIB_OPENWINDOW_ORD) );
                                            
  /* Verify the GetProcAddress() function returned data. */
  if ( fpfnTVLibOpenWindow == NULL)
  {
    CloseLibrary( fpTVLibInfo );
    MessageBox(  hWnd, (LPSTR)"GetProcAddress() failed!", (LPSTR)"Library Functions", MB_ICONHAND | MB_OK );
    return FALSE;
  }

  /* Tell the library to open a window. */
  hOpenWnd = (*fpfnTVLibOpenWindow)( fpTVOpenWndStruct );
  
  /* If function could not open a window. */
  if ( hOpenWnd == 0 ) return FALSE;

  /* 
  **  If function returned an existing window handle
  **  just return as if processed.
  */
  if( FindWndInfo( fpTVLibInfo->hTVWndInfo, hOpenWnd ) ) return TRUE;

  /*  
  **  Initialize the window extra bytes reserved for us.
  **  This include subclassing the window's WndProc().
  */
  InitWndExtraByteData( hOpenWnd );
  

  /* Store new window in list. */
  if( ! SaveWndInfo( hWnd, hOpenWnd, &(fpTVLibInfo)->hTVWndInfo ) )
  {
    /* 
    ** Error storing window information in Window List.
    ** Tell library to close window. 
    */
    
    /* Retrieve the address of the control's TVLibWindowClose() function. */
    (FARPROC) fpfnTVLibCloseWindow = GetProcAddress( fpTVLibInfo->hInstance, MAKEINTRESOURCE(TVLIB_CLOSEWINDOW_ORD) );
                                              
    /* Verify the GetProcAddress() function returned data. */
    if ( fpfnTVLibCloseWindow == NULL)
    {
      CloseLibrary( fpTVLibInfo );
      MessageBox(  hWnd, (LPSTR)"GetProcAddress() failed!", (LPSTR)"Library Functions", MB_ICONHAND | MB_OK );
      return FALSE;
    }

    /* Tell the window to close. */
    (*fpfnTVLibCloseWindow)( hOpenWnd );
    
    /* Whether or not the window is closed, free the library. */
    CloseLibrary( fpTVLibInfo );

    /* Failure. */
    return TRUE;
  }

  /* Success. */
  return TRUE;
}


void __far __pascal InitWndExtraByteData( HWND hWnd )
{
  /* Subclass child's WndProc(). */
  SetChildWindowFilter( hWnd );
  
  /* Initialize the accelerator handle to 0. */
  RegisterTVAccel( hWnd, 0 );

  /* Initialize the entry count to 0. */
  SetTVEntryCount( hWnd, 0 );
}
