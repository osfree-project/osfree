/***************************************************************
  
        bwx_tty.c       TTY front-end
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


extern void
bwx_LOCATE (int Row, int Col)
{
  /* position the cursor to Row and Col */
  /* Row is 1 based, 1..24 */
  /* Col is 1 based, 1..80 */
  assert (My != NULL);
  assert (My->SYSOUT != NULL);
  assert (My->SYSOUT->cfp != NULL);


  if (Row < 1 || Col < 1)
  {
    WARN_ILLEGAL_FUNCTION_CALL;
    return;
  }
  switch (My->OptionTerminalType)
  {
  case C_OPTION_TERMINAL_NONE:
    break;
  case C_OPTION_TERMINAL_ADM:
    fprintf (My->SYSOUT->cfp, "%c=%c%c", 27, Row + 32, Col + 32);
    break;
  case C_OPTION_TERMINAL_ANSI:
    fprintf (My->SYSOUT->cfp, "%c[%d;%dH", 27, Row, Col);
    break;
  default:
    WARN_ADVANCED_FEATURE;
    break;
  }
  fflush (My->SYSOUT->cfp);
  My->SYSOUT->row = Row;
  My->SYSOUT->col = Col;
}

extern void
bwx_CLS (void)
{
  /* clear screen */
  assert (My != NULL);
  assert (My->SYSOUT != NULL);
  assert (My->SYSOUT->cfp != NULL);
  switch (My->OptionTerminalType)
  {
  case C_OPTION_TERMINAL_NONE:
    break;
  case C_OPTION_TERMINAL_ADM:
    fprintf (My->SYSOUT->cfp, "%c", 26);
    break;
  case C_OPTION_TERMINAL_ANSI:
    fprintf (My->SYSOUT->cfp, "%c[2J", 27);
    break;
  default:
    WARN_ADVANCED_FEATURE;
    break;
  }
  bwx_LOCATE (1, 1);
}

extern void
bwx_COLOR (int Fore, int Back)
{
  /* set foreground and background color */
  /* Fore is 0 based, 0..15 */
  /* Back is 0 based, 0..15 */
  assert (My != NULL);
  assert (My->SYSOUT != NULL);
  assert (My->SYSOUT->cfp != NULL);
  if (Fore < 0 || Back < 0)
  {
    WARN_ILLEGAL_FUNCTION_CALL;
    return;
  }
  switch (My->OptionTerminalType)
  {
  case C_OPTION_TERMINAL_NONE:
    break;
  case C_OPTION_TERMINAL_ADM:
    break;
  case C_OPTION_TERMINAL_ANSI:
    fprintf (My->SYSOUT->cfp, "%c[%d;%dm", 27, 30 + Fore, 40 + Back);
    break;
  default:
    WARN_ADVANCED_FEATURE;
    break;
  }
  fflush (My->SYSOUT->cfp);
}



/* EOF */
