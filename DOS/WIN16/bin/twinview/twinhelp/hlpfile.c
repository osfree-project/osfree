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
/*******************************************************************************************
**
**                                   .HLP FILE INFO ROUTINES
**
********************************************************************************************/

/***********************************
**
**  System Includes
**
***********************************/
#include <io.h>
#include <string.h>


/***********************************
**
**  App. Includes
**
***********************************/
#include "hlpfile.h"
#include "winmem.h"
#include "msgbox.h"
#include "globals.h"
#include "twhlprc.h"
#include "wnddata.h"
#include "elements.h"
#include "wndpos.h"
#include "inifile.h"


/***********************************
**
**  Private Defines
**
***********************************/

/* Position in the TOPIC block buffer. */
typedef enum
{
  INIT_POS,
  FIRST_TOPICLINK,
  LAST_TOPICHDR
}
BLOCK_POS;


/***********************************
**
**  Private functions
**
***********************************/

static RETVAL __far __pascal AllocHelpFileInfo( HWND hErrorWnd, HGLOBAL __far * hGlobal );
static void __far __pascal ClearHelpFileInfo( HGLOBAL hHelpFileInfo );

static RETVAL __far __pascal LoadHelpHeader( HWND hErrorWnd, HFILE hHelpFile, FPHELPHEADER fpHelpHeader );

static RETVAL __far __pascal LoadWHIFSInfo( HWND hErrorWnd, HFILE hHelpFile, long BeginPos, FPWHIFSINFO fpWHIFSInfo );
static void __far __pascal FreeWHIFSInfo( FPWHIFSINFO fpWHIFSInfo );

static RETVAL __far __pascal LoadSystemFile
( 
  HWND hErrorWnd, 
  HFILE hHelpFile, 
  long BeginPos, 
  FPSYSTEMINFO fpSystemInfo, 
  char __far * szHelpFilePath 
);
static void __far __pascal FreeSystemInfo( FPSYSTEMINFO fpSystemInfo );
static BOOL __far __pascal FreeSystemVarLengthData( FPSYSTEMREC fpSysRec, LPARAM lParam );
static RETVAL __far __pascal InsertSysRecord( HWND hErrorWnd, FPREADSYSTEMREC fpRecordInfo, HGLOBAL hRecordData, HGLOBAL __far * hRecordBuffer );
static BOOL __far __pascal FreeSystemVarLengthData( FPSYSTEMREC fpSysRec, LPARAM lParam );
static HICON __far __pascal CreateHelpFileIcon( HWND hErrorWnd, HFILE hHelpFile, WORD wSize );
static BOOL __far __pascal GetCompressionStatus( FPSYSTEMINFO fpSystemInfo );
static COMPILERVER __far __pascal GetCompilerVersion( FPSYSTEMINFO fpSystemInfo );

static BOOL CALLBACK GetSecWndNumFileDataProc( FPSYSTEMREC fpSysRec, LPARAM lParam );
static BOOL CALLBACK GetMainSecWndFileDataProc( FPSYSTEMREC fpSysRec, LPARAM lParam );
void __far __pascal ProcessSecWndData
( 
  HWND hWnd, 
  FPSECWINDOW SecWndDataPtr, 
  char __far * szCaption,
  WINDOWPLACEMENT __far * WndPlacePtr,
  COLORREF __far * ScrollColorPtr,
  COLORREF __far * NonScrollColorPtr,
  BOOL  __far * bOnTop
);
                                                         
static RETVAL __far __pascal LoadPhrasesFile( HWND hErrorWnd, HFILE hHelpFile, long BeginPos, FPSYSTEMINFO fpSystemInfo, HGLOBAL __far * hPhrasesInfo );
static void __far __pascal FreePhrasesInfo( HGLOBAL __far * fphPhraseInfo );
                                                         
static RETVAL __far __pascal LoadCTXOMAPFile( HWND hErrorWnd, HFILE hHelpFile, long BeginPos, HGLOBAL __far * hCTXOMAPInfo );
static void __far __pascal FreeCTXOMAPInfo( HGLOBAL __far * fphCTXOMAPInfo );

static RETVAL __far __pascal LoadTTLBTREEFile( HWND hErrorWnd, HFILE hHelpFile, long BeginPos, HGLOBAL __far * hTTLBTREEInfo );
static void __far __pascal FreeTTLBTREEInfo( HGLOBAL __far * hTTLBTREEInfo );

static RETVAL __far __pascal LoadContextFile( HWND hErrorWnd, HFILE hHelpFile, long BeginPos, HGLOBAL __far * hContextInfo );
static void __far __pascal FreeContextInfo( HGLOBAL __far * hContextInfo );

static RETVAL __far __pascal LoadFontFile( HWND hErrorWnd, HFILE hHelpFile, long BeginPos, FPFONTINFO fpFontInfo );
static void __far __pascal FreeFontInfo( FPFONTINFO fpFontInfo );

static RETVAL __far __pascal GotoTopicData
( 
  HWND hErrorWnd,
  HFILE hHelpFile, 
  DWORD __far * dwCurrBlockNum, 
  WORD wInitBlockOffset,
  BYTE __far * fpFileDataBuffer,                                         
  long __far * BufferDataSize,
  long __far * CurrBufferPos,
  long TopicStart,
  BOOL bCompressed,
  long  int LastFilePos,     
  DWORD dwNumBlocksInFile  
);
static RETVAL __far __pascal ReadTopicBytes
(
  HWND hErrorWnd,
  HFILE hHelpFile, 
  DWORD __far * dwCurrBlockNum, 
  BYTE __far * fpFileDataBuffer,                                         
  long __far * BufferDataSize,
  long __far * CurrBufferPos,
  long TopicStart,
  BOOL bCompressed, 
  BYTE __far * fpDestBuffer, 
  long ReadNumBytes,
  BOOL bSaveDataToBuffer,
  long  int LastFilePos,     
  DWORD dwNumBlocksInFile  
); 

static RETVAL __far __pascal ReadTopicLink
(
  HWND hErrorWnd,
  HFILE hHelpFile, 
  DWORD __far * dwCurrBlockNum, 
  BYTE __far * fpFileDataBuffer,                                         
  long __far * BufferDataSize,
  long __far * CurrBufferPos,
  long TopicStart,
  BOOL bCompressed, 
  BYTE __far * fpDestBuffer, 
  long ReadNumBytes,
  long  int LastFilePos,     
  DWORD dwNumBlocksInFile  
);

static RETVAL __far __pascal ReadTopicBlock
(
  HWND hErrorWnd,
  HFILE hHelpFile, 
  DWORD dwCurrBlockNum, 
  BYTE __far * fpFileDataBuffer,                                         
  long __far * BufferDataSize,
  long __far * CurrBufferPos,
  long TopicStart,
  BOOL bCompressed, 
  long  int LastFilePos,     
  DWORD dwNumBlocksInFile,
  BLOCK_POS BlockPos
);

static long __far __pascal Decompress
( 
  HWND hErrorWnd, 
  HFILE hHelpFile, 
  long CompressedSize, 
  long MaxOutBytes, 
  char __far * Buffer, 
  long  int LastFilePos     
); 
static short int __far __pascal BytesToRead( BYTE BitMap );
static short int __far __pascal BitSet( BYTE BitMap, short int Bit );

static void __far __pascal ExtendOffsetToBlock( DWORD dwOffset, DWORD __far * dwBlockNum, WORD __far * wBlockOffset );
static void __far __pascal CharOffsetToBlock( DWORD dwOffset, DWORD __far * dwBlockNum, WORD __far * wBlockOffset );
static DWORD __far _pascal ExtendToChar( DWORD dwExtendOffset );

static RETVAL __far __pascal LoadBTreeFile( HWND hErrorWnd, HFILE hHelpFile, long BeginPos, HGLOBAL __far * hBtreeInfo );
static void __far __pascal FreeBTreeFile( HGLOBAL __far * hBtreeInfo );
static WORD __far __pascal GetLeafPageDWORD
( 
  BYTE __huge * FirstPageLocPtr, 
  BTREEHEADER __huge * BTreeHdrPtr, 
  DWORD dwKey
);
static WORD __far __pascal GetLeafPageSTRING
( 
  BYTE __huge * FirstPageLocPtr, 
  BTREEHEADER __huge * BTreeHdrPtr, 
  char __huge * KeyStringPtr
);
static WORD __far __pascal GetFirstLeafPage
( 
  BYTE __huge * FirstPageLocPtr, 
  BTREEHEADER __huge * BTreeHdrPtr 
);

static RETVAL __far __pascal AllocTopicTitle( HGLOBAL __far * hTopicTitle, char __far * fpTopicTitle );

static void __far __pascal InitKeywordData( HGLOBAL hHelpFileInfo );



/**************************** HELP FILE INFO STRUCTURE ROUTINES ********************************/


/***************************************************
*
* Load a help file's data into the help file info 
* structure. 
*
****************************************************/
RETVAL __far __pascal LoadHelpFileInfo( HWND hErrorWnd, char __far * szFilePath, HGLOBAL __far * hHelpFileInfo )
{
  HFILE         hHelpFile;            /* Handle to the help file. */
  OFSTRUCT      OpenBuffer;
  
  FPHLPFILEINFO fpHelpFileInfo;       /* Pointer to new help file info.      */

  long          FileStart;            /* Beginning position of a WHIFS file. */
                 
  RETVAL        RetVal;               /* Custom return value. */
  
  
  
  /* Open the help file. */
  hHelpFile = OpenFile( szFilePath, &OpenBuffer, OF_READ );
  if( hHelpFile == HFILE_ERROR )
  {
    /* Error message. */
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEOPEN, MB_ICONHAND | MB_OK );
    
    /* Failure. */
    return( ERR_OTHER );
  }

  /* Allocate help file info structure. */
  RetVal = AllocHelpFileInfo( hErrorWnd, hHelpFileInfo );

  /* Error? */  
  if( RetVal != NO_ERROR ) 
  {
    /* Close help file. */
    _lclose( hHelpFile );
  
    return( RetVal );
  }
  
  /* Lock help info. */
  fpHelpFileInfo = (FPHLPFILEINFO) GlobalLock( *hHelpFileInfo );

  /* Save file name. */
  _fstrcpy( fpHelpFileInfo->szFilePath, szFilePath ); 

  /* Initialize variables in the structure. */
  fpHelpFileInfo->hPhrasesInfo = NULL;
  fpHelpFileInfo->hCTXOMAPInfo = NULL;
  fpHelpFileInfo->hTTLBTREEInfo = NULL;
  fpHelpFileInfo->hContextInfo = NULL;

  /* 
  ** Load the file's main header. 
  */ 
  RetVal = LoadHelpHeader( hErrorWnd, hHelpFile, (FPHELPHEADER) &(fpHelpFileInfo)->HelpHeader );
  if( RetVal != NO_ERROR ) 
  {
    /* Failure. */

    /* Close help file. */
    _lclose( hHelpFile );
  
    /* Free help file info. */
    GlobalUnlock( *hHelpFileInfo );
    FreeHelpFileInfo( *hHelpFileInfo );

    return( RetVal );
  }


  /* 
  ** Load the file's WHIFS info. 
  */ 
  RetVal = LoadWHIFSInfo( hErrorWnd, hHelpFile, fpHelpFileInfo->HelpHeader.WHIFS, (FPWHIFSINFO) &(fpHelpFileInfo)->WHIFSInfo );
  if( RetVal != NO_ERROR ) 
  {
    /* Failure. */

    /* Close help file. */
    _lclose( hHelpFile );
  
    /* Free help file info. */
    GlobalUnlock( *hHelpFileInfo );
    FreeHelpFileInfo( *hHelpFileInfo );

    return( RetVal );
  }                                     


  /* 
  ** Load the |SYSTEM file's info. 
  */ 

  /* Get starting position of file. */
  FileStart = GetWHIFSFileStart( (FPWHIFSINFO) &(fpHelpFileInfo)->WHIFSInfo, SYSTEM_FILE );
  if( FileStart == 0 )
  { 
    /* Failure. */

    /* Close help file. */
    _lclose( hHelpFile );
  
    /* Free help file info. */
    GlobalUnlock( *hHelpFileInfo );
    FreeHelpFileInfo( *hHelpFileInfo );

    /* Error message. */
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_FINDINTERNALFILE, MB_ICONHAND | MB_OK );

    return( ERR_OTHER );
  }
  
  /* Load |SYSTEM file info. */
  RetVal = LoadSystemFile( hErrorWnd, hHelpFile, FileStart, (FPSYSTEMINFO) &(fpHelpFileInfo)->SystemInfo, szFilePath );
  if( RetVal != NO_ERROR ) 
  {
    /* Failure. */

    /* Close help file. */
    _lclose( hHelpFile );
  
    /* Free help file info. */
    GlobalUnlock( *hHelpFileInfo );
    FreeHelpFileInfo( *hHelpFileInfo );

    return( RetVal );
  }                                     


  /* 
  ** Load the |Phrases file's info. 
  ** There will be one only when compression is used.
  */ 

  /* Get starting position of file. */
  FileStart = GetWHIFSFileStart( (FPWHIFSINFO) &(fpHelpFileInfo)->WHIFSInfo, PHRASES_FILE );
  if( FileStart != 0 )
  { 
    /* Load |Phrases file info. */
    RetVal = LoadPhrasesFile( hErrorWnd, hHelpFile, FileStart, (FPSYSTEMINFO) &(fpHelpFileInfo)->SystemInfo, (HGLOBAL __far *) &(fpHelpFileInfo)->hPhrasesInfo );
    if( RetVal != NO_ERROR ) 
    {
      /* Failure. */
  
      /* Close help file. */
      _lclose( hHelpFile );
  
      /* Free help file info. */
      GlobalUnlock( *hHelpFileInfo );
      FreeHelpFileInfo( *hHelpFileInfo );

      return( RetVal );
    }                                     
  }


  /* 
  ** Load the |CTXOMAP file's info. 
  **
  ** There will be one only if a [MAP] section existed
  ** in the .HPJ file.
  */ 

  /* Get starting position of file. */
  FileStart = GetWHIFSFileStart( (FPWHIFSINFO) &(fpHelpFileInfo)->WHIFSInfo, CTXOMAP_FILE );
  if( FileStart != 0 )
  { 
    /* Load |Phrases file info. */
    RetVal = LoadCTXOMAPFile( hErrorWnd, hHelpFile, FileStart, (HGLOBAL __far *) &(fpHelpFileInfo)->hCTXOMAPInfo );
    if( RetVal != NO_ERROR ) 
    {
      /* Failure. */

      /* Close help file. */
      _lclose( hHelpFile );
  
      /* Free help file info. */
      GlobalUnlock( *hHelpFileInfo );
      FreeHelpFileInfo( *hHelpFileInfo );

      return( RetVal );
    }                                     
  }

  /* 
  ** Load the |TTLBTREE file's info. 
  */ 

  /* Get starting position of file. */
  FileStart = GetWHIFSFileStart( (FPWHIFSINFO) &(fpHelpFileInfo)->WHIFSInfo, TTLBTREE_FILE );
  if( FileStart != 0 )
  { 
    /* Load |Phrases file info. */
    RetVal = LoadTTLBTREEFile( hErrorWnd, hHelpFile, FileStart, (HGLOBAL __far *) &(fpHelpFileInfo)->hTTLBTREEInfo );
    if( RetVal != NO_ERROR ) 
    {
      /* Failure. */

      /* Close help file. */
      _lclose( hHelpFile );
  
      /* Free help file info. */
      GlobalUnlock( *hHelpFileInfo );
      FreeHelpFileInfo( *hHelpFileInfo );

      return( RetVal );
    }                                     
  }


  /* 
  ** Load the |CONTEXT file's info. 
  */ 

  /* Get starting position of file. */
  FileStart = GetWHIFSFileStart( (FPWHIFSINFO) &(fpHelpFileInfo)->WHIFSInfo, CONTEXT_FILE );
  if( FileStart != 0 )
  { 
    /* Load |CONTEXT file info. */
    RetVal = LoadContextFile( hErrorWnd, hHelpFile, FileStart, (HGLOBAL __far *) &(fpHelpFileInfo)->hContextInfo );
    if( RetVal != NO_ERROR ) 
    {
      /* Failure. */

      /* Close help file. */
      _lclose( hHelpFile );
  
      /* Free help file info. */
      GlobalUnlock( *hHelpFileInfo );
      FreeHelpFileInfo( *hHelpFileInfo );

      return( RetVal );
    }                                     
  }


  /* 
  ** Load the |FONT file's info. 
  */ 

  /* Get starting position of file. */
  FileStart = GetWHIFSFileStart( (FPWHIFSINFO) &(fpHelpFileInfo)->WHIFSInfo, FONT_FILE );
  if( FileStart != 0 )
  { 
    /* Load |FONT file info. */
    RetVal = LoadFontFile( hErrorWnd, hHelpFile, FileStart, &(fpHelpFileInfo)->FontInfo );
    if( RetVal != NO_ERROR ) 
    {
      /* Failure. */

      /* Close help file. */
      _lclose( hHelpFile );
  
      /* Free help file info. */
      GlobalUnlock( *hHelpFileInfo );
      FreeHelpFileInfo( *hHelpFileInfo );

      return( RetVal );
    }                                     
  }


  /* Close help file. */
  _lclose( hHelpFile );

  /* Unlock new help info. */
  GlobalUnlock( *hHelpFileInfo );

  /* Initialize the key word data strucure. */
  InitKeywordData( *hHelpFileInfo );

  /* Success. */
  return( NO_ERROR );
}


/***************************************************
*
* Allocate memory space for help file info struture. 
*
****************************************************/
static RETVAL __far __pascal AllocHelpFileInfo( HWND hErrorWnd, HGLOBAL __far * hGlobal )
{
  /* Allocate memory. */
  *hGlobal =  GlobalAlloc( GHND, sizeof( HLPFILEINFO ) );
  if( *hGlobal == NULL )
  {
    return( ERR_MEMORY );
  }

  /* Success. */
  return( NO_ERROR );
}


/***************************************************
*
* Free memory space of help file info struture. 
*
****************************************************/
void __far __pascal FreeHelpFileInfo( HGLOBAL hHelpFileInfo )
{
  /* Free any dynamically allocated data INSIDE of the structure. */
  ClearHelpFileInfo( hHelpFileInfo );

  /* Free the structure. */
  GlobalFree( hHelpFileInfo );
}



/***************************************************
*
* Clears any state data residing in the help file 
* info struture. 
*
* Frees any dynamically allocated memory in
* the structure.
*
* Closes any open file handles stored in the structure.
*
****************************************************/
static void __far __pascal ClearHelpFileInfo( HGLOBAL hHelpFileInfo )
{
  FPHLPFILEINFO fpHelpFileInfo;           /* Pointer to help file info. */


  /* Lock help info. */
  fpHelpFileInfo = (FPHLPFILEINFO) GlobalLock( hHelpFileInfo );

  /* 
  ** Frees any dynamically allocated memory in the
  ** WHIFSINFO structure.
  */
  FreeWHIFSInfo( (FPWHIFSINFO) &(fpHelpFileInfo)->WHIFSInfo );

  /* 
  ** Frees any dynamically allocated memory in the
  ** SYSTEMINFO structure.
  */
  FreeSystemInfo( (FPSYSTEMINFO) &(fpHelpFileInfo)->SystemInfo );

  /* 
  ** Frees any dynamically allocated memory for the Phrase info.
  */
  FreePhrasesInfo( (HGLOBAL __far *) &(fpHelpFileInfo)->hPhrasesInfo );

  /* 
  ** Frees any dynamically allocated memory for the CTXOMAP info.
  */
  FreeCTXOMAPInfo( (HGLOBAL __far *) &(fpHelpFileInfo)->hCTXOMAPInfo );

  /* 
  ** Frees any dynamically allocated memory for the TTLBTREE info.
  */
  FreeTTLBTREEInfo( (HGLOBAL __far *) &(fpHelpFileInfo)->hTTLBTREEInfo );

  /* 
  ** Frees any dynamically allocated memory for the CONTEXT info.
  */
  FreeContextInfo( (HGLOBAL __far *) &(fpHelpFileInfo)->hContextInfo );

  /* 
  ** Frees any dynamically allocated memory for the FONT info.
  */
  FreeFontInfo( &(fpHelpFileInfo)->FontInfo );


  /* Unlock help info. */
  GlobalUnlock( hHelpFileInfo );
}


/*********************************** .HLP HEADER ROUTINES ************************************/

/***************************************************
*
* Loads the HLP file header into memory. 
*
****************************************************/
static RETVAL __far __pascal LoadHelpHeader( HWND hErrorWnd, HFILE hHelpFile, FPHELPHEADER fpHelpHeader )
{
  /* Go to top of file. */ 
  _lseek( hHelpFile, 0L, SEEK_SET );

  /* Load the file's main header. */ 
  _lread( hHelpFile, fpHelpHeader, sizeof(HELPHEADER) );

  /* Verify that its a help file. */
  if( fpHelpHeader->MagicNumber != HELP_MAGIC ) 
  {
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILETYPE, MB_ICONHAND | MB_OK );
    return( ERR_OTHER );
  }
  
  /* File OK. */
  return( NO_ERROR );
}


/*********************************** WHIFS ROUTINES ************************************/

/***************************************************
*
* Loads the WHIFS file's info into memory. Need this later
* on to find other data in the help file ( e.g. topic
* data ).
*
****************************************************/
static RETVAL __far __pascal LoadWHIFSInfo( HWND hErrorWnd, HFILE hHelpFile, long BeginPos, FPWHIFSINFO fpWHIFSInfo )
{
  BTREEINDEXHEADER  IndexNode;      /* Current index node in B-Tree. */
  short int         CurrLevel;      /* Current level in B-Tree. */
  WORD              NextPage;       /* Next page to go to. */
  long int          FirstPageLoc;   /* First page of WHIFS B-Tree. */

  BTREENODEHEADER   LeafNode;       /* Current data node of WHIFS B-Tree. */
  short int         Counter;        /* Current number of the WHIFS file being processed. */
  short int         Index;          /* Index into array holding name of file. */

  char              szFileName[MAX_WHIF_NAME];     /* Name of the WHIFS file. */
  char              NameChar;                          /* Character in name of file. */
  
  FPWHIFFILEINFO    fpWHIFSFileData; /* Pointer to in-memory storage area of file info. */
  

  /* Go to the beginning of WHIFS B-Tree */
  if( _lseek( hHelpFile, BeginPos, SEEK_SET ) == -1 )
  {
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
    return( ERR_OTHER );
  }

   /* Read the WHIFS header. */
  if( _lread( hHelpFile, &(fpWHIFSInfo)->WHIFSHeader, sizeof(WHIFSHEADER) ) != sizeof(WHIFSHEADER) )
  {
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
    return( ERR_OTHER );
  }

  /* Allocate memory. */
  fpWHIFSInfo->hFileData =  GlobalAlloc( GHND, sizeof( WHIFFILEINFO ) * fpWHIFSInfo->WHIFSHeader.TotalWHIFSEntries );
  if( fpWHIFSInfo->hFileData == NULL )
  {
    return( ERR_MEMORY );
  }
  
  /* Lock data. */
  fpWHIFSFileData = ( FPWHIFFILEINFO ) GlobalLock( fpWHIFSInfo->hFileData );

  /* Save file position for the beginning of the WHIFS section. */
  FirstPageLoc = BeginPos + sizeof(WHIFSHEADER);

  /* Goto first data leaf */
  CurrLevel = 1;
  while ( CurrLevel < fpWHIFSInfo->WHIFSHeader.NLevels ) 
  {
    /* Read the first index node's data. */
    if( _lread( hHelpFile, &IndexNode, sizeof(BTREEINDEXHEADER) != sizeof(BTREEINDEXHEADER) ) )
    {
      MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
      GlobalUnlock( fpWHIFSInfo->hFileData );
      GlobalFree( fpWHIFSInfo->hFileData );
      return( ERR_OTHER );
    }
    
    /* 
    ** Page number of next index node is conveniently the 
    ** first byte of the current page. 
    */
    if( _lread( hHelpFile, &NextPage, sizeof(WORD) != sizeof(WORD) ) )
    {
      MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
      GlobalUnlock( fpWHIFSInfo->hFileData );
      GlobalFree( fpWHIFSInfo->hFileData );
      return( ERR_OTHER );
    }

    /* Goto the next index node's page. */ 
    if( _lseek( hHelpFile, FirstPageLoc + (NextPage * WHIFSPAGESIZE), SEEK_SET ) == -1 )
    {
      MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
      GlobalUnlock( fpWHIFSInfo->hFileData );
      GlobalFree( fpWHIFSInfo->hFileData );
      return( ERR_OTHER );
    }
    
    /* We just went to the next level. */
    CurrLevel++;
  }


  /* Initialize counter. */
  fpWHIFSInfo->dwFileCount = 0;

  /* 
  ** Now positioned at the first leaf node.
  ** Go through the linked list of leaf nodes. 
  */
  while( TRUE ) 
  {
    /* Read data of WHIFS B-Tree leaf. */
    if( _lread( hHelpFile, &LeafNode, sizeof(BTREENODEHEADER) ) != sizeof(BTREENODEHEADER) )
    {
      MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
      GlobalUnlock( fpWHIFSInfo->hFileData );
      GlobalFree( fpWHIFSInfo->hFileData );
      return( ERR_OTHER );
    }

    /* Get all entries in the node. */
    for( Counter = 1; Counter <= LeafNode.NEntries; Counter++ ) 
    {
      /* Set to first position in the name buffer. */
      Index = 0;

      /* Copy in the name - NULL terminated. */
      if( _lread( hHelpFile, &NameChar, sizeof(char) ) != sizeof(char) )
      {
        MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
        GlobalUnlock( fpWHIFSInfo->hFileData );
        GlobalFree( fpWHIFSInfo->hFileData );
        return( ERR_OTHER );
      }

      while( NameChar)
      {
        szFileName[Index++] = NameChar;
        if( _lread( hHelpFile, &NameChar, sizeof(char) ) != sizeof(char) )
        {
          MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
          GlobalUnlock( fpWHIFSInfo->hFileData );
          GlobalFree( fpWHIFSInfo->hFileData );
          return( ERR_OTHER );
        }
      }
      szFileName[Index] = 0;
      
      /* Save file name. */
      _fstrcpy( fpWHIFSFileData->FileName, szFileName );

      /* Read and save file's offset. */     
      if( _lread( hHelpFile, &(fpWHIFSFileData)->FileOffset, sizeof(long) ) != sizeof(long) )
      {
        MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
        GlobalUnlock( fpWHIFSInfo->hFileData );
        GlobalFree( fpWHIFSInfo->hFileData );
        return( ERR_OTHER );
      }

      /* Increment info. pointer to next position. */
      fpWHIFSFileData++;

      /* Increment the file count. */
      fpWHIFSInfo->dwFileCount = fpWHIFSInfo->dwFileCount + 1;
    }
    
    /* Goto next data leaf. */         
    if( LeafNode.NextPage == -1 ) break;

    /* Goto the next index node's page. */ 
    else
    {
      if( _lseek( hHelpFile, FirstPageLoc + (LeafNode.NextPage * WHIFSPAGESIZE), SEEK_SET ) == -1 )
      {
        MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
        GlobalUnlock( fpWHIFSInfo->hFileData );
        GlobalFree( fpWHIFSInfo->hFileData );
        return( ERR_OTHER );
      }
    }
  } 

  /* Unlock WHIFS file info. */
  GlobalUnlock( fpWHIFSInfo->hFileData );

  /* Success. */
  return( NO_ERROR );
}


/***************************************************
*
* Frees any dynamically allocated memory in the
* WHIFSINFO structure.
*
****************************************************/
static void __far __pascal FreeWHIFSInfo( FPWHIFSINFO fpWHIFSInfo )
{
  GlobalFree( fpWHIFSInfo->hFileData );
}


/***************************************************
*
* Returns the beginning file position of a WHIFS
* file. 
*
* 0 is returned on error. 
*
****************************************************/
long __far __pascal GetWHIFSFileStart( FPWHIFSINFO fpWHIFSInfo, char __far * FileName )
{
  FPWHIFFILEINFO    fpWHIFSFileData;  /* Pointer to in-memory storage area of file info. */
  DWORD             dwCounter;        /* Simple counter variable. */
  long              FilePosition;     /* The reqested file's beginning position. */


  /* Lock data. */
  fpWHIFSFileData = ( FPWHIFFILEINFO ) GlobalLock( fpWHIFSInfo->hFileData );

  /* Look for name in the list of files. */
  for( dwCounter = 0; dwCounter < fpWHIFSInfo->dwFileCount; dwCounter++ )
  {
    if( _fstrcmp( fpWHIFSFileData->FileName, FileName ) == 0 )
    {
      /* Found file. */
      FilePosition = fpWHIFSFileData->FileOffset;
      GlobalUnlock( fpWHIFSInfo->hFileData );
      return( FilePosition );
    }
  
    /* Goto next record in the WHIFS info. */
    else
    {
      fpWHIFSFileData++;
    }
  }

  /* Unlock data. */
  GlobalUnlock( fpWHIFSInfo->hFileData );


  /* Didn't find file. */
  return( 0 );
} 



/****************************** |SYSTEM FILE ROUTINES ******************************/


/***************************************************
*
* Loads the |SYSTEM file's information into memory. 
* Need this later on to determine if compression is used
* on help file, default topic number, etc.
*
****************************************************/
static RETVAL __far __pascal LoadSystemFile
( 
  HWND hErrorWnd, 
  HFILE hHelpFile, 
  long BeginPos, 
  FPSYSTEMINFO fpSystemInfo, 
  char __far * szHelpFilePath 
)
{
  FILEHEADER      FileHdr;         /* Generic WHIFS file header.            */
  SYSTEMHEADER    SysHeader;       /* Header for |SYSTEM file.              */
  READSYSTEMREC   SystemRec;       /* A record in the |SYSTEM file.         */

  HGLOBAL         hRecData;        /* Handle to a system record's data.     */
  BYTE __far *    fpRecData;       /* Pointer to a system record's data     */

  long int        DataProcessed;   /* How much data in the |SYSTEM file is left to process. */

  RETVAL          RetVal;          /* Custom return value. */
  
  WORD            wStringSize;     /* Title string length for a 3.0 help file. */
  
  
  /* Go to the beginning file position of the |SYSTEM file. */
  if( _lseek( hHelpFile, BeginPos, SEEK_SET ) == -1 )
  {
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
    return( ERR_OTHER );
  }

  /* Read the generic WHIFS file header that exists for each WHIFS file. */
  if( _lread( hHelpFile, &FileHdr, sizeof(FILEHEADER) ) != sizeof(FILEHEADER) ) 
  {
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
    return( ERR_OTHER );
  }

  /* Read the header information for the |SYSTEM file. */
  if( _lread( hHelpFile, &SysHeader, sizeof(SYSTEMHEADER) ) != sizeof(SYSTEMHEADER) )
  {
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
    return( ERR_OTHER );
  }

  /* Figure out version and revision. */
  switch( SysHeader.Version )
  {
    /* Version 3 compiler. */
    case 3:
    {
      switch( SysHeader.Revision )
      { 
        /* 3.00 compiler. */
        case COMPVER300:
        {
          MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_WRONGCOMPILER, MB_ICONHAND | MB_OK );
          return( ERR_OTHER );
        }
      
        /* 3.10 compiler. */
        case COMPVER310:
        {
          fpSystemInfo->CompilerVer = COMPILER_310;
          break;
        }
      
        /* Unsupported version. */
        default:
        {
          MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_WRONGCOMPILER, MB_ICONHAND | MB_OK );
          return( ERR_OTHER );
        }
      }
      break;
    }
    
    /* Unsupported version. */
    default:
    {
      MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_WRONGCOMPILER, MB_ICONHAND | MB_OK );
      return( ERR_OTHER );
    }
  }
  
  /* If the flag says no compression. */
  if( SysHeader.Flags == NO_COMPRESSION )
  {
    fpSystemInfo->bCompressed = FALSE;
  }
  else
  {
    fpSystemInfo->bCompressed = TRUE;
  }

  /* Initialize the SYSTEMINFO structure's fields. */
  fpSystemInfo->dwContentsTopic = 0;
  _fstrcpy( fpSystemInfo->szContentsFile, szHelpFilePath );
  fpSystemInfo->hTitleText = NULL;
  fpSystemInfo->hIcon = NULL;
  fpSystemInfo->hMacroData = NULL;
  fpSystemInfo->hSecondWndData = NULL;


  /* 
  ** If we are processing a 3.00 help file. 
  */
  if( fpSystemInfo->CompilerVer == COMPILER_300 )
  {
    /* 
    ** There is only the System Header strucure 
    ** and the title string in the SYSTEM file. 
    */
    wStringSize = (WORD) ( FileHdr.FileSize - sizeof(SYSTEMHEADER) );

    /* 
    ** Allocate space for the system record's variable length data. 
    ** 1 is added to size for the NULL character at the end.
    */
    fpSystemInfo->hTitleText =  GlobalAlloc( GHND, wStringSize );
    if( fpSystemInfo->hTitleText == NULL )
    {
      return( ERR_MEMORY );
    }
    
    /* Lock variable length data buffer. */
    fpRecData = ( BYTE __far * ) GlobalLock( fpSystemInfo->hTitleText );
          
    /* Read the system record's variable length data into buffer. */
    if( _lread( hHelpFile, fpRecData, wStringSize ) != wStringSize ) 
    {
      FreeSystemInfo( fpSystemInfo );
      MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
      return( ERR_OTHER );
    }
      
    /* Unlock variable length data buffer. */
    GlobalUnlock( fpSystemInfo->hTitleText );
  }
  

  /* 
  ** If we are processing a 3.10 help file. 
  */
  else if( fpSystemInfo->CompilerVer == COMPILER_310 )
  {
    /* 
    ** Set the current position in the |SYSTEM file's data.
    ** This will be after the header.
    */
    DataProcessed = sizeof( SYSTEMHEADER );
  
    /* Read in the system records until we are at the end of the |SYSTEM file. */
    while( DataProcessed < FileHdr.FileSize ) 
    {
      /* Read in a system record. */
      if( _lread( hHelpFile, &SystemRec, sizeof(READSYSTEMREC) ) != sizeof(READSYSTEMREC) ) 
      {
        FreeSystemInfo( fpSystemInfo );
        MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
        return( ERR_OTHER );
      }
      
      /* Get the record's variable length data. */    
      switch( SystemRec.RecordType ) 
      {
        /* Contents topic. */
        case HPJ_CONTENTS:  
        {
          /* Read the contents data. */
          if( _lread( hHelpFile, &(fpSystemInfo)->dwContentsTopic, SystemRec.DataSize ) != SystemRec.DataSize ) 
          {
            FreeSystemInfo( fpSystemInfo );
            MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
            return( ERR_OTHER );
          }
          break;
        }
        
  
        /* Title text. */
        case HPJ_TITLE:  
        {
          /* 
          ** Allocate space for the system record's variable length data. 
          ** 1 is added to size for the NULL character at the end.
          */
          fpSystemInfo->hTitleText =  GlobalAlloc( GHND, SystemRec.DataSize + 1 );
          if( fpSystemInfo->hTitleText == NULL )
          {
            FreeSystemInfo( fpSystemInfo );
            return( ERR_MEMORY );
          }
    
          /* Lock variable length data buffer. */
          fpRecData = ( BYTE __far * ) GlobalLock( fpSystemInfo->hTitleText );
          
          /* Read the system record's variable length data into buffer. */
          if( _lread( hHelpFile, fpRecData, SystemRec.DataSize ) != SystemRec.DataSize ) 
          {
            FreeSystemInfo( fpSystemInfo );
            MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
            return( ERR_OTHER );
          }
      
          /* Unlock variable length data buffer. */
          GlobalUnlock( fpSystemInfo->hTitleText );
          break;
        }
        
  
        /* Alternate help icon. */
        case HPJ_ICON:
        {  
          /* Make icon. */ 
          fpSystemInfo->hIcon = CreateHelpFileIcon( hErrorWnd, hHelpFile, SystemRec.DataSize );
  
          /* Error? */
          if( fpSystemInfo->hIcon == NULL )
          {
            FreeSystemInfo( fpSystemInfo );
            return( ERR_OTHER );
          }
          break;
        }
        
  
        /* 
        ** The following types of system information can have multiple
        ** entries of data. Save information for these record types into
        ** the appropriate record buffer. 
        */
        case HPJ_SECWINDOW:
        case HPJ_MACRO:
        {
          /* Allocate space for the system record's variable length data. */
          hRecData =  GlobalAlloc( GHND, SystemRec.DataSize );
          if( hRecData == NULL )
          {
            FreeSystemInfo( fpSystemInfo );
            return( ERR_MEMORY );
          }
    
          /* Lock variable length data buffer. */
          fpRecData = ( BYTE __far * ) GlobalLock( hRecData );
          
          /* Read the system record's variable length data into buffer. */
          if( _lread( hHelpFile, fpRecData, SystemRec.DataSize ) != SystemRec.DataSize ) 
          {
            FreeSystemInfo( fpSystemInfo );
            MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
            return( ERR_OTHER );
          }
      
          /* Unlock variable length data buffer. */
          GlobalUnlock( hRecData );
  
          /* 
          ** Save information for these record types into the 
          ** appropriate record buffer. 
          */
          switch( SystemRec.RecordType ) 
          {
            case HPJ_SECWINDOW:
            
              /* Add this records data to our list of records. */
              RetVal = InsertSysRecord( hErrorWnd, &SystemRec, hRecData, &(fpSystemInfo)->hSecondWndData );
              
              /* Insertion error? */
              if( RetVal != NO_ERROR )
              {
                FreeSystemInfo( fpSystemInfo );
                return( RetVal );
              }
              
              break;
            
            case HPJ_MACRO:
  
              /* Add this records data to our list of records. */
              RetVal = InsertSysRecord( hErrorWnd, &SystemRec, hRecData, &(fpSystemInfo)->hMacroData );
  
              /* Insertion error? */
              if( RetVal != NO_ERROR )
              {
                FreeSystemInfo( fpSystemInfo );
                return( RetVal );
              }
  
              break;
          }
  
          break;
        }
  
        
        /* Don't care about this system record type. */
        default:
        {
          /* Skip the record's variable length data. */
          if( _lseek( hHelpFile, SystemRec.DataSize, SEEK_CUR ) == -1 )
          {
            FreeSystemInfo( fpSystemInfo );
            MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
            return( ERR_OTHER );
          }
        }
        
      } /* switch */
  
      /* 
      ** Increment our file position counter to the next system record. 
      */
      DataProcessed = DataProcessed + sizeof(SYSTEMREC) + SystemRec.DataSize;
    } 
  }
  
  /* Success. */
  return( NO_ERROR );
} 


/*
** Window List Structure
*/
typedef struct tagSECWNDENUMDATA
{
  BOOL bFound;
  WORD wCurrWndNum;
  WORD wSecWndNum;
  SECWINDOW SecWndData;
}
SECWNDENUMDATA;
typedef SECWNDENUMDATA __far * FPSECWNDENUMDATA;


/*************************************
*
*  Find the secondary window 
*  information for the specified
*  secondary window number.
*                  
**************************************/
BOOL __far __pascal GetMainSecWndFileData
(  
  HWND hWnd,
  HGLOBAL hHelpFileInfo, 
  BOOL __far * bUseCaptionPtr,
  char __far * szCaption,
  WINDOWPLACEMENT __far * WndPlacePtr,
  COLORREF __far * ScrollColorPtr,
  COLORREF __far * NonScrollColorPtr,
  BOOL  __far * bOnTop
)
{
  SECWNDENUMDATA SecWndEnumData;
  FPHLPFILEINFO fpHelpFileInfo;
  
  
  /* Initialize search data. */
  SecWndEnumData.bFound = FALSE;
  
  /* Lock data. */
  fpHelpFileInfo = (FPHLPFILEINFO) GlobalLock( hHelpFileInfo );
  
  /* Enumerate the secondary window system records. */
  EnumSysRecords( GetMainSecWndFileDataProc, fpHelpFileInfo->SystemInfo.hSecondWndData, (LPARAM)(FPSECWNDENUMDATA)&SecWndEnumData );

  /* Unlock data. */
  GlobalUnlock( hHelpFileInfo );

  /* Did not find it. */
  if( SecWndEnumData.bFound == FALSE ) return( FALSE );
  
  /* Process the data. */
  ProcessSecWndData( hWnd, &(SecWndEnumData).SecWndData, szCaption, WndPlacePtr,
                     ScrollColorPtr, NonScrollColorPtr, bOnTop );

  /* Was there really a title? */
  if( SecWndEnumData.SecWndData.Flags & WSYSFLAG_CAPTION )
  {
    *bUseCaptionPtr = TRUE;
  }
  else
  {
    *bUseCaptionPtr = FALSE;
  }
  
  /* Success. */
  return( TRUE );
}


/*************************************
*
*  EnumSysRecords() callback function 
*  used in GetMainSecWndFileData().
*                  
**************************************/
static BOOL CALLBACK GetMainSecWndFileDataProc( FPSYSTEMREC fpSysRec, LPARAM lParam )
{
  FPSECWINDOW SecWindowDataPtr;
  FPSECWINDOW SaveSecWindowDataPtr;
  FPSECWNDENUMDATA SecWndEnumDataPtr;
  
  /* Get search data. */
  SecWndEnumDataPtr = (FPSECWNDENUMDATA) lParam; 

  /* Lock secondary window data. */
  SecWindowDataPtr = (FPSECWINDOW) GlobalLock( fpSysRec->hRecData );
    
  /* Correct record? */
  if( _fstrcmp(SecWindowDataPtr->Type, "main") == 0 )
  {
    /* Copy the secondary window data. */
    SaveSecWindowDataPtr = &(SecWndEnumDataPtr)->SecWndData;
    *SaveSecWindowDataPtr = *SecWindowDataPtr;

    /* Unlock secondary window data. */
    GlobalUnlock( fpSysRec->hRecData );
  
    /* Found secondary window data. */
    SecWndEnumDataPtr->bFound = TRUE;
    
    /* Stop enumerating. */
    return( FALSE );
  }
  
  /* Unlock secondary window data. */
  GlobalUnlock( fpSysRec->hRecData );
  
  /* Keep enumerating. */
  return( TRUE );
}



void __far __pascal ProcessSecWndData
( 
  HWND hWnd, 
  FPSECWINDOW SecWndDataPtr, 
  char __far * szCaption,
  WINDOWPLACEMENT __far * WndPlacePtr,
  COLORREF __far * ScrollColorPtr,
  COLORREF __far * NonScrollColorPtr,
  BOOL  __far * bOnTop
)
{
  /* 
  ** Window caption. 
  */
  if( SecWndDataPtr->Flags & WSYSFLAG_CAPTION )
  {
    _fstrcpy( szCaption, SecWndDataPtr->Caption );
  }
  else
  {
    szCaption = '\0';
  }


  /*
  ** Window size and position.
  */

  /* If maximize flag valid and window maximized. */
  if( (SecWndDataPtr->Flags & WSYSFLAG_MAXIMIZE) && SecWndDataPtr->Maximize )
  {
      WndPlacePtr->showCmd = SW_SHOWMAXIMIZED;
      WndPlacePtr->rcNormalPosition.left   = CW_USEDEFAULT;
      WndPlacePtr->rcNormalPosition.top    = CW_USEDEFAULT;
      WndPlacePtr->rcNormalPosition.right  = CW_USEDEFAULT;
      WndPlacePtr->rcNormalPosition.bottom = CW_USEDEFAULT;
  }
  else
  {
    WndPlacePtr->showCmd = SW_SHOWNORMAL;

    /* If x, y, width, height valid. */
    if( (SecWndDataPtr->Flags & WSYSFLAG_X) && 
        (SecWndDataPtr->Flags & WSYSFLAG_Y) &&
        (SecWndDataPtr->Flags & WSYSFLAG_WIDTH) &&
        (SecWndDataPtr->Flags & WSYSFLAG_HEIGHT)
    )
    {
      WndPlacePtr->rcNormalPosition.left   = SecWndDataPtr->X;
      WndPlacePtr->rcNormalPosition.top    = SecWndDataPtr->Y;
      WndPlacePtr->rcNormalPosition.right  = SecWndDataPtr->Width;
      WndPlacePtr->rcNormalPosition.bottom = SecWndDataPtr->Height;
      
      /* Convert virtual position to physical coords. */
      VirtualPosToScrnPos( hWnd, &(WndPlacePtr)->rcNormalPosition );
    }
    else
    {
      WndPlacePtr->rcNormalPosition.left   = CW_USEDEFAULT;
      WndPlacePtr->rcNormalPosition.top    = CW_USEDEFAULT;
      WndPlacePtr->rcNormalPosition.right  = CW_USEDEFAULT;
      WndPlacePtr->rcNormalPosition.bottom = CW_USEDEFAULT;
    }
  }


  /* Scrolling window's background color. */
  if( SecWndDataPtr->Flags & WSYSFLAG_RGB )
  {
    *ScrollColorPtr = RGB( SecWndDataPtr->Rgb[0],  
                           SecWndDataPtr->Rgb[1], 
                           SecWndDataPtr->Rgb[2] );
  }
  else
  {
    /* Use INI setting. */
    ReadScrollBGColor( ScrollColorPtr );
  }


  /* Non-scrolling window's background color. */
  if( SecWndDataPtr->Flags & WSYSFLAG_RGBNSR )
  {
    *NonScrollColorPtr = RGB( SecWndDataPtr->RgbNsr[0], 
                              SecWndDataPtr->RgbNsr[1], 
                              SecWndDataPtr->RgbNsr[2] );
  }
  else
  {
    /* Use INI setting. */
    ReadNonScrollBGColor( NonScrollColorPtr );
  }


  /* On top? */
  if( SecWndDataPtr->Flags & WSYSFLAG_TOP )
  {
    *bOnTop = TRUE;
  }
  else
  {
    *bOnTop = FALSE;
  }
}


/*************************************
*
*  Find the secondary window 
*  information for the specified
*  secondary window number.
*                  
**************************************/
BOOL __far __pascal GetSecWndNumFileData
(  
  HWND hWnd,
  HGLOBAL hHelpFileInfo, 
  WORD wSecWndNum,
  char __far * szCaption,
  WINDOWPLACEMENT __far * WndPlacePtr,
  COLORREF __far * ScrollColorPtr,
  COLORREF __far * NonScrollColorPtr,
  BOOL  __far * bOnTop,
  BOOL  __far * bMainWnd
)
{
  SECWNDENUMDATA SecWndEnumData;
  FPHLPFILEINFO fpHelpFileInfo;
  
  
  /* Initialize search data. */
  SecWndEnumData.bFound      = FALSE;
  SecWndEnumData.wCurrWndNum = 0;
  SecWndEnumData.wSecWndNum  = wSecWndNum;
  
  /* Lock data. */
  fpHelpFileInfo = (FPHLPFILEINFO) GlobalLock( hHelpFileInfo );
  
  /* Enumerate the records and find the data. */
  EnumSysRecords( GetSecWndNumFileDataProc, fpHelpFileInfo->SystemInfo.hSecondWndData, (LPARAM)(FPSECWNDENUMDATA)&SecWndEnumData );

  /* Unlock data. */
  GlobalUnlock( hHelpFileInfo );

  /* Did not find it. */
  if( SecWndEnumData.bFound == FALSE ) return( FALSE );
  
  /* Process the data. */
  ProcessSecWndData( hWnd, &(SecWndEnumData).SecWndData, szCaption, WndPlacePtr,
                     ScrollColorPtr, NonScrollColorPtr, bOnTop );

  /* Is this number for the main window? */
  if( _fstrcmp(SecWndEnumData.SecWndData.Type, "main") == 0 )
  {
    *bMainWnd = TRUE;
  }
  else
  {
    *bMainWnd = FALSE;
  }

  /* Found data. */
  return( TRUE );
}


/*************************************
*
*  EnumSysRecords() callback function 
*  used in GetSecWndNumFileData().
*                  
**************************************/
static BOOL CALLBACK GetSecWndNumFileDataProc( FPSYSTEMREC fpSysRec, LPARAM lParam )
{
  FPSECWINDOW SecWindowDataPtr;
  FPSECWINDOW SaveSecWindowDataPtr;
  FPSECWNDENUMDATA SecWndEnumDataPtr;
  
  /* Get search data. */
  SecWndEnumDataPtr = (FPSECWNDENUMDATA) lParam; 

  /* Correct record? */
  if( SecWndEnumDataPtr->wCurrWndNum == SecWndEnumDataPtr->wSecWndNum )
  {
    /* Lock secondary window data. */
    SecWindowDataPtr = (FPSECWINDOW) GlobalLock( fpSysRec->hRecData );
    
    /* Copy the secondary window data. */
    SaveSecWindowDataPtr = &(SecWndEnumDataPtr)->SecWndData;
    *SaveSecWindowDataPtr = *SecWindowDataPtr;

    /* Found secondary window data. */
    SecWndEnumDataPtr->bFound = TRUE;
    
    /* Unlock secondary window data. */
    GlobalUnlock( fpSysRec->hRecData );
  
    /* Stop enumerating. */
    return( FALSE );
  }
  
  /* Processed this record. */
  SecWndEnumDataPtr->wCurrWndNum = SecWndEnumDataPtr->wCurrWndNum + 1;
  
  /* Keep enumerating. */
  return( TRUE );
}




/***************************************************
*
* Create the icon for the help file.
*
****************************************************/
static HICON __far __pascal CreateHelpFileIcon( HWND hErrorWnd, HFILE hHelpFile, WORD wSize )
{
  HGLOBAL hGlobal;
  BYTE __far * DataPtr;
  
  long int  nLineWidth;
  WORD      wXORBitmapSize;
  
  WORD __far * wNumColorsPtr;

  BITMAP Bitmap;
  BITMAPINFO __far * BitMapInfoPtr;
  BITMAPINFOHEADER __far * BitMapInfoHdrPtr;

  BYTE __far * XORDIBBitsPtr;
  HBITMAP hXORBitmap;
  HGLOBAL hXORBitmapBits;
  BYTE __far * XORBitmapBitsPtr;
  WORD wXORBitmapBitsSize;

  BYTE __far * ANDDIBBitsPtr;
  HBITMAP hANDBitmap;
  HGLOBAL hANDBitmapBits;
  BYTE __far * ANDBitmapBitsPtr;
  WORD wANDBitmapBitsSize;
  
  HICON hIcon;
  HDC   hDC;
  
  
  /* Allocate space to read in the icon's data. */
  hGlobal = GlobalAlloc( GHND, wSize );
  if( hGlobal == NULL )
  {
    return( NULL );
  }
  
  /* Lock the data buffer. */
  DataPtr = ( BYTE __far * ) GlobalLock( hGlobal );
        
  /* Read the icon data into buffer. */
  if( _lread( hHelpFile, DataPtr, wSize ) != wSize ) 
  {
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );

    /* Free data buffer. */
    GlobalUnlock( hGlobal );
    GlobalFree( hGlobal );

    return( NULL );
  }
    
  /* Get number of colors. */
  wNumColorsPtr = (WORD __far *) (DataPtr + 8);

  /* Skip over icon header. */
  DataPtr = DataPtr + 22;

  /* Get the bitmap header data. */
  BitMapInfoPtr = (BITMAPINFO __far *) DataPtr;
  BitMapInfoHdrPtr = (BITMAPINFOHEADER __far *) DataPtr;

  /* Change header to have the correct height. */
  BitMapInfoHdrPtr->biHeight = BitMapInfoHdrPtr->biHeight / 2;
  
  /* Skip the bitmap info. header and the palette info. */
  DataPtr = DataPtr + sizeof(BITMAPINFOHEADER);
  DataPtr = DataPtr + (*wNumColorsPtr * sizeof(RGBQUAD));
  
  /* Start of DIB bits. */
  XORDIBBitsPtr = ( BYTE __far * ) DataPtr;
  
  /* Get size of XOR bitmap raw data. */
  nLineWidth = CalcBitmapByteWidth( BitMapInfoHdrPtr->biWidth, BitMapInfoHdrPtr->biBitCount );
  wXORBitmapSize = (WORD) ( nLineWidth * BitMapInfoHdrPtr->biHeight );
  
  /* Skip to AND bitmap. */
  DataPtr = DataPtr + wXORBitmapSize;
  ANDDIBBitsPtr = ( BYTE __far * ) DataPtr;
  
/*****
** 
** Bitmap creation 
**
******/

  /* Get DC. */
  hDC = GetDC( hErrorWnd );

  /* Make XOR bitmap. */
  hXORBitmap = CreateDIBitmap( hDC, BitMapInfoHdrPtr, CBM_INIT, XORDIBBitsPtr, BitMapInfoPtr, 
                            DIB_RGB_COLORS );
  
  /* Change BITMAPINFO structure for monochrome AND bitmap. */
  BitMapInfoHdrPtr->biPlanes = 1;
  BitMapInfoHdrPtr->biBitCount = 1; 
  BitMapInfoHdrPtr->biSizeImage = 0;
  BitMapInfoPtr->bmiColors[0].rgbBlue     = 0;
  BitMapInfoPtr->bmiColors[0].rgbGreen    = 0;
  BitMapInfoPtr->bmiColors[0].rgbRed      = 0;
  BitMapInfoPtr->bmiColors[0].rgbReserved = 0;
  BitMapInfoPtr->bmiColors[1].rgbBlue     = 255;
  BitMapInfoPtr->bmiColors[1].rgbGreen    = 255;
  BitMapInfoPtr->bmiColors[1].rgbRed      = 255;
  BitMapInfoPtr->bmiColors[1].rgbReserved = 0;

  /* Make AND bitmap. */
  hANDBitmap = CreateBitmap( (short int) BitMapInfoHdrPtr->biWidth, 
                             (short int) BitMapInfoHdrPtr->biHeight, 
                             1, 1, NULL );
  SetDIBits( hDC, hANDBitmap, 0, (UINT) BitMapInfoHdrPtr->biHeight, ANDDIBBitsPtr, BitMapInfoPtr,
             DIB_RGB_COLORS );
  
  /* Release DC. */
  ReleaseDC( hErrorWnd, hDC );


/*****
** 
** AND Bitmap 
**
******/

  /* Get the AND bitmap's info. */
  GetObject( hANDBitmap, sizeof(BITMAP), &Bitmap );
  
  /* Size of AND bitmap's bits. */
  wANDBitmapBitsSize = (WORD) (Bitmap.bmHeight * Bitmap.bmWidthBytes);
  
  /* Allocate space for the AND bitmap's bits buffer. */
  hANDBitmapBits = GlobalAlloc( GHND, wANDBitmapBitsSize );
  if( hANDBitmapBits == NULL )
  {
    /* Free data buffer. */
    GlobalUnlock( hGlobal );
    GlobalFree( hGlobal );

    /* Delete the AND bitmap. */
    DeleteObject( hANDBitmap );

    /* Delete the XOR bitmap. */
    DeleteObject( hXORBitmap );

    return( NULL );
  }
  
  /* Lock the AND bitmap's bits buffer. */
  ANDBitmapBitsPtr = ( BYTE __far * ) GlobalLock( hANDBitmapBits );
        
  /* Get the AND bitmap's bits buffer. */
  GetBitmapBits( hANDBitmap, wANDBitmapBitsSize, ANDBitmapBitsPtr );
  
/*****
** 
** XOR Bitmap 
**
******/

  /* Get the XOR bitmap's info. */
  GetObject( hXORBitmap, sizeof(BITMAP), &Bitmap );
  
  /* Size of XOR bitmap's bits. */
  wXORBitmapBitsSize = (WORD) (Bitmap.bmHeight * Bitmap.bmWidthBytes);
  
  /* Allocate space for the XOR bitmap's bits buffer. */
  hXORBitmapBits = GlobalAlloc( GHND, wXORBitmapBitsSize );
  if( hXORBitmapBits == NULL )
  {
    /* Free data buffer. */
    GlobalUnlock( hGlobal );
    GlobalFree( hGlobal );

    /* Free AND bitmap buffer. */
    GlobalUnlock( hANDBitmapBits );
    GlobalFree( hANDBitmapBits );

    /* Delete the AND bitmap. */
    DeleteObject( hANDBitmap );

    /* Delete the XOR bitmap. */
    DeleteObject( hXORBitmap );

    return( NULL );
  }
  
  /* Lock the XOR bitmap's bits buffer. */
  XORBitmapBitsPtr = ( BYTE __far * ) GlobalLock( hXORBitmapBits );
        
  /* Get the XOR bitmap's bits buffer. */
  GetBitmapBits( hXORBitmap, wXORBitmapBitsSize, XORBitmapBitsPtr );
  
/*****
** 
** Icon creation 
**
******/

  /* Create the icon. */
  hIcon = CreateIcon( GetLibInst(), GetSystemMetrics( SM_CXICON ), 
                      GetSystemMetrics( SM_CYICON ), 
                      Bitmap.bmPlanes, 
                      Bitmap.bmBitsPixel, 
                      ANDBitmapBitsPtr , XORBitmapBitsPtr );

  /* Free the XOR bitmap's bits buffer. */
  GlobalUnlock( hXORBitmapBits );
  GlobalFree( hXORBitmapBits );
        
  /* Delete the XOR bitmap. */
  DeleteObject( hXORBitmap );

  /* Free the AND bitmap's bits buffer. */
  GlobalUnlock( hANDBitmapBits );
  GlobalFree( hANDBitmapBits );
        
  /* Delete the AND bitmap. */
  DeleteObject( hANDBitmap );

  /* Free data buffer. */
  GlobalUnlock( hGlobal );
  GlobalFree( hGlobal );

  /* Return icon handle. */
  return( hIcon );
}


/***************************************************
*
* Gets the icon stored in the help file.
*
****************************************************/
HICON __far __pascal GetHelpFileIcon( HGLOBAL hHelpFileInfo )
{
  FPHLPFILEINFO fpHelpFileInfo;
  HICON         hIcon;
  
  /* Lock data. */
  fpHelpFileInfo = (FPHLPFILEINFO) GlobalLock( hHelpFileInfo );

  /* Save contents topic offset. */
  hIcon = fpHelpFileInfo->SystemInfo.hIcon;

  /* Unlock data. */
  GlobalUnlock( hHelpFileInfo );

  /* Return icon data. */
  return( hIcon );
} 




/***************************************************
*
* Frees any dynamically allocated memory in the
* SYSTEMINFO structure.
*
****************************************************/
static void __far __pascal FreeSystemInfo( FPSYSTEMINFO fpSystemInfo )
{
  /* Free title text. */
  if( fpSystemInfo->hTitleText )
  {
    GlobalFree( fpSystemInfo->hTitleText );
    fpSystemInfo->hTitleText = NULL; 
  }

  /* Free alternate icon. */
  if( fpSystemInfo->hIcon != NULL )
  {
    DestroyIcon( fpSystemInfo->hIcon );
    fpSystemInfo->hIcon = NULL;
  }

  /* Free macro records data. */
  if( fpSystemInfo->hMacroData )
  {
    EnumSysRecords( FreeSystemVarLengthData, fpSystemInfo->hMacroData, 0L );
    GlobalFree( fpSystemInfo->hMacroData );
    fpSystemInfo->hMacroData = NULL;
  }

  /* Free secondary window records data. */
  if( fpSystemInfo->hSecondWndData )
  {
    EnumSysRecords( FreeSystemVarLengthData, fpSystemInfo->hSecondWndData, 0L );
    GlobalFree( fpSystemInfo->hSecondWndData );
    fpSystemInfo->hSecondWndData = NULL;
  }
}


/***************************************************
*
* Free memory allocated for SYSTEM records' variable
* length data.
*
* Used by FreeSystemInfo(). Called by EnumSysRecords()
*
****************************************************/
static BOOL __far __pascal FreeSystemVarLengthData( FPSYSTEMREC fpSysRec, LPARAM lParam )
{
  /* Free record's varaible length data. */
  if( fpSysRec->hRecData != 0 ) GlobalFree( fpSysRec->hRecData );
    
  /* Keeping sending. */
  return( TRUE );
}


/***************************************************
*
* Insert a new |SYSTEM record into our in-memory
* buffer of records.
*
****************************************************/
static RETVAL __far __pascal InsertSysRecord( HWND hErrorWnd, FPREADSYSTEMREC fpRecordInfo, HGLOBAL hRecordData, HGLOBAL __far * hRecordBuffer )
{
  FPSYSTEMREC fpRecord;
  WORD wArrayCount;
                 
                 
  /* First record. */
  if( *hRecordBuffer == NULL )
  {
    /* Allocate space for the new system record. */
    *hRecordBuffer =  GlobalAlloc( GHND, sizeof(SYSTEMREC) * 2 );
    if( *hRecordBuffer == NULL )
    {
      return( ERR_MEMORY );
    }

    /* First space always used for count of elements in array. */
    fpRecord = ( FPSYSTEMREC ) GlobalLock( *hRecordBuffer );
    fpRecord->DataSize = 0;
    fpRecord->hRecData = 0;
    GlobalUnlock( *hRecordBuffer );
  }
  
  /* Not first record. */
  else
  {
    /* Get the number of records stored in the array. */
    fpRecord = ( FPSYSTEMREC ) GlobalLock( *hRecordBuffer );
    wArrayCount = fpRecord->DataSize;
    GlobalUnlock( *hRecordBuffer );

    /* 
    ** Allocate more space for the existing array. 
    ** Extra 2 spaces allocated for the new system record and
    ** the first slot that holds the array count.
    */
    *hRecordBuffer = GlobalReAlloc( *hRecordBuffer, sizeof(SYSTEMREC) * (wArrayCount + 2), GMEM_MOVEABLE | GMEM_ZEROINIT );
    if( *hRecordBuffer == NULL )
    {
      return( ERR_MEMORY );
    }
  }
  
  /* Lock memory. */
  fpRecord = ( FPSYSTEMREC ) GlobalLock( *hRecordBuffer );

  /* Increment the array count that is stored in first slot. */
  fpRecord->DataSize = fpRecord->DataSize + 1;

  /* Move to the position of the new record. */
  fpRecord = fpRecord + fpRecord->DataSize;

  /* Assign data to new record. */
  fpRecord->RecordType =  fpRecordInfo->RecordType;
  fpRecord->DataSize   =  fpRecordInfo->DataSize;
  fpRecord->hRecData   =  hRecordData;
 
  /* Unlock memory. */
  GlobalUnlock( *hRecordBuffer );

  /* Success. */
  return( NO_ERROR );
}


/***************************************************
*
* |SYSTEM records enumeration function.
*
****************************************************/
void __far __pascal EnumSysRecords( SYSTEMRECPROC fpCallback, HGLOBAL hRecordBuffer, LPARAM lParam )
{
  BOOL         bContinue;      /* bContinue enumerating. */
  FPSYSTEMREC  fpRecord;       /* Pointer to a SYSTEM file's record. */
  WORD         wCounter;       /* Current SYSTEM file record. */
  WORD         wArrayCount;    /* The number of elements in the array. */
  
  
  /* No buffer. */
  if( hRecordBuffer == NULL ) return;

  /* Lock memory. */
  fpRecord = ( FPSYSTEMREC ) GlobalLock( hRecordBuffer );

  /* 
  ** First element is used to hold the number of
  ** records stored in the array. 
  */
  wArrayCount = fpRecord->DataSize;
  
  /* Go to first ACTUAL record. */
  fpRecord++;
  
  /* For each record in the array. */
  for( wCounter = 0; wCounter < wArrayCount; wCounter++ )
  {
    /* Send record data to call-back function. */
    bContinue = (*fpCallback)( fpRecord, lParam );
         
    /* Stop enumerating? */
    if( ! bContinue ) 
    {
      /* Unlock memory. */
      GlobalUnlock( hRecordBuffer );

      return;
    }
    
    /* Goto next record. */
    fpRecord++;
  }

  /* Unlock memory. */
  GlobalUnlock( hRecordBuffer );
}


/***************************************************
*
* Get the compression value for the .HLP file.
*
* FALSE is returned is no compression is used.
*
* TRUE is otherwise returned.
*
* I'm not returning the actual level of
* compression because:
*
* 1 - There's no way that I can tell if MEDIUM or
*     or HIGH compression was used since the
*     flag's value was 0x0004 in both cases when
*     using the HC31.EXE compiler. Maybe
*     there's another flag set somewhere else.
*
* 2 - I really don't care about it.  If there is
*     ANY compression, then there is always block
*     compression.  If HIGH compression was used,
*     I'll know about it because I'll find 
*     references to the |Phrase file when I'm parsing
*     the |TOPIC data.
*
****************************************************/
static BOOL __far __pascal GetCompressionStatus( FPSYSTEMINFO fpSystemInfo )
{
  return( fpSystemInfo->bCompressed );
}


/***************************************************
*
* Get the compiler version for the .HLP file.
*
****************************************************/
static COMPILERVER __far __pascal GetCompilerVersion( FPSYSTEMINFO fpSystemInfo )
{
  return( fpSystemInfo->CompilerVer );
}


/***************************************************
*
* Get the "CONTENTS=" field's record.
*
* Its the topic number for the .HLP file's 
* default topic.
*
****************************************************/
void __far __pascal GetContentsTopic( HGLOBAL hHelpFileInfo, char __far * szHelpFilePath, DWORD __far * dwTopicCharOffset )
{
  FPHLPFILEINFO fpHelpFileInfo;
  
  /* Lock data. */
  fpHelpFileInfo = (FPHLPFILEINFO) GlobalLock( hHelpFileInfo );

  /* Save contents topic offset. */
  *dwTopicCharOffset = fpHelpFileInfo->SystemInfo.dwContentsTopic;
  
  /* Save help file's path. */
  _fstrcpy( szHelpFilePath, fpHelpFileInfo->SystemInfo.szContentsFile );
  
  /* Unlock data. */
  GlobalUnlock( hHelpFileInfo );
} 


/***************************************************
*
* Set the "CONTENTS=" field's record.
*
****************************************************/
BOOL __far __pascal SetContentsTopic( HWND hWnd, HGLOBAL hHelpFileInfo, char __far * szContentsHelpFilePath, unsigned long int nContentsContext )
{
  FPHLPFILEINFO fpHelpFileInfo;
  
  HGLOBAL hContentsHelpFileInfo;         /* Contents button's help file's general info. */
  DWORD dwTopicCharOffset;               /* Contents button's location of topic data. */
  

  /* Open and load the Contents button's help file's info. */
  hContentsHelpFileInfo = OpenHelpFile( hWnd, szContentsHelpFilePath );
  if( hHelpFileInfo == NULL )
  {
    /* Failure. */
    return( FALSE );
  }

  /* Get the Contents button's location of topic data. */
  if( ! GetCTXOMAPOffset( hWnd, hContentsHelpFileInfo, nContentsContext, &dwTopicCharOffset ) )
  {
    /* Failure. */
    return( FALSE );
  }

  /* Lock data for the current help file. */
  fpHelpFileInfo = (FPHLPFILEINFO) GlobalLock( hHelpFileInfo );

  /* Save contents topic offset. */
  fpHelpFileInfo->SystemInfo.dwContentsTopic = dwTopicCharOffset;

  /* Save help file's path. */
  _fstrcpy( fpHelpFileInfo->SystemInfo.szContentsFile, szContentsHelpFilePath );
  
  /* Unlock data for the current help file. */
  GlobalUnlock( hHelpFileInfo );

  /* Success. */
  return( TRUE );
} 



/***************************************************
*
* Get the "TITLE=" field's record.
*
* It is the user-defined caption text for the help
* window.
*
****************************************************/
HGLOBAL __far __pascal GetTitleText( HGLOBAL hHelpFileInfo )
{
  FPHLPFILEINFO fpHelpFileInfo;
  HGLOBAL       hTitle;
  

  /* No help file data. */
  if( hHelpFileInfo == NULL ) return( NULL );

  /* Lock help data. */
  fpHelpFileInfo = (FPHLPFILEINFO) GlobalLock( hHelpFileInfo );

  /* If we have a title string. */
  hTitle = fpHelpFileInfo->SystemInfo.hTitleText;
  
  /*Unlock help data. */
  GlobalUnlock( hHelpFileInfo );

  /* Return the memory handle for the title. */
  return( hTitle );
} 


/****************************** |Phrases FILE ROUTINES ******************************/


/***************************************************
*
* Loads the |Phrases file's information into memory. 
* Need this later on to complete the topic data.
*
****************************************************/
static RETVAL __far __pascal LoadPhrasesFile
( 
  HWND hErrorWnd, 
  HFILE hHelpFile, 
  long BeginPos, 
  FPSYSTEMINFO fpSystemInfo, 
  HGLOBAL __far * hPhrasesInfo 
)
{
  FILEHEADER         FileHdr;            /* Generic WHIFS file header.                */
  PHRASEHEADER       PhraseHdr;          /* |Phrases file header.                     */

  WORD               OffsetsTableSize;   /* Size of the offsets table.                */ 
  WORD __far *       fpOffsetsTable;     /* Offset table into phrases data.           */

  char __far *       fpPhrases;          /* Actual phrases data.                      */
  
  long  int          LastFilePos;        /* Last file position of the WHIF file. */
            
    
  /* Go to the beginning file position of the |Phrases file. */
  if( _lseek( hHelpFile, BeginPos, SEEK_SET ) == -1 )
  {
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
    return( ERR_OTHER);
  }

  /* Read the generic WHIFS file header that exists for each WHIFS file. */
  if( _lread( hHelpFile, &FileHdr, sizeof(FILEHEADER) ) != sizeof(FILEHEADER) ) 
  {
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
    return( ERR_OTHER );
  }

  /* Last file position of the WHIF file. */
  LastFilePos = _tell( hHelpFile ) + FileHdr.FileSize;
  
  /* Read the header information for the |Phrases file. */
  if( _lread( hHelpFile, (FPPHRASEHEADER) &PhraseHdr, sizeof(PHRASEHEADER) ) != sizeof(PHRASEHEADER) )
  {
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
    return( ERR_OTHER );
  }
  
  /* 
  ** How much space do we need for the offsets table. 
  **
  ** One extra offset is added to the count because
  ** because there is always one more offset than the
  ** number of phrases.  This extra offset points to the
  ** character after the last phrase's last character.
  */
  OffsetsTableSize = (PhraseHdr.NumPhrases + 1) * sizeof(WORD);
    
#if 0
  /* Get the version of the compiler used. */
  CompilerVer = GetCompilerVersion( fpSystemInfo );
  
  /* Version 3.0. */
  if( CompilerVer == COMPILER_300 )
  {
    CompressedSize;     
    UnCompressedSize = 0   
  }
  else if( CompilerVer == COMPILER_310 )
  {
  
  
  }
#endif

  /* Allocate memory for offsets table and uncompressed phrases. */
  *hPhrasesInfo =  GlobalAlloc( GHND, PhraseHdr.PhrasesSize + OffsetsTableSize );
  if( *hPhrasesInfo == NULL )
  {
    return( ERR_MEMORY );
  }

  /* Lock data buffer. */
  fpOffsetsTable = (WORD __far *) GlobalLock( *hPhrasesInfo );

  /* Read the uncompressed offsets table. */
  if( _lread( hHelpFile, fpOffsetsTable, OffsetsTableSize ) != OffsetsTableSize ) 
  {
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
    GlobalUnlock( *hPhrasesInfo );
    GlobalFree( *hPhrasesInfo );
    return( ERR_OTHER );
  }

  /* Point to place in buffer to store phrases data. */
  fpPhrases = (char __far *) fpOffsetsTable + OffsetsTableSize;

  /* 
  ** Phrases are always compressed with HC 3.1. 
  **
  ** Read phrases info. from file and uncompress the data
  ** into the buffer. 
  */
  Decompress( hErrorWnd, hHelpFile,
              (long) FileHdr.FileSize - (sizeof(PHRASEHEADER) + OffsetsTableSize), 
              PhraseHdr.PhrasesSize, fpPhrases, LastFilePos );

  /* Unlock data buffer. */
  GlobalUnlock( *hPhrasesInfo );


/*****
**
** Old code to handle non-compressed phrases.
**
*****/

#if 0
  /* Not compressed. */
  else 
  {
    /* Allocate memory for offset table and uncompressed phrases. */
    *hPhrasesInfo =  GlobalAlloc( GHND, FileHdr.FileSize - sizeof(PHRASEHEADER) );
    if( *hPhrasesInfo == NULL )
    {
      return( ERR_MEMORY );
    }

    /* 
    ** Backup in file when phrases are uncompressed.
    ** No PhrasesSize field in PHRASEHEADER structure. 
    */
    if( _lseek( hHelpFile, -1 * (int) sizeof(PhraseHdr.PhrasesSize), SEEK_CUR ) == -1 )
    {
      MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
      return( ERR_OTHER );
    }

    /* Lock data buffer. */
    fpOffsetsTable = (WORD __far *) GlobalLock( *hPhrasesInfo );

    /* Read in phrase offsets table and uncompressed phrases. */
    if( _lread( hHelpFile, fpOffsetsTable, (WORD)(FileHdr.FileSize - sizeof(DWORD)) ) !=  (WORD)(FileHdr.FileSize - sizeof(DWORD)) ) 
    {
      MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
      GlobalUnlock( *hPhrasesInfo );
      GlobalFree( *hPhrasesInfo );
      return( ERR_OTHER );
    }

    /* Unlock data buffer. */
    GlobalUnlock( *hPhrasesInfo );
  }
#endif

  /* Success. */
  return( NO_ERROR );
}


/***************************************************
*
* Frees the Phrase info buffer.
*
****************************************************/
static void __far __pascal FreePhrasesInfo( HGLOBAL __far * fphPhraseInfo )
{
  /* Free all phrase info. */
  if( *fphPhraseInfo )
  {
    GlobalFree( *fphPhraseInfo );
    *fphPhraseInfo = NULL; 
  }
}


/***************************************************
*
* Returns a pointer to a phrase in the helphrase table. 
*
****************************************************/
char __far * __far __pascal GetPhrasePtr( FPPHRASEINFO fpPhraseInfo, WORD PhraseNum, WORD __far * wSize )
{
  WORD __far * fpOffsets;
  
  /* Get the beginning of the phrase offset table. */ 
  fpOffsets = (WORD __far *) fpPhraseInfo;

  /* Get the phrase size in bytes. */
  *wSize = fpOffsets[ PhraseNum + 1 ] - fpOffsets[ PhraseNum ];

  /* Return the beginning of the phrase. */
  return( fpPhraseInfo + fpOffsets[ PhraseNum ] );
}



/****************************** |CTXOMAP FILE ROUTINES ******************************/


/***************************************************
*
* Loads the |CTXOMAP file's information into memory. 
* Need this later to display a topic given information
* passed via the SDK WinHelp() interface.
*
****************************************************/
static RETVAL __far __pascal LoadCTXOMAPFile( HWND hErrorWnd, HFILE hHelpFile, long BeginPos, HGLOBAL __far * hCTXOMAPInfo )
{
  FILEHEADER      FileHdr;            /* Generic WHIFS file header.  */
  WORD            wNumRecs,           /* Number of CTXOMAP records. */
                  wCounter;
  FPCTXOMAPREC    fpData;             /* Far pointer to a CTXOMAP record. */

  
  /* Go to the beginning file position of the |SYSTEM file. */
  if( _lseek( hHelpFile, BeginPos, SEEK_SET ) == -1 )
  {
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
    return( ERR_OTHER );
  }

  /* Read the generic WHIFS file header that exists for each WHIFS file. */
  if( _lread( hHelpFile, &FileHdr, sizeof(FILEHEADER) ) != sizeof(FILEHEADER) ) 
  {
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
    return( ERR_OTHER );
  }

  /* Get to the number of CTXOMAP records. */
  if( _lread( hHelpFile, &wNumRecs, sizeof(WORD) ) != sizeof(WORD) ) 
  {
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
    return( ERR_OTHER );
  }

  /*
  ** Allocate memory for the table of CTXOMAP records. 
  **
  ** The 1 extra record will store the record count
  ** in the MapID field.
  */
  *hCTXOMAPInfo =  GlobalAlloc( GHND, (wNumRecs + 1) * sizeof(CTXOMAPREC) );
  if( *hCTXOMAPInfo == NULL )
  {
    return( ERR_MEMORY );
  }

  /* Lock table's buffer. */
  fpData = ( FPCTXOMAPREC ) GlobalLock( *hCTXOMAPInfo );
  
  /* In first record, only store the record count. */
  fpData->MapID = (long) wNumRecs;
  fpData++;
  
  /* Now read in file's data for the rest of the CTXOMAP records. */
  for( wCounter = 0; wCounter < wNumRecs; wCounter++ ) 
  {
    /* Get to the number of CTXOMAP records. */
    if( _lread( hHelpFile, fpData, sizeof(CTXOMAPREC) ) != sizeof(CTXOMAPREC) ) 
    {
      MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
      GlobalUnlock( *hCTXOMAPInfo );
      GlobalFree( *hCTXOMAPInfo );
      return( ERR_OTHER );
    }

    /* Go to place in buffer to store next records information. */
    fpData++;
  }

  /* Unlock table's buffer. */
  GlobalUnlock( *hCTXOMAPInfo );

  /* Success. */
  return( NO_ERROR );
}


/***************************************************
*
* Frees the |CTXOMAP info buffer.
*
****************************************************/
static void __far __pascal FreeCTXOMAPInfo( HGLOBAL __far * fphCTXOMAPInfo )
{
  /* Free all phrase info. */
  if( *fphCTXOMAPInfo )
  {
    GlobalFree( *fphCTXOMAPInfo );
    *fphCTXOMAPInfo = NULL; 
  }
}


/***************************************************
*
* Gets the character offset associated with a
* given topic's ID number.
*
****************************************************/
BOOL __far __pascal GetCTXOMAPOffset( HWND hErrorWnd, HGLOBAL hHelpFileInfo, long TopicID, DWORD __far * dwTopicCharOffset )
{
  FPHLPFILEINFO   fpHelpFileInfo;       /* Pointer to the help information. */
  WORD            wNumRecs,             /* Number of CTXOMAP records. */
                  wCounter;
  FPCTXOMAPREC    fpData;               /* Far pointer to a CTXOMAP record. */
  

  /* Lock the help data. */
  fpHelpFileInfo = (FPHLPFILEINFO) GlobalLock( hHelpFileInfo );

  /* Verify that we even have a CTXOMAP table to read from. */
  if( fpHelpFileInfo->hCTXOMAPInfo == NULL )
  {
    /* Show error message. */
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_CTXOMAP, MB_ICONHAND | MB_OK );
  }
  
  /* Lock the CTXOMAP table's buffer. */
  fpData = ( FPCTXOMAPREC ) GlobalLock( fpHelpFileInfo->hCTXOMAPInfo );
  
  /* From the first record, get the record count. */
  wNumRecs = (WORD) fpData->MapID;
  fpData++;
  
  /* Now read in file's data for the rest of the CTXOMAP records. */
  for( wCounter = 0; wCounter < wNumRecs; wCounter++ ) 
  {
    /* See if the record holds the ID we are looking for. */
    if( fpData->MapID == TopicID ) 
    {
      /* Save character offset. */
      *dwTopicCharOffset = fpData->TopicOffset;
      
      /* Unlock CTXOMAP data. */
      GlobalUnlock( fpHelpFileInfo->hCTXOMAPInfo );
    
      /* Unlock help data. */
      GlobalUnlock( hHelpFileInfo );
    
      /* Success. */
      return( TRUE );
    }

    /* Go to place in buffer to store next records information. */
    fpData++;
  }

  /* Unlock CTXOMAP data. */
  GlobalUnlock( fpHelpFileInfo->hCTXOMAPInfo );

  /* Unlock help data. */
  GlobalUnlock( hHelpFileInfo );

  /* Show error message. */
  MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_CTXOMAP, MB_ICONHAND | MB_OK );

  /* Failure. */
  return( FALSE );
}


/****************************** |TTLBTREE FILE ROUTINES ******************************/

/***************************************************
*
* Loads the |TTLBTREE file's information into memory. 
*
****************************************************/
static RETVAL __far __pascal LoadTTLBTREEFile( HWND hErrorWnd, HFILE hHelpFile, long BeginPos, HGLOBAL __far * hTTLBTREEInfo )
{
  /* Use standard b-tree loading function for now. */
  return( LoadBTreeFile( hErrorWnd, hHelpFile, BeginPos, hTTLBTREEInfo ) );
}


/***************************************************
*
* Frees the |TTLBTREE info buffer.
*
****************************************************/
static void __far __pascal FreeTTLBTREEInfo( HGLOBAL __far * hTTLBTREEInfo )
{
  /* Use standard b-tree freeing function for now. */
  FreeBTreeFile( hTTLBTREEInfo );
}


/***************************************************
*
* Returns a topic's title.
*
****************************************************/
void __far __pascal GetTopicTitle( HGLOBAL hHelpFileInfo, DWORD dwTopicCharOffset, char __far * szTopicTitle )
{
  FPHLPFILEINFO   fpHelpFileInfo;       /* Pointer to new help file info.      */

  BYTE __huge *        fpFirstPageLoc;   /* Pointer to a data buffer. */
  BYTE __huge *        fpData;           /* Pointer to a data buffer. */
  BTREEHEADER __huge * fpBTreeHdr;       /* B-Tree header pointer. */
  WORD                 wLeafPage;        /* Page number of data leaf. */
  BTREENODEHEADER __huge * fpNodeHeader;     /* Current node header. */

  BYTE  __huge *       fpCurrEntry;      /* Current topic title entry. */
  BYTE  __huge *       fpLastEntry;      /* Last topic title entry. */

  DWORD __huge *       fpOffset;         /* Topic's char offset. */
  WORD  wStringSize;                     /* Size of topic's title string. */
  BOOL  bGotTitle;                       /* Did we find a title string? */
  
  WORD wCount;

  /* Lock help info. */
  fpHelpFileInfo = (FPHLPFILEINFO) GlobalLock( hHelpFileInfo );

  /* No table. */
  if( fpHelpFileInfo->hTTLBTREEInfo == NULL ) return;
  
  /* Lock the data. */
  fpData = ( BYTE __huge * ) GlobalLock( fpHelpFileInfo->hTTLBTREEInfo );
  
  /* Get the TTLBTREE table. */
  fpBTreeHdr = ( BTREEHEADER __huge * ) fpData;
  fpData = fpData + sizeof(BTREEHEADER);
  
  /* Save position of first page. */
  fpFirstPageLoc = fpData;
  
  /* Get the leaf page number for the data. */
  wLeafPage = GetLeafPageDWORD( fpFirstPageLoc, fpBTreeHdr, dwTopicCharOffset );

  /* Go to leaf page. */
  fpData = fpFirstPageLoc + ( wLeafPage * BTREEPAGESIZE );
  
  /* Get node information. */
  fpNodeHeader = (BTREENODEHEADER __huge *) fpData;
  fpData = fpData + sizeof( BTREENODEHEADER );
  
  /* Haven't got topic's title. */
  bGotTitle = FALSE;
    
  /* Init. last topic title entry with first entry. */
  fpLastEntry = fpData;

  /* For each entry in the leaf page. */
  for( wCount = 0; wCount < (WORD) fpNodeHeader->NEntries; wCount++ ) 
  {
    /* Get current entry. */
    fpCurrEntry = fpData;
    
    /* Get topic offset value. */
    fpOffset = (DWORD __huge *) fpData;
    fpData = fpData + sizeof( DWORD );
    
    /* Found correct topic offset value? */
    if( *fpOffset == dwTopicCharOffset )
    {
      /* Has a title. */
      if( *fpData != '\0' )
      {
        /* Copy topic title. */
        _fstrcpy( szTopicTitle, fpData );
      }
      
      /* Doesn't have a title. */
      else
      {
        /* Load the default string. */
        LoadString( GetLibInst(), IDS_NOTOPICTITLE, szTopicTitle, MAX_TOPIC_TITLE );
      }

      /* Got topic's title. */
      bGotTitle = TRUE;

      /* Stop looking. */
      break;
    }
    
    /* 
    ** No match found and past last possible value.
    */
    else if( *fpOffset > dwTopicCharOffset )
    {
      /* Stop looking. */
      break;
    }
    
    /* Not correct string. */
    else
    {
      /* Get length of string. */
      wStringSize = (WORD) _fstrlen( fpData ) + 1;
    
      /* Skip to next topic offset value. */
      fpData = fpData + wStringSize;
    }

    /* Save position of last entry. */
    fpLastEntry = fpCurrEntry;
  }
    
  /* Haven't got topic's title, use the last entry considered. */
  if( !bGotTitle )
  {
    /* Get last entry. */
    fpData = fpLastEntry;
    
    /* Get topic offset value. */
    fpOffset = (DWORD __huge *) fpData;
    fpData = fpData + sizeof( DWORD );
    
    /* Has a title. */
    if( *fpData != '\0' )
    {
      /* Copy topic title. */
      _fstrcpy( szTopicTitle, fpData );
    }
      
    /* Doesn't have a title. */
    else
    {
      /* Load the default string. */
      LoadString( GetLibInst(), IDS_NOTOPICTITLE, szTopicTitle, MAX_TOPIC_TITLE );
    }
  }

  /* Unlock the TTLBTREE table. */
  GlobalUnlock( fpHelpFileInfo->hTTLBTREEInfo );

  /* Unlock help file info. */
  GlobalUnlock( hHelpFileInfo );
}



/********************************* |CONTEXT FILE ROUTINES **********************************/

/***************************************************
*
* Loads the |CONTEXT file's information into memory. 
*
****************************************************/
static RETVAL __far __pascal LoadContextFile( HWND hErrorWnd, HFILE hHelpFile, long BeginPos, HGLOBAL __far * hContextInfo )
{
  /* Use standard b-tree loading function for now. */
  return( LoadBTreeFile( hErrorWnd, hHelpFile, BeginPos, hContextInfo ) );
}
           
           
/***************************************************
*
* Frees the |CONTEXT file's information from memory. 
*
****************************************************/
static void __far __pascal FreeContextInfo( HGLOBAL __far * hContextInfo )
{
  /* Use standard b-tree freeing function for now. */
  FreeBTreeFile( hContextInfo );
}


/***************************************************
*
* Returns the |CONTEXT offset given a topic's 
* hash value.
*
****************************************************/
BOOL __far __pascal GetContextOffset( HGLOBAL hHelpFileInfo, DWORD dwHashValue, DWORD __far * dwTopicCharOffset )
{
  FPHLPFILEINFO   fpHelpFileInfo;        /* Pointer to new help file info.      */

  BYTE __huge *        fpFirstPageLoc;   /* Pointer to the location of the first leaf page. */
  BYTE __huge *        fpData;           /* Pointer to a data buffer. */
  BTREEHEADER __huge * fpBTreeHdr;       /* B-Tree header pointer. */
  WORD                 wLeafPage;        /* Page number of data leaf. */
  BTREENODEHEADER __huge * fpNodeHeader;     /* Current node header. */

  WORD wCount;

  DWORD __huge *   fpHashValue; 
  DWORD __huge *   fpTopicCharOffset;   /* Character Offset of topic's data in the |TOPIC file. */
  
  DWORD dwFileOffset;
  
  BOOL bFoundIt;

  
  /* Lock help info. */
  fpHelpFileInfo = (FPHLPFILEINFO) GlobalLock( hHelpFileInfo );

  /* No table. */
  if( fpHelpFileInfo->hContextInfo == NULL ) return FALSE;
  
  /* Lock the data. */
  fpData = ( BYTE __huge * ) GlobalLock( fpHelpFileInfo->hContextInfo );
  
  /* Get the TTLBTREE table. */
  fpBTreeHdr = ( BTREEHEADER __huge * ) fpData;
  fpData = fpData + sizeof(BTREEHEADER);
  
  /* Save position of first page. */
  fpFirstPageLoc = fpData;
  
  /* Get the leaf page number for the data. */
  wLeafPage = GetLeafPageDWORD( fpFirstPageLoc, fpBTreeHdr, dwHashValue );

  /* Go to leaf page. */
  dwFileOffset = (DWORD) wLeafPage * (DWORD) BTREEPAGESIZE;
  fpData = fpFirstPageLoc + dwFileOffset;
  
  /* Get node information. */
  fpNodeHeader = (BTREENODEHEADER __huge *) fpData;
  fpData = fpData + sizeof( BTREENODEHEADER );
  
  /* Haven't found character offset of topic's data. */
  bFoundIt = FALSE;
    
  /* For each entry in the leaf page. */
  for( wCount = 0; wCount < (WORD) fpNodeHeader->NEntries; wCount++ ) 
  {
    /* Get hash value. */
    fpHashValue = (DWORD __huge *) fpData;
    fpData = fpData + sizeof( DWORD );
    
    /* Get topic char. offset. */
    fpTopicCharOffset = (DWORD __huge *) fpData;
    fpData = fpData + sizeof( DWORD );
    
    /* Found correct hash value? */
    if( *fpHashValue == dwHashValue )
    {
      /* Have found character offset of topic's data. */
      *dwTopicCharOffset = *fpTopicCharOffset;
      bFoundIt = TRUE;
      
      /* Stop looking. */
      break;
    }
  }

  /* Unlock the TTLBTREE table. */
  GlobalUnlock( fpHelpFileInfo->hContextInfo );

  /* Unlock help file info. */
  GlobalUnlock( hHelpFileInfo );
  
  /* Always successful/failure. */
  return( bFoundIt );
}


/****************************** |KWBTREE & |KWDATA FILE ROUTINES *******************************/

/***************************************************
*
* Show keywords in combobox window.
*
****************************************************/
BOOL __far __pascal ShowKeywords( HWND hComboBox, char chKey, HGLOBAL hHelpFileInfo )
{
  FPHLPFILEINFO fpHelpFileInfo;          /* Pointer to new help file info.      */

  BYTE __huge *        fpData;             /* Pointer to a data buffer. */
  BYTE __huge *        fpFirstPageLoc;     /* Pointer to a data buffer. */
  BTREEHEADER __huge * fpBTreeHdr;         /* B-Tree header pointer. */
  BTREENODEHEADER __huge * CurrPageHdrPtr; /* Leaf page header. */

  WORD wLeafPage;        /* Page number of data leaf. */
  WORD wCount;           /* Dummy counter var. */

  WORD wStringSize;      /* Size of a keyword string. */
  
  
  /* Lock help info. */
  fpHelpFileInfo = (FPHLPFILEINFO) GlobalLock( hHelpFileInfo );

  /* Lock the data. */
  fpData = ( BYTE __huge * ) GlobalLock( fpHelpFileInfo->KeywordInfo.hKWBTREEInfo );
  
  /* Get the KWBTREE header. */
  fpBTreeHdr = ( BTREEHEADER __huge * ) fpData;
  fpData = fpData + sizeof(BTREEHEADER);
  
  /* Save position of first page. */
  fpFirstPageLoc = fpData;
  
  /* Get the first leaf's page. */
  wLeafPage = GetFirstLeafPage( fpFirstPageLoc, fpBTreeHdr );
  
  /* 
  ** For each leaf page. 
  */
  do 
  {
    /* Go to the current leaf page. */
    fpData = fpFirstPageLoc + ( wLeafPage * BTREEPAGESIZE );
  
    /* Get the current leaf page's header. */
    CurrPageHdrPtr = ( BTREENODEHEADER __huge * ) fpData;
    fpData = fpData + sizeof(BTREENODEHEADER);
    
    /* If there is another page. */
    if( CurrPageHdrPtr->NextPage != -1 )
    { 
      /* Get its page number. */
      wLeafPage = CurrPageHdrPtr->NextPage;
    }
    
    /* For each entry in the leaf page. */
    for( wCount = 0; wCount < (WORD) CurrPageHdrPtr->NEntries; wCount++ ) 
    {
      /* Get length of keyword string. */
      wStringSize = (WORD) _fstrlen( fpData ) + 1;
      
      /* Insert string into listbox. */
      SendMessage( hComboBox, LB_ADDSTRING, 0, (LPARAM)(char __far *) fpData );
      
      /* Skip past keyword string. */
      fpData = fpData + wStringSize;
    
      /* Skip past count of associated topics in KWDATA. */
      fpData = fpData + sizeof( WORD );
    
      /* Skip past offset of associated topics in KWDATA. */
      fpData = fpData + sizeof( DWORD );
    }
  } 
  while( CurrPageHdrPtr->NextPage != -1 );

  /* Unlock the KWBTREE table. */
  GlobalUnlock( fpHelpFileInfo->KeywordInfo.hKWBTREEInfo );

  /* Unlock help file info. */
  GlobalUnlock( hHelpFileInfo );

  /* Success. */
  return( TRUE );
}      



/***************************************************
*
* Show the topics for the selected keyword in combobox
* window.
*
****************************************************/
BOOL __far __pascal ShowKeywordTopics( HWND hListBox, DWORD dwKeySelect, HGLOBAL hHelpFileInfo, DWORD __far * dwKWDATAOffset )
{
  FPHLPFILEINFO fpHelpFileInfo;          /* Pointer to new help file info.      */

  BYTE __huge *        fpData;             /* Pointer to a data buffer. */
  BYTE __huge *        fpFirstPageLoc;     /* Pointer to a data buffer. */
  BTREEHEADER __huge * fpBTreeHdr;         /* B-Tree header pointer. */
  BTREENODEHEADER __huge * CurrPageHdrPtr; /* Leaf page header. */

  WORD wLeafPage;        /* Page number of data leaf. */
  WORD wCount;           /* Dummy counter var. */

  DWORD dwTotal;
  WORD  wKeyNumber;
  
  WORD  __huge * wNumTopicsPtr;
  WORD  wNumTopics;
  DWORD __huge * dwTopicsOffsetPtr;
  DWORD dwTopicsOffset;
  
  DWORD __huge * TopicOffsetPtr;
  
  char szTopicTitle[MAX_TOPIC_TITLE];

  WORD wStringSize;
  
    
  /* Lock help info. */
  fpHelpFileInfo = (FPHLPFILEINFO) GlobalLock( hHelpFileInfo );

  /**************************************
  **
  **  Stage 1: Find keyword's topic count
  **           and KWDATA offset.
  **
  **************************************/

  /* Lock the data. */
  fpData = ( BYTE __huge * ) GlobalLock( fpHelpFileInfo->KeywordInfo.hKWBTREEInfo );
  
  /* Get the KWBTREE header. */
  fpBTreeHdr = ( BTREEHEADER __huge * ) fpData;
  fpData = fpData + sizeof(BTREEHEADER);
  
  /* Save position of first page. */
  fpFirstPageLoc = fpData;
  
  /* Get the first leaf's page. */
  wLeafPage = GetFirstLeafPage( fpFirstPageLoc, fpBTreeHdr );
  
  /* 
  ** Goto correct leaf page. 
  */
  dwTotal = 0;
  do 
  {
    /* Go to the current leaf page. */
    fpData = fpFirstPageLoc + ( wLeafPage * BTREEPAGESIZE );
  
    /* Get the current leaf page's header. */
    CurrPageHdrPtr = ( BTREENODEHEADER __huge * ) fpData;
    fpData = fpData + sizeof(BTREENODEHEADER);
    
    /* If there is another page. */
    if( CurrPageHdrPtr->NextPage != -1 )
    { 
      /* Get its page number. */
      wLeafPage = CurrPageHdrPtr->NextPage;
    }
    
    /* How many so far. */
    dwTotal = (DWORD) CurrPageHdrPtr->NEntries + dwTotal;
    
    /* In the right page for the keyword. */
    if( dwKeySelect < dwTotal )
    {
      break;
    }
  } 
  while( CurrPageHdrPtr->NextPage != -1 );

  /* Get key number for current key page. */  
  wKeyNumber = (WORD) (dwKeySelect - ( dwTotal - (WORD) CurrPageHdrPtr->NEntries ));
  
  /* Go to the correct keyword. */
  for( wCount = 0; wCount < wKeyNumber; wCount++ ) 
  {
    /* Get length of keyword string. */
    wStringSize = (WORD) _fstrlen( fpData ) + 1;
      
    /* Skip past keyword string. */
    fpData = fpData + wStringSize;
    
    /* Skip past count of associated topics in KWDATA. */
    fpData = fpData + sizeof( WORD );
    
    /* Skip past offset of associated topics in KWDATA. */
    fpData = fpData + sizeof( DWORD );
  }

  /* Get length of keyword string. */
  wStringSize = (WORD) _fstrlen( fpData ) + 1;
      
  /* Skip past keyword string. */
  fpData = fpData + wStringSize;
    
  /* Skip past count of associated topics in KWDATA. */
  wNumTopicsPtr = (WORD __huge *) fpData;
  wNumTopics = *wNumTopicsPtr;
  fpData = fpData + sizeof( WORD );
    
  /* Skip past offset of associated topics in KWDATA. */
  dwTopicsOffsetPtr = (DWORD __huge *) fpData;
  dwTopicsOffset = *dwTopicsOffsetPtr;

  /* Unlock the TTLBTREE table. */
  GlobalUnlock( fpHelpFileInfo->KeywordInfo.hKWBTREEInfo );


  /**************************************
  **
  **  Stage 2: Show topic titles.
  **
  **************************************/
  
  /* Lock the data. */
  TopicOffsetPtr = ( DWORD __huge * ) GlobalLock( fpHelpFileInfo->KeywordInfo.hKWDATAInfo );
  
  /* Move to correct offset. */
  TopicOffsetPtr = TopicOffsetPtr + (dwTopicsOffset / sizeof(DWORD));
  
  /* Get topic title for each offset and put in list box. */
  for( wCount = 0; wCount < wNumTopics; wCount++ ) 
  {
    /* Get title. */
    GetTopicTitle( hHelpFileInfo, *TopicOffsetPtr, szTopicTitle );

   /* Insert string into listbox. */
    SendMessage( hListBox, LB_ADDSTRING, 0, (LPARAM)(char __far *) szTopicTitle );

    /* Move to correct offset. */
    TopicOffsetPtr = TopicOffsetPtr + 1;
  }
     
  /* Unlock the data. */
  GlobalUnlock( fpHelpFileInfo->KeywordInfo.hKWDATAInfo );
  
  /* Unlock help file info. */
  GlobalUnlock( hHelpFileInfo );
  
  /* Save offset into KWDATA. */
  *dwKWDATAOffset = dwTopicsOffset;

  /* Success. */
  return( TRUE );
}
  



/***************************************************
*
* Get the topic offset for the selected a keyword's
* topic.
*
****************************************************/
DWORD __far __pascal GetKeywordTopicOffset( HGLOBAL hHelpFileInfo, DWORD dwKWDATAOffset, DWORD dwTopicIndex )
{
  FPHLPFILEINFO fpHelpFileInfo;          /* Pointer to new help file info.      */
  
  DWORD __huge * TopicOffsetPtr;
  DWORD dwTopicOffset;
  
    
  /* Lock help info. */
  fpHelpFileInfo = (FPHLPFILEINFO) GlobalLock( hHelpFileInfo );

  /* Lock the data. */
  TopicOffsetPtr = ( DWORD __huge * ) GlobalLock( fpHelpFileInfo->KeywordInfo.hKWDATAInfo );
  
  /* Move to correct offset. */
  TopicOffsetPtr = TopicOffsetPtr + (dwKWDATAOffset / sizeof(DWORD)) + dwTopicIndex;
  
  /* Save the value. */
  dwTopicOffset = *TopicOffsetPtr;
  
  /* Unlock the data. */
  GlobalUnlock( fpHelpFileInfo->KeywordInfo.hKWDATAInfo );
  
  /* Unlock help file info. */
  GlobalUnlock( hHelpFileInfo );
  
  /* Success. */
  return( dwTopicOffset );
}
  


/***************************************************
*
* Sets the last selected item in the keyword
* list combo box. 
*
****************************************************/
void __far __pascal SetLastKeywordIndex( HGLOBAL hHelpFileInfo, DWORD dwIndex )
{
  FPHLPFILEINFO fpHelpFileInfo;          /* Pointer to new help file info.      */


  /* Lock help info. */
  fpHelpFileInfo = (FPHLPFILEINFO) GlobalLock( hHelpFileInfo );

  /* Save index. */
  fpHelpFileInfo->KeywordInfo.dwLastIndex = dwIndex;

  /* Unlock help info. */
  GlobalUnlock( hHelpFileInfo );
}


/***************************************************
*
* Gets the last selected item in the keyword
* list combo box. 
*
****************************************************/
DWORD __far __pascal GetLastKeywordIndex( HGLOBAL hHelpFileInfo )
{
  FPHLPFILEINFO fpHelpFileInfo;          /* Pointer to new help file info.      */
  DWORD dwIndex;
  

  /* Lock help info. */
  fpHelpFileInfo = (FPHLPFILEINFO) GlobalLock( hHelpFileInfo );

  /* Save index. */
  dwIndex = fpHelpFileInfo->KeywordInfo.dwLastIndex;

  /* Unlock help info. */
  GlobalUnlock( hHelpFileInfo );

  return( dwIndex );
}



/***************************************************
*
* Loads keyword data.

* Find a keyword in the keyword list. 
*
* If only one topic is associated with the keyword, 
* the function gets the topic's offset and returns
* the value of 1.
*
* If more than one topic is associated with the keyword, 
* the function gets the first topic's offset and returns
* the value number of associated topics.
*
*
****************************************************/
WORD __far __pascal GetKeyTopic( HWND hErrorWnd, HGLOBAL hHelpFileInfo, char __far * KeywordPtr, DWORD __far * dwTopicCharOffsetPtr )
{
  FPHLPFILEINFO fpHelpFileInfo;          /* Pointer to new help file info.      */

  BYTE __huge *        fpData;             /* Pointer to a data buffer. */
  BYTE __huge *        fpFirstPageLoc;     /* Pointer to a data buffer. */
  BTREEHEADER __huge * fpBTreeHdr;         /* B-Tree header pointer. */
  BTREENODEHEADER __huge * CurrPageHdrPtr; /* Leaf page header. */

  WORD wLeafPage;        /* Page number of data leaf. */
  WORD wCount;           /* Dummy counter var. */

  WORD wStringSize;      /* Size of a keyword string. */
  
  BOOL bFoundMatch;
  
  WORD  __huge * wNumTopicsPtr;
  WORD  wNumTopics;
  DWORD __huge * dwTopicsOffsetPtr;
  DWORD dwTopicsOffset;
  
  DWORD __huge * CurrOffsetPtr;


  /* Lock help info. */
  fpHelpFileInfo = (FPHLPFILEINFO) GlobalLock( hHelpFileInfo );

  /**************************************
  **
  **  Stage 1: Find keyword in ?WBTREE buffer.
  **
  **************************************/

  /* Lock the data. */
  fpData = ( BYTE __huge * ) GlobalLock( fpHelpFileInfo->KeywordInfo.hKWBTREEInfo );
  
  /* Get the KWBTREE header. */
  fpBTreeHdr = ( BTREEHEADER __huge * ) fpData;
  fpData = fpData + sizeof(BTREEHEADER);
  
  /* Save position of first page. */
  fpFirstPageLoc = fpData;
  
  /* Get the first leaf's page. */
  wLeafPage = GetLeafPageSTRING( fpFirstPageLoc, fpBTreeHdr, KeywordPtr );
  
  /* Have not found keyword. */
  bFoundMatch = FALSE;
  
  /* Go to the current leaf page. */
  fpData = fpFirstPageLoc + ( wLeafPage * BTREEPAGESIZE );
  
  /* Get the current leaf page's header. */
  CurrPageHdrPtr = ( BTREENODEHEADER __huge * ) fpData;
  fpData = fpData + sizeof(BTREENODEHEADER);
    
  /* For each entry in the leaf page. */
  for( wCount = 0; wCount < (WORD) CurrPageHdrPtr->NEntries; wCount++ ) 
  {
    /* Get length of keyword string. */
    wStringSize = (WORD) _fstrlen( fpData ) + 1;
      
    /* Keyword MATCH - Case insensitive. */
    if( _fstricmp( KeywordPtr, fpData ) == 0 )
    {
      /* Found a match. */
      bFoundMatch = TRUE;
        
      /* Stop. */
      break;
    }
      
    /* Skip past keyword string. */
    fpData = fpData + wStringSize;
    
    /* Skip past count of associated topics in KWDATA. */
    fpData = fpData + sizeof( WORD );
    
    /* Skip past offset of associated topics in KWDATA. */
    fpData = fpData + sizeof( DWORD );
  }

  /* No match found. */
  if( bFoundMatch == FALSE )
  {
    /* Unlock the KWBTREE table. */
    GlobalUnlock( fpHelpFileInfo->KeywordInfo.hKWBTREEInfo );

    /* Unlock help info. */
    GlobalUnlock( hHelpFileInfo );

    /* Failure. */
    return( 0 );
  }
  
  /* Skip past keyword string. */
  fpData = fpData + wStringSize;
    
  /* Skip past count of associated topics in KWDATA. */
  wNumTopicsPtr = (WORD __huge *) fpData;
  wNumTopics = *wNumTopicsPtr;
  fpData = fpData + sizeof( WORD );
    
  /* Skip past offset of associated topics in KWDATA. */
  dwTopicsOffsetPtr = (DWORD __huge *) fpData;
  dwTopicsOffset = *dwTopicsOffsetPtr;

  /* Unlock the TTLBTREE table. */
  GlobalUnlock( fpHelpFileInfo->KeywordInfo.hKWBTREEInfo );


  /**************************************
  **
  **  Stage 2: Get first topic's offset.
  **
  **************************************/
  
  /* Lock the data. */
  CurrOffsetPtr = ( DWORD __huge * ) GlobalLock( fpHelpFileInfo->KeywordInfo.hKWDATAInfo );
  
  /* Move to correct offset. */
  CurrOffsetPtr = CurrOffsetPtr + (dwTopicsOffset / sizeof(DWORD));
  
  /* Save the topic's char offset. */
  *dwTopicCharOffsetPtr = *CurrOffsetPtr;
  
  /* Unlock the data. */
  GlobalUnlock( fpHelpFileInfo->KeywordInfo.hKWDATAInfo );
  
  /* Unlock help file info. */
  GlobalUnlock( hHelpFileInfo );
  
  /* Success. */
  return( wNumTopics );
}


/***************************************************
*
* Initializes the keyword data structure.
*
****************************************************/
static void __far __pascal InitKeywordData
( 
  HGLOBAL hHelpFileInfo 
)
{ 
  FPHLPFILEINFO fpHelpFileInfo;          /* Pointer to new help file info.      */

  /* Lock help info. */
  fpHelpFileInfo = (FPHLPFILEINFO) GlobalLock( hHelpFileInfo );

  /* No data in the buffers. */
  fpHelpFileInfo->KeywordInfo.hKWBTREEInfo = NULL;
  fpHelpFileInfo->KeywordInfo.hKWDATAInfo = NULL;

  /* Unlock help info. */
  GlobalUnlock( hHelpFileInfo );

  /* No last keyword selected in help file's 'K' keyword table. */
  SetLastKeywordIndex( hHelpFileInfo, 0 );
}


/***************************************************
*
* Loads keyword data into memory. 
*
****************************************************/
BOOL __far __pascal LoadKeywordData( HWND hErrorWnd, char chKey, HGLOBAL hHelpFileInfo )
{
  FPHLPFILEINFO fpHelpFileInfo;          /* Pointer to new help file info.      */

  char          WHIFName[MAX_WHIF_NAME]; /* WHIF filenames. */
  FILEHEADER    FileHdr;                 /* Generic WHIFS file header.  */
  long          BeginPos;                /* Beginning position of a WHIFS file. */

  HFILE         hHelpFile;    /* Handle to the help file. */
  OFSTRUCT      OpenBuffer;

  RETVAL        RetVal;
  
  BYTE __huge * fpData;       /* Pointer to a data buffer. */


  /* Lock help info. */
  fpHelpFileInfo = (FPHLPFILEINFO) GlobalLock( hHelpFileInfo );
  
  /* Open the help file. */
  hHelpFile = OpenFile( fpHelpFileInfo->szFilePath, &OpenBuffer, OF_READ );
  if( hHelpFile == HFILE_ERROR )
  {
    /* Error message. */
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEOPEN, MB_ICONHAND | MB_OK );
  
    /* Unlock help info. */
    GlobalUnlock( hHelpFileInfo );
  
    /* Failure. */
    return( FALSE );
  }
  
  
  /**************************************
  **
  **  Stage 1: Load KWBTREE data into memory.
  **
  **************************************/

  /* If not already loaded. */
  if( fpHelpFileInfo->KeywordInfo.hKWBTREEInfo == NULL )
  {
    /* Make KWBTREE filename. */
    _fstrcpy( WHIFName, KWBTREE_FILE );
    WHIFName[1] = chKey;  
    
    /* Get starting position of file. */
    BeginPos = GetWHIFSFileStart( (FPWHIFSINFO) &(fpHelpFileInfo)->WHIFSInfo, WHIFName );
    if( BeginPos == 0 )
    { 
      /* Error message. */
      MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADKEYTABLE, MB_ICONHAND | MB_OK );
  
      /* Unlock help info. */
      GlobalUnlock( hHelpFileInfo );
  
      return FALSE;                                     
    }
  
    /* Use standard b-tree loading function for now. */
    RetVal = LoadBTreeFile( hErrorWnd, hHelpFile, BeginPos, &(fpHelpFileInfo->KeywordInfo).hKWBTREEInfo );
    if( RetVal != NO_ERROR )
    {
      /* Init. handle. */
      fpHelpFileInfo->KeywordInfo.hKWBTREEInfo = NULL;
      
      /* Unlock help info. */
      GlobalUnlock( hHelpFileInfo );
  
      /* Error. */
      return( FALSE );
    }
  }

  /**************************************
  **
  **  Stage 2: Load KWDATA data into memory.
  **
  **************************************/

  /* If not already loaded. */
  if( fpHelpFileInfo->KeywordInfo.hKWDATAInfo == NULL )
  {
    /* Make KWDATA filename. */
    _fstrcpy( WHIFName, KWDATA_FILE );
    WHIFName[1] = chKey;  
    
    /* Get starting position of file. */
    BeginPos = GetWHIFSFileStart( (FPWHIFSINFO) &(fpHelpFileInfo)->WHIFSInfo, WHIFName );
    if( BeginPos == 0 )
    { 
      /* Error message. */
      MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADKEYDATA, MB_ICONHAND | MB_OK );
  
      /* Unlock help info. */
      GlobalUnlock( hHelpFileInfo );
  
      return FALSE;                                     
    }
  
    /* Go to the beginning file position of the |KWDATA file. */
    if( _lseek( hHelpFile, BeginPos, SEEK_SET ) == -1 )
    {
      MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
  
      /* Unlock help info. */
      GlobalUnlock( hHelpFileInfo );
  
      return FALSE;                                     
    }
  
    /* Read the generic WHIFS file header that exists for each WHIFS file. */
    if( _lread( hHelpFile, &FileHdr, sizeof(FILEHEADER) ) != sizeof(FILEHEADER) ) 
    {
      MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
  
      /* Unlock help info. */
      GlobalUnlock( hHelpFileInfo );
  
      return FALSE;                                     
    }
  
    /*
    ** Allocate memory for the KWDATA. 
    **
    */
    if( !GlobalAllocMem( hErrorWnd, &(fpHelpFileInfo->KeywordInfo).hKWDATAInfo, FileHdr.FileSize ) )
    {
      /* Unlock help info. */
      GlobalUnlock( hHelpFileInfo );
  
      return FALSE;                                     
    }
  
    /* Lock the data. */
    fpData = ( BYTE __huge * ) GlobalLock( fpHelpFileInfo->KeywordInfo.hKWDATAInfo );
    
    /* Read the KWDATA file's data. */
    if( _hread( hHelpFile, fpData, FileHdr.FileSize ) != FileHdr.FileSize ) 
    {
      MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
  
      /* Free the data. */
      GlobalUnlock( fpHelpFileInfo->KeywordInfo.hKWDATAInfo );
      GlobalFree( fpHelpFileInfo->KeywordInfo.hKWDATAInfo );
    
      /* Unlock help info. */
      GlobalUnlock( hHelpFileInfo );
  
      return FALSE;                                     
    }
  
    /* Unlock the data. */
    GlobalUnlock( fpHelpFileInfo->KeywordInfo.hKWDATAInfo );
  }
  
  /* Unlock help info. */
  GlobalUnlock( hHelpFileInfo );

  /* Close help file. */
  _lclose( hHelpFile );

  /* Success. */
  return( TRUE );
}  


/***************************************************
*
* Frees the keyword data from memory. 
*
****************************************************/
void __far __pascal FreeKeywordData( HGLOBAL hHelpFileInfo )
{
  FPHLPFILEINFO fpHelpFileInfo;          /* Pointer to new help file info.      */


  /* Lock help info. */
  fpHelpFileInfo = (FPHLPFILEINFO) GlobalLock( hHelpFileInfo );

  /* Is there KWBTREE data to free. */
  if( fpHelpFileInfo->KeywordInfo.hKWBTREEInfo != NULL )
  {
    /* Use standard b-tree freeing function for now. */
    FreeBTreeFile( &(fpHelpFileInfo->KeywordInfo).hKWBTREEInfo );
    fpHelpFileInfo->KeywordInfo.hKWBTREEInfo = NULL;
  }

  /* Is there KWDATA data to free. */
  if( fpHelpFileInfo->KeywordInfo.hKWDATAInfo != NULL )
  {
    /* Use standard b-tree freeing function for now. */
    GlobalFree( fpHelpFileInfo->KeywordInfo.hKWDATAInfo );
    fpHelpFileInfo->KeywordInfo.hKWDATAInfo = NULL;
  }

  /* Unlock help info. */
  GlobalUnlock( hHelpFileInfo );
}


/***************************************************
*
* Determines if a keyword table exists. 
*
****************************************************/
BOOL __far __pascal KeywordTableExists( HGLOBAL hHelpFileInfo, char chKey )
{
  FPHLPFILEINFO fpHelpFileInfo;          /* Pointer to new help file info.      */
  char          WHIFName[MAX_WHIF_NAME]; /* WHIF filenames. */
  long int      BeginPos;
  BOOL          bExists;
  

  /* Lock help info. */
  fpHelpFileInfo = (FPHLPFILEINFO) GlobalLock( hHelpFileInfo );
  
  /* Make KWBTREE filename. */
  _fstrcpy( WHIFName, KWBTREE_FILE );
  WHIFName[1] = chKey;  
    
  /* Get starting position of file. */
  BeginPos = GetWHIFSFileStart( (FPWHIFSINFO) &(fpHelpFileInfo)->WHIFSInfo, WHIFName );
  if( BeginPos == 0 )
  { 
    /* Table does not exist. */
    bExists = FALSE;
  }
  else
  { 
    /* Table does exist. */
    bExists = TRUE;
  }
  
  /* Unlock help info. */
  GlobalUnlock( hHelpFileInfo );
  
  /* Success. */
  return( bExists );
}  


/********************************* |FONT FILE ROUTINES **********************************/

/***************************************************
*
* Loads the |FONT file's information into memory. 
*
****************************************************/
static RETVAL __far __pascal LoadFontFile( HWND hErrorWnd, HFILE hHelpFile, long BeginPos, FPFONTINFO fpFontInfo )
{
  FILEHEADER      FileHdr;            /* Generic WHIFS file header.  */
  FONTHEADER      FontHdr;            /* |FONT file header. */
  FONTDESC        FontDesc;           /* Font description information. */
  
  HGLOBAL         hFontFaces;         /* Buffer of font face strings. */
  char __far *    fpFontFaces;        /* Pointer to font face strings. */
  char __far *    fpFontFace;         /* Pointer to a single font face string. */
  UINT            FontFacesSize;      /* Size of font face strings buffer. */
  
  FPFONTLISTREC   fpFontListRec;      /* Font information stored in memory. */

  WORD            wCounter;           /* Simple counter var. */

  
  /* Go to the beginning file position of the |FONT file. */
  if( _lseek( hHelpFile, BeginPos, SEEK_SET ) == -1 )
  {
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
    return( ERR_OTHER );
  }

  /* Read the generic WHIFS file header that exists for each WHIFS file. */
  if( _lread( hHelpFile, &FileHdr, sizeof(FILEHEADER) ) != sizeof(FILEHEADER) ) 
  {
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
    return( ERR_OTHER );
  }

  /* Read the |FONT file header. */
  if( _lread( hHelpFile, &FontHdr, sizeof(FONTHEADER) ) != sizeof(FONTHEADER) ) 
  {
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
    return( ERR_OTHER );
  }

  /*
  ** Allocate memory for the font faces. 
  ** We only keep this in memory temporarily.
  */
  FontFacesSize = FontHdr.NumFonts * FONTFACESIZE;
  hFontFaces = GlobalAlloc( GHND, FontFacesSize );
  if( hFontFaces == NULL )
  {
    return( ERR_MEMORY );
  }

  /* Lock font face string buffer. */
  fpFontFaces = ( char __far * ) GlobalLock( hFontFaces );

  /* Read the font faces into the memory buffer. */
  if( _lread( hHelpFile, fpFontFaces, FontFacesSize ) != FontFacesSize ) 
  {
    /* Unlock font face string buffer. */
    GlobalUnlock( hFontFaces );
  
    /* Free font face string buffer. */
    GlobalFree( hFontFaces );
  
    /* Error message. */
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );

    return( ERR_OTHER );
  }


  /* Save font count. */
  fpFontInfo->wFontCount = FontHdr.NumDescriptors;

  /*
  ** Allocate memory for the font information. 
  **
  ** The 1 extra record will store the record count
  ** in the dwTopicCharOffset field.
  */
  fpFontInfo->hFontList = GlobalAlloc( GHND, fpFontInfo->wFontCount * sizeof( FONTLISTREC ) );
  if( fpFontInfo->hFontList == NULL )
  {
    /* Unlock font face string buffer. */
    GlobalUnlock( hFontFaces );
  
    /* Free font face string buffer. */
    GlobalFree( hFontFaces );
  
    return( ERR_MEMORY );
  }

  /* Lock font info. */
  fpFontListRec = ( FPFONTLISTREC ) GlobalLock( fpFontInfo->hFontList );

  for( wCounter = 0; wCounter < fpFontInfo->wFontCount; wCounter++ )
  {
    /* Read the font description. */
    if( _lread( hHelpFile, &FontDesc, sizeof(FONTDESC) ) != sizeof(FONTDESC) ) 
    {
      /* Unlock font info. */
      GlobalUnlock( fpFontInfo->hFontList );
    
      /* Free font info. */
      FreeFontInfo( fpFontInfo );

      /* Unlock font face string buffer. */
      GlobalUnlock( hFontFaces );
    
      /* Free font face string buffer. */
      GlobalFree( hFontFaces );

      MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
      return( ERR_OTHER );
    }

    /* Save font's information. */
    fpFontFace = fpFontFaces + ( FontDesc.FontName * FONTFACESIZE );
    _fmemcpy( fpFontListRec->szFontName, fpFontFace, FONTFACESIZE );
    
    fpFontListRec->Attributes = FontDesc.Attributes;        
    fpFontListRec->PointSize  = FontDesc.HalfPoints / 2;         
    fpFontListRec->FontFamily = FontDesc.FontFamily;      
    fpFontListRec->fgTextColor = RGB(FontDesc.FGRGB[0], FontDesc.FGRGB[1], FontDesc.FGRGB[2]);

    /* Goto next font list record. */
    fpFontListRec++;
  }

  /* Unlock font info. */
  GlobalUnlock( fpFontInfo->hFontList );

  /* Unlock font face string buffer. */
  GlobalUnlock( hFontFaces );

  /* Free font face string buffer. */
  GlobalFree( hFontFaces );

  /* Success. */
  return( NO_ERROR );
}


BOOL __far __pascal GetFontListRec( FPHLPFILEINFO fpHelpFileInfo, WORD wFontNumber, FPFONTLISTREC fpSaveRec )
{
  FPFONTLISTREC fpFontListRec;
  
  
  /* Lock font info. */
  fpFontListRec = ( FPFONTLISTREC ) GlobalLock( fpHelpFileInfo->FontInfo.hFontList );
  
  /* Bad font number - Numbers start with 0. */
  if( wFontNumber >= fpHelpFileInfo->FontInfo.wFontCount )
  {
    /* Unlock font info. */
    GlobalUnlock( fpHelpFileInfo->FontInfo.hFontList );

    /* Failure. */
    return( FALSE );
  }
  
  /* Go to correct record. */
  fpFontListRec = fpFontListRec + wFontNumber;

  /* Get font record's data. */
  _fstrcpy( fpSaveRec->szFontName, fpFontListRec->szFontName );
  fpSaveRec->Attributes = fpFontListRec->Attributes;
  fpSaveRec->PointSize  = fpFontListRec->PointSize;
  fpSaveRec->FontFamily = fpFontListRec->FontFamily;
  fpSaveRec->fgTextColor = fpFontListRec->fgTextColor;
                        
  /* Unlock font info. */
  GlobalUnlock( fpHelpFileInfo->FontInfo.hFontList );

  /* Success. */
  return( TRUE );
}



/***************************************************
*
* Frees the |FONT info buffer.
*
****************************************************/
static void __far __pascal FreeFontInfo( FPFONTINFO fpFontInfo )
{
  /* Free the CONTEXT table. */
  GlobalFree( fpFontInfo->hFontList );
  
  fpFontInfo->hFontList = NULL;
}



/********************************* |BM# FILE ROUTINES **********************************/

/***************************************************
*
* Loads a picture file's information into memory.
*
* A picture file's name starts with "|bm" and
* ends in a number of the file.
*
****************************************************/
RETVAL __far __pascal LoadPictFile( HWND hErrorWnd, FPHLPFILEINFO fpHelpFileInfo, HGLOBAL __far * hPictData, WORD wPictFileNum )
{
  HFILE         hHelpFile;                  /* Handle to the help file. */
  OFSTRUCT      OpenBuffer;

  FILEHEADER    FileHdr;                    /* Generic WHIFS file header.  */
  long          BeginPos;                   /* File's beginning position. */
  char          szFileNum[5];               /* Number of picture file. */
  char          szWHIFName[MAX_WHIF_NAME];  /* Name of picture file. */
  
  BYTE __huge * fpPictData;         /* Picture file's data. */
  

  /* Open the help file. */
  hHelpFile = OpenFile( fpHelpFileInfo->szFilePath, &OpenBuffer, OF_READ );
  if( hHelpFile == HFILE_ERROR )
  {
    /* Error message. */
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEOPEN, MB_ICONHAND | MB_OK );

    /* Failure. */
    return( ERR_OTHER );
  }

  /* Create picture file's WHIFS filename. */  
  itoa( (int) wPictFileNum, szFileNum, 10 );
  wsprintf( (LPSTR) szWHIFName, "%s%s", BITMAP_FILE_PREFIX, szFileNum );
  
  /* Get starting position of the picture file. */
  BeginPos = GetWHIFSFileStart( (FPWHIFSINFO) &(fpHelpFileInfo)->WHIFSInfo, szWHIFName );
  if( BeginPos == 0 )
  { 
    /* Failure. */

    /* Close help file. */
    _lclose( hHelpFile );
  
    /* Error message. */
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_FINDINTERNALFILE, MB_ICONHAND | MB_OK );

    return( ERR_OTHER );
  }
  
  /* Go to the beginning file position of the picture file. */
  if( _lseek( hHelpFile, BeginPos, SEEK_SET ) == -1 )
  {
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );

    /* Close help file. */
    _lclose( hHelpFile );
  
    return( ERR_OTHER );
  }

  /* Read the generic WHIFS file header that exists for each WHIFS file. */
  if( _lread( hHelpFile, &FileHdr, sizeof(FILEHEADER) ) != sizeof(FILEHEADER) ) 
  {
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );

    /* Close help file. */
    _lclose( hHelpFile );
  
    return( ERR_OTHER );
  }

  /* Allocate space for the picture's data. */
  *hPictData = GlobalAlloc( GHND, FileHdr.FileSize );
  if( *hPictData == NULL )
  {
    /* Close help file. */
    _lclose( hHelpFile );
  
    return( ERR_MEMORY );
  }

  /* Lock the picture's data. */
  fpPictData = ( BYTE __huge * ) GlobalLock( *hPictData );

  /* Read the font description. */
  if( _hread( hHelpFile, fpPictData, FileHdr.FileSize ) != FileHdr.FileSize ) 
  {
    /* Unlock the picture's data. */
    GlobalUnlock( *hPictData );
    
    /* Free the picture's data. */
    GlobalFree( *hPictData );

    /* Close help file. */
    _lclose( hHelpFile );
  
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
    return( ERR_OTHER );
  }

  /* Unlock the picture's data. */
  GlobalUnlock( *hPictData );
    
  /* Close help file. */
  _lclose( hHelpFile );

  /* Success. */
  return( NO_ERROR );
}



/**************************** TOPIC DATA STRUCTURE ROUTINES ***************************/

/***************************************************
*
* Open the topic data stream.
*
****************************************************/
RETVAL __far __pascal OpenTopicDataStream
( 
  TOPICSTREAMSTRUCT __far * TopicStreamPtr, 
  HWND hErrorWnd,
  DWORD dwTopicCharOffset, 
  FPHLPFILEINFO fpHelpFileInfo, 
  BOOL __far * bNonScrollDataPtr, 
  long int __far * PrevTopicCharOffsetPtr, 
  long int __far * NextTopicCharOffsetPtr 
)
{
  OFSTRUCT      OpenBuffer;       /* OpenFile() buffer. */
  long int      FileStart;        /* Beginning position of a WHIFS file. */

  DWORD           dwInitBlockNum;     /* Block containing topic data. */
  WORD            wInitBlockOffset;   /* Offset in block of topic data. */

  FILEHEADER      FileHdr;            /* Generic WHIFS file header.          */
  TOPICLINK       TopicLink;          /* Topic link data.                    */
  TOPICHEADER     TopicHeader;        /* Topic header data. */
  
  RETVAL          RetVal;             /* Custom return value. */
  

  
  /* Init. topic stream structure. */
  TopicStreamPtr->hHelpFile       = HFILE_ERROR;
  TopicStreamPtr->hFileDataBuffer = NULL;
  TopicStreamPtr->hText           = NULL;
  TopicStreamPtr->hCodes          = NULL;

  /* Open the help file. */
  TopicStreamPtr->hHelpFile = OpenFile( fpHelpFileInfo->szFilePath, &OpenBuffer, OF_READ );
  if( TopicStreamPtr->hHelpFile == HFILE_ERROR )
  {
    /* Error message. */
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEOPEN, MB_ICONHAND | MB_OK );

    /* Close the data stream. */
    CloseTopicDataStream( TopicStreamPtr );
    
    /* Failure. */
    return( ERR_OTHER );
  }

  /* Get starting position of the |TOPIC file. */
  FileStart = GetWHIFSFileStart( (FPWHIFSINFO) &(fpHelpFileInfo)->WHIFSInfo, TOPIC_FILE );
  if( FileStart == 0 )
  { 
    /* Close the data stream. */
    CloseTopicDataStream( TopicStreamPtr );
    
    /* Error message. */
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_FINDINTERNALFILE, MB_ICONHAND | MB_OK );

    return( ERR_OTHER );
  }

  /* See whether we are using compression or not. */
  TopicStreamPtr->bCompressed = GetCompressionStatus( (FPSYSTEMINFO) &(fpHelpFileInfo)->SystemInfo );

  /* Convert character offset into block/block offset values. */
  CharOffsetToBlock( dwTopicCharOffset, &dwInitBlockNum, &wInitBlockOffset );

  /* Go to the beginning file position of the |TOPIC file. */
  if( _lseek( TopicStreamPtr->hHelpFile, FileStart, SEEK_SET ) == -1 )
  {
    /* Close the data stream. */
    CloseTopicDataStream( TopicStreamPtr );
    
    /* Error message. */
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );

    return( ERR_OTHER );
  }

  /* Read the generic WHIFS file header that exists for each WHIFS file. */
  if( _lread( TopicStreamPtr->hHelpFile, &FileHdr, sizeof(FILEHEADER) ) != sizeof(FILEHEADER) ) 
  {
    /* Close the data stream. */
    CloseTopicDataStream( TopicStreamPtr );
    
    /* Error message. */
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );

    return( ERR_OTHER );
  }

  /*
  ** Allocate memory for a data buffer that we
  ** will hold the file data that we have read.
  */
  /* Allocate space for buffer for compressed data. */
  if( TopicStreamPtr->bCompressed ) 
  {
    /* 
    ** Assume 4k of compressed data will uncompress
    ** to 16k - that's being VERY generous. 
    */
    TopicStreamPtr->hFileDataBuffer =  GlobalAlloc( GHND, 4 * TOPICBLOCKSIZE );
  }

  /* Otherwise, allocate space for buffer for non-compressed data. */
  else 
  {
    /* No compression - data read in normally. */
    TopicStreamPtr->hFileDataBuffer =  GlobalAlloc( GHND, TOPICBLOCKSIZE );
  }
  
  /* Allocation eror. */
  if( TopicStreamPtr->hFileDataBuffer == NULL )
  {
    /* Close the data stream. */
    CloseTopicDataStream( TopicStreamPtr );
    
    return( ERR_MEMORY );
  }

  /* Lock file data buffer. */
  TopicStreamPtr->fpFileDataBuffer = (BYTE __far *) GlobalLock( TopicStreamPtr->hFileDataBuffer );
          
  /* Get the first location of topic data. */
  TopicStreamPtr->TopicStart = _tell( TopicStreamPtr->hHelpFile );

  /* Last file position of the WHIF TOPIC file. */
  TopicStreamPtr->LastFilePos = TopicStreamPtr->TopicStart + FileHdr.FileSize;
  
  /* How many blocks are in the file. */ 
  TopicStreamPtr->dwNumBlocksInFile = FileHdr.FileSize / TOPICBLOCKSIZEWITHHEADER;
  if( FileHdr.FileSize % TOPICBLOCKSIZEWITHHEADER )
  {
    /* Add one if there is a partial block in the file. */ 
    TopicStreamPtr->dwNumBlocksInFile = TopicStreamPtr->dwNumBlocksInFile + 1;
  }  

  /* 
  ** Goto the topic topiclink record at the
  ** position requested.
  **
  ** Fill the file data buffer with the topic's data.
  **
  */
  RetVal = GotoTopicData( hErrorWnd, 
                          TopicStreamPtr->hHelpFile, 
                          &dwInitBlockNum, 
                          wInitBlockOffset, 
                          TopicStreamPtr->fpFileDataBuffer, 
                          &(TopicStreamPtr)->BufferDataSize, 
                          &(TopicStreamPtr)->CurrBufferPos, 
                          TopicStreamPtr->TopicStart, 
                          TopicStreamPtr->bCompressed, 
                          TopicStreamPtr->LastFilePos, 
                          TopicStreamPtr->dwNumBlocksInFile );
            
  /* Error? */
  if( RetVal != NO_ERROR )
  {
    /* Close the data stream. */
    CloseTopicDataStream( TopicStreamPtr );
    
    return( RetVal );
  }

  /* Set the current block to the initial block. */
  TopicStreamPtr->CurrBlockNum = dwInitBlockNum;
  
  /* 
  ** The first topic link record is for the topic's
  ** header data.
  */
  RetVal = ReadTopicLink( hErrorWnd, 
                          TopicStreamPtr->hHelpFile, 
                          &(TopicStreamPtr)->CurrBlockNum, 
                          TopicStreamPtr->fpFileDataBuffer, 
                          &(TopicStreamPtr)->BufferDataSize, 
                          &(TopicStreamPtr)->CurrBufferPos, 
                          TopicStreamPtr->TopicStart, 
                          TopicStreamPtr->bCompressed, 
                          (BYTE __far *) &TopicLink, 
                          sizeof(TOPICLINK), 
                          TopicStreamPtr->LastFilePos, 
                          TopicStreamPtr->dwNumBlocksInFile );

  /* Error? */
  if( RetVal != NO_ERROR )
  {
    /* Close the data stream. */
    CloseTopicDataStream( TopicStreamPtr );

    return( RetVal );
  }

  /* If not the header record - problem. */
  if( TopicLink.RecordType != TL_TOPICHDR ) 
  {
    /* Close the data stream. */
    CloseTopicDataStream( TopicStreamPtr );

    return( ERR_OTHER );
  }
  
  /* 
  ** TopicLink.DataLen2 value assumes you are doing Phrase replacement.  
  */
  if( TopicStreamPtr->bCompressed )
  {
    TopicLink.DataLen2 = TopicLink.TopicLinkSize - TopicLink.DataLen1;
  }
  
  /* 
  ** The calculation for DataLen1 includes the space required
  ** for the TOPICLINK structure.  We're already past that position
  ** in the file buffer so its size can be subtracted.
  **
  */
  TopicLink.DataLen1 = TopicLink.DataLen1 - sizeof(TOPICLINK);

  /* Read Data1 - Topic header data.  */
  RetVal = ReadTopicBytes( hErrorWnd, 
                           TopicStreamPtr->hHelpFile, 
                           &(TopicStreamPtr)->CurrBlockNum, 
                           TopicStreamPtr->fpFileDataBuffer, 
                           &(TopicStreamPtr)->BufferDataSize, 
                           &(TopicStreamPtr)->CurrBufferPos, 
                           TopicStreamPtr->TopicStart, 
                           TopicStreamPtr->bCompressed, 
                           (BYTE __far *) &TopicHeader, 
                           TopicLink.DataLen1, 
                           TRUE, 
                           TopicStreamPtr->LastFilePos, 
                           TopicStreamPtr->dwNumBlocksInFile );

  /* Error? */
  if( RetVal != NO_ERROR )
  {
    /* Close the data stream. */
    CloseTopicDataStream( TopicStreamPtr );

    return( RetVal );
  }
        
  /* Skip Data2 - Topic title. */
  if( TopicLink.DataLen2 > 0 )
  {
    /* Skip Data2.  */
    RetVal = ReadTopicBytes( hErrorWnd, 
                             TopicStreamPtr->hHelpFile, 
                             &(TopicStreamPtr)->CurrBlockNum, 
                             TopicStreamPtr->fpFileDataBuffer, 
                             &(TopicStreamPtr)->BufferDataSize, 
                             &(TopicStreamPtr)->CurrBufferPos, 
                             TopicStreamPtr->TopicStart, 
                             TopicStreamPtr->bCompressed,  
                             NULL, 
                             TopicLink.DataLen2, 
                             FALSE,
                             TopicStreamPtr->LastFilePos, 
                             TopicStreamPtr->dwNumBlocksInFile );
    /* Error? */
    if( RetVal != NO_ERROR )
    {
      /* Close the data stream. */
      CloseTopicDataStream( TopicStreamPtr );

      return( RetVal );
    }
  }

  /* Get offsets for prev. and next topic in Browse sequence */
  if( TopicHeader.BrowsePrev == -1 )
  {
    *PrevTopicCharOffsetPtr = -1;
  }
  else
  {
    *PrevTopicCharOffsetPtr = (long int) ExtendToChar( (DWORD) TopicHeader.BrowsePrev);
  }
  if( TopicHeader.BrowseNext == -1 )
  {
    *NextTopicCharOffsetPtr = -1;
  }
  else
  {
    *NextTopicCharOffsetPtr = (long int) ExtendToChar( (DWORD) TopicHeader.BrowseNext);
  }

  /* See if we have a non-scrollable area. */
  if( TopicHeader.NonScrollOffset != -1 )
  {
    /* First data will be for non-scrollable. */
    *bNonScrollDataPtr = TRUE;        
    TopicStreamPtr->bFoundScrollOffset = FALSE;
    TopicStreamPtr->ScrollOffset = TopicHeader.ScrollOffset;      
  }
  else
  {
    /* First data will be for non-scrollable. */
    *bNonScrollDataPtr = FALSE;        
    TopicStreamPtr->bFoundScrollOffset = TRUE;
  }

  /* No error. */
  return( NO_ERROR );
}


/***************************************************
*
* Close the topic data stream.
*
****************************************************/
void __far __pascal CloseTopicDataStream
( 
  TOPICSTREAMSTRUCT __far * TopicStreamPtr
)
{
  /* Close help file. */
  if( TopicStreamPtr->hHelpFile != HFILE_ERROR )
  {
    _lclose( TopicStreamPtr->hHelpFile );
  }

  /* Free file buffer. */
  if( TopicStreamPtr->hFileDataBuffer != NULL )
  {        
    GlobalUnlock( TopicStreamPtr->hFileDataBuffer );
    GlobalFree( TopicStreamPtr->hFileDataBuffer );
  }

  /* Free topic text buffer. */
  if( TopicStreamPtr->hText != NULL )
  {
    GlobalFree( TopicStreamPtr->hText );
  }

  /* Free topic codes buffer. */
  if( TopicStreamPtr->hCodes != NULL )
  {
    GlobalFree( TopicStreamPtr->hCodes );
  }
}

 

/***************************************************
*
* Read in the next topiclink record for the topic. 
*
****************************************************/
RETVAL __far __pascal ReadTopicData
( 
  TOPICSTREAMSTRUCT __far * TopicStreamPtr, 
  HWND hErrorWnd,
  TOPICDATATYPE __far * TopicDataTypePtr,
  HGLOBAL __far * hTextPtr, 
  DWORD __far * dwTextSizePtr,
  HGLOBAL __far * hCodesPtr,
  BOOL __far * bMoreDataPtr
)
{
  TOPICLINK    TopicLink;          /* Topic link data.     */

  BYTE __far * fpLinkData1;        /* First set of data.   */
  BYTE __far * fpLinkData2;        /* Second set of data   */

  RETVAL       RetVal;             /* Custom return value. */
  
  
  /* Read in the next topiclink record for the topic. */
  RetVal = ReadTopicLink( hErrorWnd, 
                          TopicStreamPtr->hHelpFile, 
                          &(TopicStreamPtr)->CurrBlockNum, 
                          TopicStreamPtr->fpFileDataBuffer, 
                          &(TopicStreamPtr)->BufferDataSize, 
                          &(TopicStreamPtr)->CurrBufferPos, 
                          TopicStreamPtr->TopicStart, 
                          TopicStreamPtr->bCompressed, 
                          (BYTE __far *) &TopicLink, 
                          sizeof(TOPICLINK), 
                          TopicStreamPtr->LastFilePos, 
                          TopicStreamPtr->dwNumBlocksInFile );

  /* Error? */
  if( RetVal != NO_ERROR )
  {
    return( RetVal );
  }
  
  /* 
  ** TopicLink.DataLen2 value assumes you are doing Phrase replacement.  
  ** We only want to know the length of the topic text with the
  ** phrase codes left in and not replaced. Therefore we're going to 
  ** modify the value of DataLen2 to reflect the value without phrase 
  ** replacement.
  */
  if( TopicStreamPtr->bCompressed )
  {
    TopicLink.DataLen2 = TopicLink.TopicLinkSize - TopicLink.DataLen1;
  }
  
  /* 
  ** The calculation for DataLen1 includes the space required
  ** for the TOPICLINK structure.  We're already past that position
  ** in the file buffer so its size can be subtracted.
  **
  */
  TopicLink.DataLen1 = TopicLink.DataLen1 - sizeof(TOPICLINK);
    
  
  /* 
  ** Process the TOPICLINK structure's data
  ** based on the structure's TopicLink.RecordType
  ** value.
  */
  switch( TopicLink.RecordType )
  {
    /* 
    ** If the topiclink record's record type says its a 
    ** Topic Header record, we are at the next topic's
    ** topic header and have finished our work.
    */
    case TL_TOPICHDR:
    {
      /* 
      ** We have already read a header for the topic.
      ** This header is for a different topic and
      ** we are finished.
      */
      *bMoreDataPtr = FALSE;
      
      /* Success. */
      return( NO_ERROR );
    }
    
      
    /* 
    ** If the topiclink record's record type says its a 
    ** general topic data record or a record containing a
    ** table, read the text and codes.
    */
    case TL_GENERAL:
    case TL_TABLE:
    {
      /* If there is topic code data. */
      if( TopicLink.DataLen1 > 0 ) 
      {
        /* First time reading topic code data for this topic. */
        if( TopicStreamPtr->hCodes == NULL )
        {
          /* Allocate space for the display codes information.  */
          TopicStreamPtr->hCodes = GlobalAlloc( GHND, (DWORD) TopicLink.DataLen1 );
          if( TopicStreamPtr->hCodes == NULL )
          {
            return( ERR_MEMORY );
          }
      
          /* Save the size of the codes buffer. */
          TopicStreamPtr->dwCodesSize = (DWORD) TopicLink.DataLen1;
        }
          
        /* 
        ** Not first time reading topic code data. 
        ** Add this new data to the previous code data.
        */
        else
        {
          /* Is there space in the codes buffer? */
          if( (DWORD) TopicLink.DataLen1 > TopicStreamPtr->dwCodesSize )
          {
            /* Allocate space for the display codes information.  */
            TopicStreamPtr->hCodes = GlobalReAlloc( TopicStreamPtr->hCodes, (DWORD) TopicLink.DataLen1, GMEM_MOVEABLE | GMEM_ZEROINIT );
            if( TopicStreamPtr->hCodes == NULL )
            {
              return( ERR_MEMORY );
            }

            /* Save the size of the codes buffer. */
            TopicStreamPtr->dwCodesSize = (DWORD) TopicLink.DataLen1;
          }
        }
          
        /* Lock the codes buffer. */
        fpLinkData1 = ( BYTE __far * ) GlobalLock( TopicStreamPtr->hCodes );

        /* Read the display codes information.  */
        RetVal = ReadTopicBytes( hErrorWnd, 
                                 TopicStreamPtr->hHelpFile, 
                                 &(TopicStreamPtr)->CurrBlockNum, 
                                 TopicStreamPtr->fpFileDataBuffer, 
                                 &(TopicStreamPtr)->BufferDataSize, 
                                 &(TopicStreamPtr)->CurrBufferPos, 
                                 TopicStreamPtr->TopicStart, 
                                 TopicStreamPtr->bCompressed, 
                                 fpLinkData1, 
                                 TopicLink.DataLen1, 
                                 TRUE,
                                 TopicStreamPtr->LastFilePos, 
                                 TopicStreamPtr->dwNumBlocksInFile );
        /* Error? */            
        if( RetVal != NO_ERROR )
        {
          return( RetVal );
        }
          
        /* Unlock the codes buffer. */
        GlobalUnlock( TopicStreamPtr->hCodes );
      }
  

      /* If there is topic text data. */
      if( TopicLink.DataLen2 > 0 ) 
      {
        /* First time reading topic text data. */
        if( TopicStreamPtr->hText == NULL )
        {
          /* Allocate space for the text data.  */
          TopicStreamPtr->hText = GlobalAlloc( GHND, (DWORD) TopicLink.DataLen2 );
          if( TopicStreamPtr->hText == NULL )
          {
            return( ERR_MEMORY );
          }
    
          /* Save the size of the text buffer. */
          TopicStreamPtr->dwTextSize = (DWORD) TopicLink.DataLen2;
        }
          
        /* 
        ** Not first time reading topic text data. 
        ** Add this new data to the previous code data.
        */
        else
        {
          /* Is there space in the text buffer? */
          if( (DWORD) TopicLink.DataLen2 > TopicStreamPtr->dwTextSize )
          {
            /* Allocate space for the display text information.  */
            TopicStreamPtr->hText = GlobalReAlloc( TopicStreamPtr->hText, (DWORD) TopicLink.DataLen2, GMEM_MOVEABLE | GMEM_ZEROINIT );
            if( TopicStreamPtr->hText == NULL )
            {
              return( ERR_MEMORY );
            }

            /* Save the size of the codes buffer. */
            TopicStreamPtr->dwTextSize = (DWORD) TopicLink.DataLen2;
          }
        }
          
        /* Lock the text buffer. */
        fpLinkData2 = ( BYTE __far * ) GlobalLock( TopicStreamPtr->hText );

        /* Read the display codes information.  */
        RetVal = ReadTopicBytes( hErrorWnd, 
                                 TopicStreamPtr->hHelpFile, 
                                 &(TopicStreamPtr)->CurrBlockNum, 
                                 TopicStreamPtr->fpFileDataBuffer, 
                                 &(TopicStreamPtr)->BufferDataSize, 
                                 &(TopicStreamPtr)->CurrBufferPos, 
                                 TopicStreamPtr->TopicStart, 
                                 TopicStreamPtr->bCompressed, 
                                 fpLinkData2, 
                                 TopicLink.DataLen2, 
                                 TRUE,
                                 TopicStreamPtr->LastFilePos, 
                                 TopicStreamPtr->dwNumBlocksInFile );
        /* Error? */            
        if( RetVal != NO_ERROR )
        {
          return( RetVal );
        }
          
        /* Unlock the codes buffer. */
        GlobalUnlock( TopicStreamPtr->hText );
      }
      
      break;
    }
  
#if 0      
    /* 
    ** Unsupported topiclink record type.
    ** Skip data.
    */
    default: 
    {
      /* Skip Data1.  */
      RetVal = ReadTopicBytes( hErrorWnd, hHelpFile, &CurrBlockNum, fpFileDataBuffer, 
                              &BufferDataSize, &CurrBufferPos, TopicStart, bCompressed, 
                              NULL, TopicLink.DataLen1, FALSE,
                              LastFilePos, dwNumBlocksInFile );
      /* Error? */
      if( RetVal != NO_ERROR )
      {
        /* Free the file buffer. */
        GlobalUnlock( hFileDataBuffer );
        GlobalFree( hFileDataBuffer );
  
        /* Free memory allocated for topic code data. */
        GlobalUnlock( *hCodes );
        if( *hCodes != NULL ) GlobalFree( *hCodes );
          
        /* Free memory allocated for topic text data. */
        GlobalUnlock( *hText );
        if( *hText != NULL ) GlobalFree( *hText );
          
        return( RetVal );
      }
        
      /* If there is Data2 for this record. */
      if( TopicLink.DataLen2 > 0 )
      {
        /* Skip Data2.  */
        RetVal = ReadTopicBytes( hErrorWnd, hHelpFile, &CurrBlockNum, fpFileDataBuffer, 
                                &BufferDataSize, &CurrBufferPos, TopicStart, bCompressed,  
                                NULL, TopicLink.DataLen2, FALSE,
                                LastFilePos, dwNumBlocksInFile );
        /* Error? */
        if( RetVal != NO_ERROR )
        {
          /* Free the file buffer. */
          GlobalUnlock( hFileDataBuffer );
          GlobalFree( hFileDataBuffer );
      
          /* Free memory allocated for topic code data. */
          GlobalUnlock( *hCodes );
          if( *hCodes != NULL ) GlobalFree( *hCodes );
              
          /* Free memory allocated for topic text data. */
          GlobalUnlock( *hText );
          if( *hText != NULL ) GlobalFree( *hText );
              
          return( RetVal );
        }
      }
      
      break;
    }
#endif

  }
    
  /* 
  ** If there is a non-scrollable region and we're reading that data,
  ** at some point we should start reading the scrollable data.
  ** Will the next topiclink record be the start of the non-scrollable
  ** data.
  */
  if( TopicStreamPtr->bFoundScrollOffset != TRUE )
  { 
    /* Next topiclink record has the scrollable data. */
    if( TopicLink.NextTopicLink == TopicStreamPtr->ScrollOffset )
    {
      *bMoreDataPtr = FALSE;
    }
  }
        
  /* Save size of text buffer. */
  *dwTextSizePtr = TopicLink.DataLen2;

  /* Save handles to the data. */
  *hTextPtr       = TopicStreamPtr->hText; 
  *hCodesPtr      = TopicStreamPtr->hCodes;
  
  /* Save the type of topic data that it is. */  
  *TopicDataTypePtr = TopicLink.RecordType;
  
  /* No error. */
  return( NO_ERROR );
}



/***************************************************
*
* Free topic data.
*
****************************************************/
void __far __pascal FreeTopicInfo
( 
  HGLOBAL hScrollText, 
  HGLOBAL hScrollCodes, 
  HGLOBAL hNonScrollText, 
  HGLOBAL hNonScrollCodes 
)
{
  if( hScrollText != NULL ) GlobalFree( hScrollText );
  if( hScrollCodes != NULL ) GlobalFree( hScrollCodes );
  if( hNonScrollText != NULL ) GlobalFree( hNonScrollText );
  if( hNonScrollCodes != NULL ) GlobalFree( hNonScrollCodes );
}
  


/***************************************************
*
* Initializing the file reading buffer using a 
* Block Number/Block Offset.
*
****************************************************/
static RETVAL __far __pascal GotoTopicData
( 
  HWND hErrorWnd,
  HFILE hHelpFile, 
  DWORD __far * dwCurrBlockNum, 
  WORD wInitBlockOffset,
  BYTE __far * fpFileDataBuffer,                                         
  long __far * BufferDataSize,
  long __far * CurrBufferPos,
  long TopicStart,
  BOOL bCompressed,
  long  int LastFilePos,     
  DWORD dwNumBlocksInFile  
)
{                                             
  FPTOPICLINK       fpTopicLink;     /* Pointer to a topic's topiclink record.    */
  
  DWORD dwDataLen2Total;             /* Running tally of topiclink TL_GENERAL record's
                                        datalen2 field. */
  BOOL  bFoundTopic;                 /* Have we found the start of the desired topic's
                                        data? */
  RETVAL RetVal;                     /* Return value from function call. */
  
  long int PrevTopicHeaderPos;
  
  
  /*** 
  **** Step I: Read in block's data. 
  ***/                                                                           

  RetVal = ReadTopicBlock( hErrorWnd, hHelpFile, *dwCurrBlockNum, fpFileDataBuffer,                                         
                           BufferDataSize, CurrBufferPos, TopicStart, bCompressed, 
                           LastFilePos, dwNumBlocksInFile, FIRST_TOPICLINK );

  if( RetVal != NO_ERROR )
  {
    return( RetVal );
  }
  
  
  /*** 
  **** Step II: If we are not in block 0 and are looking for offset 0 in the block, 
  ****          verify that the current position begins with a TOPICLINK record of type 
  ****          TL_TOPICHDR. If not go back to the last block and position ourselves
  ****          at the last TOPICLINK record in that block that is of type TL_TOPICHDR.   
  ***/                                                                           
  
  /* If we didn't read in the first block in the file and we're looking for offset 0. */
  if( *dwCurrBlockNum != 0 && wInitBlockOffset == 0 )
  {
    /* Make a pointer to the topiclink structure at the current position. */
    fpTopicLink = (FPTOPICLINK) ( fpFileDataBuffer + *CurrBufferPos );
  
    /* No at a TOPICLINK record of type TL_TOPICHDR. */
    if( fpTopicLink->RecordType != TL_TOPICHDR )
    {
      /* Assume that the header began near the last TOPICLINK record in the last block. */
      
      /* Read previous block. */
      *dwCurrBlockNum = *dwCurrBlockNum - 1;
      
      /* Read in block's data. */
      RetVal = ReadTopicBlock( hErrorWnd, hHelpFile, *dwCurrBlockNum, fpFileDataBuffer,                                         
                               BufferDataSize, CurrBufferPos, TopicStart, bCompressed, 
                               LastFilePos, dwNumBlocksInFile, LAST_TOPICHDR );

      /* Now at the correct block/offset in the file buffer. */
      return( NO_ERROR );
    }
  }


  /*** 
  **** Step III: Move to position in buffer to begin reading topic's data. 
  ***/                                                                           

  /* 
  ** Initialize running tally of topiclink TL_GENERAL
  ** record's datalen2 field. 
  */
  dwDataLen2Total = 0;       
  
  /* 
  ** We have not found the start of the 
  ** desired topic's data. 
  */
  bFoundTopic = FALSE;

  /* 
  ** The position of the last TL_TOPICHDR structure 
  ** that we saw in the buffer.
  */
  PrevTopicHeaderPos = 0;
    
  /* Until we are at the correct TopicLink record file buffer. */
  while( *CurrBufferPos < *BufferDataSize )
  {
    /* Make a pointer to the topiclink structure at the current position. */
    fpTopicLink = (FPTOPICLINK) ( fpFileDataBuffer + *CurrBufferPos );
  
    /* 
    ** If the numbers equal, we are are at the topiclink
    ** record that holds the topic's TL_TOPICHDR info.
    */     
    if( dwDataLen2Total == (DWORD) wInitBlockOffset )
    {
      /* Found it!  */
      bFoundTopic = TRUE;

      /* Make a pointer to the topiclink structure at the current position. */
      fpTopicLink = (FPTOPICLINK) ( fpFileDataBuffer + *CurrBufferPos );
  
      /* Not at a TOPICLINK record of type TL_TOPICHDR. */
      while( fpTopicLink->RecordType != TL_TOPICHDR )
      {
        /* Get the position of the previous TOPICLINK record in the block. */
        *CurrBufferPos = ( fpTopicLink->PrevTopicLink & 0x3FFF ) 
                         - sizeof( TOPICBLOCKHEADER );
        
        /* Make a pointer to the topiclink structure at the current position. */
        fpTopicLink = (FPTOPICLINK) ( fpFileDataBuffer + *CurrBufferPos );
      }

      break;
    }
  
    /* If we are not yet at the correct TL_TOPICHDR topiclink record. */     
    else if( dwDataLen2Total < (DWORD) wInitBlockOffset )
    {
      /* 
      ** Only accumulate DataLen2 values for
      ** topiclink records of type TL_GENERAL
      */
      if( fpTopicLink->RecordType != TL_TOPICHDR )
      {
        /* Increment our DataLen2 counter. */
        dwDataLen2Total += (DWORD) fpTopicLink->DataLen2;
      } 
      
      /* 
      ** If we are at any TL_TOPICHDR topiclink record,
      ** save its buffer position in case we need to go back to it.
      */
      else if( fpTopicLink->RecordType == TL_TOPICHDR )
      {
        /* Increment our DataLen2 counter. */
        PrevTopicHeaderPos = *CurrBufferPos;
      } 
    }
    
    /* If we have passed the correct TL_TOPICHDR topiclink record. */     
    else if( dwDataLen2Total > (DWORD) wInitBlockOffset )
    {
      /* 
      ** Previous topiclink TL_TOPICHDR record was it. 
      ** Get its file buffer position. 
      */
      *CurrBufferPos = PrevTopicHeaderPos;
      bFoundTopic = TRUE;
      break;
    }

    /* 
    ** Move in buffer to a place past the current TOPICLINK record's
    ** structure data and any associated Data1 and Data2 data.
    */
    *CurrBufferPos = *CurrBufferPos + fpTopicLink->TopicLinkSize;

  }

  /* 
  ** If we went past the end of the file buffer
  ** and did not find the correct offset, assume
  ** that the last TL_TOPICHDR topiclink record
  ** is the correct offset.
  */
  if( ! bFoundTopic )
  {
    /* 
    ** Previous topiclink TL_TOPICHDR record was it. 
    ** Get its file buffer position. 
    */
    *CurrBufferPos = PrevTopicHeaderPos;
  }
  
  /* Now at the correct block/offset in the file buffer. */
  return( NO_ERROR );
}


/***************************************************
*
* Copy decompressed topic data from the file
* buffer ( where the file's data was moved after it
* was read and decompresed ) into the given buffer.
*
* If we are asking for more data than is left in the
* file buffer, read some more data from the file,
* decompress it, fill the file buffer back up, and
* then keep copying the data to the given buffer.
*
****************************************************/
static RETVAL __far __pascal ReadTopicBytes
(
  HWND hErrorWnd,
  HFILE hHelpFile, 
  DWORD __far * dwCurrBlockNum, 
  BYTE __far * fpFileDataBuffer,                                         
  long __far * BufferDataSize,
  long __far * CurrBufferPos,
  long TopicStart,
  BOOL bCompressed, 
  BYTE __far * fpDestBuffer, 
  long ReadNumBytes,
  BOOL bSaveDataToBuffer,
  long  int LastFilePos,     
  DWORD dwNumBlocksInFile  
) 
{
  RETVAL RetVal;                         /* Return value from function call. */
  long               BytesLeft;          /* # Bytes left to copy from file buffer. 
                                            to the destination buffer. */
  

  /* 
  ** Get the number of bytes to copy from 
  ** file buffer to the destination buffer. 
  */
  BytesLeft = ReadNumBytes;
  
  /* While we have bytes to copy... */  
  while( BytesLeft ) 
  {
    /* If there's not enough left in the buffer. */
    if( *CurrBufferPos + BytesLeft > *BufferDataSize )
    {
      /* Are we saving data to the buffer? */
      if( bSaveDataToBuffer )
      {
        /* Copy what's left in the buffer. */
        _fmemcpy( fpDestBuffer, fpFileDataBuffer + *CurrBufferPos, 
                  (size_t) (*BufferDataSize - *CurrBufferPos) );
      }
      
      /* Less to copy. */
      BytesLeft = BytesLeft - (*BufferDataSize - *CurrBufferPos);
      
      /* Move dest. buffer pointer position. */
      fpDestBuffer = fpDestBuffer + (*BufferDataSize - *CurrBufferPos);
      
      /* Read in next block. */
      *dwCurrBlockNum = *dwCurrBlockNum + 1;
      
      RetVal = ReadTopicBlock( hErrorWnd, hHelpFile, *dwCurrBlockNum, fpFileDataBuffer,                                         
                               BufferDataSize, CurrBufferPos, TopicStart, bCompressed, 
                               LastFilePos, dwNumBlocksInFile, INIT_POS );
    
      if( RetVal != NO_ERROR )
      {
        return( RetVal );
      }
    }
    
    /* Copy all of what we want from the buffer. */
    else
    {
      /* Are we saving data to the buffer? */
      if( bSaveDataToBuffer )
      {
        /* Copy what's left in the buffer. */
        _fmemcpy( fpDestBuffer, fpFileDataBuffer + *CurrBufferPos, (size_t) BytesLeft );
      }
      
      /* Increment buffer position. */
      *CurrBufferPos = *CurrBufferPos + BytesLeft;      
      
      /* Copied all. */
      break;
    }
  }

  /* Succcess. */
  return( NO_ERROR );
}


/***************************************************
*
* Copy decompressed TOPICLINK strucuture data from 
* the file buffer ( where the file's data was moved 
* after it was read and decompresed ) into the given
* buffer.
*
****************************************************/
static RETVAL __far __pascal ReadTopicLink
(
  HWND hErrorWnd,
  HFILE hHelpFile, 
  DWORD __far * dwCurrBlockNum, 
  BYTE __far * fpFileDataBuffer,                                         
  long __far * BufferDataSize,
  long __far * CurrBufferPos,
  long TopicStart,
  BOOL bCompressed, 
  BYTE __far * fpDestBuffer, 
  long ReadNumBytes,
  long  int LastFilePos,     
  DWORD dwNumBlocksInFile  
) 
{
  RETVAL RetVal;             /* Return value from function call. */
  long   BytesLeft;          /* # Bytes left to copy from file buffer. 
                                to the destination buffer. */
  

  /* 
  ** Get the number of bytes to copy from 
  ** file buffer to the destination buffer. 
  */
  BytesLeft = ReadNumBytes;
  
  /* While we have bytes to copy... */  
  while( BytesLeft ) 
  {
    /* If there's not enough left in the buffer. */
    if( *CurrBufferPos + BytesLeft > *BufferDataSize )
    {
      /* Copy what's left in the buffer. */
      _fmemcpy( fpDestBuffer, fpFileDataBuffer + *CurrBufferPos, 
                (size_t) (*BufferDataSize - *CurrBufferPos) );
      
      /* Less to copy. */
      BytesLeft = BytesLeft - (*BufferDataSize - *CurrBufferPos);
      
      /* Move dest. buffer pointer position. */
      fpDestBuffer = fpDestBuffer + (*BufferDataSize - *CurrBufferPos);
      
      /* Read in next block. */
      *dwCurrBlockNum = *dwCurrBlockNum + 1;
      
      RetVal = ReadTopicBlock( hErrorWnd, hHelpFile, *dwCurrBlockNum, fpFileDataBuffer,                                         
                               BufferDataSize, CurrBufferPos, TopicStart, bCompressed, 
                               LastFilePos, dwNumBlocksInFile, FIRST_TOPICLINK );
    
      if( RetVal != NO_ERROR )
      {
        return( RetVal );
      }
    }
    
    /* Copy all of what we want from the buffer. */
    else
    {
      /* Copy what's left in the buffer. */
      _fmemcpy( fpDestBuffer, fpFileDataBuffer + *CurrBufferPos, (size_t) BytesLeft );
      
      /* Increment buffer position. */
      *CurrBufferPos = *CurrBufferPos + BytesLeft;      
      
      /* Copied all. */
      break;
    }
  }

  /* Succcess. */
  return( NO_ERROR );
}



/***************************************************
*
* Read a block of data from the TOPIC file into a
* buffer.  Position the CurrBufferPos in the buffer
* at the first or last TOPICLINK record.
*
****************************************************/
static RETVAL __far __pascal ReadTopicBlock
(
  HWND hErrorWnd,
  HFILE hHelpFile, 
  DWORD dwCurrBlockNum, 
  BYTE __far * fpFileDataBuffer,                                         
  long __far * BufferDataSize,
  long __far * CurrBufferPos,
  long TopicStart,
  BOOL bCompressed, 
  long  int LastFilePos,     
  DWORD dwNumBlocksInFile,
  BLOCK_POS BlockPos
)
{
  
  TOPICBLOCKHEADER  CurrBlockHeader,  /* Header for the current 4K block. */
                    NextBlockHeader;  /* Header for the next 4K block. */
  FPTOPICLINK       fpTopicLink;      /* Pointer to a topic's topiclink record. */
  long int          Offset;           /* Offset position into file data buffer. */


  /******* 
  ******** Step I: Move to 4k block in file data and fill the file data buffer with data. 
  *******/
      
  /* Align ourselves at correct 4k block in the |TOPIC file. */
  if( _lseek( hHelpFile, TopicStart + ( TOPICBLOCKSIZEWITHHEADER * (dwCurrBlockNum) ), SEEK_SET ) == -1 )
  {
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
    return( ERR_OTHER );
  }
      
  /* Read the current 4K block's blockheader. */
  if( _lread( hHelpFile, &CurrBlockHeader, sizeof(TOPICBLOCKHEADER) ) != sizeof(TOPICBLOCKHEADER) ) 
  {
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
    return( ERR_OTHER );
  }
      
  /* If topic data is compressed block, decompress it. */
  if( bCompressed ) 
  {
    /* Read the compressed data in the current block. */
    Decompress( hErrorWnd, hHelpFile, TOPICBLOCKSIZE - 1, 0, fpFileDataBuffer, LastFilePos );
  }
        
  else
  {
    /* Otherwise, read the uncompressed data in the current 4K block. */
    if( _lread( hHelpFile, fpFileDataBuffer, (UINT) TOPICBLOCKSIZE ) == HFILE_ERROR ) 
    {
      MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
      return( ERR_OTHER );
    }
  }
      
      
  /****** 
  ******* Step II: Calculate file buffer data. 
  ******/
      
  /* If we have another block after the one we just read. */
  if( dwCurrBlockNum + 1 < dwNumBlocksInFile )
  {
    /* If topic data is compressed, align ourselves at next 4K block */
    if( bCompressed ) 
    {
      /* Align ourselves at next 4K block */
      if( _lseek( hHelpFile, TopicStart + ( TOPICBLOCKSIZEWITHHEADER * (dwCurrBlockNum + 1) ), SEEK_SET ) == -1 )
      {
        MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
        return( ERR_OTHER );
      }
    }
        
    /* Read the block header for the NEXT block. */     
    if( _lread( hHelpFile, &NextBlockHeader, sizeof(TOPICBLOCKHEADER) ) != sizeof(TOPICBLOCKHEADER) ) 
    {
      MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
      return( ERR_OTHER );
    }
        
    /* 
    ** Get position of the LAST topiclink structure in the file data buffer. 
    ** Don't need the block #, hence 3FFFh
    **
    ** Don't include size of TOPICBLOCKHEADER in calc. since its not in the 
    ** file data buffer.
    */
    Offset = (NextBlockHeader.LastTopicLink & 0x3FFF) - sizeof(TOPICBLOCKHEADER);
          
    /* Make a pointer to the last topiclink structure in the current block. */
    fpTopicLink = (FPTOPICLINK) ( fpFileDataBuffer + Offset );
                
    /* 
    ** Get the topic length data length and add it to the
    ** topiclink's starting position.
    ** 
    ** The BufferDataSize value is the total length of data needed by the
    ** topiclink records in the block. 
    */
    *BufferDataSize = Offset + fpTopicLink->TopicLinkSize;
              
    /* 
    ** If the first topiclink record in the next block does not start
    ** immediately after the block's TOPICBLOCKHEADER, then
    ** some of the data for the last topiclink record in the previous block
    ** ( the block that we just read into the file data buffer )
    ** is not in the file data buffer.  Adjust the buffer size to reflect this fact.
    */ 
    *BufferDataSize = *BufferDataSize - 
                      ( (NextBlockHeader.FirstTopicLink & 0x3FFF) - sizeof( TOPICBLOCKHEADER ) );
  }
  
  /* No more blocks... Assume that the entire buffer is used. */
  else
  {
    *BufferDataSize = TOPICBLOCKSIZE;
  }    



  /****** 
  ******* Step III: Set our initial position in the file data buffer.
  ******/
      
  /* Set the buffer position. */
  switch( BlockPos )
  {
    /* Beginning of the buffer. */
    case INIT_POS:
    {
      /* Initial file data buffer position. */
      *CurrBufferPos = 0;

      break;
    }
  

    /* Move to first TOPICLINK record in the block. */
    case FIRST_TOPICLINK:
    {
      /* 
      ** If the block says that the first TOPICLINK record's position is not directly
      ** after the block's TOPICBLOCKHEADER structure, then we have to find the position
      ** of the block's first TOPICLINK record ourselves.  This occurs when the last 
      ** TOPICLINK structure in the preceeding block stretched across the 4K block 
      ** boundary into our current block.  Fortunately, the TOPICBLOCKHEADER tells
      ** us the exact number of bytes and we can skip that many byte in the file data
      ** buffer ( minus the header size since its not in the file data buffer) .
      */
      if( (CurrBlockHeader.FirstTopicLink & 0x3FFF) != sizeof( TOPICBLOCKHEADER ) )
      {
        /* Initial file data buffer position. */
        *CurrBufferPos = (CurrBlockHeader.FirstTopicLink & 0x3FFF) - sizeof( TOPICBLOCKHEADER );
      }
      else
      {
        /* Initial file data buffer position. */
        *CurrBufferPos = 0;
      }

      break;
    }


    /* Move to last TOPICLINK record in the block that is type TL_TOPICHDR. */
    case LAST_TOPICHDR:
    {
      /* Get the position of the last TOPICLINK record in the block. */
      *CurrBufferPos = ( NextBlockHeader.LastTopicLink & 0x3FFF ) 
                       - sizeof( TOPICBLOCKHEADER );
      
      /* Make a pointer to the topiclink structure at the current position. */
      fpTopicLink = (FPTOPICLINK) ( fpFileDataBuffer + *CurrBufferPos );
  
      /* No at a TOPICLINK record of type TL_TOPICHDR. */
      while( fpTopicLink->RecordType != TL_TOPICHDR )
      {
        /* Get the position of the last TOPICLINK record in the block. */
        *CurrBufferPos = ( fpTopicLink->PrevTopicLink & 0x3FFF ) 
                         - sizeof( TOPICBLOCKHEADER );
        
        /* Make a pointer to the topiclink structure at the current position. */
        fpTopicLink = (FPTOPICLINK) ( fpFileDataBuffer + *CurrBufferPos );
      }
      
      break;
    }
  }
  
  
  /* No error. */
  return( NO_ERROR );
}



/******************************** COMPRESSION ROUTINES **************************************/

/***************************************************
*
* Decompresses the data using Microsoft's LZ77
* derivative.    
*
* This version of the decompress routine reads
* file data and decompresses it.
*
* There is another version of the routine in the
* ELEMENTS.C file that reads the compressed data
* from a buffer and decompressed the data.  The
* function is called UncompressPict2().
*
* This routine and the ELEMENTS.C version
* used the same algorithm - just different
* sources.
*
****************************************************/
static long __far __pascal Decompress
( 
  HWND hErrorWnd, 
  HFILE hHelpFile, 
  long CompressedSize, 
  long MaxOutBytes, 
  char __far * Buffer, 
  long  int LastFilePos     
) 
{
  #define SET_SIZE 16
  
  long InBytes = 0;           /* How many bytes read in                    */
  long OutBytes = 0;          /* How many bytes written out                */
  BYTE BitMap, Set[SET_SIZE]; /* Bitmap and bytes associated with it       */
  short int  NumToRead;       /* Number of bytes to read for next group    */
  int  Counter, Index;        /* Going through next 8-16 codes or chars    */
  int  Length, Distance;      /* Code length and distance back in 'window' */
  char __far * CurrPos;       /* Where we are at any given moment          */
  char __far * CodePtr;       /* Pointer to back-up in LZ77 'window'       */
  
     
  /* Point to the first position in the decompressed buffer. */
  CurrPos = Buffer;
     
  /* While we haven't read everything. */
  while ( InBytes < CompressedSize )
  {
    /* If we are at the end of the file. */
    if( LastFilePos < _tell( hHelpFile ) )
    {
      /* Return the decompression size. */
      return( OutBytes );
    }

    /* Read bitmap. */
    if( _lread( hHelpFile, &BitMap, sizeof(BYTE) ) != sizeof(BYTE) ) 
    {
      MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
      return( OutBytes );
    }
  
    /* Get number to read. */
    NumToRead = (UINT) BytesToRead( BitMap );
  
    /* If we are being asked to read more than we should. */
    if( ( CompressedSize - InBytes) < NumToRead )
    { 
      /* Only read what we have left. */
      NumToRead = (short int) (CompressedSize - InBytes);   
    }
        
    /* Read in the compressed data. */
    if( _lread( hHelpFile, &Set, (UINT) NumToRead ) != (UINT) NumToRead ) 
    {
      MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
      return( OutBytes );
    }
    
    /* 
    ** The extra 1 added for file read of the BitMap byte above.  It is
    ** add here because if you add it prior to calculating NumToRead, you'll
    ** be in trouble.
    ** A lesson hard learned!
    */
    InBytes = InBytes + NumToRead + 1;
  
    /* Go through the compressed data and decode it. */
    for( Counter = 0, Index = 0; Counter < 8; Counter++ ) 
    {
      /* It's a code, so decode it and copy the data. */
      if( BitSet( BitMap, Counter ) ) 
      {
        Length = ( (Set[Index+1] & 0xF0) >> 4 ) + 3;
        Distance = ( 256 * (Set[Index+1] & 0x0F) ) + Set[Index] + 1;
            
        /* Pointer into decompress window. */
        CodePtr = CurrPos - Distance;   
            
        while( Length )
        { 
          /* 
          ** The |Phrases decompression asked for more than
          ** was needed.  If MaxBytes != 0, check before writing to
          ** buffer. Don't decompress more than we
          ** are supposed to decompress.  P.E.K.
          */
          if( MaxOutBytes != 0 )
          {
            if( (long) OutBytes == MaxOutBytes ) return OutBytes;
          }

          *CurrPos++ = *CodePtr++; 
          OutBytes++; 
          Length--; 
        } 
        Index += 2;  /* codes are 2 bytes */
      }
          
      /* Not a code. */
      else 
      { 
        /* 
        ** The |Phrases decompression asked for more than
        ** was needed.  If MaxBytes != 0, check before writing to
        ** buffer. Don't decompress more than we
        ** are supposed to decompress.  P.E.K.
        */
        if( MaxOutBytes != 0 )
        {
          if( (long) OutBytes == MaxOutBytes ) return OutBytes;
        }

        *CurrPos++ = Set[Index++]; 
        OutBytes++; 
      }
    }
  }
  
  /* Return the decompression size. */
  return( OutBytes );
} 


/***************************************************
*
* Decides how many bytes to read, depending on the
* number of bits set in the Bitmap.
*
****************************************************/
static short int __far __pascal BytesToRead( BYTE BitMap ) 
{
  short int TempSum,
            Counter;

  TempSum = 8;
  
  for( Counter = 0; Counter < 8; Counter ++ )
  {
    TempSum += BitSet( BitMap, Counter );
  }
  
  return TempSum;
}


/***************************************************
*
* Returns:
*
*    1 = bit is set.
*    0 = bit not set.
*
****************************************************/
static short int __far __pascal BitSet( BYTE BitMap, short int Bit ) 
{
  if( BitMap & ( 1 << Bit ) ) return 1;
  else return 0;
}


/****************************** OFFSET CONVERSION ROUTINES **********************************/

/***************************************************
*
* Gets an Extended offset into the |TOPIC file.
*
****************************************************/
static void __far _pascal ExtendOffsetToBlock( DWORD dwOffset, DWORD __far * dwBlockNum, WORD __far * wBlockOffset )
{
 /* 
 ** Get the 4k block number offset
 ** using the upper 18 bits.
 **
 ** 0x3FFFF verifies that the upper 14 bits of dwBlockNum are set to 0.
 */
 *dwBlockNum = (dwOffset >> 14) & 0x3FFFF;
 
 /* 
 ** Get the offset into the block's data
 ** from the lower 14 bits.
 **
 ** 0x3FFF verifies that the upper 18 bits of dwBlockNum are set to 0.
 */
 *wBlockOffset = ( WORD ) ( dwOffset & 0x3FFF );
}


/***************************************************
*
* Convert an Extended offset to Character offset.
*
****************************************************/
static DWORD __far _pascal ExtendToChar( DWORD dwExtendOffset )
{
 DWORD dwBlockNum;
 WORD  wBlockOffset;

 /* 
 ** Get the 4k block number offset
 ** using the upper 18 bits.
 **
 ** 0x3FFFF verifies that the upper 14 bits of dwBlockNum are set to 0.
 */
 dwBlockNum = (dwExtendOffset >> 14) & 0x3FFFF;
 
 /* 
 ** Get the offset into the block's data
 ** from the lower 14 bits.
 **
 ** 0x3FFF verifies that the upper 18 bits of dwBlockNum are set to 0.
 */
 wBlockOffset = ( WORD ) ( dwExtendOffset & 0x3FFF );

 /* Now make a char offset. */
 return( (dwBlockNum << 14) | wBlockOffset );
}


/***************************************************
*
* Gets an Character offset into the |TOPIC file.
*
****************************************************/
static void __far _pascal CharOffsetToBlock( DWORD dwOffset, DWORD __far * dwBlockNum, WORD __far * wBlockOffset )
{
 /* 
 ** Get the 4k block number offset
 ** using the upper 17 bits.
 **
 ** 0x1FFFF verifies that the upper 15 bits of dwBlockNum are set to 0.
 */
 *dwBlockNum = (dwOffset >> 15) & 0x1FFFF;
 
 /* 
 ** Get the offset into the block's data
 ** from the lower 15 bits.
 **
 ** 0x3FFF verifies that the upper 17 bits of dwBlockNum are set to 0.
 */
 *wBlockOffset = (WORD) dwOffset & 0x7FFF;
}


/********************************* BTREE ROUTINES ************************************/


/***************************************************
*
* Loads B-Tree info. into memory. 
*
****************************************************/
static RETVAL __far __pascal LoadBTreeFile( HWND hErrorWnd, HFILE hHelpFile, long BeginPos, HGLOBAL __far * hBtreeInfo )
{
  FILEHEADER           FileHdr;        /* Generic WHIFS file header.  */
  BTREEHEADER          BTreeHdr;       /* B-Tree header. */
  BTREEHEADER __huge * fpBTreeHdr;     /* B-Tree header pointer. */
  BYTE __huge *        fpData;         /* Pointer to data buffer. */
  
  
  /* Go to the beginning file position of the B-Tree file. */
  if( _lseek( hHelpFile, BeginPos, SEEK_SET ) == -1 )
  {
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
    return( ERR_OTHER );
  }

  /* Read the generic WHIFS file header that exists for each WHIFS file. */
  if( _lread( hHelpFile, &FileHdr, sizeof(FILEHEADER) ) != sizeof(FILEHEADER) ) 
  {
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
    return( ERR_OTHER );
  }

  /* Read the B-Tree header. */
  if( _lread( hHelpFile, &BTreeHdr, sizeof(BTREEHEADER) ) != sizeof(BTREEHEADER) ) 
  {
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
    return( ERR_OTHER );
  }

  /* If no entries, then return. */
  if( BTreeHdr.TotalBtreeEntries == 0 )
  {
    return( NO_ERROR );
  }

  /*
  ** Allocate memory for the B-Tree data.
  */
  *hBtreeInfo = GlobalAlloc( GHND, FileHdr.FileSize );
  if( *hBtreeInfo == NULL )
  {
    return( ERR_MEMORY );
  }

  /* Lock the data. */
  fpData = ( BYTE __huge * ) GlobalLock( *hBtreeInfo );

  /* Point to header space. */
  fpBTreeHdr = (BTREEHEADER __huge *) fpData;

  /* Copy B-Tree header to space. */
  *fpBTreeHdr = BTreeHdr;
  
  /* Move data pointer past header. */
  fpData = fpData + sizeof(BTREEHEADER);

  /* Read the B-Tree data. */
  if( _hread( hHelpFile, fpData, (long) (FileHdr.FileSize - sizeof(BTREEHEADER)) ) 
              != (long) (FileHdr.FileSize - sizeof(BTREEHEADER)) ) 
  {
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_BADFILEREAD, MB_ICONHAND | MB_OK );
    return( ERR_OTHER );
  }

  /* Unlock the B-Tree data. */
  GlobalUnlock( *hBtreeInfo );

  /* Success. */
  return( NO_ERROR );
}


/***************************************************
*
* Frees a b-tree loaded into memory by the
* LoadBTreeFile() function.
*
****************************************************/
static void __far __pascal FreeBTreeFile( HGLOBAL __far * hBtreeInfo )
{
  /* No table. */
  if( *hBtreeInfo == NULL ) return;
  
  /* Free the B-Tree table. */
  GlobalFree( *hBtreeInfo );
  
  /* Handle is invalid. */
  *hBtreeInfo = NULL;
}



/***************************************************
*
*  Get page number of correct data leaf. 
*  Used for CONTEXT and TTLBTREE.   
*
****************************************************/
static WORD __far __pascal GetLeafPageDWORD
( 
  BYTE __huge * FirstPageLocPtr, 
  BTREEHEADER __huge * BTreeHdrPtr, 
  DWORD dwKey
)
{
  BYTE __huge *            DataPtr; 
  WORD                     wCurrPage; 
  BTREENODEHEADER __huge * CurrPageHdrPtr;
  DWORD __huge *           CurrPagesKeyPtr;
  DWORD __huge *           NextPagesKeyPtr; 

  
  /* Get the first leaf's page. */
  wCurrPage = GetFirstLeafPage( FirstPageLocPtr, BTreeHdrPtr );
  
  /* 
  ** For each leaf page. 
  */
  do 
  {
    /* Go to the current leaf page. */
    DataPtr = FirstPageLocPtr + ( wCurrPage * BTREEPAGESIZE );
  
    /* Get the current leaf page's header. */
    CurrPageHdrPtr = ( BTREENODEHEADER __huge * ) DataPtr;
    DataPtr = DataPtr + sizeof(BTREENODEHEADER);

    /* Get the current leaf page's first key. */ 
    CurrPagesKeyPtr = (DWORD __huge *) DataPtr; 
     
    /* If there is another page. */
    if( CurrPageHdrPtr->NextPage != -1 )
    {
      /* Go to the next leaf page. */
      DataPtr = FirstPageLocPtr + ( CurrPageHdrPtr->NextPage * BTREEPAGESIZE );
    
      /* Skip the next leaf page's header. */
      DataPtr = DataPtr + sizeof(BTREENODEHEADER);
  
      /* Get the next leaf page's first key. */ 
      NextPagesKeyPtr = (DWORD __huge *) DataPtr; 

      /* 
      ** Could the key be in the current page?
      **
      ** 1. Yes, if the key is less than the next page's
      **    first entry and equal to or greater than the current
      **    page's first entry.
      **
      ** 2. Yes, if the values on the current page wrap around (the
      **    current page has the last highest value
      **    and first lowest value on it) and the key is less than the next
      **    page's first entry.
      */
      if( 
          /* Normal Compare. */
          ( dwKey >= *CurrPagesKeyPtr && dwKey < *NextPagesKeyPtr ) ||

          /* Page has highest and lowest values - key is positioned before values turnover. */
          ( dwKey >= *CurrPagesKeyPtr && *CurrPagesKeyPtr > *NextPagesKeyPtr ) ||

          /* Page has highest and lowest values - key is positioned after values turnover. */
          ( dwKey < *NextPagesKeyPtr && *CurrPagesKeyPtr > *NextPagesKeyPtr ) 
        )
      {
        break;
      }
      
      /* Goto the next page. */
      wCurrPage = (WORD) CurrPageHdrPtr->NextPage;
    }
  } 
  while( CurrPageHdrPtr->NextPage != -1 );

  /* 
  ** Return the page number of leaf that
  ** contains the key. 
  */
  return( wCurrPage );
}


/***************************************************
*
*  Get page number of correct data leaf. 
*  Used by KWBTREE.   
*
****************************************************/
static WORD __far __pascal GetLeafPageSTRING
( 
  BYTE __huge * FirstPageLocPtr, 
  BTREEHEADER __huge * BTreeHdrPtr, 
  char __huge * KeyStringPtr
)
{
  BYTE __huge *            DataPtr; 
  WORD                     wCurrPage; 
  BTREENODEHEADER __huge * CurrPageHdrPtr;
  char __huge *            CurrPagesKeyPtr;
  char __huge *            NextPagesKeyPtr; 

  short int Compare1, 
            Compare2, 
            Compare3;
            
  
  /* Get the first leaf's page. */
  wCurrPage = GetFirstLeafPage( FirstPageLocPtr, BTreeHdrPtr );
  
  /* 
  ** For each leaf page. 
  */
  do 
  {
    /* Go to the current leaf page. */
    DataPtr = FirstPageLocPtr + ( wCurrPage * BTREEPAGESIZE );
  
    /* Get the current leaf page's header. */
    CurrPageHdrPtr = ( BTREENODEHEADER __huge * ) DataPtr;
    DataPtr = DataPtr + sizeof(BTREENODEHEADER);

    /* Get the current leaf page's first key. */ 
    CurrPagesKeyPtr = (char __huge *) DataPtr; 
     
    /* If there is another page. */
    if( CurrPageHdrPtr->NextPage != -1 )
    {
      /* Go to the next leaf page. */
      DataPtr = FirstPageLocPtr + ( CurrPageHdrPtr->NextPage * BTREEPAGESIZE );
    
      /* Skip the next leaf page's header. */
      DataPtr = DataPtr + sizeof(BTREENODEHEADER);
  
      /* Get the next leaf page's first key. */ 
      NextPagesKeyPtr = (char __huge *) DataPtr; 

      /* 
      ** Could the key be in the current page?
      **
      ** 1. Yes, if the key is less than the next page's
      **    first entry and equal to or greater than the current
      **    page's first entry.
      **
      ** 2. Yes, if the values on the current page wrap around (the
      **    current page has the last highest value
      **    and first lowest value on it) and the key is less than the next
      **    page's first entry.
      */
      Compare1 = _fstricmp( KeyStringPtr, CurrPagesKeyPtr );
      Compare2 = _fstricmp( KeyStringPtr, NextPagesKeyPtr );
      Compare3 = _fstricmp( CurrPagesKeyPtr, NextPagesKeyPtr );
      if( 
          ( Compare1 >= 0  &&  Compare2 < 0 ) ||   /* Normal compare. */
          ( Compare2 < 0   &&  Compare3 > 0 ) ||   /* Page has highest and lowest values. */
          ( Compare1 >= 0  &&  Compare3 > 0 )      /* Page has highest and lowest values. */
        )
      {
        break;
      }

      /* Goto the next page. */
      wCurrPage = (WORD) CurrPageHdrPtr->NextPage;
    }
  } 
  while( CurrPageHdrPtr->NextPage != -1 );

  /* 
  ** Return the page number of leaf that
  ** contains the key. 
  */
  return( wCurrPage );
}


/***************************************************
*
*  Get the page numbr of the first data leaf page
*  in the CONTEXT b-tree.
*   
****************************************************/
static WORD __far __pascal GetFirstLeafPage
( 
  BYTE __huge * FirstPageLocPtr, 
  BTREEHEADER __huge * BTreeHdrPtr 
)
{
  short int     CurrLevel;
  BYTE __huge * DataPtr;
  WORD          wLeafPage;
  
  
  /* First B-Tree level. */
  CurrLevel = 1;

  /* Initialize leaf page number. */
  wLeafPage = 0;
  
  /* Go to root page of B-Tree. */
  DataPtr = FirstPageLocPtr + (BTreeHdrPtr->RootPage * BTREEPAGESIZE);
  
  /* While we are not at a leaf ( leaf nodes contain data ). */
  while( CurrLevel < BTreeHdrPtr->NLevels ) 
  {
    /* Skip the index node' header. */
    DataPtr = DataPtr + sizeof(BTREEINDEXHEADER);
    
    /* Get first page number listed in index node's entries. */
    wLeafPage = *((WORD __huge *) DataPtr);

    /* Go to the page (its in the next level). */
    DataPtr = FirstPageLocPtr + ( wLeafPage * BTREEPAGESIZE );

    /* We are at the next level. */ 
    CurrLevel++;
  }

  /* Return page number of first leaf node. */
  return( wLeafPage );
}


#if 0
/***************************************************
*
*  UNUSED FUNCTION.
*  Replaced by universal GetLeafPageDWORD() function.
*
*
*  Get page number of correct data leaf. 
*  Used for TTLBTREE.
*   
****************************************************/
static WORD __far __pascal GetTTLBTREELeafPage
( 
  BYTE __huge * fpFirstPageLoc, 
  BTREEHEADER __huge * BTreeHdr, 
  DWORD dwKey
)
{
  BTREEINDEXHEADER __huge * IndexHeader;
  short int    CurrLevel;
  BYTE __huge * fpData;

  WORD __huge * fpCurrPage; 
  DWORD __huge * fpNextPagesKey; 
  WORD wCounter;
  
  WORD wLeafPage;         
  
  
  /* First B-Tree level. */
  CurrLevel = 1;

  /* Initialize leaf page number. */
  wLeafPage = 0;
  
  /* Go to root page of B-Tree. */
  fpData = fpFirstPageLoc + (BTreeHdr->RootPage * BTREEPAGESIZE);
  
  /* While we are not at a leaf ( leaf nodes contain data ). */
  while( CurrLevel < BTreeHdr->NLevels ) 
  {
    /* Get the index node. */
    IndexHeader = (BTREEINDEXHEADER __huge *) fpData;
    fpData = fpData + sizeof(BTREEINDEXHEADER);
    
    /* 
    ** For each page listed in the index node,
    ** find the one to jump to at the next 
    ** B-Tree level.
    */
    for( wCounter = 0; wCounter < (WORD) IndexHeader->NEntries; wCounter++ )
    {
      /* Get next page listed in node. */
      fpCurrPage = (WORD __huge *) fpData;
      fpData = fpData + sizeof(WORD);

      /* Get next page's offset. */
      fpNextPagesKey = (DWORD __huge *) fpData;
      fpData = fpData + sizeof(DWORD);

      /* 
      ** If next page's offset to large, 
      ** offset is in the current page or a descedent 
      ** of the current page.
      */
      if( dwKey < *fpNextPagesKey )
      {
        break;
      }
    }           
    
    /* 
    ** Didn't find the key in the places that 
    ** we've looked in so far. 
    */
    if( wCounter == (WORD) IndexHeader->NEntries )
    {
      /* 
      ** If we didn't find it, it must be in the last page.
      ** Last entry in index entries is the last page.
      ** It doesn't have a matching key entry.
      */ 

      /* Use last page's number. */
      fpCurrPage = (WORD __huge *) fpData;
    }
    
    /* Get the next B-Tree level's page. */
    wLeafPage = *fpCurrPage;

    /* Go to page at the next level. */
    fpData = fpFirstPageLoc + ( wLeafPage * BTREEPAGESIZE );

    /* We are at the next level. */ 
    CurrLevel++;
  }
  
  /* Return correct leaf's page number. */
  return( wLeafPage );
}
#endif
