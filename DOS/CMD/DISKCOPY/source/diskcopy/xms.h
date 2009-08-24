/*
   DISKCOPY.EXE, floppy diskette duplicator similar to MSDOS Diskcopy.
   Copyright (C) 1998, Matthew Stanford.
   Copyright (C) 1999, 2000, Imre Leber.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have recieved a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.


   If you have any questions, comments, suggestions, or fixes please
   email me at:  imre.leber@worldonline.be

 */

/*
   ** XMS.H
   **
   ** Header file for Extended Memory routines in XMS.ASM.
 */

#ifndef _XMS_H
#define _XMS_H

/* initialisation routine. */
int XMMinit (void);

/* routines for managing EMB's */
int XMSinit (void);
int XMSversion (void);
unsigned long XMScoreleft (void);
int XMSfree (unsigned int handle);
long XMSmemcpy (unsigned int desthandle, long destoff,
		unsigned int srchandle, long srcoff, long n);
int DOStoXMSmove (unsigned int desthandle, long destoff,
		  const char *src, unsigned n);
int XMStoDOSmove (char *dest, unsigned int srchandle, long srcoff, unsigned n);

unsigned int XMSalloc (long size);

int XMSrealloc (unsigned int handle, long size);

/* routines for managing the HMA. */
int HMAalloc (void);
int HMAcoreleft (void);
int HMAfree (void);

/* routines for managing UMB's. */
unsigned int UMBalloc (void);
unsigned int GetUMBsize (void);
int UMBfree (unsigned int segment);

#endif
