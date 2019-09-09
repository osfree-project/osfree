/*  System.h	1.4
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
 
#ifndef __W_SYSTEM_H__
#define __W_SYSTEM_H__


#define WCharSet_PathSeparators             "/\\"
#define WCharSet_ExtentionSeparators        "."
#define WCharSet_CurrentDirectory           "."
#define WCharSet_DriveSeparator             ":"
#define WCharSet_WildCards                  "*?"
#ifdef WINOS
#define WChar_PathSeparator                 '\\'
#define WChar_SecondaryPathSeparator        '/'
#else
#define WChar_PathSeparator                 '/'
#define WChar_SecondaryPathSeparator        '\\'
#endif


#endif /*--- __W_SYSTEM_H__ ---*/

