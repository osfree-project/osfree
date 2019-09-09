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
**                               .INI FILE INFO ROUTINES
**
********************************************************************************************/

/***********************************
**
**  System Includes
**
***********************************/
#include <stdlib.h>
#include <string.h>


/***********************************
**
**  App. Includes
**
***********************************/
#include "inifile.h"
#include "twinhelp.h"


/***********************************
**
**  Private Defines
**
***********************************/

/* .INI file defines. */
#define INI_FILENAME              "TWINHELP.INI"

#define INI_GENERAL_SECTION       "General"
#define INI_WNDPOS_ENTRY          "LastPos"
#define INI_WNDPOS_TOKEN          ","
#define MAX_WNDPOS_STRING         40

#define INI_HISTORY_SECTION       "History"
#define INI_MAXHISTORY_ENTRY      "MaxHistory"
#define DEFAULT_MAXHISTORY        30

#define INI_BACK_SECTION          "Back"
#define INI_MAXBACK_ENTRY         "MaxBack"
#define DEFAULT_MAXBACK           30

#define INI_COLORS_SECTION        "Colors"

#define INI_HOTSPOTCOLOR_ENTRY    "HotSpot"
#define DEFAULT_HOTSPOTCOLOR      32768       /* COLORREF value for green. 0,128,0 */

#define INI_NOSCROLLBG_ENTRY      "NoScrollBG"
#define INI_SCROLLBG_ENTRY        "ScrollBG"
#define DEFAULT_BKCOLOR           0xFFFFFF    /* COLORREF value for green. 255,255,255 */

#define COLOR_ENTRY_BUFF_SIZE     20
#define COLOR_SEP                 ","



#define INI_ENTRY_NOT_FOUND       "ENTRY NOT FOUND"



/***********************************
**
**  Private functions
**
***********************************/

static void __far __pascal GetINIPath( char __far * szINIPath );
static BOOL __far __pascal ParseColorEntry( char __far * szEntry, COLORREF __far * ColorRef );


/***********************************************************************************************
*
*               Routines to read the entries in the [General] section.
*
************************************************************************************************/

/****************************************************
**
**  Read last position and size of main help window.
**
****************************************************/
BOOL __far __pascal ReadLastMainWndPos( WINDOWPLACEMENT __far * WndPlacePtr )
{ 
  BOOL bMaxmized;
  
  char __far * TokenPtr; 
  char szString[ MAX_WNDPOS_STRING ];
  char szINIFilePath[ _MAX_PATH ];

  short int nBytesRead;

  
  /* Get path to the INI file. */
  GetINIPath( szINIFilePath );
  
  /* Read entry "LastPos" */
  nBytesRead = GetPrivateProfileString( INI_GENERAL_SECTION, INI_WNDPOS_ENTRY, 
                           INI_ENTRY_NOT_FOUND, szString, MAX_WNDPOS_STRING, 
                           szINIFilePath );            
                           
  /* No bytes read or entry not there. */
  if( nBytesRead == 0 || _fstrcmp( szString, INI_ENTRY_NOT_FOUND ) == 0 )
  {
    /* Set to default. */
    WndPlacePtr->showCmd = SW_SHOWNORMAL;
    WndPlacePtr->rcNormalPosition.left   = CW_USEDEFAULT;
    WndPlacePtr->rcNormalPosition.top    = CW_USEDEFAULT;
    WndPlacePtr->rcNormalPosition.right  = CW_USEDEFAULT;
    WndPlacePtr->rcNormalPosition.bottom = CW_USEDEFAULT;
  
    /* Setting not found. */
    return( FALSE );
  }
                                                                
  /* 
  ** Parse entry. 
  */

  /* Get maximize state. */
  TokenPtr = _fstrtok( szString, INI_WNDPOS_TOKEN );
  if( TokenPtr != NULL )
  { 
    bMaxmized = (BOOL) atoi( TokenPtr ); 
  }
  
  /* If maximized. */
  if( bMaxmized )
  {
    WndPlacePtr->showCmd = SW_SHOWMAXIMIZED;
    WndPlacePtr->rcNormalPosition.left   = CW_USEDEFAULT;
    WndPlacePtr->rcNormalPosition.top    = CW_USEDEFAULT;
    WndPlacePtr->rcNormalPosition.right  = CW_USEDEFAULT;
    WndPlacePtr->rcNormalPosition.bottom = CW_USEDEFAULT;

    /* Setting found. */
    return( TRUE );
  }

  /* Not maximized. */
  WndPlacePtr->showCmd = SW_SHOWNORMAL;
    
  /* Get x position. */
  TokenPtr = _fstrtok( NULL, INI_WNDPOS_TOKEN );
  if( TokenPtr != NULL )
  { 
    WndPlacePtr->rcNormalPosition.left = atoi( TokenPtr ); 
  }
  
  /* Get y position. */
  TokenPtr = _fstrtok( NULL, INI_WNDPOS_TOKEN );
  if( TokenPtr != NULL )
  { 
    WndPlacePtr->rcNormalPosition.top = atoi( TokenPtr ); 
  }
  
  /* Get width. */
  TokenPtr = _fstrtok( NULL, INI_WNDPOS_TOKEN );
  if( TokenPtr != NULL )
  { 
    WndPlacePtr->rcNormalPosition.right = atoi( TokenPtr ); 
  }

  /* Get height. */
  TokenPtr = _fstrtok( NULL, INI_WNDPOS_TOKEN );
  if( TokenPtr != NULL )
  { 
    WndPlacePtr->rcNormalPosition.bottom = atoi( TokenPtr ); 
  }

  /* Setting found. */
  return( TRUE );
}
  

/****************************************************
**
**  Save last position and size of main help window.
**
****************************************************/
void __far __pascal WriteLastMainWndPos( WINDOWPLACEMENT __far * WndPlacePtr )
{ 
  char szNumber[ 8 ];
  char szString[ MAX_WNDPOS_STRING ];
  char szINIFilePath[ _MAX_PATH ];
  
  /* Get path to the INI file. */
  GetINIPath( szINIFilePath );
  

  /* If maximized. */
  if( WndPlacePtr->showCmd == SW_SHOWMAXIMIZED )
  {
    /* Save window state. */
    itoa( TRUE, szNumber, 10 );
    _fstrcpy( szString, szNumber );
  }
  
  /* If not maximized. */
  else
  {
    /* Save window state. */
    itoa( FALSE, szNumber, 10 );
    _fstrcpy( szString, szNumber );
    _fstrcat( szString, INI_WNDPOS_TOKEN );

    /* Process x position. */
    itoa( WndPlacePtr->rcNormalPosition.left, szNumber, 10 );
    _fstrcat( szString, szNumber );
    _fstrcat( szString, INI_WNDPOS_TOKEN );
    
    /* Process y position. */
    itoa( WndPlacePtr->rcNormalPosition.top, szNumber, 10 );
    _fstrcat( szString, szNumber );
    _fstrcat( szString, INI_WNDPOS_TOKEN );
    
    /* Process width position. */
    itoa( WndPlacePtr->rcNormalPosition.right, szNumber, 10 );
    _fstrcat( szString, szNumber );
    _fstrcat( szString, INI_WNDPOS_TOKEN );
    
    /* Process height position. */
    itoa( WndPlacePtr->rcNormalPosition.bottom, szNumber, 10 );
    _fstrcat( szString, szNumber );
  }

  /* Write entry "LastPos" */
  WritePrivateProfileString( INI_GENERAL_SECTION, INI_WNDPOS_ENTRY, 
                             szString, szINIFilePath );
}
 


/***********************************************************************************************
*
*                 Routines to read the entries in the [Back] section.
*
************************************************************************************************/

/****************************************************
**
**  Read maximum number of BACK entries from INI file.
**
****************************************************/
void __far __pascal ReadINIMaxBack( WORD __far * wMaxBack )
{
  char szINIFilePath[ _MAX_PATH ];

  /* Get path to the INI file. */
  GetINIPath( szINIFilePath );
  
  /* No history. */
  *wMaxBack = 0;
  
  /* Entry "MaxHistory" */
  *wMaxBack = GetPrivateProfileInt( INI_BACK_SECTION, INI_MAXBACK_ENTRY, 
                                    DEFAULT_MAXBACK, szINIFilePath );
   
  /* Entry was there but it wasn't an integer. */
  if( *wMaxBack == 0 ) *wMaxBack = DEFAULT_MAXBACK;
} 


/***********************************************************************************************
*
*                 Routines to read the entries in the [History] section.
*
************************************************************************************************/

/****************************************************
**
**  Read maximum number of history entries from INI file.
**
****************************************************/
void __far __pascal ReadINIMaxHistory( WORD __far * wMaxHistory )
{
  char szINIFilePath[ _MAX_PATH ];

  /* Get path to the INI file. */
  GetINIPath( szINIFilePath );
  
  /* No history. */
  *wMaxHistory = 0;
  
  /* Entry "MaxHistory" */
  *wMaxHistory = GetPrivateProfileInt( INI_HISTORY_SECTION, INI_MAXHISTORY_ENTRY, 
                                       DEFAULT_MAXHISTORY, szINIFilePath );
   
  /* Entry was there but it wasn't an integer. */
  if( *wMaxHistory == 0 ) *wMaxHistory = DEFAULT_MAXHISTORY;
} 



/***********************************************************************************************
*
*                 Routines to read the entries in the [Colors] section.
*
************************************************************************************************/

/*********************************************
**
**  Read hotspot underlining color from INI file.
**
*********************************************/
void __far __pascal ReadINIHotSpotColor( COLORREF __far * HotSpotColor )
{
  short int nBytesRead;
  char szEntry[ COLOR_ENTRY_BUFF_SIZE ];
  char szINIFilePath[ _MAX_PATH ];
  

  /* Get path to the INI file. */
  GetINIPath( szINIFilePath );
  
  /* Entry "Hotspot" */
  nBytesRead = GetPrivateProfileString( INI_COLORS_SECTION, 
                                        INI_HOTSPOTCOLOR_ENTRY, 
                                        INI_ENTRY_NOT_FOUND, 
                                        szEntry, 
                                        COLOR_ENTRY_BUFF_SIZE, 
                                        szINIFilePath );
   
  /* No bytes read. */
  if( nBytesRead == 0 || _fstrcmp( szEntry, INI_ENTRY_NOT_FOUND ) == 0 )
  {
    /* Set to default. */
    *HotSpotColor = DEFAULT_HOTSPOTCOLOR;
    return;
  }
  
  /* Parse and make COLORREF value. */
  if( ! ParseColorEntry( szEntry, HotSpotColor ) )
  {
    /* Set to default. */
    *HotSpotColor = DEFAULT_HOTSPOTCOLOR;
  }
} 

          
/*********************************************
**
**  Read window erasing color for scrolling
**  window. 
**
*********************************************/
void __far __pascal ReadScrollBGColor( COLORREF __far * ColorRefPtr )
{
  short int nBytesRead;
  char szEntry[ COLOR_ENTRY_BUFF_SIZE ];
  char szINIFilePath[ _MAX_PATH ];
  

  /* Get path to the INI file. */
  GetINIPath( szINIFilePath );
  
  /* Entry "ScrollBG" */
  nBytesRead = GetPrivateProfileString( INI_COLORS_SECTION, 
                                        INI_SCROLLBG_ENTRY, 
                                        INI_ENTRY_NOT_FOUND, 
                                        szEntry, 
                                        COLOR_ENTRY_BUFF_SIZE, 
                                        szINIFilePath );
   
  /* No bytes read or no entry. */
  if( nBytesRead == 0 || _fstrcmp( szEntry, INI_ENTRY_NOT_FOUND ) == 0 )
  {
    /* Set to default. */
    *ColorRefPtr = DEFAULT_BKCOLOR;
  }
  
  /* Parse and make COLORREF value. */
  else
  {
    if( ! ParseColorEntry( szEntry, ColorRefPtr ) )
    {
      /* Set to default. */
      *ColorRefPtr = DEFAULT_BKCOLOR;
    }
  }
} 


/*********************************************
**
**  Read window erasing color for non-scrolling
**  window. 
**
*********************************************/
void __far __pascal ReadNonScrollBGColor( COLORREF __far * ColorRefPtr )
{
  short int nBytesRead;
  char szEntry[ COLOR_ENTRY_BUFF_SIZE ];
  char szINIFilePath[ _MAX_PATH ];
  

  /* Get path to the INI file. */
  GetINIPath( szINIFilePath );
  
  /* Entry "NoScrollBG" */
  nBytesRead = GetPrivateProfileString( INI_COLORS_SECTION, 
                                        INI_NOSCROLLBG_ENTRY, 
                                        INI_ENTRY_NOT_FOUND, 
                                        szEntry, 
                                        COLOR_ENTRY_BUFF_SIZE, 
                                        szINIFilePath );
   
  /* No bytes read or no entry. */
  if( nBytesRead == 0 || _fstrcmp( szEntry, INI_ENTRY_NOT_FOUND ) == 0 )
  {
    /* Set to default. */
    *ColorRefPtr = DEFAULT_BKCOLOR;
  }
  
  /* Parse and make COLORREF value. */
  else
  {
    if( ! ParseColorEntry( szEntry, ColorRefPtr ) )
    {
      /* Set to default. */
      *ColorRefPtr = DEFAULT_BKCOLOR;
    }
  }
} 


/*********************************************
**
**  Makes a COLORREF value from a color entry. 
**
*********************************************/
static BOOL __far __pascal ParseColorEntry( char __far * szEntry, COLORREF __far * ColorRef )
{
  BYTE Red, Green, Blue;
  char __far * szValue;
  WORD wCounter;


  /* Read RED color value. */
  szValue = _fstrtok( szEntry, COLOR_SEP );
        
  /* Error? */
  if( szValue == NULL ) 
  {
    /* Error. */
    return( FALSE );
    
  }
  else
  {
    Red = (BYTE) atoi( szValue );
  }
  
  /* Parse the GREEN and BLUE color values from the line. */
  for( wCounter = 0; wCounter < 2; wCounter++ )
  {
    /* Read next color value. */
    szValue = _fstrtok( NULL, COLOR_SEP );
      
    /* Error? */
    if( szValue == NULL )
    {
      /* Error. */
      return( FALSE );
    }
  
    switch( wCounter )
    {
      case 0:
      {
        Green = (BYTE) atoi( szValue );
        break;
      }
  
      case 1:
      {
        Blue = (BYTE) atoi( szValue );
        break;
      }
    }
  }
  
  /* Have color values, create COLORREF. */
  *ColorRef = RGB( Red, Green, Blue );

  /* Success. */
  return( TRUE );
}



/***********************************************************************************************
*
*                                Private Rountines
*
************************************************************************************************/
static void __far __pascal GetINIPath( char __far * szINIPath )
{
  /* Get the windows directory. */
  GetWindowsDirectory( szINIPath, _MAX_PATH );

  /* Append the INI file path. */
  _fstrcat( szINIPath, DIR_SEP_STR );
  _fstrcat( szINIPath, INI_FILENAME );
}

