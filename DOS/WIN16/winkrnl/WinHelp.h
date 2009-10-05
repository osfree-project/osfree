/*******************************************************************************
	@(#)WinHelp.h	1.6
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

*******************************************************************************/
 

#ifndef WINHELP_H
#define WINHELP_H

/*************************************
*
*  Shared Defines
*
**************************************/

/* 
** Structure for decoding the LPARAM
** information of a WM_WINHELP
** message.
*/
typedef struct tagHELPPARAMS 
{
  short int     Size;
  short int     Message;
  unsigned long Context;
  long          Unknown;
  short int     PathOffset;
  short int     ExtraDataOffset;
  char          Path[1];
}
HELPPARAMS;
typedef HELPPARAMS * FPHELPPARAMS;


/*
**
** Message atoms strings.
**
*/
/* String used when registering the WM_WINHELP message. */
#define MSWIN_HELP_MSG_STRING  "WM_WINHELP"


/* 
** Name of the message redirector window class. WinHelp() API 
** function calls TWINVIEW.EXE with the -X parameter.
*/
#define MS_WINHELP_CLASS   "MS_WINHELP"


/*************************************
*
*  Shared Functions
*
**************************************/

BOOL WinHelp( HWND hWnd, LPCSTR lpszHelpFile, UINT fuCommand, DWORD dwData );

#endif
