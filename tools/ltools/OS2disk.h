/*ldir/lread
   Program to read Linux extended 2 filesystems under DOS

   Module UNIXdisk.c
   Low level harddisk partition table and harddisk data read

   This is the operating system specific file for UNIX systems,
   hacked by Richard Zidlicky.

   Copyright information and copying policy see file README.TXT

   History see file MAIN.C
 */

/* ignore LBA/CHS for now - assume "partition special file" */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#ifdef __WATCOM__
#include "porting.h"
#endif

#include "ldir.h"
#include "ext2.h"
#include "proto.h"

extern FILE *STDOUT, *STDERR;

#define  PRINT_PARTITION_TABLE    for (k=0;k<4;k++)     				\
					 { printf("Entry %i: ",k);          	      	\
					   printf("%X %X %X %X %X %X %X %X %lX %lX\n",	\
                                            		pTEntry[k]->bootIndicator,  	\
                                                        pTEntry[k]->startHead,       	\
                                                        pTEntry[k]->startSector,      	\
                                                        pTEntry[k]->startCylinder,     	\
                                                        pTEntry[k]->system,          	\
                                                        pTEntry[k]->endHead,          	\
                                                        pTEntry[k]->endSector,         	\
                                                        pTEntry[k]->endCylinder,      	\
                                                        pTEntry[k]->leadSectors,      	\
                                                        pTEntry[k]->numSectors); }	\
				  printf("Partition Code %x%x\n",buf[510],buf[511]);


//*** Error codes ***
#define ERROR_NONE				 0
#define ERROR_CANNOT_READ_DISK_GEOMETRY		-1
#define ERROR_CANNOT_READ_PARTITON_TABLE	-2
#define ERROR_READ_DISK				-3
#define ERROR_WRITE_DISK			-4
#define ERROR_OPEN_DISK				-5
#define ERROR_READ_DISK_PARAMETER		-6
#define ERROR_PARAMETER_INVALID			-7

//*** Data types for disk access ***
typedef struct
{   unsigned int heads;
    unsigned int sectors;
    unsigned int cylinders;
} DISK_GEOMETRY_;

typedef struct 
{   unsigned int  partitionNumber;
    unsigned int  partitionType;
    unsigned char bootable;
    unsigned long startLBA;
    unsigned long numberOfSectors;
} PARTITION_TABLE_;

typedef struct
{   unsigned char bootIndicator;
    unsigned char startHead;
    unsigned char startSector;
    unsigned char startCylinder;
    unsigned char system;
    unsigned char endHead;
    unsigned char endSector;
    unsigned char endCylinder;
    unsigned long leadSectors;
    unsigned long numSectors;
} DISK_PARTITION_TABLE_ENTRY;


/* globals */
unsigned int HEADS;						/*your harddisk's # of heads               ) drive's */
unsigned int SECTORS;						/*                # of sectors per cylinder) geometry */
unsigned int CYLINDERS;						/*                # of cylinders           )         */
unsigned long start;						/*logical block address(LBA) of your Linux partition */
unsigned long num_sect;						/*total # of sectors of your Linux partition */
extern unsigned int disk_no;					/*DOS' disk #, eg. 0x80=first harddisk */
extern unsigned int part_no;					/*# of your Linux partition */
extern char ext2_disk[256];					/*your linux partition name, eg. /dev/hda5 */
extern char *disk_name;
extern int disk_fd;

extern enum
{
    LDIR, LREAD, LWRITE, LTEST
}
modus;

extern void *MALLOC(size_t size);
extern void FREE(void *block);

char image = 0;

#define INCL_LONGLONG
#define INCL_DOSERRORS
#define INCL_DOSDEVICES
#define INCL_DOSDEVIOCTL
#include <os2.h>

int ExtBiosdisk(int cmd, int drive, unsigned long lba, int nsects, unsigned char *buffer);
int open_disk(HFILE *ph, char *disk_name);
int close_disk(HFILE *ph);
int read_disk(HFILE handle,            // disk handle
	      unsigned char  *buffer,  // buffer 
	      unsigned long  lba,      // starting lba 
	      unsigned long nsec);     // number of sectors to read
int read_track(HFILE handle,           // Physical Disk handle
                unsigned char *buffer, // data buffer  
		unsigned short cyl,    // Starting Cylinder
                unsigned short head,   // Starting Head
                unsigned short sec,    // Starting Sector
                unsigned short count); // Count of sectors to read
int write_disk(HFILE handle,           // disk handle
	      unsigned char  *buffer,  // buffer 
	      unsigned long  lba,      // starting lba 
	      unsigned long nsec);     // number of sectors to write
int write_track(HFILE handle,          // Physical Disk handle
                unsigned char *buffer, // data buffer  
		unsigned short cyl,    // Starting Cylinder
                unsigned short head,   // Starting Head
                unsigned short sec,    // Starting Sector
                unsigned short count); // Count of sectors to write

int getDiskGeometry(unsigned int disk_no, DISK_GEOMETRY_ *diskGeometry);

union  {
  TRACKLAYOUT	t;
  char  s[320];
} u;


//This is the routine which does the direct harddisk access
int ExtBiosdisk(int cmd, int drive, unsigned long lba, int nsects, unsigned char *buffer)
{   
    unsigned long lba2;
    HFILE hPhysicalDrive;
    long dwBytesRead;
    DISK_GEOMETRY_ *pDiskGeometry;
    unsigned long long  fofs;
    char temp[64];
    char s[] = "1:";

    struct 	_DEVICEPARAMETERBLOCK d;
    APIRET	rc;
    UCHAR	t;
    ULONG	parm_len;
    ULONG       data_len;

    DebugOut(4, "cmd=%u disk_name='%s'  ext2_name='%s'  drive=%u lba=%12u nsects=%u\n", cmd, disk_name, ext2_disk, drive, lba, nsects);

    //modus = LDIR;

    if (modus==LTEST)
    { if ((drive==0) || (drive==1))
      {   sprintf(temp,"%c:\0", (char) (drive+'a'));
      } else if (drive >= 128)
      {   sprintf(temp,"%c:\0", (char) (drive-128+'1'));
      } else
      {   return ERROR_OPEN_DISK;
      }
	
      if (strchr(temp, ':') == NULL) 
          image = 1;
			     
    } else
    {	
      if (disk_name != NULL) { 
        if (strchr(disk_name, ':') == NULL) {
          image = 1;			     
          strncpy(temp, disk_name, strlen(disk_name) + 1);
        }
      } else {
          image = 0;			     
          strncpy(temp, s, strlen(s) + 1);
      }
    }	
    
    rc = open_disk(&hPhysicalDrive, temp);

    if (hPhysicalDrive == NULLHANDLE)
    {
	DebugOut(2,"---Error opening the drive # %d----\n", drive);
        return ERROR_OPEN_DISK;
    }

    if (cmd == PARA_CMD)
    {   

	t            = 0;
	parm_len     = sizeof(t);
        data_len     = sizeof(struct _DEVICEPARAMETERBLOCK);

        rc = DosDevIOCtl((HFILE)hPhysicalDrive,
	                 IOCTL_PHYSICALDISK,
			 PDSK_GETPHYSDEVICEPARAMS,
            		 &t, parm_len, &parm_len,
	                 &d, data_len, &data_len);

	if (rc != NO_ERROR)
	{
	   DebugOut(2, "DosDevIOCtl error: return code = %u\n", rc);
	   return ERROR_READ_DISK_PARAMETER;
	}    

	pDiskGeometry = (DISK_GEOMETRY_ *) buffer;

	pDiskGeometry->cylinders = d.cCylinders;
    	pDiskGeometry->heads     = d.cHeads;
    	pDiskGeometry->sectors   = d.cSectorsPerTrack;

	CYLINDERS   = pDiskGeometry->cylinders;
    	HEADS       = pDiskGeometry->heads;
    	SECTORS     = pDiskGeometry->sectors;

        rc = close_disk(&hPhysicalDrive);

    	return ERROR_NONE; 	
    } else if (cmd == READ_CMD)
    { 
        rc = read_disk((HFILE)hPhysicalDrive, buffer, lba, nsects);
        if (rc != NO_ERROR) {
            DebugOut(2, "Read failed\n");
	    close_disk(&hPhysicalDrive);
	    return ERROR_READ_DISK;
        };
        close_disk(&hPhysicalDrive);
	return ERROR_NONE;
    } else if (cmd == WRITE_CMD)    
    {   
    
/*    
	fofs = lba;
        fofs = fofs<<9;
        
	if (lseek64(hPhysicalDrive, fofs, SEEK_SET) == -1)
	{
	    DebugOut(2, "Seek failed---lba:%ld---fofs:%ld\n", lba, fofs);
	    close(hPhysicalDrive);
	    return ERROR_READ_DISK;
	}

	// Write sector ...
	if (write(hPhysicalDrive, buffer, nsects * DISK_BLOCK_SIZE) == -1)
	{
	    DebugOut(2, "Write failed\n");
	    close(hPhysicalDrive);
	    return ERROR_WRITE_DISK;
	}
	close(hPhysicalDrive);
 */

        rc = write_disk((HFILE)hPhysicalDrive, buffer, lba, nsects);
        if (rc != NO_ERROR) {
            DebugOut(2, "Write failed\n");
	    close_disk(&hPhysicalDrive);
	    return ERROR_READ_DISK;
        };
        close_disk(&hPhysicalDrive); 
	return ERROR_NONE;
    } else
	//DebugOut(2, "Illegal command in function ExtBiosdisk()  %X\n",cmd);
    return ERROR_PARAMETER_INVALID;
}

int open_disk(HFILE *ph, char *f)
{
    int rc;

    if (image) {

      *ph = open(f, O_RDWR | O_BINARY);

      if (*ph == -1) {
        DebugOut(2, "Error opening file %s\n", f);
        return ERROR_OPEN_DISK;
      }

      return ERROR_NONE;

    } 

    rc = DosPhysicalDisk(INFO_GETIOCTLHANDLE, 
			ph, 
	                2L, 
			f,
			strlen(f) + 1);

    if (rc != NO_ERROR)
    {
      DebugOut(2, "DosPhysicalDisk error: return code = %u\n", rc);
      return ERROR_OPEN_DISK;
    }

    return rc;
}

int close_disk(HFILE *ph)
{
    int rc;

    if (image) {
      close(*ph);
      return ERROR_NONE;
    }

    rc = DosPhysicalDisk(INFO_FREEIOCTLHANDLE, 
			NULL, 
	                0L, 
			ph,
			2L);

    if (rc != NO_ERROR) {
       DebugOut(2, "close_disk() error: %u\n", rc);
       return ERROR_READ_DISK_PARAMETER;
    }

    return rc;
}

//
// Read nsec sectors to buffer,
// beginning from sector with LBA address lba.
//
int read_disk(HFILE handle,           // disk handle
	      unsigned char  *buffer, // buffer 
	      unsigned long  lba,     // starting lba 
	      unsigned long nsec)     // number of sectors to read
{ 
    unsigned short cyl, head, sec;
    unsigned long  n;
    unsigned long long fofs;
    APIRET rc;

    if (image) {

      fofs = lba;
      fofs = fofs<<9;
        
      if (lseek64(handle, fofs, SEEK_SET) == -1)
      {
        DebugOut(2, "Seek failed---lba:%ld---fofs:%ld\n", lba, fofs);
        close(handle);
        return ERROR_READ_DISK;
      }
      // Read sector off of the drive...
      if (read(handle, buffer, nsec * DISK_BLOCK_SIZE) == -1)
      {
        DebugOut(2, "Read failed\n");
        close(handle);
        return ERROR_READ_DISK;
      }
      close(handle);      

      return ERROR_NONE;
    }

    do {
      // lba --> chs
      cyl  = lba  / (HEADS * SECTORS);
      head = (lba - HEADS  * SECTORS * cyl) / SECTORS;
      sec  = lba  - HEADS  * SECTORS * cyl - head * SECTORS + 1;
      // number of sectors to read at once
      if (sec + nsec > SECTORS)
         n = SECTORS - sec + 1;
      else
         n = nsec;
      // read a track or its part
      rc = read_track(handle, buffer, cyl, head, sec, n);
      if (rc != NO_ERROR)
      {
        DebugOut(2, "read_disk error: return code = %u\n", rc);
        return ERROR_READ_DISK;
      } 

      lba    = lba + n;
      nsec   = nsec - n;
      buffer = buffer + n * DISK_BLOCK_SIZE;

    } while (nsec > 0);

    return ERROR_NONE;
}

//
// Read count sectors from current sector with coordinates
// cyl, head, sec (count <= SECTORS) to buffer.
//
int read_track(HFILE handle,           // Physical Disk handle
                unsigned char *buffer, // data buffer  
		unsigned short cyl,    // Starting Cylinder
                unsigned short head,   // Starting Head
                unsigned short sec,    // Starting Sector
                unsigned short count)  // Count of sectors to read
{
    APIRET	   rc;
    ULONG          data_len,
		   parm_len;

    data_len        = DISK_BLOCK_SIZE;
    parm_len        = 9l;

    u.t.bCommand      = 1;
    u.t.usHead        = head;
    u.t.usCylinder    = cyl;
    u.t.usFirstSector = sec - 1;
    u.t.cSectors      = count;

    DebugOut(2, "\n%u, %u, %u\n", cyl, head, sec);
    DebugOut(2, "%u\n", count);

    rc = DosDevIOCtl(handle, 
		IOCTL_PHYSICALDISK, 
		PDSK_READPHYSTRACK,
		&u.t, parm_len, &parm_len,
		buffer, data_len, &data_len);

    if (rc != NO_ERROR)
    {
      DebugOut(2, "DosDevIOCtl error: return code = %u\n", rc);
      return ERROR_READ_DISK;
    } 
   
    return ERROR_NONE;
}
	
int write_disk(HFILE handle,          // disk handle
	      unsigned char  *buffer, // buffer 
	      unsigned long  lba,     // starting lba 
	      unsigned long nsec)     // number of sectors to write
{


    return ERROR_NONE;
}

int write_track(HFILE handle,          // Physical Disk handle
                unsigned char *buffer, // data buffer  
		unsigned short cyl,    // Starting Cylinder
                unsigned short head,   // Starting Head
                unsigned short sec,    // Starting Sector
                unsigned short count)  // Count of sectors to write
{


    return ERROR_NONE;
}

//	
// Get the disks geometry (cylinders, heads, sectors CHS)
//
int getDiskGeometry(unsigned int disk_no, DISK_GEOMETRY_ *diskGeometry)
{   
    static unsigned char *buf;
    DISK_GEOMETRY_ *pDiskGeometry; 
     
    buf = (unsigned char *) malloc(DISK_BLOCK_SIZE);
 
    if (ExtBiosdisk(PARA_CMD, disk_no, 0, 1, buf))		//Get the disks geometry
    {   free(buf);
    	return ERROR_CANNOT_READ_DISK_GEOMETRY;			//Return with error, if the BIOS can't get the disk's geometry
    }
    pDiskGeometry = (DISK_GEOMETRY_ *) buf;
    diskGeometry->cylinders = pDiskGeometry->cylinders;
    diskGeometry->heads = pDiskGeometry->heads;
    diskGeometry->sectors = pDiskGeometry->sectors;
    
    free(buf);
    return ERROR_NONE;
}


/*	
Scan the partition table
*/
int getPartitionTable(unsigned int disk_no, PARTITION_TABLE_ partitionTable[])
{
    int i, k, p, ptemp[8];
    static unsigned char *buf;
    char found = NONE;
    DISK_PARTITION_TABLE_ENTRY *pTEntry[4];
    unsigned int tableEntry=0;

    unsigned long lba=0, lbaExtended=0;

    buf = (unsigned char *) malloc(DISK_BLOCK_SIZE);

    if (ExtBiosdisk(READ_CMD, disk_no, lba, 1, buf))		//Read the Primary Partition Table from the disk, starting at lba=0
    {	partitionTable[tableEntry].partitionType=0;		//Mark end of table
	return ERROR_CANNOT_READ_PARTITON_TABLE;		//Return with error, if the BIOS can't read the partition table
    }

    pTEntry[0] = (DISK_PARTITION_TABLE_ENTRY *) & buf[446];
    pTEntry[1] = pTEntry[0] + 1;
    pTEntry[2] = pTEntry[0] + 2;
    pTEntry[3] = pTEntry[0] + 3;

    for (k = 0; k < 4; k++)					//Scan the Primary Partition Table and copy valid entries into return variable partitionTable[]
    {   p = k+1;
	if (pTEntry[k]->system != 0)
	{   partitionTable[tableEntry].partitionNumber = p;
  	    partitionTable[tableEntry].partitionType = pTEntry[k]->system;
	    partitionTable[tableEntry].bootable = pTEntry[k]->bootIndicator;
	    partitionTable[tableEntry].startLBA = pTEntry[k]->leadSectors+lba;
	    partitionTable[tableEntry].numberOfSectors = pTEntry[k]->numSectors;

	    DebugOut(2, ">>> Disk %3u Part.%2u Type %2X startLBA %10u  numSect %10u %s\n", 
		    disk_no, partitionTable[tableEntry].partitionNumber, partitionTable[tableEntry].partitionType, 
		    partitionTable[tableEntry].startLBA, partitionTable[tableEntry].numberOfSectors, partitionTable[tableEntry].bootable ? "bootable" : "");

	    tableEntry++;
	}
    }	
    for (k = 0; k < 4; k++)					//Scan the Primary Partition Table for an Extended Partition Table
    {						
        if ((pTEntry[k]->system == 0x05) || (pTEntry[k]->system == 0x55) || (pTEntry[k]->system == 0x0F) || (pTEntry[k]->system == 0x85))	/*0x05, 0x0F or 0x85 are the signatures for extended partitions */
	{   found = EXTENDED;
       
	    lbaExtended = pTEntry[k]->leadSectors;
	    lba=lbaExtended;
	 }
    } 
    
    if (found != EXTENDED)					//If no Extended Partition Table was found, return
    {   free(buf); 
        partitionTable[tableEntry].partitionType=0;		//Mark end of table  
	return ERROR_NONE;					//Return ok
    }

    p = 5;   							//Extended Partitions start at partition number 5 	
    {	do
	{   if (ExtBiosdisk(READ_CMD, disk_no, lba, 1, buf) & 0xff)//Read the Extended Partition Table from the disk starting at lba
	    {	partitionTable[tableEntry].partitionType=0;	//Mark end of table
		return ERROR_CANNOT_READ_PARTITON_TABLE;	//Return with error, if the BIOS can't read the partition table
	    }

	    pTEntry[0] = (DISK_PARTITION_TABLE_ENTRY *) & buf[446];
	    pTEntry[1] = pTEntry[0] + 1;
	    pTEntry[2] = pTEntry[0] + 2;
	    pTEntry[3] = pTEntry[0] + 3;
 
	    for (k = 0; k < 4; k++)				//Scan the Extended Partition Table and copy entries into return variable partitionTable[]
	    {	
		if (pTEntry[k]->system != 0 && pTEntry[k]->system != 5)
		{   partitionTable[tableEntry].partitionNumber = p++;
  	    	    partitionTable[tableEntry].partitionType = pTEntry[k]->system;
	    	    partitionTable[tableEntry].bootable = pTEntry[k]->bootIndicator;
	    	    partitionTable[tableEntry].startLBA = pTEntry[k]->leadSectors+lba;
	    	    partitionTable[tableEntry].numberOfSectors = pTEntry[k]->numSectors;

	    	    DebugOut(2, "<<< Disk %3u Part.%2u Type %2X startLBA %10u  numSect %10u %s\n",
		    	disk_no, partitionTable[tableEntry].partitionNumber, partitionTable[tableEntry].partitionType, 
		    	partitionTable[tableEntry].startLBA, partitionTable[tableEntry].numberOfSectors, partitionTable[tableEntry].bootable ? "bootable" : "");

	    	    tableEntry++;
		}
	    }
	    for (k = 0; k < 4; k++)				//Scan for another Extended Partition Table
	    {	if ((pTEntry[k]->system == 0x05) || (pTEntry[k]->system == 0x55) || (pTEntry[k]->system == 0x0F) || (pTEntry[k]->system == 0x85))
		{
	    	    lba = pTEntry[k]->leadSectors + lbaExtended;
		}
	    }
	}
	while ((pTEntry[1]->system != 0) && (buf[510] == 0x55) && (buf[511] == 0xAA));
    }
    free(buf);
    
    partitionTable[tableEntry].partitionType=0;			//Mark end of table  
    return 0;
}

int displayPartitionTable(void)
{   int i;
//    unsigned int disk_no;
    DISK_GEOMETRY_ diskGeometry;
    PARTITION_TABLE_ partitionTable[32];
    char *pTyp;
    int  haveDiskGeometry = 0;

//    printf("Here we are\n");
//    for (disk_no=128; disk_no < 135; disk_no++)
    {
#ifdef __DOS__
	isBiosExtensionInstalled=ExtBiosdisk(EXT_CHK, disk_no, 0, 0, NULL);
#endif

	//Get and display the harddisks geometry
    	if (getDiskGeometry(disk_no, &diskGeometry)==ERROR_NONE)
    	{   haveDiskGeometry = 1;
	}

	//Get and display the partition table
    	if (getPartitionTable(disk_no, partitionTable)!=ERROR_NONE)
	    return -1;

	if (haveDiskGeometry)
    	{   fprintf(STDOUT,"# LTOOLS infos ---------------------------------------------------------------\n");
	    if (strcmp(disk_name, LINUX_DISK)!=0)
	    	fprintf(STDOUT,"##### Disk %3u = %s : CHS=%4i:%4i:%4i\n", 0, disk_name,
    		    diskGeometry.cylinders, diskGeometry.heads, diskGeometry.sectors);
	    else
	    	fprintf(STDOUT,"##### Disk %3u = /dev/hd%c : CHS=%4i:%4i:%4i\n", disk_no, disk_no-128+'a',
    		    diskGeometry.cylinders, diskGeometry.heads, diskGeometry.sectors);
	} else
	{   fprintf(STDOUT,"# LTOOLS infos ---------------------------------------------------------------\n");
	    fprintf(STDOUT,"##### Disk %3u = %s : CHS=not available\n", disk_no, disk_name);
		//return -1; //continue;
	}
    
    	for (i=0; i < 32; i++)
	{   if (partitionTable[i].partitionType==0)
	    	break;

	    switch (partitionTable[i].partitionType)
	    {	case 0x01: 	pTyp = "FAT12      ";	break;
		case 0x04:    	pTyp = "FAT16      ";   break;
		case 0x05:   	pTyp = "DOS   ExPar";   break;
		case 0x06:    	pTyp = "FAT16 >32M ";   break;
		case 0x07:    	pTyp = "NTFS / HPFS";   break;
		case 0x0A:    	pTyp = "OS/2       ";   break;
		case 0x0B:
		case 0x0C:    	pTyp = "Win9x FAT32";   break;
		case 0x0E:    	pTyp = "Win9x FAT16";   break;
		case 0x0F:    	pTyp = "Win   ExPar";   break;
		case 0x80:
		case 0x81:    	pTyp = "Minix      ";   break;
		case 0x82:    	pTyp = "Linux Swap ";   break;
		case 0x83:    	pTyp = "Linux EXT2 ";   break;
		case 0x85:	pTyp = "Lin   ExPar";   break;
		case 0x8E:	pTyp = "Lin   LVM  ";   break;
		case 0xBE:	pTyp = "SolarisBoot";   break;
		case 0xBF:	pTyp = "Solaris    ";   break;
	    	default:	pTyp = "unknown    ";
	    }

	    fprintf(STDOUT,"# %2u Type:%s   %6ldMB  from start=%10lu  %s\n",
		       	partitionTable[i].partitionNumber, pTyp, partitionTable[i].numberOfSectors/(2*1024L),
		       	partitionTable[i].startLBA, partitionTable[i].bootable ? "bootable" : "");


	}
    }
    printf("\n");
    printf("*******************************************************\n");
    return 0;
}

/*########################################################################## */

/*This procedure looks for your Linux partition.
   You have to specify the disk to search in ldir.h or via the -s command
   line switch, specification is done in Linux 'style', your specification is
   converted into a DOS 'style' specification in global variables disk_no and
   part_no, eg.:

   /dev/hdaX           first harddisk        disk_no=0x80
   /dev/hdbX           second harddisk               0x81
   /dev/fd0            first floppy disk             0x00
   /dev/fd1            second floppy disk            0x01

   If you do not specify a partition number, i.e. if X is a space, the procedure
   will search the four entrys of the partition table for a primary Linux Ext2
   partition, if it finds one, it will use it. If it does not find a primary
   Linux Ext2 partition, it searches for a DOS extended partition, read's the
   DOS extended partition's table and searches it for a Linux Ext2 partition.
   As DOS extended partitions may contain an unlimited number of 'logical drives',
   this search is recursively until we find a Linux Ext2 partition or until we
   reach the end of the tables.

   If you additionally specify a partition number, i.e. set X to 5, for each
   Linux partition we find we also check, if it is the specified partition num-
   ber. If not, we continue the search of the partition table. The partition
   number is transfered to the procedure via global variable part_no.
 */

int examine_drive(void)
{   PARTITION_TABLE_ partitionTable[32];
    struct stat sbuf;
    int i;

    if (modus==LTEST)
    {	displayPartitionTable();
    	return (0);
    }

    /* fake version, don't bother with partitions now */
    DebugOut(2,"--------Executing 'examine drive' disk_name= %s  disk_no=%u  part_no=%u----------------\n", disk_name, disk_no, part_no);

    if(getPartitionTable(disk_no, partitionTable)!=ERROR_NONE)
    {
	fprintf(STDERR, "ERROR: Could not read the partition table %s\n",disk_name);
	return -1;
    }

    if (part_no==0)
    {	for (i=0; i<32; i++)
    	{   if (partitionTable[i].partitionType==0)
    	    	break;
    	    if ((partitionTable[i].partitionType==EXT2PART) || (partitionTable[i].partitionType==EXT2PARTNEW))
    	    {	part_no=partitionTable[i].partitionNumber;
    	        sprintf(disk_name,"%s%u",disk_name,part_no);
    	        break;
    	    }
    	}
    }

    DebugOut(2, "Trying to open disk_name=%s  disk_no=%u  part=%u  type=%X\n", disk_name, disk_no, part_no, partitionTable[i].partitionType);

    disk_fd = open(disk_name, O_RDWR);				/* !!! */
    if (disk_fd < 0)
    {
	fprintf(STDERR,"*could not open partition special file:");
	fprintf(STDERR,"tried %s\n", disk_name);
	return -1;
    }
    fstat(disk_fd, &sbuf);
    SECTORS = sbuf.st_size / DISK_BLOCK_SIZE;

    start = (long) 0;						/* start at 0 */
    num_sect = SECTORS;

    DebugOut(2,"Found drive with %d sectors\n",num_sect);
    return 0;

}

unsigned long readdisk(unsigned char *buf, unsigned long lba, unsigned short offset, unsigned long size)
{
    unsigned int no_sect = size / DISK_BLOCK_SIZE;
    int err, rc;
    char *temp;

    DebugOut(2,"--------Executing 'readdisk'---lba=%lu  offset=%u   size=%d\n",lba,offset,size);

    if ((offset > 0) || (size % DISK_BLOCK_SIZE))
	no_sect++;

    if ((temp = (unsigned char *) MALLOC(no_sect * DISK_BLOCK_SIZE)) == NULL)
    {
	fprintf(STDERR, "Memory allocation failed in readdisk\n");
	exit(1);
    }
    if (lseek64(disk_fd, ((long long) lba) * DISK_BLOCK_SIZE, SEEK_SET)==-1)
    {
	perror("readdisk: seek failed");
	exit(1);
    }
    rc = read(disk_fd, temp, no_sect * DISK_BLOCK_SIZE);

/* should do byteswapping here?!? */

    if (rc == no_sect * DISK_BLOCK_SIZE)
    {
	memcpy(buf, &temp[offset], size);			/* no ending NULL */
	FREE(temp);
	return size;
    } else
    {
	FREE(temp);
	return 0;
    }
}

unsigned long writedisk(unsigned char *buf, unsigned long lba, unsigned short offset, unsigned long size)
{
    unsigned int no_sect = size / DISK_BLOCK_SIZE;
    int err, rc;
    char *temp;

    DebugOut(2,"--------Executing 'writedisk'---lba=%lu  offset=%u   size=%d\n",lba,offset,size);

    if ((offset > 0) || (size % DISK_BLOCK_SIZE))
	no_sect++;

    if ((temp = (unsigned char *) MALLOC(no_sect * DISK_BLOCK_SIZE)) == NULL)
    {
	fprintf(STDERR, "Memory allocation failed in writedisk\n");
	exit(1);
    }
    if (lseek64(disk_fd, ((long long) lba) * DISK_BLOCK_SIZE, SEEK_SET)==-1)
    {
	perror("writedisk: seek failed");
	exit(1);
    }
    if (read(disk_fd, temp, no_sect * DISK_BLOCK_SIZE) != no_sect * DISK_BLOCK_SIZE)
    {
	fprintf(STDERR, "Read Error %xh in Write Disk\n", rc);
	exit(1);
    }
    memcpy(&temp[offset], buf, size);
    if (lseek64(disk_fd, ((long long) lba) * DISK_BLOCK_SIZE, SEEK_SET)==-1)
    {
	perror("writedisk: seek failed");
	exit(1);
    }
    rc = write(disk_fd, temp, no_sect * DISK_BLOCK_SIZE);

/* should do byteswapping here?!? */

    if (rc != no_sect * DISK_BLOCK_SIZE)
    {
	perror("writedisk: write failed:");
	FREE(temp);
	return 0;
    }
    FREE(temp);
    return size;
}
