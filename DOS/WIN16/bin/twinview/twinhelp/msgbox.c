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

#include "msgbox.h"


/***********************************
**
**  Private Defines
**
***********************************/

#define MAX_CAPTION  100
#define MAX_MESSAGE  200

/***********************************
**
**  Private Function Prototypes
**
***********************************/


/********************************** Public Functions ****************************/

int __far cdecl MsgBox 
( 
  HANDLE hInstance, 
  HWND hWnd, 
  WORD wCaptionID, 
  WORD wMessageID, 
  WORD wType, 
  ...
) 
{
   char szCaption[ MAX_CAPTION ];                /* Caption from the resource file. */
   char szMsgUnfilled[ MAX_MESSAGE ];            /* Message from the resource file. */
   char szMsgFilled[ MAX_MESSAGE ];              /* Filled in message string. */
   void FAR * VarArgList;                        /* Pointe to message string's data. */
   

   /* Get the data for the message string's variables. */
   VarArgList = (WORD FAR *) &wType + 1;   
   
   /* Load the caption string. */
   LoadString( hInstance, wCaptionID, szCaption, sizeof( szCaption ) - 1 );
   
   /* Load the message string. */
   LoadString( hInstance, wMessageID, szMsgUnfilled, sizeof( szMsgUnfilled ) - 1 );
   
   /* Fill the message string's variables with the data passed in. */
   wvsprintf( szMsgFilled, szMsgUnfilled, VarArgList );
   
   /* Show the message. */
   return( MessageBox( hWnd, szMsgFilled, szCaption, wType ) );
}


