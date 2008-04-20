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
   email me at:  imre.leber@worldonline.be

 */

#include <conio.h>
#include <ctype.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>

#ifndef HI_TECH_C
#include <io.h>
#include <fcntl.h>
#endif

#include "drive.h"
#include "diskcopy.h"
#include "memtypes.h"
#include "misc.h"
#include "waitfinp.h"
#include "tdrvcpy.h"
#include "parser.h"
#include "exepath.h"
#include "fastcopy.h"
#include "smdskcpy.h"
#include "critical.h"
#include "nlsaspct.h"
#include "lfnapi.h"
#include "simplcpy.h"
#include "serialnm.h"

int SavedCBreak;

CROSSCUT_NLS_DATA

int
main (int argc, char *argv[])
{
  int memkind, handle;
  char loop1, loop2;
  struct dfree free;
  char buf[BUFFERSIZE];
  char cmpbuf[BUFFERSIZE];
  unsigned long sector, total, disksize, allocated, beginsector, endsector;
  unsigned long FilledDiskSize, FilledTotal, FileSize = 0;
  int Arg1IsFile, Arg2IsFile;
  int UseImageFile, ImageModus, Overwrite;
  int i, enable;
  int audible, verify, fallthrough, askdisk, asktdisk;
  char sdrive, tdrive;
  int HardDiskOk;
  int informative, copyfast;
  char switchchar;
  int tocopysectors;
  unsigned tobuffer;
#ifndef ALL_RECOVERY_MODE
  int recoverymode;
#endif
  int bytespersector;
  int CopyingToSameDisk = FALSE;
  int action, success;
  struct LFNAttribute attr;

  struct IniParserStruct *ParsedData;
  struct TwoDriveCopyData tdrvdata;

  attr.output = TRUE;

  memkind = UNALLOCATED;
  UseImageFile = FALSE;

  SetExePath (argv[0]);		/* Remember executable's path.     */
  CriticalHandlerOn ();		/* Install critical handler.       */
  InitLFNAPI ();		/* Initialise LFN API.             */

  CROSSCUT_NLS_OPEN

    switchchar = SwitchChar ();

  /* Check arguments */
  if ((argc == 2) && ((argv[1][0] == '/') || (argv[1][0] == switchchar)) &&
      (argv[1][1] == '?') && (argv[1][2] == '\0'))
    {
      ShowHelp (switchchar);
      return COPYSUCCESS;
    }

  if (argc > 2)
    {
      Arg1IsFile = IsFile (argv[1]);
      Arg2IsFile = IsFile (argv[2]);
    }

#ifdef DEF_HELP
  if (argc < 3)
    {
      /* The way we do it.                */
      ShowHelp (switchchar);
      return INITERROR;
    }
  else if ((!HasFloppyForm (argv[1]) && !Arg1IsFile) ||
	   (!HasFloppyForm (argv[2]) && !Arg2IsFile))
#else
  if ((argc < 3) || (!HasFloppyForm (argv[1]) && !Arg1IsFile)
      || (!HasFloppyForm (argv[2]) && !Arg2IsFile))
#endif
    {
      /* The way (DR and MS) DOS does it. */
      NLS_PUTSTRING (1, 0, "Invalid drive specification or non removable media.");
      return INITERROR;
    }

  if (ParseIniFile (NULL) < 0)	/* Parse .ini file. */
    {
      NLS_PUTSTRING (1, 1, "Error reading configuration file.");
      return INITERROR;
    }

  ParsedData = GetParsedData ();

  audible = ParsedData->audible;
  verify = ParsedData->verify;
  HardDiskOk = ParsedData->UseSWAP;
  informative = ParsedData->informative;
  Overwrite = ParsedData->overwrite;
  fallthrough = ParsedData->autoexit;
  askdisk = ParsedData->askdisk;
  asktdisk = ParsedData->asktdisk;
  copyfast = (ParsedData->speed == FAST);
#ifndef ALL_RECOVERY_MODE
  recoverymode = ParsedData->mode;
#endif

  /* Check arguments. */
  for (i = 3; i < argc; i++)
    {
      if (strlen (argv[i]) > 3)
	{
	  NLS_PRINTSTRING (1, 2, "Invalid switch:");
	  printf (" %s\n", argv[i]);
	  return INITERROR;
	}

      enable = TRUE;
      if (strlen (argv[i]) == 3)
	{
	  if (argv[i][2] == '-')
	    {
	      enable = FALSE;
	    }
	  else
	    {
	      NLS_PRINTSTRING (1, 2, "Invalid switch");
	      printf (" %s\n", argv[i]);
	      return INITERROR;
	    }
	}

      if ((argv[i][0] == switchchar) || (argv[i][0] == '/'))
	switch (argv[i][1])
	  {
	  case 'a':		/* DOS is case insensitive. */
	  case 'A':
	    audible = enable;
	    break;
	  case 'v':
	  case 'V':
	    verify = enable;
	    break;
	  case 'm':
	  case 'M':
	    HardDiskOk = !enable;
	    break;
	  case 'i':
	  case 'I':
	    informative = enable;
	    break;
	  case 'o':
	  case 'O':
	    Overwrite = enable;
	    break;
	  case 'x':
	  case 'X':
	    fallthrough = enable;
	    break;
	  case 'd':
	  case 'D':
	    askdisk = !enable;
	    break;

	  case 'f':
	  case 'F':
	    copyfast = enable;
	    break;

#ifndef ALL_RECOVERY_MODE
	  case 'r':
	  case 'R':
	    recoverymode = enable;
	    break;
#else
	  case 'r':
	  case 'R':
#endif
	  case 't':
	  case 'T':
	    asktdisk = !enable;
	    break;

	  case '1':
	    NLS_PRINTSTRING (2, 0, "Warning: option");
	    printf (" %s ", argv[i]);
	    NLS_PUTSTRING (2, 1, "doesn't do anything!");
	    break;

	  default:
	    NLS_PRINTSTRING (1, 2, "Invalid switch");
	    printf (" %s\n", argv[i]);
	    return INITERROR;
	  }
      else
	{
	  NLS_PRINTSTRING (1, 3, "Too many parameters:");
	  printf (" %s\n", argv[i]);
	  return INITERROR;
	}
    }

  if (Arg1IsFile && Arg2IsFile)
    {
      ConvertToSFN (argv[1], INFILE_ID);
      ConvertToSFN (argv[2], OUTFILE_ID);

      SetLFNAttribute (&attr, OUTFILE_ID);

      if ((access (GetSFN (OUTFILE_ID), EXISTS) == 0) &&
	  Overwrite &&
	  (remove (GetSFN (OUTFILE_ID)) == -1))
	{
	  NLS_PUTSTRING (1, 4, "File is write protected!");
	  return INITERROR;
	}

      if (CopyFile (GetSFN (INFILE_ID), GetSFN (OUTFILE_ID)))
	{
	  printf ("%s ", argv[1]);
	  NLS_PRINTSTRING (1, 5, "copied to");
	  printf (" %s", argv[2]);
	  if (audible)
	    Beep ();

	  SynchronizeLFNs ();
	  return COPYSUCCESS;
	}
      else
	{
	  NLS_PRINTSTRING (3, 0, "Problem copying");
	  printf (" %s ", argv[1]);
	  NLS_PRINTSTRING (3, 1, "to");
	  printf (" %s.\n", argv[2]);
	  return CRITICAL;
	}
    }
  else if (Arg1IsFile)
    {
      ConvertToSFN (argv[1], INFILE_ID);

      if (access (GetSFN (INFILE_ID), READPERMISSION) != 0)
	{
	  NLS_PRINTSTRING (1, 6, "File not found:");
	  printf (" %s\n", argv[1]);
	  return INITERROR;
	}

      CopyingToSameDisk = IsCopyToSameDisk (argv[1], argv[2]);
      UseImageFile = TRUE;
      ImageModus = READIMAGE;
    }
  else if (Arg2IsFile)
    {
      CopyingToSameDisk = IsCopyToSameDisk (argv[2], argv[1]);

      ConvertToSFN (argv[2], OUTFILE_ID);

      SetLFNAttribute (&attr, OUTFILE_ID);

      if (!CopyingToSameDisk)
	{
	  if (access (GetSFN (OUTFILE_ID), EXISTS) == 0)
	    {
	      if (Overwrite)
		{
		  if (remove (GetSFN (OUTFILE_ID)) != 0)
		    {
		      NLS_PRINTSTRING (1, 4, "File is write protected!");
		      return INITERROR;
		    }
		}
	      else
		{
		  NLS_PRINTSTRING (1, 7, "File already exists!");
		  return INITERROR;
		}
	    }
	}

      if (CopyingToSameDisk)
	copyfast = TRUE;
      UseImageFile = TRUE;
      ImageModus = WRITEIMAGE;
    }

  sdrive = (char) toupper (argv[1][0]) - 65;
  tdrive = (char) toupper (argv[2][0]) - 65;

  loop1 = CatYES;		/* initialize loop1         */
  loop2 = CatYES;		/* initialize loop2         */

  ctrlbrk (OnCBreak);
  SavedCBreak = getcbrk ();
  setcbrk (1);			/* set control-break to ON  */
  atexit (OnExit);		/* make sure that all allocated memory is released when
				   program stops. */

  if ((sdrive != tdrive) && (!UseImageFile) && BiosReportsTwoDrives ())
    {
      tdrvdata.sourcedrv = sdrive;
      tdrvdata.destdrv = tdrive;
      tdrvdata.cpybuf = buf;
      tdrvdata.cmpbuf = cmpbuf;
      tdrvdata.bufsize = BUFFERSIZE;
      tdrvdata.bel = audible;
      tdrvdata.fallthrough = fallthrough;
      tdrvdata.askdisk = askdisk;
      tdrvdata.copyfast = copyfast;
      tdrvdata.verify = verify;
#ifndef ALL_RECOVERY_MODE
      tdrvdata.recoverymode = recoverymode;
#endif
#ifdef UPDATE_SERIALNUM
      tdrvdata.updateserial = ParsedData->serialnumber;
#endif

      TwoDriveCopy (&tdrvdata);

      return COPYSUCCESS;
    }

  endsector = 0;
  while (!NLS_TEST_NO (loop1))	/* loop1 */
    {
      if (!UseImageFile || (ImageModus == WRITEIMAGE))
	{
	  if (askdisk)
	    {
	      puts ("");
	      NLS_PRINTSTRING (1, 8, "Insert SOURCE diskette into drive");
	      printf (" %s\n\n", argv[1]);
	      NLS_PRINTSTRING (1, 9, "Press any key to continue . . .");
	      WaitForInput ();
	      puts ("");
	    }
	  else
	    askdisk = TRUE;	/* Always ask after the first run. */
	}

      if (endsector == 0)
	{
	  if (!UseImageFile || (ImageModus == WRITEIMAGE))
	    {
	      if (!DiskReadBootInfo (sdrive, &free) ||
                  (DetermineFATType() != FAT12))
		{
		  puts ("");
		  NLS_PUTSTRING (1, 10, "Disk not ready!");
		  return CRITICAL;
		}
	    }
	  else
	    {
	      /* Remember file size */
	      handle = open (GetSFN (INFILE_ID), O_RDONLY | O_BINARY);
	      if (handle < 0)
		{
		  puts ("");
		  NLS_PUTSTRING (1, 11, "Unable to open image file.");
		  return INITERROR;
		}
	      FileSize = filelength (handle);
	      close (handle);

	      if (!FileReadBootInfo (GetSFN (INFILE_ID), &free))
		{
		  puts ("");
		  NLS_PUTSTRING (1, 11, "Unable to open image file.");
		  return INITERROR;
		}
	    }

	  total = free.df_total * free.df_sclus;
	  bytespersector = free.df_bsec;
	  disksize = total * bytespersector;

	  if (disksize == 0)
	    {
	      puts ("");
	      NLS_PUTSTRING (1, 0, "Invalid drive specification or non removable media.");
	      return INITERROR;
	    }

	  /* Initialize fast copy:
	     - After checking if disk in drive is ok, BUT
	     - before initializing memory                  */
	  /*
	     In case of writing an image file to disk. We'd like the
	     program to succeed regardless of wether we used the
	     /f switch or corresponding .ini file entry             */
	  if (((copyfast) ||
	       ((UseImageFile && (ImageModus == READIMAGE)) &&
		disksize != FileSize)) &&
	      (disksize != FileSize))
	    {
	      SetCopySpeed (FAST);

	      if (!UseImageFile || (ImageModus == WRITEIMAGE))
		{
		  if (!DiskReadFatInfo (sdrive))
		    SetCopySpeed (FULL);
		}
	      else if (!FileReadFatInfo (GetSFN (INFILE_ID)))
		{
		  NLS_PUTSTRING (1, 32, "Can not copy image file");
		  return INITERROR;
		}
	    }
	  else
	    SetCopySpeed (FULL);
	}

      puts ("");

      /* Get the size of the meaningfull data on the disk */
      FilledDiskSize = GetDiskFilledSize (BUFFERSIZE);
      FilledTotal = (FilledDiskSize / free.df_bsec) / free.df_sclus;

      if ((UseImageFile) && (!CopyingToSameDisk))
	switch (SetImageFile (GetSFN (ImageModus - 1), ImageModus, FilledDiskSize))
	  {
	  case EZERO:
	    allocated = disksize;
	    break;

	  case DISKTOSMALL:
	    puts ("");
	    NLS_PUTSTRING (1, 12, "Not enough disk space on target drive!");
	    return INITERROR;

	  default:
	    NLS_PRINTSTRING (1, 13, "Error accessing image file:");
	    printf (" %s\n", argv[ImageModus]);
	    return INITERROR;
	  }
      else
	{
	  if (CopyingToSameDisk)
	    switch (ImageModus)
	      {
	      case READIMAGE:
		memkind = InitializeFittingMemory (FilledDiskSize, HardDiskOk, &allocated,
						   '\0', argv[2][0]);
		break;
	      case WRITEIMAGE:
		memkind = InitializeFittingMemory (FilledDiskSize, HardDiskOk, &allocated,
						   argv[1][0], '\0');
		break;
	      }
	  else
	    memkind = InitializeFittingMemory (FilledDiskSize, HardDiskOk, &allocated,
					       argv[1][0], argv[2][0]);

	  if (memkind == 0)
	    {
	      NLS_PUTSTRING (1, 14, "Insufficient memory for disk copy.\n");
	      return INITERROR;
	    }
	}

      if (CopyingToSameDisk)
	{
	  if (allocated != FilledDiskSize)
	    {
	      NLS_PUTSTRING (1, 14, "Insufficient memory for disk copy.\n");
	      return INITERROR;
	    }

	  if (ImageModus == READIMAGE)
	    {
	      PrepareForWriting ();
	      if (!ReadFileIntoMemory (GetSFN (INFILE_ID), buf, BUFFERSIZE))
		{
		  NLS_PUTSTRING (1, 33, "Problem reading image file.");
		  return CRITICAL;
		}
	    }
	  else
	    /* ImageModus == WRITEIMAGE */ if (!asktdisk)
	    {
	      if (!DiskLargeEnough (GetSFN (OUTFILE_ID), FilledDiskSize))
		{
		  puts ("");
		  NLS_PUTSTRING (1, 12, "Not enough disk space on target drive!");
		  return INITERROR;
		}
	    }
	}

      if (!UseImageFile || (ImageModus == WRITEIMAGE))
	{
	  beginsector = endsector;
	  endsector += allocated / BYTESPERSECTOR;
	  if ((endsector > total) || (allocated == FilledDiskSize))
	    endsector = total;
	}

      if (!UseImageFile || (ImageModus == WRITEIMAGE))
	{
	  NLS_PRINTSTRING (4, 0, "Copying");
	  printf (" %d ", (int) FilledTotal);
	  NLS_PRINTSTRING (4, 1, "clusters");
	  printf (", %d ", free.df_sclus);
	  NLS_PRINTSTRING (4, 2, "sectors per cluster");
	  printf (", %d ", free.df_bsec);
	  NLS_PRINTSTRING (4, 3, "bytes per sector");
	  puts (".");
	  NLS_PRINTSTRING (4, 4, "Relevant drive size is");
	  printf (" %lu ", FilledDiskSize);
	  NLS_PRINTSTRING (4, 5, "bytes");
	  printf (".");
	}

      if (informative && (!UseImageFile))
	{
	  printf (" ");
	  NLS_PRINTSTRING (1, 15, "Using");
	  printf (" ");

	  switch (memkind)
	    {
	    case EMS:
	      puts ("EMS.\n");
	      break;
	    case XMS:
	      puts ("XMS.\n");
	      break;
	    case HARDDISK:
	      NLS_PUTSTRING (1, 16, "temporary file");
	      puts ("");
	      break;
	    case BUFFERS:
	      NLS_PRINTSTRING (5, 0, "buffer of");
	      printf (" %ld ", allocated);
	      NLS_PUTSTRING (5, 1, "bytes.");
	      puts ("");
	      break;
	    }
	}
      else if (!UseImageFile || (ImageModus == WRITEIMAGE))
	puts ("\n");

      if (!UseImageFile || (ImageModus == WRITEIMAGE))
	{

	  for (action = DISKREADING;
	       (verify) ? (action <= VERIFICATION) : (action < VERIFICATION);
	       action++)
	    {
	      if (action == DISKREADING)
		{
		  if (!UseImageFile)
		    {
		      NLS_PRINTSTRING (1, 17, "Reading SOURCE diskette . . .");
		    }
		  else
		    NLS_PRINTSTRING (1, 34, "Creating image file . . .");

		  PrepareForWriting ();
		}
	      else
		{
		  puts ("");
		  NLS_PRINTSTRING (1, 38, "Verifying . . .");

		  PrepareForReading ();
		}

	      for (sector = beginsector; sector < endsector;
		   sector = sector + TOCOPYSECTORS)
		{
		  if (sector < endsector - TOCOPYSECTORS)
		    tocopysectors = TOCOPYSECTORS;
		  else
		    tocopysectors = (int) (endsector - sector);
		  tobuffer = (unsigned) (tocopysectors * BYTESPERSECTOR);

		  if (IsDiskReadRequired (sector, tocopysectors))	/* Fast copy */
		    {
#ifndef ALL_RECOVERY_MODE
		      if (recoverymode)
#endif

			ReadSectors (sdrive, tocopysectors, (int) sector, buf,
				     bytespersector);
#ifndef ALL_RECOVERY_MODE
		      else if (absread (sdrive, tocopysectors, (int) sector, buf) != 0)
			{
			  puts ("");
			  NLS_PRINTSTRING (1, 18, "Media error reading from sector");
			  printf (" %ld.\n", sector);
			}
#endif
		      if (action == DISKREADING)
			success = WriteMemoryBlock (buf, tobuffer);
		      else
			success = ReadMemoryBlock (cmpbuf, tobuffer);

		      if (!success)
			{
			  puts ("");
			  NLS_PUTSTRING (1, 20, "Unsuspected memory error.");
			  SetErrorStopped ();
			  return CRITICAL;
			}

		      if (action == VERIFICATION)
			{
			  if (memcmp (buf, cmpbuf, 5 /*tobuffer */ ) != 0)
			    {
			      puts ("");
			      NLS_PRINTSTRING (1, 37, "Compare error on sector");
			      printf (" %ld.\n", sector);
			    }
			}
		    }
		}
	    }
	}

      if (audible && (!UseImageFile))
	Beep ();

      if (!UseImageFile || (ImageModus == READIMAGE))
	{
	  while (!NLS_TEST_NO (loop2))	/* loop2 */
	    {
	      if (askdisk)
		{
		  if (!UseImageFile)
		    puts ("\n");
		  NLS_PRINTSTRING (1, 21, "Insert TARGET diskette into drive");
		  printf (" %s\n\n", argv[2]);
		  NLS_PRINTSTRING (1, 9, "Press any key to continue . . .");
		  WaitForInput ();
		}
	      else
		askdisk = TRUE;

	      /* Check disk capacity is the same as that of the original
	         diskette. */
	      for (;;)
		{
		  if (!DiskReadBootInfo (tdrive, &free))
		    {
		      puts ("");
		      NLS_PUTSTRING (1, 10, "Disk not ready!");
		      total = 0;
		    }
		  else
		    total = free.df_total * free.df_sclus;

		  if (((UseImageFile) && (FileSize != FilledDiskSize)) ||
		      (disksize != total * free.df_bsec))
		    {
		      puts ("");
		      NLS_PUTSTRING (1, 22, "Diskette does not have the same capacity as the original.");

		      if (fallthrough)
			return NONFATAL;

		      puts ("");
		      NLS_PRINTSTRING (1, 23, "Put a diskette with the right capacity in drive");
		      printf (" %s, \n", argv[2]);
		      NLS_PUTSTRING (1, 24, "or press CTRL-C to cancel.");
		      WaitForInput ();	/* When the user presses CTRL-C this function does not return */
		    }
		  else
		    break;
		}

	      if (UseImageFile)
		{
		  puts ("\n");
		  NLS_PRINTSTRING (4, 0, "Copying");
		  printf (" %d ", (int) FilledTotal);
		  NLS_PRINTSTRING (4, 1, "clusters");
		  printf (", %d ", free.df_sclus);
		  NLS_PRINTSTRING (4, 2, "sectors per cluster");
		  printf (", %d ", free.df_bsec);
		  NLS_PRINTSTRING (4, 3, "bytes per sector");
		  puts (".");
		  NLS_PRINTSTRING (4, 4, "Relevant drive size is");
		  printf (" %lu ", FilledDiskSize);
		  NLS_PRINTSTRING (4, 5, "bytes");
		  puts (".");

		  beginsector = endsector;
		  endsector += allocated / BYTESPERSECTOR;
		  if (endsector > total)
		    endsector = total;
		}

	      for (action = DISKWRITING;
	      (verify) ? (action <= VERIFICATION) : (action < VERIFICATION);
		   action++)
		{
		  if (action == DISKWRITING)
		    {
		      if (UseImageFile)
			{
			  puts ("");
			  NLS_PRINTSTRING (1, 35, "Writing image file . . .");
			}
		      else	

			{
			  puts ("\n");
			  NLS_PRINTSTRING (1, 25, "Writing to TARGET diskette in drive . . .");
			}
		      PrepareForReading ();
		    }
		  else	/* VERIFICATION */
		    {
		      puts ("");
		      NLS_PRINTSTRING (1, 38, "Verifying . . .");
		      PrepareForReading ();	/* Rewind */

		    }


		  for (sector = beginsector; sector < endsector;
		       sector = sector + TOCOPYSECTORS)
		    {
		      if (sector < endsector - TOCOPYSECTORS)
			tocopysectors = TOCOPYSECTORS;
		      else
			tocopysectors = (int) (endsector - sector);
		      tobuffer = (unsigned) (tocopysectors * BYTESPERSECTOR);

		      if (IsDiskReadRequired (sector, tocopysectors))
			{
			  if (!ReadMemoryBlock (buf, tobuffer))
			    {
			      puts ("");
			      NLS_PUTSTRING (1, 20, "Unsuspected memory error.");
			      SetErrorStopped ();
			      return CRITICAL;
			    }

			  if (action == DISKWRITING)
			    {
#ifdef UPDATE_SERIALNUM
			      if (sector == 0)
				{
				  if (ParsedData->serialnumber == UPDATE)
				    UpdateDiskSerialNumber (buf);
				}
#endif
			      if (abswrite (tdrive, tocopysectors, (int) sector, buf) != 0)
				{
				    if (sector == 0)
				    {
					if (abswrite(tdrive, 1, 0, buf) != 0)
					{
					    puts("\n");
					    NLS_PRINTSTRING(1, 41, "Sector 0 unwritable! Write protected?");
					    
					    if (FilledDiskSize == allocated) 
					 	break; 
					    else 
						return CRITICAL;	
					}
				     }				    				   
				    
				  puts ("");
				  NLS_PRINTSTRING (1, 27, "Media error writing to sector");
				  printf (" %ld.\n", sector);
				}
			    }
			  else
			    {

			      if (absread (tdrive, tocopysectors, (int) sector, cmpbuf) != 0)
				{
				  puts ("");
				  NLS_PRINTSTRING (1, 18, "Media error reading from sector");
				  printf (" %ld.\n", sector);
				}

			      if (sector == 0)
				{
				  if (ParsedData->serialnumber == UPDATE)
				    {
				      ClearDiskSerialNumber (buf);
				      ClearDiskSerialNumber (cmpbuf);
				    }
				}

			      if (memcmp (buf, cmpbuf, tobuffer) != 0)
				{
				  puts ("");
				  NLS_PRINTSTRING (1, 37, "Compare error on sector");
				  printf (" %ld.\n", sector);
				}
			    }
			}
		    }
		}
	      if (!UseImageFile)
		puts ("");

	      if (UseImageFile || (FilledDiskSize == allocated))	/* If everything fitted in memory */
		{
		  loop2 = (fallthrough) ? CatNO : CatNO + 1;
		  if (loop2 == CatYES)
		    loop2++;
		  while (!NLS_TEST_YES_NO (loop2))
		    {
		      ClrKbd ();
		      if (audible)
			Beep ();
		      puts ("");
		      if (UseImageFile)
			puts ("");
		      NLS_PRINTSTRING (1, 28, "Do you want another copy of this ");
		      if (UseImageFile)
			{
			  endsector = 0;
			  NLS_PRINTSTRING (1, 29, "image file (Y/N)?");
			}
		      else
			NLS_PRINTSTRING (1, 30, "disk (Y/N)?");

		      loop2 = toupper (getch ());
		      puts ("");
		    }
		  if (UseImageFile && (NLS_TEST_YES (loop2)))
		    puts ("");
		}
	      else
		{
		  puts ("");
		  loop2 = CatNO;
		}

	      /* Change the serial number of the target disk. */
#ifdef UPDATE_SERIALNUM
	      if (ParsedData->serialnumber == UPDATE)
		{
		  puts ("");
		  PrintDiskSerialNumber ();
		  puts ("");
		}
#endif
	    }			/*  loop2 */

	  loop2 = CatYES;
	  if (loop2 == CatNO)
	    loop2++;
	  ReleaseMemory ();
	}
      else
	{
	  if (CopyingToSameDisk)
	    {
	      PrepareForReading ();
	      if (!WriteFileFromMemory (GetSFN (OUTFILE_ID), buf, BUFFERSIZE,
					FilledDiskSize, asktdisk,
					fallthrough, Overwrite))
		{
		  NLS_PUTSTRING (1, 36, "Problem writing image file.");
		  return CRITICAL;
		}
	    }
	}

      loop1 = CatYES + 1;
      if (loop1 == CatNO)
	loop1++;
      if (!UseImageFile && !fallthrough && (endsector == total))
	{
	  while (!NLS_TEST_YES_NO (loop1))
	    {
	      ClrKbd ();
	      puts ("");
	      NLS_PRINTSTRING (1, 31, "Copy another disk (Y/N)?");
	      loop1 = toupper (getch ());
	      puts ("");
	      endsector = 0;
	    }
	}
      else if (UseImageFile || (fallthrough && (endsector == total)))
	{
	  loop1 = CatNO;
	  if (fallthrough)
	    puts ("");
	  else if (audible && (ImageModus == WRITEIMAGE))
	    Beep ();
	}

    }				/* end loop1 */

  if ((UseImageFile) && (ImageModus == WRITEIMAGE))
    puts ("");

  return COPYSUCCESS;
}

void
ShowHelp (char switchchar)
{
  printf ("Diskcopy %s\n", VERSION);
  NLS_PRINTSTRING (9, 50, "Copy one diskette or image file to another diskette or image file.");
  printf ("\n");

  printf ("(C) 1998 ");
  NLS_PRINTSTRING (9, 51, "by");
  printf (" Matthew Stanford.\n(C) 1999, 2000, 2001, 2002, 2003, 2004 ");
  NLS_PRINTSTRING (9, 51, "by");
  printf (" Imre Leber.\n\n");

#ifdef ALL_RECOVERY_MODE
  printf ("Diskcopy <");
  NLS_PRINTSTRING (9, 52, "source");
  printf ("> <");
  NLS_PRINTSTRING (9, 53, "destination");
  printf ("> [%ca][%cv][%cm][%ci][%co][%cx][%cd][%cf][%ct]\n",
	  switchchar,
	  switchchar,
	  switchchar,
	  switchchar,
	  switchchar,
	  switchchar,
	  switchchar,
	  switchchar,
	  switchchar);

#else

  printf ("Diskcopy <");
  NLS_PRINTSTRING (9, 52, "source");
  printf ("> <");
  NLS_PRINTSTRING (9, 53, "destination");
  printf ("> [%ca][%cv][%cm][%ci][%co][%cx][%cd][%cr][%cf][%ct]\n",
	  switchchar,
	  switchchar,
	  switchchar,
	  switchchar,
	  switchchar,
	  switchchar,
	  switchchar,
	  switchchar,
	  switchchar,
	  switchchar);

#endif

  NLS_PUTSTRING (9, 54, "source:      drive or image file to copy from.");
  NLS_PUTSTRING (9, 55, "destination: drive or image file to copy to.");
  puts ("");
  printf ("%ca : ", switchchar);
  NLS_PUTSTRING (9, 56, "give an audible warning for user action.");
  printf ("%cv : ", switchchar);
  NLS_PUTSTRING (9, 57, "verify reads and writes.");
  printf ("%cm : ", switchchar);
  NLS_PUTSTRING (9, 58, "only use memory for disk copy.");
  printf ("%ci : ", switchchar);
  NLS_PUTSTRING (9, 59, "show memory usage (informative).");
  printf ("%co : ", switchchar);
  NLS_PUTSTRING (9, 60, "overwrite destination, if it already exists (in case of an image file).");
  printf ("%cx : ", switchchar);
  NLS_PUTSTRING (9, 61, "always automaticaly exit.");
  printf ("%cd : ", switchchar);
  NLS_PUTSTRING (9, 62, "assume disk already in drive.");
#ifndef ALL_RECOVERY_MODE
  printf ("%cr : ", switchchar);
  NLS_PUTSTRING (9, 63, "go into disk error recovery mode.");
#endif
  printf ("%cf : ", switchchar);
  NLS_PUTSTRING (9, 64, "perform fast diskcopy (only copy filled sectors).");
  printf ("%ct : ", switchchar);
  NLS_PUTSTRING (9, 40, "don't ask target disk if copying image file to same disk.");
  printf ("%c1 : ", switchchar);
  NLS_PUTSTRING (9, 67, "no-op included for MS-DOS compatibility.");
  printf ("\n");
  NLS_PUTSTRING (9, 65, "Remarks: a minus sign after an option disables the option.");
  NLS_PUTSTRING (9, 66, "         You may specify the same drive for source and destination.");
}

void
OnExit (void)
{
  ReleaseMemory ();
  ReleaseFatInfo ();
  setcbrk (SavedCBreak);

  SynchronizeLFNs ();

  CROSSCUT_NLS_CLOSE
}

int
OnCBreak (void)
{
  OnExit ();
  return ABORT;
}
