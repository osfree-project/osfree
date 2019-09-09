/*    
	fat_tools.c	1.7
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

*	fat filesystem utilities
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

/*----------------------------------------------------------------------------*/

static WORD	CurDirStart_save;
static int	CurrentDirCluster_save;
static WORD	CurDirEntry_save;

static char	cwd[256];

int	DosFlag;

static int	DiskHandle = 0;
static WORD 	BytesPerSector;
static WORD	BytesPerCluster;
static WORD	CurDirStart;

static BYTE	SectorsPerCluster;
static WORD	ReservedSectors;
static BYTE	Fats;
static BYTE	SectorsPerFat;
static WORD	RootDirEntries;
static WORD	RootDirSectors;
static WORD	SectorCount;
static WORD	DirEntriesPerCluster;
static WORD	DirEntriesPerSector;

static int	CurrentDirCluster;
static WORD	CurDirEntry;
static int	CurrentDirSector; 
static WORD	RootDirSectors;
static WORD	CurrentDirSectorMax;
static int	RootSearch;
static BYTE 	FatChain[3 * 1024];

static WORD  	ChainStart = 0;

static char 	StreamName[128];

/*----------------------------------------------------------------------------*/
char	*strtolower(char *string);
/*----------------------------------------------------------------------------*/
int	GetStartCluster(void);
WORD	GetWord(BYTE *Number);
DWORD   GetDword(BYTE *Number);
void	PutWord(BYTE *Ptr, WORD Value);
void	PutDword(BYTE *Ptr, DWORD Value);
int	GetNextFreeCluster(WORD Current);
int	GetNextCluster(WORD Current);
int	SetNextCluster(WORD Cluster, WORD Value);
int	ClusterToSector(WORD ClusterNo);
int	NameSplit(char *FullName, char *BaseName, char *Ext);
int	NameCombine(char *EightPlusThree, char *FileName);
int	FindEntry(char *FileName,struct DirEntry *DirPtr,long *EntryOffset);
char	*GetComponent(char *Component, char *Path);
int	GetTailCluster(int Current);
int	GetFreeDirEnt(struct DirEntry *DirPtr, long *EntryOffset);
int	InitDirEnt(void);
int	GetNextDirEnt(struct DirEntry *DirPtr, long *EntryOffset);
int	GetNextRootDirEnt(struct DirEntry *DirPtr, long *EntryOffset);
char	*GetTail(char *Path);
long	ClusterToOffset(int Handle, WORD *OffsetPtr, int *ClusterPtr);
void	SaveInfo(void);
void	RestoreInfo(void);
void	GetTimeDate(const struct DirEntry *Entry, char *DateStr, 
			char *TimeStr);
void	SetTimeDate(struct DirEntry *Entry);
void	dos_cd(char *Name, struct DirEntry *DirPtr);
int	dos_chdir(char *Path, struct DirEntry *DirPtr, long *EntryOffset);

int	DosExist(char *Path, struct DirEntry *DirPtr, long *EntryOffset);

int
MakeNewCluster(struct DirEntry *Dir)
{
	char 	Acluster[2048];
	long	DirOffset;
	struct	DirEntry *DirDot, *DirDotDot;

	/* Make an Empty Cluster for new directory */
	DirOffset = ClusterToSector(GetWord((BYTE *)&Dir->Start[0]));
	DirOffset *= BytesPerSector;
	memset((char *)&Acluster[0], 0, BytesPerCluster);

	/* Make the '.' Entry */
	DirDot = (struct DirEntry *)&Acluster[0];
	memset((char *)&DirDot->Name[0], 0x20, 11);
	DirDot->Name[0] = '.';
	DirDot->Attrib = DIRECTORY;
	memcpy((char *)&Dir->Start[0], (char *)&DirDot->Start, sizeof(WORD));
	PutDword((BYTE *)&DirDot->Size[0], 0);
	SetTimeDate(DirDot);

	/* Make the '..' entry */
	DirDotDot = (struct DirEntry *) (&Acluster[0]) + 1;
	memcpy((char *)DirDotDot, (char *)DirDot, sizeof(struct DirEntry));
	DirDotDot->Name[1] = '.';
	PutWord((BYTE *)&DirDotDot->Start[0], CurDirStart);

	lseek(DiskHandle, DirOffset, SEEK_SET);
	if (write(DiskHandle, Acluster, BytesPerCluster) < (int)BytesPerCluster) {
		DosFlag = CANNOT_MAKE_DIRECTORY_ENTRY | CARRY_FLAG;
		errno = ENOENT;
		return(-1);
	} else
		return(0);
}

/*----------------------------------------------------------------------------*/
int
GetNextFreeCluster(WORD Current)
{	
	WORD MaxClusters;
	int temp;
	WORD i;
	
	if ((temp =  GetNextCluster(Current)) != -1)
		return ( temp);

	MaxClusters =  SectorCount / SectorsPerCluster ;
	for ( i = 2; i <= MaxClusters; i++) {
		if ( GetNextCluster(i) == 0) {
			if ( Current != 0)
				SetNextCluster(Current, i);
			SetNextCluster(i, 0x0ff8);
			return(i);
		}
	}
 	errno = EFBIG;
	return(-1);
}

/*----------------------------------------------------------------------------*/
int
GetNextCluster(WORD Current)
{
	WORD shiftcount = 0;
	WORD NextCluster;
	WORD SectorOffset, ByteOffset;
	WORD index;
	if (Current == 0)
		return(-1);
	index = ( Current * 3 );
	if ( index & 0x0001)  
		shiftcount = 4;
	index /= 2;
	SectorOffset = (int)index / (int)( BytesPerSector * 3 ); 
	ByteOffset = (int)index %  (int)( BytesPerSector * 3 );
	if (SectorOffset != ChainStart) {
		lseek(DiskHandle, (SectorOffset * 3 + ReservedSectors) * 
			BytesPerSector,SEEK_SET);
		read(DiskHandle, FatChain, BytesPerSector * 3);
		ChainStart = SectorOffset;
	}
	NextCluster = GetWord((BYTE *)FatChain + ByteOffset) >> shiftcount;
	NextCluster &= 0x0fff;
 	if ( (NextCluster >=  0x0ff8) && (NextCluster <= 0x0fff ))
		return( -1);
	else
		return ( NextCluster );
}

/*----------------------------------------------------------------------------*/
int
SetNextCluster(WORD Cluster, WORD Value)
{
	int	rc;
	WORD shiftcount = 0;
	WORD CurrentValue;
	WORD SectorOffset, ByteOffset;
	WORD index;

	index = ( Cluster * 3);
	if ( index & 0x0001)
		shiftcount = 4;
	index /= 2;
        SectorOffset   = (int)index / (int)( BytesPerSector * 3 );
        ByteOffset = (int)index %  (int)( BytesPerSector * 3 );
        if (SectorOffset != ChainStart) {
			lseek(DiskHandle, (SectorOffset * 3 + ReservedSectors) *
				BytesPerSector,SEEK_SET);
			read(DiskHandle, FatChain, sizeof(FatChain));
        	ChainStart = SectorOffset;
        }
	CurrentValue = GetWord((BYTE *)(&FatChain[0] + ByteOffset));
	if ( shiftcount == 0) {
		CurrentValue &= 0xf000;
		CurrentValue |= Value & 0x0fff;
	}
	if ( shiftcount == 4) {
		CurrentValue &= 0x000f;
		CurrentValue |= (Value << shiftcount);
	}
	PutWord((BYTE *)(FatChain + ByteOffset), CurrentValue);	
	lseek(DiskHandle, (ChainStart * 3 + ReservedSectors) * 
		BytesPerSector,SEEK_SET);
	rc=write(DiskHandle, FatChain, BytesPerSector * 3);
	if(rc<0)
		printf("write error #1\n");
}

/*----------------------------------------------------------------------------*/
int
ClusterToSector(WORD ClusterNo)
{	
	int DataStart;

	DataStart = (ReservedSectors + Fats * SectorsPerFat) +
		RootDirSectors; 
	return ( (ClusterNo - 2) * SectorsPerCluster + DataStart);
}

/*----------------------------------------------------------------------------*/
int
NameSplit(char *FullName, char *BaseName, char *Ext)
{
	char *index;
	int count;
	int i;

	if ((index = strchr(FullName,'.')) != NULL) {
		count = index - FullName;
		count = ( count > 8 ) ? 8 : count;
		strncpy(BaseName, FullName, count);
		for (;count < 8; count++)
			*(BaseName + count) = ' ';
		count = FullName + strlen(FullName) - index - 1;
		count = ( count > 3 ) ? 3 : count;
		strncpy(Ext,index + 1, count);
		for (;count < 3; count++)
			*(Ext + count) = ' ';
	} else {
		count = strlen(FullName);
		count = ( count > 8 ) ? 8 : count;
		strncpy(BaseName, FullName, count);
		for (;count < 8; count++)
			*(BaseName + count) = ' ';
		strcpy(Ext,"   ");
	}
	for(i = 0; i < 8; i++)
		if ( islower(BaseName[i]))
			BaseName[i] = toupper(BaseName[i]);

	for(i = 0; i < 3; i++)
		if ( islower(Ext[i]))
			Ext[i] = toupper(Ext[i]);
}

/*----------------------------------------------------------------------------*/
/* Copies the 8+3 form string to a name.ext string into FileName */
int
NameCombine(char *EightPlusThree, char *FileName)
{
	register int	i;

	for (i = 0; i < 8; i++)
		if (EightPlusThree[i] != ' ')
			*FileName++ = EightPlusThree[i];
		else
			break;
	*FileName++ = '.';
	for (i = 8; i < 11; i++)
		if (EightPlusThree[i] != ' ')
			*FileName++ = EightPlusThree[i];
		else
			break;
	if (*(FileName - 1) == '.')
		*(FileName - 1) = 0;
	else
		*FileName = 0;
}

/*----------------------------------------------------------------------------*/
int
FindEntry(char *FileName, struct DirEntry *DirPtr, long *EntryOffset)
{
	char BaseName[9];
	char Ext[4];
	char Name[13];

	BaseName[8] = 0;
	Ext[3] = 0;
	
	if ((strcmp(FileName,"..") != 0 ) && 
	    (strcmp(FileName,".") != 0)) {
		NameSplit(FileName,BaseName, Ext);
		strcpy(Name, BaseName);
		strcat(Name, Ext);
	} else
		strcpy(Name,"..         ");
	InitDirEnt();
	while ( GetNextDirEnt(DirPtr, EntryOffset) != -1)
		if ( strncmp((char *)&DirPtr->Name[0], Name, 11) == 0)
			return(0);	
	errno = ENOENT;
	DosFlag = (FILE_NOT_FOUND | CARRY_FLAG);
	return(-1);
}


/*----------------------------------------------------------------------------*/
/*
 * extracts a dirname component from the path and puts into Component.
 * returns the path with the component removed. 
 */ 

char * 
GetComponent(char *Component, char *Path)
{
	if (Path[1] == ':')
		Path += 2;
	if (*Path == '\\')
		Path++;
	while((*Path != '\\') && (*Path != 0))
		*Component++ = *Path++;
	*Component = 0;
	if ( *Path == 0)
		return NULL;
	else
		return(Path);
}

/*----------------------------------------------------------------------------*/
/* Seperates a pathname into two pieces:
 * eg. if a:\alpha\beta\gamma\delta is passes as Path
 * a pointer to "delta" is returned and \alpha\beta\gamma is copied to
 * the buffer pointed by Head
 */
char *
GetHead(char *Head, char *Path)
{
	char	*cptr;
	int	count;

	if ((cptr = strrchr(Path, '\\')) == (char *)NULL) {
		*Head = 0;
		return(Path);
	} else {
		count = cptr - Path;
		strncpy(Head, Path, count);
		Head[count] = 0;
		return(++cptr);
	}
}

/*----------------------------------------------------------------------------*/
int
DiskInit()
{
	int i;
	int CurStartCluster;
	char Asector[512];
	short LittleEndian;

	strcat(StreamName,"/dev/fd0");

	while( (DiskHandle = open(StreamName, O_RDWR )) == -1) {
		fprintf(stderr, "Accessing stream image %s\n",StreamName);
		fprintf(stderr, "A)bort, R)etry, F)ail FileStream = ");
		fflush(stderr);
		fgets(StreamName,128,stdin);
		if(i=strlen(StreamName)) 
			StreamName[i-1] = 0;
		if(strlen(StreamName) == 0) 
			strcat(StreamName,"/dev/fd0");
		printf("using stream [%s](%d)\n",StreamName,strlen(StreamName));
	}

	read(DiskHandle, Asector, 512);

	if ( GetWord((BYTE *)&Asector[510]) == 0x55AA )
		LittleEndian = TRUE;
	else if ( GetWord((BYTE *)&Asector[510]) == 0xAA55 )
		LittleEndian = FALSE;
	else {
		fprintf(stderr, "ERROR: Dos signature not found in image\n");
		return(ERR_INVMEDIA);
	}

	BootBlock = (struct BpbInfo *)Asector;
	BytesPerSector = GetWord((BYTE *)&BootBlock->BytesPerSector[0]);
	SectorsPerCluster = BootBlock->SectorsPerCluster;
	ReservedSectors = GetWord((BYTE *)&BootBlock->ReservedSectors[0]);
	SectorsPerFat = GetWord((BYTE *)&BootBlock->SectorsPerFat[0]);
	SectorCount = GetWord((BYTE *)&BootBlock->SectorCount[0]);
	Fats = BootBlock->Fats;
	RootDirEntries = GetWord((BYTE *)&BootBlock->RootDirEntries[0]);
	BytesPerCluster = BytesPerSector * SectorsPerCluster;
	DirEntriesPerCluster = BytesPerCluster / sizeof ( struct DirEntry );
	DirEntriesPerSector = BytesPerSector / sizeof ( struct DirEntry );

	CurStartCluster = (int)(ReservedSectors + Fats * SectorsPerFat) /
		(int)SectorsPerCluster;

	cwd[0] = 0;
	lseek(DiskHandle, ReservedSectors * BytesPerSector,SEEK_SET);
	read(DiskHandle, FatChain, BytesPerSector * 3);
	ChainStart = 0;

	for ( i = 0; i < FTABLE_SIZE; i++)
		Ftable[i].Flags = F_CLOSED;

	return(0);
}

/*----------------------------------------------------------------------------*/
int
DiskCleanup()
{
	if (DiskHandle != -1) {
		close(DiskHandle);
		DiskHandle = -1;
		return(0);
	}
	return(-1);
}

/*----------------------------------------------------------------------------*/
int
GetTailCluster(int Current)
{	
	int Next2, Next1;
	Next1 = Current;
	while (( Next2 = GetNextCluster(Next1)) != -1)
		Next1 = Next2;
	return( Next1);
}

/*----------------------------------------------------------------------------*/
int
GetFreeDirEnt(struct DirEntry *DirPtr, long *EntryOffset)
{
	int  i, temp;
	static char Acluster[2048];

	InitDirEnt();

	while(GetNextDirEnt(DirPtr, EntryOffset) != -1)
		if (( DirPtr->Name[0] == 0x00) || (DirPtr->Name[0] == 0xe5))
			return(0);

	if ( *cwd == 0) {
		DosFlag = (ACCESS_DENIED | CARRY_FLAG);
		return(-1);
	} else {
		temp = GetTailCluster(CurDirStart);
		temp = GetNextFreeCluster(temp);
		*EntryOffset = ClusterToSector(temp) * BytesPerSector;	
		memset((char *)&Acluster[0], 0, BytesPerCluster);
		lseek(DiskHandle, *EntryOffset, SEEK_SET);
		if (write(DiskHandle, Acluster,BytesPerCluster) < (int)BytesPerCluster){
			DosFlag =  ( ACCESS_DENIED | CARRY_FLAG);
			printf("write error #2\n");
			return(-1);
		} else
			return(0);
	}
}

/*----------------------------------------------------------------------------*/
int
InitDirEnt()
{
	CurrentDirCluster = CurDirStart;
	CurDirEntry = 0;
	CurrentDirSector = (ReservedSectors + Fats * SectorsPerFat); 
	RootDirSectors = RootDirEntries /  DirEntriesPerSector;
	CurrentDirSectorMax = CurrentDirSector + RootDirSectors;
}

/*----------------------------------------------------------------------------*/
int
GetNextDirEnt(struct DirEntry *DirPtr, long *EntryOffset)
{
	long DiskOffset;

	if ( *cwd == 0)
		return (GetNextRootDirEnt(DirPtr, EntryOffset));

	if ( CurDirEntry == DirEntriesPerCluster) {
		CurrentDirCluster = GetNextCluster(CurrentDirCluster);
		if ( CurrentDirCluster == -1) {
			errno = ENOENT;
			return ( -1);
		}
		CurDirEntry = 0;
	}
	DiskOffset = ClusterToSector(CurrentDirCluster) * BytesPerSector
			+ CurDirEntry * 32;
	lseek(DiskHandle, DiskOffset, SEEK_SET);
	read(DiskHandle,DirPtr, 32);

	*EntryOffset = DiskOffset;
	CurDirEntry++;
	return (0);
}

int
WriteDirEntry( struct DirEntry *Dir, long EntryAddress)
{
	int rc;
	lseek(DiskHandle, EntryAddress, SEEK_SET);
	rc = write(DiskHandle, Dir, 32);
	if(rc <0) 
		printf("write error #11\n");
	return (0);
}

/*----------------------------------------------------------------------------*/
int
GetNextRootDirEnt(struct DirEntry *DirPtr, long *EntryOffset)
{
	long DiskOffset;

	if (CurDirEntry == DirEntriesPerSector) {
		if( ++CurrentDirSector == CurrentDirSectorMax) {
			errno = ENOENT;
			return(-1);
		}
		CurDirEntry = 0;
	}
	DiskOffset = CurrentDirSector * BytesPerSector + CurDirEntry * 32;
	lseek(DiskHandle, DiskOffset, SEEK_SET);
	read(DiskHandle,DirPtr, 32);
	*EntryOffset = DiskOffset;
	CurDirEntry++;
	return(0);
}

/*----------------------------------------------------------------------------*/
char *
GetTail(char *Path)
{
	char *temp;
	if (( temp = strrchr(Path,'\\')) != NULL)
		return (++temp);
	else
		return (Path);
}

/*----------------------------------------------------------------------------*/
long
ClusterToOffset(int Handle, WORD *OffsetPtr, int *ClusterPtr)
{
        int Count = 1;
        long Pos;

        *ClusterPtr = GetWord((BYTE *)&(Ftable[Handle].Dir.Start[0]));
        Pos = Ftable[Handle].CurPos;
        while ( Pos >= (long)(Count * BytesPerCluster)) {
                Count++;
                *ClusterPtr = GetNextFreeCluster(*ClusterPtr);
                }
        *OffsetPtr = Pos % (long)BytesPerCluster;
        return (0);
}

/*----------------------------------------------------------------------------*/
void
SaveInfo()
{
	CurDirEntry_save       = CurDirEntry;
	CurrentDirCluster_save = CurrentDirCluster;
	CurDirStart_save       = CurDirStart;
}

/*----------------------------------------------------------------------------*/
void
RestoreInfo()
{
	CurDirEntry       = CurDirEntry_save;
	CurrentDirCluster = CurrentDirCluster_save;
	CurDirStart       = CurDirStart_save;
}

/*----------------------------------------------------------------------------*/
/* Formats date and time information in Entry structure into char buffers
 * date and time respectively. Note DateStr should be atleast 9 bytes long
 * and the TimeStr atleast 7 bytes long
 */
void
GetTimeDate(const struct DirEntry *Entry, char *DateStr, char *TimeStr)
{
	WORD	wTmp;
	int	i1, i2, i3;
	char	cTmp;

	/* Get time */
	wTmp = GetWord((BYTE *)&Entry->Time[0]);
	i1 = (int)(wTmp & HOUR_MASK) >> HOUR_SHIFT;
	i2 = (int)(wTmp & MIN_MASK) >> MIN_SHIFT;
	if (i1 > 12) {
		i1 -= 12;
		cTmp = 'p';
	} else {
		cTmp = 'a';
	}
	sprintf(TimeStr, "%2d:%02d%c", i1, i2, cTmp);

	/* and date .. */
	wTmp = GetWord((BYTE *)&Entry->Date[0]);
	i1 = (int)(wTmp & MONTH_MASK) >> MONTH_SHIFT;
	i2 = (int)(wTmp & DAY_MASK) >> DAY_SHIFT;
	i3 = ((int)(wTmp & YEAR_MASK) >> YEAR_SHIFT) + 1980;
	i3 -= 1900;
	sprintf(DateStr, "%2d-%02d-%2d", i1, i2, i3);

	return;
}

/*----------------------------------------------------------------------------*/
/* Sets the current time and date in the Entry structure */
void
SetTimeDate(struct DirEntry *Entry)
{
	struct tm	*time;
	time_t	clock;
	struct timeval	tv;
	struct timezone	tz;
	WORD	wTmp;

	/* gettimeofday(&tv, &tz); */
	tv.tv_sec = 0;
	clock = (time_t)(tv.tv_sec);
	time = localtime(&clock);

	/* Set the time */
	wTmp = time->tm_hour << HOUR_SHIFT;
	wTmp |= time->tm_min << MIN_SHIFT;
	wTmp |= (time->tm_sec / 2) << SEC_SHIFT;
	PutWord((BYTE *)(Entry->Time), wTmp);

	/* And date.. */
	wTmp = time->tm_mday << DAY_SHIFT;
	wTmp |= (time->tm_mon + 1) << MONTH_SHIFT;
	wTmp |= (time->tm_year - 80) << YEAR_SHIFT;
	PutWord((BYTE *)(Entry->Date), wTmp);
}

/*----------------------------------------------------------------------------*/
/* 
 * changes the current directory to 'Name' if it exists and returns 0
 * returns -1 otherwise.
 */
 
void 
dos_cd(char *Name, struct DirEntry *DirPtr)
{
	char *temp;

	if (strcmp(Name,"..") == 0) {
		temp = strrchr(cwd,'\\');
		*temp = 0;
	} else if(strcmp(Name,".") != 0) {
		strcat(cwd,"\\");
		strcat(cwd,Name);
	}
	CurDirStart = GetWord((BYTE *)&DirPtr->Start[0]);
}

/*----------------------------------------------------------------------------*/
int
dos_chdir(char *Path, struct DirEntry *DirPtr, long *EntryOffset)
{
    char DirName[256];

    /* this is the current directory, and flag to get first entry */
    cwd[0] = 0;

    while( (Path = GetComponent(DirName, Path)) != NULL ) {
    	if (FindEntry(DirName, DirPtr, EntryOffset) < 0) {
			errno = ENOTDIR;
            return (-1);
        } else  if (DirPtr->Attrib & DIRECTORY ) {
            dos_cd(DirName, DirPtr);
        }
    }
	if (DirName[0] == 0) {
		InitDirEnt();
		return(0);
	}
    if (FindEntry(DirName, DirPtr, EntryOffset) < 0) {
		errno = ENOTDIR;
        return (-1);
    } 
	if (DirPtr->Attrib & DIRECTORY ) 
        dos_cd(DirName, DirPtr);
	InitDirEnt();
	return (0);
}

/*----------------------------------------------------------------------------*/
/*
 * returns 0 if the file pointed by Path exists and -1 otherwise. ERROR
 * is returned in errno;
 */
int
DosExist(char *Path, struct DirEntry *DirPtr, long *EntryOffset)
{
    char DirName[256];
	int len;

	cwd[0] = 0;
	len  = strlen(Path);
	if ( Path[len-1] == '\\')
		Path[len-1] = 0;
    	while( (Path = GetComponent(DirName, Path)) != NULL ) {
        	if (FindEntry(DirName, DirPtr, EntryOffset) < 0) {
			errno = ENOTDIR;
            		DosFlag = (PATH_NOT_FOUND | CARRY_FLAG);
            		return(-1);
		} else  if (DirPtr->Attrib & DIRECTORY ) {
			dos_cd(DirName, DirPtr);
		}
        }

    	return (FindEntry(DirName, DirPtr, EntryOffset));
}

int 
dos_open(char *Path, WORD Mode)
{	
    int  rc;
    struct DirEntry Dir;
    int     Handle;
    long EntryAddress;
    char FullName[12];
    char Base[9];
    char Ext[4];

    DosFlag = 0;

    if (DosExist(Path, &Dir, &EntryAddress) < 0) {
        if ((Mode & O_CREAT) && ( DosFlag & FILE_NOT_FOUND)) {
            if (GetFreeDirEnt(&Dir, &EntryAddress) == -1)
				return(-1);
            strcpy(FullName,GetTail(Path));
            NameSplit(FullName,Base,Ext);
            strncpy((char *)&Dir.Name[0], Base,8);
            strncpy((char *)&Dir.Name[8], Ext,3);
            Dir.Attrib = 0;
            PutWord((BYTE *)&Dir.Start[0], GetNextFreeCluster(0));
            PutDword((BYTE *)&Dir.Size[0], 0);
			SetTimeDate(&Dir);
            lseek(DiskHandle,EntryAddress,SEEK_SET);
            rc = write(DiskHandle,&Dir, 32);
	    if(rc <0)
		printf("write error #3\n");
        } else {
            return (-1);
		}
    } else if (Dir.Attrib & DIRECTORY) {
		DosFlag = (FILE_NOT_FOUND | CARRY_FLAG);
		return(-1);
	}
    for ( Handle = 0; Handle < FTABLE_SIZE; Handle++) {
        if (Ftable[Handle].Flags & F_CLOSED ) {
            strcpy(Ftable[Handle].Name, Path);
            Ftable[Handle].Mode = Mode;
            Ftable[Handle].Flags &= ~F_CLOSED;
            Ftable[Handle].Dir = Dir;
            Ftable[Handle].CurPos = 0;
            Ftable[Handle].EntryOffset = EntryAddress;
            return(Handle);
        }
    }

    DosFlag = (NO_HANDLES | CARRY_FLAG);
    return(-1);
}
	
/*----------------------------------------------------------------------------*/
int
dos_lseek(int Handle, long Offset, int Point)
{
	long  CurOffset;
	int	CurrentCluster;

	DosFlag = 0;
	switch ( Point) {
		case SEEK_SET:
			Ftable[Handle].CurPos = 0;
			break;
		case L_INCR:
			break;
		case L_XTND: 
			Ftable[Handle].CurPos = GetDword((BYTE *)
				&(Ftable[Handle].Dir.Size[0]));
		}
	Ftable[Handle].CurPos += Offset;
	return (Ftable[Handle].CurPos);
}

/*----------------------------------------------------------------------------*/
int
dos_close(int Handle)
{
	DosFlag = 0;
	Ftable[Handle].Flags |= F_CLOSED;
	return (0);
}

/*----------------------------------------------------------------------------*/
int
dos_read(int Handle, char *Buf, int Nbytes)
{
	int Count;
	long BytesLeft;
	int BytesToRead;
	char *TmpPtr;
	long DiskOffset;
	WORD ByteOffset;
	int CurrentCluster;

	DosFlag = 0;
	if ( Ftable[Handle].Flags & F_CLOSED ) {
		errno = EBADF;
		return (-1);
	}
	TmpPtr = Buf;
	BytesLeft = GetDword((BYTE *)&(Ftable[Handle].Dir.Size[0])) - 
		Ftable[Handle].CurPos;
	if ( BytesLeft == 0)
		return (0);
	Count = 0;

	if ( Nbytes  >  BytesLeft ) 
		BytesToRead = BytesLeft;
	else
		BytesToRead = Nbytes;
	ClusterToOffset(Handle, &ByteOffset, &CurrentCluster);
	DiskOffset = ClusterToSector(CurrentCluster) *
		BytesPerSector + ByteOffset;
	lseek(DiskHandle, DiskOffset, SEEK_SET);
	if ( (ByteOffset != 0) && ( CurrentCluster != -1)) {
		if ( BytesToRead > (int)( BytesPerCluster - ByteOffset))
			Count = (int)(BytesPerCluster - ByteOffset);
		else
			Count = BytesToRead; 
		read(DiskHandle, TmpPtr, Count);
		TmpPtr += Count;
		BytesToRead -=  Count;
	}
	while ( BytesToRead > 0 ) { 
		DiskOffset = ClusterToSector(CurrentCluster) *
			BytesPerSector;
		lseek(DiskHandle, DiskOffset, SEEK_SET);
		if ( BytesToRead >  (int)BytesPerCluster)
			Count = BytesPerCluster;
		else
			Count = BytesToRead;
		read(DiskHandle, TmpPtr, Count);
		TmpPtr += Count;
		BytesToRead -= Count;
		CurrentCluster = GetNextCluster(CurrentCluster);
	}
	Ftable[Handle].CurPos += (int)(TmpPtr - Buf);
	return ( (int)(TmpPtr - Buf));
}


/*----------------------------------------------------------------------------*/
dos_write(int Handle, char *Buf, int Nbytes)
{
	int   rc;
	char *TmpPtr;
	int Count;
	int BytesToWrite;
	WORD ByteOffset;
	int CurrentCluster;
	long DiskOffset;
	long CurSize;

	DosFlag = 0;
	CurSize = GetDword((BYTE *)&(Ftable[Handle].Dir.Size[0]));
	if ( Ftable[Handle].Flags & F_CLOSED ) {
		errno = EBADF;
		return (-1);
	}
	TmpPtr = Buf;
	BytesToWrite = Nbytes;
	ClusterToOffset(Handle, &ByteOffset, &CurrentCluster);
	DiskOffset = ClusterToSector(CurrentCluster) * BytesPerSector +
		ByteOffset;
	lseek(DiskHandle, DiskOffset, SEEK_SET);
	if ( ByteOffset != 0 && ( CurrentCluster != -1)) {
                if ( BytesToWrite > (int)(BytesPerCluster - ByteOffset))
                        Count = BytesPerCluster - ByteOffset;
                else
                        Count = BytesToWrite; 
                rc = write(DiskHandle, TmpPtr, Count);
		if(rc <0) 
			printf("write error #4\n");
                TmpPtr += Count;
                BytesToWrite -=  Count;
        }
        while ( BytesToWrite > 0 ) { 
                DiskOffset = ClusterToSector(CurrentCluster) *
					BytesPerSector;
                lseek(DiskHandle, DiskOffset, SEEK_SET);
                if ( BytesToWrite >  (int)BytesPerCluster)
                        Count = BytesPerCluster;
                else
                        Count = BytesToWrite;
                rc = write(DiskHandle, TmpPtr, Count);
		if(rc <0) 
			printf("write error #5\n");
                TmpPtr += Count;
                BytesToWrite -= Count;
                CurrentCluster = GetNextFreeCluster(CurrentCluster);
        }
        Ftable[Handle].CurPos += (int)(TmpPtr - Buf);
	if (Ftable[Handle].CurPos > CurSize) {
		PutDword((BYTE *)&(Ftable[Handle].Dir.Size[0]), Ftable[Handle].CurPos);
		lseek(DiskHandle, Ftable[Handle].EntryOffset,SEEK_SET);
		rc = write(DiskHandle, &Ftable[Handle].Dir, 32);
		if(rc <0) 
			printf("write error #6\n");
	}
        return ( (int)(TmpPtr - Buf));
}


