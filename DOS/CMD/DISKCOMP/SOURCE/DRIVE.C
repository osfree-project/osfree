#include <stdio.h>
#include <dos.h>
#include "md5.h"
#include "global.h"
#include "drive.h"

/*
	needed for bios calls
*/

enum boolean usebios, checkonlyoneside, checkonlyeightsectors, checkonlyfourtytracks;
unsigned short headcount, trackcount, secpertrackcount;


enum boolean isdriveready(unsigned short driveno)
{
	short result;
	char buffer[1024];

	/* assumes that sector size isn't larger than 1024 butes */
	result = absread(driveno, 1, 0, &buffer);
	if (result == 0)
		return (true);
	else
		return (false);
}

void biosreaddriveinfo(unsigned short driveno)
{
	unsigned short result;
	char buffer[512], tmp;

recognize :
	absread(driveno, 1, 1, &buffer);
	result = biosdisk(2,driveno,0,0,1,1, &buffer);
	if (result == 0)
	{
		tmp = (short)buffer[21];
		switch (tmp)
		{
		case '\xff' :	/* 320K 5 1/2" */

			headcount=1;
			trackcount=39;
			secpertrackcount=8;
			break;

		case '\xfe' :	/* 160K 5 1/2" */

			headcount=0;
			trackcount=39;
			secpertrackcount=8;
			break;

		case '\xfd' :	/* 360K 5 1/2" */

			headcount=1;
			trackcount=39;
			secpertrackcount=9;
			break;

		case '\xfc' :	/* 180K 5 1/2" or 720K 3 1/2", so we must */
				/* recognize */

			tmp=buffer[0x1a];
			if (tmp!='\x2') /* 180K */
			{
				headcount=0;
				trackcount=39;
				secpertrackcount=9;
			}
			else
			{
				headcount=1;
				trackcount=79;
				secpertrackcount=9;
			}
			break;


		case '\xf9' :	/* 1,2M 5 1/2" or 720K 3 1/2", so we must */
				/* recognize */

			tmp=buffer[0x18];
			if (tmp!='\x9') /* 1,2M */
			{
				headcount=1;
				trackcount=79;
				secpertrackcount=15;
			}
			else
			{
				headcount=1;
				trackcount=79;
				secpertrackcount=9;
			}
			break;


		case '\xf0' :	/* 1,44 3 1/2" or 2,88 3 1/2", so we must */
				/* recognize */
			tmp=buffer[0x18];
			if (tmp!='\x12') /* 2,88M */
			{
				headcount=1;
				trackcount=79;
				secpertrackcount=36;
			}
			else
			{
				headcount=1;
				trackcount=79;
				secpertrackcount=18;
			}
			break;

		default :

			printf("Cannot recognize disk format. Please restart diskcomp without BIOS forcing oprions");
			exit(-1);

			break;
		}
	}
	else
	{
		printf("Drive not ready \nTry again? Y/N");
		tmp=getch();
		if ((tmp=='Y')||(tmp=='y')) goto recognize;
		   else
		   {
			exit(-1);
		   }
	}

	if (checkonlyoneside == true) headcount = 0;
	if (checkonlyeightsectors==true) secpertrackcount = 8;
	if (checkonlyfourtytracks==true) trackcount = 39;	/* I hope this is good */
}

void readdriveinfo(unsigned char driveno, struct mydiskinfo *diskinfo)
{
	static struct mydiskinfo olddiskinfo;
	static enum boolean firstdisk=true;
	char tmp;
	char buffer[1024];
	unsigned long nsecs;

	getfat(driveno+1, (struct fatinfo *)diskinfo);

	tmp=diskinfo->fi_fatid;

	switch (tmp)
	{
	case '\xff' :	/* 320K 5 1/2" */
		diskinfo->di_nsecs = 315;
		break;
	case '\xfe' :	/* 160K 5 1/2" */
		diskinfo->di_nsecs = 313;
		break;
	case '\xfd' :	/* 360K 5 1/2" */
		diskinfo->di_nsecs = 351;
		break;
	case '\xfc' :	/* 180K 5 1/2" or 720K 3 1/2", so we must */
			/* recognize */
		diskinfo->di_nsecs = 354;
		break;
	case '\xf9' :	/* 1,2M 5 1/2" or 720K 3 1/2", so we must */
			/* recognize */
		if (diskinfo->fi_sclus == 1)
			diskinfo->di_nsecs = 2371;
		else
			diskinfo->di_nsecs = 720;

		break;
	case '\xf0' :	/* 1,44 3 1/2" or 2,88 3 1/2", so we must */
			/* recognize */
		diskinfo->di_nsecs=2880;
		break;
	default :
		/*
		printf("Cannot recognize disk format");
		//exit(-1);
		//extract data from sector 0
		*/
		absread(driveno, 1, 0, &buffer);
		/*
		  read sectror count
		  offset 13h-14h  - less than 32MB
		  offset 20h-23h  - more than 32MB
		*/
		nsecs = (unsigned int)buffer[0x13]<<0;
		nsecs += (unsigned int)buffer[0x14]<<8;
		if (nsecs==0) {
			nsecs = 0;                 /* abcd  dcba */
			nsecs = (long)buffer[0x20]<<0;
			nsecs +=(long)buffer[0x21]<<8;
			nsecs +=(long)buffer[0x22]<<16;
			nsecs +=(long)buffer[0x23]<<24;
		}
		diskinfo->di_nsecs = nsecs;
		break;
	}


	if ((firstdisk==false)&& (memcmp(&olddiskinfo, diskinfo, sizeof(struct fatinfo))!=0))
	{
		/* if not the same format quit */
		printf("Disks formats are not the same\n");
		exit(-1);
	}

	firstdisk=false;
	olddiskinfo = *diskinfo;
}


void drivecheck(short *drive)
{
	short i;
	/* check if drive given */
	if (drive[0] == 0)
	{
		drive[0]=getdisk()+'A';
		drive[1]=drive[0];

	}
	if (drive[1] == 0) drive[1] = drive[0];
	/* change the drive from literal format to int */
	for (i=0;i<=1;i++)
		if ((drive[i] >= 'a') && (drive[i] <= 'z')) drive[i] = (drive[i] - 'a');
		else
		if ((drive[i] >= 'A') && (drive[i] <= 'Z')) drive[i] = (drive[i] - 'A');
		else
		{
			if (usebios!=true)
			{
				printf("\"%c:\" is incorrect drive\n", drive[i]);
				exit(-1);
			}
		}
}


void bioscheckside(short driveno, short sideno, md5_byte_t *digest)
{
	long tmp;
	short result;
	register short i;
	register int m;
	int j,k,l;
	char answer;
	char *buffer;
	md5_state_t state;
	int di;

	/* initialize md5 */
	md5_init(&state);

	if (sideno <= headcount)
	{
	printf("\nInsert disk into drive %c:\n\nStrike any key when ready . . .\n", driveno+'A');
	getch();
	if ((sideno==driveno)&&(sideno==0))
	{
		/* read data about drive */
		biosreaddriveinfo(driveno);

		/* some info about drive */
		printf("\nComparing %i tracks,\n%i sectors per track, %i side(s)\n", trackcount+1, secpertrackcount, headcount+1);
	}
	printf("\nReading side %i of disk ... ", sideno);

	/* allocate the memory */
	buffer=(char *)calloc(secpertrackcount, 512);
	for(l=0;l<secpertrackcount*512;l++) buffer[l]=0x00;

	/*
	  why this piece of code ? well, when disk is changed my comp bacomes angry and
	  simple read via biosdisk is impossibile; after calling int25 everythisg is all right
	*/
    read :
	if (biosdisk(0x16, driveno, 0,0,0,0,NULL)!=0) { absread(driveno,1,1,buffer);}

	j=sideno;
		for (k=0;k<trackcount;k++)
		{
			if ((j==0) && (k==0))
			{
				l=2;
				result=biosdisk(2, driveno, j,k,l,secpertrackcount-2, buffer);
			}
			else
			{
				l=1;
				result=biosdisk(2, driveno, j,k,l,secpertrackcount, buffer);
			}
			if (result==0)
			{
				/* generating the checksum */
/*				for (m=0; m<secpertrackcount*512; m=m+8) */
  /*				{*/
					md5_append(&state, (const md5_byte_t *)buffer, secpertrackcount*512);
	/*			}*/
			}
			else
			{
				printf("\nRead error\nTry again ? Y/N");
				answer=getch();
				if ((answer=='Y')||(answer=='y')) goto read;
				   else
				   {
					free(buffer);
					exit(-1);
				   }
			}
		}
	free(buffer);
	printf("done\n");
	}
	md5_finish(&state, digest);


}



void checksumdrive(short driveno, md5_byte_t *digest)
{
	struct mydiskinfo diskinfo;
	char *buffer, answer;
	md5_state_t state;
	short result;
	long i;

	printf("\nInsert disk into drive %c:\n\nStrike any key when ready . . .\n", driveno+'A');
	getch();

recognize:
	if (isdriveready(driveno)==true)
	{
		/* get disk format */
		readdriveinfo(driveno, &diskinfo);

		/* initialize md5 */
		md5_init(&state);
		/* alloc mem */
		buffer=(char *)calloc(diskinfo.di_bysec,1);

		/* here we go */
		for (i=1; i<=diskinfo.di_nsecs-1;i++)
		{
			read :
			result = absread(driveno,1 , i, buffer);
			/* do we have errors during read ? */
			if (result==0)
			{
				/* no */
				md5_append(&state, (const md5_byte_t *)buffer, diskinfo.di_bysec);
			}
			else
			{
				/* yes */
				printf("\nRead error\nTry again ? Y/N");
				answer=getch();
				if ((answer=='Y')||(answer=='y')) goto read;
				   else
				   {
					free(buffer);
					exit(-1);
				   }
			}
		}
		/* cleanup */
		free(buffer);
		md5_finish(&state, digest);
	}
	else
	{
		/* we can't read drive before getfat, so it won't work */
		printf("Drive not ready \nTry again? Y/N");
		answer=getch();
		if ((answer=='Y')||(answer=='y')) goto recognize;
		   else
		   {
			exit(-1);
		   }
	}
}

