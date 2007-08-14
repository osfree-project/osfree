/*
	Free DiskComp

	Copyright (C) 1999 Michal Meller
	Changes 2003 by Eric Auer, eric -at- coli.uni-sb.de

	This program is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License,
	or (at your option) any later version. 

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU 
	General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, write to the
	Free Software Foundation,
	Inc., 59 Temple Place - Suite 330,
	Boston, MA 02111-1307, USA.

	Contact : maceman@priv4.onet.pl, mellerm@witual.pl

	How does it all works?
	Well it's pretty simple. The program reads side of the disk
	and genetares a checksum of it using the des_encrypt fuction.
	When the checksums are identical that means the disk are identical too

	Changes:
	0.74: heavy code cleanup (by EA). Harddisks no longer accepted.
	0.73: cleaned up drive detection a bit and increased buffer size in
	      DOS mode (not in BIOS mode) from 1 to 32 sectors. Sector size
	      now fixed at 512 bytes. Smaller stack footprint.
	      Changed directory structure again, to match FreeDOSes.
	      *** (by Eric Auer / EA).
	0.72: changed directory structure, added makefile, fixed bug within
	      biosreaddriveinfo, now it compiles with tc 2.01 too; with TINY
	      memory model it's almost 10 kilos lighter than 0.71!
	0.71: added ability to compare hard drive, zips etc.
	0.7 : int13h is back and /1 /8 /40 options too. BIOS is used only when
	      you use one of these options.
	      *** Note EA: BIOS format detection or user-specified geometry
	      *** would be an idea for non-DOS disk formats. /1 /8 /40 less.
	0.65: does not uses int13h anymore; everything is done through int25;
	      temporary (I hope) removed /1 /8 and /40 options. Checged the disk
	      format detection fuction again
	0.6 : uses md5 algorithm
	      *** Note EA: How about RIPEMD / RIPEMD128? Or no checksum, just
	      *** XMS for comparison of all data?
	0.55: not official version: added some fireworks, new disk
	      identification function, which support 160K 180K 320K
	      360K 720K 1,2M and 1,44M diskes (I hope), '/40' option,
	      better error handling, and the GNU public license in
	      the beginning
	0.5 : initial release

	P.S. Sorry for my poor English ...
*/

#include <stdio.h>
#include <dos.h>
#include "md5.h"
#include "global.h"
#include "drive.h"

/*
	needed for bios calls
*/

extern enum boolean usebios, checkonlyoneside, checkonlyeightsectors, checkonlyfourtytracks;


short drive[2];

void printDigests(md5_byte_t *digest1, md5_byte_t *digest2)
{
	int i,j;
	char hexbuf[3];
	char hexdig[16] = "0123456789abcdef";
	
	PutString(  "MD5 checksums: ");
	for (i=0; i<16; i++) {
		hexbuf[0] = hexdig[digest1[i] >> 4];
		hexbuf[1] = hexdig[digest1[i] & 15];
		hexbuf[2] = '\0';
		PutString(hexbuf);
	}
	j=0;
	for (i=0; i<16; i++)
		if (digest1[i] != digest2[i])
			j++;
	if (j==0) {
		PutString(" (both the same)\r\n");
		return;
	}
	PutString("\r\n          and: ");
	for (i=0; i<16; i++) {
		hexbuf[0] = hexdig[digest2[i] >> 4];
		hexbuf[1] = hexdig[digest2[i] & 15];
		hexbuf[2] = '\0';
		PutString(hexbuf);
	}
	PutString("\r\n");
}


int check(void) /* return 0 if same, 1 if different */
{
	md5_byte_t digest1[16], digest2[16];

	/* checking sides */
	if (usebios==false)
	{
		doschecksumdrive(drive[0], digest1);
		doschecksumdrive(drive[1], digest2);
		printDigests(digest1,digest2);
		if (checkdigests(digest1, digest2)==true)
			return 0;
		return 1;
	}

	if ((drive[0]>1) || (drive[1]>1)) {
		PutString("Note that BIOS mode does not support harddisks!\r\n");
		/* ... but people may have > 2 floppy drives ... */
	}
	
	bioschecksumdrive(drive[0], digest1);
	bioschecksumdrive(drive[1], digest2);
	printDigests(digest1,digest2);
	if (checkdigests(digest1, digest2)==true)
		return 0;
	return 1;
}

void printusage(void)
{
	PutString("Usage :\r\ndiskcomp [drive: [drive:]] [/1] [/8] [/4[0]] [/b] [/?]\r\n\r\n"
		"Options:\r\n"
		"/1  - check only the first side of each disk\r\n"
		"/8  - check only the first eight sectors per track\r\n"
		"/40 - check only first 40 tracks (shortcut: /4)\r\n"
		"/b  - force BIOS (instead of DOS) disk access mode\r\n"
		"/?  - display this help\r\n\r\n"
		"This program compares two disks (max 32 MB each) and shows the\r\n"
		"md5sum digest of the contents. It tells whether disk contents\r\n"
		"differ (shows a message and sets the errorlevel appropriately).\r\n\r\n"
		"If you only give one drive, you have to change disks\r\n"
		"after the first disk is read. If you give no drive option,\r\n"
		"two disks in the current drive are compared to each other.\r\n");
}

int main(int argc, char *argv[])
{
	short i;
	short currentdrive=0;

	usebios = false;

	/* get the command parameters */
	for (i=1;i<=argc-1;i++)
	{
		if (argv[i][0] == '/')
			switch (argv[i][1])
			{
				case '1' :
					checkonlyoneside = true;
					usebios = true;
					break;
				case '8' :
					checkonlyeightsectors = true;
					usebios = true;
					break;
				case '4' :
					/* if (argv[i][2]=='0') { ... */
					checkonlyfourtytracks = true;
					usebios = true;
					break;
				case 'b' :
					usebios = true;
					break;
				case '?':
					printusage();
					return 2;
					/* break; */
				default :
					PutString("\r\nInvalid option ");
					PutString(argv[i]);
					PutString("\r\n");
					printusage();
					return 3;
			}
		else if (argv[i][1] == ':')
		{
			if (currentdrive>1) {
				PutString("\r\nYou cannot compare more than 2 disks at a time\r\n");
				printusage();
				return 4;
			}
			drive[currentdrive] = argv[i][0];
			currentdrive++;
		}

	}

	/* check the drives options, translate A to 0 and so on */
	drivelettercheck(drive);

	/* check */
	i = (short) check();

	if (i==0)
		PutString("\r\nBoth disks contain the same data.\r\n");
	else
		PutString("\r\nDisk contents are different.\r\n");

	return i;
}

