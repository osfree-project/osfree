#include <stdio.h>  /* all kinds of things, file stream i/o */
#include <dos.h>

#include <stdlib.h> /* exit */
#include <bios.h>   /* biosdisk */
#include <mem.h>    /* memcmp */
#include <dir.h>    /* getdisk */
#include <io.h>     /* read/write with simple "unstreamed" handles */

#include "md5.h"
#include "global.h"
#include "drive.h"

/*
	needed for bios calls
*/

enum boolean usebios, checkonlyoneside, checkonlyeightsectors, checkonlyfourtytracks;
unsigned short headcount, trackcount, secpertrackcount;

char numbuf[16];	/* scratch space for itoa() and ultoa() */

#define POOLSIZE 63 /* 2 times 42 sectors? Too big... */
unsigned char bufferpool[POOLSIZE * 512];

unsigned char * sectorbuffer;

unsigned char * ensureDMA(unsigned int howbig)
{
	unsigned long bstart;
	unsigned long bend;
	static unsigned int theoffset = 0xffff; /* force initial change */
	static unsigned int thesize = 0;
	int didchange = 0;  /* if ... changed, display current settings */

	if (thesize >= howbig)
		return (&bufferpool[theoffset]);
		/* we already knew a big enough buffer */

	if (theoffset == 0xffff) {
		theoffset = 0;
		didchange = 1;
	}

	while (1) {

		if ((theoffset+howbig) > sizeof(bufferpool)) {
			PutString("Cannot allocate ");
			PutString(ultoa(howbig, numbuf, 10));
			PutString(" bytes of DMA buffer.\r\n");
			exit(-1);
		}

		bstart = FP_SEG(&bufferpool[theoffset]);
		bstart <<= 4;
		bstart += FP_OFF(&bufferpool[theoffset]);
		
		bend = FP_SEG(&bufferpool[theoffset+howbig]);
		bend <<= 4;
		bend += FP_OFF(&bufferpool[theoffset+howbig]);
		
		if ((bstart & 0xffff0000L) != (bend & 0xffff0000L)) {
			/* oops, DMA boundary crossed. Change that. */
			theoffset += 512;
			/* fast search but small enough steps */
			didchange = 1;
		} else {
			if (didchange) {
				PutString(ultoa(howbig, numbuf, 10));
				PutString(" bytes of disk buffer at 0x");
				PutString(ultoa(bstart, numbuf, 16));
				PutString(" (");
				PutString(ultoa(FP_SEG(&bufferpool[theoffset]), numbuf, 16));
				PutString(":");
				PutString(ultoa(FP_OFF(&bufferpool[theoffset]), numbuf, 16));
				PutString(")\r\n");
			} else {
				PutString("Buffer size increased to ");
				PutString(ultoa(howbig, numbuf, 10));
				PutString(" bytes now.\r\n");
			}

			thesize = howbig; /* buffer size has grown */
			return (&bufferpool[theoffset]);
		}
	}
}


void drivelettercheck(short *drive)
{
	short i;
	/* check if drive given */
	if (drive[0] == 0)
	{
		drive[0]=getdisk()+'A';
		drive[1]=drive[0];
	}

	if (drive[1] == 0)
		drive[1] = drive[0];

	/* change the drive from literal format to int */
	for (i=0;i<=1;i++) {
		if ((drive[i] >= 'a') && (drive[i] <= 'z'))
			drive[i] = (drive[i] - 'a');
		if ((drive[i] >= 'A') && (drive[i] <= 'Z'))
			drive[i] = (drive[i] - 'A');
		if (drive[i] > 26) {
			numbuf[0] = '\"';
			numbuf[1] = drive[i];
			numbuf[2] = '\0';
			PutString(numbuf);
			PutString(":\" is not a valid drive letter\r\n");
			exit(-1);
		}
	}
}


int mygetch(void);

	/** less overhead than the stream-oriented default getch* */
	/*  (getch/getchar would make the file 1-2k bigger in TC, */
	/*  as the TC version uses buffered file streams...)      */
int mygetch(void)
{
	char kbuf[5];
	int count = read(0 /* stdin */, &kbuf, 1);
	if (count > 0)
		return kbuf[0];
	return 0;
}

int againprompt(int result, char * message);

int againprompt(int result, char * message)
{
	char tmp;

	if (result==0)
		return 0; /* no "again" needed */

	PutString(message);
	PutString("- Try again? [Y/N]\r\n");

	while (1) {
		tmp=mygetch();
		if ((tmp=='Y')||(tmp=='y'))
			return 1; /* Yes, again */
		if ((tmp=='N')||(tmp=='n')||(tmp==27)||(tmp==3))
			exit(-1); /* No or escape or ^C, so quit. */
		PutString("\b");
	}
}


void biosreaddriveinfo(unsigned short driveno, struct mydiskinfo * diskinfo)
{
	unsigned short result;
	unsigned int tmp;

	diskinfo->heads = 0;
	diskinfo->nsecs = 0;
	diskinfo->secpertrack = 0;
	diskinfo->tracks = 0;
	
	/* sectorbuffer = ensureDMA(512); */
	/* we make an educated guess that we want 18 sectors of  */
	/* buffers later, so we allocate them right now already! */
	/* (any value < 1/2 sector pool size is known to work,   */
	/* for bigger values you need a bit of a lucky DS value) */
	sectorbuffer = ensureDMA(18 * 512);

recognize :
	tmp = 0;
	do {
		result = biosdisk(2,driveno,0,0,1,1, sectorbuffer);
		tmp++;
		if (result!=0)
			biosdisk(0,driveno,0,0,0,0,NULL);
	} while ((result!=0) && (tmp<5));
	
	if (againprompt(result, "Drive not ready"))
		goto recognize;

	tmp = sectorbuffer[0x15];
	if (tmp==0xf8) {
		PutString("You cannot compare harddisk partitions in BIOS mode!\r\n");
		exit(-1);
	}

	tmp = sectorbuffer[0x0c];
	tmp <<= 8;
	tmp |= sectorbuffer[0x0b];
	if (tmp!=512) {
		PutString("Sector size must be 512 bytes\r\n");
		exit(-1);
	}

	diskinfo->secpertrack = sectorbuffer[0x18];
	/* we ignore the high byte of this word */

	diskinfo->heads = sectorbuffer[0x1a];
	/* we ignore the high byte of this word */

	if ((diskinfo->heads < 1) || (diskinfo->heads > 2)) {
		PutString("Disks must have at most 2 sides\r\n");
		exit(-1);
	}

	if (diskinfo->secpertrack < 8) {
	    PutString("Disks must have at least 8 sectors per track\r\n");
	    exit(-1);
	}

	diskinfo->tracks = 40;

	tmp = sectorbuffer[0x14];
	tmp <<= 8;
	tmp |= sectorbuffer[0x13];

	if (tmp == 0) {
		PutString("You cannot check FAT32 or > 32MB disks\r\n");
		exit(-1);
	}

	diskinfo->nsecs = tmp;

	tmp = tmp / (diskinfo->heads * diskinfo->secpertrack);
	diskinfo->tracks = tmp;

	if ((diskinfo->tracks < 37) || (diskinfo->tracks > 83)) {
		PutString("Disks must have 37 to 83 tracks\r\n");
		exit(-1);
	}

	if (checkonlyoneside == true)
		diskinfo->heads = 1;
	if (checkonlyeightsectors==true)
		diskinfo->secpertrack = 8;
	if (checkonlyfourtytracks==true)
		diskinfo->tracks = 40;
	/* NOT adjusting nsecs for now! */
}


void dosreaddriveinfo(unsigned short driveno, struct mydiskinfo *diskinfo)
{
	static struct mydiskinfo olddiskinfo;
	static enum boolean firstdisk=true;
	unsigned int tmp;
	int result;
	unsigned int nsecs;

	/* sectorbuffer = ensureDMA(512); */
	/* we know that we will want 16k of buffers later, */
	/* so we allocate them right now already!          */
	sectorbuffer = ensureDMA(32 * 512);

	/* extract data from sector 0 */
	tmp = 0;
	do {
		result = absread(driveno, 1, 0, sectorbuffer);
		tmp++;
	} while ((result!=0) && (tmp<10));

	if (result!=0) {
		PutString("DOS cannot read this disk, sorry. Maybe there is no disk in drive,\r\n");
		PutString("or it is a network share or CD-ROM drive, which DISKCOMP cannot use.\r\n");
		exit(-1);
	}

	tmp = sectorbuffer[0x0c];
	tmp <<= 8;
	tmp |= sectorbuffer[0x0b];
	if (tmp != 512) {
		PutString("Only 512 (0x200) bytes per sector are allowed, not 0x");
		PutString(ultoa(tmp, numbuf, 16));
		PutString(".\r\n");
		exit(-1);
	}

	tmp = sectorbuffer[0x15];
	if (tmp==0xf8) {
		PutString("Info: Harddisk comparison only supported for < 32 MB currently.\r\n");
	}

	/* total sector count is a word at [0x13], or 0 */
	/* if it is 0, a dword at [0x20] is used, FAT32 or > 32 MB */
	nsecs = sectorbuffer[0x14];
	nsecs <<= 8;
	nsecs |= sectorbuffer[0x13];
	if (nsecs==0) {
		PutString("You cannot compare disks > 32MB\r\n");
		exit(-1);
	}

	diskinfo->nsecs = nsecs;
	diskinfo->heads = (nsecs > 400) ? 2 : 1; /* 1 for 160/180k */
	/* ignored in DOS mode */
	diskinfo->tracks = (nsecs > 800) ? 80 : 40; /* 40 for max 360k */
	/* ignored in DOS mode */
	diskinfo->secpertrack = 18; /* ignored anyway... */
	/* ignored in DOS mode */

	if ((firstdisk==false) &&
		(memcmp(&olddiskinfo, diskinfo, sizeof(struct fatinfo))!=0))
	{
		/* if not the same format quit */
		PutString("Disks formats are not the same\r\n");
		exit(-1);
	}

	firstdisk = false;
	olddiskinfo = *diskinfo;
}


void promptfordisk(unsigned short driveno)
{
	PutString("Insert disk into drive ");
	numbuf[0] = driveno + 'A';
	numbuf[1] = '\0';
	PutString(numbuf);
	PutString(": - press enter when ready...\r\n");
	(void)mygetch();
}


void bioschecksumdrive(unsigned short driveno, md5_byte_t *digest)
{
	struct mydiskinfo diskinfo;
	short result;
	unsigned int hd,cyl;
	char answer;
	unsigned char *buffer;
	md5_state_t state;

	/* initialize md5 */
	md5_init(&state);

	promptfordisk(driveno);

	/* get disk format */
	biosreaddriveinfo(driveno, &diskinfo);

	/* some info about drive */
	PutString("Comparing ");
	PutString(itoa(diskinfo.tracks, numbuf, 10));
	PutString(" tracks, ");
	PutString(itoa(diskinfo.heads, numbuf, 10));
	PutString(" side(s), ");
	PutString(itoa(diskinfo.secpertrack, numbuf, 10));
	PutString(" sectors per track (");
	PutString(ultoa(diskinfo.nsecs, numbuf, 10));
	PutString(" sectors), BIOS mode\r\n");

	/* "allocate" the memory */
	buffer = ensureDMA(diskinfo.secpertrack * 512);

	hd=0; /* head */
	for (cyl=0; cyl<diskinfo.tracks; )
	{
	read:
		PutString("Reading track ");
		PutString(itoa(cyl+1, numbuf, 10));
		PutString(" of ");
		PutString(itoa(diskinfo.tracks, numbuf, 10));
		PutString("\r"); /* only \r ! */
			
		answer = 0;
		do {
			result=biosdisk(2, driveno, hd /* head */,
				cyl /* track */, 1 /* sect */,
				diskinfo.secpertrack /* count */, buffer);
			answer++;
		} while ((result!=0) && (answer<5));

		if (againprompt(result, "Read error"))
			goto read;

		/* generating the checksum */
		md5_append(&state, (const md5_byte_t *)buffer,
			diskinfo.secpertrack*512);
		
		hd++; /* next head */
		if (hd == diskinfo.heads) {
			hd=0;
			cyl++; /* next track */
		}
	}

	PutString("\r\ndone\r\n");
	md5_finish(&state, digest);
}



void doschecksumdrive(unsigned short driveno, md5_byte_t *digest)
{
	struct mydiskinfo diskinfo;
	unsigned char *buffer; 
	char answer;
	md5_state_t state;
	short result;
	unsigned int i;

	promptfordisk(driveno);

	/* get disk format */
	dosreaddriveinfo(driveno, &diskinfo);

	/* initialize md5 */
	md5_init(&state);

	/* "alloc" mem */
	buffer = ensureDMA(32 * 512);
	/* was 1 sector only */

	PutString("Comparing ");
	PutString(ultoa(diskinfo.nsecs, numbuf, 10));
	PutString(" sectors, DOS mode (CHS = ");
	PutString(itoa(diskinfo.tracks, numbuf, 10));
	PutString(" * ");
	PutString(itoa(diskinfo.heads, numbuf, 10));
	PutString(" * ");
	PutString(itoa(diskinfo.secpertrack, numbuf, 10));
	PutString(")\r\n");

	/* here we go */
	for (i=0; i<diskinfo.nsecs; i+=32)
	{
		int howmany = 32;
		if ((howmany + i) > diskinfo.nsecs)
			  howmany = diskinfo.nsecs - i;
		read :
		answer = 0;
		do {
			result = absread(driveno, howmany , (int)i, buffer);
			answer++;
		} while ((result!=0) && (answer < 5));

		if (againprompt(result, "Read error"))
			goto read;

		PutString("Reading sectors ");
		PutString(ultoa(i, numbuf, 10));
		PutString(" - ");
		PutString(ultoa(i+howmany, numbuf, 10));
		PutString(" of ");
		PutString(ultoa(diskinfo.nsecs, numbuf, 10));
		PutString("\r"); /* \r only! */

		md5_append(&state, (const md5_byte_t *)buffer, howmany * 512);
	}

	PutString("\r\ndone\r\n");
	/* cleanup */
	md5_finish(&state, digest);
}

