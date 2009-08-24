/*  cderr.h 	- error codes definitions for common dialogs
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

The maintainer of the Willows TWIN Libraries may be reached (Email) 
at the address twin@willows.com	

*/

#ifndef cderr_h
#define cderr_h

/********************************/
/* Common Dialog Routine Failed */
/********************************/

#define CDERR_DIALOGFAILURE   0xFFFF

/* Initializing error in structure */
#define CDERR_STRUCTSIZE       0x0001
#define CDERR_INITIALIZATION   0x0002
#define CDERR_NOTEMPLATE       0x0003
#define CDERR_NOHINSTANCE      0x0004

/* Run-Time Error loading resources */
#define CDERR_LOADSTRFAILURE   0x0005
#define CDERR_FINDRESFAILURE   0x0006
#define CDERR_LOADRESFAILURE   0x0007
#define CDERR_LOCKRESFAILURE   0x0008

/* Run-Time Error Memory Manager    */
#define CDERR_MEMALLOCFAILURE  0x0009
#define CDERR_MEMLOCKFAILURE   0x000A


/* Run-Time Error No Hook Routine   */
#define CDERR_NOHOOK           0x000B

/****************************/
/* Print Dialog Error Codes */
/****************************/

#define PDERR_PARSEFAILURE     0x1002
#define PDERR_RETDEFFAILURE    0x1003

#define PDERR_LOADDRVFAILURE   0x1004

#define PDERR_GETDEVMODEFAIL   0x1005
#define PDERR_NODEVICES        0x1007
#define PDERR_NODEFAULTPRN     0x1008

#define PDERR_PRINTERNOTFOUND  0x100B

#define PDERR_CREATEICFAILURE  0x100A

/****************************/
/* Choose Font Error Codes  */
/****************************/

#define CFERR_MAXLESSTHANMIN   0x2002

#endif /* cderr__h */




