/*
   DISKCOPY.EXE, floppy diskette duplicator similar to MSDOS Diskcopy.
   Copyright (C) 1998, Matthew Stanford.
   Copyright (C) 1999, 2000, 2001,  Imre Leber.

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

 */

#include <bios.h>
#include <dos.h>
#include <ctype.h>
#include <stdio.h>
#include <conio.h>
#include <stdlib.h>
#include <string.h>

#include "diskcopy.h"
#include "tdrvcpy.h"
#include "memtypes.h"
#include "waitfinp.h"
#include "drive.h"
#include "fastcopy.h"
#include "nlsaspct.h"
#include "parser.h"
#include "serialnm.h"

CROSSCUT_NLS_DATA_IMPORT

void
TwoDriveCopy (struct TwoDriveCopyData *pars)
{
  int answer, action, success;
  struct dfree dskfree;
  struct dfree dskfreed;
  int sector;
  int sectorsperblock;

  unsigned long disksize, endsector, total, FilledDiskSize, FilledTotal;

  answer = CatYES;

  while (answer == CatYES)
    {
      if (pars->askdisk)
	{
	  puts ("");
	  NLS_PRINTSTRING (7, 0, "Put source diskette in drive");
	  printf (" %c:\n", pars->sourcedrv + 'A');
	  NLS_PRINTSTRING (7, 1, "Put destination diskette in drive");
	  printf (" %c:\n\n", pars->destdrv + 'A');
	  NLS_PRINTSTRING (7, 2, "Press any key to continue...");
	  WaitForInput ();
	}

      pars->askdisk = TRUE;

      for (;;)
	{
	  if (!DiskReadBootInfo (pars->sourcedrv, &dskfree))
	    {
	      puts ("");
	      NLS_PUTSTRING (7, 3, "Invalid src drive specification or non removable media.");
	      exit (INITERROR);
	    }

	  total = dskfree.df_total * dskfree.df_sclus;
	  disksize = total * dskfree.df_bsec;

	  if (disksize == 0)	/* If any of the numbers == 0 */
	    {
	      puts ("");
	      NLS_PUTSTRING (7, 4, "Invalid dst drive specification or non removable media.");
	      exit (INITERROR);
	    }

	  if (!DiskReadBootInfo (pars->destdrv, &dskfreed))
	    {
	      puts ("");
	      NLS_PUTSTRING (7, 4, "Invalid dst drive specification or non removable media.");
	      exit (INITERROR);
	    }

	  if ((dskfree.df_total == dskfreed.df_total) &&
	      (dskfree.df_bsec == dskfreed.df_bsec) &&
	      (dskfree.df_sclus == dskfreed.df_sclus))
	    break;

	  puts ("\n");

	  NLS_PRINTSTRING (7, 5, "Diskette does not have the same capacity as the original.");
	  NLS_PRINTSTRING (8, 0, "Put a diskette with the right capacity in drive");
	  printf (" %c:", pars->sourcedrv + 'A');
	  NLS_PRINTSTRING (8, 1, "or");
	  printf ("%c:\n", pars->destdrv + 'A');
	  NLS_PUTSTRING (8, 2, "or press CTRL-C to cancel.");
	  WaitForInput ();
	}

      if (pars->copyfast)
	{
	  SetCopySpeed (FAST);
	  if (!DiskReadFatInfo (pars->sourcedrv))
	    SetCopySpeed (FULL);
	}
      else
	SetCopySpeed (FULL);

      FilledDiskSize = GetDiskFilledSize (pars->bufsize);
      FilledTotal = (FilledDiskSize / dskfree.df_bsec) / dskfree.df_sclus;

      puts ("\n");

      NLS_PRINTSTRING (9, 0, "Copying");
      printf (" %d ", FilledTotal);
      NLS_PRINTSTRING (9, 1, "clusters");
      printf (", %d ", dskfree.df_sclus);
      NLS_PRINTSTRING (9, 2, "sectors per cluster");
      printf (", %d ", dskfree.df_bsec);
      NLS_PUTSTRING (9, 3, "bytes per sector.");
      NLS_PRINTSTRING (9, 4, "Relevant drive size is");
      printf (" %ld ", FilledDiskSize);
      NLS_PUTSTRING (9, 5, "bytes.");

      sectorsperblock = pars->bufsize / BYTESPERSECTOR;
      endsector = dskfree.df_total * dskfree.df_sclus;

      for (action = DISKWRITING;
	(pars->verify) ? (action <= VERIFICATION) : (action < VERIFICATION);
	   action++)
	{
	  puts ("");
	  if (action == DISKWRITING)
	    {
	      NLS_PRINTSTRING (9, 6, "Copying . . .");
	    }
	  else
	    {
	      NLS_PRINTSTRING (9, 7, "Verifying . . .");
	    }

	  for (sector = 0; sector < endsector; sector += sectorsperblock)
	    {
	      if (IsDiskReadRequired (sector, sectorsperblock))
		{
#ifndef ALL_RECOVERY_MODE
		  if (pars->recoverymode)
#endif
		    ReadSectors (pars->sourcedrv, sectorsperblock, sector, pars->cpybuf,
				 dskfree.df_bsec);
#ifndef ALL_RECOVERY_MODE
		  else if (absread (pars->sourcedrv, sectorsperblock, sector, pars->cpybuf) != 0)
		    {
		      puts ("");
		      NLS_PRINTSTRING (7, 7, "Media error reading from sector");
		      printf (" %d\n", sector);
		    }
#endif

#ifdef UPDATE_SERIALNUM
		  if (sector == 0)
		    {
		      if (action == DISKWRITING)
			{
			  if (pars->updateserial == UPDATE)
			    UpdateDiskSerialNumber (pars->cpybuf);
			}
		      else
			{
			  if (pars->updateserial == UPDATE)
			    {
			      ClearDiskSerialNumber (pars->cpybuf);
			    }
			}
		    }
#endif

		  if (action == VERIFICATION)
		    {
		      success = absread (pars->destdrv, sectorsperblock, sector, pars->cmpbuf) == 0;

#ifdef UPDATE_SERIALNUM
		      if ((sector == 0) && (pars->updateserial == UPDATE))
			ClearDiskSerialNumber (pars->cmpbuf);
#endif
		    }
		  else
		    success = abswrite (pars->destdrv, sectorsperblock, sector, pars
					->cpybuf) == 0;


		  if (!success)
		    {
		      puts ("");
		      NLS_PRINTSTRING (7, 8, "Media error writing to sector");
		      printf (" %d\n", sector);
		    }

		  if (action == VERIFICATION)
		    if (memcmp (pars->cpybuf, pars->cmpbuf, sectorsperblock * BYTESPERSECTOR) != 0)
		      {
			puts ("");
			NLS_PRINTSTRING (1, 37, "Compare error on sector");
			printf (" %ld.\n", sector);
		      }
		}
	    }
	}

      if (pars->bel)
	Beep ();

      answer = (pars->fallthrough) ? CatNO : CatNO + 1;
      if (answer == CatYES)
	answer++;

      while (!NLS_TEST_YES_NO (answer))
	{
	  puts ("\n");
	  NLS_PRINTSTRING (7, 9, "Do you want to copy two other diskettes (y/n)?");
	  answer = toupper (getch ());
	  puts ("");
	}

#ifdef UPDATE_SERIALNUM
      if (pars->updateserial == UPDATE)
	{
	  puts ("");
	  PrintDiskSerialNumber ();
	  puts ("");
	}
#endif

    }

  if (pars->fallthrough)
    puts ("");
}

int
BiosReportsTwoDrives ()
{
  int configuration = biosequip ();

  if ((configuration & 1) &&	/* At least one floppy drive. */
      (((configuration >> 6) & 3) >= 1))
    return 1;
  else
    return 0;
}
