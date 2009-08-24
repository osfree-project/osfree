/*  dos.h	2.10
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

The maintainer of the Willows TWIN Libraries may be reached (Email) 
at the address twin@willows.com	

*/
 

#ifndef DOS_H
#define	DOS_H


#include "windows.h"

#ifdef __cplusplus
extern "C" {            /* Assume C declarations for C++ */
#endif	/* __cplusplus */

/* all this really has to be in something like xdos.h... */

DWORD xdoscall(UINT,UINT,LPVOID,LPVOID);

/* command opcodes */
#define XDOS_INIT	0
#define XDOS_GETDOSNAME	1	/* to DOS w/o expanding */
#define XDOS_GETDOSPATH	2	/* to DOS w/o expanding in C: format */
#define XDOS_GETALTNAME	3	/* to NATIVE w/o expanding */
#define	XDOS_GETALTPATH	4	/* to NATIVE with expanding (full path) */
#define XDOS_INT86	5
#define XDOS_SETHANDLE	6

#define XDOS_ERROR	-1

#define	INTERRUPT(intno,env)	xdoscall(XDOS_INT86,intno, 0,(LPVOID) env)

typedef struct {
	int	ec_extended;
	int	ec_class;
	int	ec_action;
	int	ec_locus;
} ERRORCODE;

/* the following are the MS-DOS definitions used by xdos/MFC fileio */
/* MSC names for file attributes. */
#define _A_NORMAL   0x00        /* Normal file, no attributes */
#define _A_RDONLY   0x01        /* Read only attribute */
#define _A_HIDDEN   0x02        /* Hidden file */
#define _A_SYSTEM   0x04        /* System file */
#define _A_VOLID    0x08        /* Volume label */
#define _A_SUBDIR   0x10        /* Directory */
#define _A_ARCH     0x20        /* Archive */

#define FA_NORMAL   0x00        /* Normal file, no attributes */
#define FA_RDONLY   0x01        /* Read only attribute */
#define FA_HIDDEN   0x02        /* Hidden file */
#define FA_SYSTEM   0x04        /* System file */
#define FA_LABEL    0x08        /* Volume label */
#define FA_DIREC    0x10        /* Directory */
#define FA_ARCH     0x20        /* Archive */

#define _LK_UNLCK   0   /* unlock the file region */
#define _LK_LOCK    1   /* lock the file region */
#define _LK_NBLCK   2   /* non-blocking lock */
#define _LK_RLCK    3   /* lock for writing */
#define _LK_NBRLCK  4   /* non-blocking lock for writing */

/* End additions. */

struct find_t {
    char reserved[21];
    char attrib;
    unsigned wr_time;
    unsigned wr_date;
    long size;
    char name[13];
};

typedef struct _dosdate_t {
        long	year;
        long    month;
        long    day;
} DOSDATE;

typedef struct _dostime_t {
	long   hour;	
	long   minute;	
	long   second;	
} DOSTIME;

struct time
{
    unsigned char ti_min;
    unsigned char ti_hour;
    unsigned char ti_hund;
    unsigned char ti_sec;
};

struct date
{
    int da_year;  /* Year - 1980 */
    char da_day;  /* Day of month */
    char da_mon;  /* Month (1 = Jan) */
};


/*
 *  Microsoft and Borland library function prototypes.
 */
unsigned _dos_getftime(int handle, unsigned *datep, unsigned *timep);
unsigned _dos_setftime(int handle, unsigned date, unsigned time);
unsigned _dos_getfileattr(char *path, unsigned short *attribp);
unsigned _dos_setfileattr(char *path, unsigned short attrib);
unsigned int _dos_findfirst(char *filename, unsigned attrib,
			    struct find_t *fileinfo);
unsigned _dos_findnext(struct find_t *fileinfo);

long dostounix(struct date *d, struct time *t);
void unixtodos(long time, struct date *d, struct time *t);


#ifdef __cplusplus
}                       /* End of extern "C" { */
#endif	/* __cplusplus */

#endif	/* DOS_H */
