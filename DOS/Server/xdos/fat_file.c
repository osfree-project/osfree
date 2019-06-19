/*    
	fat_file.c	1.7
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

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <string.h>

#include "fat_tools.h"

#include "unistd.h"

extern 	int 	DosFlag;

int 
dos_delete(char *Path)
{
	int	rc;
	struct DirEntry Dir;
	long	EntryAddress;
	int temp;

	DosFlag = 0;
    	if (DosExist(Path, &Dir, &EntryAddress) < 0) {
		DosFlag = (PATH_NOT_FOUND | CARRY_FLAG);
		return(-1);
	}

	if (Dir.Attrib & DIRECTORY) {
		DosFlag = (FILE_NOT_FOUND | CARRY_FLAG);
		return(-1);
	}

	if (Dir.Attrib & READ_ONLY) {
		DosFlag = (ACCESS_DENIED | CARRY_FLAG);
		return(-1);
	}

	temp = GetWord((BYTE *)&Dir.Start[0]);
	while ((temp = GetNextCluster(temp)) != -1)
		SetNextCluster(temp, 0);

	temp = GetWord((BYTE *)&Dir.Start[0]);
	SetNextCluster(temp, 0);
	Dir.Name[0] = 0xE5;
        return WriteDirEntry(&Dir,EntryAddress);
}


int
dos_mkdir(char *Path)
{
	char	*Tail;
	char	Head[255];
	long	EntryAddress;
	struct	DirEntry Dir;
	char FullName[12];
	char Base[9];
	char Ext[4];

    	if (DosExist(Path, &Dir, &EntryAddress) == 0) {
		DosFlag = (FILE_EXISTS | CARRY_FLAG);
		return(-1);
	}

	Tail = GetHead(&Head[0], Path);
	if (Head[0] != 0)
		dos_chdir((char *)&Head[0], &Dir, &EntryAddress);

	if (GetFreeDirEnt(&Dir, &EntryAddress) == -1) {
		errno = ENOENT;
		DosFlag = CANNOT_MAKE_DIRECTORY_ENTRY | CARRY_FLAG;
		return(-1);
	}

	/* Split the name */
	strcpy(FullName, Tail);
	NameSplit(FullName,Base,Ext);
	strncpy((char *)&Dir.Name[0], Base,8);
	strncpy((char *)&Dir.Name[8], Ext,3);

	/* Initialise the fields */
	Dir.Attrib = DIRECTORY;
	PutWord((BYTE *)&Dir.Start[0], GetNextFreeCluster(0));
	PutDword((BYTE *)&Dir.Size[0], 0);
	SetTimeDate(&Dir);

	/* Make a directory entry */
	WriteDirEntry(&Dir,EntryAddress);

	/* Write out a new cluster for this directory entry */
	MakeNewCluster(&Dir);
}

int
dos_rmdir(char *Path)
{
	struct DirEntry Dir, TempDir;
	long	EntryAddress, EntryOffset;
	int temp;
	int rc;

	DosFlag = 0;
    	if (DosExist(Path, &Dir, &EntryAddress) < 0) {
		DosFlag = (PATH_NOT_FOUND | CARRY_FLAG);
		return(-1);
	}

	if (Dir.Attrib & READ_ONLY) {
		DosFlag = (ACCESS_DENIED | CARRY_FLAG);
		return(-1);
	}
	dos_chdir(Path, &Dir, &EntryAddress);
	while ( GetNextDirEnt(&TempDir, &EntryOffset) != -1) {
		if ((TempDir.Name[0] == '.') ||
			(TempDir.Name[0] == 0xe5))
				continue;
		if (TempDir.Name[0] == 0)
			break;
		DosFlag = (FILE_EXISTS | CARRY_FLAG);
		return (-1);
	}

	temp = GetWord((BYTE *)&Dir.Start[0]);
	while ((temp = GetNextCluster(temp)) != -1)
		SetNextCluster(temp, 0);
	temp = GetWord((BYTE *)&Dir.Start[0]);
	SetNextCluster(temp, 0);
	Dir.Name[0] = 0xE5;
	return WriteDirEntry(&Dir,EntryAddress);
}


/*----------------------------------------------------------------------------*/
int
dos_findfirst(char *Directory, FFENTRY *FFEntry)
{
	long	lDummy;
	struct DirEntry	Entry;

	dos_chdir(Directory, &Entry, &lDummy);
	return(dos_findnext(FFEntry));
}


/*----------------------------------------------------------------------------*/
int
dos_findnext(FFENTRY *FFEntry)
{
	long	lDummy;
	struct DirEntry	Entry;

	while (1) {
		if (GetNextDirEnt(&Entry, &lDummy) < 0) {
			errno = ENOENT;
			return(-1);
		}
		if (Entry.Name[0] == 0) {
			errno = EMFILE;
			return(-1);
		}
		if (Entry.Name[0] != 0xE5)
			break;
	}

	NameCombine((char *)&Entry.Name[0], (char *)&FFEntry->Name[0]);
	FFEntry->Attrib = Entry.Attrib;
	FFEntry->Time = GetWord((BYTE *)&Entry.Time[0]);
	FFEntry->Date = GetWord((BYTE *)&Entry.Date[0]);
	FFEntry->Size = GetDword((BYTE *)&Entry.Size[0]);
	return(0);
}

