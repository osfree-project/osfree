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

#include <stdio.h>
#include <dos.h>

#ifndef HI_TECH_C
#include <io.h>
#include <fcntl.h>
#endif

#include "drive.h"
#include "parser.h"
#include "scanner.h"
#include "datgen.h"
#include "fastcopy.h"

static int IsDATNewer (char *IniFile, char *DatFile);
static int CloseFiles (int ihandle, int dhandle);
static int WriteDATFile (char *filename, struct IniParserStruct *ParsedData);
static int ReadDATFile (char *filename, struct IniParserStruct *ParsedData);

#define ITEMCOUNT 12

int
ExploreDATFile (struct IniParserStruct *ParsedData)
{
  char IniFile[128];
  char DatFile[128];

  /* See where we need to find the file. */
  GetIniDir ("diskcopy.ini", IniFile);
  GetIniDir ("diskcopy.dat", DatFile);

  if (IsDATNewer (IniFile, DatFile))
    return ReadDATFile (DatFile, ParsedData);
  else
    return FALSE;
}

int
MakeDATFile (struct IniParserStruct *ParsedData)
{
  char IniFile[128];
  char DatFile[128];

  /* See where we need to find the file. */
  GetIniDir ("diskcopy.ini", IniFile);
  GetIniDir ("diskcopy.dat", DatFile);

  if (IsDATNewer (IniFile, DatFile))
    return FALSE;
  else
    return WriteDATFile (DatFile, ParsedData);
}

static int
IsDATNewer (char *IniFile, char *DatFile)
{
  int ihandle, dhandle, result;
  struct ftime itime, dtime;

  ihandle = open (IniFile, O_RDONLY);
  if (ihandle == -1)
    return FALSE;

  dhandle = open (DatFile, O_RDONLY);
  if (dhandle == -1)
    {
      close (ihandle);
      return FALSE;
    }

  if (getftime (ihandle, &itime) == -1)
    return CloseFiles (ihandle, dhandle);
  if (getftime (dhandle, &dtime) == -1)
    return CloseFiles (ihandle, dhandle);

  if (dtime.ft_year > itime.ft_year)
    result = TRUE;
  else if (dtime.ft_year < itime.ft_year)
    result = FALSE;
  else if (dtime.ft_month > itime.ft_month)
    result = TRUE;
  else if (dtime.ft_month < itime.ft_month)
    result = FALSE;
  else if (dtime.ft_day > itime.ft_day)
    result = TRUE;
  else if (dtime.ft_day < itime.ft_day)
    result = FALSE;
  else if (dtime.ft_hour > itime.ft_hour)
    result = TRUE;
  else if (dtime.ft_hour < itime.ft_hour)
    result = FALSE;
  else if (dtime.ft_min > itime.ft_min)
    result = TRUE;
  else if (dtime.ft_min < itime.ft_min)
    result = FALSE;
  else if (dtime.ft_tsec > itime.ft_tsec)
    result = TRUE;
  else
    result = FALSE;

  CloseFiles (ihandle, dhandle);
  return result;
}

static int
CloseFiles (int ihandle, int dhandle)
{
  close (ihandle);
  close (dhandle);
  return FALSE;
}

static int
WriteDATFile (char *filename, struct IniParserStruct *ParsedData)
{
  char buffer[6];
  int handle, result;

  if ((handle = _creat (filename, FA_ARCH)) == -1)
    return FALSE;

  buffer[0] = '!';		/* My initials IL. */
  buffer[1] = 'œ';

  buffer[2] = ITEMCOUNT;	/* 12 items.        */

  /* Mode: 256 posibilities. */
  buffer[3] = (char) ParsedData->mode;

  /* The items. */
  buffer[4] = (ParsedData->UseEMS) ? 1 : 0;
  buffer[4] = buffer[4] + ((ParsedData->UseXMS) ? 2 : 0);
  buffer[4] = buffer[4] + ((ParsedData->UseSWAP) ? 4 : 0);

  buffer[4] = buffer[4] + ((ParsedData->audible) ? 8 : 0);
  buffer[4] = buffer[4] + ((ParsedData->verify) ? 16 : 0);
  buffer[4] = buffer[4] + ((ParsedData->informative) ? 32 : 0);
  buffer[4] = buffer[4] + ((ParsedData->overwrite) ? 64 : 0);
  buffer[4] = buffer[4] + ((ParsedData->autoexit) ? 128 : 0);
  buffer[5] = (ParsedData->askdisk) ? 1 : 0;
  buffer[5] = buffer[5] + ((ParsedData->speed == FULL) ? 2 : 0);
  buffer[5] = buffer[5] + ((ParsedData->asktdisk) ? 4 : 0);
  buffer[5] = buffer[5] + ((ParsedData->serialnumber == UPDATE) ? 8 : 0);

  result = write (handle, buffer, 6) != -1;
  close (handle);

  if (!result)
    remove (filename);		/* Make sure that there is no
				   problem with synchronism.   */
  return result;
}

static int
ReadDATFile (char *filename, struct IniParserStruct *ParsedData)
{
  char buffer[6];
  int handle, result;

  if ((handle = open (filename, O_RDONLY)) == -1)
    return FALSE;

  result = read (handle, buffer, 6) == -1;
  close (handle);
  if (result)
    return FALSE;

  if (buffer[0] != '!')
    return FALSE;
  if (buffer[1] != 'œ')
    return FALSE;
  if (buffer[2] < ITEMCOUNT)
    return FALSE;		/* Old DAT file */

  /* Mode: 256 posibilities. */
  ParsedData->mode = (int) buffer[3];

  /* The items. */
  ParsedData->UseEMS = buffer[4] & 1;
  ParsedData->UseXMS = (buffer[4] & 2) > 0;
  ParsedData->UseSWAP = (buffer[4] & 4) > 0;

  ParsedData->audible = (buffer[4] & 8) > 0;
  ParsedData->verify = (buffer[4] & 16) > 0;
  ParsedData->informative = (buffer[4] & 32) > 0;
  ParsedData->overwrite = (buffer[4] & 64) > 0;
  ParsedData->autoexit = (buffer[4] & 128) > 0;
  ParsedData->askdisk = (buffer[5] & 1) > 0;
  ParsedData->speed = ((buffer[5] & 2) > 0) ? FULL : FAST;
  ParsedData->asktdisk = (buffer[5] & 4) > 0;
  ParsedData->serialnumber = ((buffer[5] & 8) > 0) ? UPDATE : LEAVE;

  return TRUE;
}
