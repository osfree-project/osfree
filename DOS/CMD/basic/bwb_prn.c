/***************************************************************
  
        bwb_prn.c       Print and Error-Handling Commands
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

static int buff_read_using (char *buffer, int *position, char *format_string,
                            int format_length);
static LineType *bwb_mat_dump (LineType * l, int IsWrite);
static int bwb_print_at (LineType * l);
static void CleanNumericString (char *prnbuf, int RemoveDot);
static int CountDigits (char *Buffer);
static LineType *D71_PUT (LineType * l);
static LineType *file_write_matrix (LineType * l, char delimit);
static LineType *H14_PUT (LineType * Line);
static void internal_print (LineType * l, int IsCSV);
static int is_magic_number (char *buffer);
static int is_magic_string (char *buffer);
static int line_read_using (LineType * l, char *format_string,
                            int format_length);
static void next_zone (void);
static int parse_file_number (LineType * l);
static void print_using_number (char *buffer, int *position, VariantType * e);
static void print_using_string (char *buffer, int *position, VariantType * e);
static void print_using_variant (char *buffer, int *position, VariantType * e,
                                 int IsCSV);
static LineType *S70_PUT (LineType * l);
static void xputc1 (char c);
static void xputc2 (char c);
static void xputs (char *buffer);


/*
We try to allow as many legacy PRINT USING formats as reasonable.
Many legacy PRINT USING formats are incompatible with one another.
For example:
1) some use '%' for strings, others use '%' for numbers, others consider '%' as a lieral.
2) some count a leading or traling signs in the width, while others do not.
3) when a value requires more digits than the assigned width:
   a) some truncate the displayed value to the width, 
   b) some expand the width, 
   c) some print a number of '%' or '*', and 
   d) some halt processing.
There is no perfect solution that will work for all possible dialects.
*/


#define PrintUsingNumberDigit    My->CurrentVersion->OptionUsingDigit        /* Digit placeholder, usually '#' */
#define PrintUsingNumberComma    My->CurrentVersion->OptionUsingComma        /* Comma, such as thousands, usually ',' */
#define PrintUsingNumberPeriod   My->CurrentVersion->OptionUsingPeriod        /* Period, such as dollars and cents, usually '.' */
#define PrintUsingNumberPlus     My->CurrentVersion->OptionUsingPlus        /* Plus  sign, positive value, usually '+' */
#define PrintUsingNumberMinus    My->CurrentVersion->OptionUsingMinus        /* Minus sign, negative value, usually '-' */
#define PrintUsingNumberExponent My->CurrentVersion->OptionUsingExrad        /* Exponential format, usually '^' */
#define PrintUsingNumberDollar   My->CurrentVersion->OptionUsingDollar        /* Currency symbol, usually '$' */
#define PrintUsingNumberFiller   My->CurrentVersion->OptionUsingFiller        /* Print filler, such as checks, usually '*' */
#define PrintUsingLiteral        My->CurrentVersion->OptionUsingLiteral        /* The next char is a literal, usually '_' */
#define PrintUsingStringFirst    My->CurrentVersion->OptionUsingFirst        /* The first character of the string, usually '!' */
#define PrintUsingStringAll      My->CurrentVersion->OptionUsingAll        /* Print the entire string, usually '&' */
#define PrintUsingStringLength   My->CurrentVersion->OptionUsingLength        /* Print a substring, usually '%' */


/*
**
** ZoneChar is a MAGIC character code used by file_write_matrix() to request printing by zones.
** ZoneChar can be any character, other than NulChar, that the user will not use as a literal delimiter.
** The user is allowed to specify CHR$(9), '\t', as a literal delimiter.
**
*/
#define ZoneChar 0x01                /* an unlikely literal delimiter */


int
is_empty_string (char *Buffer)
{
   

  if (Buffer == NULL)
  {
    return TRUE;
  }
  while (*Buffer == ' ')
  {
    Buffer++;
  }
  if (*Buffer == NulChar)
  {
    return TRUE;
  }
  return FALSE;
}


FileType *
find_file_by_name (char *FileName)
{
  FileType *F;
   
  if (is_empty_string (FileName))
  {
    /* the rules for Console and Printer vary by command */
    return NULL;
  }
  /* search the list of OPEN files */
  assert( My != NULL );
  for (F = My->FileHead; F != NULL; F = F->next)
  {
    assert( F != NULL );
    if (F->DevMode == DEVMODE_CLOSED)
    {
    }
    else if (F->FileName == NULL)
    {
    }
    else if (bwb_stricmp (F->FileName, FileName) == 0)
    {
      /* FOUND */
      return F;
    }
  }
  /* NOT FOUND */
  return NULL;
}


FileType *
find_file_by_number (int FileNumber)
{
  FileType *F;
   

  /* handle MAGIC file numbers */
  if (FileNumber <= 0)
  {
    /* the rules for Console and Printer vary by command */
    return NULL;
  }
  /* search the list of OPEN files */
  assert( My != NULL );
  for (F = My->FileHead; F != NULL; F = F->next)
  {
    assert( F != NULL );
    if (F->DevMode != DEVMODE_CLOSED)
    {
      if (F->FileNumber == FileNumber)
      {
        /* FOUND */
        return F;
      }
    }
  }
  /* NOT FOUND */
  return NULL;
}


FileType *
file_new (void)
{
  /* search for an empty slot.  If not found, add a new slot. */
  FileType *F;
   
  assert( My != NULL );
  for (F = My->FileHead; F != NULL; F = F->next)
  {
    assert( F != NULL );
    if (F->DevMode == DEVMODE_CLOSED)
    {
      /* FOUND */
      return F;
    }
  }
  /* NOT FOUND */
  if ((F = (FileType *) calloc (1, sizeof (FileType))) == NULL)
  {
    WARN_OUT_OF_MEMORY;
    return NULL;
  }
  assert( F != NULL );
  F->next = My->FileHead;
  My->FileHead = F;
  return F;
}


void
file_clear (FileType * F)
{
  /* clean up a file slot that is no longer needed */
   
  assert (F != NULL);

  clear_virtual_by_file (F->FileNumber);
  F->FileNumber = 0;
  F->DevMode = DEVMODE_CLOSED;        /* DEVMODE_ item */
  F->width = 0;                        /* width for OUTPUT and APPEND; reclen for RANDOM; not used for INPUT or BINARY */
  F->col = 0;                        /* current column for OUTPUT and APPEND */
  F->row = 0;                        /* current row for OUTPUT and APPEND */
  F->EOF_LineNumber = 0;        /* CBASIC-II: IF END # filenumber THEN linenumber */
  F->delimit = NulChar;                /* DELIMIT for READ and WRITE */
  if (F->FileName != NULL)
  {
    free (F->FileName);
    F->FileName = NULL;
  }
  if (F->cfp != NULL)
  {
    bwb_fclose (F->cfp);
    F->cfp = NULL;
  }
  if (F->buffer != NULL)
  {
    free (F->buffer);
    F->buffer = NULL;
  }

}

int
file_next_number (void)
{
  int FileNumber;
  FileType *F;
   

  FileNumber = 0;
  assert( My != NULL );
  for (F = My->FileHead; F != NULL; F = F->next)
  {
    assert( F != NULL );
    if (F->DevMode != DEVMODE_CLOSED)
    {
      if (F->FileNumber > FileNumber)
      {
        FileNumber = F->FileNumber;
      }
    }
  }
  /* 'FileNumber' is the highest FileNumber that is currently open */
  FileNumber++;
  return FileNumber;
}



/***************************************************************
  
      FUNCTION:       bwx_putc()
  
   DESCRIPTION:    This function outputs a single character
         to the default output device.
  
***************************************************************/

static void
CleanNumericString (char *prnbuf, int RemoveDot)
{
  /* remove trailing zeroes */
  char *E;
  char *D;
   
  assert (prnbuf != NULL);

  E = bwb_strchr (prnbuf, 'E');
  if (E == NULL)
  {
    E = bwb_strchr (prnbuf, 'e');
  }
  if (E)
  {
    /* SCIENTIFIC == SCALED notation */
    /* trim leading zeroes in exponent */
    char *F;
    char *G;

    F = E;
    while (bwb_isalpha (*F))
    {
      F++;
    }
    while (*F == '+' || *F == '-')
    {
      /* skip sign */
      F++;
    }
    G = F;
    while (*G == '0' || *G == ' ')
    {
      /* skip leading zeroes or spaces */
      G++;
    }
    if (G > F)
    {
      bwb_strcpy (F, G);
    }
    G = NULL;                        /* no longer valid */
    *E = NulChar;                /* for bwb_strlen()  */
  }
  D = bwb_strchr (prnbuf, '.');
  if (D)
  {
    int N;

    N = bwb_strlen (D);
    if (N > 1)
    {
      int M;

      N--;
      M = N;
      while (D[N] == '0')
      {
        /* remove trailing zeroes */
        D[N] = '_';
        N--;
      }
      if (RemoveDot)
      {
        if (E)
        {
          /* SCIENTIFIC == SCALED notation */
          /* do NOT remove '.' */
        }
        else
        {
          /* NORMAL  == UNSCALED notation */
          /* remove trailing '.' */
          /* this will only occur for integer values */
          while (D[N] == '.')
          {
            /* _###. POSITIVE INTEGER */
            /* -###. NEGATIVE INTEGER */
            D[N] = '_';
            N--;
          }
        }
      }
      if (N < M)
      {
        if (E)
        {
          /* SCIENTIFIC == SCALED notation */
          *E = 'E';
          E = NULL;
        }
        N++;
        /* if INTEGER, then N == 0, else N > 0 */
        M++;
        /* if SCIENTIFIC, then  *M == 'E' else *M == NulChar */
        bwb_strcpy (&(D[N]), &(D[M]));
      }
    }
  }
  if (E)
  {
    /* SCIENTIFIC == SCALED notation */
    *E = 'E';
    E = NULL;
  }
  if (prnbuf[1] == '0' && prnbuf[2] == '.')
  {
    /* _0.### POSITIVE FRACTION ==> _.### */
    /* -0.### NEGATIVE FRACTION ==> -.### */
    bwb_strcpy (&(prnbuf[1]), &(prnbuf[2]));
  }
  if (prnbuf[1] == '.' && prnbuf[2] == 'E')
  {
    /* _.E POSITIVE ZERO ==> _0 */
    /* -.E NEGATIVE ZERO ==> _0 */
    bwb_strcpy (prnbuf, " 0");
  }
}

static int
CountDigits (char *Buffer)
{
  int NumDigits;
  char *P;
   
  assert (Buffer != NULL);


  /* determine the number of significant digits */
  NumDigits = 0;
  P = Buffer;
  while (*P)
  {
    if (bwb_isalpha (*P))
    {
      /* 'E', 'e', and so on. */
      break;
    }
    if (bwb_isdigit (*P))
    {
      NumDigits++;
    }
    P++;
  }
  return NumDigits;
}

extern void
FormatBasicNumber (DoubleType Input, char *Output /* [ NUMLEN ] */ )
{
   /*******************************************************************************
   
   This is essentially sprintf( Output, "%g", Input ), 
   except the rules for selecting between "%e", "%f", and "%d" are different.

   The C rules depend upon the value of the exponent.
   The BASIC rules depend upon the number of significant digits.

   The results of this routine have been verified by the NBS2 test suite, so...
      
   THINK VERY CAREFULLY BEFORE MAKING ANY CHANGES TO THIS ROUTINE.   
   
   *******************************************************************************/
  char *E;
   
  assert (Output != NULL);

  assert( My != NULL );
  if (My->OptionScaleInteger >= 1
      && My->OptionScaleInteger <= My->OptionDigitsInteger)
  {
    /* round */
    DoubleType Scale;
    Scale = pow (10, My->OptionScaleInteger);
    assert( Scale != 0 );
    Input = bwb_rint (Input * Scale) / Scale;
  }
  /* print in scientific form first, to determine exponent and significant digits */
  sprintf (Output, "% 1.*E", My->OptionDigitsInteger - 1, Input);
  E = bwb_strchr (Output, 'E');
  if (E == NULL)
  {
    E = bwb_strchr (Output, 'e');
  }
  if (E)
  {
    /* valid */
    int Exponent;
    int NumDigits;
    int DisplayDigits;
    int zz;
    char *F;                        /* pointer to the exponent's value */
    F = E;
    while (bwb_isalpha (*F))
    {
      F++;
    }
    Exponent = atoi (F);
    CleanNumericString (Output, FALSE);
    NumDigits = CountDigits (Output);
    DisplayDigits = MIN (NumDigits, My->OptionDigitsInteger);
    zz = MAX (Exponent, DisplayDigits - Exponent - 2);
    if (zz >= My->OptionDigitsInteger)
    {
      /* SCIENTIFIC */
      sprintf (Output, "%# 1.*E", DisplayDigits - 1, Input);
    }
    else if (Input == (int) Input)
    {
      /* INTEGER */
      sprintf (Output, "% *d", DisplayDigits, (int) Input);
    }
    else
    {
      /* FLOAT */
      int Before;                /* number of digits before the '.' */
      int After;                /* number of digits after  the '.' */

      Before = Exponent + 1;
      if (Before < 0)
      {
        Before = 0;
      }
      After = My->OptionDigitsInteger - Before;
      if (After < 0)
      {
        After = 0;
      }
      sprintf (Output, "%# *.*f", Before, After, Input);
    }
    CleanNumericString (Output, FALSE);
  }
  else
  {
    /* ERROR, NAN, INFINITY, ETC. */
  }
}



LineType *
bwb_LPRINT (LineType * l)
{
  int IsCSV;
   
  assert (l != NULL);

  assert( My != NULL );
  assert( My->SYSPRN != NULL );
  My->CurrentFile = My->SYSPRN;
  IsCSV = FALSE;
  internal_print (l, IsCSV);
  return (l);
}


/***************************************************************
  
        FUNCTION:       bwb_print()
  
        DESCRIPTION:    This function implements the BASIC PRINT
                        command.
  
   SYNTAX:     PRINT [# device-number,][USING format-string$;] expressions...
  
***************************************************************/


static int
bwb_print_at (LineType * l)
{
  int position;
  int r;
  int c;
   
  assert (l != NULL);


  position = 0;
  r = 0;
  c = 0;
  if (line_read_integer_expression (l, &position))
  {
    /* OK */
  }
  else
  {
    WARN_SYNTAX_ERROR;
    return FALSE;
  }

  if (line_skip_seperator (l))
  {
    /* OK */
  }
  else
  {
    WARN_SYNTAX_ERROR;
    return FALSE;
  }

  if (position < 0)
  {
    WARN_SYNTAX_ERROR;
    return FALSE;
  }


  assert( My != NULL );
  assert( My->SYSOUT != NULL );
  if (My->SYSOUT->width <= 0)
  {
    WARN_SYNTAX_ERROR;
    return FALSE;
  }
  if (My->SCREEN_ROWS <= 0)
  {
    WARN_SYNTAX_ERROR;
    return FALSE;
  }
  assert( My->CurrentFile == My->SYSOUT );
  /* position is 0-based.  0 is top left, */
  assert( My->CurrentFile != NULL );
  assert( My->CurrentFile->width != 0 );
  r = position / My->CurrentFile->width;
  c = position - r * My->CurrentFile->width;
  while (r >= My->SCREEN_ROWS)
  {
    r -= My->SCREEN_ROWS;
  }
  r++;                                /* 0-based to 1-based */
  c++;                                /* 0-based to 1-based */
  bwx_LOCATE (r, c);
  return TRUE;
}


static int
parse_file_number (LineType * l)
{
  /* ... # FileNumber , ... */
  int FileNumber;
   
  assert (l != NULL);


  if (line_read_integer_expression (l, &FileNumber) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return FALSE;
  }

  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
  if (My->CurrentVersion->OptionVersionValue & (C77))
  {
    /* 
       CBASIC-II: SERIAL & RANDOM file writes
       PRINT # file_number                 ; expression [, expression] ' SERIAL write
       PRINT # file_number , record_number ; expression [, expression] ' RANDOM write
     */

    if (FileNumber <= 0)
    {
      WARN_BAD_FILE_NUMBER;
      return FALSE;
    }
    /* normal file */
    My->CurrentFile = find_file_by_number (FileNumber);
    if (My->CurrentFile == NULL)
    {
      WARN_BAD_FILE_NUMBER;
      return FALSE;
    }


    if (line_skip_CommaChar (l) /* comma specific */ )
    {
      /* 
         PRINT # file_number , record_number ; expression [, expression] ' RANDOM write
       */
      /* get the RecordNumber */
      int RecordNumber;

      if ((My->CurrentFile->DevMode & DEVMODE_RANDOM) == 0)
      {
        WARN_BAD_FILE_MODE;
        return FALSE;
      }
      if (My->CurrentFile->width <= 0)
      {
        WARN_FIELD_OVERFLOW;
        return FALSE;
      }
      if (line_read_integer_expression (l, &RecordNumber) == FALSE)
      {
        WARN_SYNTAX_ERROR;
        return FALSE;
      }
      if (RecordNumber <= 0)
      {
        WARN_BAD_RECORD_NUMBER;
        return FALSE;
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
    if (line_is_eol (l))
    {
      /* PRINT # filenum          */
      /* PRINT # filenum , recnum */
    }
    else if (line_skip_SemicolonChar (l) /* semicolon specific */ )
    {
      /* PRINT # filenum          ; */
      /* PRINT # filenum , recnum ; */
    }
    else
    {
      WARN_SYNTAX_ERROR;
      return FALSE;
    }
    return TRUE;
  }
  /* 
     SERIAL file writes:
     PRINT # file_number   
     PRINT # file_number [, expression]
   */
  if (FileNumber < 0)
  {
    My->CurrentFile = My->SYSPRN;
  }
  else if (FileNumber == 0)
  {
    My->CurrentFile = My->SYSOUT;
  }
  else
  {
    /* normal file */
    My->CurrentFile = find_file_by_number (FileNumber);
  }
  if (My->CurrentFile == NULL)
  {
    WARN_BAD_FILE_NUMBER;
    return FALSE;
  }
  if ((My->CurrentFile->DevMode & DEVMODE_WRITE) == 0)
  {
    WARN_BAD_FILE_NUMBER;
    return FALSE;
  }
  if (line_is_eol (l))
  {
    /* PRINT # 2 */
  }
  else if (line_skip_seperator (l))
  {
    /* PRINT # 2 , ... */
  }
  else
  {
    WARN_SYNTAX_ERROR;
    return FALSE;
  }
  return TRUE;
}

LineType *
bwb_PRINT (LineType * l)
{
  int IsCSV;
   
  assert (l != NULL);

  IsCSV = FALSE;
  assert( My != NULL );
  if (My->IsPrinter == TRUE)
  {
    My->CurrentFile = My->SYSPRN;
  }
  else
  {
    My->CurrentFile = My->SYSOUT;
  }
  internal_print (l, IsCSV);
  return (l);
}

/***************************************************************
  
        FUNCTION:       internal_print()
  
   DESCRIPTION:    This function implements the PRINT
         command, utilizing a specified file our
         output device.
  
***************************************************************/

static int
buff_read_using (char *buffer, int *position, char *format_string,
                 int format_length)
{
  int p;
   
  assert (buffer != NULL);
  assert (position != NULL);
  assert (format_string != NULL);

  p = *position;

  if (buff_skip_word (buffer, &p, "USING"))
  {
    buff_skip_spaces (buffer, &p);        /* keep this */
    if (bwb_isdigit (buffer[p]))
    {
      /* PRINT USING ### */
      int n;
      int LineNumber;
      LineType *x;
      char *C;
      char *F;

      n = 0;
      LineNumber = 0;
      x = NULL;
      if (buff_read_line_number (buffer, &p, &LineNumber) == FALSE)
      {
        WARN_SYNTAX_ERROR;
        return FALSE;
      }
      /* check for target label */
      x = find_line_number (LineNumber);        /* USING 100 */
      if (x == NULL)
      {
        WARN_UNDEFINED_LINE;
        return FALSE;
      }
      /* line exists */
      if (x->cmdnum != C_IMAGE)
      {
        WARN_UNDEFINED_LINE;
        return FALSE;
      }
      /* line contains IMAGE command */
      C = x->buffer;
      C += x->Startpos;
      F = format_string;
      /* look for leading quote in IMAGE "..." */
      while (*C == ' ')
      {
        C++;
      }
      assert( My != NULL );
      assert( My->CurrentVersion != NULL );
      if (*C == My->CurrentVersion->OptionQuoteChar)
      {
        /* QUOTED */
        /* skip leading quote */
        C++;
        while (*C != NulChar && *C != My->CurrentVersion->OptionQuoteChar)
        {
          /* copy format string, but not the trailing quote */
          if (n == format_length)
          {
            WARN_STRING_TOO_LONG;
            break;
          }
          *F = *C;
          C++;
          F++;
          n++;
        }
        /* skip trailing quote */
      }
      else
      {
        /* UNQUOTED */
        while (*C)
        {
          /* copy format string verbatim */
          if (n == format_length)
          {
            WARN_STRING_TOO_LONG;
            break;
          }
          *F = *C;
          C++;
          F++;
          n++;
        }
      }
      /* terminate format string */
      *F = NulChar;
      if (buff_skip_seperator (buffer, &p) == FALSE)
      {
        WARN_SYNTAX_ERROR;
        return FALSE;
      }
    }
    else
    {
      {
        char *Value;

        Value = NULL;
        if (buff_read_string_expression (buffer, &p, &Value) == FALSE)
        {
          WARN_SYNTAX_ERROR;
          return FALSE;
        }
        if (Value == NULL)
        {
          WARN_SYNTAX_ERROR;
          return FALSE;
        }
        if (bwb_strlen (Value) > format_length)
        {
          WARN_STRING_TOO_LONG;
          Value[format_length] = NulChar;
        }
        bwb_strcpy (format_string, Value);
        free (Value);
        Value = NULL;
      }
      if (buff_skip_seperator (buffer, &p) == FALSE)
      {
        WARN_SYNTAX_ERROR;
        return FALSE;
      }
    }
    *position = p;
    return TRUE;
  }
  return FALSE;
}

static int
line_read_using (LineType * l, char *format_string, int format_length)
{
  assert (l != NULL);
  assert (format_string != NULL);
  return buff_read_using (l->buffer, &(l->position), format_string,
                          format_length);
}

static void
internal_print (LineType * l, int IsCSV)
{
  /* if no arguments, simply print CR and return */
  /* 1980 PRINT  , , ,"A" */
  int OutputCR;
  char *format_string;
  int format_length;
  int format_position;
   
  assert (l != NULL);


  OutputCR = TRUE;
  assert( My != NULL );
  assert( My->ConsoleOutput != NULL );
  assert( MAX_LINE_LENGTH > 1 );
  format_string = My->ConsoleOutput;
  format_length = MAX_LINE_LENGTH;
  format_position = 0;
  format_string[0] = NulChar;

  if (line_skip_FilenumChar (l))
  {
    /* PRINT # file, ... */
    if (parse_file_number (l) == FALSE)
    {
      return;
    }
    assert( My->CurrentVersion != NULL );
    if (My->CurrentVersion->OptionVersionValue & (C77)
        && My->CurrentFile->FileNumber > 0)
    {
      /* 
       **
       ** CBASIC-II files are CSV files.
       **
       ** Strings are quoted other than PRINT USING.
       ** Comma seperator writes a literal comma.
       ** Semicolon seperator writes a literal comma.
       ** Numbers do NOT have leading or trailing spaces.
       **
       */
      IsCSV = TRUE;
    }
    OutputCR = TRUE;
  }
  else if (line_skip_AtChar (l))
  {
    /* PRINT @ position, ... */
    assert( My->SYSOUT != NULL );
    My->CurrentFile = My->SYSOUT;
    if (bwb_print_at (l) == FALSE)
    {
      return;
    }
    OutputCR = TRUE;
  }
  else if (My->CurrentVersion->OptionVersionValue & (B15|T80|HB1|HB2) 
  &&  line_skip_word (l, "AT"))
  {
    /* PRINT AT position, ... */
    assert( My->SYSOUT != NULL );
    My->CurrentFile = My->SYSOUT;
    if (bwb_print_at (l) == FALSE)
    {
      return;
    }
    OutputCR = TRUE;
  }
  assert( My->CurrentFile != NULL );

  while (line_is_eol (l) == FALSE)
  {
    /* LOOP THROUGH PRINT ELEMENTS */
    VariantType e;
    VariantType *E;

    E = &e;
    CLEAR_VARIANT (E);
    if (line_skip_CommaChar (l) /* comma-specific */ )
    {
      if (format_string[0])
      {
        /* PRINT USING active */
      }
      else if (IsCSV)
      {
        xputc1 (',');
      }
      else
      {
        /* tab over */
        next_zone ();
      }
      OutputCR = FALSE;
    }
    else if (line_skip_SemicolonChar (l) /* semicolon-specific */ )
    {
      if (format_string[0])
      {
        /* PRINT USING active */
      }
      else if (IsCSV)
      {
        xputc1 (',');
      }
      else
      {
        /* concatenate strings */
      }
      OutputCR = FALSE;
    }
    else if (line_read_using (l, format_string, format_length))
    {
      format_position = 0;
      OutputCR = TRUE;
    }
    else if (line_read_expression (l, E))        /* internal_print */
    {
      /* resolve the string */
      if (My->IsErrorPending /* Keep This */ )
      {
        /* 
         **
         ** this might look odd... 
         ** but we want to abort printing on the first warning.
         ** The expression list could include a function with side-effects,
         ** so any error should immediately halt further evaluation.
         **
         */
        RELEASE_VARIANT (E);
        return;
      }
      print_using_variant (format_string, &format_position, E, IsCSV);
      RELEASE_VARIANT (E);
      OutputCR = TRUE;
    }
    else
    {
      WARN_SYNTAX_ERROR;
      return;
    }
  }

  if (OutputCR == TRUE)
  {
    /* did not end with ',' or ';' */
    xputc1 ('\n');
  }
  if (My->CurrentFile == My->SYSOUT)
  {
    /* FOR I = 1 TO 1000: PRINT "."; : NEXT I : PRINT */
    fflush (My->SYSOUT->cfp);
  }
}


/***************************************************************
  
        FUNCTION:       print_using_variant()
  
   DESCRIPTION:    This function gets the PRINT USING
         format string, returning a structure
         to the format.
  
***************************************************************/
static void
print_using_number (char *buffer, int *position, VariantType * e)
{
  /*
     Format a NUMBER.
     'buffer' points to the beginning of a PRINT USING format string, such as "###.##".
     'position' is the current offset in 'buffer'.
     'e' is the current expression to print.
   */
  int width;
  int precision;
  int exponent;
  char HeadChar;
  char FillChar;
  char CurrChar;
  char ComaChar;
  char TailChar;
  int p;
  char *tbuf;
   
  assert (buffer != NULL);
  assert (position != NULL);
  assert (e != NULL);



  width = 0;
  precision = 0;
  exponent = 0;
  HeadChar = ' ';
  FillChar = ' ';
  CurrChar = ' ';
  ComaChar = ' ';
  TailChar = ' ';
  assert( My != NULL );
  assert( My->ConsoleInput != NULL );
  tbuf = My->ConsoleInput;


  p = *position;
  while (IS_CHAR (buffer[p], PrintUsingNumberPlus)
         || IS_CHAR (buffer[p], PrintUsingNumberMinus))
  {
    HeadChar = buffer[p];
    width++;
    p++;
  }
  while (IS_CHAR (buffer[p], PrintUsingNumberFiller)
         || IS_CHAR (buffer[p], PrintUsingNumberDollar))
  {
    if (IS_CHAR (buffer[p], PrintUsingNumberFiller))
    {
      FillChar = PrintUsingNumberFiller;
    }
    else if (IS_CHAR (buffer[p], PrintUsingNumberDollar))
    {
      CurrChar = PrintUsingNumberDollar;
    }
    width++;
    p++;
  }
  while (IS_CHAR (buffer[p], PrintUsingNumberDigit)
         || IS_CHAR (buffer[p], PrintUsingNumberComma))
  {
    if (IS_CHAR (buffer[p], PrintUsingNumberComma))
    {
      ComaChar = PrintUsingNumberComma;
    }
    width++;
    p++;
  }
  if (IS_CHAR (buffer[p], PrintUsingNumberPeriod))
  {
    while (IS_CHAR (buffer[p], PrintUsingNumberPeriod))
    {
      width++;
      p++;
    }
    while (IS_CHAR (buffer[p], PrintUsingNumberDigit))
    {
      precision++;
      width++;
      p++;
    }
  }
  while (IS_CHAR (buffer[p], PrintUsingNumberExponent))
  {
    exponent++;
    precision++;
    width++;
    p++;
  }
  while (IS_CHAR (buffer[p], PrintUsingNumberPlus)
         || IS_CHAR (buffer[p], PrintUsingNumberMinus))
  {
    TailChar = buffer[p];
    width++;
    p++;
  }
  /* format the number */


  /* displaying both a Heading and a Trailing sign is NOT supported */
  if (TailChar == ' ')
  {
    /* do nothing */
  }
  else
    if (IS_CHAR (TailChar, PrintUsingNumberPlus)
        || IS_CHAR (TailChar, PrintUsingNumberMinus))
  {
    /* force the sign to be printed, so we can move it */
    HeadChar = TailChar;
  }
  else
  {
    WARN_INTERNAL_ERROR;
    return;
  }


  if (HeadChar == ' ')
  {
    /* only display a '-' sign */
    if (exponent > 0)
    {
      sprintf (tbuf, "%*.*e", width, precision, e->Number);
    }
    else
    {
      sprintf (tbuf, "%*.*f", width, precision, e->Number);
    }
  }
  else
    if (IS_CHAR (HeadChar, PrintUsingNumberPlus)
        || IS_CHAR (HeadChar, PrintUsingNumberMinus))
  {
    /* force a leading sign '+' or '-' */
    if (exponent > 0)
    {
      sprintf (tbuf, "%+*.*e", width, precision, e->Number);
    }
    else
    {
      sprintf (tbuf, "%+*.*f", width, precision, e->Number);
    }
  }
  else
  {
    WARN_INTERNAL_ERROR;
    return;
  }

  if (TailChar == ' ')
  {
    /* do nothing */
  }
  else
    if (IS_CHAR (TailChar, PrintUsingNumberPlus)
        || IS_CHAR (TailChar, PrintUsingNumberMinus))
  {
    /* move sign '+' or '-' to end */
    int i;
    int n;

    n = bwb_strlen (tbuf);

    for (i = 0; i < n; i++)
    {
      if (tbuf[i] != ' ')
      {
        if (IS_CHAR (tbuf[i], PrintUsingNumberPlus))
        {
          tbuf[i] = ' ';
          if (IS_CHAR (TailChar, PrintUsingNumberPlus))
          {
            /* TailChar of '+' does print a '+' */
            bwb_strcat (tbuf, "+");
          }
          else if (IS_CHAR (TailChar, PrintUsingNumberMinus))
          {
            /* TailChar of '-' does NOT print a '+' */
            bwb_strcat (tbuf, " ");
          }
        }
        else if (IS_CHAR (tbuf[i], PrintUsingNumberMinus))
        {
          tbuf[i] = ' ';
          bwb_strcat (tbuf, "-");
        }
        break;
      }
    }
    if (tbuf[0] == ' ')
    {
      n = bwb_strlen (tbuf);
      /* n > 0 */
      for (i = 1; i < n; i++)
      {
        tbuf[i - 1] = tbuf[i];
      }
      tbuf[n - 1] = NulChar;
    }
  }
  else
  {
    WARN_INTERNAL_ERROR;
    return;
  }


  if (CurrChar == ' ')
  {
    /* do nothing */
  }
  else if (IS_CHAR (CurrChar, PrintUsingNumberDollar))
  {
    int i;
    int n;

    n = bwb_strlen (tbuf);

    for (i = 0; i < n; i++)
    {
      if (tbuf[i] != ' ')
      {
        if (i > 0)
        {
          if (bwb_isdigit (tbuf[i]))
          {
            tbuf[i - 1] = CurrChar;
          }
          else
          {
            /* sign char */
            tbuf[i - 1] = tbuf[i];
            tbuf[i] = CurrChar;
          }
        }
        break;
      }
    }
  }
  else
  {
    WARN_INTERNAL_ERROR;
    return;
  }

  if (FillChar == ' ')
  {
    /* do nothing */
  }
  else if (IS_CHAR (FillChar, PrintUsingNumberFiller))
  {
    int i;
    int n;

    n = bwb_strlen (tbuf);

    for (i = 0; i < n; i++)
    {
      if (tbuf[i] != ' ')
      {
        break;
      }
      tbuf[i] = PrintUsingNumberFiller;
    }
  }
  else
  {
    WARN_INTERNAL_ERROR;
    return;
  }

  if (ComaChar == ' ')
  {
    xputs (tbuf);
  }
  else if (IS_CHAR (ComaChar, PrintUsingNumberComma))
  {
    int dig_pos;
    int dec_pos;
    int i;
    int n;
    int commas;

    dig_pos = -1;
    dec_pos = -1;
    n = bwb_strlen (tbuf);

    for (i = 0; i < n; i++)
    {
      if ((bwb_isdigit (tbuf[i]) != 0) && (dig_pos == -1))
      {
        dig_pos = i;
      }
      if ((tbuf[i] == PrintUsingNumberPeriod) && (dec_pos == -1))
      {
        dec_pos = i;
      }
      if ((dig_pos != -1) && (dec_pos != -1))
      {
        break;
      }
    }
    if (dig_pos == -1)
    {
      dec_pos = n;
    }
    if (dec_pos == -1)
    {
      dec_pos = n;
    }
    /* count the number of commas */
    commas = 0;
    for (i = 0; i < n; i++)
    {
      if (((dec_pos - i) % 3 == 0) && (i > dig_pos) && (i < dec_pos))
      {
        commas++;
      }
    }
    /* now, actually print */
    for (i = 0; i < n; i++)
    {
      if (i < commas && tbuf[i] == FillChar)
      {
        /* 
           Ignore the same number of leading spaces as there are commas.
           While not perfect for all possible cases, 
           it is usually good enough for practical purposes.
         */
      }
      else
      {
        if (((dec_pos - i) % 3 == 0) && (i > dig_pos) && (i < dec_pos))
        {
          xputc1 (PrintUsingNumberComma);
        }
        xputc1 (tbuf[i]);
      }
    }
  }
  else
  {
    WARN_INTERNAL_ERROR;
    return;
  }
  *position = p;
}

static void
print_using_string (char *buffer, int *position, VariantType * e)
{
  /*
     Format a STRING.
     'buffer' points to the beginning of a PRINT USING format string, such as "###.##".
     'position' is the current offset in 'buffer'.
     'e' is the current expression to print.
   */
  int p;
  char *tbuf;
   
  assert (buffer != NULL);
  assert (position != NULL);
  assert (e != NULL);
  assert( My != NULL );
  assert( My->NumLenBuffer != NULL );

  p = *position;

  if (e->VariantTypeCode == StringTypeCode)
  {
    tbuf = e->Buffer;
  }
  else
  {
    tbuf = My->NumLenBuffer;
    FormatBasicNumber (e->Number, tbuf);
  }

  if (IS_CHAR (buffer[p], PrintUsingStringFirst))
  {
    /* print first character only */
    int i;

    i = 0;
    if (tbuf[i] == NulChar)
    {
      xputc1 (' ');
    }
    else
    {
      xputc1 (tbuf[i]);
      i++;
    }
    p++;
  }
  else if (IS_CHAR (buffer[p], PrintUsingStringAll))
  {
    /* print entire string */
    p++;
    xputs (tbuf);
  }
  else if (IS_CHAR (buffer[p], PrintUsingStringLength))
  {
    /* print N characters or spaces */
    int i;

    i = 0;
    if (tbuf[i] == NulChar)
    {
      xputc1 (' ');
    }
    else
    {
      xputc1 (tbuf[i]);
      i++;
    }
    p++;

    while (buffer[p] != NulChar && buffer[p] != PrintUsingStringLength)
    {
      if (tbuf[i] == NulChar)
      {
        xputc1 (' ');
      }
      else
      {
        xputc1 (tbuf[i]);
        i++;
      }
      p++;
    }
    if (IS_CHAR (buffer[p], PrintUsingStringLength))
    {
      if (tbuf[i] == NulChar)
      {
        xputc1 (' ');
      }
      else
      {
        xputc1 (tbuf[i]);
        i++;
      }
      p++;
    }
  }
  *position = p;
}

static int
is_magic_string (char *buffer)
{
  /* 
     for the character string pointed to 'buffer':
     return TRUE if it is a MagicString sequence,
     return FALSE otherwise.
   */
   
  assert (buffer != NULL);


  /* 1 character sequences */
  if (IS_CHAR (buffer[0], PrintUsingStringFirst))
  {
    /* "!" */
    return TRUE;
  }
  if (IS_CHAR (buffer[0], PrintUsingStringAll))
  {
    /* "&" */
    return TRUE;
  }
  if (IS_CHAR (buffer[0], PrintUsingStringLength))
  {
    /* "%...%" */
    return TRUE;
  }

  /* 2 character sequences */

  /* 3 character sequences */

  return FALSE;
}

static int
is_magic_number (char *buffer)
{
  /* 
     for the character string pointed to 'buffer':
     return TRUE if it is a MagicNumber sequence,
     return FALSE otherwise.
   */
   
  assert (buffer != NULL);

  /* 1 character sequences */
  if (IS_CHAR (buffer[0], PrintUsingNumberDigit))
  {
    /* "#" */
    return TRUE;
  }

  /* 2 character sequences */
  if (IS_CHAR (buffer[0], PrintUsingNumberFiller))
    if (IS_CHAR (buffer[1], PrintUsingNumberFiller))
    {
      /* "**" */
      return TRUE;
    }
  if (IS_CHAR (buffer[0], PrintUsingNumberDollar))
    if (IS_CHAR (buffer[1], PrintUsingNumberDollar))
    {
      /* "$$" */
      return TRUE;
    }

  if (IS_CHAR (buffer[0], PrintUsingNumberPlus))
    if (IS_CHAR (buffer[1], PrintUsingNumberDigit))
    {
      /* "+#" */
      return TRUE;
    }
  if (IS_CHAR (buffer[0], PrintUsingNumberMinus))
    if (IS_CHAR (buffer[1], PrintUsingNumberDigit))
    {
      /* "-#" */
      return TRUE;
    }

  /* 3 character sequences */
  if (IS_CHAR (buffer[0], PrintUsingNumberPlus))
    if (IS_CHAR (buffer[1], PrintUsingNumberFiller))
      if (IS_CHAR (buffer[2], PrintUsingNumberFiller))
      {
        /* "+**" */
        return TRUE;
      }
  if (IS_CHAR (buffer[0], PrintUsingNumberPlus))
    if (IS_CHAR (buffer[1], PrintUsingNumberDollar))
      if (IS_CHAR (buffer[2], PrintUsingNumberDollar))
      {
        /* "+$$" */
        return TRUE;
      }
  if (IS_CHAR (buffer[0], PrintUsingNumberMinus))
    if (IS_CHAR (buffer[1], PrintUsingNumberFiller))
      if (IS_CHAR (buffer[2], PrintUsingNumberFiller))
      {
        /* "-**" */
        return TRUE;
      }
  if (IS_CHAR (buffer[0], PrintUsingNumberMinus))
    if (IS_CHAR (buffer[1], PrintUsingNumberDollar))
      if (IS_CHAR (buffer[2], PrintUsingNumberDollar))
      {
        /* "-$$" */
        return TRUE;
      }

  return FALSE;
}

static void
print_using_variant (char *buffer, int *position, VariantType * e, int IsCSV)
{
  /*
     Format an EXPRESSION.
     'buffer' points to the beginning of a PRINT USING format string, such as "###.##".
     'position' is the current offset in 'buffer'.
     'e' is the current expression to print.
   */
  int IsUsed;
   
  assert (buffer != NULL);
  assert (position != NULL);
  assert (e != NULL);
  assert( My != NULL );
  assert( My->NumLenBuffer != NULL );

  /* PRINT A, B, C */
  /* PRINT USING "", A, B, C */
  /* PRINT USING "#", A, B, C */

  IsUsed = FALSE;
  if (buffer[0])
  {
    /* we have a format string */
    int p;
    p = *position;

    if (p > 0 && buffer[p] == NulChar)
    {
      /* recycle the format string */
      p = 0;
    }
    while (buffer[p])
    {
      if (is_magic_string (&buffer[p]))
      {
        if (IsUsed)
        {
          /* stop here, ready for next string value */
          break;
        }
        if (e->VariantTypeCode != StringTypeCode)
        {
          /* we are a number value, so we cannot match a magic string */
          break;
        }
        /* magic and value are both string */
        print_using_string (buffer, &p, e);
        IsUsed = TRUE;
      }
      else if (is_magic_number (&buffer[p]))
      {
        if (IsUsed)
        {
          /* stop here, ready for next number value */
          break;
        }
        if (e->VariantTypeCode == StringTypeCode)
        {
          /* we are a string value, so we cannot match a magic number */
          break;
        }
        /* magic and value are both number */
        print_using_number (buffer, &p, e);
        IsUsed = TRUE;
      }
      else if (IS_CHAR (buffer[p], PrintUsingLiteral))
      {
        /* print next character as literal */
        p++;
        if (buffer[p] == NulChar)
        {
          /* PRINT USING "_" */
          xputc1 (' ');
        }
        else
        {
          /* PRINT USING "_%" */
          xputc1 (buffer[p]);
          p++;
        }
      }
      else
      {
        /* print this character as literal */
        /* PRINT USING "A" */
        xputc1 (buffer[p]);
        p++;
      }
    }
    *position = p;
  }

  if (IsUsed == FALSE)
  {
    /* we did not actually print the vlue */
    if (e->VariantTypeCode == StringTypeCode)
    {
      /*
       **
       ** PRINT A$    
       ** PRINT USING "";A$    
       ** PRINT USING "ABC";A$ 
       **
       */
      if (IsCSV)
      {
        xputc1 ('\"');
        xputs (e->Buffer);
        xputc1 ('\"');
      }
      else
      {
        xputs (e->Buffer);
      }
    }
    else
    {
      /*
       **
       ** PRINT X     
       ** PRINT USING "";X     
       ** PRINT USING "ABC";X  
       **
       ** [space]number[space]   POSITIVE or ZERO
       ** [minus]number[space]   NEGATIVE 
       **
       **/
      char *tbuf;

      tbuf = My->NumLenBuffer;

      FormatBasicNumber (e->Number, tbuf);

      if (IsCSV)
      {
        char *P;
        P = tbuf;
        while (*P == ' ')
        {
          P++;
        }
        xputs (P);
      }
      else
      {
        xputs (tbuf);
        xputc1 (' ');
      }
    }
  }
}

/***************************************************************
  
        FUNCTION:       xputs()
  
   DESCRIPTION:    This function outputs a null-terminated
         string to a specified file or output
         device.
  
***************************************************************/

static void
xputs (char *buffer)
{
   
  assert (buffer != NULL);
  assert( My != NULL );  
  assert (My->CurrentFile != NULL);

  if (My->CurrentFile->width > 0)
  {
    /* check to see if the width will be exceeded */
    int n;
    n = My->CurrentFile->col + bwb_strlen (buffer) - 1;
    if (n > My->CurrentFile->width)
    {
      xputc1 ('\n');
    }
  }
  /* output the string */
  while (*buffer)
  {
    xputc1 (*buffer);
    buffer++;
  }
}


/***************************************************************
  
        FUNCTION:       next_zone()
  
   DESCRIPTION:    Advance to the next print zone.
  
***************************************************************/
static void
next_zone (void)
{
  assert( My != NULL );
  assert (My->CurrentFile != NULL);

  if (My->CurrentFile->width > 0)
  {
    /*
     **
     ** check to see if width will be exceeded 
     **
     */
    int LastZoneColumn;

    LastZoneColumn = 1;
    while (LastZoneColumn < My->CurrentFile->width)
    {
      LastZoneColumn += My->OptionZoneInteger;
    }
    LastZoneColumn -= My->OptionZoneInteger;

    if (My->CurrentFile->col >= LastZoneColumn)
    {
      /*
       **
       ** width will be exceeded, so advance to a new line
       **
       */
      xputc1 ('\n');
      return;
    }
  }
  /*
   **
   ** advance to the next print zone
   **
   */
  if ((My->CurrentFile->col % My->OptionZoneInteger) == 1)
  {
    xputc1 (' ');
  }
  while ((My->CurrentFile->col % My->OptionZoneInteger) != 1)
  {
    xputc1 (' ');
  }
}

/***************************************************************
  
        FUNCTION:       xputc1()
  
   DESCRIPTION:    This function outputs a character to a
         specified file or output device, checking
         to be sure the PRINT width is within
         the bounds specified for that device.
  
***************************************************************/

static void
xputc1 (char c)
{
  assert( My != NULL );
  assert (My->CurrentFile != NULL);

  if (My->CurrentFile->width > 0)
  {
    /*
     **
     ** check to see if width has been exceeded 
     **
     */
    if (c != '\n')
    {
      /*
       **
       ** REM this should print one line, not two lines 
       ** WIDTH 80
       ** PRINT SPACE$( 80 ) 
       **
       */
      if (My->CurrentFile->col > My->CurrentFile->width)
      {
        xputc2 ('\n');                /* output LF */
      }
    }
  }
  /*
   **
   ** output the character
   **
   */
  xputc2 (c);
}

/***************************************************************
  
   FUNCTION:       xputc2()
  
   DESCRIPTION:    This function sends a character to a
         specified file or output device.
  
***************************************************************/


static void
xputc2 (char c)
{
  assert( My != NULL );
  assert (My->CurrentFile != NULL);
  assert (My->CurrentFile->cfp != NULL);
  assert( My->CurrentVersion != NULL );

  if (c == '\n')
  {
    /*
     **
     ** CBASIC-II: RANDOM files are padded on the right with spaces 
     **
     */
    if (My->CurrentVersion->OptionVersionValue & (C77))
      if (My->CurrentFile->DevMode & DEVMODE_RANDOM)
        if (My->CurrentFile->width > 0)
        {
#if HAVE_MSDOS
          /* "\n" is converted to "\r\n" */
          while (My->CurrentFile->col < (My->CurrentFile->width - 1))
#else /*  ! HAVE_MSDOS */
          while (My->CurrentFile->col < My->CurrentFile->width)
#endif /* ! HAVE_MSDOS */
          {
            fputc (' ', My->CurrentFile->cfp);
            My->CurrentFile->col++;
          }
        }
    /*
     **
     ** output the character 
     **
     */
    fputc (c, My->CurrentFile->cfp);
    /*
     **
     ** NULLS 
     **
     */
    if (My->LPRINT_NULLS > 0)
      if (My->CurrentFile == My->SYSPRN)
        if (My->CurrentFile->width > 0)
        {
          int i;
          for (i = 0; i < My->LPRINT_NULLS; i++)
          {
            fputc (NulChar, My->SYSPRN->cfp);
          }
        }
    /*
     **
     ** update current column position 
     **
     */
    My->CurrentFile->col = 1;
    My->CurrentFile->row++;
    return;
  }
  /*
   **
   ** output the character 
   **
   */
  fputc (c, My->CurrentFile->cfp);
  /*
   **
   ** update current column position 
   **
   */
  My->CurrentFile->col++;
}


extern void
ResetConsoleColumn (void)
{
  assert( My != NULL );
  assert (My->SYSOUT != NULL);

  My->SYSOUT->col = 1;
}

static LineType *
S70_PUT (LineType * l)
{
  /* PUT filename$ , value [, ...] */
  VariantType e;
  VariantType *E;
   
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
  assert( My->NumLenBuffer != NULL );

  E = &e;
  CLEAR_VARIANT (E);
  if (line_read_expression (l, E) == FALSE)        /* bwb_PUT */
  {
    goto EXIT;
  }
  if (E->VariantTypeCode == StringTypeCode)
  {
    /* STRING */
    /* PUT filename$ ... */
    if (is_empty_string (E->Buffer))
    {
      /* PUT "" ... is an error */
      WARN_BAD_FILE_NAME;
      goto EXIT;
    }
    My->CurrentFile = find_file_by_name (E->Buffer);
    if (My->CurrentFile == NULL)
    {
      /* implicitly OPEN for writing */
      My->CurrentFile = file_new ();
      My->CurrentFile->cfp = fopen (E->Buffer, "w");
      if (My->CurrentFile->cfp == NULL)
      {
        WARN_BAD_FILE_NAME;
        goto EXIT;
      }
      My->CurrentFile->FileNumber = file_next_number ();
      My->CurrentFile->DevMode = DEVMODE_OUTPUT;
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
      My->CurrentFile->FileName = E->Buffer;
      E->Buffer = NULL;
    }
  }
  else
  {
    /* NUMBER -- file must already be OPEN */
    /* PUT filenumber ... */
    if (E->Number < 0)
    {
      /* "PUT # -1" is an error */
      WARN_BAD_FILE_NUMBER;
      goto EXIT;
    }
    if (E->Number == 0)
    {
      /* "PUT # 0" is an error */
      WARN_BAD_FILE_NUMBER;
      goto EXIT;
    }
    /* normal file */
    My->CurrentFile = find_file_by_number ((int) bwb_rint (E->Number));
    if (My->CurrentFile == NULL)
    {
      /* file not OPEN */
      WARN_BAD_FILE_NUMBER;
      goto EXIT;
    }
  }
  if (My->CurrentFile == NULL)
  {
    WARN_BAD_FILE_NUMBER;
    goto EXIT;
  }
  if ((My->CurrentFile->DevMode & DEVMODE_WRITE) == 0)
  {
    WARN_BAD_FILE_NUMBER;
    goto EXIT;
  }
  if (line_is_eol (l))
  {
    /* PUT F$ */
    /* PUT #1 */
    xputc1 ('\n');
    goto EXIT;
  }
  else if (line_skip_seperator (l))
  {
    /* OK */
  }
  else
  {
    WARN_SYNTAX_ERROR;
    goto EXIT;
  }

  /* loop through elements */

  while (line_is_eol (l) == FALSE)
  {
    while (line_skip_seperator (l))
    {
      /* PUT F$, ,,,A,,,B,,, */
      /* PUT #1, ,,,A,,,B,,, */
      xputc1 (My->CurrentFile->delimit);
    }

    if (line_is_eol (l) == FALSE)
    {
      /* print this item */

      CLEAR_VARIANT (E);
      if (line_read_expression (l, E) == FALSE)        /* bwb_PUT */
      {
        goto EXIT;
      }
      if (E->VariantTypeCode == StringTypeCode)
      {
        /* STRING */
        xputc1 (My->CurrentVersion->OptionQuoteChar);
        xputs (E->Buffer);
        xputc1 (My->CurrentVersion->OptionQuoteChar);
      }
      else
      {
        /* NUMBER */
        char *tbuf;

        tbuf = My->NumLenBuffer;
        FormatBasicNumber (E->Number, tbuf);
        xputs (tbuf);
      }
      RELEASE_VARIANT (E);
    }
  }
  /* print LF */
  xputc1 ('\n');
  /* OK */
EXIT:
  RELEASE_VARIANT (E);
  return (l);
}


static LineType *
D71_PUT (LineType * l)
{
  /* PUT # file_number [ , RECORD record_number ] */
  int file_number;
   
  assert (l != NULL);
  assert( My != NULL );

  file_number = 0;
  if (line_skip_FilenumChar (l))
  {
    /* OPTIONAL */
  }
  if (line_read_integer_expression (l, &file_number) == FALSE)
  {
    WARN_BAD_FILE_NUMBER;
    return (l);
  }
  if (file_number < 1)
  {
    WARN_BAD_FILE_NUMBER;
    return (l);
  }
  My->CurrentFile = find_file_by_number (file_number);
  if (My->CurrentFile == NULL)
  {
    WARN_BAD_FILE_NUMBER;
    return (l);
  }
  if (My->CurrentFile->DevMode != DEVMODE_RANDOM)
  {
    WARN_BAD_FILE_NUMBER;
    return (l);
  }
  if (My->CurrentFile->width <= 0)
  {
    WARN_BAD_FILE_NUMBER;
    return (l);
  }
  if (line_is_eol (l))
  {
    /* PUT # file_number */
  }
  else
  {
    /* PUT # file_number , RECORD record_number */
    int record_number;
    long offset;

    record_number = 0;
    offset = 0;
    if (line_skip_seperator (l) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    if (line_skip_word (l, "RECORD") == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    if (line_read_integer_expression (l, &record_number) == FALSE)
    {
      WARN_BAD_RECORD_NUMBER;
      return (l);
    }
    if (record_number <= 0)
    {
      WARN_BAD_RECORD_NUMBER;
      return (l);
    }
    record_number--;                /* BASIC to C */
    offset = record_number;
    offset *= My->CurrentFile->width;
    if (fseek (My->CurrentFile->cfp, offset, SEEK_SET) != 0)
    {
      WARN_BAD_RECORD_NUMBER;
      return (l);
    }
  }
  field_put (My->CurrentFile);
  /* if( TRUE ) */
  {
    int i;
    for (i = 0; i < My->CurrentFile->width; i++)
    {
      char c;
      c = My->CurrentFile->buffer[i];
      fputc (c, My->CurrentFile->cfp);
    }
  }
  /* OK */
  return (l);
}

static LineType *
H14_PUT (LineType * Line)
{
  /* PUT # FileNumber [ , RecordNumber ]                   ' RANDOM */
  /* PUT # FileNumber   , [ BytePosition ] , scalar [,...] ' BINARY */
  int file_number;
   
  assert (Line != NULL);
  assert( My != NULL );

  file_number = 0;
  if (line_skip_FilenumChar (Line))
  {
    /* OPTIONAL */
  }
  if (line_read_integer_expression (Line, &file_number) == FALSE)
  {
    WARN_BAD_FILE_NUMBER;
    return (Line);
  }
  if (file_number < 1)
  {
    WARN_BAD_FILE_NUMBER;
    return (Line);
  }
  My->CurrentFile = find_file_by_number (file_number);
  if (My->CurrentFile == NULL)
  {
    WARN_BAD_FILE_NUMBER;
    return (Line);
  }
  if (My->CurrentFile->DevMode == DEVMODE_RANDOM)
  {
    /* PUT # FileNumber [ , RecordNumber ]                   ' RANDOM */
    if (My->CurrentFile->width <= 0)
    {
      WARN_BAD_FILE_NUMBER;
      return (Line);
    }
    if (line_is_eol (Line))
    {
      /* PUT # file_number */
    }
    else
    {
      /* PUT # FileNumber , RecordNumber                   ' RANDOM */
      int record_number;
      long offset;

      record_number = 0;
      offset = 0;
      if (line_skip_seperator (Line) == FALSE)
      {
        WARN_SYNTAX_ERROR;
        return (Line);
      }
      if (line_read_integer_expression (Line, &record_number) == FALSE)
      {
        WARN_BAD_RECORD_NUMBER;
        return (Line);
      }
      if (record_number <= 0)
      {
        WARN_BAD_RECORD_NUMBER;
        return (Line);
      }
      record_number--;                /* BASIC to C */
      offset = record_number;
      offset *= My->CurrentFile->width;
      if (fseek (My->CurrentFile->cfp, offset, SEEK_SET) != 0)
      {
        WARN_BAD_RECORD_NUMBER;
        return (Line);
      }
    }
    field_put (My->CurrentFile);
    /* if( TRUE ) */
    {
      int i;
      for (i = 0; i < My->CurrentFile->width; i++)
      {
        char c;
        c = My->CurrentFile->buffer[i];
        fputc (c, My->CurrentFile->cfp);
      }
    }
    /* OK */
    return (Line);
  }
  else if (My->CurrentFile->DevMode == DEVMODE_BINARY)
  {
    /* PUT # FileNumber   , [ BytePosition ] , scalar [,...] ' BINARY */
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
      if (binary_get_put (v, TRUE) == FALSE)
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
bwb_PUT (LineType * Line)
{
   
  assert (Line != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );

  if (My->CurrentVersion->OptionVersionValue & (S70 | I70 | I73))
  {
    return S70_PUT (Line);
  }
  if (My->CurrentVersion->OptionVersionValue & (D71 | R86))
  {
    return D71_PUT (Line);
  }
  if (My->CurrentVersion->OptionVersionValue & (H14))
  {
    return H14_PUT (Line);
  }
  WARN_INTERNAL_ERROR;
  return (Line);
}


/***************************************************************
  
        FUNCTION:       bwb_write()
  
   DESCRIPTION:    This C function implements the BASIC WRITE
         command.
  
   SYNTAX:     WRITE [# device-number,] element [, element ]....
  
***************************************************************/


extern LineType *
bwb_WRITE (LineType * l)
{
  int IsCSV;
   
  assert (l != NULL);

  IsCSV = TRUE;
  assert( My != NULL );
  assert( My->SYSOUT != NULL );
  My->CurrentFile = My->SYSOUT;
  internal_print (l, IsCSV);
  return (l);
}

static LineType *
file_write_matrix (LineType * l, char delimit)
{
  /* MAT PRINT  [ # filenumber , ] matrix [;|,] ... */
  /* MAT WRITE  [ # filenumber , ] matrix [;|,] ... */
  /* MAT PUT    filename$      ,   matrix [;|,] ... */
  /* MAT PUT    filenumber     ,   matrix [;|,] ... */
  /* Array must be 1, 2 or 3 dimensions    */
  /* Array may be either NUMBER or STRING  */
   
  assert (l != NULL);

  do
  {
    VariableType *v;
    char ItemSeperator;

    /* get matrix name */
    if ((v = line_read_matrix (l)) == NULL)
    {
      WARN_SUBSCRIPT_OUT_OF_RANGE;
      return (l);
    }

    /* variable MUST be an array of 1, 2 or 3 dimensions */
    if (v->dimensions < 1)
    {
      WARN_SUBSCRIPT_OUT_OF_RANGE;
      return (l);
    }
    if (v->dimensions > 3)
    {
      WARN_SUBSCRIPT_OUT_OF_RANGE;
      return (l);
    }
    /*
     **
     ** This may look odd, but MAT PRINT is special.
     ** The variable seperator AFTER the variable determines how the variable's values are printed.
     ** The number of dimension determines:
     ** a) the meaning of comma (,) and semicolon (;)
     ** b) the default of row-by-row or col-by-col
     **
     */
    ItemSeperator = NulChar;        /* concatenate the columns */
    if (line_skip_CommaChar (l) /* comma-specific */ )
    {
      /*
       **
       ** force printing with the specified delimiter,
       ** which is usually a Comma but can be any character.
       **
       */
      ItemSeperator = delimit;        /* for MAT PRINT this is forced to be a ZoneChar */
    }
    else if (line_skip_SemicolonChar (l) /* semicolon-specific */ )
    {
      /*
       **
       ** force concatenating the columns,
       ** ignoring the specified delimiter.
       **
       */
      ItemSeperator = NulChar;
    }
    else
    {
      /*
       **
       ** default the item seperator based upon variable's dimensions
       **
       */
      switch (v->dimensions)
      {
      case 1:
        /* by default, a one dimension array is printed row-by-row */
        ItemSeperator = '\n';
        break;
      case 2:
        /* by default, a two dimension array is printed col-by-col */
        ItemSeperator = delimit;
        break;
      case 3:
        /* by default, a three dimension array is printed col-by-col */
        ItemSeperator = delimit;
        break;
      }
    }
    /* print array */
    switch (v->dimensions)
    {
    case 1:
      {
        /*
           OPTION BASE 0
           DIM A(5)
           ...
           MAT PRINT A 
           ...
           FOR I = 0 TO 5
           PRINT A(I)
           NEXT I
           ...
         */
        for (v->VINDEX[0] = v->LBOUND[0]; v->VINDEX[0] <= v->UBOUND[0];
             v->VINDEX[0]++)
        {
          VariantType variant;
          CLEAR_VARIANT (&variant);

          if (v->VINDEX[0] > v->LBOUND[0])
          {
            switch (ItemSeperator)
            {
            case NulChar:
              break;
            case ZoneChar:
              next_zone ();
              break;
            default:
              xputc1 (ItemSeperator);
            }
          }
          if (var_get (v, &variant) == FALSE)
          {
            WARN_VARIABLE_NOT_DECLARED;
            return (l);
          }
          if (variant.VariantTypeCode == StringTypeCode)
          {
            xputs (variant.Buffer);
          }
          else
          {
            char *tbuf;

            tbuf = My->NumLenBuffer;
            FormatBasicNumber (variant.Number, tbuf);
            xputs (tbuf);
          }
        }
        xputc1 ('\n');
      }
      break;
    case 2:
      {
        /*
           OPTION BASE 0
           DIM B(2,3)
           ...
           MAT PRINT B 
           ...
           FOR I = 0 TO 2
           FOR J = 0 TO 3
           PRINT B(I,J),
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
            VariantType variant;
            CLEAR_VARIANT (&variant);

            if (v->VINDEX[1] > v->LBOUND[1])
            {
              switch (ItemSeperator)
              {
              case NulChar:
                break;
              case ZoneChar:
                next_zone ();
                break;
              default:
                xputc1 (ItemSeperator);
              }
            }
            if (var_get (v, &variant) == FALSE)
            {
              WARN_VARIABLE_NOT_DECLARED;
              return (l);
            }
            if (variant.VariantTypeCode == StringTypeCode)
            {
              xputs (variant.Buffer);
            }
            else
            {
              char *tbuf;

              tbuf = My->NumLenBuffer;
              FormatBasicNumber (variant.Number, tbuf);
              xputs (tbuf);
            }
          }
          xputc1 ('\n');
        }
      }
      break;
    case 3:
      {
        /*
           OPTION BASE 0
           DIM C(2,3,4)
           ...
           MAT PRINT C 
           ...
           FOR I = 0 TO 2
           FOR J = 0 TO 3
           FOR K = 0 TO 4
           PRINT C(I,J,K),
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
              VariantType variant;
              CLEAR_VARIANT (&variant);

              if (v->VINDEX[2] > v->LBOUND[2])
              {
                switch (ItemSeperator)
                {
                case NulChar:
                  break;
                case ZoneChar:
                  next_zone ();
                  break;
                default:
                  xputc1 (ItemSeperator);
                }
              }
              if (var_get (v, &variant) == FALSE)
              {
                WARN_VARIABLE_NOT_DECLARED;
                return (l);
              }
              if (variant.VariantTypeCode == StringTypeCode)
              {
                xputs (variant.Buffer);
              }
              else
              {
                char *tbuf;

                tbuf = My->NumLenBuffer;
                FormatBasicNumber (variant.Number, tbuf);
                xputs (tbuf);
              }
            }
            xputc1 ('\n');
          }
          xputc1 ('\n');
        }
      }
      break;
    }
    /* process the next variable, if any  */
  }
  while (line_is_eol (l) == FALSE);
  return (l);
}

extern LineType *
bwb_MAT_PUT (LineType * l)
{
  /* MAT PUT filename$  , matrix [;|,] ... */
  /* MAT PUT filenumber , matrix [;|,] ... */
  /* Array must be 1, 2 or 3 dimensions    */
  /* Array may be either NUMBER or STRING  */
  VariantType x;
  VariantType *X;
   
  assert (l != NULL);
  assert( My != NULL );
  assert( My->SYSOUT != NULL );

  My->CurrentFile = My->SYSOUT;
  X = &x;
  CLEAR_VARIANT (X);
  if (line_read_expression (l, X) == FALSE)        /* bwb_MAT_PUT */
  {
    goto EXIT;
  }
  if (X->VariantTypeCode == StringTypeCode)
  {
    /* STRING */
    /* MAT PUT filename$ ... */
    if (is_empty_string (X->Buffer))
    {
      /* MAT PUT "" ... is an error */
      WARN_BAD_FILE_NAME;
      goto EXIT;
    }
    My->CurrentFile = find_file_by_name (X->Buffer);
    if (My->CurrentFile == NULL)
    {
      /* implicitly OPEN for writing */
      My->CurrentFile = file_new ();
      My->CurrentFile->cfp = fopen (X->Buffer, "w");
      if (My->CurrentFile->cfp == NULL)
      {
        WARN_BAD_FILE_NAME;
        goto EXIT;
      }
      My->CurrentFile->FileNumber = file_next_number ();
      My->CurrentFile->DevMode = DEVMODE_OUTPUT;
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
      My->CurrentFile->FileName = X->Buffer;
      X->Buffer = NULL;
    }
  }
  else
  {
    /* NUMBER -- file must already be OPEN */
    /* MAT PUT filenumber ... */
    if (X->Number < 0)
    {
      /* "MAT PUT # -1" is an error */
      WARN_BAD_FILE_NUMBER;
      goto EXIT;
    }
    if (X->Number == 0)
    {
      /* "MAT PUT # 0" is an error */
      WARN_BAD_FILE_NUMBER;
      goto EXIT;
    }
    /* normal file */
    My->CurrentFile = find_file_by_number ((int) bwb_rint (X->Number));
    if (My->CurrentFile == NULL)
    {
      /* file not OPEN */
      WARN_BAD_FILE_NUMBER;
      goto EXIT;
    }
  }
  RELEASE_VARIANT (X);
  if (My->CurrentFile == NULL)
  {
    WARN_BAD_FILE_NUMBER;
    goto EXIT;
  }
  if ((My->CurrentFile->DevMode & DEVMODE_WRITE) == 0)
  {
    WARN_BAD_FILE_NUMBER;
    goto EXIT;
  }
  if (line_skip_seperator (l))
  {
    /* OK */
  }
  else
  {
    WARN_SYNTAX_ERROR;
    goto EXIT;
  }
  return file_write_matrix (l, My->CurrentFile->delimit);
EXIT:
  RELEASE_VARIANT (X);
  return (l);
}

static LineType *
bwb_mat_dump (LineType * l, int IsWrite)
{
  /* MAT PRINT  [ # filenumber , ] matrix [;|,] ... */
  /* MAT WRITE  [ # filenumber , ] matrix [;|,] ... */
  /* Array must be 1, 2 or 3 dimensions    */
  /* Array may be either NUMBER or STRING  */
  char delimit;
   
  assert (l != NULL);
  assert( My != NULL );
  assert( My->SYSOUT != NULL );

  My->CurrentFile = My->SYSOUT;
  if (line_skip_FilenumChar (l))
  {
    /* ... # file, ... */
    if (parse_file_number (l) == FALSE)
    {
      return (l);
    }
    if (line_is_eol (l))
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
  }

  if (IsWrite)
  {
    /* MAT WRITE */
    delimit = My->CurrentFile->delimit;
  }
  else
  {
    /* MAT PRINT */
    delimit = ZoneChar;
  }
  return file_write_matrix (l, delimit);
}

extern LineType *
bwb_MAT_WRITE (LineType * l)
{
   
  assert (l != NULL);

  return bwb_mat_dump (l, TRUE);
}

extern LineType *
bwb_MAT_PRINT (LineType * l)
{
   
  assert (l != NULL);

  return bwb_mat_dump (l, FALSE);
}



/*  EOF  */
