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
   email me at:  imre.leber@worlonline.be

   module: scanner.c - diskcopy.ini lexical analyser.

 */

#ifndef HI_TECH_C
#include <fcntl.h>
#include <io.h>
#include <dir.h>
#endif

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <string.h>

#include "parser.h"
#include "scanner.h"
#include "drive.h"		/* FALSE and TRUE */
#include "exepath.h"

#include "lfnapi.h"		/* Long file name support */

static int ScanKeyWord (char *keyword);
static int Identify (char *karakters);
static void UpperCaseScannerData (void);
static int EatKeyWord (int len);
static void ScanNextToken (TOKEN * result, int *len);
static int SkipComment (void);


static int ScannerLine = 1;

static char *KeyWords[] =
{"DISK",
 "EMS",
 "XMS",
 "YES",
 "AUDIBLE",
 "VERIFY",
 "INFORMATIVE",
 "OVERWRITE",
 "ALWAYS",
 "NEVER",
 "AUTOEXIT",
 "MODE",
 "RECOVERY",
 "NORMAL",
 "NO",
 "ASKDISK",
 "USEDATFILE",
 "FAST",
 "FULL",
 "SPEED",
 "ASKTARGET",
 "SERIALNUMBER",
 "UPDATE",
 "LEAVE",
 "OPTIONS",
 "OPTION",
 "MEMORY",
 "GENERATE",
 "GENERATION"};

static int KeyTokens[] =
{TknDISK,
 TknEMS,
 TknXMS,
 TknYES,
 TknAUDIBLE,
 TknVERIFY,
 TknINFORMATIVE,
 TknOVERWRITE,
 TknALWAYS,
 TknNEVER,
 TknAUTOEXIT,
 TknMODE,
 TknRECOVERY,
 TknNORMAL,
 TknNO,
 TknASKDISK,
 TknUSEDATFILE,
 TknFAST,
 TknFULL,
 TknSPEED,
 TknASKTARGET,
 TknSERIALNUMBER,
 TknUPDATE,
 TknLEAVE,
 TknOPTIONHEADER,
 TknOPTIONHEADER,
 TknMEMORYHEADER,
 TknGENERATEHEADER,
 TknGENERATEHEADER};

#define AMOFKEYWORDS (int) (sizeof(KeyWords) / sizeof(*KeyWords))

/* Check the consistency of these tables */
#define AMOFKWTOKENS (int) (sizeof(KeyTokens) / sizeof(*KeyTokens))

#ifndef HI_TECH_C

#if AMOFKEYWORDS > AMOFKWTOKENS
#error Too many keywords
#endif

#if AMOFKEYWORDS < AMOFKWTOKENS
#error Too many keyword tokens
#endif

#endif

#define LONGESTKEYWORD 12

static char *ScannerEnd, *ScannerPosition;

static char ScannerData[LONGESTKEYWORD];
static char BackupData[LONGESTKEYWORD + 1];	/* Used for backup (poke & string) */

static FILE *ScanFilePtr = NULL;

static char INIPathName[128];
static int Initialized = FALSE;
static int EndReached = FALSE;

/*
   ** Get the directory where to search for the ini file.
   **
   ** Input: filename: file name to append to the directory.
   **
   ** Output: buffer : full path of .ini file.
 */
char *
GetIniDir (char *filename, char *buffer)
{
  char *pathname;

  if (!Initialized)
    {
      Initialized = TRUE;

      /* a) is there an environment variable? */
      pathname = getenv ("DKCPINI");
      if (!pathname)
	pathname = getenv ("dkcpini");

      if (pathname)
	{
	  strcpy (INIPathName, pathname);

	  if (INIPathName[strlen (INIPathName)] != '\\')
	    {
	      INIPathName[strlen (INIPathName) + 1] = '\0';
	      INIPathName[strlen (INIPathName)] = '\\';
	    }

	  /* Convert path to sfn */
	  ConvertToSFN (pathname, ENVVAR_ID);
	  strcpy (buffer, GetSFN (ENVVAR_ID));
	  strcat (buffer, "diskcopy.ini");
	  if (access (buffer, 0) != 0)
	    pathname = NULL;
	}

      /* b) is there an .ini file in the executable's directory? */
      if (!pathname)
	{
	  GetExePath (buffer, 128);
	  strcpy (INIPathName, buffer);
	  if (buffer[0] == 0)
	    return SCANFILENOTFOUND;	/* buffer to small!? */
	  strncat (buffer, "diskcopy.ini", 128);
	  if (access (buffer, 0) != 0)
	    {
	      /* c) is there an .ini file in the current directory?     */
	      INIPathName[0] = 0;
	    }
	}
    }

  strcpy (buffer, INIPathName);
  strcat (buffer, filename);

  return buffer;
}

/*
   ** Opens the ini file, so that it can be read.
   **
   ** Returns:  0  if file not found.
   **          -1  if error when reading file.
   **           1  if success.
 */

int
OpenScannerFile (char *filename)
{
  int i, c;
  FILE *fptr;
  char buffer[128];

  /* First look where we can find the ini file. */
  if (!filename)
    filename = GetIniDir ("diskcopy.ini", buffer);
  if (access (filename, 0) != 0)
    return SCANFILENOTFOUND;

  /* Now that we have found the file open it. */
  if ((fptr = fopen (filename, "rt")) != NULL)
    ScanFilePtr = fptr;
  else
    return SCANFILEERROR;

  /* Fill the buffer for the first time. */
  ScannerPosition = ScannerEnd = ScannerData;
  for (i = 0; i < LONGESTKEYWORD; i++)
    {
      c = fgetc (ScanFilePtr);
      if (c == EOF)
	{
	  EndReached = TRUE;
	  break;
	}

      *ScannerEnd = c;
      ScannerEnd++;
    }

  return SCANSUCCESS;
}

void
CloseScanner (void)
{
  if (ScanFilePtr != NULL)
    fclose (ScanFilePtr);

  ScanFilePtr = NULL;
}

/*
   ** Restarts the scanning process.
 */
void
RestartScanning (char *filename)
{
  EndReached = FALSE;
  CloseScanner ();
  OpenScannerFile (filename);
  ScannerPosition = ScannerEnd = ScannerData;
}

/*
   ** Returns next token in the buffer, or TknDONE if end of buffer reached.
   **
   ** Note: scanning is done in our very litle buffer.
 */

TOKEN
GetNextToken (void)
{
  TOKEN token;
  int toeat;

  if (ScannerPosition == ScannerEnd)
    return TknDONE;

  ScanNextToken (&token, &toeat);

  if (token == TknDONE)		/* File ends on a comment. */
    {
      EndReached = TRUE;
      return TknDONE;
    }
  else if (token != TknNONE)
    {
      memcpy (BackupData, ScannerData, toeat);
      BackupData[toeat] = 0;

      if (EndReached)
	ScannerPosition += toeat;
      else
	EndReached = EatKeyWord (toeat);
    }

  return token;
}

/*
   ** Returns the next token, without advancing the input.
 */
TOKEN
PeekToken ()
{
  TOKEN token;
  int toeat, line = ScannerLine;

  if (ScannerPosition == ScannerEnd)
    return TknDONE;

  ScanNextToken (&token, &toeat);

  ScannerLine = line;

  return token;
}

static void
ScanNextToken (TOKEN * result, int *toeat)
{
  int i;

  /* See if we have a comment. */
  if (Identify ("#"))
    {
      if (SkipComment ())
	{
	  *result = TknDONE;
	  *toeat = 0;
	  return;
	}
    }

  /* See if we have a simple white space. */
  if (Identify (" \t\x1A"))
    {
      *result = TknSPACE;
      *toeat = 1;
      return;
    }

  /* See if we have CR/LF */
  if (ScanKeyWord ("\r\n"))
    {
      ScannerLine++;
      *toeat = 2;
      *result = TknRETURN;
      return;
    }

  /* See if we have a line feed. */
  if (Identify ("\n"))
    {
      ScannerLine++;
      *toeat = 1;
      *result = TknRETURN;
      return;
    }

  /* See if we have cariage return. */
  if (Identify ("\r"))
    {
      *toeat = 1;
      ScannerLine++;
      *result = TknRETURN;
      return;
    }

  /* See if we have a = token. */
  if (Identify ("="))
    {
      *result = TknASSIGN;
      *toeat = 1;
      return;
    }

  /* See if we have a [- token. */
  if (ScanKeyWord ("[-"))
    {
      *result = TknOPENBLOCK;
      *toeat = 2;
      return;
    }

  /* See if we have a [ token. */
  if (Identify ("["))
    {
      *result = TknOPENBLOCK;
      *toeat = 1;
      return;
    }

  /* See if we have a -] token. */
  if (ScanKeyWord ("-]"))
    {
      *result = TknCLOSEBLOCK;
      *toeat = 2;
      return;
    }

  /* See if we have a ] token. */
  if (ScanKeyWord ("]"))
    {
      *result = TknCLOSEBLOCK;
      *toeat = 1;
      return;
    }

  /* Scan normal keywords. */
  for (i = 0; i < AMOFKEYWORDS; i++)
    if (ScanKeyWord (KeyWords[i]))
      {
	*toeat = strlen (KeyWords[i]);
	*result = KeyTokens[i];
	return;
      }

  *toeat = 0;
  *result = TknNONE;
}

/*
   ** Returns the string corresponding to the last scannend token.
 */
char *
ScannerString ()
{
  return BackupData;
}

int
GetScannerLine ()
{
  return ScannerLine;
}

static int
ScanKeyWord (char *keyword)
{
  return (strncmp (keyword, ScannerPosition, strlen (keyword)) == 0);
}

static int
Identify (char *characters)
{
  while (*characters)
    if (*ScannerPosition == *characters)
      return TRUE;
    else
      characters++;

  return FALSE;
}

static void
UpperCaseScannerData ()
{
  char *pointer = ScannerData;

  while (pointer != ScannerEnd)
    {
      *pointer = toupper (*pointer);
      pointer++;
    }
}

static int
EatKeyWord (int len)
{
  int done = FALSE;
  char *p;

  /* Bring the rest of the data to the front. */
  for (p = ScannerData; p != ScannerData + (LONGESTKEYWORD - len); p++)
    *p = *(p + len);

  /* Read the rest from the .ini file. */
  for (; p != ScannerEnd; p++)
    if ((*p = fgetc (ScanFilePtr)) == EOF)
      {
	ScannerEnd = p;
	done = TRUE;
	break;
      }

  /* Convert to upper case. */
  UpperCaseScannerData ();

  /* Return wether we are at the end of input. */
  return done;
}

static int
SkipComment (void)
{
  int c;
  char *p = ScannerPosition;

  if (EndReached)
    {
      while (ScannerPosition != ScannerEnd)
	if ((*ScannerPosition == '\n') || (*ScannerPosition == '\r'))
	  return FALSE;
	else
	  ScannerPosition++;

      return TRUE;
    }

  /* See wether the end of the comment is in the buffer. */
  while (p != ScannerEnd)
    {
      if (*p == '\n')
	return EatKeyWord (p - ScannerData);
      else if (*p == '\r')
	if ((p + 1 == ScannerEnd) || (*(p + 1) != '\n'))
	  return EatKeyWord (p - ScannerData);
	else
	  return EatKeyWord (p - ScannerData + 1);

      p++;
    }

  /* Skip the comment in the file */
  while (((c = fgetc (ScanFilePtr)) != EOF) && (c != '\n') && (c != '\r'));
  if (c == EOF)
    return TRUE;

  /* Fill the buffer again. */
  if (c == '\r')
    {
      *(ScannerEnd - 1) = fgetc (ScanFilePtr);
      if (*(ScannerEnd - 1) == EOF)
	return TRUE;
      else if (*(ScannerEnd - 1) == '\n')
	return EatKeyWord (LONGESTKEYWORD - 1);
      else
	{
	  *(ScannerEnd - 2) = '\n';
	  return EatKeyWord (LONGESTKEYWORD - 2);
	}
    }
  else
    /* c == '\n' */
    {
      *(ScannerEnd - 1) = '\n';
    }

  return EatKeyWord (LONGESTKEYWORD - 1);
}
