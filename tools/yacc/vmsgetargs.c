/****************************************************************
 * VMS version of getargs; Uses DCL command parsing.
 * Copyright (C) 1989, 1992 Free Software Foundation, Inc.
 *
 * This file is part of Bison, the GNU Compiler Compiler.
 *
 * Bison is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * Bison is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Bison; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 ****************************************************************/

/*!
 *  @file vmsgetargs.c
 *  @brief VMS version of getargs; uses DCL command parsing.
 *
 *  Implements the same entry point as getargs.c, but obtains the
 *  options and the input file name by querying the DCL command line
 *  through the cli$present() and cli$get_value() routines instead of
 *  parsing argv directly. Only used when Bison is built on VMS.
 *
 *  @copyright Copyright (C) 1989, 1992 Free Software Foundation, Inc.
 *             Licensed under the GNU General Public License v2 or later.
 */


#include <ctype.h>
#include <stdio.h>
#include "files.h"

/*
 *	VMS version of getargs: Uses DCL command parsing
 *		(argc and argv are ignored)
 */

int verboseflag;        /*!< Non-zero when /VERBOSE was given. */
int definesflag;        /*!< Non-zero when /DEFINES was given. */
int debugflag;          /*!< Non-zero when /DEBUG was given. */
int nolinesflag;        /*!< Non-zero when /NOLINES was given. */
extern int noparserflag;/*!< Non-zero when /NOPARSER was given. */
extern int toknumflag;  /*!< Non-zero when /TOKEN_TABLE was given. */
extern int rawtoknumflag;/*!< Non-zero when /RAW was given. */
extern int fixed_outfiles;/*!< Non-zero when /FIXED_OUTFILES or /YACC was given. */
extern char * version_string;   /*!< Version string printed on /VERSION. */

/* Allocate storgate and initialize, since bison uses them elsewhere.  */

char *spec_name_prefix; /*!< Name prefix set by /NAME_PREFIX. */
char *spec_file_prefix; /*!< File prefix set by /FILE_PREFIX. */

/*!
 *  @brief Parses the VMS command line.
 *
 *  @param[in] argc Ignored on VMS.
 *  @param[in] argv Ignored on VMS.
 */
getargs(argc,argv)
     int argc;
     char *argv[];
{
  register char *cp;
  static char Input_File[256];
  static char output_spec[256], name_prefix_spec[256], file_prefix_spec[256];
  extern char *infile;

  verboseflag = 0;
  definesflag = 0;
  debugflag = 0;
  fixed_outfiles = 0;
  nolinesflag = 0;
  noparserflag = 0;
  toknumflag = 0;
  rawtoknumflag = 0;
  /*
   *	Check for /VERBOSE qualifier
   */
  if (cli_present("BISON$VERBOSE")) verboseflag = 1;
  /*
   *	Check for /DEFINES qualifier
   */
  if (cli_present("BISON$DEFINES")) definesflag = 1;
  /*
   *	Check for /FIXED_OUTFILES qualifier
   */
  if (cli_present("BISON$FIXED_OUTFILES")) fixed_outfiles = 1;
  if (cli_present("BISON$YACC")) fixed_outfiles = 1;
  /*
   *	Check for /VERSION qualifier
   */
  if (cli_present("BISON$VERSION")) printf("%s",version_string);
  /*
   *	Check for /NOLINES qualifier
   */
  if (cli_present("BISON$NOLINES")) nolinesflag = 1;
  /*
   *	Check for /NOPARSER qualifier
   */
  if (cli_present("BISON$NOPARSER")) noparserflag = 1;
  /*
   *	Check for /RAW qualifier
   */
  if (cli_present("BISON$RAW")) rawtoknumflag = 1;
  /*
   *	Check for /TOKEN_TABLE qualifier
   */
  if (cli_present("BISON$TOKEN_TABLE")) toknumflag = 1;
  /*
   *	Check for /DEBUG qualifier
   */
  if (cli_present("BISON$DEBUG")) debugflag = 1;
  /*
   *	Get the filename
   */
  cli_get_value("BISON$INFILE", Input_File, sizeof(Input_File));
  /*
   *	Lowercaseify the input filename
   */
  cp = Input_File;
  while(*cp)
    {
      if (isupper(*cp)) *cp = tolower(*cp);
      cp++;
    }
  infile = Input_File;
  /*
   *	Get the output file
   */
  if (cli_present("BISON$OUTPUT"))
    {
      cli_get_value("BISON$OUTPUT", output_spec, sizeof(output_spec));
      for (cp = spec_outfile = output_spec; *cp; cp++)
	if (isupper(*cp))
	  *cp = tolower(*cp);
    }
  /*
   *	Get the output file
   */
  if (cli_present("BISON$FILE_PREFIX"))
    {
      cli_get_value("BISON$FILE_PREFIX", file_prefix_spec, 
		     sizeof(file_prefix_spec));
      for (cp = spec_file_prefix = file_prefix_spec; *cp; cp++)
	if (isupper(*cp))
	  *cp = tolower(*cp);
    }
  /*
   *	Get the output file
   */
  if (cli_present("BISON$NAME_PREFIX"))
    {
      cli_get_value("BISON$NAME_PREFIX", name_prefix_spec, 
		     sizeof(name_prefix_spec));
      for (cp = spec_name_prefix = name_prefix_spec; *cp; cp++)
	if (isupper(*cp))
	  *cp = tolower(*cp);
    }
}

/************		DCL PARSING ROUTINES		**********/

/*
 *	See if "NAME" is present
 */

/*!
 *  @brief Tests whether the given DCL qualifier is present.
 *
 *  @param[in] Name Qualifier name.
 *
 *  @return Non-zero when the qualifier is present.
 */
int
cli_present(Name)
     char *Name;
{
  struct {int Size; char *Ptr;} Descr;

  Descr.Ptr = Name;
  Descr.Size = strlen(Name);
  return((cli$present(&Descr) & 1) ? 1 : 0);
}

/*
 *	Get value of "NAME"
 */

/*!
 *  @brief Retrieves the value of a DCL qualifier.
 *
 *  @param[in]  Name   Qualifier name.
 *  @param[out] Buffer Output buffer.
 *  @param[in]  Size   Size of the output buffer.
 *
 *  @return Non-zero when the value was retrieved.
 */
int
cli_get_value(Name,Buffer,Size)
     char *Name;
     char *Buffer;
{
  struct {int Size; char *Ptr;} Descr1,Descr2;

  Descr1.Ptr = Name;
  Descr1.Size = strlen(Name);
  Descr2.Ptr = Buffer;
  Descr2.Size = Size-1;
  if (cli$get_value(&Descr1,&Descr2,&Descr2.Size) & 1) {
    Buffer[Descr2.Size] = 0;
    return(1);
  }
  return(0);
}
