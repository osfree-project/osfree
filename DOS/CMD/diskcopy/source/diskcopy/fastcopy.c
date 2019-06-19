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

#include <dos.h>
#include <mem.h>
#include <alloc.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>

#include "drive.h"
#include "boot.h"
#include "fastcopy.h"

static int PerformFastCopy = FALSE;

static char far *FatInfo = NULL;	/* FAT information             */
static struct BootSectorStruct bsect;	/* Keep boot sector info handy */

/* Cached values */
static unsigned long DataStart = 0;	/* At least this one is only calculated
					   once                                 */

unsigned long
GetDataStart (void)		/* in sectors */
{
  if (!DataStart)
    DataStart =
      bsect.ReservedSectors +	/* Skip boot sector, */
      (bsect.Fats * bsect.SectorsPerFat) +	/* FAT(s),        */
      (bsect.NumberOfFiles / ENTRIESPERSECTOR);		/* and root dir    */

  return DataStart;
}

unsigned
DataSectorToCluster (unsigned long sector)
{
  unsigned long datastart = GetDataStart ();

  return (unsigned) ((sector + (2 * bsect.SectorsPerCluster) - datastart) /
		     bsect.SectorsPerCluster);
}

static int
IsSectorFilled (unsigned long sector)	/* Remember it is for FAT12 */
 {
  unsigned cluster1, label;
  unsigned cluster = DataSectorToCluster (sector);

  cluster1 = cluster + (cluster >> 1);	/* multiply cluster * 1.5 */
  label = *((unsigned far *) (FatInfo + cluster1));

  if ((cluster & 1) == 0)	/* If cluster is even */
    label &= 0x0fff;
  else
    label &= 0xfff0;

  return (label != 0);
}

/* Return the meaningfull bytes on the disk */
unsigned long
GetDiskFilledSize (unsigned blocksize)
{
  unsigned sectorsperblock, j;
  unsigned long count = 0, blocks, i;

  if (!PerformFastCopy)
    return (unsigned long) bsect.NumberOfSectors * bsect.BytesPerSector;

  /* Calculate the number of blocks in the volume */
  blocks = ((unsigned long) bsect.NumberOfSectors * bsect.BytesPerSector) / blocksize;

  /* Calculate the number of sectors per block of 32Kb */
  sectorsperblock = blocksize / bsect.BytesPerSector;

  /* Count the number of filled blocks */

  /* Take into account system area of disk */
  count = (GetDataStart () / sectorsperblock) +
    ((GetDataStart () % sectorsperblock) != 0);

  /* Data area */
  for (i = count; i < blocks; i++)
    for (j = 0; j < sectorsperblock; j++)
      if (IsSectorFilled (i * sectorsperblock + j))
	{
	  count++;
	  break;
	}

  return count * blocksize;
}

int
IsSectorUsed (unsigned long sector)
{
  if (!PerformFastCopy)
    return TRUE;		/* Always copy in full method */

  if (sector <= GetDataStart ())
    return TRUE;

  return IsSectorFilled (sector);
}

static int
FileReader (int handle, int nsects, long lsect, void *buffer)
{
  int size = nsects * BYTESPERSECTOR;

  lseek (handle, lsect * BYTESPERSECTOR, SEEK_SET);
  return read (handle, buffer, size) != size;	/* Return 0 on success */
}

static int
ReadBootInfo (int drive, struct dfree *free,
	      int (*readfunc) (int drive, int nsects, long lsect,
			       void *buffer))
{
  if (readfunc (drive, 1, 0, (void *) &bsect) != 0)
    return FALSE;

  free->df_total = bsect.NumberOfSectors / bsect.SectorsPerCluster;
  free->df_bsec = BYTESPERSECTOR;	/* bsect.BytesPerSector HAS TO BE 512 */
  free->df_sclus = bsect.SectorsPerCluster;

  return TRUE;
}

int
DiskReadBootInfo (int drive, struct dfree *free)
{
  return ReadBootInfo (drive, free, absread);
}

int
FileReadBootInfo (char *file, struct dfree *free)
{
  int handle, result;

  handle = open (file, O_RDONLY | O_BINARY);
  if (handle == -1)
    return FALSE;

  result = ReadBootInfo (handle, free, FileReader);

  close (handle);

  return result;
}

static int
ReadFatInfo (int driveorfile,
	     int (*readfunc) (int drive, int nsects, long lsect,
			      void *buffer))
{
  int i;
  char buf[512];
  unsigned long FirstFatSector;
  unsigned segment, offset;

  /* Allocate memory for fat info (far heap) */
  ReleaseFatInfo ();		/* But first release any previously allocated memory. */
  FatInfo = farmalloc ((unsigned long) bsect.SectorsPerFat * BYTESPERSECTOR);
  if (!FatInfo)
    return FALSE;

  /* Read FAT in memory */
  FirstFatSector = bsect.ReservedSectors;	/* FAT starts right after boot */
  segment = FP_SEG (FatInfo);
  offset = FP_OFF (FatInfo);
  segment = segment + (offset / 16);
  offset = offset % 16;

  for (i = 0; i < bsect.SectorsPerFat; i++)
    {
      if (readfunc (driveorfile, 1, FirstFatSector + i, (void *) buf) != 0)
	{
	  farfree (FatInfo);
	  FatInfo = NULL;
	  return FALSE;
	}
      movedata (FP_SEG (buf), FP_OFF (buf),
		segment, offset, BYTESPERSECTOR);
      segment += BYTESPERSECTOR / 16;
    }

  return TRUE;
}

int
FileReadFatInfo (char *file)
{
  int handle, result;

  handle = open (file, O_RDONLY | O_BINARY);
  if (handle == -1)
    return FALSE;

  result = ReadFatInfo (handle, FileReader);

  close (handle);

  return result;
}

int
DiskReadFatInfo (int drive)
{
  return ReadFatInfo (drive, absread);
}

void
SetCopySpeed (int method)
{
  PerformFastCopy = (method == FAST);
}

int
IsDiskReadRequired (unsigned long beginsector, int amount)
{
  int i;

  if (!PerformFastCopy)
    return TRUE;		/* Always copy in full method */

  /* Always copy system area of disk */
  if (beginsector <= GetDataStart ())
    return TRUE;

  for (i = 0; i < amount; i++)
    if (IsSectorFilled (beginsector + i))
      return TRUE;

  return FALSE;			/* Don't read sector from disk */
}

void
ReleaseFatInfo (void)
{
  if (FatInfo)
    farfree (FatInfo);
}

/************************************************************
**                Determine FAT type
*************************************************************
** Returns the kind of FAT being used.
**
** Notice that the previous function returns the FAT
** determination string the value returned there is only
** informative and has no real value. The type of FAT is ONLY
** determined by the cluster number.
*************************************************************/
int DetermineFATType(void)
{
    unsigned long RootDirSectors, FatSize, TotalSectors, DataSector;
    unsigned long CountOfClusters;
    
    RootDirSectors = ((bsect.NumberOfFiles * 32) +
                      (bsect.BytesPerSector-1)) /
                      (bsect.BytesPerSector);
                      
    if (bsect.SectorsPerFat != 0)
        FatSize = bsect.SectorsPerFat;
    else
        FatSize = bsect.fs.spc32.SectorsPerFat;
        
    if (bsect.NumberOfSectors != 0)
       TotalSectors = bsect.NumberOfSectors;
    else
       TotalSectors = bsect.NumberOfSectors32;
       
    DataSector = TotalSectors - 
                     (bsect.ReservedSectors + 
                          (bsect.Fats * FatSize) +
                             RootDirSectors);
                             
   CountOfClusters = DataSector / bsect.SectorsPerCluster;
   
   if (CountOfClusters < 4085)
   {
      return FAT12;
   }
   else if (CountOfClusters < 65525L)
   {
      return FAT16;
   }
   else
   {
      return FAT32;
   }    
}