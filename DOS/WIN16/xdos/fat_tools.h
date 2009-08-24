/*************************************************************************
*
*       @(#)fat_tools.h	1.9
*	fat file system access routines 
*       Copyright (c) 1995-1996, Willows Software Inc.  All rights reserved.

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

**************************************************************************/

#ifndef fat_tools__h
#define fat_tools__h

/*----------------------------------------------------------------------------*/
/* DOS error codes from dosdefn.h */
#if 0
#define NO_ERROR				0x00
#endif
#define	INVALID_FUNCTION		0x01
#define	FILE_NOT_FOUND			0x02
#define	PATH_NOT_FOUND			0x03
#define	NO_HANDLES				0x04
#define	ACCESS_DENIED			0x05
#define	INVALID_HANDLE			0x06
#define	MEMORY_DESTROYED		0x07
#define	INSUFFICIENT_MEMORY		0x08
#define	INVALID_ADDRESS			0x09
#define	INVALID_ENVIRONMENT		0x0a
#define	INVALID_FORMAT			0x0b
#define	INVALID_ACCESS			0x0c
#define	INVALID_DATA			0x0d
#define	INVALID_DRIVE			0x0f
#define	REDIR_OFF_SERVER		0x10
#define	NOT_SAME_DEVICE			0x11
#define	NO_MORE_FILES			0x12
#define SEEK_ERROR         		0x19
#define	SHARING_VIOLATION		0x20
#define	LOCK_VIOLATION			0x21
#define SHARE_BUF_EXCEED     	0x24
#define	NETWORK_REQUEST_UNSUPPORTED	0x32
#define	FILE_EXISTS				0x50
#define	CANNOT_MAKE_DIRECTORY_ENTRY	0x52
#define	FAIL_ON_INT24			0x53
#define	TOO_MANY_REDIRECTIONS	0x54
#define	DUPLICATE_REDIRECTION	0x55
#define	INVALID_PASSWORD		0x56
#define	INVALID_PARAMETER		0x57
#define NOV_TIMEOUT				0xfe
#define NOV_LOCK_FAILURE		0xff

/* Attributes for a directory entry */
#define	READ_ONLY	0x01
#define	HIDDEN		0x02
#define SYSTEM		0x04
#define	VOLUME_LABEL	0x08
#define DIRECTORY	0x10
#define	ARCHIVED	0x20

/* Errors returned by DiskInit() */
#define	ERR_ACCESS	-1		/* Error accessing stream */
#define	ERR_INVMEDIA	-2		/* Not a DOS file system */

#define	FAT_CARRY_FLAG				0x04000000

/* For setting/getting file attributes */
#define	GET_ATTRIBUTE			0
#define	SET_ATTRIBUTE			1

/*----------------------------------------------------------------------------*/
#ifndef windows__h
typedef	unsigned char	BYTE;
typedef	unsigned short	WORD;
typedef	unsigned long	DWORD;
#endif

struct DirEntry {
	BYTE 	Name[11];
	BYTE 	Attrib;
	BYTE	Reserved[10];
	BYTE 	Time[2];
	BYTE 	Date[2];
	BYTE 	Start[2];
	BYTE 	Size[4];
};

typedef struct ffEntry {
	char	Name[13];	/* ASCIIZ string */
	BYTE	Attrib;
	WORD	Time;
	WORD	Date;
	DWORD	Size;
} FAT_FFENTRY;

/*----------------------------------------------------------------------------*/
/* NOTE: All functions return 0 on success and negative values for failure.
 * DosFlag can be checked for possible additional information and the external
 * variable 'errno'
 */
/* Needs to be called when a new disk is inserted */
int DiskInit(char *);
int TheSameDisk(int); /* checks whether it's the same floppy */

/* converts the string to legalized lower case values */
char *strtolower(char *);
char *GetHead(char *, char *);

/* To be called when a disk is to be removed */
int	DiskCleanup(void);

/* Opens a file in the DOS image stream and returns a DOS handle */
int	DosOpen(char *Path, WORD Mode);

/* Other file operations like the unix equivalents */
int	DosLseek(int Handle, long Offset, int Point);
int	DosClose(int Handle);
int	DosRead(int Handle, char *Buf, int Nbytes);
int	DosWrite(int Handle, char *Buf, int Nbytes);
int	DosDelete(char *Path);

/* Directory functions */
int	DosMakeDir(char *Path);
int	DosRemoveDir(char *Path);

/* Get/Set file attribs depending on operation */
int	DosChAttrib(char *filename, unsigned int *attribute, int operation);

/* File search file spec is ????????.???
 * first is set for the first directory entry(excluding ., ..) in the Path
 * Subsequent calls to DosSearch would give the next file, size and attribute
 * until it returns -1
 */
int	DosSearch(int first, char *Path, char *filename, 
	unsigned int *attribute, unsigned long *size);

/*----------------------------------------------------------------------------*/

#define	TRUE		1
#define	FALSE		0

#define	UNUSED		0x00
#define	START_E5	0x05
#define	ERASED		0xe5

#define	FAT_START	512
#define	FTABLE_SIZE	10
#define	F_CLOSED	0x0001

#define	MAX_DIR_PER_DIR	64	/* Max directories per directory */

/* Time field(WORD) in directory entry */
#define	HOUR_MASK	0xf800
#define	HOUR_SHIFT	11
#define	MIN_MASK	0x07e0
#define	MIN_SHIFT	5
#define	SEC_MASK	0x001f	/* times 2 in seconds ! */
#define	SEC_SHIFT	0

/* Date field(WORD) in directory entry */
#define	YEAR_MASK	0xfe00
#define	YEAR_SHIFT	9
#define	MONTH_MASK	0x01e0
#define	MONTH_SHIFT	5
#define	DAY_MASK	0x001f
#define	DAY_SHIFT	0


/*----------------------------------------------------------------------------*/
typedef struct tagBpbInfo {
	BYTE	Skip[3];
	char	OemName[8];
	BYTE 	BytesPerSector[2];
	BYTE	SectorsPerCluster;
	BYTE	ReservedSectors[2];
	BYTE	Fats;
	BYTE	RootDirEntries[2];
	BYTE	SectorCount[2];
	BYTE	MediaType;
	BYTE	SectorsPerFat[2];
	BYTE	SectorsPerTrack[2];
	BYTE	Heads[2];
	BYTE	HiddenSectors[4];
	} BpbInfo;

typedef struct tagFtableEntry {
	char Name[256];
	WORD Mode;
	long CurPos;
	WORD Flags;
	DWORD EntryOffset; 
	struct DirEntry Dir;
	} FtableEntry;

extern BpbInfo *BootBlock;
extern FtableEntry Ftable[FTABLE_SIZE];

#endif		/* fat_tools__h */
