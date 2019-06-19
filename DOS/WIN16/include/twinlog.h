/*  twinlog.h	1.1  This is for more fine control of logging messages.
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
 

#ifndef Twinlog__h
#define Twinlog__h

/*
*  These are intented mostly for source ports. For example,
*  if you've narrowed down a Windows application bug to a
*  few contiguous API calls, then simply bracket your calls as follows:
*
*	TwinLogOn();
*	MoveTo(...);
*	TextOut(...);
*	TwinLogReset();
*
*  The above translates as follows
*
*	force all debug and trace messages to be enabled
*	find out all library messages about MoveTo() and TextOut()
*	restore to initial startup debug and trace levels
*
*  To make use of these, you will have to compile with
*  the DEBUG libraries. Note that these will work in conjunction with
*  the environment variable WIN_OPENLOG, particularly the file name.
*  If WIN_OPENLOG is not set, then standard output is assumed.
*/

extern unsigned long TwinLogOn(void);	/* force ON all library messages */
extern unsigned long TwinLogOff(void);	/* force OFF all library messages */
extern unsigned long TwinLogReset(void); /* go back to initial logging state */
extern unsigned long TwinLogGet(void);	/* get current logging mask */
extern unsigned long TwinLogSet(unsigned long); /* set current logging mask */


#endif /* Twinlog__h */
