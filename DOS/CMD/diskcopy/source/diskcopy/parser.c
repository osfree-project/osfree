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

   module: parser.c - diskcopy.ini syntactic analyser.

 */

#include <stdio.h>
#include <stdlib.h>

#include "parser.h"
#include "scanner.h"
#include "drive.h"
#include "diskcopy.h"
#include "datgen.h"
#include "fastcopy.h"
#include "nlsaspct.h"

CROSSCUT_NLS_DATA_IMPORT

/*
   #define PARSER_DEBUG
 */

/*
   Grammar: not accurate!

   INIFILE  -> (TknSPACE|TknRETURN)* TknMEMORYHEADER TknSPACE* TknRETURN MEMORIES
   -> (TknSPACE|TknRETURN)* TknOPTIONHEADER TknSPACE* TknRETURN OPTIONS

   MEMORIES -> MEMORIES TknRETURN MEMORIES
   -> TknSPACE* TknEMS  TknSPACE* TknASSIGN TknSPACE* BOOLEAN     TknSPACE* 
   -> TknSPACE* TknXMS  TknSPACE* TknASSIGN TknSPACE* BOOLEAN     TknSPACE* 
   -> TknSPACE* TknDISK TknSPACE* TknASSIGN TknSPACE* BOOLEAN     TknSPACE* 
   -> 

   OPTIONS  -> OPTIONS TknRETURN OPTIONS
   -> TknSPACE* TknAUDIBLE     TknSPACE* TknASSIGN TknSPACE* BOOLEAN TknSPACE*
   -> TknSPACE* TknVERIFY      TknSPACE* TknASSIGN TknSPACE* BOOLEAN TknSPACE* 
   -> TknSPACE* TknINFORMATIVE TknSPACE* TknASSIGN TknSPACE* BOOLEAN TknSPACE* 
   -> TknSPACE* TknOVERWRITE   TknSPACE* TknASSIGN TknSPACE* TIME    TknSPACE*
   -> TknSPACE* TknAUTOEXIT    TknSPACE* TknASSIGN TknSPACE* BOOLEAN TknSPACE* 
   -> TknSPACE* TknMODE        TknSPACE* TknASSIGN TknSPACE* MODE    TknSPACE*
   ->

   BOOLEAN  -> TknYES
   -> TknNO

   TIME     -> TknALWAYS
   -> TknNEVER


   MODE     -> TknRECOVERY
   -> TknNORMAL

 */

static void SkipWhiteSpace (int losely);
static void ParseINIFILE (void);
static void ShowParserError (void);
static void InitParserStruct (void);

static void ActOnXMS (TOKEN rvalue);
static void ActOnEMS (TOKEN rvalue);
static void ActOnDISK (TOKEN rvalue);
static void ActOnAUDIBLE (TOKEN rvalue);
static void ActOnVERIFY (TOKEN rvalue);
static void ActOnINFORMATIVE (TOKEN rvalue);
static void ActOnAUTOEXIT (TOKEN rvalue);
static void ActOnASKDISK (TOKEN rvalue);
static void ActOnASKTARGET (TOKEN rvalue);
static void ActOnOVERWRITE (TOKEN rvalue);
static void ActOnMODE (TOKEN rvalue);
static void ActOnUSEDATFILE (TOKEN rvalue);
static void ActOnSPEED (TOKEN rvalue);
static void ActOnSERIALNUMBER (TOKEN rvalue);

static void ParseBlock (struct LValueArray *lvalues, int amount);

static struct IniParserStruct INIParserData;

struct LValueArray MemoryLvals[] =
{
  {TknXMS, ActOnXMS},
  {TknEMS, ActOnEMS},
  {TknDISK, ActOnDISK}};

struct LValueArray OptionLvals[] =
{
  {TknAUDIBLE, ActOnAUDIBLE},
  {TknVERIFY, ActOnVERIFY},
  {TknINFORMATIVE, ActOnINFORMATIVE},
  {TknAUTOEXIT, ActOnAUTOEXIT},
  {TknASKDISK, ActOnASKDISK},
  {TknASKTARGET, ActOnASKTARGET},
  {TknOVERWRITE, ActOnOVERWRITE},
  {TknMODE, ActOnMODE},
  {TknSPEED, ActOnSPEED},
  {TknSERIALNUMBER, ActOnSERIALNUMBER}};

struct LValueArray GenerateLVals[] =
{
  {TknUSEDATFILE, ActOnUSEDATFILE}};

#define AMOFMEMORYLVALS   (int) (sizeof (MemoryLvals) / sizeof (*MemoryLvals))
#define AMOFOPTIONLVALS   (int) (sizeof (OptionLvals) / sizeof (*OptionLvals))
#define AMOFGENERATELVALS (int) (sizeof (GenerateLVals) / sizeof (*GenerateLVals))

struct HeaderArray Headers[] =
{
  {AMOFMEMORYLVALS, TknMEMORYHEADER, MemoryLvals},
  {AMOFOPTIONLVALS, TknOPTIONHEADER, OptionLvals},
  {AMOFGENERATELVALS, TknGENERATEHEADER, GenerateLVals}};

#define AMOFHEADERS (int) (sizeof(Headers) / sizeof(*Headers))

/*
   ** Public interface to the parser.
 */

int
ParseIniFile (char *filename)
{
  int result;

  InitParserStruct ();

  if (ExploreDATFile (&INIParserData))
    return PARSERSUCCESS;

  if ((result = OpenScannerFile (filename)) <= 0)
    return result;

  ParseINIFILE ();

  if (INIParserData.MakeDAT)
    MakeDATFile (&INIParserData);

  /* Close ini file. */
  CloseScanner ();
  return PARSERSUCCESS;
}

/*
   ** Return a pointer to the parsed results.
 */
struct IniParserStruct *
GetParsedData ()
{
  return &INIParserData;
}

/* 
   ** Skip white space. 
 */

static void
SkipWhiteSpace (int losely)
{
  int token;

  for (;;)
    {
      token = PeekToken ();
      if ((token == TknSPACE) || (losely && (token == TknRETURN)))
	GetNextToken ();
      else
	break;
    }

}

/*
   ** Parse an .ini file.
 */

static void
ParseINIFILE ()
{
  int token, i, found;

  for (;;)
    {
      SkipWhiteSpace (TRUE);

      token = GetNextToken ();
      if (token == TknDONE)
	return;

      if (token != TknOPENBLOCK)
	ShowParserError ();

      SkipWhiteSpace (FALSE);

      token = GetNextToken ();

      found = FALSE;
      for (i = 0; i < AMOFHEADERS; i++)
	if (token == Headers[i].header)
	  {
	    found = TRUE;
	    break;
	  }

      if (!found)
	ShowParserError ();

      SkipWhiteSpace (FALSE);

      token = GetNextToken ();
      if (token != TknCLOSEBLOCK)
	ShowParserError ();

      SkipWhiteSpace (FALSE);

      if (token == TknDONE)
	break;
      if (GetNextToken () != TknRETURN)
	ShowParserError ();

      ParseBlock (Headers[i].lvalues, Headers[i].amount);
    }
}

/*
   ** Parse an individual block.
 */

static void
ParseBlock (struct LValueArray *lvalues, int amount)
{
  int token, i, found;

  for (;;)
    {
      SkipWhiteSpace (TRUE);
      token = PeekToken ();

      found = FALSE;
      for (i = 0; i < amount; i++)
	{
	  if (lvalues[i].token == token)
	    {
	      found = TRUE;
	      break;
	    }
	}
      if (!found)
	return;

      GetNextToken ();
      SkipWhiteSpace (FALSE);

      token = GetNextToken ();
      if (token == TknNONE)
	ShowParserError ();
      if ((token == TknRETURN) || (token == TknDONE))
	continue;

      if (token == TknASSIGN)
	{
	  SkipWhiteSpace (FALSE);
	  token = PeekToken ();

	  if ((token != TknRETURN) && (token != TknDONE))
	    lvalues[i].func (GetNextToken ());
	}
      else
	lvalues[i].func (token);

      SkipWhiteSpace (FALSE);
      if (((token = GetNextToken ()) != TknRETURN) && (token != TknDONE))
	ShowParserError ();
    }
}

/*
   ** Show a syntactic error message.
 */
static void
ShowParserError ()
{
  NLS_PRINTSTRING (6, 0, "Syntax error on line");
  printf (" %d ", GetScannerLine ());
  NLS_PUTSTRING (6, 1, "in configuration file.");

  CloseScanner ();
  exit (INITERROR);
}

/*
   ** Show a semantic error message.
 */
static void
ShowParserErrorMsg (char *msg)
{
  NLS_PRINTSTRING (6, 2, "Semantic error in configuration file");

  printf (" (%d): %s!\n", GetScannerLine (), msg);

  CloseScanner ();
  exit (INITERROR);
}

/*
   ** Initialise parser result struct.
 */

static void
InitParserStruct ()
{
  INIParserData.UseEMS = YES;
  INIParserData.UseXMS = YES;
  INIParserData.UseSWAP = YES;

  INIParserData.audible = NO;
  INIParserData.verify = NO;
  INIParserData.informative = NO;
  INIParserData.overwrite = NEVER;
  INIParserData.autoexit = NO;
  INIParserData.askdisk = YES;
  INIParserData.mode = NORMAL;
  INIParserData.MakeDAT = NO;
  INIParserData.speed = FULL;
  INIParserData.asktdisk = YES;
  INIParserData.serialnumber = UPDATE;
}

/*
   ** Act on <lvalue> = YES|NO
 */

static void
PrivateActOnYesNo (int *field, TOKEN rvalue)
{
  switch (rvalue)
    {
    case TknYES:
#ifdef PARSER_DEBUG
      printf ("YES\n");
#endif
      *field = TRUE;
      break;

    case TknNO:
#ifdef PARSER_DEBUG
      printf ("NO\n");
#endif
      *field = FALSE;
      break;

    case TknASSIGN:
      ShowParserErrorMsg (NLS_STRING (6, 3, "do not enter more than one '='"));
      break;

    case TknRETURN:
      break;

    default:
      ShowParserErrorMsg (NLS_STRING (6, 4, "please enter YES or NO"));
    }
}

/*
   ** Act on XMS = YES|NO
 */

static void
ActOnXMS (TOKEN rvalue)
{
#ifdef PARSER_DEBUG
  printf ("XMS = ");
#endif

  PrivateActOnYesNo (&INIParserData.UseXMS, rvalue);
}

/*
   ** Act on EMS = YES|NO
 */

static void
ActOnEMS (TOKEN rvalue)
{
#ifdef PARSER_DEBUG
  printf ("EMS = ");
#endif

  PrivateActOnYesNo (&INIParserData.UseEMS, rvalue);
}

/*
   ** Act on DISK = YES|NO
 */

static void
ActOnDISK (TOKEN rvalue)
{
#ifdef PARSER_DEBUG
  printf ("DISK = ");
#endif

  PrivateActOnYesNo (&INIParserData.UseSWAP, rvalue);
}

/*
   ** Act on ASKTARGET = YES|NO
 */

static void
ActOnASKTARGET (TOKEN rvalue)
{
#ifdef PARSER_DEBUG
  printf ("ASKTARGET = ");
#endif

  PrivateActOnYesNo (&INIParserData.asktdisk, rvalue);
}

/*
   ** Act on AUDIBLE = YES|NO
 */

static void
ActOnAUDIBLE (TOKEN rvalue)
{
#ifdef PARSER_DEBUG
  printf ("AUDIBLE = ");
#endif

  PrivateActOnYesNo (&INIParserData.audible, rvalue);
}

/*
   ** Act on VERIFY = YES|NO
 */

static void
ActOnVERIFY (TOKEN rvalue)
{
#ifdef PARSER_DEBUG
  printf ("VERIFY = ");
#endif

  PrivateActOnYesNo (&INIParserData.verify, rvalue);
}

/*
   ** Act on INFORMATIVE = YES|NO
 */

static void
ActOnINFORMATIVE (TOKEN rvalue)
{
#ifdef PARSER_DEBUG
  printf ("INFORMATIVE = ");
#endif

  PrivateActOnYesNo (&INIParserData.informative, rvalue);
}

/*
   ** Act on AUTOEXIT = YES|NO
 */

static void
ActOnAUTOEXIT (TOKEN rvalue)
{
#ifdef PARSER_DEBUG
  printf ("AUTOEXIT = ");
#endif

  PrivateActOnYesNo (&INIParserData.autoexit, rvalue);
}

/*
   ** Act on ASKDISK = YES|NO
 */

static void
ActOnASKDISK (TOKEN rvalue)
{
#ifdef PARSER_DEBUG
  printf ("ASKDISK = ");
#endif

  PrivateActOnYesNo (&INIParserData.askdisk, rvalue);
}

/*
   ** Act on OVERWRITE = ALWAYS|NEVER
 */

static void
ActOnOVERWRITE (TOKEN rvalue)
{
#ifdef PARSER_DEBUG
  printf ("OVERWRITE = ");
#endif

  switch (rvalue)
    {
    case TknNEVER:
#ifdef PARSER_DEBUG
      printf ("NEVER\n");
#endif
      INIParserData.overwrite = NEVER;
      break;

    case TknALWAYS:
#ifdef PARSER_DEBUG
      printf ("ALWAYS\n");
#endif
      INIParserData.overwrite = ALWAYS;
      break;

    case TknASSIGN:
      ShowParserErrorMsg (NLS_STRING (6, 3, "do not enter more than one '='"));
      break;

    case TknRETURN:
      break;

    default:
      ShowParserErrorMsg (NLS_STRING (6, 5, "please enter NEVER or ALWAYS"));
    }
}

/*
   ** Act on MODE = RECOVERY|NORMAL
 */

static void
ActOnMODE (TOKEN rvalue)
{
#ifdef PARSER_DEBUG
  printf ("MODE = ");
#endif

  switch (rvalue)
    {
    case TknRECOVERY:
#ifdef PARSER_DEBUG
      printf ("RECOVERY\n");
#endif
      INIParserData.mode = RECOVERY;
      break;

    case TknNORMAL:
#ifdef PARSER_DEBUG
      printf ("NORMAL\n");
#endif
      INIParserData.mode = NORMAL;
      break;

    case TknASSIGN:
      ShowParserErrorMsg (NLS_STRING (6, 3, "do not enter more than one '='"));
      break;

    case TknRETURN:
      break;

    default:
      ShowParserErrorMsg (NLS_STRING (6, 6, "please enter RECOVERY or NORMAL"));
    }
}

/*
   ** Act on SPEED = FAST|FULL
 */
static void
ActOnSPEED (TOKEN rvalue)
{
#ifdef PARSER_DEBUG
  printf ("SPEED = ");
#endif

  switch (rvalue)
    {
    case TknFULL:
#ifdef PARSER_DEBUG
      printf ("FULL\n");
#endif
      INIParserData.speed = FULL;
      break;

    case TknFAST:
#ifdef PARSER_DEBUG
      printf ("FAST\n");
#endif
      INIParserData.speed = FAST;
      break;

    case TknASSIGN:
      ShowParserErrorMsg (NLS_STRING (6, 3, "do not enter more than one '='"));
      break;

    case TknRETURN:
      break;

    default:
      ShowParserErrorMsg (NLS_STRING (6, 7, "please enter FAST or FULL"));
    }
}


/*
   ** Act on USEDATFILE = YES|NO
 */
static void
ActOnUSEDATFILE (TOKEN rvalue)
{
#ifdef PARSER_DEBUG
  printf ("USEDATFILE = ");
#endif

  PrivateActOnYesNo (&INIParserData.MakeDAT, rvalue);
}

static void
ActOnSERIALNUMBER (TOKEN rvalue)
{
#ifdef PARSER_DEBUG
  printf ("SERIALNUMBER = ");
#endif

  switch (rvalue)
    {
    case TknUPDATE:
#ifdef PARSER_DEBUG
      printf ("UPDATE\n");
#endif
      INIParserData.serialnumber = UPDATE;
      break;

    case TknLEAVE:
#ifdef PARSER_DEBUG
      printf ("LEAVE\n");
#endif
      INIParserData.serialnumber = LEAVE;
      break;

    case TknASSIGN:
      ShowParserErrorMsg (NLS_STRING (6, 3, "do not enter more than one '='"));
      break;

    case TknRETURN:
      break;

    default:
      ShowParserErrorMsg (NLS_STRING (6, 8, "please enter UPDATE or LEAVE"));
    }
}
