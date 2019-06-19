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
   email me at: imre.leber@worldonline.be

 */

#include <stdio.h>
#include <dos.h>
#include <string.h>
#include <time.h>

#include "nlsaspct.h"
#include "boot.h"

#ifdef HI_TECH_C
#include "pacc\timdat.h"
#endif

CROSSCUT_NLS_DATA_IMPORT

static unsigned char SerialNumber[4];

void
ClearDiskSerialNumber (char *buf)
{
  struct BootSectorStruct *boot = (struct BootSectorStruct *) buf;

  memset (boot->fs.spc1216.SerialNumber, '\0', 4);
}

void
UpdateDiskSerialNumber (char *buf)
{
  struct time theTime;
  struct date theDate;

  struct BootSectorStruct *boot = (struct BootSectorStruct *) buf;
  unsigned first, second;

  gettime (&theTime);
  getdate (&theDate);

  first = ((theTime.ti_hour << 8) + theTime.ti_min) +
    (theDate.da_year + 1980);

  second = ((theTime.ti_sec << 8) + theTime.ti_hund) +
    ((theDate.da_mon << 8) + theDate.da_day);

  memcpy (boot->fs.spc1216.SerialNumber, &first, 2);
  memcpy (((char *) boot->fs.spc1216.SerialNumber) + 2, &second, 2);

  memcpy (SerialNumber, boot->fs.spc1216.SerialNumber, 4);
}

void
PrintDiskSerialNumber (void)
{
  int i;

  NLS_PRINTSTRING (1, 40, "Volume serial number is");
  printf (" ");

  for (i = 3; i >= 0; i--)
    {
      if (i == 1)
   printf ("-");
      printf ("%.2X", SerialNumber[i]);
    }
}