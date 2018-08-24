/***************************************************************
  
        bwb_dio.c       Device Input/Output Routines
                        for Bywater BASIC Interpreter
  
                        Copyright (c) 1993, Ted A. Campbell
                        Bywater Software
  
                        email: tcamp@delphi.com
  
        Copyright and Permissions Information:
  
        All U.S. and international rights are claimed by the author,
        Ted A. Campbell.
  
   This software is released under the terms of the GNU General
   Public License (GPL), which is distributed with this software
   in the file "COPYING".  The GPL specifies the terms under
   which users may copy and use the software in this distribution.
  
   A separate license is available for commercial distribution,
   for information on which you should contact the author.
  
***************************************************************/

/*---------------------------------------------------------------*/
/* NOTE: Modifications marked "JBV" were made by Jon B. Volkoff, */
/* 11/1995 (eidetics@cerf.net).                                  */
/*                                                               */
/* Those additionally marked with "DD" were at the suggestion of */
/* Dale DePriest (daled@cadence.com).                            */
/*                                                               */
/* Version 3.00 by Howard Wulf, AF5NE                            */
/*                                                               */
/* Version 3.10 by Howard Wulf, AF5NE                            */
/*                                                               */
/* Version 3.20 by Howard Wulf, AF5NE                            */
/*                                                               */
/*---------------------------------------------------------------*/



#include "bwbasic.h"


static LineType *C77_OPEN (LineType * l);
static LineType *D71_OPEN (LineType * l);
static LineType *H80_OPEN (LineType * l);
static LineType *M80_OPEN (LineType * l);
static LineType *S70_OPEN (LineType * l);
static LineType *T79_OPEN (LineType * l);


/***************************************************************
  
        FUNCTION:       bwb_open()
  
   DESCRIPTION:    This function implements the BASIC OPEN
         command to open a stream for device input/output.
  
        SYNTAX: 1. OPEN "I"|"O"|"R"|"A", [#]n, filename [,rlen]
                2. OPEN filename [FOR INPUT|OUTPUT|APPEND|RANDOM|BINARY] AS [#]n [LEN=n]
  
***************************************************************/

extern void
bwb_file_open (char A, int x, char *B, int y)
{
  /* OPEN "I"|"O"|"R"|"A", [#] filenum, filename [,reclen] */
  /* P1STR|P2NUM|P3STR|P4NUM */
  /* P1BYT|P2DEV|P3BYT|P4INT */
  int mode;
  FileType *F;
  FILE *fp;
  char *buffer;
   
  assert (B != NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);
  assert(My->SYSIN != NULL);
  assert(My->SYSOUT != NULL);
  assert(My->SYSPRN != NULL);

  mode = 0;
  F = NULL;
  fp = NULL;
  buffer = NULL;
  if (y < 0)
  {
    WARN_FIELD_OVERFLOW;
    return;
  }

  mode = bwb_toupper (A);
  switch (mode)
  {
  case 'I':
  case 'O':
  case 'A':
  case 'B':
  case 'V':
  case 'R':
    /* valid mode */
    break;
  default:
    WARN_BAD_FILE_MODE;
    return;
  }
  /* valid mode */
  F = find_file_by_number (x);
  if (F == NULL)
  {
    F = file_new ();
    if (F == NULL)
    {
      WARN_OUT_OF_MEMORY;
      return;
    }
  }
  if (F == My->SYSIN)
  {
    WARN_BAD_FILE_NUMBER;
    return;
  }
  if (F == My->SYSOUT)
  {
    WARN_BAD_FILE_NUMBER;
    return;
  }
  if (F == My->SYSPRN)
  {
    WARN_BAD_FILE_NUMBER;
    return;
  }
  if (F->DevMode != DEVMODE_CLOSED)
  {
    WARN_BAD_FILE_NUMBER;
    return;
  }
  /* valid filenumber */
  switch (mode)
  {
  case 'I':
    mode = DEVMODE_INPUT;
    fp = fopen (B, "r");
    if (fp == NULL)
    {
      WARN_BAD_FILE_NAME;
      return;
    }
    if (y > 0)
    {
      /* RECLEN == INPUT BUFFER SIZE */
      if ((buffer =
           (char *) calloc (y + 1 /* NulChar */ , sizeof (char))) == NULL)
      {
        WARN_OUT_OF_MEMORY;
        return;
      }
      bwb_memset (buffer, ' ', y);        /* flush */
      buffer[y] = NulChar;
    }
    break;
  case 'O':
    mode = DEVMODE_OUTPUT;
    fp = fopen (B, "w");
    if (fp == NULL)
    {
      WARN_BAD_FILE_NAME;
      return;
    }
    if (y > 0)
    {
      /* RECLEN == OUTPUT WRAP WIDTH */
    }
    break;
  case 'A':
    mode = DEVMODE_APPEND;
    fp = fopen (B, "a");
    if (fp == NULL)
    {
      WARN_BAD_FILE_NAME;
      return;
    }
    fseek (fp, 0, SEEK_END);
    if (y > 0)
    {
      /* RECLEN == OUTPUT WRAP WIDTH */
    }
    break;
  case 'B':
    mode = DEVMODE_BINARY;
    fp = fopen (B, "r+");
    if (fp == NULL)
    {
      fp = fopen (B, "w");
      if (fp != NULL)
      {
        bwb_fclose (fp);
        fp = fopen (B, "r+");
      }
    }
    if (fp == NULL)
    {
      WARN_BAD_FILE_NAME;
      return;
    }
    if (y > 0)
    {
      /* RECLEN == SILENTLY IGNORED */
    }
    break;
  case 'V':
    mode = DEVMODE_VIRTUAL;
    fp = fopen (B, "r+");
    if (fp == NULL)
    {
      fp = fopen (B, "w");
      if (fp != NULL)
      {
        bwb_fclose (fp);
        fp = fopen (B, "r+");
      }
    }
    if (fp == NULL)
    {
      WARN_BAD_FILE_NAME;
      return;
    }
    if (y > 0)
    {
      /* RECLEN == SILENTLY IGNORED */
    }
    break;
  case 'R':
    if (y == 0)
    {
      /* dialect-specific default record length */
      y = My->CurrentVersion->OptionReclenInteger;
      if (y == 0)
      {
        /* there is no default record length */
        WARN_FIELD_OVERFLOW;
        return;
      }
    }
    mode = DEVMODE_RANDOM;
    fp = fopen (B, "r+");
    if (fp == NULL)
    {
      fp = fopen (B, "w");
      if (fp != NULL)
      {
        bwb_fclose (fp);
        fp = fopen (B, "r+");
      }
    }
    if (fp == NULL)
    {
      WARN_BAD_FILE_NAME;
      return;
    }
    if (y > 0)
    {
      /* RECLEN == RANDOM BUFFER SIZE */
      if ((buffer = (char *) calloc (y, sizeof (char))) == NULL)
      {
        WARN_OUT_OF_MEMORY;
        return;
      }
      bwb_memset (buffer, ' ', y);        /* flush */
    }
    break;
  default:
    /* should not happen */
    WARN_BAD_FILE_MODE;
    return;
  }
  /* OK */
  F->FileNumber = x;
  F->DevMode = mode;
  F->cfp = fp;
  F->width = y;                        /* WIDTH == RECLEN */
  F->col = 1;
  F->row = 1;
  F->delimit = ',';
  if (F->buffer != NULL)
  {
    free (F->buffer);
    F->buffer = NULL;
  }
  F->buffer = buffer;
  if (F->FileName != NULL)
  {
    free (F->FileName);
    F->FileName = NULL;
  }
  F->FileName = bwb_strdup (B);        /* 'B' is free'd by caller */
  return;
}



static LineType *
C77_OPEN (LineType * l)
{
  int FileNumber;
  int RecordLength;
  char *FileName;
   
  assert (l != NULL);
  assert(My != NULL);

  FileNumber = 0;
  RecordLength = 0;
  FileName = NULL;
  /* 
     SYNTAX: OPEN filename$ [ RECL reclen ] AS filenumber [ BUFF ignored ] [ RECS ignored ] 
   */

  /* 
     --------------------------------------------- 
     FILE NAME
     --------------------------------------------- 
   */
  if (line_read_string_expression (l, &FileName) == FALSE)
  {
    WARN_BAD_FILE_NAME;
    return (l);
  }
  if (FileName == NULL)
  {
    WARN_BAD_FILE_NAME;
    return (l);
  }
  if (is_empty_string (FileName))
  {
    WARN_BAD_FILE_NAME;
    return (l);
  }
  /* 
     --------------------------------------------- 
     RECORD LENGTH
     --------------------------------------------- 
   */
  if (line_skip_word (l, "RECL"))
  {
    if (line_read_integer_expression (l, &RecordLength) == FALSE)
    {
      WARN_FIELD_OVERFLOW;
      return (l);
    }
    if (RecordLength <= 0)
    {
      WARN_FIELD_OVERFLOW;
      return (l);
    }
  }
  /* 
     --------------------------------------------- 
     FILE NUMBER
     --------------------------------------------- 
   */
  if (line_skip_word (l, "AS") == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  if (line_skip_FilenumChar (l))
  {
    /* OPTIONAL */
  }
  if (line_read_integer_expression (l, &FileNumber) == FALSE)
  {
    WARN_BAD_FILE_NUMBER;
    return (l);
  }
  if (FileNumber <= 0)
  {
    WARN_BAD_FILE_NUMBER;
    return (l);
  }
  /* 
     --------------------------------------------- 
     IGNORED
     --------------------------------------------- 
   */
  /* if( TRUE ) */
  {
    int Ignored;

    Ignored = 0;
    /*
       these are all parsed but ignored
     */

    if (line_skip_word (l, "BUFF"))
    {
      if (line_read_integer_expression (l, &Ignored) == FALSE)
      {
        WARN_ILLEGAL_FUNCTION_CALL;
        return (l);
      }
      if (Ignored <= 0)
      {
        WARN_ILLEGAL_FUNCTION_CALL;
        return (l);
      }
    }
    if (line_skip_word (l, "RECS"))
    {
      if (line_read_integer_expression (l, &Ignored) == FALSE)
      {
        WARN_ILLEGAL_FUNCTION_CALL;
        return (l);
      }
      if (Ignored <= 0)
      {
        WARN_ILLEGAL_FUNCTION_CALL;
        return (l);
      }
    }
  }
  /* 
     --------------------------------------------- 
     FILE MODE 
     --------------------------------------------- 
   */
  /* 
     --------------------------------------------- 
     DO IT
     --------------------------------------------- 
   */
  My->CurrentFile = find_file_by_number (FileNumber);
  if (My->CurrentFile == NULL)
  {
    My->CurrentFile = file_new ();
    My->CurrentFile->FileNumber = FileNumber;
  }
  if (My->CurrentFile->FileName != NULL)
  {
    free (My->CurrentFile->FileName);
    My->CurrentFile->FileName = NULL;
  }
  My->CurrentFile->FileName = FileName;
  FileName = NULL;
  if (My->CurrentFile->DevMode != DEVMODE_CLOSED)
  {
    My->CurrentFile->DevMode = DEVMODE_CLOSED;
  }
  if (My->CurrentFile->cfp != NULL)
  {
    bwb_fclose (My->CurrentFile->cfp);
    My->CurrentFile->cfp = NULL;
  }
  if (My->CurrentFile->buffer != NULL)
  {
    free (My->CurrentFile->buffer);
    My->CurrentFile->buffer = NULL;
  }
  My->CurrentFile->width = 0;
  My->CurrentFile->col = 1;
  My->CurrentFile->row = 1;
  My->CurrentFile->delimit = ',';
  /* open EXISTING text file for update (reading and writing) */
  if (is_empty_string (My->CurrentFile->FileName))
  {
    WARN_BAD_FILE_NAME;
    return (l);
  }
  if ((My->CurrentFile->cfp =
       fopen (My->CurrentFile->FileName, "r+")) == NULL)
  {
    /* IF END # file_number THEN line_number */
    if (My->CurrentFile->EOF_LineNumber > 0)
    {
      LineType *x;

      x = find_line_number (My->CurrentFile->EOF_LineNumber);        /* not found in the cache */
      if (x != NULL)
      {
        /* FOUND */
        line_skip_eol (l);
        x->position = 0;
        free (FileName);
        FileName = NULL;
        return x;
      }
      /* NOT FOUND */
      WARN_UNDEFINED_LINE;
      return (l);
    }
    WARN_BAD_FILE_NAME;
    return (l);
  }
  if (RecordLength > 0)
  {
    My->CurrentFile->width = RecordLength;        /* includes the terminating '\n' */
    My->CurrentFile->DevMode = DEVMODE_RANDOM;
    if ((My->CurrentFile->buffer =
         (char *) calloc (RecordLength, sizeof (char))) == NULL)
    {
      WARN_OUT_OF_MEMORY;
      return (l);
    }
    bwb_memset (My->CurrentFile->buffer, ' ', RecordLength);        /* flush */
  }
  else
  {
    My->CurrentFile->DevMode = DEVMODE_INPUT | DEVMODE_OUTPUT;
  }
  /* OK */
  free (FileName);
  FileName = NULL;
  return (l);
}

static LineType *
S70_OPEN (LineType * l)
{
  int FileNumber;
  char FileMode;
  int RecordLength;
  char *FileName;
   
  assert (l != NULL);

  FileNumber = 0;
  FileMode = 'R';
  RecordLength = 0;
  FileName = NULL;
  /* 
     SYNTAX: OPEN filenumber, filename$, INPUT | OUTPUT | APPEND | VIRTUAL
   */
  /* 
     --------------------------------------------- 
     FILE NUMBER
     --------------------------------------------- 
   */
  if (line_skip_FilenumChar (l))
  {
    /* OPTIONAL */
  }
  if (line_read_integer_expression (l, &FileNumber) == FALSE)
  {
    WARN_BAD_FILE_NUMBER;
    return (l);
  }
  if (FileNumber <= 0)
  {
    WARN_BAD_FILE_NUMBER;
    return (l);
  }
  if (line_skip_seperator (l) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  /* 
     --------------------------------------------- 
     FILE NAME
     --------------------------------------------- 
   */
  if (line_read_string_expression (l, &FileName) == FALSE)
  {
    WARN_BAD_FILE_NAME;
    return (l);
  }
  if (FileName == NULL)
  {
    WARN_BAD_FILE_NAME;
    return (l);
  }
  if (is_empty_string (FileName))
  {
    WARN_BAD_FILE_NAME;
    return (l);
  }
  if (line_skip_seperator (l) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  /* 
     --------------------------------------------- 
     FILE MODE 
     --------------------------------------------- 
   */
  /* if( TRUE ) */
  {
    if (line_skip_word (l, "INPUT"))
    {
      FileMode = 'I';
    }
    else if (line_skip_word (l, "OUTPUT"))
    {
      FileMode = 'O';
    }
    else if (line_skip_word (l, "APPEND"))
    {
      FileMode = 'A';
    }
    else if (line_skip_word (l, "VIRTUAL"))
    {
      FileMode = 'V';
    }
    else
    {
      WARN_BAD_FILE_MODE;
      return (l);
    }
  }
  /* 
     --------------------------------------------- 
     RECORD LENGTH
     --------------------------------------------- 
   */
  RecordLength = 0;
  /* 
     --------------------------------------------- 
     DO IT
     --------------------------------------------- 
   */
  bwb_file_open (FileMode, FileNumber, FileName, RecordLength);
  free (FileName);
  FileName = NULL;
  return (l);
}

static LineType *
D71_OPEN (LineType * l)
{
  int FileNumber;
  char FileMode;
  int RecordLength;
  char *FileName;
   
  assert (l != NULL);

  FileNumber = 0;
  FileMode = 'R';
  RecordLength = 0;
  FileName = NULL;
  /* 
     SYNTAX: OPEN filename$ [FOR mode] AS FILE filenumber [ ,RECORDSIZE ignored ] [ ,CLUSTERSIZE ignored ] [ ,MODE ignored ]
   */
  /* 
     --------------------------------------------- 
     FILE NAME
     --------------------------------------------- 
   */
  if (line_read_string_expression (l, &FileName) == FALSE)
  {
    WARN_BAD_FILE_NAME;
    return (l);
  }
  if (FileName == NULL)
  {
    WARN_BAD_FILE_NAME;
    return (l);
  }
  if (is_empty_string (FileName))
  {
    WARN_BAD_FILE_NAME;
    return (l);
  }
  /* 
     --------------------------------------------- 
     FILE MODE
     --------------------------------------------- 
   */
  if (line_skip_word (l, "FOR"))
  {
    if (line_skip_word (l, "INPUT"))
    {
      FileMode = 'I';
    }
    else if (line_skip_word (l, "OUTPUT"))
    {
      FileMode = 'O';
    }
    else if (line_skip_word (l, "APPEND"))
    {
      FileMode = 'A';
    }
    else if (line_skip_word (l, "RANDOM"))
    {
      FileMode = 'R';
    }
    else if (line_skip_word (l, "BINARY"))
    {
      FileMode = 'B';
    }
    else if (line_skip_word (l, "VIRTUAL"))
    {
      FileMode = 'V';
    }
    else
    {
      WARN_BAD_FILE_MODE;
      return (l);
    }
  }
  /* 
     --------------------------------------------- 
     FILE NUMBER
     --------------------------------------------- 
   */
  if (line_skip_word (l, "AS") == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  if (line_skip_word (l, "FILE") == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  if (line_skip_FilenumChar (l))
  {
    /* OPTIONAL */
  }
  if (line_read_integer_expression (l, &FileNumber) == FALSE)
  {
    WARN_BAD_FILE_NUMBER;
    return (l);
  }
  if (FileNumber <= 0)
  {
    WARN_BAD_FILE_NUMBER;
    return (l);
  }
  /* 
     --------------------------------------------- 
     RECORD LENGTH
     --------------------------------------------- 
   */
  RecordLength = 0;
  while (line_is_eol (l) == FALSE)
  {
    int Ignored;

    Ignored = 0;
    /*
       these are all parsed but ignored
     */
    if (line_skip_seperator (l))
    {
      /* OK */
    }
    else if (line_skip_word (l, "RECORDSIZE"))
    {
      if (line_read_integer_expression (l, &Ignored) == FALSE)
      {
        WARN_SYNTAX_ERROR;
        return (l);
      }
    }
    else if (line_skip_word (l, "CLUSTERSIZE"))
    {
      if (line_read_integer_expression (l, &Ignored) == FALSE)
      {
        WARN_SYNTAX_ERROR;
        return (l);
      }
    }
    else if (line_skip_word (l, "FILESIZE"))
    {
      if (line_read_integer_expression (l, &Ignored) == FALSE)
      {
        WARN_SYNTAX_ERROR;
        return (l);
      }
    }
    else if (line_skip_word (l, "MODE"))
    {
      if (line_read_integer_expression (l, &Ignored) == FALSE)
      {
        WARN_SYNTAX_ERROR;
        return (l);
      }
    }
    else
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
  }
  /* 
     --------------------------------------------- 
     DO IT
     --------------------------------------------- 
   */
  bwb_file_open (FileMode, FileNumber, FileName, RecordLength);
  free (FileName);
  FileName = NULL;
  return (l);
}

static LineType *
H80_OPEN (LineType * l)
{
  int FileNumber;
  char FileMode;
  int RecordLength;
  char *FileName;
   
  assert (l != NULL);

  FileNumber = 0;
  FileMode = 'R';
  RecordLength = 0;
  FileName = NULL;
  /* 
     SYNTAX: OPEN filename$ FOR mode AS FILE filenumber 
   */
  /* 
     --------------------------------------------- 
     FILE NAME
     --------------------------------------------- 
   */
  if (line_read_string_expression (l, &FileName) == FALSE)
  {
    WARN_BAD_FILE_NAME;
    return (l);
  }
  if (FileName == NULL)
  {
    WARN_BAD_FILE_NAME;
    return (l);
  }
  if (is_empty_string (FileName))
  {
    WARN_BAD_FILE_NAME;
    return (l);
  }
  /* 
     --------------------------------------------- 
     FILE MODE
     --------------------------------------------- 
   */
  if (line_skip_word (l, "FOR") == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  if (line_skip_word (l, "READ"))
  {
    FileMode = 'I';
  }
  else if (line_skip_word (l, "WRITE"))
  {
    FileMode = 'O';
  }
  else if (line_skip_word (l, "VIRTUAL"))
  {
    FileMode = 'V';
  }
  else
  {
    WARN_BAD_FILE_MODE;
    return (l);
  }
  /* 
     --------------------------------------------- 
     FILE NUMBER
     --------------------------------------------- 
   */
  if (line_skip_word (l, "AS") == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  if (line_skip_word (l, "FILE") == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  if (line_skip_FilenumChar (l))
  {
    /* OPTIONAL */
  }
  if (line_read_integer_expression (l, &FileNumber) == FALSE)
  {
    WARN_BAD_FILE_NUMBER;
    return (l);
  }
  if (FileNumber <= 0)
  {
    WARN_BAD_FILE_NUMBER;
    return (l);
  }
  /* 
     --------------------------------------------- 
     RECORD LENGTH
     --------------------------------------------- 
   */
  RecordLength = 0;
  /* 
     --------------------------------------------- 
     DO IT
     --------------------------------------------- 
   */
  bwb_file_open (FileMode, FileNumber, FileName, RecordLength);
  free (FileName);
  FileName = NULL;
  return (l);
}


static LineType *
M80_OPEN (LineType * l)
{
  int FileNumber;
  char FileMode;
  int RecordLength;
  char *FileName;
   
  assert (l != NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);

  FileNumber = 0;
  FileMode = 'R';
  RecordLength = 0;
  FileName = NULL;
  /* 
     SYNTAX: OPEN filename$ [FOR mode] AS filenumber [LEN reclen] 
   */
  /* 
     --------------------------------------------- 
     FILE NAME
     --------------------------------------------- 
   */
  if (line_read_string_expression (l, &FileName) == FALSE)
  {
    WARN_BAD_FILE_NAME;
    return (l);
  }
  if (FileName == NULL)
  {
    WARN_BAD_FILE_NAME;
    return (l);
  }
  if (is_empty_string (FileName))
  {
    WARN_BAD_FILE_NAME;
    return (l);
  }
  /* 
     --------------------------------------------- 
     FILE MODE
     --------------------------------------------- 
   */
  if (line_skip_word (l, "FOR"))
  {
    if (line_skip_word (l, "INPUT"))
    {
      FileMode = 'I';
    }
    else if (line_skip_word (l, "OUTPUT"))
    {
      FileMode = 'O';
    }
    else if (line_skip_word (l, "APPEND"))
    {
      FileMode = 'A';
    }
    else if (line_skip_word (l, "RANDOM"))
    {
      FileMode = 'R';
    }
    else if (line_skip_word (l, "BINARY"))
    {
      FileMode = 'B';
    }
    else if (line_skip_word (l, "VIRTUAL"))
    {
      FileMode = 'V';
    }
    else
    {
      WARN_BAD_FILE_MODE;
      return (l);
    }
  }

  if (My->CurrentVersion->OptionVersionValue & (H14))
  {
    /*
     **
     ** these are parsed but ignored
     **
     */
    if (line_skip_word (l, "ACCESS"))
    {
      /* ACCESS */
      if (line_skip_word (l, "READ"))
      {
        /* ACCESS READ */
        if (line_skip_word (l, "WRITE"))
        {
          /* ACCESS READ WRITE */
        }
      }
      else if (line_skip_word (l, "WRITE"))
      {
        /* ACCESS WRITE */
      }
    }
    if (line_skip_word (l, "SHARED"))
    {
      /* SHARED */
    }
    else if (line_skip_word (l, "LOCK"))
    {
      /* LOCK */
      if (line_skip_word (l, "READ"))
      {
        /* LOCK READ */
      }
      else if (line_skip_word (l, "WRITE"))
      {
        /* LOCK WRITE */
      }
    }
  }
  /* 
     --------------------------------------------- 
     FILE NUMBER
     --------------------------------------------- 
   */
  if (line_skip_word (l, "AS") == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  if (My->CurrentVersion->OptionVersionValue & (H14))
  {
    if (line_skip_word (l, "FILE"))
    {
      /* OPTIONAL */
    }
  }

  if (line_skip_FilenumChar (l))
  {
    /* OPTIONAL */
  }
  if (line_read_integer_expression (l, &FileNumber) == FALSE)
  {
    WARN_BAD_FILE_NUMBER;
    return (l);
  }
  if (FileNumber <= 0)
  {
    WARN_BAD_FILE_NUMBER;
    return (l);
  }
  /* 
     --------------------------------------------- 
     RECORD LENGTH
     --------------------------------------------- 
   */
  RecordLength = 0;
  if (line_skip_word (l, "LEN"))
  {
    if (line_skip_EqualChar (l))
    {
      /* OPTIONAL */
    }
    if (line_read_integer_expression (l, &RecordLength) == FALSE)
    {
      WARN_FIELD_OVERFLOW;
      return (l);
    }
    if (RecordLength <= 0)
    {
      WARN_FIELD_OVERFLOW;
      return (l);
    }
  }
  /* 
     --------------------------------------------- 
     DO IT
     --------------------------------------------- 
   */
  bwb_file_open (FileMode, FileNumber, FileName, RecordLength);
  free (FileName);
  FileName = NULL;
  return (l);
}

static LineType *
T79_OPEN (LineType * l)
{
  int FileNumber;
  char FileMode;
  int RecordLength;
  char *FileName;
   
  assert (l != NULL);

  FileNumber = 0;
  FileMode = 'R';
  RecordLength = 0;
  FileName = NULL;
  /* 
     SYNTAX: OPEN [NEW | OLD] filename$ AS filenumber
   */
  /* 
     --------------------------------------------- 
     FILE MODE
     --------------------------------------------- 
   */
  if (line_skip_word (l, "NEW"))
  {
    FileMode = 'O';
  }
  else if (line_skip_word (l, "OLD"))
  {
    FileMode = 'I';
  }
  else if (line_skip_word (l, "VIRTUAL"))
  {
    FileMode = 'V';
  }
  /* 
     --------------------------------------------- 
     FILE NAME
     --------------------------------------------- 
   */
  if (line_read_string_expression (l, &FileName) == FALSE)
  {
    WARN_BAD_FILE_NAME;
    return (l);
  }
  if (FileName == NULL)
  {
    WARN_BAD_FILE_NAME;
    return (l);
  }
  if (is_empty_string (FileName))
  {
    WARN_BAD_FILE_NAME;
    return (l);
  }
  /* 
     --------------------------------------------- 
     FILE NUMBER
     --------------------------------------------- 
   */
  if (line_skip_word (l, "AS") == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  if (line_skip_FilenumChar (l))
  {
    /* OPTIONAL */
  }
  if (line_read_integer_expression (l, &FileNumber) == FALSE)
  {
    WARN_BAD_FILE_NUMBER;
    return (l);
  }
  if (FileNumber <= 0)
  {
    WARN_BAD_FILE_NUMBER;
    return (l);
  }
  /* 
     --------------------------------------------- 
     RECORD LENGTH
     --------------------------------------------- 
   */
  RecordLength = 0;
  /* 
     --------------------------------------------- 
     DO IT
     --------------------------------------------- 
   */
  bwb_file_open (FileMode, FileNumber, FileName, RecordLength);
  free (FileName);
  FileName = NULL;
  return (l);
}

LineType *
bwb_OPEN (LineType * l)
{
   
  assert (l != NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);

  if (My->CurrentVersion->OptionVersionValue & (S70 | I70 | I73))
  {
    return S70_OPEN (l);
  }
  if (My->CurrentVersion->OptionVersionValue & (D71))
  {
    return D71_OPEN (l);
  }
  if (My->CurrentVersion->OptionVersionValue & (C77))
  {
    return C77_OPEN (l);
  }
  if (My->CurrentVersion->OptionVersionValue & (H80))
  {
    return H80_OPEN (l);
  }
  if (My->CurrentVersion->OptionVersionValue & (T79 | R86))
  {
    return T79_OPEN (l);
  }
  /* default */
  return M80_OPEN (l);
}

/***************************************************************
  
        FUNCTION:       bwb_name()
  
   DESCRIPTION:    This function implements the BASIC NAME
         command to rename a disk file.
  
   SYNTAX:         NAME old_filename AS new_filename
  
***************************************************************/


LineType *
bwb_NAME (LineType * l)
{
  char *OldName;
  char *NewName;
   
  assert (l != NULL);

  OldName = NULL;
  NewName = NULL;
  if (line_read_string_expression (l, &OldName) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  if (is_empty_string (OldName))
  {
    WARN_BAD_FILE_NAME;
    return (l);
  }
  if (line_skip_word (l, "AS") == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  if (line_read_string_expression (l, &NewName) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  if (is_empty_string (NewName))
  {
    WARN_BAD_FILE_NAME;
    return (l);
  }
  /* try to rename the file */
  if (rename (OldName, NewName) != 0)
  {
    WARN_BAD_FILE_NAME;
  }
  free (OldName);
  OldName = NULL;
  free (NewName);
  NewName = NULL;
  return (l);
}


/* EOF */
