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
#ifndef ELEMENTS_H
#define ELEMENTS_H

/***********************************
**
**  System Includes
**
***********************************/
#include <windows.h>


/***********************************
**
**  App. Includes
**
***********************************/
#include "hlpfile.h"
#include "filebuff.h"
#include "winmem.h"


/***********************************
**
**  Shared Defines
**
***********************************/

/* Element list and list info. */
typedef struct tagELEMENTLIST
{
  WORD           wNumRecords;    /* Number of used records in element list. */
  WORD           wMaxRecords;    /* Total of number of used/unused records in element list. */
  HGLOBAL        hRecordList;   /* List of topic display elements. */

  HGLOBAL        hMemoryList;    /* Memory used for element list records. */
  FPMEMORYLIST   fpMemoryList;   /* Pointer to the memory list. */
}
ELEMENTLIST;
typedef ELEMENTLIST __far * FPELEMENTLIST;

/* 
** Record in an Element List. 
**
** Holds data for each element in a topic. 
** e.g. topic text string, phrase pointer, topic code.
*/
typedef struct tagTOPICELEMENT
{
  WORD    wRecType;     /* Type of record data. See display record type #defines. */
  
  /* 
  ** One of the fields is used depending on if the data in the
  ** record is global alloc'd or alloc'd using MyAlloc() function.
  */
  LPVOID  RecDataPtr;   /* Pointer to the record's data. */
  HGLOBAL hRecData;     /* Handle to the record's data. */
}
TOPICELEMENT;
typedef TOPICELEMENT __far * FPTOPICELEMENT;


/* Record types of TOPICELEMENT records. */
#define TE_TEXT          1     /* Text string. */

/* 
** Code formed from topic codes stream data. 
*/
#define TE_CHARBITMAP   11     /* Bitmap as next character. */
#define TE_LEFTBITMAP   12     /* Bitmap at left margin. */
#define TE_RIGHTBITMAP  13     /* Bitmap at right margin. */
#define TE_CHARMETA     14     /* MetaFile as next character. */
#define TE_LEFTMETA     15     /* MetaFile at left margin. */
#define TE_RIGHTMETA    16     /* MetaFile at right margin. */

#define TE_FONT         20     /* Font setting. */
#define TE_PAR          21     /* End of paragraph. */
#define TE_TAB          22     /* Tab. */
#define TE_HOTBEGIN     23     /* Beginning of a hotspot. */
#define TE_HOTEND       25     /* End of a hotspot. */
#define TE_PARD         26     /* New paragraph defaults. */
#define TE_LINE         27     /* Line break - not a new paragraph */

                       
/* Paragraph info. found at the beginning of the topic's code data. */
#define TE_SL        101     /* Minimum vertical line space between lines in a paragraph. */
#define TE_LI        102     /* Left indent. */
#define TE_FI        103     /* First line indent. */
#define TE_SA        104     /* Vertical space after paragraph. */
#define TE_SB        105     /* Vertical space before paragraph. */
#define TE_TX        106     /* Custom tab stop positions. */
#define TE_RI        107     /* Right indent. */
#define TE_QC        108     /* Center justified. */
#define TE_QR        109     /* Right justified. */
#define TE_KEEP      110     /* Keep. */
#define TE_BORDER    111     /* Drawing border lines around paragraph. */

#define TE_TABLEBEGIN  112   /* Start of table. */
#define TE_TABLEEND    113   /* End of table. */
#define TE_TABLECOLUMN 114   /* Next table's column starting. */


/* 
** The structures below define the unique data that is 
** stored in each type of TOPICELEMENT stucture listed 
** above.  A handle or a pointer to the unique data is 
** is stored in the TOPICELEMENT structure's 
** RecDataPtr/hRecData fields.
*/

/* Topic element data for the TE_BORDER type. */
typedef enum
{
  SINGLE_LINE,
  DOUBLE_LINE,
  SHADOW_LINE,
  DOTTED_LINE
}
BORDERTYPE;

typedef struct tagBORDERREC
{
  BOOL bTop;
  BOOL bLeft;
  BOOL bRight;
  BOOL bBottom;
  BOOL bThickLine;
  BORDERTYPE LineType;
}
BORDERREC;
typedef BORDERREC __far * FPBORDERREC;


/* 
** Topic element data for the TE_HOTBEG. 
*/

/* Types of hotspots. */
typedef enum
{
  HOTSPOT_JUMP,   /* Jump hotspot. */
  HOTSPOT_POPUP,  /* Popup hotspot. */
  HOTSPOT_MACRO,  /* Macro hotspot. */
  HOTSPOT_SEC     /* Secondary window hotspot. */
}
HOTSPOTTYPE;

/* Hotspot data record. */
typedef struct tagHOTSPOTREC
{
  HOTSPOTTYPE HotSpotType;      /* What type of hotspot is it. */
  char __far * HelpFilePathPtr; /* Path to hotspot's help file. */
  BOOL    bColorAndUnder;       /* Color and underline the hotspot - If hotspot is text. */
  BOOL    bVisible;             /* Is hotspot rectangle visible. */
  DWORD   dwHotSpotContext;     /* |CONTEXT table hash value - If hotspot is linked to a topic. */
  char __far * MacroPtr;        /* Macro string - If hotspot is linked to a macro. */
  WORD   wSecWndNum;            /* Secondary window's number. */
}
HOTSPOTREC;
typedef HOTSPOTREC __far * FPHOTSPOTREC;


/* SHED hotspot record. */
typedef struct tagSHEDREC
{
  RECT       PosInBitmap;    /* Left, top, width, height relative to picture. */
  HOTSPOTREC HotSpotRec;     /* General hotspot info. */
}
SHEDREC;
typedef SHEDREC __far * FPSHEDREC;


/* Topic element data for the TE_CHARBITMAP, TE_LEFTBITMAP, TE_RIGHTBITMAP type. */
typedef struct tagBITMAPREC
{
  HGLOBAL hBitmapInfo;
  HGLOBAL hBitmapData;

  WORD      wNumSHEDRecs; /* How many SHED hotspots does it have. */
  FPSHEDREC fpSHEDRecs;   /* Array of SHED hotspot records. */
}
BITMAPREC;
typedef BITMAPREC __far * FPBITMAPREC;


/* Topic element data for the TE_CHARMETA, TE_LEFTMETA, TE_RIGHTMETA type. */
typedef struct tagMETAREC
{
  HMETAFILE hMetaFile;
  char      szMetaFile[ _MAX_PATH ];
  WORD      wWidth;
  WORD      wHeight;

  WORD      wNumSHEDRecs;  /* How many SHED hotspots does it have. */
  FPSHEDREC fpSHEDRecs;    /* Array of SHED hotspot records. */
}
METAREC;
typedef METAREC __far * FPMETAREC;


/* Topic element data for the TE_TEXT type. */
typedef struct tagTEXTREC
{
  char __far * TextPtr;    /* Pointer to the text (NULL Appended). */
}
TEXTREC;
typedef TEXTREC __far * FPTEXTREC;


/* Topic element data for the TE_FONT type. */
typedef struct tagFONTREC
{
  WORD         wFontIndex;  /* Index number of font in the font descriptor table. */
  FONTLISTREC  FontInfo;    /* Information about font. */
}
FONTREC;
typedef FONTREC __far * FPFONTREC;


/* Topic element data for the TE_DEFFONT type. */
typedef struct tagDEFFONTREC
{
  WORD         wFontIndex;  /* Index number of DEFAULT font in the font descriptor table. */
  FONTLISTREC  FontInfo;    /* Information about font. */
}
DEFFONTREC;
typedef DEFFONTREC __far * FPDEFFONTREC;


/* Topic element data for the TE_SL type. */
typedef struct tagSLREC
{
  long int VertLineSpace;
}
SLREC;
typedef SLREC __far * FPSLREC;
                                               

/* Topic element data for the TE_LI type. */
typedef struct tagLIREC
{
  long int LeftIndent;
}
LIREC;
typedef LIREC __far * FPLIREC;

/* Topic element data for the TE_RI type. */
typedef struct tagRIREC
{
  long int RightIndent;
}
RIREC;
typedef RIREC __far * FPRIREC;
                                               

/* Topic element data for the TE_FI type. */
typedef struct tagFIREC
{
  long int FirstLineIndent;
}
FIREC;
typedef FIREC __far * FPFIREC;
                                               

/* Topic element data for the TE_SA type. */
typedef struct tagSAREC
{
  long int SpaceAfterParagraph;
}
SAREC;
typedef SAREC __far * FPSAREC;


/* Topic element data for the TE_SB type. */
typedef struct tagSBREC
{
  long int SpaceBeforeParagraph;
}
SBREC;
typedef SBREC __far * FPSBREC;


/* Topic element data for the TE_TX type. */
typedef long int __far * TABSTOPPTR;
typedef struct tagTXREC
{
  WORD       wTabCount;
  TABSTOPPTR TabStopsPtr;
}
TXREC;
typedef TXREC __far * FPTXREC;


/* Columns position data for a column in a table. */
typedef struct tagCOLUMNPOSREC
{
  long int Width;
  long int SpaceBetween;
}
COLUMNPOSREC;
typedef COLUMNPOSREC __far * COLUMNPOSPTR;

/* Topic element data for the TE_TABLE type. */
typedef struct tagTABLEREC
{
  BOOL         bFitColumns;       /* Is the \trqc flag set to on. */
  WORD         wNumColumns;       /* Number of columns in table's row. */
  COLUMNPOSPTR ColumnPosPtr;      /* Positions of each column in the table's row. */
}
TABLEREC;
typedef TABLEREC __far * FPTABLEREC;



/***********************************
**
**  Shared Function Prototypes
**
***********************************/

RETVAL __far __pascal CreateElementLists
( 
  HWND hErrorWnd,  
  DWORD dwTopicCharOffset,
  HGLOBAL __far * hNonScrollElementsPtr,
  HGLOBAL __far * hScrollElementsPtr,
  long int __far * PrevTopicCharOffsetPtr,
  long int __far * NextTopicCharOffsetPtr,
  HGLOBAL hHelpFileInfo
);

void __far __pascal DestroyElementList( HGLOBAL __far * hElementList );

BOOL __far __pascal RetrieveElementListRec
( 
  FPELEMENTLIST fpElementList, 
  FPTOPICELEMENT fpData, 
  WORD wElementNum 
);

long int __far __pascal CalcBitmapByteWidth
(
  long int nBitWidth,
  WORD wBitsPerPixel
);

#endif
                          


