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

/***********************************
**
**  System Includes
**
***********************************/


/***********************************
**
**  App. Includes
**
***********************************/

#include "fontbuff.h"
#include "winmem.h"
#include "wnddata.h"


/***********************************
**
**  Private File Defines
**
***********************************/

typedef struct tagFONTBUFFER
{
  WORD    wNumFonts;  /* Number of fonts in the buffer. */
  HGLOBAL hFontInfo;  /* Handle to the first FONTINFO record in the buffer. */
} 
FONTBUFFER;
typedef FONTBUFFER __far * FPFONTBUFFER;


/* Maximum number of fonts held in the buffer. */
#define MAX_FONTS  30

typedef struct tagFONTINFO
{
  WORD  wFontNumber;    /* Font index number in FontInfo list. */
  HFONT hFont;          /* Handle to the font. */
  DWORD dwTickCount;    /* Tickcount when the font was created or last used. */
} 
FONTINFO;
typedef FONTINFO __far * FPFONTINFO;



/***********************************************************************************
**
**                              Shared Functions
**
************************************************************************************/


/*********************************************************
**
**  Allocates the buffer's structure.
**
**  Initially, only the first slot is allocated.
**  It will be used to hold a count of the number of fonts
**  in the buffer in the dwTickCount variable. 
**
**********************************************************/
HFONTBUFFER __far __pascal AllocFontBuffer( HWND hWnd )
{
  HFONTBUFFER hFontBuffer;
  FPFONTBUFFER fpFontBuffer;

  /* 
  ** Allocate the font buffer. 
  */
  if( ! GlobalAllocMem( hWnd, &hFontBuffer, sizeof(FONTBUFFER) ) )
  {
    /* Failure. */
    return( NULL );
  }
  
  /* Get pointer to the font buffer. */
  fpFontBuffer = ( FPFONTBUFFER ) GlobalLock( hFontBuffer );
  
  /* Allocate the FONTINFO nodes. */
  if( ! GlobalAllocMem( hWnd, &(fpFontBuffer)->hFontInfo, sizeof(FONTINFO) * MAX_FONTS ) )
  {
    /* Unlock the font buffer. */
    GlobalUnlock( hFontBuffer );
    
    /* Free the font buffer. */
    GlobalFree( hFontBuffer );
    
    /* Failure. */
    return( NULL );
  }
  
  /* No fonts in the buffer. */
  fpFontBuffer->wNumFonts = 0;
  
  /* Unlock the font buffer. */
  GlobalUnlock( hFontBuffer );

  /* Success. */
  return( hFontBuffer );
}



/*********************************************************
**
**  Destroys the fonts in the buffer and frees the
**  memory associated with the buffer's structure.
**
**  NOTE: The function selects the SYSTEM font into the
**        given HDC so that it can be sure that the fonts 
**        being deleted are not currently selected.
**
**********************************************************/
void  __far __pascal FreeFontBuffer( HWND hWnd, HFONTBUFFER hFontBuffer )
{
  FPFONTBUFFER fpFontBuffer;

  /* No font buffer.  */
  if( hFontBuffer == NULL ) return;
  
  /* Destroy the fonts in the buffer. */
  ClearFontBuffer( hWnd, hFontBuffer );

  /* Get pointer to the font buffer. */
  fpFontBuffer = ( FPFONTBUFFER ) GlobalLock( hFontBuffer );
  
  /* Free the font info area. */
  GlobalFree( fpFontBuffer->hFontInfo );

  /* Unlock the font buffer. */
  GlobalUnlock( hFontBuffer );
    
  /* Free the buffer. */
  GlobalFree( hFontBuffer );
}



/*********************************************************
**
**  Destroys the fonts in the buffer.
**
**  NOTE: The function selects the SYSTEM font into the
**        given HDC so that it can be sure that the fonts 
**        being deleted are not currently selected.
**
**********************************************************/
void __far __pascal ClearFontBuffer( HWND hWnd, HFONTBUFFER hFontBuffer )
{
  FPFONTBUFFER fpFontBuffer;
  FPFONTINFO   fpFontInfo;
  WORD         wIndex;
  HDC          hDC;


  /* No font buffer.  */
  if( hFontBuffer == NULL ) return;
  
  /* Get pointer to the font buffer. */
  fpFontBuffer = ( FPFONTBUFFER ) GlobalLock( hFontBuffer );
  
  /* If there are fonts in the buffer. */
  if( fpFontBuffer->wNumFonts != 0 )
  {
    /* Get pointer to the first font. */
    fpFontInfo = ( FPFONTINFO ) GlobalLock( fpFontBuffer->hFontInfo );
    
    /* Make sure none of the fonts in the font buffer are selected. */
    hDC = GetDC( hWnd );
    SelectObject( hDC, GetStockObject( SYSTEM_FONT ) ); 
    ReleaseDC( hWnd, hDC );

    /* Destroy all fonts in the buffer. */
    for( wIndex = 0; wIndex < fpFontBuffer->wNumFonts; wIndex++ )
    {
      /* Free the font. */
      DeleteObject( fpFontInfo->hFont );
            
      /* Goto next node. */
      fpFontInfo++;
    }

    /* Unlock the font info. */
    GlobalUnlock( fpFontBuffer->hFontInfo );

    /* No fonts in the buffer. */
    fpFontBuffer->wNumFonts = 0;
  
  }
  
  /* Unlock the font buffer. */
  GlobalUnlock( hFontBuffer );
}


/*********************************************************
**
**  Adds a new font to the buffer.
**
**  If no space is available, the least used font is 
**  replaced.
**
**  NOTE: If an existing font is being replaced, 
**        the function selects the SYSTEM font into the
**        given HDC so that it can be sure that the font 
**        being deleted is not currently selected.
**
**********************************************************/
BOOL __far __pascal AddFont( HWND hWnd, HFONTBUFFER hFontBuffer, HFONT hFont, WORD wFontNumber )
{
  FPFONTBUFFER fpFontBuffer;
  FPFONTINFO   fpFontInfo, 
               fpFontLeastUsed;
  WORD         wIndex;
  HDC          hDC;
   

  /* No font buffer.  */
  if( hFontBuffer == NULL ) return FALSE;
  
  /* Get pointer to the font buffer. */
  fpFontBuffer = ( FPFONTBUFFER ) GlobalLock( hFontBuffer );
  
  /* Get pointer to the first font info node. */
  fpFontInfo = GlobalLock( fpFontBuffer->hFontInfo );

  /* If we haven't reached - add space for the new font to the buffer. */
  if( fpFontBuffer->wNumFonts < MAX_FONTS )
  {
    /* Skip to the next unused slot.*/
    fpFontInfo = fpFontInfo + fpFontBuffer->wNumFonts;

    /* 
    ** Increment the count of how many fonts are
    ** in the font buffer.
    */
    fpFontBuffer->wNumFonts = fpFontBuffer->wNumFonts + 1;
  }


  /* We've reached maximum buffer space. */
  else
  {
    /* 1st font will be replaced. */
    fpFontLeastUsed = fpFontInfo;
    
    /* Find the font that hasn't been used for the longest time. */
    for( wIndex = 0; wIndex < fpFontBuffer->wNumFonts; wIndex++ )
    {
      /* Last time font used. */
      if( fpFontInfo->dwTickCount < fpFontLeastUsed->dwTickCount ) 
      {
        /* New replacement font. */
        fpFontLeastUsed = fpFontInfo;
      }
          
      /* Goto next node. */
      fpFontInfo++;
    }
  
    /* Make sure the font is not selected. */
    hDC = GetDC( hWnd );
    SelectObject( hDC, GetStockObject( SYSTEM_FONT ) ); 
    ReleaseDC( hWnd, hDC );

    /* Free the least used font. */
    DeleteObject( fpFontInfo->hFont );
    
    /* Point to the least used slot. */
    fpFontInfo = fpFontLeastUsed;
  }

  /* Modify the data. */
  fpFontInfo->wFontNumber  = wFontNumber; 
  fpFontInfo->hFont        = hFont;
  fpFontInfo->dwTickCount  = GetTickCount(); 

  /* Unlock the font info. */
  GlobalUnlock( fpFontBuffer->hFontInfo );
  
  /* Unlock the font buffer. */
  GlobalUnlock( hFontBuffer );

  /* Success. */
  return( TRUE );
}



/*********************************************************
**
**  Finds a font in the buffer that has the given 
**  wFontNumber.
**
**********************************************************/
HFONT  __far __pascal FindFont( HFONTBUFFER hFontBuffer, WORD wFontNumber )
{
  FPFONTBUFFER fpFontBuffer;
  FPFONTINFO   fpFontInfo;
  WORD         wIndex;
  HFONT        hFont = NULL;
  

  /* No font buffer.  */
  if( hFontBuffer == NULL ) return NULL;
  
  /* Get pointer to the font buffer. */
  fpFontBuffer = ( FPFONTBUFFER ) GlobalLock( hFontBuffer );
  
  /* If there are fonts in the buffer. */
  if( fpFontBuffer->wNumFonts != 0 )
  {
    /* Get pointer to the first font info node. */
    fpFontInfo = ( FPFONTINFO ) GlobalLock( fpFontBuffer->hFontInfo );
    
    /* Find a font in the buffer with the attributres. */
    for( wIndex = 0; wIndex < fpFontBuffer->wNumFonts; wIndex++ )
    {
      /* Found font? */
      if( fpFontInfo->wFontNumber == wFontNumber )
      {
        /* Save font's handle. */
        hFont = fpFontInfo->hFont;

        /* Update the usage info. */
        fpFontInfo->dwTickCount = GetTickCount();
        
        break;
      }
            
      /* Goto next node. */
      fpFontInfo++;
    }

    /* Unlock the font info. */
    GlobalUnlock( fpFontBuffer->hFontInfo );
  }
  
  /* Unlock the font buffer. */
  GlobalUnlock( hFontBuffer );

  /* Failure. */
  return( hFont );
}





