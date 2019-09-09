/* Functions that emulate UNIX catgets */

/* Copyright (C) 2007 Imre Leber <imre. leber @ telenet. be> */

/*
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110, USA
*/

//#include <dir.h>
#include <string.h>
#include <dos.h>

#include "find95.h"
#include "lfnapi.h"
#include "l2s95.h"

#include "tcc2wat.h"

int getdisk(void)
{
        unsigned drive;
        _dos_getdrive(&drive);
        return drive-1;
}

int CheckDriveOnLFN(char drive)
{
    union REGPACK rp;       /* temporary stack for the registers */
    static char strDrive[4];
    static char filesys[32];

    strcpy(strDrive, "?:\\");
    strDrive[0] = drive;

    rp.r_flags = 1;

    rp.r_ax = 0x71A0;

    rp.r_ds = FP_SEG(strDrive);
    rp.r_dx = FP_OFF(strDrive);

    rp.r_es = FP_SEG(filesys);
    rp.r_di = FP_OFF(filesys);

    rp.r_cx = 32;

    intr(0x21, &rp);

    if (rp.r_flags & 1)
        return 0;

    return (rp.r_bx & 16384) > 0;
}

static int
IsLFNSupported (char *filename)
{
  if (filename && filename[0] && (filename[1] == ':'))
    return CheckDriveOnLFN (filename[0]);
  else
    return CheckDriveOnLFN (getdisk () + 'A');
}

int LFNConvertToSFN(char* file)
{
    static char buffer[67];

    if (IsLFNSupported(file))
    {
        if (lfn2sfn95(file, buffer) == 0)
        {
            strcpy(file, buffer);
            return 1;
        }
        else
            return 0;
    }

    return 1;
}

static int lfnSupported;
static struct ffblk95 ff95;
static struct ffblk ff;

int LFNFirstFile(char* wildcard, char* file, char* longfile)
{
    int retVal;

    if (IsLFNSupported(wildcard))
    {
        lfnSupported = 1;

        retVal = findfirst95(wildcard, &ff95, 0);

        if (retVal == 0)
        {
            strcpy(file, ff95.ff_95.ff_shortname);
            strcpy(longfile, ff95.ff_95.ff_longname);
        }

        return retVal;
    }
    else
    {
        lfnSupported = 0;

        retVal = findfirst(wildcard, &ff, 0);

        if (retVal == 0)
        {
            strcpy(file, ff.ff_name);
            strcpy(longfile, ff.ff_name);
        }

        return retVal;
    }
}

int LFNNextFile(char* file, char* longfile)
{
    int retVal;

    if (lfnSupported)
    {
        retVal = findnext95(&ff95);

        if (retVal == 0)
        {
            strcpy(file, ff95.ff_95.ff_shortname);
            strcpy(longfile, ff95.ff_95.ff_longname);
        }

        return retVal;
    }
    else
    {
        retVal = findnext(&ff);

        if (retVal == 0)
        {
            strcpy(file, ff.ff_name);
            strcpy(longfile, ff.ff_name);
        }

        return retVal;
    }
}

void LFNFindStop()
{
   if (lfnSupported)
   {
        findstop95(&ff95);
   }
}

