/*!
 * @file cond.c
 * @brief Conditional expressions.
 *
 * Parses and evaluates boolean conditional expressions used by
 * emxdoc's \c if / \c else / \c endif preprocessing directives.
 *
 * Copyright (c) 1993-1999 Eberhard Mattes
 *
 * This file is part of emxdoc.
 *
 * emxdoc is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2, or (at your option)
 * any later version.
 *
 * emxdoc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with emxdoc; see the file COPYING.  If not, write to
 * the Free Software Foundation, 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "emxdoc.h"
#include "cond.h"

/*!
 * @brief Lexical tokens produced by cond_fetch().
 */
enum ctok
{
  CTOK_END,
  CTOK_LPAR,
  CTOK_RPAR,
  CTOK_OR,
  CTOK_AND,
  CTOK_NOT,
  CTOK_CONST
};

/*!
 * @brief A named boolean variable set by cond_set().
 */
struct cond_var
{
  struct cond_var *next;
  uchar *name;
  int value;
};

/*!
 * @brief Current parse position.
 */
static const uchar *cond_ptr;
/*!
 * @brief Current token.
 */
static enum ctok ct_token;
/*!
 * @brief Value of the current constant token.
 */
static int ct_value;
/*!
 * @brief Head of the list of conditional variables.
 */
static struct cond_var *cond_vars = NULL;


/*!
 * @brief Parse and evaluate an "or" expression.
 *
 * @return Result of the expression.
 */
static int cond_or (void);
/*!
 * @brief Fetch the next token into ct_token.
 */
static void cond_fetch (void);
/*!
 * @brief Find a variable by name.
 *
 * @param[in] name Variable name. Not NULL.
 *
 * @return Pointer to the variable, or NULL if not found.
 */
static struct cond_var *cond_find (const uchar *name);


/*!
 * @brief Evaluate a conditional expression.
 *
 * @param[in] p Expression text. Not NULL.
 *
 * @return Non-zero if the expression is true, zero otherwise.
 */
int condition (const uchar *p)
{
  int result;

  cond_ptr = p;
  cond_fetch ();
  result = cond_or ();
  if (ct_token != CTOK_END)
    fatal ("%s:%d: End of line expected after condition expression",
           input_fname, line_no);
  return result;
}


/*!
 * @brief Assign a value to a conditional variable.
 *
 * @param[in] name  Variable name. Not NULL.
 * @param[in] value New value.
 */
void cond_set (const uchar *name, int value)
{
  struct cond_var *v;

  v = cond_find (name);
  if (v == NULL)
    {
      v = xmalloc (sizeof (*v));
      v->name = xstrdup (name);
      v->next = cond_vars;
      cond_vars = v;
    }
  v->value = value;
}


/*!
 * @brief Find a variable by name.
 *
 * @param[in] name Variable name. Not NULL.
 *
 * @return Pointer to the variable, or NULL if not found.
 */
static struct cond_var *cond_find (const uchar *name)
{
  struct cond_var *v;

  for (v = cond_vars; v != NULL; v = v->next)
    if (strcmp (v->name, name) == 0)
      return v;
  return NULL;
}


/*!
 * @brief Fetch the next token into ct_token.
 */
static void cond_fetch (void)
{
  static uchar name[512];
  int len;
  struct cond_var *v;

  while (isspace (*cond_ptr))
    ++cond_ptr;
  switch (*cond_ptr)
    {
    case 0:
      ct_token = CTOK_END;
      return;
    case '!':
      ct_token = CTOK_NOT; ++cond_ptr;
      return;
    case '&':
      ct_token = CTOK_AND; ++cond_ptr;
      return;
    case '|':
      ct_token = CTOK_OR; ++cond_ptr;
      return;
    case '(':
      ct_token = CTOK_LPAR; ++cond_ptr;
      return;
    case ')':
      ct_token = CTOK_RPAR; ++cond_ptr;
      return;
    case 'f':
      if (strncmp (cond_ptr, "false", 5) == 0)
        {
          ct_token = CTOK_CONST; ct_value = FALSE;
          cond_ptr += 5;
          return;
        }
      break;
    case 'h':
      if (strncmp (cond_ptr, "html", 4) == 0)
        {
          ct_token = CTOK_CONST; ct_value = (mode == 'H');
          cond_ptr += 4;
          return;
        }
      break;
    case 'i':
      if (strncmp (cond_ptr, "ipf", 3) == 0)
        {
          ct_token = CTOK_CONST; ct_value = (mode == 'I');
          cond_ptr += 3;
          return;
        }
      break;
    case 'l':
      if (strncmp (cond_ptr, "latex", 5) == 0)
        {
          ct_token = CTOK_CONST; ct_value = (mode == 'L');
          cond_ptr += 5;
          return;
        }
      break;
    case 't':
      if (strncmp (cond_ptr, "text", 4) == 0)
        {
          ct_token = CTOK_CONST; ct_value = (mode == 'T');
          cond_ptr += 4;
          return;
        }
      else if (strncmp (cond_ptr, "true", 4) == 0)
        {
          ct_token = CTOK_CONST; ct_value = TRUE;
          cond_ptr += 4;
          return;
        }
      break;
    }
  if (isalpha (*cond_ptr))
    {
      len = 0;
      name[len] = cond_ptr[len]; ++len;
      while (isalnum (cond_ptr[len]) || cond_ptr[len] == '_')
        name[len] = cond_ptr[len], ++len;
      name[len] = 0;
      v = cond_find (name);
      if (v != NULL)
        {
          ct_token = CTOK_CONST; ct_value = v->value;
          cond_ptr += len;
          return;
        }
    }
  fatal ("%s:%d: Invalid token in condition expression",
         input_fname, line_no);
}


/*!
 * @brief Parse a primary factor.
 *
 * @return Value of the factor.
 */
static int cond_factor (void)
{
  int result;

  switch (ct_token)
    {
    case CTOK_LPAR:
      cond_fetch ();
      result = cond_or ();
      if (ct_token != CTOK_RPAR)
        fatal ("%s:%d: Missing right parenthesis in condition expression",
               input_fname, line_no);
      break;

    case CTOK_END:
      fatal ("%s:%d: Operand expected in condition expression",
             input_fname, line_no);

    case CTOK_CONST:
      result = ct_value;
      cond_fetch ();
      break;

    default:
      fatal ("%s:%d: Invalid operand in condition expression",
             input_fname, line_no);
    }
  return result;
}


/*!
 * @brief Parse a "not" expression.
 *
 * @return Result of the expression.
 */
static int cond_not (void)
{
  if (ct_token == CTOK_NOT)
    {
      cond_fetch ();
      return !cond_not ();
    }
  else
    return cond_factor ();
}


/*!
 * @brief Parse an "and" expression.
 *
 * @return Result of the expression.
 */
static int cond_and (void)
{
  int result;

  result = cond_not ();
  while (ct_token == CTOK_AND)
    {
      cond_fetch ();
      if (!cond_not ())
        result = FALSE;
    }
  return result;
}


/*!
 * @brief Parse an "or" expression.
 *
 * @return Result of the expression.
 */
static int cond_or (void)
{
  int result;

  result = cond_and ();
  while (ct_token == CTOK_OR)
    {
      cond_fetch ();
      if (cond_and ())
        result = TRUE;
    }
  return result;
}
