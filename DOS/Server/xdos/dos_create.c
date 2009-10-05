/*    
	dos_create.c	1.11
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

 */

#include "platform.h"

#include <unistd.h> 
#include <string.h>

#include "xdos.h"
#include "dosdefn.h"
#include "xdosproc.h"

DWORD mfs_create(DWORD, DWORD, DWORD, DWORD);
unsigned int 
dos_create (char *filename, unsigned int attribute, unsigned int mode)
{
	unsigned int	fileid = 0;
	unsigned int    pid;
	char        	tmpname[10];
	char        	basename[128];
	static int  	uniqnum = 0;

	if (mode == DOS_CREATE_TMPF) {
		pid = getpid();
		strcpy(basename, filename);
		while ( !fileid ) {
			sprintf(tmpname, "%.2x%.4xXD", uniqnum, pid);
			strcpy(filename, basename);
			strcat(filename, tmpname);
			fileid = mfs_create((DWORD)filename, (DWORD)attribute, (DWORD)mode, 0L );
			if (fileid == (CARRY_FLAG | FILE_EXISTS)) {
				fileid = 0;
				if (++uniqnum > 0xff)
					break;
			}
		}
	}
	else
		fileid = mfs_create((DWORD)filename, (DWORD)attribute, (DWORD)mode, 0L );

	return fileid;
}

