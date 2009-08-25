/*ldir/lread
   Program to read Linux extended 2 filesystems under DOS

   Module readdisk.c
   Low level harddisk partition table and harddisk data read

   This is the operating system specific file for DOS harddisks.

   Copyright information and copying policy see file README.TXT

   History see file MAIN.C
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dos.h>
#include <io.h>
#include "ldir.h"
#include "ext2.h"
#include "proto.h"


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

/* globals */
unsigned int HEADS;						/*your harddisk's # of heads               ) drive's */
unsigned int SECTORS;						/*                # of sectors per cylinder) geometry */
unsigned int CYLINDERS;						/*                # of cylinders           )         */
unsigned long start=0;						/*logical block address(LBA) of your Linux partition */
unsigned long num_sect;						/*total # of sectors of your Linux partition */
extern unsigned int disk_no;					/*DOS' disk #, eg. 0x80=first harddisk */
extern unsigned int part_no;					/*# of your Linux partition */
extern char ext2_disk[256];					/*your linux partition name, eg. /dev/hda5 */
extern char ext2_image_file_name[256];				/*ext2 image file to be read */
extern int isOldStyleLba;
extern enum
{
    LDIR, LREAD, LWRITE, LTEST
}
modus;
extern FILE *STDOUT, *STDERR;
extern void *MALLOC(size_t size);
extern void FREE(void *block);
extern super_block sb;
extern int isUseBiosExtensions;
extern int isNoUseBiosExtensions;
extern int isWin;
int isBiosExtensionInstalled=0;

/*########################################################################## */
#if defined(_Windows) && !defined(__WIN32__)
#include <windows.h>

typedef struct
{
    unsigned long edi, esi, ebp, reserved, ebx, edx, ecx, eax;
    unsigned flags, es, ds, fs, gs, ip, cs, sp, ss;
}
RMODE_CALL;

/* Performs a real mode interrupt from protected mode
   routines dpmi_rmode_intr and real_int86x are 'stolen' from A.Schulman's
   Undocumented DOS */
#pragma warn -rvl
#pragma argsused
BOOL dpmi_rmode_intr(WORD intno, WORD flags, WORD copywords, RMODE_CALL far * rmode_call)
{
     _asm push di
     _asm push bx
     _asm push cx
     _asm mov ax, 0300 h					/* simulate real mode interrupt */
     _asm mov bx, intno						/* interrupt number, flags */
     _asm mov cx, copywords					/* words to copy from pmode to rmode stack */
     _asm les di, rmode_call					/* ES:DI = address of rmode call struct */
     _asm int 31 h						/* call DPMI */
     _asm jc error
     _asm mov ax, 1						/* return TRUE */
     _asm jmp short done
     error:
     _asm mov ax, 0						/* return FALSE */
     done:
     _asm pop cx
     _asm pop bx
     _asm pop di
}

int real_int86x(int intno, union REGS *inregs, union REGS *outregs, struct SREGS *sregs)
{
    RMODE_CALL r;

    memset(&r, 0, sizeof(r));					/* initialize all fields to zero: important! */
    r.edi = inregs->x.di;
    r.esi = inregs->x.si;
    r.ebx = inregs->x.bx;
    r.edx = inregs->x.dx;
    r.ecx = inregs->x.cx;
    r.eax = inregs->x.ax;
    r.flags = inregs->x.cflag;
    r.es = sregs->es;
    r.ds = sregs->ds;
    r.cs = sregs->cs;
    if (!dpmi_rmode_intr(intno, 0, 0, &r))
    {
	outregs->x.cflag = 1;					/* error: set carry flag! */
	return -1;
    }
    sregs->es = r.es;
    sregs->cs = r.cs;
    sregs->ss = r.ss;
    sregs->ds = r.ds;
    outregs->x.ax = r.eax;
    outregs->x.bx = r.ebx;
    outregs->x.cx = r.ecx;
    outregs->x.dx = r.edx;
    outregs->x.si = r.esi;
    outregs->x.di = r.edi;
    outregs->x.cflag = r.flags & 1;				/* carry flag */
    return outregs->x.ax;
}

int biosdisk(int cmd, int drive, int head, int cyl, int sector, int nsects, char far * buffer)
{
    DWORD lparam;
    int rmSegment, pmSelektor;
    union REGS regs;
    struct SREGS sregs;

    if (cmd == READ_CMD)
    {

	DebugOut(2,"---READ_CMD----\n");

	lparam = GlobalDosAlloc(DISK_BLOCK_SIZE * nsects);	/*allocate DOS-Memory */
	if (!lparam)
	{
	    fprintf(STDERR, "cannot allocate DOS memory\n");
	    exit(-1);
	}
	rmSegment = HIWORD(lparam);
	pmSelektor = LOWORD(lparam);
	sregs.cs = 0;
	sregs.ds = 0;
	sregs.es = rmSegment;
	regs.x.bx = 0;
	regs.h.dl = drive;
	regs.h.dh = head;
	regs.h.cl = (sector & 0x3F) + ((cyl >> 2) & 0xC0);
	regs.h.ch = cyl;
	regs.h.al = nsects;
	regs.h.ah = 0x02;
	real_int86x(0x13, &regs, &regs, &sregs);		/*Bios Disk Read Interrupt */
	if (regs.x.flags)
	{
	    GlobalDosFree(pmSelektor);				/*free DOS-Memory */
	    return -1;						/*Fehler */
	}
	_fmemcpy(buffer, MK_FP(pmSelektor, 0), DISK_BLOCK_SIZE * nsects);
	GlobalDosFree(pmSelektor);				/*free DOS-Memory */
	return (regs.x.ax) & 0xFF00;
    } else if (cmd == PARA_CMD)
    {

	DebugOut(2,"---PARA_CMD----\n");

	regs.h.ah = 0x08;
	regs.h.dl = drive;
	real_int86x(0x13, &regs, &regs, &sregs);		/*Bios Disk Read Interrupt */
	if (regs.x.flags)
	{
	    return -1;						/*Fehler */
	}
	buffer[0] = regs.h.cl;					/*no of sectors */
	buffer[1] = regs.h.ch;					/*no of cylinders */
	buffer[2] = regs.h.dl;					/*no of drives */
	buffer[3] = regs.h.dh;					/*no of heads */
	return (regs.x.ax) & 0xFF00;
    } else if (cmd == WRITE_CMD)
    {
	fprintf(STDERR,"Sorry, lwrite currently not supported under Windows GUI. Use DOS box\n");
    } else
	fprintf(STDERR,"Illegal command in function biosdisk()\n");

    return -1;
}
#endif								/*end #if defined(_Windows) && !defined(__WIN32__) */

/*########################################################################## */
#ifdef __WIN32__
#include <windows.h>
#include <winnt.h>
#include <winioctl.h>
#else
#include <bios.h>
#endif


int ExtBiosdisk(int cmd, int drive, unsigned long lba, int nsects, unsigned char far * buffer)
{ unsigned long lba2;
#ifdef __WIN32__
    HANDLE hPhysicalDrive;
    DWORD dwBytesRead;
    DISK_GEOMETRY diskGeo;
    unsigned long locLow, locHigh;
#else
    union REGS inregs, outregs;
    struct SREGS segregs;
    int rc;
#pragma option -a
    struct
    {   unsigned short mysize;
    	unsigned short flags;
    	unsigned long cylinders;
    	unsigned long heads;
    	unsigned long sectors;
    	unsigned long totalSectorsLow;
    	unsigned long totalSectorsHigh;
    	unsigned short bytesPerSector;
    	unsigned long configParams;
    } driveParams;
    struct
    {   unsigned char mysize;
    	unsigned char reserved;
    	unsigned short blockCount;
    	unsigned short bufAdrOffset;
    	unsigned short bufAdrSeg;
    	unsigned long  lbaLow;
    	unsigned long  lbaHigh;
    } driveAdrPacket;
#endif
    if (isBiosExtensionInstalled)
    {   if (cmd == PARA_CMD)
            cmd = EXT_PARA;
    	else if (cmd == READ_CMD)
    	    cmd = EXT_READ;
        else if (cmd == WRITE_CMD)
            cmd = EXT_WRITE;
    }

    DebugOut(2,"Calling ExtBiosdisk with cmd=%X   LBA=%ld\n",cmd,lba);

#ifdef __WIN32__
    switch (drive)
    {
	case 128:
	    hPhysicalDrive = CreateFile("\\\\.\\PhysicalDrive0",
					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					0, OPEN_EXISTING, 0, 0);
	    break;
	case 129:
	    hPhysicalDrive = CreateFile("\\\\.\\PhysicalDrive1",
					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					0, OPEN_EXISTING, 0, 0);
	    break;
	case 130:
	    hPhysicalDrive = CreateFile("\\\\.\\PhysicalDrive2",
					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					0, OPEN_EXISTING, 0, 0);
	    break;
	case 131:
	    hPhysicalDrive = CreateFile("\\\\.\\PhysicalDrive3",
					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					0, OPEN_EXISTING, 0, 0);
	    break;
	case 132:
	    hPhysicalDrive = CreateFile("\\\\.\\PhysicalDrive4",
					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					0, OPEN_EXISTING, 0, 0);
	    break;
	case 133:
	    hPhysicalDrive = CreateFile("\\\\.\\PhysicalDrive5",
					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					0, OPEN_EXISTING, 0, 0);
	    break;
	case 134:
	    hPhysicalDrive = CreateFile("\\\\.\\PhysicalDrive6",
					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					0, OPEN_EXISTING, 0, 0);
	    break;
	case 135:
	    hPhysicalDrive = CreateFile("\\\\.\\PhysicalDrive7",
					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					0, OPEN_EXISTING, 0, 0);
	    break;
	case 0:
	    hPhysicalDrive = CreateFile("\\\\.\\a:",
					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					0, OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, 0);
	    break;
	case 1:
	    hPhysicalDrive = CreateFile("\\\\.\\b:",
					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					0, OPEN_EXISTING,
					FILE_ATTRIBUTE_NORMAL | FILE_FLAG_RANDOM_ACCESS, 0);
	    break;
  	case 2: //Read an image file
	    hPhysicalDrive = CreateFile(ext2_image_file_name, 
	    				GENERIC_READ | GENERIC_WRITE, 
	    				FILE_SHARE_READ | FILE_SHARE_WRITE, 
	    				0, OPEN_EXISTING, 0, 0);
	    break;
	default:
	    return -1;
    }

    if (hPhysicalDrive == INVALID_HANDLE_VALUE)
    {	DebugOut(2,"---Error opening the drive # %d----\n", drive);
	return -1;
    }

    if (cmd == READ_CMD)					/*----------------------------------------------------- */
    {
	locLow = lba << 9;
	locHigh = lba >> 23;

	if (SetFilePointer(hPhysicalDrive, locLow, (PLONG) & locHigh, FILE_BEGIN) == 0xFFFFFFFF)
	{
	    fprintf(STDERR, "Seek failed---lba:%ld---locHigh:%ld--locLow=%ld\n", lba, locHigh, locLow);
	    CloseHandle(hPhysicalDrive);
	    return -1;
	}
	/* Read sector off of the drive... */
	if (!ReadFile(hPhysicalDrive, buffer, nsects * DISK_BLOCK_SIZE, &dwBytesRead, NULL))
	{
	    fprintf(STDERR, "Read failed\n");
	    CloseHandle(hPhysicalDrive);
	    return -1;
	}
	CloseHandle(hPhysicalDrive);
	return 0;
    } else if (cmd == WRITE_CMD)				/*--------------------------------------------- */
    {
	locLow = lba << 9;
	locHigh = lba >> 23;

	if (SetFilePointer(hPhysicalDrive, locLow, (PLONG) & locHigh, FILE_BEGIN) == 0xFFFFFFFF)
	{
	    fprintf(STDERR, "Seek failed---lba:%ld---locHigh:%ld--locLow=%ld\n", lba, locHigh, locLow);
	    CloseHandle(hPhysicalDrive);
	    return -1;
	}
	/* Write sector ... */
	if (!WriteFile(hPhysicalDrive, buffer, nsects * DISK_BLOCK_SIZE, &dwBytesRead, NULL))
	{
	    fprintf(STDERR, "Write failed\n");
	    CloseHandle(hPhysicalDrive);
	    return -1;
	}
	CloseHandle(hPhysicalDrive);
	return 0;
    } else if (cmd == PARA_CMD)					/*------------------------------------------------- */
    {
	if (!DeviceIoControl(hPhysicalDrive, IOCTL_DISK_GET_DRIVE_GEOMETRY, NULL, 0, &diskGeo, sizeof(diskGeo), &dwBytesRead, NULL))
	{
	    CloseHandle(hPhysicalDrive);
	    return -1;
	} else
	{
    	    CYLINDERS = *(UINT *) &diskGeo.Cylinders.QuadPart -1;
    	    HEADS = diskGeo.TracksPerCylinder;
    	    SECTORS = diskGeo.SectorsPerTrack;

	    CloseHandle(hPhysicalDrive);
	    return 0;
	}
    } else
	fprintf(STDERR, "Illegal command in function ExtBiosdisk()  %X\n",cmd);
    return -1;
#else
    if (cmd == EXT_CHK)						//installation check
    { 	inregs.h.ah = EXT_CHK;
    	inregs.x.bx = 0x55AA;
    	inregs.h.dl = drive;
    	int86x(0x13, &inregs, &outregs, &segregs);
    	if ((outregs.x.cflag==0) && (outregs.x.bx == 0xAA55) && (outregs.x.cx && 0x0001))
    	    return 0;						//is installed
    	else
    	    return 1;						//is not installed
    } else if (cmd == EXT_PARA)					//disk parameters
    {	driveParams.mysize = sizeof(driveParams);
        inregs.h.ah=EXT_PARA;
	inregs.h.dl=drive;
	segregs.ds=FP_SEG(&driveParams);
	inregs.x.si=FP_OFF(&driveParams);
    	int86x(0x13, &inregs, &outregs, &segregs);
  	if ((outregs.x.cflag == 0) && (outregs.h.ah == 0))
  	{
    	    CYLINDERS = driveParams.cylinders;
    	    HEADS = driveParams.heads;
    	    SECTORS = driveParams.sectors;

    	    if ((modus==LTEST) && (driveParams.flags & 0x02 == 0))
    	    	fprintf(STDOUT,"----- Bios Extension Drive Parameter CHS invalid-----\n");
    	    if ((modus==LTEST) &&  (driveParams.flags & 0x04))
    	    	fprintf(STDOUT,"----- Bios Extension Drive Parameter CHS set to maximum -----\n");
 	    return 0;
 	 } else
 	 {  return 1;
 	 }
    } else if (cmd == EXT_READ)					//Extended disk read
    {	driveAdrPacket.mysize = 16;
	driveAdrPacket.reserved=0;
        driveAdrPacket.blockCount =  nsects;
        driveAdrPacket.bufAdrOffset = FP_OFF(buffer);
        driveAdrPacket.bufAdrSeg = FP_SEG(buffer);
 	driveAdrPacket.lbaLow = lba;
	driveAdrPacket.lbaHigh = 0;

        inregs.h.ah=EXT_READ;
	inregs.h.dl=drive;
	segregs.ds=FP_SEG(&driveAdrPacket);
	inregs.x.si=FP_OFF(&driveAdrPacket);
    	int86x(0x13, &inregs, &outregs, &segregs);
  	if ((outregs.x.cflag == 0) && (outregs.h.ah == 0))
  	{   return 0;
	} else
	{   return 1;
	}
    } else if (cmd == EXT_WRITE)				//Extended disk write
    {	driveAdrPacket.mysize = 16;
	driveAdrPacket.reserved=0;
        driveAdrPacket.blockCount =  nsects;
        driveAdrPacket.bufAdrOffset = FP_OFF(buffer);
        driveAdrPacket.bufAdrSeg = FP_SEG(buffer);
 	driveAdrPacket.lbaLow = lba;
	driveAdrPacket.lbaHigh = 0;

        inregs.h.ah=EXT_WRITE;
        inregs.h.al=0;
	inregs.h.dl=drive;
	segregs.ds=FP_SEG(&driveAdrPacket);
	inregs.x.si=FP_OFF(&driveAdrPacket);
    	int86x(0x13, &inregs, &outregs, &segregs);
  	if ((outregs.x.cflag == 0) && (outregs.h.ah == 0))
  	{   return 0;
	} else
	{   return 1;
	}
    } else if (cmd == PARA_CMD)
    {   rc = biosdisk(cmd, drive, 0, 0, 1, nsects,  buffer);
    	HEADS = buffer[3] + 1;
    	CYLINDERS =  buffer[1] + ((unsigned char) buffer[0] >> 6) * 256 + 1;
    	SECTORS = buffer[0] & 0x3F;
	//printf("PARA_CMD  CHS=%d:%d:%d\n",CYLINDERS,HEADS,SECTORS);
      	return rc;
    } else
    {   int c,h,s;
    	c = lba / (SECTORS * HEADS);
    	h = (lba / SECTORS) % HEADS;
    	s = (lba % SECTORS) + 1;
	//printf("cmd=%d    CHS=%d:%d:%d   chs=%d:%d:%d   CHS=%d:%d:%d\n",cmd,cyl,head,sector,c,h,s,CYLINDERS,HEADS,SECTORS);

    	rc = biosdisk(cmd, drive, h, c, s, nsects,  buffer);
      	return rc;
    }
#endif

}
/*########################################################################## */


/*########################################################################## */


/*Convert Logical Block Number (LBA) adress to Cylinder-Head-Sector (CHS) address
   DOS and Linux software doesn't care about heads, sectors, cylinders, ie.
   your harddisk's hardware details and therefore works with LBA, but if you
   want to access your harddisk via the BIOS, you have to call the BIOS rou-
   tines with CHS addresses.

void convert(unsigned long x, unsigned short *head, unsigned short *cylinder, unsigned short *sector, unsigned short *offset)
{
    unsigned long logicsect, absolsect;


    DebugOut(2,"--------Executing 'convert'----------------\n");


    logicsect = x >> 9;
    *offset = x & 0x1FF;
    absolsect = logicsect + start;

    *cylinder = absolsect / (SECTORS * HEADS);
    *head = (absolsect / SECTORS) % HEADS;
    *sector = (absolsect % SECTORS) + 1;


    DebugOut(2,"Beginning at sect=%ld...\n", start);
    DebugOut(2,"byte=%4ld logic: %-4ld absol: %-4ld head: %-3d cylinder: %-4d sect: %-4d  offset: %-d\n",
	   x, logicsect, absolsect, *head, *cylinder, *sector, *offset);

    if (absolsect > start + num_sect)
    {
	fprintf(STDERR, "You cannot access ABOVE the Linux partition.\n");
	exit(-1);
    }
}
*/

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
{
    int i, j = 0, k, kfound, p = 1, ptemp[8];
    static unsigned char *buf;
    static char *pTyp;
    char found = NONE;
    partitionTableEntry *pTEntry[4];

    unsigned int head = 0, cyl = 0, sect = 1;

//  unsigned int first_head, first_sect, first_cyl;
//  unsigned int end_head, end_sect, end_cyl;

    unsigned int boot;
    unsigned int system;
    unsigned long lba=0, lbaExtended=0;

    DebugOut(2,"--------Executing 'examine drive'----------------\n");

    /* If we read from an image file, we just check, if it contains a
     * valid ext2 file system, i.e. if it has a valid superblock...
    */
    if (disk_no == 2)
	return load_super();

    buf = (unsigned char *) MALLOC(DISK_BLOCK_SIZE);

#if !defined(__WIN32__)
    if (/*isWin &&*/ ExtBiosdisk(EXT_CHK, disk_no, 0, 1, buf)==0) /*Check, if the BIOS extensions are installed*/
    	isBiosExtensionInstalled=1;

    if (isUseBiosExtensions)			//Used to overwrite detection via command line switch
    	isBiosExtensionInstalled=1;
    else if (isNoUseBiosExtensions)
    	isBiosExtensionInstalled=0;
    DebugOut(2,"Bios Int 13h extensions %s\n",isBiosExtensionInstalled?"used":"not used");

#endif
    if (ExtBiosdisk(PARA_CMD, disk_no, 0, 1, buf))	/*let BIOS tell us the drive's geometry */
    {   if (modus != LTEST)
	    fprintf(STDERR, "Could not get drive's geometry - exiting\n");
	return (-1);
    }

    DebugOut(2,"Search for %s   disk=%i  part=%i  heads=%i   sectors=%i   cylinders=%i\n",
	   ext2_disk, disk_no, part_no, HEADS, SECTORS, CYLINDERS);

    if (modus == LTEST)
    {	fprintf(STDOUT,"# LTOOLS infos ---------------------------------------------------------------\n");
    	fprintf(STDOUT,"##### Disk %3u = /dev/hd%c : CHS=%4i:%4i:%4i\n", disk_no, disk_no-128+'a', CYLINDERS, HEADS, SECTORS);
#if !defined(__WIN32__)
    	fprintf(STDOUT,"----- Bios Int 13h extensions %s --- %s mode\n",isBiosExtensionInstalled?"used":"not used", isOldStyleLba?"CHS":"LBA");
#endif
    }

    if ((disk_no == 0) || (disk_no == 1))	/*Is it a floppy drive? 0=A:  1=B: */
    {
	start = (long) 0;					/*floppy's do not have partitions */
	SECTORS = SECTORS;					/*all data starts at LBA 0 */
	num_sect = ((unsigned long) HEADS) * ((unsigned long) SECTORS) * ((unsigned long) CYLINDERS);

	DebugOut(2,"heads=%u  sectors=%u  cylinders=%u num_sect:%lu\n", HEADS, SECTORS, CYLINDERS, num_sect);

	FREE(buf);
	return 0;
    }
    /* read the partition table from the first hard drive, partition table starts at lba=0 == CHS=0 0 1 */
    if (isOldStyleLba)
	lba = (long) sect - 1L + (long) head * (long) SECTORS + (long) cyl * (long) HEADS * (long) SECTORS;
    else
    	lba = 0;

    if (ExtBiosdisk(READ_CMD, disk_no, lba, 1, buf)) 
    {
	fprintf(STDERR, "Error Reading Partition Table on %s\n", ext2_disk);
	fprintf(STDERR, "Use command line switch -s/dev/hdx[y] , where x=a,b,... is the\n");
	fprintf(STDERR, "Linux harddisk and y=1,2,... (may be omitted) is your partition number\n");
	fprintf(STDERR, "Example: ldir -s/dev/hdb6\n");
	fprintf(STDERR, "To avoid this message, use command 'ldrive' or edit ldir.h and recompile \n(see README.TXT)\n");
	fprintf(STDERR, "If you use a mix of IDE and SCSI disks, you definitely should read README.TXT\n");
	exit(-1);						/*exit, if the BIOS can't read the partition table */
    }

    pTEntry[0] = (partitionTableEntry *) & buf[446];
    pTEntry[1] = pTEntry[0] + 1;
    pTEntry[2] = pTEntry[0] + 2;
    pTEntry[3] = pTEntry[0] + 3;

//??DebugOut(2,"------------------------\nPrimary Partition Table at Head:%i  Cylinder:%i   Sector:%i Partition:1-4 \n", head, cyl, sect);
    DebugOut(2,"------------------------\nPrimary Partition Table at lba=%u (CHS=%u:%u:%u) Partition:1-4 \n", lba, cyl, head, sect);
    for (k=0; k < 4; k++)
    {	DebugOut(2,"# %2u Type:%2X   %4ldMB from CHS=%3d:%3d:%3d to CHS=%3d:%3d:%2d %s\n                                 secs=%lu     start=%lu\n",
		       k, pTEntry[k]->system, pTEntry[k]->numSectors / (2 * 1024),
	       	       pTEntry[k]->startCylinder+((pTEntry[k]->startSector>>6)*256), pTEntry[k]->startHead, pTEntry[k]->startSector&0x3F,
	               pTEntry[k]->endCylinder+((pTEntry[k]->endSector>>6)*256), pTEntry[k]->endHead, pTEntry[k]->endSector&0x3F,
	       	       pTEntry[k]->bootIndicator ? "bootable" : "",pTEntry[k]->numSectors,pTEntry[k]->leadSectors);
    }
    DebugOut(2,"----------------------------------------------------------------------------------\n");


    for (k = 0; k < 4; k++)					/*Search the Primary Partition Table for a Linux partition */
    {
	if (isOldStyleLba)
	   pTEntry[k]->leadSectors+=(long) sect + ((long) head) * SECTORS + ((long) cyl) * (HEADS * SECTORS) - 1;

	if ((pTEntry[k]->system == EXT2PART) || (pTEntry[k]->system == EXT2PARTNEW))	/*EXT2PART=0x83 is the Linux Ext2 partition signature */
	{

	    DebugOut(2,"Linux Partition Found        at Entry#=%i Part#=%i  ", k, k + 1);

	    if ((part_no == 0) || (k == part_no - 1))		/*we found a Linux partition, but does it have the correct partition number? */
	    {
		found = LINUX;
		kfound = k;					/*yes */

		DebugOut(2,"->it's the right one\n");

		if (modus != LTEST)
		    break;
	    }
	    ptemp[j++] = k + 1;					/*no */

	    if (modus != LTEST) DebugOut(2,"->it's NOT the right one\n");

	}
    }

    if (modus == LTEST)
    {
	for (k = 0; k < 4; k++)
	{
	    switch (pTEntry[k]->system)
	    {
		case 0x01:
                    pTyp = "FAT12      ";
		    break;
		case 0x04:
		    pTyp = "FAT16      ";
		    break;
		case 0x05:
		    pTyp = "DOS   ExPar";
		    break;
		case 0x06:
		    pTyp = "FAT16 >32M ";
		    break;
		case 0x07:
		    pTyp = "NTFS / HPFS";
		    break;
		case 0x0A:
		    pTyp = "OS/2       ";
		    break;
		case 0x0B:
		case 0x0C:
		    pTyp = "Win9x FAT32";
		    break;
		case 0x0E:
		    pTyp = "Win9x FAT16";
		    break;
		case 0x0F:
		    pTyp = "Win   ExPar";
		    break;
		case 0x80:
		case 0x81:
		    pTyp = "Minix      ";
		    break;
		case 0x82:
		    pTyp = "Linux Swap ";
		    break;
		case 0x83:
		    pTyp = "Linux EXT2 ";
		    break;
		case 0x85:
	     	    pTyp = "Lin   ExPar";
	     	    break;
		case 0x8E:
	     	    pTyp = "Lin   LVM  ";
	     	    break;     	    
		case 0xBE:
	     	    pTyp = "SolarisBoot";
	     	    break;
		case 0xBF:
	     	    pTyp = "Solaris    ";
	     	    break;
		default:
		    itoa(pTEntry[k]->system, (char *) buf, 16);
		    pTyp = (char *) buf;
	    }
	    if ((pTEntry[k]->system)) // && (pTEntry[k]->system != 0x05) && (pTEntry[k]->system != 0x0F) && (pTEntry[k]->system != 0x85))
		fprintf(STDOUT,"# %2u Type:%s   %4ldMB from CHS=%3d:%3d:%3d to CHS=%3d:%3d:%2d %s\n                                 secs=%lu     start=%lu\n",
		       k+1, pTyp, pTEntry[k]->numSectors / (2 * 1024),
		       pTEntry[k]->startCylinder+((pTEntry[k]->startSector>>6)*256), pTEntry[k]->startHead, pTEntry[k]->startSector&0x3F,
		       pTEntry[k]->endCylinder+((pTEntry[k]->endSector>>6)*256), pTEntry[k]->endHead, pTEntry[k]->endSector&0x3F,
		       pTEntry[k]->bootIndicator ? "bootable" : "",pTEntry[k]->numSectors,pTEntry[k]->leadSectors);
	}
	if (modus == LTEST)
	    found = NONE;
    }
    if (found != LINUX)						/*We haven't found a primary Linux partition, so we search for an extended partition */
    {   found = NONE;
	for (k = 0; k < 4; k++)
	    if ((pTEntry[k]->system == 0x05) || (pTEntry[k]->system == 0x55) || (pTEntry[k]->system == 0x0F) || (pTEntry[k]->system == 0x85))	/*0x05, 0x0F or 0x85 are the signatures for extended partitions */
	    {	if (found == EXTENDED)
	    	    fprintf(STDOUT,"Warning: There is more than one extended partition - ldir may not work correctly\n");
		found = EXTENDED;
		p = 5;						/*Extended partitions start at /dev/hd?5 */
		kfound = k;
	        if (isOldStyleLba)
	    	{
	    	    head = pTEntry[kfound]->startHead;			/*find out the CHS-address of the extended partition's partition table */
	    	    sect = pTEntry[kfound]->startSector;
	    	    cyl = pTEntry[kfound]->startCylinder;
	    	    cyl += (sect >> 6) * 256;				/* add top 2 bits of first_sect */
	    	    sect = sect & 0x3F;					/* and remove them from first_sect */

	    	    lba = (long) sect - 1L + (long) head * (long) SECTORS + (long) cyl * (long) HEADS * (long) SECTORS;
	    	    lbaExtended = lba;
		} else
		{   lbaExtended = pTEntry[k]->leadSectors;
		    lba=lbaExtended;
		}
//		printf("1: lba=%lu  lbaExtended=%lu\n", lba, lbaExtended);
	    }
    }
    if (found == EXTENDED)					/*Searching extended partition table */
	do
	{
	    if (ExtBiosdisk(READ_CMD, disk_no, lba, 1, buf) & 0xff)	/*read it */
	    {
		fprintf(STDERR, "Error Reading Partition Table\n");
		exit(-1);
	    }

	    pTEntry[0] = (partitionTableEntry *) & buf[446];
	    pTEntry[1] = pTEntry[0] + 1;
	    pTEntry[2] = pTEntry[0] + 2;
	    pTEntry[3] = pTEntry[0] + 3;

//???	    DebugOut(2,"------------------------\nSecondary Partition Table at Head:%i  Cylinder:%i   Sector:%i Partition:%i \n", head, cyl, sect, p);
	    DebugOut(2,"------------------------\nSecondary Partition Table at lba=%lu  (CHS=%u:%u:%u) Partition:%i \n", lba, cyl, head, sect, p);
    	    for (k=0; k < 4; k++)
	    {	DebugOut(2,"# %2u Type:%X   %4ldMB from CHS=%3d:%3d:%3d to CHS=%3d:%3d:%2d %s\n                                 secs=%lu     start=%lu\n",
			       k, pTEntry[k]->system, pTEntry[k]->numSectors / (2 * 1024),
		       	       pTEntry[k]->startCylinder+((pTEntry[k]->startSector>>6)*256), pTEntry[k]->startHead, pTEntry[k]->startSector&0x3F,
		               pTEntry[k]->endCylinder+((pTEntry[k]->endSector>>6)*256), pTEntry[k]->endHead, pTEntry[k]->endSector&0x3F,
		       	       pTEntry[k]->bootIndicator ? "bootable" : "",pTEntry[k]->numSectors,pTEntry[k]->leadSectors+lbaExtended);
	    }
	    DebugOut(2,"----------------------------------------------------------------------------------\n");

	    for (k = 0; k < 4; k++)				/*Check extended Partition Table, same game as with the primary table */
	    {
		if (isOldStyleLba)
		    pTEntry[k]->leadSectors+=(long) sect + ((long) head) * SECTORS + ((long) cyl) * (HEADS * SECTORS) - 1;

		if (modus == LTEST)
		{
		    switch (pTEntry[k]->system)
		    {
			case 0x01:
			    pTyp = "FAT12      ";
			    break;
			case 0x04:
			    pTyp = "FAT16      ";
			    break;
			case 0x05:
		     	    pTyp = "DOS   ExPar";
		     	    break;
			case 0x06:
			    pTyp = "FAT16 >32M ";
			    break;
			case 0x07:
			    pTyp = "NTFS / HPFS";
			    break;
			case 0x0A:
			    pTyp = "OS/2       ";
			    break;
			case 0x0B:
			case 0x0C:
			    pTyp = "Win9x FAT32";
			    break;
			case 0x0E:
			    pTyp = "Win9x FAT16";
			    break;
			case 0x0F:
		     	    pTyp = "Win   ExPar";
		     	    break;
			case 0x80:
			case 0x81:
			    pTyp = "Minix      ";
			    break;
			case 0x82:
			    pTyp = "Linux Swap ";
			    break;
			case 0x83:
			    pTyp = "Linux EXT2 ";
			    break;
			case 0x85:
		     	    pTyp = "Lin   ExPar";
		     	    break;
			case 0x8E:
	     	    	    pTyp = "Lin   LVM  ";
	     	    	    break;     	    
			default:
			    itoa(pTEntry[k]->system, (char *) buf, 16);
			    pTyp = (char *) buf;
		    }
    	            if ((pTEntry[k]->system) && (pTEntry[k]->system != 0x05) && (pTEntry[k]->system != 0x0F) && (pTEntry[k]->system != 0x85))
			fprintf(STDOUT,"# %2u Type:%s   %4ldMB from CHS=%3d:%3d:%3d to CHS=%3d:%3d:%2d %s\n                                 secs=%lu     start=%lu\n",
			       p, pTyp, pTEntry[k]->numSectors / (2 * 1024),
		       	       pTEntry[k]->startCylinder+((pTEntry[k]->startSector>>6)*256), pTEntry[k]->startHead, pTEntry[k]->startSector&0x3F,
		               pTEntry[k]->endCylinder+((pTEntry[k]->endSector>>6)*256), pTEntry[k]->endHead, pTEntry[k]->endSector&0x3F,
		       	       pTEntry[k]->bootIndicator ? "bootable" : "",pTEntry[k]->numSectors,pTEntry[k]->leadSectors+lba);
		}

		if ((pTEntry[k]->system == EXT2PART) || (pTEntry[k]->system == EXT2PARTNEW))		/*hurrah, it's Linux */
		{

		    DebugOut(2,"Linux Partition Found        at Entry#=%i Part#=%i  ", k, p);

		    if ((part_no == 0) || (p == part_no))	/*does it have the correct partition number? */
		    {
			found = LINUX;
			kfound = k;				/*yes */

			DebugOut(2,"->it's the right one\n");

			if (modus != LTEST)
			    break;
		    }
		    ptemp[j++] = p;				/*no */

		    if (modus != LTEST) DebugOut(2,"->it's NOT the right one\n");

		}
		if ((pTEntry[k]->system == 0x05) || (pTEntry[k]->system == 0x55) || (pTEntry[k]->system == 0x0F) || (pTEntry[k]->system == 0x85))
		{
		    found = EXTENDED;				/*hurrah, another extended partion */
		    kfound = k;
		    p++;
	    	    if (isOldStyleLba)
	    	    {
	    		head = pTEntry[kfound]->startHead;			/*find out the CHS-address of the extended partition's partition table */
	    		sect = pTEntry[kfound]->startSector;
	    		cyl = pTEntry[kfound]->startCylinder;
	    		cyl += (sect >> 6) * 256;				/* add top 2 bits of first_sect */
	    		sect = sect & 0x3F;					/* and remove them from first_sect */

	    		lba = (long) sect - 1L + (long) head * (long) SECTORS + (long) cyl * (long) HEADS * (long) SECTORS;
	    	    } else
	    	    {	lba = pTEntry[k]->leadSectors+lbaExtended;
	    	    }

		    DebugOut(2,"DOS Extended Partition Found at Entry#=%i Part#=%i   %lu\n", kfound, p, pTEntry[k]->leadSectors+lba);

		}
	    }
	}
	while ((pTEntry[1]->system != 0) && (found != LINUX) &&
	       (buf[510] == 0x55) && (buf[511] == 0xAA));

/*our search continues until either -we found the last entry which's signature is 0
   or     -we found the right Linux Ext2 partition
   or     -the sector is not a partition table (signature 0xAA55
 */
    if (found != LINUX)						/*if we finally did not find the Linux Ext2 partition we exit */
    {
	if (modus != LTEST)
	    fprintf(STDERR, "Could not find Linux ext2/3 partition on %s\n", ext2_disk);
	else
	    return (0);
	if (j > 0)
	    for (i = 0; i < j; i++)
		fprintf(STDERR, "Found Linux at partition number %i\n", ptemp[i]);
	fprintf(STDERR, "\nUse command line switch -s/dev/hdx[y] , where x=a,b,... is the\n");
	fprintf(STDERR, "Linux harddisk and y=1,2,... (may be omitted) is your partition number\n");
	fprintf(STDERR, "Example: ldir -s/dev/hdb6\n\n");
	fprintf(STDERR, "To avoid this message, use command 'ldrive' or edit ldir.h and recompile \n(see README.TXT)\n");
	fprintf(STDERR, "If you use a mix of IDE and SCSI disks, you definitely should read README.TXT\n");
	return(-1);
    }
    /*if we reach this point, we've found the right Linux Ext2 partition */

    if (isOldStyleLba)
    {  	head = pTEntry[kfound]->startHead;			/*get the partitions CHS address from the partition table */
       	sect = pTEntry[kfound]->startSector;
  	cyl = pTEntry[kfound]->startCylinder;
  	cyl += (sect >> 6) * 256;				/* add top 2 bits of first_sect */
  	sect = sect & 0x3F;					/* and remove them from first_sect */
  	start = (long) sect + ((long) head) * SECTORS + ((long) cyl) * (HEADS * SECTORS) - 1;
    } else
    {	start = pTEntry[kfound]->leadSectors+lba;
    }

    num_sect = pTEntry[kfound]->numSectors;

//  end_head = pTEntry[kfound]->endHead;
//  end_sect = pTEntry[kfound]->endSector;
//  end_cyl = pTEntry[kfound]->endCylinder;
//  end_cyl += (end_sect >> 6) * 256;				/* add top 2 bits of end_sect */
//  end_sect = end_sect & 0x3F;					/* and remove them from end_sect */
    boot = pTEntry[kfound]->bootIndicator;
    system = pTEntry[kfound]->system;

    /* Brag about the results */

    DebugOut(2,"\n      THE DRIVE HAS:\n");
    DebugOut(2,"    number of heads: %u\n", HEADS);
    DebugOut(2,"  number of sectors: %u\n", SECTORS);
    DebugOut(2,"number of cylinders: %u\n", CYLINDERS);
    DebugOut(2,"\n   LINUX ITSELF HAS:\n");
    DebugOut(2,"     boot indicator: %xH\n", boot);
//  DebugOut(2,"     beginning head: %u\n", first_head);
//  DebugOut(2,"   beginning sector: %u\n", first_sect);
//  DebugOut(2," beginning cylinder: %u\n", first_cyl);
    DebugOut(2,"   system indicator: %xH\n", system);
//  DebugOut(2,"        ending head: %u\n", end_head);
//  DebugOut(2,"      ending sector: %u\n", end_sect);
//  DebugOut(2,"    ending cylinder: %u\n", end_cyl);
    DebugOut(2,"         start sect: %lu\n", start);
    DebugOut(2,"  number of sectors: %lu\n", num_sect);

    FREE(buf);
    return 0;
}


unsigned long readdisk(unsigned char *buf, unsigned long lba, unsigned short offset, unsigned long size)
{
//  unsigned short head, cyl, sect, offset;
    unsigned short rc, x;
    unsigned short no_sect;
    static unsigned char *temp = NULL;


    DebugOut(2,"--------Executing 'readdisk'----size:%lu--lba:%lu--offset:%u\n", size, lba, offset);

     /*With floppys there are problems when trying to read beyond the last sector
       of a cylinder. So we try to deal with that. I never had this problem with
       harddisks, most likely, because my harddisks work in lba-mode, translating
       heads/sector/cylinder on their own. If we get read errors (rd!=0), the
       following piece of code needs to be improved
     */
    no_sect = size / DISK_BLOCK_SIZE;
    if ((offset != 0) || (size % DISK_BLOCK_SIZE))
	no_sect++;
    if (no_sect < 2)
	if (disk_no < 128)
	    no_sect = 1;					/*with floppys: minimum read is 1 sector */
	else
	    no_sect = 2;					/*with harddisks:               2 sectors */

    if ((temp = (unsigned char *) MALLOC(no_sect * DISK_BLOCK_SIZE)) == NULL)
    {
	fprintf(STDERR, "Memory allocation failed in readdisk\n");
    }

    DebugOut(2,"--------Reading----disk_no:%u  nsects=%u  lba:%ld  offset:%u\n",disk_no, no_sect, lba + start,offset);

    for (x = 0; x < 3; x++)					/*for floppy disks we sometimes have to try more than once */
    {
	if ((rc = ExtBiosdisk(READ_CMD, (int) disk_no, lba + start, (int) no_sect, (void *) temp)) == 0)
	    break;
    }
    if (rc == 0)
    {
	memcpy(buf, &temp[offset], size);			/* no ending NULL */
	FREE(temp);
	return size;
    } else
    {
	fprintf(STDERR, "Read Error %xh in readdisk - lba=%ld offset=%d  no_sect:%u\n", rc, lba+start, offset, no_sect);
	FREE(temp);
	return 0;
    }
}

unsigned long writedisk(unsigned char *buf, unsigned long lba, unsigned short offset, unsigned long size)
{
//  unsigned short head, cyl, sect, offset;
    unsigned short rc, x;
    unsigned short no_sect;
    static unsigned char *temp = NULL;


    DebugOut(2,"--------Executing 'writedisk'----size:%lu--lba:%lu--offset:%u\n", size, lba,offset);

    no_sect = size / DISK_BLOCK_SIZE;
    if ((offset != 0) || (size % DISK_BLOCK_SIZE))
	no_sect++;
    if (no_sect < 2)
	if (disk_no < 128)
	    no_sect = 1;					/*with floppys: minimum read is 1 sector */
	else
	    no_sect = 2;					/*with harddisks:               2 sectors */

    if ((temp = (unsigned char *) MALLOC(no_sect * DISK_BLOCK_SIZE)) == NULL)
    {
	fprintf(STDERR, "Memory allocation failed in writedisk\n");
	return -1;
    }
    /*we first have to read from disk and fill our temporary buffer */
    for (x = 0; x < 3; x++)					/*for floppy disks we sometimes have to try more than once */
    {
	if ((rc = ExtBiosdisk(READ_CMD, disk_no, lba + start, no_sect, temp)) == 0)
	    break;
    }
    if (rc)
	fprintf(STDERR, "Read Error %xh in writedisk\n", rc);

    /*now we copy our own stuff into the temporary buffer */
    memcpy(&temp[offset], buf, size);				/* no ending NULL */

    /*now we can write to disk */
    for (x = 0; x < 3; x++)					/*for floppy disks we sometimes have to try more than once */
    {
	if ((rc = ExtBiosdisk(WRITE_CMD, disk_no, lba + start, no_sect, temp)) == 0)
	    break;
    }
    if (rc == 0)
    {
	rc = size;
    } else if (rc == 3)
    {
	fprintf(STDERR, "Write Error %xh in writedisk - Disk is write protected !\nLwrite does not work in a DOS box under Windows 9x\n", rc);
	exit(-1);
    } else
    {
	fprintf(STDERR, "Write Error %xh in writedisk - lba=%ld  no_sect:%u\n", rc, lba + start, no_sect);
	rc = 0;
    }

    FREE(temp);

    return rc;

}

#if !defined(__WIN32__)
int dos_long_creat(char *filename)
{
    int fd;
    union REGS regs;
    struct SREGS sregs;

    /* initially, indicate that LFN functions aren't supported */
    regs.x.cflag = 1;
    regs.x.ax = 0x7100;

   /* if this is a modern DOS version, try using long filenames */
    if (_osmajor >= 6)
    {
	regs.x.ax = 0x716C;                                     /* possible LFN support */
	regs.x.bx = 0x0002;
	regs.x.cx = 0x0000;
	regs.x.dx = 0x0012;
	regs.x.si = FP_OFF(filename);
	sregs.ds = FP_SEG(filename);
	sregs.es = 0;
	fd = intdosx(&regs, &regs, &sregs);
    }

    /* if long filenames aren't supported, use short filename instead */
    if (regs.x.cflag && (regs.x.ax == 0x7100))	
    {
	if (_osmajor >= 7)
	{
	    regs.x.ax = 0x6C00;					/* Dos 7.0+ */
	    regs.x.bx = 0x0002;
	    regs.x.dx = 0x0012;
	    regs.x.si = FP_OFF(filename);
	}
	else
	{
	    regs.x.ax = 0x3C00;					/* Dos < 7.0 */
	    regs.x.dx = FP_OFF(filename);
	}
	regs.x.cx = 0x0000;
	sregs.ds = FP_SEG(filename);
	sregs.es = 0;
	fd = intdosx(&regs, &regs, &sregs);
    }

/*
    if (_osmajor > 6)
    {
	regs.x.ax = 0x1600;					/* is Windows 95/98 running? */
	int86(0x2F, &regs, &regs);
	if (regs.h.al)
	{
	    regs.x.ax = 0x716C;					/* if yes, long filenames are supported */
	} else
	{
	    regs.x.ax = 0x6C00;					/* Dos 7.0 */
	}
    } else
	regs.x.ax = 0x6C00;					/* Dos before 7.0 */
    regs.x.bx = 0x0002;
    regs.x.cx = 0x0000;
    regs.x.dx = 0x0012;
    regs.x.si = FP_OFF(filename);
    sregs.ds = FP_SEG(filename);
    sregs.es = 0;

    fd = intdosx(&regs, &regs, &sregs);
*/

    /* if carry flag is set, there was an error */
    if (regs.x.cflag)
    {   fprintf(STDERR, "Error creating filename\n");
	return (-1);
    } else
	return (fd);
}

/* Lock a disk volume (needed under Windows 95)
   mode = 0   unlock
   mode = 1   lock
 */
int lockDrive(char mode)
{
    union REGS regs;
    short argdx = 0x1;

    if (disk_no < 128)
	return 0;						/*floppy drives do not need locking */

    if (_osmajor < 7)
	return 0;						/*is not DOS 7 or later */

    regs.x.ax = 0x1600;
    int86(0x2F, &regs, &regs);
    if (regs.h.al == 0)						/*is it a DOS box? */
	return 0;						/*->if no, no locking required */

    if (mode == 1)						/*lock disk level 1 */
    {
        if ((ioctl(0x0100 | disk_no, 0x0D, &argdx, 0x084B)) != 0x440D)     /*lock:         int 21h, ax=440Dh, cx=084Bh */
	{
            fprintf(STDERR, "Volume locking failed - LWRITE does not work in a Windows 9x DOS-box\n");
	    exit(-1);
	}
    } else							/*unlock level disk level 1 */
    {
        if ((ioctl(0x0000 | disk_no, 0x0D, &argdx, 0x086B)) != -1)         /*unlock:       int 21h, ax=440Dh, cx=086Bh */
	{
            fprintf(STDERR, "Volume unlocking failed - LWRITE does not work in a Windows 9x DOS-box\n");
	    exit(-1);
	}
    }
    return 0;
}
#endif
