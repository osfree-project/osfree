/*    
	fat_main.c	1.7
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

 *	main fat filesystem routines
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

int 
print (const struct DirEntry *Entry)
{
	char	Name[14];
	char	Date[16];
	char	Time[16];
	char	Size[16];

	if (Entry->Name[0] == 0xE5)
		return 0;

	strncpy(Name, (char *)&Entry->Name[0], 11);
	Name[11] = Name[10];
	Name[10] = Name[9];
	Name[9] = Name[8];
	Name[8] = ' ';
	Name[12] = 0;
	GetTimeDate(Entry, &Date[0], &Time[0]);

	if ( Entry->Attrib & DIRECTORY )
		strcpy(Size, "<DIR>");
	else if (Entry->Attrib & HIDDEN)
		return 0;
	else if (Entry->Attrib & SYSTEM)
		strcpy(Size, "<SYS>");
	else if (Entry->Attrib & VOLUME_LABEL)
		return 0;
	else
		sprintf(Size, "%ld", GetDword((BYTE *)&Entry->Size[0]));

	printf("%s    %8s   %s  %-16s\n", Name, Size, Date, Time);
	return 1;
}

void
fsdir(char *Path)
{
	struct DirEntry Entry;
	long EntryOffset;
	FFENTRY FFEntry;
	char	Label[13];
	int	count;
		
	dos_findfirst("\\", &FFEntry);
	while((FFEntry.Attrib & VOLUME_LABEL) == 0) {
		if(dos_findnext(&FFEntry))
			break;
	}

	if(FFEntry.Attrib & VOLUME_LABEL)
		strcpy(Label,FFEntry.Name);
	else	*Label = '\0';
	count = 0;
	
	printf(" Volume in drive %s\n Directory for %s\n\n", Label,Path);

	if (strcmp(Path,"\\") == 0) {
		InitDirEnt();
		while(GetNextDirEnt(&Entry, &EntryOffset) != -1) {
			if (Entry.Name[0] == 0) {
				printf("      %d File(s)  %-d bytes free\n",
					count,100000);
				return;
			}
			else
				if(print(&Entry))
					count++;
		}
	}

	if (DosExist(Path, &Entry, &EntryOffset) != -1) {
		if (Entry.Attrib & DIRECTORY) {
			dos_cd((char *)(Entry.Name), &Entry);
			InitDirEnt();
			while(GetNextDirEnt(&Entry, &EntryOffset) != -1)
				if (Entry.Name[0] != 0)
					if(print(&Entry))
						count++;
		} else
			if(print(&Entry))
				count++;
	}
	printf("      %d File(s)  %-d bytes free\n",count,100000);
}

/*----------------------------------------------------------------------------*/
int
fscat(char *Path)
{
	int Handle;
	int	count;
	int	i;
	char Buf[512];

	if ( (Handle = dos_open(Path, O_RDONLY)) == -1) {
		fprintf(stderr, "Could not open File %s \n", Path);
		return(-1);
	}
	while ( (count = dos_read(Handle, Buf, sizeof(Buf))) > 0) {
		for (i = 0; i < count; i++)
			putchar(Buf[i]);
	}	
	dos_close(Handle);
	return(0);
}


/*----------------------------------------------------------------------------*/
/* copies a file from DOS filesystem to a UNIX filesystem 
 * ie. SrcPath is a DOS path and DestPath is a Unix path 
 */
int
dtoucp(char *SrcPath, char *DestPath)
{
	int 	DosHandle;
	int	UnixHandle;
	int	count;
	char Buf[512];

	if ( (DosHandle = dos_open(SrcPath, O_RDONLY)) == -1) {
		fprintf(stderr, "Could not open DOS File %s \n", SrcPath);
		return(-1);
	}
	if ( (UnixHandle = open(DestPath, O_RDWR | O_CREAT, 0664)) == -1) {
		fprintf(stderr, "Could not open Unix File %s \n", DestPath);
		dos_close(DosHandle);
		return(-2);
	}
	while ( (count = dos_read(DosHandle, Buf, sizeof(Buf))) > 0) 
		if (write(UnixHandle, Buf, count) != count) {
			dos_close(DosHandle);
			close(UnixHandle);
			printf("write error #13\n");
			return(-3);
		}
	dos_close(DosHandle);
	close(UnixHandle);
	return(0);
}

/*----------------------------------------------------------------------------*/
int
fscopy(char *SrcPath, char *DestPath)
{
	int Handle1, Handle2, count;
	int flag;
	char Buf[512];

	printf("fscopy %s %s\n",SrcPath,DestPath);
}

/* CRUDE copy, DestPath is expected to be a Unix path */
int
fscp(char *SrcPath, char *DestPath)
{
	int Handle1, Handle2, count;
	char Buf[512];

	if ( (Handle1 = dos_open(SrcPath, O_RDONLY)) == -1) {
		fprintf(stderr, "Could not open File %s \n", SrcPath);
		return(-1);
	}
	if ( (Handle2 = dos_open(DestPath, O_RDWR | O_CREAT)) == -1) {
		fprintf(stderr, "Could not open File %s \n", DestPath);
		return(-2);
	}

	while ( (count = dos_read(Handle1, Buf, sizeof(Buf))) > 0) 
		if (dos_write(Handle2, Buf, count) != count)
			return(-3);

	dos_close(Handle1);
	dos_close(Handle2);
	return(0);
}

/* CRUDE recursive copy, DestPath is expected to be a Unix path */
int
fscpr(char *SrcPath, char *DestPath)
{
	FFENTRY	ffe;
	char	DirName[MAX_DIR_PER_DIR][11];
	char	SrcPathName[255];
	char	DestPathName[255];
	char	FileName[13];
	char	Tmp1[255], Tmp2[255];
	int	nDirectories = 0;
	int	nCurDir = 0;
	register char	*cptr;

	fprintf(stdout, "\n*******fscpr %s %s\n", SrcPath, DestPath);
	if (dos_findfirst(SrcPath, &ffe) < 0)
		return;

	/* make a copy of the parameters and add the last '/' or '\' if required */
	strcpy(SrcPathName, SrcPath);
	strcpy(DestPathName, DestPath);
	cptr = SrcPathName;
	while (*cptr != 0)
		++cptr;
	if (cptr[-1] != '\\') {
		cptr[0] = '\\';
		cptr[1] = 0;
	}
	cptr = DestPathName;
	while (*cptr != 0)
		++cptr;
	if (cptr[-1] != '/') {
		cptr[0] = '/';
		cptr[1] = 0;
	}
		
	do {
		if (ffe.Attrib & DIRECTORY) {
			if (ffe.Name[0] == '.')
				continue;
			NameCombine((char *)&ffe.Name[0], &DirName[nDirectories++][0]);
			sprintf(Tmp1, "%s%s", DestPathName, 
				strtolower((char *)&ffe.Name[0]));
				fprintf(stdout, "mkdir %s", Tmp1);
				if (mkdir(Tmp1, 0777) < 0)
					fprintf(stdout, "\tFailed: Error %d\n", errno);
				else
					fprintf(stdout, "\n");
		} else {
			strcpy(FileName, (char *)&ffe.Name[0]);
			sprintf(Tmp1, "%s%s", SrcPathName,strtolower((char *)&ffe.Name[0]));
			sprintf(Tmp2, "%s%s", DestPathName, strtolower(FileName));
			fprintf(stdout, "dtoucp %s %s", Tmp1, Tmp2);
			if (dtoucp(Tmp1, Tmp2) < 0)
				fprintf(stdout, "\tFailed\n");
			else
				fprintf(stdout, "\n");
		}
	} while (dos_findnext(&ffe) != -1);

	if (nDirectories == 0)
		return;

	while (nCurDir < nDirectories) {
		strcpy(Tmp1, SrcPathName);
		strcpy(Tmp2, DestPathName);
		strcat(Tmp1, strtolower(&DirName[nCurDir][0]));
		strcat(Tmp2, strtolower(&DirName[nCurDir][0]));
		++nCurDir;
		fscpr(Tmp1, Tmp2);
	}
}

/*----------------------------------------------------------------------------*/
int
main (int argc , char **argv)
{
	char	*cmdline;

	if (DiskInit() < 0)
		exit(-1);

	cmdline = argv[0];

	/* convert to basename... */
	if(strrchr(cmdline,'/')) {
		cmdline = strrchr(cmdline,'/');
		cmdline++;
	}
	if(strrchr(cmdline,'\\')) {
		cmdline = strrchr(cmdline,'\\');
		cmdline++;
	}

	if (strcmp(cmdline,"fscat") == 0) {

		if ( argc < 2) {
			printf("Usage: fscat PathName\n");
			exit(-1);
		}
		if (fscat(argv[1]) < 0) {
			exit(-2);
		}

	} else if (strcmp(cmdline,"fscp") == 0) {

		if ( argc < 3) {
			printf("Usage: fscp SourceName DestName\n");
			exit(-1);
		}
		if (fscp(argv[1], argv[2]) < 0) {
			printf("Copy failed\n");
			exit(-2);
		}

	} else if (strcmp(cmdline,"fsdir") == 0) {

		if ( argc < 2) 
			fsdir("\\");
		else
			fsdir(argv[1]);

	} else if (strcmp(cmdline,"fscpr") == 0) {
		if (argc < 3)
			printf("usage: fscpr source destination\n");
		else
			fscpr(argv[1], argv[2]);

	} else if (strcmp(cmdline,"fscopy") == 0) {
		if (argc < 3)
			printf("usage: fscopy source destination\n");
		else
			fscopy(argv[1], argv[2]);

	} else if (strcmp(cmdline,"fsrmdir") == 0) {

		if ( argc < 2) {
			printf("Usage: fsrmdir Pathname\n");
			exit(-1);
		}
		if (dos_rmdir(argv[1]) < 0) {
			printf("fsrmdir failed\n");
			exit(-2);
		}

	} else if (strcmp(cmdline,"fsdel") == 0) {

		if ( argc < 2) {
			printf("Usage: fsdel Pathname\n");
			exit(-1);
		}
		dos_delete(argv[1]);

	} else if (strcmp(cmdline,"fsmkdir") == 0) {

		if ( argc < 2) {
			printf("Usage: fsmkdir Pathname\n");
			exit(-1);
		}
		if (dos_mkdir(argv[1]) < 0) {
			printf("fsmkdir failed\n");
			exit(-2);
		}

	} else {
		printf("usage: %s\n",argv[0]);
		exit(-2);
	}

	DiskCleanup();
	return(0);
}

