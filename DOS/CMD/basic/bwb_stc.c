/***************************************************************
  
        bwb_stc.c       Commands Related to Structured Programming
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

static int ErrorMessage (LineType * current);
static LineType *FindParentCommand (int cmdnum, unsigned int Indention,
                                    LineType * Previous[]);
static LineType *find_BottomLineInCode (LineType * l);
static int MissingBottomLine (LineType * current, int cmdnum);
static int scan_readargs (UserFunctionType * f, LineType * l);
static int UserFunction_clear (void);

/***************************************************************
  
   FUNCTION:       UserFunction_init()
  
   DESCRIPTION:    This function initializes the FUNCTION-SUB
         lookup table.
  
***************************************************************/

extern int
UserFunction_init (void)
{
  assert( My != NULL );

  My->UserFunctionHead = NULL;
  return TRUE;
}

/***************************************************************
  
        FUNCTION:       bwb_scan()
  
        DESCRIPTION: This function scans all lines of the
            program in memory and creates a FUNCTION-
            SUB lookup table (fslt) for the program.
  
***************************************************************/

static int
ErrorMessage (LineType * current)
{
  char tbuf[64];
   
  assert (current != NULL);

  switch (current->cmdnum)
  {
  case C_FOR:
    bwb_strcpy (tbuf, "FOR without NEXT");
    break;
  case C_EXIT_FOR:
    bwb_strcpy (tbuf, "EXIT FOR without FOR");
    break;
  case C_NEXT:
    bwb_strcpy (tbuf, "NEXT without FOR");
    break;
  case C_DO:
    bwb_strcpy (tbuf, "DO without LOOP");
    break;
  case C_EXIT_DO:
    bwb_strcpy (tbuf, "EXIT DO without DO");
    break;
  case C_LOOP:
    bwb_strcpy (tbuf, "LOOP without DO");
    break;
  case C_REPEAT:
    bwb_strcpy (tbuf, "REPEAT without UNTIL");
    break;
  case C_EXIT_REPEAT:
    bwb_strcpy (tbuf, "EXIT REPEAT without REPEAT");
    break;
  case C_UNTIL:
    bwb_strcpy (tbuf, "UNTIL without REPEAT");
    break;
  case C_WHILE:
    bwb_strcpy (tbuf, "WHILE without WEND");
    break;
  case C_EXIT_WHILE:
    bwb_strcpy (tbuf, "EXIT WHILE without WHILE");
    break;
  case C_WEND:
    bwb_strcpy (tbuf, "WEND without WHILE");
    break;
  case C_SUB:
    bwb_strcpy (tbuf, "SUB without END SUB");
    break;
  case C_EXIT_SUB:
    bwb_strcpy (tbuf, "EXIT SUB without SUB");
    break;
  case C_END_SUB:
    bwb_strcpy (tbuf, "END SUB without SUB");
    break;
  case C_FUNCTION:
    bwb_strcpy (tbuf, "FUNCTION without END FUNCTION");
    break;
  case C_EXIT_FUNCTION:
    bwb_strcpy (tbuf, "EXIT FUNCTION without FUNCTION");
    break;
  case C_END_FUNCTION:
    bwb_strcpy (tbuf, "END FUNCTION without FUNCTION");
    break;
  case C_IF8THEN:
    bwb_strcpy (tbuf, "IF THEN without END IF");
    break;
  case C_ELSEIF:
    bwb_strcpy (tbuf, "ELSEIF without IF THEN");
    break;
  case C_ELSE:
    bwb_strcpy (tbuf, "ELSE without IF THEN");
    break;
  case C_END_IF:
    bwb_strcpy (tbuf, "END IF without IF THEN");
    break;
  case C_SELECT_CASE:
    bwb_strcpy (tbuf, "SELECT CASE without END SELECT");
    break;
  case C_CASE:
    bwb_strcpy (tbuf, "CASE without SELECT CASE");
    break;
  case C_CASE_ELSE:
    bwb_strcpy (tbuf, "CASE ELSE without SELECT CASE");
    break;
  case C_END_SELECT:
    bwb_strcpy (tbuf, "END SELECT without SELECT CASE");
    break;
  default:
    bwb_strcpy (tbuf, "UNKNOWN COMMAND");
    break;
  }
  fprintf (My->SYSOUT->cfp, "%s: %d %s\n", tbuf, current->number,
           current->buffer);
  ResetConsoleColumn ();
  return FALSE;
}

static LineType *
find_BottomLineInCode (LineType * l)
{
   

  if (l == NULL)
  {
    return NULL;
  }
  while (l->OtherLine != NULL)
  {
    switch (l->cmdnum)
    {
    case C_NEXT:
    case C_LOOP:
    case C_UNTIL:
    case C_WEND:
    case C_END_SUB:
    case C_END_FUNCTION:
    case C_END_IF:
    case C_END_SELECT:
      return l;
    }
    l = l->OtherLine;
  }
  /* l->OtherLine == NULL */
  return l;
}

static int
MissingBottomLine (LineType * current, int cmdnum)
{
  LineType *BottomLineInCode;
   

  BottomLineInCode = find_BottomLineInCode (current);
  if (BottomLineInCode == NULL)
  {
    return TRUE;
  }
  if (BottomLineInCode->cmdnum != cmdnum)
  {
    return TRUE;
  }
  return FALSE;
}

static LineType *
FindParentCommand (int cmdnum, unsigned int Indention,
                   LineType * Previous[ /* EXECLEVELS */ ])
{
   
  assert (Previous != NULL);

  if (Indention > 0)
  {
    unsigned int i;
    for (i = Indention - 1; /* i >= 0 */ ; i--)
    {
      if (Previous[i]->cmdnum == cmdnum)
      {
        /* FOUND */
        return Previous[i];        /* EXIT_FOR->OtherLine == FOR */
      }
      if (i == 0)
      {
        /* NOT FOUND */
      }
    }
  }
  /* NOT FOUND */
  return NULL;
}

extern int
bwb_scan (void)
{
  /*
     STATIC ANALYSIS


     Background.
     This routine began as a way to record the line numbers associated with the cmdnum of FUNCTION, SUB, or LABEL.

     Pretty-printing.
     Indention was added for pretty-printing by LIST, based upon the cmdnum (Indetion++, Indention--).

     Mismatched structured commands.
     When reviewing a properly indented listing, mismatched structured commands are easier to visually indentify 
     (FOR without NEXT and so on), so Previous[] was added to record the previous cmdnum at a given Indention.
     Comparing Current->cmdnum with Previous->cmdnum allows mismatched structured commands to be detected.

     Reduce stack usage for structured loops.
     OtherLine, which was previously determined at runtime for loops, could now be determined during the scan.
     Previously all loops used the stack so the EXIT command could find the loop's bottom line.
     The EXIT commands could now look in Previous[] to determine their loop's top line and follow that to the loop's bottom line.
     As a result, now the FOR loops use the stack to hold the current iteration value, but all other loops do not.

     Reduce stack usaage for structured IF/SELECT.
     Previuosly the structured IF/SELECT command used the stack to hold the results of comparisons and intermediate values.
     OtherLine is now used to link these commands to their next occurring command.
     As a result, the path thru the structure is now chosen at the IF/SELECT command, and the stack is no longer used.
     The RESUME command knows about this linkage, so a simple "RESUME" jumps to the "IF THEN" or "SELECT CASE"
     and "RESUME NEXT" jumps to the "END IF" or "END SELECT".

     Caching for unstructured commands.
     OtherLine was not previously used for any purpose for the unstructured GOTO, GOSUB, IF and ON commands.
     It is now used to cache the line last executed by these commands to reduce the time required to find the target line.
     The cache reduces execution time because the target line is usually (but not always) the same.
     For the simple commands "GOTO 100", "GOSUB 100" and "IF x THEN 100", the cache will always succeed.
     For the command "IF x THEN 100 ELSE 200", the cache will succeed for the last taken path.
     Because programs are typically written so one path is more likely, the cache usually succeeds.
     For the "ON x GOTO ..." and "ON x GOSUB ...", the cache succeeds when the result of the test expression repeats, such as:
     FOR I = 1 TO 100
     ON INT(I/10) GOSUB ...
     NEXT I
     In this example, the cache will succeed 90 times and fail 10 times.

     Checking FOR/NEXT variable names.
     If a variable name is provided for a NEXT command, then it is compared against the variable name of the matching FOR command.
     This detects the following kind of mismatch:
     FOR I = ...
     NEXT J

     OtherLine is now used for different purposes depending upon the command.

     For structured IF8THEN and SELECT_CASE, OtherLine is used to form a one-way list:
     IF8THEN->OtherLine == next occuring ELSE_IF, ELSE, END_IF
     ELSE_IF->Otherline == next occuring ELSE_IF, ELSE, END_IF
     ELSE->OtherLine    == END_IF
     END_IF->OtherLine  == NULL


     For the structured loops, OtherLine is uses as a circular list:
     WHILE->OtherLine      == WEND
     EXIT_WHILE->OtherLine == WHILE
     WEND->OtherLine       == WHILE

     For unstructured flow-of-control commands, OtherLine is used as a one-entry cache.  
     It contains a pointer to the Most Recently Used line returned by the command:

     GOTO->OtherLine       == MRU target line
     GOSUB->OtherLine      == MRU target line
     IF->OtherLine         == MRU target line
     ON->OtherLine         == MRU target line

     For DATA commands, OtherLine points to the next DATA line (if it exists), otherwise it points to EndMarker.
     StartMarker->OtherLine points to the first DATA line (if it exists), otherwise it points to EndMarker.
     RESTORE knows about this.

     For other commands, OtherLine is not used.

     Any command which _requires_ OtherLine is not allowed be executed from the console in immediate mode.

   */
  LineType *current;
  LineType *prev_DATA;                /* previous DATA statement */
  unsigned int Indention;
  LineType *Previous[EXECLEVELS];        /* previous part of structured command */


  assert( My != NULL );
  assert( My->StartMarker != NULL );
  assert( My->EndMarker != NULL );


  prev_DATA = NULL;
  if (My->IsScanRequired == FALSE)
  {
    /* program is clean, no scan required */
    return TRUE;
  }
  /* program needs to be scanned again, because a line was added or deleted */

  /* reset these whenever a SCAN occurs */
  My->StartMarker->OtherLine = My->EndMarker;        /* default when no DATA statements exist */
  My->DataLine = My->EndMarker;        /* default when no DATA statements exist */
  My->DataPosition = My->DataLine->Startpos;
  My->ERL = NULL;
  My->ContinueLine = NULL;


  /* first run through the FUNCTION - SUB loopkup table and free any existing memory */

  UserFunction_clear ();



  for (Indention = 0; Indention < EXECLEVELS; Indention++)
  {
    Previous[Indention] = NULL;
  }
  Indention = 0;

  for (current = My->StartMarker->next; current != My->EndMarker;
       current = current->next)
  {
    assert( current != NULL );
    current->OtherLine = NULL;

    if (current->cmdnum == C_DATA)
    {
      if (prev_DATA == NULL)
      {
        /* I am the first DATA statement */
        My->StartMarker->OtherLine = current;
        My->DataLine = current;
        My->DataPosition = My->DataLine->Startpos;
      }
      else
      {
        /* link the previous DATA statement to me */
        prev_DATA->OtherLine = current;
      }
      /* I am the last DATA statement so far */
      current->OtherLine = My->EndMarker;
      /* I should point at the next DATA statement */
      prev_DATA = current;
    }

    switch (current->cmdnum)
    {
    case C_DEF:
    case C_FUNCTION:
    case C_SUB:
    case C_DEF8LBL:
      UserFunction_add (current);
    }

    /* verify the 'current' command is consistent with a 'previous' command at a lower indention */
    switch (current->cmdnum)
    {
    case C_EXIT_FOR:
      current->OtherLine = FindParentCommand (C_FOR, Indention, Previous);        /* EXIT_FOR->OtherLine == FOR */
      if (current->OtherLine == NULL)
      {
        return ErrorMessage (current);
      }
      break;
    case C_EXIT_WHILE:
      current->OtherLine = FindParentCommand (C_WHILE, Indention, Previous);        /* EXIT_WHILE->OtherLine == WHILE */
      if (current->OtherLine == NULL)
      {
        return ErrorMessage (current);
      }
      break;
    case C_EXIT_REPEAT:
      current->OtherLine = FindParentCommand (C_REPEAT, Indention, Previous);        /* EXIT_REPEAT->OtherLine == REPEAT */
      if (current->OtherLine == NULL)
      {
        return ErrorMessage (current);
      }
      break;
    case C_EXIT_FUNCTION:
      current->OtherLine = FindParentCommand (C_FUNCTION, Indention, Previous);        /* EXIT_FUNCTION->OtherLine == FUNCTION */
      if (current->OtherLine == NULL)
      {
        return ErrorMessage (current);
      }
      break;
    case C_EXIT_SUB:
      current->OtherLine = FindParentCommand (C_SUB, Indention, Previous);        /* EXIT_SUB->OtherLine == SUB */
      if (current->OtherLine == NULL)
      {
        return ErrorMessage (current);
      }
      break;
    case C_EXIT_DO:
      current->OtherLine = FindParentCommand (C_DO, Indention, Previous);        /* EXIT_DO->OtherLine == DO */
      if (current->OtherLine == NULL)
      {
        return ErrorMessage (current);
      }
      break;
    }


    /* verify the 'current' command is consistent with a 'previous' command at the same indention */
    switch (current->cmdnum)
    {
    case C_NEXT:
      if (Indention == 0)
      {
        return ErrorMessage (current);
      }
      Indention--;
      switch (Previous[Indention]->cmdnum)
      {
      case C_FOR:
        /* if( TRUE ) */
        {
          /* compare the 'NEXT' variable with the 'FOR' variable */
          current->position = current->Startpos;
          Previous[Indention]->position = Previous[Indention]->Startpos;
          if (line_is_eol (current))
          {
            /* NEXT */
            /* there is no variable to compare */
          }
          else
          {
            /* NEXT variable */
            char NextVarName[NameLengthMax + 1];
            char ForVarName[NameLengthMax + 1];

            if (line_read_varname (current, NextVarName) == FALSE)
            {
              return ErrorMessage (current);
            }
            if (line_read_varname (Previous[Indention], ForVarName) == FALSE)
            {
              return ErrorMessage (current);
            }
            if (bwb_stricmp (ForVarName, NextVarName) != 0)
            {
              return ErrorMessage (current);
            }
          }
          /* MATCHED */
          current->Startpos = current->position;
          Previous[Indention]->position = Previous[Indention]->Startpos;
        }
        /* OK */
        Previous[Indention]->OtherLine = current;        /* FOR->OtherLine = NEXT */
        current->OtherLine = Previous[Indention];        /* NEXT->OtherLine = FOR */
        Previous[Indention] = current;        /* last command at this level = NEXT */
        break;
      default:
        return ErrorMessage (current);
      }
      break;
    case C_LOOP:
      if (Indention == 0)
      {
        return ErrorMessage (current);
      }
      Indention--;
      switch (Previous[Indention]->cmdnum)
      {
      case C_DO:
        /* OK */
        Previous[Indention]->OtherLine = current;        /* DO->OtherLine = LOOP */
        current->OtherLine = Previous[Indention];        /* LOOP->OtherLine = DO */
        Previous[Indention] = current;        /* last command at this level = LOOP */
        break;
      default:
        return ErrorMessage (current);
      }
      break;
    case C_UNTIL:
      if (Indention == 0)
      {
        return ErrorMessage (current);
      }
      Indention--;
      switch (Previous[Indention]->cmdnum)
      {
      case C_REPEAT:
        /* OK */
        Previous[Indention]->OtherLine = current;        /* REPEAT->OtherLine = UNTIL */
        current->OtherLine = Previous[Indention];        /* UNTIL->OtherLine = REPEAT */
        Previous[Indention] = current;        /* last command at this level = UNTIL */
        break;
      default:
        return ErrorMessage (current);
      }
      break;
    case C_WEND:
      if (Indention == 0)
      {
        fprintf (My->SYSOUT->cfp, "Unmatched command %d %s\n",
                 current->number, current->buffer);
        ResetConsoleColumn ();
        return FALSE;
      }
      Indention--;
      switch (Previous[Indention]->cmdnum)
      {
      case C_WHILE:
        /* OK */
        Previous[Indention]->OtherLine = current;        /* WHILE->OtherLine = WEND */
        current->OtherLine = Previous[Indention];        /* WEND->OtherLine = WHILE */
        Previous[Indention] = current;        /* last command at this level = WEND */
        break;
      default:
        return ErrorMessage (current);
      }
      break;
    case C_END_SUB:
      if (Indention == 0)
      {
        return ErrorMessage (current);
      }
      Indention--;
      switch (Previous[Indention]->cmdnum)
      {
      case C_SUB:
        /* OK */
        Previous[Indention]->OtherLine = current;        /* SUB->OtherLine = END_SUB */
        current->OtherLine = Previous[Indention];        /* END_SUB->OtherLine = SUB */
        Previous[Indention] = current;        /* last command at this level = END_SUB */
        break;
      default:
        return ErrorMessage (current);
      }
      break;
    case C_END_FUNCTION:
      if (Indention == 0)
      {
        return ErrorMessage (current);
      }
      Indention--;
      switch (Previous[Indention]->cmdnum)
      {
      case C_FUNCTION:
        /* OK */
        Previous[Indention]->OtherLine = current;        /* FUNCTION->OtherLine = END_FUNCTION */
        current->OtherLine = Previous[Indention];        /* END_FUNCTION->OtherLine = FUNCTION */
        Previous[Indention] = current;        /* last command at this level = END_FUNCTION */
        break;
      default:
        return ErrorMessage (current);
      }
      break;
    case C_ELSEIF:
      if (Indention == 0)
      {
        return ErrorMessage (current);
      }
      Indention--;
      switch (Previous[Indention]->cmdnum)
      {
      case C_IF8THEN:
        /* OK */
        Previous[Indention]->OtherLine = current;        /* IF8THEN->OtherLine = ELSEIF */
        Previous[Indention] = current;        /* last command at this level = ELSEIF */
        break;
      case C_ELSEIF:
        /* OK */
        Previous[Indention]->OtherLine = current;        /* ELSEIF->OtherLine = ELSEIF */
        Previous[Indention] = current;        /* last command at this level = ELSEIF */
        break;
      default:
        return ErrorMessage (current);
      }
      break;
    case C_ELSE:
      if (Indention == 0)
      {
        return ErrorMessage (current);
      }
      Indention--;
      switch (Previous[Indention]->cmdnum)
      {
      case C_IF8THEN:
        /* OK */
        Previous[Indention]->OtherLine = current;        /* IF8THEN->OtherLine = ELSE */
        Previous[Indention] = current;        /* last command at this level = ELSE */
        break;
      case C_ELSEIF:
        /* OK */
        Previous[Indention]->OtherLine = current;        /* ELSEIF->OtherLine = ELSE */
        Previous[Indention] = current;        /* last command at this level = ELSE */
        break;
      default:
        return ErrorMessage (current);
      }
      break;
    case C_END_IF:
      if (Indention == 0)
      {
        return ErrorMessage (current);
      }
      Indention--;
      switch (Previous[Indention]->cmdnum)
      {
      case C_IF8THEN:
        /* OK */
        Previous[Indention]->OtherLine = current;        /* IF8THEN->OtherLine = END_IF */
        Previous[Indention] = current;        /* last command at this level = END_IF */
        break;
      case C_ELSEIF:
        /* OK */
        Previous[Indention]->OtherLine = current;        /* ELSEIF->OtherLine = END_IF */
        Previous[Indention] = current;        /* last command at this level = END_IF */
        break;
      case C_ELSE:
        /* OK */
        Previous[Indention]->OtherLine = current;        /* ELSE->OtherLine = END_IF */
        Previous[Indention] = current;        /* last command at this level = END_IF */
        break;
      default:
        return ErrorMessage (current);
      }
      break;
    case C_CASE:
      if (Indention == 0)
      {
        return ErrorMessage (current);
      }
      Indention--;
      switch (Previous[Indention]->cmdnum)
      {
      case C_SELECT_CASE:
        /* OK */
        Previous[Indention]->OtherLine = current;        /* C_SELECT_CASE->OtherLine = C_CASE */
        Previous[Indention] = current;        /* last command at this level = C_CASE */
        break;
      case C_CASE:
        /* OK */
        Previous[Indention]->OtherLine = current;        /* CASE->OtherLine = C_CASE */
        Previous[Indention] = current;        /* last command at this level = C_CASE */
        break;
      default:
        return ErrorMessage (current);
      }
      break;
    case C_CASE_ELSE:
      if (Indention == 0)
      {
        return ErrorMessage (current);
      }
      Indention--;
      switch (Previous[Indention]->cmdnum)
      {
      case C_CASE:
        /* OK */
        Previous[Indention]->OtherLine = current;        /* CASE->OtherLine = C_CASE_ELSE */
        Previous[Indention] = current;        /* last command at this level = C_CASE_ELSE */
        break;
      default:
        return ErrorMessage (current);
      }
      break;
    case C_END_SELECT:
      if (Indention == 0)
      {
        return ErrorMessage (current);
      }
      Indention--;
      switch (Previous[Indention]->cmdnum)
      {
      case C_CASE:
        /* OK */
        Previous[Indention]->OtherLine = current;        /* CASE->OtherLine = END_SELECT */
        Previous[Indention] = current;        /* last command at this level = END_SELECT */
        break;
      case C_CASE_ELSE:
        /* OK */
        Previous[Indention]->OtherLine = current;        /* CASE_ELSE->OtherLine = END_SELECT */
        Previous[Indention] = current;        /* last command at this level = END_SELECT */
        break;
      default:
        return ErrorMessage (current);
      }
      break;
    }
    /* OK */

    current->Indention = Indention;

    /* record the 'current' command as the 'previous' command at this indention */
    switch (current->cmdnum)
    {
    case C_FUNCTION:
    case C_SUB:
      /* this 'command' can NOT be inside the structure of another 'command' */
      if (Indention > 0)
      {
        return ErrorMessage (Previous[Indention - 1]);
      }
    case C_FOR:
    case C_DO:
    case C_REPEAT:
    case C_WHILE:
    case C_IF8THEN:
    case C_SELECT_CASE:
      if (Previous[Indention] != NULL)
      {
        /* we are NOT the first command at this indention level */
        /* verify the 'previous' command at this level was properly closed */
        switch (Previous[Indention]->cmdnum)
        {
        case C_FOR:
        case C_DO:
        case C_REPEAT:
        case C_WHILE:
        case C_FUNCTION:
        case C_SUB:
        case C_IF8THEN:
        case C_ELSEIF:
        case C_ELSE:
        case C_SELECT_CASE:
        case C_CASE:
        case C_CASE_ELSE:
          /* there is an existing unclosed structure */
          return ErrorMessage (Previous[Indention]);
        }
      }
      Previous[Indention] = current;
      Indention++;
      if (Indention == EXECLEVELS)
      {
        fprintf (My->SYSOUT->cfp, "Program is nested too deep\n");
        ResetConsoleColumn ();
        return FALSE;
      }
      Previous[Indention] = NULL;
      break;
    case C_ELSEIF:
    case C_ELSE:
    case C_CASE:
    case C_CASE_ELSE:
      /* 
         Previous[ Indention ] was already checked and assigned above, just indent.
       */
      Indention++;
      if (Indention == EXECLEVELS)
      {
        fprintf (My->SYSOUT->cfp, "Program is nested too deep\n");
        ResetConsoleColumn ();
        return FALSE;
      }
      Previous[Indention] = NULL;
      break;
    }
  }

  if (Indention > 0)
  {
    return ErrorMessage (Previous[Indention - 1]);
  }

  /* verify the OtherLine chain terminates correctly; we should find the bottom command */
  for (current = My->StartMarker->next; current != My->EndMarker;
       current = current->next)
  {
    assert( current != NULL );
    switch (current->cmdnum)
    {
    case C_FOR:
    case C_EXIT_FOR:
      if (MissingBottomLine (current, C_NEXT))
      {
        return ErrorMessage (current);
      }
      break;
    case C_DO:
    case C_EXIT_DO:
      if (MissingBottomLine (current, C_LOOP))
      {
        return ErrorMessage (current);
      }
      break;
    case C_REPEAT:
    case C_EXIT_REPEAT:
      if (MissingBottomLine (current, C_UNTIL))
      {
        return ErrorMessage (current);
      }
      break;
    case C_WHILE:
    case C_EXIT_WHILE:
      if (MissingBottomLine (current, C_WEND))
      {
        return ErrorMessage (current);
      }
      break;
    case C_FUNCTION:
    case C_EXIT_FUNCTION:
      if (MissingBottomLine (current, C_END_FUNCTION))
      {
        return ErrorMessage (current);
      }
      break;
    case C_SUB:
    case C_EXIT_SUB:
      if (MissingBottomLine (current, C_END_SUB))
      {
        return ErrorMessage (current);
      }
      break;
    case C_IF8THEN:
      if (MissingBottomLine (current, C_END_IF))
      {
        return ErrorMessage (current);
      }
      break;
    case C_SELECT_CASE:
      if (MissingBottomLine (current, C_END_SELECT))
      {
        return ErrorMessage (current);
      }
      break;
    }
  }

  /* return */

  My->IsScanRequired = FALSE;
  return TRUE;

}

/***************************************************************
  
        FUNCTION:       UserFunction_clear()
  
        DESCRIPTION: This C function clears all existing memory
            in the FUNCTION-SUB lookup table.
  
***************************************************************/

static int
UserFunction_clear (void)
{
  UserFunctionType *current;

  assert( My != NULL );
   

  /* run through table and clear memory */
  for (current = My->UserFunctionHead; current != NULL;)
  {
    UserFunctionType *next;

    assert( current != NULL );
    next = current->next;

    /* check for local variables and free them */
    if (current->local_variable != NULL)
    {
      var_free (current->local_variable);
      current->local_variable = NULL;
    }

    if (current->name != NULL)
    {
      free (current->name);
      current->name = NULL;
    }
    free (current);
    current = next;
  }
  My->UserFunctionHead = NULL;
  return TRUE;
}

extern int
UserFunction_name (char *name)
{
  /* search USER functions */
  UserFunctionType *L;
   
  assert (name != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );

  for (L = My->UserFunctionHead; L != NULL; L = L->next)
  {
    if (My->CurrentVersion->OptionVersionValue & L->OptionVersionBitmask)
    {
      if (bwb_stricmp (L->name, name) == 0)
      {
        return TRUE;
      }
    }
  }
  return FALSE;
}

extern UserFunctionType *
UserFunction_find_exact (char *name, unsigned char ParameterCount,
                         ParamBitsType ParameterTypes)
{
  /* search USER functions */
  UserFunctionType *L;
   
  assert (name != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );

  for (L = My->UserFunctionHead; L != NULL; L = L->next)
  {
    if (My->CurrentVersion->OptionVersionValue & L->OptionVersionBitmask)
    {
      if (L->ParameterCount == ParameterCount
          && L->ParameterTypes == ParameterTypes)
      {
        if (bwb_stricmp (L->name, name) == 0)
        {
          /* FOUND */
          return L;
        }
      }
    }
  }
  /* NOT FOUND */
  return NULL;
}

/***************************************************************
  
        FUNCTION:       UserFunction_add()
  
        DESCRIPTION: This C function adds an entry to the
            FUNCTION-SUB lookup table.
  
***************************************************************/

extern int
UserFunction_add (LineType * l /* , int *position , int code */ )
{
  char *name;
  UserFunctionType *f;
  char TypeCode;
  char varname[NameLengthMax + 1];
   
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
  assert( My->DefaultVariableType != NULL );

  
  /* get the element for name */
  switch (l->cmdnum)
  {
  case C_DEF:
  case C_FUNCTION:
  case C_SUB:
    l->position = l->Startpos;
    if (line_read_varname (l, varname) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return FALSE;
    }
    break;
  case C_DEF8LBL:
    l->position = 0;
    if (line_read_label (l, varname) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return FALSE;
    }
    l->position = l->Startpos;
    break;
  default:
    WARN_SYNTAX_ERROR;
    return FALSE;
  }

  /* get memory for name buffer */
  if ((name =
       (char *) calloc (bwb_strlen (varname) + 1 /* NulChar */ ,
                        sizeof (char))) == NULL)
  {
    WARN_OUT_OF_MEMORY;
    return FALSE;
  }
  bwb_strcpy (name, varname);

  /* get memory for fslt structure */
  if ((f =
       (UserFunctionType *) calloc (1, sizeof (UserFunctionType))) == NULL)
  {
    WARN_OUT_OF_MEMORY;
    return FALSE;
  }
  /* fill in structure */

  f->line = l;
  f->name = name;
  f->local_variable = NULL;
  f->ParameterCount = 0;        /* 0..32, 255 == (...) */
  f->ParameterTypes = 0;        /* bit 0 is first parameter */
  f->startpos = l->position;
  f->OptionVersionBitmask = My->CurrentVersion->OptionVersionValue;




  /* read arguments */
  switch (l->cmdnum)
  {
  case C_DEF:
  case C_FUNCTION:
  case C_SUB:
    TypeCode = var_nametype (varname);
    if (line_peek_LparenChar (l))
    {
      if (scan_readargs (f, l))
      {
        f->startpos = l->position;
      }
    }
    /* determine function type */
    if (TypeCode == NulChar)
    {
      /* function has no explicit type char */
      TypeCode = line_read_type_declaration (l);
      if (TypeCode == NulChar)
      {
        /* function has no declared type */
        int i;
        i = VarTypeIndex (varname[0]);
        if (i < 0)
        {
          TypeCode = DoubleTypeCode;        /* default */
        }
        else
        {
          TypeCode = My->DefaultVariableType[i];
        }
      }
    }
    break;
  case C_DEF8LBL:
    TypeCode = LongTypeCode;
    break;
  default:
    WARN_SYNTAX_ERROR;
    return FALSE;
  }
  f->ReturnTypeCode = TypeCode;
  /* establish linkages */
  f->next = My->UserFunctionHead;
  My->UserFunctionHead = f;

  return TRUE;
}

/***************************************************************
  
        FUNCTION:       scan_readargs()
  
        DESCRIPTION: This C function reads arguments (variable
            names for an entry added to the FUNCTION-
         SUB lookup table.
  
***************************************************************/

static int
scan_readargs (UserFunctionType * f, LineType * l)
{
   
  assert (f != NULL);
  assert (l != NULL);
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
  assert( My->DefaultVariableType != NULL );

  f->ParameterCount = 0;        /* 0..32, 255 == (...) */
  f->ParameterTypes = 0;        /* bit 0 is first parameter */

  /* we should be at begin paren */
  if (line_skip_LparenChar (l) == FALSE)
  {
    WARN_SYNTAX_ERROR;
    return FALSE;
  }
  if (line_skip_RparenChar (l))
  {
    /* end of NO argument list */
    /* FUNCTION ABC() */
    return TRUE;
  }
  if (line_skip_word (l, "..."))
  {
    /* FUNCTION FRED( ... ) */
    if (line_skip_RparenChar (l))
    {
      f->ParameterCount = 0xFF;        /* VARIANT */
      f->ParameterTypes = 0;
      return TRUE;
    }
    WARN_SYNTAX_ERROR;
    return FALSE;
  }

  /* loop through looking for arguments */
  do
  {
    VariableType *v;
    char TypeCode;
    char varname[NameLengthMax + 1];

    /* presume beginning of argument == variable name */
    if (line_read_varname (l, varname) == FALSE)
    {
      WARN_SYNTAX_ERROR;
      return FALSE;
    }
    /* determine variable type */
    TypeCode = var_nametype (varname);
    if (TypeCode == NulChar)
    {
      /* variable has no explicit type char */
      TypeCode = line_read_type_declaration (l);
      if (TypeCode == NulChar)
      {
        /* variable has no declared type */
        int i;
        i = VarTypeIndex (varname[0]);
        if (i < 0)
        {
          TypeCode = DoubleTypeCode;        /* default */
        }
        else
        {
          TypeCode = My->DefaultVariableType[i];
        }
      }
    }

    /* initialize the variable and add it to local chain */
    v = var_new (varname, TypeCode);
    UserFunction_addlocalvar (f, v);
    if (VAR_IS_STRING (v))
    {
      f->ParameterTypes |= (1 << f->ParameterCount);
    }
    f->ParameterCount++;        /* 0..32, 255 == (...) */
    if (f->ParameterCount > MAX_FARGS)
    {
      /* should have been declared VARIANT */
      WARN_SYNTAX_ERROR;
      return FALSE;
    }
  }
  while (line_skip_seperator (l));

  if (line_skip_RparenChar (l))
  {
    /* end of argument list */
    return TRUE;
  }

  /* FUNCTION ABC( A$, B$, */
  WARN_SYNTAX_ERROR;
  return FALSE;
}



/***************************************************************
  
   FUNCTION:       UserFunction_addlocalvar()
  
   DESCRIPTION:    This function adds a local variable
         to the FUNCTION-SUB lookup table at
         a specific level.
  
***************************************************************/

extern int
UserFunction_addlocalvar (UserFunctionType * f, VariableType * v)
{
   
  assert (f != NULL);
  assert (v != NULL);

  /* find end of local chain */
  if (f->local_variable == NULL)
  {
    f->local_variable = v;
  }
  else
  {
    VariableType *p;
    VariableType *c;

    p = f->local_variable;
    for (c = f->local_variable->next; c != NULL; c = c->next)
    {
      p = c;
    }
    p->next = v;
  }
  v->next = NULL;
  return TRUE;
}

LineType *
bwb_DEF8LBL (LineType * l)
{
  /*
   **
   ** this command is used for a line that is a user label
   **
   */
   
  assert (l != NULL);

  if (l->LineFlags & (LINE_USER))
  {
    WARN_ILLEGAL_DIRECT;
    return (l);
  }

  line_skip_eol (l);
  return (l);
}

/***************************************************************
  
        FUNCTION:       bwb_def()
  
        DESCRIPTION:    This C function implements the BASIC
                        DEF statement. 
  
   SYNTAX:     DEF FNname(arg...)] = expression
  
   NOTE:    It is not a strict requirement that the
         function name should begin with "FN".
  
***************************************************************/

LineType *
bwb_DEF (LineType * l)
{
   
  assert (l != NULL);

  if (l->LineFlags & (LINE_USER))
  {
    WARN_ILLEGAL_DIRECT;
    return (l);
  }

  /* this line will be executed by IntrinsicFunction_deffn() in bwb_fnc.c */
  line_skip_eol (l);

  return (l);
}



/***************************************************************
  
        FUNCTION:       bwb_call()
  
        DESCRIPTION: This C function implements the BASIC
            CALL subroutine command.
  
   SYNTAX:     CALL subroutine-name( param1, param2 )
  
***************************************************************/

LineType *
bwb_CALL (LineType * l)
{
  VariantType x;
  VariantType *X;
   
  assert (l != NULL);

  X = &x;
  CLEAR_VARIANT (X);
  /* Call the expression interpreter to evaluate the function */
  if (line_read_expression (l, X) == FALSE)        /* bwb_CALL */
  {
    WARN_SYNTAX_ERROR;
    goto EXIT;
  }
EXIT:
  RELEASE_VARIANT (X);
  return (l);
}


/***************************************************************
  
   FUNCTION:   find_label()
  
   DESCRIPTION:   This C function finds a program line that
         begins with the label included in <buffer>.
  
***************************************************************/
LineType *
find_line_number (int number)
{
  /* 
   ** 
   ** LABELS are resolved to their line number by the expresson parser.
   ** However, LABELS usually do not have the LINE_NUMBERED flag set.
   **
   */
  assert( My != NULL );
  assert( My->StartMarker != NULL );
  assert( My->EndMarker != NULL );
   

  if (MINLIN <= number && number <= MAXLIN)
  {
    /* 
     **
     ** brute force search 
     **
     */
    LineType *x;

    for (x = My->StartMarker->next; x != NULL && x != My->EndMarker && x->number < number;
         x = x->next);
    assert( x != NULL );
    if (x->number == number)
    {
      /* FOUND */
      return x;
    }
  }
  /* NOT FOUND */
  WARN_UNDEFINED_LINE;
  return NULL;
}

extern VariableType *
var_chain (VariableType * argv)
{
  /* create a variable chain */
  VariableType *argn;
   

  if (argv == NULL)
  {
    /* we are the first variable in the chain */
    if ((argn = (VariableType *) calloc (1, sizeof (VariableType))) == NULL)
    {
      WARN_OUT_OF_MEMORY;
      return NULL;
    }
  }
  else
  {
    /* find the end of the chain */
    assert( argv != NULL );
    for (argn = argv; argn->next != NULL; argn = argn->next);

    /* add ourself to the end */
    assert( argn != NULL );
    if ((argn->next =
         (VariableType *) calloc (1, sizeof (VariableType))) == NULL)
    {
      WARN_OUT_OF_MEMORY;
      return NULL;
    }
    argn = argn->next;
  }
  assert( argn != NULL );
  argn->next = NULL;

  /* return pointer to the variable just created */
  return argn;
}


/* EOF */
