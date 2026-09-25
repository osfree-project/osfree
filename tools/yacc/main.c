/****************************************************************
 * Top level entry point of bison,
 * Copyright (C) 1984, 1986, 1989, 1992, 1995 Free Software Foundation, Inc.
 *
 * This file is part of Bison, the GNU Compiler Compiler.
 *
 * Bison is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * Bison is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Bison; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 ****************************************************************/

/*!
 *  @file main.c
 *  @brief Top level entry point of Bison.
 *
 *  Parses the command line, opens the input and output files, then
 *  runs the whole pipeline: read the grammar, reduce it, build the
 *  LALR tables, resolve conflicts, print the requested reports and
 *  write the generated parser. Also provides the diagnostic helpers
 *  used throughout the program.
 *
 *  @copyright Copyright (C) 1984, 1986, 1989, 1992, 1995 Free Software Foundation, Inc.
 *             Licensed under the GNU General Public License v2 or later.
 */


#include <stdio.h>
#include "system.h"
#include "machine.h"    /* for MAXSHORT */

#ifdef __LINUX__
#include <unistd.h>
#endif

extern  int lineno;         /*!< Current input line number. */
extern  int verboseflag;    /*!< Non-zero when the verbose report was requested. */
extern  char *infile;       /*!< Input grammar file name. */

/* Nonzero means failure has been detected; don't write a parser file.  */
int failure;                /*!< Non-zero when a failure was detected. */

/* The name this program was run with, for messages.  */
char *program_name;         /*!< Name of the running program. */
char program_name_buf[1024];/*!< Buffer for the resolved program path. */

/*!
 *  @brief Returns a printable form of a character code.
 *
 *  @param[in] c Character code.
 *
 *  @return Pointer to a static buffer with the printable form.
 */
char *printable_version PARAMS((int));

/*!
 *  @brief Converts an integer to a printable string.
 *
 *  @param[in] i Integer to convert.
 *
 *  @return Pointer to a static buffer with the result.
 */
char *int_to_string PARAMS((int));

/*!
 *  @brief Prints a fatal error and exits.
 *
 *  @param[in] s Message.
 */
void fatal PARAMS((char *));

/*!
 *  @brief Prints a fatal error with one argument.
 *
 *  @param[in] fmt Format string.
 *  @param[in] x1  Argument for the format.
 */
void fatals PARAMS((char *, char *));

/*!
 *  @brief Prints a warning.
 *
 *  @param[in] s Message.
 */
void warn PARAMS((char *));

/*!
 *  @brief Prints a warning with an integer argument.
 *
 *  @param[in] fmt Format string.
 *  @param[in] x1  Argument for the format.
 */
void warni PARAMS((char *, int));

/*!
 *  @brief Prints a warning with one string argument.
 *
 *  @param[in] fmt Format string.
 *  @param[in] x1  Argument for the format.
 */
void warns PARAMS((char *, char *));

/*!
 *  @brief Prints a warning with two string arguments.
 *
 *  @param[in] fmt Format string.
 *  @param[in] x1  First argument.
 *  @param[in] x2  Second argument.
 */
void warnss PARAMS((char *, char *, char *));

/*!
 *  @brief Prints a warning with three string arguments.
 *
 *  @param[in] fmt Format string.
 *  @param[in] x1  First argument.
 *  @param[in] x2  Second argument.
 *  @param[in] x3  Third argument.
 */
void warnsss PARAMS((char *, char *, char *, char *));

/*!
 *  @brief Reports that too many of an object were generated.
 *
 *  @param[in] s Description of the object kind.
 */
void toomany PARAMS((char *));

/*!
 *  @brief Reports an internal error.
 *
 *  @param[in] s Description of the error.
 */
void berror PARAMS((char *));

/*!
 *  @brief Parses the command line.
 *
 *  @param[in] argc Argument count.
 *  @param[in] argv Argument vector.
 */
extern void getargs PARAMS((int, char *[]));

/*!
 *  @brief Computes output file names and opens the files.
 */
extern void openfiles PARAMS((void));

/*!
 *  @brief Reads the input grammar.
 */
extern void reader PARAMS((void));

/*!
 *  @brief Removes useless rules and nonterminals from the grammar.
 */
extern void reduce_grammar PARAMS((void));

/*!
 *  @brief Builds the derives table.
 */
extern void set_derives PARAMS((void));

/*!
 *  @brief Computes which nonterminals can be empty.
 */
extern void set_nullable PARAMS((void));

/*!
 *  @brief Builds the LR(0) automaton.
 */
extern void generate_states PARAMS((void));

/*!
 *  @brief Builds the LALR lookahead sets.
 */
extern void lalr PARAMS((void));

/*!
 *  @brief Finds and resolves conflicts.
 */
extern void initialize_conflicts PARAMS((void));

/*!
 *  @brief Writes the verbose report.
 */
extern void verbose PARAMS((void));

/*!
 *  @brief Writes the terse report.
 */
extern void terse PARAMS((void));

/*!
 *  @brief Writes the parser output.
 */
extern void output PARAMS((void));

/*!
 *  @brief Closes files and exits the program.
 *
 *  @param[in] k Exit status.
 */
extern void done PARAMS((int));


/* VMS complained about using `int'.  */

/*!
 *  @brief Main entry point of Bison.
 *
 *  @param[in] argc Argument count.
 *  @param[in] argv Argument vector.
 *
 *  @return Exit status.
 */
int
main (int argc, char *argv[])
{
  program_name = argv[0];
#ifdef __LINUX__
  if (readlink ("/proc/self/exe", program_name_buf, 1024) != -1)
  {
    program_name=&program_name_buf;    
  }
#endif
  setlocale (LC_ALL, "");
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);

  failure = 0;
  lineno = 0;


  getargs(argc, argv);
  openfiles();


  /* read the input.  Copy some parts of it to fguard, faction, ftable and fattrs.
     In file reader.c.
     The other parts are recorded in the grammar; see gram.h.  */
  reader();
  if (failure)
        done(failure);

  /* find useless nonterminals and productions and reduce the grammar.  In
     file reduce.c */
  reduce_grammar();

  /* record other info about the grammar.  In files derives and nullable.  */
  set_derives();
  set_nullable();

  /* convert to nondeterministic finite state machine.  In file LR0.
     See state.h for more info.  */
  generate_states();

  /* make it deterministic.  In file lalr.  */
  lalr();

  /* Find and record any conflicts: places where one token of lookahead is not
     enough to disambiguate the parsing.  In file conflicts.
     Also resolve s/r conflicts based on precedence declarations.  */
  initialize_conflicts();

  /* print information about results, if requested.  In file print. */
  if (verboseflag)
    verbose();
  else
    terse();

  /* output the tables and the parser to ftable.  In file output. */
  output();
  done(failure);
  return failure;
}

/* functions to report errors which prevent a parser from being generated */


/*!
 *  @brief Returns a printable form of a character code.
 *
 *  Return a string containing a printable version of @p c: either
 *  the character itself, or the corresponding \DDD code.
 *
 *  @param[in] c Character code.
 *
 *  @return Pointer to a static buffer with the printable form.
 */
char *
printable_version (int c)
{
  static char buf[10];
  if (c < ' ' || c >= '\177')
    sprintf(buf, "\\%o", c);
  else
    {
      buf[0] = c;
      buf[1] = '\0';
    }
  return buf;
}

/*!
 *  @brief Converts an integer to a printable string.
 *
 *  Generate a string from the integer @p i. Return a ptr to internal
 *  memory containing the string.
 *
 *  @param[in] i Integer to convert.
 *
 *  @return Pointer to a static buffer with the result.
 */
char *
int_to_string (int i)
{
  static char buf[20];
  sprintf(buf, "%d", i);
  return buf;
}

/*!
 *  @brief Prints the location prefix of a fatal error message.
 */
static void
fatal_banner (void)
{
  if (infile == 0)
    fprintf(stderr, _("%s: fatal error: "), program_name);
  else
    fprintf(stderr, _("%s:%d: fatal error: "), infile, lineno);
}

/*!
 *  @brief Prints a fatal error and exits.
 *
 *  Print the message S for a fatal error.
 *
 *  @param[in] s Message.
 */
void
fatal (char *s)
{
  fatal_banner ();
  fputs (s, stderr);
  fputc ('\n', stderr);
  done (1);
}


/*!
 *  @brief Prints a fatal error with one argument.
 *
 *  Print a message for a fatal error. Use FMT to construct the
 *  message and incorporate string X1.
 *
 *  @param[in] fmt Format string.
 *  @param[in] x1  Argument for the format.
 */
void
fatals (char *fmt, char *x1)
{
  fatal_banner ();
  fprintf (stderr, fmt, x1);
  fputc ('\n', stderr);
  done (1);
}

/*!
 *  @brief Prints the location prefix of a warning message.
 */
static void
warn_banner (void)
{
  if (infile == 0)
    fprintf(stderr, _("%s: "), program_name);
  else
    fprintf(stderr, _("%s:%d: "), infile, lineno);
  failure = 1;
}

/*!
 *  @brief Prints a warning.
 *
 *  Print a warning message S.
 *
 *  @param[in] s Message.
 */
void
warn (char *s)
{
  warn_banner ();
  fputs (s, stderr);
  fputc ('\n', stderr);
}

/*!
 *  @brief Prints a warning with an integer argument.
 *
 *  Print a warning message containing the string for the integer X1.
 *  The message is given by the format FMT.
 *
 *  @param[in] fmt Format string.
 *  @param[in] x1  Integer argument.
 */
void
warni (char *fmt, int x1)
{
  warn_banner ();
  fprintf (stderr, fmt, x1);
  fputc ('\n', stderr);
}

/*!
 *  @brief Prints a warning with one string argument.
 *
 *  Print a warning message containing the string X1. The message is
 *  given by the format FMT.
 *
 *  @param[in] fmt Format string.
 *  @param[in] x1  String argument.
 */
void
warns (char *fmt, char *x1)
{
  warn_banner ();
  fprintf (stderr, fmt, x1);
  fputc ('\n', stderr);
}

/*!
 *  @brief Prints a warning with two string arguments.
 *
 *  Print a warning message containing the two strings X1 and X2. The
 *  message is given by the format FMT.
 *
 *  @param[in] fmt Format string.
 *  @param[in] x1  First string argument.
 *  @param[in] x2  Second string argument.
 */
void
warnss (char *fmt, char *x1, char *x2)
{
  warn_banner ();
  fprintf (stderr, fmt, x1, x2);
  fputc ('\n', stderr);
}

/*!
 *  @brief Prints a warning with three string arguments.
 *
 *  Print a warning message containing the 3 strings X1, X2, X3. The
 *  message is given by the format FMT.
 *
 *  @param[in] fmt Format string.
 *  @param[in] x1  First string argument.
 *  @param[in] x2  Second string argument.
 *  @param[in] x3  Third string argument.
 */
void
warnsss (char *fmt, char *x1, char *x2, char *x3)
{
  warn_banner ();
  fprintf (stderr, fmt, x1, x2, x3);
  fputc ('\n', stderr);
}

/*!
 *  @brief Reports that too many of an object were generated.
 *
 *  Print a message for the fatal occurence of more than MAXSHORT
 *  instances of whatever is denoted by the string S.
 *
 *  @param[in] s Description of the object kind.
 */
void
toomany (char *s)
{
  fatal_banner ();
  fprintf (stderr, _("too many %s (max %d)"), s, MAXSHORT);
  fputc ('\n', stderr);
  done (1);
}

/*!
 *  @brief Reports an internal error.
 *
 *  Abort for an internal error denoted by string S.
 *
 *  @param[in] s Description of the error.
 */
void
berror (char *s)
{
  fprintf(stderr, _("%s: internal error: %s\n"), program_name, s);
  abort();
}
