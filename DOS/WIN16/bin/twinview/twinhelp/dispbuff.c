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


/***********************************
**
**  Private Defines
**
***********************************/

/* |SYSTEM record call-back function. */
typedef BOOL ( CALLBACK * SYSTEMRECPROC )( FPSYSTEMREC fpSysRec, LPARAM lParam );


/***********************************
**
**  Private functions
**
***********************************/


GetDisplayBuffer( void   )
{
  long            FileStart;          /* Beginning position of a WHIFS file. */
  BOOL            bCompressed;        /* Are we using compression? */

  /* Get starting position of |TOPIC file. */
  FileStart = GetWHIFSFileStart( hErrorWnd, (FPWHIFSINFO) &(fpHelpFileInfo)->WHIFSInfo, TOPIC_FILE );
  
  /* File not found. */
  if( FileStart == 0 )
  {
    /* Error message. */
    MsgBox( GetLibInst(), hErrorWnd, IDS_ERRORTITLE, IDS_FINDINTERNALFILE, MB_ICONHAND | MB_OK );
    return FALSE;
  }
  
  bCompressed = GetCompressionStatus( FPSYSTEMINFO fpSystemInfo )

}


