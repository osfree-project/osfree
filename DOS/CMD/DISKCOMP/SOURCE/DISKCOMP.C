/*
	Free DiskComp

	Copyright (C) 1999 Michal Meller

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
	0.72: changed directory structure, added makefile, fixed bug within
	      biosreaddriveinfo, now it compiles with tc 2.01 too; with TINY
	      memory model it's almost 10 kilos lighter than 0.71!
	0.71: added ability to compare hard drive, zips etc.
	0.7 : int13h is back and /1 /8 /40 options too. BIOS is used only when
	      you use one of these options.
	0.65: does not uses int13h anymore; everything is done throug int25;
	      temporary (Ihope) removed /1 /8 and /40 options. Checged the disk
	      format detection fuction again
	0.6 : uses md5 algorithm
	0.55: not official version :added some fireworks, new disk
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
extern unsigned short headcount, trackcount, secpertrackcount;


short drive[2];

void check(void)
{
	md5_byte_t digest1[16], digest2[16];

	/* checking sides */
	if (usebios==false)
	{
		checksumdrive(drive[0],digest1);
		checksumdrive(drive[1],digest2);

	}
	else
	{
		bioscheckside(drive[0],0,digest1);
		bioscheckside(drive[1],0,digest2);
		if (checkdigests(digest1, digest2)==true)
		{
			bioscheckside(drive[0],1,digest1);
			bioscheckside(drive[1],1,digest2);
			if (checkdigests(digest1, digest2)==true)
				printf("Diffrences not found\n");
			else
				printf("Diffrences found\n");
		}
	else
		printf("Not OK\n");

	}
	if (checkdigests(digest1, digest2)==true)
	{
		printf("\Diffrences not found\n");
	}
	else
	{
		printf("\Diffrences found\n");
	}

}

void printusage(void)
{
	printf("Usage :\ndiskcomp [drive[drive]] [/1] [/8] [/?]\n\n"
		"Options:\n\n"
		"/1	-	check only one side of disk\n"
		"/8	-	check only eight sectors on each side\n"
		"/40	-	check only 40 tracks\n"
		"/b	-	force to use BIOS\n"
		"/?	-	display this help");
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
				{
					checkonlyoneside = true;
					usebios = true;
					break;
				}
				case '8' :
				{
					checkonlyeightsectors = true;
					usebios = true;
					break;
				}
				case '4' :
				{
					if (argv[i][2]=='0')
					{
						checkonlyfourtytracks = true;
						usebios = true;
					}
					break;
				}
				case 'b' :
				{
					usebios = true;
					break;
				}
				case '?':
				{
					  printusage();
					  exit(0);
				}
				default :
				{
					printf("\nInvalid option %s\n", argv[i]);
					printusage();
					exit(-1);
				}

			}
		else if (argv[i][1] == ':')
		{
			drive[currentdrive] = argv[i][0];
			currentdrive++;
		}

	}

	/* check the drives options */
	drivecheck(drive);

	/* check */
	check();

	return(0);
}

