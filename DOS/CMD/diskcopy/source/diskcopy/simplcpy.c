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
   email me at: imre.leber@worlonline.be

 */

#include <stdio.h>
#include <alloc.h>

#ifndef HI_TECH_C
#include <io.h>			/* for access */
#else
#include <stat.h>
#endif

#include "drive.h"
#include "diskcopy.h"

#define FAILSAFE

static int FastCopy (FILE* sfptr, FILE* tfptr, unsigned bufsize, void* buf)
{
  int i;
  long flen;

  if (fseek(sfptr, 0, SEEK_END))
     return FALSE;
     
  if ((flen = ftell(sfptr)) == -1)
     return FALSE;
     
  if (fseek(sfptr, 0, SEEK_SET))
     return FALSE;

  for (i = 0; i < flen / bufsize; i++)
  {
      if (fread(buf, 1, bufsize, sfptr) != bufsize)
	 return FALSE;

      if (fwrite(buf, 1, bufsize, tfptr) != bufsize)
         return FALSE;
  }

  if (fread(buf, 1, flen % bufsize, sfptr) != flen % bufsize)
     return FALSE;
  if (fwrite(buf, 1, flen % bufsize, tfptr) != flen % bufsize)
     return FALSE;

  return TRUE;
}

#ifdef FAILSAFE

static int SlowCopy(FILE* sfptr, FILE* tfptr)
{
  int c;

  while ((c = fgetc (sfptr)) != EOF)
	if (fputc (c, tfptr) == EOF)
	   return FALSE;

  return TRUE;
}

#endif

int
CopyFile (char *source, char *destination)
{
  FILE *sfptr, *tfptr;
  void* buf;
  unsigned memsize;
  int result;

  /* Look if the destination does not already exists. */
#ifndef HI_TECH_C
  if (access (destination, EXISTS) == 0)
    return FALSE;
#else
  if (ffirst(destination) != NULL)
    return FALSE;
#endif

  sfptr = fopen (source, "rb");
  if (sfptr == NULL)
    return FALSE;

  tfptr = fopen (destination, "wb");
  if (tfptr == NULL)
    {
      fclose (sfptr);
      return FALSE;
    }

  memsize = coreleft();
  buf = malloc(memsize);

#ifdef FAILSAFE
  if (!buf)
     result = SlowCopy(sfptr, tfptr);
  else
  {
#endif
     result = FastCopy(sfptr, tfptr, memsize, buf);
     free(buf);
#ifdef FAILSAFE
     if (!result)
     {
	fseek(sfptr, 0, SEEK_SET);
	fseek(tfptr, 0, SEEK_SET);
	result = SlowCopy(sfptr, tfptr);
     }
  }
#endif

  fclose (sfptr);
  fclose (tfptr);

  if (result)
     return TRUE;
  else
  {
     unlink(destination);
     return FALSE;
  }
}
