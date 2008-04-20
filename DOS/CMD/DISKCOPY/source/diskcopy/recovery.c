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
   email me at:  imre.leber@worlonline.be

   module: recovery.c : routine for handling recovery mode.

 */

#include "diskcopy.h"
#include "drive.h"
#include "nlsaspct.h"

#include <dos.h>
#include <stdio.h>

CROSSCUT_NLS_DATA_IMPORT

/*
   ** Reads sectors one by one retrying on every failed sector.
   **
   ** Note: a) doesn't return a error status, because it is supposed to never
   **          fail.
   **       b) this routine is written in order to do be able to do
   **          error recovery.
 */

#define RETRIES 3		/* Retry three times before failing. */

void
ReadSectors (int drive, int nsects, int lsect, void *buffer,
	     int bytespersector)
{
  int i, j, success;
  char* bptr;

  if (absread (drive, nsects, lsect, buffer) == 0)
    return;			/* SUCCESS */


  puts ("");
  NLS_PUTSTRING (6, 10, "Media error reading from disk, rescanning...");

  for (i = 0; i < nsects; i++)
    {
      success = FALSE;

      if (absread (drive, 1, lsect, buffer) == -1)
	{
	  for (j = 1; j < RETRIES; j++)		/* already tried once. */
	    {
	      if (absread (drive, 1, lsect, buffer) == 0)
		{
		  success = TRUE;
		  break;
		}
	    }

	  if (!success)
	    {
	      NLS_PRINTSTRING (6, 11, "Unreadable sector at position");
	      printf (" %d.\n", lsect);
	    }
	}

      lsect++;
      bptr = (char *) buffer;
      bptr += bytespersector;
    }
}
