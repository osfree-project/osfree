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
/***************************************************************************************
**
**
**                              Dialog Box Related Functions
**
**
***************************************************************************************/

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
#include "globals.h"
#include "dialogs.h"
#include "twhlprc.h"


/***********************************
**
**  Private File Defines
**
***********************************/


/***********************************
**
**  Private Function Prototypes
**
***********************************/



/***********************************************************************************
**
**                              Shared Functions
**
************************************************************************************/

/*********************************
**
**  "DLG_ABOUT" dialog box procedure.
**
**********************************/
BOOL CALLBACK __export AboutDlgProc( HWND hWnd, UINT wMsg, WPARAM wParam, LPARAM lParam )
{
  switch( wMsg ) 
  {
    case WM_COMMAND:
    {
      switch (wParam) 
      {
        case IDOK: 
        case IDCANCEL:
        {
          if( HIWORD(lParam) == BN_CLICKED )
          {
            EndDialog( hWnd, wParam );
          }
          
          return( TRUE );
        }
      }
      
      return( TRUE );
    }
  }

  /* Return message processed result. */
  return( FALSE );
}








