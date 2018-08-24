/***************************************************************
  
        bwbasic.c       Main Program File
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

static void break_handler (void);
static void break_mes (int x);
static int bwb_init (void);
static void bwb_initialize_warnings (void);
static void bwb_interact (void);
static int bwb_ladd (char *buffer, LineType * p, int IsUser);
static void bwb_single_step (char *buffer);
static void bwb_xtxtline (char *buffer);
static int bwx_signon (void);
static void execute_profile (char *FileName);
static void execute_program (char *FileName);
static char *FindClassicStatementEnd (char *C);
static int FixQuotes (char *tbuf);
static void ImportClassicIfThenElse (char *InBuffer);
static int is_ln (char *buffer);
static int is_numconst (char *buffer);
static void mark_preset_variables (void);
static FILE *nice_open (char *BaseName);
static void process_basic_line (char *buffer);

GlobalType *My = NULL;

static char *Banner[] = {
  "########  ##    ## ##      ##    ###    ######## ######## ########            ",
  "##     ##  ##  ##  ##  ##  ##   ## ##      ##    ##       ##     ##           ",
  "##     ##   ####   ##  ##  ##  ##   ##     ##    ##       ##     ##           ",
  "########     ##    ##  ##  ## ##     ##    ##    ######   ########            ",
  "##     ##    ##    ##  ##  ## #########    ##    ##       ##   ##             ",
  "##     ##    ##    ##  ##  ## ##     ##    ##    ##       ##    ##            ",
  "########     ##     ###  ###  ##     ##    ##    ######## ##     ##           ",
  "                                                                              ",
  "                                                                              ",
  "                                    ########     ###     ######  ####  ###### ",
  "                                    ##     ##   ## ##   ##    ##  ##  ##    ##",
  "                                    ##     ##  ##   ##  ##        ##  ##      ",
  "                                    ########  ##     ##  ######   ##  ##      ",
  "                                    ##     ## #########       ##  ##  ##      ",
  "                                    ##     ## ##     ## ##    ##  ##  ##    ##",
  "                                    ########  ##     ##  ######  ####  ###### ",
  "                                                                              ",
  "Bywater BASIC Interpreter, version 3.20                                       ",
  "Copyright (c) 1993, Ted A. Campbell                                           ",
  "Copyright (c) 1995-1997, Jon B. Volkoff                                       ",
  "Copyright (c) 2014-2017, Howard Wulf, AF5NE                                   ",
  "                                                                              ",
  NULL
};

#define NUM_WARNINGS 80

static char *ERROR4[NUM_WARNINGS];

static void
bwb_initialize_warnings (void)
{
  int i;
  for (i = 0; i < NUM_WARNINGS; i++)
  {
    ERROR4[i] = NULL;
  }
  ERROR4[1] = "NEXT without FOR";
  ERROR4[2] = "Syntax error";
  ERROR4[3] = "RETURN without GOSUB";
  ERROR4[4] = "Out of DATA";
  ERROR4[5] = "Illegal function call";
  ERROR4[6] = "Overflow";
  ERROR4[7] = "Out of memory";
  ERROR4[8] = "Undefined line";
  ERROR4[9] = "Subscript out of range";
  ERROR4[10] = "Redimensioned array";
  ERROR4[11] = "Division by zero";
  ERROR4[12] = "Illegal direct";
  ERROR4[13] = "Type mismatch";
  ERROR4[14] = "Out of string space";
  ERROR4[15] = "String too long";
  ERROR4[16] = "String formula too complex";
  ERROR4[17] = "Can't continue";
  ERROR4[18] = "Undefined user function";
  ERROR4[19] = "No RESUME";
  ERROR4[20] = "RESUME without error";
  ERROR4[21] = "Unprintable error";
  ERROR4[22] = "Missing operand";
  ERROR4[23] = "Line buffer overflow";
  ERROR4[26] = "FOR without NEXT";
  ERROR4[27] = "Bad DATA";
  ERROR4[29] = "WHILE without WEND";
  ERROR4[30] = "WEND without WHILE";
  ERROR4[31] = "EXIT FUNCTION without FUNCTION";
  ERROR4[32] = "END FUNCTION without FUNCTION";
  ERROR4[33] = "EXIT SUB without SUB";
  ERROR4[34] = "END SUB without SUB";
  ERROR4[35] = "EXIT FOR without FOR";
  ERROR4[50] = "Field overflow";
  ERROR4[51] = "Internal error";
  ERROR4[52] = "Bad file number";
  ERROR4[53] = "File not found";
  ERROR4[54] = "Bad file mode";
  ERROR4[55] = "File already open";
  ERROR4[57] = "Disk I/O error";
  ERROR4[58] = "File already exists";
  ERROR4[61] = "Disk full";
  ERROR4[62] = "Input past end";
  ERROR4[63] = "Bad record number";
  ERROR4[64] = "Bad file name";
  ERROR4[66] = "Direct statement in file";
  ERROR4[67] = "Too many files";
  ERROR4[70] = "Variable Not Declared";
  ERROR4[73] = "Advanced Feature";
}

/***************************************************************
  
      FUNCTION:       bwx_terminate()
  
   DESCRIPTION:    This function terminates program execution.
  
***************************************************************/

void
bwx_terminate (void)
{
   
  exit (0);
}



/***************************************************************
  
   FUNCTION:       break_handler()
  
   DESCRIPTION:    This function is called by break_mes()
         and handles program interruption by break
         (or by the STOP command).
  
***************************************************************/
static void
break_handler (void)
{
  /*
   **
   **
   */
  assert( My != NULL );

  My->AutomaticLineNumber = 0;
  My->AutomaticLineIncrement = 0;

  if (My->IsInteractive)
  {
    /* INTERACTIVE: terminate program */

    /* reset all stack counters */
    bwb_clrexec ();
    SetOnError (0);

    My->ERR = -1;                /* in break_handler() */


    /* reset the break handler */
    signal (SIGINT, break_mes);

    longjmp (My->mark, -1);


    return;
  }
  /* NOT INTERACTIVE:  terminate immediately */
  bwx_terminate ();
}

/***************************************************************
  
   FUNCTION:       break_mes()
  
   DESCRIPTION:    This function is called (a) by a SIGINT
         signal or (b) by bwb_STOP via bwx_STOP.
         It prints an error message then calls
         break_handler() to terminate the program.
  
***************************************************************/

static void
break_mes (int x /* Parameter 'x' is never used */ )
{
  /*
   **
   ** break_mes is FATAL.
   ** x == SIGINT for control-C
   ** x == 0 for bwx_STOP
   **
   */
  assert( My != NULL );
  assert( My->SYSOUT != NULL );
  assert( My->SYSOUT->cfp != NULL );
  assert( My->CurrentVersion != NULL );

   
  if (My->ERR < 0)                /* in break_mes(), do not make a bad situation worse */
  {
    /* an error has already ben reported */
  }
  else
  {
    fputc ('\n', My->SYSOUT->cfp);
    ResetConsoleColumn ();
    if (My->CurrentVersion->OptionVersionValue & (C77))
    {
      if (is_empty_string (My->ProgramFilename) == FALSE)
      {
        fprintf (My->SYSOUT->cfp, "FILE:%s, ", My->ProgramFilename);
      }
    }
    fprintf (My->SYSOUT->cfp, "Program interrupted");
    if (My->ThisLine)                /* break_mes */
    {
      if (My->ThisLine->LineFlags & (LINE_USER))        /* break_mes */
      {
        /* don't print the line number */
      }
      else
      {
        fprintf (My->SYSOUT->cfp, " at line %d", My->ThisLine->number);        /* break_mes */
      }
    }
    fputc ('\n', My->SYSOUT->cfp);
    ResetConsoleColumn ();
    fflush (My->SYSOUT->cfp);
  }
  break_handler ();
}

extern void
bwx_STOP (int IsShowMessage)
{
   
  if (IsShowMessage)
  {
    break_mes (0);
  }
  else
  {
    break_handler ();
  }
}

static int
bwx_signon (void)
{
  /*
   **
   ** Display a sign-on banner.
   ** NOT called if a file is provided on the command line.
   **
   */
  int i;
  assert( My != NULL );
  assert( My->SYSOUT != NULL );
  assert( My->SYSOUT->cfp != NULL );

   
  for (i = 0; Banner[i] != NULL; i++)
  {
    fprintf (My->SYSOUT->cfp, "%s\n", Banner[i]);
  }
  ResetConsoleColumn ();
  return TRUE;
}



DoubleType
bwx_TIMER (DoubleType Seconds)
{
  /*
   **
   ** Return a number representing Seconds in the future.  Seconds >= 0.
   ** Seconds may be non-integer, such as 0.001 or 86399.999. The
   ** maximum allowed Seconds is MAXDBL. This is used two ways: 
   **
   ** 1) in bwb_TIMER(), the ON TIMER count (>0) is used to determine 
   ** the expiration time.   In this case, simply return what the value 
   ** will be in the future.  Note that ON TIMER enforces 
   ** Seconds > (1 / CLOCKS_PER_SEC), and
   **
   ** 2) in bwb_execline(), zero (=0) is used to determine the current 
   ** time and compare it to #1. In this case, simply return what the 
   ** value is now.
   **
   ** Both the resolution of the timer, and the frequency of update, 
   ** are implementation defined. 
   **
   */
   
  if (Seconds < 0)
  {
    WARN_INTERNAL_ERROR;
    return 0;
  }
  else
  {
    DoubleType Result;
    Result = clock ();
    assert (CLOCKS_PER_SEC > 0);
    Result /= CLOCKS_PER_SEC;
    if (Seconds > 0)
    {
      Result += Seconds;
    }
    return Result;
  }
}

void
CleanLine (char *buffer)
{
  /* 
   **
   ** cleanup the line, so it is easier to parse 
   **
   */
  char *newbuffer;
   


  if (is_empty_string (buffer))
  {
    /* do nothing */
    return;
  }

  /* remove CR/LF */
  newbuffer = bwb_strchr (buffer, '\r');
  if (newbuffer != NULL)
  {
    *newbuffer = NulChar;
  }
  newbuffer = bwb_strchr (buffer, '\n');
  if (newbuffer != NULL)
  {
    *newbuffer = NulChar;
  }

  /* remove ALL embedded control characters */
  /* if you want a control character, then use CHR$ */
  newbuffer = buffer;
  while (*newbuffer != NulChar)
  {
    if (bwb_isprint (*newbuffer))
    {
      /* OK */
    }
    else
    {
      *newbuffer = ' ';
    }
    newbuffer++;
  }
  /* LTRIM$ */
  newbuffer = buffer;
  if (*newbuffer != NulChar)
  {
    /* not an empty line, so remove one (or more) leading spaces */
    while (*newbuffer == ' ')
    {
      newbuffer++;
    }
    if (newbuffer > buffer)
    {
      bwb_strcpy (buffer, newbuffer);
    }
  }
  /* RTRIM$ */
  newbuffer = buffer;
  if (*newbuffer != NulChar)
  {
    /* not an empty line, so remove one (or more) trailing spaces */
    char *E;

    E = bwb_strchr (newbuffer, NulChar);
    E--;
    while (E >= newbuffer && *E == ' ')
    {
      *E = NulChar;
      E--;
    }
  }
}

static int
bwb_init (void)
{
  /*
   **
   ** initialize Bywater BASIC
   **
   */
  int n;
  static char start_buf[] = "";
  static char end_buf[] = "";

  /* Memory allocation */
  if ((My = (GlobalType *) calloc (1, sizeof (GlobalType))) == NULL)
  {
    return FALSE;
  }
  if ((My->MaxLenBuffer =
       (char *) calloc (MAXLEN + 1 /* NulChar */ , sizeof (char))) == NULL)
  {
    return FALSE;
  }
  if ((My->NumLenBuffer =
       (char *) calloc (NUMLEN + 1 /* NulChar */ , sizeof (char))) == NULL)
  {
    return FALSE;
  }
  if ((My->ConsoleOutput =
       (char *) calloc (MAX_LINE_LENGTH + 1 /* NulChar */ ,
                        sizeof (char))) == NULL)
  {
    return FALSE;
  }
  if ((My->ConsoleInput =
       (char *) calloc (MAX_LINE_LENGTH + 1 /* NulChar */ ,
                        sizeof (char))) == NULL)
  {
    return FALSE;
  }
  if ((My->SYSIN = (FileType *) calloc (1, sizeof (FileType))) == NULL)
  {
    return FALSE;
  }
  if ((My->SYSOUT = (FileType *) calloc (1, sizeof (FileType))) == NULL)
  {
    return FALSE;
  }
  if ((My->SYSPRN = (FileType *) calloc (1, sizeof (FileType))) == NULL)
  {
    return FALSE;
  }
  if ((My->StartMarker = (LineType *) calloc (1, sizeof (LineType))) == NULL)
  {
    return FALSE;
  }
  if ((My->UserMarker = (LineType *) calloc (1, sizeof (LineType))) == NULL)
  {
    return FALSE;
  }
  if ((My->EndMarker = (LineType *) calloc (1, sizeof (LineType))) == NULL)
  {
    return FALSE;
  }
  if ((My->EndMarker = (LineType *) calloc (1, sizeof (LineType))) == NULL)
  {
    return FALSE;
  }
  if ((My->ERROR4 =
       (char *) calloc (MAX_ERR_LENGTH + 1 /* NulChar */ ,
                        sizeof (char))) == NULL)
  {
    return FALSE;
  }

  My->CurrentVersion = &bwb_vertable[0];
  My->IsInteractive = TRUE;
  My->OptionSleepDouble = 1;
  My->OptionIndentInteger = 2;
  My->OptionTerminalType = C_OPTION_TERMINAL_NONE;
  My->OptionRoundType = C_OPTION_ROUND_BANK;
  My->NextValidLineNumber = MINLIN;
  My->IncludeLevel = 0;                /* %INCLUDE */
  My->IsCommandLineFile = FALSE;
  My->ExternalInputFile = NULL;        /* for automated testing, --TAPE command line parameter */
  My->IsPrinter = FALSE;        /* CBASIC-II: CONSOLE and LPRINTER commands */
  My->OptionPromptString = DEF_PROMPT;
  My->OptionEditString = DEF_EDITOR;
  My->OptionFilesString = DEF_FILES;
  My->OptionRenumString = DEF_RENUM;
  My->OptionExtensionString = DEF_EXTENSION;
  My->OptionScaleInteger = 0;
  My->OptionDigitsInteger = SIGNIFICANT_DIGITS;
  My->OptionZoneInteger = ZONE_WIDTH;
  My->UseParameterString = NULL;
  My->ProgramFilename = NULL;

  My->StartMarker->number = MINLIN - 1;
  My->StartMarker->next = My->EndMarker;
  My->StartMarker->position = 0;
  My->StartMarker->buffer = start_buf;

  My->EndMarker->number = MAXLIN + 1;
  My->EndMarker->next = My->EndMarker;
  My->EndMarker->position = 0;
  My->EndMarker->buffer = end_buf;

  My->UserMarker->number = MINLIN - 1;
  My->UserMarker->next = My->EndMarker;
  My->UserMarker->position = 0;
  My->UserMarker->buffer = NULL;

  My->DataLine = My->EndMarker;
  My->DataPosition = 0;

  My->StackHead = NULL;
  My->StackDepthInteger = 0;

  My->FieldHead = NULL;

  My->VirtualHead = NULL;
  My->FileHead = NULL;
  My->ThisLine = My->StartMarker;        /* bwb_init */

  My->SYSIN->DevMode = DEVMODE_INPUT;
  My->SYSIN->width = 80;
  My->SYSIN->col = 1;
  My->SYSIN->row = 1;
  My->SYSIN->delimit = ',';
  My->SYSIN->cfp = stdin;

  My->SYSOUT->DevMode = DEVMODE_OUTPUT;
  My->SYSOUT->width = 80;
  My->SYSOUT->col = 1;
  My->SYSOUT->row = 1;
  My->SYSOUT->delimit = ',';
  My->SYSOUT->cfp = stdout;

  My->SYSPRN->DevMode = DEVMODE_OUTPUT;
  My->SYSPRN->width = 80;
  My->SYSPRN->col = 1;
  My->SYSPRN->row = 1;
  My->SYSPRN->delimit = ',';
  My->SYSPRN->cfp = stderr;

  My->LPRINT_NULLS = 0;
  My->SCREEN_ROWS = 24;

  /* OPEN #0 is an ERROR. */
  /* CLOSE #0 is an ERROR. */
  /* WIDTH #0, 80 is the same as WIDTH 80. */
  /* LPRINT and LLIST are sent to bwx_LPRINT() */

  /* default variable type */
  for (n = 0; n < 26; n++)
  {
    My->DefaultVariableType[n] = DoubleTypeCode;
  }
  /* default COMMAND$(0-9) */
  for (n = 0; n < 10; n++)
  {
    My->COMMAND4[n] = NULL;
  }

  /* initialize tables of variables, functions */
  bwb_initialize_warnings ();
  SortAllCommands ();
  SortAllFunctions ();
  SortAllOperators ();
  var_init ();
  IntrinsicFunction_init ();
  UserFunction_init ();
  OptionVersionSet (0);
  /* OK */
  return TRUE;
}

/***************************************************************
  
        FUNCTION:       main()
  
        DESCRIPTION:    As in any C program, main() is the basic
                        function from which the rest of the
                        program is called. Some environments,
         however, provide their own main() functions
         (Microsoft Windows (tm) is an example).
         In these cases, the following code will
         have to be included in the initialization
         function that is called by the environment.
  
***************************************************************/

static void
process_basic_line (char *buffer)
{
  CleanLine (buffer);
  if (is_empty_string (buffer))
  {
    /* empty -- do nothing */
  }
  else if (is_ln (buffer) == FALSE)
  {
    /* If there is no line number, then execute the line as received */
    /* RUN */
    WARN_CLEAR;                        /* process_basic_line */
    SetOnError (0);
    bwb_xtxtline (buffer);        /* process_basic_line */
  }
  else if (is_numconst (buffer) == TRUE)
  {
      /*-----------------------------------------------------------------*/
    /* Another possibility: if buffer is a numeric constant,           */
    /* then delete the indicated line number (JBV)                     */
      /*-----------------------------------------------------------------*/
    /* 100 */
    int LineNumber;
    LineNumber = atoi (buffer);
    WARN_CLEAR;                        /* process_basic_line */
    SetOnError (0);
    sprintf (buffer, "delete %d", LineNumber);
    bwb_xtxtline (buffer);        /* process_basic_line */
  }
  else
  {
    /* If there is a line number, then add it to the BASIC program */
    /* 100 REM */
    bwb_ladd (buffer, My->StartMarker, FALSE);
  }
}
static void
bwb_single_step (char *buffer)
{
  assert( My != NULL );
  assert (buffer != NULL);

  process_basic_line (buffer);
  while (My->StackHead != NULL)
  {
    bwb_execline ();
  }
}

static void
mark_preset_variables (void)
{
  /* mark all existing variables as preset */
  /* this includes all variables declared in any PROFILE */
  VariableType *v;
  assert( My != NULL );
   

  for (v = My->VariableHead; v != NULL; v = v->next)
  {
    v->VariableFlags |= VARIABLE_PRESET;
    v->VariableFlags |= VARIABLE_COMMON;
  }
}


static void
execute_profile (char *FileName)
{
  FILE *profile;
  assert( My != NULL );
  assert (FileName != NULL);

  My->NextValidLineNumber = MINLIN;
  profile = fopen (FileName, "r");
  if (profile == NULL)
  {
    /* NOT FOUND */
    /* OPTIONAL */
    return;
  }
  /* FOUND */
  if (My->IsInteractive)
  {
    /* 
     **
     ** set a buffer for jump: program execution returns to this point in
     ** case of a jump (error, interrupt, or finish program) 
     **
     */
    My->program_run = 0;
    signal (SIGINT, break_mes);
    setjmp (My->mark);
    if (My->program_run > 0)
    {
      /* error in PROFILE */
      exit (1);
    }
    My->program_run++;
  }

  /* 
     The profile only exists to allow executing OPTION ... commands.  No other use is supported. 
   */
  {
    char *tbuf;
    int tlen;

    tbuf = My->ConsoleInput;
    tlen = MAX_LINE_LENGTH;
    while (fgets (tbuf, tlen, profile))        /* execute_profile */
    {
      tbuf[tlen] = NulChar;
      bwb_single_step (tbuf);        /* in execute_profile() */
    }
    bwb_fclose (profile);
    mark_preset_variables ();
  }
}



static void
execute_program (char *FileName)
{
  assert( My != NULL );
  assert( My->SYSOUT != NULL );
  assert( My->SYSOUT->cfp != NULL );
   
  assert (FileName != NULL);

  My->NextValidLineNumber = MINLIN;
  My->IsCommandLineFile = TRUE;
  if (bwb_fload (FileName) == FALSE)
  {
    fprintf (My->SYSOUT->cfp, "Failed to open file %s\n", FileName);
    /* the file load has failed, so do NOT run the program */
    exit (1);
  }
  if (My->ERR < 0 /* in execute_program(), file load failed */ )
  {
    /* the file load has failed, so do NOT run the program */
    exit (1);
  }
  bwb_RUN (My->StartMarker);
}

extern int
main (int argc, char **argv)
{
  int i;
  assert (argc >= 0);
  assert (argv != NULL);

  if (bwb_init () == FALSE)
  {
    /* FATAL */
    puts ("Out of memory");
    return 1;
  }
  assert( My != NULL );
  assert( My->SYSOUT != NULL );
  assert( My->SYSOUT->cfp != NULL );
  assert( My->SYSIN != NULL );
  assert( My->SYSIN->cfp != NULL );

  /* Signon message banner */
  if (argc < 2)
  {
    /* no parameters */
    if (My->IsInteractive)
    {
      bwx_signon ();
    }
    else
    {
      /* if INTERACTIVE is FALSE, then we must have a program file */
      fputs ("Program file not specified\n", My->SYSOUT->cfp);
      return 1;
    }
  }

  if (My->IsInteractive)
  {
    /* 
     **
     ** set a buffer for jump: program execution returns to this point in
     ** case of a jump (error, interrupt, or finish program) 
     **
     */
    My->program_run = 0;
    signal (SIGINT, break_mes);
    setjmp (My->mark);
    if (My->program_run > 0)
    {
      /* error in profile */
      return 1;
    }
    My->program_run++;
  }

#if PROFILE
  execute_profile (PROFILENAME);
#endif

  /* check to see if there is a program file: but do this only the first time around! */
  for (i = 1; i < argc; i++)
  {
    /* 
       SYNTAX:  bwbasic [ --profile profile.bas ] [ --tape tapefile.inp ] [ program.bas ]
     */
    if (bwb_stricmp (argv[i], "--profile") == 0
        || bwb_stricmp (argv[i], "-p") == 0
        || bwb_stricmp (argv[i], "/profile") == 0
        || bwb_stricmp (argv[i], "/p") == 0)
    {
      i++;
      if (i < argc)
      {
        /* --profile profile.bas */
        execute_profile (argv[i]);
      }
    }
    else
      if (bwb_stricmp (argv[i], "--tape") == 0
          || bwb_stricmp (argv[i], "-t") == 0
          || bwb_stricmp (argv[i], "/tape") == 0
          || bwb_stricmp (argv[i], "/t") == 0)
    {
      i++;
      if (i < argc)
      {
        /* --tape tapefile.inp */
        My->ExternalInputFile = fopen (argv[i], "r");
      }
    }
    else
    {
      /* program.bas */
      {
        int j;
        int n;

        j = i;
        for (n = 0; n < 10 && j < argc; n++, j++)
        {
          My->COMMAND4[n] = argv[j];
        }
      }
      execute_program (argv[i]);
      break;
    }
  }

  if (My->IsInteractive)
  {
    /* 
     **
     ** set a buffer for jump: program execution returns to this point in
     ** case of a jump (error, interrupt, or finish program) 
     **
     */
    My->program_run = 0;
    signal (SIGINT, break_mes);
    setjmp (My->mark);
    if (My->program_run > 0)
    {
      /* error in console mode */
    }
    My->program_run++;
  }

  /* main program loop */
  My->NextValidLineNumber = MINLIN;
  while (!feof (My->SYSIN->cfp))        /* condition !feof( My->SYSIN->cfp ) added in v1.11 */
  {
    bwb_mainloop ();
  }

  bwx_terminate ();                /* allow ^D (Unix) exit with grace */

  return 0;
}



/***************************************************************
  
   FUNCTION:       bwb_interact()
  
   DESCRIPTION:   This function gets a line from the user
         and processes it.
  
***************************************************************/

static void
bwb_interact (void)
{
  char *tbuf;
  int tlen;
  assert( My != NULL );
   

  tbuf = My->ConsoleInput;
  tlen = MAX_LINE_LENGTH;
  My->NextValidLineNumber = MINLIN;
  /* take input from keyboard */
  if (My->AutomaticLineNumber > 0 && My->AutomaticLineIncrement > 0)
  {
    /* AUTO 100, 10 */
    char *zbuf;                        /* end of the prompt, start of the response */
    int zlen;                        /* length of the prompt */
    char LineExists;
    LineType *l;

    LineExists = ' ';
    for (l = My->StartMarker->next; l != My->EndMarker; l = l->next)
    {
      if (l->number == My->AutomaticLineNumber)
      {
        /* FOUND */
        LineExists = '*';
        break;
      }
      else if (l->number > My->AutomaticLineNumber)
      {
        /* NOT FOUND */
        LineExists = ' ';
        break;
      }
    }
    sprintf (tbuf, "%d%c", My->AutomaticLineNumber, LineExists);
    zbuf = bwb_strchr (tbuf, NulChar);
    zlen = bwb_strlen (tbuf);
    bwx_input (tbuf, FALSE, zbuf, tlen - zlen);
    zbuf[-1] = ' ';                /* remove LineExists indicator */
    CleanLine (zbuf);                /* JBV */
    if (is_empty_string (zbuf))
    {
      /* empty response */
      if (LineExists == '*')
      {
        /*
           An empty response with an existing line,
           causes AUTO to continue with the next line,
           leaving the current line intact.
         */
        My->AutomaticLineNumber += My->AutomaticLineIncrement;
      }
      else
      {
        /* 
           An empty response with a non-existing line,
           causes AUTO to terminate.
         */
        My->AutomaticLineNumber = 0;
        My->AutomaticLineIncrement = 0;
      }
    }
    else
    {
      /* non-empty response */
      if (bwb_stricmp (zbuf, "MAN") == 0)
      {
        /* MAN terminates AUTO mode */
        My->AutomaticLineNumber = 0;
        My->AutomaticLineIncrement = 0;
      }
      else
      {
        /* overwrite any existing line */
        bwb_ladd (tbuf, My->StartMarker, FALSE);
        My->AutomaticLineNumber += My->AutomaticLineIncrement;
      }
    }
  }
  else
  {
    bwx_input (My->OptionPromptString, FALSE, tbuf, tlen);
    process_basic_line (tbuf);
  }
}



/***************************************************************
  
      FUNCTION:       bwb_fload()
  
      DESCRIPTION:   This function loads a BASIC program
         file into memory given a FILE pointer.
  
***************************************************************/

static int
FixQuotes (char *tbuf)
{
  /* fix unbalanced quotes */
  /* 'tbuf' shall be declared "char tbuf[ tlen + 1]". */
  int p;
  int QuoteCount;
  int tlen;
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
   
  assert (tbuf != NULL);

  QuoteCount = 0;
  tlen = MAX_LINE_LENGTH;
  tbuf[tlen] = NulChar;
  p = 0;
  while (tbuf[p])
  {
    if (tbuf[p] == My->CurrentVersion->OptionQuoteChar)
    {
      QuoteCount++;
    }
    p++;
  }
  if (QuoteCount & 1)
  {
    /* odd == missing trailing quote */
    if (p < tlen)
    {
      /* we have room to put the missig quote */
      tbuf[p] = My->CurrentVersion->OptionQuoteChar;
      p++;
      tbuf[p] = NulChar;
    }
    else
    {
      /* we cannot fix it */
      return FALSE;
    }
  }
  /* OK */
  return TRUE;
}

static FILE *
nice_open (char *BaseName)
{
  FILE *file;
  assert( My != NULL );

  if (BaseName == NULL)
  {
    BaseName = My->ProgramFilename;
  }
  if (is_empty_string (BaseName))
  {
    WARN_BAD_FILE_NAME;
    return NULL;
  }

  file = fopen (BaseName, "r");
  if (file == NULL)
  if (is_empty_string (My->OptionExtensionString) == FALSE)
  {
    char *FileName;

    FileName = bwb_strdup2 (BaseName, My->OptionExtensionString);
    if (FileName == NULL)
    {
      WARN_OUT_OF_MEMORY;
      return NULL;
    }
    file = fopen (FileName, "r");
    if (file == NULL)
    {
      free (FileName);
    }
    else if (BaseName == My->ProgramFilename)
    {
      if (My->ProgramFilename != NULL)
      {
        free (My->ProgramFilename);
      }
      My->ProgramFilename = FileName;
    }
  }
  return file;
}

extern int
bwb_fload (char *FileName)
{
  /*
   **
   ** Load a BASIC program from the specified 'FileName'.
   ** If 'FileName' is NULL, then load from My->ProgramFilename,
   **
   */
  char *Magic_Word;                /* SYNTAX: %INCLUDE literal.file.name */
  int Magic_Length;
  FILE *file;
  char *tbuf;
  int tlen;
   



  Magic_Word = "%INCLUDE ";        /* SYNTAX: %INCLUDE literal.file.name */
  Magic_Length = bwb_strlen (Magic_Word);
  tbuf = My->MaxLenBuffer;
  tlen = MAXLEN;


  /*
     Just in case you are wondering...
     Although this handles the most common cases, it does not handle all possible cases.
     The correct solution is to provide the actual filename (with extension),
     as it exists in the operating system.
   */
  file = nice_open (FileName);
  if (file == NULL)
  {
    WARN_BAD_FILE_NAME;
    return FALSE;
  }
  My->NextValidLineNumber = MINLIN;
  while (fgets (tbuf, tlen, file))        /* bwb_fload */
  {
    tbuf[tlen] = NulChar;
    CleanLine (tbuf);
    if (is_empty_string (tbuf))
    {
      /* ignore */
    }
    else if (bwb_strnicmp (tbuf, Magic_Word, Magic_Length) == 0)
    {
      /* %iNCLUDE */
      int Result;
      int p;
      char varname[NameLengthMax + 1];

      p = Magic_Length;
      if (buff_read_varname (tbuf, &p, varname) == FALSE)
      {
        fprintf (My->SYSOUT->cfp, "%s Filename\n", Magic_Word);
        ResetConsoleColumn ();
        return FALSE;
      }
      My->IncludeLevel++;        /* %INCLUDE */
      Result = bwb_fload (varname);
      My->IncludeLevel--;        /* %INCLUDE */
      if (Result == FALSE)
      {
        fprintf (My->SYSOUT->cfp, "%s %s Failed\n", Magic_Word, varname);
        ResetConsoleColumn ();
        return FALSE;
      }
    }
    else
    {
      /* normal */
      bwb_ladd (tbuf, My->StartMarker, FALSE);
    }
  }

  /* close file stream */

  bwb_fclose (file);                /* file != NULL */

  My->NextValidLineNumber = MINLIN;

  return TRUE;
}


static char *
FindClassicStatementEnd (char *C)
{
  /* 
   ** find the end of the current statement
   */
  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
   
  assert (C != NULL);


  if (My->CurrentVersion->OptionStatementChar == NulChar
      && My->CurrentVersion->OptionCommentChar == NulChar)
  {
    /* DO NOTHING: Multi-statment lines are not possible */
    return NULL;
  }
  /* skip line number */
  while (bwb_isdigit (*C))
  {
    C++;
  }
  /* skip spaces */
  while (*C == ' ')
  {
    C++;
  }
  if (IS_CHAR (*C, My->CurrentVersion->OptionCommentChar))
  {
    /* The entire line is a comment */
    return NULL;
  }
  if (bwb_strnicmp (C, "REM", 3) == 0)
  {
    /* The entire line is a comment */
    return NULL;
  }
  if ((My->CurrentVersion->OptionFlags & OPTION_LABELS_ON)
      && (My->CurrentVersion->OptionStatementChar != NulChar))
  {
    /* determine if this line is a LABEL */
    int p;
    char label[NameLengthMax + 1];

    p = 0;
    if (buff_read_label (C, &p, label))
    {
      if (buff_skip_char (C, &p, My->CurrentVersion->OptionStatementChar))
      {
        if (buff_is_eol (C, &p))
        {
          /* The entire line is a label */
          /* LABEL : \0 */
          return NULL;
        }
      }
    }
  }
  /* not a special case, so split on the first unquoted OptionCommentChar or OptionStatementChar */
  while (*C != NulChar)
  {
    if (*C == My->CurrentVersion->OptionQuoteChar)
    {
      /* skip leading quote */
      C++;
      while (*C != NulChar && *C != My->CurrentVersion->OptionQuoteChar)
      {
        /* skip string constant */
        C++;
      }
      if (*C == My->CurrentVersion->OptionQuoteChar)
      {
        /* skip trailing quote */
        C++;
      }
    }
    else if (IS_CHAR (*C, My->CurrentVersion->OptionCommentChar) /* ', ! */ )
    {
      /* FOUND */
      return C;
    }
    else
      if (IS_CHAR (*C, My->CurrentVersion->OptionStatementChar) /* :, \ */ )
    {
      /* FOUND */
      return C;
    }
    else
    {
      C++;
    }
  }
  /* NOT FOUND */
  return NULL;
}


static void
ImportClassicIfThenElse (char *InBuffer)
{
/*
**
** Determine the type of IF command:
**
** a) STANDARD: 
**    IF x THEN line ELSE line
**
** b) CLASSIC:
**    IF x THEN stmt(s) ELSE stmt(s)
**
** c) STRUCTURED:
**    IF x THEN
**       stmts
**    ELSE
**      stmts
**    END IF
**
** The STANDARD and STRUCTURED forms
** are natively supported.
**
** The CLASSIC form is converted to
** the STRUCTURED form.
**
*/

  int i;

  int nIF = 0;
  int nTHEN = 0;
  int nELSE = 0;
  int nENDIF = 0;

#define NO_COMMAND    0
#define IF_COMMAND    1
#define THEN_COMMAND  2
#define ELSE_COMMAND  3
#define ENDIF_COMMAND 4
  int LastCommand = NO_COMMAND;

  const char *REM = "REM ";
  const char *IF = "IF ";
  const char *THEN = "THEN ";
  const char *THEN2 = "THEN";
  const char *ELSE = "ELSE ";
  const char *ENDIF = "END IF";
  const char *GOTO = "GOTO ";
  const char *DATA = "DATA ";
  const char *CASE = "CASE ";
  char *OutBuffer = My->ConsoleOutput;
  char *Input;
  char *Output;
  char LastChar = My->CurrentVersion->OptionStatementChar;

  int REM_len = bwb_strlen (REM);
  int IF_len = bwb_strlen (IF);
  int THEN_len = bwb_strlen (THEN);
  int THEN2_len = bwb_strlen (THEN2);
  int ELSE_len = bwb_strlen (ELSE);
  int ENDIF_len = bwb_strlen (ENDIF);
  int GOTO_len = bwb_strlen (GOTO);
  int DATA_len = bwb_strlen (DATA);
  int CASE_len = bwb_strlen (CASE);

#define OUTPUTCHAR( c ) { *Output = c; Output++; }
#define COPYCHAR  { LastChar = *Input; *Output = *Input; Output++; Input++; }
#define COPY_LINENUMBER  while( bwb_isdigit( *Input )       ) COPYCHAR;
#define COPY_SPACES      while( *Input == ' '     ) COPYCHAR;
#define COPY_IF          for( i = 0; i < IF_len; i++    ) COPYCHAR;
#define COPY_THEN        for( i = 0; i < THEN_len; i++  ) COPYCHAR;
#define COPY_THEN2       for( i = 0; i < THEN2_len; i++ ) COPYCHAR;
#define COPY_ELSE        for( i = 0; i < ELSE_len; i++  ) COPYCHAR;
#define COPY_ENDIF       for( i = 0; i < ENDIF_len; i++ ) COPYCHAR;
#define FORCE_ENDIF      for( i = 0; i < ENDIF_len; i++ ) OUTPUTCHAR( ENDIF[ i ] );
#define FORCE_GOTO       for( i = 0; i < GOTO_len; i++  ) OUTPUTCHAR( GOTO[ i ] );
#define FORCE_COLON if( LastChar != My->CurrentVersion->OptionStatementChar ) OUTPUTCHAR( My->CurrentVersion->OptionStatementChar );

  assert( My != NULL );
  assert( My->CurrentVersion != NULL );
  assert (InBuffer != NULL);


  Input = InBuffer;
  Output = OutBuffer;





  if (My->CurrentVersion->OptionStatementChar == NulChar)
  {
    /* DO NOTHING: All IFs must be STANDARD or STRUCTURED */
    return;
  }


  COPY_LINENUMBER;
  COPY_SPACES;
  LastChar = My->CurrentVersion->OptionStatementChar;


  while (*Input != NulChar)
  {
    if (*Input == My->CurrentVersion->OptionCommentChar)
    {
      /* comment */
      break;
    }
    else if (*Input == My->CurrentVersion->OptionQuoteChar)
    {
      /* string constant */
      COPYCHAR;
      while (*Input != NulChar
             && *Input != My->CurrentVersion->OptionQuoteChar)
      {
        COPYCHAR;
      }
      if (*Input == My->CurrentVersion->OptionQuoteChar)
      {
        COPYCHAR;
      }
      else
      {
        /* add missing Quote */
        OUTPUTCHAR (My->CurrentVersion->OptionQuoteChar);
      }
      COPY_SPACES;
    }
    else if (bwb_isalnum (LastChar))
    {
      /* can NOT be the start of a command */
      COPYCHAR;
    }
    else if (!bwb_isalpha (*Input))
    {
      /* can NOT be the start of a command */
      COPYCHAR;
    }
    else if (bwb_strnicmp (Input, REM, REM_len) == 0)
    {
      break;
    }
    else if (bwb_strnicmp (Input, DATA, DATA_len) == 0)
    {
      /* DATA ... */
      break;
    }
    else if (bwb_strnicmp (Input, CASE, CASE_len) == 0)
    {
      /* CASE ... */
      break;
    }
    else if (bwb_strnicmp (Input, IF, IF_len) == 0)
    {
      /* IF */
      LastCommand = IF_COMMAND;
      nIF++;
      COPY_IF;
      COPY_SPACES;
    }
    else if (bwb_strnicmp (Input, GOTO, GOTO_len) == 0 && nIF > nTHEN)
    {
      /* IF x GOTO line ELSE line */
      LastCommand = THEN_COMMAND;
      nTHEN++;
      COPY_THEN;
      COPY_SPACES;
      COPY_LINENUMBER;
      COPY_SPACES;
      if (bwb_strnicmp (Input, ELSE, ELSE_len) == 0)
      {
        /* ELSE line */
        COPY_ELSE;
        COPY_SPACES;
        COPY_LINENUMBER;
        COPY_SPACES;
      }
      /* IS STANDARD, NOT CLASSIC */
      nENDIF++;
      LastCommand = ENDIF_COMMAND;
    }
    else if (bwb_strnicmp (Input, THEN, THEN_len) == 0)
    {
      /* THEN */
      LastCommand = THEN_COMMAND;
      nTHEN++;
      COPY_THEN;
      COPY_SPACES;
      if (bwb_isdigit (*Input))
      {
        /* STANDARD: IF x THEN line ELSE line */
        char *SavedInput;
        char *SavedOutput;
        SavedInput = Input;
        SavedOutput = Output;

        COPY_LINENUMBER;
        COPY_SPACES;
        if (bwb_strnicmp (Input, ELSE, ELSE_len) == 0)
        {
          /* ELSE line */
          COPY_ELSE;
          COPY_SPACES;
          if (bwb_isdigit (*Input))
          {
            COPY_LINENUMBER;
            COPY_SPACES;
            /* IS STANDARD, NOT CLASSIC */
            nENDIF++;
            LastCommand = ENDIF_COMMAND;
          }
          else
          {
            /* IF x THEN line ELSE stmts */
            Input = SavedInput;
            Output = SavedOutput;
            FORCE_COLON;
            FORCE_GOTO;
            COPY_LINENUMBER;
            COPY_SPACES;
          }
        }
        else
        {
          /* IS STANDARD, NOT CLASSIC */
          nENDIF++;
          LastCommand = ENDIF_COMMAND;
        }
      }
      else
        if (*Input == My->CurrentVersion->OptionCommentChar
            || *Input == NulChar)
      {
        /* IS STRUCTURED, NOT CLASSIC */
        nENDIF++;
        LastCommand = ENDIF_COMMAND;
      }
      else
      {
        /* CLASSIC: IF x THEN stmts ELSE stmts */
      }
      FORCE_COLON;
    }
    else if (bwb_strnicmp (Input, THEN2, THEN2_len) == 0)
    {
      /* trailing THEN ? */
      char *PeekInput;

      PeekInput = Input;
      PeekInput += THEN2_len;
      while (*PeekInput == ' ')
      {
        PeekInput++;
      }
      if (*PeekInput == My->CurrentVersion->OptionCommentChar
          || *PeekInput == NulChar)
      {
        /* IS STRUCTURED, NOT CLASSIC */
        nTHEN++;
        COPY_THEN2;
        nENDIF++;
        LastCommand = ENDIF_COMMAND;
        FORCE_COLON;
      }
      else
      {
        /* THEN line */
      }
    }
    else if (bwb_strnicmp (Input, ELSE, ELSE_len) == 0)
    {
      /* ELSE */
      if (LastCommand == ELSE_COMMAND)
      {
        /* we need an ENDIF here */
        FORCE_COLON;
        FORCE_ENDIF;
        FORCE_COLON;
        nENDIF++;
      }
      LastCommand = ELSE_COMMAND;
      nELSE++;
      FORCE_COLON;
      COPY_ELSE;
      FORCE_COLON;
      COPY_SPACES;
      if (bwb_isdigit (*Input))
      {
        /* IF x THEN stmts ELSE line */
        FORCE_GOTO;
        COPY_LINENUMBER;
        COPY_SPACES;
      }
      FORCE_COLON;
    }
    else if (bwb_strnicmp (Input, ENDIF, ENDIF_len) == 0)
    {
      /* ENDIF */
      LastCommand = ENDIF_COMMAND;
      nENDIF++;
      COPY_ENDIF;
      FORCE_COLON;
    }
    else
    {
      /* was NOT the start of a command */
      COPYCHAR;
    }
  }
  /* end of input */
  if (nENDIF < nIF)
  {
    while (nENDIF < nIF)
    {
      /* we need trailing ENDIF's */
      nENDIF++;
      FORCE_COLON;
      FORCE_ENDIF;
    }
  }
  /* fixup trailing REMark command */
  if (bwb_strnicmp (Input, REM, REM_len) == 0)
  {
    /* REMark */
    /* 100 A=1 REMark */
    /* 100 A=1:REMark */
    /* 100 A=1'REMark */
    FORCE_COLON;
  }
  /* copy the comments */
  while (*Input != NulChar)
  {
    COPYCHAR;
    /* cppcheck: (style) Variable 'LastChar' is assigned a value that is never used. */
  }
  OUTPUTCHAR (NulChar);
  bwb_strcpy (InBuffer, OutBuffer);
}


/***************************************************************
  
        FUNCTION:       bwb_ladd()
  
        DESCRIPTION:    This function adds a new line (in the
                        buffer) to the program in memory.
  
***************************************************************/
static int
bwb_ladd (char *buffer, LineType * p, int IsUser)
{
  LineType *l;
  LineType *previous;
  char *newbuffer;
  char *NextStatement;
  char *ThisStatement;
  int Replace;
  char BreakChar;

  assert( My != NULL );
  assert( My->SYSOUT != NULL );
  assert( My->SYSOUT->cfp != NULL );
  assert( My->CurrentVersion != NULL );
  assert (buffer != NULL);
  assert (p != NULL);


  Replace = TRUE;
  BreakChar = NulChar;
  CleanLine (buffer);
  if (is_empty_string (buffer))
  {
    /* silengtly ignore blank lines */
    return FALSE;
  }
  /*
     from here, the line WILL be added so the user can EDIT it,  
     we just complain and refuse to run if any error is detected.
   */
  My->IsScanRequired = TRUE;        /* program needs to be scanned again */

  /* AUTO-FIX UNBALANCED QUOTES */
  if (FixQuotes (buffer) == FALSE)
  {
    /* ERROR */
    fprintf (My->SYSOUT->cfp, "UNBALANCED QUOTES: %s\n", buffer);
    ResetConsoleColumn ();
    My->ERR = -1;                /* bwb_ladd, UNBALANCED QUOTES */
  }

  if (IS_CHAR (*buffer, My->CurrentVersion->OptionStatementChar))
  {
    /* part of a multi-statement line */
  }
  else if (IS_CHAR (*buffer, My->CurrentVersion->OptionCommentChar))
  {
    /* part of a multi-statement line */
  }
  else
  {
    ImportClassicIfThenElse (buffer);
  }
  ThisStatement = buffer;
  NextStatement = NULL;
  BreakChar = NulChar;

  do
  {
    if (BreakChar == NulChar)
    {
      /* first pass thru the do{} while loop, do nothing */
    }
    else if (IS_CHAR (BreakChar, My->CurrentVersion->OptionCommentChar))
    {
      /* ThisStatment will turn out to be the last */
      *ThisStatement = My->CurrentVersion->OptionCommentChar;
    }
    else if (IS_CHAR (BreakChar, My->CurrentVersion->OptionStatementChar))
    {
      /* we are NOT the last statement, skip over the OptionStatementChar */
      ThisStatement++;
    }
    else
    {
      /* Internal Error */
    }

    if (BreakChar == NulChar
        && IS_CHAR (*buffer, My->CurrentVersion->OptionStatementChar))
    {
      /* first pass thru and line begins with colon */
      /* part of a multi-statement line */
      NextStatement = NULL;
      if (My->NextValidLineNumber > 1)
      {
        My->NextValidLineNumber--;
      }
      Replace = FALSE;
    }
    else
      if (BreakChar == NulChar
          && IS_CHAR (*buffer, My->CurrentVersion->OptionCommentChar))
    {
      /* first pass thru and line begins with apostrophe */
      /* part of a multi-statement line */
      NextStatement = NULL;
      if (My->NextValidLineNumber > 1)
      {
        My->NextValidLineNumber--;
      }
      Replace = FALSE;
    }
    else
    {
      NextStatement = FindClassicStatementEnd (ThisStatement);
    }

    if (NextStatement == NULL)
    {
      /* we are the last statement */
    }
    else
    {
      /* another statement follows */
      BreakChar = *NextStatement;
      *NextStatement = NulChar;
    }
    CleanLine (ThisStatement);
    if (is_empty_string (ThisStatement) == FALSE)
    {

      /* get memory for this line */
      if ((l = (LineType *) calloc (1, sizeof (LineType))) == NULL)
      {
        /* ERROR */
        fprintf (My->SYSOUT->cfp, "Out of memory\n");
        ResetConsoleColumn ();
        My->ERR = -1;                /* bwb_ladd, Out of memory */
        return FALSE;
      }

      /* this line has not been executed or numbered */
      l->LineFlags = 0;
      if (IsUser)
      {
        l->LineFlags |= LINE_USER;
      }
      l->IncludeLevel = My->IncludeLevel;        /* %INCLUDE */
      l->position = 0;

      /*
       **
       ** ALL lines have a line number.  
       ** If a line number is not provided, 
       ** then the next available line number is assigned.
       **
       */
      newbuffer = ThisStatement;
      l->number = My->NextValidLineNumber;

      if (buff_read_line_number (newbuffer, &(l->position), &l->number))
      {
        if (l->number < My->NextValidLineNumber)
        {
          /* ERROR */
          fprintf (My->SYSOUT->cfp, "%d < %d - LINE OUT OF ORDER: %s\n",
                   l->number, My->NextValidLineNumber, buffer);
          ResetConsoleColumn ();
          My->ERR = -1;                /* bwb_ladd, LINE OUT OF ORDER */
          l->number = My->NextValidLineNumber;        /* sane default */
        }
        else if (l->number < MINLIN || l->number > MAXLIN)
        {
          /* ERROR */
          fprintf (My->SYSOUT->cfp, "INVALID LINE NUMBER: %s\n", buffer);
          ResetConsoleColumn ();
          My->ERR = -1;                /* bwb_ladd, INVALID LINE NUMBER */
          l->number = My->NextValidLineNumber;        /* sane default */
        }
        else
        {
          /* OK */
          My->NextValidLineNumber = l->number;
          l->LineFlags |= LINE_NUMBERED;        /* line was manually numbered */
        }
        /* A SPACE IS REQUIRED AFTER THE LINE NUMBER -- NO EXCEPTIONS */
        if (newbuffer[l->position] != ' ')
        {
          /* ERROR */
          fprintf (My->SYSOUT->cfp, "MISSING SPACE AFTER LINE NUMBER: %s\n",
                   buffer);
          ResetConsoleColumn ();
          My->ERR = -1;                /* bwb_ladd, MISSING SPACE AFTER LINE NUMBER */
        }
        /* newuffer does NOT contain the line number */
        newbuffer += l->position;
      }
      /* the next valid line number is this line number plus one */
      CleanLine (newbuffer);
      if (*newbuffer != NulChar
          && *newbuffer == My->CurrentVersion->OptionStatementChar)
      {
        /* this is part of a multi-statement line */
        newbuffer++;
        CleanLine (newbuffer);
      }
      /* 
       **
       ** copy into the line buffer 
       **
       */
      if (l->buffer != NULL)
      {
        free (l->buffer);
        l->buffer = NULL;        /* JBV */
      }
      if ((l->buffer =
           (char *) calloc (bwb_strlen (newbuffer) + 1 /* NulChar */ ,
                            sizeof (char))) == NULL)
      {
        /* ERROR */
        fprintf (My->SYSOUT->cfp, "Out of memory\n");
        ResetConsoleColumn ();
        My->ERR = -1;                /* bwb_ladd, Out of memory */
        return FALSE;                /* we cannot determine the command */
      }
      bwb_strcpy (l->buffer, newbuffer);
      /*
       **
       ** determine command
       **
       */
      line_start (l);
      if (l->cmdnum)
      {
        /* OK */
      }
      else
      {
        /* ERROR */
        fprintf (My->SYSOUT->cfp,
                 "ILLEGAL COMMAND AFTER LINE NUMBER: %d %s\n", l->number,
                 l->buffer);
        ResetConsoleColumn ();
        My->ERR = -1;                /* bwb_ladd, ILLEGAL COMMAND AFTER LINE NUMBER */
      }
      /*
       **
       ** add the line to the linked-list of lines
       **
       */
      for (previous = p; previous != My->EndMarker; previous = previous->next)
      {
        if (previous->number == l->number)
        {
          if (Replace == TRUE)
          {
            /* REPLACE 'previous' */
            while (previous->number == l->number)
            {
              LineType *z;

              z = previous;
              previous = previous->next;
              bwb_freeline (z);
            }
            l->next = previous;
            p->next = l;

          }
          else
          {
            /* APPEND after 'previous' */
            while (previous->next->number == l->number)
            {
              previous = previous->next;
            }
            l->next = previous->next;
            previous->next = l;
          }
          break;
        }
        else
          if (previous->number < l->number
              && l->number < previous->next->number)
        {
          /* INSERT BETWEEN 'previous' AND 'previous->next' */
          l->next = previous->next;
          previous->next = l;
          break;
        }
        p = previous;
      }

    }
    /* another statement may follow */
    ThisStatement = NextStatement;
    Replace = FALSE;
  }
  while (ThisStatement != NULL);
  My->NextValidLineNumber++;
  return TRUE;
}

/***************************************************************
  
        FUNCTION:       bwb_xtxtline()
  
        DESCRIPTION:    This function executes a text line, i.e.,
                        places it in memory and then relinquishes
                        control.
  
***************************************************************/

static void
bwb_xtxtline (char *buffer)
{
  assert( My != NULL );
  assert (buffer != NULL);

  /* remove old program from memory */
  bwb_xnew (My->UserMarker);

  CleanLine (buffer);
  if (is_empty_string (buffer))
  {
    /* silengtly ignore blank lines */
    return;
  }

  /* add to the user line list */
  bwb_ladd (buffer, My->UserMarker, TRUE);

  /* execute the line as BASIC command line */
  if (bwb_incexec ())
  {
    My->StackHead->line = My->UserMarker->next;        /* and set current line in it */
    My->StackHead->ExecCode = EXEC_NORM;
  }
}

/***************************************************************
  
        FUNCTION:       bwb_incexec()
  
        DESCRIPTION:    This function increments the EXEC
         stack counter.
  
***************************************************************/

int
bwb_incexec (void)
{
  StackType *StackItem;
  assert( My != NULL );
   

  if (My->StackDepthInteger >= EXECLEVELS)
  {
    WARN_OUT_OF_MEMORY;
    return FALSE;
  }
  if ((StackItem = (StackType *) calloc (1, sizeof (StackType))) == NULL)
  {
    WARN_OUT_OF_MEMORY;
    return FALSE;
  }
  StackItem->ExecCode = EXEC_NORM;        /* sane default */
  StackItem->line = My->ThisLine;        /* bwb_incexec */
  StackItem->LoopTopLine = NULL;
  StackItem->local_variable = NULL;
  StackItem->OnErrorGoto = 0;
  StackItem->next = My->StackHead;
  My->StackHead = StackItem;
  My->StackDepthInteger++;
  return TRUE;
}

/***************************************************************
  
        FUNCTION:       bwb_decexec()
  
        DESCRIPTION:    This function decrements the EXEC
         stack counter.
  
***************************************************************/
void
bwb_clrexec (void)
{
  assert( My != NULL );
   
  while (My->StackHead != NULL)
  {
    bwb_decexec ();
  }
}

void
bwb_decexec (void)
{
  StackType *StackItem;
  assert( My != NULL );
   

  if (My->StackHead == NULL)
  {
    WARN_RETURN_WITHOUT_GOSUB;
    return;
  }
  StackItem = My->StackHead;
  My->StackHead = StackItem->next;
  free (StackItem);
  My->StackDepthInteger--;
}

/***************************************************************
  
        FUNCTION:       bwb_mainloop()
  
        DESCRIPTION:    This C function performs one iteration
                        of the interpreter. In a non-preemptive
                        scheduler, this function should be called
                        by the scheduler, not by bwBASIC code.
  
***************************************************************/

void
bwb_mainloop (void)
{
  assert( My != NULL );
   

  if (My->StackHead)
  {
    /* BASIC program running */
    bwb_execline ();                /* execute one line of program */
    return;
  }
  /* BASIC program completed */

  if (My->ExternalInputFile != NULL)
  {
    /* for automated testing, --TAPE command line parameter */
    if (bwb_is_eof (My->ExternalInputFile) == FALSE)
    {
      /* --TAPE command line parameter is active */
      bwb_interact ();                /* get user interaction */
      return;
    }
  }
  /* TAPE command inactive or completed */

  if (My->IsCommandLineFile)
  {
    /* BASIC program was started from command line */
    bwx_terminate ();
    return;
  }
  /* BASIC program was not started from command line */

  if (My->IsInteractive)
  {
    /* interactive */
    bwb_interact ();                /* get user interaction */
    return;
  }
  /* non-interactive */

  bwx_terminate ();
}

/***************************************************************
  
        FUNCTION:       bwb_execline()
  
        DESCRIPTION:    This function executes a single line of
                        a program in memory. This function is
         called by bwb_mainloop().
  
***************************************************************/

extern int
bwx_Error (int ERR, char *ErrorMessage)
{
  /*
     ERR is the error number
     ErrorMessage is used to override the default error message, and is usually NULL
   */
  assert( My != NULL );
   
  switch (ERR)
  {
  case 0:
    /* 
     **
     ** Clear any existing error 
     **
     */
    My->IsErrorPending = FALSE;        /* bwx_Error, ERR == 0 */
    My->ERR = 0;                /* bwx_Error, ERR == 0 */
    My->ERL = NULL;                /* bwx_Error, ERR == 0 */
    bwb_strcpy (My->ERROR4, "");        /* bwx_Error, ERR == 0 */
    return FALSE;
  case 6:                        /* WARN_OVERFLOW */
  case 11:                        /* WARN_DIVISION_BY_ZERO */
  case 15:                        /* WARN_STRING_TOO_LONG */
    /* 
     **
     ** Behavior depends upon whether an Error handler is active.
     **
     */
    if (GetOnError () == 0)
    {
      /*
       **
       ** Error handler is NOT active.  
       ** Do NOT set ERL, ERR, and ERROR$.
       ** Continue processing.
       **
       */
      if (ErrorMessage == NULL)
      {
        /* use the default error message */
        if (ERR > 0 && ERR < NUM_WARNINGS)
        {
          ErrorMessage = ERROR4[ERR];
        }
      }
      if (ErrorMessage != NULL)
      {
        if (bwb_strlen (ErrorMessage) > 0)
        {
          fprintf (My->SYSOUT->cfp, "%s\n", ErrorMessage);
          ResetConsoleColumn ();
        }
      }
      return FALSE;                /* continue processing */
    }
    /*
     **
     ** Error handler IS active.  
     ** Fall-thru to set ERL, ERR, and ERROR$. 
     ** Abort processing.
     **
     */
  }
  if (My->IsErrorPending == FALSE)        /* no errors pending */
  {
    /* 
     **
     ** only keep the first pending error to occur 
     **
     */
    My->IsErrorPending = TRUE;        /* bwx_Error, ERR != 0 */
    My->ERR = ERR;                /* bwx_Error, ERR != 0 */
    My->ERL = NULL;                /* bwx_Error, ERR != 0 */
    bwb_strcpy (My->ERROR4, "");        /* bwx_Error, ERR != 0 */
    if (My->StackHead)
    {
      My->ERL = My->StackHead->line;
    }
    if (ErrorMessage == NULL)
    {
      /* use the default error message */
      if (ERR > 0 && ERR < NUM_WARNINGS)
      {
        ErrorMessage = ERROR4[ERR];
      }
    }
    if (ErrorMessage != NULL)
    {
      if (bwb_strlen (ErrorMessage) > MAX_ERR_LENGTH)
      {
        ErrorMessage[MAX_ERR_LENGTH] = NulChar;
      }
      bwb_strcpy (My->ERROR4, ErrorMessage);
    }
  }
  return TRUE;                        /* abort processing */
}

void
bwb_execline (void)
{
  LineType *r, *l;
  assert( My != NULL );
  assert( My->SYSOUT != NULL );
  assert( My->SYSOUT->cfp != NULL );
  assert( My->CurrentVersion != NULL );

  if (My->StackHead == NULL)        /* in bwb_execline(), FATAL ERROR PENDING */
  {
    return;
  }

  l = My->StackHead->line;

  /* if the line is My->EndMarker, then break out of EXEC loops */
  if (l == NULL || l == My->EndMarker || My->ERR < 0)        /* in bwb_execline(), FATAL ERROR PENDING */
  {
    bwb_clrexec ();
    return;
  }

  My->ThisLine = l;                /* bwb_execline */

  /* Print line number if trace is on */
  if (My->IsTraceOn == TRUE)
  {
    if (l->LineFlags & (LINE_USER))
    {
      /* USER line in console */
    }
    else if (l->number > 0)
    {
      fprintf (My->SYSOUT->cfp, "[ %d %s ]", l->number, l->buffer);
    }
  }
  l->position = l->Startpos;

  /* if there is a BASIC command in the line, execute it here */
  if (l->cmdnum)
  {
    /* OK */
  }
  else
  {
    WARN_ILLEGAL_DIRECT;
    l->cmdnum = C_REM;
  }
  /* l->cmdnum != 0 */

  if (l->LineFlags & LINE_BREAK)
  {
    /* BREAK line */
    l->LineFlags &= ~LINE_BREAK;
    My->ContinueLine = l;
    bwx_STOP (TRUE);
    return;
  }

  /* advance beyond whitespace */
  line_skip_spaces (l);                /* keep this */

  /* execute the command vector */
  if (My->CurrentVersion->OptionFlags & (OPTION_COVERAGE_ON))
  {
    /* We do this here so "END" and "STOP" are marked */
    if (l->cmdnum == C_DATA)
    {
      /* DATA lines are marked when they are READ */
    }
    else
    {
      /* this line was executed */
      l->LineFlags |= LINE_EXECUTED;
    }
  }
  r = bwb_vector (l);
  if (r == NULL)
  {
    WARN_INTERNAL_ERROR;
    return;
  }
  assert (r != NULL);

  if (My->ERR < 0)                /* in bwb_execline(), FATAL ERROR PENDING */
  {
    /* FATAL */
    bwb_clrexec ();
    return;
  }

  if (My->IsErrorPending /* Keep This */ )
  {
    /* we are probably not at the end-of-the-line */
  }
  else if (r == l)
  {
    /* we should be at the end-of-the-line */
    if (line_is_eol (l))
    {
      /* OK */
    }
    else
    {
      WARN_SYNTAX_ERROR;
      return;
    }
  }
  else
  {
    /* we are probably not at the end-of-the-line */
  }

  if (My->IsErrorPending /* Keep This */ )
  {
    /* 
     **
     ** a NON-FATAL ERROR has occurred. ERR, ERL, and ERROR$ were
     ** already set using bwb_warning(ERR,ERROR$) 
     **
     */
    int err_gotol;
    My->IsErrorPending = FALSE;        /* Error Recognized */
    err_gotol = GetOnError ();
    if (l->LineFlags & (LINE_USER))
    {
      /*
       **
       ** -------------------------------------------------------------------------
       ** USER line in console
       ** -------------------------------------------------------------------------
       **
       ** fall thru to the DEFAULT ERROR HANDLER 
       **
       */
    }
    else if (l->number == 0)
    {
      /* fall thru to the DEFAULT ERROR HANDLER */
    }
    else if (My->ERL == NULL)
    {
      /* fall thru to the DEFAULT ERROR HANDLER */
    }
    else if (My->ERL->number == 0)
    {
      /* fall thru to the DEFAULT ERROR HANDLER */
    }
    else if (err_gotol == -1)
    {
      /*
       **
       ** -------------------------------------------------------------------------
       ** ON ERROR RESUME NEXT
       ** -------------------------------------------------------------------------
       **
       */
      assert (r != NULL);
      assert (r->next != NULL);

      r->next->position = 0;
      assert (My->StackHead != NULL);
      My->StackHead->line = r->next;
      return;
    }
    else if (err_gotol == 0)
    {
      /*
       **
       ** -------------------------------------------------------------------------
       ** ON ERROR GOTO 0
       ** -------------------------------------------------------------------------
       **
       ** fall thru to the DEFAULT ERROR HANDLER
       **
       */
    }
    else if (err_gotol == My->ERL->number)
    {
      /*
       **
       ** -------------------------------------------------------------------------
       ** RECURSION
       ** -------------------------------------------------------------------------
       **
       ** For example: 
       ** 10 ON ERROR GOTO 20
       ** 20 ERROR 1
       **
       ** fall thru to the DEFAULT ERROR HANDLER
       **
       */
    }
    else
    {
      /* USER ERROR HANDLER SPECIFIED */
      /* find the user-specified error handler and jump there */
      LineType *x;
      for (x = My->StartMarker->next; x != My->EndMarker; x = x->next)
      {
        if (x->number == err_gotol)
        {
          /* FOUND */
          if (My->CurrentVersion->OptionFlags & (OPTION_ERROR_GOSUB))
          {
            /* 
             **
             ** -------------------------------------------------------------------------
             ** OPTION ERROR GOSUB
             ** -------------------------------------------------------------------------
             **
             ** RETURN should act like RESUME NEXT...
             ** Execution resumes at the statement immediately following the one which caused the error. 
             ** For structured commands, this is the bottom line of the structure.
             **
             */
            switch (l->cmdnum)
            {
            case C_IF8THEN:
              /* skip to END_IF */
              assert (l->OtherLine != NULL);
              for (l = l->OtherLine; l->cmdnum != C_END_IF; l = l->OtherLine);
              break;
            case C_SELECT_CASE:
              /* skip to END_SELECT */
              assert (l->OtherLine != NULL);
              for (l = l->OtherLine; l->cmdnum != C_END_SELECT;
                   l = l->OtherLine);
              break;
            default:
              l = l->next;
            }
            l->position = 0;
            My->StackHead->line = l;
            if (bwb_incexec ())
            {
              x->position = 0;
              assert (My->StackHead != NULL);
              My->StackHead->line = x;
              My->StackHead->ExecCode = EXEC_GOSUB;
            }
            else
            {
              /* ERROR -- OUT OF MEMORY */
              assert (My->StackHead != NULL);
              My->StackHead->line = My->EndMarker;
            }
          }
          else
          {
            /*
             **
             ** -------------------------------------------------------------------------
             ** OPTION ERROR GOTO
             ** -------------------------------------------------------------------------
             **
             */
            x->position = 0;        /* start of line */
            assert (My->StackHead != NULL);
            My->StackHead->line = x;
          }
          return;
        }
      }
      /* NOT FOUND */
      /* fall thru to the DEFAULT ERROR HANDLER */
    }
    /*
     **
     ** -------------------------------------------------------------------------
     **                           DEFAULT ERROR HANDLER (FATAL)
     ** -------------------------------------------------------------------------
     **
     */
    /*
     **
     ** display error message
     **
     */
    if (l->LineFlags & (LINE_USER) || l->number <= 0)
    {
      /* USER line in console */
      fprintf (My->SYSOUT->cfp, "\nERROR: %s\n", My->ERROR4);
      ResetConsoleColumn ();
    }
    else
    {
      /* BASIC program line */
      fprintf (My->SYSOUT->cfp, "\nERROR in line %d: %s\n", l->number,
               My->ERROR4);
      ResetConsoleColumn ();
      /*
       **
       ** display stack trace
       **
       */
      if (My->CurrentVersion->OptionFlags & (OPTION_TRACE_ON))
      {
        /* 
         ** Dump the BASIC stack trace when a FATAL error occurs.
         ** First line is the TOP of the stack.
         ** Last line is the BOTTOM of the stack.
         */
        StackType *StackItem;
        fprintf (My->SYSOUT->cfp, "\nSTACK TRACE:\n");
        for (StackItem = My->StackHead; StackItem != NULL;
             StackItem = StackItem->next)
        {
          LineType *l;

          l = StackItem->line;
          if (l != NULL)
          {
            if (MINLIN <= l->number && l->number <= MAXLIN)
            {
              /* BASIC program line */
              if (l->buffer == NULL)
              {
                fprintf (My->SYSOUT->cfp, "%d\n", l->number);
              }
              else
              {
                fprintf (My->SYSOUT->cfp, "%d:%s\n", l->number, l->buffer);
              }
            }
          }
        }
        ResetConsoleColumn ();
      }
    }

    My->AutomaticLineNumber = 0;
    My->AutomaticLineIncrement = 0;

    if (My->IsInteractive)
    {
      /* INTERACTIVE: terminate program */

      /* reset all stack counters */
      bwb_clrexec ();
      SetOnError (0);

      My->ERR = -1;                /* in bwb_execline(), default error handler */


      /* reset the break handler */
      signal (SIGINT, break_mes);


      return;
    }
    /* NOT INTERACTIVE:  terminate immediately */
    bwx_terminate ();
    return;                        /* never reached */
  }
  if (l->number > 0)
  {
    /* These events only occur in running programs */
    if (My->IsTimerOn)
    {
      /* TIMER ON */
      if (My->OnTimerCount > 0)
      {
        if (bwx_TIMER (0) > My->OnTimerExpires)
        {
          My->IsTimerOn = FALSE;
          if (My->OnTimerLineNumber > 0)
          {
            /* ON TIMER( My->OnTimerCount ) GOSUB My->OnTimerLineNumber */
            LineType *x;

            for (x = My->StartMarker->next; x != My->EndMarker; x = x->next)
            {
              if (x->number == My->OnTimerLineNumber)
              {
                /* FOUND */
                /* save current stack level */
                assert (My->StackHead != NULL);
                My->StackHead->line = r;
                /* increment exec stack */
                if (bwb_incexec ())
                {
                  /* set the new position to x and return x */
                  x->position = 0;
                  assert (My->StackHead != NULL);
                  My->StackHead->line = x;
                  My->StackHead->ExecCode = EXEC_GOSUB;
                }
                else
                {
                  /* ERROR */
                  assert (My->StackHead != NULL);
                  My->StackHead->line = My->EndMarker;
                }
                return;
              }
            }
            /* NOT FOUND */
          }
        }
      }
    }
  }
  /* check for end of line: if so, advance to next line and return */
  if (r == l)
  {
    /* advance to the next line */
    l->next->position = 0;
    r = l->next;
  }
  else if (line_is_eol (r))
  {
    /* we could be at the end-of-the-line, added for RETURN */
    /* advance to the next line */
    r->next->position = 0;
    r = r->next;
  }
  /* else reset with the value in r */
  assert (My->StackHead != NULL);
  My->StackHead->line = r;
}

/***************************************************************
  
   FUNCTION:       is_ln()
  
   DESCRIPTION:    This function determines whether a program
         line (in buffer) begins with a line number.
  
***************************************************************/

static int
is_ln (char *buffer)
{
  int position;
   
  assert (buffer != NULL);

  position = 0;
  buff_skip_spaces (buffer, &position);        /* keep this */
  if (bwb_isdigit (buffer[position]))
  {
    return TRUE;
  }
  return FALSE;
}

/***************************************************************
  
        FUNCTION:       is_numconst()
  
        DESCRIPTION:    This function reads the string in <buffer>
                        and returns TRUE if it is a numerical
                        constant and FALSE if it is not. At
                        this point, only decimal (base 10)
                        constants are detected.
  
***************************************************************/

static int
is_numconst (char *buffer)
{
  char *p;
   
  assert (buffer != NULL);

  /* Return FALSE for empty buffer */

  if (buffer[0] == NulChar)
  {
    return FALSE;
  }
  /* else check digits */

  p = buffer;
  while (*p != NulChar)
  {
    switch (*p)
    {
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      break;
    default:
      return FALSE;
    }
    p++;
  }

  /* only numerical characters detected */

  return TRUE;

}

/* SWITCH */
LineType *
bwb_vector( LineType *l )
{
   LineType *r;
   assert (l != NULL);
   switch( l->cmdnum )
   {
   case C_DEF8LBL:
      r = bwb_DEF8LBL( l );
      break;
   case C_APPEND:
      r = bwb_APPEND( l );
      break;
   case C_AS:
      r = bwb_AS( l );
      break;
   case C_AUTO:
      r = bwb_AUTO( l );
      break;
   case C_BACKSPACE:
      r = bwb_BACKSPACE( l );
      break;
   case C_BREAK:
      r = bwb_BREAK( l );
      break;
   case C_BUILD:
      r = bwb_BUILD( l );
      break;
   case C_BYE:
      r = bwb_BYE( l );
      break;
   case C_CALL:
      r = bwb_CALL( l );
      break;
   case C_CASE:
      r = bwb_CASE( l );
      break;
   case C_CASE_ELSE:
      r = bwb_CASE_ELSE( l );
      break;
   case C_CHAIN:
      r = bwb_CHAIN( l );
      break;
   case C_CHANGE:
      r = bwb_CHANGE( l );
      break;
   case C_CLEAR:
      r = bwb_CLEAR( l );
      break;
   case C_CLOAD:
      r = bwb_CLOAD( l );
      break;
   case C_CLOAD8:
      r = bwb_CLOAD8( l );
      break;
   case C_CLOSE:
      r = bwb_CLOSE( l );
      break;
   case C_CLR:
      r = bwb_CLR( l );
      break;
   case C_CMDS:
      r = bwb_CMDS( l );
      break;
   case C_COMMON:
      r = bwb_COMMON( l );
      break;
   case C_CONSOLE:
      r = bwb_CONSOLE( l );
      break;
   case C_CONST:
      r = bwb_CONST( l );
      break;
   case C_CONT:
      r = bwb_CONT( l );
      break;
   case C_CONTINUE:
      r = bwb_CONTINUE( l );
      break;
   case C_COPY:
      r = bwb_COPY( l );
      break;
   case C_CREATE:
      r = bwb_CREATE( l );
      break;
   case C_CSAVE:
      r = bwb_CSAVE( l );
      break;
   case C_CSAVE8:
      r = bwb_CSAVE8( l );
      break;
   case C_DATA:
      r = bwb_DATA( l );
      break;
   case C_DEC:
      r = bwb_DEC( l );
      break;
   case C_DEF:
      r = bwb_DEF( l );
      break;
   case C_DEFBYT:
      r = bwb_DEFBYT( l );
      break;
   case C_DEFCUR:
      r = bwb_DEFCUR( l );
      break;
   case C_DEFDBL:
      r = bwb_DEFDBL( l );
      break;
   case C_DEFINT:
      r = bwb_DEFINT( l );
      break;
   case C_DEFLNG:
      r = bwb_DEFLNG( l );
      break;
   case C_DEFSNG:
      r = bwb_DEFSNG( l );
      break;
   case C_DEFSTR:
      r = bwb_DEFSTR( l );
      break;
   case C_DELETE:
      r = bwb_DELETE( l );
      break;
   case C_DELIMIT:
      r = bwb_DELIMIT( l );
      break;
   case C_DIM:
      r = bwb_DIM( l );
      break;
   case C_DISPLAY:
      r = bwb_DISPLAY( l );
      break;
   case C_DO:
      r = bwb_DO( l );
      break;
   case C_DOS:
      r = bwb_DOS( l );
      break;
   case C_DSP:
      r = bwb_DSP( l );
      break;
   case C_EDIT:
      r = bwb_EDIT( l );
      break;
   case C_ELSE:
      r = bwb_ELSE( l );
      break;
   case C_ELSEIF:
      r = bwb_ELSEIF( l );
      break;
   case C_END:
      r = bwb_END( l );
      break;
   case C_END_FUNCTION:
      r = bwb_END_FUNCTION( l );
      break;
   case C_END_IF:
      r = bwb_END_IF( l );
      break;
   case C_END_SELECT:
      r = bwb_END_SELECT( l );
      break;
   case C_END_SUB:
      r = bwb_END_SUB( l );
      break;
   case C_ERASE:
      r = bwb_ERASE( l );
      break;
   case C_EXCHANGE:
      r = bwb_EXCHANGE( l );
      break;
   case C_EXIT:
      r = bwb_EXIT( l );
      break;
   case C_EXIT_DO:
      r = bwb_EXIT_DO( l );
      break;
   case C_EXIT_FOR:
      r = bwb_EXIT_FOR( l );
      break;
   case C_EXIT_FUNCTION:
      r = bwb_EXIT_FUNCTION( l );
      break;
   case C_EXIT_REPEAT:
      r = bwb_EXIT_REPEAT( l );
      break;
   case C_EXIT_SUB:
      r = bwb_EXIT_SUB( l );
      break;
   case C_EXIT_WHILE:
      r = bwb_EXIT_WHILE( l );
      break;
   case C_FEND:
      r = bwb_FEND( l );
      break;
   case C_FIELD:
      r = bwb_FIELD( l );
      break;
   case C_FILE:
      r = bwb_FILE( l );
      break;
   case C_FILES:
      r = bwb_FILES( l );
      break;
   case C_FLEX:
      r = bwb_FLEX( l );
      break;
   case C_FNCS:
      r = bwb_FNCS( l );
      break;
   case C_FNEND:
      r = bwb_FNEND( l );
      break;
   case C_FOR:
      r = bwb_FOR( l );
      break;
   case C_FUNCTION:
      r = bwb_FUNCTION( l );
      break;
   case C_GET:
      r = bwb_GET( l );
      break;
   case C_GO:
      r = bwb_GO( l );
      break;
   case C_GO_SUB:
      r = bwb_GO_SUB( l );
      break;
   case C_GO_TO:
      r = bwb_GO_TO( l );
      break;
   case C_GOODBYE:
      r = bwb_GOODBYE( l );
      break;
   case C_GOSUB:
      r = bwb_GOSUB( l );
      break;
   case C_GOTO:
      r = bwb_GOTO( l );
      break;
   case C_HELP:
      r = bwb_HELP( l );
      break;
   case C_IF:
      r = bwb_IF( l );
      break;
   case C_IF_END:
      r = bwb_IF_END( l );
      break;
   case C_IF_MORE:
      r = bwb_IF_MORE( l );
      break;
   case C_IF8THEN:
      r = bwb_IF8THEN( l );
      break;
   case C_IMAGE:
      r = bwb_IMAGE( l );
      break;
   case C_INC:
      r = bwb_INC( l );
      break;
   case C_INPUT:
      r = bwb_INPUT( l );
      break;
   case C_INPUT_LINE:
      r = bwb_INPUT_LINE( l );
      break;
   case C_LET:
      r = bwb_LET( l );
      break;
   case C_LINE:
      r = bwb_LINE( l );
      break;
   case C_LINE_INPUT:
      r = bwb_LINE_INPUT( l );
      break;
   case C_LIST:
      r = bwb_LIST( l );
      break;
   case C_LISTNH:
      r = bwb_LISTNH( l );
      break;
   case C_LLIST:
      r = bwb_LLIST( l );
      break;
   case C_LOAD:
      r = bwb_LOAD( l );
      break;
   case C_LOCAL:
      r = bwb_LOCAL( l );
      break;
   case C_LOOP:
      r = bwb_LOOP( l );
      break;
   case C_LPRINT:
      r = bwb_LPRINT( l );
      break;
   case C_LPRINTER:
      r = bwb_LPRINTER( l );
      break;
   case C_LPT:
      r = bwb_LPT( l );
      break;
   case C_LSET:
      r = bwb_LSET( l );
      break;
   case C_MAINTAINER:
      r = bwb_MAINTAINER( l );
      break;
   case C_MAINTAINER_CMDS:
      r = bwb_MAINTAINER_CMDS( l );
      break;
   case C_MAINTAINER_CMDS_HTML:
      r = bwb_MAINTAINER_CMDS_HTML( l );
      break;
   case C_MAINTAINER_CMDS_ID:
      r = bwb_MAINTAINER_CMDS_ID( l );
      break;
   case C_MAINTAINER_CMDS_MANUAL:
      r = bwb_MAINTAINER_CMDS_MANUAL( l );
      break;
   case C_MAINTAINER_CMDS_SWITCH:
      r = bwb_MAINTAINER_CMDS_SWITCH( l );
      break;
   case C_MAINTAINER_CMDS_TABLE:
      r = bwb_MAINTAINER_CMDS_TABLE( l );
      break;
   case C_MAINTAINER_DEBUG:
      r = bwb_MAINTAINER_DEBUG( l );
      break;
   case C_MAINTAINER_DEBUG_OFF:
      r = bwb_MAINTAINER_DEBUG_OFF( l );
      break;
   case C_MAINTAINER_DEBUG_ON:
      r = bwb_MAINTAINER_DEBUG_ON( l );
      break;
   case C_MAINTAINER_FNCS:
      r = bwb_MAINTAINER_FNCS( l );
      break;
   case C_MAINTAINER_FNCS_HTML:
      r = bwb_MAINTAINER_FNCS_HTML( l );
      break;
   case C_MAINTAINER_FNCS_ID:
      r = bwb_MAINTAINER_FNCS_ID( l );
      break;
   case C_MAINTAINER_FNCS_MANUAL:
      r = bwb_MAINTAINER_FNCS_MANUAL( l );
      break;
   case C_MAINTAINER_FNCS_SWITCH:
      r = bwb_MAINTAINER_FNCS_SWITCH( l );
      break;
   case C_MAINTAINER_FNCS_TABLE:
      r = bwb_MAINTAINER_FNCS_TABLE( l );
      break;
   case C_MAINTAINER_MANUAL:
      r = bwb_MAINTAINER_MANUAL( l );
      break;
   case C_MAINTAINER_STACK:
      r = bwb_MAINTAINER_STACK( l );
      break;
   case C_MARGIN:
      r = bwb_MARGIN( l );
      break;
   case C_MAT:
      r = bwb_MAT( l );
      break;
   case C_MAT_GET:
      r = bwb_MAT_GET( l );
      break;
   case C_MAT_INPUT:
      r = bwb_MAT_INPUT( l );
      break;
   case C_MAT_PRINT:
      r = bwb_MAT_PRINT( l );
      break;
   case C_MAT_PUT:
      r = bwb_MAT_PUT( l );
      break;
   case C_MAT_READ:
      r = bwb_MAT_READ( l );
      break;
   case C_MAT_WRITE:
      r = bwb_MAT_WRITE( l );
      break;
   case C_MERGE:
      r = bwb_MERGE( l );
      break;
   case C_MID4:
      r = bwb_MID4( l );
      break;
   case C_MON:
      r = bwb_MON( l );
      break;
   case C_NAME:
      r = bwb_NAME( l );
      break;
   case C_NEW:
      r = bwb_NEW( l );
      break;
   case C_NEXT:
      r = bwb_NEXT( l );
      break;
   case C_OF:
      r = bwb_OF( l );
      break;
   case C_OLD:
      r = bwb_OLD( l );
      break;
   case C_ON:
      r = bwb_ON( l );
      break;
   case C_ON_ERROR:
      r = bwb_ON_ERROR( l );
      break;
   case C_ON_ERROR_GOSUB:
      r = bwb_ON_ERROR_GOSUB( l );
      break;
   case C_ON_ERROR_GOTO:
      r = bwb_ON_ERROR_GOTO( l );
      break;
   case C_ON_ERROR_RESUME:
      r = bwb_ON_ERROR_RESUME( l );
      break;
   case C_ON_ERROR_RESUME_NEXT:
      r = bwb_ON_ERROR_RESUME_NEXT( l );
      break;
   case C_ON_ERROR_RETURN:
      r = bwb_ON_ERROR_RETURN( l );
      break;
   case C_ON_ERROR_RETURN_NEXT:
      r = bwb_ON_ERROR_RETURN_NEXT( l );
      break;
   case C_ON_TIMER:
      r = bwb_ON_TIMER( l );
      break;
   case C_OPEN:
      r = bwb_OPEN( l );
      break;
   case C_OPTION:
      r = bwb_OPTION( l );
      break;
   case C_OPTION_ANGLE:
      r = bwb_OPTION_ANGLE( l );
      break;
   case C_OPTION_ANGLE_DEGREES:
      r = bwb_OPTION_ANGLE_DEGREES( l );
      break;
   case C_OPTION_ANGLE_GRADIANS:
      r = bwb_OPTION_ANGLE_GRADIANS( l );
      break;
   case C_OPTION_ANGLE_RADIANS:
      r = bwb_OPTION_ANGLE_RADIANS( l );
      break;
   case C_OPTION_ARITHMETIC:
      r = bwb_OPTION_ARITHMETIC( l );
      break;
   case C_OPTION_ARITHMETIC_DECIMAL:
      r = bwb_OPTION_ARITHMETIC_DECIMAL( l );
      break;
   case C_OPTION_ARITHMETIC_FIXED:
      r = bwb_OPTION_ARITHMETIC_FIXED( l );
      break;
   case C_OPTION_ARITHMETIC_NATIVE:
      r = bwb_OPTION_ARITHMETIC_NATIVE( l );
      break;
   case C_OPTION_BASE:
      r = bwb_OPTION_BASE( l );
      break;
   case C_OPTION_BUGS:
      r = bwb_OPTION_BUGS( l );
      break;
   case C_OPTION_BUGS_BOOLEAN:
      r = bwb_OPTION_BUGS_BOOLEAN( l );
      break;
   case C_OPTION_BUGS_OFF:
      r = bwb_OPTION_BUGS_OFF( l );
      break;
   case C_OPTION_BUGS_ON:
      r = bwb_OPTION_BUGS_ON( l );
      break;
   case C_OPTION_COMPARE:
      r = bwb_OPTION_COMPARE( l );
      break;
   case C_OPTION_COMPARE_BINARY:
      r = bwb_OPTION_COMPARE_BINARY( l );
      break;
   case C_OPTION_COMPARE_DATABASE:
      r = bwb_OPTION_COMPARE_DATABASE( l );
      break;
   case C_OPTION_COMPARE_TEXT:
      r = bwb_OPTION_COMPARE_TEXT( l );
      break;
   case C_OPTION_COVERAGE:
      r = bwb_OPTION_COVERAGE( l );
      break;
   case C_OPTION_COVERAGE_OFF:
      r = bwb_OPTION_COVERAGE_OFF( l );
      break;
   case C_OPTION_COVERAGE_ON:
      r = bwb_OPTION_COVERAGE_ON( l );
      break;
   case C_OPTION_DATE:
      r = bwb_OPTION_DATE( l );
      break;
   case C_OPTION_DIGITS:
      r = bwb_OPTION_DIGITS( l );
      break;
   case C_OPTION_DISABLE:
      r = bwb_OPTION_DISABLE( l );
      break;
   case C_OPTION_DISABLE_COMMAND:
      r = bwb_OPTION_DISABLE_COMMAND( l );
      break;
   case C_OPTION_DISABLE_FUNCTION:
      r = bwb_OPTION_DISABLE_FUNCTION( l );
      break;
   case C_OPTION_DISABLE_OPERATOR:
      r = bwb_OPTION_DISABLE_OPERATOR( l );
      break;
   case C_OPTION_EDIT:
      r = bwb_OPTION_EDIT( l );
      break;
   case C_OPTION_ENABLE:
      r = bwb_OPTION_ENABLE( l );
      break;
   case C_OPTION_ENABLE_COMMAND:
      r = bwb_OPTION_ENABLE_COMMAND( l );
      break;
   case C_OPTION_ENABLE_FUNCTION:
      r = bwb_OPTION_ENABLE_FUNCTION( l );
      break;
   case C_OPTION_ENABLE_OPERATOR:
      r = bwb_OPTION_ENABLE_OPERATOR( l );
      break;
   case C_OPTION_ERROR:
      r = bwb_OPTION_ERROR( l );
      break;
   case C_OPTION_ERROR_GOSUB:
      r = bwb_OPTION_ERROR_GOSUB( l );
      break;
   case C_OPTION_ERROR_GOTO:
      r = bwb_OPTION_ERROR_GOTO( l );
      break;
   case C_OPTION_EXPLICIT:
      r = bwb_OPTION_EXPLICIT( l );
      break;
   case C_OPTION_EXTENSION:
      r = bwb_OPTION_EXTENSION( l );
      break;
   case C_OPTION_FILES:
      r = bwb_OPTION_FILES( l );
      break;
   case C_OPTION_IMPLICIT:
      r = bwb_OPTION_IMPLICIT( l );
      break;
   case C_OPTION_INDENT:
      r = bwb_OPTION_INDENT( l );
      break;
   case C_OPTION_LABELS:
      r = bwb_OPTION_LABELS( l );
      break;
   case C_OPTION_LABELS_OFF:
      r = bwb_OPTION_LABELS_OFF( l );
      break;
   case C_OPTION_LABELS_ON:
      r = bwb_OPTION_LABELS_ON( l );
      break;
   case C_OPTION_PROMPT:
      r = bwb_OPTION_PROMPT( l );
      break;
   case C_OPTION_PUNCT:
      r = bwb_OPTION_PUNCT( l );
      break;
   case C_OPTION_PUNCT_AT:
      r = bwb_OPTION_PUNCT_AT( l );
      break;
   case C_OPTION_PUNCT_BYTE:
      r = bwb_OPTION_PUNCT_BYTE( l );
      break;
   case C_OPTION_PUNCT_COMMENT:
      r = bwb_OPTION_PUNCT_COMMENT( l );
      break;
   case C_OPTION_PUNCT_CURRENCY:
      r = bwb_OPTION_PUNCT_CURRENCY( l );
      break;
   case C_OPTION_PUNCT_DOUBLE:
      r = bwb_OPTION_PUNCT_DOUBLE( l );
      break;
   case C_OPTION_PUNCT_FILENUM:
      r = bwb_OPTION_PUNCT_FILENUM( l );
      break;
   case C_OPTION_PUNCT_IMAGE:
      r = bwb_OPTION_PUNCT_IMAGE( l );
      break;
   case C_OPTION_PUNCT_INPUT:
      r = bwb_OPTION_PUNCT_INPUT( l );
      break;
   case C_OPTION_PUNCT_INTEGER:
      r = bwb_OPTION_PUNCT_INTEGER( l );
      break;
   case C_OPTION_PUNCT_LONG:
      r = bwb_OPTION_PUNCT_LONG( l );
      break;
   case C_OPTION_PUNCT_LPAREN:
      r = bwb_OPTION_PUNCT_LPAREN( l );
      break;
   case C_OPTION_PUNCT_PRINT:
      r = bwb_OPTION_PUNCT_PRINT( l );
      break;
   case C_OPTION_PUNCT_QUOTE:
      r = bwb_OPTION_PUNCT_QUOTE( l );
      break;
   case C_OPTION_PUNCT_RPAREN:
      r = bwb_OPTION_PUNCT_RPAREN( l );
      break;
   case C_OPTION_PUNCT_SINGLE:
      r = bwb_OPTION_PUNCT_SINGLE( l );
      break;
   case C_OPTION_PUNCT_STATEMENT:
      r = bwb_OPTION_PUNCT_STATEMENT( l );
      break;
   case C_OPTION_PUNCT_STRING:
      r = bwb_OPTION_PUNCT_STRING( l );
      break;
   case C_OPTION_RECLEN:
      r = bwb_OPTION_RECLEN( l );
      break;
   case C_OPTION_RENUM:
      r = bwb_OPTION_RENUM( l );
      break;
   case C_OPTION_ROUND:
      r = bwb_OPTION_ROUND( l );
      break;
   case C_OPTION_ROUND_BANK:
      r = bwb_OPTION_ROUND_BANK( l );
      break;
   case C_OPTION_ROUND_MATH:
      r = bwb_OPTION_ROUND_MATH( l );
      break;
   case C_OPTION_ROUND_TRUNCATE:
      r = bwb_OPTION_ROUND_TRUNCATE( l );
      break;
   case C_OPTION_SCALE:
      r = bwb_OPTION_SCALE( l );
      break;
   case C_OPTION_SLEEP:
      r = bwb_OPTION_SLEEP( l );
      break;
   case C_OPTION_STDERR:
      r = bwb_OPTION_STDERR( l );
      break;
   case C_OPTION_STDIN:
      r = bwb_OPTION_STDIN( l );
      break;
   case C_OPTION_STDOUT:
      r = bwb_OPTION_STDOUT( l );
      break;
   case C_OPTION_STRICT:
      r = bwb_OPTION_STRICT( l );
      break;
   case C_OPTION_STRICT_OFF:
      r = bwb_OPTION_STRICT_OFF( l );
      break;
   case C_OPTION_STRICT_ON:
      r = bwb_OPTION_STRICT_ON( l );
      break;
   case C_OPTION_TERMINAL:
      r = bwb_OPTION_TERMINAL( l );
      break;
   case C_OPTION_TERMINAL_ADM:
      r = bwb_OPTION_TERMINAL_ADM( l );
      break;
   case C_OPTION_TERMINAL_ANSI:
      r = bwb_OPTION_TERMINAL_ANSI( l );
      break;
   case C_OPTION_TERMINAL_NONE:
      r = bwb_OPTION_TERMINAL_NONE( l );
      break;
   case C_OPTION_TIME:
      r = bwb_OPTION_TIME( l );
      break;
   case C_OPTION_TRACE:
      r = bwb_OPTION_TRACE( l );
      break;
   case C_OPTION_TRACE_OFF:
      r = bwb_OPTION_TRACE_OFF( l );
      break;
   case C_OPTION_TRACE_ON:
      r = bwb_OPTION_TRACE_ON( l );
      break;
   case C_OPTION_USING:
      r = bwb_OPTION_USING( l );
      break;
   case C_OPTION_USING_ALL:
      r = bwb_OPTION_USING_ALL( l );
      break;
   case C_OPTION_USING_COMMA:
      r = bwb_OPTION_USING_COMMA( l );
      break;
   case C_OPTION_USING_DIGIT:
      r = bwb_OPTION_USING_DIGIT( l );
      break;
   case C_OPTION_USING_DOLLAR:
      r = bwb_OPTION_USING_DOLLAR( l );
      break;
   case C_OPTION_USING_EXRAD:
      r = bwb_OPTION_USING_EXRAD( l );
      break;
   case C_OPTION_USING_FILLER:
      r = bwb_OPTION_USING_FILLER( l );
      break;
   case C_OPTION_USING_FIRST:
      r = bwb_OPTION_USING_FIRST( l );
      break;
   case C_OPTION_USING_LENGTH:
      r = bwb_OPTION_USING_LENGTH( l );
      break;
   case C_OPTION_USING_LITERAL:
      r = bwb_OPTION_USING_LITERAL( l );
      break;
   case C_OPTION_USING_MINUS:
      r = bwb_OPTION_USING_MINUS( l );
      break;
   case C_OPTION_USING_PERIOD:
      r = bwb_OPTION_USING_PERIOD( l );
      break;
   case C_OPTION_USING_PLUS:
      r = bwb_OPTION_USING_PLUS( l );
      break;
   case C_OPTION_VERSION:
      r = bwb_OPTION_VERSION( l );
      break;
   case C_OPTION_ZONE:
      r = bwb_OPTION_ZONE( l );
      break;
   case C_PAUSE:
      r = bwb_PAUSE( l );
      break;
   case C_PDEL:
      r = bwb_PDEL( l );
      break;
   case C_POP:
      r = bwb_POP( l );
      break;
   case C_PRINT:
      r = bwb_PRINT( l );
      break;
   case C_PTP:
      r = bwb_PTP( l );
      break;
   case C_PTR:
      r = bwb_PTR( l );
      break;
   case C_PUT:
      r = bwb_PUT( l );
      break;
   case C_QUIT:
      r = bwb_QUIT( l );
      break;
   case C_READ:
      r = bwb_READ( l );
      break;
   case C_RECALL:
      r = bwb_RECALL( l );
      break;
   case C_REM:
      r = bwb_REM( l );
      break;
   case C_RENAME:
      r = bwb_RENAME( l );
      break;
   case C_RENUM:
      r = bwb_RENUM( l );
      break;
   case C_RENUMBER:
      r = bwb_RENUMBER( l );
      break;
   case C_REPEAT:
      r = bwb_REPEAT( l );
      break;
   case C_REPLACE:
      r = bwb_REPLACE( l );
      break;
   case C_RESET:
      r = bwb_RESET( l );
      break;
   case C_RESTORE:
      r = bwb_RESTORE( l );
      break;
   case C_RESUME:
      r = bwb_RESUME( l );
      break;
   case C_RETURN:
      r = bwb_RETURN( l );
      break;
   case C_RSET:
      r = bwb_RSET( l );
      break;
   case C_RUN:
      r = bwb_RUN( l );
      break;
   case C_RUNNH:
      r = bwb_RUNNH( l );
      break;
   case C_SAVE:
      r = bwb_SAVE( l );
      break;
   case C_SCRATCH:
      r = bwb_SCRATCH( l );
      break;
   case C_SELECT:
      r = bwb_SELECT( l );
      break;
   case C_SELECT_CASE:
      r = bwb_SELECT_CASE( l );
      break;
   case C_STEP:
      r = bwb_STEP( l );
      break;
   case C_STOP:
      r = bwb_STOP( l );
      break;
   case C_STORE:
      r = bwb_STORE( l );
      break;
   case C_SUB:
      r = bwb_SUB( l );
      break;
   case C_SUB_END:
      r = bwb_SUB_END( l );
      break;
   case C_SUB_EXIT:
      r = bwb_SUB_EXIT( l );
      break;
   case C_SUBEND:
      r = bwb_SUBEND( l );
      break;
   case C_SUBEXIT:
      r = bwb_SUBEXIT( l );
      break;
   case C_SWAP:
      r = bwb_SWAP( l );
      break;
   case C_SYSTEM:
      r = bwb_SYSTEM( l );
      break;
   case C_TEXT:
      r = bwb_TEXT( l );
      break;
   case C_THEN:
      r = bwb_THEN( l );
      break;
   case C_TIMER:
      r = bwb_TIMER( l );
      break;
   case C_TIMER_OFF:
      r = bwb_TIMER_OFF( l );
      break;
   case C_TIMER_ON:
      r = bwb_TIMER_ON( l );
      break;
   case C_TIMER_STOP:
      r = bwb_TIMER_STOP( l );
      break;
   case C_TLOAD:
      r = bwb_TLOAD( l );
      break;
   case C_TO:
      r = bwb_TO( l );
      break;
   case C_TRACE:
      r = bwb_TRACE( l );
      break;
   case C_TRACE_OFF:
      r = bwb_TRACE_OFF( l );
      break;
   case C_TRACE_ON:
      r = bwb_TRACE_ON( l );
      break;
   case C_TSAVE:
      r = bwb_TSAVE( l );
      break;
   case C_TTY:
      r = bwb_TTY( l );
      break;
   case C_TTY_IN:
      r = bwb_TTY_IN( l );
      break;
   case C_TTY_OUT:
      r = bwb_TTY_OUT( l );
      break;
   case C_UNTIL:
      r = bwb_UNTIL( l );
      break;
   case C_USE:
      r = bwb_USE( l );
      break;
   case C_VARS:
      r = bwb_VARS( l );
      break;
   case C_WEND:
      r = bwb_WEND( l );
      break;
   case C_WHILE:
      r = bwb_WHILE( l );
      break;
   case C_WRITE:
      r = bwb_WRITE( l );
      break;
   default:
      WARN_INTERNAL_ERROR;
      r = l;
      break;
   }
   return r;
}

/* EOF */
