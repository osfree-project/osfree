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

#ifndef MEMTYPES_H_
#define MEMTYPES_H_

/* Memory types. */
#define EMS         4
#define XMS         3
#define HARDDISK    2
#define BUFFERS     1
#define NONEFITTING 0
#define UNALLOCATED 0

#define READING 0
#define WRITING 1

#define READ  0
#define WRITE 1

#define BUFFERSIZE     16384u
#define BYTESPERSECTOR   512
#define TOCOPYSECTORS  (BUFFERSIZE / BYTESPERSECTOR)

#define DISKTOSMALL -2

int InitializeFittingMemory (unsigned long size, int HardDiskOk,
			     unsigned long *allocated,
			     char sdrv, char tdrv);
int ReadMemoryBlock (char *buffer, unsigned size);
int WriteMemoryBlock (char *buffer, unsigned size);

void PrepareForReading (void);
void PrepareForWriting (void);

void ReleaseMemory (void);

int SetImageFile (char *imagefile, int ImageModus, unsigned long filesize);
void SetErrorStopped (void);
int DiskLargeEnough (char *imagefile, unsigned long floppysize);

#endif
