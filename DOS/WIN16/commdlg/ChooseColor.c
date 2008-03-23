/*    
	ChooseColor.c	1.16  common dialogs Choose Color
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
#include "windows.h"
#include "windowsx.h"
#include "commdlg.h"
#include "cderr.h"

#include <stdlib.h>
#include <string.h>
#include "colordlg.h"
#include "dlgs.h"

#include "CommdlgRC.h"



/*===== External Platform specific defines =====*/
#define WCCCALLBACK                        CALLBACK _export



/*===== Required defines =====*/

#define TError				DWORD
#define NUM_BASIC_COLORS                48
#define NUM_CUSTOM_COLORS               16
#define MAX_HUE                         240
#define MAX_LUMINOSITY                  240
#define MAX_SATURATION                  240
#define MAX_RGB                         255
#define NUM_COLORS_PER_ROW              8
#define NUM_BASIC_COLOR_ROWS           ( NUM_BASIC_COLORS / NUM_COLORS_PER_ROW )
#define NUM_CUSTOM_COLOR_ROWS          ( NUM_CUSTOM_COLORS / NUM_COLORS_PER_ROW )
#define NUM_PIXELS_BETWEEN_COLORS       5
#define GRADATION_SIZE                  4


/*=== Redefine standard windows control names to more meaningful names =====*/
#define WCCWinBasicColors               COLOR_BOX1
#define WCCWinCustomColors              COLOR_CUSTOM1
#define WCCBtnDefineCustomColors        COLOR_MIX
#define WCCWinColors                    COLOR_RAINBOW
#define WCCWinLuminosity                COLOR_LUMSCROLL
#define WCCWinCurrentColor              COLOR_CURRENT
#define WCCBtnColorSolid                COLOR_SOLID
#define WCCEdtColorHue                  COLOR_HUE
#define WCCEdtColorSaturation           COLOR_SAT
#define WCCEdtColorLuminosity           COLOR_LUM
#define WCCEdtColorRed                  COLOR_RED
#define WCCEdtColorGreen                COLOR_GREEN
#define WCCEdtColorBlue                 COLOR_BLUE
#define WCCBtnAddCustomColor            COLOR_ADD
#define WCCBtnHelp                      pshHelp

#define SCALE( X, MAX_X, Y )            ( WORD )( ( ( float )( Y ) * ( float )( X ) + ( float )0.5 ) / ( float )( MAX_X ) )
#define WIDTH( lpRect )                 ( (lpRect)->right - (lpRect)->left )
#define HEIGHT( lpRect )                ( (lpRect)->bottom - (lpRect)->top )
#define DIVIDEWORDS( N, D )             ( WORD )( ( float )N / ( float )D )
#define MIN( A, B )                     ( ( (A) < (B) ) ? (A) : (B) )
#define MAX( A, B )                     ( ( (A) > (B) ) ? (A) : (B) )

COLORREF WCCBasicColors [ NUM_BASIC_COLORS ] = {
                                                 RGB ( 255, 128, 128 ),
                                                 RGB ( 255, 255, 232 ),
                                                 RGB ( 255, 251, 240 ),
                                                 RGB ( 0  , 255, 128 ),
                                                 RGB ( 128, 255, 255 ),
                                                 RGB ( 0  , 128, 255 ),
                                                 RGB ( 255, 128, 192 ),
                                                 RGB ( 255, 128, 255 ),
                                                 RGB ( 255, 0  , 0   ),
                                                 RGB ( 255, 255, 128 ),
                                                 RGB ( 128, 255, 0   ),
                                                 RGB ( 192, 220, 192 ),
                                                 RGB ( 0  , 255, 255 ),
                                                 RGB ( 164, 200, 240 ),
                                                 RGB ( 128, 128, 192 ),
                                                 RGB ( 255, 0  , 255 ),
                                                 RGB ( 128, 64 , 64  ),
                                                 RGB ( 255, 255, 0   ),
                                                 RGB ( 0  , 255, 0   ),
                                                 RGB ( 0  , 128, 128 ),
                                                 RGB ( 0  , 64 , 128 ),
                                                 RGB ( 128, 128, 255 ),
                                                 RGB ( 128, 0  , 64  ),
                                                 RGB ( 255, 0  , 128 ),
                                                 RGB ( 128, 0  , 0   ),
                                                 RGB ( 255, 128, 0   ),
                                                 RGB ( 0  , 128, 0   ),
                                                 RGB ( 0  , 128, 64  ),
                                                 RGB ( 0  , 0  , 255 ),
                                                 RGB ( 0  , 0  , 64  ),
                                                 RGB ( 128, 0  , 128 ),
                                                 RGB ( 128, 0  , 255 ),
                                                 RGB ( 64 , 0  , 0   ),
                                                 RGB ( 128, 64 , 0   ),
                                                 RGB ( 0  , 64 , 0   ),
                                                 RGB ( 0  , 64 , 64  ),
                                                 RGB ( 0  , 0  , 128 ),
                                                 RGB ( 160, 160, 160 ),
                                                 RGB ( 64 , 0  , 64  ),
                                                 RGB ( 64 , 0  , 128 ),
                                                 RGB ( 0  , 0  , 0   ),
                                                 RGB ( 128, 128, 0   ),
                                                 RGB ( 128, 128, 64  ),
                                                 RGB ( 128, 128, 128 ),
                                                 RGB ( 64 , 128, 128 ),
                                                 RGB ( 192, 192, 192 ),
                                                 RGB ( 64 , 0  , 64  ),
                                                 RGB ( 255, 255, 255 )
                                               };

typedef struct WCC_TAG
{
  CHOOSECOLOR FAR*            lpChooseColor;
	BOOL                        ColorPaletteInit;
  HCURSOR                     hColorCursor;
  HICON                       hIconColorCursor;
  HCURSOR                     hLuminosityCursor;
  HICON                       hIconLuminosityCursor;
  HBITMAP                     hColorBitmap;
  WORD                        FullWindowWidth;
  WORD                        BasicColor;
  WORD                        CustomColor;
  WORD                        Hue;
  WORD                        Luminosity;
  WORD                        Saturation;
  WORD                        Red;
  WORD                        Green;
  WORD                        Blue;
  BOOL                        InCommandMessage;
	BOOL												Hiding;
  WORD                        CustomColorIndex;
	RECT												CurrentFocusRect;
}
WCC;


/*===== Globals =====*/
static WCC                    WCCStruct;
static float                  MaxHue2  = ( float )MAX_HUE / ( float )2;
static float                  MaxHue3  = ( float )MAX_HUE / ( float )3;
static float                  MaxHue6  = ( float )MAX_HUE / ( float )6;
static float                  MaxHue12 = ( float )MAX_HUE / ( float )12;


HINSTANCE
GetInstance
  ( void )
{
  static HINSTANCE            hInst = ( HINSTANCE ) NULL;

  if ( ! hInst )
  {
    if (!(hInst =  GetModuleHandle("COMMDLG")))
      hInst = LoadLibrary("COMMDLG");
  }

  return hInst;

}



int WINAPI
fatoi
  ( LPSTR                     lpStr );

static WORD
HueToRGB
  ( WORD                      Value1,
    WORD                      Value2,
    WORD                      Hue )

{

	float                       fValue1     = Value1;
	float                       fValue2     = Value2;
  float                       fHue        = Hue;
  float                       ReturnValue = Value1;

  if ( Hue >= MAX_HUE )
    Hue %= MAX_HUE;

  if ( fHue < MaxHue6 )
    ReturnValue = fValue1 + ( ( fValue2 - fValue1 ) * fHue + MaxHue12 ) / 
                            MaxHue12;
	else
	if ( fHue < MaxHue2 )
		ReturnValue = fValue2;
	else
	if ( Hue < ( WORD )MulDiv ( MAX_HUE, 2, 3 ) )
    ReturnValue = fValue1 + 
                  ( ( ( fValue2 - fValue1 ) * MulDiv ( MAX_HUE, 2, 3 ) - fHue + 
                      MaxHue12 ) / MaxHue6 );

  return ( ( WORD ) ReturnValue );

}

static COLORREF
HLSToRGB
  ( WORD                      Hue,
    WORD                      Luminosity,
    WORD                      Saturation )

{

  float                       Red;
  float                       Green;
  float                       Blue;
  float                       IntermValue1;
  float                       IntermValue2;
	float                       fHue        = Hue;
  float                       fLuminosity = Luminosity;
  float                       fSaturation = Saturation;

  if ( Saturation == 0 )
    Red = Green = Blue =  fLuminosity * ( float )MAX_RGB / ( float )MAX_HUE;
  else
  {
    if ( fLuminosity <= MaxHue2 )
      IntermValue2 = ( fLuminosity * ( MAX_HUE + fSaturation ) + MaxHue2 ) / 
                     MAX_HUE;
    else
      IntermValue2 = fLuminosity + fSaturation - ( fLuminosity * fSaturation + 
                                                 MaxHue2 ) / MAX_HUE;
    IntermValue1 = fLuminosity * 2 - IntermValue2;

    Red = HueToRGB ( ( WORD )IntermValue1, ( WORD )IntermValue2,
                     ( WORD )( fHue + MaxHue3 ) );
    Red *= MAX_RGB;
    Red += MaxHue2;
    Red /= MAX_HUE;

    Green = HueToRGB ( ( WORD )IntermValue1, ( WORD )IntermValue2, Hue );
    Green *= MAX_RGB;
    Green += MaxHue2;
    Green /= MAX_HUE;

    Blue = HueToRGB ( ( WORD )IntermValue1, ( WORD )IntermValue2,
                       Hue - ( WORD )MaxHue3 );
    Blue *= MAX_RGB;
    Blue += MaxHue2;
    Blue /= MAX_HUE;

  }

  return ( RGB ( ( WORD )Red, ( WORD )Green, ( WORD )Blue ) );

}

static void RGBToHLS
  ( COLORREF                  RGBValue,
    LPWORD                    lpHue,
    LPWORD                    lpLuminosity,
    LPWORD                    lpSaturation )

{

  float                        DeltaRed;
  float                        DeltaGreen;
  float                        DeltaBlue;
  float                        Red           = GetRValue ( RGBValue );
  float                        Green         = GetGValue ( RGBValue );
  float                        Blue          = GetBValue ( RGBValue );
  float                        MaxBrightness = MAX ( MAX ( Red, Green ), Blue );
  float                        MinBrightness = MIN ( MIN ( Red, Green ), Blue );
  WORD                         Hue;


  *lpLuminosity = ( WORD )( ( ( ( MaxBrightness + 
																	MinBrightness ) *  
																	MAX_HUE ) + MAX_RGB ) / 
																	( MAX_RGB * 2 ) + 0.5 );

  if ( MaxBrightness == MinBrightness )
    *lpSaturation = 0;
  else
  {
    if ( *lpLuminosity <= ( WORD )( MaxHue2 ) )
      *lpSaturation = ( WORD ) ( ( ( ( MaxBrightness - 
																				MinBrightness ) * 
																				MAX_HUE ) +
                          ( ( MaxBrightness + 
																MinBrightness ) / 2 ) ) / 
													( MaxBrightness + MinBrightness ) + 
														0.5 );
    else
      *lpSaturation = ( WORD ) ( ( ( ( MaxBrightness - 
																				MinBrightness ) * 
																				MAX_HUE ) +
                          ( ( 2 * MAX_RGB -
															MaxBrightness - 
															MinBrightness ) / 2 ) ) /
                      ( 2 * MAX_RGB - MaxBrightness -
												MinBrightness ) + 0.5 );
    DeltaRed   = ( ( ( ( MaxBrightness - Red ) * 
														MaxHue6 ) + 
														( ( MaxBrightness - 
															MinBrightness ) / 2 ) ) /
                            ( MaxBrightness - 
															MinBrightness ) + 0.5 );
    DeltaGreen = ( ( ( ( MaxBrightness - Green ) * MaxHue6 ) + ( ( MaxBrightness - MinBrightness ) / 2 ) ) /
                            ( MaxBrightness - MinBrightness ) + 0.5 );
    DeltaBlue  = ( ( ( ( MaxBrightness - Blue ) * MaxHue6 ) + ( ( MaxBrightness - MinBrightness ) / 2 ) ) /
                            ( MaxBrightness - MinBrightness ) + 0.5 );
    if ( Red == MaxBrightness )
      Hue = ( WORD )( DeltaBlue - DeltaGreen );
    else
    if ( Green == MaxBrightness )
      Hue = ( WORD)( MaxHue3 + DeltaRed - DeltaBlue );
    else
      Hue = ( WORD )( ( ( 2 * MAX_HUE ) / 3 ) + DeltaGreen - DeltaRed );

    if ( Hue >= MAX_HUE )
      Hue %= MAX_HUE;
/*
 * Removed LJW 09/05/96, Hue is unsigned so the following is unnecessary
 *
    else
    while ( Hue < 0 )
      Hue += MAX_HUE;
*/

    *lpHue = ( WORD )Hue;
  }

}
/*------------------------------------------------------------------------------
**
** Get the control's rectangle in client cordinates of its parent
**
**----------------------------------------------------------------------------*/
void WINAPI
WCCGetControlRect
  ( HWND                      hWnd,
    WORD                      ControlID,
    LPRECT                    lpControlRect )

{

  RECT                        ControlWindowRect;
  RECT                        ControlClientRect;
  RECT                        WindowRect;
	POINT												Point;


  GetWindowRect ( GetDlgItem ( hWnd, ControlID ), &ControlWindowRect );
  GetClientRect ( GetDlgItem ( hWnd, ControlID ), &ControlClientRect );
  GetWindowRect ( hWnd, &WindowRect );
  lpControlRect->left   = ControlWindowRect.left - WindowRect.left - 
													GetSystemMetrics ( SM_CXDLGFRAME );
  lpControlRect->top    = ControlWindowRect.top - WindowRect.top -
                          GetSystemMetrics ( SM_CYDLGFRAME ) - 
													GetSystemMetrics ( SM_CYCAPTION );
  lpControlRect->right  = lpControlRect->left + ControlClientRect.right - 2;
  lpControlRect->bottom = lpControlRect->top + ControlClientRect.bottom - 2;

	Point.x = ControlWindowRect.left;
	Point.y = ControlWindowRect.top;
	ScreenToClient ( hWnd, &Point );
	lpControlRect->top    = Point.y;
	lpControlRect->left   = Point.x;
  lpControlRect->right  = lpControlRect->left + ControlClientRect.right - 2;
  lpControlRect->bottom = lpControlRect->top + ControlClientRect.bottom - 2;

}

/*------------------------------------------------------------------------------
**
**  Check that all parameters passed to choose font make sense in relation to
**  each other
**
**----------------------------------------------------------------------------*/

TError WINAPI
WCCCheckParameters
  ( void )

{

  TError                       ErrorCode = 0;

  /* Check for non-NULL pointers to needed data */
  if ( ! WCCStruct.lpChooseColor )
    ErrorCode = CDERR_INITIALIZATION;
  else
  /* Check that struct sizes match */
/*--- this check is not valid due to the differences in binary and source ports
  if ( WCCStruct.lpChooseColor->lStructSize != sizeof ( CHOOSECOLOR ) )
    ErrorCode = CDERR_STRUCTSIZE;
  else
----------*/
  /* if the CC_ENABLEHOOK flag is set then there must be a function
      that needs to be called */
  if ( ( WCCStruct.lpChooseColor->Flags & CC_ENABLEHOOK ) && 
				( ! WCCStruct.lpChooseColor->lpfnHook ) )
    ErrorCode = CDERR_NOHOOK;
  else
  /* if the CC_ENABLETEMPLATE flag is set then there must be a hInstance
      and a lpTemplate but this flag is only to be checked if the
      CC_ENABLETEMPLAGEHANDLE flag is not set */
  if ( ( WCCStruct.lpChooseColor->Flags & CC_ENABLETEMPLATE ) &&
      ( ! ( WCCStruct.lpChooseColor->Flags & CC_ENABLETEMPLATEHANDLE ) ) &&
      ( ( ! WCCStruct.lpChooseColor->hInstance ) || 
				( ! WCCStruct.lpChooseColor->lpTemplateName ) ) )
    ErrorCode = CDERR_NOTEMPLATE;
  else
  /* if the CC_ENABLETEMPLATEHANDLE flag is set then the hInstance is
      the dialog handle. */
  if ( ( WCCStruct.lpChooseColor->Flags & CC_ENABLETEMPLATEHANDLE ) &&
    ( ! WCCStruct.lpChooseColor->hInstance ) )
    ErrorCode = CDERR_NOHINSTANCE;
  else
  /* if the CC_SHOWHELP flag is set then there must be an owner window */
  if ( ( WCCStruct.lpChooseColor->Flags & CC_SHOWHELP ) &&
    ( ! WCCStruct.lpChooseColor->hwndOwner ) )
    ErrorCode = CDERR_INITIALIZATION;

  return ( ErrorCode );

}


/*------------------------------------------------------------------------------
**
**  Convert a color index value to its rectangle
**
**----------------------------------------------------------------------------*/
void WINAPI
WCCColorIndexToRect
  ( HWND                      hWnd,
    WORD                      ControlID,
    WORD                      ColorIndex,
    WORD                      NumRows,
    WORD                      PixelsBetween,
    LPRECT                    lpColorRect )

{

  RECT                        ClientRect;
  RECT                        ControlRect;

  GetClientRect ( GetDlgItem ( hWnd, ControlID ), &ClientRect );
  WCCGetControlRect ( hWnd, ControlID, &ControlRect );

  lpColorRect->left   = ( ColorIndex % NUM_COLORS_PER_ROW ) *
                          ( int )( WIDTH ( &ClientRect ) / NUM_COLORS_PER_ROW ) + 
													PixelsBetween;
  lpColorRect->top    = ( int )( ( float )ColorIndex / 
                                 ( float )NUM_COLORS_PER_ROW ) *
                        ( int )( ( float )HEIGHT ( &ClientRect ) / 
                                 ( float )NumRows ) + 
                        PixelsBetween;
  lpColorRect->right  = ( ( ColorIndex % NUM_COLORS_PER_ROW ) + 1 ) *
                          ( int )( ( float )WIDTH ( &ClientRect ) / 
                                   ( float )NUM_COLORS_PER_ROW );
  lpColorRect->bottom = ( ( ColorIndex / NUM_COLORS_PER_ROW ) + 1 ) *
                          ( int )( ( float )HEIGHT ( &ClientRect ) / 
                                   ( float )NumRows );

  OffsetRect ( lpColorRect, ControlRect.left, ControlRect.top );

}
/*------------------------------------------------------------------------------
**
**  Convert a Point to the appropreate color index
**
**----------------------------------------------------------------------------*/
void WINAPI
WCCColorPointToIndex
  ( HWND                      hWnd,
    WORD                      ControlID,
    POINT                     Point,
    WORD                      NumRows,
    WORD                      PixelsBetween,
    LPWORD                    lpColorIndex )

{

	POINT												WindowPoint;
	RECT												WindowRect;
	WORD												Width;
	WORD												Height;


	GetWindowRect ( GetDlgItem ( hWnd, ControlID ), &WindowRect );
	Width         = ( WORD )( WIDTH ( &WindowRect ) - NUM_PIXELS_BETWEEN_COLORS ) / 
									NUM_COLORS_PER_ROW;
	Height        = ( WORD )( HEIGHT ( &WindowRect ) - NUM_PIXELS_BETWEEN_COLORS ) / 
									NumRows;
	WindowPoint.x = WindowRect.left;
	WindowPoint.y = WindowRect.top;
	ScreenToClient ( hWnd, &WindowPoint );
	Point.x -= WindowPoint.x;
	Point.y -= WindowPoint.y;
	/*--- The point is now in Client Coordinates of ControlID ---*/
  *lpColorIndex = ( WORD )( ( float )Point.y / ( float )Height ) * 
                  NUM_COLORS_PER_ROW +
                  ( WORD )( ( float )Point.x / ( float )Width );

}
/*------------------------------------------------------------------------------
**
**  Paint Color Rectangle
**
**----------------------------------------------------------------------------*/
TError WINAPI
WCCPaintRect
  ( HDC                       hDC,
    LPRECT                    lpRect,
    COLORREF                  Color,
    BOOL                      DrawBorder )

{

  RECT                        RectToUse;
  HBRUSH                      hBrush = ( HBRUSH ) NULL;
	HGDIOBJ											hObject;

  TError                       ErrorCode = 0;


  CopyRect ( &RectToUse, lpRect );
  if ( DrawBorder )
  {
		hObject = SelectObject ( hDC, GetStockObject ( BLACK_PEN ) );
    Rectangle ( hDC, RectToUse.left, RectToUse.top, RectToUse.right, 
								RectToUse.bottom );
    InflateRect ( &RectToUse, -1, -1 );
		SelectObject ( hDC, hObject );
  }

  if ( ! ( hBrush = CreateSolidBrush ( Color ) ) )
    ErrorCode = CDERR_DIALOGFAILURE;
  else
    FillRect ( hDC, &RectToUse, hBrush );

  if ( hBrush )
    DeleteObject ( hBrush );

  return ( ErrorCode );

}

/*------------------------------------------------------------------------------
**
**  Paint Basic Color Controls
**
**----------------------------------------------------------------------------*/
TError WINAPI
WCCPaintBasicColors
  ( HWND                      hWnd,
    LPPAINTSTRUCT             lpPaintStruct,
    LPRECT                    lpPaintRect )

{

  WORD                        ColorIndex;
  RECT                        BasicColorRect;

  TError                       ErrorCode = 0;


  for ( ColorIndex = 0; ( ! ErrorCode ) && (  ColorIndex < NUM_BASIC_COLORS ); 
				ColorIndex++ )
  {
    WCCColorIndexToRect ( hWnd, WCCWinBasicColors, ColorIndex, 
													NUM_BASIC_COLOR_ROWS, NUM_PIXELS_BETWEEN_COLORS, 
													&BasicColorRect );
    ErrorCode = WCCPaintRect ( lpPaintStruct->hdc, &BasicColorRect, 
																WCCBasicColors [ ColorIndex ], TRUE );
  }

  return ( ErrorCode );

}
/*------------------------------------------------------------------------------
**
**  Paint Custom Color Controls
**
**----------------------------------------------------------------------------*/
TError WINAPI
WCCPaintCustomColors
  ( HWND                      hWnd,
    LPPAINTSTRUCT             lpPaintStruct,
    LPRECT                    lpPaintRect )

{

  WORD                        ColorIndex;
  RECT                        CustomColorRect;

  TError                       ErrorCode = 0;


  for ( ColorIndex = 0; ( ! ErrorCode ) && (  ColorIndex < NUM_CUSTOM_COLORS ); 				ColorIndex++ )
  {
    WCCColorIndexToRect ( hWnd, WCCWinCustomColors, ColorIndex, 
													NUM_CUSTOM_COLOR_ROWS,
                          NUM_PIXELS_BETWEEN_COLORS, &CustomColorRect );
    ErrorCode = 
			WCCPaintRect ( lpPaintStruct->hdc, &CustomColorRect,
                      WCCStruct.lpChooseColor->lpCustColors [ ColorIndex ], 
											TRUE );
  }

  return ( ErrorCode );

}
/*------------------------------------------------------------------------------
**
**  Paint Colors Control
**
**----------------------------------------------------------------------------*/
TError WINAPI
WCCPaintColors
  ( HWND                      hWnd,
    LPPAINTSTRUCT             lpPaintStruct,
    LPRECT                    lpPaintRect )

{

  RECT                        Rect;
  HDC                         hDC = ( HDC )NULL;
  HDC                         hColorDC = ( HDC )NULL;

  TError                       ErrorCode = 0;


  GetClientRect ( GetDlgItem ( hWnd, WCCWinColors ), &Rect );
  if ( ( hDC = GetDC ( GetDlgItem ( hWnd, WCCWinColors ) ) ) &&
       ( hColorDC = CreateCompatibleDC ( hDC ) ) &&
       ( WCCStruct.hColorBitmap != (HBITMAP)NULL ) )
  {
    SelectObject ( hColorDC, WCCStruct.hColorBitmap );
    BitBlt ( hDC, 1, 1, WIDTH ( &Rect ) - 2, HEIGHT ( &Rect ) - 2,
              hColorDC, 0, 0, SRCCOPY );
  }

  if ( hColorDC )
    DeleteDC ( hColorDC );

  if ( hDC )
    ReleaseDC ( GetDlgItem ( hWnd, WCCWinColors ), hDC );

  return ( ErrorCode );

}
/*------------------------------------------------------------------------------
**
**  Paint Luminosity Control
**
**----------------------------------------------------------------------------*/
TError WINAPI
WCCPaintLuminosity
  ( HWND                      hWnd,
    LPPAINTSTRUCT             lpPaintStruct,
    LPRECT                    lpPaintRect )

{

  RECT                        LuminosityRect;
  HDC                         hDC;
  int                         MaxY;
  int                         YValue;
  WORD                        Luminosity;
  HBRUSH                      hBrush;

  TError                       ErrorCode = 0;

  if ( ! ( hDC = GetDC ( GetDlgItem ( hWnd, WCCWinLuminosity ) ) ) )
    ErrorCode = CDERR_DIALOGFAILURE;
  else
  {
    GetClientRect ( GetDlgItem ( hWnd, WCCWinLuminosity ), &LuminosityRect );
    InflateRect ( &LuminosityRect, -1, -1 );
    MaxY = LuminosityRect.bottom;
    for ( YValue = 1; YValue < MaxY; YValue += GRADATION_SIZE )
    {
      Luminosity = MAX_LUMINOSITY - SCALE ( YValue, MaxY, MAX_LUMINOSITY );
      hBrush = CreateSolidBrush ( HLSToRGB ( WCCStruct.Hue, Luminosity, 
																							WCCStruct.Saturation ) );
      LuminosityRect.top    = YValue;
      LuminosityRect.bottom = YValue + GRADATION_SIZE;
      if ( LuminosityRect.bottom >= MaxY )
        LuminosityRect.bottom = MaxY;
      FillRect ( hDC, &LuminosityRect, hBrush );
      DeleteObject ( hBrush );
    }
    ReleaseDC ( hWnd, hDC );
  }


  return ( ErrorCode );

}
/*------------------------------------------------------------------------------
**
**  Paint Colors Control
**
**----------------------------------------------------------------------------*/
TError WINAPI
WCCPaintCurrentColor
  ( HWND                      hWnd,
    LPPAINTSTRUCT             lpPaintStruct,
    LPRECT                    lpPaintRect )

{

  RECT                        Rect;
  HDC                         hDC;
  HBRUSH                      hBrush;

  TError                       ErrorCode = 0;


  if ((hDC = GetDC ( GetDlgItem ( hWnd, WCCWinCurrentColor ) )))
  {
    GetClientRect ( GetDlgItem ( hWnd, WCCWinCurrentColor ), &Rect );
    InflateRect ( &Rect, -1, -1 );
    Rect.right = ( Rect.left + Rect.right ) / 2;
    FillRect ( hDC, &Rect,
      hBrush = CreateSolidBrush ( RGB ( WCCStruct.Red, WCCStruct.Green, 
																	WCCStruct.Blue ) ) );
    DeleteObject ( hBrush );
    GetClientRect ( GetDlgItem ( hWnd, WCCWinCurrentColor ), &Rect );
    InflateRect ( &Rect, -1, -1 );
    Rect.left = ( Rect.left + Rect.right ) / 2;
    FillRect ( hDC, &Rect,
      hBrush = CreateSolidBrush ( GetNearestColor ( hDC, RGB ( WCCStruct.Red, WCCStruct.Green, WCCStruct.Blue ) ) ) );
    DeleteObject ( hBrush );
    ReleaseDC ( GetDlgItem ( hWnd, WCCWinCurrentColor ), hDC );
  }

  return ( ErrorCode );

}
/*------------------------------------------------------------------------------
**
**  Paint Control
**
**----------------------------------------------------------------------------*/
TError WINAPI
WCCPaintControl
  ( HWND                      hWnd,
    WORD                      ControlID,
    LPPAINTSTRUCT             lpPaintStruct,
    FARPROC                   lpfnPaintControl )

{

  RECT                        ControlRect;
  RECT                        CommonRect;

  TError                       ErrorCode = 0;


  WCCGetControlRect ( hWnd, ControlID, &ControlRect );
  if ( IntersectRect ( &CommonRect, &ControlRect, &( lpPaintStruct->rcPaint ) ) )
    ErrorCode = ( *lpfnPaintControl )( hWnd, lpPaintStruct, ( LPRECT )&CommonRect );

  return ( ErrorCode );

}
/*------------------------------------------------------------------------------
**
**  Color Initialization
**
**----------------------------------------------------------------------------*/
TError WINAPI
WCCInitColor
  ( HWND                      hWnd )

{

  RECT                        Rect;
  HBRUSH                      hBrush;
  int                         MaxX;
  int                         MaxY;
  int                         XValue;
  int                         YValue;
  int                         HueIndex;
  int                         UseRed;
  int                         UseGreen;
  int                         UseBlue;
  int                         Red           = 0;
  int                         Green         = 0;
  int                         Blue          = 0;
  HDC                         hDC           = ( HDC )NULL;
  HDC                         hColorDC      = ( HDC )NULL;
  WORD                        ModifiedHueIndex;

  TError                       ErrorCode     = 0;


  if ( ! ( hDC = GetDC ( GetDlgItem ( hWnd, WCCWinColors ) ) ) )
    ErrorCode = CDERR_DIALOGFAILURE;
  else
  {
    GetClientRect ( GetDlgItem ( hWnd, WCCWinColors ), &Rect );
    MaxX = Rect.right;
    MaxY = Rect.bottom;
    if ( ! ( hColorDC = CreateCompatibleDC ( hDC ) ) )
      ErrorCode = CDERR_DIALOGFAILURE;
    else
    if ( ! ( WCCStruct.hColorBitmap = CreateCompatibleBitmap ( hDC, MaxX - 1, MaxY - 1 ) ) )
      ErrorCode = CDERR_DIALOGFAILURE;
    else
    {
      SelectObject ( hColorDC, WCCStruct.hColorBitmap );
      for ( XValue = 0; XValue < MaxX; XValue += GRADATION_SIZE )
      {
        HueIndex = SCALE ( XValue, ( int )MaxX - 1, MAX_HUE );
        ModifiedHueIndex = ( WORD )( HueIndex % ( int )MaxHue6 );
        switch ( ( WORD ) ( HueIndex / MaxHue6 ) )
        {
          case 0:
            Red   = MAX_RGB;
            Green = SCALE ( HueIndex, MaxHue6, MAX_RGB );
            Blue  = 0;
            break;
          case 1:
            Red   = MAX_RGB - SCALE ( ModifiedHueIndex, MaxHue6, 
                                      MAX_RGB );
            Green = MAX_RGB;
            Blue  = 0;
            break;
          case 2:
            Red   = 0;
            Green = MAX_RGB;
            Blue  = SCALE ( ModifiedHueIndex, MaxHue6, MAX_RGB );
            break;
          case 3:
            Red   = 0;
            Green = MAX_RGB - SCALE ( ModifiedHueIndex, MaxHue6, 
                                      MAX_RGB );
            Blue  = MAX_RGB;
            break;
          case 4:
            Red   = SCALE ( ModifiedHueIndex, MaxHue6, MAX_RGB );
            Green = 0;
            Blue  = MAX_RGB;
            break;
          case 5:
            Red   = MAX_RGB;
            Green = 0;
            Blue  = MAX_RGB - SCALE ( ModifiedHueIndex, MaxHue6, 
                                      MAX_RGB );
            break;
        }
        for ( YValue = 0; YValue < MaxY - 1; YValue += GRADATION_SIZE )
        {
          if ( Red > 127 )
            UseRed = Red - SCALE ( YValue, MaxY, abs ( Red - 127 ) );
          else
            UseRed = Red + SCALE ( YValue, MaxY, abs ( Red - 127 ) );
          if ( Green > 127 )
            UseGreen = Green - SCALE ( YValue, MaxY, abs ( Green - 127 ) );
          else
            UseGreen = Green + SCALE ( YValue, MaxY, abs ( Green - 127 ) );
          if ( Blue > 127 )
            UseBlue = Blue - SCALE ( YValue, MaxY, abs ( Blue - 127 ) );
          else
            UseBlue = Blue + SCALE ( YValue, MaxY, abs ( Blue - 127 ) );
          hBrush = CreateSolidBrush ( RGB ( UseRed, UseGreen, UseBlue ) );
          Rect.left  = XValue;
          Rect.right = XValue + GRADATION_SIZE;
          if ( Rect.right >= MaxX )
            Rect.right = MaxX - 1;
          Rect.top    = YValue;
          Rect.bottom = YValue + GRADATION_SIZE;
          if ( Rect.bottom >= MaxY )
            Rect.bottom = MaxY - 1;
          FillRect ( hColorDC, &Rect, hBrush );
          DeleteObject ( hBrush );
        } /*--- end for YValue ---*/
      } /*--- end for XValue ---*/
    } /*--- end else CreateCompatibleBitmap ---*/
  } /*--- end GetDC ---*/

  if ( hColorDC )
    DeleteDC ( hColorDC );

  return ( ErrorCode );

}
/*------------------------------------------------------------------------------
**
**  Show or Hide the Color Cursor
**
**----------------------------------------------------------------------------*/
TError WINAPI
WCCShowColorCursor
  ( HWND                      hWnd,
    BOOL                      Show,
    WORD                      X,
    WORD                      Y,
    LPRECT                    lpColorRect )

{

  HDC                         hDC = ( HDC )NULL;
  RECT                        ClipRect;
  RECT                        ControlRect;
  HRGN                        hClipRegion;

  TError                       ErrorCode = 0;


  if ( ! Show )
    ErrorCode = WCCPaintColors ( hWnd, NULL, NULL );
  else
  {
    if ( ! lpColorRect )
    {
      lpColorRect = &ControlRect;
      WCCGetControlRect ( hWnd, WCCWinColors, lpColorRect );
      X = SCALE ( WCCStruct.Hue, MAX_HUE, WIDTH ( lpColorRect ) ) + lpColorRect->left;
      Y = SCALE ( MAX_SATURATION - ( int )WCCStruct.Saturation, MAX_SATURATION, HEIGHT ( lpColorRect ) ) + ( WORD )lpColorRect->top;
    }
    if (( hDC = GetDC ( GetDlgItem ( hWnd, WCCWinColors ) ) ))
    {
      switch ( GetClipBox ( hDC, &ClipRect ) )
      {
        case SIMPLEREGION:
        case NULLREGION:
          if (( hClipRegion = CreateRectRgn ( 0, 0,
                                            lpColorRect->right - lpColorRect->left + 2,
                                            lpColorRect->bottom - lpColorRect->top + 1 ) ))
          {
            SelectClipRgn ( hDC, hClipRegion );
            DrawIcon ( hDC, X - 14 - lpColorRect->left, Y - 15 - lpColorRect->top, WCCStruct.hIconColorCursor );
            DeleteObject ( hClipRegion );
            if (( hClipRegion = CreateRectRgn ( ClipRect.left, ClipRect.top,
                                           ClipRect.right, ClipRect.bottom)))
            {
              SelectClipRgn ( hDC, hClipRegion );
              DeleteObject ( hClipRegion );
            }
          }
          break;

        case COMPLEXREGION:
        default:
          break;
      }
      ReleaseDC ( GetDlgItem ( hWnd, WCCWinColors ), hDC );
    }
  }

  return ( ErrorCode );

}

/*------------------------------------------------------------------------------
**
**  Show or Hide the Luminosity Cursor
**
**----------------------------------------------------------------------------*/
TError WINAPI
WCCShowLuminosityCursor
  ( HWND                      hWnd,
    BOOL                      Show,
    WORD                      X,
    WORD                      Y,
    LPRECT                    lpLuminosityRect )

{

  HDC                         hDC;
  RECT                        Rect;

  TError                       ErrorCode = 0;


  if ( lpLuminosityRect )
  {
    Rect.left = lpLuminosityRect->right + 1;
    Rect.top  = lpLuminosityRect->bottom - 
								SCALE ( WCCStruct.Luminosity, MAX_LUMINOSITY,
                        HEIGHT ( lpLuminosityRect ) ) - 16;
  }
  else
  {
    WCCGetControlRect ( hWnd, WCCWinLuminosity, &Rect );
    Rect.left = Rect.right + 1;
    Rect.top  = Rect.bottom - 
								SCALE ( WCCStruct.Luminosity, MAX_LUMINOSITY, 
												HEIGHT ( &Rect ) ) - 16;
  }

  Rect.right  = Rect.left + 32;
  Rect.bottom = Rect.top + 32;
  if ( Show )
  {
    if (( hDC = GetDC ( hWnd ) ))
    {
      DrawIcon ( hDC, Rect.left, Rect.top, WCCStruct.hIconLuminosityCursor );
      ReleaseDC ( hWnd, hDC );
    }
  }
  else
	{
    InvalidateRect ( hWnd, &Rect, TRUE );
		WCCStruct.Hiding = TRUE;
		UpdateWindow ( hWnd );
		WCCStruct.Hiding = FALSE;
	}

  return ( ErrorCode );

}

/*------------------------------------------------------------------------------
**
**  Set a Custom Color control
**
**----------------------------------------------------------------------------*/
void WINAPI
WCCSetCustomColor
  ( HWND                      hWnd )

{

  HDC                         hDC;
  RECT                        CustomColorRect;

	if (( hDC = GetDC ( hWnd ) ))
	{
		if ( WIDTH ( &WCCStruct.CurrentFocusRect ) )
			DrawFocusRect ( hDC, &WCCStruct.CurrentFocusRect );
		if ( WCCStruct.CustomColorIndex + 1 == NUM_CUSTOM_COLORS )
			WCCStruct.CustomColorIndex = 0;
		else
		if ( WCCStruct.CustomColorIndex < ( WORD )( NUM_CUSTOM_COLORS / 2 ) )
			WCCStruct.CustomColorIndex += NUM_CUSTOM_COLORS / 2;
		else
			WCCStruct.CustomColorIndex -= ( NUM_CUSTOM_COLORS / 2 - 1 );
		WCCStruct.lpChooseColor->lpCustColors [ WCCStruct.CustomColorIndex ] = 
			RGB ( WCCStruct.Red, WCCStruct.Green, WCCStruct.Blue );
	 	WCCColorIndexToRect ( hWnd, WCCWinCustomColors, WCCStruct.CustomColorIndex, 
													NUM_CUSTOM_COLOR_ROWS, NUM_PIXELS_BETWEEN_COLORS, 
													&CustomColorRect );
 		WCCPaintRect ( hDC, &CustomColorRect, RGB ( WCCStruct.Red, WCCStruct.Green, 
																								WCCStruct.Blue ), TRUE );
		CopyRect ( &WCCStruct.CurrentFocusRect, &CustomColorRect );
		InflateRect ( &WCCStruct.CurrentFocusRect, 2, 2 );
		DrawFocusRect ( hDC, &WCCStruct.CurrentFocusRect );
    ReleaseDC ( hWnd, hDC );
	}

}

/*------------------------------------------------------------------------------
**
**  Set an edit control
**
**----------------------------------------------------------------------------*/
WORD WINAPI
WCCSetEditControl
  ( HWND                      hWnd,
    WORD                      ControlID,
    WORD                      Value )

{

  char                        ValueString [ 10 ];

  wsprintf ( ValueString, "%d", Value );

  SetWindowText ( GetDlgItem ( hWnd, ControlID ), ValueString );
  return ( Value );

}

/*------------------------------------------------------------------------------
**
**  Set the Hue
**
**----------------------------------------------------------------------------*/
void WINAPI
WCCSetHue
  ( HWND                      hWnd,
    WORD                      Value,
    BOOL                      Force )

{

  if ( Value == MAX_HUE ) Value--; /*--- Don't ask why all the other values are
                                         allowed to equal their max but Hue is not ---*/
  if ( ( WCCStruct.Hue != Value ) || ( Force ) )
  {
		if ( Value > MAX_HUE )
			Value = MAX_HUE;
    WCCStruct.Hue = Value;
    WCCSetEditControl ( hWnd, WCCEdtColorHue, Value );
  }

}

/*------------------------------------------------------------------------------
**
**  Set the Saturation
**
**----------------------------------------------------------------------------*/
void WINAPI
WCCSetSaturation
  ( HWND                      hWnd,
    WORD                      Value,
    BOOL                      Force )

{

  if ( ( WCCStruct.Saturation != Value ) || ( Force ) )
  {
		if ( Value > MAX_SATURATION )
			Value = MAX_SATURATION;
    WCCStruct.Saturation = Value;
    WCCSetEditControl ( hWnd, WCCEdtColorSaturation, Value );
  }

}
/*------------------------------------------------------------------------------
**
**  Set the Luminosity
**
**----------------------------------------------------------------------------*/
void WINAPI
WCCSetLuminosity
  ( HWND                      hWnd,
    WORD                      Value,
    BOOL                      Force )

{

  if ( ( WCCStruct.Luminosity != Value ) || ( Force ) )
  {
		if ( Value > MAX_LUMINOSITY )
			Value = MAX_LUMINOSITY;
    WCCStruct.Luminosity = Value;
    WCCSetEditControl ( hWnd, WCCEdtColorLuminosity, Value );
  }

}
/*------------------------------------------------------------------------------
**
**  Set the Red
**
**----------------------------------------------------------------------------*/
void WINAPI
WCCSetRed
  ( HWND                      hWnd,
    WORD                      Value,
    BOOL                      Force )

{

  if ( ( WCCStruct.Red != Value ) || ( Force ) )
  {
		if ( Value > MAX_RGB ) 
			Value = MAX_RGB;
    WCCStruct.Red = Value;
    WCCSetEditControl ( hWnd, WCCEdtColorRed, Value );
    WCCPaintCurrentColor ( hWnd, 0, 0 );
  }

}
/*------------------------------------------------------------------------------
**
**  Set the Green
**
**----------------------------------------------------------------------------*/
void WINAPI
WCCSetGreen
  ( HWND                      hWnd,
    WORD                      Value,
    BOOL                      Force )

{

  if ( ( WCCStruct.Green != Value ) || ( Force ) )
  {
		if ( Value > MAX_RGB ) 
			Value = MAX_RGB;
    WCCStruct.Green = Value;
    WCCSetEditControl ( hWnd, WCCEdtColorGreen, Value );
    WCCPaintCurrentColor ( hWnd, 0, 0 );
  }

}
/*------------------------------------------------------------------------------
**
**  Set the Blue
**
**----------------------------------------------------------------------------*/
void WINAPI
WCCSetBlue
  ( HWND                      hWnd,
    WORD                      Value,
    BOOL                      Force )

{

  if ( ( WCCStruct.Blue != Value ) || ( Force ) )
  {
		if ( Value > MAX_RGB ) 
			Value = MAX_RGB;
    WCCStruct.Blue = Value;
    WCCSetEditControl ( hWnd, WCCEdtColorBlue, Value );
    WCCPaintCurrentColor ( hWnd, 0, 0 );
  }

}
/*------------------------------------------------------------------------------
**
**  Handle Color Control Mouse Events
**
**----------------------------------------------------------------------------*/
void WINAPI
WCCColorMouseEvent
  ( HWND                      hWnd,
    UINT                      Message,
    WPARAM                    wParam,
    LPARAM                    lParam,
    BOOL FAR*                 lpIsMouseCaptured,
    LPRECT                    lpControlRect )

{

  static HCURSOR              hCursor;

  RECT                        ColorWindowRect;
  COLORREF                    Color;


  if ( *lpIsMouseCaptured )
  {
    switch ( Message )
    {
      case WM_LBUTTONUP:
        *lpIsMouseCaptured = FALSE;
        ReleaseCapture ();
        SetCursor ( hCursor );
        ClipCursor ( NULL );
        WCCShowColorCursor ( hWnd, TRUE, LOWORD ( lParam ), HIWORD ( lParam ), lpControlRect );
        break;

      case WM_MOUSEMOVE:
        WCCShowColorCursor ( hWnd, FALSE, LOWORD ( lParam ), HIWORD ( lParam ), lpControlRect );
        WCCSetHue ( hWnd, SCALE ( ( int )LOWORD ( lParam ) - lpControlRect->left,
                                  WIDTH ( lpControlRect ) - 1 ,
                                  MAX_HUE ), FALSE );
        WCCSetSaturation ( hWnd,
                            MAX_SATURATION + 1 - SCALE ( ( int )HIWORD ( lParam ) - lpControlRect->top,
                                                          HEIGHT ( lpControlRect ) - 2,
                                                          MAX_SATURATION ), FALSE );
        WCCPaintLuminosity ( hWnd, 0, 0 );
        Color = HLSToRGB ( WCCStruct.Hue, WCCStruct.Luminosity, WCCStruct.Saturation );
        WCCSetRed ( hWnd, GetRValue ( Color ), FALSE );
        WCCSetGreen ( hWnd, GetGValue ( Color ), FALSE );
        WCCSetBlue ( hWnd, GetBValue ( Color ), FALSE );
        WCCShowColorCursor ( hWnd, FALSE, LOWORD ( lParam ), HIWORD ( lParam ), lpControlRect );
        break;
    }
  }
  else
  {
    switch ( Message )
    {
      case WM_LBUTTONDOWN:
        SetCapture ( hWnd );
        *lpIsMouseCaptured = TRUE;
        hCursor = SetCursor ( WCCStruct.hColorCursor );
        GetWindowRect ( GetDlgItem ( hWnd, WCCWinColors ), &ColorWindowRect );
        InflateRect ( &ColorWindowRect, -1, -1 );
        ClipCursor ( &ColorWindowRect );
        WCCShowColorCursor ( hWnd, FALSE, LOWORD ( lParam ), HIWORD ( lParam ), lpControlRect );
        PostMessage ( hWnd, WM_MOUSEMOVE, wParam, lParam );
        break;
    }
  }

}
/*------------------------------------------------------------------------------
**
**  Handle Basic Color Control Mouse Events
**
**----------------------------------------------------------------------------*/
void WINAPI
WCCCustomColorsMouseEvent
  ( HWND                      hWnd,
    UINT                      Message,
    WPARAM                    wParam,
    LPARAM                    lParam,
    BOOL FAR*                 lpIsMouseCaptured,
    LPRECT                    lpControlRect )

{
  POINT	MousePoint;
  WORD	Hue;
  WORD	Saturation;
  WORD	Luminosity;
  WORD	ColorIndex;
  HDC	hDC;


  switch ( Message )
  {
    case WM_LBUTTONDOWN:
			WCCShowColorCursor ( hWnd, FALSE, 0, 0, NULL );
      WCCShowLuminosityCursor ( hWnd, FALSE, 0, 0, NULL );
			MousePoint.x = LOWORD ( lParam );
			MousePoint.y = HIWORD ( lParam );
			WCCColorPointToIndex ( hWnd, WCCWinCustomColors, MousePoint, NUM_CUSTOM_COLOR_ROWS, NUM_PIXELS_BETWEEN_COLORS, &ColorIndex );
			RGBToHLS ( WCCBasicColors [ ColorIndex ], &Hue, &Luminosity, &Saturation );
			WCCSetRed ( hWnd, 
									GetRValue ( WCCStruct.lpChooseColor->lpCustColors [ ColorIndex ] ), 
									FALSE );
			WCCSetGreen ( hWnd, 
										GetGValue ( WCCStruct.lpChooseColor->lpCustColors [ ColorIndex ] ), 
										FALSE );
			WCCSetBlue ( hWnd, 
										GetBValue ( WCCStruct.lpChooseColor->lpCustColors [ ColorIndex ] ), 
										FALSE );
			WCCSetHue ( hWnd, Hue, FALSE );
			WCCSetSaturation ( hWnd, Saturation, FALSE );
			WCCSetLuminosity ( hWnd, Luminosity, FALSE );
			WCCShowColorCursor ( hWnd, TRUE, 0, 0, NULL );
      WCCShowLuminosityCursor ( hWnd, TRUE, 0, 0, NULL );
      WCCPaintLuminosity ( hWnd, 0, 0 );
			if ((hDC = GetDC ( hWnd ) ))
			{
				if ((WIDTH ( &WCCStruct.CurrentFocusRect )))
					DrawFocusRect ( hDC, &WCCStruct.CurrentFocusRect );

    		    WCCColorIndexToRect ( hWnd, WCCWinBasicColors, ColorIndex, 
						NUM_BASIC_COLOR_ROWS, NUM_PIXELS_BETWEEN_COLORS, 
						&WCCStruct.CurrentFocusRect );
				InflateRect ( &WCCStruct.CurrentFocusRect, 2, 2 );
				DrawFocusRect ( hDC, &WCCStruct.CurrentFocusRect );
				ReleaseDC ( hWnd, hDC );
			}
      break;
  }

}

/*------------------------------------------------------------------------------
**
**  Handle Basic Color Control Mouse Events
**
**----------------------------------------------------------------------------*/
void WINAPI
WCCBasicColorsMouseEvent
  ( HWND                      hWnd,
    UINT                      Message,
    WPARAM                    wParam,
    LPARAM                    lParam,
    BOOL FAR*                 lpIsMouseCaptured,
    LPRECT                    lpControlRect )

{
POINT	MousePoint;
WORD	Hue;
WORD	Saturation;
WORD	Luminosity;
WORD	ColorIndex;
HDC	hDC;


  switch ( Message )
  {
    case WM_LBUTTONDOWN:
			WCCShowColorCursor ( hWnd, FALSE, 0, 0, NULL );
      WCCShowLuminosityCursor ( hWnd, FALSE, 0, 0, NULL );
			MousePoint.x = LOWORD ( lParam );
			MousePoint.y = HIWORD ( lParam );
			WCCColorPointToIndex ( hWnd, WCCWinBasicColors, MousePoint, NUM_BASIC_COLOR_ROWS, NUM_PIXELS_BETWEEN_COLORS, &ColorIndex );
			RGBToHLS ( WCCBasicColors [ ColorIndex ], &Hue, &Luminosity, &Saturation );
			WCCSetRed ( hWnd, GetRValue ( WCCBasicColors [ ColorIndex ] ), FALSE );
			WCCSetGreen ( hWnd, GetGValue ( WCCBasicColors [ ColorIndex ] ), FALSE );
			WCCSetBlue ( hWnd, GetBValue ( WCCBasicColors [ ColorIndex ] ), FALSE );
			WCCSetHue ( hWnd, Hue, FALSE );
			WCCSetSaturation ( hWnd, Saturation, FALSE );
			WCCSetLuminosity ( hWnd, Luminosity, FALSE );
			WCCShowColorCursor ( hWnd, TRUE, 0, 0, NULL );
      WCCShowLuminosityCursor ( hWnd, TRUE, 0, 0, NULL );
      WCCPaintLuminosity ( hWnd, 0, 0 );
			if ((hDC = GetDC ( hWnd )))
			{
				if ( WIDTH ( &WCCStruct.CurrentFocusRect ) )
					DrawFocusRect ( hDC, &WCCStruct.CurrentFocusRect );
    		WCCColorIndexToRect ( hWnd, WCCWinBasicColors, ColorIndex, 
															NUM_BASIC_COLOR_ROWS, NUM_PIXELS_BETWEEN_COLORS, 
															&WCCStruct.CurrentFocusRect );
				InflateRect ( &WCCStruct.CurrentFocusRect, 2, 2 );
				DrawFocusRect ( hDC, &WCCStruct.CurrentFocusRect );
				ReleaseDC ( hWnd, hDC );
			}
      break;
  }

}
/*------------------------------------------------------------------------------
**
**  Handle Current Color Control Mouse Events
**
**----------------------------------------------------------------------------*/
void WINAPI
WCCCurrentColorMouseEvent
  ( HWND                      hWnd,
    UINT                      Message,
    WPARAM                    wParam,
    LPARAM                    lParam,
    BOOL FAR*                 lpIsMouseCaptured,
    LPRECT                    lpControlRect )

{

  COLORREF                    Color;
  HDC                         hDC;
  WORD                        Hue;
  WORD                        Luminosity;
  WORD                        Saturation;


  switch ( Message )
  {
    case WM_LBUTTONDBLCLK:
      if ( LOWORD ( lParam ) > ( WORD ) ( lpControlRect->left + WIDTH ( lpControlRect ) / 2 ) )
      {
        if ((hDC = GetDC ( hWnd )))
        {
        	WCCShowLuminosityCursor ( hWnd, FALSE, 0, 0, NULL );
	        WCCShowColorCursor ( hWnd, FALSE, 0, 0, NULL );
	        Color      = GetNearestColor ( hDC, RGB ( WCCStruct.Red, WCCStruct.Green, WCCStruct.Blue ) );
	        Hue        = WCCStruct.Hue;
	        Luminosity = WCCStruct.Luminosity;
	        Saturation = WCCStruct.Saturation;
	        RGBToHLS ( Color, &Hue, &Luminosity, &Saturation );
	        WCCSetHue ( hWnd, Hue, FALSE );
	        WCCSetLuminosity ( hWnd, Luminosity, FALSE );
	        WCCSetSaturation ( hWnd, Saturation, FALSE );
	        WCCSetRed ( hWnd, GetRValue ( Color ), FALSE );
	        WCCSetGreen ( hWnd, GetGValue ( Color ), FALSE );
	        WCCSetBlue ( hWnd, GetBValue ( Color ), FALSE );
	        ReleaseDC ( hWnd, hDC );
	        WCCShowLuminosityCursor ( hWnd, TRUE, 0, 0, NULL );
	        WCCShowColorCursor ( hWnd, TRUE, 0, 0, NULL );
        }
      }
      WCCSetCustomColor ( hWnd );
      break;
  }

}
/*------------------------------------------------------------------------------
**
**  Handle Luminosity Control Mouse Events
**
**----------------------------------------------------------------------------*/
void WINAPI
WCCLuminosityMouseEvent
  ( HWND                      hWnd,
    UINT                      Message,
    WPARAM                    wParam,
    LPARAM                    lParam,
    BOOL FAR*                 lpIsMouseCaptured,
    LPRECT                    lpControlRect )

{

  static HCURSOR              hCursor;

  RECT                        LuminosityCursorRect;
  RECT                        WindowRect;
  COLORREF                    Color;


  if ( *lpIsMouseCaptured )
  {
    switch ( Message )
    {
      case WM_LBUTTONUP:
        *lpIsMouseCaptured = FALSE;
        ReleaseCapture ();
        SetCursor ( hCursor );
        ClipCursor ( NULL );
        WCCShowLuminosityCursor ( hWnd, TRUE, LOWORD ( lParam ), HIWORD ( lParam ), lpControlRect );
        break;

      case WM_MOUSEMOVE:
        WCCShowLuminosityCursor ( hWnd, FALSE, LOWORD ( lParam ), HIWORD ( lParam ), lpControlRect );
        WCCSetLuminosity ( hWnd,
                            MAX_LUMINOSITY - SCALE ( ( int )HIWORD ( lParam ) - lpControlRect->top,
                                                          HEIGHT ( lpControlRect ) - 2,
                                                          MAX_LUMINOSITY ) + 1, FALSE );
        Color = HLSToRGB ( WCCStruct.Hue, WCCStruct.Luminosity, WCCStruct.Saturation );
        WCCSetRed ( hWnd, GetRValue ( Color ), FALSE );
        WCCSetGreen ( hWnd, GetGValue ( Color ), FALSE );
        WCCSetBlue ( hWnd, GetBValue ( Color ), FALSE );
        WCCShowLuminosityCursor ( hWnd, FALSE, LOWORD ( lParam ), HIWORD ( lParam ), lpControlRect );
        break;
    }
  }
  else
  {
    switch ( Message )
    {
      case WM_LBUTTONDOWN:
        SetCapture ( hWnd );
        *lpIsMouseCaptured = TRUE;
        hCursor = SetCursor ( WCCStruct.hLuminosityCursor );
        GetWindowRect ( GetDlgItem ( hWnd, WCCWinLuminosity ), &LuminosityCursorRect );
        LuminosityCursorRect.top    += 1;
        LuminosityCursorRect.bottom -= 2;
        LuminosityCursorRect.left   = LuminosityCursorRect.right + 1;
        LuminosityCursorRect.right  = LuminosityCursorRect.left + 1;
        ClipCursor ( &LuminosityCursorRect );
        WCCShowLuminosityCursor ( hWnd, FALSE, LOWORD ( lParam ), HIWORD ( lParam ), lpControlRect );
        GetWindowRect ( hWnd, &WindowRect );
        SetCursorPos ( LuminosityCursorRect.left,
                        HIWORD ( lParam ) + WindowRect.top +
                        GetSystemMetrics ( SM_CYCAPTION ) + GetSystemMetrics ( SM_CYDLGFRAME ) );
        PostMessage ( hWnd, WM_MOUSEMOVE, wParam, lParam );
        break;
    }
  }

}
/*------------------------------------------------------------------------------
**
**  Handle Control Mouse Events
**
**----------------------------------------------------------------------------*/
BOOL WINAPI
WCCControlMouseEvent
  ( HWND                      hWnd,
    UINT                      Message,
    WPARAM                    wParam,
    LPARAM                    lParam,
    BOOL FAR*                 lpIsMouseCaptured,
    WORD                      ControlID,
    FARPROC                   lpfnControlMouseEvent )

{

  POINT                       MousePoint;
  RECT                        ControlRect;

  BOOL                        EventHandled = FALSE;


  MousePoint.x = LOWORD ( lParam );
  MousePoint.y = HIWORD ( lParam );

  WCCGetControlRect ( hWnd, ControlID, &ControlRect );
  ControlRect.bottom++;
  if ((EventHandled = PtInRect ( &ControlRect, MousePoint )))
    ( *lpfnControlMouseEvent )( hWnd, Message, wParam, lParam, lpIsMouseCaptured, ( LPRECT )&ControlRect );

  return ( EventHandled );

}

/*------------------------------------------------------------------------------
**
**  Handle Mouse Events
**
**----------------------------------------------------------------------------*/
BOOL WINAPI
WCCMouseEvent
  ( HWND                      hWnd,
    UINT                      Message,
    WPARAM                    wParam,
    LPARAM                    lParam,
    BOOL                      IsMouseCaptured )

{

  RECT                        ControlRect;


  if ( ( ! WCCControlMouseEvent ( hWnd, Message, wParam, lParam, 
																	( BOOL FAR* )&IsMouseCaptured, WCCWinColors, 
																	( FARPROC )WCCColorMouseEvent ) ) &&
        ( ! WCCControlMouseEvent ( hWnd, Message, wParam, lParam, 
																		( BOOL FAR* )&IsMouseCaptured,
                                    WCCWinBasicColors, 
																		( FARPROC )WCCBasicColorsMouseEvent ) ) &&
        ( ! WCCControlMouseEvent ( hWnd, Message, wParam, lParam, 
																		( BOOL FAR* )&IsMouseCaptured,
                                    WCCWinCustomColors, 
																		( FARPROC )WCCCustomColorsMouseEvent ) ) &&
        ( ! WCCControlMouseEvent ( hWnd, Message, wParam, lParam, 
																		( BOOL FAR* )&IsMouseCaptured,
                                    WCCWinCurrentColor, 
																		( FARPROC )WCCCurrentColorMouseEvent ) ) )
  {
    WCCGetControlRect ( hWnd, WCCWinLuminosity, &ControlRect );
    if ( ( LOWORD ( lParam ) > ( WORD )ControlRect.left ) &&
          ( ( WORD )ControlRect.top < HIWORD ( lParam ) ) && ( HIWORD ( lParam ) < ( WORD )ControlRect.bottom ) )
      WCCLuminosityMouseEvent ( hWnd, Message, wParam, lParam, &IsMouseCaptured, &ControlRect );
  }

  return ( IsMouseCaptured );

}

/*------------------------------------------------------------------------------
**
**  Choose color Dialog Proc
**
**----------------------------------------------------------------------------*/

TError WINAPI
WCFGetEditControlValueAndSetState
  ( HWND                      hWnd,
    WORD                      ControlID )

{

  WORD                        Value;
	DWORD												SelectedText;
  char                        ValueString [ 10 ];

  GetWindowText ( GetDlgItem ( hWnd, ControlID ), ValueString, sizeof ( ValueString ) );
  Value = fatoi ( ValueString );
	SelectedText = SendMessage ( GetDlgItem ( hWnd, ControlID ), EM_GETSEL,
																0, 0 );
	WCCShowColorCursor ( hWnd, FALSE, 0, 0, NULL );
	WCCShowLuminosityCursor ( hWnd, FALSE, 0, 0, NULL );

  switch ( ControlID )
  {
    case WCCEdtColorHue:
      WCCSetHue ( hWnd, Value > MAX_HUE ? MAX_HUE : Value, FALSE );
      break;

    case WCCEdtColorSaturation:
      WCCSetSaturation ( hWnd, Value > MAX_SATURATION ? MAX_SATURATION : Value, FALSE );
      break;

    case WCCEdtColorLuminosity:
      WCCSetLuminosity ( hWnd, Value > MAX_LUMINOSITY ? MAX_LUMINOSITY : Value, FALSE );
      break;

    case WCCEdtColorRed:
      WCCSetRed ( hWnd, Value > MAX_RGB ? MAX_RGB : Value, FALSE );
      break;

    case WCCEdtColorGreen:
      WCCSetGreen ( hWnd, Value > MAX_RGB ? MAX_RGB : Value, FALSE );
      break;

    case WCCEdtColorBlue:
      WCCSetBlue ( hWnd, Value > MAX_RGB ? MAX_RGB : Value, FALSE );
      break;

  }

	SendMessage ( GetDlgItem ( hWnd, ControlID ), EM_SETSEL, 0, SelectedText );
	WCCShowColorCursor ( hWnd, TRUE, 0, 0, NULL );
	WCCShowLuminosityCursor ( hWnd, TRUE, 0, 0, NULL );

  return ( 0 );

}
/*------------------------------------------------------------------------------
**
**  Initialize Dialog
**
**----------------------------------------------------------------------------*/

TError WINAPI
WCCInitDialog
  ( HWND                      hWnd )

{

  RECT                        WindowRect;
  COLORREF                    Color;

  TError                       ErrorCode  = 0;

  if ( ! ( WCCStruct.hColorCursor = LoadCursor ( GetInstance (), MAKEINTRESOURCE ( IDC_COLORCROSSHAIR ) ) ) )
    ErrorCode = CDERR_LOADRESFAILURE;
  else
  if ( ! ( WCCStruct.hIconColorCursor = LoadIcon ( GetInstance (), MAKEINTRESOURCE ( IDI_COLORCROSSHAIR ) ) ) )
    ErrorCode = CDERR_LOADRESFAILURE;
  else
  if ( ! ( WCCStruct.hLuminosityCursor = LoadCursor ( GetInstance (), MAKEINTRESOURCE ( IDC_LUMINOSITY ) ) ) )
    ErrorCode = CDERR_LOADRESFAILURE;
  else
  if ( ! ( WCCStruct.hIconLuminosityCursor = LoadIcon ( GetInstance (), MAKEINTRESOURCE ( IDI_LUMINOSITY ) ) ) )
    ErrorCode = CDERR_LOADRESFAILURE;
  else
  {
		ShowWindow ( GetDlgItem ( hWnd, WCCBtnHelp ), 
		             WCCStruct.lpChooseColor->Flags & CC_SHOWHELP ? 
		             SW_SHOW : SW_HIDE );
    GetWindowRect ( hWnd, &WindowRect );
    WCCStruct.FullWindowWidth = WIDTH ( &WindowRect );
		if ( WCCStruct.lpChooseColor->Flags & CC_FULLOPEN )
			WCCInitColor ( hWnd );
		else
      MoveWindow ( hWnd, WindowRect.left, WindowRect.top, WCCStruct.FullWindowWidth / 2, WindowRect.bottom - WindowRect.top, TRUE );
    if ( ( WCCStruct.lpChooseColor->Flags & CC_PREVENTFULLOPEN ) || ( WCCStruct.lpChooseColor->Flags & CC_FULLOPEN ) )
      EnableWindow ( GetDlgItem ( hWnd, WCCBtnDefineCustomColors ), FALSE );
    if ( WCCStruct.lpChooseColor->Flags & CC_RGBINIT )
    {
      WCCSetRed ( hWnd, GetRValue ( WCCStruct.lpChooseColor->rgbResult ), TRUE );
      WCCSetGreen ( hWnd, GetGValue ( WCCStruct.lpChooseColor->rgbResult ), TRUE );
      WCCSetBlue ( hWnd, GetBValue ( WCCStruct.lpChooseColor->rgbResult ), TRUE );
    }
    else
    {
      WCCSetHue ( hWnd, 0, TRUE );
      WCCSetLuminosity ( hWnd, 0, TRUE );
      WCCSetSaturation ( hWnd, 0, TRUE );
      Color = HLSToRGB ( 0, 0, 0 );
      WCCSetRed ( hWnd, GetRValue ( Color ), TRUE );
      WCCSetGreen ( hWnd, GetGValue ( Color ), TRUE );
      WCCSetBlue ( hWnd, GetBValue ( Color ), TRUE );
    }
  }

  return ( ErrorCode );

}
/*------------------------------------------------------------------------------
**
**  Paint Dialog
**
**----------------------------------------------------------------------------*/
TError WINAPI
WCCPaint
  ( HWND                      hWnd )

{

  PAINTSTRUCT                 PaintStruct;

  TError                       ErrorCode = 0;


	if ( ! WCCStruct.Hiding )
  	if ( BeginPaint ( hWnd, &PaintStruct ) )
  	{
	    ErrorCode = WCCPaintControl ( hWnd, WCCWinBasicColors, &PaintStruct,
 	                                 ( FARPROC )WCCPaintBasicColors );
 	   if ( ! ErrorCode )
 	     ErrorCode = WCCPaintControl ( hWnd, WCCWinCustomColors, &PaintStruct,
 	                                 ( FARPROC )WCCPaintCustomColors );
 	   if ( ! ErrorCode )
 	     ErrorCode = WCCPaintControl ( hWnd, WCCWinColors, &PaintStruct,
 	                                 ( FARPROC ) WCCPaintColors );
 	   if ( ! ErrorCode )
 	     ErrorCode = WCCPaintControl ( hWnd, WCCWinLuminosity, &PaintStruct,
 	                                 ( FARPROC ) WCCPaintLuminosity );
 	   if ( ! ErrorCode )
 	     ErrorCode = WCCPaintControl ( hWnd, WCCWinCurrentColor, &PaintStruct,
 	                                 ( FARPROC ) WCCPaintCurrentColor );
 	   WCCShowLuminosityCursor ( hWnd, TRUE, 0, 0, 0 );
 	   WCCShowColorCursor ( hWnd, TRUE, 0, 0, 0 );
 	   EndPaint ( hWnd, &PaintStruct );
 	 }

  return ( ErrorCode );

}
/*------------------------------------------------------------------------------
**
**  Choose color Dialog Proc
**
**----------------------------------------------------------------------------*/

TError WCCCALLBACK
WCCDialogProc
  ( HWND                      hWnd,
    UINT                      Message,
    WPARAM                    wParam,
    LPARAM                    lParam )

{

  static BOOL                 MouseIsCaptured = FALSE;
  RECT                        WindowRect;
  RECT                        Rect;
  HDC                         hDC;
  HBRUSH                      hBrush;

  TError                       ErrorCode = 0;


  if ( ( WCCStruct.lpChooseColor->Flags & CC_ENABLEHOOK ) && 
				( WCCStruct.lpChooseColor->lpfnHook ) )
	ErrorCode = ( *WCCStruct.lpChooseColor->lpfnHook )( hWnd, Message, wParam, lParam );

  if ( ( ErrorCode ) && ( Message != WM_INITDIALOG ) )
    return ( LOWORD ( ErrorCode ) );

  switch ( Message )
  {
    case WM_LBUTTONDOWN:
    case WM_LBUTTONUP:
    case WM_MOUSEMOVE:
    case WM_LBUTTONDBLCLK:
      MouseIsCaptured = WCCMouseEvent ( hWnd, Message, wParam, lParam, MouseIsCaptured );
      break;

    case WM_ERASEBKGND:
	if ( ( hDC = ( HDC ) wParam ) &&
             ( hBrush = ( HBRUSH ) SendMessage ( hWnd,
                          GET_WM_CTLCOLOR_MSG(CTLCOLOR_DLG),
                          GET_WM_CTLCOLOR_MPS(hDC, hWnd, CTLCOLOR_DLG) ) ) )
        {
          GetClientRect ( hWnd, &Rect );
          FillRect ( hDC, &Rect, hBrush );
        }
        break;

    case WM_PAINT:
      ErrorCode = WCCPaint ( hWnd );
      break;

    case WM_INITDIALOG:
      WCCInitDialog ( hWnd );
      break;

    case WM_COMMAND:
      switch ( GET_WM_COMMAND_ID(wParam, lParam) )
      {
        case WCCBtnAddCustomColor:
          WCCSetCustomColor ( hWnd );
          break;
        case WCCEdtColorHue:
        case WCCEdtColorSaturation:
        case WCCEdtColorLuminosity:
        case WCCEdtColorRed:
        case WCCEdtColorGreen:
        case WCCEdtColorBlue:
          WCFGetEditControlValueAndSetState ( hWnd, GET_WM_COMMAND_ID(wParam, lParam) );
          break;

        case WCCBtnDefineCustomColors:
		WCCInitColor ( hWnd );
	  GetWindowRect ( hWnd, &WindowRect );
	  MoveWindow ( hWnd, WindowRect.left, WindowRect.top, 
	               WCCStruct.FullWindowWidth, 
	               WindowRect.bottom - WindowRect.top, TRUE );
	  EnableWindow ( GetDlgItem ( hWnd, WCCBtnDefineCustomColors ), FALSE );
	  break; 

        case IDOK:
          WCCStruct.lpChooseColor->rgbResult = RGB ( WCCStruct.Red, WCCStruct.Green, WCCStruct.Blue );
        case IDCANCEL:
          EndDialog ( hWnd, GET_WM_COMMAND_ID(wParam, lParam) );
          break;
      }
  }

  return ( LOWORD ( ErrorCode ) );

}

/*------------------------------------------------------------------------------
**
**  General Initialization
**
**----------------------------------------------------------------------------*/
TError WINAPI
WCCInitialize ( void )

{

  TError                       ErrorCode  = 0;

  WCCStruct.BasicColor    = NUM_BASIC_COLORS;
  WCCStruct.CustomColor   = NUM_CUSTOM_COLORS;

  return ( ErrorCode );

}

/*------------------------------------------------------------------------------
**
**  General Uninitialization
**
**----------------------------------------------------------------------------*/
void WINAPI
WCCUninitialize ( void )

{

  if ( WCCStruct.hColorCursor )
    DestroyCursor ( WCCStruct.hColorCursor );

  if ( WCCStruct.hIconColorCursor )
    DestroyIcon ( WCCStruct.hIconColorCursor );

  if ( WCCStruct.hColorBitmap )
    DeleteObject ( WCCStruct.hColorBitmap );

}

/*------------------------------------------------------------------------------
**
** Replacement for windows common dialog to choose color
**
**----------------------------------------------------------------------------*/

BOOL WINAPI
ChooseColor
  ( CHOOSECOLOR FAR*          lpCC )

{

  HRSRC                       hResource    = ( HRSRC )NULL;
  HGLOBAL                     hResDialog   = ( HGLOBAL )NULL;
  LPVOID                      lpResource   = ( LPVOID )NULL;
  HGLOBAL                     hUseDialog   = ( HGLOBAL )NULL;
  HINSTANCE                   hUseInstance = ( HINSTANCE )NULL;
	CHOOSECOLOR									CurrentChooseColor;

  TError                       ReturnedCode = 0;
  TError                       ErrorCode    = 0;


  _fmemset ( &WCCStruct, 0, sizeof ( WCCStruct ) );
  WCCStruct.lpChooseColor = &CurrentChooseColor;
	WCCStruct.CustomColorIndex = NUM_CUSTOM_COLORS - 1;
	if ( lpCC )
		_fmemcpy ( WCCStruct.lpChooseColor, lpCC, sizeof ( CHOOSECOLOR ) );
  /*----- Check for parameters that make sense -----*/
  if ( ( ReturnedCode = WCCCheckParameters ( ) ) ||
  /*----- Initialize state -----*/
    ( ( ReturnedCode = WCCInitialize ( ) ) ) )
    ErrorCode = ReturnedCode;
  else
  {
    /*--- Get Dialog handle ---*/
    if ( WCCStruct.lpChooseColor->Flags & CC_ENABLETEMPLATEHANDLE )
      /*-- Dialog handle was passed by caller --*/
      hUseDialog = WCCStruct.lpChooseColor->hInstance;
    else
    { /*-- Get Dialog from resource --*/
      if ( WCCStruct.lpChooseColor->Flags & CC_ENABLETEMPLATE )
      { /*-- Dialog is in callers resource section
              NOTE that hUseInstance is set --*/
        if ( ! ( hResource = FindResource ( hUseInstance = WCCStruct.lpChooseColor->hInstance,
                                            WCCStruct.lpChooseColor->lpTemplateName,
                                            RT_DIALOG ) ) )
          ErrorCode = CDERR_FINDRESFAILURE;
      }
      else
      { /*-- Use default dialog in library resource section
              NOTE that hUseInstance is set --*/
        if ( ! ( hResource = FindResource ( hUseInstance = GetInstance (),
                                            "CHOOSECOLOR",
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
		switch ( ReturnedCode = DialogBoxIndirectParam ( GetInstance (), lpResource,
                                                        WCCStruct.lpChooseColor->hwndOwner,
                                                        ( DLGPROC )WCCDialogProc, 0L ) )
#else
		switch ( ReturnedCode = DialogBoxIndirectParam ( GetInstance (), hResDialog,
                                                        WCCStruct.lpChooseColor->hwndOwner,
                                                        ( DLGPROC )WCCDialogProc, 0L ) )
#endif
      {
        default:
          break;

        case IDOK:
          ReturnedCode = 0;
					_fmemcpy ( lpCC, WCCStruct.lpChooseColor, sizeof ( CHOOSECOLOR ) );
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

  WCCUninitialize ();

  /*----- Set Error conditions -----*/
  if ( ReturnedCode )
    ErrorCode = ReturnedCode;


  /*----- if and error occured return FALSE otherwise return TRUE -----*/
  return ( ErrorCode ? FALSE : TRUE );

}
