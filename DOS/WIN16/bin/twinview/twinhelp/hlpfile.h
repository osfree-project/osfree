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
#ifndef HLPFILE_H                                                                                 
#define HLPFILE_H

/***********************************
**
**  System Includes
**
***********************************/

#include <windows.h>
#include <stdlib.h>

/***********************************
**
**  Shared Defines
**
***********************************/

/* Far pointer to a BYTE. */
typedef BYTE __far * FPBYTE;


/****************************** In-Memory Data Structures *************************/

/* 
** Unique ID that appears in the 
** .HLP file header's MagicNumber
** variable.
*/
#define HELP_MAGIC  0x00035F3FL


/* Help file Header record */
typedef struct tagHELPHEADER 
{
  DWORD   MagicNumber;      /* 0x00035F3F                */
  long    WHIFS;            /* Pointer to WHIFS header   */
  long    Negative1;
  long    FileSize;         /* Size of entire .HLP File  */
} 
HELPHEADER;
typedef HELPHEADER __far * FPHELPHEADER;


/* 
** Pages sizes for B-Tree pages.
*/
#define WHIFSPAGESIZE 1024L  /* WHIFS B-Tree page size. */
#define BTREEPAGESIZE 2048L  /* Page size for all other B-Trees. */

/* Standard WHIFS file's names. */
#define CONTEXT_FILE        "|CONTEXT"   
#define CTXOMAP_FILE        "|CTXOMAP"   
#define FONT_FILE           "|FONT"      
#define KWBTREE_FILE        "|KWBTREE"   
#define KWDATA_FILE         "|KWDATA"    
#define KWMAP_FILE          "|KWMAP"     
#define PHRASES_FILE        "|Phrases"   
#define SYSTEM_FILE         "|SYSTEM"    
#define TOPIC_FILE          "|TOPIC"     
#define TTLBTREE_FILE       "|TTLBTREE"  
#define BITMAP_FILE_PREFIX  "|bm"        

/* Information for a WHIF file. */
#define MAX_WHIF_NAME  31
typedef struct tagWHIFFILEINFO
{
  char FileName[ MAX_WHIF_NAME ];
  long FileOffset;
}
WHIFFILEINFO;
typedef WHIFFILEINFO __far * FPWHIFFILEINFO;

/* WHIF info. header. */
typedef struct tagWHIFSHEADER
{
  char    Magic[18];        /* Not exactly magic for some .MVB files   */
  char    Garbage[13];
  short   MustBeZero;       /* Probably First Leaf Page!!!             */
  short   NSplits;          /* Number of page split Btree has suffered */
  short   RootPage;         /* Page # of root page                     */
  short   MustBeNegOne;     /* Probably shows up when B-Tree is HUGE!! */
  short   TotalPages;       /* total # to 2Kb pages in Btree           */
  short   NLevels;          /* Number of levels in this Btree          */
  DWORD   TotalWHIFSEntries;
}
WHIFSHEADER;


/* Information from WHIFS Section. */
typedef struct tagWHIFSINFO
{
  WHIFSHEADER  WHIFSHeader;     /* WHIFS Header information. */
  DWORD        dwFileCount;
  HGLOBAL      hFileData;
}
WHIFSINFO;
typedef WHIFSINFO __far * FPWHIFSINFO;


/* File Header for WHIFS files */
typedef struct tagFILEHEADER 
{
  long    FilePlusHeader;  /* File size including this header */
  long    FileSize;        /* File size not including header  */
  char    TermNull;
}
FILEHEADER;


/* Keyword & TTL BTREE Headers - Slightly different than WHIFS B-tree Header.
   Both WHIFS and Keyword B-Trees use same leaf and index node headers.    */
typedef struct tagBTREEHEADER
{
  char    Unknown1[22];
  short   MustBeZero1;      /* Probably First Leaf page!!!         */
  short   NSplits;          /* # of page splits Btree has suffered */
  short   RootPage;         /* page #of root page                  */
  short   MustBeNegOne1;
  short   TotalPages;       /* total # of 2Kb pages in Btree       */
  short   NLevels;          /* # of levels in this Btree           */
  DWORD   TotalBtreeEntries;
}
BTREEHEADER;


/* Modified B-Tree Node header to handle a pointer to the page */
typedef struct tagBTREENODEHEADER
{
  WORD    Signature;      /* Signature word            */
  short   NEntries;       /* Number of entries         */
  short   PreviousPage;   /* Index of Previous Page    */
  short   NextPage;       /* Index of Next Page        */
}
BTREENODEHEADER;


/* Modified B-Tree Index header to handle a pointer to the page */
typedef struct tagBTREEINDEXHEADER
{
  WORD    Signature;      /* Signature byte            */
  short   NEntries;       /* Number of entries in node */
} 
BTREEINDEXHEADER;


/* Header for |SYSTEM file */
typedef struct tagSYSTEMHEADER
{
  BYTE    Magic;     /* 0x6C                  */
  BYTE    Version;   /* Version #             */
  BYTE    Revision;  /* Revision code         */
  BYTE    Always0;   /* Unknown               */
  WORD    Always1;   /* Always 0x0001         */
  DWORD   GenDate;   /* Date/Time that the help file was generated    */
  WORD    Flags;     /* Values seen: 0x0000 0x0004, 0x0008, 0x000A    */
} 
SYSTEMHEADER;
typedef SYSTEMHEADER __far * FPSYSTEMHEADER;


/* 
** SYSTEMHEADER Flags values.
*/
#define NO_COMPRESSION   0x0000      /* No Compression. */

/* 
** SYSTEMHEADER Revision values.
*/
#define COMPVER300     0x0F        /* Compiled with HC30. */
#define COMPVER310     0x15        /* Compiled with HC31. */

/* 
** Version of the compiler used. 
*/
typedef enum
{
  COMPILER_300,
  COMPILER_310
}
COMPILERVER;


/* 
** Help compiler 3.1 system record. 
** Multiple records can be read from file.
*/
typedef struct tagREADSYSTEMREC 
{
  WORD    RecordType;   /* Type of Data in record      */
  WORD    DataSize;     /* Size of RData               */
} 
READSYSTEMREC;
typedef READSYSTEMREC __far * FPREADSYSTEMREC;


/* Values that can be in the RecordType field of a SYSTEMREC structure. */  
#define HPJ_TITLE       0x0001      /* Title from .HPJ file            */
#define HPJ_COPYRIGHT   0x0002      /* Copyright notice from .HPJ file */
#define HPJ_CONTENTS    0x0003      /* Contents=CharOffset into |TOPIC file. */
#define HPJ_MACRO       0x0004      /* SData = 4 nulls if no macros    */
#define HPJ_ICON        0x0005      /* Alternate icon data. */
#define HPJ_SECWINDOW   0x0006      /* Secondary window info in .HPJ   */
#define HPJ_CITATION    0x0008      /* CITATION= under [OPTIONS]       */

#define MAX_TYPE     10
#define MAX_NAME      9
#define MAX_CAPTION  51

/*
** Secondary Window Record following type 0x0006 System Record 
*/
typedef struct tagSECWINDOW 
{
  WORD    Flags;                  /* Flags (See Below).     */
  char    Type[MAX_TYPE];         /* Type of window.        */
  char    Name[MAX_NAME];         /* Window name.           */
  char    Caption[MAX_CAPTION];   /* Caption for window.    */
  WORD    X;              /* X coordinate to start at.      */
  WORD    Y;              /* Y coordinate to start at.      */
  WORD    Width;          /* Width to create for.           */
  WORD    Height;         /* Height to create for.          */
  WORD    Maximize;       /* Maximize flag.                 */
  BYTE    Rgb[3];         /* RGB for scrollable region. */
  BYTE    Unknown1;
  BYTE    RgbNsr[3];      /* RGB for non scrollable region. */
  BYTE    Unknown2;
} 
SECWINDOW;
typedef SECWINDOW __far * FPSECWINDOW;


/* 
** Values for secondary window flags. 
*/
#define WSYSFLAG_TYPE       0x0001  /* Type is valid.              */
#define WSYSFLAG_NAME       0x0002  /* Name is valid.              */
#define WSYSFLAG_CAPTION    0x0004  /* Caption is valid.           */
#define WSYSFLAG_X          0x0008  /* X is valid.                 */
#define WSYSFLAG_Y          0x0010  /* Y is valid.                 */
#define WSYSFLAG_WIDTH      0x0020  /* Width is valid.             */
#define WSYSFLAG_HEIGHT     0x0040  /* Height is valid.            */
#define WSYSFLAG_MAXIMIZE   0x0080  /* Maximize is valid.          */
#define WSYSFLAG_RGB        0x0100  /* Rgb is valid.               */
#define WSYSFLAG_RGBNSR     0x0200  /* RgbNsr is valid.            */
#define WSYSFLAG_TOP        0x0400  /* On top was set in HPJ file. */


/* |Phrase WHIFS file header */
typedef struct tagPHRASEHEADER    
{
  WORD    NumPhrases;     /* Number of phrases in table                     */
  WORD    OneHundred;     /* 0x0100                                         */
  long    PhrasesSize;    /* Amount of space uncompressed phrases require.  */
} 
PHRASEHEADER;
typedef PHRASEHEADER __far * FPPHRASEHEADER;


/* Record from |CTXOMAP file. Created from the [MAP] section of .HPJ file */
typedef struct tagCTXOMAPREC
{
  long MapID;                /* #define topic ID used by application when calling WinHelp(). */
  long TopicOffset;          /* Character Offset of topic's data in the |TOPIC file. */
} 
CTXOMAPREC;
typedef CTXOMAPREC __far * FPCTXOMAPREC;


/* 
** Define the size of a topic block: 
**
**   4k Topic Block - sizeof(TOPICLINK)
*/
#define TOPICBLOCKSIZE  4084L
#define TOPICBLOCKSIZEWITHHEADER  4096L

/* 
** |TOPIC Block header - Header for a block of topic data. If
** uncompressed, there's only one of these at the beginning of the
** file. If the help file is compressed, then these occur in 4k
** increments. (e.g. 0x0000, 0x1000, 0x2000, 0x3000, 0x4000, etc. ) 
*/
typedef struct tagTOPICBLOCKHEADER 
{
  long LastTopicLink;   /* Offset of last TOPICLINK structure in previous block. */
  long FirstTopicLink;  /* Position of the first TOPICLINK strcuture relative to the 
                           begining of the block. */                   
  long LastTopicHeader; /* Offset of last topic header in previous block */
} 
TOPICBLOCKHEADER;

/* Linked list record for |TOPIC file */
typedef struct tagTOPICLINK 
{
  long int TopicLinkSize; /* Size of the topiclink record ( data + topiclink structure) */
  long int DataLen2;      /* Length of LinkData2              */
  long int PrevTopicLink; /* Position of last TOPICLINK strcuture relative to the 
                             begining of its block. */
  long int NextTopicLink; /* Position of next TOPICLINK structure relative to the 
                             begining of its block. */
  long int DataLen1;      /* Length of LinkData1 + size of TOPICLINK structure. */
  BYTE RecordType;        /* See below. */
} 
TOPICLINK;
typedef TOPICLINK __far * FPTOPICLINK;

/* Known TOPICLINK structure record types. */
#define TL_TOPICHDR    0x02  /* TOPICHEADER information. */
#define TL_GENERAL     0x20  /* Topic information. */
#define TL_TABLE       0x23  /* Table.                   */
typedef BYTE           TOPICDATATYPE;  /* Var. type to hold one of the above values. */

/* Topic header. LinkData1 for a TL_TOPICHDR record. */
typedef struct tagTOPICHEADER 
{
  DWORD    BlockSize;        /* Size of topic, including internal topic links.  */
  long int BrowsePrev;       /* Topic offset for prev topic in Browse 
                                sequence. Can be -1. */
  long int BrowseNext;       /* Topic offset for next topic in Browse 
                                sequence. Can be -1. */
  DWORD    TopicNum;         /* Topic Number(?)                                */
  long int NonScrollOffset;  /* Offset for topiclink containing start of 
                                non-scrollable data. Can be -1. */
  long int ScrollOffset;     /* Offset for topiclink containing start of 
                                scrollable data. Can be -1. */          
  DWORD    NextTopic;        /* Offset for next topiclink of type 0x02. */
} 
TOPICHEADER;
typedef TOPICHEADER __far * FPTOPICHEADER;
        

typedef struct tagTOPICSTREAMSTRUCT 
{
  HFILE    hHelpFile;         /* Handle to the help file. */
  long int TopicStart;        /* Start of |TOPIC file's topic data.  */ 
  long int LastFilePos;       /* Last file position of the WHIF TOPIC file. */
  BOOL     bCompressed;       /* Is the help file compressed? */
  DWORD    dwNumBlocksInFile; /* Number of blocks in the file. */

  HGLOBAL hText;              /* Buffer to use when reading a topic's text. */
  DWORD   dwTextSize;         /* What is the size of the text buffer. */
  DWORD   dwTextUsed;         /* How much of the text buffer is being used. */

  HGLOBAL hCodes;             /* Buffer to use when reading a topic's codes. */
  DWORD   dwCodesSize;         /* What is the size of the codes buffer. */
  DWORD   dwCodesUsed;         /* How much of the codes buffer is being used. */

  HGLOBAL         hFileDataBuffer;    /* Handle to the file data buffer. */
  BYTE __far *    fpFileDataBuffer;   /* Pointer to the file data buffer. */

  long int        BufferDataSize;     /* Size of data in file buffer.              */
  long int        CurrBufferPos;      /* Current position in the file data buffer. */
  long int        CurrBlockNum;       /* Current block number in topic data.       */

  long int        ScrollOffset;       /* What is the offset for scrollable topic data. */
  BOOL            bFoundScrollOffset; /* Have we found the offset for scrollable topic data. */
}
TOPICSTREAMSTRUCT;



/* Help file topic codes. */
#define TC_FONT        0x80      /* Font setting. */
#define TC_LINE        0x81      /* New line, but not end of paragraph. */
#define TC_PAR         0x82      /* End of paragraph. */
#define TC_TAB         0x83      /* Tab. */
#define TC_CHARPICT    0x86      /* Picture positioned as next character in line. */
#define TC_LEFTPICT    0x87      /* Picture positioned at left margin. */
#define TC_RIGHTPICT   0x88      /* Picture positioned at right margin. */
#define TC_HOTEND      0x89      /* End of a hotspot. */
#define TC_HOTBEG_C8   0xC8      /* Beginning of macro hotspot. */
#define TC_HOTBEG_2    0xE2      /* Beginning of a popup hotspot. */
#define TC_HOTBEG_3    0xE3      /* Beginning of a jump hotspot. */
#define TC_HOTBEG_6    0xE6      /* Beginning of a popup hotspot.
                                    Text in hotspot is not underlined or colored as hotspot. */
#define TC_HOTBEG_7    0xE7      /* Beginning of a jump hotspot.
                                    Text in hotspot is not underlined or colored as hotspot. */
#define TC_HOTBEG_A    0xEA      /* Beginning of a popup hotspot. 
                                    Topic is in another help file than the one being shown. */
#define TC_HOTBEG_B    0xEB      /* Beginning of a jump hotspot. 
                                    Topic is in another help file than the one being shown. */
#define TC_PARD        0xFF      /* Set paragraph formatting information to default values. */

        
/* Information from |SYSTEM file. */
typedef struct tagSYSTEMINFO
{
  COMPILERVER   CompilerVer;                /* What version of the compiler was used to build
                                               the help file. */
  BOOL          bCompressed;                /* Do we have at least block compression? */
  DWORD         dwContentsTopic;            /* The contents topic number. */
  char          szContentsFile[_MAX_PATH];  /* The contents topic number. */
  HGLOBAL       hTitleText;                 /* Text for caption of help window. */
  HICON         hIcon;                      /* Substitute icon for help window. */
  HGLOBAL       hMacroData;                 /* Macro data records. */
  HGLOBAL       hSecondWndData;             /* Second window data records. */
}
SYSTEMINFO;
typedef SYSTEMINFO __far * FPSYSTEMINFO;


/* 
** Records used to store multiple records
** of data.  i.e. second windows, macros, etc. 
**
*/
typedef struct tagSYSTEMREC 
{
  WORD         RecordType;   /* Type of Data in record.      */           
  WORD         DataSize;     /* Size of RData.               */
  HGLOBAL      hRecData;     /* Raw data (Icon, title, etc). */
} 
SYSTEMREC;
typedef SYSTEMREC __far * FPSYSTEMREC;

/* |SYSTEM record enumeration call-back function. */
typedef BOOL ( CALLBACK * SYSTEMRECPROC )( FPSYSTEMREC fpSysRec, LPARAM lParam );


typedef char __far * FPPHRASEINFO;


#define MAX_TOPIC_TITLE  128   /* Maximum length of a topic's title 
                                  (127 published by microsoft). */


/* Record for the |CONTEXT file. */
typedef struct tagCONTEXTREC
{
  DWORD dwHashValue;         /* Topic hash value. */
  DWORD dwTopicCharOffset;   /* Character Offset of topic's data in the |TOPIC file. */
} 
CONTEXTREC;
typedef CONTEXTREC __far * FPCONTEXTREC;


/* Header for |FONT file */
typedef struct tagFONTHEADER 
{
  WORD NumFonts;           /* Number of fonts in Font List */
  WORD NumDescriptors;     /* Number of font descriptors   */
  WORD DefDescriptor;      /* Default font descriptor      */
  WORD DescriptorsOffset;  /* Offset to descriptor list    */
} 
FONTHEADER;

#define FONTFACESIZE  20   /* Size of font face string in file. */

typedef struct tagFONTDESC 
{
  BYTE Attributes;         /* Font Attributes See values below */
  BYTE HalfPoints;         /* PointSize * 2                    */
  BYTE FontFamily;         /* Font Family. See values below    */
  BYTE FontName;           /* Number of font in Font List      */
  BYTE Unknown;
  BYTE FGRGB[3];           /* RGB values of foreground         */
  BYTE BGRGB[3];           /* background RGB Values (?? Not sure */
} 
FONTDESC;

/* Font Attributes */
#define FONT_NORM    0x00       /* Normal         */
#define FONT_BOLD    0x01       /* Bold           */
#define FONT_ITAL    0x02       /* Italics        */
#define FONT_UNDR    0x04       /* Underline      */
#define FONT_STRK    0x08       /* Strike Through */
#define FONT_DBUN    0x10       /* Dbl Underline  */
#define FONT_SMCP    0x20       /* Small Caps     */

/* Font Families */
#define FAM_MODERN   0x01
#define FAM_ROMAN    0x02
#define FAM_SWISS    0x03
#define FAM_TECH     0x03
#define FAM_NIL      0x03
#define FAM_SCRIPT   0x04
#define FAM_DECOR    0x05

/* Font table information stored in memory. */
typedef struct tagFONTINFOREC
{
  char szFontName[FONTFACESIZE];  /* Font face.        */
  BYTE Attributes;                /* Font attributes.  */
  BYTE PointSize;                 /* Point size.       */
  BYTE FontFamily;                /* Font Family.      */
  COLORREF fgTextColor;           /* Font foreground text color. */
} 
FONTLISTREC;
typedef FONTLISTREC __far * FPFONTLISTREC;

/* Font file information stored in memory. */
typedef struct tagFONTINFO
{
  WORD    wFontCount;
  HGLOBAL hFontList;
} 
FONTINFO;
typedef FONTINFO __far * FPFONTINFO;

/* Keyword information. */
typedef struct tagKEYWORDINFO
{
  DWORD   dwLastIndex;         /* Last index to keywords combo box for the
                                  'K' keyword list. */
  HGLOBAL hKWBTREEInfo;        /* |KWBTREE file information.    */
  HGLOBAL hKWDATAInfo;         /* |KWDATA file information.    */
} 
KEYWORDINFO;

#define MAX_KEYWORD_STRING  100


/* Help file info stored in memory. */
typedef struct tagHLPFILEINFO
{
  char        szFilePath[_MAX_PATH]; /* Help file's path.             */
  HELPHEADER  HelpHeader;            /* Header for help file.         */
  WHIFSINFO   WHIFSInfo;             /* WHIFS file information.       */
  SYSTEMINFO  SystemInfo;            /* |SYSTEM file information.     */
  HGLOBAL     hPhrasesInfo;          /* |Phrases file information.    */
  HGLOBAL     hCTXOMAPInfo;          /* |CTXOMAP file information.    */
  HGLOBAL     hTTLBTREEInfo;         /* |TTLBTREE file information.   */
  HGLOBAL     hContextInfo;          /* |CONTEXT file information.    */
  FONTINFO    FontInfo;              /* |FONT file information.       */
  KEYWORDINFO KeywordInfo;           /* |KWBTREE & |KWDATA file information. */
}
HLPFILEINFO;
typedef HLPFILEINFO __far * FPHLPFILEINFO;


/* 
** Return values for functions. 
**
*/
typedef enum
{
  NO_ERROR,
  ERR_MEMORY,
  ERR_BADCODE,
  ERR_OTHER
}
RETVAL;


/***********************************
**
**  Shared Function Prototypes
**
***********************************/

RETVAL __far __pascal LoadHelpFileInfo( HWND hErrorWnd, char __far * szFilePath, HGLOBAL __far * hHelpFileInfo );
void __far __pascal FreeHelpFileInfo( HGLOBAL hHelpFileInfo );

long __far __pascal GetWHIFSFileStart( FPWHIFSINFO fpWHIFSInfo, char __far * FileName );

RETVAL __far __pascal OpenTopicDataStream
( 
  TOPICSTREAMSTRUCT __far * TopicStreamPtr, 
  HWND hErrorWnd,
  DWORD dwTopicCharOffset, 
  FPHLPFILEINFO fpHelpFileInfo, 
  BOOL __far * bNonScrollDataPtr, 
  long int __far * PrevTopicCharOffsetPtr, 
  long int __far * NextTopicCharOffsetPtr 
);
void __far __pascal CloseTopicDataStream
( 
  TOPICSTREAMSTRUCT __far * TopicStreamPtr
);
RETVAL __far __pascal ReadTopicData
( 
  TOPICSTREAMSTRUCT __far * TopicStreamPtr, 
  HWND hErrorWnd,
  TOPICDATATYPE __far * TopicDataTypePtr,
  HGLOBAL __far * hTextPtr, 
  DWORD __far * dwTextSizePtr,
  HGLOBAL __far * hCodesPtr,
  BOOL __far * bMoreDataPtr
);


BOOL __far __pascal SetContentsTopic( HWND hWnd, HGLOBAL hHelpFileInfo, char __far * szContentsHelpFilePath, unsigned long int nContentsContext );
void __far __pascal GetContentsTopic( HGLOBAL hHelpFileInfo, char __far * szHelpFilePath, DWORD __far * dwTopicCharOffset );

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
);


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
);


HGLOBAL __far __pascal GetTitleText( HGLOBAL hHelpFileInfo );

BOOL __far __pascal GetCTXOMAPOffset( HWND hErrorWnd, HGLOBAL hHelpFileInfo, long TopicID, DWORD __far * dwTopicCharOffset );

char __far * __far __pascal GetPhrasePtr( FPPHRASEINFO fpPhraseInfo, WORD PhraseNum, WORD __far * wSize );

void __far __pascal GetTopicTitle( HGLOBAL hHelpFileInfo, DWORD dwTopicCharOffset, char __far * szTopicTitle );

BOOL __far __pascal GetContextOffset( HGLOBAL hHelpFileInfo, DWORD dwHashValue, DWORD __far * dwTopicCharOffset );

BOOL __far __pascal GetFontListRec( FPHLPFILEINFO fpHelpFileInfo, WORD wFontNumber, FPFONTLISTREC fpSaveRec );

RETVAL __far __pascal LoadPictFile( HWND hErrorWnd, FPHLPFILEINFO fpHelpFileInfo, HGLOBAL __far * hPictData, WORD wPictFileNum );

BOOL __far __pascal LoadKeywordData( HWND hErrorWnd, char chKey, HGLOBAL hHelpFileInfo );
void __far __pascal FreeKeywordData( HGLOBAL hHelpFileInfo );
BOOL __far __pascal ShowKeywords( HWND hListBox, char chKey, HGLOBAL hHelpFileInfo ); 
BOOL __far __pascal ShowKeywordTopics( HWND hListBox, DWORD dwKeySelect, HGLOBAL hHelpFileInfo, DWORD __far * dwKWDATAOffset );
DWORD __far __pascal GetKeywordTopicOffset( HGLOBAL hHelpFileInfo, DWORD dwKWDATAOffset, DWORD dwTopicIndex );
void __far __pascal SetLastKeywordIndex( HGLOBAL hHelpFileInfo, DWORD dwIndex );
DWORD __far __pascal GetLastKeywordIndex( HGLOBAL hHelpFileInfo );
WORD __far __pascal GetKeyTopic( HWND hErrorWnd, HGLOBAL hHelpFileInfo, char __far * KeywordPtr, DWORD __far * dwTopicCharOffsetPtr );
BOOL __far __pascal KeywordTableExists( HGLOBAL hHelpFileInfo, char chKey );

HICON __far __pascal GetHelpFileIcon( HGLOBAL hHelpFileInfo );

void __far __pascal EnumSysRecords( SYSTEMRECPROC fpCallback, HGLOBAL hRecordBuffer, LPARAM lParam );


#endif



