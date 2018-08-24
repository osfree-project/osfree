/***************************************************************
  
        bwb_inp.c       Input Routines
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


static LineType *C77_file_input (LineType * Line, int FileNumber);
static LineType *C77_file_input_finish (LineType * Line);
static LineType *C77_file_input_line (LineType * Line);
static LineType *C77_user_input_line (LineType * Line, char *Prompt,
                                      int IsDisplayQuestionMark);
static void CleanTextInput (char *buffer);
static LineType *D71_GET (LineType * Line);
static LineType *data_if_end (LineType * Line);
static LineType *data_restore (LineType * Line);
static ResultType file_data (VariableType * Variable, char *tbuf, int tlen);
static LineType *file_if_end (LineType * Line);
static LineType *file_input (LineType * Line);
static LineType *file_read_matrix (LineType * Line);
static LineType *file_restore (LineType * Line);
static LineType *H14_GET (LineType * Line);
static ResultType input_data (VariableType * Variable, char *tbuf, int tlen);
static ResultType parse_number (char *buffer, int *position, VariantType * X,
                                int IsConsoleInput);
static ResultType parse_string (char *buffer, int *position, VariantType * X);
static ResultType parse_string_isquoted (char *buffer, int *position,
                                         VariantType * X);
static ResultType parse_string_unquoted (char *buffer, int *position,
                                         VariantType * X);
static ResultType read_data (VariableType * Variable);
static LineType *read_file (LineType * Line);
static LineType *read_list (LineType * Line);
static LineType *S70_GET (LineType * Line);
static LineType *user_input_loop (LineType * Line);
static ResultType user_input_values (LineType * Line, char *buffer,
                                     int IsReal);


int
bwb_is_eof (FILE * fp)
{
  /* 
     Have you ever wondered why C file I/O is slow?   Here is the reason:
     feof() is not set until after a file read error occurs; sad but true. 
     In order to determine whether you are at the end-of-file,
     you have to call both ftell() and fseek() twice,
     which effectively trashes any I/O cache scheme.  
   */
   
  assert (fp != NULL);

  if (fp != NULL)
  {
    long current;
    long total;

    current = ftell (fp);
    fseek (fp, 0, SEEK_END);
    total = ftell (fp);
    if (total == current)
    {
      /* EOF */
      return TRUE;
    }
    else
    {
      /* NOT EOF */
      fseek (fp, current, SEEK_SET);
      return FALSE;
    }
  }
  /* a closed file is always EOF */
  return TRUE;
}


static void
CleanTextInput (char *buffer)
{
  /*
   **
   ** Clean the TEXT in the INPUT buffer after fgets().  Not for RANDOM or BINARY.
   **
   */
  char *E;
   
  assert (buffer != NULL);
  /*
   **
   ** some compilers remove CR, but not LF.
   ** some compilers remove LF, but not CR.
   ** some compilers remove CR/LF but not LF/CR.
   ** some compilers remove both CR and LF.
   ** some compilers remove the first CR or LF, but not the second LF or CR.
   ** some compilers don't remove either CR or LF.
   ** and so on.
   **
   */
  E = bwb_strchr (buffer, '\r');
  if (E != NULL)
  {
    *E = NulChar;
  }
  E = bwb_strchr (buffer, '\n');
  if (E != NULL)
  {
    *E = NulChar;
  }
  /*
   **
   ** Suppress all control characters.
   ** In theory, there should not be any control characters at all.
   ** In reality, they occassionally occur.
   ** 
   */
  while (*buffer)
  {
    if (bwb_isprint (*buffer) == FALSE)
    {
      *buffer = ' ';
    }
    buffer++;
  }
}



/***************************************************************
  
      FUNCTION:       bwx_input()
  
   DESCRIPTION:    This function outputs the string pointed
         to by 'prompt', then inputs a character
         string.
  
***************************************************************/

extern int
bwx_input (char *prompt, int IsDisplayQuestionMark, char *answer, int MaxLen)
{
   
  assert (answer != NULL);
  assert(My != NULL);
  assert(My->SYSOUT != NULL);
  assert(My->SYSOUT->cfp != NULL);
  assert(My->SYSIN != NULL);
  assert(My->SYSIN->cfp != NULL);

  if (prompt != NULL)
  {
    while (*prompt)
    {
      if (*prompt == '\n')
      {
        My->SYSOUT->col = 0;        /* incremented below */
        My->SYSOUT->row++;
      }
      else
        if (My->SYSOUT->width > 0
            && My->SYSOUT->col > My->SYSOUT->width /* && *prompt != '\n' */ )
      {
        fputc ('\n', My->SYSOUT->cfp);
        My->SYSOUT->col = 0;        /* incremented below */
        My->SYSOUT->row++;
      }
      fputc (*prompt, My->SYSOUT->cfp);
      My->SYSOUT->col++;
      prompt++;
    }
  }
  if (IsDisplayQuestionMark)
  {
    fputs ("? ", My->SYSOUT->cfp);
    My->SYSOUT->col += 2;
  }
  fflush (My->SYSOUT->cfp);
  /*
   **
   ** for PTR or OPTION STDIN ...
   **
   */
  if (My->SYSIN->cfp != stdin)
  {
    /* this file was opened by PTR or OPTION STDIN commands */
    if (fgets (answer, MaxLen, My->SYSIN->cfp))        /* bwx_input */
    {
      answer[MaxLen] = NulChar;
      CleanTextInput (answer);
      fputs (answer, My->SYSOUT->cfp);
      fputc ('\n', My->SYSOUT->cfp);
      fflush (My->SYSOUT->cfp);
      ResetConsoleColumn ();
      return TRUE;
    }
    /* stop reading from PTR or OPTION STDIN once all INPUT lines have been read */
    bwb_fclose (My->SYSIN->cfp);
    My->SYSIN->cfp = stdin;
    /* INPUT reverts to stdin */
  }
  /* My->SYSIN->cfp == stdin */
  /*
   **
   ** ... for PTR or OPTION STDIN
   **
   */
  /*
   **
   ** for automated testing ... 
   **
   */
  if (My->ExternalInputFile != NULL)
  {
    /* this file was opened by --TAPE command line parameter */
    if (fgets (answer, MaxLen, My->ExternalInputFile))        /* bwx_input */
    {
      answer[MaxLen] = NulChar;
      CleanTextInput (answer);
      fputs (answer, My->SYSOUT->cfp);
      fputc ('\n', My->SYSOUT->cfp);
      fflush (My->SYSOUT->cfp);
      ResetConsoleColumn ();
      return TRUE;
    }
    /* stop reading from --TAPE once all INPUT lines have been read */
    bwb_fclose (My->ExternalInputFile);
    My->ExternalInputFile = NULL;
    /* INPUT reverts to My->SYSIN->cfp */
  }
  /*
   **
   ** ... for automated testing
   **
   */
  if (fgets (answer, MaxLen, My->SYSIN->cfp))        /* bwx_input */
  {
    /* this is stdin */
    answer[MaxLen] = NulChar;
    CleanTextInput (answer);
    ResetConsoleColumn ();
    return TRUE;
  }
  /* nothing was read from stdin */
  answer[0] = NulChar;
  CleanTextInput (answer);
  ResetConsoleColumn ();
  return FALSE;
}


extern LineType *
bwb_BACKSPACE (LineType * Line)
{

   
  assert (Line != NULL);
  assert(My != NULL);
  assert(My->SYSIN != NULL);
  assert(My->SYSIN->cfp != NULL);

  My->CurrentFile = My->SYSIN;

  if (line_skip_FilenumChar (Line))
  {
    /* BACKSPACE # filenum */
    int FileNumber;

    if (line_read_integer_expression (Line, &FileNumber) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (Line);
    }
    if (FileNumber < 0)
    {
      /* "BACKSPACE # -1" is silently ignored */
      return (Line);
    }
    if (FileNumber == 0)
    {
      /* "BACKSPACE # 0" is silently ignored */
      return (Line);
    }
    My->CurrentFile = find_file_by_number (FileNumber);
    if (My->CurrentFile == NULL)
    {
      WARN_BAD_FILE_NUMBER;
      return (Line);
    }
    if ((My->CurrentFile->DevMode & DEVMODE_READ) == 0)
    {
      WARN_BAD_FILE_NUMBER;
      return (Line);
    }
    /* not for the console */
    /* if( TRUE ) */
    {
      FILE *f;
      long Offset;
      int DelimiterCount;
      int InQuote;
      int C;

      f = My->CurrentFile->cfp;
      Offset = ftell (f);
      Offset--;
      DelimiterCount = 0;
      InQuote = FALSE;

    AGAIN:
      if (Offset <= 0)
      {
        goto DONE;
      }
      fseek (f, Offset, SEEK_SET);
      C = fgetc (f);

      if (InQuote)
      {
        if (C == My->CurrentVersion->OptionQuoteChar)
        {
          InQuote = FALSE;
        }
        Offset--;
        goto AGAIN;
      }

      if (C == My->CurrentVersion->OptionQuoteChar)
      {
        InQuote = TRUE;
        Offset--;
        goto AGAIN;
      }


      if (C == ',')
      {
        DelimiterCount++;
        if (DelimiterCount > 1)
        {
          Offset++;
          goto DONE;
        }
        Offset--;
        goto AGAIN;
      }

      if (C == '\n')
      {
        DelimiterCount++;
        if (DelimiterCount > 1)
        {
          Offset++;
          goto DONE;
        }
        Offset--;
        if (Offset <= 0)
        {
          goto DONE;
        }
        fseek (f, Offset, SEEK_SET);
        C = fgetc (f);
        if (C == '\r')
        {
          Offset--;
        }
        goto AGAIN;
      }

      if (C == '\r')
      {
        DelimiterCount++;
        if (DelimiterCount > 1)
        {
          Offset++;
          goto DONE;
        }
        Offset--;
        if (Offset <= 0)
        {
          goto DONE;
        }
        fseek (f, Offset, SEEK_SET);
        C = fgetc (f);
        if (C == '\n')
        {
          Offset--;
        }
        goto AGAIN;
      }

      Offset--;
      goto AGAIN;

    DONE:
      if (Offset < 0)
      {
        Offset = 0;
      }
      fseek (f, Offset, SEEK_SET);
    }
  }
  /* BACKSPACE for console is silently ignored */
  return (Line);
}




/***************************************************************
  
        FUNCTION:       bwb_read()
  
        DESCRIPTION:    This function implements the BASIC READ
                        statement.
  
        SYNTAX:         READ variable[, variable...]
  
***************************************************************/

static LineType *
C77_file_input (LineType * Line, int FileNumber)
{
  /* 
     CBASIC-II: SERIAL & RANDOM file reads
     READ  # FileNumber                ; [ scalar   [ , ... ] ] ' SERIAL
     READ  # FileNumber , RecordNumber ; [ scalar   [ , ... ] ] ' RANDOM
   */
  assert (Line != NULL);
  assert(My != NULL);

  if (FileNumber <= 0)
  {
    WARN_BAD_FILE_NUMBER;
    return (Line);
  }
  /* normal file */
  My->CurrentFile = find_file_by_number (FileNumber);
  if (My->CurrentFile == NULL)
  {
    WARN_BAD_FILE_NUMBER;
    return (Line);
  }
  if (line_skip_CommaChar (Line) /* comma-specific */ )
  {
    /* 
       READ # FileNumber , RecordNumber ; [ scalar   [ , ... ] ] ' RANDOM
     */
    /* get the RecordNumber */
    int RecordNumber;

    if ((My->CurrentFile->DevMode & DEVMODE_RANDOM) == 0)
    {
      WARN_BAD_FILE_MODE;
      return (Line);
    }
    if (My->CurrentFile->width <= 0)
    {
      WARN_FIELD_OVERFLOW;
      return (Line);
    }
    if (line_read_integer_expression (Line, &RecordNumber) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (Line);
    }
    if (RecordNumber <= 0)
    {
      WARN_BAD_RECORD_NUMBER;
      return (Line);
    }
    RecordNumber--;                /* BASIC to C */
    /* if( TRUE ) */
    {
      long offset;
      offset = RecordNumber;
      offset *= My->CurrentFile->width;
      fseek (My->CurrentFile->cfp, offset, SEEK_SET);
    }
  }

  if (line_is_eol (Line))
  {
    /* READ # filenum          */
    /* READ # filenum , recnum */
    return (Line);
  }

  if (line_skip_SemicolonChar (Line) /* semicolon specific */ )
  {
    /* READ # filenum          ; */
    /* READ # filenum , recnum ; */
  }
  else
  {
    WARN_SYNTAX_ERROR;
    return (Line);
  }
  if (line_is_eol (Line))
  {
    return (Line);
  }
  /* input is not from #0, so branch to file_input() */
  return file_input (Line);
}

static LineType *
data_if_end (LineType * Line)
{
  WARN_OUT_OF_DATA;
  return (Line);
}

static ResultType
read_data (VariableType * Variable)
{
  /*
   **
   ** read one DATA item
   **
   */
  ResultType Result;
  VariantType Variant;
  VariantType *X;
   
  assert (Variable != NULL);
  assert(My != NULL);
  assert(My->SYSIN != NULL);
  assert(My->SYSIN->cfp != NULL);
  assert (My->CurrentFile == My->SYSIN);
  assert(My->DataLine != NULL);
  assert(My->EndMarker != NULL);

  Result = RESULT_UNPARSED;
  X = &Variant;
  CLEAR_VARIANT (X);
  if (My->DataLine == My->EndMarker)
  {
    return RESULT_UNPARSED;
  }
  if (My->DataLine->cmdnum != C_DATA)
  {
    WARN_INTERNAL_ERROR;
    return RESULT_UNPARSED;
  }
  if (VAR_IS_STRING (Variable))
  {
    Result = parse_string (My->DataLine->buffer, &My->DataPosition, X);
  }
  else
  {
    Result = parse_number (My->DataLine->buffer, &My->DataPosition, X, FALSE);
  }
  if (Result == RESULT_UNPARSED)
  {
    WARN_BAD_DATA;
  }
  if (Result != RESULT_SUCCESS)
  {
    return Result;
  }
  /*
   **
   ** OK
   **
   */
  if (X->VariantTypeCode == StringTypeCode
      && My->CurrentVersion->OptionFlags & OPTION_BUGS_ON ) /* DATA allows embedded quote pairs */
  {
    int i;
    int n;
    n = X->Length;
    for (i = 0; i < n; i++)
    {
      if (X->Buffer[i + 0] == My->CurrentVersion->OptionQuoteChar
          && X->Buffer[i + 1] == My->CurrentVersion->OptionQuoteChar)
      {
        bwb_strncpy (&X->Buffer[i + 0], &X->Buffer[i + 1], n - i);
        n--;
      }
    }
    X->Length = n;
  }
  if (var_set (Variable, X) == FALSE)
  {
    WARN_VARIABLE_NOT_DECLARED;
    return RESULT_UNPARSED;
  }
  /*
   **
   ** Note: do NOT free() or RELEASE_VARIANT because 'X->Buffer' points into 'buffer'
   **
   */
  if (buff_is_eol (My->DataLine->buffer, &My->DataPosition))
  {
    /* at the end of the current DATA statement */
    if (My->CurrentVersion->OptionFlags & OPTION_COVERAGE_ON)
    {
      /* this line has been READ */
      My->DataLine->LineFlags |= LINE_EXECUTED;
    }
    My->DataLine = My->DataLine->OtherLine;
    My->DataPosition = My->DataLine->Startpos;
    return RESULT_SUCCESS;
  }
  if (buff_skip_char (My->DataLine->buffer, &My->DataPosition, My->CurrentFile->delimit))        /* buff_skip_comma */
  {
    return RESULT_SUCCESS;
  }
  /* garbage after the value we just READ */
  WARN_BAD_DATA;
  return RESULT_UNPARSED;
}

static LineType *
read_list (LineType * Line)
{
  /* READ varlist */
  assert (Line != NULL);
  assert(My != NULL);
  assert(My->SYSIN != NULL);
  assert(My->SYSIN->cfp != NULL);
  assert (My->CurrentFile == My->SYSIN);

  do
  {
    VariableType *Variable;

    /* get a variable */
    if ((Variable = line_read_scalar (Line)) == NULL)
    {
      WARN_SYNTAX_ERROR;
      return (Line);
    }
    /* READ data into the variable */
    if (read_data (Variable) != RESULT_SUCCESS)
    {
      return data_if_end (Line);
    }
  }
  while (line_skip_seperator (Line));
  return (Line);
}

static LineType *
read_file (LineType * Line)
{
  /* READ # filenum, varlist */
  int FileNumber;
  assert (Line != NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);

  if (line_read_integer_expression (Line, &FileNumber) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (Line);
  }
  if (My->CurrentVersion->OptionVersionValue & (C77))
  {
    return C77_file_input (Line, FileNumber);
  }
  /* 
     SERIAL file reads:
     READ # FileNumber   
     READ # FileNumber [, scalar]
   */
  if (line_skip_seperator (Line))
  {
    /* required */
  }
  else
  {
    WARN_SYNTAX_ERROR;
    return (Line);
  }
  if (FileNumber < 0)
  {
    /* "READ # -1" is an error */
    WARN_BAD_FILE_NUMBER;
    return (Line);
  }
  if (FileNumber > 0)
  {
    /* normal file */
    My->CurrentFile = find_file_by_number (FileNumber);
    if (My->CurrentFile == NULL)
    {
      WARN_BAD_FILE_NUMBER;
      return (Line);
    }
    if ((My->CurrentFile->DevMode & DEVMODE_READ) == 0)
    {
      WARN_BAD_FILE_NUMBER;
      return (Line);
    }
    /* input is not from #0, so branch to file_input() */
    return file_input (Line);
  }
  /* "READ # 0, varlist" is the same as "READ varlist" */
  return read_list (Line);
}

extern LineType *
bwb_READ (LineType * Line)
{

   
  assert (Line != NULL);
  assert(My != NULL);
  assert(My->SYSIN != NULL);
  assert(My->SYSIN->cfp != NULL);

  My->CurrentFile = My->SYSIN;

  if (line_skip_FilenumChar (Line))
  {
    return read_file (Line);
  }
  return read_list (Line);
}

/***************************************************************
  
        FUNCTION:       bwb_data()
  
        DESCRIPTION:    This function implements the BASIC DATA
                        statement, although at the point at which
                        DATA statements are encountered, no
                        processing is done.  All actual processing
                        of DATA statements is accomplished by READ
                        (bwb_read()).
  
        SYNTAX:         DATA constant[, constant]...
  
***************************************************************/

extern LineType *
bwb_DATA (LineType * Line)
{
   
  assert (Line != NULL);

  if (Line->LineFlags & (LINE_USER))
  {
    WARN_ILLEGAL_DIRECT;
    return (Line);
  }
  line_skip_eol (Line);
  return (Line);
}






/***************************************************************
  
        FUNCTION:       bwb_restore()
  
        DESCRIPTION:    This function implements the BASIC RESTORE
                        statement.
  
        SYNTAX:         RESTORE [line number]
  
***************************************************************/

extern LineType *
bwb_RESET (LineType * Line)
{
  /* RESET filename$ [, ...] */
  VariantType E;
  VariantType *e;
   
  assert (Line != NULL);
  assert(My != NULL);
  assert(My->SYSIN != NULL);
  assert(My->SYSIN->cfp != NULL);

  e = &E;                        /* no leaks */
  My->CurrentFile = My->SYSIN;

  do
  {
    CLEAR_VARIANT (e);
    if (line_read_expression (Line, e) == FALSE)        /* bwb_RESET */
    {
      WARN_SYNTAX_ERROR;
      return (Line);
    }
    if (e->VariantTypeCode == StringTypeCode)
    {
      /* STRING */
      /* RESET filename$ ... */
      My->CurrentFile = find_file_by_name (e->Buffer);
    }
    else
    {
      /* NUMBER -- file must already be OPEN */
      /* RESET filenumber ... */
      My->CurrentFile = find_file_by_number ((int) bwb_rint (e->Number));
    }
    RELEASE_VARIANT (e);
    if (My->CurrentFile == NULL)
    {
      /* file not OPEN */
      /* silently ignored */
    }
    else if (My->CurrentFile == My->SYSIN)
    {
      /* silently ignored */
    }
    else if (My->CurrentFile == My->SYSOUT)
    {
      /* silently ignored */
    }
    else if (My->CurrentFile == My->SYSPRN)
    {
      /* silently ignored */
    }
    else
    {
      /* normal file is OPEN */
      My->CurrentFile->width = 0;
      My->CurrentFile->col = 1;
      My->CurrentFile->row = 1;
      My->CurrentFile->delimit = ',';
      fseek (My->CurrentFile->cfp, 0, SEEK_SET);
    }
  }
  while (line_skip_seperator (Line));
  return (Line);
}

extern LineType *
bwb_CLOSE (LineType * Line)
{
  /* CLOSE filename$  ' can be any string expression */
  /* CLOSE filenumber ' can be any numeric expression */
   
  assert (Line != NULL);
  assert(My != NULL);
  assert(My->SYSIN != NULL);
  assert(My->SYSIN->cfp != NULL);
  assert(My->SYSOUT != NULL);
  assert(My->SYSOUT->cfp != NULL);
  assert(My->SYSPRN != NULL);
  assert(My->SYSPRN->cfp != NULL);

  My->CurrentFile = My->SYSIN;

  if (line_is_eol (Line))
  {
    /* CLOSE */
    FileType *F;

    for (F = My->FileHead; F != NULL; F = F->next)
    {
      field_close_file (F);
      file_clear (F);
    }
    return (Line);
  }

  do
  {
    VariantType E;
    VariantType *e;
    e = &E;

    if (line_read_expression (Line, e) == FALSE)        /* bwb_CLOSE */
    {
      WARN_SYNTAX_ERROR;
      return (Line);
    }
    if (e->VariantTypeCode == StringTypeCode)
    {
      /* STRING */
      /* CLOSE filename$ ... */
      My->CurrentFile = find_file_by_name (e->Buffer);
    }
    else
    {
      /* CLOSE filenumber */
      My->CurrentFile = find_file_by_number (e->Number);
    }
    if (My->CurrentFile == NULL)
    {
      /* file not OPEN */
      /* silently ignored */
    }
    else if (My->CurrentFile == My->SYSIN)
    {
      /* silently ignored */
    }
    else if (My->CurrentFile == My->SYSOUT)
    {
      /* silently ignored */
    }
    else if (My->CurrentFile == My->SYSPRN)
    {
      /* silently ignored */
    }
    else
    {
      /* normal file is OPEN */
      field_close_file (My->CurrentFile);
      file_clear (My->CurrentFile);
    }
    RELEASE_VARIANT (e);
  }
  while (line_skip_seperator (Line));
  return (Line);
}

static LineType *
data_restore (LineType * Line)
{
  int LineNumber;
  LineType *x;
  assert (Line != NULL);
  assert(My != NULL);

  if (line_is_eol (Line))
  {
    /* RESTORE */
    assert (My->StartMarker != NULL);
    My->DataLine = My->StartMarker->OtherLine;
    assert (My->DataLine != NULL);
    My->DataPosition = My->DataLine->Startpos;
    return (Line);
  }
  if (line_read_integer_expression (Line, &LineNumber))
  {
    /* RESTORE linenumber */
    x = find_line_number (LineNumber);        /* RESTORE 100 */
    if (x != NULL)
    {
      for (; x->cmdnum != C_DATA && x != My->EndMarker; x = x->next);
      My->DataLine = x;
      assert (My->DataLine != NULL);
      My->DataPosition = My->DataLine->Startpos;
      return (Line);
    }
  }
  WARN_SYNTAX_ERROR;
  return (Line);
}
static LineType *
file_restore (LineType * Line)
{
  /* RESTORE # FileNumber */
  int FileNumber;
  assert (Line != NULL);
  assert(My != NULL);

  if (line_read_integer_expression (Line, &FileNumber) == FALSE)
  {
    WARN_BAD_FILE_NUMBER;
    return (Line);
  }
  if (FileNumber < 0)
  {
    /* "RESTORE # -1" is silently ignored */
    return (Line);
  }
  if (FileNumber > 0)
  {
    /* normal file */
    My->CurrentFile = find_file_by_number (FileNumber);
    if (My->CurrentFile == NULL)
    {
      WARN_BAD_FILE_NUMBER;
      return (Line);
    }
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
    if (is_empty_string (My->CurrentFile->FileName))
    {
      WARN_BAD_FILE_NAME;
      return (Line);
    }
    if (bwb_strcmp (My->CurrentFile->FileName, "*") != 0)
    {
      if ((My->CurrentFile->cfp =
           fopen (My->CurrentFile->FileName, "r")) == NULL)
      {
        WARN_BAD_FILE_NAME;
        return (Line);
      }
      My->CurrentFile->DevMode = DEVMODE_INPUT;
    }
    /* OK */
    return (Line);
  }
  /* "RESTORE # 0" is the same as "RESTORE"  */
  return data_restore (Line);
}

extern LineType *
bwb_RESTORE (LineType * Line)
{
   
  assert (Line != NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);
  assert(My->SYSIN != NULL);
  assert(My->SYSIN->cfp != NULL);

  My->CurrentFile = My->SYSIN;

  if (line_skip_FilenumChar (Line))
  {
    return file_restore (Line);
  }
  if (My->CurrentVersion->OptionVersionValue & (S70 | I70 | I73))
  {
    /* RESTORE [comment] */
    line_skip_eol (Line);
    /* fall-thru */
  }
  return data_restore (Line);
}

/***************************************************************
  
        FUNCTION:       bwb_input()
  
        DESCRIPTION:    This function implements the BASIC INPUT
                        statement.
  
        SYNTAX:         INPUT [;][prompt$;]variable[$,variable]...
                        INPUT#n variable[$,variable]...
  
***************************************************************/

static LineType *
S70_GET (LineType * Line)
{
  /* GET filename$ , scalar [, ...] */
  VariantType E;
  VariantType *e;
   
  assert (Line != NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);
  assert(My->SYSIN != NULL);
  assert(My->SYSIN->cfp != NULL);

  e = &E;
  My->CurrentFile = My->SYSIN;

  if (line_read_expression (Line, e) == FALSE)        /* bwb_GET */
  {
    WARN_SYNTAX_ERROR;
    return (Line);
  }
  if (e->VariantTypeCode == StringTypeCode)
  {
    /* STRING */
    /* GET filename$ ... */
    if (is_empty_string (e->Buffer))
    {
      /* GET "", ... is an error */
      WARN_BAD_FILE_NAME;
      return (Line);
    }
    My->CurrentFile = find_file_by_name (e->Buffer);
    if (My->CurrentFile == NULL)
    {
      /* implicitly OPEN for reading */
      My->CurrentFile = file_new ();
      My->CurrentFile->cfp = fopen (e->Buffer, "r");
      if (My->CurrentFile->cfp == NULL)
      {
        WARN_BAD_FILE_NAME;
        return (Line);
      }
      My->CurrentFile->FileNumber = file_next_number ();
      My->CurrentFile->DevMode = DEVMODE_INPUT;
      My->CurrentFile->width = 0;
      /* WIDTH == RECLEN */
      My->CurrentFile->col = 1;
      My->CurrentFile->row = 1;
      My->CurrentFile->delimit = ',';
      My->CurrentFile->buffer = NULL;
      if (My->CurrentFile->FileName != NULL)
      {
        free (My->CurrentFile->FileName);
        My->CurrentFile->FileName = NULL;
      }
      My->CurrentFile->FileName = e->Buffer;
      e->Buffer = NULL;
    }
  }
  else
  {
    /* NUMBER -- file must already be OPEN */
    /* GET filenumber ... */
    if (e->Number < 0)
    {
      /* "GET # -1" is an error */
      WARN_BAD_FILE_NUMBER;
      return (Line);
    }
    if (e->Number == 0)
    {
      /* "GET # 0" is an error */
      WARN_BAD_FILE_NUMBER;
      return (Line);
    }
    My->CurrentFile = find_file_by_number ((int) bwb_rint (e->Number));
    if (My->CurrentFile == NULL)
    {
      /* file not OPEN */
      WARN_BAD_FILE_NUMBER;
      return (Line);
    }
  }
  if (My->CurrentFile == NULL)
  {
    WARN_BAD_FILE_NUMBER;
    return (Line);
  }
  if ((My->CurrentFile->DevMode & DEVMODE_READ) == 0)
  {
    WARN_BAD_FILE_NUMBER;
    return (Line);
  }
  if (line_skip_seperator (Line))
  {
    /* OK */
  }
  else
  {
    WARN_SYNTAX_ERROR;
    return (Line);
  }
  return file_input (Line);
}

static LineType *
D71_GET (LineType * Line)
{
  /* GET # FileNumber [ , RECORD RecordNumber ] */
  int FileNumber;
   
  assert (Line != NULL);
  assert(My != NULL);

  FileNumber = 0;
  if (line_skip_FilenumChar (Line))
  {
    /* OPTIONAL */
  }
  if (line_read_integer_expression (Line, &FileNumber) == FALSE)
  {
    WARN_BAD_FILE_NUMBER;
    return (Line);
  }
  if (FileNumber < 1)
  {
    WARN_BAD_FILE_NUMBER;
    return (Line);
  }
  My->CurrentFile = find_file_by_number (FileNumber);
  if (My->CurrentFile == NULL)
  {
    WARN_BAD_FILE_NUMBER;
    return (Line);
  }
  if (My->CurrentFile->DevMode != DEVMODE_RANDOM)
  {
    WARN_BAD_FILE_NUMBER;
    return (Line);
  }
  if (My->CurrentFile->width <= 0)
  {
    WARN_BAD_FILE_NUMBER;
    return (Line);
  }
  if (line_is_eol (Line))
  {
    /* GET # FileNumber */
  }
  else
  {
    /* GET # FileNumber , RECORD RecordNumber */
    int RecordNumber;
    long offset;

    RecordNumber = 0;
    offset = 0;
    if (line_skip_seperator (Line) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (Line);
    }
    if (line_skip_word (Line, "RECORD") == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (Line);
    }
    if (line_read_integer_expression (Line, &RecordNumber) == FALSE)
    {
      WARN_BAD_RECORD_NUMBER;
      return (Line);
    }
    if (RecordNumber <= 0)
    {
      WARN_BAD_RECORD_NUMBER;
      return (Line);
    }
    RecordNumber--;                /* BASIC to C */
    offset = RecordNumber;
    offset *= My->CurrentFile->width;
    if (fseek (My->CurrentFile->cfp, offset, SEEK_SET) != 0)
    {
      WARN_BAD_RECORD_NUMBER;
      return (Line);
    }
  }
  /* if( TRUE ) */
  {
    int i;
    for (i = 0; i < My->CurrentFile->width; i++)
    {
      int c;
      c = fgetc (My->CurrentFile->cfp);
      if ( /* EOF */ c < 0)
      {
        c = NulChar;
      }
      My->CurrentFile->buffer[i] = c;
    }
  }
  field_get (My->CurrentFile);
  /* OK */
  return (Line);
}

extern int
binary_get_put (VariableType * Variable, int IsPUT)
{
  VariantType variant;
  VariantType *Variant;
   
  assert(My != NULL);
  assert (My->CurrentFile != NULL);
  assert (My->CurrentFile->cfp != NULL);
  assert (My->CurrentFile->DevMode == DEVMODE_BINARY);

  Variant = &variant;
  CLEAR_VARIANT (Variant);
  if (var_get (Variable, Variant) == FALSE)
  {
    WARN_VARIABLE_NOT_DECLARED;
    return FALSE;
  }
  switch (Variant->VariantTypeCode)
  {
  case ByteTypeCode:
    {
      ByteType Value;
      Value = (ByteType) Variant->Number;
      if (IsPUT)
      {
        if (fwrite (&(Value), sizeof (Value), 1, My->CurrentFile->cfp) != 1)
        {
          WARN_DISK_IO_ERROR;
          return FALSE;
        }
      }
      else
      {
        if (fread (&(Value), sizeof (Value), 1, My->CurrentFile->cfp) != 1)
        {
          WARN_DISK_IO_ERROR;
          return FALSE;
        }
      }
      Variant->Number = Value;
    }
    break;
  case IntegerTypeCode:
    {
      IntegerType Value;
      Value = (IntegerType) Variant->Number;
      if (IsPUT)
      {
        if (fwrite (&(Value), sizeof (Value), 1, My->CurrentFile->cfp) != 1)
        {
          WARN_DISK_IO_ERROR;
          return FALSE;
        }
      }
      else
      {
        if (fread (&(Value), sizeof (Value), 1, My->CurrentFile->cfp) != 1)
        {
          WARN_DISK_IO_ERROR;
          return FALSE;
        }
      }
      Variant->Number = Value;
    }
    break;
  case LongTypeCode:
    {
      LongType Value;
      Value = (LongType) Variant->Number;
      if (IsPUT)
      {
        if (fwrite (&(Value), sizeof (Value), 1, My->CurrentFile->cfp) != 1)
        {
          WARN_DISK_IO_ERROR;
          return FALSE;
        }
      }
      else
      {
        if (fread (&(Value), sizeof (Value), 1, My->CurrentFile->cfp) != 1)
        {
          WARN_DISK_IO_ERROR;
          return FALSE;
        }
      }
      Variant->Number = Value;
    }
    break;
  case CurrencyTypeCode:
    {
      CurrencyType Value;
      Value = (CurrencyType) Variant->Number;
      if (IsPUT)
      {
        if (fwrite (&(Value), sizeof (Value), 1, My->CurrentFile->cfp) != 1)
        {
          WARN_DISK_IO_ERROR;
          return FALSE;
        }
      }
      else
      {
        if (fread (&(Value), sizeof (Value), 1, My->CurrentFile->cfp) != 1)
        {
          WARN_DISK_IO_ERROR;
          return FALSE;
        }
      }
      Variant->Number = Value;
    }
    break;
  case SingleTypeCode:
    {
      SingleType Value;
      Value = (SingleType) Variant->Number;
      if (IsPUT)
      {
        if (fwrite (&(Value), sizeof (Value), 1, My->CurrentFile->cfp) != 1)
        {
          WARN_DISK_IO_ERROR;
          return FALSE;
        }
      }
      else
      {
        if (fread (&(Value), sizeof (Value), 1, My->CurrentFile->cfp) != 1)
        {
          WARN_DISK_IO_ERROR;
          return FALSE;
        }
      }
      Variant->Number = Value;
    }
    break;
  case DoubleTypeCode:
    {
      DoubleType Value;
      Value = (DoubleType) Variant->Number;
      if (IsPUT)
      {
        if (fwrite (&(Value), sizeof (Value), 1, My->CurrentFile->cfp) != 1)
        {
          WARN_DISK_IO_ERROR;
          return FALSE;
        }
      }
      else
      {
        if (fread (&(Value), sizeof (Value), 1, My->CurrentFile->cfp) != 1)
        {
          WARN_DISK_IO_ERROR;
          return FALSE;
        }
      }
      Variant->Number = Value;
    }
    break;
  case StringTypeCode:
    if (IsPUT)
    {
#if FALSE /* keep this ... */
      if (fwrite
          (&(Variant->Length), sizeof (Variant->Length), 1,
           My->CurrentFile->cfp) != 1)
      {
        WARN_DISK_IO_ERROR;
        return FALSE;
      }
#endif
      if (fwrite (Variant->Buffer, Variant->Length, 1, My->CurrentFile->cfp)
          != 1)
      {
        WARN_DISK_IO_ERROR;
        return FALSE;
      }
    }
    else
    {
#if FALSE /* keep this ... */
      if (fread
          (&(Variant->Length), sizeof (Variant->Length), 1,
           My->CurrentFile->cfp) != 1)
      {
        WARN_DISK_IO_ERROR;
        return FALSE;
      }
#endif
      if (fread (Variant->Buffer, Variant->Length, 1, My->CurrentFile->cfp) !=
          1)
      {
        WARN_DISK_IO_ERROR;
        return FALSE;
      }
      Variant->Buffer[Variant->Length] = NulChar;
    }
    break;
  default:
    {
      WARN_INTERNAL_ERROR;
      return FALSE;
    }
  }
  if (IsPUT)
  {
    /* not needed */
  }
  else
  {
    if (var_set (Variable, Variant) == FALSE)
    {
      WARN_VARIABLE_NOT_DECLARED;
      return FALSE;
    }
  }
  RELEASE_VARIANT (Variant);
  /* OK */
  return TRUE;
}

static LineType *
H14_GET (LineType * Line)
{
  /* GET # FileNumber [ , RecordNumber ]                   ' RANDOM */
  /* GET # FileNumber   , [ BytePosition ] , scalar [,...] ' BINARY */
  int FileNumber;
   
  assert (Line != NULL);
  assert(My != NULL);

  FileNumber = 0;
  if (line_skip_FilenumChar (Line))
  {
    /* OPTIONAL */
  }
  if (line_read_integer_expression (Line, &FileNumber) == FALSE)
  {
    WARN_BAD_FILE_NUMBER;
    return (Line);
  }
  if (FileNumber < 1)
  {
    WARN_BAD_FILE_NUMBER;
    return (Line);
  }
  My->CurrentFile = find_file_by_number (FileNumber);
  if (My->CurrentFile == NULL)
  {
    WARN_BAD_FILE_NUMBER;
    return (Line);
  }
  if (My->CurrentFile->DevMode == DEVMODE_RANDOM)
  {
    /* GET # FileNumber [ , RecordNumber ]                   ' RANDOM */
    if (My->CurrentFile->width <= 0)
    {
      WARN_BAD_FILE_NUMBER;
      return (Line);
    }
    if (line_is_eol (Line))
    {
      /* GET # FileNumber */
    }
    else
    {
      /* GET # FileNumber , RecordNumber */
      int RecordNumber;
      long offset;

      RecordNumber = 0;
      offset = 0;
      if (line_skip_seperator (Line) == FALSE)
      {
        WARN_SYNTAX_ERROR;
        return (Line);
      }
      if (line_read_integer_expression (Line, &RecordNumber) == FALSE)
      {
        WARN_BAD_RECORD_NUMBER;
        return (Line);
      }
      if (RecordNumber <= 0)
      {
        WARN_BAD_RECORD_NUMBER;
        return (Line);
      }
      RecordNumber--;                /* BASIC to C */
      offset = RecordNumber;
      offset *= My->CurrentFile->width;
      if (fseek (My->CurrentFile->cfp, offset, SEEK_SET) != 0)
      {
        WARN_BAD_RECORD_NUMBER;
        return (Line);
      }
    }
    /* if( TRUE ) */
    {
      int i;
      for (i = 0; i < My->CurrentFile->width; i++)
      {
        int c;
        c = fgetc (My->CurrentFile->cfp);
        if ( /* EOF */ c < 0)
        {
          c = NulChar;
        }
        My->CurrentFile->buffer[i] = c;
      }
    }
    field_get (My->CurrentFile);
    /* OK */
    return (Line);
  }
  else if (My->CurrentFile->DevMode == DEVMODE_BINARY)
  {
    /* GET # FileNumber   , [ BytePosition ] , scalar [,...] ' BINARY */
    if (line_skip_seperator (Line) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (Line);
    }
    if (line_skip_seperator (Line))
    {
      /* BytePosition not provided */
    }
    else
    {
      int RecordNumber;
      long offset;

      RecordNumber = 0;
      offset = 0;
      if (line_read_integer_expression (Line, &RecordNumber) == FALSE)
      {
        WARN_BAD_RECORD_NUMBER;
        return (Line);
      }
      if (RecordNumber <= 0)
      {
        WARN_BAD_RECORD_NUMBER;
        return (Line);
      }
      RecordNumber--;                /* BASIC to C */
      offset = RecordNumber;
      if (fseek (My->CurrentFile->cfp, offset, SEEK_SET) != 0)
      {
        WARN_BAD_RECORD_NUMBER;
        return (Line);
      }
      if (line_skip_seperator (Line) == FALSE)
      {
        WARN_SYNTAX_ERROR;
        return (Line);
      }
    }
    do
    {
      VariableType *v;

      if ((v = line_read_scalar (Line)) == NULL)
      {
        WARN_SYNTAX_ERROR;
        return (Line);
      }
      if (binary_get_put (v, FALSE) == FALSE)
      {
        WARN_SYNTAX_ERROR;
        return (Line);
      }
    }
    while (line_skip_seperator (Line));
    /* OK */
    return (Line);
  }
  WARN_BAD_FILE_MODE;
  return (Line);
}

extern LineType *
bwb_GET (LineType * Line)
{
   
  assert (Line != NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);

  if (My->CurrentVersion->OptionVersionValue & (S70 | I70 | I73))
  {
    return S70_GET (Line);
  }
  if (My->CurrentVersion->OptionVersionValue & (D71 | T79 | R86))
  {
    return D71_GET (Line);
  }
  if (My->CurrentVersion->OptionVersionValue & (H14))
  {
    return H14_GET (Line);
  }
  WARN_INTERNAL_ERROR;
  return (Line);
}

static ResultType
file_data (VariableType * Variable, char *tbuf, int tlen)
{
  ResultType Result;
  VariantType Variant;
  VariantType *X;
  int p;
   
  assert (Variable != NULL);
  assert (tbuf != NULL);
  assert (tlen > 0);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);
  assert (My->CurrentFile != NULL);
  assert ((My->CurrentFile->DevMode & DEVMODE_READ) != 0);


  Result = RESULT_UNPARSED;
  X = &Variant;
  p = 0;
  CLEAR_VARIANT (X);
  if (tbuf[0] == NulChar)
  {
    /* Get more data */
    if (fgets (tbuf, tlen, My->CurrentFile->cfp))        /* file_data */
    {
      tbuf[tlen] = NulChar;
      CleanTextInput (tbuf);
    }
    else
    {
      return RESULT_UNPARSED;        /* causes file_if_end() */
    }
  }
  if (VAR_IS_STRING (Variable))
  {
    Result = parse_string (tbuf, &p, X);
  }
  else
  {
    Result = parse_number (tbuf, &p, X, FALSE);
  }
  if (Result == RESULT_UNPARSED)
  {
    WARN_BAD_DATA;
  }
  if (Result != RESULT_SUCCESS)
  {
    return Result;
  }
  /*
   **
   ** OK
   **
   */
  if (X->VariantTypeCode == StringTypeCode
      && My->CurrentVersion->
      OptionFlags & OPTION_BUGS_ON /* DATA allows embedded quote pairs */ )
  {
    int i;
    int n;
    n = X->Length;
    for (i = 0; i < n; i++)
    {
      if (X->Buffer[i + 0] == My->CurrentVersion->OptionQuoteChar
          && X->Buffer[i + 1] == My->CurrentVersion->OptionQuoteChar)
      {
        bwb_strncpy (&X->Buffer[i + 0], &X->Buffer[i + 1], n - i);
        n--;
      }
    }
    X->Length = n;
  }
  if (var_set (Variable, X) == FALSE)
  {
    WARN_VARIABLE_NOT_DECLARED;
    return RESULT_UNPARSED;
  }
  /*
   **
   ** Note: do NOT free() or RELEASE_VARIANT because 'X->Buffer' points into 'buffer'
   **
   */
  if (buff_is_eol (tbuf, &p))
  {
    tbuf[0] = NulChar;
    return RESULT_SUCCESS;
  }
  if (buff_skip_char (tbuf, &p, My->CurrentFile->delimit))        /* buff_skip_comma */
  {
    /* shift left past comma */
    bwb_strcpy (tbuf, &tbuf[p]);
    return RESULT_SUCCESS;
  }
  /* garbage after the value we just READ */
  WARN_BAD_DATA;
  return RESULT_UNPARSED;
}

static LineType *
C77_file_input_line (LineType * Line)
{
  /* 
     CBASIC-II: READ # filenumber [, recnum ] ; LINE variable$
   */
  /* a flavor of LINE INPUT */
  VariableType *v;
  assert (Line != NULL);
  assert(My != NULL);
  assert (My->CurrentFile != NULL);
  assert ((My->CurrentFile->DevMode & DEVMODE_READ) != 0);

  if ((v = line_read_scalar (Line)) == NULL)
  {
    WARN_SYNTAX_ERROR;
    return (Line);
  }
  if (VAR_IS_STRING (v))
  {

    char *tbuf;
    int tlen;

    assert (My->ConsoleInput != NULL); 
    tbuf = My->ConsoleInput;
    tlen = MAX_LINE_LENGTH;
    /* CBASIC-II: RANDOM files are padded on the right with spaces with a '\n' in the last position */
    if (My->CurrentFile->width > MAX_LINE_LENGTH)
    {
      if (My->CurrentFile->buffer != NULL)
      {
        /* use the bigger buffer */
        tbuf = My->CurrentFile->buffer;
        tlen = My->CurrentFile->width;
      }
    }
    if (fgets (tbuf, tlen, My->CurrentFile->cfp))        /* C77_file_input_line */
    {
      tbuf[tlen] = NulChar;
      CleanTextInput (tbuf);
    }
    else
    {
      return file_if_end (Line);
    }
    /* if( TRUE ) */
    {
      VariantType variant;

      variant.VariantTypeCode = StringTypeCode;
      variant.Buffer = tbuf;
      variant.Length = bwb_strlen (variant.Buffer);
      if (var_set (v, &variant) == FALSE)
      {
        WARN_VARIABLE_NOT_DECLARED;
        return (Line);
      }
    }
    return (Line);
  }
  WARN_TYPE_MISMATCH;
  return (Line);
}

static LineType *
C77_file_input_finish (LineType * Line)
{
  /* 
     CBASIC-II: RANDOM file reads always acccess a complete record
   */
  long ByteOffset;
  assert (Line != NULL);
  assert(My != NULL);
  assert (My->CurrentFile != NULL);
  assert ((My->CurrentFile->DevMode & DEVMODE_READ) != 0);

  /* advance to the end-of-record */
  if (My->CurrentFile->width <= 0)
  {
    WARN_FIELD_OVERFLOW;
    return (Line);
  }
  ByteOffset = ftell (My->CurrentFile->cfp);
  ByteOffset %= My->CurrentFile->width;
  if (ByteOffset != 0)
  {
    long RecordNumber;
    RecordNumber = ftell (My->CurrentFile->cfp);
    RecordNumber /= My->CurrentFile->width;
    RecordNumber++;
    RecordNumber *= My->CurrentFile->width;
    fseek (My->CurrentFile->cfp, RecordNumber, SEEK_SET);
  }
  return (Line);
}


static LineType *
file_if_end (LineType * Line)
{
  /* IF END # FileNumber THEN LineNumber */
  assert (Line != NULL);
  assert(My != NULL);
  assert (My->CurrentFile != NULL);
  assert ((My->CurrentFile->DevMode & DEVMODE_READ) != 0);

  if (My->CurrentFile->EOF_LineNumber > 0)
  {
    LineType *x;

    x = find_line_number (My->CurrentFile->EOF_LineNumber);        /* not found in the cache */
    if (x != NULL)
    {
      /* FOUND */
      line_skip_eol (Line);
      x->position = 0;
      return x;
    }
    /* NOT FOUND */
    WARN_UNDEFINED_LINE;
    return (Line);
  }
  WARN_INPUT_PAST_END;
  return (Line);
}

static LineType *
file_input (LineType * Line)
{
  /* INPUT # is similar to READ, where each file line is a DATA line */
  char *tbuf;
  int tlen;

   
  assert (Line != NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);
  assert (My->CurrentFile != NULL);
  assert ((My->CurrentFile->DevMode & DEVMODE_READ) != 0);

  tbuf = My->ConsoleInput;
  tlen = MAX_LINE_LENGTH;
  if (My->CurrentVersion->OptionVersionValue & (C77))
  {
    if (line_skip_word (Line, "LINE"))
    {
      return C77_file_input_line (Line);
    }
  }

  if (My->CurrentFile->width > 0 && My->CurrentFile->buffer != NULL)
  {
    tlen = My->CurrentFile->width;
    tbuf = My->CurrentFile->buffer;
  }
  tbuf[0] = NulChar;

  /* Process each variable read from the INPUT # statement */
  do
  {
    VariableType *v;

    /* Read a variable name */
    if ((v = line_read_scalar (Line)) == NULL)
    {
      WARN_SYNTAX_ERROR;
      return (Line);
    }

    /* Read a file value */
    if (file_data (v, tbuf, tlen) != RESULT_SUCCESS)
    {
      return file_if_end (Line);
    }
    /* OK */
  }
  while (line_skip_seperator (Line));

  if (My->CurrentVersion->OptionVersionValue & (C77)
      && My->CurrentFile->DevMode & DEVMODE_RANDOM)
  {
    return C77_file_input_finish (Line);
  }
  return (Line);
}


/***************************************************************
  
        FUNCTION:       user_input_*()
  
        DESCRIPTION:    This function does INPUT processing
                        from a determined string of input
                        data and a determined variable list
                        (both in memory).  This presupposes
         that input has been taken from My->SYSIN,
         not from a disk file or device.
  
***************************************************************/
static ResultType
parse_string_isquoted (char *buffer, int *position, VariantType * X)
{
  /*
   **
   ** QUOTED STRING 
   **
   ** Note: do NOT free() or RELEASE_VARIANT because 'X->Buffer' points into 'buffer'
   **
   */
  int p;
  assert (buffer != NULL);
  assert (position != NULL);
  assert (X != NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);
  assert (My->CurrentFile != NULL);
  assert ((My->CurrentFile->DevMode & DEVMODE_READ) != 0);

  p = *position;

  buff_skip_spaces (buffer, &p);        /* keep this */
  if (buffer[p] == My->CurrentVersion->OptionQuoteChar)
  {
    int Length;
    int Start;
    int QuoteCount;

    Length = 0;
    QuoteCount = 0;

    QuoteCount++;
    p++;
    Start = p;
    while (buffer[p])
    {
      if (buffer[p] == My->CurrentVersion->OptionQuoteChar
          && buffer[p + 1] == My->CurrentVersion->OptionQuoteChar
          && My->CurrentVersion->
          OptionFlags & OPTION_BUGS_ON /* INPUT allows embedded quote pairs */
          )
      {
        /* embedded quote pair "...""..." */
        QuoteCount++;
        QuoteCount++;
        p++;
        p++;
        Length++;
        Length++;
      }
      else if (buffer[p] == My->CurrentVersion->OptionQuoteChar)
      {
        /* properly terminated string "...xx..." */
        QuoteCount++;
        p++;
        break;
      }
      else
      {
        /* normal character */
        p++;
        Length++;
      }
    }
    if (My->CurrentVersion->
        OptionFlags & OPTION_BUGS_ON /* INPUT allows unmatched quotes pairs */
        )
    {
      /* silently ignore */
    }
    else if (QuoteCount & 1)
    {
      /* an ODD number of quotes (including embedded quotes) is an ERROR */
      return RESULT_UNPARSED;
    }
    /*
     **
     ** OK
     **
     */
    X->VariantTypeCode = StringTypeCode;
    X->Buffer = &buffer[Start];
    X->Length = Length;
    *position = p;
    return RESULT_SUCCESS;
  }
  return RESULT_UNPARSED;
}
static ResultType
parse_string_unquoted (char *buffer, int *position, VariantType * X)
{
  /*
   **
   ** UNQUOTED STRING 
   **
   ** Note: do NOT free() or RELEASE_VARIANT because 'X->Buffer' points into 'buffer'
   **
   */
  int p;
  int Length;
  int Start;
  assert (buffer != NULL);
  assert (position != NULL);
  assert (X != NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);
  assert (My->CurrentFile != NULL);
  assert ((My->CurrentFile->DevMode & DEVMODE_READ) != 0);

  Length = 0;
  p = *position;
  buff_skip_spaces (buffer, &p);        /* keep this */
  Start = p;
  while (buffer[p] != NulChar && buffer[p] != My->CurrentFile->delimit)
  {
    char C;
    C = buffer[p];

    if (My->CurrentVersion->
        OptionFlags & OPTION_BUGS_ON /* INPUT allows unquoted strings */ )
    {
      /* silently ignore */
    }
    else if (C == ' ' || C == '+' || C == '-' || C == '.' || bwb_isalnum (C))
    {
      /* if was NOT quoted, then the only valid chars are ' ', '+', '-', '.', digit, letter */
    }
    else
    {
      /* ERROR */
      return RESULT_UNPARSED;
    }
    Length++;
    p++;
  }
  /* RTRIM */
  while (Length > 0 && buffer[Start + Length - 1] == ' ')
  {
    Length--;
  }
  /*
   **
   ** OK
   **
   */
  X->VariantTypeCode = StringTypeCode;
  X->Buffer = &buffer[Start];
  X->Length = Length;
  *position = p;
  return RESULT_SUCCESS;
}

static ResultType
parse_string (char *buffer, int *position, VariantType * X)
{
  /*
   **
   ** STRING 
   **
   ** Note: do NOT free() or RELEASE_VARIANT because 'X->Buffer' points into 'buffer'
   **
   */
  ResultType Result;
  int p;
  assert (buffer != NULL);
  assert (position != NULL);
  assert (X != NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);
  assert (My->CurrentFile != NULL);
  assert ((My->CurrentFile->DevMode & DEVMODE_READ) != 0);

  p = *position;
  buff_skip_spaces (buffer, &p);        /* keep this */
  if (buff_is_eol (buffer, &p)
      || buff_peek_char (buffer, &p, My->CurrentFile->delimit))
  {
    /* process EMPTY response */
    if (My->CurrentVersion->
        OptionFlags & OPTION_BUGS_ON /* INPUT allows empty values */ )
    {
      /* silently ignore, value is "" */
      X->VariantTypeCode = StringTypeCode;
      X->Buffer = &buffer[p];
      X->Length = 0;
      Result = RESULT_SUCCESS;
    }
    else
    {
      return RESULT_UNPARSED;
    }
  }
  Result = parse_string_isquoted (buffer, &p, X);
  if (Result == RESULT_UNPARSED)
  {
    Result = parse_string_unquoted (buffer, &p, X);
  }
  if (Result == RESULT_SUCCESS)
  {
    *position = p;
  }
  return Result;
}

static ResultType
parse_number (char *buffer, int *position, VariantType * X,
              int IsConsoleInput)
{
  ResultType Result = RESULT_UNPARSED;
  int p;
  assert (buffer != NULL);
  assert (position != NULL);
  assert (X != NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);
  assert (My->CurrentFile != NULL);
  assert ((My->CurrentFile->DevMode & DEVMODE_READ) != 0);

  p = *position;
  buff_skip_spaces (buffer, &p);        /* keep this */
  if (buff_is_eol (buffer, &p)
      || buff_peek_char (buffer, &p, My->CurrentFile->delimit))
  {
    /* process EMPTY response */
    if (My->CurrentVersion->
        OptionFlags & OPTION_BUGS_ON /* INPUT allows empty values */ )
    {
      /* silently ignore, value is 0 */
      X->VariantTypeCode = DoubleTypeCode;
      X->Number = 0;
      return RESULT_SUCCESS;
    }
    else
    {
      return RESULT_UNPARSED;
    }
  }
  Result = buff_read_hexadecimal_constant (buffer, &p, X, IsConsoleInput);
  if (Result == RESULT_UNPARSED)
  {
    Result = buff_read_octal_constant (buffer, &p, X, IsConsoleInput);
  }
  if (Result == RESULT_UNPARSED)
  {
    int IsNegative;

    IsNegative = FALSE;
    if (buff_skip_PlusChar (buffer, &p))
    {
      /* ignore */
    }
    else if (buff_skip_MinusChar (buffer, &p))
    {
      IsNegative = TRUE;
    }
    Result = buff_read_decimal_constant (buffer, &p, X, IsConsoleInput);
    if (Result == RESULT_SUCCESS)
    {
      if (IsNegative)
      {
        X->Number = -X->Number;
      }
    }
  }
  if (Result == RESULT_SUCCESS)
  {
    *position = p;
  }
  return Result;
}

static ResultType
user_input_values (LineType * Line, char *buffer, int IsReal)
{
  /*
   **
   ** given a response, match with the list of variables
   **
   */
  int p;
   
  assert (Line != NULL);
  assert (buffer != NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);
  assert (My->CurrentFile == My->SYSIN);

  p = 0;
  /* Read elements in buffer and assign them to variables in Line */
  do
  {
    ResultType Result;
    VariableType *Variable;
    VariantType Variant;
    VariantType *X;

    X = &Variant;
    CLEAR_VARIANT (X);

    /* get a variable name from the list */
    if ((Variable = line_read_scalar (Line)) == NULL)
    {
      WARN_SYNTAX_ERROR;
      return RESULT_UNPARSED;
    }

    /* get a value from the console response */
    Result = RESULT_UNPARSED;
    if (VAR_IS_STRING (Variable))
    {
      Result = parse_string (buffer, &p, X);
    }
    else
    {
      Result = parse_number (buffer, &p, X, TRUE);
    }
    if (Result != RESULT_SUCCESS)
    {
      return Result;
    }
    /*
     **
     ** OK
     **
     */
    if (IsReal)
    {
      /*
       **
       ** actually assign the value
       **
       */
      if (X->VariantTypeCode == StringTypeCode
          && My->CurrentVersion->
          OptionFlags & OPTION_BUGS_ON /* INPUT allows embedded quote pairs */
          )
      {
        int i;
        int n;
        n = X->Length;
        for (i = 0; i < n; i++)
        {
          if (X->Buffer[i + 0] == My->CurrentVersion->OptionQuoteChar
              && X->Buffer[i + 1] == My->CurrentVersion->OptionQuoteChar)
          {
            bwb_strncpy (&X->Buffer[i + 0], &X->Buffer[i + 1], n - i);
            n--;
          }
        }
        X->Length = n;
      }
      if (var_set (Variable, X) == FALSE)
      {
        WARN_VARIABLE_NOT_DECLARED;
        return RESULT_UNPARSED;
      }
    }
    /*
     **
     ** STRING 
     **
     ** Note: do NOT free() or RELEASE_VARIANT because 'X->Buffer' points into 'buffer'
     **
     */
  }
  while (line_skip_seperator (Line)
         && buff_skip_char (buffer, &p, My->CurrentFile->delimit));

  /* verify all variables and values consumed */
  if (line_is_eol (Line) && buff_is_eol (buffer, &p))
  {
    /*
     **
     ** OK
     **
     */
    return RESULT_SUCCESS;
  }
  /* Count mismatch */
  return RESULT_UNPARSED;
}

static LineType *
C77_user_input_line (LineType * Line, char *Prompt, int IsDisplayQuestionMark)
{
  /* 
   **
   ** CBASIC-II: INPUT "prompt" ; LINE variable$
   **
   */
  VariableType *v;
  assert (Line != NULL);
  assert(My != NULL);
  assert (My->CurrentFile != NULL);
  assert ((My->CurrentFile->DevMode & DEVMODE_READ) != 0);

  if ((v = line_read_scalar (Line)) == NULL)
  {
    WARN_SYNTAX_ERROR;
    return (Line);
  }
  if (v->VariableFlags & (VARIABLE_CONSTANT))
  {
    WARN_VARIABLE_NOT_DECLARED;
    return (Line);
  }
  if (VAR_IS_STRING (v))
  {
    VariantType variant;
    char *tbuf;
    int tlen;

    tbuf = My->ConsoleInput;
    tlen = MAX_LINE_LENGTH;
    bwx_input (Prompt, IsDisplayQuestionMark, tbuf, tlen);
    variant.VariantTypeCode = StringTypeCode;
    variant.Buffer = tbuf;
    variant.Length = bwb_strlen (variant.Buffer);
    if (var_set (v, &variant) == FALSE)
    {
      WARN_VARIABLE_NOT_DECLARED;
      return (Line);
    }
    /* OK */
    if (Prompt != NULL)
    {
      free (Prompt);
      /* Prompt = NULL; */
    }
    return (Line);
  }
  WARN_TYPE_MISMATCH;
  return (Line);
}

static LineType *
user_input_loop (LineType * Line)
{
  char *Prompt;
  int IsDisplayQuestionMark;
  int SavePosition;
  assert (Line != NULL);
  assert(My != NULL);
  assert(My->SYSIN != NULL);
  assert(My->SYSIN->cfp != NULL);

  Prompt = NULL;
  IsDisplayQuestionMark = TRUE;
  My->CurrentFile = My->SYSIN;
  /*
   **
   ** Step 1. Determine the prompt
   ** Step 2. Verify all variables exist and are not CONST
   ** Step 3. Display prompt and get user response
   ** Step 4. Assign user response to variables
   **
   */

  /* 
   **
   ** Step 1. Determine the prompt
   ** 
   */
  /* INPUT , "prompt" A, B, C */
  /* INPUT ; "prompt" A, B ,C */
  /* INPUT : "prompt" A, B, C */
  if (line_skip_seperator (Line))
  {
    /* optional */
    IsDisplayQuestionMark = FALSE;
  }

  if (line_peek_QuoteChar (Line))
  {
    /* get prompt string */
    if (line_read_string_expression (Line, &Prompt) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (Line);
    }
    if (line_skip_seperator (Line) == ',' /* comma-specific */ )
    {
      /* optional */
      IsDisplayQuestionMark = FALSE;
    }
  }

  if (My->CurrentVersion->OptionVersionValue & (C77)
      && line_skip_word (Line, "LINE"))
  {
    /* INPUT "prompt" ; LINE variable$ */
    return C77_user_input_line (Line, Prompt, IsDisplayQuestionMark);
  }
  /* 
   **
   ** Step 2. Verify all variables exist and are not CONST
   ** 
   */
  SavePosition = Line->position;
  do
  {
    VariableType *v;

    if ((v = line_read_scalar (Line)) == NULL)
    {
      WARN_SYNTAX_ERROR;
      return (Line);
    }
    if (v->VariableFlags & (VARIABLE_CONSTANT))
    {
      WARN_VARIABLE_NOT_DECLARED;
      return (Line);
    }
  }
  while (line_skip_seperator (Line));
  if (line_is_eol (Line))
  {
    /* OK */
  }
  else
  {
    WARN_SYNTAX_ERROR;
    return (Line);
  }
  while (TRUE)
  {
    char *tbuf;
    int tlen;
    ResultType Result;

    tbuf = My->ConsoleInput;
    tlen = MAX_LINE_LENGTH;
    /* 
     **
     ** Step 3. Display prompt and get user response
     ** 
     */
    bwx_input (Prompt, IsDisplayQuestionMark, tbuf, tlen);
    /* 
     **
     ** Step 4. Assign user response to variables
     ** 
     */
    Line->position = SavePosition;
    Result = user_input_values (Line, tbuf, FALSE /* FAKE run */ );        /* bwb_INPUT, user_input_loop */
    if (Result == RESULT_SUCCESS)        /* bwb_INPUT */
    {
      /* successful input, FAKE run  */
      Line->position = SavePosition;
      Result = user_input_values (Line, tbuf, TRUE /* REAL run */ );        /* bwb_INPUT, user_input_loop */
      if (Result == RESULT_SUCCESS)
      {
        /* successful input, REAL run  */
        if (Prompt != NULL)
        {
          free (Prompt);
          Prompt = NULL;
        }
        return (Line);
      }
    }
    /* Result == RESULT_UNPARSED, RETRY */
    fputs ("?Redo from start\n", My->SYSOUT->cfp);        /* "*** Retry INPUT ***\n" */
    ResetConsoleColumn ();
  }
  /* never reached */
  return (Line);
}

extern LineType *
bwb_INPUT (LineType * Line)
{
   
  assert (Line != NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);
  assert(My->SYSIN != NULL);
  assert(My->SYSIN->cfp != NULL);

  My->CurrentFile = My->SYSIN;
  if (line_skip_FilenumChar (Line))
  {
    /* INPUT # X */
    int FileNumber;

    if (line_read_integer_expression (Line, &FileNumber) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (Line);
    }
    if (line_skip_seperator (Line))
    {
      /* required */
    }
    else
    {
      WARN_SYNTAX_ERROR;
      return (Line);
    }
    /* INPUT # X , */
    if (FileNumber < 0)
    {
      /* "INPUT # -1" is an error */
      WARN_BAD_FILE_NUMBER;
      return (Line);
    }
    if (FileNumber > 0)
    {
      /* normal file */
      My->CurrentFile = find_file_by_number (FileNumber);
      if (My->CurrentFile == NULL)
      {
        WARN_BAD_FILE_NUMBER;
        return (Line);
      }
      if ((My->CurrentFile->DevMode & DEVMODE_READ) == 0)
      {
        WARN_BAD_FILE_NUMBER;
        return (Line);
      }
      return file_input (Line);
    }
    /* "INPUT #0, varlist" is the same as "INPUT varlist"  */
  }
  /* input is from My->SYSIN */
  return user_input_loop (Line);
}



/***************************************************************
  
        FUNCTION:       bwb_LINE()
  
        DESCRIPTION:    This function implements the BASIC LINE
                        INPUT statement.
  
   SYNTAX:         LINE INPUT [[#] device-number,]["prompt string";] string-variable$
  
***************************************************************/
extern LineType *
bwb_LINE (LineType * Line)
{
   
  assert (Line != NULL);

  WARN_SYNTAX_ERROR;
  return (Line);
}

extern LineType *
bwb_INPUT_LINE (LineType * Line)
{
   
  assert (Line != NULL);

  return bwb_LINE_INPUT (Line);
}

extern LineType *
bwb_LINE_INPUT (LineType * Line)
{
  int FileNumber;
  VariableType *v;
  char *tbuf;
  int tlen;
  char *Prompt;
   
  assert (Line != NULL);
  assert(My != NULL);
  assert(My->SYSIN != NULL);
  assert(My->SYSIN->cfp != NULL);
  assert(My->ConsoleInput != NULL);
  assert(MAX_LINE_LENGTH > 1);

  /* assign default values */

  tbuf = My->ConsoleInput;
  tlen = MAX_LINE_LENGTH;
  Prompt = NULL;
  My->CurrentFile = My->SYSIN;

  /* check for leading semicolon */
  if (line_skip_seperator (Line))
  {
    /* optional */
  }
  if (line_skip_FilenumChar (Line))
  {
    if (line_read_integer_expression (Line, &FileNumber) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (Line);
    }
    if (FileNumber < 0)
    {
      /* "LINE INPUT # -1" is an error */
      WARN_BAD_FILE_NUMBER;
      return (Line);
    }
    if (FileNumber > 0)
    {
      /* normal file */
      My->CurrentFile = find_file_by_number (FileNumber);
      if (My->CurrentFile == NULL)
      {
        WARN_BAD_FILE_NUMBER;
        return (Line);
      }
      if ((My->CurrentFile->DevMode & DEVMODE_READ) == 0)
      {
        WARN_BAD_FILE_NUMBER;
        return (Line);
      }
      if (My->CurrentFile->cfp == NULL)
      {
        WARN_BAD_FILE_NUMBER;
        return (Line);
      }
    }
    /* check for comma */
    if (line_skip_seperator (Line))
    {
      /* optional */
    }
  }

  /* check for quotation mark indicating prompt */
  if (line_peek_QuoteChar (Line))
  {
    /* get prompt string */
    if (line_read_string_expression (Line, &Prompt) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (Line);
    }
    /* check for comma */
    if (line_skip_seperator (Line))
    {
      /* optional */
    }
  }

  /* read the variable for assignment */
  if ((v = line_read_scalar (Line)) == NULL)
  {
    WARN_SYNTAX_ERROR;
    return (Line);
  }
  if (VAR_IS_STRING (v))
  {
    /* OK */
  }
  else
  {
    /* ERROR */
    WARN_TYPE_MISMATCH;
    return (Line);
  }

  /* read a line of text into the bufffer */
  if (My->CurrentFile == My->SYSIN)
  {
    /* LINE INPUT never displays a '?' regardless of the ',' or ';' */
    bwx_input (Prompt, FALSE, tbuf, tlen);
  }
  else
  {
    if (fgets (tbuf, tlen, My->CurrentFile->cfp))        /* bwb_LINE_INPUT */
    {
      tbuf[tlen] = NulChar;
      CleanTextInput (tbuf);
    }
    else
    {
      return file_if_end (Line);
    }
  }
  /* if( TRUE ) */
  {
    VariantType variant;

    variant.VariantTypeCode = StringTypeCode;
    variant.Buffer = tbuf;
    variant.Length = bwb_strlen (variant.Buffer);
    if (var_set (v, &variant) == FALSE)
    {
      WARN_VARIABLE_NOT_DECLARED;
      return (Line);
    }
  }
  if (Prompt != NULL)
  {
    free (Prompt);
    Prompt = NULL;
  }
  return (Line);
}

static LineType *
file_read_matrix (LineType * Line)
{
  /* MAT GET filename$ , matrix [, ...] */
  /* MAT READ arrayname [;|,] */
  /* Array must be 1, 2 or 3 dimensions */
  /* Array may be either NUMBER or STRING */
  VariableType *v;
   
  assert (Line != NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);
  assert(My->SYSIN != NULL);
  assert(My->SYSIN->cfp != NULL);
  assert(My->ConsoleInput != NULL);
  assert(MAX_LINE_LENGTH > 1);
  assert(My->CurrentFile != NULL);

  My->LastInputCount = 0;
  do
  {
    char *tbuf;
    int tlen;

    tbuf = My->ConsoleInput;
    tlen = MAX_LINE_LENGTH;
    if (My->CurrentFile->width > 0 && My->CurrentFile->buffer != NULL)
    {
      tlen = My->CurrentFile->width;
      tbuf = My->CurrentFile->buffer;
    }
    tbuf[0] = NulChar;

    My->LastInputCount = 0;
    if ((v = line_read_matrix (Line)) == NULL)
    {
      WARN_SUBSCRIPT_OUT_OF_RANGE;
      return (Line);
    }
    /* variable MUST be an array of 1, 2 or 3 dimensions */
    if (v->dimensions < 1)
    {
      WARN_SUBSCRIPT_OUT_OF_RANGE;
      return (Line);
    }
    if (v->dimensions > 3)
    {
      WARN_SUBSCRIPT_OUT_OF_RANGE;
      return (Line);
    }

    /* READ array */
    switch (v->dimensions)
    {
    case 1:
      {
        /*
           OPTION BASE 0
           DIM A(5)
           ...
           MAT READ A 
           ...
           FOR I = 0 TO 5
           READ A(I)
           NEXT I
           ...
         */
        for (v->VINDEX[0] = v->LBOUND[0]; v->VINDEX[0] <= v->UBOUND[0];
             v->VINDEX[0]++)
        {
          if (My->CurrentFile == My->SYSIN)
          {
            if (read_data (v) != RESULT_SUCCESS)
            {
              return data_if_end (Line);
            }
          }
          else
          {
            if (file_data (v, tbuf, tlen) != RESULT_SUCCESS)
            {
              return file_if_end (Line);
            }
          }
          /* OK */
          My->LastInputCount++;
        }
      }
      break;
    case 2:
      {
        /*
           OPTION BASE 0
           DIM B(2,3)
           ...
           MAT READ B 
           ...
           FOR I = 0 TO 2
           FOR J = 0 TO 3
           READ B(I,J)
           NEXT J
           PRINT
           NEXT I
           ...
         */
        for (v->VINDEX[0] = v->LBOUND[0]; v->VINDEX[0] <= v->UBOUND[0];
             v->VINDEX[0]++)
        {
          for (v->VINDEX[1] = v->LBOUND[1]; v->VINDEX[1] <= v->UBOUND[1];
               v->VINDEX[1]++)
          {
            if (My->CurrentFile == My->SYSIN)
            {
              if (read_data (v) != RESULT_SUCCESS)
              {
                return data_if_end (Line);
              }
            }
            else
            {
              if (file_data (v, tbuf, tlen) != RESULT_SUCCESS)
              {
                return file_if_end (Line);
              }
            }
            /* OK */
            My->LastInputCount++;
          }
        }
      }
      break;
    case 3:
      {
        /*
           OPTION BASE 0
           DIM C(2,3,4)
           ...
           MAT READ C 
           ...
           FOR I = 0 TO 2
           FOR J = 0 TO 3
           FOR K = 0 TO 4
           READ C(I,J,K)
           NEXT K
           PRINT
           NEXT J
           PRINT
           NEXT I
           ...
         */
        for (v->VINDEX[0] = v->LBOUND[0]; v->VINDEX[0] <= v->UBOUND[0];
             v->VINDEX[0]++)
        {
          for (v->VINDEX[1] = v->LBOUND[1]; v->VINDEX[1] <= v->UBOUND[1];
               v->VINDEX[1]++)
          {
            for (v->VINDEX[2] = v->LBOUND[2]; v->VINDEX[2] <= v->UBOUND[2];
                 v->VINDEX[2]++)
            {
              if (My->CurrentFile == My->SYSIN)
              {
                if (read_data (v) != RESULT_SUCCESS)
                {
                  return data_if_end (Line);
                }
              }
              else
              {
                if (file_data (v, tbuf, tlen) != RESULT_SUCCESS)
                {
                  return file_if_end (Line);
                }
              }
              /* OK */
              My->LastInputCount++;
            }
          }
        }
      }
      break;
    }
    /* process the next variable, if any  */
  }
  while (line_skip_seperator (Line));
  return (Line);
}

extern LineType *
bwb_MAT_GET (LineType * Line)
{
  /* MAT GET filename$ , matrix [, ...] */
  VariantType E;
  VariantType *e;
   
  assert (Line != NULL);
  assert(My != NULL);
  assert(My->SYSIN != NULL);
  assert(My->SYSIN->cfp != NULL);

  e = &E;
  My->CurrentFile = My->SYSIN;
  if (line_read_expression (Line, e) == FALSE)        /* bwb_MAT_GET */
  {
    WARN_SYNTAX_ERROR;
    return (Line);
  }
  if (e->VariantTypeCode == StringTypeCode)
  {
    /* STRING */
    /* MAT GET filename$ ... */
    if (is_empty_string (e->Buffer))
    {
      /* MAT GET "" ... is an error */
      WARN_BAD_FILE_NAME;
      return (Line);
    }
    My->CurrentFile = find_file_by_name (e->Buffer);
    if (My->CurrentFile == NULL)
    {
      /* implicitly OPEN for reading */
      My->CurrentFile = file_new ();
      My->CurrentFile->cfp = fopen (e->Buffer, "r");
      if (My->CurrentFile->cfp == NULL)
      {
        WARN_BAD_FILE_NAME;
        return (Line);
      }
      My->CurrentFile->FileNumber = file_next_number ();
      My->CurrentFile->DevMode = DEVMODE_INPUT;
      My->CurrentFile->width = 0;
      /* WIDTH == RECLEN */
      My->CurrentFile->col = 1;
      My->CurrentFile->row = 1;
      My->CurrentFile->delimit = ',';
      My->CurrentFile->buffer = NULL;
      if (My->CurrentFile->FileName != NULL)
      {
        free (My->CurrentFile->FileName);
        My->CurrentFile->FileName = NULL;
      }
      My->CurrentFile->FileName = e->Buffer;
      e->Buffer = NULL;
    }
  }
  else
  {
    /* NUMBER -- file must already be OPEN */
    /* GET filenumber ... */
    if (e->Number < 0)
    {
      /* "MAT GET # -1" is an error */
      WARN_BAD_FILE_NUMBER;
      return (Line);
    }
    if (e->Number == 0)
    {
      /* "MAT GET # 0" is an error */
      WARN_BAD_FILE_NUMBER;
      return (Line);
    }
    /* normal file */
    My->CurrentFile = find_file_by_number ((int) bwb_rint (e->Number));
    if (My->CurrentFile == NULL)
    {
      /* file not OPEN */
      WARN_BAD_FILE_NUMBER;
      return (Line);
    }
  }
  if (My->CurrentFile == NULL)
  {
    WARN_BAD_FILE_NUMBER;
    return (Line);
  }
  if ((My->CurrentFile->DevMode & DEVMODE_READ) == 0)
  {
    WARN_BAD_FILE_NUMBER;
    return (Line);
  }
  if (line_skip_seperator (Line))
  {
    /* OK */
  }
  else
  {
    WARN_SYNTAX_ERROR;
    return (Line);
  }
  return file_read_matrix (Line);
}


extern LineType *
bwb_MAT_READ (LineType * Line)
{
  /* MAT READ arrayname [;|,] */
  /* Array must be 1, 2 or 3 dimensions */
  /* Array may be either NUMBER or STRING */
   
  assert (Line != NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);
  assert(My->SYSIN != NULL);
  assert(My->SYSIN->cfp != NULL);

  My->CurrentFile = My->SYSIN;
  My->LastInputCount = 0;
  if (line_skip_FilenumChar (Line))
  {
    /* MAT READ # filenum, varlist */
    int FileNumber;

    if (line_read_integer_expression (Line, &FileNumber) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (Line);
    }

    if (line_skip_seperator (Line))
    {
      /* OK */
    }
    else
    {
      WARN_SYNTAX_ERROR;
      return (Line);
    }
    My->CurrentFile = find_file_by_number (FileNumber);
    if (My->CurrentFile == NULL)
    {
      WARN_BAD_FILE_NUMBER;
      return (Line);
    }
    if (My->CurrentFile != My->SYSIN)
    {
      if ((My->CurrentFile->DevMode & DEVMODE_READ) == 0)
      {
        WARN_BAD_FILE_NUMBER;
        return (Line);
      }
      if (My->CurrentFile->cfp == NULL)
      {
        WARN_BAD_FILE_NUMBER;
        return (Line);
      }
    }
    /* "MAT READ # 0, varlist" is the same as "MAT READ varlist" */
  }
  return file_read_matrix (Line);
}

static ResultType
input_data (VariableType * Variable, char *tbuf, int tlen)
{
  /*
   **
   ** read one INPUT item
   **
   */
  int p;
  ResultType Result;
  VariantType Variant;
  VariantType *X;
   
  assert (Variable != NULL);
  assert (tbuf != NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);
  assert(My->SYSIN != NULL);
  assert(My->SYSIN->cfp != NULL);
  assert(My->CurrentFile != NULL);
  assert (My->CurrentFile == My->SYSIN);

  Result = RESULT_UNPARSED;
  X = &Variant;
  CLEAR_VARIANT (X);
  if (tbuf[0] == NulChar)
  {
    /* Get more data */
    bwx_input ("?", FALSE, tbuf, tlen);
    if (tbuf[0] == NulChar)
    {
      return RESULT_UNPARSED;
    }
    /*
     **
     ** make sure we can parse everything in tbuf
     **
     */
    p = 0;
    do
    {
      do
      {
        if (VAR_IS_STRING (Variable))
        {
          Result = parse_string (tbuf, &p, X);
        }
        else
        {
          Result = parse_number (tbuf, &p, X, FALSE);
        }
      }
      while (buff_skip_seperator (tbuf, &p) && Result == RESULT_SUCCESS);
      /* verify we consumed all user values */
      if (buff_is_eol (tbuf, &p))
      {
        /* we reached the end of the user's input */
      }
      else
      {
        /* garbage in user's input */
        Result = RESULT_UNPARSED;
      }
      if (Result != RESULT_SUCCESS)
      {
        tbuf[0] = NulChar;
        bwx_input ("?Redo", FALSE, tbuf, tlen);
        if (tbuf[0] == NulChar)
        {
          return RESULT_UNPARSED;
        }
        p = 0;
      }
    }
    while (Result != RESULT_SUCCESS);
    /*
     **
     ** so, we can parse all of the user's input (everything in tbuf)
     **
     */
  }
  /* process one value */
  p = 0;
  if (VAR_IS_STRING (Variable))
  {
    Result = parse_string (tbuf, &p, X);
  }
  else
  {
    Result = parse_number (tbuf, &p, X, FALSE);
  }
  if (Result != RESULT_SUCCESS)
  {
    WARN_INTERNAL_ERROR;
    return RESULT_UNPARSED;
  }
  if (X->VariantTypeCode == StringTypeCode
      && My->CurrentVersion->
      OptionFlags & OPTION_BUGS_ON /* DATA allows embedded quote pairs */ )
  {
    int i;
    int n;
    n = X->Length;
    for (i = 0; i < n; i++)
    {
      if (X->Buffer[i + 0] == My->CurrentVersion->OptionQuoteChar
          && X->Buffer[i + 1] == My->CurrentVersion->OptionQuoteChar)
      {
        bwb_strncpy (&X->Buffer[i + 0], &X->Buffer[i + 1], n - i);
        n--;
      }
    }
    X->Length = n;
  }
  if (var_set (Variable, X) == FALSE)
  {
    WARN_VARIABLE_NOT_DECLARED;
    return RESULT_UNPARSED;
  }
  /* determine whether all user input was consumed */
  if (buff_is_eol (tbuf, &p))
  {
    /* we have consumed the entire buffer */
    tbuf[0] = NulChar;
    return RESULT_SUCCESS;
  }
  if (buff_skip_char (tbuf, &p, My->CurrentFile->delimit))        /* buff_skip_comma */
  {
    /* shift the buffer left, just past the comma (,) */
    bwb_strcpy (tbuf, &tbuf[p]);
    return RESULT_SUCCESS;
  }
  /* garbage after the value we just READ */
  WARN_BAD_DATA;
  return RESULT_UNPARSED;
}

extern LineType *
bwb_MAT_INPUT (LineType * Line)
{
  /* MAT INPUT arrayname [;|,] */
  /* Array must be 1, 2 or 3 dimensions */
  /* Array may be either NUMBER or STRING */
  VariableType *v;
   
  assert (Line != NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);
  assert(My->SYSIN != NULL);
  assert(My->SYSIN->cfp != NULL);
  assert(My->ConsoleInput != NULL);
  assert(MAX_LINE_LENGTH > 1);

  My->CurrentFile = My->SYSIN;
  My->LastInputCount = 0;
  if (line_skip_FilenumChar (Line))
  {
    /* MAT INPUT # filenum, varlist */
    int FileNumber;

    if (line_read_integer_expression (Line, &FileNumber) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (Line);
    }

    if (line_skip_seperator (Line))
    {
      /* OK */
    }
    else
    {
      WARN_SYNTAX_ERROR;
      return (Line);
    }
    My->CurrentFile = find_file_by_number (FileNumber);
    if (My->CurrentFile == NULL)
    {
      WARN_BAD_FILE_NUMBER;
      return (Line);
    }
    if (My->CurrentFile != My->SYSIN)
    {
      if ((My->CurrentFile->DevMode & DEVMODE_READ) == 0)
      {
        WARN_BAD_FILE_NUMBER;
        return (Line);
      }
      if (My->CurrentFile->cfp == NULL)
      {
        WARN_BAD_FILE_NUMBER;
        return (Line);
      }
    }
    /* "MAT INPUT # 0, varlist" is the same as "MAT INPUT varlist" */
  }

  do
  {
    char *tbuf;
    int tlen;

    tbuf = My->ConsoleInput;
    tlen = MAX_LINE_LENGTH;
    if (My->CurrentFile->width > 0 && My->CurrentFile->buffer != NULL)
    {
      tlen = My->CurrentFile->width;
      tbuf = My->CurrentFile->buffer;
    }
    tbuf[0] = NulChar;


    My->LastInputCount = 0;
    if ((v = line_read_matrix (Line)) == NULL)
    {
      WARN_SUBSCRIPT_OUT_OF_RANGE;
      return (Line);
    }
    /* variable MUST be an array of 1, 2 or 3 dimensions */
    if (v->dimensions < 1)
    {
      WARN_SUBSCRIPT_OUT_OF_RANGE;
      return (Line);
    }
    if (v->dimensions > 3)
    {
      WARN_SUBSCRIPT_OUT_OF_RANGE;
      return (Line);
    }

    /* INPUT array */
    switch (v->dimensions)
    {
    case 1:
      {
        /*
           OPTION BASE 0
           DIM A(5)
           ...
           MAT INPUT A 
           ...
           FOR I = 0 TO 5
           INPUT A(I)
           NEXT I
           ...
         */
        My->LastInputCount = 0;
        for (v->VINDEX[0] = v->LBOUND[0]; v->VINDEX[0] <= v->UBOUND[0];
             v->VINDEX[0]++)
        {
          if (My->CurrentFile == My->SYSIN)
          {
            if (input_data (v, tbuf, tlen) != RESULT_SUCCESS)
            {
              /*
                 WARN_INPUT_PAST_END;
               */
              return (Line);
            }
          }
          else
          {
            if (file_data (v, tbuf, tlen) != RESULT_SUCCESS)
            {
              return file_if_end (Line);
            }
          }
          /* OK */
          My->LastInputCount++;
        }
      }
      break;
    case 2:
      {
        /*
           OPTION BASE 0
           DIM B(2,3)
           ...
           MAT INPUT B 
           ...
           FOR I = 0 TO 2
           FOR J = 0 TO 3
           INPUT B(I,J)
           NEXT J
           PRINT
           NEXT I
           ...
         */
        My->LastInputCount = 0;
        for (v->VINDEX[0] = v->LBOUND[0]; v->VINDEX[0] <= v->UBOUND[0];
             v->VINDEX[0]++)
        {
          for (v->VINDEX[1] = v->LBOUND[1]; v->VINDEX[1] <= v->UBOUND[1];
               v->VINDEX[1]++)
          {
            if (My->CurrentFile == My->SYSIN)
            {
              if (input_data (v, tbuf, tlen) != RESULT_SUCCESS)
              {
                /*
                   WARN_INPUT_PAST_END;
                 */
                return (Line);
              }
            }
            else
            {
              if (file_data (v, tbuf, tlen) != RESULT_SUCCESS)
              {
                return file_if_end (Line);
              }
            }
            /* OK */
            My->LastInputCount++;
          }
        }
      }
      break;
    case 3:
      {
        /*
           OPTION BASE 0
           DIM C(2,3,4)
           ...
           MAT INPUT C 
           ...
           FOR I = 0 TO 2
           FOR J = 0 TO 3
           FOR K = 0 TO 4
           INPUT C(I,J,K)
           NEXT K
           PRINT
           NEXT J
           PRINT
           NEXT I
           ...
         */
        My->LastInputCount = 0;
        for (v->VINDEX[0] = v->LBOUND[0]; v->VINDEX[0] <= v->UBOUND[0];
             v->VINDEX[0]++)
        {
          for (v->VINDEX[1] = v->LBOUND[1]; v->VINDEX[1] <= v->UBOUND[1];
               v->VINDEX[1]++)
          {
            for (v->VINDEX[2] = v->LBOUND[2]; v->VINDEX[2] <= v->UBOUND[2];
                 v->VINDEX[2]++)
            {
              if (My->CurrentFile == My->SYSIN)
              {
                if (input_data (v, tbuf, tlen) != RESULT_SUCCESS)
                {
                  /*
                     WARN_INPUT_PAST_END;
                   */
                  return (Line);
                }
              }
              else
              {
                if (file_data (v, tbuf, tlen) != RESULT_SUCCESS)
                {
                  return file_if_end (Line);
                }
              }
              /* OK */
              My->LastInputCount++;
            }
          }
        }
      }
      break;
    }
    /* process the next variable, if any  */
  }
  while (line_skip_seperator (Line));
  return (Line);
}

/* EOF */
