/* $Id: statscall.c,v 1.2 2004/03/21 02:43:12 pasha Exp $ */

static char *SCCSID = "@(#)1.2  7/31/98 08:21:25 src/jfs/utils/os2stats/statscall.c, jfsutil, w45.fs32, 010429.3";
/*
 *
 *
 *   Copyright (c) International Business Machines  Corp., 2000
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or 
 *   (at your option) any later version.
 * 
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software 
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * Module: fscntl.c
 */

#define INCL_DOSPROCESS
#include <os2.h>
#include <stdio.h>

#include "jfs_types.h"
#include "jfs_cntl.h"

#pragma pack(1)
struct _opstats
{
    int allocatepagespace;  /* 00 */
    int attach;             /* 01 */
    int chdir;              /* 02 */
    int chgfileptr;         /* 03 */
    int close;              /* 04 */
    int commit;             /* 05 */
    int copy;               /* 06 */
    int delete;             /* 07 */
    int dopageio;           /* 08 */
    int exit;               /* 09 */
    int fileattribute;      /* 0a */
    int fileinfo;           /* 0b */
    int fileio;             /* 0c */
    int findclose;          /* 0d */
    int findfirst;          /* 0e */
    int findfromname;       /* 0f */
    int findnext;           /* 10 */
    int findnotifyclose;    /* 11 */
    int findnotifyfirst;    /* 12 */
    int findnotifynext;     /* 13 */
    int flushbuf;           /* 14 */
    int fsctl;              /* 15 */
    int fsinfo;             /* 16 */
    int ioctl;              /* 17 */
    int mkdir;              /* 18 */
    int mount;              /* 19 */
    int move;               /* 1a */
    int newsize;            /* 1b */
    int nmpipe;             /* 1c */
    int opencreate;         /* 1d */
    int openpagefile;       /* 1e */
    int pathinfo;           /* 1f */
    int processname;        /* 20 */
    int read;               /* 21 */
    int rmdir;              /* 22 */
    int setswap;            /* 23 */
    int shutdown;           /* 24 */
    int write;              /* 25 */
    int cancellockreq;      /* 26 */
    int filelocks;          /* 27 */
    int cacheread;          /* 28 */
    int returnfilecache;    /* 29 */
    int LastCalled;         /* PS */
} opstats;
#pragma pack()

/*
 *	main()
 */
main(argc, argv)
int argc;
char **argv;
{
	int rc;
	int t, type;
        unsigned long dummyd = 0;
        unsigned long dummyp = 0;

        rc = DosFSCtl(  &opstats,                      /* data area */
                        (int)sizeof(struct _opstats),       /* size of data */
                        &dummyd,        /* returned data length */
                        NULL,           /* parameter list */
                        0,              /* size of parameter list */
                        &dummyp,        /* return parm length */
                        JFSCTL_CALLSTATS,   /* function code */
                        "JFS",          /* target file system */
                        -1,             /* file handle */
                        FSCTL_FSDNAME); /* method of file system id */
	
	if (rc != 0)
	{
#ifdef FreeJFS
		printf("FreeJFS not present\n");
#else
		printf("fscntl call returned %d\n", rc);
#endif
		exit(2);
	}
printf("AllocatePageSpace %u\n",    opstats.allocatepagespace);  /* 00 */
printf("Attach            %u\n",    opstats.attach);             /* 01 */
printf("ChDir             %u\n",    opstats.chdir);              /* 02 */
printf("ChgFilePtr        %u\n",    opstats.chgfileptr);         /* 03 */
printf("Close             %u\n",    opstats.close);              /* 04 */
printf("Commit            %u\n",    opstats.commit);             /* 05 */
printf("Copy              %u\n",    opstats.copy);               /* 06 */
printf("Delete            %u\n",    opstats.delete);             /* 07 */
printf("DoPageIO          %u\n",    opstats.dopageio);           /* 08 */
printf("Exit              %u\n",    opstats.exit);               /* 09 */
printf("FileAttribute     %u\n",    opstats.fileattribute);      /* 0a */
printf("FileInfo          %u\n",    opstats.fileinfo);           /* 0b */
printf("FileIO            %u\n",    opstats.fileio);             /* 0c */
printf("FindClose         %u\n",    opstats.findclose);          /* 0d */
printf("FindFirst         %u\n",    opstats.findfirst);          /* 0e */
printf("FindFromName      %u\n",    opstats.findfromname);       /* 0f */
printf("FindNext          %u\n",    opstats.findnext);           /* 10 */
printf("FindNotifyClose   %u\n",    opstats.findnotifyclose);    /* 11 */
printf("FindNotifyFirst   %u\n",    opstats.findnotifyfirst);    /* 12 */
printf("FindNotifyNext    %u\n",    opstats.findnotifynext);     /* 13 */
printf("FlushBuf          %u\n",    opstats.flushbuf);           /* 14 */
printf("FSCtl             %u\n",    opstats.fsctl);              /* 15 */
printf("FSInfo            %u\n",    opstats.fsinfo);             /* 16 */
printf("IOCtl             %u\n",    opstats.ioctl);              /* 17 */
printf("MkDir             %u\n",    opstats.mkdir);              /* 18 */
printf("Mount             %u\n",    opstats.mount);              /* 19 */
printf("Move              %u\n",    opstats.move);               /* 1a */
printf("NewSize           %u\n",    opstats.newsize);            /* 1b */
printf("NMPipe            %u\n",    opstats.nmpipe);             /* 1c */
printf("OpenCreate        %u\n",    opstats.opencreate);         /* 1d */
printf("OpenPageFile      %u\n",    opstats.openpagefile);       /* 1e */
printf("PathInfo          %u\n",    opstats.pathinfo);           /* 1f */
printf("ProcessName       %u\n",    opstats.processname);        /* 20 */
printf("Read              %u\n",    opstats.read);               /* 21 */
printf("RmDir             %u\n",    opstats.rmdir);              /* 22 */
printf("SetSwap           %u\n",    opstats.setswap);            /* 23 */
printf("ShutDown          %u\n",    opstats.shutdown);           /* 24 */
printf("Write             %u\n",    opstats.write);              /* 25 */
printf("CancelLockReq     %u\n",    opstats.cancellockreq);      /* 26 */
printf("FileLocks         %u\n",    opstats.filelocks);          /* 27 */
printf("Cache Read        %u\n",    opstats.cacheread);          /* 29 */
printf("Return File Cache %u\n",    opstats.returnfilecache);    /* 2A */
printf("Last Called Fun.  %x\n",    opstats.LastCalled);         /* PS */
}
usage(char *n)
{
	printf("usage: %s -[njl]\n", n);
	exit(1);
}
