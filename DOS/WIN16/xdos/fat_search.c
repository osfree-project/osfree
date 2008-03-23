/*    
	fat_search.c	1.7
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

 *	fat filesystem file/directory  search operation
 */

#include "platform.h"

int
DosSearch(int first, char *Path, char *filename, 
	unsigned int *attribute, unsigned long *size)
{
	struct DirEntry Entry;
	long EntryOffset;
	int status;
	char PathCopy[256];
	char *temp;

	DosFlag = 0;
	if (first) {
		strcpy(PathCopy, Path);
		temp = strrchr(PathCopy, '\\');
		temp[1] = 0;
		if (PathCopy[1] == ':')
			temp = &PathCopy[2];
		else
			temp = &PathCopy[0];
		if (strcmp(temp, "\\") == 0) {
			InitDirEnt();
			SaveInfo();
			RootSearch = 1;
		} else if (DosExist(temp, &Entry, &EntryOffset) != -1) {
			if (Entry.Attrib & DIRECTORY) {
				DosCd((char *)&Entry.Name[0], &Entry);
				InitDirEnt();
				SaveInfo();
				RootSearch = 0;
			} else {
				strncpy(filename, (char *)&Entry.Name[0], 8);
				temp = strchr(filename, ' ');
				if (temp)
					*temp = 0;
				filename[8] = 0;
				if (strncmp((char *)&Entry.Name[8], "   ",3) != 0) {
					strcat(filename,".");
					strncat(filename, (char *)&Entry.Name[8], 3);
				} 
				*attribute = Entry.Attrib;
				*size = GetDword((BYTE *)&Entry.Size[0]);
				return (0);
			}
		} else
			return (-1);
	}

	RestoreInfo();
	do {
		if (RootSearch == 1)
			status = GetNextRootDirEnt(&Entry, &EntryOffset);
		else
			status = GetNextDirEnt(&Entry, &EntryOffset);
	} while ((Entry.Name[0] == 0xe5) && (status != -1));

	if ((status == -1) || (Entry.Name[0] == 0x00))
		return (-1);
	strncpy(filename, (char *)&Entry.Name[0], 8);
	temp = strchr(filename, ' ');
	if (temp)
		*temp = 0;
	filename[8] = 0;
	if (strncmp((char *)&Entry.Name[8], "   ",3) != 0) {
		strcat(filename,".");
		strncat(filename, (char *)&Entry.Name[8], 3);
	}
	*attribute = Entry.Attrib;
	*size = GetDword((BYTE *)&Entry.Size[0]);
	SaveInfo();
	return (0);
}


