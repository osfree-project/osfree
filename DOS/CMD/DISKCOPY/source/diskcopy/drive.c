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
   email me at: imre.leber@worldonline.be

   module: drive.c - file with drive functions.

 */

#include <string.h>
#include <ctype.h>
#include <dos.h>

#ifndef HI_TECH_C
#include <dir.h>
#else
#include <sys.h>
#endif

#ifndef HI_TECH_C
#define CURRENT_DRIVE (getdisk() + 'A')
#else
#define CURRENT_DRIVE (*(getdrv()))
#endif

#include "misc.h"
#include "drive.h"

static int
HasAllFloppyForm (char *drive)
{
  int drv;

  if (strlen (drive) != 2)
    return FALSE;

  drv = toupper (drive[0]);
  if ((drv >= 'A') && (drv <= 'Z') && (drive[1] == ':'))
    return TRUE;

  return FALSE;
}

int
HasFloppyForm (char *drive)
{
  return HasAllFloppyForm(drive);
}

int
IsFile (char *x)
{
  return (!HasAllFloppyForm (x) && x[0] != '/' && x[0] != SwitchChar ());
}

int
IsCopyToSameDisk (char *drvorfle1, char *drvorfle2)
{
  /* diskcopy a: a: */
  if (HasAllFloppyForm (drvorfle1) && HasAllFloppyForm (drvorfle2))
    return FALSE;

  /* diskcopy a: a:test.img */
  if (HasAllFloppyForm (drvorfle1))
    return IsCopyToSameDisk (drvorfle2, drvorfle1);

  /* diskcopy a:test.img a: */
  if (HasAllFloppyForm (drvorfle2))
    if (*drvorfle1 && (drvorfle1[1] == ':'))
      /* diskcopy a:test.img a: */
      return toupper (*drvorfle1) == toupper (*drvorfle2);
    else
      /* A:\> diskcopy test.img a: */
      return (CURRENT_DRIVE == toupper (drvorfle2[0]));

  return FALSE;			/* Both are files */
}

char
GetDiskFromPathName (char *path)
{
  if (HasAllFloppyForm (path) || (path && (path[1] == ':')))
    return path[0];
  else
    return tolower(CURRENT_DRIVE);
}
