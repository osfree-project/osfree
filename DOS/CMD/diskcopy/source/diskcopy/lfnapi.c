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

 */

#include <string.h>
#include <stdlib.h>
#ifndef HI_TECH_C
#include <dir.h>
#endif
#include <stdio.h>

#include "io95\io95.h"
#include "lfnapi.h"
#include "drive.h"
#include "diskcopy.h"
#include "critical.h"

static char ShortNames[MAXSFNS][MAXPATH95];
static char LongNames[MAXSFNS][MAXPATH95];

static char scratchpad[MAXPATH95];

static struct LFNAttribute Attributes[MAXSFNS];

static int
IsLFNSupported (char *filename)
{
  int result;

  if (filename && filename[0] && (filename[1] == ':'))
    result = CheckDriveOnLFN (filename[0]);
  else
    result = CheckDriveOnLFN (getdisk () + 'A');

  if (CriticalErrorOccured ())
    return MAYBE;		/* If this caused a reaction, LFN would be supported.
				   Maybe not on the indicated drive though. */
  else
    return result;
}

static void
GenerateUniqueCurrentSFN (char *sfnpath)
{
  char sfn[9] = "AAAAAAAA\0";
  static int counter1 = 0;
  static int counter2 = 0;
  int len = strlen (sfnpath);

  do
    {
      memset (sfn, 'A', 8);
      itoa (counter1++, sfn, 16);
      sfn[strlen (sfn)] = 'A';

      if (counter1 == 32767)
	{
	  counter1 = 0;
	  counter2++;
	}

      itoa (counter2, sfn + 4, 16);
      sfnpath[len] = '\0';
    }
  while (access (strcat (sfnpath, sfn), EXISTS) == 0);
}

static void
GenerateUniqueSFN (char *lfn, char *sfn)
{
  char *p;
  int found = FALSE;

  /* Copy path to sfn. */
  strcpy (sfn, lfn);
  for (p = sfn + strlen (sfn); p != sfn; p--)
    if ((*p == '\\') || (*p == ':'))
      {
	*(p + 1) = '\0';
	found = TRUE;
      }

  if (!found)
    *sfn = 0;

  /* Add imaginary long filename. */
  GenerateUniqueCurrentSFN (sfn);
}

void
InitLFNAPI (void)
{
  int i;

  for (i = 0; i < MAXSFNS; i++)
    {
      ShortNames[i][0] = LongNames[i][0] = '\0';
      Attributes[i].output = FALSE;
    }
}

void
ConvertToSFN (char *lfn, int index)
{
  int supported;
  ShortNames[index][0] = '\0';	/* important */

  supported = IsLFNSupported (lfn);

  if (supported == TRUE)
    {
      if (lfn2sfn95 (lfn, ShortNames[index]) != 0)
	ShortNames[index][0] = '\0';
    }
  else if (supported == MAYBE)
    ShortNames[index][0] = '\0';	/* Don't know wether LFN is supported. */
  else
    strcpy (ShortNames[index], lfn);

  strcpy (LongNames[index], lfn);
}

char *
GetSFN (int index)
{
  int supported;

  if (ShortNames[index][0] == 0)
    {
      /* At this time the disk has to be in the station */
      supported = IsLFNSupported (LongNames[index]);
      if (supported == TRUE)
	{
	  if (LongNames[index][0] != 0)
	    GenerateUniqueSFN (LongNames[index], ShortNames[index]);
	  else
	    return NULL;
	}
      else if (supported == MAYBE)
	return NULL;		/* Disk not in drive */
      else
	strcpy (ShortNames[index], LongNames[index]);	/* LFN not supported after all. */
    }

  return ShortNames[index];
}

void
SetLFNAttribute (struct LFNAttribute *attrib, int index)
{
  memcpy (&Attributes[index], attrib, sizeof (struct LFNAttribute));
}

char *
GetLFN (int index)
{
  return LongNames[index];
}

void
SynchronizeLFNs (void)
{
  int i;

  for (i = 0; i < MAXSFNS; i++)
    {
      if ((LongNames[i] != '\0') && (ShortNames[i] != '\0') &&
	  (IsLFNSupported (LongNames[i]) == TRUE))
	{
	  lfn2sfn95 (LongNames[i], scratchpad);

	  if ((strcmpi (ShortNames[i], scratchpad) != 0) &&
	      (Attributes[i].output))
	    {
	      if (!rename95 (ShortNames[i], LongNames[i]))
		{
		  rename (ShortNames[i], LongNames[i]);
		  strcpy (ShortNames[i], LongNames[i]);
		}
	      else
		lfn2sfn95 (LongNames[i], ShortNames[i]);
	    }
	}
    }
}
