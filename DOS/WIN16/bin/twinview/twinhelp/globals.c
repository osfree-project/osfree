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
**                Functions for Accessing Projects Globals Variables
**
**
***************************************************************************************/

#ifndef GLOBALS_H
#define GLOBALS_H

/*************************************
*
*  Custom Includes
*
**************************************/
#include "globals.h"
#include "windows.h"


/*************************************
*
*  Global variables used in this file.
*
**************************************/

static HINSTANCE hLibInst = 0;         /* Instance of the DLL. */
static UINT      WM_WINHELP = 0;       /* Registered window message. */
static UINT      WM_WINDOC = 0;        /* Registered window message. */
static HCURSOR   hHandCursor = NULL;   /* Shared icon resource. */



/******************************  Shared Functions  *************************************/

HCURSOR GetHandCursor( void )
{
  return hHandCursor;
}


void SetHandCursor( HCURSOR hCursor )
{
  hHandCursor = hCursor;
}


HINSTANCE GetLibInst( void )
{
  return hLibInst;
}


void SetLibInst( HINSTANCE hNewLibInst )
{
  hLibInst = hNewLibInst;
}


UINT GetWINDOCMsg( void )
{
  return WM_WINDOC;
}

void SetWINDOCMsg( UINT MsgValue )
{
  WM_WINDOC = MsgValue;
}


UINT GetWINHELPMsg( void )
{
  return WM_WINHELP;
}

void SetWINHELPMsg( UINT MsgValue )
{
  WM_WINHELP = MsgValue;
}


#endif
