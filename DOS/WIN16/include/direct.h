/*  direct.h	1.2
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
 

#ifndef DIRECT_H
#define	DIRECT_H

#include <unistd.h>

/*
 *  Various MSC definitions expected in this file.
 */
int _chdrive(int drive);
char *_getdcwd(int drive, char *buff, int buflen);
int _getdrive(void);

/*
 *  Map various MSC-flavor ANSI names onto standard names.
 */
#define _chdir chdir
#define _getcwd getcwd
#define _mkdir mkdir
#define _rmdir rmdir

#endif /* DIRECT_H */

