/* $Id: bufpool.h,v 1.2 2004/03/21 02:43:20 pasha Exp $ */

/* static char *SCCSID = "@(#)1.1  7/30/96 15:56:54 src/jfs/utils/libfs/bufpool.h, jfslib, w45.fs32, 990417.1";*/
/* @(#)bufpool.h	1.5  @(#)bufpool.h	1.5 3/25/96 12:47:24  */
/*
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
 *   COMPONENT_NAME: cmdufs
 *
 *   FUNCTIONS:
 *
 */

/*
 * -> BPDEBUG should be defined if you want megs of debug output...
 */

#ifndef H_BUFPOOL
#define H_BUFPOOL

/*
 * turn buffer pool debugging on/off
 */
#undef	BPDEBUG

#define FBWRITE_ERR	1
#define SEEK_ERR	2
#define READ_ERR	3
#define	WRITE_ERR	4
#define READ_CLEAR_ERR	5


void bpinit(int, void (*)(), uint32, uint32);
void *bpread(uint64);
void  bptouch(void *);
void  bprelease(void *);
void  bpflush(void *);
void  bpclose(void);
void  bpassert(char *, void *, int);


/*
 * buffer pool debug stuff
 */
#ifdef	BPDEBUG
# define	BPPRINTF(args)		printf args 
#else	/* BPDEBUG */
# define	BPPRINTF(args)
#endif	/* BPDEBUG */

#endif	/* H_BUFPOOL */
