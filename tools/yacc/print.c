/****************************************************************
 * Print information on generated parser, for bison,
 * Copyright (C) 1984, 1986, 1989 Free Software Foundation, Inc.
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
 *  @file print.c
 *  @brief Prints information about the generated parser.
 *
 *  Implements the verbose and terse reports requested with -v: prints
 *  the conflict log, the grammar, and for each state the kernel items
 *  and the available actions.
 *
 *  @copyright Copyright (C) 1984, 1986, 1989 Free Software Foundation, Inc.
 *             Licensed under the GNU General Public License v2 or later.
 */


#include <stdio.h>
#include "system.h"
#include "machine.h"
#include "alloc.h"
#include "files.h"
#include "gram.h"
#include "state.h"


extern char **tags;             /*!< Printable names of all symbols. */
extern int nstates;             /*!< Number of parser states. */
extern short *accessing_symbol; /*!< Accessing symbol of each state. */
extern core **state_table;      /*!< State structures indexed by number. */
extern shifts **shift_table;    /*!< Shift structures indexed by number. */
extern errs **err_table;        /*!< Explicit error tokens per state. */
extern reductions **reduction_table; /*!< Reduction structures indexed by number. */
extern char *consistent;        /*!< Non-zero for states requiring no lookahead. */
extern char any_conflicts;      /*!< Non-zero if any state has a conflict. */
extern char *conflicts;         /*!< Per-state conflict flag. */
extern int final_state;         /*!< Number of the termination state. */

/*!
 *  @brief Writes the terse conflict log.
 */
extern void conflict_log PARAMS((void));

/*!
 *  @brief Writes the verbose conflict log.
 */
extern void verbose_conflict_log PARAMS((void));

/*!
 *  @brief Prints the reductions available in a state.
 *
 *  @param[in] state State number.
 */
extern void print_reductions PARAMS((int));

/*!
 *  @brief Writes the terse report about the parser.
 */
void terse PARAMS((void));

/*!
 *  @brief Writes the verbose report about the parser.
 */
void verbose PARAMS((void));

/*!
 *  @brief Prints a token with its external number.
 *
 *  @param[in] extnum External token number.
 *  @param[in] token  Internal token number.
 */
void print_token PARAMS((int, int));

/*!
 *  @brief Prints the contents of a parser state.
 *
 *  @param[in] state State number.
 */
void print_state PARAMS((int));

/*!
 *  @brief Prints the kernel items of a parser state.
 *
 *  @param[in] state State number.
 */
void print_core PARAMS((int));

/*!
 *  @brief Prints the actions available in a parser state.
 *
 *  @param[in] state State number.
 */
void print_actions PARAMS((int));

/*!
 *  @brief Prints the grammar as a whole.
 */
void print_grammar PARAMS((void));

/*!
 *  @brief Writes the terse report about the parser.
 */
void
terse (void)
{
  if (any_conflicts)
    {
      conflict_log();
    }
}


/*!
 *  @brief Writes the verbose report about the parser.
 */
void
verbose (void)
{
  register int i;

  if (any_conflicts)
    verbose_conflict_log();

  print_grammar();

  for (i = 0; i < nstates; i++)
    {
      print_state(i);
    }
}


/*!
 *  @brief Prints a token with its external number.
 *
 *  @param[in] extnum External token number.
 *  @param[in] token  Internal token number.
 */
void
print_token (int extnum, int token)
{
  fprintf(foutput, _(" type %d is %s\n"), extnum, tags[token]);
}


/*!
 *  @brief Prints the contents of a parser state.
 *
 *  @param[in] state State number.
 */
void
print_state (int state)
{
  fprintf(foutput, _("\n\nstate %d\n\n"), state);
  print_core(state);
  print_actions(state);
}


/*!
 *  @brief Prints the kernel items of a parser state.
 *
 *  @param[in] state State number.
 */
void
print_core (int state)
{
  register int i;
  register int k;
  register int rule;
  register core *statep;
  register short *sp;
  register short *sp1;

  statep = state_table[state];
  k = statep->nitems;

  if (k == 0) return;

  for (i = 0; i < k; i++)
    {
      sp1 = sp = ritem + statep->items[i];

      while (*sp > 0)
	sp++;

      rule = -(*sp);
      fprintf(foutput, "    %s  ->  ", tags[rlhs[rule]]);

      for (sp = ritem + rrhs[rule]; sp < sp1; sp++)
	{
	  fprintf(foutput, "%s ", tags[*sp]);
	}

      putc('.', foutput);

      while (*sp > 0)
	{
	  fprintf(foutput, " %s", tags[*sp]);
	  sp++;
	}

      fprintf (foutput, _("   (rule %d)"), rule);
      putc('\n', foutput);
    }

  putc('\n', foutput);
}


/*!
 *  @brief Prints the actions available in a parser state.
 *
 *  @param[in] state State number.
 */
void
print_actions (int state)
{
  register int i;
  register int k;
  register int state1;
  register int symbol;
  register shifts *shiftp;
  register errs *errp;
  register reductions *redp;
  register int rule;

  shiftp = shift_table[state];
  redp = reduction_table[state];
  errp = err_table[state];

  if (!shiftp && !redp)
    {
      if (final_state == state)
	fprintf(foutput, _("    $default\taccept\n"));
      else
	fprintf(foutput, _("    NO ACTIONS\n"));
      return;
    }

  if (shiftp)
    {
      k = shiftp->nshifts;

      for (i = 0; i < k; i++)
	{
	  if (! shiftp->shifts[i]) continue;
	  state1 = shiftp->shifts[i];
	  symbol = accessing_symbol[state1];
	  /* The following line used to be turned off.  */
	  if (ISVAR(symbol)) break;
          if (symbol==0)      /* I.e. strcmp(tags[symbol],"$")==0 */
            fprintf(foutput, _("    $   \tgo to state %d\n"), state1);
          else
            fprintf(foutput, _("    %-4s\tshift, and go to state %d\n"),
                    tags[symbol], state1);
	}

      if (i > 0)
	putc('\n', foutput);
    }
  else
    {
      i = 0;
      k = 0;
    }

  if (errp)
    {
      int j, nerrs;

      nerrs = errp->nerrs;

      for (j = 0; j < nerrs; j++)
	{
	  if (! errp->errs[j]) continue;
	  symbol = errp->errs[j];
	  fprintf(foutput, _("    %-4s\terror (nonassociative)\n"), tags[symbol]);
	}

      if (j > 0)
	putc('\n', foutput);
    }

  if (consistent[state] && redp)
    {
      rule = redp->rules[0];
      symbol = rlhs[rule];
      fprintf(foutput, _("    $default\treduce using rule %d (%s)\n\n"),
     	        rule, tags[symbol]);
    }
  else if (redp)
    {
      print_reductions(state);
    }

  if (i < k)
    {
      for (; i < k; i++)
	{
	  if (! shiftp->shifts[i]) continue;
	  state1 = shiftp->shifts[i];
	  symbol = accessing_symbol[state1];
	  fprintf(foutput, _("    %-4s\tgo to state %d\n"), tags[symbol], state1);
	}

      putc('\n', foutput);
    }
}

/*!
 *  @brief Ends a line of the grammar listing, wrapping if needed.
 *
 *  @param[in] end Column limit beyond which the line is wrapped.
 *  @def END_TEST
 */
#define END_TEST(end) \
  if (column + strlen(buffer) > (end))					 \
    { fprintf (foutput, "%s\n   ", buffer); column = 3; buffer[0] = 0; } \
  else

/*!
 *  @brief Prints the grammar as a whole.
 */
void
print_grammar (void)
{
  int i, j;
  short* rule;
  char buffer[90];
  int column = 0;

  /* rule # : LHS -> RHS */
  fputs(_("\nGrammar\n"), foutput);
  for (i = 1; i <= nrules; i++)
    /* Don't print rules disabled in reduce_grammar_tables.  */
    if (rlhs[i] >= 0)
      {
	fprintf(foutput, _("rule %-4d %s ->"), i, tags[rlhs[i]]);
	rule = &ritem[rrhs[i]];
	if (*rule > 0)
	  while (*rule > 0)
	    fprintf(foutput, " %s", tags[*rule++]);
	else
	  fputs (_("		/* empty */"), foutput);
	putc('\n', foutput);
      }

  /* TERMINAL (type #) : rule #s terminal is on RHS */
  fputs(_("\nTerminals, with rules where they appear\n\n"), foutput);
  fprintf(foutput, "%s (-1)\n", tags[0]);
  if (translations)
    {
      for (i = 0; i <= max_user_token_number; i++)
	if (token_translations[i] != 2)
	  {
	    buffer[0] = 0;
	    column = strlen (tags[token_translations[i]]);
	    fprintf(foutput, "%s", tags[token_translations[i]]);
	    END_TEST (50);
	    sprintf (buffer, " (%d)", i);

	    for (j = 1; j <= nrules; j++)
	      {
		for (rule = &ritem[rrhs[j]]; *rule > 0; rule++)
		  if (*rule == token_translations[i])
		    {
		      END_TEST (65);
		      sprintf (buffer + strlen(buffer), " %d", j);
		      break;
		    }
	      }
	    fprintf (foutput, "%s\n", buffer);
	  }
    }
  else
    for (i = 1; i < ntokens; i++)
      {
	buffer[0] = 0;
	column = strlen (tags[i]);
	fprintf(foutput, "%s", tags[i]);
	END_TEST (50);
	sprintf (buffer, " (%d)", i);

	for (j = 1; j <= nrules; j++)
	  {
	    for (rule = &ritem[rrhs[j]]; *rule > 0; rule++)
	      if (*rule == i)
		{
		  END_TEST (65);
		  sprintf (buffer + strlen(buffer), " %d", j);
		  break;
		}
	  }
	fprintf (foutput, "%s\n", buffer);
      }

  fputs(_("\nNonterminals, with rules where they appear\n\n"), foutput);
  for (i = ntokens; i <= nsyms - 1; i++)
    {
      int left_count = 0, right_count = 0;

      for (j = 1; j <= nrules; j++)
	{
	  if (rlhs[j] == i)
	    left_count++;
	  for (rule = &ritem[rrhs[j]]; *rule > 0; rule++)
	    if (*rule == i)
	      {
		right_count++;
		break;
	      }
	}

      buffer[0] = 0;
      fprintf(foutput, "%s", tags[i]);
      column = strlen (tags[i]);
      sprintf (buffer, " (%d)", i);
      END_TEST (0);

      if (left_count > 0)
	{
	  END_TEST (50);
	  sprintf (buffer + strlen(buffer), _(" on left:"));

	  for (j = 1; j <= nrules; j++)
	    {
	      END_TEST (65);
	      if (rlhs[j] == i)
		sprintf (buffer + strlen(buffer), " %d", j);
	    }
	}

      if (right_count > 0)
	{
	  if (left_count > 0)
	    sprintf (buffer + strlen(buffer), ",");
	  END_TEST (50);
	  sprintf (buffer + strlen(buffer), _(" on right:"));
	  for (j = 1; j <= nrules; j++)
	    {
	      for (rule = &ritem[rrhs[j]]; *rule > 0; rule++)
		if (*rule == i)
		  {
		    END_TEST (65);
		    sprintf (buffer + strlen(buffer), " %d", j);
		    break;
		  }
	    }
	}
      fprintf (foutput, "%s\n", buffer);
    }
}
