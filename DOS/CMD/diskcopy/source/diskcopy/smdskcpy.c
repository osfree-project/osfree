/*
   DISKCOPY.EXE, floppy diskette duplicator similar to MS-DOS Diskcopy.
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

   module: smdskcpy.c - (small drive copy) contains routines to read and
   write a small image file to the same disk as where
   the file is stored.

 */
#include <dos.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>

#ifndef HI_TECH_C
#include <dir.h>
#include <io.h>
#include <fcntl.h>
#include <alloc.h>
#include <sys\stat.h>
#endif

#include "memtypes.h"
#include "drive.h"
#include "waitfinp.h"
#include "critical.h"
#include "nlsaspct.h"
#include "diskcopy.h"

CROSSCUT_NLS_DATA_IMPORT

int
ReadFileIntoMemory (char *file, char *buffer, unsigned bsize)
{
  int handle;
  unsigned long fsize, i;

  handle = open (file, O_RDONLY | O_BINARY);
  if (handle == -1)
    return FALSE;

  fsize = filelength (handle);
  if ((fsize % bsize) != 0)
    {
      close (handle);
      return FALSE;
    }

  for (i = 0; i < fsize / bsize; i++)
    {
      if ((read (handle, buffer, bsize) != bsize) ||
	  (!WriteMemoryBlock (buffer, bsize)))
	{
	  close (handle);
	  return FALSE;
	}
    }


  close (handle);
  return TRUE;
}

int
WriteFileFromMemory (char *file, char *buffer, unsigned bsize,
		     unsigned long floppysize, int askdisk, int fallthrough,
		     int overwrite)
{
  unsigned long i;
  int criterr = 0, wrhandle;

  if (askdisk)
    {
      puts ("\n");
      NLS_PUTSTRING (1, 21, "Insert TARGET diskette into drive");
      puts ("");
      NLS_PRINTSTRING (1, 9, "Press any key to continue . . .");
      WaitForInput ();
    }

  /* Check disk capacity is the same as that of the original
     diskette. */
  for (;;)
    {
      if (!DiskLargeEnough (file, floppysize) ||
	  ((criterr = CriticalErrorOccured ()) != 0))
	{
	  puts ("");
	  if (criterr)
	    {
	      NLS_PUTSTRING (1, 10, "Disk not ready!");
	    }
	  else
	    NLS_PUTSTRING (1, 12, "Not enough disk space on target drive!");

	  if (fallthrough)
	    return FALSE;

	  puts ("");
	  NLS_PRINTSTRING (1, 23, "Put a diskette with the right capacity in drive");
	  printf (" %c:,\n", GetDiskFromPathName (file));
	  NLS_PUTSTRING (1, 24, "or press CTRL-C to cancel.");
	  WaitForInput ();	/* When the user presses CTRL-C this function does not return */
	}
      else
	break;
    }

  if (access (file, EXISTS) == 0)
    {
      if (overwrite)
	{
	  if (remove (file) == -1)
	    {
	      puts ("");
	      NLS_PUTSTRING (1, 4, "File is write protected!");
	      return FALSE;
	    }
	}
      else
	{
	  puts ("\n");
	  NLS_PUTSTRING (1, 7, "File already exists!");
	  return FALSE;
	}
    }

  wrhandle = open (file, O_WRONLY | O_BINARY | O_CREAT);

  if (wrhandle == -1)
    return FALSE;

  for (i = 0; i < floppysize / bsize; i++)
    {
      if (!ReadMemoryBlock (buffer, bsize) ||
	  (write (wrhandle, buffer, bsize) != bsize))
	{
	  close (wrhandle);
	  if (access (file, EXISTS) == 0)
	    {
	      chmod (file, S_IWRITE);
	      remove (file);
	    }
	  return FALSE;
	}
    }

  close (wrhandle);
  chmod (file, S_IWRITE);
  return TRUE;
}
