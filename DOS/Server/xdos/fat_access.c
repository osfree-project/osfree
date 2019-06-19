/*    
	fat_access.c	1.7
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

#include "fat_tools.h"

int
dos_access(char *filename, unsigned int *attribute, int operation)
{
	struct DirEntry Dir;
	long	EntryAddress;
	int	rc;

    	if (DosExist(filename, &Dir, &EntryAddress) < 0)
		return(-1);
	if ( operation == GET_ATTRIBUTE) {
		*attribute = Dir.Attrib;
		return (0);
	} else if (operation == SET_ATTRIBUTE) {
		Dir.Attrib = *attribute;
		lseek(DiskHandle, EntryAddress, SEEK_SET);
		rc = write(DiskHandle, &Dir, 32);
		if(rc <0) 
			ERRSTR(LF_ERROR,"write error #12\n");
		return (0);
	}
		
}

