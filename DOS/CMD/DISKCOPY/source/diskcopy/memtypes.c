/*
   DISKCOPY.EXE, floppy diskette duplicator similar to MSDOS Diskcopy.
   Copyright (C) 1998, Matthew Stanford.
   Copyright (C) 1999, 2000, 2001 Imre Leber.

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

   01-14-2000 : optimized for speed (selecting function = é(1)).

 */

#include <dos.h>
#include <stdlib.h>
#include <string.h>

#ifndef HI_TECH_C
#include <dir.h>
#include <io.h>
#include <fcntl.h>
#include <alloc.h>
#endif
#include <ctype.h>
#include <errno.h>
#include <stdio.h>

#include "memtypes.h"
#include "xms.h"
#include "ems.h"
#include "diskcopy.h"
#include "drive.h"
#include "parser.h"
#include "fastcopy.h"
#include "lfnapi.h"


/*
   #define TRUE  1
   #define FALSE 0
 */

static int MemType = UNALLOCATED;
static unsigned EMSBase;
static int Handle;
static unsigned long AmofProcessed;
static char far *FarBlock;
static unsigned LastSegment, SavedSegment;
static unsigned LastOffset, SavedOffset;
static int IsImageFile = FALSE;
static int Initialised = FALSE;

static char tempfile[128];

static int (*ReadFunc) (char*, unsigned);
static int (*WriteFunc) (char*, unsigned);

static void (*CloseFunc) (void);

static void (*PrepareToReadFunc) (void);
static void (*PrepareToWriteFunc) (void);

/*================================== EMS ==================================*/

static int
EMSRead (char *buffer, unsigned size)
{
  int start, i;
  unsigned blocks;

  start = (int) (AmofProcessed / 16384);
  blocks = size / 16384;

  for (i = 0; i < blocks; i++)
    {
      if (EMSmap (0, Handle, start + i) == -1)
	return FALSE;
      movedata (EMSBase, 0,
		FP_SEG (buffer), FP_OFF (buffer) + (i * 16384), 16384);
    }

  return TRUE;
}

static int
EMSWrite (char *buffer, unsigned size)
{
  int start, i;
  unsigned blocks;

  start = (int) (AmofProcessed / 16384);
  blocks = size / 16384;

  for (i = 0; i < blocks; i++)
    {
      if (EMSmap (0, Handle, start + i) == -1)
	return FALSE;
      movedata (FP_SEG (buffer), FP_OFF (buffer) + (i * 16384),
		EMSBase, 0, 16384);
    }

  return TRUE;
}

static void
CloseEMS (void)
{
  EMSfree (Handle);
}

/*================================== XMS ==================================*/

static int
XMSRead (char *buffer, unsigned size)
{
  return (XMStoDOSmove (buffer, Handle, AmofProcessed, size) == size);
}

static int
XMSWrite (char *buffer, unsigned size)
{
  return (DOStoXMSmove (Handle, AmofProcessed, buffer, size) == size);
}

static void
CloseXMS (void)
{
  XMSfree (Handle);
}

/*================================= DISK ==================================*/

static int
DiskRead (char *buffer, unsigned size)
{
  return (read (Handle, buffer, size) == size);
}

static int
DiskWrite (char *buffer, unsigned size)
{
  return (write (Handle, buffer, size) == size);
}

static void
PrepareForReadingDisk (void)
{
  close (Handle);
  Handle = open (tempfile, O_RDONLY | O_BINARY);
}

static void
PrepareForWritingDisk (void)
{
  close (Handle);
  Handle = open (tempfile, O_WRONLY | O_BINARY);
}

static void
CloseDisk (void)
{
  close (Handle);
  remove (tempfile);
}

static void
CloseImageFile (void)
{
  close (Handle);
}

/*================================= BUFFERS ===============================*/

static int
BufferRead (char *buffer, unsigned size)
{
  movedata (LastSegment, LastOffset,
	    FP_SEG (buffer), FP_OFF (buffer), size);
  LastSegment = LastSegment + (size / 16);

  return TRUE;
}

static int
BufferWrite (char *buffer, unsigned size)
{
  movedata (FP_SEG (buffer), FP_OFF (buffer),
	    LastSegment, LastOffset, size);
  LastSegment = LastSegment + (size / 16);

  return TRUE;
}

static void
CloseBuffers (void)
{
  farfree (FarBlock);
}

static void
PrepareForChangingBuffers (void)
{
  LastSegment = SavedSegment;
  LastOffset = SavedOffset;
}

/*=========================================================================*/

static void
DoNothing (void)
{
}
static void
ResetAmof (void)
{
  AmofProcessed = 0;
}

/****************************** PUBLIC FUNCTIONS ***************************/

int
InitializeFittingMemory (unsigned long size, int HardDiskOk,
			 unsigned long *allocated,
			 /* Needed to check wether a swap file is
			    not created on the source or destination
			    drive: */
			 char sdrv, char tdrv)
{
  char *tempvar = NULL;
  struct dfree free;
  int curdisk, tempdisk, run = 1;
  unsigned long farfree;
  struct IniParserStruct *ParsedData;

  ParsedData = GetParsedData ();

  Initialised = TRUE;
  AmofProcessed = 0;

  CloseFunc = DoNothing;
  PrepareToReadFunc = ResetAmof;
  PrepareToWriteFunc = ResetAmof;

  /* First of all see wether main memory is not large enough */
  farfree = farcoreleft ();
  if (farfree > size)
    {
      FarBlock = (char far *) farmalloc (size);

      LastOffset = FP_OFF (FarBlock);
      LastSegment = FP_SEG (FarBlock);
      SavedSegment = LastSegment = LastSegment + LastOffset / 16;
      SavedOffset = LastOffset = LastOffset % 16;

      MemType = BUFFERS;
      CloseFunc = CloseBuffers;
      ReadFunc = BufferRead;
      WriteFunc = BufferWrite;
      PrepareToReadFunc = PrepareForChangingBuffers;
      PrepareToWriteFunc = PrepareForChangingBuffers;

      *allocated = size;
    }


  /* See wether EMS is installed. */
  if ((!MemType) && ParsedData->UseEMS && ((EMSBase = EMSbaseaddress ()) != 0))
    {
      /* See if there is enough memory. */
      if ((size / 16384) + 1 < EMSpages ())
	{
	  Handle = EMSalloc ((int) (size / 16384) + 1);
	  if (Handle >= 0)
	    {
	      CloseFunc = CloseEMS;
	      WriteFunc = EMSWrite;
	      ReadFunc = EMSRead;
	      MemType = EMS;
	    }

	  *allocated = size;
	}
    }

  /* See wether XMS is installed. */
  if (ParsedData->UseXMS && (!MemType) && XMSinit ())
    {
      /* See if there is enough memory. */
      if (size < XMScoreleft ())
	{
	  Handle = XMSalloc (size);
	  if (Handle != 0)
	    {
	      MemType = XMS;
	      CloseFunc = CloseXMS;
	      WriteFunc = XMSWrite;
	      ReadFunc = XMSRead;
	    }

	  *allocated = size;
	}
    }

  /* See wether the hard disk can be used. */
  if (!MemType && HardDiskOk)
    {
      tempvar = getenv ("temp");
      if (!tempvar)
	tempvar = getenv ("TEMP");
      if (tempvar && tempvar[0])
	strcpy (tempfile, tempvar);
      else
	strcpy (tempfile, "C:\\");

      /* Make sure that the disk where the temporary file is created
         is not either the source of the target of the disk copy.     */
      if ((toupper (tempfile[0]) == toupper (sdrv)) ||
	  (toupper (tempfile[0]) == toupper (tdrv)))
	strcpy (tempfile, "C:\\");

      curdisk = getdisk ();
      tempdisk = toupper (tempfile[0]) - 65;

      /* If the TEMP environment variable doesn't have a path form,
         use the root of c: instead. */
      if ((tempdisk < 0) || (tempdisk > 26) || (tempfile[1] != ':'))
	{
	  strcpy (tempfile, "C:\\");
	  tempdisk = 2;
	}

      for (;;)
	{
	  setdisk (tempdisk);
	  if (getdisk () == tempdisk)
	    {
	      unsigned long total;

	      setdisk (curdisk);

	      getdfree (tempdisk + 1, &free);
	      total = (long) free.df_avail * (long) free.df_bsec *
		(long) free.df_sclus;

	      if (total >= size)
		{
                  char* pSFNSlot;
                  ConvertToSFN(tempfile, ENVVAR_ID);
                  pSFNSlot = GetSFN(ENVVAR_ID);
                  if (pSFNSlot)
                  {
                     strcpy(tempfile, pSFNSlot);
		     Handle = creattemp (tempfile, 0);
		     if (Handle != -1)
		     {
		        MemType = HARDDISK;
		        CloseFunc = CloseDisk;
		        ReadFunc = DiskRead;
		        WriteFunc = DiskWrite;
		        PrepareToReadFunc = PrepareForReadingDisk;
		        PrepareToWriteFunc = PrepareForWritingDisk;
		        *allocated = size;
		        break;
		     }
                  }
		}
	    }
	  if (tempdisk != 2 || run == 1)	/* if not C drive. */
	    {
	      run++;
	      strcpy (tempfile, "C:\\");
	      tempdisk = 2;	/* try C drive instead. */
	    }
	  else
	    break;
	}
    }

  /* See if there is enough memory left on the far heap. */
  if (!MemType && farfree > BUFFERSIZE)
    {
      size = farfree - (farfree % BUFFERSIZE);
      FarBlock = (char far *) farmalloc (size);

      LastOffset = FP_OFF (FarBlock);
      LastSegment = FP_SEG (FarBlock);
      SavedSegment = LastSegment = LastSegment + LastOffset / 16;
      SavedOffset = LastOffset = LastOffset % 16;

      MemType = BUFFERS;
      CloseFunc = CloseBuffers;
      ReadFunc = BufferRead;
      WriteFunc = BufferWrite;
      PrepareToReadFunc = PrepareForChangingBuffers;
      PrepareToWriteFunc = PrepareForChangingBuffers;

      *allocated = size;
    }

  return MemType;
}

int
ReadMemoryBlock (char *buffer, unsigned size)
{
  if (ReadFunc (buffer, size))
    {
      AmofProcessed += (long) size;
      return TRUE;
    }
  return FALSE;
}

int
WriteMemoryBlock (char *buffer, unsigned size)
{
  if (WriteFunc (buffer, size))
    {
      AmofProcessed += (long) size;
      return TRUE;
    }
  return FALSE;
}

void
PrepareForReading (void)
{
  PrepareToReadFunc ();
}

void
PrepareForWriting (void)
{
  PrepareToWriteFunc ();
}

void
ReleaseMemory (void)
{
  if (Initialised)
    {
      CloseFunc ();
      CloseFunc = DoNothing;
      MemType = 0;
    }
}

int
DiskLargeEnough (char *imagefile, unsigned long floppysize)
{
  int disk;
  struct dfree free;

  if (imagefile[0] != 0 && imagefile[1] == ':')
    disk = toupper (imagefile[0]) - 65;
  else
    disk = getdisk ();

  getdfree (disk + 1, &free);	/* absread doesn't work on FAT16! */

  return (floppysize <= (long) free.df_avail * (long) free.df_bsec *
	  (long) free.df_sclus);
}

int
SetImageFile (char *imagefile, int ImageModus, unsigned long floppysize)
{
  if (ImageModus == WRITEIMAGE)
    {
      if (!DiskLargeEnough (imagefile, floppysize))
	return DISKTOSMALL;

      Handle = open (imagefile, O_WRONLY | O_BINARY | O_CREAT);
    }
  else
    Handle = open (imagefile, O_RDONLY | O_BINARY);

  if (Handle == -1)
    return errno;

  CloseFunc = CloseImageFile;
  ReadFunc = DiskRead;
  WriteFunc = DiskWrite;
  PrepareToReadFunc = PrepareForReadingDisk;
  PrepareToWriteFunc = PrepareForWritingDisk;

  IsImageFile = TRUE;
  Initialised = TRUE;

  strcpy (tempfile, imagefile);

  return EZERO;
}

void
SetErrorStopped (void)
{
  if (IsImageFile)
    {
      CloseFunc = CloseDisk;
    }
}
