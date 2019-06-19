/*
   DISKCOPY.EXE, floppy diskette duplicator similar to MSDOS Diskcopy.
   Copyright (C) 1998, Matthew Stanford.
   Copyright (C) 1999, 2004, Imre Leber.

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

#ifndef DISKCOPY_H_
#define DISKCOPY_H_

#define ON  1
#define OFF 0

/* Error levels. */
#define COPYSUCCESS 0
#define NONFATAL    1
#define CTRL_C      2
#define CRITICAL    3
#define INITERROR   4

#define READIMAGE   1		/* Numbering is important. */
#define WRITEIMAGE  2

#define EXISTS          0
#define READPERMISSION  4	/* Numbering is important. */
#define WRITEPERMISSION 2

#define ABORT           0

#define DISKREADING    0
#define DISKWRITING    0
#define VERIFICATION   1

#define VERSION "beta 0.94"

#define Beep() printf("\a")

/* Function to show help. */
void ShowHelp (char switchchar);

int RegularDiskCopy (char sdrive, char tdrive,
		     int audible, int HardDiskOk, int informative);

int ReadImageFile (char *ImageFile, char drive);
int WriteImageFile (char *ImageFile, char drive);

void OnExit (void);
int OnCBreak (void);

void ReadSectors (int drive, int nsects, int lsect, void *buffer,
		  int bytespersector);

#endif
