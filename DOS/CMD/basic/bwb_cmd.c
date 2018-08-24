/***************************************************************
  
        bwb_cmd.c       Miscellaneous Commands
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

static void bwb_copy_file (char *Source, char *Target);
static LineType *bwb_delete (LineType * l);
static void bwb_display_file (char *Source);
static LineType *bwb_load (LineType * Line, char *Prompt, int IsNew);
static void bwb_new (void);
static LineType *bwb_run_filename_or_linenumber (LineType * L);
static LineType *bwb_save (LineType * Line, char *Prompt);
static LineType *bwb_system (LineType * l);
static LineType *bwb_xlist (LineType * l, FILE * file);
static LineType *bwx_run (LineType * Line, char *ProgramName);
static void CommandOptionVersion (int n, char *OutputLine);
static void CommandUniqueID (int i, char *UniqueID);
static void CommandVector (int i, char *Vector);
static VariableType *find_variable_by_type (char *name, int dimensions,
                                            char VariableTypeCode);
static void FixUp (char *Name);
static LineType *H14_RENAME (LineType * l);
static int line_read_matrix_redim (LineType * l, VariableType * v);
static void ProcessEscapeChars (const char *Input, char *Output);
static int xl_line (FILE * file, LineType * l);


/* 
   fprintf( file, "------------------------------------------------------------\n");
                   123456789012345678901234567890123456789012345678901234567890
   fprintf( file, "     SYNTAX: %s\n", IntrinsicCommandTable[n].Syntax);
   sprintf( tbuf, "DESCRIPTION: %s\n", IntrinsicCommandTable[n].Description);
   fprintf( file, "             " );
   fprintf( file, "             [%c] %s\n", X, bwb_vertable[i].Name);
                   1234567890123
*/
#define LEFT_LENGTH  13
#define RIGHT_LENGTH 47
#define TOTAL_LENGTH ( LEFT_LENGTH + RIGHT_LENGTH )

/* 
--------------------------------------------------------------------------------------------
                               EDIT, RENUM, RENUMBER
--------------------------------------------------------------------------------------------
*/

static LineType *
bwx_run (LineType * Line, char *ProgramName)
{
  size_t n;
  char *tbuf;
   
  assert (Line != NULL);
  assert( My != NULL );

  if (is_empty_string (ProgramName))
  {
    WARN_BAD_FILE_NAME;
    return (Line);
  }
  if (is_empty_string (My->ProgramFilename))
  {
    WARN_BAD_FILE_NAME;
    return (Line);
  }
  n = bwb_strlen (ProgramName) + 1 + bwb_strlen (My->ProgramFilename);
  if ((tbuf = (char *) calloc (n + 1 /* NulChar */ , sizeof (char))) == NULL)
  {
    WARN_OUT_OF_MEMORY;
    return (Line);
  }
  bwb_strcpy (tbuf, ProgramName);
  bwb_strcat (tbuf, " ");
  bwb_strcat (tbuf, My->ProgramFilename);
  system (tbuf);
  free (tbuf);
  tbuf = NULL;

  /* open edited file for read */
  bwb_NEW (Line);                /* Relocated by JBV (bug found by DD) */
  if (bwb_fload (NULL) == FALSE)
  {
    WARN_BAD_FILE_NAME;
    return (Line);
  }
  return (Line);
}



/***************************************************************
  
      FUNCTION:       bwb_edit()
  
   DESCRIPTION:    This function implements the BASIC EDIT
         program by shelling out to a default editor
         specified by the variable BWB.EDITOR$.
  
   SYNTAX:     EDIT
  
***************************************************************/

LineType *
bwb_EDIT (LineType * Line)
{
  /* 
     SYNTAX:     EDIT
   */
   
  assert (Line != NULL);
  assert( My != NULL );

  return bwx_run (Line, My->OptionEditString);
}

/***************************************************************
  
        FUNCTION:       bwb_renum()
  
   DESCRIPTION:    This function implements the BASIC RENUM
         command by shelling out to a default
         renumbering program called "renum".
         Added by JBV 10/95
  
   SYNTAX:     RENUM
  
***************************************************************/

LineType *
bwb_RENUM (LineType * Line)
{
  /* 
     SYNTAX:     RENUM
   */
   
  assert (Line != NULL);
  assert( My != NULL );

  return bwx_run (Line, My->OptionRenumString);
}

LineType *
bwb_RENUMBER (LineType * Line)
{
  /* 
     SYNTAX:     RENUMBER
   */
   
  assert (Line != NULL);
  assert( My != NULL );

  return bwx_run (Line, My->OptionRenumString);
}

/* 
--------------------------------------------------------------------------------------------
                               REM
--------------------------------------------------------------------------------------------
*/

LineType *
bwb_REM (LineType * L)
{
  /* 
     SYNTAX:     REM comment
   */
  /*
     This line holds BASIC comments.
   */
   
  assert (L != NULL);

  line_skip_eol (L);
  return L;
}

/* 
--------------------------------------------------------------------------------------------
                               IMAGE
--------------------------------------------------------------------------------------------
*/

LineType *
bwb_IMAGE (LineType * L)
{
  /* 
     SYNTAX:     IMAGE print-using-format
   */
   
  assert (L != NULL);

  line_skip_eol (L);
  return L;
}

/* 
--------------------------------------------------------------------------------------------
                               LET
--------------------------------------------------------------------------------------------
*/


LineType *
bwb_LET (LineType * L)
{
  /*
     SYNTAX:     LET variable [,...] = expression
   */
  VariableType *v;
  VariantType x;
  VariantType *X;
   
  assert (L != NULL);
  X = &x;
  CLEAR_VARIANT (X);
  /* read the list of variables */
  do
  {
    if ((v = line_read_scalar (L)) == NULL)
    {
      WARN_VARIABLE_NOT_DECLARED;
      goto EXIT;
    }
  }
  while (line_skip_seperator (L));

  /* skip the equal sign */
  if (line_skip_EqualChar (L))
  {
    /* OK */
  }
  else if (line_skip_word (L, "EQ"))
  {
    /* OK */
  }
  else if (line_skip_word (L, ".EQ."))
  {
    /* OK */
  }
  else
  {
    WARN_SYNTAX_ERROR;
    goto EXIT;
  }

  /* evaluate the expression */
  if (line_read_expression (L, X))        /* bwb_LET */
  {
    /* save the value */
    if (line_is_eol (L) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      goto EXIT;
    }
    L->position = L->Startpos;

    /* for each variable, assign the value */
    do
    {
      /* read a variable */
      if ((v = line_read_scalar (L)) == NULL)
      {
        WARN_VARIABLE_NOT_DECLARED;
        goto EXIT;
      }
      assert (v != NULL);
      assert (X != NULL);
      if (var_set (v, X) == FALSE)
      {
        WARN_TYPE_MISMATCH;
        goto EXIT;
      }
    }
    while (line_skip_seperator (L));

    /* we are now at the equals sign */
    line_skip_eol (L);
  }
  else
  {
    WARN_SYNTAX_ERROR;
  }
EXIT:
  RELEASE_VARIANT (X);
  return L;
}


LineType *
bwb_CONST (LineType * L)
{
  /*
     SYNTAX:     CONST variable [,...] = expression
   */
  VariableType *v;
  VariantType x;
  VariantType *X;
   
  assert (L != NULL);

  X = &x;
  CLEAR_VARIANT (X);
  /* read the list of variables */
  do
  {
    if ((v = line_read_scalar (L)) == NULL)
    {
      WARN_VARIABLE_NOT_DECLARED;
      goto EXIT;
    }
  }
  while (line_skip_seperator (L));
  /* we are now at the equals sign */

  /* skip the equal sign */
  if (line_skip_EqualChar (L))
  {
    /* OK */
  }
  else if (line_skip_word (L, "EQ"))
  {
    /* OK */
  }
  else if (line_skip_word (L, ".EQ."))
  {
    /* OK */
  }
  else
  {
    WARN_SYNTAX_ERROR;
    goto EXIT;
  }

  /* evaluate the expression */
  if (line_read_expression (L, X))        /* bwb_LET */
  {
    /* save the value */
    if (line_is_eol (L) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      goto EXIT;
    }

    /* for each variable, assign the value */
    L->position = L->Startpos;
    do
    {
      /* read a variable */
      if ((v = line_read_scalar (L)) == NULL)
      {
        WARN_VARIABLE_NOT_DECLARED;
        goto EXIT;
      }
      assert (v != NULL);
      assert (X != NULL);
      if (var_set (v, X) == FALSE)
      {
        WARN_TYPE_MISMATCH;
        goto EXIT;
      }
    }
    while (line_skip_seperator (L));
    /* we are now at the equals sign */

    /* for each variable, mark as constant */
    L->position = L->Startpos;
    do
    {
      /* read a variable */
      if ((v = line_read_scalar (L)) == NULL)
      {
        WARN_VARIABLE_NOT_DECLARED;
        goto EXIT;
      }
      assert (v != NULL);
      v->VariableFlags |= VARIABLE_CONSTANT;
    }
    while (line_skip_seperator (L));
    /* we are now at the equals sign */

    line_skip_eol (L);
  }
  else
  {
    WARN_SYNTAX_ERROR;
  }
EXIT:
  RELEASE_VARIANT (X);
  return L;
}

LineType *
bwb_DEC (LineType * L)
{
  /*
     SYNTAX:     DEC variable [,...]
   */
  VariableType *v;
  VariantType x;
  VariantType *X;
   
  assert (L != NULL);

  X = &x;
  CLEAR_VARIANT (X);
  /* read the list of variables */
  do
  {
    if ((v = line_read_scalar (L)) == NULL)
    {
      WARN_VARIABLE_NOT_DECLARED;
      goto EXIT;
    }
    if (v->VariableTypeCode == StringTypeCode)
    {
      WARN_TYPE_MISMATCH;
      goto EXIT;
    }
  }
  while (line_skip_seperator (L));
  /* we are now at the end of the line */

  if (line_is_eol (L) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    goto EXIT;
  }
  L->position = L->Startpos;

  /* for each variable, assign the value */
  do
  {
    /* read a variable */
    if ((v = line_read_scalar (L)) == NULL)
    {
      WARN_VARIABLE_NOT_DECLARED;
      goto EXIT;
    }
    assert (v != NULL);
    assert (X != NULL);
    if (var_get (v, X) == FALSE)
    {
      WARN_VARIABLE_NOT_DECLARED;
      goto EXIT;
    }
    X->Number--;
    if (var_set (v, X) == FALSE)
    {
      WARN_VARIABLE_NOT_DECLARED;
      goto EXIT;
    }
  }
  while (line_skip_seperator (L));
  /* we are now at the end of the line */
EXIT:
  RELEASE_VARIANT (X);
  return L;
}

LineType *
bwb_INC (LineType * L)
{
  /*
     SYNTAX:     INC variable [,...]
   */
  VariableType *v;
  VariantType x;
  VariantType *X;
   
  assert (L != NULL);

  X = &x;
  CLEAR_VARIANT (X);
  /* read the list of variables */
  do
  {
    if ((v = line_read_scalar (L)) == NULL)
    {
      WARN_VARIABLE_NOT_DECLARED;
      goto EXIT;
    }
    if (v->VariableTypeCode == StringTypeCode)
    {
      WARN_TYPE_MISMATCH;
      goto EXIT;
    }
  }
  while (line_skip_seperator (L));
  /* we are now at the end of the line */

  if (line_is_eol (L) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    goto EXIT;
  }
  L->position = L->Startpos;

  /* for each variable, assign the value */
  do
  {
    /* read a variable */
    if ((v = line_read_scalar (L)) == NULL)
    {
      WARN_VARIABLE_NOT_DECLARED;
      goto EXIT;
    }
    assert (v != NULL);
    assert (X != NULL);
    if (var_get (v, X) == FALSE)
    {
      WARN_VARIABLE_NOT_DECLARED;
      goto EXIT;
    }
    X->Number++;
    if (var_set (v, X) == FALSE)
    {
      WARN_VARIABLE_NOT_DECLARED;
      goto EXIT;
    }
  }
  while (line_skip_seperator (L));
  /* we are now at the end of the line */
EXIT:
  RELEASE_VARIANT (X);
  return L;
}



/* 
--------------------------------------------------------------------------------------------
                               GO
--------------------------------------------------------------------------------------------
*/

LineType *
bwb_GO (LineType * L)
{
   
  assert (L != NULL);
  WARN_SYNTAX_ERROR;
  return L;
}

LineType *
bwb_THEN (LineType * L)
{
   
  assert (L != NULL);
  WARN_SYNTAX_ERROR;
  return L;
}

LineType *
bwb_TO (LineType * L)
{
   
  assert (L != NULL);
  WARN_SYNTAX_ERROR;
  return L;
}

LineType *
bwb_STEP (LineType * L)
{
   
  assert (L != NULL);
  WARN_SYNTAX_ERROR;
  return L;
}

LineType *
bwb_OF (LineType * L)
{
   
  assert (L != NULL);
  WARN_SYNTAX_ERROR;
  return L;
}

LineType *
bwb_AS (LineType * L)
{
   
  assert (L != NULL);
  WARN_SYNTAX_ERROR;
  return L;
}


/* 
--------------------------------------------------------------------------------------------
                               AUTO
--------------------------------------------------------------------------------------------
*/


LineType *
bwb_BUILD (LineType * L)
{
  /* 
     SYNTAX:     BUILD 
     SYNTAX:     BUILD start
     SYNTAX:     BUILD start, increment 
   */
   
  assert (L != NULL);
  return bwb_AUTO (L);
}

LineType *
bwb_AUTO (LineType * L)
{
  /*
     SYNTAX:     AUTO
     SYNTAX:     AUTO start
     SYNTAX:     AUTO start , increment
   */
   
  assert (L != NULL);
  assert( My != NULL );

  My->AutomaticLineNumber = 0;
  My->AutomaticLineIncrement = 0;

  if (line_is_eol (L))
  {
    /* AUTO                   */
    My->AutomaticLineNumber = 10;
    My->AutomaticLineIncrement = 10;
    return L;
  }
  if (line_read_line_number (L, &My->AutomaticLineNumber))
  {
    /* AUTO ### ... */
    if (My->AutomaticLineNumber < MINLIN || My->AutomaticLineNumber > MAXLIN)
    {
      WARN_UNDEFINED_LINE;
      return L;
    }
    if (line_is_eol (L))
    {
      /* AUTO start             */
      My->AutomaticLineIncrement = 10;
      return L;
    }
    else if (line_skip_seperator (L))
    {
      /* AUTO ### ,  ... */
      if (line_read_line_number (L, &My->AutomaticLineIncrement))
      {
        /* AUTO start , increment */
        if (My->AutomaticLineIncrement < MINLIN
            || My->AutomaticLineIncrement > MAXLIN)
        {
          WARN_UNDEFINED_LINE;
          return L;
        }
        return L;
      }
    }
  }
  My->AutomaticLineNumber = 0;
  My->AutomaticLineIncrement = 0;
  WARN_SYNTAX_ERROR;
  return L;
}

/* 
--------------------------------------------------------------------------------------------
                               BREAK
--------------------------------------------------------------------------------------------
*/


LineType *
bwb_BREAK (LineType * l)
{
  /*
     SYNTAX:     BREAK
     SYNTAX:     BREAK line [,...]
     SYNTAX:     BREAK line - line
   */
   
  assert (l != NULL);
  assert( My != NULL );
  assert( My->StartMarker != NULL );
  assert( My->EndMarker != NULL );

  if (line_is_eol (l))
  {
    /* BREAK */
    /* remove all line breaks */
    LineType *x;
    for (x = My->StartMarker->next; x != My->EndMarker; x = x->next)
    {
      x->LineFlags &= ~LINE_BREAK;
    }
    return (l);
  }
  else
  {
    do
    {
      int head;
      int tail;

      if (line_read_line_sequence (l, &head, &tail))
      {
        /* BREAK 's' - 'e' */
        LineType *x;
        if (head < MINLIN || head > MAXLIN)
        {
          WARN_UNDEFINED_LINE;
          return (l);
        }
        if (tail < MINLIN || tail > MAXLIN)
        {
          WARN_UNDEFINED_LINE;
          return (l);
        }
        if (head > tail)
        {
          WARN_SYNTAX_ERROR;
          return (l);
        }
        /* valid range */
        /* now go through and list appropriate lines */
        for (x = My->StartMarker->next; x != My->EndMarker; x = x->next)
        {
          if (head <= x->number && x->number <= tail)
          {
            if (x->LineFlags & LINE_NUMBERED)
            {
              x->LineFlags |= LINE_BREAK;
            }
          }
        }
      }
      else
      {
        WARN_SYNTAX_ERROR;
        return (l);
      }
    }
    while (line_skip_seperator (l));
  }
  return (l);
}


/* 
--------------------------------------------------------------------------------------------
                               DSP
--------------------------------------------------------------------------------------------
*/

LineType *
bwb_DSP (LineType * l)
{
  /*
     SYNTAX:     DSP
     SYNTAX:     DSP variablename [,...]
   */
  VariableType *v;
   
  assert (l != NULL);
  assert( My != NULL );


  if (line_is_eol (l))
  {
    /* DSP */
    /* remove all variable displays */
    for (v = My->VariableHead; v != NULL; v = v->next)
    {
      v->VariableFlags &= ~VARIABLE_DISPLAY;        /* bwb_DSP() */
    }
    return (l);
  }
  /* DSP variablename [,...] */
  do
  {
    char varname[NameLengthMax + 1];

    if (line_read_varname (l, varname))
    {
      /* mark the variable */
      for (v = My->VariableHead; v != NULL; v = v->next)
      {
        if (bwb_stricmp (v->name, varname) == 0)
        {
          v->VariableFlags |= VARIABLE_DISPLAY;        /* bwb_DSP() */
        }
      }
    }
  }
  while (line_skip_seperator (l));
  return (l);
}


/* 
--------------------------------------------------------------------------------------------
                               GOTO
--------------------------------------------------------------------------------------------
*/

LineType *
bwb_GO_TO (LineType * l)
{
   
  assert (l != NULL);
  return bwb_GOTO (l);
}

LineType *
bwb_GOTO (LineType * l)
{
  /*
     SYNTAX:     GOTO line                      ' standard GOTO
     SYNTAX:     GOTO expression                ' calculated GOTO
     SYNTAX:     GOTO expression OF line,...    ' indexed GOTO, same as ON expression GOTO line,...
     SYNTAX:     GOTO line [,...] ON expression ' indexed GOTO, same as ON expression GOTO line,...
   */
  int Value;
  int LineNumber;
  LineType *x;
   
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );

  Value = 0;
  LineNumber = 0;
  if (l->LineFlags & (LINE_USER))
  {
    WARN_ILLEGAL_DIRECT;
    return (l);
  }

  if (line_is_eol (l))
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  if (line_read_integer_expression (l, &Value) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }

  if (line_is_eol (l))
  {
    /* GOTO linenumber */
    /* 'Value' is the line number */
    LineNumber = Value;
  }
  else if (line_skip_word (l, "OF"))
  {
    /* GOTO expression OF line, ... */
    /* 'Value' is an index into a list of line numbers */
    if (line_read_index_item (l, Value, &LineNumber))
    {
      /* found 'LineNumber' */
    }
    else if (My->CurrentVersion->OptionFlags & (OPTION_BUGS_ON) ) /* GOTO X OF ... */
    {
      /* silently fall-thru to the following line */
      line_skip_eol (l);
      return (l);
    }
    else
    {
      /* ERROR */
      WARN_UNDEFINED_LINE;
      return (l);
    }
  }
  else if (My->CurrentVersion->OptionVersionValue & (S70 | I70 | I73))
  {
    /* GOTO line [,...] ON expression */
    while (line_skip_seperator (l))
    {
      if (line_read_integer_expression (l, &Value) == FALSE)
      {
        WARN_SYNTAX_ERROR;
        return (l);
      }
    }
    if (line_skip_word (l, "ON") == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    if (line_read_integer_expression (l, &Value) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    /* 'Value' is an index into a list of line numbers */
    l->position = l->Startpos;
    if (line_read_index_item (l, Value, &LineNumber))
    {
      /* found 'LineNumber' */
    }
    else
    {
      /* silently fall-thru to the following line */
      line_skip_eol (l);
      return (l);
    }
    line_skip_eol (l);
  }
  else
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }

  if (LineNumber < MINLIN || LineNumber > MAXLIN)
  {
    WARN_UNDEFINED_LINE;
    return (l);
  }
  /* valid range */
  x = NULL;
#if THE_PRICE_IS_RIGHT
  if (l->OtherLine != NULL)
  {
    /* look in the cache */
    if (l->OtherLine->number == LineNumber)
    {
      x = l->OtherLine;                /* found in cache */
    }
  }
#endif /* THE_PRICE_IS_RIGHT */
  if (x == NULL)
  {
    x = find_line_number (LineNumber);        /* not found in the cache */
  }
  if (x != NULL)
  {
    /* FOUND */
    line_skip_eol (l);
    x->position = 0;
#if THE_PRICE_IS_RIGHT
    l->OtherLine = x;                /* save in cache */
#endif /* THE_PRICE_IS_RIGHT */
    return x;
  }
  /* NOT FOUND */
  WARN_UNDEFINED_LINE;
  return (l);
}


/* 
--------------------------------------------------------------------------------------------
                               GOSUB
--------------------------------------------------------------------------------------------
*/

LineType *
bwb_GO_SUB (LineType * l)
{
   
  assert (l != NULL);
  return bwb_GOSUB (l);
}

LineType *
bwb_GOSUB (LineType * l)
{
  /*
     SYNTAX:     GOSUB line                      ' standard GOSUB
     SYNTAX:     GOSUB expression                ' calculated GOSUB
     SYNTAX:     GOSUB expression OF line,...    ' indexed GOSUB, same as ON expression GOSUB line,...
     SYNTAX:     GOSUB line [,...] ON expression ' indexed GOSUB, same as ON expression GOSUB line,...
   */
  int Value;
  int LineNumber;
  LineType *x;
   
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );

  Value = 0;
  LineNumber = 0;
  x = NULL;
  if (l->LineFlags & (LINE_USER))
  {
    WARN_ILLEGAL_DIRECT;
    return (l);
  }

  if (line_is_eol (l))
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  if (line_read_integer_expression (l, &Value) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }

  if (line_is_eol (l))
  {
    /* GOSUB linenumber */
    /* 'Value' is the line number */
    LineNumber = Value;
  }
  else if (line_skip_word (l, "OF"))
  {
    /* GOSUB linenumber [,...] OF expression */
    /* 'Value' is an index into a list of line numbers */
    if (line_read_index_item (l, Value, &LineNumber))
    {
      /* found 'LineNumber' */
    }
    else if (My->CurrentVersion->OptionFlags & (OPTION_BUGS_ON) ) /* GOSUB X OF ... */ 
    {
      /* silently fall-thru to the following line */
      line_skip_eol (l);
      return (l);
    }
    else
    {
      /* ERROR */
      WARN_UNDEFINED_LINE;
      return (l);
    }
  }
  else if (My->CurrentVersion->OptionVersionValue & (S70 | I70 | I73))
  {
    /* GOSUB line [,...] ON expression */
    while (line_skip_seperator (l))
    {
      if (line_read_integer_expression (l, &Value) == FALSE)
      {
        WARN_SYNTAX_ERROR;
        return (l);
      }
    }
    if (line_skip_word (l, "ON") == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    if (line_read_integer_expression (l, &Value) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    /* 'Value' is an index into a list of line numbers */
    l->position = l->Startpos;
    if (line_read_index_item (l, Value, &LineNumber))
    {
      /* found 'LineNumber' */
    }
    else
    {
      /* silently fall-thru to the following line */
      line_skip_eol (l);
      return (l);
    }
    line_skip_eol (l);
  }
  else
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }

  if (LineNumber < MINLIN || LineNumber > MAXLIN)
  {
    WARN_UNDEFINED_LINE;
    return (l);
  }
  /* valid range */
  x = NULL;
#if THE_PRICE_IS_RIGHT
  if (l->OtherLine != NULL)
  {
    /* look in the cache */
    if (l->OtherLine->number == LineNumber)
    {
      x = l->OtherLine;                /* found in cache */
    }
  }
#endif /* THE_PRICE_IS_RIGHT */
  if (x == NULL)
  {
    x = find_line_number (LineNumber);        /* not found in the cache */
  }
  if (x != NULL)
  {
    /* FOUND */
    line_skip_eol (l);
    /* save current stack level */
    My->StackHead->line = l;
    /* increment exec stack */
    if (bwb_incexec ())
    {
      /* set the new position to x and return x */
      x->position = 0;
      My->StackHead->line = x;
      My->StackHead->ExecCode = EXEC_GOSUB;
#if THE_PRICE_IS_RIGHT
      l->OtherLine = x;                /* save in cache */
#endif /* THE_PRICE_IS_RIGHT */
      return x;
    }
    else
    {
      /* ERROR */
      WARN_OUT_OF_MEMORY;
      return My->EndMarker;
    }
  }
  /* NOT FOUND */
  WARN_UNDEFINED_LINE;
  return (l);
}



/* 
--------------------------------------------------------------------------------------------
                               RETURN
--------------------------------------------------------------------------------------------
*/

LineType *
bwb_RETURN (LineType * l)
{
  /*
     SYNTAX:     RETURN
   */
   
  assert (l != NULL);
  assert (My != NULL);
  assert (My->CurrentVersion != NULL);
  assert (My->StackHead != NULL);

  if (My->CurrentVersion->OptionVersionValue & (S70 | I70 | I73))
  {
    /* RETURN [comment] */
    line_skip_eol (l);
  }

  if (My->CurrentVersion->OptionVersionValue & (C77))
  {
    /* CBASIC-II: RETURN exits the first FUNCTION or GOSUB */

    while (My->StackHead->ExecCode != EXEC_GOSUB
           && My->StackHead->ExecCode != EXEC_FUNCTION)
    {
      bwb_decexec ();
      if (My->StackHead == NULL)
      {
        WARN_RETURN_WITHOUT_GOSUB;
        return (l);
      }
      if (My->StackHead->ExecCode == EXEC_NORM)        /* End of the line? */
      {
        WARN_RETURN_WITHOUT_GOSUB;
        return (l);
      }
    }
  }
  else
  {
    /* RETURN exits the first GOSUB */

    while (My->StackHead->ExecCode != EXEC_GOSUB)
    {
      bwb_decexec ();
      if (My->StackHead == NULL)
      {
        WARN_RETURN_WITHOUT_GOSUB;
        return (l);
      }
      if (My->StackHead->ExecCode == EXEC_NORM)        /* End of the line? */
      {
        WARN_RETURN_WITHOUT_GOSUB;
        return (l);
      }
    }
  }


  /* decrement the EXEC stack counter */

  bwb_decexec ();
  assert (My->StackHead != NULL);
  return My->StackHead->line;
}

/* 
--------------------------------------------------------------------------------------------
                               POP
--------------------------------------------------------------------------------------------
*/


LineType *
bwb_POP (LineType * l)
{
  /*
     SYNTAX:     POP
   */
  StackType *StackItem;
   
  assert (l != NULL);
  assert (My != NULL);
  assert (My->CurrentVersion != NULL);
  assert (My->StackHead != NULL);

  StackItem = My->StackHead;
  while (StackItem->ExecCode != EXEC_GOSUB)
  {
    StackItem = StackItem->next;
    if (StackItem == NULL)
    {
      WARN_RETURN_WITHOUT_GOSUB;
      return (l);
    }
    if (StackItem->ExecCode == EXEC_NORM)
    {
      /* End of the line */
      WARN_RETURN_WITHOUT_GOSUB;
      return (l);
    }
  }
  /* hide the GOSUB */
  StackItem->ExecCode = EXEC_POPPED;
  return (l);
}

/* 
--------------------------------------------------------------------------------------------
                               ON
--------------------------------------------------------------------------------------------
*/

LineType *
bwb_ON (LineType * l)
{
  /*
     SYNTAX:     ON expression GOTO  line,...    ' expression evaluates to an index
     SYNTAX:     ON expression GOSUB line,...    ' expression evaluates to an index
   */
  int Value;
  int command;
  int LineNumber;
  LineType *x;
   
  assert (l != NULL);
  assert (My != NULL);
  assert (My->CurrentVersion != NULL);

  Value = 0;
  command = 0;
  LineNumber = 0;
  x = NULL;
  if (l->LineFlags & (LINE_USER))
  {
    WARN_ILLEGAL_DIRECT;
    return (l);
  }

  if (line_is_eol (l))
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  if (line_read_integer_expression (l, &Value) == FALSE)
  {
    WARN_UNDEFINED_LINE;
    return (l);
  }
  if (line_skip_word (l, "GO"))
  {
    if (line_skip_word (l, "TO"))
    {
      command = C_GOTO;
    }
    else if (line_skip_word (l, "SUB"))
    {
      command = C_GOSUB;
    }
    else
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
  }
  else if (line_skip_word (l, "GOTO"))
  {
    command = C_GOTO;
  }
  else if (line_skip_word (l, "GOSUB"))
  {
    command = C_GOSUB;
  }
  else
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }

  /* 'Value' is an index into a list of line numbers */
  if (line_read_index_item (l, Value, &LineNumber))
  {
    /* found 'LineNumber' */
  }
  else if (My->CurrentVersion->OptionFlags & (OPTION_BUGS_ON) ) /* ON X GOTO|GOSUB ... */
  {
    /* silently fall-thru to the following line */
    line_skip_eol (l);
    return (l);
  }
  else
  {
    /* ERROR */
    WARN_UNDEFINED_LINE;
    return (l);
  }

  if (LineNumber < MINLIN || LineNumber > MAXLIN)
  {
    WARN_UNDEFINED_LINE;
    return (l);
  }
  /* valid range */
  x = NULL;
#if THE_PRICE_IS_RIGHT
  if (l->OtherLine != NULL)
  {
    /* look in the cache */
    if (l->OtherLine->number == LineNumber)
    {
      x = l->OtherLine;                /* found in cache */
    }
  }
#endif /* THE_PRICE_IS_RIGHT */
  if (x == NULL)
  {
    x = find_line_number (LineNumber);        /* not found in the cache */
  }
  if (x != NULL)
  {
    /* FOUND */
    if (command == C_GOTO)
    {
      /* ON ... GOTO ... */
      line_skip_eol (l);
      x->position = 0;
#if THE_PRICE_IS_RIGHT
      l->OtherLine = x;                /* save in cache */
#endif /* THE_PRICE_IS_RIGHT */
      return x;
    }
    else if (command == C_GOSUB)
    {
      /* ON ... GOSUB ... */
      line_skip_eol (l);
      /* save current stack level */
      My->StackHead->line = l;
      /* increment exec stack */
      if (bwb_incexec ())
      {
        /* set the new position to x and return x */
        x->position = 0;
        My->StackHead->line = x;
        My->StackHead->ExecCode = EXEC_GOSUB;
#if THE_PRICE_IS_RIGHT
        l->OtherLine = x;        /* save in cache */
#endif /* THE_PRICE_IS_RIGHT */
        return x;
      }
      else
      {
        /* ERROR */
        WARN_OUT_OF_MEMORY;
        return My->EndMarker;
      }
    }
    else
    {
      /* ERROR */
      WARN_SYNTAX_ERROR;
      return (l);
    }
  }
  /* NOT FOUND */
  WARN_UNDEFINED_LINE;
  return (l);
}

/* 
--------------------------------------------------------------------------------------------
                               PAUSE
--------------------------------------------------------------------------------------------
*/

LineType *
bwb_PAUSE (LineType * l)
{
  /*
     SYNTAX:     PAUSE
   */
  char *pstring;
  char *tbuf;
  int tlen;
   
  assert (l != NULL);
  assert (My != NULL);
  assert (My->CurrentVersion != NULL);
  assert (My->ConsoleOutput != NULL);
  assert (My->ConsoleInput != NULL);

  pstring = My->ConsoleOutput;
  tbuf = My->ConsoleInput;
  tlen = MAX_LINE_LENGTH;
  if (My->CurrentVersion->OptionVersionValue & (S70 | I70 | I73))
  {
    /* PAUSE [comment] */
    line_skip_eol (l);
  }
  sprintf (pstring, "PAUSE AT %d\n", l->number);
  bwx_input (pstring, FALSE, tbuf, tlen);
  return (l);
}


/* 
--------------------------------------------------------------------------------------------
                               STOP
--------------------------------------------------------------------------------------------
*/

LineType *
bwb_STOP (LineType * l)
{
  /*
     SYNTAX:     STOP
   */
   
  assert (l != NULL);
  assert (My != NULL);
  assert (My->CurrentVersion != NULL);

  if (My->CurrentVersion->OptionVersionValue & (S70 | I70 | I73))
  {
    /* STOP [comment] */
    line_skip_eol (l);
  }
  My->ContinueLine = l->next;
  bwx_STOP (TRUE);
  return bwb_END (l);
}


/* 
--------------------------------------------------------------------------------------------
                               END
--------------------------------------------------------------------------------------------
*/

LineType *
bwb_END (LineType * l)
{
  /*
     SYNTAX:     END
   */
   
  assert (l != NULL);
  assert (My != NULL);
  assert (My->CurrentVersion != NULL);

  if (My->CurrentVersion->OptionVersionValue & (S70 | I70 | I73))
  {
    /* END [comment] */
    line_skip_eol (l);
  }
  My->ContinueLine = l->next;
  bwx_STOP (FALSE);
  return My->EndMarker;
}

/* 
--------------------------------------------------------------------------------------------
                               RUN
--------------------------------------------------------------------------------------------
*/

static LineType *
bwb_run_filename_or_linenumber (LineType * L)
{
  LineType *current = NULL;
  VariantType x;
  VariantType *X;
   
  assert (L != NULL);
  assert (My != NULL);
  assert (My->StartMarker != NULL);

  X = &x;
  CLEAR_VARIANT (X);
  if (line_read_expression (L, X) == FALSE)        /* bwb_run_filename_or_linenumber */
  {
    WARN_SYNTAX_ERROR;
    return L;
  }
  if (X->VariantTypeCode == StringTypeCode)
  {
    /* RUN "filename" */
    /* RUN A$ */
    if (is_empty_string (X->Buffer))
    {
      WARN_BAD_FILE_NAME;
      return L;
    }
    /* open the file and execute it */
    bwb_new ();                        /* clear memory */
    if (My->ProgramFilename != NULL)
    {
      free (My->ProgramFilename);
      My->ProgramFilename = NULL;
    }
    My->ProgramFilename = bwb_strdup (X->Buffer);
    if (bwb_fload (NULL) == FALSE)
    {
      WARN_BAD_FILE_NAME;
      return L;
    }
    /*
     **
     ** FORCE SCAN
     **
     */
    if (bwb_scan () == FALSE)
    {
      WARN_CANT_CONTINUE;
      return L;
    }
    current = My->StartMarker->next;
  }
  else
  {
    /* RUN 100 */
    /* RUN N */
    /* execute the line */
    int LineNumber;


    LineNumber = (int) bwb_rint (X->Number);
    /*
     **
     ** FORCE SCAN
     **
     */
    if (bwb_scan () == FALSE)
    {
      WARN_CANT_CONTINUE;
      goto EXIT;
    }
    current = find_line_number (LineNumber);        /* RUN 100 */
    if (current == NULL)
    {
      WARN_CANT_CONTINUE;
      return L;
    }
  }
EXIT:
  RELEASE_VARIANT (X);
  return current;
}

LineType *
bwb_RUNNH (LineType * L)
{
   
  assert (L != NULL);
  return bwb_RUN (L);
}

LineType *
bwb_RUN (LineType * L)
{
  /*
     SYNTAX:     RUN
     SYNTAX:     RUN filename$
     SYNTAX:     RUN linenumber
   */
  LineType *current;
   
  assert (L != NULL);
  assert (My != NULL);
  assert (My->EndMarker != NULL);
  assert (My->DefaultVariableType != NULL);
  
  /* clear the STACK */
  bwb_clrexec ();
  if (bwb_incexec ())
  {
    /* OK */
  }
  else
  {
    /* ERROR */
    WARN_OUT_OF_MEMORY;
    return My->EndMarker;
  }

  if (line_is_eol (L))
  {
    /* RUN */

    var_CLEAR ();

    /* if( TRUE ) */
    {
      int n;
      for (n = 0; n < 26; n++)
      {
        My->DefaultVariableType[n] = DoubleTypeCode;
      }
    }
    /*
     **
     ** FORCE SCAN
     **
     */
    if (bwb_scan () == FALSE)
    {
      WARN_CANT_CONTINUE;
      return My->EndMarker;
    }
    current = My->StartMarker->next;
  }
  else
  {
    /* RUN 100 : RUN filename$ */
    current = bwb_run_filename_or_linenumber (L);
    if (current == NULL)
    {
      WARN_UNDEFINED_LINE;
      return My->EndMarker;
    }
  }
  current->position = 0;

  assert (My->StackHead != NULL);
  My->StackHead->line = current;
  My->StackHead->ExecCode = EXEC_NORM;

  /* RUN */
  WARN_CLEAR;                        /* bwb_RUN */
  My->ContinueLine = NULL;
  SetOnError (0);

  /* if( TRUE ) */
  {
    time_t t;
    struct tm *lt;

    time (&t);
    lt = localtime (&t);
    My->StartTimeInteger = lt->tm_hour;
    My->StartTimeInteger *= 60;
    My->StartTimeInteger += lt->tm_min;
    My->StartTimeInteger *= 60;
    My->StartTimeInteger += lt->tm_sec;
    /* number of seconds since midnight */
  }

  return current;
}

/* 
--------------------------------------------------------------------------------------------
                               CONT
--------------------------------------------------------------------------------------------
*/
LineType *
bwb_CONTINUE (LineType * l)
{
  /*
     SYNTAX:     CONTINUE
   */
   
  assert (l != NULL);
  return bwb_CONT (l);
}

LineType *
bwb_CONT (LineType * l)
{
  /*
     SYNTAX:     CONT
   */
  LineType *current;
   
  assert (l != NULL);
  assert (My != NULL);
  assert (My->EndMarker != NULL);
  assert (My->StartMarker != NULL);

  current = NULL;
  /* see if there is an element */
  if (line_is_eol (l))
  {
    /* CONT */
    current = My->ContinueLine;
  }
  else
  {
    /* CONT 100 */
    int LineNumber;

    LineNumber = 0;
    if (line_read_line_number (l, &LineNumber))
    {
      current = find_line_number (LineNumber);        /* CONT 100 */
    }
  }


  if (current == NULL || current == My->EndMarker)
  {
    /* same as RUN */
    current = My->StartMarker->next;
  }
  /*
   **
   ** FORCE SCAN
   **
   */
  if (bwb_scan () == FALSE)
  {
    WARN_CANT_CONTINUE;
    return (l);
  }
  current->position = 0;
  bwb_clrexec ();
  if (bwb_incexec ())
  {
    /* OK */
    My->StackHead->line = current;
    My->StackHead->ExecCode = EXEC_NORM;
  }
  else
  {
    /* ERROR */
    WARN_OUT_OF_MEMORY;
    return My->EndMarker;
  }


  /* CONT */
  My->ContinueLine = NULL;
  return current;
}


/* 
--------------------------------------------------------------------------------------------
                               NEW
--------------------------------------------------------------------------------------------
*/

void
bwb_xnew (LineType * l)
{
  LineType *current;
  LineType *previous;
  int wait;
   
  assert (l != NULL);
  assert (My != NULL);
  assert (My->EndMarker != NULL);

  previous = NULL;                /* JBV */
  wait = TRUE;
  for (current = l->next; current != My->EndMarker; current = current->next)
  {
    assert (current != NULL);
    if (wait == FALSE)
    {
      free (previous);
      previous = NULL;
    }
    wait = FALSE;
    previous = current;
  }
  l->next = My->EndMarker;
}

static void
bwb_new ()
{
  assert (My != NULL);
  assert (My->StartMarker != NULL);
  assert (My->DefaultVariableType != NULL);
   

  /* clear program in memory */
  bwb_xnew (My->StartMarker);

  /* clear all variables */
  var_CLEAR ();
  /* if( TRUE ) */
  {
    int n;
    for (n = 0; n < 26; n++)
    {
      My->DefaultVariableType[n] = DoubleTypeCode;
    }
  }

  /* NEW */
  WARN_CLEAR;                        /* bwb_new */
  My->ContinueLine = NULL;
  SetOnError (0);
}

LineType *
bwb_NEW (LineType * l)
{
  /*
     SYNTAX:     NEW
   */
   
  assert (l != NULL);
  assert (My != NULL);
  assert (My->CurrentVersion != NULL);

  bwb_new ();
  if (My->CurrentVersion->OptionVersionValue & (D64 | G65 | G67 | G74))
  {
    if (line_is_eol (l))
    {
      /* NEW */
      char *tbuf;
      int tlen;

      tbuf = My->ConsoleInput;
      tlen = MAX_LINE_LENGTH;
      /* prompt for the program name */
      bwx_input ("NEW PROBLEM NAME:", FALSE, tbuf, tlen);
      if (is_empty_string (tbuf))
      {
        WARN_BAD_FILE_NAME;
        return l;
      }
      if (My->ProgramFilename != NULL)
      {
        free (My->ProgramFilename);
        My->ProgramFilename = NULL;
      }
      My->ProgramFilename = bwb_strdup (tbuf);
    }
    else
    {
      /* NEW filename$ */
      /* the parameter is the program name */

      char *Value;

      Value = NULL;
      if (line_read_string_expression (l, &Value) == FALSE)
      {
        WARN_SYNTAX_ERROR;
        return (l);
      }
      if (is_empty_string (Value))
      {
        WARN_BAD_FILE_NAME;
        return l;
      }
      if (My->ProgramFilename != NULL)
      {
        free (My->ProgramFilename);
        My->ProgramFilename = NULL;
      }
      My->ProgramFilename = Value;
    }
  }
  else
  {
    /* ignore any parameters */
    line_skip_eol (l);
  }
  return (l);
}

/* 
--------------------------------------------------------------------------------------------
                               SCRATCH
--------------------------------------------------------------------------------------------
*/

LineType *
bwb_SCRATCH (LineType * l)
{
  /*
     SYNTAX:     SCRATCH              -- same as NEW
     SYNTAX:     SCRATCH # filenumber -- close file and re-open for output
   */
   
  assert (l != NULL);

  if (line_is_eol (l))
  {
    /* SCRATCH */
    bwb_new ();
    return (l);
  }
  if (line_skip_FilenumChar (l))
  {
    /* SCRATCH # X */
    int FileNumber;

    if (line_read_integer_expression (l, &FileNumber) == FALSE)
    {
      WARN_BAD_FILE_NUMBER;
      return (l);
    }
    if (FileNumber < 0)
    {
      /* SCRATCH # -1 is silently ignored */
      return (l);
    }
    if (FileNumber == 0)
    {
      /* SCRATCH # 0 is silently ignored */
      return (l);
    }
    My->CurrentFile = find_file_by_number (FileNumber);
    if (My->CurrentFile == NULL)
    {
      WARN_BAD_FILE_NUMBER;
      return (l);
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
      return (l);
    }
    if (bwb_strcmp (My->CurrentFile->FileName, "*") != 0)
    {
      if ((My->CurrentFile->cfp =
           fopen (My->CurrentFile->FileName, "w")) == NULL)
      {
        WARN_BAD_FILE_NAME;
        return (l);
      }
      My->CurrentFile->DevMode = DEVMODE_OUTPUT;
    }
    /* OK */
    return (l);
  }
  WARN_SYNTAX_ERROR;
  return (l);
}

/* 
============================================================================================
                               SYSTEM and so on
============================================================================================
*/
static LineType *
bwb_system (LineType * l)
{
  /*
     SYNTAX:     SYSTEM
   */
  assert (l != NULL);
  assert (My != NULL);
  assert (My->SYSOUT != NULL);
  assert (My->SYSOUT->cfp != NULL);

  fprintf (My->SYSOUT->cfp, "\n");
  fflush (My->SYSOUT->cfp);
  bwx_terminate ();
  return (l);                        /* never reached */
}

/* 
--------------------------------------------------------------------------------------------
                               BYE
--------------------------------------------------------------------------------------------
*/

LineType *
bwb_BYE (LineType * l)
{
  /*
     SYNTAX:     BYE
   */
   
  assert (l != NULL);
  return bwb_system (l);
}

/* 
--------------------------------------------------------------------------------------------
                               DOS
--------------------------------------------------------------------------------------------
*/

LineType *
bwb_DOS (LineType * l)
{
  /*
     SYNTAX:     DOS
   */
   
  assert (l != NULL);
  return bwb_system (l);
}

/* 
--------------------------------------------------------------------------------------------
                               FLEX
--------------------------------------------------------------------------------------------
*/

LineType *
bwb_FLEX (LineType * l)
{
  /*
     SYNTAX:     FLEX
   */
   
  assert (l != NULL);
  return bwb_system (l);
}

/* 
--------------------------------------------------------------------------------------------
                               GOODBYE
--------------------------------------------------------------------------------------------
*/
LineType *
bwb_GOODBYE (LineType * l)
{
  /*
     SYNTAX:     GOODBYE
   */
   
  assert (l != NULL);
  return bwb_system (l);
}

/* 
--------------------------------------------------------------------------------------------
                               MON
--------------------------------------------------------------------------------------------
*/

LineType *
bwb_MON (LineType * l)
{
  /*
     SYNTAX:     MON
   */
   
  assert (l != NULL);
  return bwb_system (l);
}

/* 
--------------------------------------------------------------------------------------------
                               QUIT
--------------------------------------------------------------------------------------------
*/
LineType *
bwb_QUIT (LineType * l)
{
  /*
     SYNTAX:     QUIT
   */
   
  assert (l != NULL);
  return bwb_system (l);
}

/* 
--------------------------------------------------------------------------------------------
                               SYSTEM
--------------------------------------------------------------------------------------------
*/
LineType *
bwb_SYSTEM (LineType * l)
{
  /*
     SYNTAX:     SYSTEM
   */
   
  assert (l != NULL);
  return bwb_system (l);
}


/* 
============================================================================================
                               LOAD and so on
============================================================================================
*/

static LineType *
bwb_load (LineType * Line, char *Prompt, int IsNew)
{
  /*
   **
   ** load a BASIC program from a file
   **
   */
  /*
     SYNTAX:     ... [filename$]
   */
   
  assert (Line != NULL);
  assert (Prompt != NULL);
  assert (My != NULL);
  assert (My->CurrentVersion != NULL);

  if (IsNew)
  {
    /* TRUE == LOAD */
    bwb_new ();
  }
  else
  {
    /* FALSE == MERGE */
    if (My->ProgramFilename != NULL)
    {
      free (My->ProgramFilename);
      My->ProgramFilename = NULL;
    }
  }
  if (line_is_eol (Line))
  {
    /* default is the last filename used by LOAD or SAVE */
    /* if( My->CurrentVersion->OptionVersionValue & (D64 | G65 | G67 | G74) ) */
    if (is_empty_string (My->ProgramFilename))
    {
      /* prompt for the program name */
      char *tbuf;
      int tlen;

      tbuf = My->ConsoleInput;
      tlen = MAX_LINE_LENGTH;
      bwx_input (Prompt, FALSE, tbuf, tlen);
      if (is_empty_string (tbuf))
      {
        WARN_BAD_FILE_NAME;
        return (Line);
      }
      if (My->ProgramFilename != NULL)
      {
        free (My->ProgramFilename);
        My->ProgramFilename = NULL;
      }
      My->ProgramFilename = bwb_strdup (tbuf);
    }
    fprintf (My->SYSOUT->cfp, "Loading %s\n", My->ProgramFilename);
    ResetConsoleColumn ();
  }
  else
  {
    /* Get an argument for filename */
    char *Value;

    Value = NULL;
    if (line_read_string_expression (Line, &Value) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (Line);
    }
    if (is_empty_string (Value))
    {
      WARN_BAD_FILE_NAME;
      return (Line);
    }
    if (My->ProgramFilename != NULL)
    {
      free (My->ProgramFilename);
      My->ProgramFilename = NULL;
    }
    My->ProgramFilename = Value;
  }
  if (bwb_fload (NULL) == FALSE)
  {
    WARN_BAD_FILE_NAME;
    return (Line);
  }
  if (IsNew)
  {
    /* TRUE == LOAD */
  }
  else
  {
    /* FALSE == MERGE */
    if (My->ProgramFilename != NULL)
    {
      free (My->ProgramFilename);
      My->ProgramFilename = NULL;
    }
  }
  /*
   **
   ** FORCE SCAN
   **
   */
  if (bwb_scan () == FALSE)
  {
    WARN_CANT_CONTINUE;
  }
  return (Line);
}

/* 
--------------------------------------------------------------------------------------------
                               CLOAD
--------------------------------------------------------------------------------------------
*/
LineType *
bwb_CLOAD (LineType * Line)
{
  /*
     SYNTAX:     CLOAD [filename$]
   */
   
  assert (Line != NULL);
  return bwb_load (Line, "CLOAD FILE NAME:", TRUE);
}

/* 
--------------------------------------------------------------------------------------------
                               LOAD
--------------------------------------------------------------------------------------------
*/
LineType *
bwb_LOAD (LineType * Line)
{
  /*
     SYNTAX:     LOAD [filename$]
   */
   
  assert (Line != NULL);
  return bwb_load (Line, "LOAD FILE NAME:", TRUE);
}

/* 
--------------------------------------------------------------------------------------------
                               MERGE
--------------------------------------------------------------------------------------------
*/
LineType *
bwb_MERGE (LineType * l)
{
  /*
     SYNTAX:     MERGE [filename$]
   */
   
  assert (l != NULL);
  return bwb_load (l, "MERGE FILE NAME:", FALSE);
}

/* 
--------------------------------------------------------------------------------------------
                               OLD
--------------------------------------------------------------------------------------------
*/
LineType *
bwb_OLD (LineType * Line)
{
  /*
     SYNTAX:     OLD [filename$]
   */
   
  assert (Line != NULL);
  return bwb_load (Line, "OLD PROBLEM NAME:", TRUE);
}

/* 
--------------------------------------------------------------------------------------------
                               TLOAD
--------------------------------------------------------------------------------------------
*/
LineType *
bwb_TLOAD (LineType * Line)
{
  /*
     SYNTAX:     TLOAD [filename$]
   */
   
  assert (Line != NULL);
  return bwb_load (Line, "TLOAD FILE NAME:", TRUE);
}


/* 
--------------------------------------------------------------------------------------------
                               RENAME
--------------------------------------------------------------------------------------------
*/
static LineType *
H14_RENAME (LineType * l)
{
  /*
     SYNTAX:     RENAME from$ TO to$
   */
  char *From;
  char *To;
   
  assert (l != NULL);

  From = NULL;
  To = NULL;
  if (line_read_string_expression (l, &From) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  if (is_empty_string (From))
  {
    WARN_BAD_FILE_NAME;
    return (l);
  }
  if (line_skip_word (l, "TO") == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  if (line_read_string_expression (l, &To) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  if (is_empty_string (To))
  {
    WARN_BAD_FILE_NAME;
    return (l);
  }
  if (rename (From, To))
  {
    WARN_BAD_FILE_NAME;
    return (l);
  }
  return (l);
}

LineType *
bwb_RENAME (LineType * l)
{
  /*
     SYNTAX:     RENAME filename$
   */
   
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
  assert( My->ConsoleInput != NULL );

  if (My->CurrentVersion->OptionVersionValue & (H14))
  {
    /* RENAME == change an exisiting file's name */
    return H14_RENAME (l);
  }
  /* RENAME == change the BASIC program's name for a later SAVE */
  if (line_is_eol (l))
  {
    /* RENAME */
    if (My->CurrentVersion->OptionVersionValue & (D64 | G65 | G67 | G74))
    {
      /* prompt for the program name */
      char *tbuf;
      int tlen;

      tbuf = My->ConsoleInput;
      tlen = MAX_LINE_LENGTH;
      bwx_input ("RENAME PROBLEM NAME:", FALSE, tbuf, tlen);
      if (is_empty_string (tbuf))
      {
        WARN_BAD_FILE_NAME;
        return (l);
      }
      if (My->ProgramFilename != NULL)
      {
        free (My->ProgramFilename);
        My->ProgramFilename = NULL;
      }
      My->ProgramFilename = bwb_strdup (tbuf);
    }
    else
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
  }
  else
  {
    /* RENAME value$ */
    char *Value;

    Value = NULL;
    if (line_read_string_expression (l, &Value) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    if (is_empty_string (Value))
    {
      WARN_BAD_FILE_NAME;
      return (l);
    }
    if (My->ProgramFilename != NULL)
    {
      free (My->ProgramFilename);
      My->ProgramFilename = NULL;
    }
    My->ProgramFilename = Value;
  }
  return (l);
}


/* 
--------------------------------------------------------------------------------------------
                               MAT
--------------------------------------------------------------------------------------------
*/

extern void
Determinant (VariableType * v)
{
  /* http://easy-learn-c-language.blogspot.com/search/label/Numerical%20Methods */
  /* Numerical Methods: Determinant of nxn matrix using C */

  DoubleType **matrix;
  DoubleType ratio;

  int i;
  int j;
  int k;
  int n;
   
  assert (v != NULL);
  assert( My != NULL );


  My->LastDeterminant = 0;        /* default */

  n = v->UBOUND[0] - v->LBOUND[0] + 1;

  if ((matrix = (DoubleType **) calloc (n, sizeof (DoubleType *))) == NULL)
  {
    goto EXIT;
  }
  assert( matrix != NULL );
  for (i = 0; i < n; i++)
  {
    if ((matrix[i] = (DoubleType *) calloc (n, sizeof (DoubleType))) == NULL)
    {
      goto EXIT;
    }
    assert( matrix[i] != NULL );
  }

  for (i = 0; i < n; i++)
  {
    for (j = 0; j < n; j++)
    {
      VariantType variant;
      CLEAR_VARIANT (&variant);
      v->VINDEX[0] = v->LBOUND[0] + i;
      v->VINDEX[1] = v->LBOUND[1] + j;
      if (var_get (v, &variant) == FALSE)
      {
        WARN_VARIABLE_NOT_DECLARED;
        goto EXIT;
      }
      if (variant.VariantTypeCode == StringTypeCode)
      {
        WARN_TYPE_MISMATCH;
        goto EXIT;
      }
      matrix[i][j] = variant.Number;
    }
  }

  /* Conversion of matrix to upper triangular */

  for (i = 0; i < n; i++)
  {
    for (j = 0; j < n; j++)
    {
      if (j > i)
      {
        if (matrix[i][i] == 0)
        {
          /* - Evaluation of an expression results in division
           * by zero (nonfatal, the recommended recovery
           * procedure is to supply machine infinity with the
           * sign of the numerator and continue) 
           */
          if (WARN_DIVISION_BY_ZERO)
          {
            /* ERROR */
            goto EXIT;
          }
          /* CONTINUE */
          if (matrix[j][i] < 0)
          {
            ratio = MINDBL;
          }
          else
          {
            ratio = MAXDBL;
          }
        }
        else
        {
          ratio = matrix[j][i] / matrix[i][i];
        }
        for (k = 0; k < n; k++)
        {
          matrix[j][k] -= ratio * matrix[i][k];
        }
      }
    }
  }


  My->LastDeterminant = 1;        /* storage for determinant */

  for (i = 0; i < n; i++)
  {
    DoubleType Value;

    Value = matrix[i][i];
    My->LastDeterminant *= Value;
  }

EXIT:
  if( matrix != NULL )
  {
    for (i = 0; i < n; i++)
    {
      if( matrix[i] != NULL )
      {
        free (matrix[i]);
        /* matrix[i] = NULL; */
      }
    }
    free (matrix);
    /* matrix = NULL; */
  }
}

int
InvertMatrix (VariableType * vOut, VariableType * vIn)
{
  /* http://easy-learn-c-language.blogspot.com/search/label/Numerical%20Methods */
  /* Numerical Methods: Inverse of nxn matrix using C */

  int Result;
  DoubleType **matrix;
  DoubleType ratio;

  int i;
  int j;
  int k;
  int n;
   
  assert (vOut != NULL);
  assert (vIn != NULL);

  Result = FALSE;
  n = vIn->UBOUND[0] - vIn->LBOUND[0] + 1;

  if ((matrix = (DoubleType **) calloc (n, sizeof (DoubleType *))) == NULL)
  {
        goto EXIT;
  }
  assert( matrix != NULL );

  for (i = 0; i < n; i++)
  {
    if ((matrix[i] =
         (DoubleType *) calloc (n + n, sizeof (DoubleType))) == NULL)
    {
        goto EXIT;
    }
    assert( matrix[i] != NULL );
  }

  for (i = 0; i < n; i++)
  {
    for (j = 0; j < n; j++)
    {
      VariantType variant;
      CLEAR_VARIANT (&variant);

      vIn->VINDEX[0] = vIn->LBOUND[0] + i;
      vIn->VINDEX[1] = vIn->LBOUND[1] + j;
      if (var_get (vIn, &variant) == FALSE)
      {
        WARN_VARIABLE_NOT_DECLARED;
        goto EXIT;
      }
      if (variant.VariantTypeCode == StringTypeCode)
      {
        WARN_TYPE_MISMATCH;
        goto EXIT;
      }
      matrix[i][j] = variant.Number;
    }
  }

  for (i = 0; i < n; i++)
  {
    for (j = n; j < 2 * n; j++)
    {
      if (i == (j - n))
      {
        matrix[i][j] = 1.0;
      }
      else
      {
        matrix[i][j] = 0.0;
      }
    }
  }

  for (i = 0; i < n; i++)
  {
    for (j = 0; j < n; j++)
    {
      if (i != j)
      {
        if (matrix[i][i] == 0)
        {
          /* - Evaluation of an expression results in division
           * by zero (nonfatal, the recommended recovery
           * procedure is to supply machine infinity with the
           * sign of the numerator and continue) 
           */
          if (WARN_DIVISION_BY_ZERO)
          {
            /* ERROR */
            goto EXIT;
          }
          /* CONTINUE */
          if (matrix[j][i] < 0)
          {
            ratio = MINDBL;
          }
          else
          {
            ratio = MAXDBL;
          }
        }
        else
        {
          ratio = matrix[j][i] / matrix[i][i];
        }
        for (k = 0; k < 2 * n; k++)
        {
          matrix[j][k] -= ratio * matrix[i][k];
        }
      }
    }
  }

  for (i = 0; i < n; i++)
  {
    DoubleType a;

    a = matrix[i][i];
    if (a == 0)
    {
      /* - Evaluation of an expression results in division
       * by zero (nonfatal, the recommended recovery
       * procedure is to supply machine infinity with the
       * sign of the numerator and continue) 
       */
      if (WARN_DIVISION_BY_ZERO)
      {
        /* ERROR */
        goto EXIT;
      }
      /* CONTINUE */
      for (j = 0; j < 2 * n; j++)
      {
        if (matrix[i][j] < 0)
        {
          matrix[i][j] = MINDBL;
        }
        else
        {
          matrix[i][j] = MAXDBL;
        }
      }
    }
    else
    {
      for (j = 0; j < 2 * n; j++)
      {
        matrix[i][j] /= a;
      }
    }
  }

  for (i = 0; i < n; i++)
  {
    for (j = 0; j < n; j++)
    {
      VariantType variant;
      CLEAR_VARIANT (&variant);

      vOut->VINDEX[0] = vOut->LBOUND[0] + i;
      vOut->VINDEX[1] = vOut->LBOUND[0] + j;
      variant.VariantTypeCode = vOut->VariableTypeCode;
      variant.Number = matrix[i][j + n];
      if (var_set (vOut, &variant) == FALSE)
      {
        WARN_VARIABLE_NOT_DECLARED;
        goto EXIT;
      }
    }
  }
  /*
  ** 
  ** Everything is OK
  **
  */
  Result = TRUE;
  
  
EXIT:
  if (matrix != NULL)
  {
    for (i = 0; i < n; i++)
    {
      if (matrix[i] != NULL)
      {
        free (matrix[i]);
        /* matrix[i] = NULL; */
      }
    }
    free (matrix);
    /* matrix = NULL; */
  }
  return Result;
}

static int
line_read_matrix_redim (LineType * l, VariableType * v)
{
  /* get OPTIONAL parameters if the variable is dimensioned */
   
  assert (l != NULL);
  assert (v != NULL);

  if (line_peek_LparenChar (l))
  {
    /* get requested size, which is <= original array size */
    size_t array_units;
    int n;
    int dimensions;
    int LBOUND[MAX_DIMS];
    int UBOUND[MAX_DIMS];

    if (line_read_array_redim (l, &dimensions, LBOUND, UBOUND) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return FALSE;
    }
    /* update array dimensions */
    array_units = 1;
    for (n = 0; n < dimensions; n++)
    {
      if (UBOUND[n] < LBOUND[n])
      {
        WARN_SUBSCRIPT_OUT_OF_RANGE;
        return FALSE;
      }
      array_units *= UBOUND[n] - LBOUND[n] + 1;
    }
    if (array_units > v->array_units)
    {
      WARN_SUBSCRIPT_OUT_OF_RANGE;
      return FALSE;
    }
    v->dimensions = dimensions;
    for (n = 0; n < dimensions; n++)
    {
      v->LBOUND[n] = LBOUND[n];
      v->UBOUND[n] = UBOUND[n];
    }
  }
  return TRUE;
}

LineType *
bwb_MAT (LineType * l)
{
  /*
     SYNTAX:     MAT A = CON
     SYNTAX:     MAT A = IDN
     SYNTAX:     MAT A = ZER
     SYNTAX:     MAT A = INV B
     SYNTAX:     MAT A = TRN B
     SYNTAX:     MAT A = (k) * B
     SYNTAX:     MAT A = B
     SYNTAX:     MAT A = B + C
     SYNTAX:     MAT A = B - C
     SYNTAX:     MAT A = B * C
   */
  VariableType *v_A;
  char varname_A[NameLengthMax + 1];
   
  assert (l != NULL);

  /* just a placeholder for now. this will grow. */

  if (line_read_varname (l, varname_A) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  v_A = mat_find (varname_A);
  if (v_A == NULL)
  {
    WARN_VARIABLE_NOT_DECLARED;
    return (l);
  }
  /* variable MUST be numeric */
  if (VAR_IS_STRING (v_A))
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  if (line_read_matrix_redim (l, v_A) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  if (line_skip_EqualChar (l) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  /* MAT A = ... */
  if (line_skip_word (l, "CON"))
  {
    /* MAT A = CON */
    /* MAT A = CON(I) */
    /* MAT A = CON(I,J) */
    /* MAT A = CON(I,J,K) */
    /* OK */
    int i;
    int j;
    int k;

    if (line_read_matrix_redim (l, v_A) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }

    /* both arrays are of the same size */
    switch (v_A->dimensions)
    {
    case 1:
      for (i = v_A->LBOUND[0]; i <= v_A->UBOUND[0]; i++)
      {
        VariantType variant;
        CLEAR_VARIANT (&variant);

        variant.VariantTypeCode = v_A->VariableTypeCode;
        variant.Number = 1;
        v_A->VINDEX[0] = i;
        if (var_set (v_A, &variant) == FALSE)
        {
          WARN_VARIABLE_NOT_DECLARED;
          return (l);
        }
      }
      break;
    case 2:
      for (i = v_A->LBOUND[0]; i <= v_A->UBOUND[0]; i++)
      {
        for (j = v_A->LBOUND[1]; j <= v_A->UBOUND[1]; j++)
        {
          VariantType variant;
          CLEAR_VARIANT (&variant);

          variant.VariantTypeCode = v_A->VariableTypeCode;
          variant.Number = 1;
          v_A->VINDEX[0] = i;
          v_A->VINDEX[1] = j;
          if (var_set (v_A, &variant) == FALSE)
          {
            WARN_VARIABLE_NOT_DECLARED;
            return (l);
          }
        }
      }
      break;
    case 3:
      for (i = v_A->LBOUND[0]; i <= v_A->UBOUND[0]; i++)
      {
        for (j = v_A->LBOUND[1]; j <= v_A->UBOUND[1]; j++)
        {
          for (k = v_A->LBOUND[2]; k <= v_A->UBOUND[2]; k++)
          {
            VariantType variant;
            CLEAR_VARIANT (&variant);

            variant.VariantTypeCode = v_A->VariableTypeCode;
            variant.Number = 1;
            v_A->VINDEX[0] = i;
            v_A->VINDEX[1] = j;
            v_A->VINDEX[2] = k;
            if (var_set (v_A, &variant) == FALSE)
            {
              WARN_VARIABLE_NOT_DECLARED;
              return (l);
            }
          }
        }
      }
      break;
    default:
      WARN_SYNTAX_ERROR;
      return (l);
    }
  }
  else if (line_skip_word (l, "IDN"))
  {
    /* MAT A = IDN */
    /* MAT A = IDN(I,J) */
    /* OK */
    int i;
    int j;

    if (line_read_matrix_redim (l, v_A) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }

    if (v_A->dimensions != 2)
    {
      WARN_SUBSCRIPT_OUT_OF_RANGE;
      return (l);
    }
    if (v_A->LBOUND[0] != v_A->LBOUND[1])
    {
      WARN_SUBSCRIPT_OUT_OF_RANGE;
      return (l);
    }
    if (v_A->UBOUND[0] != v_A->UBOUND[1])
    {
      WARN_SUBSCRIPT_OUT_OF_RANGE;
      return (l);
    }
    /* square matrix */
    for (i = v_A->LBOUND[0]; i <= v_A->UBOUND[0]; i++)
    {
      for (j = v_A->LBOUND[1]; j <= v_A->UBOUND[1]; j++)
      {
        VariantType variant;
        CLEAR_VARIANT (&variant);

        variant.VariantTypeCode = v_A->VariableTypeCode;
        if (i == j)
        {
          variant.Number = 1;
        }
        else
        {
          variant.Number = 0;
        }
        v_A->VINDEX[0] = i;
        v_A->VINDEX[1] = j;
        if (var_set (v_A, &variant) == FALSE)
        {
          WARN_VARIABLE_NOT_DECLARED;
          return (l);
        }
      }
    }
  }
  else if (line_skip_word (l, "ZER"))
  {
    /* MAT A = ZER */
    /* MAT A = ZER(I) */
    /* MAT A = ZER(I,J) */
    /* MAT A = ZER(I,J,K) */
    /* OK */
    int i;
    int j;
    int k;

    if (line_read_matrix_redim (l, v_A) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    /* both arrays are of the same size */
    switch (v_A->dimensions)
    {
    case 1:
      for (i = v_A->LBOUND[0]; i <= v_A->UBOUND[0]; i++)
      {
        VariantType variant;
        CLEAR_VARIANT (&variant);

        variant.VariantTypeCode = v_A->VariableTypeCode;
        variant.Number = 0;
        v_A->VINDEX[0] = i;
        if (var_set (v_A, &variant) == FALSE)
        {
          WARN_VARIABLE_NOT_DECLARED;
          return (l);
        }
      }
      break;
    case 2:
      for (i = v_A->LBOUND[0]; i <= v_A->UBOUND[0]; i++)
      {
        for (j = v_A->LBOUND[1]; j <= v_A->UBOUND[1]; j++)
        {
          VariantType variant;
          CLEAR_VARIANT (&variant);

          variant.VariantTypeCode = v_A->VariableTypeCode;
          variant.Number = 0;
          v_A->VINDEX[0] = i;
          v_A->VINDEX[1] = j;
          if (var_set (v_A, &variant) == FALSE)
          {
            WARN_VARIABLE_NOT_DECLARED;
            return (l);
          }
        }
      }
      break;
    case 3:
      for (i = v_A->LBOUND[0]; i <= v_A->UBOUND[0]; i++)
      {
        for (j = v_A->LBOUND[1]; j <= v_A->UBOUND[1]; j++)
        {
          for (k = v_A->LBOUND[2]; k <= v_A->UBOUND[2]; k++)
          {
            VariantType variant;
            CLEAR_VARIANT (&variant);

            variant.VariantTypeCode = v_A->VariableTypeCode;
            variant.Number = 0;
            v_A->VINDEX[0] = i;
            v_A->VINDEX[1] = j;
            v_A->VINDEX[2] = k;
            if (var_set (v_A, &variant) == FALSE)
            {
              WARN_VARIABLE_NOT_DECLARED;
              return (l);
            }
          }
        }
      }
      break;
    default:
      WARN_SYNTAX_ERROR;
      return (l);
    }
  }
  else if (line_skip_word (l, "INV"))
  {
    /* MAT A = INV B */
    /* MAT A = INV( B ) */
    /* OK */
    VariableType *v_B;
    char varname_B[NameLengthMax + 1];

    if (v_A->dimensions != 2)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    if (v_A->LBOUND[0] != v_A->LBOUND[1] || v_A->UBOUND[0] != v_A->UBOUND[1])
    {
      WARN_SUBSCRIPT_OUT_OF_RANGE;
      return (l);
    }
    if (line_skip_LparenChar (l))
    {
      /* optional */
    }
    if (line_read_varname (l, varname_B) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }

    if ((v_B = mat_find (varname_B)) == NULL)
    {
      WARN_VARIABLE_NOT_DECLARED;
      return (l);
    }
    /* variable MUST be numeric */
    if (VAR_IS_STRING (v_B))
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    if (line_read_matrix_redim (l, v_B) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    if (line_skip_RparenChar (l))
    {
      /* optional */
    }
    if (v_B->dimensions != 2)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    if (v_B->LBOUND[0] != v_B->LBOUND[1] || v_B->UBOUND[0] != v_B->UBOUND[1])
    {
      WARN_SUBSCRIPT_OUT_OF_RANGE;
      return (l);
    }
    if (v_A->LBOUND[0] != v_B->LBOUND[0] || v_A->UBOUND[0] != v_B->UBOUND[0])
    {
      WARN_SUBSCRIPT_OUT_OF_RANGE;
      return (l);
    }
    /* square matrix */
    Determinant (v_B);
    if (My->LastDeterminant == 0)
    {
      WARN_ILLEGAL_FUNCTION_CALL;
      return (l);
    }
    if (InvertMatrix (v_A, v_B) == FALSE)
    {
      WARN_ILLEGAL_FUNCTION_CALL;
      return (l);
    }
  }
  else if (line_skip_word (l, "TRN"))
  {
    /* MAT A = TRN B */
    /* MAT A = TRN( B ) */
    /* OK */
    int i;
    int j;
    VariableType *v_B;
    char varname_B[NameLengthMax + 1];

    if (v_A->dimensions != 2)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    if (line_skip_LparenChar (l))
    {
      /* optional */
    }
    if (line_read_varname (l, varname_B) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    if ((v_B = mat_find (varname_B)) == NULL)
    {
      WARN_VARIABLE_NOT_DECLARED;
      return (l);
    }
    /* variable MUST be numeric */
    if (VAR_IS_STRING (v_B))
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    if (line_read_matrix_redim (l, v_B) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    if (line_skip_RparenChar (l))
    {
      /* optional */
    }
    if (v_B->dimensions != 2)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    /* MxN */
    if (v_A->LBOUND[0] != v_B->LBOUND[1] || v_A->UBOUND[0] != v_B->UBOUND[1])
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    if (v_A->LBOUND[1] != v_B->LBOUND[0] || v_A->UBOUND[1] != v_B->UBOUND[0])
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    /* transpose matrix */
    for (i = v_A->LBOUND[0]; i <= v_A->UBOUND[0]; i++)
    {
      for (j = v_A->LBOUND[1]; j <= v_A->UBOUND[1]; j++)
      {
        VariantType variant;
        CLEAR_VARIANT (&variant);

        v_B->VINDEX[1] = i;
        v_B->VINDEX[0] = j;
        if (var_get (v_B, &variant) == FALSE)
        {
          WARN_VARIABLE_NOT_DECLARED;
          return (l);
        }
        v_A->VINDEX[0] = i;
        v_A->VINDEX[1] = j;
        if (var_set (v_A, &variant) == FALSE)
        {
          WARN_VARIABLE_NOT_DECLARED;
          return (l);
        }
      }
    }
  }
  else if (line_peek_LparenChar (l))
  {
    /* MAT A = (k) * B */
    DoubleType Multiplier;
    VariableType *v_B;
    int i;
    int j;
    int k;
    char *E;
    int p;
    char varname_B[NameLengthMax + 1];
    char *tbuf;

    tbuf = My->ConsoleInput;
    bwb_strcpy (tbuf, &(l->buffer[l->position]));
    E = bwb_strrchr (tbuf, '*');
    if (E == NULL)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    *E = NulChar;
    p = 0;
    if (buff_read_numeric_expression (tbuf, &p, &Multiplier) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    l->position += p;
    if (line_skip_StarChar (l) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    if (line_read_varname (l, varname_B) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }

    if ((v_B = mat_find (varname_B)) == NULL)
    {
      WARN_VARIABLE_NOT_DECLARED;
      return (l);
    }
    /* variable MUST be numeric */
    if (VAR_IS_STRING (v_B))
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    if (line_read_matrix_redim (l, v_B) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    if (v_A->dimensions != v_B->dimensions)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    /* both arrays are of the same size */
    switch (v_A->dimensions)
    {
    case 1:
      for (i = v_A->LBOUND[0]; i <= v_A->UBOUND[0]; i++)
      {
        VariantType variant;
        CLEAR_VARIANT (&variant);

        v_B->VINDEX[0] = i;
        if (var_get (v_B, &variant) == FALSE)
        {
          WARN_VARIABLE_NOT_DECLARED;
          return (l);
        }

        variant.Number *= Multiplier;

        v_A->VINDEX[0] = i;
        if (var_set (v_A, &variant) == FALSE)
        {
          WARN_VARIABLE_NOT_DECLARED;
          return (l);
        }
      }
      break;
    case 2:
      for (i = v_A->LBOUND[0]; i <= v_A->UBOUND[0]; i++)
      {
        for (j = v_A->LBOUND[1]; j <= v_A->UBOUND[1]; j++)
        {
          VariantType variant;
          CLEAR_VARIANT (&variant);

          v_B->VINDEX[0] = i;
          v_B->VINDEX[1] = j;
          if (var_get (v_B, &variant) == FALSE)
          {
            WARN_VARIABLE_NOT_DECLARED;
            return (l);
          }

          variant.Number *= Multiplier;

          v_A->VINDEX[0] = i;
          v_A->VINDEX[1] = j;
          if (var_set (v_A, &variant) == FALSE)
          {
            WARN_VARIABLE_NOT_DECLARED;
            return (l);
          }
        }
      }
      break;
    case 3:
      for (i = v_A->LBOUND[0]; i <= v_A->UBOUND[0]; i++)
      {
        for (j = v_A->LBOUND[1]; j <= v_A->UBOUND[1]; j++)
        {
          for (k = v_A->LBOUND[2]; k <= v_A->UBOUND[2]; k++)
          {
            VariantType variant;
            CLEAR_VARIANT (&variant);

            v_B->VINDEX[0] = i;
            v_B->VINDEX[1] = j;
            v_B->VINDEX[2] = k;
            if (var_get (v_B, &variant) == FALSE)
            {
              WARN_VARIABLE_NOT_DECLARED;
              return (l);
            }

            variant.Number *= Multiplier;

            v_A->VINDEX[0] = i;
            v_A->VINDEX[1] = j;
            v_A->VINDEX[2] = k;
            if (var_set (v_A, &variant) == FALSE)
            {
              WARN_VARIABLE_NOT_DECLARED;
              return (l);
            }
          }
        }
      }
      break;
    default:
      WARN_SYNTAX_ERROR;
      return (l);
    }
  }
  else
  {
    /* MAT A = B */
    /* MAT A = B + C */
    /* MAT A = B - C */
    /* MAT A = B * C */
    VariableType *v_B;
    char varname_B[NameLengthMax + 1];

    if (line_read_varname (l, varname_B) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }

    if ((v_B = mat_find (varname_B)) == NULL)
    {
      WARN_VARIABLE_NOT_DECLARED;
      return (l);
    }
    /* variable MUST be numeric */
    if (VAR_IS_STRING (v_B))
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    if (line_read_matrix_redim (l, v_B) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    if (line_is_eol (l))
    {
      /* MAT A = B */
      /* OK */
      int i;
      int j;
      int k;

      if (v_A->dimensions != v_B->dimensions)
      {
        WARN_SYNTAX_ERROR;
        return (l);
      }
      /* both arrays are of the same size */
      switch (v_A->dimensions)
      {
      case 1:
        for (i = v_A->LBOUND[0]; i <= v_A->UBOUND[0]; i++)
        {
          VariantType variant;
          CLEAR_VARIANT (&variant);

          v_B->VINDEX[0] = i;
          if (var_get (v_B, &variant) == FALSE)
          {
            WARN_VARIABLE_NOT_DECLARED;
            return (l);
          }

          v_A->VINDEX[0] = i;
          if (var_set (v_A, &variant) == FALSE)
          {
            WARN_VARIABLE_NOT_DECLARED;
            return (l);
          }
        }
        break;
      case 2:
        for (i = v_A->LBOUND[0]; i <= v_A->UBOUND[0]; i++)
        {
          for (j = v_A->LBOUND[1]; j <= v_A->UBOUND[1]; j++)
          {
            VariantType variant;
            CLEAR_VARIANT (&variant);

            v_B->VINDEX[0] = i;
            v_B->VINDEX[1] = j;
            if (var_get (v_B, &variant) == FALSE)
            {
              WARN_VARIABLE_NOT_DECLARED;
              return (l);
            }

            v_A->VINDEX[0] = i;
            v_A->VINDEX[1] = j;
            if (var_set (v_A, &variant) == FALSE)
            {
              WARN_VARIABLE_NOT_DECLARED;
              return (l);
            }
          }
        }
        break;
      case 3:
        for (i = v_A->LBOUND[0]; i <= v_A->UBOUND[0]; i++)
        {
          for (j = v_A->LBOUND[1]; j <= v_A->UBOUND[1]; j++)
          {
            for (k = v_A->LBOUND[2]; k <= v_A->UBOUND[2]; k++)
            {
              VariantType variant;
              CLEAR_VARIANT (&variant);

              v_B->VINDEX[0] = i;
              v_B->VINDEX[1] = j;
              v_B->VINDEX[2] = k;
              if (var_get (v_B, &variant) == FALSE)
              {
                WARN_VARIABLE_NOT_DECLARED;
                return (l);
              }

              v_A->VINDEX[0] = i;
              v_A->VINDEX[1] = j;
              v_A->VINDEX[2] = k;
              if (var_set (v_A, &variant) == FALSE)
              {
                WARN_VARIABLE_NOT_DECLARED;
                return (l);
              }
            }
          }
        }
        break;
      default:
        WARN_SYNTAX_ERROR;
        return (l);
      }
    }
    else if (line_skip_PlusChar (l))
    {
      /* MAT A = B + C */
      /* OK */
      int i;
      int j;
      int k;
      VariableType *v_C;
      char varname_C[NameLengthMax + 1];

      if (v_A->dimensions != v_B->dimensions)
      {
        WARN_SYNTAX_ERROR;
        return (l);
      }
      /* both arrays are of the same size */

      if (line_read_varname (l, varname_C) == FALSE)
      {
        WARN_SYNTAX_ERROR;
        return (l);
      }

      if ((v_C = mat_find (varname_C)) == NULL)
      {
        WARN_VARIABLE_NOT_DECLARED;
        return (l);
      }
      /* variable MUST be numeric */
      if (VAR_IS_STRING (v_C))
      {
        WARN_SYNTAX_ERROR;
        return (l);
      }
      if (line_read_matrix_redim (l, v_C) == FALSE)
      {
        WARN_SYNTAX_ERROR;
        return (l);
      }
      if (v_B->dimensions != v_C->dimensions)
      {
        WARN_SYNTAX_ERROR;
        return (l);
      }
      /* both arrays are of the same size */
      switch (v_A->dimensions)
      {
      case 1:
        for (i = v_A->LBOUND[0]; i <= v_A->UBOUND[0]; i++)
        {
          VariantType variant_L;
          VariantType variant_R;
          CLEAR_VARIANT (&variant_L);
          CLEAR_VARIANT (&variant_R);

          v_B->VINDEX[0] = i;
          if (var_get (v_B, &variant_L) == FALSE)
          {
            WARN_VARIABLE_NOT_DECLARED;
            return (l);
          }

          v_C->VINDEX[0] = i;
          if (var_get (v_C, &variant_R) == FALSE)
          {
            WARN_VARIABLE_NOT_DECLARED;
            return (l);
          }

          variant_L.Number += variant_R.Number;

          v_A->VINDEX[0] = i;
          if (var_set (v_A, &variant_L) == FALSE)
          {
            WARN_VARIABLE_NOT_DECLARED;
            return (l);
          }
        }
        break;
      case 2:
        for (i = v_A->LBOUND[0]; i <= v_A->UBOUND[0]; i++)
        {
          for (j = v_A->LBOUND[1]; j <= v_A->UBOUND[1]; j++)
          {
            VariantType variant_L;
            VariantType variant_R;
            CLEAR_VARIANT (&variant_L);
            CLEAR_VARIANT (&variant_R);

            v_B->VINDEX[0] = i;
            v_B->VINDEX[1] = j;
            if (var_get (v_B, &variant_L) == FALSE)
            {
              WARN_VARIABLE_NOT_DECLARED;
              return (l);
            }

            v_C->VINDEX[0] = i;
            v_C->VINDEX[1] = j;
            if (var_get (v_C, &variant_R) == FALSE)
            {
              WARN_VARIABLE_NOT_DECLARED;
              return (l);
            }

            variant_L.Number += variant_R.Number;

            v_A->VINDEX[0] = i;
            v_A->VINDEX[1] = j;
            if (var_set (v_A, &variant_L) == FALSE)
            {
              WARN_VARIABLE_NOT_DECLARED;
              return (l);
            }
          }
        }
        break;
      case 3:
        for (i = v_A->LBOUND[0]; i <= v_A->UBOUND[0]; i++)
        {
          for (j = v_A->LBOUND[1]; j <= v_A->UBOUND[1]; j++)
          {
            for (k = v_A->LBOUND[2]; k <= v_A->UBOUND[2]; k++)
            {
              VariantType variant_L;
              VariantType variant_R;
              CLEAR_VARIANT (&variant_L);
              CLEAR_VARIANT (&variant_R);

              v_B->VINDEX[0] = i;
              v_B->VINDEX[1] = j;
              v_B->VINDEX[2] = k;
              if (var_get (v_B, &variant_L) == FALSE)
              {
                WARN_VARIABLE_NOT_DECLARED;
                return (l);
              }

              v_C->VINDEX[0] = i;
              v_C->VINDEX[1] = j;
              v_C->VINDEX[2] = k;
              if (var_get (v_C, &variant_R) == FALSE)
              {
                WARN_VARIABLE_NOT_DECLARED;
                return (l);
              }

              variant_L.Number += variant_R.Number;

              v_A->VINDEX[0] = i;
              v_A->VINDEX[1] = j;
              v_A->VINDEX[2] = k;
              if (var_set (v_A, &variant_L) == FALSE)
              {
                WARN_VARIABLE_NOT_DECLARED;
                return (l);
              }
            }
          }
        }
        break;
      default:
        WARN_SYNTAX_ERROR;
        return (l);
      }
    }
    else if (line_skip_MinusChar (l))
    {
      /* MAT A = B - C */
      /* OK */
      int i;
      int j;
      int k;
      VariableType *v_C;
      char varname_C[NameLengthMax + 1];

      if (v_A->dimensions != v_B->dimensions)
      {
        WARN_SYNTAX_ERROR;
        return (l);
      }
      /* both arrays are of the same size */

      if (line_read_varname (l, varname_C) == FALSE)
      {
        WARN_SYNTAX_ERROR;
        return (l);
      }

      if ((v_C = mat_find (varname_C)) == NULL)
      {
        WARN_VARIABLE_NOT_DECLARED;
        return (l);
      }
      /* variable MUST be numeric */
      if (VAR_IS_STRING (v_C))
      {
        WARN_SYNTAX_ERROR;
        return (l);
      }
      if (line_read_matrix_redim (l, v_C) == FALSE)
      {
        WARN_SYNTAX_ERROR;
        return (l);
      }
      if (v_B->dimensions != v_C->dimensions)
      {
        WARN_SYNTAX_ERROR;
        return (l);
      }
      /* both arrays are of the same dimension */
      switch (v_A->dimensions)
      {
      case 1:
        for (i = v_A->LBOUND[0]; i <= v_A->UBOUND[0]; i++)
        {
          VariantType variant_L;
          VariantType variant_R;
          CLEAR_VARIANT (&variant_L);
          CLEAR_VARIANT (&variant_R);

          v_B->VINDEX[0] = i;
          if (var_get (v_B, &variant_L) == FALSE)
          {
            WARN_VARIABLE_NOT_DECLARED;
            return (l);
          }

          v_C->VINDEX[0] = i;
          if (var_get (v_C, &variant_R) == FALSE)
          {
            WARN_VARIABLE_NOT_DECLARED;
            return (l);
          }

          variant_L.Number -= variant_R.Number;

          v_A->VINDEX[0] = i;
          if (var_set (v_A, &variant_L) == FALSE)
          {
            WARN_VARIABLE_NOT_DECLARED;
            return (l);
          }
        }
        break;
      case 2:
        for (i = v_A->LBOUND[0]; i <= v_A->UBOUND[0]; i++)
        {
          for (j = v_A->LBOUND[1]; j <= v_A->UBOUND[1]; j++)
          {
            VariantType variant_L;
            VariantType variant_R;
            CLEAR_VARIANT (&variant_L);
            CLEAR_VARIANT (&variant_R);

            v_B->VINDEX[0] = i;
            v_B->VINDEX[1] = j;
            if (var_get (v_B, &variant_L) == FALSE)
            {
              WARN_VARIABLE_NOT_DECLARED;
              return (l);
            }

            v_C->VINDEX[0] = i;
            v_C->VINDEX[1] = j;
            if (var_get (v_C, &variant_R) == FALSE)
            {
              WARN_VARIABLE_NOT_DECLARED;
              return (l);
            }

            variant_L.Number -= variant_R.Number;

            v_A->VINDEX[0] = i;
            v_A->VINDEX[1] = j;
            if (var_set (v_A, &variant_L) == FALSE)
            {
              WARN_VARIABLE_NOT_DECLARED;
              return (l);
            }
          }
        }
        break;
      case 3:
        for (i = v_A->LBOUND[0]; i <= v_A->UBOUND[0]; i++)
        {
          for (j = v_A->LBOUND[1]; j <= v_A->UBOUND[1]; j++)
          {
            for (k = v_A->LBOUND[2]; k <= v_A->UBOUND[2]; k++)
            {
              VariantType variant_L;
              VariantType variant_R;
              CLEAR_VARIANT (&variant_L);
              CLEAR_VARIANT (&variant_R);

              v_B->VINDEX[0] = i;
              v_B->VINDEX[1] = j;
              v_B->VINDEX[2] = k;
              if (var_get (v_B, &variant_L) == FALSE)
              {
                WARN_VARIABLE_NOT_DECLARED;
                return (l);
              }

              v_C->VINDEX[0] = i;
              v_C->VINDEX[1] = j;
              v_C->VINDEX[2] = k;
              if (var_get (v_C, &variant_R) == FALSE)
              {
                WARN_VARIABLE_NOT_DECLARED;
                return (l);
              }

              variant_L.Number -= variant_R.Number;

              v_A->VINDEX[0] = i;
              v_A->VINDEX[1] = j;
              v_A->VINDEX[2] = k;
              if (var_set (v_A, &variant_L) == FALSE)
              {
                WARN_VARIABLE_NOT_DECLARED;
                return (l);
              }
            }
          }
        }
        break;
      default:
        WARN_SYNTAX_ERROR;
        return (l);
      }
    }
    else if (line_skip_StarChar (l))
    {
      /* MAT A = B * C */
      int i;
      int j;
      int k;
      VariableType *v_C;
      char varname_C[NameLengthMax + 1];


      if (v_A->dimensions != 2)
      {
        WARN_SUBSCRIPT_OUT_OF_RANGE;
        return (l);
      }
      if (v_B->dimensions != 2)
      {
        WARN_SUBSCRIPT_OUT_OF_RANGE;
        return (l);
      }
      if (line_read_varname (l, varname_C) == FALSE)
      {
        WARN_SYNTAX_ERROR;
        return (l);
      }
      if ((v_C = mat_find (varname_C)) == NULL)
      {
        WARN_VARIABLE_NOT_DECLARED;
        return (l);
      }
      /* variable MUST be numeric */
      if (VAR_IS_STRING (v_C))
      {
        WARN_TYPE_MISMATCH;
        return (l);
      }
      if (line_read_matrix_redim (l, v_C) == FALSE)
      {
        WARN_SYNTAX_ERROR;
        return (l);
      }
      if (v_C->dimensions != 2)
      {
        WARN_SUBSCRIPT_OUT_OF_RANGE;
        return (l);
      }
      if (v_A->LBOUND[0] != v_B->LBOUND[0])
      {
        WARN_SUBSCRIPT_OUT_OF_RANGE;
        return (l);
      }
      if (v_A->UBOUND[0] != v_B->UBOUND[0])
      {
        WARN_SUBSCRIPT_OUT_OF_RANGE;
        return (l);
      }
      if (v_A->LBOUND[1] != v_C->LBOUND[1])
      {
        WARN_SUBSCRIPT_OUT_OF_RANGE;
        return (l);
      }
      if (v_A->UBOUND[1] != v_C->UBOUND[1])
      {
        WARN_SUBSCRIPT_OUT_OF_RANGE;
        return (l);
      }
      if (v_B->LBOUND[1] != v_C->LBOUND[0])
      {
        WARN_SUBSCRIPT_OUT_OF_RANGE;
        return (l);
      }
      if (v_B->UBOUND[1] != v_C->UBOUND[0])
      {
        WARN_SUBSCRIPT_OUT_OF_RANGE;
        return (l);
      }
      for (i = v_A->LBOUND[0]; i <= v_A->UBOUND[0]; i++)
      {
        for (j = v_A->LBOUND[1]; j <= v_A->UBOUND[1]; j++)
        {
          VariantType variant_A;
          CLEAR_VARIANT (&variant_A);

          variant_A.VariantTypeCode = v_A->VariableTypeCode;
          variant_A.Number = 0;

          v_A->VINDEX[0] = i;
          v_A->VINDEX[1] = j;
          if (var_set (v_A, &variant_A) == FALSE)
          {
            WARN_VARIABLE_NOT_DECLARED;
            return (l);
          }
          for (k = v_C->LBOUND[0]; k <= v_C->UBOUND[0]; k++)
          {
            VariantType variant_B;
            VariantType variant_C;
            CLEAR_VARIANT (&variant_B);
            CLEAR_VARIANT (&variant_C);

            v_A->VINDEX[0] = i;
            v_A->VINDEX[1] = j;
            if (var_get (v_A, &variant_A) == FALSE)
            {
              WARN_VARIABLE_NOT_DECLARED;
              return (l);
            }

            v_B->VINDEX[0] = i;
            v_B->VINDEX[1] = k;
            if (var_get (v_B, &variant_B) == FALSE)
            {
              WARN_VARIABLE_NOT_DECLARED;
              return (l);
            }

            v_C->VINDEX[0] = k;
            v_C->VINDEX[1] = j;
            if (var_get (v_C, &variant_C) == FALSE)
            {
              WARN_VARIABLE_NOT_DECLARED;
              return (l);
            }

            variant_A.Number += variant_B.Number * variant_C.Number;

            v_A->VINDEX[0] = i;
            v_A->VINDEX[1] = j;
            if (var_set (v_A, &variant_A) == FALSE)
            {
              WARN_VARIABLE_NOT_DECLARED;
              return (l);
            }
          }
        }
      }
    }
    else
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
  }
  return (l);
}



/* 
--------------------------------------------------------------------------------------------
                               STORE
--------------------------------------------------------------------------------------------
*/


LineType *
bwb_STORE (LineType * l)
{
  /*
     SYNTAX:     STORE NumericArrayName
   */
   
  assert (l != NULL);
  return bwb_CSAVE8 (l);
}


/* 
--------------------------------------------------------------------------------------------
                               CSAVE*
--------------------------------------------------------------------------------------------
*/

#define CSAVE_VERSION_1 0x20150218L

LineType *
bwb_CSAVE8 (LineType * l)
{
  /*
     SYNTAX:     CSAVE* NumericArrayName
   */
  VariableType *v = NULL;
  FILE *f;
  unsigned long n;
  size_t t;
  char varname[NameLengthMax + 1];
   
  assert (l != NULL);

  if (line_read_varname (l, varname) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }

  v = mat_find (varname);
  if (v == NULL)
  {
    WARN_VARIABLE_NOT_DECLARED;
    return (l);
  }
  /* variable MUST be numeric */
  if (VAR_IS_STRING (v))
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  /* variable MUST be an array */
  if (v->dimensions == 0)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  if (line_read_matrix_redim (l, v) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  /* variable storage is a mess, we bypass that tradition here. */
  t = v->array_units;
  if (t <= 1)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  /* open file */
  f = fopen (v->name, "w");
  if (f == NULL)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  /* write version number */
  n = CSAVE_VERSION_1;
  fwrite (&n, sizeof (long), 1, f);
  /* write total number of elements */
  fwrite (&t, sizeof (long), 1, f);
  /* write data */
  fwrite (v->Value.Number, sizeof (DoubleType), t, f);
  /* OK */
  bwb_fclose (f);
  return (l);
}

/* 
--------------------------------------------------------------------------------------------
                               RECALL
--------------------------------------------------------------------------------------------
*/

LineType *
bwb_RECALL (LineType * l)
{
  /*
     SYNTAX:     RECALL NumericArrayName
   */
   
  assert (l != NULL);
  return bwb_CLOAD8 (l);
}

/* 
--------------------------------------------------------------------------------------------
                               CLOAD*
--------------------------------------------------------------------------------------------
*/


LineType *
bwb_CLOAD8 (LineType * l)
{
  /*
     SYNTAX:     CLOAD* NumericArrayName
   */
  VariableType *v = NULL;
  FILE *f;
  unsigned long n;
  size_t t;
  char varname[NameLengthMax + 1];
   
  assert (l != NULL);

  if (line_read_varname (l, varname) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }

  v = mat_find (varname);
  if (v == NULL)
  {
    WARN_VARIABLE_NOT_DECLARED;
    return (l);
  }

  /* variable MUST be numeric */
  if (VAR_IS_STRING (v))
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  /* variable MUST be an array */
  if (v->dimensions == 0)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  if (line_read_matrix_redim (l, v) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  /* variable storage is a mess, we bypass that tradition here. */
  t = v->array_units;
  if (t <= 1)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  /* open file */
  f = fopen (v->name, "r");
  if (f == NULL)
  {
    WARN_BAD_FILE_NAME;
    return (l);
  }
  /* read version number */
  n = 0;
  fread (&n, sizeof (long), 1, f);
  if (n != CSAVE_VERSION_1)
  {
    bwb_fclose (f);
    WARN_BAD_FILE_NAME;
    return (l);
  }
  /* read total number of elements */
  n = 0;
  fread (&n, sizeof (long), 1, f);
  if (n != t)
  {
    bwb_fclose (f);
    WARN_BAD_FILE_NAME;
    return (l);
  }
  /* read data */
  fread (v->Value.Number, sizeof (DoubleType), t, f);
  /* OK */
  bwb_fclose (f);
  return (l);
}




/* 
============================================================================================
                               SAVE and so on
============================================================================================
*/

static LineType *
bwb_save (LineType * Line, char *Prompt)
{
  /*
     SYNTAX:     SAVE [filename$]
   */
  FILE *outfile;
   
  assert (Line != NULL);
  assert (Prompt != NULL);
  assert( My != NULL );
  assert( My->ConsoleInput != NULL );
  assert( My->SYSOUT != NULL );
  assert( My->SYSOUT->cfp != NULL );


  /* Get an argument for filename */
  if (line_is_eol (Line))
  {
    /* default is the last filename used by LOAD or SAVE */
    if (is_empty_string (My->ProgramFilename) && Prompt != NULL)
    {
      /* prompt for the program name */
      char *tbuf;
      int tlen;

      tbuf = My->ConsoleInput;
      tlen = MAX_LINE_LENGTH;
      bwx_input (Prompt, FALSE, tbuf, tlen);
      if (is_empty_string (tbuf))
      {
        WARN_BAD_FILE_NAME;
        return (Line);
      }
      if (My->ProgramFilename != NULL)
      {
        free (My->ProgramFilename);
        My->ProgramFilename = NULL;
      }
      My->ProgramFilename = bwb_strdup (tbuf);
    }
    assert( My->ProgramFilename != NULL );
    fprintf (My->SYSOUT->cfp, "Saving %s\n", My->ProgramFilename);
    ResetConsoleColumn ();
  }
  else
  {
    char *Value;

    Value = NULL;
    if (line_read_string_expression (Line, &Value) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (Line);
    }
    if (is_empty_string (Value))
    {
      WARN_BAD_FILE_NAME;
      return (Line);
    }
    if (My->ProgramFilename != NULL)
    {
      free (My->ProgramFilename);
    }
    My->ProgramFilename = Value;
  }
  assert( My->ProgramFilename != NULL );
  if ((outfile = fopen (My->ProgramFilename, "w")) == NULL)
  {
    WARN_BAD_FILE_NAME;
    return (Line);
  }
  bwb_xlist (Line, outfile);
  bwb_fclose (outfile);
  return (Line);
}

/* 
--------------------------------------------------------------------------------------------
                               CSAVE
--------------------------------------------------------------------------------------------
*/

LineType *
bwb_CSAVE (LineType * Line)
{
  /*
     SYNTAX:     CSAVE [filename$]
   */
   
  assert (Line != NULL);
  return bwb_save (Line, "CSAVE FILE NAME:");
}

/* 
--------------------------------------------------------------------------------------------
                               REPLACE
--------------------------------------------------------------------------------------------
*/
LineType *
bwb_REPLACE (LineType * Line)
{
  /*
     SYNTAX:     REPLACE [filename$]
   */
   
  assert (Line != NULL);
  return bwb_save (Line, "REPLACE FILE NAME:");
}

/* 
--------------------------------------------------------------------------------------------
                               SAVE
--------------------------------------------------------------------------------------------
*/
LineType *
bwb_SAVE (LineType * l)
{
  /*
     SYNTAX:     SAVE [filename$]
   */
   
  assert (l != NULL);
  return bwb_save (l, "SAVE FILE NAME:");
}

/* 
--------------------------------------------------------------------------------------------
                               TSAVE
--------------------------------------------------------------------------------------------
*/

LineType *
bwb_TSAVE (LineType * Line)
{
  /*
     SYNTAX:     TSAVE [filename$]
   */
   
  assert (Line != NULL);
  return bwb_save (Line, "TSAVE FILE NAME:");
}


/* 
============================================================================================
                               LIST and so on
============================================================================================
*/
static int
xl_line (FILE * file, LineType * l)
{
  char LineExecuted;
  char *C;                        /* start of comment text */
  char *buffer;                        /* 0...99999 */
   
  assert (file != NULL);
  assert (l != NULL);
  assert( My != NULL );
  assert( My->NumLenBuffer != NULL );
  assert( My->CurrentVersion != NULL );
  assert( My->SYSOUT != NULL );
  assert( My->SYSOUT->cfp != NULL );
  assert( My->SYSPRN != NULL );
  assert( My->SYSPRN->cfp != NULL );

  /*
   ** The only difference between LIST, LLIST and SAVE is:
   ** LIST and LLIST display an '*' 
   ** when a line has been executed
   ** and OPTION COVERAGE ON is enabled.
   */

  buffer = My->NumLenBuffer;
  LineExecuted = ' ';
  if (My->CurrentVersion->OptionFlags & (OPTION_COVERAGE_ON))
  {
    if (l->LineFlags & LINE_EXECUTED)
    {
      if (file == My->SYSOUT->cfp || file == My->SYSPRN->cfp)
      {
        /* LIST */
        /* LLIST */
        LineExecuted = '*';
      }
      else
      {
        /* SAVE */
        /* EDIT implies SAVE */
      }
    }
  }

  C = l->buffer;
  if (l->LineFlags & LINE_NUMBERED)
  {
    /* explicitly numbered */
    sprintf (buffer, "%*d", LineNumberDigits, l->number);
    /* ##### xxx */
  }
  else
  {
    /* implicitly numbered */
    if (My->LastLineNumber == l->number)
    {
      /* multi-statement line */
      if (l->cmdnum == C_REM
          && IS_CHAR (l->buffer[0], My->CurrentVersion->OptionCommentChar))
      {
        /* trailing comment */
        sprintf (buffer, "%*s%c", LineNumberDigits - 1, "",
                 My->CurrentVersion->OptionCommentChar);
        C++;                        /* skip comment char */
        while (*C == ' ')
        {
          /* skip spaces */
          C++;
        }
        /* ____' xxx */
      }
      else if (My->CurrentVersion->OptionStatementChar)
      {
        /* all other commands, add a colon */
        sprintf (buffer, "%*s%c", LineNumberDigits - 1, "",
                 My->CurrentVersion->OptionStatementChar);
        /* ____: xxx */
      }
      else
      {
        /*
           The user is trying to list a multi-line statement 
           in a dialect that does NOT support multi-line statements.  
           This could occur when LOADing in one dialect and then SAVEing as another dialect, such as:
           OPTION VERSION BASIC-80
           LOAD "TEST1.BAS"
           100 REM TEST
           110 PRINT:PRINT:PRINT
           OPTION VERSION MARK-I
           EDIT
           100 REM TEST
           110 PRINT
           PRINT
           PRINT
           The only thing we can reasonably do is put spaces for the line number,
           since the user will have to edit the results manually anyways.
         */
        sprintf (buffer, "%*s", LineNumberDigits, "");
        /* _____ xxx */
      }
    }
    else
    {
      /* single-statement line */
      sprintf (buffer, "%*s", LineNumberDigits, "");
      /* _____ xxx */
    }
  }

  fprintf (file, "%s", buffer);
  fprintf (file, "%c", LineExecuted);

  /* if( TRUE ) */
  {
    /* %INCLUDE */
    int i;
    for (i = 0; i < l->IncludeLevel; i++)
    {
      fputc (' ', file);
    }
  }
  if (My->OptionIndentInteger > 0)
  {
    int i;

    for (i = 0; i < l->Indention; i++)
    {
      int j;
      for (j = 0; j < My->OptionIndentInteger; j++)
      {
        fputc (' ', file);
      }
    }
  }
  fprintf (file, "%s\n", C);

  My->LastLineNumber = l->number;

  return TRUE;
}

static LineType *
bwb_xlist (LineType * l, FILE * file)
{
   
  assert (l != NULL);
  assert (file != NULL);
  assert( My != NULL );
  assert( My->StartMarker != NULL );
  assert( My->EndMarker != NULL );

  /*
   **
   ** FORCE SCAN
   **
   */
  if (bwb_scan () == FALSE)
  {
    /*
     **
     ** we are used by bwb_SAVE and bwb_EDIT
     **
     WARN_CANT_CONTINUE;
     return (l);
     */
  }

  if (line_is_eol (l))
  {
    /* LIST */
    LineType *x;
    /* now go through and list appropriate lines */
    My->LastLineNumber = -1;
    for (x = My->StartMarker->next; x != My->EndMarker; x = x->next)
    {
      xl_line (file, x);
    }
    fprintf (file, "\n");
  }
  else
  {
    do
    {
      int head;
      int tail;

      if (line_read_line_sequence (l, &head, &tail))
      {
        /* LIST 's' - 'e' */
        LineType *x;
        if (head < MINLIN || head > MAXLIN)
        {
          WARN_UNDEFINED_LINE;
          return (l);
        }
        if (tail < MINLIN || tail > MAXLIN)
        {
          WARN_UNDEFINED_LINE;
          return (l);
        }
        if (head > tail)
        {
          WARN_UNDEFINED_LINE;
          return (l);
        }
        /* valid range */
        /* now go through and list appropriate lines */
        My->LastLineNumber = -1;
        for (x = My->StartMarker->next; x != My->EndMarker; x = x->next)
        {
          if (head <= x->number && x->number <= tail)
          {
            xl_line (file, x);
          }
        }
        fprintf (file, "\n");
      }
      else
      {
        WARN_SYNTAX_ERROR;
        return (l);
      }
    }
    while (line_skip_seperator (l));
  }
  if (file == My->SYSOUT->cfp)
  {
    ResetConsoleColumn ();
  }
  return (l);
}

/* 
--------------------------------------------------------------------------------------------
                               LIST
--------------------------------------------------------------------------------------------
*/
LineType *
bwb_LIST (LineType * l)
{
  /*
     SYNTAX:     LIST
     SYNTAX:     LIST line [,...]
     SYNTAX:     LIST line - line
   */
   
  assert (l != NULL);

  return bwb_xlist (l, My->SYSOUT->cfp);
}

/* 
--------------------------------------------------------------------------------------------
                               LISTNH
--------------------------------------------------------------------------------------------
*/
LineType *
bwb_LISTNH (LineType * l)
{
  /*
     SYNTAX:     LISTNH
     SYNTAX:     LISTNH line [,...]
     SYNTAX:     LISTNH line - line
   */
   
  assert (l != NULL);

  return bwb_xlist (l, My->SYSOUT->cfp);
}

/* 
--------------------------------------------------------------------------------------------
                               LLIST
--------------------------------------------------------------------------------------------
*/

LineType *
bwb_LLIST (LineType * l)
{
  /*
     SYNTAX:     LLIST
     SYNTAX:     LLIST line [,...]
     SYNTAX:     LLIST line - line
   */
   
  assert (l != NULL);

  return bwb_xlist (l, My->SYSPRN->cfp);
}



/* 
============================================================================================
                               DELETE and so on
============================================================================================
*/

static LineType *
bwb_delete (LineType * l)
{
   
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
  assert( My->StartMarker != NULL );
  assert( My->EndMarker != NULL );

  if (line_is_eol (l))
  {
    /* DELETE */
    WARN_SYNTAX_ERROR;
    return (l);
  }
  else if (My->CurrentVersion->OptionVersionValue & (C77))
  {
    /*
       SYNTAX:     DELETE filenum [,...]
     */
    do
    {
      int FileNumber;

      FileNumber = 0;
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

      My->CurrentFile = find_file_by_number (FileNumber);
      if (My->CurrentFile == NULL)
      {
        WARN_BAD_FILE_NUMBER;
        return (l);
      }
      if (My->CurrentFile->DevMode == DEVMODE_CLOSED)
      {
        WARN_BAD_FILE_NUMBER;
        return (l);
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
      My->CurrentFile->DevMode = DEVMODE_CLOSED;
      if (My->CurrentFile->FileName == NULL)
      {
        WARN_BAD_FILE_NAME;
        return (l);
      }
      remove (My->CurrentFile->FileName);
      free (My->CurrentFile->FileName);
      My->CurrentFile->FileName = NULL;
    }
    while (line_skip_seperator (l));
    /* OK */
    return (l);
  }
  else
  {
    /*
       SYNTAX:     DELETE line [,...]
       SYNTAX:     DELETE line - line
     */

    do
    {
      int head;
      int tail;

      if (line_read_line_sequence (l, &head, &tail))
      {
        /* DELETE 's' - 'e' */
        LineType *x;
        LineType *previous;
        if (head < MINLIN || head > MAXLIN)
        {
          WARN_UNDEFINED_LINE;
          return (l);
        }
        if (tail < MINLIN || tail > MAXLIN)
        {
          WARN_UNDEFINED_LINE;
          return (l);
        }
        if (head > tail)
        {
          WARN_UNDEFINED_LINE;
          return (l);
        }
        /* valid range */

        /* avoid deleting ourself */

        if (l->LineFlags & (LINE_USER))
        {
          /* console line (immediate mode) */
        }
        else if (head <= l->number && l->number <= tail)
        {
          /* 100 DELETE 100 */
          WARN_CANT_CONTINUE;
          return (l);
        }
        /* now go through and list appropriate lines */
        previous = My->StartMarker;
        for (x = My->StartMarker->next; x != My->EndMarker;)
        {
          LineType *next;

          next = x->next;
          if (x->number < head)
          {
            previous = x;
          }
          else if (head <= x->number && x->number <= tail)
          {
            if (x == l)
            {
              /* 100 DELETE 100 */
              WARN_CANT_CONTINUE;
              return (l);
            }
            bwb_freeline (x);
            previous->next = next;
          }
          x = next;
        }
      }
      else
      {
        WARN_SYNTAX_ERROR;
        return (l);
      }
    }
    while (line_skip_seperator (l));
    /*
     **
     ** FORCE SCAN
     **
     */
    if (bwb_scan () == FALSE)
    {
      WARN_CANT_CONTINUE;
      return (l);
    }
  }
  return (l);
}

/* 
--------------------------------------------------------------------------------------------
                               DELETE
--------------------------------------------------------------------------------------------
*/
LineType *
bwb_DELETE (LineType * l)
{
   
  assert (l != NULL);

  return bwb_delete (l);
}

/* 
--------------------------------------------------------------------------------------------
                               PDEL
--------------------------------------------------------------------------------------------
*/
LineType *
bwb_PDEL (LineType * l)
{
   
  assert (l != NULL);

  return bwb_delete (l);
}

#if FALSE                        /* keep the source to DONUM and DOUNNUM */

/* 
--------------------------------------------------------------------------------------------
                               DONUM
--------------------------------------------------------------------------------------------
*/

LineType *
bwb_donum (LineType * l)
{
  /*
     SYNTAX:     DONUM
   */
  LineType *current;
  int lnumber;
   
  assert (l != NULL);
  assert( My != NULL );
  assert( My->StartMarker != NULL );
  assert( My->EndMarker != NULL );

  lnumber = 10;
  for (current = My->StartMarker->next; current != My->EndMarker;
       current = current->next)
  {
    current->number = lnumber;

    lnumber += 10;
    if (lnumber > MAXLIN)
    {
      return (l);
    }
  }

  return (l);
}

/* 
--------------------------------------------------------------------------------------------
                               DOUNUM
--------------------------------------------------------------------------------------------
*/

LineType *
bwb_dounnum (LineType * l)
{
  /*
     SYNTAX:     DOUNNUM
   */
  LineType *current;
   
  assert (l != NULL);
  assert( My != NULL );
  assert( My->StartMarker != NULL );
  assert( My->EndMarker != NULL );

  for (current = My->StartMarker->next; current != My->EndMarker;
       current = current->next)
  {
    current->number = 0;
  }

  return (l);
}

#endif /* FALSE */



/* 
--------------------------------------------------------------------------------------------
                               FILES
--------------------------------------------------------------------------------------------
*/
LineType *
bwb_FILES (LineType * l)
{
  /*
     SYNTAX:     FILES A$ [, ...]
   */
  /* open a list of files in READ mode */
   
  assert (l != NULL);
  assert( My != NULL );

  do
  {
    int FileNumber;

    FileNumber = My->LastFileNumber;
    FileNumber++;
    if (FileNumber < 0)
    {
      WARN_BAD_FILE_NUMBER;
      return (l);
    }
    if (FileNumber == 0)
    {
      WARN_BAD_FILE_NUMBER;
      return (l);
    }
    My->CurrentFile = find_file_by_number (FileNumber);
    if (My->CurrentFile == NULL)
    {
      My->CurrentFile = file_new ();
      My->CurrentFile->FileNumber = FileNumber;
    }
    {
      char *Value;

      Value = NULL;
      if (line_read_string_expression (l, &Value) == FALSE)
      {
        WARN_SYNTAX_ERROR;
        return (l);
      }
      if (Value == NULL)
      {
        WARN_SYNTAX_ERROR;
        return (l);
      }
      if (My->CurrentFile->FileName != NULL)
      {
        free (My->CurrentFile->FileName);
        My->CurrentFile->FileName = NULL;
      }
      My->CurrentFile->FileName = Value;
      Value = NULL;
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
      return (l);
    }
    if (bwb_strcmp (My->CurrentFile->FileName, "*") != 0)
    {
      if ((My->CurrentFile->cfp =
           fopen (My->CurrentFile->FileName, "r")) == NULL)
      {
        WARN_BAD_FILE_NAME;
        return (l);
      }
      My->CurrentFile->DevMode = DEVMODE_INPUT;
    }
    My->LastFileNumber = FileNumber;
    /* OK */
  }
  while (line_skip_seperator (l));
  return (l);
}

/* 
--------------------------------------------------------------------------------------------
                               FILE
--------------------------------------------------------------------------------------------
*/
LineType *
bwb_FILE (LineType * l)
{
   
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );

  if (My->CurrentVersion->OptionVersionValue & (C77))
  {
    /* 
       CBASIC-II: 
       FILE file_name$                      ' filename$ must be a simple string scalar (no arrays)
       FILE file_name$ ( record_length% )   ' filename$ must be a simple string scalar (no arrays)
       -- if the file exists, 
       then it is used, 
       else it is created.
       -- Does not trigger IF END #
     */
    do
    {
      int FileNumber;
      VariableType *v;
      char varname[NameLengthMax + 1];

      if (line_read_varname (l, varname) == FALSE)
      {
        WARN_BAD_FILE_NAME;
        return (l);
      }
      if (is_empty_string (varname))
      {
        WARN_BAD_FILE_NAME;
        return (l);
      }
      v = find_variable_by_type (varname, 0, StringTypeCode);
      if (v == NULL)
      {
        WARN_VARIABLE_NOT_DECLARED;
        return (l);
      }
      if (VAR_IS_STRING (v))
      {
        /* OK */
      }
      else
      {
        WARN_TYPE_MISMATCH;
        return (l);
      }

      FileNumber = My->LastFileNumber;
      FileNumber++;
      if (FileNumber < 0)
      {
        WARN_BAD_FILE_NUMBER;
        return (l);
      }
      if (FileNumber == 0)
      {
        WARN_BAD_FILE_NUMBER;
        return (l);
      }
      My->CurrentFile = find_file_by_number (FileNumber);
      if (My->CurrentFile == NULL)
      {
        My->CurrentFile = file_new ();
        My->CurrentFile->FileNumber = FileNumber;
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
      /* OK */
      if (line_skip_LparenChar (l))
      {
        /* RANDOM file */
        int RecLen;

        if (line_read_integer_expression (l, &RecLen) == FALSE)
        {
          WARN_FIELD_OVERFLOW;
          return (l);
        }
        if (RecLen <= 0)
        {
          WARN_FIELD_OVERFLOW;
          return (l);
        }
        if (line_skip_RparenChar (l) == FALSE)
        {
          WARN_SYNTAX_ERROR;
          return (l);
        }
        if ((My->CurrentFile->buffer =
             (char *) calloc (RecLen + 1 /* NulChar */ ,
                              sizeof (char))) == NULL)
        {
          WARN_OUT_OF_MEMORY;
          return (l);
        }
        My->CurrentFile->width = RecLen;
      }

      /* if( TRUE ) */
      {
        VariantType variant;
        CLEAR_VARIANT (&variant);

        if (var_get (v, &variant) == FALSE)
        {
          WARN_VARIABLE_NOT_DECLARED;
          return (l);
        }
        if (variant.VariantTypeCode == StringTypeCode)
        {
          if (My->CurrentFile->FileName != NULL)
          {
            free (My->CurrentFile->FileName);
            My->CurrentFile->FileName = NULL;
          }
          My->CurrentFile->FileName = variant.Buffer;
          variant.Buffer = NULL;
        }
        else
        {
          WARN_TYPE_MISMATCH;
          return (l);
        }
      }
      if (is_empty_string (My->CurrentFile->FileName))
      {
        WARN_BAD_FILE_NAME;
        return (l);
      }
      My->CurrentFile->cfp = fopen (My->CurrentFile->FileName, "r+");
      if (My->CurrentFile->cfp == NULL)
      {
        My->CurrentFile->cfp = fopen (My->CurrentFile->FileName, "w");
        if (My->CurrentFile->cfp != NULL)
        {
          bwb_fclose (My->CurrentFile->cfp);
          My->CurrentFile->cfp = fopen (My->CurrentFile->FileName, "r+");
        }
      }
      if (My->CurrentFile->cfp == NULL)
      {
        WARN_BAD_FILE_NAME;
        return (l);
      }
      if (My->CurrentFile->width > 0)
      {
        /* RANDOM file */
        My->CurrentFile->DevMode = DEVMODE_RANDOM;
      }
      else
      {
        /* SERIAL file */
        My->CurrentFile->DevMode = DEVMODE_INPUT | DEVMODE_OUTPUT;
      }
      /* OK */
      My->LastFileNumber = FileNumber;
    }
    while (line_skip_seperator (l));
    /* OK */
    return (l);
  }
  if (line_skip_FilenumChar (l))
  {
    /*
       SYNTAX:     FILE # X, A$
     */
    int FileNumber;

    if (line_read_integer_expression (l, &FileNumber) == FALSE)
    {
      WARN_BAD_FILE_NUMBER;
      return (l);
    }
    if (line_skip_seperator (l))
    {
      /* OK */
    }
    else
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    if (FileNumber < 0)
    {
      /* "FILE # -1" is an ERROR */
      WARN_BAD_FILE_NUMBER;
      return (l);
    }
    if (FileNumber == 0)
    {
      /* "FILE # 0" is an ERROR */
      WARN_BAD_FILE_NUMBER;
      return (l);
    }
    My->CurrentFile = find_file_by_number (FileNumber);
    if (My->CurrentFile == NULL)
    {
      My->CurrentFile = file_new ();
      My->CurrentFile->FileNumber = FileNumber;
    }
    {
      char *Value;

      Value = NULL;
      if (line_read_string_expression (l, &Value) == FALSE)
      {
        WARN_SYNTAX_ERROR;
        return (l);
      }
      if (Value == NULL)
      {
        WARN_SYNTAX_ERROR;
        return (l);
      }
      if (My->CurrentFile->FileName != NULL)
      {
        free (My->CurrentFile->FileName);
        My->CurrentFile->FileName = NULL;
      }
      My->CurrentFile->FileName = Value;
      Value = NULL;
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
      return (l);
    }
    if (bwb_strcmp (My->CurrentFile->FileName, "*") != 0)
    {
      if ((My->CurrentFile->cfp =
           fopen (My->CurrentFile->FileName, "r")) == NULL)
      {
        WARN_BAD_FILE_NAME;
        return (l);
      }
      My->CurrentFile->DevMode = DEVMODE_INPUT;
    }
    /* OK */
    return (l);
  }
  WARN_SYNTAX_ERROR;
  return (l);
}

/* 
--------------------------------------------------------------------------------------------
                               DELIMIT
--------------------------------------------------------------------------------------------
*/
LineType *
bwb_DELIMIT (LineType * l)
{
  /*
     SYNTAX:     DELIMIT # X, A$
   */
   
  assert (l != NULL);
  assert( My != NULL );
  assert( My->SYSIN != NULL );

  if (line_skip_FilenumChar (l))
  {
    /* DELIMIT # */
    int FileNumber;
    char delimit;

    My->CurrentFile = My->SYSIN;

    if (line_read_integer_expression (l, &FileNumber) == FALSE)
    {
      WARN_BAD_FILE_NUMBER;
      return (l);
    }
    if (line_skip_seperator (l))
    {
      /* OK */
    }
    else
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    {
      char *Value;

      Value = NULL;
      if (line_read_string_expression (l, &Value) == FALSE)
      {
        WARN_SYNTAX_ERROR;
        return (l);
      }
      if (Value == NULL)
      {
        WARN_SYNTAX_ERROR;
        return (l);
      }
      delimit = Value[0];
      free (Value);
      Value = NULL;
      if (bwb_ispunct (delimit))
      {
        /* OK */
      }
      else
      {
        WARN_ILLEGAL_FUNCTION_CALL;
        return (l);
      }
    }
    if (FileNumber < 0)
    {
      /* "DELIMIT # -1" is SYSPRN */
      My->SYSPRN->delimit = delimit;
      return (l);
    }
    if (FileNumber == 0)
    {
      /* "DELIMIT # 0" is SYSOUT */
      My->SYSOUT->delimit = delimit;
      return (l);
    }
    /* normal file */
    My->CurrentFile = find_file_by_number (FileNumber);
    if (My->CurrentFile == NULL)
    {
      WARN_BAD_FILE_NUMBER;
      return (l);
    }
    My->CurrentFile->delimit = delimit;
    /* OK */
    return (l);
  }
  WARN_SYNTAX_ERROR;
  return (l);
}

/* 
--------------------------------------------------------------------------------------------
                               MARGIN
--------------------------------------------------------------------------------------------
*/
LineType *
bwb_MARGIN (LineType * l)
{
  /*
     SYNTAX:     MARGIN # X, Y
   */
  /* set width for OUTPUT  */
  int FileNumber;
  int Value;
   
  assert (l != NULL);
  assert( My != NULL );
  assert( My->SYSIN != NULL );

  if (line_skip_FilenumChar (l))
  {
    /* MARGIN # */
    My->CurrentFile = My->SYSIN;

    if (line_read_integer_expression (l, &FileNumber) == FALSE)
    {
      WARN_BAD_FILE_NUMBER;
      return (l);
    }
    if (line_skip_seperator (l))
    {
      /* OK */
    }
    else
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    if (line_read_integer_expression (l, &Value) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    if (Value < 0)
    {
      WARN_ILLEGAL_FUNCTION_CALL;
      return (l);
    }
    if (FileNumber < 0)
    {
      /* "MARGIN # -1" is SYSPRN */
      My->SYSPRN->width = Value;
      return (l);
    }
    if (FileNumber == 0)
    {
      /* "MARGIN # 0" is SYSOUT */
      My->SYSOUT->width = Value;
      return (l);
    }
    /* normal file */
    My->CurrentFile = find_file_by_number (FileNumber);
    if (My->CurrentFile == NULL)
    {
      WARN_BAD_FILE_NUMBER;
      return (l);
    }
    if ((My->CurrentFile->DevMode & DEVMODE_WRITE) == 0)
    {
      WARN_BAD_FILE_NUMBER;
      return (l);
    }
    My->CurrentFile->width = Value;
    /* OK */
    return (l);
  }
  WARN_SYNTAX_ERROR;
  return (l);
}

/* 
--------------------------------------------------------------------------------------------
                               USE
--------------------------------------------------------------------------------------------
*/

LineType *
bwb_USE (LineType * l)
{
  /*
     SYNTAX:     USE parameter$ ' CALL/360, System/360, System/370
   */
  VariableType *v;
   
  assert (l != NULL);
  assert( My != NULL );

  if ((v = line_read_scalar (l)) == NULL)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  if (v->VariableTypeCode != StringTypeCode)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  /* OK */
  if (My->UseParameterString)
  {
    VariantType variant;
    CLEAR_VARIANT (&variant);

    variant.VariantTypeCode = StringTypeCode;
    variant.Buffer = My->UseParameterString;
    variant.Length = bwb_strlen (My->UseParameterString);
    var_set (v, &variant);
  }
  return (l);
}

/* 
--------------------------------------------------------------------------------------------
                               CHAIN
--------------------------------------------------------------------------------------------
*/

LineType *
bwb_CHAIN (LineType * l)
{
  /*
     SYNTAX:     CHAIN file-name$ [, linenumber] ' most dialects
     SYNTAX:     CHAIN file-name$ [, parameter$] ' CALL/360, System/360, System/370
   */
  /* originally based upon bwb_load() */
  int LineNumber;
  LineType *x;
   
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
  assert( My->StartMarker != NULL );
  assert( My->EndMarker != NULL );

  /* Get an argument for filename */
  if (line_is_eol (l))
  {
    WARN_BAD_FILE_NAME;
    return (l);
  }
  else
  {
    char *Value;

    Value = NULL;
    if (line_read_string_expression (l, &Value) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    if (is_empty_string (Value))
    {
      WARN_BAD_FILE_NAME;
      return (l);
    }
    if (My->ProgramFilename != NULL)
    {
      free (My->ProgramFilename);
      My->ProgramFilename = NULL;
    }
    My->ProgramFilename = Value;
  }
  /* optional linenumber */
  LineNumber = 0;
  if (line_skip_seperator (l))
  {
    if (My->CurrentVersion->OptionVersionValue & (S70 | I70 | I73))
    {
      /* CHAIN filename$, parameter$ */
      {
        char *Value;

        Value = NULL;
        if (line_read_string_expression (l, &Value) == FALSE)
        {
          WARN_SYNTAX_ERROR;
          return (l);
        }
        if (Value == NULL)
        {
          WARN_SYNTAX_ERROR;
          return (l);
        }
        if (My->UseParameterString)
        {
          free (My->UseParameterString);
          My->UseParameterString = NULL;
        }
        My->UseParameterString = Value;
      }
    }
    else
    {
      /* CHAIN filename$, linenumber */
      if (line_read_integer_expression (l, &LineNumber) == FALSE)
      {
        WARN_SYNTAX_ERROR;
        return (l);
      }
      if (LineNumber < MINLIN || LineNumber > MAXLIN)
      {
        WARN_UNDEFINED_LINE;
        return (l);
      }
    }
  }

  /* deallocate all variables except common ones */
  var_delcvars ();

  /* remove old program from memory */
  bwb_xnew (My->StartMarker);

  /* load new program in memory */
  if (bwb_fload (NULL) == FALSE)
  {
    WARN_BAD_FILE_NAME;
    return (l);
  }
  /* FIXME */
  x = My->StartMarker;
  if (MINLIN <= LineNumber && LineNumber <= MAXLIN)
  {
    /* search for a matching line number */
    while (x->number != LineNumber && x != My->EndMarker)
    {
      x = x->next;
    }
    if (x == My->EndMarker)
    {
      /* NOT FOUND */
      x = My->StartMarker;
    }
  }
  x->position = 0;
  /*
   **
   ** FORCE SCAN
   **
   */
  if (bwb_scan () == FALSE)
  {
    WARN_CANT_CONTINUE;
    return (l);
  }
  /* reset all stack counters */
  bwb_clrexec ();
  if (bwb_incexec ())
  {
    /* OK */
    My->StackHead->line = x;
    My->StackHead->ExecCode = EXEC_NORM;
  }
  else
  {
    /* ERROR */
    WARN_OUT_OF_MEMORY;
    return My->EndMarker;
  }

  /* run the program */

  /* CHAIN */
  WARN_CLEAR;                        /* bwb_CHAIN */
  My->ContinueLine = NULL;
  SetOnError (0);
  return x;
}

/* 
--------------------------------------------------------------------------------------------
                               APPEND
--------------------------------------------------------------------------------------------
*/
LineType *
bwb_APPEND (LineType * l)
{
  /*
     SYNTAX:     APPEND # filenumber ' Dartmouth, Mark-I, Mark-II, GCOS
     SYNTAX:     APPEND  [filename$] ' all others
   */
   
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );

  if (My->CurrentVersion->OptionVersionValue & (D64 | G65 | G67 | G74))
  {
    if (line_skip_FilenumChar (l))
    {
      /* APPEND # filenumber */
      int FileNumber;

      if (line_read_integer_expression (l, &FileNumber) == FALSE)
      {
        WARN_SYNTAX_ERROR;
        return (l);
      }
      if (FileNumber < 0)
      {
        /* "APPEND # -1" is silently ignored */
        return (l);
      }
      if (FileNumber == 0)
      {
        /* "APPEND # 0" is silently ignored */
        return (l);
      }
      My->CurrentFile = find_file_by_number (FileNumber);
      if (My->CurrentFile == NULL)
      {
        WARN_BAD_FILE_NUMBER;
        return (l);
      }
      /* normal file */
      fseek (My->CurrentFile->cfp, 0, SEEK_END);
      My->CurrentFile->DevMode = DEVMODE_APPEND;
      /* OK */
      return (l);
    }
  }
  /* APPEND filename$ */
  return bwb_load (l, "APPEND FILE NAME:", FALSE);
}



/* 
--------------------------------------------------------------------------------------------
                               ON ERROR and so on
--------------------------------------------------------------------------------------------
*/

extern void
SetOnError (int LineNumber)
{
  /* scan the stack looking for a FUNCTION/SUB */
  StackType *StackItem;
  assert( My != NULL );
   

  if (My->StackHead == NULL)
  {
    return;
  }

  for (StackItem = My->StackHead; StackItem->next != NULL;
       StackItem = StackItem->next)
  {
    LineType *current;

    current = StackItem->LoopTopLine;
    if (current != NULL)
    {
      switch (current->cmdnum)
      {
      case C_FUNCTION:
      case C_SUB:
        /* FOUND */
        /* we are in a FUNCTION/SUB, so this is LOCAL */
        StackItem->OnErrorGoto = LineNumber;
        return;
        /* break; */
      }
    }
  }
  /* StackItem->next == NULL */
  /* NOT FOUND */
  /* we are NOT in a FUNCTION/SUB  */
  assert (StackItem != NULL);
  StackItem->OnErrorGoto = LineNumber;
}


extern int
GetOnError (void)
{
  /* scan the stack looking for an active "ON ERROR GOTO linenumber" */
  StackType *StackItem;
  assert( My != NULL );
   

  for (StackItem = My->StackHead; StackItem != NULL;
       StackItem = StackItem->next)
  {
    if (StackItem->OnErrorGoto != 0)
    {
      /* FOUND */
      return StackItem->OnErrorGoto;
    }
  }
  /* NOT FOUND */
  return 0;
}

/* 
--------------------------------------------------------------------------------------------
                               ON ERROR
--------------------------------------------------------------------------------------------
*/
LineType *
bwb_ON_ERROR (LineType * l)
{
   
  assert (l != NULL);
  WARN_SYNTAX_ERROR;
  return (l);
}

/* 
--------------------------------------------------------------------------------------------
                               ON ERROR GOTO
--------------------------------------------------------------------------------------------
*/
LineType *
bwb_ON_ERROR_GOTO (LineType * l)
{
  /* ON ERROR GOTO line */
  int LineNumber;
   
  assert (l != NULL);

  WARN_CLEAR;                        /* bwb_ON_ERROR_GOTO */

  /* get the line number */
  LineNumber = 0;
  if (line_is_eol (l))
  {
    /* ON ERROR GOTO */
    SetOnError (0);
    return (l);
  }
  if (line_read_integer_expression (l, &LineNumber) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  /* ON ERORR GOTO linenumber */
  if (LineNumber == 0)
  {
    /* ON ERROR GOTO 0 */
    SetOnError (0);
    return (l);
  }
  if (LineNumber < MINLIN || LineNumber > MAXLIN)
  {
    /* ERROR */
    WARN_UNDEFINED_LINE;
    return (l);
  }
  /* OK */
  SetOnError (LineNumber);
  return (l);
}

/* 
--------------------------------------------------------------------------------------------
                               ON ERROR GOSUB
--------------------------------------------------------------------------------------------
*/
LineType *
bwb_ON_ERROR_GOSUB (LineType * l)
{
  /* ON ERROR GOSUB line */
   
  assert (l != NULL);
  return bwb_ON_ERROR_GOTO (l);
}

/* 
--------------------------------------------------------------------------------------------
                               ON ERROR RESUME
--------------------------------------------------------------------------------------------
*/
LineType *
bwb_ON_ERROR_RESUME (LineType * l)
{
   
  assert (l != NULL);
  WARN_SYNTAX_ERROR;
  return (l);
}

/* 
--------------------------------------------------------------------------------------------
                               ON ERROR RESUME NEXT
--------------------------------------------------------------------------------------------
*/
LineType *
bwb_ON_ERROR_RESUME_NEXT (LineType * l)
{
   
  assert (l != NULL);
  WARN_CLEAR;                        /* bwb_ON_ERROR_RESUME_NEXT */
  SetOnError (-1);
  return (l);
}

/* 
--------------------------------------------------------------------------------------------
                               ON ERROR RETURN
--------------------------------------------------------------------------------------------
*/
LineType *
bwb_ON_ERROR_RETURN (LineType * l)
{
   
  assert (l != NULL);
  WARN_SYNTAX_ERROR;
  return (l);
}

/* 
--------------------------------------------------------------------------------------------
                               ON ERROR RETURN NEXT
--------------------------------------------------------------------------------------------
*/
LineType *
bwb_ON_ERROR_RETURN_NEXT (LineType * l)
{
   
  assert (l != NULL);
  return bwb_ON_ERROR_RESUME_NEXT (l);
}

/* 
--------------------------------------------------------------------------------------------
                               ON TIMER
--------------------------------------------------------------------------------------------
*/
LineType *
bwb_ON_TIMER (LineType * l)
{
  /* ON TIMER(...) GOSUB ... */
  DoubleType v;
  DoubleType minv;
  int LineNumber;
   
  assert (l != NULL);
  assert( My != NULL );

  My->IsTimerOn = FALSE;        /* bwb_ON_TIMER */
  My->OnTimerLineNumber = 0;
  My->OnTimerCount = 0;


  /* get the SECOMDS parameter */
  if (line_read_numeric_expression (l, &v) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }

  minv = 1;
  assert (CLOCKS_PER_SEC > 0);
  minv /= CLOCKS_PER_SEC;
  if (v < minv)
  {
    /* ERROR */
    WARN_ILLEGAL_FUNCTION_CALL;
    return (l);
  }

  /* get the GOSUB keyword */
  if (line_skip_word (l, "GOSUB") == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  /* ON TIMER(X) GOSUB line */
  if (line_read_integer_expression (l, &LineNumber) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  if (LineNumber < MINLIN || LineNumber > MAXLIN)
  {
    /* ERROR */
    WARN_UNDEFINED_LINE;
    return (l);
  }
  /* OK */
  My->OnTimerLineNumber = LineNumber;
  My->OnTimerCount = v;
  return (l);
}


/* 
--------------------------------------------------------------------------------------------
                               TIMER
--------------------------------------------------------------------------------------------
*/
LineType *
bwb_TIMER (LineType * l)
{
   
  assert (l != NULL);
  assert( My != NULL );

  My->IsTimerOn = FALSE;        /* bwb_TIMER */
  WARN_SYNTAX_ERROR;
  return (l);
}

/* 
--------------------------------------------------------------------------------------------
                               TIMER OFF
--------------------------------------------------------------------------------------------
*/
LineType *
bwb_TIMER_OFF (LineType * l)
{
   
  assert (l != NULL);
  assert( My != NULL );

  /* TIMER OFF */
  My->IsTimerOn = FALSE;        /* bwb_TIMER_OFF */
  My->OnTimerLineNumber = 0;
  My->OnTimerCount = 0;
  return (l);
}

/* 
--------------------------------------------------------------------------------------------
                               TIMER ON
--------------------------------------------------------------------------------------------
*/
LineType *
bwb_TIMER_ON (LineType * l)
{
   
  assert (l != NULL);
  assert( My != NULL );

  My->IsTimerOn = FALSE;        /* bwb_TIMER_ON */
  /* TIMER ON */
  if (My->OnTimerCount > 0 && My->OnTimerLineNumber > 0)
  {
    My->OnTimerExpires = bwx_TIMER (My->OnTimerCount);
    My->IsTimerOn = TRUE;        /* bwb_TIMER_ON */
  }
  return (l);
}

/* 
--------------------------------------------------------------------------------------------
                               TIMER STOP
--------------------------------------------------------------------------------------------
*/
LineType *
bwb_TIMER_STOP (LineType * l)
{
   
  assert (l != NULL);
  assert( My != NULL );

  My->IsTimerOn = FALSE;        /* bwb_TIMER_STOP */

  return (l);
}

/* 
--------------------------------------------------------------------------------------------
                               RESUME
--------------------------------------------------------------------------------------------
*/
LineType *
bwb_RESUME (LineType * l)
{
  int LineNumber;
  LineType *x;
   
  assert (l != NULL);
  assert( My != NULL );

  LineNumber = 0;
  x = My->ERL;                        /* bwb_RESUME */
  WARN_CLEAR;                        /* bwb_RESUME */

  if (l->LineFlags & (LINE_USER))
  {
    WARN_ILLEGAL_DIRECT;
    return (l);
  }

  if (x == NULL)
  {
    WARN_RESUME_WITHOUT_ERROR;
    return (l);
  }
  /* Get optional argument for RESUME */
  if (line_is_eol (l))
  {
    /* RESUME */
    /* 
       Execution resumes at the statement which caused the error 
       For structured commands, this is the top line of the structure.
     */
    x->position = 0;
    return x;
  }
  if (line_skip_word (l, "NEXT"))
  {
    /* RESUME NEXT */
    /* 
       Execution resumes at the statement immediately following the one which caused the error. 
       For structured commands, this is the bottom line of the structure.
     */
    switch (x->cmdnum)
    {
    case C_IF8THEN:
      /* skip to END_IF */
      assert (x->OtherLine != NULL);
      for (x = x->OtherLine; x->cmdnum != C_END_IF; x = x->OtherLine);
      break;
    case C_SELECT_CASE:
      /* skip to END_SELECT */
      assert (x->OtherLine != NULL);
      for (x = x->OtherLine; x->cmdnum != C_END_SELECT; x = x->OtherLine);
      break;
    default:
      x = x->next;
    }
    x->position = 0;
    return x;
  }
  /* RESUME ### */
  if (line_read_integer_expression (l, &LineNumber) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  if (LineNumber == 0)
  {
    /* SPECIAL CASE */
    /* RESUME 0 */
    /* Execution resumes at the statement which caused the error */
    x->position = 0;
    return x;
  }
  /* VERIFY LINE EXISTS */
  x = find_line_number (LineNumber);        /* RESUME 100 */
  if (x != NULL)
  {
    /* FOUND */
    x->position = 0;
    return x;
  }
  /* NOT FOUND */
  WARN_UNDEFINED_LINE;
  return (l);
}

/* 
--------------------------------------------------------------------------------------------
                              CMDS
--------------------------------------------------------------------------------------------
*/
LineType *
bwb_CMDS (LineType * l)
{
  int n;
  int t;
   
  assert (l != NULL);
  assert( My != NULL );
  assert( My->SYSOUT != NULL );
  assert( My->SYSOUT->cfp != NULL );

  My->CurrentFile = My->SYSOUT;
  fprintf (My->SYSOUT->cfp, "BWBASIC COMMANDS AVAILABLE:\n");

  /* run through the command table and print comand names */

  t = 0;
  for (n = 0; n < NUM_COMMANDS; n++)
  {
    fprintf (My->SYSOUT->cfp, IntrinsicCommandTable[n].name);
    if (t < 4)
    {
      fprintf (My->SYSOUT->cfp, "\t");
      t++;
    }
    else
    {
      fprintf (My->SYSOUT->cfp, "\n");
      t = 0;
    }
  }
  if (t > 0)
  {
    fprintf (My->SYSOUT->cfp, "\n");
  }
  ResetConsoleColumn ();
  return (l);
}

static void
FixUp (char *Name)
{
  char *C;
   
  assert (Name != NULL);

  C = Name;
  while (*C)
  {
    if (bwb_isalnum (*C))
    {
      /* OK */
    }
    else
    {
      /* FIX */
      switch (*C)
      {
      case '!':
        *C = '1';
        break;
      case '@':
        *C = '2';
        break;
      case '#':
        *C = '3';
        break;
      case '$':
        *C = '4';
        break;
      case '%':
        *C = '5';
        break;
      case '^':
        *C = '6';
        break;
      case '&':
        *C = '7';
        break;
      case '*':
        *C = '8';
        break;
      case '(':
        *C = '9';
        break;
      case ')':
        *C = '0';
        break;
      default:
        *C = '_';
      }
    }
    C++;
  }
}


static void
CommandUniqueID (int i, char *UniqueID)
{
   
  assert (UniqueID != NULL);

  bwb_strcpy (UniqueID, "C_");
  bwb_strcat (UniqueID, IntrinsicCommandTable[i].name);
  FixUp (UniqueID);
}

static void
CommandVector (int i, char *Vector)
{
   
  assert (Vector != NULL);

  bwb_strcpy (Vector, "bwb_");
  bwb_strcat (Vector, IntrinsicCommandTable[i].name);
  FixUp (Vector);
}

static void
CommandOptionVersion (int n, char *OutputLine)
{
  int i;
  int j;
   
  assert (OutputLine != NULL);

  bwb_strcpy (OutputLine, "");
  j = 0;
  for (i = 0; i < NUM_VERSIONS; i++)
  {
    if (IntrinsicCommandTable[n].OptionVersionBitmask & bwb_vertable[i].
        OptionVersionValue)
    {
      if (j > 0)
      {
        bwb_strcat (OutputLine, " | ");
      }
      bwb_strcat (OutputLine, bwb_vertable[i].ID);
      j++;
    }
  }
}


void
SortAllCommands (void)
{
  /* sort by name */
  int i;
  assert( My != NULL );
   

  for (i = 0; i < NUM_COMMANDS - 1; i++)
  {
    int j;
    int k;
    k = i;
    for (j = i + 1; j < NUM_COMMANDS; j++)
    {
      if (bwb_stricmp
          (IntrinsicCommandTable[j].name, IntrinsicCommandTable[k].name) < 0)
      {
        k = j;
      }
    }
    if (k > i)
    {
      CommandType t;
      bwb_memcpy (&t, &(IntrinsicCommandTable[i]), sizeof (CommandType));
      bwb_memcpy (&(IntrinsicCommandTable[i]), &(IntrinsicCommandTable[k]),
                  sizeof (CommandType));
      bwb_memcpy (&(IntrinsicCommandTable[k]), &t, sizeof (CommandType));
    }
  }
#if THE_PRICE_IS_RIGHT
  for (i = 0; i < 26; i++)
  {
    My->CommandStart[i] = -1;
  }
  for (i = 0; i < NUM_COMMANDS; i++)
  {
    int j;
    j = VarTypeIndex (IntrinsicCommandTable[i].name[0]);
    if (j < 0)
    {
      /* non-alpha */
    }
    else if (My->CommandStart[j] < 0)
    {
      /* this is the first command starting with this letter */
      My->CommandStart[j] = i;
    }
  }
#endif /* THE_PRICE_IS_RIGHT */
}

void
SortAllFunctions (void)
{
  /* sort by name then number of parameters */
  int i;
  assert( My != NULL );
   

  for (i = 0; i < NUM_FUNCTIONS - 1; i++)
  {
    int j;
    int k;
    k = i;
    for (j = i + 1; j < NUM_FUNCTIONS; j++)
    {
      int n;
      n =
        bwb_stricmp (IntrinsicFunctionTable[j].Name,
                     IntrinsicFunctionTable[k].Name);
      if (n < 0)
      {
        k = j;
      }
      else if (n == 0)
      {
        if (IntrinsicFunctionTable[j].ParameterCount <
            IntrinsicFunctionTable[k].ParameterCount)
        {
          k = j;
        }
      }
    }
    if (k > i)
    {
      IntrinsicFunctionType t;
      bwb_memcpy (&t, &(IntrinsicFunctionTable[i]),
                  sizeof (IntrinsicFunctionType));
      bwb_memcpy (&(IntrinsicFunctionTable[i]), &(IntrinsicFunctionTable[k]),
                  sizeof (IntrinsicFunctionType));
      bwb_memcpy (&(IntrinsicFunctionTable[k]), &t,
                  sizeof (IntrinsicFunctionType));
    }
  }
#if THE_PRICE_IS_RIGHT
  for (i = 0; i < 26; i++)
  {
    My->IntrinsicFunctionStart[i] = -1;
  }
  for (i = 0; i < NUM_FUNCTIONS; i++)
  {
    int j;
    j = VarTypeIndex (IntrinsicFunctionTable[i].Name[0]);
    if (j < 0)
    {
      /* non-alpha */
    }
    else if (My->IntrinsicFunctionStart[j] < 0)
    {
      /* this is the first command starting with this letter */
      My->IntrinsicFunctionStart[j] = i;
    }
  }
#endif /* THE_PRICE_IS_RIGHT */
}


void
DumpAllCommandUniqueID (FILE * file)
{
  int i;
  int j;
  char LastUniqueID[NameLengthMax + 1];
   
  assert (file != NULL);

  j = 0;
  LastUniqueID[0] = NulChar;

  fprintf (file, "/* COMMANDS */\n");

  /* run through the command table and print comand #define */

  for (i = 0; i < NUM_COMMANDS; i++)
  {
    char UniqueID[NameLengthMax + 1];

    CommandUniqueID (i, UniqueID);
    if (bwb_stricmp (LastUniqueID, UniqueID) != 0)
    {
      /* not a duplicate */
      bwb_strcpy (LastUniqueID, UniqueID);
      j = j + 1;
      fprintf (file, "#define %-30s %3d /* %-30s */\n", UniqueID, j,
               IntrinsicCommandTable[i].name);
    }
  }
  fprintf (file, "#define NUM_COMMANDS %d\n", j);
  fflush (file);
}

static void
ProcessEscapeChars (const char *Input, char *Output)
{
  int n;
   
  assert (Input != NULL);
  assert (Output != NULL);

  n = 0;

  while (*Input)
  {
    /* \a  \b  \f  \n  \r  \t  \v \" \\ */
    switch (*Input)
    {
    case '\a':
      *Output = '\\';
      Output++;
      *Output = 'a';
      Output++;
      break;
    case '\b':
      *Output = '\\';
      Output++;
      *Output = 'b';
      Output++;
      break;
    case '\f':
      *Output = '\\';
      Output++;
      *Output = 'f';
      Output++;
      break;
    case '\n':
      *Output = '\\';
      Output++;
      *Output = 'n';
      Output++;
      break;
    case '\r':
      *Output = '\\';
      Output++;
      *Output = 'r';
      Output++;
      break;
    case '\t':
      *Output = '\\';
      Output++;
      *Output = 't';
      Output++;
      break;
    case '\v':
      *Output = '\\';
      Output++;
      *Output = 'n';
      Output++;
      break;
    case '\"':
      *Output = '\\';
      Output++;
      *Output = '"';
      Output++;
      break;
    case '\\':
      *Output = '\\';
      Output++;
      *Output = '\\';
      Output++;
      break;
    default:
      *Output = *Input;
      Output++;
      break;
    }
    *Output = NulChar;
    n++;
    if (n > 60 && *Input == ' ')
    {
      *Output = '\"';
      Output++;
      *Output = '\n';
      Output++;
      *Output = ' ';
      Output++;
      *Output = ' ';
      Output++;
      *Output = '\"';
      Output++;
      *Output = NulChar;
      n = 0;
    }
    Input++;
  }
}

void
DumpAllCommandTableDefinitions (FILE * file)
{
  /* generate bwd_cmd.c */

  int i;
   
  assert (file != NULL);

  fprintf (file, "/* COMMAND TABLE */\n\n");
  fprintf (file, "#include \"bwbasic.h\"\n\n");
  fprintf (file,
           "CommandType IntrinsicCommandTable[ /* NUM_COMMANDS */ ] =\n");
  fprintf (file, "{\n");

  /* run through the command table and print comand #define */

  for (i = 0; i < NUM_COMMANDS; i++)
  {
    char tbuf[MAINTAINER_BUFFER_LENGTH + 1];        /* DumpAllCommandTableDefinitions */


    fprintf (file, "{\n");

    fprintf (file, "  ");
    CommandUniqueID (i, tbuf);
    fprintf (file, tbuf);
    fprintf (file, ",  /* UniqueID */\n");

    fprintf (file, "  ");
    fprintf (file, "\"");
    ProcessEscapeChars (IntrinsicCommandTable[i].Syntax, tbuf);
    fprintf (file, tbuf);
    fprintf (file, "\"");
    fprintf (file, ",  /* Syntax */\n");

    fprintf (file, "  ");
    fprintf (file, "\"");
    ProcessEscapeChars (IntrinsicCommandTable[i].Description, tbuf);
    fprintf (file, tbuf);
    fprintf (file, "\"");
    fprintf (file, ",  /* Description */\n");

    fprintf (file, "  ");
    fprintf (file, "\"");
    fprintf (file, IntrinsicCommandTable[i].name);
    fprintf (file, "\"");
    fprintf (file, ",  /* Name */\n");

    fprintf (file, "  ");
    CommandOptionVersion (i, tbuf);
    fprintf (file, tbuf);
    fprintf (file, "  /* OptionVersionBitmask */\n");

    fprintf (file, "},\n");
  }
  fprintf (file, "};\n");
  fprintf (file, "\n");
  fprintf (file,
           "const size_t NUM_COMMANDS = sizeof( IntrinsicCommandTable ) / sizeof( CommandType );\n");
  fprintf (file, "\n");
  fflush (file);
}

void
DumpAllCommandSwitchStatement (FILE * file)
{
  int i;
  char LastUniqueID[NameLengthMax + 1];
   
  assert (file != NULL);

  LastUniqueID[0] = NulChar;

  /* run through the command table and print comand #define */
  fprintf (file, "/* SWITCH */\n");
  fprintf (file, "LineType *bwb_vector( LineType *l )\n");
  fprintf (file, "{\n");

  fprintf (file, "   ");
  fprintf (file, "LineType *r;\n");

  fprintf (file, "   ");
  fprintf (file, "switch( l->cmdnum )\n");

  fprintf (file, "   ");
  fprintf (file, "{\n");

  for (i = 0; i < NUM_COMMANDS; i++)
  {
    char tbuf[NameLengthMax + 1];

    CommandUniqueID (i, tbuf);
    if (bwb_stricmp (LastUniqueID, tbuf) != 0)
    {
      /* not a duplicate */
      bwb_strcpy (LastUniqueID, tbuf);

      fprintf (file, "   ");
      fprintf (file, "case ");
      CommandUniqueID (i, tbuf);
      fprintf (file, tbuf);
      fprintf (file, ":\n");

      fprintf (file, "   ");
      fprintf (file, "   ");
      fprintf (file, "r = ");
      CommandVector (i, tbuf);
      fprintf (file, tbuf);
      fprintf (file, "( l );\n");

      fprintf (file, "   ");
      fprintf (file, "   ");
      fprintf (file, "break;\n");
    }
  }

  fprintf (file, "   ");
  fprintf (file, "default:\n");

  fprintf (file, "   ");
  fprintf (file, "   ");
  fprintf (file, "WARN_INTERNAL_ERROR;\n");

  fprintf (file, "   ");
  fprintf (file, "   ");
  fprintf (file, "r = l;\n");

  fprintf (file, "   ");
  fprintf (file, "   ");
  fprintf (file, "break;\n");


  fprintf (file, "   ");
  fprintf (file, "}\n");

  fprintf (file, "   ");
  fprintf (file, "return r;\n");

  fprintf (file, "}\n");

  fflush (file);
}

void
FixDescription (FILE * file, const char *left, const char *right)
{
  char buffer[MAINTAINER_BUFFER_LENGTH + 1];        /* FixDescription */
  int l;                        /* length of left side */
  int p;                        /* current position */
  int n;                        /* position of the last space character, zero means none yet seen */
  int i;                        /* number of characters since last '\n' */
   
  assert (left != NULL);
  assert (right != NULL);

  l = bwb_strlen (left);
  p = 0;
  n = 0;
  i = 0;
  bwb_strcpy (buffer, right);

  while (buffer[p])
  {
    if (buffer[p] == '\n')
    {
      n = p;
      i = 0;
    }
    if (buffer[p] == ' ')
    {
      n = p;
    }
    if (i > 45 && n > 0)
    {
      buffer[n] = '\n';
      i = p - n;
    }
    p++;
    i++;
  }
  fputs (left, file);
  p = 0;
  while (buffer[p])
  {
    if (buffer[p] == '\n')
    {
      fputc (buffer[p], file);
      p++;
      while (buffer[p] == ' ')
      {
        p++;
      }
      for (i = 0; i < l; i++)
      {
        fputc (' ', file);
      }
    }
    else
    {
      fputc (buffer[p], file);
      p++;
    }
  }
  fputc ('\n', file);

}

void
DumpOneCommandSyntax (FILE * file, int IsXref, int n)
{
   
  assert (file != NULL);

  if (n < 0 || n >= NUM_COMMANDS)
  {
    return;
  }
  /* NAME */
  {
    FixDescription (file, "     SYNTAX: ", IntrinsicCommandTable[n].Syntax);
  }
  /* DESCRIPTION */
  {
    FixDescription (file, "DESCRIPTION: ",
                    IntrinsicCommandTable[n].Description);
  }
  /* COMPATIBILITY */
  if (IsXref)
  {
    int i;
    fprintf (file, "   VERSIONS:\n");
    for (i = 0; i < NUM_VERSIONS; i++)
    {
      char X;
      if (IntrinsicCommandTable[n].OptionVersionBitmask & bwb_vertable[i].
          OptionVersionValue)
      {
        /* SUPPORTED */
        X = 'X';
      }
      else
      {
        /* NOT SUPPORTED */
        X = '_';
      }
      fprintf (file, "             [%c] %s\n", X, bwb_vertable[i].Name);
    }
  }

  fflush (file);
}

void
DumpAllCommandSyntax (FILE * file, int IsXref,
                      OptionVersionType OptionVersionValue)
{
  /* for the C maintainer */
  int i;
   
  assert (file != NULL);

  fprintf (file,
           "============================================================\n");
  fprintf (file,
           "                    COMMANDS                                \n");
  fprintf (file,
           "============================================================\n");
  fprintf (file, "\n");
  fprintf (file, "\n");
  for (i = 0; i < NUM_COMMANDS; i++)
  {
    if (IntrinsicCommandTable[i].OptionVersionBitmask & OptionVersionValue)
    {
      fprintf (file,
               "------------------------------------------------------------\n");
      DumpOneCommandSyntax (file, IsXref, i);
    }

  }
  fprintf (file,
           "------------------------------------------------------------\n");


  fprintf (file, "\n");
  fprintf (file, "\n");
  fflush (file);
}


void
DumpAllCommandHtmlTable (FILE * file)
{
  /* generate bwd_cmd.htm */

  int i;
  int j;
   
  assert (file != NULL);


  /* LEGEND */
  fprintf (file, "<html><head><title>CMDS</title></head><body>\n");
  fprintf (file, "<h1>LEGEND</h1><br>\n");
  fprintf (file, "<table>\n");

  fprintf (file, "<tr>");
  fprintf (file, "<td>");
  fprintf (file, "<b>");
  fprintf (file, "ID");
  fprintf (file, "</b>");
  fprintf (file, "</td>");
  fprintf (file, "<td>");
  fprintf (file, "<b>");
  fprintf (file, "NAME");
  fprintf (file, "</b>");
  fprintf (file, "</td>");
  fprintf (file, "<td>");
  fprintf (file, "<b>");
  fprintf (file, "DESCRIPTION");
  fprintf (file, "</b>");
  fprintf (file, "</td>");
  fprintf (file, "</tr>\n");

  for (j = 0; j < NUM_VERSIONS; j++)
  {
    fprintf (file, "<tr>");
    fprintf (file, "<td>");
    fprintf (file, bwb_vertable[j].ID);
    fprintf (file, "</td>");
    fprintf (file, "<td>");
    fprintf (file, bwb_vertable[j].Name);
    fprintf (file, "</td>");
    fprintf (file, "<td>");
    fprintf (file, bwb_vertable[j].Description);
    fprintf (file, "</td>");
    fprintf (file, "</tr>\n");
  }
  fprintf (file, "</table>\n");
  fprintf (file, "<hr>\n");


  /* DETAILS */
  fprintf (file, "<h1>DETAILS</h1><br>\n");
  fprintf (file, "<table>\n");

  fprintf (file, "<tr>");
  fprintf (file, "<td>");
  fprintf (file, "<b>");
  fprintf (file, "COMMAND");
  fprintf (file, "</b>");
  fprintf (file, "</td>");
  for (j = 0; j < NUM_VERSIONS; j++)
  {
    fprintf (file, "<td>");
    fprintf (file, "<b>");
    fprintf (file, bwb_vertable[j].ID);
    fprintf (file, "</b>");
    fprintf (file, "</td>");
  }
  fprintf (file, "</tr>\n");


  /* run through the command table and print comand -vs- OPTION VERSION */

  for (i = 0; i < NUM_COMMANDS; i++)
  {
    fprintf (file, "<tr>");
    fprintf (file, "<td>");
    fprintf (file, (char *) IntrinsicCommandTable[i].Syntax);
    fprintf (file, "</td>");

    for (j = 0; j < NUM_VERSIONS; j++)
    {
      fprintf (file, "<td>");
      if (IntrinsicCommandTable[i].OptionVersionBitmask & bwb_vertable[j].
          OptionVersionValue)
      {
        fprintf (file, "X");
      }
      else
      {
        fprintf (file, " ");
      }
      fprintf (file, "</td>");
    }
    fprintf (file, "</tr>\n");
  }
  fprintf (file, "</table>\n");
  fprintf (file, "</body></html>\n");
  fprintf (file, "\n");

  fflush (file);
}

/* 
--------------------------------------------------------------------------------------------
                               HELP
--------------------------------------------------------------------------------------------
*/
LineType *
bwb_HELP (LineType * l)
{
  /* HELP ... */
  int n;
  int Found;
  char *C;
  char *tbuf;
   
  assert (l != NULL);
  assert( My != NULL );
  assert( My->ConsoleInput != NULL );
  assert( My->SYSOUT != NULL );
  assert( My->SYSOUT->cfp != NULL );

  tbuf = My->ConsoleInput;
  Found = FALSE;

  C = l->buffer;
  C += l->position;
  bwb_strcpy (tbuf, C);

  /* RTRIM$ */
  C = tbuf;
  if (*C != 0)
  {
    /* not an empty line, so remove one (or more) trailing spaces */
    char *E;

    E = bwb_strchr (tbuf, 0);
    E--;
    while (E >= tbuf && *E == ' ')
    {
      *E = 0;
      E--;
    }
  }


  /* EXACT match */
  for (n = 0; n < NUM_COMMANDS; n++)
  {
    if (bwb_stricmp (IntrinsicCommandTable[n].name, tbuf) == 0)
    {
      fprintf (My->SYSOUT->cfp,
               "------------------------------------------------------------\n");
      DumpOneCommandSyntax (My->SYSOUT->cfp, FALSE, n);
      Found = TRUE;
    }
  }

  for (n = 0; n < NUM_FUNCTIONS; n++)
  {
    if (bwb_stricmp (IntrinsicFunctionTable[n].Name, tbuf) == 0)
    {
      fprintf (My->SYSOUT->cfp,
               "------------------------------------------------------------\n");
      DumpOneFunctionSyntax (My->SYSOUT->cfp, FALSE, n);
      Found = TRUE;
    }
  }

  if (Found == FALSE)
  {
    /* PARTIAL match */
    int Length;
    Length = bwb_strlen (tbuf);

    for (n = 0; n < NUM_COMMANDS; n++)
    {
      if (bwb_strnicmp (IntrinsicCommandTable[n].name, tbuf, Length) == 0)
      {
        if (Found == FALSE)
        {
          fprintf (My->SYSOUT->cfp,
                   "The following topics are a partial match:\n");
        }
        fprintf (My->SYSOUT->cfp, IntrinsicCommandTable[n].name);
        fprintf (My->SYSOUT->cfp, "\t");
        Found = TRUE;
      }
    }

    for (n = 0; n < NUM_FUNCTIONS; n++)
    {
      if (bwb_strnicmp (IntrinsicFunctionTable[n].Name, tbuf, Length) == 0)
      {
        if (Found == FALSE)
        {
          fprintf (My->SYSOUT->cfp,
                   "The following topics are a partial match:\n");
        }
        fprintf (My->SYSOUT->cfp, IntrinsicFunctionTable[n].Name);
        fprintf (My->SYSOUT->cfp, "\t");
        Found = TRUE;
      }
    }
    if (Found == TRUE)
    {
      /* match */
      fprintf (My->SYSOUT->cfp, "\n");
    }
  }
  if (Found == FALSE)
  {
    /* NO match */
    fprintf (My->SYSOUT->cfp, "No help found.\n");
  }
  ResetConsoleColumn ();
  line_skip_eol (l);
  return (l);

}

int
NumberValueCheck (ParamTestType ParameterTests, DoubleType X)
{
  DoubleType XR;                /* rounded value */
  unsigned char TestNibble;
   


  /* VerifyNumeric */
  if (isnan (X))
  {
    /* INTERNAL ERROR */
    return -1;
  }
  if (isinf (X))
  {
    /* - Evaluation of an expression results in an overflow
     * (nonfatal, the recommended recovery procedure is to supply
     * machine in- finity with the algebraically correct sign and
     * continue). */
    if (X < 0)
    {
      X = MINDBL;
    }
    else
    {
      X = MAXDBL;
    }
    if (WARN_OVERFLOW)
    {
      /* ERROR */
      return -1;
    }
    /* CONTINUE */
  }
  /* OK */
  /* VALID NUMERIC VALUE */
  XR = bwb_rint (X);
  ParameterTests &= 0x0000000F;
  TestNibble = (unsigned char) ParameterTests;
  switch (TestNibble)
  {
  case P1ERR:
    /* INTERNAL ERROR */
    return -1;
    /* break; */
  case P1ANY:
    if (X < MINDBL || X > MAXDBL)
    {
      /* ERROR */
      return -1;
    }
    /* OK */
    return 0;
    /* break; */
  case P1BYT:
    if (XR < MINBYT || XR > MAXBYT)
    {
      /* ERROR */
      return -1;
    }
    /* OK */
    return 0;
    /* break; */
  case P1INT:
    if (XR < MININT || XR > MAXINT)
    {
      /* ERROR */
      return -1;
    }
    /* OK */
    return 0;
    /* break; */
  case P1LNG:
    if (XR < MINLNG || XR > MAXLNG)
    {
      /* ERROR */
      return -1;
    }
    /* OK */
    return 0;
    /* break; */
  case P1CUR:
    if (XR < MINCUR || XR > MAXCUR)
    {
      /* ERROR */
      return -1;
    }
    /* OK */
    return 0;
    /* break; */
  case P1FLT:
    if (X < MINSNG || X > MAXSNG)
    {
      /* ERROR */
      return -1;
    }
    /* OK */
    return 0;
    /* break; */
  case P1DBL:
    if (X < MINDBL || X > MAXDBL)
    {
      /* ERROR */
      return -1;
    }
    /* OK */
    return 0;
    /* break; */
  case P1DEV:
    /* ERROR */
    return -1;
    /* break; */
  case P1LEN:
    if (XR < MINLEN || XR > MAXLEN)
    {
      /* ERROR */
      return -1;
    }
    /* OK */
    return 0;
    /* break; */
  case P1POS:
    if (XR < 1 || XR > MAXLEN)
    {
      /* ERROR */
      return -1;
    }
    /* OK */
    return 0;
    /* break; */
  case P1COM:
    /* ERROR */
    return -1;
    /* break; */
  case P1LPT:
    /* ERROR */
    return -1;
    /* break; */
  case P1GTZ:
    if (X > 0)
    {
      /* OK */
      return 0;
    }
    break;
  case P1GEZ:
    if (X >= 0)
    {
      /* OK */
      return 0;
    }
    break;
  case P1NEZ:
    if (X != 0)
    {
      /* OK */
      return 0;
    }
    break;
  }
  /* ERROR */
  return -1;
}

int
StringLengthCheck (ParamTestType ParameterTests, int s)
{
  unsigned char TestNibble;
   

  /* check for invalid string length */
  if (s < 0 || s > MAXLEN)
  {
    /* INTERNAL ERROR */
    return -1;
  }
  /* VALID STRING LENGTH */
  ParameterTests &= 0x0000000F;
  TestNibble = (unsigned char) ParameterTests;
  switch (TestNibble)
  {
  case P1ERR:
    /* INTERNAL ERROR */
    return -1;
    /* break; */
  case P1ANY:
    /* OK */
    return 0;
    /* break; */
  case P1BYT:
    if (s >= sizeof (ByteType))
    {
      /* OK */
      return 0;
    }
    break;
  case P1INT:
    if (s >= sizeof (IntegerType))
    {
      /* OK */
      return 0;
    }
    break;
  case P1LNG:
    if (s >= sizeof (LongType))
    {
      /* OK */
      return 0;
    }
    break;
  case P1CUR:
    if (s >= sizeof (CurrencyType))
    {
      /* OK */
      return 0;
    }
    break;
  case P1FLT:
    if (s >= sizeof (SingleType))
    {
      /* OK */
      return 0;
    }
    break;
  case P1DBL:
    if (s >= sizeof (DoubleType))
    {
      /* OK */
      return 0;
    }
    break;
  case P1DEV:
    /* ERROR */
    return -1;
    /* break; */
  case P1LEN:
    /* ERROR */
    return -1;
    /* break; */
  case P1POS:
    /* ERROR */
    return -1;
    /* break; */
  case P1GEZ:
    /* ERROR */
    return -1;
    /* break; */
  case P1GTZ:
    /* ERROR */
    return -1;
    /* break; */
  case P1NEZ:
    /* ERROR */
    return -1;
    /* break; */
  }
  /* ERROR */
  return -1;
}

void
IntrinsicFunctionDefinitionCheck (IntrinsicFunctionType * f)
{
  /* function definition check -- look for obvious errors */
   
  assert (f != NULL);
  assert( My != NULL );
  assert( My->SYSOUT != NULL );
  assert( My->SYSOUT->cfp != NULL );


  /* sanity check */
  if (f->ParameterCount == PNONE || f->ParameterCount == 0xFF)
  {
    /* function has NO explicit parameters */
    if (f->ParameterTypes == PNONE)
    {
      /* OK */
    }
    else
    {
      /* oops */
      fprintf (My->SYSOUT->cfp, "invalid ParameterTypes <%s>\n", f->Name);
    }
    if (f->ParameterTests == PNONE)
    {
      /* OK */
    }
    else
    {
      /* oops */
      fprintf (My->SYSOUT->cfp, "invalid ParameterTests <%s>\n", f->Name);
    }
  }
  else
  {
    /* function HAS an explicit number of parameters */
    int i;
    ParamTestType ParameterTests;

    ParameterTests = f->ParameterTests;
    for (i = 0; i < f->ParameterCount; i++)
    {
      /* sanity check this parameter */
      ParamTestType thischeck;
      thischeck = ParameterTests & 0x0000000F;
      /* verify parameter check  */
      if (f->ParameterTypes & (1 << i))
      {
        /* STRING */
        if (thischeck >= P1ANY && thischeck <= P1DBL)
        {
          /* OK */
        }
        else
        {
          /* oops */
          fprintf (My->SYSOUT->cfp,
                   "invalid ParameterTests <%s> parameter %d\n", f->Name,
                   i + 1);
        }
      }
      else
      {
        /* NUMBER */
        if (thischeck >= P1ANY && thischeck <= P1NEZ)
        {
          /* OK */
        }
        else
        {
          /* oops */
          fprintf (My->SYSOUT->cfp,
                   "invalid ParameterTests <%s> parameter %d\n", f->Name,
                   i + 1);
        }
      }
      ParameterTests = ParameterTests >> 4;
    }
    if (ParameterTests != 0)
    {
      /* oops */
      fprintf (My->SYSOUT->cfp, "invalid ParameterTests <%s> parameter %d\n",
               f->Name, i + 1);
    }
  }
}

void
IntrinsicFunctionUniqueID (IntrinsicFunctionType * f, char *UniqueID)
{
  /* generate the function's UniqueID */
  /* manual fixup required for duplicates */
  char NumVar;
  char StrVar;
   
  assert (f != NULL);
  assert (UniqueID != NULL);


  NumVar = 'X';
  StrVar = 'A';

  /* F_  */
  bwb_strcpy (UniqueID, "F_");
  /* NAME  */
  bwb_strcat (UniqueID, f->Name);
  /* PARAMETERS */
  if (f->ParameterCount == PNONE || f->ParameterCount == 0xFF)
  {
    /* function has NO explicit parameters */
  }
  else
  {
    /* function HAS explicit parameters */
    int i;
    ParamBitsType ParameterTypes;
    ParameterTypes = f->ParameterTypes;
    for (i = 0; i < f->ParameterCount; i++)
    {
      char VarName[NameLengthMax + 1];
      if (ParameterTypes & 1)
      {
        /* STRING */
        sprintf (VarName, "_%c", StrVar);
        StrVar++;
      }
      else
      {
        /* NUMBER */
        sprintf (VarName, "_%c", NumVar);
        NumVar++;
      }
      bwb_strcat (UniqueID, VarName);
      ParameterTypes = ParameterTypes >> 1;
    }
  }
  /* RETURN TYPE */
  if (f->ReturnTypeCode == StringTypeCode)
  {
    bwb_strcat (UniqueID, "_S");
  }
  else
  {
    bwb_strcat (UniqueID, "_N");
  }
  /* fixup illegal characters,  "DEF FN" "BLOAD:", "CLOAD*" */
  FixUp (UniqueID);
}


void
IntrinsicFunctionSyntax (IntrinsicFunctionType * f, char *Syntax)
{
  /* generate the function's Syntax */
  char NumVar;
  char StrVar;
   
  assert (f != NULL);
  assert (Syntax != NULL);


  NumVar = 'X';
  StrVar = 'A';

  /* RETURN TYPE */
  if (f->ReturnTypeCode == StringTypeCode)
  {
    bwb_strcpy (Syntax, "S$ = ");
  }
  else
  {
    bwb_strcpy (Syntax, "N  = ");
  }
  /* NAME  */
  bwb_strcat (Syntax, f->Name);
  /* PARAMETERS */
  if (f->ParameterCount == PNONE)
  {
    /* function has NO explicit parameters */
  }
  else if (f->ParameterCount == 0xFF)
  {
    /* function has a variable number of parameters */
    bwb_strcat (Syntax, "( ... )");
  }
  else
  {
    /* function HAS explicit parameters */
    int i;
    ParamBitsType ParameterTypes;
    ParameterTypes = f->ParameterTypes;

    if (f->ReturnTypeCode == StringTypeCode)
    {
      bwb_strcat (Syntax, "( ");
    }
    else
    {
      bwb_strcat (Syntax, "( ");
    }

    for (i = 0; i < f->ParameterCount; i++)
    {
      char VarName[NameLengthMax + 1];
      if (i > 0)
      {
        bwb_strcat (Syntax, ", ");
      }
      /* verify parameter check  */
      if (ParameterTypes & 1)
      {
        /* STRING */
        sprintf (VarName, "%c$", StrVar);
        StrVar++;
      }
      else
      {
        /* NUMBER */
        sprintf (VarName, "%c", NumVar);
        NumVar++;
      }
      bwb_strcat (Syntax, VarName);
      ParameterTypes = ParameterTypes >> 1;
    }
    if (f->ReturnTypeCode == StringTypeCode)
    {
      bwb_strcat (Syntax, " )");
    }
    else
    {
      bwb_strcat (Syntax, " )");
    }
  }
}

void
DumpAllFunctionUniqueID (FILE * file)
{
  /* for the C maintainer */
  int i;
  int j;
  char LastUniqueID[NameLengthMax + 1];
   
  assert (file != NULL);

  j = 0;
  LastUniqueID[0] = NulChar;

  fprintf (file, "/* FUNCTIONS */\n");
  for (i = 0; i < NUM_FUNCTIONS; i++)
  {
    char UniqueID[NameLengthMax + 1];

    IntrinsicFunctionUniqueID (&(IntrinsicFunctionTable[i]), UniqueID);
    if (bwb_stricmp (LastUniqueID, UniqueID) != 0)
    {
      /* not a duplicate */
      char Syntax[NameLengthMax + 1];

      bwb_strcpy (LastUniqueID, UniqueID);
      j = j + 1;
      IntrinsicFunctionSyntax (&(IntrinsicFunctionTable[i]), Syntax);
      fprintf (file, "#define %-30s %3d /* %-30s */\n", UniqueID, j, Syntax);
    }
  }
  fprintf (file, "#define NUM_FUNCTIONS %d\n", j);
  fflush (file);
}

void
DumpAllFunctionSwitch (FILE * file)
{
  /* for the C maintainer */
  int i;
   
  assert (file != NULL);


  fprintf (file, "/* SWITCH */\n");
  fprintf (file, "switch( UniqueID )\n");
  fprintf (file, "{\n");
  for (i = 0; i < NUM_FUNCTIONS; i++)
  {
    char tbuf[MAINTAINER_BUFFER_LENGTH + 1];        /* DumpAllFunctionSwitch */

    fprintf (file, "case ");
    IntrinsicFunctionUniqueID (&(IntrinsicFunctionTable[i]), tbuf);
    fprintf (file, tbuf);
    fprintf (file, ":\n");
    fprintf (file, "    break;\n");
  }
  fprintf (file, "}\n");
  fflush (file);
}

static const char *ParameterRangeID[16] = {
  "P%dERR",
  "P%dANY",
  "P%dBYT",
  "P%dINT",
  "P%dLNG",
  "P%dCUR",
  "P%dFLT",
  "P%dDBL",
  "P%dDEV",
  "P%dLEN",
  "P%dPOS",
  "P%dCOM",
  "P%dLPT",
  "P%dGTZ",
  "P%dGEZ",
  "P%dNEZ",
};

static const char *NumberVariableRange[16] = {
  /* P1ERR */ "  PARAMETER: %c  is a number, INTERNAL ERROR",
  /* P1ANY */ "  PARAMETER: %c  is a number",
  /* P1BYT */ "  PARAMETER: %c  is a number, [0,255]",
  /* P1INT */ "  PARAMETER: %c  is a number, [MININT,MAXINT]",
  /* P1LNG */ "  PARAMETER: %c  is a number, [MINLNG,MAXLNG]",
  /* P1CUR */ "  PARAMETER: %c  is a number, [MINCUR,MAXCUR]",
  /* P1FLT */ "  PARAMETER: %c  is a number, [MINFLT,MAXFLT]",
  /* P1DBL */ "  PARAMETER: %c  is a number, [MINDBL,MAXDBL]",
  /* P1DEV */ "  PARAMETER: %c  is a number, RESERVED",
  /* P1LEN */ "  PARAMETER: %c  is a number, [0,MAXLEN]",
  /* P1POS */ "  PARAMETER: %c  is a number, [1,MAXLEN]",
  /* P1COM */ "  PARAMETER: %c  is a number, RESERVED",
  /* P1LPT */ "  PARAMETER: %c  is a number, RESERVED",
  /* P1GTZ */ "  PARAMETER: %c  is a number, >  0",
  /* P1GEZ */ "  PARAMETER: %c  is a number, >= 0",
  /* P1NEZ */ "  PARAMETER: %c  is a number, <> 0",
};

static const char *StringVariableRange[16] = {
  /* P1ERR */ "  PARAMETER: %c$ is a string, INTERNAL ERROR",
  /* P1ANY */ "  PARAMETER: %c$ is a string, LEN >= 0",
  /* P1BYT */ "  PARAMETER: %c$ is a string, LEN >= 1",
  /* P1INT */ "  PARAMETER: %c$ is a string, LEN >= sizeof(INT)",
  /* P1LNG */ "  PARAMETER: %c$ is a string, LEN >= sizeof(LNG)",
  /* P1CUR */ "  PARAMETER: %c$ is a string, LEN >= sizeof(CUR)",
  /* P1FLT */ "  PARAMETER: %c$ is a string, LEN >= sizeof(FLT)",
  /* P1DBL */ "  PARAMETER: %c$ is a string, LEN >= sizeof(DBL)",
  /* P1DEV */ "  PARAMETER: %c$ is a string, RESERVED",
  /* P1LEN */ "  PARAMETER: %c$ is a string, RESERVED",
  /* P1POS */ "  PARAMETER: %c$ is a string, RESERVED",
  /* P1COM */ "  PARAMETER: %c$ is a string, RESERVED",
  /* P1LPT */ "  PARAMETER: %c$ is a string, RESERVED",
  /* P1GTZ */ "  PARAMETER: %c$ is a string, RESERVED",
  /* P1GEZ */ "  PARAMETER: %c$ is a string, RESERVED",
  /* P1NEZ */ "  PARAMETER: %c$ is a string, RESERVED",
};

void
DumpAllFuctionTableDefinitions (FILE * file)
{
  /* generate bwd_fun.c */
  int n;
   
  assert (file != NULL);


  fprintf (file, "/* FUNCTION TABLE */\n");
  fprintf (file, "\n");
  fprintf (file, "#include \"bwbasic.h\"\n");
  fprintf (file, "\n");
  fprintf (file,
           "IntrinsicFunctionType IntrinsicFunctionTable[ /* NUM_FUNCTIONS */ ] =\n");
  fprintf (file, "{\n");
  for (n = 0; n < NUM_FUNCTIONS; n++)
  {
    int i;
    int j;
    char tbuf[MAINTAINER_BUFFER_LENGTH + 1];        /* DumpAllFuctionTableDefinitions */
    char UniqueID[MAINTAINER_BUFFER_LENGTH + 1];        /* DumpAllFuctionTableDefinitions */
    char Syntax[MAINTAINER_BUFFER_LENGTH + 1];        /* DumpAllFuctionTableDefinitions */
    IntrinsicFunctionType *f;

    f = &(IntrinsicFunctionTable[n]);

    IntrinsicFunctionUniqueID (f, UniqueID);
    IntrinsicFunctionSyntax (f, Syntax);
    fprintf (file, "{\n");
    fprintf (file, "  %s, /* UniqueID */\n", UniqueID);
    fprintf (file, "  \"%s\", /* Syntax */\n", Syntax);
    fprintf (file, "  ");
    fprintf (file, "\"");
    ProcessEscapeChars (f->Description, tbuf);
    fprintf (file, tbuf);
    fprintf (file, "\"");
    fprintf (file, ",  /* Description */\n");
    fprintf (file, "  \"%s\", /* Name */\n", f->Name);
    switch (f->ReturnTypeCode)
    {
    case ByteTypeCode:
      fprintf (file, "  %s, /* ReturnTypeCode */\n", "ByteTypeCode");
      break;
    case IntegerTypeCode:
      fprintf (file, "  %s, /* ReturnTypeCode */\n", "IntegerTypeCode");
      break;
    case LongTypeCode:
      fprintf (file, "  %s, /* ReturnTypeCode */\n", "LongTypeCode");
      break;
    case CurrencyTypeCode:
      fprintf (file, "  %s, /* ReturnTypeCode */\n", "CurrencyTypeCode");
      break;
    case SingleTypeCode:
      fprintf (file, "  %s, /* ReturnTypeCode */\n", "SingleTypeCode");
      break;
    case DoubleTypeCode:
      fprintf (file, "  %s, /* ReturnTypeCode */\n", "DoubleTypeCode");
      break;
    case StringTypeCode:
      fprintf (file, "  %s, /* ReturnTypeCode */\n", "StringTypeCode");
      break;
    default:
      fprintf (file, "  %s, /* ReturnTypeCode */\n", "INTERNAL ERROR");
      break;
    }
    fprintf (file, "  %d, /* ParameterCount */\n", f->ParameterCount);
    if (f->ParameterCount == 0 || f->ParameterCount == 0xFF)
    {
      /* function has NO explicit parameters */
      fprintf (file, "  %s, /* ParameterTypes */\n", "PNONE");
      fprintf (file, "  %s, /* ParameterTests */\n", "PNONE");
    }
    else
    {
      /* function has explicit parameters */
      bwb_strcpy (tbuf, "  ");
      for (i = 0; i < f->ParameterCount; i++)
      {
        ParamBitsType ParameterTypes;
        ParameterTypes = f->ParameterTypes >> i;
        ParameterTypes &= 0x1;
        if (i > 0)
        {
          bwb_strcat (tbuf, " | ");
        }
        if (ParameterTypes)
        {
          sprintf (bwb_strchr (tbuf, NulChar), "P%dSTR", i + 1);
        }
        else
        {
          sprintf (bwb_strchr (tbuf, NulChar), "P%dNUM", i + 1);
        }
      }
      bwb_strcat (tbuf, ", /* ParameterTypes */\n");
      fprintf (file, tbuf);


      bwb_strcpy (tbuf, "  ");
      for (i = 0; i < f->ParameterCount; i++)
      {
        ParamTestType ParameterTests;
        ParameterTests = f->ParameterTests >> (i * 4);
        ParameterTests &= 0xF;

        if (i > 0)
        {
          bwb_strcat (tbuf, " | ");
        }
        sprintf (bwb_strchr (tbuf, 0), ParameterRangeID[ParameterTests],
                 i + 1);
        /* Conversion may lose significant digits */
      }
      bwb_strcat (tbuf, ", /* ParameterTests */\n");
      fprintf (file, tbuf);
    }
    bwb_strcpy (tbuf, "  ");
    j = 0;
    for (i = 0; i < NUM_VERSIONS; i++)
    {
      if (f->OptionVersionBitmask & bwb_vertable[i].OptionVersionValue)
      {
        if (j > 0)
        {
          bwb_strcat (tbuf, " | ");
        }
        bwb_strcat (tbuf, bwb_vertable[i].ID);
        j++;
      }
    }
    bwb_strcat (tbuf, " /* OptionVersionBitmask */\n");
    fprintf (file, tbuf);
    fprintf (file, "},\n");
  }
  fprintf (file, "};\n");
  fprintf (file, "\n");
  fprintf (file,
           "const size_t NUM_FUNCTIONS = sizeof( IntrinsicFunctionTable ) / sizeof( IntrinsicFunctionType  );\n");
  fprintf (file, "\n");
  fflush (file);
}

void
DumpOneFunctionSyntax (FILE * file, int IsXref, int n)
{
  IntrinsicFunctionType *f;
  assert (file != NULL);

   

  if (n < 0 || n >= NUM_FUNCTIONS)
  {
    return;
  }
  f = &(IntrinsicFunctionTable[n]);
  /* NAME */
  {
    char UniqueID[MAINTAINER_BUFFER_LENGTH + 1];        /* DumpOneFunctionSyntax */
    char Syntax[MAINTAINER_BUFFER_LENGTH + 1];        /* DumpOneFunctionSyntax */

    IntrinsicFunctionUniqueID (f, UniqueID);
    IntrinsicFunctionSyntax (f, Syntax);
    fprintf (file, "     SYNTAX: %s\n", Syntax);
  }
  /* PARAMETERS */
  if (f->ParameterCount == PNONE || f->ParameterCount == 0xFF)
  {
    /* function has NO explicit parameters */
  }
  else
  {
    /* function HAS explicit parameters */
    int i;
    ParamBitsType ParameterTypes;
    ParamTestType ParameterTests;
    char NumVar;
    char StrVar;
    ParameterTypes = f->ParameterTypes;
    ParameterTests = f->ParameterTests;
    NumVar = 'X';
    StrVar = 'A';
    for (i = 0; i < f->ParameterCount; i++)
    {
      /* sanity check this parameter */
      unsigned long thischeck;
      char tbuf[MAINTAINER_BUFFER_LENGTH + 1];        /* DumpOneFunctionSyntax */

      thischeck = ParameterTests & 0x0000000F;
      /* verify parameter check  */
      if (ParameterTypes & 1)
      {
        /* STRING */
        sprintf (tbuf, StringVariableRange[thischeck], StrVar);
        /* Conversion may lose significant digits */
        StrVar++;
      }
      else
      {
        /* NUMBER */
        sprintf (tbuf, NumberVariableRange[thischeck], NumVar);
        /* Conversion may lose significant digits */
        NumVar++;
      }
      fprintf (file, tbuf);
      fprintf (file, "\n");
      ParameterTypes = ParameterTypes >> 1;
      ParameterTests = ParameterTests >> 4;
    }
  }
  /* DESCRIPTION */
  {
    FixDescription (file, "DESCRIPTION: ", f->Description);
  }
  /* COMPATIBILITY */
  if (IsXref)
  {
    int i;
    fprintf (file, "   VERSIONS:\n");
    for (i = 0; i < NUM_VERSIONS; i++)
    {
      char X;
      if (f->OptionVersionBitmask & bwb_vertable[i].OptionVersionValue)
      {
        /* SUPPORTED */
        X = 'X';
      }
      else
      {
        /* NOT SUPPORTED */
        X = '_';
      }
      fprintf (file, "             [%c] %s\n", X, bwb_vertable[i].Name);
    }
  }

  fflush (file);
}

void
DumpAllFunctionSyntax (FILE * file, int IsXref,
                       OptionVersionType OptionVersionValue)
{
  /* for the C maintainer */
  int i;
   
  assert (file != NULL);


  fprintf (file,
           "============================================================\n");
  fprintf (file,
           "                    FUNCTIONS                               \n");
  fprintf (file,
           "============================================================\n");
  fprintf (file, "\n");
  fprintf (file, "\n");
  for (i = 0; i < NUM_FUNCTIONS; i++)
  {
    if (IntrinsicFunctionTable[i].OptionVersionBitmask & OptionVersionValue)
    {
      fprintf (file,
               "------------------------------------------------------------\n");
      DumpOneFunctionSyntax (file, IsXref, i);
    }
  }
  fprintf (file,
           "------------------------------------------------------------\n");
  fprintf (file, "\n");
  fprintf (file, "\n");
  fflush (file);
}

void
DumpAllFunctionHtmlTable (FILE * file)
{
  /* generate bwd_cmd.htm */
  int i;
  int j;
   
  assert (file != NULL);


  /* LEGEND */
  fprintf (file, "<html><head><title>FNCS</title></head><body>\n");
  fprintf (file, "<h1>LEGEND</h1><br>\n");
  fprintf (file, "<table>\n");

  fprintf (file, "<tr>");
  fprintf (file, "<td>");
  fprintf (file, "<b>");
  fprintf (file, "ID");
  fprintf (file, "</b>");
  fprintf (file, "</td>");
  fprintf (file, "<td>");
  fprintf (file, "<b>");
  fprintf (file, "NAME");
  fprintf (file, "</b>");
  fprintf (file, "</td>");
  fprintf (file, "<td>");
  fprintf (file, "<b>");
  fprintf (file, "DESCRIPTION");
  fprintf (file, "</b>");
  fprintf (file, "</td>");
  fprintf (file, "</tr>\n");

  for (j = 0; j < NUM_VERSIONS; j++)
  {
    fprintf (file, "<tr>");
    fprintf (file, "<td>");
    fprintf (file, bwb_vertable[j].ID);
    fprintf (file, "</td>");
    fprintf (file, "<td>");
    fprintf (file, bwb_vertable[j].Name);
    fprintf (file, "</td>");
    fprintf (file, "<td>");
    fprintf (file, bwb_vertable[j].Description);
    fprintf (file, "</td>");
    fprintf (file, "</tr>\n");
  }
  fprintf (file, "</table>\n");
  fprintf (file, "<hr>\n");


  /* DETAILS */
  fprintf (file, "<h1>DETAILS</h1><br>\n");
  fprintf (file, "<table>\n");

  fprintf (file, "<tr>");
  fprintf (file, "<td>");
  fprintf (file, "<b>");
  fprintf (file, "FUNCTION");
  fprintf (file, "</b>");
  fprintf (file, "</td>");
  for (j = 0; j < NUM_VERSIONS; j++)
  {
    fprintf (file, "<td>");
    fprintf (file, "<b>");
    fprintf (file, bwb_vertable[j].ID);
    fprintf (file, "</b>");
    fprintf (file, "</td>");
  }
  fprintf (file, "</tr>\n");


  /* run through the command table and print comand -vs- OPTION VERSION */

  for (i = 0; i < NUM_FUNCTIONS; i++)
  {
    fprintf (file, "<tr>");
    fprintf (file, "<td>");
    fprintf (file, (char *) IntrinsicFunctionTable[i].Syntax);
    fprintf (file, "</td>");

    for (j = 0; j < NUM_VERSIONS; j++)
    {
      fprintf (file, "<td>");
      if (IntrinsicFunctionTable[i].OptionVersionBitmask & bwb_vertable[j].
          OptionVersionValue)
      {
        fprintf (file, "X");
      }
      else
      {
        fprintf (file, " ");
      }
      fprintf (file, "</td>");
    }
    fprintf (file, "</tr>\n");
  }
  fprintf (file, "</table>\n");
  fprintf (file, "</body></html>\n");
  fprintf (file, "\n");

  fflush (file);
}

/* 
--------------------------------------------------------------------------------------------
                               FNCS
--------------------------------------------------------------------------------------------
*/
LineType *
bwb_FNCS (LineType * l)
{
  int n;
  int t;
   
  assert (l != NULL);
  assert( My != NULL );
  assert( My->SYSOUT != NULL );
  assert( My->SYSOUT->cfp != NULL );


  My->CurrentFile = My->SYSOUT;
  fprintf (My->SYSOUT->cfp, "BWBASIC FUNCTIONS AVAILABLE:\n");

  /* run through the command table and print comand names */

  t = 0;
  for (n = 0; n < NUM_FUNCTIONS; n++)
  {
    fprintf (My->SYSOUT->cfp, IntrinsicFunctionTable[n].Name);
    if (t < 4)
    {
      fprintf (My->SYSOUT->cfp, "\t");
      t++;
    }
    else
    {
      fprintf (My->SYSOUT->cfp, "\n");
      t = 0;
    }
  }
  if (t > 0)
  {
    fprintf (My->SYSOUT->cfp, "\n");
  }
  ResetConsoleColumn ();
  return (l);
}

/* 
--------------------------------------------------------------------------------------------
                               MAINTAINER
--------------------------------------------------------------------------------------------
*/
LineType *
bwb_MAINTAINER (LineType * l)
{
   
  assert (l != NULL);
  WARN_SYNTAX_ERROR;
  return (l);
}

LineType *
bwb_MAINTAINER_CMDS (LineType * l)
{
   
  assert (l != NULL);
  WARN_SYNTAX_ERROR;
  return (l);
}

LineType *
bwb_MAINTAINER_CMDS_HTML (LineType * l)
{
   
  assert (l != NULL);
  assert(My != NULL);
  assert(My->SYSPRN != NULL);
  assert(My->SYSPRN->cfp != NULL);
  DumpAllCommandHtmlTable (My->SYSPRN->cfp);
  return (l);
}

LineType *
bwb_MAINTAINER_CMDS_ID (LineType * l)
{
   
  assert (l != NULL);
  assert(My != NULL);
  assert(My->SYSPRN != NULL);
  assert(My->SYSPRN->cfp != NULL);
  DumpAllCommandUniqueID (My->SYSPRN->cfp);
  return (l);
}

LineType *
bwb_MAINTAINER_CMDS_MANUAL (LineType * l)
{
   
  assert (l != NULL);
  assert(My != NULL);
  assert(My->SYSPRN != NULL);
  assert(My->SYSPRN->cfp != NULL);
  DumpAllCommandSyntax (My->SYSPRN->cfp, TRUE, (OptionVersionType)(-1));
  return (l);
}

LineType *
bwb_MAINTAINER_CMDS_SWITCH (LineType * l)
{
   
  assert (l != NULL);
  assert(My != NULL);
  assert(My->SYSPRN != NULL);
  assert(My->SYSPRN->cfp != NULL);
  DumpAllCommandSwitchStatement (My->SYSPRN->cfp);
  return (l);
}

LineType *
bwb_MAINTAINER_CMDS_TABLE (LineType * l)
{
   
  assert (l != NULL);
  assert(My != NULL);
  assert(My->SYSPRN != NULL);
  assert(My->SYSPRN->cfp != NULL);
  DumpAllCommandTableDefinitions (My->SYSPRN->cfp);
  return (l);
}

LineType *
bwb_MAINTAINER_DEBUG (LineType * l)
{
   
  assert (l != NULL);
  WARN_SYNTAX_ERROR;
  return (l);
}

LineType *
bwb_MAINTAINER_DEBUG_ON (LineType * l)
{
   
  assert (l != NULL);
  return (l);
}

LineType *
bwb_MAINTAINER_DEBUG_OFF (LineType * l)
{
   
  assert (l != NULL);
  return (l);
}

LineType *
bwb_MAINTAINER_FNCS (LineType * l)
{
   
  assert (l != NULL);
  WARN_SYNTAX_ERROR;
  return (l);
}

LineType *
bwb_MAINTAINER_FNCS_HTML (LineType * l)
{
   
  assert (l != NULL);
  assert(My != NULL);
  assert(My->SYSPRN != NULL);
  assert(My->SYSPRN->cfp != NULL);
  DumpAllFunctionHtmlTable (My->SYSPRN->cfp);
  return (l);
}

LineType *
bwb_MAINTAINER_FNCS_ID (LineType * l)
{
   
  assert (l != NULL);
  assert(My != NULL);
  assert(My->SYSPRN != NULL);
  assert(My->SYSPRN->cfp != NULL);
  DumpAllFunctionUniqueID (My->SYSPRN->cfp);
  return (l);
}

LineType *
bwb_MAINTAINER_FNCS_MANUAL (LineType * l)
{
   
  assert (l != NULL);
  assert(My != NULL);
  assert(My->SYSPRN != NULL);
  assert(My->SYSPRN->cfp != NULL);
  DumpAllFunctionSyntax (My->SYSPRN->cfp, TRUE, (OptionVersionType)(-1));
  DumpAllOperatorSyntax (My->SYSPRN->cfp, TRUE, (OptionVersionType)(-1));
  return (l);
}

LineType *
bwb_MAINTAINER_FNCS_SWITCH (LineType * l)
{
   
  assert (l != NULL);
  assert(My != NULL);
  assert(My->SYSPRN != NULL);
  assert(My->SYSPRN->cfp != NULL);
  DumpAllFunctionSwitch (My->SYSPRN->cfp);
  return (l);
}

LineType *
bwb_MAINTAINER_FNCS_TABLE (LineType * l)
{
   
  assert (l != NULL);
  assert(My != NULL);
  assert(My->SYSPRN != NULL);
  assert(My->SYSPRN->cfp != NULL);
  DumpAllFuctionTableDefinitions (My->SYSPRN->cfp);
  return (l);
}

void
DumpHeader (FILE * file)
{
  char c;
   
  assert (file != NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);

  fprintf (file,
           "============================================================\n");
  fprintf (file,
           "                    GENERAL                                 \n");
  fprintf (file,
           "============================================================\n");
  fprintf (file, "\n");
  fprintf (file, "\n");

  fprintf (file, "OPTION VERSION \"%s\"\n", My->CurrentVersion->Name);
  fprintf (file, "REM INTERNAL ID: %s\n", My->CurrentVersion->ID);
  fprintf (file, "REM DESCRIPTION: %s\n", My->CurrentVersion->Description);
  fprintf (file, "REM   REFERENCE: %s\n", My->CurrentVersion->ReferenceTitle);
  fprintf (file, "REM              %s\n",
           My->CurrentVersion->ReferenceAuthor);
  fprintf (file, "REM              %s\n",
           My->CurrentVersion->ReferenceCopyright);
  fprintf (file, "REM              %s\n", My->CurrentVersion->ReferenceURL1);
  fprintf (file, "REM              %s\n", My->CurrentVersion->ReferenceURL2);
  fprintf (file, "REM\n");

  if (My->CurrentVersion->OptionFlags & (OPTION_STRICT_ON))
  {
    fprintf (file, "OPTION STRICT ON\n");
  }
  else
  {
    fprintf (file, "OPTION STRICT OFF\n");
  }

  if (My->CurrentVersion->OptionFlags & (OPTION_ANGLE_DEGREES))
  {
    fprintf (file, "OPTION ANGLE DEGREES\n");
  }
  else if (My->CurrentVersion->OptionFlags & (OPTION_ANGLE_GRADIANS))
  {
    fprintf (file, "OPTION ANGLE GRADIANS\n");
  }
  else
  {
    fprintf (file, "OPTION ANGLE RADIANS\n");
  }

  if (My->CurrentVersion->OptionFlags & (OPTION_BUGS_ON))
  {
    fprintf (file, "OPTION BUGS ON\n");
  }
  else
  {
    fprintf (file, "OPTION BUGS OFF\n");
  }

  if (My->CurrentVersion->OptionFlags & (OPTION_LABELS_ON))
  {
    fprintf (file, "OPTION LABELS ON\n");
  }
  else
  {
    fprintf (file, "OPTION LABELS OFF\n");
  }

  if (My->CurrentVersion->OptionFlags & (OPTION_COMPARE_TEXT))
  {
    fprintf (file, "OPTION COMPARE TEXT\n");
  }
  else
  {
    fprintf (file, "OPTION COMPARE BINARY\n");
  }

  if (My->CurrentVersion->OptionFlags & (OPTION_COVERAGE_ON))
  {
    fprintf (file, "OPTION COVERAGE ON\n");
  }
  else
  {
    fprintf (file, "OPTION COVERAGE OFF\n");
  }

  if (My->CurrentVersion->OptionFlags & (OPTION_TRACE_ON))
  {
    fprintf (file, "OPTION TRACE ON\n");
  }
  else
  {
    fprintf (file, "OPTION TRACE OFF\n");
  }

  if (My->CurrentVersion->OptionFlags & (OPTION_ERROR_GOSUB))
  {
    fprintf (file, "OPTION ERROR GOSUB\n");
  }
  else
  {
    fprintf (file, "OPTION ERROR GOTO\n");
  }

  if (My->CurrentVersion->OptionFlags & (OPTION_EXPLICIT_ON))
  {
    fprintf (file, "OPTION EXPLICIT\n");
  }
  else
  {
    fprintf (file, "OPTION IMPLICIT\n");
  }

  fprintf (file, "OPTION BASE          %d\n",
           My->CurrentVersion->OptionBaseInteger);
  fprintf (file, "OPTION RECLEN        %d\n",
           My->CurrentVersion->OptionReclenInteger);
  fprintf (file, "OPTION DATE          \"%s\"\n",
           My->CurrentVersion->OptionDateFormat);
  fprintf (file, "OPTION TIME          \"%s\"\n",
           My->CurrentVersion->OptionTimeFormat);

  c = My->CurrentVersion->OptionStringChar;
  if (!bwb_isgraph (c))
  {
    c = ' ';
  };
  fprintf (file, "OPTION PUNCT STRING        \"%c\"\n", c);

  c = My->CurrentVersion->OptionDoubleChar;
  if (!bwb_isgraph (c))
  {
    c = ' ';
  };
  fprintf (file, "OPTION PUNCT DOUBLE        \"%c\"\n", c);

  c = My->CurrentVersion->OptionSingleChar;
  if (!bwb_isgraph (c))
  {
    c = ' ';
  };
  fprintf (file, "OPTION PUNCT SINGLE        \"%c\"\n", c);

  c = My->CurrentVersion->OptionCurrencyChar;
  if (!bwb_isgraph (c))
  {
    c = ' ';
  };
  fprintf (file, "OPTION PUNCT CURRENCY      \"%c\"\n", c);

  c = My->CurrentVersion->OptionLongChar;
  if (!bwb_isgraph (c))
  {
    c = ' ';
  };
  fprintf (file, "OPTION PUNCT LONG          \"%c\"\n", c);

  c = My->CurrentVersion->OptionIntegerChar;
  if (!bwb_isgraph (c))
  {
    c = ' ';
  };
  fprintf (file, "OPTION PUNCT INTEGER       \"%c\"\n", c);

  c = My->CurrentVersion->OptionByteChar;
  if (!bwb_isgraph (c))
  {
    c = ' ';
  };
  fprintf (file, "OPTION PUNCT BYTE          \"%c\"\n", c);

  c = My->CurrentVersion->OptionQuoteChar;
  if (!bwb_isgraph (c))
  {
    c = ' ';
  };
  fprintf (file, "OPTION PUNCT QUOTE         \"%c\"\n", c);

  c = My->CurrentVersion->OptionCommentChar;
  if (!bwb_isgraph (c))
  {
    c = ' ';
  };
  fprintf (file, "OPTION PUNCT COMMENT       \"%c\"\n", c);

  c = My->CurrentVersion->OptionStatementChar;
  if (!bwb_isgraph (c))
  {
    c = ' ';
  };
  fprintf (file, "OPTION PUNCT STATEMENT     \"%c\"\n", c);

  c = My->CurrentVersion->OptionPrintChar;
  if (!bwb_isgraph (c))
  {
    c = ' ';
  };
  fprintf (file, "OPTION PUNCT PRINT         \"%c\"\n", c);

  c = My->CurrentVersion->OptionInputChar;
  if (!bwb_isgraph (c))
  {
    c = ' ';
  };
  fprintf (file, "OPTION PUNCT INPUT         \"%c\"\n", c);

  c = My->CurrentVersion->OptionImageChar;
  if (!bwb_isgraph (c))
  {
    c = ' ';
  };
  fprintf (file, "OPTION PUNCT IMAGE         \"%c\"\n", c);

  c = My->CurrentVersion->OptionLparenChar;
  if (!bwb_isgraph (c))
  {
    c = ' ';
  };
  fprintf (file, "OPTION PUNCT LPAREN        \"%c\"\n", c);

  c = My->CurrentVersion->OptionRparenChar;
  if (!bwb_isgraph (c))
  {
    c = ' ';
  };
  fprintf (file, "OPTION PUNCT RPAREN        \"%c\"\n", c);

  c = My->CurrentVersion->OptionFilenumChar;
  if (!bwb_isgraph (c))
  {
    c = ' ';
  };
  fprintf (file, "OPTION PUNCT FILENUM       \"%c\"\n", c);

  c = My->CurrentVersion->OptionAtChar;
  if (!bwb_isgraph (c))
  {
    c = ' ';
  };
  fprintf (file, "OPTION PUNCT AT            \"%c\"\n", c);

  c = My->CurrentVersion->OptionUsingDigit;
  if (!bwb_isgraph (c))
  {
    c = ' ';
  };
  fprintf (file, "OPTION USING DIGIT   \"%c\"\n", c);

  c = My->CurrentVersion->OptionUsingComma;
  if (!bwb_isgraph (c))
  {
    c = ' ';
  };
  fprintf (file, "OPTION USING COMMA   \"%c\"\n", c);

  c = My->CurrentVersion->OptionUsingPeriod;
  if (!bwb_isgraph (c))
  {
    c = ' ';
  };
  fprintf (file, "OPTION USING PERIOD  \"%c\"\n", c);

  c = My->CurrentVersion->OptionUsingPlus;
  if (!bwb_isgraph (c))
  {
    c = ' ';
  };
  fprintf (file, "OPTION USING PLUS    \"%c\"\n", c);

  c = My->CurrentVersion->OptionUsingMinus;
  if (!bwb_isgraph (c))
  {
    c = ' ';
  };
  fprintf (file, "OPTION USING MINUS   \"%c\"\n", c);

  c = My->CurrentVersion->OptionUsingExrad;
  if (!bwb_isgraph (c))
  {
    c = ' ';
  };
  fprintf (file, "OPTION USING EXRAD   \"%c\"\n", c);

  c = My->CurrentVersion->OptionUsingDollar;
  if (!bwb_isgraph (c))
  {
    c = ' ';
  };
  fprintf (file, "OPTION USING DOLLAR  \"%c\"\n", c);

  c = My->CurrentVersion->OptionUsingFiller;
  if (!bwb_isgraph (c))
  {
    c = ' ';
  };
  fprintf (file, "OPTION USING FILLER  \"%c\"\n", c);

  c = My->CurrentVersion->OptionUsingLiteral;
  if (!bwb_isgraph (c))
  {
    c = ' ';
  };
  fprintf (file, "OPTION USING LITERAL \"%c\"\n", c);

  c = My->CurrentVersion->OptionUsingFirst;
  if (!bwb_isgraph (c))
  {
    c = ' ';
  };
  fprintf (file, "OPTION USING FIRST   \"%c\"\n", c);

  c = My->CurrentVersion->OptionUsingAll;
  if (!bwb_isgraph (c))
  {
    c = ' ';
  };
  fprintf (file, "OPTION USING ALL     \"%c\"\n", c);

  c = My->CurrentVersion->OptionUsingLength;
  if (!bwb_isgraph (c))
  {
    c = ' ';
  };
  fprintf (file, "OPTION USING LENGTH  \"%c\"\n", c);

  fprintf (file, "\n");
  fprintf (file, "\n");
  fflush (file);
}

LineType *
bwb_MAINTAINER_MANUAL (LineType * l)
{
   
  assert (l != NULL);

  DumpHeader (My->SYSPRN->cfp);
  DumpAllCommandSyntax (My->SYSPRN->cfp, FALSE,
                        My->CurrentVersion->OptionVersionValue);
  DumpAllFunctionSyntax (My->SYSPRN->cfp, FALSE,
                         My->CurrentVersion->OptionVersionValue);
  DumpAllOperatorSyntax (My->SYSPRN->cfp, FALSE,
                         My->CurrentVersion->OptionVersionValue);
  return (l);
}

LineType *
bwb_MAINTAINER_STACK (LineType * l)
{
  /*
     dump the current execution stack, 
     Leftmost is the top, 
     Rigthmost is the bottom. 
   */
  StackType *StackItem;
   
  assert (l != NULL);

  for (StackItem = My->StackHead; StackItem != NULL;
       StackItem = StackItem->next)
  {
    LineType *l;

    l = StackItem->line;
    if (l != NULL)
    {
      fprintf (My->SYSOUT->cfp, "%d:", l->number);
    }
  }
  fprintf (My->SYSOUT->cfp, "\n");
  ResetConsoleColumn ();
  return (l);
}


/***************************************************************
  
        FUNCTION:       IntrinsicFunction_init()
  
        DESCRIPTION:    This command initializes the function
                        linked list, placing all predefined functions
                        in the list.
  
***************************************************************/

int
IntrinsicFunction_init (void)
{
  int n;
   

  for (n = 0; n < NUM_FUNCTIONS; n++)
  {
    IntrinsicFunctionDefinitionCheck (&(IntrinsicFunctionTable[n]));
  }
  return TRUE;
}



VariableType *
IntrinsicFunction_deffn (int argc, VariableType * argv, UserFunctionType * f)
{
  /* 
     The generic handler for user defined functions.  
     When called by exp_function(), f->id will be set to the line number of a specific DEF USR.
   */
  VariableType *v;
  VariableType *argn;
  int i;
  LineType *call_line;
  StackType *save_elevel;
   
  assert (argc >= 0);
  assert (argv != NULL);
  assert (f != NULL);
  assert(My != NULL);

  /* initialize the variable if necessary */

  /* these errors should not occur */
  if (f == NULL)
  {
    WARN_INTERNAL_ERROR;
    return NULL;
  }
  if (f->line == NULL)
  {
    WARN_INTERNAL_ERROR;
    return NULL;
  }
  if (argv == NULL)
  {
    WARN_INTERNAL_ERROR;
    return NULL;
  }
  if (f->ParameterCount == 0xFF)
  {
    /* VARIANT */
  }
  else if (argc != f->ParameterCount)
  {
    WARN_INTERNAL_ERROR;
    return NULL;
  }
  if (f->ParameterCount == 0xFF)
  {
    /* VARIANT */
    f->local_variable = argv;
  }
  else if (argc > 0)
  {
    v = f->local_variable;
    argn = argv;
    for (i = 0; i < argc; i++)
    {
      argn = argn->next;
      if (v == NULL)
      {
        WARN_INTERNAL_ERROR;
        return NULL;
      }
      if (argn == NULL)
      {
        WARN_INTERNAL_ERROR;
        return NULL;
      }
      if (VAR_IS_STRING (v) != VAR_IS_STRING (argn))
      {
        WARN_INTERNAL_ERROR;
        return NULL;
      }
      if (is_empty_string (v->name) == FALSE)
      {
        int IsError;
        IsError = 0;
        switch (v->VariableTypeCode)
        {
        case ByteTypeCode:
          IsError = NumberValueCheck (P1BYT, PARAM_NUMBER);
          break;
        case IntegerTypeCode:
          IsError = NumberValueCheck (P1INT, PARAM_NUMBER);
          break;
        case LongTypeCode:
          IsError = NumberValueCheck (P1LNG, PARAM_NUMBER);
          break;
        case CurrencyTypeCode:
          IsError = NumberValueCheck (P1CUR, PARAM_NUMBER);
          break;
        case SingleTypeCode:
          IsError = NumberValueCheck (P1FLT, PARAM_NUMBER);
          break;
        case DoubleTypeCode:
          IsError = NumberValueCheck (P1DBL, PARAM_NUMBER);
          break;
        case StringTypeCode:
          IsError = StringLengthCheck (P1ANY, PARAM_LENGTH);
          break;
        default:
          WARN_TYPE_MISMATCH;
          return NULL;
        }
        if (IsError != 0)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
          return argv;
        }
      }
      v = v->next;
    }
  }
  /* OK */
  call_line = f->line;                /* line to call for function */
  call_line->position = f->startpos;

  if (call_line->cmdnum == C_DEF)
  {
    if (line_skip_EqualChar (call_line) == FALSE)
    {
      WARN_INTERNAL_ERROR;
      return NULL;
    }
  }
/* PUSH STACK */

  save_elevel = My->StackHead;
  if (bwb_incexec ())
  {
    /* OK */
    My->StackHead->line = call_line;
    My->StackHead->ExecCode = EXEC_FUNCTION;
  }
  else
  {
    /* ERROR */
    WARN_OUT_OF_MEMORY;
    return NULL;
  }


  /* create variable chain */
  if (f->ParameterCount == 0xFF)
  {
    /* VARIANT */
  }
  else if (argc > 0)
  {
    VariableType *source = NULL;        /* source variable */
    source = f->local_variable;
    argn = argv;
    for (i = 0; i < argc; i++)
    {
      argn = argn->next;
      /* copy the name */
      bwb_strcpy (argn->name, source->name);

      if (VAR_IS_STRING (source))
      {
      }
      else
      {
        int IsError;
        double Value;
        VariantType variant;
        CLEAR_VARIANT (&variant);

        if (var_get (argn, &variant) == FALSE)
        {
          WARN_VARIABLE_NOT_DECLARED;
          return NULL;
        }
        if (variant.VariantTypeCode == StringTypeCode)
        {
          WARN_TYPE_MISMATCH;
          return NULL;
        }
        Value = variant.Number;
        IsError = 0;
        switch (source->VariableTypeCode)
        {
        case ByteTypeCode:
          IsError = NumberValueCheck (P1BYT, Value);
          Value = bwb_rint (Value);
          break;
        case IntegerTypeCode:
          IsError = NumberValueCheck (P1INT, Value);
          Value = bwb_rint (Value);
          break;
        case LongTypeCode:
          IsError = NumberValueCheck (P1LNG, Value);
          Value = bwb_rint (Value);
          break;
        case CurrencyTypeCode:
          IsError = NumberValueCheck (P1CUR, Value);
          Value = bwb_rint (Value);
          break;
        case SingleTypeCode:
          IsError = NumberValueCheck (P1FLT, Value);
          break;
        case DoubleTypeCode:
          IsError = NumberValueCheck (P1DBL, Value);
          break;
        case StringTypeCode:
          WARN_TYPE_MISMATCH;
          return NULL;
          /* break; */
        default:
          WARN_TYPE_MISMATCH;
          return NULL;
        }
        if (IsError != 0)
        {
          WARN_ILLEGAL_FUNCTION_CALL;
          return argv;
        }
        variant.Number = Value;
        if (var_set (argn, &variant) == FALSE)
        {
          WARN_VARIABLE_NOT_DECLARED;
          return NULL;
        }
      }
      source = source->next;
    }
  }
  if (call_line->cmdnum == C_DEF)
  {
    VariantType x;
    VariantType *X;

    X = &x;
    CLEAR_VARIANT (X);
    /* the function return variable is hidden */
    My->StackHead->local_variable = argv->next;
    /* var_islocal() uses the LoopTopLine to find local variables */
    My->StackHead->LoopTopLine = call_line;        /* FUNCTION, SUB */

    /* evaluate the expression */
    if (line_read_expression (call_line, X) == FALSE)        /* IntrinsicFunction_deffn */
    {
      WARN_SYNTAX_ERROR;
      goto EXIT;
    }

    /* save the value */
    switch (X->VariantTypeCode)
    {
    case ByteTypeCode:
    case IntegerTypeCode:
    case LongTypeCode:
    case CurrencyTypeCode:
    case SingleTypeCode:
    case DoubleTypeCode:
      if (argv->VariableTypeCode == StringTypeCode)
      {
        WARN_TYPE_MISMATCH;
        goto EXIT;
      }
      /* OK */
      {
        int IsError;
        double Value;

        IsError = 0;
        Value = X->Number;
        /* VerifyNumeric */
        if (isnan (Value))
        {
               /*** FATAL - INTERNAL ERROR - SHOULD NEVER HAPPEN ***/
          WARN_INTERNAL_ERROR;
          return FALSE;
        }
        if (isinf (Value))
        {
          /* - Evaluation of an expression results in an overflow
           * (nonfatal, the recommended recovery procedure is to supply
           * machine in- finity with the algebraically correct sign and
           * continue). */
          if (Value < 0)
          {
            Value = MINDBL;
          }
          else
          {
            Value = MAXDBL;
          }
          if (WARN_OVERFLOW)
          {
            /* ERROR */
            goto EXIT;
          }
          /* CONTINUE */
        }
        /* OK */
        switch (argv->VariableTypeCode)
        {
        case ByteTypeCode:
          IsError = NumberValueCheck (P1BYT, Value);
          Value = bwb_rint (Value);
          break;
        case IntegerTypeCode:
          IsError = NumberValueCheck (P1INT, Value);
          Value = bwb_rint (Value);
          break;
        case LongTypeCode:
          IsError = NumberValueCheck (P1LNG, Value);
          Value = bwb_rint (Value);
          break;
        case CurrencyTypeCode:
          IsError = NumberValueCheck (P1CUR, Value);
          Value = bwb_rint (Value);
          break;
        case SingleTypeCode:
          IsError = NumberValueCheck (P1FLT, Value);
          break;
        case DoubleTypeCode:
          IsError = NumberValueCheck (P1DBL, Value);
          break;
        default:
          WARN_TYPE_MISMATCH;
          goto EXIT;
          /* break; */
        }
        if (IsError != 0)
        {
          if (WARN_OVERFLOW)
          {
            /* ERROR */
            goto EXIT;
          }
          /* CONTINUE */
        }
        /* assign Value */
        RESULT_NUMBER = Value;
      }
      break;
    case StringTypeCode:
      if (argv->VariableTypeCode != StringTypeCode)
      {
        WARN_TYPE_MISMATCH;
        goto EXIT;
      }
      /* OK */
      if (RESULT_BUFFER != My->MaxLenBuffer)
      {
        WARN_INTERNAL_ERROR;
        goto EXIT;
      }
      if (X->Length > MAXLEN)
      {
        WARN_STRING_TOO_LONG;        /* IntrinsicFunction_deffn */
        X->Length = MAXLEN;
      }
      bwb_memcpy (RESULT_BUFFER, X->Buffer, X->Length);
      RESULT_LENGTH = X->Length;
      break;
    default:
      WARN_TYPE_MISMATCH;
      goto EXIT;
      /* break; */
    }
  EXIT:
    RELEASE_VARIANT (X);


    /* break variable chain */
    My->StackHead->local_variable = NULL;



/* POP STACK */
    bwb_decexec ();

  }
  else
  {
    /* the function return variable is visible */
    My->StackHead->local_variable = argv;
    /* var_islocal() uses the LoopTopLine to find local variables */
    My->StackHead->LoopTopLine = call_line;        /* FUNCTION, SUB */
    /* execute until function returns */
    while (My->StackHead != save_elevel)
    {
      bwb_execline ();
    }
  }

  if (f->ParameterCount == 0xFF)
  {
    /* VARIANT */
    f->local_variable = NULL;
  }

  if (is_empty_string (argv->name) == FALSE)
  {
    int IsError;

    IsError = 0;
    switch (argv->VariableTypeCode)
    {
    case ByteTypeCode:
      IsError = NumberValueCheck (P1BYT, RESULT_NUMBER);
      break;
    case IntegerTypeCode:
      IsError = NumberValueCheck (P1INT, RESULT_NUMBER);
      break;
    case LongTypeCode:
      IsError = NumberValueCheck (P1LNG, RESULT_NUMBER);
      break;
    case CurrencyTypeCode:
      IsError = NumberValueCheck (P1CUR, RESULT_NUMBER);
      break;
    case SingleTypeCode:
      IsError = NumberValueCheck (P1FLT, RESULT_NUMBER);
      break;
    case DoubleTypeCode:
      IsError = NumberValueCheck (P1DBL, RESULT_NUMBER);
      break;
    case StringTypeCode:
      IsError = StringLengthCheck (P1ANY, RESULT_LENGTH);
      break;
    default:
      /* no check */
      break;
    }
    if (IsError != 0)
    {
      if (WARN_OVERFLOW)
      {
        /* ERROR */
      }
      /* CONTINUE */
    }
  }
  return argv;
}

/***************************************************************
  
        FUNCTION:       IntrinsicFunction_find()
  
        DESCRIPTION:    This C function attempts to locate
                        a BASIC function with the specified name.
                        If successful, it returns a pointer to
                        the C structure for the BASIC function,
                        if not successful, it returns NULL.
  
***************************************************************/

extern int
IntrinsicFunction_name (char *name)
{
  /* search INTRINSIC functions */
  IntrinsicFunctionType *f;
  int i;
   
  assert (name != NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);


#if THE_PRICE_IS_RIGHT
  /* start with the closest function, without going over */
  i = VarTypeIndex (name[0]);
  if (i < 0)
  {
    /* non-alpha */
    return FALSE;
  }
  i = My->IntrinsicFunctionStart[i];        /* first function starting with this letter */
  if (i < 0)
  {
    /* NOT FOUND */
    return FALSE;
  }
#else /* THE_PRICE_IS_RIGHT */
  i = 0;
#endif /* THE_PRICE_IS_RIGHT */
  for (; i < NUM_FUNCTIONS; i++)
  {
    f = &IntrinsicFunctionTable[i];
    if (My->CurrentVersion->OptionVersionValue & f->OptionVersionBitmask)
    {
      int result;

      result = bwb_stricmp (f->Name, name);

      if (result == 0)
      {
        /* FOUND */
        return TRUE;
      }
      if (result > 0 /* found > searched */ )
      {
        /* NOT FOUND */
        return FALSE;
      }
    }
  }
  /* NOT FOUND */
  return FALSE;
}


IntrinsicFunctionType *
IntrinsicFunction_find_exact (char *name, int ParameterCount,
                              ParamBitsType ParameterTypes)
{
  IntrinsicFunctionType *f;
  int i;
   
  assert (name != NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);

  /* search INTRINSIC functions */
#if THE_PRICE_IS_RIGHT
  /* start with the closest function, without going over */
  i = VarTypeIndex (name[0]);
  if (i < 0)
  {
    /* non-alpha */
    return NULL;
  }
  i = My->IntrinsicFunctionStart[i];        /* first function starting with this letter */
  if (i < 0)
  {
    /* NOT FOUND */
    return NULL;
  }
#else /* THE_PRICE_IS_RIGHT */
  i = 0;
#endif /* THE_PRICE_IS_RIGHT */
  for (; i < NUM_FUNCTIONS; i++)
  {
    f = &IntrinsicFunctionTable[i];
    if (My->CurrentVersion->OptionVersionValue & f->OptionVersionBitmask)
    {
      if (f->ParameterCount == ParameterCount)
      {
        if (f->ParameterTypes == ParameterTypes)
        {
          int result;

          result = bwb_stricmp (f->Name, name);

          if (result == 0)
          {
            /* FOUND */
            return f;
          }
          if (result > 0 /* found > searched */ )
          {
            /* NOT FOUND */
            return NULL;
          }
        }
      }
    }
  }
  /* NOT FOUND */
  return NULL;
}

static VariableType *
find_variable_by_type (char *name, int dimensions, char VariableTypeCode)
{
  VariableType *v = NULL;
   
  assert (name != NULL);

  v = var_find (name, dimensions, FALSE);
  if (v)
  {
    if (VAR_IS_STRING (v))
    {
      if (VariableTypeCode == StringTypeCode)
      {
        /* found */
        return v;
      }
    }
    else
    {
      if (VariableTypeCode != StringTypeCode)
      {
        /* found */
        return v;
      }
    }
  }
  /* not found */
  return NULL;
}

/* 
--------------------------------------------------------------------------------------------
                               CHANGE
--------------------------------------------------------------------------------------------
*/

LineType *
bwb_CHANGE (LineType * l)
{
  /* SYNTAX: CHANGE A$ TO X */
  /* SYNTAX: CHANGE X TO A$ */
  char varname[NameLengthMax + 1];
  VariableType *v;
  VariableType *A;
  VariableType *X;
  int IsStringToArray;
   
  assert (l != NULL);

  v = NULL;
  A = NULL;
  X = NULL;
  IsStringToArray = FALSE;

  /* get 1st variable */
  if (line_read_varname (l, varname) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }

  v = find_variable_by_type (varname, 0, StringTypeCode);
  if (v)
  {
    /* STRING to ARRAY */
    A = v;
    IsStringToArray = TRUE;
  }
  else
  {
    /* ARRAY to STRING */
    v = find_variable_by_type (varname, 1, DoubleTypeCode);
    if (v)
    {
      X = v;
      IsStringToArray = FALSE;
    }
  }
  if (v == NULL)
  {
    WARN_VARIABLE_NOT_DECLARED;
    return (l);
  }

  /* get "TO" */
  if (line_skip_word (l, "TO") == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  /* get 2nd variable */
  if (line_read_varname (l, varname) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }

  if (IsStringToArray)
  {
    /* STRING to ARRAY */
    v = find_variable_by_type (varname, 1, DoubleTypeCode);
    if (v == NULL)
    {
      v = var_find (varname, 1, TRUE);
    }
    if (v)
    {
      X = v;
    }
  }
  else
  {
    /* ARRAY to STRING */
    v = find_variable_by_type (varname, 0, StringTypeCode);
    if (v == NULL)
    {
      v = var_find (varname, 0, TRUE);
    }
    if (v)
    {
      A = v;
    }
  }

  if (v == NULL)
  {
    WARN_VARIABLE_NOT_DECLARED;
    return (l);
  }
  assert(A != NULL);
  assert(X != NULL);
  if (IsStringToArray)
  {
    /* CHANGE A$ TO X */
    int i;
    int n;
    char *a;
    DoubleType *x;
    unsigned long t;

    if (A->Value.String == NULL)
    {
      WARN_INTERNAL_ERROR;
      return (l);
    }
    if (A->Value.String->sbuffer == NULL)
    {
      WARN_INTERNAL_ERROR;
      return (l);
    }
    /* variable storage is a mess, we bypass that tradition here. */
    t = 1;
    for (n = 0; n < X->dimensions; n++)
    {
      t *= X->UBOUND[n] - X->LBOUND[n] + 1;
    }
    if (t <= A->Value.String->length)
    {
      WARN_SUBSCRIPT_OUT_OF_RANGE;
      return (l);
    }
    n = A->Value.String->length;
    a = A->Value.String->sbuffer;
    x = X->Value.Number;
    *x = n;
    x++;
    for (i = 0; i < n; i++)
    {
      char C;
      DoubleType V;

      C = *a;
      V = C;
      *x = V;
      x++;
      a++;
    }
  }
  else
  {
    /* CHANGE X TO A$ */
    int i;
    int n;
    char *a;
    DoubleType *x;
    unsigned long t;

    /* variable storage is a mess, we bypass that tradition here. */
    t = 1;
    for (n = 0; n < X->dimensions; n++)
    {
      t *= X->UBOUND[n] - X->LBOUND[n] + 1;
    }
    if (t <= 1)
    {
      WARN_SUBSCRIPT_OUT_OF_RANGE;
      return (l);
    }
    if (t > MAXLEN)
    {
      WARN_STRING_TOO_LONG;        /* bwb_CHANGE */
      t = MAXLEN;
    }
    if (A->Value.String == NULL)
    {
      if ((A->Value.String =
           (StringType *) calloc (1, sizeof (StringType))) == NULL)
      {
        WARN_OUT_OF_MEMORY;
        return (l);
      }
      A->Value.String->sbuffer = NULL;
      A->Value.String->length = 0;
    }
    if (A->Value.String->sbuffer != NULL)
    {
      free (A->Value.String->sbuffer);
      A->Value.String->sbuffer = NULL;
      A->Value.String->length = 0;
    }
    if (A->Value.String->sbuffer == NULL)
    {
      A->Value.String->length = 0;
      if ((A->Value.String->sbuffer =
           (char *) calloc (t + 1 /* NulChar */ , sizeof (char))) == NULL)
      {
        WARN_OUT_OF_MEMORY;
        return (l);
      }
    }
    a = A->Value.String->sbuffer;
    x = X->Value.Number;
    n = (int) bwb_rint (*x);
    if (n > MAXLEN)
    {
      WARN_STRING_TOO_LONG;        /* bwb_CHANGE */
      n = MAXLEN;
    }
    A->Value.String->length = n;
    x++;
    for (i = 0; i < n; i++)
    {
      char C;
      DoubleType V;

      V = *x;
      C = V;
      *a = C;
      x++;
      a++;
    }
  }
  return (l);
}

/* 
--------------------------------------------------------------------------------------------
                               CONSOLE
--------------------------------------------------------------------------------------------
*/

LineType *
bwb_CONSOLE (LineType * l)
{
  /* SYNTAX: CONSOLE */
  /* SYNTAX: CONSOLE WIDTH width */
   
  assert (l != NULL);
  assert(My != NULL);
  assert(My->SYSPRN != NULL);
  assert(My->SYSPRN->cfp != NULL);
  assert(My->SYSOUT != NULL);
  assert(My->SYSOUT->cfp != NULL);


  if (My->IsPrinter == TRUE)
  {
    /* reset printer column */
    if (My->SYSPRN->col != 1)
    {
      fputc ('\n', My->SYSPRN->cfp);
      My->SYSPRN->col = 1;
    }
    My->IsPrinter = FALSE;
  }
  if (line_skip_word (l, "WIDTH"))
  {
    int width;

    width = 0;
    if (line_read_integer_expression (l, &width) == FALSE)
    {
      WARN_ILLEGAL_FUNCTION_CALL;
      return (l);
    }
    if (width < 0)
    {
      WARN_ILLEGAL_FUNCTION_CALL;
      return (l);
    }
    My->SYSOUT->width = width;
  }

  return (l);
}

/* 
--------------------------------------------------------------------------------------------
                               LPRINTER
--------------------------------------------------------------------------------------------
*/


LineType *
bwb_LPRINTER (LineType * l)
{
  /* SYNTAX: LPRINTER */
  /* SYNTAX: LPRINTER WIDTH width */
   
  assert (l != NULL);
  assert(My != NULL);
  assert(My->SYSPRN != NULL);
  assert(My->SYSPRN->cfp != NULL);
  assert(My->SYSOUT != NULL);
  assert(My->SYSOUT->cfp != NULL);


  if (My->IsPrinter == FALSE)
  {
    /* reset console column */
    if (My->SYSOUT->col != 1)
    {
      fputc ('\n', My->SYSOUT->cfp);
      My->SYSOUT->col = 1;
    }
    My->IsPrinter = TRUE;
  }
  if (line_skip_word (l, "WIDTH"))
  {
    int width;

    width = 0;
    if (line_read_integer_expression (l, &width) == FALSE)
    {
      WARN_ILLEGAL_FUNCTION_CALL;
      return (l);
    }
    if (width < 0)
    {
      WARN_ILLEGAL_FUNCTION_CALL;
      return (l);
    }
    My->SYSPRN->width = width;
  }
  return (l);
}

extern void
bwb_fclose (FILE * file)
{
  if (file == NULL)
  {
    /* don't close */
  }
  else if (file == stdin)
  {
    /* don't close */
  }
  else if (file == stdout)
  {
    /* don't close */
  }
  else if (file == stderr)
  {
    /* don't close */
  }
  else
  {
    fclose (file);
  }
}
LineType *
bwb_LPT (LineType * l)
{
  /* SYNTAX: LPT */
  /* SYNTAX: LPT filename$ */
  FILE *file;
  char *filename;
   
  assert (l != NULL);
  assert(My != NULL);
  assert(My->SYSOUT != NULL);
  assert(My->SYSOUT->cfp != NULL);


  file = NULL;
  filename = NULL;
  if (line_is_eol (l))
  {
    /* OK */
    file = stderr;
  }
  else if (line_read_string_expression (l, &filename))
  {
    /* OK */
    if (is_empty_string (filename))
    {
      WARN_BAD_FILE_NAME;
      return (l);
    }
    file = fopen (filename, "w");
    free (filename);
  }
  else
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  if (file == NULL)
  {
    WARN_BAD_FILE_NAME;
    return (l);
  }
  bwb_fclose (My->SYSOUT->cfp);
  My->SYSOUT->cfp = file;
  return (l);
}

LineType *
bwb_PTP (LineType * l)
{
  /* SYNTAX: PTP */
  /* SYNTAX: PTP filename$ */
  FILE *file;
  char *filename;
   
  assert (l != NULL);
  assert(My != NULL);
  assert(My->SYSOUT != NULL);
  assert(My->SYSOUT->cfp != NULL);

  file = NULL;
  filename = NULL;
  if (line_is_eol (l))
  {
    /* OK */
    file = fopen ("PTP", "w");
  }
  else if (line_read_string_expression (l, &filename))
  {
    /* OK */
    if (is_empty_string (filename))
    {
      WARN_BAD_FILE_NAME;
      return (l);
    }
    file = fopen (filename, "w");
    free (filename);
  }
  else
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  if (file == NULL)
  {
    WARN_BAD_FILE_NAME;
    return (l);
  }
  bwb_fclose (My->SYSOUT->cfp);
  My->SYSOUT->cfp = file;
  return (l);
}

LineType *
bwb_PTR (LineType * l)
{
  /* SYNTAX: PTR */
  /* SYNTAX: PTR filename$ */
  FILE *file;
  char *filename;
   
  assert (l != NULL);
  assert(My != NULL);
  assert(My->SYSIN != NULL);
  assert(My->SYSIN->cfp != NULL);

  file = NULL;
  filename = NULL;
  if (line_is_eol (l))
  {
    /* OK */
    file = fopen ("PTR", "r");
  }
  else if (line_read_string_expression (l, &filename))
  {
    /* OK */
    if (is_empty_string (filename))
    {
      WARN_BAD_FILE_NAME;
      return (l);
    }
    file = fopen (filename, "r");
    free (filename);
  }
  else
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  if (file == NULL)
  {
    WARN_BAD_FILE_NAME;
    return (l);
  }
  bwb_fclose (My->SYSIN->cfp);
  My->SYSIN->cfp = file;
  return (l);
}

LineType *
bwb_TTY (LineType * l)
{
  /* SYNTAX: TTY */
  assert (l != NULL);
   
  bwb_TTY_IN (l);
  bwb_TTY_OUT (l);
  return (l);
}

LineType *
bwb_TTY_IN (LineType * l)
{
  /* SYNTAX: TTY IN */
  assert (l != NULL);
  assert(My != NULL);
  assert(My->SYSIN != NULL);
  assert(My->SYSIN->cfp != NULL);
   
  bwb_fclose (My->SYSIN->cfp);
  My->SYSIN->cfp = stdin;
  return (l);
}

LineType *
bwb_TTY_OUT (LineType * l)
{
  /* SYNTAX: TTY OUT */
  assert (l != NULL);
  assert(My != NULL);
  assert(My->SYSOUT != NULL);
  assert(My->SYSOUT->cfp != NULL);
   
  bwb_fclose (My->SYSOUT->cfp);
  My->SYSOUT->cfp = stdout;
  return (l);
}

/* 
--------------------------------------------------------------------------------------------
                               CREATE
--------------------------------------------------------------------------------------------
*/

LineType *
bwb_CREATE (LineType * l)
{
  /* SYNTAX: CREATE filename$ [ RECL reclen ] AS filenum [ BUFF number ] [ RECS size ] */
  int FileNumber;
  int width;
  int buffnum;
  int recsnum;
  char *filename;
   
  assert (l != NULL);
  assert(My != NULL);


  FileNumber = 0;
  width = 0;
  buffnum = 0;
  recsnum = 0;
  filename = NULL;
  if (line_read_string_expression (l, &filename) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  if (is_empty_string (filename))
  {
    WARN_BAD_FILE_NAME;
    return (l);
  }
  if (line_skip_word (l, "RECL"))
  {
    if (line_read_integer_expression (l, &width) == FALSE)
    {
      WARN_FIELD_OVERFLOW;
      return (l);
    }
    if (width <= 0)
    {
      WARN_FIELD_OVERFLOW;
      return (l);
    }
  }
  if (line_skip_word (l, "AS") == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
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
  if (line_skip_word (l, "BUFF"))
  {
    if (line_read_integer_expression (l, &buffnum) == FALSE)
    {
      WARN_FIELD_OVERFLOW;
      return (l);
    }
    if (buffnum <= 0)
    {
      WARN_FIELD_OVERFLOW;
      return (l);
    }
  }
  if (line_skip_word (l, "RECS"))
  {
    if (line_read_integer_expression (l, &recsnum) == FALSE)
    {
      WARN_FIELD_OVERFLOW;
      return (l);
    }
    if (recsnum <= 0)
    {
      WARN_FIELD_OVERFLOW;
      return (l);
    }
  }
  /* now, we are ready to create the file */
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
  My->CurrentFile->FileName = filename;
  filename = NULL;
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
  /* truncate to zero length or create text file for update  (reading and writing) */
  if (is_empty_string (My->CurrentFile->FileName))
  {
    WARN_BAD_FILE_NAME;
    return (l);
  }
  if ((My->CurrentFile->cfp =
       fopen (My->CurrentFile->FileName, "w+")) == NULL)
  {
    WARN_BAD_FILE_NAME;
    return (l);
  }
  if (width > 0)
  {
    My->CurrentFile->width = width;
    My->CurrentFile->DevMode = DEVMODE_RANDOM;
  }
  else
  {
    My->CurrentFile->DevMode = DEVMODE_INPUT | DEVMODE_OUTPUT;
  }
  return (l);
}

/* 
--------------------------------------------------------------------------------------------
                               COPY
--------------------------------------------------------------------------------------------
*/

static void
bwb_copy_file (char *Source, char *Target)
{
  FILE *source;
  FILE *target;
   
  source = NULL;
  target = NULL;

  if (is_empty_string (Source))
  {
    WARN_BAD_FILE_NAME;
    goto EXIT;
  }
  if (is_empty_string (Target))
  {
    WARN_BAD_FILE_NAME;
    goto EXIT;
  }
  source = fopen (Source, "rb");
  if (source == NULL)
  {
    WARN_BAD_FILE_NAME;
    goto EXIT;
  }
  target = fopen (Target, "wb");
  if (target == NULL)
  {
    WARN_BAD_FILE_NAME;
    goto EXIT;
  }
  /* OK */
  while (TRUE)
  {
    int C;

    C = fgetc (source);
    if (C < 0 /* EOF */  || feof (source) || ferror (source))
    {
      break;
    }
    fputc (C, target);
    if (ferror (target))
    {
      break;
    }
  }
  /* DONE */
EXIT:
  if (source)
  {
    fclose (source);
  }
  if (target)
  {
    fclose (target);
  }
}

LineType *
bwb_COPY (LineType * Line)
{
  /* SYNTAX: COPY source$ TO target$ */
  char *Source;
  char *Target;
   
  assert (Line != NULL);

  Source = NULL;
  Target = NULL;
  if (line_read_string_expression (Line, &Source) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    goto EXIT;
  }
  if (line_skip_word (Line, "TO") == FALSE)
  {
    WARN_SYNTAX_ERROR;
    goto EXIT;
  }
  if (line_read_string_expression (Line, &Target) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    goto EXIT;
  }
  bwb_copy_file (Source, Target);
EXIT:
  if (Source)
  {
    free (Source);
  }
  if (Target)
  {
    free (Target);
  }
  return (Line);
}

/* 
--------------------------------------------------------------------------------------------
                               DISPLAY
--------------------------------------------------------------------------------------------
*/

static void
bwb_display_file (char *Source)
{
  FILE *source;
   
  assert (My->SYSOUT != NULL);
  assert (My->SYSOUT->cfp != NULL);

  source = NULL;

  if (is_empty_string (Source))
  {
    WARN_BAD_FILE_NAME;
    goto EXIT;
  }
  source = fopen (Source, "rb");
  if (source == NULL)
  {
    WARN_BAD_FILE_NAME;
    goto EXIT;
  }
  /* OK */
  while (TRUE)
  {
    int C;

    C = fgetc (source);
    if (C < 0 /* EOF */  || feof (source) || ferror (source))
    {
      break;
    }
    fputc (C, My->SYSOUT->cfp);
  }
  /* DONE */
EXIT:
  if (source)
  {
    fclose (source);
  }
}

LineType *
bwb_DISPLAY (LineType * Line)
{
  /* SYNTAX: DISPLAY source$  */
  char *Source;
   
  assert (Line != NULL);
  Source = NULL;
  if (line_read_string_expression (Line, &Source) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    goto EXIT;
  }
  bwb_display_file (Source);
EXIT:
  if (Source)
  {
    free (Source);
  }
  return (Line);
}

/* 
--------------------------------------------------------------------------------------------
                               EOF
--------------------------------------------------------------------------------------------
*/



/* EOF */
