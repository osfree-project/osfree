/*    
	ChooseFont.c	1.18 common dialogs font dialog.
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

/*===== Include files =====*/
#include "platform.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "windows.h"
//#include "windowsx.h"
#include "commdlg.h"
#include "cderr.h"
#include "dlgs.h"
#include "CommdlgRC.h"
#include "Log.h"
#include "porting.h"

/*===== External Platform specific defines =====*/
#define WCFCALLBACK                        CALLBACK _export

/*===== Required defines =====*/
#define NUM_TRUETYPE_FONT_SIZES         16
#define NUM_FONTSTYLES                  4
#define NUM_COLORS                      16
#define CHOOSEFONTDIALOG								MAKEINTRESOURCE(1543)
/*=== Redefine standard windows control names to more meaningful names =====*/
#define WCFButtonApply                  psh3
#define WCFButtonHelp                   pshHelp
#define WCFCheckStrikeout               chx2
#define WCFCheckUnderline               chx1
#define WCFGroupEffects                 grp1
#define WCFGroupSample                  grp2
#define WCFTextFont                     stc1
#define WCFTextFontStyle                stc2
#define WCFTextSize                     stc3
#define WCFTextColor                    stc4
#define WCFTextSample                   stc5
#define WCFText                         stc6
#define WCFCBFontName                   cmb1
#define WCFCBFontStyle                  cmb2
#define WCFCBFontSize                   cmb3
#define WCFCBFontColor                  cmb4


/*===== Types and Structures =====*/

#define TError                          DWORD

typedef struct tagNEWLOGFONT
{
    int     lfHeight;
    int     lfWidth;
    int     lfEscapement;
    int     lfOrientation;
    int     lfWeight;
    BYTE    lfItalic;
    BYTE    lfUnderline;
    BYTE    lfStrikeOut;
    BYTE    lfCharSet;
    BYTE    lfOutPrecision;
    BYTE    lfClipPrecision;
    BYTE    lfQuality;
    BYTE    lfPitchAndFamily;
    char    lfFaceName[LF_FACESIZE];
    char    lfFullName[2*LF_FACESIZE];
    char    lfStyle[LF_FACESIZE];
} NEWLOGFONT;

/*=== Font size selections for True Type fonts ===*/
typedef struct
{
  char                        String [ 3 ]; /*-- Font size string ie. "10" --*/
  int                         Size;         /*-- Value of font size ie. 10 --*/
}
WCFFONTSIZESELCTIONS;

/*=== User data passed to EnumFontFamilies ===*/
typedef struct
{
  HDC                         hDC;          /*-- hDC to be used --*/
  HWND                        hDialogWnd;   /*-- window handle of font
                                                  dialog --*/
  BOOL                        PrinterFonts; /*-- if enum printer
                                                  fonts --*/
}
WCFENUMFONTDATA;

/*=== Color selections ===*/
typedef struct
{
  WORD                        StringID;     /*-- Color name ie. "Black" --*/
  COLORREF                    Color;        /*-- RGB value ie. RGB(0,0,0) --*/
}
WCFCOLORDATA;


/*===== Globals =====*/

/*=== True Type Font sizes ===*/
WCFFONTSIZESELCTIONS          TrueTypeFontSizes [ NUM_TRUETYPE_FONT_SIZES ] =
                              { { "8\0", 8 },
                                { "9\0", 9 },
                                { "10\0", 10 },
                                { "11\0", 11 },
                                { "12\0", 12 },
                                { "14\0", 14 },
                                { "16\0", 16 },
                                { "18\0", 18 },
                                { "20\0", 20 },
                                { "22\0", 22 },
                                { "24\0", 24 },
                                { "26\0", 26 },
                                { "28\0", 28 },
                                { "36\0", 36 },
                                { "48\0", 48 },
                                { "72\0", 72 }
                              };

/*=== Color selections ===*/
WCFCOLORDATA                  ColorData [ NUM_COLORS ] =
                              { { WCF_IDS_COLORBLACK,   RGB (   0,   0,   0 ) },
                                { WCF_IDS_COLORMAROON,  RGB ( 128,   0,   0 ) },
                                { WCF_IDS_COLORGREEN,   RGB (   0, 128,   0 ) },
                                { WCF_IDS_COLOROLIVE,   RGB ( 128, 128,   0 ) },
                                { WCF_IDS_COLORNAVY,    RGB (   0,   0, 128 ) },
                                { WCF_IDS_COLORPURPLE,  RGB ( 128,   0, 128 ) },
                                { WCF_IDS_COLORTEAL,    RGB (   0, 128, 128 ) },
                                { WCF_IDS_COLORGRAY,    RGB ( 128, 128, 128 ) },
                                { WCF_IDS_COLORSILVER,  RGB ( 192, 192, 192 ) },
                                { WCF_IDS_COLORRED,     RGB ( 255,   0,   0 ) },
                                { WCF_IDS_COLORLIME,    RGB (   0, 255,   0 ) },
                                { WCF_IDS_COLORYELLOW,  RGB ( 255, 255,   0 ) },
                                { WCF_IDS_COLORBLUE,    RGB (   0,   0, 255 ) },
                                { WCF_IDS_COLORFUCHSIA, RGB ( 255,   0, 255 ) },
                                { WCF_IDS_COLORAQUA,    RGB (   0, 255, 255 ) },
                                { WCF_IDS_COLORWHITE,   RGB ( 255, 255, 255 ) }
                              };


char                          SampleText [ 10 ];          /*--- AaBbYyZz ---*/
CHOOSEFONT FAR*               lpCurrentChooseFont = NULL;
HBITMAP                       hTrueTypeBitmap = ( HBITMAP )NULL;
HBITMAP                       hPrinterBitmap = ( HBITMAP )NULL;

/*HINSTANCE WINAPI
GetInstance ( void );*/
HINSTANCE
GetInstance
  ( void );
TError
WCFDisplayFontData
  ( HWND                      hDialogWnd );

/*------------------------------------------------------------------------------
**
**  atoi for far strings
**
**----------------------------------------------------------------------------*/

int WINAPI
fatoi
  ( LPSTR                     lpStr )
{

  int                         Number = 0;

  while ( *lpStr )
    Number = Number * 10 + ( int )( *lpStr++ - '0' );

  return ( Number );

}

/*------------------------------------------------------------------------------
**
**  Unlock a handle and then set the pointer which was pointing to the locked
**  handle to NULL
**
**----------------------------------------------------------------------------*/

void WINAPI
WCFUnlockHandle
  ( HANDLE FAR*               lpHandle,
    LPVOID FAR*               lplpMemory )

{

  if ( *lplpMemory )
    GlobalUnlock ( *lpHandle );

  *lplpMemory = NULL;

}

/*------------------------------------------------------------------------------
**
**  Free an allocated handle and then set its value to NULL
**
**----------------------------------------------------------------------------*/

void WINAPI
WCFFreeHandle
  ( HANDLE FAR*               lpHandle )

{

  if ( *lpHandle )
    GlobalFree ( *lpHandle );

  *lpHandle = (HANDLE)NULL;

}

/*------------------------------------------------------------------------------
**
**  Since the window text could be of varring length allocate the proper amount
**  of memory, lock the handle and then copy the string to the pointer.
**
**----------------------------------------------------------------------------*/

TError WINAPI
WCFGetWindowString
  ( HWND                      hWnd,
    HANDLE FAR*               lpHandle,
    LPSTR FAR*                lplpString )

{

  WORD                        StringLength;

  TError                       ErrorCode = 0;


  *lpHandle    = ( HANDLE )NULL;
  *lplpString  = NULL;

  if ( ! ( StringLength = ( WORD ) GetWindowTextLength ( hWnd ) ) )
    ErrorCode = CDERR_DIALOGFAILURE;
  else
  if ( ! ( *lpHandle = GlobalAlloc ( GHND, StringLength + 1 ) ) )
    ErrorCode = CDERR_MEMALLOCFAILURE;
  else
  if ( ! ( *lplpString = ( LPSTR ) GlobalLock ( *lpHandle ) ) )
    ErrorCode = CDERR_MEMLOCKFAILURE;
  else
  if ( ! ( GetWindowText ( hWnd, *lplpString, StringLength + 1 ) ) )
    ErrorCode = CDERR_DIALOGFAILURE;

  if ( ErrorCode )
  {
    WCFUnlockHandle ( lpHandle, ( LPVOID )lplpString );
    WCFFreeHandle ( lpHandle );
  }

  return ( ErrorCode );

}

/*------------------------------------------------------------------------------
**
**  Get the selected item index for a combination box
**
**----------------------------------------------------------------------------*/

TError WINAPI
WCFGetSelectedItemIndex
  ( HWND                      hWnd,
    WORD FAR*                 lpSelectedItemIndex )

{

  DWORD                       SelectedItem;

  TError                       ErrorCode = 0;


  *lpSelectedItemIndex = 0;
  if ( ( SelectedItem = SendMessage ( hWnd, CB_GETCURSEL, 0, 0 ) ) == CB_ERR )
    ErrorCode = CDERR_DIALOGFAILURE;
  else
    *lpSelectedItemIndex = ( WORD ) SelectedItem;

  return ( ErrorCode );

}

/*------------------------------------------------------------------------------
**
**  Get the Item Data for the current selected item in a combo box
**
**----------------------------------------------------------------------------*/

TError WINAPI
WCFGetSelectedItemData
  ( HWND                      hWnd,
    LRESULT FAR*              lpSelectedItemData )

{

  WORD                        SelectedItem;

  TError                       ErrorCode = 0;


  if ((ErrorCode = WCFGetSelectedItemIndex ( hWnd, &SelectedItem )))
    ErrorCode = CDERR_DIALOGFAILURE;
  else
  if ( ( *lpSelectedItemData = SendMessage ( hWnd, CB_GETITEMDATA, SelectedItem,
                                             0 ) ) == CB_ERR )
    ErrorCode = CDERR_DIALOGFAILURE;

  return ( ErrorCode );

}

/*------------------------------------------------------------------------------
**
**  Add a string to a combo box and set its item data
**
**----------------------------------------------------------------------------*/

TError WINAPI
WCFAddStringAndItemData
  ( HWND                      hControlWnd,
    WORD                      StringID,
    DWORD                     ItemData )

{

  char                        String [ 128 ];
  DWORD                       ItemIndex;

  TError                      ErrorCode = 0;


  if ( ! LoadString ( GetInstance (), StringID, ( LPSTR ) String,
                      sizeof ( String ) ) )
    ErrorCode = CDERR_LOADSTRFAILURE;
  else
  if ( SendMessage ( hControlWnd, CB_ADDSTRING, 0,
                     ( LPARAM ) ( ( LPSTR ) String ) ) == CB_ERR )
    ErrorCode = CDERR_DIALOGFAILURE;
  else
  if ( ( ItemIndex = SendMessage ( hControlWnd, CB_FINDSTRINGEXACT,
                                   0, ( LPARAM ) ( ( LPSTR ) String ) ) ) == CB_ERR )
    ErrorCode = CDERR_DIALOGFAILURE;
  else
  if ( SendMessage ( hControlWnd, CB_SETITEMDATA, ItemIndex,
                     ItemData ) == CB_ERR )
    ErrorCode = CDERR_DIALOGFAILURE;

  return ( ErrorCode );

}

/*------------------------------------------------------------------------------
**
**  Draw the TrueType symbol in the given rect at 0, 0
**
**----------------------------------------------------------------------------*/

static void
DrawSymbol
  ( HDC                       hDC,
    RECT FAR*                 lpRect,
    HBITMAP                   hBitmap ) 

{

  BITMAP                      Bitmap;
  HDC                         hMemDC = ( HDC )NULL;
  HGDIOBJ                     hOldObject = ( HGDIOBJ )NULL;


  if ( GetObject ( hBitmap, sizeof ( BITMAP ), &Bitmap ) &&
    ( hMemDC = CreateCompatibleDC ( hDC ) ) &&
    ( hOldObject = SelectObject ( hMemDC, hBitmap ) ) )
    BitBlt ( hDC, 0, lpRect->top, Bitmap.bmWidth, Bitmap.bmHeight, hMemDC,
             0, 0, SRCCOPY );

  if ( hOldObject )
    SelectObject ( hMemDC, hOldObject );

  if ( hMemDC )
    DeleteDC ( hMemDC );

}

/*------------------------------------------------------------------------------
**
**  Check that all parameters passed to choose font make sense in relation to
**  each other
**
**----------------------------------------------------------------------------*/

TError WINAPI
WCFCheckParameters
  ( void )

{

  TError                       ErrorCode = 0;

  /* Check for non-NULL pointers to needed data */
  if ( ( ! lpCurrentChooseFont ) || ( ! lpCurrentChooseFont->lpLogFont ) )
    ErrorCode = CDERR_INITIALIZATION;
  else
  /* Check that struct sizes match */
/*--- this check is not valid due to the differences in binary and source ports
  if ( lpCurrentChooseFont->lStructSize != sizeof ( CHOOSEFONT ) )
    ErrorCode = CDERR_STRUCTSIZE;
  else
----------*/
  /* if the CF_ENABLEHOOK flag is set then there must be a function
      that needs to be called */
  if ( ( lpCurrentChooseFont->Flags & CF_ENABLEHOOK ) &&
    ( ! lpCurrentChooseFont->lpfnHook ) )
    ErrorCode = CDERR_NOHOOK;
  else
  /* if the CF_ENABLETEMPLATE flag is set then there must be a hInstance
      and a lpTemplate but this flag is only to be checked if the
      CF_ENABLETEMPLAGEHANDLE flag is not set */
  if ( ( lpCurrentChooseFont->Flags & CF_ENABLETEMPLATE ) &&
      ( ! ( lpCurrentChooseFont->Flags & CF_ENABLETEMPLATEHANDLE ) ) &&
      ( ( ! lpCurrentChooseFont->hInstance ) ||
        ( ! lpCurrentChooseFont->lpTemplateName ) ) )
    ErrorCode = CDERR_NOTEMPLATE;
  else
  /* if the CF_ENABLETEMPLATEHANDLE flag is set then the hInstance is
      the dialog handle. */
  if ( ( lpCurrentChooseFont->Flags & CF_ENABLETEMPLATEHANDLE ) &&
    ( ! lpCurrentChooseFont->hInstance ) )
    ErrorCode = CDERR_NOHINSTANCE;
  else
  /* if the CF_LIMITSIZE flag is set then make sure that the minimum size is
      not greater than the maximum size */
  if ( ( lpCurrentChooseFont->Flags & CF_LIMITSIZE ) &&
    ( lpCurrentChooseFont->nSizeMin > lpCurrentChooseFont->nSizeMax ) )
    ErrorCode = CFERR_MAXLESSTHANMIN;
  else
  /* if the CF_SHOWHELP flag is set then there must be an owner window */
  if ( ( lpCurrentChooseFont->Flags & CF_SHOWHELP ) &&
    ( ! lpCurrentChooseFont->hwndOwner ) )
    ErrorCode = CDERR_INITIALIZATION;
  else
  /* if the CF_USESTYLE flag is set then check the lpszStyle */
  if ( ( lpCurrentChooseFont->Flags & CF_USESTYLE ) &&
    ( ! lpCurrentChooseFont->lpszStyle ) )
    ErrorCode = CDERR_INITIALIZATION;
  else
  /* if the CF_WYSIWYG is set then check to see that the CF_BOTH and
      CF_SCALABLE flags are also set */
  if ( ( lpCurrentChooseFont->Flags & CF_WYSIWYG ) &&
    ( ( ! ( lpCurrentChooseFont->Flags & CF_BOTH ) ) ||
      ( ! ( lpCurrentChooseFont->Flags & CF_SCALABLEONLY ) ) ) )
    ErrorCode = CDERR_INITIALIZATION;

  return ( ErrorCode );

}

/*------------------------------------------------------------------------------
**
**  Initialize any variables
**
**----------------------------------------------------------------------------*/

TError WINAPI
WCFInitialize
  ( void )

{

  TError                       ErrorCode = 0;

  if ( ! ( LoadString ( GetInstance (), WCF_IDS_SAMPLETEXT,
                        ( LPSTR ) SampleText, sizeof ( SampleText ) ) ) )
    ErrorCode = CDERR_LOADSTRFAILURE;
  else
  if ( ! ( hTrueTypeBitmap = LoadBitmap ( GetInstance (),
                                          MAKEINTRESOURCE ( WCF_IDB_TRUETYPE ) ) ) )
    ErrorCode = CDERR_FINDRESFAILURE;
  if ( ! ( hPrinterBitmap = LoadBitmap ( GetInstance (),
                                         MAKEINTRESOURCE ( WCF_IDB_PRINTER ) ) ) )
    ErrorCode = CDERR_FINDRESFAILURE;


  return ( ErrorCode );

}
/*------------------------------------------------------------------------------
**
** Call Dialog callback hook
**
**----------------------------------------------------------------------------*/

BOOL WINAPI
WCFCallHook
  ( HWND                      hWnd,
    UINT                      Message,
    WPARAM                    wParam,
    LPARAM                    lParam )

{

  if ( ( lpCurrentChooseFont ) &&
      ( lpCurrentChooseFont->Flags & CF_ENABLEHOOK ) )
    return ( LOWORD((*lpCurrentChooseFont->lpfnHook )( hWnd, Message, wParam,
                                                lParam ) ));

  return ( FALSE );

}

/*------------------------------------------------------------------------------
**
** Add fonts Callback
**
**----------------------------------------------------------------------------*/

int WCFCALLBACK
WCFAddFontsProc
  ( LOGFONT FAR*              lpLF,
    NEWTEXTMETRIC FAR*        lpNTM,
    int                       FontType,
    LPARAM                    lParam )

{

  WCFENUMFONTDATA FAR*        lpEnumFontData = ( WCFENUMFONTDATA FAR* ) lParam;
  HWND                        hFontNameWnd   =
                                GetDlgItem ( lpEnumFontData->hDialogWnd,
                                              WCFCBFontName );
  WORD                        ItemIndex;

  int                         ErrorCode      = 0;


  if ( ( lpCurrentChooseFont->Flags & CF_NOOEMFONTS ) ||
       ( lpCurrentChooseFont->Flags & CF_NOVECTORFONTS ) )
    if ( lpNTM->tmPitchAndFamily & TMPF_VECTOR )
      return ( TRUE );

  if ( lpCurrentChooseFont->Flags & CF_FIXEDPITCHONLY ) 
    if ( !(lpNTM->tmPitchAndFamily & TMPF_FIXED_PITCH) )
      return ( TRUE );

  if ( lpCurrentChooseFont->Flags & CF_SCALABLEONLY ) 
    if ( lpNTM->tmPitchAndFamily & TMPF_FIXED_PITCH )
      return ( TRUE );

  if ( ( ItemIndex = SendMessage ( hFontNameWnd, CB_ADDSTRING, 0,
                                   ( LPARAM ) lpLF->lfFaceName ) ) == ( WORD )CB_ERR )
    ErrorCode = CDERR_INITIALIZATION;
  else
  if ( SendMessage ( hFontNameWnd, CB_SETITEMDATA, ItemIndex,
                      lpEnumFontData->PrinterFonts ? hPrinterBitmap :
                      lpNTM->tmPitchAndFamily & TMPF_TRUETYPE ?
                        hTrueTypeBitmap : FALSE ) == CB_ERR )
    ErrorCode = CDERR_INITIALIZATION;


  return ( ! ErrorCode );

}

/*------------------------------------------------------------------------------
**
** Initialize font selectors with values
**
**----------------------------------------------------------------------------*/

TError WINAPI
WCFInitFontSelector
  ( CHOOSEFONT FAR*           lpCF,
    HWND                      hWnd )

{

  HDC                         hDC           = ( HDC )NULL;
  FONTENUMPROC                lpfnEnumProc = NULL;
  WCFENUMFONTDATA             EnumFontData;
  int                         ColorIndex;
  char                        ColorString [ 128 ];
  BOOL                        SelectedColor = FALSE;
  HWND                        hColorWnd = GetDlgItem ( hWnd, WCFCBFontColor );

  TError                      ErrorCode = 0;


  _fmemset ( &EnumFontData, 0, sizeof ( EnumFontData ) );

  if ( ! ( lpfnEnumProc = MakeProcInstance ( WCFAddFontsProc,
                                              GetInstance () ) ) )
    ErrorCode = CDERR_DIALOGFAILURE;
  else
  {
    EnumFontData.hDialogWnd = hWnd;
    if ( ( lpCurrentChooseFont->hDC ) &&
          ( lpCurrentChooseFont->Flags & CF_PRINTERFONTS ) )
    {
      EnumFontData.PrinterFonts = TRUE;
      EnumFontData.hDC          = lpCurrentChooseFont->hDC;
      if ( ! EnumFontFamilies (lpCurrentChooseFont->hDC,
                                 NULL,
                                 lpfnEnumProc,
                                 (LPSTR)&EnumFontData))
        ErrorCode = CDERR_INITIALIZATION;
    }
    if ( ( ! ( lpCurrentChooseFont->Flags & CF_PRINTERFONTS ) ) ||
         ( lpCurrentChooseFont->Flags & CF_SCREENFONTS ) )
      if ((hDC = GetDC ( hWnd )))
      {
        EnumFontData.PrinterFonts = FALSE;
        EnumFontData.hDC          = hDC;
        if ( ! EnumFontFamilies (hDC,
                                 NULL,
                                 lpfnEnumProc,
                                 (LPSTR)&EnumFontData))

          ErrorCode = CDERR_INITIALIZATION;
      }
    if ( ! ErrorCode )
      WCFDisplayFontData ( hWnd );
  }

  for ( ColorIndex = 0; ( ! ErrorCode ) && ( ColorIndex < NUM_COLORS );
        ColorIndex++ )
  {
    if ( ! ( LoadString ( GetInstance (), ColorData [ ColorIndex ].StringID,
                          ( LPSTR ) ColorString,
          sizeof ( ColorString ) ) ) )
      ErrorCode = CDERR_LOADSTRFAILURE;
    else
    if ( SendMessage ( hColorWnd, CB_ADDSTRING, 0,
                        ( LPARAM )( ( LPSTR )ColorString ) ) == CB_ERR )
      ErrorCode = CDERR_INITIALIZATION;
    else
    if ( SendMessage ( hColorWnd, CB_SETITEMDATA, 
                       ColorIndex, 
                       ColorData [ ColorIndex ].Color ) == CB_ERR )
      ErrorCode = CDERR_INITIALIZATION;
    else
    if ( ColorData [ ColorIndex ].Color == lpCurrentChooseFont->rgbColors )
    {
      SelectedColor = TRUE;
      if ( SendMessage ( hColorWnd, CB_SELECTSTRING, 0,
                          ( LPARAM )( ( LPSTR )ColorString ) ) == CB_ERR )
        ErrorCode = CDERR_INITIALIZATION;
    }
  }

  /*--- Clean up ---*/
  if ( hDC )
  {
    ReleaseDC ( hWnd, hDC );
    hDC = (HDC)NULL;
  }

  if ( lpfnEnumProc )
  {
    FreeProcInstance ( lpfnEnumProc );
    lpfnEnumProc = NULL;
  }

  return ( ErrorCode );

}
/*------------------------------------------------------------------------------
**
** Display font styles
**
**----------------------------------------------------------------------------*/

int WCFCALLBACK
WCFFontStyleProc
  ( LOGFONT FAR*              lpLF,
    NEWTEXTMETRIC FAR*        lpNTM,
    int                       FontType,
    LPARAM                    lParam )

{

  WCFENUMFONTDATA FAR*        lpEnumFontData = ( WCFENUMFONTDATA FAR* ) lParam;
  NEWLOGFONT FAR*             lpNLF          = ( NEWLOGFONT FAR* ) lpLF;
  HWND                        hFontStyleWnd  =
                                GetDlgItem ( lpEnumFontData->hDialogWnd,
                                             WCFCBFontStyle );
  WORD                        StringID;
  DWORD                       Index;


  int                         ErrorCode      = 0;


  if ( ( lpNTM->tmPitchAndFamily & TMPF_TRUETYPE ) ||
        ( lpNTM->tmPitchAndFamily & TMPF_VECTOR ) )
  { /*--- this is a sizeable font ---*/
    switch ( lpNLF->lfWeight )
    {
      default:
        StringID = WCF_IDS_REGULAR;
        break;

      case FW_THIN:
        StringID = WCF_IDS_THIN;
        break;

      /*--- FW_ULTRALIGHT == FW_EXTRALIGHT
      case FW_ULTRALIGHT:
        StringID = WCF_IDS_ULTRALIGHT;
      ---*/
      case FW_EXTRALIGHT:
        StringID = WCF_IDS_EXTRALIGHT;
        break;

      case FW_LIGHT:
        StringID = WCF_IDS_LIGHT;
        break; 
        
      case FW_BOLD:
        StringID = WCF_IDS_BOLD;
        break;

      case FW_MEDIUM:
        StringID = WCF_IDS_MEDIUM;
        break;

      /*--- FW_DEMIBOLD == FW_SEMIBOLD
      case FW_DEMIBOLD:
        StringID = WCF_IDS_DEMIBOLD;
      ---*/
      case FW_SEMIBOLD:
        StringID = WCF_IDS_SEMIBOLD;
        break;

      /*--- FW_ULTRABOLD == FW_EXTRABOLD
      case FW_ULTRABOLD:
        StringID = WCF_IDS_ULTRABOLD;
      ---*/
      case FW_EXTRABOLD:
        StringID = WCF_IDS_EXTRABOLD;
        break;

      /*--- FW_HEAVY == FW_BLACK
      case FW_HEAVY:
        StringID = WCF_IDS_HEAVY;
      ---*/
      case FW_BLACK:
        StringID = WCF_IDS_BLACK;
        break;

    }
    if ( lpNLF->lfItalic )
      switch ( StringID )
      {
        default:
          StringID = WCF_IDS_ITALIC;
          break;

        case WCF_IDS_BOLD:
          StringID = WCF_IDS_ITALICBOLD;
          break;
      }
    SendMessage ( hFontStyleWnd, CB_ADDSTRING, 0,
                  ( LPARAM ) ( ( LPSTR ) lpNLF->lfStyle ) );
    if ( ( Index = SendMessage ( hFontStyleWnd, CB_FINDSTRINGEXACT, 0,
                  ( LPARAM ) ( ( LPSTR ) lpNLF->lfStyle ) ) ) != ( DWORD )CB_ERR )
      SendMessage ( hFontStyleWnd, CB_SETITEMDATA, ( WPARAM ) Index, StringID );
  }
  else
  { /*--- this is a fixed font so add the standard styles ---*/
    WCFAddStringAndItemData ( hFontStyleWnd, WCF_IDS_REGULAR, WCF_IDS_REGULAR );
    WCFAddStringAndItemData ( hFontStyleWnd, WCF_IDS_ITALIC, WCF_IDS_ITALIC );
    WCFAddStringAndItemData ( hFontStyleWnd, WCF_IDS_BOLD, WCF_IDS_BOLD );
    WCFAddStringAndItemData ( hFontStyleWnd, WCF_IDS_ITALICBOLD, WCF_IDS_ITALICBOLD );
    ErrorCode = TRUE;      
  }
  return ( ! ErrorCode );

}

TError WINAPI
WCFAddFontStyles
  ( HWND                      hDialogWnd )

{

  char                        FontName [ 128 ];
  char                        FontStyle [ 128 ];
  FARPROC                     lpfnFontStyleProc = NULL;
  WCFENUMFONTDATA             EnumFontData;
  DWORD                       ItemIndex;

  TError                      ErrorCode = 0;


  FontStyle [ 0 ]         = '\0';
  FontName [ 0 ]          = '\0';
  EnumFontData.hDialogWnd = hDialogWnd;
  GetWindowText ( GetDlgItem ( hDialogWnd, WCFCBFontStyle ),
                  ( LPSTR ) FontStyle, sizeof ( FontStyle ) );
  if ( ( ItemIndex = SendMessage ( GetDlgItem ( hDialogWnd, WCFCBFontName ), 
                                    CB_GETCURSEL, 0, 0 ) ) == ( DWORD )CB_ERR )
    ErrorCode = CDERR_DIALOGFAILURE;
  else
  if ( SendMessage ( GetDlgItem ( hDialogWnd, WCFCBFontName ), CB_GETLBTEXT,
                      ItemIndex, ( LPARAM )( ( LPSTR ) FontName ) ) == CB_ERR )
    ErrorCode = CDERR_DIALOGFAILURE;
  else                        
  if ( lstrlen ( ( LPSTR ) FontName ) )
  {
    if ( ! ( lpfnFontStyleProc = MakeProcInstance ( WCFFontStyleProc,
                                                    GetInstance () ) ) )
      ErrorCode = CDERR_DIALOGFAILURE;
    else
    if ( SendMessage ( GetDlgItem ( hDialogWnd, WCFCBFontStyle ),
                       CB_RESETCONTENT, 0, 0 ) == CB_ERR )
      ErrorCode = CDERR_DIALOGFAILURE;
    else
    if ( ! ( EnumFontData.hDC = GetDC ( hDialogWnd ) ) )
      ErrorCode = CDERR_DIALOGFAILURE;
    else
    {
      if ( ! EnumFontFamilies ( EnumFontData.hDC,
                                FontName,
                                lpfnFontStyleProc,
                                ( LPSTR )&EnumFontData ) )
        ErrorCode = CDERR_INITIALIZATION;
      else
      {
        if ( ( ItemIndex = SendMessage ( GetDlgItem ( hDialogWnd, WCFCBFontStyle ),
                                         CB_FINDSTRINGEXACT, 0,
                                         ( LPARAM ) ( ( LPSTR ) FontStyle ) ) ) == ( DWORD )CB_ERR )
          ItemIndex = 0;
        SendMessage ( GetDlgItem ( hDialogWnd, WCFCBFontStyle ),
                      CB_SETCURSEL, ( WPARAM )ItemIndex, 0 );
      }
      ReleaseDC ( hDialogWnd, EnumFontData.hDC );
    }
  }

  return ( ErrorCode );

}
/*------------------------------------------------------------------------------
**
** Display all of the font data such as style, pitch sizes colors etc.
**
**----------------------------------------------------------------------------*/

int WCFCALLBACK
WCFAddFontData
  ( LOGFONT FAR*              lpLF,
    NEWTEXTMETRIC FAR*        lpNTM,
    int                       FontType,
    LPARAM                    lParam )

{

  WCFENUMFONTDATA FAR*        lpEnumFontData    =
                                ( WCFENUMFONTDATA FAR* ) lParam;
  HWND                        hControlWnd       =
                                GetDlgItem ( lpEnumFontData->hDialogWnd,
                                              WCFCBFontSize );
  char                        StrFontSize [ 20 ];
  int                         FontSize;
  LRESULT                     ReturnValue;
  WORD                        SelectedIndex;

  TError                      ErrorCode = 0;


  if ( ( lpNTM->tmPitchAndFamily & TMPF_TRUETYPE ) ||
        ( lpNTM->tmPitchAndFamily & TMPF_VECTOR ) )
  { /*--- this is a sizeable font ---*/
    {
      if ( SendMessage ( hControlWnd, CB_RESETCONTENT, 0, 0 ) == CB_ERR )
        ErrorCode = CDERR_DIALOGFAILURE;
      else
      for ( FontSize = 0; (
            FontSize < NUM_TRUETYPE_FONT_SIZES ) && ( ! ErrorCode );
            FontSize++ )
        if ( SendMessage ( hControlWnd, CB_ADDSTRING, 0,
                            ( LPARAM ) ( ( LPSTR ) TrueTypeFontSizes [ FontSize ].String ) ) == CB_ERR )
          ErrorCode = CDERR_DIALOGFAILURE;
        else
        if ( SendMessage ( hControlWnd, CB_SETITEMDATA, FontSize,
                            ( LPARAM ) TrueTypeFontSizes [ FontSize ].Size ) ==
              CB_ERR )
          ErrorCode = CDERR_DIALOGFAILURE;
        else
          _fmemcpy ( lpCurrentChooseFont->lpLogFont, lpLF, sizeof ( LOGFONT ) );
    }
  }
  else
  { /*--- this is not a sizeable font so find the sizes available ---*/
    wsprintf ( ( LPSTR ) StrFontSize, "%0d", lpLF->lfHeight );
    if ( SendMessage ( hControlWnd, CB_ADDSTRING, 0,
                      ( LPARAM ) ( ( LPSTR ) StrFontSize ) ) == CB_ERR )
      ErrorCode = CDERR_DIALOGFAILURE;
    else
    if ( ( ReturnValue = SendMessage ( hControlWnd, CB_FINDSTRINGEXACT, 0,
                          ( LPARAM ) ( ( LPSTR ) StrFontSize ) ) ) == CB_ERR )
      ErrorCode = CDERR_DIALOGFAILURE;
    else
    if ( SendMessage ( hControlWnd, CB_SETITEMDATA, SelectedIndex =
                        ( WORD ) ReturnValue, lpLF->lfHeight ) == CB_ERR )
      ErrorCode = CDERR_DIALOGFAILURE;
    else
      _fmemcpy ( lpCurrentChooseFont->lpLogFont, lpLF, sizeof ( LOGFONT ) );
  }

  return ( ErrorCode == 0 );

}

/*------------------------------------------------------------------------------
**
** Display Sample Text
**
**----------------------------------------------------------------------------*/

TError WCFDisplaySampleText
  ( HWND                      hDialogWnd,
    BOOL                      ForceDisplay )

{

  HWND                        hSampleWnd =
                                GetDlgItem ( hDialogWnd, WCFTextSample );
  HWND                        hSizeWnd =
                                GetDlgItem ( hDialogWnd, WCFCBFontSize );
  HWND                        hStyleWnd =
                                GetDlgItem ( hDialogWnd, WCFCBFontStyle );
  HDC                         hSampleTextDC = ( HDC )NULL;
  HFONT                       hSampleFont = ( HFONT )NULL;
  HGLOBAL                     hFontSizeStr = ( HGLOBAL )NULL;
  LPSTR                       lpFontSizeStr = ( LPSTR )NULL;
  DWORD                       SelectedFontStyle = ( DWORD )NULL;
  DWORD                       Size;
  RECT                        SampleTextRect;
  RECT                        RequiredRect;
  COLORREF                    Color = 0;
  double                      fSize;
  BOOL                        DrawSampleText = ForceDisplay;
  WORD                        Weight = FW_NORMAL;
  BOOL                        Italic = FALSE;

  TError                      ErrorCode = 0;


  if ( WCFGetSelectedItemData ( hSizeWnd, ( LRESULT FAR* )&Size ) )
    if ( ! ( ErrorCode = WCFGetWindowString ( hSizeWnd, &hFontSizeStr,
                                              &lpFontSizeStr ) ) )
      Size = fatoi ( lpFontSizeStr );

  if ( ( ! ErrorCode ) && ( Size ) &&
        ( ! ( ErrorCode = WCFGetSelectedItemData ( hStyleWnd,
                                                  ( LRESULT FAR* )&SelectedFontStyle ) ) ) &&
        ( SelectedFontStyle ) &&
        ( ! ( ErrorCode = WCFGetSelectedItemData ( GetDlgItem ( hDialogWnd, WCFCBFontColor ),
                                                    ( LRESULT FAR* )&Color ) ) ) )
  if ( ! ( hSampleTextDC = GetDC ( hSampleWnd ) ) )
    ErrorCode = CDERR_DIALOGFAILURE;
  else
  {
    fSize = ( double ) Size;
    fSize = ( double ) Size * ( double )GetDeviceCaps ( hSampleTextDC, LOGPIXELSY ) / ( double ) 72;
    fSize *= ( -1 );
    fSize -= ( 0.5000 );
    /*--- The following is a fudge factor, why?, I don't know ---*/
    /* fSize                                       += ( ( ( double )Size / 15.0 ) +
                                                      1.0 ); */
    if ( lpCurrentChooseFont->lpLogFont->lfHeight != fSize )
      DrawSampleText = TRUE;
    lpCurrentChooseFont->lpLogFont->lfHeight    = fSize;
    lpCurrentChooseFont->lpLogFont->lfWidth     = 0;
    switch ( SelectedFontStyle )
    {
      case WCF_IDS_THIN:
        Weight = FW_THIN;
        break;

      case WCF_IDS_EXTRALIGHT:
      case WCF_IDS_ULTRALIGHT:
        Weight = FW_EXTRALIGHT;
        break;

      case WCF_IDS_LIGHT:
        Weight = FW_LIGHT;
        break;

      case WCF_IDS_ITALIC:
      case WCF_IDS_NORMAL:
      case WCF_IDS_REGULAR:
        Weight = FW_NORMAL;
        break;

      case WCF_IDS_MEDIUM:
        Weight = FW_MEDIUM;
        break;

      case WCF_IDS_SEMIBOLD:
      case WCF_IDS_DEMIBOLD:
        Weight = FW_SEMIBOLD;
        break;

      case WCF_IDS_ITALICBOLD:
      case WCF_IDS_BOLD:
        Weight = FW_BOLD;
        break;

      case WCF_IDS_EXTRABOLD:
      case WCF_IDS_ULTRABOLD:
        Weight = FW_EXTRABOLD;
        break;

      case WCF_IDS_BLACK:
      case WCF_IDS_HEAVY:
        Weight = FW_BLACK;
        break;
    }
    switch ( SelectedFontStyle )
    {
      case WCF_IDS_ITALIC:
      case WCF_IDS_ITALICBOLD:
        Italic = TRUE;
        break;

      case WCF_IDS_NORMAL:
      case WCF_IDS_BOLD:
      case WCF_IDS_THIN:
      case WCF_IDS_EXTRALIGHT:
      case WCF_IDS_ULTRALIGHT:
      case WCF_IDS_LIGHT:
      case WCF_IDS_REGULAR:
      case WCF_IDS_MEDIUM:
      case WCF_IDS_SEMIBOLD:
      case WCF_IDS_DEMIBOLD:
      case WCF_IDS_EXTRABOLD:
      case WCF_IDS_ULTRABOLD:
      case WCF_IDS_BLACK:
      case WCF_IDS_HEAVY:
        Italic = FALSE;
        break;
    }
    if ( lpCurrentChooseFont->lpLogFont->lfItalic != Italic )
      DrawSampleText = TRUE;
    if ( lpCurrentChooseFont->lpLogFont->lfWeight != Weight )
      DrawSampleText = TRUE;
    if ( lpCurrentChooseFont->lpLogFont->lfUnderline !=
          IsDlgButtonChecked ( hDialogWnd, WCFCheckStrikeout ) )
      DrawSampleText = TRUE;
    if ( lpCurrentChooseFont->lpLogFont->lfStrikeOut !=
          IsDlgButtonChecked ( hDialogWnd, WCFCheckUnderline ) )
      DrawSampleText = TRUE;
    lpCurrentChooseFont->lpLogFont->lfItalic = Italic;
    lpCurrentChooseFont->lpLogFont->lfWeight = Weight;
    lpCurrentChooseFont->lpLogFont->lfUnderline =
      IsDlgButtonChecked ( hDialogWnd, WCFCheckStrikeout );
    lpCurrentChooseFont->lpLogFont->lfStrikeOut =
      IsDlgButtonChecked ( hDialogWnd, WCFCheckUnderline );
    if ( DrawSampleText )
    {
      if ( ! ( hSampleFont =
                CreateFontIndirect ( lpCurrentChooseFont->lpLogFont ) ) )
       ErrorCode = CDERR_DIALOGFAILURE;
     else
     {
       SetBkMode ( hSampleTextDC, TRANSPARENT );
       SetTextColor ( hSampleTextDC, lpCurrentChooseFont->rgbColors =
                                      Color );
       SelectObject ( hSampleTextDC, hSampleFont );
       GetClientRect ( hSampleWnd, &SampleTextRect );
       InvalidateRect ( hSampleWnd, &SampleTextRect, TRUE );
       UpdateWindow ( hSampleWnd );
       ShowWindow ( hSampleWnd, SW_SHOW );
       CopyRect ( &RequiredRect, &SampleTextRect );
       DrawText ( hSampleTextDC, ( LPSTR ) SampleText, lstrlen ( SampleText ),
                    &RequiredRect,
         DT_SINGLELINE | DT_CALCRECT );
       if ( ( RequiredRect.right <= SampleTextRect.right ) &&
              ( RequiredRect.bottom <= SampleTextRect.bottom ) )
         DrawText ( hSampleTextDC, ( LPSTR ) SampleText, lstrlen ( SampleText ),
                      &SampleTextRect, DT_CENTER | DT_SINGLELINE | DT_VCENTER );
       else
       {
         DrawText ( hSampleTextDC, ( LPSTR ) SampleText, lstrlen ( SampleText ),                    &SampleTextRect, DT_SINGLELINE | DT_VCENTER );
       }
     }
   }
  }

  if ( hSampleTextDC )
  {
    ReleaseDC ( hSampleWnd, hSampleTextDC );
    hSampleTextDC = ( HDC )NULL;
  }

  if ( hSampleFont )
  {
    DeleteObject ( hSampleFont );
    hSampleFont = ( HDC )NULL;
  }

  WCFUnlockHandle ( &hFontSizeStr, ( LPVOID )&lpFontSizeStr );
  WCFFreeHandle ( &hFontSizeStr );

  return ( ErrorCode );

}
/*------------------------------------------------------------------------------
**
** Display all of the font data such as style, pitch sizes colors etc.
**
**----------------------------------------------------------------------------*/

TError
WCFDisplayFontData
  ( HWND                      hDialogWnd )

{

  LRESULT                     ReturnValue;
  HGLOBAL                     hFontName = ( HGLOBAL )NULL;
  LPSTR                       lpFontName = ( LPSTR )NULL;
  HGLOBAL                     hFontSize = ( HGLOBAL )NULL;
  LPSTR                       lpFontSize = ( LPSTR )NULL;
  HDC                         hDC = ( HDC )NULL;
  WORD                        SelectedIndex;
  HWND                        hFontNameWnd;
  HWND                        hFontStyleWnd;
  HWND                        hFontSizeWnd;
  WCFENUMFONTDATA             EnumFontData;

  TError                       ErrorCode = 0;

  hFontNameWnd = GetDlgItem ( hDialogWnd, WCFCBFontName );
  hFontStyleWnd = GetDlgItem ( hDialogWnd, WCFCBFontStyle );
  hFontSizeWnd = GetDlgItem ( hDialogWnd, WCFCBFontSize );

  /*----- Get the Currently selected font name -----*/
  if ( ( ReturnValue = SendMessage ( hFontNameWnd, CB_GETCURSEL, 0, 0 ) ) ==
          CB_ERR )
    ErrorCode = CDERR_DIALOGFAILURE;
  else
  if ( ( ReturnValue = SendMessage ( hFontNameWnd, CB_GETLBTEXTLEN,
                                      SelectedIndex =
                                        ( WORD ) ReturnValue, 0 ) ) == CB_ERR )
    ErrorCode = CDERR_DIALOGFAILURE;
  else
  if ( ! ( hFontName = GlobalAlloc ( GHND, ReturnValue + 1 ) ) )
    ErrorCode = CDERR_MEMALLOCFAILURE;
  else
  if ( ! ( lpFontName = ( LPSTR ) GlobalLock ( hFontName ) ) )
    ErrorCode = CDERR_MEMLOCKFAILURE;
  else
  if ( ( ReturnValue = SendMessage ( hFontNameWnd, CB_GETLBTEXT, SelectedIndex,
                                      ( LPARAM ) lpFontName ) ) == CB_ERR )
    ErrorCode = CDERR_DIALOGFAILURE;
  else
  if ( ! ( hDC = GetDC ( hDialogWnd ) ) )
    ErrorCode = CDERR_DIALOGFAILURE;
  else
  {
    EnumFontData.hDC = hDC;
    EnumFontData.hDialogWnd = hDialogWnd;
    /*----- Get the Currently selected font size -----*/
    ReturnValue = GetWindowTextLength ( hFontSizeWnd );
    if ( ! ( hFontSize = GlobalAlloc ( GHND, 
		                                   ReturnValue ? 3 : ReturnValue + 1 ) ) )
      ErrorCode = CDERR_MEMALLOCFAILURE;
    else
    if ( ! ( lpFontSize = ( LPSTR ) GlobalLock ( hFontSize ) ) )
      ErrorCode = CDERR_MEMLOCKFAILURE;
    else
		if ( ReturnValue )
		{
    	if ( ! ( GetWindowText ( hFontSizeWnd, lpFontSize,
                            	( WORD )ReturnValue + 1 ) ) )
      	ErrorCode = CDERR_DIALOGFAILURE;
		}
		else
			lstrcpy ( lpFontSize, "10" );

    if ( ! ErrorCode )
      if ( SendMessage ( hFontSizeWnd, CB_RESETCONTENT, 0, 0 ) == CB_ERR )
        ErrorCode = CDERR_DIALOGFAILURE;
      else
      {
/*        SendMessage(hFontStyleWnd, WM_SETREDRAW, FALSE, 0L);*/
        SendMessage(hFontSizeWnd, WM_SETREDRAW, FALSE, 0L);
        if ( ! EnumFontFamilies ( hDC, lpFontName, WCFAddFontData,
                                  ( LPSTR )&EnumFontData ) )
          ErrorCode = CDERR_DIALOGFAILURE;
        else
        if ( WCFAddFontStyles ( hDialogWnd ) )
          ErrorCode = CDERR_DIALOGFAILURE;
/*        SendMessage(hFontStyleWnd, WM_SETREDRAW, TRUE, 0L);*/
        /*RedrawWindow ( hFontStyleWnd, NULL, NULL, RDW_INVALIDATE );*/
        SendMessage(hFontSizeWnd, WM_SETREDRAW, TRUE, 0L);
	RedrawWindow ( hFontSizeWnd, NULL, (HRGN)NULL, RDW_INVALIDATE );
      }

    if ( ( ! ErrorCode ) && ( lpFontSize ) )
      if ( SendMessage ( hFontSizeWnd, CB_FINDSTRINGEXACT, 0,
                        ( LPARAM ) lpFontSize ) == CB_ERR )
      {
        SetWindowText ( hFontSizeWnd, lpFontSize );
	PostMessage ( hDialogWnd, WM_COMMAND,
		GET_WM_COMMAND_MPS(WCFCBFontSize,hFontSizeWnd,CBN_SELCHANGE) );
      }
      else
      if ( SendMessage ( hFontSizeWnd, CB_SELECTSTRING, 0,
                          ( LPARAM ) lpFontSize ) == CB_ERR )
        ErrorCode = CDERR_DIALOGFAILURE;
  }

  if ( ! ErrorCode )
  {
    WCFDisplaySampleText ( hDialogWnd, TRUE );
  }

  if ( hDC )
  {
    ReleaseDC ( hDialogWnd, hDC );
    hDC = ( HDC )NULL;
  }

  if ( lpFontSize )
  {
    GlobalUnlock ( hFontSize );
    lpFontSize = NULL;
  }

  if ( hFontSize )
  {
    GlobalFree ( hFontSize );
    hFontSize = ( HGLOBAL )NULL;
  }

  if ( lpFontName )
  {
    GlobalUnlock ( hFontName );
    lpFontName = NULL;
  }

  if ( hFontName )
  {
    GlobalFree ( hFontName );
    hFontName = ( HGLOBAL )NULL;
  }

  return ( ErrorCode );

}

/*------------------------------------------------------------------------------
**
** Dialog callback procedure
**
**----------------------------------------------------------------------------*/

BOOL WCFCALLBACK
WCFDialogProc
  ( HWND      hWnd,
    UINT      Message,
    WPARAM    wParam,
    LPARAM    lParam )

{

  MEASUREITEMSTRUCT FAR*      lpMeasureItem;
  DRAWITEMSTRUCT FAR*         lpDrawItem;
  HDC                         hDC;
  TEXTMETRIC                  TextMetric;
  RECT                        Rect;
  RECT                        UpdateRect;
  RECT                        IntRect;
  POINT                       TopLeftPoint;
  char                        FontName [ 128 ];
  char                        ColorName [ 128 ];
  char                        String [ 128 ];
  HBRUSH                      hBackgroundBrush;
  HBRUSH                      hBrush;
  int                         ResourceID;
  double                      fSize;
  DWORD                       ItemData;

  TError                       ErrorCode = 0;
  int                          ReturnValue = -1;


  if ( ( ErrorCode = WCFCallHook ( hWnd, Message, wParam, lParam ) ) &&
	( Message != WM_INITDIALOG ) )
	return ( ErrorCode );

    switch ( Message )
    {
      case WM_PAINT:
        GetUpdateRect ( hWnd, &UpdateRect, FALSE );
        GetWindowRect ( GetDlgItem ( hWnd, WCFTextSample ), &Rect );
        TopLeftPoint.x = Rect.left;
        TopLeftPoint.y = Rect.top;
        ScreenToClient ( hWnd, &TopLeftPoint );
        Rect.bottom = TopLeftPoint.y + ( Rect.bottom - Rect.top );
        Rect.right  = TopLeftPoint.x + ( Rect.right - Rect.left );
        Rect.top    = TopLeftPoint.y;
        Rect.left   = TopLeftPoint.x;
        if ( IntersectRect ( &IntRect, &Rect, &UpdateRect ) )
          WCFDisplaySampleText ( hWnd, TRUE );
      default:
        ReturnValue = FALSE;
        break;

      case WM_ERASEBKGND:
        if ( ( hDC = GetDC ( hWnd ) ) &&
             ( hBrush = ( HBRUSH ) SendMessage ( hWnd, 
                                                 GET_WM_CTLCOLOR_MSG ( CTLCOLOR_DLG ),
                                                 GET_WM_CTLCOLOR_MPS ( hDC, hWnd, CTLCOLOR_DLG ) ) ) )
        {
	  GetClientRect ( hWnd, &Rect );
	  FillRect ( ( HDC )wParam, &Rect, hBrush ); 
        }
	if ( hDC ) 
	  ReleaseDC ( hWnd, hDC );
	ReturnValue = 1;
        break; 

      case WM_CLOSE:
        EndDialog ( hWnd, wParam );
	ReturnValue = 1;
        break;

      case WM_MEASUREITEM:
        lpMeasureItem = ( MEASUREITEMSTRUCT FAR* ) lParam;
        switch ( lpMeasureItem->CtlID )
        {
          default:
            ReturnValue = FALSE;
            break;
          case WCFCBFontColor:
          case WCFCBFontName:
            if ( (hDC = GetDC ( GetDlgItem ( hWnd, lpMeasureItem->CtlID ) ) ) )
            {
              if (GetTextMetrics ( hDC, &TextMetric ) )
                lpMeasureItem->itemHeight = TextMetric.tmHeight - TextMetric.tmInternalLeading;
              SendMessage ( GetDlgItem ( hWnd, lpMeasureItem->CtlID ),
                            CB_GETDROPPEDCONTROLRECT,
                            0, ( LPARAM ) ( ( LPVOID ) &Rect ) );
              lpMeasureItem->itemWidth = Rect.right - Rect.left;
              ReleaseDC ( GetDlgItem ( hWnd, lpMeasureItem->CtlID ), hDC );
            }
            break;
        }
	ReturnValue = 1;
        break;

      case WM_DRAWITEM:
        lpDrawItem = ( DRAWITEMSTRUCT FAR* ) lParam;
        switch ( lpDrawItem->CtlID )
        {
          default:
            ReturnValue = FALSE;
            break;
          case WCFCBFontColor:
            if ( lpDrawItem->itemState & ODS_SELECTED )
            {
              SetTextColor ( lpDrawItem->hDC,
                              GetSysColor ( COLOR_HIGHLIGHTTEXT ) );
              hBackgroundBrush =
                CreateSolidBrush ( GetSysColor ( COLOR_HIGHLIGHT ) );
            }
            else
            {
              SetTextColor ( lpDrawItem->hDC,
                              GetSysColor ( COLOR_WINDOWTEXT ) );
              hBackgroundBrush =
                CreateSolidBrush ( GetSysColor ( COLOR_WINDOW ) );
            }
            if ( hBackgroundBrush )
            {
              FillRect ( lpDrawItem->hDC, &lpDrawItem->rcItem,
                          hBackgroundBrush );
              SetBkMode ( lpDrawItem->hDC, TRANSPARENT );
              DeleteObject ( hBackgroundBrush );
              SetRect ( &Rect, lpDrawItem->rcItem.left + 1,
                        lpDrawItem->rcItem.top + 1,
                        lpDrawItem->rcItem.left + 25,
                        lpDrawItem->rcItem.top + 12 );
              Rectangle ( lpDrawItem->hDC, Rect.left, Rect.top, Rect.right,
                          Rect.bottom );
              InflateRect ( &Rect, -1, -1 );
              ItemData = SendMessage ( lpDrawItem->hwndItem, CB_GETITEMDATA,
                                       lpDrawItem->itemID, 0 );
              if ((hBrush = CreateSolidBrush ( ItemData ) ))
              {
                FillRect ( lpDrawItem->hDC, &Rect, hBrush );
                DeleteObject ( hBrush );
              }
              SendMessage ( lpDrawItem->hwndItem, CB_GETLBTEXT,
                            lpDrawItem->itemID,
                            ( LPARAM ) ( ( LPVOID )ColorName ) );
              lpDrawItem->rcItem.left += 27;
              DrawText ( lpDrawItem->hDC, ColorName, lstrlen ( ColorName ),
                          &lpDrawItem->rcItem, DT_SINGLELINE );
              lpDrawItem->rcItem.left -= 27;
            }
	    ReturnValue = 1;
            break;
          case WCFCBFontName:
            if ( lpDrawItem->itemState & ODS_SELECTED )
            {
              SetTextColor ( lpDrawItem->hDC,
                              GetSysColor ( COLOR_HIGHLIGHTTEXT ) );
              hBackgroundBrush =
                CreateSolidBrush ( GetSysColor ( COLOR_HIGHLIGHT ) );
            }
            else
            {
              SetTextColor ( lpDrawItem->hDC,
                GetSysColor ( COLOR_WINDOWTEXT ) );
              hBackgroundBrush =
                CreateSolidBrush ( GetSysColor ( COLOR_WINDOW ) );
            }
            if ( hBackgroundBrush )
            {
              FillRect ( lpDrawItem->hDC, &lpDrawItem->rcItem,
                hBackgroundBrush );
              SetBkMode ( lpDrawItem->hDC, TRANSPARENT );
              DeleteObject ( hBackgroundBrush );
              
              if ( lpDrawItem->itemID == ( UINT )-1 ) 
              {
                lpDrawItem->rcItem.left -= 20;
                lpDrawItem->itemID = ( UINT )SendMessage ( lpDrawItem->hwndItem,
                                                           CB_GETCURSEL, 0, 0 );
              }
              else
              if ((ItemData = SendMessage ( lpDrawItem->hwndItem, 
                                            CB_GETITEMDATA,
                                            lpDrawItem->itemID, 0 ) ))
                DrawSymbol ( lpDrawItem->hDC, &lpDrawItem->rcItem, 
                             ( HBITMAP )ItemData );
              SendMessage ( lpDrawItem->hwndItem, CB_GETLBTEXT,
                            lpDrawItem->itemID,
                            ( LPARAM ) ( ( LPVOID )FontName ) );
              lpDrawItem->rcItem.left += 20;
              DrawText ( lpDrawItem->hDC, FontName, lstrlen ( FontName ),
                          &lpDrawItem->rcItem, DT_SINGLELINE );
              lpDrawItem->rcItem.left -= 20;
            }
	    ReturnValue = 1;
            break;
        }
        break;

      case WM_INITDIALOG:
        /*----- Enable/Disable Show/Hide controls base upon flags -----*/
        ShowWindow ( GetDlgItem ( hWnd, WCFButtonApply ),
                     ( BOOL ) ( lpCurrentChooseFont->Flags & CF_APPLY ) );
        EnableWindow ( GetDlgItem ( hWnd, WCFGroupEffects ),
                        ( BOOL ) ( lpCurrentChooseFont->Flags & CF_EFFECTS ) );
        EnableWindow ( GetDlgItem ( hWnd, WCFCheckStrikeout ),
                        ( BOOL ) ( lpCurrentChooseFont->Flags & CF_EFFECTS ) );
        EnableWindow ( GetDlgItem ( hWnd, WCFCheckUnderline ),
                        ( BOOL ) ( lpCurrentChooseFont->Flags & CF_EFFECTS ) );
        EnableWindow ( GetDlgItem ( hWnd, WCFCBFontName ),
                        !( BOOL )( lpCurrentChooseFont->Flags & CF_NOFACESEL ));
        EnableWindow ( GetDlgItem ( hWnd, WCFCBFontStyle ),
                        !( BOOL )(lpCurrentChooseFont->Flags & CF_NOSTYLESEL ));
        EnableWindow ( GetDlgItem ( hWnd, WCFCBFontSize ),
                        ! ( BOOL )
                            ( lpCurrentChooseFont->Flags & CF_NOSIZESEL ) );
        ShowWindow ( GetDlgItem ( hWnd, WCFButtonHelp ),
                      lpCurrentChooseFont->Flags & CF_SHOWHELP ?
                        SW_SHOW : SW_HIDE );

        /*----- Add font names to font then selector if the selector is
                available to the user -----*/
        if ( ! ( lpCurrentChooseFont->Flags & CF_NOFACESEL ) )
        {
          ReturnValue= WCFInitFontSelector ( lpCurrentChooseFont, hWnd );
        }
        if ( lpCurrentChooseFont->Flags & CF_INITTOLOGFONTSTRUCT )
        {
          if ( lpCurrentChooseFont->lpLogFont->lfHeight > 0 )
            wsprintf ( String, "%d", lpCurrentChooseFont->lpLogFont->lfHeight );
          else
          if ( lpCurrentChooseFont->lpLogFont->lfHeight == 0 )
            wsprintf ( String, "%d", 10 );
          else
          {
            fSize = lpCurrentChooseFont->lpLogFont->lfHeight * ( -1 );
            fSize -=
              ( ( ( double ) lpCurrentChooseFont->lpLogFont->lfHeight / 15.0 ) +
                1.0 );
            fSize *= ( double ) 72.0;
            fSize /= ( double ) LOGPIXELSY;
            wsprintf ( String, "%d", ( int ) fSize );
          }
          SetWindowText ( GetDlgItem ( hWnd, WCFCBFontSize ),
                          ( LPSTR ) String );
          switch ( lpCurrentChooseFont->lpLogFont->lfWeight )
          {
            default:
              ResourceID = WCF_IDS_NORMAL;
              break;
            case FW_THIN:
              ResourceID = WCF_IDS_THIN;
              break;
            /*--- FW_ULTRALIGHT == FW_EXTRALIGHT
            case FW_ULTRALIGHT:
              ResourceID = WCF_IDS_ULTRALIGHT;
            ---*/
            case FW_EXTRALIGHT:
              ResourceID = WCF_IDS_EXTRALIGHT;
              break;
            case FW_LIGHT:
              ResourceID = WCF_IDS_LIGHT;
              break;
            case FW_MEDIUM:
              ResourceID = WCF_IDS_MEDIUM;
              break;
            /*--- FW_DEMIBOLD == FW_SEMIBOLD
            case FW_DEMIBOLD:
              ResourceID = WCF_IDS_DEMIBOLD;
            ---*/
            case FW_SEMIBOLD:
              ResourceID = WCF_IDS_SEMIBOLD;
              break;
            /*--- FW_ULTRABOLD == FW_EXTRABOLD
            case FW_ULTRABOLD:
              ResourceID = WCF_IDS_ULTRABOLD;
            ---*/
            case FW_EXTRABOLD:
              ResourceID = WCF_IDS_EXTRABOLD;
            break;
            /*--- FW_HEAVY == FW_BLACK
            case FW_HEAVY:
              ResourceID = WCF_IDS_HEAVY;
            ---*/
            case FW_BLACK:
              ResourceID = WCF_IDS_BLACK;
              break;
          }
          SendMessage ( GetDlgItem ( hWnd, WCFCBFontName ), CB_SELECTSTRING, 0,
            ( LPARAM ) ( ( LPSTR ) lpCurrentChooseFont->lpLogFont->lfFaceName ) );
          WCFDisplayFontData ( hWnd );
          if ( LoadString ( GetInstance (), ResourceID, String,
                            sizeof ( String ) ) )
            if ( SendMessage ( GetDlgItem ( hWnd, WCFCBFontStyle ),
                                CB_SELECTSTRING, 0,
                                ( LPARAM ) ( ( LPSTR ) String ) ) == CB_ERR )
              SendMessage ( GetDlgItem ( hWnd, WCFCBFontStyle ), CB_SETCURSEL,
                            0, (LPARAM)NULL );
          CheckDlgButton ( hWnd, WCFCheckStrikeout,
                            lpCurrentChooseFont->lpLogFont->lfStrikeOut );
          CheckDlgButton ( hWnd, WCFCheckUnderline,
                            lpCurrentChooseFont->lpLogFont->lfUnderline );
          WCFDisplaySampleText ( hWnd, TRUE );
       }
        if ( lpCurrentChooseFont->Flags & CF_ENABLEHOOK )
          (*lpCurrentChooseFont->lpfnHook )( hWnd, Message, wParam, lParam );
	ReturnValue = 1;
        break;

      case WM_COMMAND:
        switch ( GET_WM_COMMAND_ID(wParam, lParam) )
        {
          default:
            ReturnValue = FALSE;
            break;

          case WCFCBFontName:
            if ( GET_WM_COMMAND_CMD(wParam, lParam) == CBN_SELCHANGE )
              WCFDisplayFontData ( hWnd );
            ReturnValue = 1;
            break;

          case WCFCBFontColor:
            if ( GET_WM_COMMAND_CMD(wParam, lParam) == CBN_SELCHANGE )
              WCFDisplaySampleText ( hWnd, TRUE );
            ReturnValue = 1;
            break;

          case WCFCBFontSize:
          case WCFCBFontStyle:
            if ( GET_WM_COMMAND_CMD(wParam, lParam) == CBN_SELCHANGE )
              WCFDisplaySampleText ( hWnd, FALSE );
            ReturnValue = 1;
            break;

          case WCFCheckUnderline:
          case WCFCheckStrikeout:
            WCFDisplaySampleText ( hWnd, FALSE );
            ReturnValue = 1;
            break;

          case IDCANCEL:
          case IDOK:
            PostMessage ( hWnd, WM_CLOSE, GET_WM_COMMAND_ID(wParam, lParam), ( LPARAM )NULL );
            ReturnValue = 1;
            break;
        }
        break;
    }

  if ( ReturnValue == -1 )
	ReturnValue = 0;

  SetWindowLong ( hWnd, DWL_MSGRESULT, ReturnValue ); 

  return ( ReturnValue );

}

/*------------------------------------------------------------------------------
**
** Replacement for windows common dialog to choose font
**
**----------------------------------------------------------------------------*/

BOOL WINAPI
ChooseFont
  ( CHOOSEFONT FAR*           lpCF )

{

  HRSRC                       hResource    = ( HRSRC )NULL;
  HGLOBAL                     hResDialog   = ( HGLOBAL )NULL;
  LPVOID                      lpResource   = ( LPVOID )NULL;
  HGLOBAL                     hUseDialog   = ( HGLOBAL )NULL;
  HINSTANCE                   hUseInstance = ( HINSTANCE )NULL;
  CHOOSEFONT                  CurrentChooseFont;

  TError                       ReturnedCode = 0;
  TError                       ErrorCode    = 0;


  lpCurrentChooseFont = &CurrentChooseFont;
  if ( lpCF )
    _fmemcpy ( lpCurrentChooseFont, lpCF, sizeof ( CHOOSEFONT ) );
  /*----- Check for parameters that make sense -----*/
  if ( ( ReturnedCode = WCFCheckParameters ( ) ) ||
  /*----- Initialize state -----*/
    ( ( ReturnedCode = WCFInitialize ( ) ) ) )
    ErrorCode = ReturnedCode;
  else
  {
    /*--- Get Dialog handle ---*/
    if ( lpCurrentChooseFont->Flags & CF_ENABLETEMPLATEHANDLE )
      /*-- Dialog handle was passed by caller --*/
      hUseDialog = lpCurrentChooseFont->hInstance;
    else
    { /*-- Get Dialog from resource --*/
      if ( lpCurrentChooseFont->Flags & CF_ENABLETEMPLATE )
      { /*-- Dialog is in callers resource section
              NOTE that hUseInstance is set --*/
        if ( ! ( hResource = FindResource ( hUseInstance =
                                              lpCurrentChooseFont->hInstance,
                                            lpCurrentChooseFont->lpTemplateName,
                                            RT_DIALOG ) ) )
          ErrorCode = CDERR_FINDRESFAILURE;
      }
      else
      { /*-- Use default dialog in library resource section
              NOTE that hUseInstance is set --*/
        if ( ! ( hResource = FindResource ( hUseInstance = GetInstance (),
                                            CHOOSEFONTDIALOG,
                                            RT_DIALOG ) ) )
          ErrorCode = CDERR_FINDRESFAILURE;
      }
      if ( ! ErrorCode )
        /*-- See above FindResource call for initialization of hUseInstance --*/
        if ( ! ( hResDialog = LoadResource ( hUseInstance, hResource ) ) )
          ErrorCode = CDERR_LOADRESFAILURE;
        else
        if ( ! ( lpResource = LockResource ( hResDialog ) ) )
          ErrorCode = CDERR_LOCKRESFAILURE;
        else
          hUseDialog = hResDialog;
    }
    if ( lpResource )
    {   /*--- Which would mean that no error has occured ---*/
#ifdef TWIN32
		switch ( ReturnedCode = DialogBoxIndirectParam( GetInstance (),
                                   lpResource,
                                   lpCurrentChooseFont->hwndOwner,
                                   ( DLGPROC )WCFDialogProc,
                                   ( LPARAM )lpCurrentChooseFont ) )
#else
		switch ( ReturnedCode = DialogBoxIndirectParam( GetInstance (),
                                   hResDialog,
                                   lpCurrentChooseFont->hwndOwner,
                                   ( DLGPROC )WCFDialogProc,
                                   ( LPARAM )lpCurrentChooseFont ) )
#endif
      {
        default:
          break;

        case IDOK:
          ReturnedCode = 0;
          _fmemcpy ( lpCF, lpCurrentChooseFont, sizeof ( CHOOSEFONT ) );
          break;

      }
    }
  }

  /*----- Clean up -----*/
  if ( lpResource )
  {
    UnlockResource ( hResDialog );
    lpResource = NULL;
  }
  if ( hResDialog )
  {
    FreeResource ( hResDialog );
    hResDialog = ( HGLOBAL )NULL;
  }

  /*----- Set Error conditions -----*/
  if ( ReturnedCode )
    ErrorCode = ReturnedCode;


  /*----- if and error occured return FALSE otherwise return TRUE -----*/
  return ( ErrorCode ? FALSE : TRUE );

}

