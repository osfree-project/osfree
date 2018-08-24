/***************************************************************
  
        bwb_cnd.c       Conditional Expressions and Commands
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

static LineType *bwb_then_else (LineType * l, int Value);
static LineType *bwb_if_file (LineType * l, int ThenValue);
static int FindTopLineOnStack (LineType * l);
static int for_limit_check (DoubleType Value, DoubleType Target,
                            DoubleType Step);
static int IsTypeMismatch (char LeftTypeCode, char RightTypeCode);


/* 
--------------------------------------------------------------------------------------------
                               EXIT
--------------------------------------------------------------------------------------------
*/

LineType *
bwb_EXIT (LineType * l)
{
   
  assert (l != NULL);
  WARN_SYNTAX_ERROR;
  return (l);
}

/* 
--------------------------------------------------------------------------------------------
                               SELECT
--------------------------------------------------------------------------------------------
*/
LineType *
bwb_SELECT (LineType * l)
{
   
  assert (l != NULL);
  WARN_SYNTAX_ERROR;
  return (l);
}


/* 
--------------------------------------------------------------------------------------------
                               FUNCTION - END FUNCTION
--------------------------------------------------------------------------------------------
*/

/***************************************************************
  
        FUNCTION:       bwb_FUNCTION()
  
   DESCRIPTION:    This function implements the BASIC
         FUNCTION command, introducing a named
         function.
  
   SYNTAX: FUNCTION subroutine-name
              ...
              [ EXIT FUNCTION ]
              ...
           END FUNCTION
  
***************************************************************/

LineType *
bwb_FUNCTION (LineType * l)
{
   
  assert (l != NULL);

  if (l->LineFlags & (LINE_USER))
  {
    WARN_ILLEGAL_DIRECT;
    return (l);
  }

  /* check current exec level */
  assert(My != NULL);
  assert(My->StackHead != NULL);
  if (My->StackHead->next == NULL)
  {
    /* skip over the entire function definition */
    l = l->OtherLine;                /* line of END SUB */
    l = l->next;                /* line after END SUB */
    l->position = 0;
    return l;
  }

  /* we are being executed via IntrinsicFunction_deffn() */

  /* if this is the first time at this SUB statement, note it */
  if (My->StackHead->LoopTopLine != l)
  {
    if (bwb_incexec ())
    {
      /* OK */
      My->StackHead->LoopTopLine = l;
    }
    else
    {
      /* ERROR */
      WARN_OUT_OF_MEMORY;
      return My->EndMarker;
    }
  }
  line_skip_eol (l);
  return (l);
}


LineType *
bwb_EXIT_FUNCTION (LineType * l)
{
   
  assert (l != NULL);

  if (l->LineFlags & (LINE_USER))
  {
    WARN_ILLEGAL_DIRECT;
    return (l);
  }

  /* check integrity of SUB commmand */
  if (FindTopLineOnStack (l->OtherLine))
  {
    /* FOUND */
    LineType *r;
    bwb_decexec ();
    r = l->OtherLine;                /* line of FUNCTION */
    r = r->OtherLine;                /* line of END FUNCTION */
    r = r->next;                /* line after END FUNCTION */
    r->position = 0;
    return r;
  }
  /* NOT FOUND */
  WARN_EXIT_FUNCTION_WITHOUT_FUNCTION;
  return (l);
}

LineType *
bwb_END_FUNCTION (LineType * l)
{
   
  assert (l != NULL);

  if (l->LineFlags & (LINE_USER))
  {
    WARN_ILLEGAL_DIRECT;
    return (l);
  }

  /* check integrity of SUB commmand */
  if (FindTopLineOnStack (l->OtherLine) == FALSE)
  {
    /* NOT FOUND */
    WARN_END_FUNCTION_WITHOUT_FUNCTION;
    return (l);
  }
  /* decrement the stack */
  bwb_decexec ();

  /* and return next from old line */
  assert(My != NULL);
  assert(My->StackHead != NULL);
  My->StackHead->line->next->position = 0;
  return My->StackHead->line->next;
}

LineType *
bwb_FNEND (LineType * l)
{
   
  assert (l != NULL);
  return bwb_END_FUNCTION (l);
}

LineType *
bwb_FEND (LineType * l)
{
   
  assert (l != NULL);
  return bwb_END_FUNCTION (l);
}


/* 
--------------------------------------------------------------------------------------------
                               SUB - END SUB
--------------------------------------------------------------------------------------------
*/

/***************************************************************
  
        FUNCTION:       bwb_sub()
  
   DESCRIPTION:    This function implements the BASIC
         SUB command, introducing a named
         subroutine.
  
   SYNTAX: SUB subroutine-name
              ...
              [ EXIT SUB ]
              ...
           END SUB
  
***************************************************************/

LineType *
bwb_SUB (LineType * l)
{
   
  assert (l != NULL);

  if (l->LineFlags & (LINE_USER))
  {
    WARN_ILLEGAL_DIRECT;
    return (l);
  }

  /* check current exec level */
  assert(My != NULL);
  assert(My->StackHead != NULL);
  if (My->StackHead->next == NULL)
  {
    /* skip over the entire function definition */
    l = l->OtherLine;                /* line of END SUB */
    l = l->next;                /* line after END SUB */
    l->position = 0;
    return l;
  }
  /* we are being executed via IntrinsicFunction_deffn() */

  /* if this is the first time at this SUB statement, note it */
  if (My->StackHead->LoopTopLine != l)
  {
    if (bwb_incexec ())
    {
      /* OK */
      My->StackHead->LoopTopLine = l;
    }
    else
    {
      /* ERROR */
      WARN_OUT_OF_MEMORY;
      return My->EndMarker;
    }
  }
  line_skip_eol (l);
  return (l);
}

LineType *
bwb_EXIT_SUB (LineType * l)
{
   
  assert (l != NULL);

  if (l->LineFlags & (LINE_USER))
  {
    WARN_ILLEGAL_DIRECT;
    return (l);
  }

  /* check integrity of SUB commmand */
  if (FindTopLineOnStack (l->OtherLine))
  {
    /* FOUND */
    LineType *r;
    bwb_decexec ();
    r = l->OtherLine;                /* line of FUNCTION */
    r = r->OtherLine;                /* line of END FUNCTION */
    r = r->next;                /* line after END FUNCTION */
    r->position = 0;
    return r;
  }
  /* NOT FOUND */
  WARN_EXIT_SUB_WITHOUT_SUB;
  return (l);
}

LineType *
bwb_SUBEXIT (LineType * l)
{
   
  assert (l != NULL);
  return bwb_EXIT_SUB (l);
}

LineType *
bwb_SUB_EXIT (LineType * l)
{
   
  assert (l != NULL);
  return bwb_EXIT_SUB (l);
}

LineType *
bwb_END_SUB (LineType * l)
{
   
  assert (l != NULL);

  if (l->LineFlags & (LINE_USER))
  {
    WARN_ILLEGAL_DIRECT;
    return (l);
  }

  /* check integrity of SUB commmand */
  if (FindTopLineOnStack (l->OtherLine) == FALSE)
  {
    /* NOT FOUND */
    WARN_END_SUB_WITHOUT_SUB;
    return (l);
  }
  /* decrement the stack */
  bwb_decexec ();

  /* and return next from old line */
  assert(My != NULL);
  assert(My->StackHead != NULL);
  My->StackHead->line->next->position = 0;
  return My->StackHead->line->next;
}

LineType *
bwb_SUBEND (LineType * l)
{
   
  assert (l != NULL);
  return bwb_END_SUB (l);
}

LineType *
bwb_SUB_END (LineType * l)
{
   
  assert (l != NULL);
  return bwb_END_SUB (l);
}


/* 
--------------------------------------------------------------------------------------------
                                IF - END IF
--------------------------------------------------------------------------------------------
*/


/***************************************************************
  
        FUNCTION:       bwb_IF()
  
        DESCRIPTION:    This function handles the BASIC IF
                        statement, standard flavor.
  standard
      SYNTAX:     IF expression  THEN line [ELSE line]
                  IF END  # file THEN line [ELSE line]
                  IF MORE # file THEN line [ELSE line]
  
***************************************************************/
LineType *
bwb_IF (LineType * l)
{
  /* classic IF */
  /* IF expression THEN 100          */
  /* IF expression THEN 100 ELSE 200 */
  int Value;
   
  assert (l != NULL);

  if (l->LineFlags & (LINE_USER))
  {
    WARN_ILLEGAL_DIRECT;
    return (l);
  }

  if (line_read_integer_expression (l, &Value) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  return bwb_then_else (l, Value);
}

LineType *
bwb_IF_END (LineType * l)
{
  /* IF END #1 THEN 100          */
   
  assert (l != NULL);

  if (l->LineFlags & (LINE_USER))
  {
    WARN_ILLEGAL_DIRECT;
    return (l);
  }

  assert(My != NULL);
  assert(My->CurrentVersion != NULL);
  if (My->CurrentVersion->OptionVersionValue & (C77))
  {
    /* sets a linenumber to branch to on EOF */
    int FileNumber = 0;
    int LineNumber = 0;


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
    if (line_skip_word (l, "THEN") == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    if (line_read_integer_expression (l, &LineNumber) == FALSE)
    {
      WARN_UNDEFINED_LINE;
      return (l);
    }
    if (LineNumber < 0)
    {
      WARN_UNDEFINED_LINE;
      return (l);
    }
    /* now, we are ready to create the file */
    My->CurrentFile = find_file_by_number (FileNumber);
    if (My->CurrentFile == NULL)
    {
      My->CurrentFile = file_new ();
      My->CurrentFile->FileNumber = FileNumber;
    }
    My->CurrentFile->EOF_LineNumber = LineNumber;
    return (l);
  }
  /* branch to the line if we are currently at EOF */
  return bwb_if_file (l, TRUE);
}

LineType *
bwb_IF_MORE (LineType * l)
{
  /* IF MORE #1 THEN 100          */
   
  assert (l != NULL);

  if (l->LineFlags & (LINE_USER))
  {
    WARN_ILLEGAL_DIRECT;
    return (l);
  }
  /* branch to the line if we are not currently at EOF */
  return bwb_if_file (l, FALSE);
}





/***************************************************************
  
        FUNCTION:       bwb_IF8THEN()
  
        DESCRIPTION:    This function handles the BASIC IF
                        statement, structured flavor.
  
      SYNTAX:     IF expression THEN
                     ...
                  ELSEIF expression
                     ...
                  ELSE
                     ...
                  END IF
  
***************************************************************/
LineType *
bwb_IF8THEN (LineType * l)
{
  /* structured IF */
  LineType *else_line;
  int Value;
   
  assert (l != NULL);

  if (l->LineFlags & (LINE_USER))
  {
    WARN_ILLEGAL_DIRECT;
    return (l);
  }

  /* evaluate the expression */
  if (line_read_integer_expression (l, &Value) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  if (Value)
  {
    /* expression is TRUE */
    l->next->position = 0;
    return l->next;
  }

  /* 
     RESUME knows we iterate thru the various ELSEIF commands, and restarts at the IF THEN command.
     RESUME NEXT knows we iterate thru the various ELSEIF commands, and restarts at the END IF command.
   */

  for (else_line = l->OtherLine; else_line->cmdnum == C_ELSEIF;
       else_line = else_line->OtherLine)
  {
    else_line->position = else_line->Startpos;

    /* evaluate the expression */
    if (line_read_integer_expression (else_line, &Value) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
    if (Value)
    {
      /* expression is TRUE */
      else_line->next->position = 0;
      return else_line->next;
    }
  }
  /* ELSE or END IF */
  else_line->next->position = 0;
  return else_line->next;
}

LineType *
bwb_ELSEIF (LineType * l)
{
   
  assert (l != NULL);

  if (l->LineFlags & (LINE_USER))
  {
    WARN_ILLEGAL_DIRECT;
    return (l);
  }

  for (l = l->OtherLine; l->OtherLine != NULL; l = l->OtherLine);
  l = l->next;                        /* line after END IF */
  l->position = 0;
  return l;
}

LineType *
bwb_ELSE (LineType * l)
{
   
  assert (l != NULL);

  if (l->LineFlags & (LINE_USER))
  {
    WARN_ILLEGAL_DIRECT;
    return (l);
  }

  for (l = l->OtherLine; l->OtherLine != NULL; l = l->OtherLine);
  l = l->next;                        /* line after END IF */
  l->position = 0;
  return l;
}

LineType *
bwb_END_IF (LineType * l)
{
   
  assert (l != NULL);

  if (l->LineFlags & (LINE_USER))
  {
    WARN_ILLEGAL_DIRECT;
    return (l);
  }
  return (l);
}

/* 
--------------------------------------------------------------------------------------------
                                 SELECT CASE - END SELECT
--------------------------------------------------------------------------------------------
*/


/***************************************************************
  
        FUNCTION:       bwb_select()
  
        DESCRIPTION:    This C function handles the BASIC SELECT
                        statement.
  
   SYNTAX:     SELECT CASE expression           ' examples:
               CASE value                       ' CASE 5
               CASE min TO max                  ' CASE 1 TO 10
               CASE IF relationaloperator value ' CASE IF > 5
               CASE IS relationaloperator value ' CASE IS > 5
               CASE ELSE
               END SELECT
  
***************************************************************/



LineType *
bwb_SELECT_CASE (LineType * l)
{
  VariantType selectvalue;
  VariantType *e;
  LineType *else_line;
   
  assert (l != NULL);

  e = &selectvalue;
  CLEAR_VARIANT (e);
  if (l->LineFlags & (LINE_USER))
  {
    WARN_ILLEGAL_DIRECT;
    return (l);
  }

  /* evaluate the expression */
  if (line_read_expression (l, e) == FALSE)        /* bwb_SELECT_CASE */
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  /* 
   **
   ** RESUME knows we iterate thru the various CASE commands, and restarts at the SELECT CASE command.
   ** RESUME NEXT knows we iterate thru the various CASE commands, and restarts at the END SELECT command.
   **
   */
  for (else_line = l->OtherLine; else_line->cmdnum == C_CASE;
       else_line = else_line->OtherLine)
  {
    else_line->position = else_line->Startpos;
    do
    {
      /* evaluate the expression */
      if (line_skip_word (else_line, "IF")
          || line_skip_word (else_line, "IS"))
      {
        /* CASE IS < 10    */
        /* CASE IF < "DEF" */
        /* CASE IS > 7     */
        /* CASE IS > "ABC" */
        char *tbuf;
        int tlen;
        size_t n;                /* number of characters we want to put in tbuf */
        int position;
        VariantType casevalue;
        VariantType *r;

        assert(My != NULL);
        assert(My->ConsoleOutput != NULL);
        assert(MAX_LINE_LENGTH > 1);
        tbuf = My->ConsoleOutput;
        tlen = MAX_LINE_LENGTH;
        n = 0;
        r = &casevalue;
        CLEAR_VARIANT (r);

        /*
         ** 
         ** Available choices:
         ** 1.  Parse every possible operator combination, depending upon the BASIC flavor.
         ** 2.  Jump into the middle of the expression parser, by exposing the parser internals.
         ** 3.  Limit the length of the expression.  This is the choice I made.
         **
         */

        if (e->VariantTypeCode == StringTypeCode)
        {
          /* STRING */
          n += bwb_strlen (e->Buffer);
          if (n > tlen)
          {
            WARN_STRING_FORMULA_TOO_COMPLEX;        /* bwb_SELECT_CASE */
            return (l);
          }
          /* OK , everything will fit */
          bwb_strcpy (tbuf, e->Buffer);
        }
        else
        {
          /* NUMBER */
          FormatBasicNumber (e->Number, tbuf);
          n += bwb_strlen (tbuf);
          if (n > tlen)
          {
            WARN_STRING_FORMULA_TOO_COMPLEX;        /* bwb_SELECT_CASE */
            return (l);
          }
          /* OK , everything will fit */
        }
        {
          char *Space;

          Space = " ";
          n += bwb_strlen (Space);
          if (n > tlen)
          {
            WARN_STRING_FORMULA_TOO_COMPLEX;        /* bwb_SELECT_CASE */
            return (l);
          }
          /* OK , everything will fit */
          bwb_strcat (tbuf, Space);
        }
        {
          n += bwb_strlen (&(else_line->buffer[else_line->position]));

          if (n > tlen)
          {
            WARN_STRING_FORMULA_TOO_COMPLEX;        /* bwb_SELECT_CASE */
            return (l);
          }
          /* OK , everything will fit */
          bwb_strcat (tbuf, &(else_line->buffer[else_line->position]));
        }
        position = 0;
        if (buff_read_expression (tbuf, &position, r) == FALSE)        /* bwb_SELECT_CASE */
        {
          WARN_SYNTAX_ERROR;
          return (l);
        }
        if (r->VariantTypeCode == StringTypeCode)
        {
          RELEASE_VARIANT (r);
          WARN_TYPE_MISMATCH;
          return (l);
        }
        if (r->Number)
        {
          /* expression is TRUE */
          else_line->next->position = 0;
          return else_line->next;
        }
        /* condition is FALSE */
        /* proceed to next CASE line if there is one */
      }
      else
      {
        /* CASE 7 */
        /* CASE 7 TO 10 */
        /* CASE "ABC" */
        /* CASE "ABC" TO "DEF" */
        VariantType minvalue;
        VariantType *minval;

        minval = &minvalue;
        CLEAR_VARIANT (minval);
        /* evaluate the MIN expression */
        if (line_read_expression (else_line, minval) == FALSE)        /* bwb_SELECT_CASE */
        {
          WARN_SYNTAX_ERROR;
          return (l);
        }
        if (IsTypeMismatch (e->VariantTypeCode, minval->VariantTypeCode))
        {
          RELEASE_VARIANT (minval);
          WARN_TYPE_MISMATCH;
          return (l);
        }
        if (line_skip_word (else_line, "TO"))
        {
          /* CASE 7 TO 10 */
          /* CASE "ABC" TO "DEF" */
          VariantType maxvalue;
          VariantType *maxval;

          maxval = &maxvalue;
          CLEAR_VARIANT (maxval);

          /* evaluate the MAX expression */
          if (line_read_expression (else_line, maxval) == FALSE)        /* bwb_SELECT_CASE */
          {
            WARN_SYNTAX_ERROR;
            return (l);
          }
          if (IsTypeMismatch (e->VariantTypeCode, maxval->VariantTypeCode))
          {
            RELEASE_VARIANT (maxval);
            WARN_TYPE_MISMATCH;
            return (l);
          }
          if (e->VariantTypeCode == StringTypeCode)
          {
            /* STRING */
            if (bwb_strcmp (e->Buffer, minval->Buffer) >= 0
                && bwb_strcmp (e->Buffer, maxval->Buffer) <= 0)
            {
              /* expression is TRUE */
              RELEASE_VARIANT (maxval);
              else_line->next->position = 0;
              return else_line->next;
            }
            RELEASE_VARIANT (maxval);
          }
          else
          {
            /* NUMBER */
            if (e->Number >= minval->Number && e->Number <= maxval->Number)
            {
              /* expression is TRUE */
              else_line->next->position = 0;
              return else_line->next;
            }
          }
        }
        else
        {
          /* CASE 7 */
          /* CASE "ABC" */
          if (e->VariantTypeCode == StringTypeCode)
          {
            /* STRING */
            if (bwb_strcmp (e->Buffer, minval->Buffer) == 0)
            {
              /* expression is TRUE */
              RELEASE_VARIANT (minval);
              else_line->next->position = 0;
              return else_line->next;
            }
            RELEASE_VARIANT (minval);
          }
          else
          {
            /* NUMBER */
            if (e->Number == minval->Number)
            {
              /* expression is TRUE */
              else_line->next->position = 0;
              return else_line->next;
            }
          }
        }
        /* condition is FALSE */
        /* proceed to next CASE line if there is one */
      }
    }
    while (line_skip_seperator (else_line));
  }
  /* CASE_ELSE or END_SELECT */
  RELEASE_VARIANT (e);
  else_line->next->position = 0;
  return else_line->next;
}

LineType *
bwb_CASE (LineType * l)
{
   
  assert (l != NULL);

  if (l->LineFlags & (LINE_USER))
  {
    WARN_ILLEGAL_DIRECT;
    return (l);
  }

  for (l = l->OtherLine; l->OtherLine != NULL; l = l->OtherLine);
  l = l->next;                        /* line after END SELECT */
  l->position = 0;
  return l;
}

LineType *
bwb_CASE_ELSE (LineType * l)
{
   
  assert (l != NULL);

  if (l->LineFlags & (LINE_USER))
  {
    WARN_ILLEGAL_DIRECT;
    return (l);
  }

  for (l = l->OtherLine; l->OtherLine != NULL; l = l->OtherLine);
  l = l->next;                        /* line after END SELECT */
  l->position = 0;
  return l;
}


LineType *
bwb_END_SELECT (LineType * l)
{
   
  assert (l != NULL);

  if (l->LineFlags & (LINE_USER))
  {
    WARN_ILLEGAL_DIRECT;
    return (l);
  }

  return (l);
}

/* 
--------------------------------------------------------------------------------------------
                                DO - LOOP
--------------------------------------------------------------------------------------------
*/

/***************************************************************
  
   FUNCTION:       bwb_DO()
  
   DESCRIPTION:    This C function implements the ANSI BASIC
         DO statement.
  
   SYNTAX:     DO [UNTIL|WHILE condition]
                  ...
                  [EXIT DO]
                  ...
               LOOP [UNTIL|WHILE condition]
  
***************************************************************/

LineType *
bwb_DO (LineType * l)
{
  LineType *r;
  int Value;
   
  assert (l != NULL);

  /* DO ' forever */
  /* DO UNTIL ' exits when != 0 */
  /* DO WHILE ' exits when == 0 */

  if (l->LineFlags & (LINE_USER))
  {
    WARN_ILLEGAL_DIRECT;
    return (l);
  }

  do
  {
    /* evaluate the expression */
    if (line_is_eol (l))
    {
      break;                        /* exit 'do' */
    }
    else if (line_skip_word (l, "UNTIL"))
    {
      /* DO UNTIL */
      if (line_read_integer_expression (l, &Value) == FALSE)
      {
        WARN_SYNTAX_ERROR;
        return (l);
      }

      if (Value != 0)
      {
        /* EXIT DO */
        r = l->OtherLine;        /* line of LOOP */
        r = r->next;                /* line after LOOP */
        r->position = 0;
        return r;
      }
    }
    else if (line_skip_word (l, "WHILE"))
    {
      /* DO WHILE */
      if (line_read_integer_expression (l, &Value) == FALSE)
      {
        WARN_SYNTAX_ERROR;
        return (l);
      }

      if (Value == 0)
      {
        /* EXIT DO */
        r = l->OtherLine;        /* line of LOOP */
        r = r->next;                /* line after LOOP */
        r->position = 0;
        return r;
      }
    }

  }
  while (line_skip_seperator (l));

  return (l);
}


LineType *
bwb_EXIT_DO (LineType * l)
{
  LineType *r;
   
  assert (l != NULL);

  /* EXIT DO */

  if (l->LineFlags & (LINE_USER))
  {
    WARN_ILLEGAL_DIRECT;
    return (l);
  }

  r = l->OtherLine;                /* line of DO */
  r = r->OtherLine;                /* line of LOOP */
  r = r->next;                        /* line after LOOP */
  r->position = 0;
  return r;
}


LineType *
bwb_LOOP (LineType * l)
{
  LineType *r;
  int Value;
   
  assert (l != NULL);

  /* LOOP ' forever */
  /* LOOP UNTIL ' exits when != 0 */
  /* LOOP WHILE ' exits when == 0 */

  if (l->LineFlags & (LINE_USER))
  {
    WARN_ILLEGAL_DIRECT;
    return (l);
  }

  do
  {
    /* evaluate the expression */
    if (line_is_eol (l))
    {
      break;                        /* exit 'do' */
    }
    else if (line_skip_word (l, "UNTIL"))
    {
      /* LOOP UNTIL */
      if (line_read_integer_expression (l, &Value) == FALSE)
      {
        WARN_SYNTAX_ERROR;
        return (l);
      }

      if (Value != 0)
      {
        /* EXIT DO */
        return (l);
      }
    }
    else if (line_skip_word (l, "WHILE"))
    {
      /* LOOP WHILE */
      if (line_read_integer_expression (l, &Value) == FALSE)
      {
        WARN_SYNTAX_ERROR;
        return (l);
      }

      if (Value == 0)
      {
        /* EXIT DO */
        return (l);
      }
    }

  }
  while (line_skip_seperator (l));

  /* loop around to DO again */
  r = l->OtherLine;                /* line of DO */
  r->position = 0;
  return r;
}



/* 
--------------------------------------------------------------------------------------------
                                WHILE - WEND
--------------------------------------------------------------------------------------------
*/




/***************************************************************
  
        FUNCTION:       bwb_WHILE()
  
        DESCRIPTION:    This function handles the BASIC
                        WHILE statement.
  
   SYNTAX:     WHILE expression ' exits when == 0
                 ...
                 [EXIT WHILE]
                 ...
               WEND
  
  
***************************************************************/
LineType *
bwb_WHILE (LineType * l)
{
  int Value;

  LineType *r;
   
  assert (l != NULL);

  if (l->LineFlags & (LINE_USER))
  {
    WARN_ILLEGAL_DIRECT;
    return (l);
  }

  if (line_read_integer_expression (l, &Value) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  if (Value == 0)
  {
    /* EXIT WHILE */
    r = l->OtherLine;                /* line of WEND */
    r = r->next;                /* line after WEND */
    r->position = 0;
    return r;
  }
  return (l);
}


LineType *
bwb_EXIT_WHILE (LineType * l)
{
  LineType *r;
   
  assert (l != NULL);

  /* EXIT WHILE */

  if (l->LineFlags & (LINE_USER))
  {
    WARN_ILLEGAL_DIRECT;
    return (l);
  }

  r = l->OtherLine;                /* line of WHILE */
  r = r->OtherLine;                /* line of WEND */
  r = r->next;                        /* line after WEND */
  r->position = 0;
  return r;
}

LineType *
bwb_WEND (LineType * l)
{
  LineType *r;
   
  assert (l != NULL);

  if (l->LineFlags & (LINE_USER))
  {
    WARN_ILLEGAL_DIRECT;
    return (l);
  }

  r = l->OtherLine;                /* line of WHILE */
  r->position = 0;
  return r;
}


/* 
--------------------------------------------------------------------------------------------
                                REPEAT - UNTIL
--------------------------------------------------------------------------------------------
*/







/***************************************************************
  
        FUNCTION:       bwb_UNTIL()
  
        DESCRIPTION:    This function handles the BASIC 
                         UNTIL statement.
  
   SYNTAX:     UNTIL expression ' exits when != 0
                 ...
                 [EXIT UNTIL]
                 ...
               UEND
                       
  
***************************************************************/
LineType *
bwb_REPEAT (LineType * l)
{
   
  assert (l != NULL);

  if (l->LineFlags & (LINE_USER))
  {
    WARN_ILLEGAL_DIRECT;
    return (l);
  }

  return (l);
}


LineType *
bwb_EXIT_REPEAT (LineType * l)
{
  LineType *r;
   
  assert (l != NULL);

  /* EXIT REPEAT */

  if (l->LineFlags & (LINE_USER))
  {
    WARN_ILLEGAL_DIRECT;
    return (l);
  }

  r = l->OtherLine;                /* line of REPEAT */
  r = r->OtherLine;                /* line of UNTIL */
  r = r->next;                        /* line after UNTIL */
  r->position = 0;
  return r;
}


LineType *
bwb_UNTIL (LineType * l)
{
  int Value;
   
  assert (l != NULL);

  if (l->LineFlags & (LINE_USER))
  {
    WARN_ILLEGAL_DIRECT;
    return (l);
  }

  if (line_read_integer_expression (l, &Value) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }

  if (Value == 0)
  {
    /* GOTO REPEAT */
    LineType *r;

    r = l->OtherLine;                /* line of REPEAT */
    r->position = 0;
    return r;
  }
  /* EXITS when Value != 0 */
  return (l);

}


/* 
--------------------------------------------------------------------------------------------
                                FOR - NEXT
--------------------------------------------------------------------------------------------
*/



/***************************************************************
  
        FUNCTION:       bwb_for()
  
        DESCRIPTION:    This function handles the BASIC FOR
                        statement.
  
   SYNTAX:     FOR counter = start TO finish [STEP increment]
                 ...
                 [EXIT FOR]
                 ...
               NEXT [counter]
  
NOTE:    This is controlled by the OptionVersion bitmask.
  
    The order of expression evaluation and variable creation varies.
    For example:
        FUNCTION FNA( Y )
            PRINT "Y="; Y
            FNA = Y
        END FUNCTION
        FOR X = FNA(3) TO FNA(1) STEP FNA(2)
        NEXT X
    ANSI/ECMA;
        Y= 1
        Y= 2
        Y= 3
        X is created (if it does not exist)
        X is assigned the value of 3
    MICROSOFT;
        X is created (if it does not exist)
        Y= 3
        X is assigned the value of 3
        Y= 1
        Y= 2
  
  
ECMA-55: Section 13.4
       ...
       The action of the for-statement and the next-statement is de-
       fined in terms of other statements, as follows:
  
              FOR v = initial-value TO limit STEP increment
              (block)
              NEXT v
  
       is equivalent to:
  
              LET own1 = limit
              LET own2 = increment
              LET v = initial-value
       line1  IF (v-own1) * SGN (own2) > 0 THEN line2
              (block)
              LET v = v + own2
              GOTO line1
       line2  REM continued in sequence
       ...
  
***************************************************************/


LineType *
bwb_FOR (LineType * l)
{
  LineType *r;
  VariableType *v;
  DoubleType Value;
  DoubleType Target;
  DoubleType Step;
  VariantType variant;
  CLEAR_VARIANT (&variant);
   
  assert (l != NULL);
  assert(My != NULL);
  assert(My->CurrentVersion != NULL);

  if (l->LineFlags & (LINE_USER))
  {
    WARN_ILLEGAL_DIRECT;
    return (l);
  }

  /* if this is the first time at this FOR statement, note it */
  if (FindTopLineOnStack (l) == FALSE)
  {
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
  }

  /* INITIALIZE */


  if ((v = line_read_scalar (l)) == NULL)
  {
    WARN_VARIABLE_NOT_DECLARED;
    return (l);
  }
  if (v->dimensions > 0)
  {
    WARN_TYPE_MISMATCH;
    return (l);
  }
  if (v->VariableTypeCode == StringTypeCode)
  {
    WARN_TYPE_MISMATCH;
    return (l);
  }
  if (line_skip_EqualChar (l) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  if (line_read_numeric_expression (l, &Value) == FALSE)
  {
    WARN_ILLEGAL_FUNCTION_CALL;
    return (l);
  }
  if (My->CurrentVersion->OptionFlags & OPTION_BUGS_ON /* FOR X = ... */ )
  {
    /* Assign Variable */
    variant.VariantTypeCode = v->VariableTypeCode;
    variant.Number = Value;
    if (var_set (v, &variant) == FALSE)
    {
      WARN_VARIABLE_NOT_DECLARED;
      return (l);
    }
  }
  else
  {
    /* assigned below */
  }
  if (line_skip_word (l, "TO") == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }
  if (line_read_numeric_expression (l, &Target) == FALSE)
  {
    WARN_ILLEGAL_FUNCTION_CALL;
    return (l);
  }
  if (line_skip_word (l, "STEP"))
  {
    if (line_read_numeric_expression (l, &Step) == FALSE)
    {
      WARN_ILLEGAL_FUNCTION_CALL;
      return (l);
    }
  }
  else
  {
    Step = 1;
  }
  if (My->CurrentVersion->OptionFlags & OPTION_BUGS_ON /* FOR X = ... */ )
  {
    /* assigned above */
  }
  else
  {
    /* Assign Variable */
    variant.VariantTypeCode = v->VariableTypeCode;
    variant.Number = Value;
    if (var_set (v, &variant) == FALSE)
    {
      WARN_VARIABLE_NOT_DECLARED;
      return (l);
    }
  }

  /* CHECK */
  if (for_limit_check (Value, Target, Step))
  {
    /* EXIT FOR */
    bwb_decexec ();

    r = l->OtherLine;                /* line of NEXT */
    r = r->next;                /* line after NEXT */
    r->position = 0;
    return r;
  }

  /* we will loop at least once */
  assert(My->StackHead != NULL);
  My->StackHead->line = l;
  My->StackHead->ExecCode = EXEC_FOR;
  My->StackHead->local_variable = v;
  My->StackHead->for_step = Step;
  My->StackHead->for_target = Target;
  My->StackHead->LoopTopLine = l;
  My->StackHead->OnErrorGoto = 0;
  /* proceed with processing */
  return (l);
}


LineType *
bwb_EXIT_FOR (LineType * l)
{
  LineType *r;
   
  assert (l != NULL);

  if (l->LineFlags & (LINE_USER))
  {
    WARN_ILLEGAL_DIRECT;
    return (l);
  }

  if (FindTopLineOnStack (l->OtherLine) == FALSE)
  {
    WARN_EXIT_FOR_WITHOUT_FOR;
    return (l);
  }
  assert(My != NULL);
  assert(My->StackHead != NULL);
  My->StackHead->ExecCode = EXEC_FOR;
  bwb_decexec ();

  r = l->OtherLine;                /* line of FOR */
  r = r->OtherLine;                /* line of NEXT */
  r = r->next;                        /* line after NEXT */
  r->position = 0;
  return r;
}


LineType *
bwb_NEXT (LineType * l)
{
  LineType *r;
  VariableType *v;
  DoubleType Value;
  DoubleType Target;
  DoubleType Step;
   
  assert (l != NULL);

  if (l->LineFlags & (LINE_USER))
  {
    WARN_ILLEGAL_DIRECT;
    return (l);
  }

  if (FindTopLineOnStack (l->OtherLine) == FALSE)
  {
    WARN_NEXT_WITHOUT_FOR;
    return (l);
  }
  assert(My != NULL);
  assert(My->StackHead != NULL);
  My->StackHead->ExecCode = EXEC_FOR;

  /* INCREMENT */
  v = My->StackHead->local_variable;
  Target = My->StackHead->for_target;
  Step = My->StackHead->for_step;

  /* if( TRUE ) */
  {
    VariantType variant;
    CLEAR_VARIANT (&variant);
    if (var_get (v, &variant) == FALSE)
    {
      WARN_NEXT_WITHOUT_FOR;
      return (l);
    }
    if (variant.VariantTypeCode == StringTypeCode)
    {
      WARN_NEXT_WITHOUT_FOR;
      return (l);
    }
    variant.Number += Step;
    Value = variant.Number;
    if (var_set (v, &variant) == FALSE)
    {
      WARN_NEXT_WITHOUT_FOR;
      return (l);
    }
  }

  /* CHECK */
  if (for_limit_check (Value, Target, Step))
  {
    /* EXIT FOR */
    bwb_decexec ();
    return (l);
  }
  /* proceed with processing */
  r = l->OtherLine;                /* line of FOR */
#if FALSE                        /* keep this ... */
  /* 
     This example causes a Syntax Error:
     100 FOR I = 1 TO 1000:NEXT
     The error is actually caused by execline().
     Note that the example is a delay loop.
     Only NEXT has this issue, because it jumps to TOP->next.
     All other loop structures jump to either TOP or BOTTOM->next.
   */
  r = r->next;                        /* line after FOR */
  r->position = 0;
#endif
  line_skip_eol (r);
  return r;
}


/* 
--------------------------------------------------------------------------------------------
                                STATIC UTILITY ROUTINES
--------------------------------------------------------------------------------------------
*/


static int
FindTopLineOnStack (LineType * l)
{
  /* since we are at the top of a loop, we MIGHT be on the stack */
  StackType *StackItem;
   
  assert (l != NULL);
  assert(My != NULL);

  for (StackItem = My->StackHead; StackItem != NULL;
       StackItem = StackItem->next)
  {
    LineType *current;

    current = StackItem->LoopTopLine;
    if (current != NULL)
    {
      if (current == l)
      {
        /* FOUND */
        while (My->StackHead != StackItem)
        {
          bwb_decexec ();
        }
        /* we are now the top item on the stack */
        return TRUE;
      }
      /* do NOT cross a function/sub boundary */
      switch (current->cmdnum)
      {
      case C_FUNCTION:
      case C_SUB:
      case C_GOSUB:
        /* NOT FOUND */
        return FALSE;
        /* break; */
      }
    }
  }
  /* NOT FOUND */
  return FALSE;
}

static LineType *
bwb_if_file (LineType * l, int ThenValue)
{
  /* IF END  # filenumber THEN linenumber */
  /* IF MORE # filenumber THEN linenumber */
  int Value;
  int FileNumber;
   
  assert (l != NULL);


  if (line_skip_FilenumChar (l))
  {
    /* IF END # */
    FileType *F;

    if (line_read_integer_expression (l, &FileNumber) == FALSE)
    {
      WARN_BAD_FILE_NUMBER;
      return (l);
    }
    if (FileNumber < 0)
    {
      /* Printer is NOT EOF */
      Value = FALSE;
    }
    else if (FileNumber == 0)
    {
      /* Console is NOT EOF */
      Value = FALSE;
    }
    else
    {
      /* normal file */
      F = find_file_by_number (FileNumber);
      if (F == NULL)
      {
        WARN_BAD_FILE_NUMBER;
        return (l);
      }
      /* if( TRUE ) */
      {
        /* actual file -- are we at the end? */
        FILE *fp;
        long current;
        long total;
        fp = F->cfp;
        assert( fp != NULL );
        current = ftell (fp);
        fseek (fp, 0, SEEK_END);
        total = ftell (fp);
        if (total == current)
        {
          /* EOF */
          Value = TRUE;
        }
        else
        {
          /* NOT EOF */
          Value = FALSE;
          fseek (fp, current, SEEK_SET);
        }
      }
    }
  }
  else
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }

  if (Value == ThenValue)
  {
    /* expression is TRUE, take THEN path */
    return bwb_then_else (l, TRUE);
  }
  /* expression is FALSE, take ELSE path */
  return bwb_then_else (l, FALSE);
}

static LineType *
bwb_then_else (LineType * l, int Value)
{
  /* 
     ... THEN 100 
     ... THEN 100 ELSE 200 

     The deciding expression has already been parsed and evaluated.
     If Value != 0, then we want to take the THEN path.
     If Value == 0, then we want to take the ELSE path.
   */
  int LineNumber;
  LineType *x;
   
  assert (l != NULL);

  if (line_skip_seperator (l))
  {
    /* OK */
  }
  else
  {
    /* OPTIONAL */
  }

  if (line_skip_word (l, "THEN"))
  {
    /* OK */
  }
  else if (line_skip_word (l, "GOTO"))
  {
    /* OK */
  }
  else
  {
    /* REQUIRED */
    WARN_SYNTAX_ERROR;
    return (l);
  }

  /* read THEN's LineNumber */
  if (line_read_integer_expression (l, &LineNumber) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return (l);
  }

  if (Value == 0)
  {
    /* expression is FALSE, take ELSE path */
    if (line_is_eol (l))
    {
      /* OPTIONAL */
      return (l);
    }

    if (line_skip_seperator (l))
    {
      /* OK */
    }
    else
    {
      /* OPTIONAL */
    }

    if (line_skip_word (l, "ELSE"))
    {
      /* OK */
    }
    else
    {
      /* REQUIRED */
      WARN_SYNTAX_ERROR;
      return (l);
    }

    if (line_read_integer_expression (l, &LineNumber) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return (l);
    }
  }

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
    x = find_line_number (LineNumber);        /* bwb_then_else */
  }
  if (x != NULL)
  {
    line_skip_eol (l);
    x->position = 0;
#if THE_PRICE_IS_RIGHT
    l->OtherLine = x;                /* save in cache */
#endif /* THE_PRICE_IS_RIGHT */
    return x;
  }
  WARN_SYNTAX_ERROR;
  return (l);

}

static int
IsTypeMismatch (char LeftTypeCode, char RightTypeCode)
{
   
  if (LeftTypeCode == StringTypeCode && RightTypeCode == StringTypeCode)
  {
    /* both STRING */
    return FALSE;
  }
  if (LeftTypeCode != StringTypeCode && RightTypeCode != StringTypeCode)
  {
    /* both NUMBER */
    return FALSE;
  }
  /* TYPE MISMATCH */
  return TRUE;
}

static int
for_limit_check (DoubleType Value, DoubleType Target, DoubleType Step)
{
   
  if (Step > 0)
  {
    /* POSITIVE */
    if (Value > Target)
    {
      /* FOR I = 3 TO 2 STEP 1 */
      return TRUE;
    }
  }
  else
  {
    /* NEGATIVE */
    if (Value < Target)
    {
      /* FOR I = -3 TO -2 STEP -1 */
      return TRUE;
    }
  }
  return FALSE;
}

/* EOF */
