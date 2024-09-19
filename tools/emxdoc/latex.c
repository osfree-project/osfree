/* latex.c -- LaTeX output
   Copyright (c) 1993-2000 Eberhard Mattes

This file is part of emxdoc.

emxdoc is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

emxdoc is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with emxdoc; see the file COPYING.  If not, write to
the Free Software Foundation, 59 Temple Place - Suite 330,
Boston, MA 02111-1307, USA.  */


/* TODO: LaTeX output is work in progress */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "emxdoc.h"
#include "latex.h"

static const uchar *index_word1;
static int index_flag;

void latex_output (const uchar *p, int may_break)
{
  const uchar *q;

  if (*p == ' ' && output_x >= 60 && may_break)
    {
      write_nl ();
      ++p;
    }
  while ((q = strpbrk (p, "\\&#$%_^")) != NULL)
    {
      write_nstring (p, q - p);
      switch (*q)
        {
        case '\\':
          /* TODO: Warning if no such character in current font! */
          write_string ("\\bs/");
          break;
        case '&':
          write_string ("\\&");
          break;
        case '#':
          write_string ("\\#");
          break;
        case '$':
          write_string ("\\$");
          break;
        case '%':
          write_string ("\\%");
          break;
        case '_':
          /* TODO: Warning if no such character in current font! */
          write_string ("\\_");
          break;
        case '^':
          /* TODO: Warning if no such character in current font! */
          write_string ("\\^");
          break;
        default:
          abort ();
        }
      p = q + 1;
    }
  write_string (p);
}


static void latex_elements (enum style style, int alltt)
{
  const struct element *ep, *ep2;
  enum style style_stack[STYLE_STACK_SIZE];
  int style_sp, ignore_spaces;
  size_t len;
  uchar *p, last;

  if (alltt)
    write_line ("\\begin{alltt}");
  style_sp = 0;
  style_stack[style_sp] = style;
  ignore_spaces = FALSE;
  for (ep = elements; ep->el != EL_END; ++ep)
    switch (ep->el)
      {
      case EL_WORD:
      case EL_PUNCT:
        p = ep->wp->str;
        if (strcmp (p, ".") == 0 && ep != elements && ep[-1].el == EL_WORD)
          {
            ep2 = ep + 1;
            while (ep2->el == EL_STYLE || ep2->el == EL_ENDSTYLE)
              ++ep2;
            if (ep2->el == EL_SPACE)
              {
                len = strlen (ep[-1].wp->str);
                if (len != 0)
                  {
                    last = ep[-1].wp->str[len-1];
                    if (isupper (last) && ep2->n == 2)
                      {
                        ignore_spaces = TRUE;
                        write_string ("\\@. ");
                      }
                    else if (!isupper (last) && ep2->n == 1)
                      {
                        ignore_spaces = TRUE;
                        write_string (".\\ ");
                      }
                    else
                      write_string (".");
                    break;
                  }
              }
          }
        if (strcmp (p, "--") == 0)
          p = "---";
        if (ep->wp->special != NULL && ep->wp->special->latex != NULL)
          write_string (ep->wp->special->latex);
        else if (style_sp == 0 || style_stack[style_sp] == STYLE_NORMAL)
          format_string (p, (ep->wp->style != STYLE_NORMAL
                             ? ep->wp->style : style), FALSE);
        else
          format_string (p, style_stack[style_sp], FALSE);
        break;
      case EL_SPACE:
        if (ignore_spaces)
          ignore_spaces = FALSE;
        else
          format_spaces (ep->n, style_stack[style_sp], FALSE);
        break;
      case EL_BREAK:
        if (alltt)
          write_nl ();
        else
          write_line ("\\break");
        break;
      case EL_STYLE:
        if (style_sp + 1 >= STYLE_STACK_SIZE)
          fatal ("%s:%d: Style stack overflow", input_fname, line_no);
        style_stack[++style_sp] = ep->n;
        break;
      case EL_ENDSTYLE:
        if (style_sp == 0)
          fatal ("%s:%d: Style stack underflow", input_fname, line_no);
        --style_sp;
        break;
      default:
        abort ();
      }
  if (alltt)
    write_line ("\\end{alltt}");
}


void latex_start_hilite (void)
{
  if (hl_stack[hl_sp] & HL_TT)
    write_string ("\\texttt{");
  else if (hl_stack[hl_sp] & HL_BF)
    write_string ("\\textbf{");
  else if (hl_stack[hl_sp] & HL_SL)
    write_string ("\\textsl{");
  else if (hl_stack[hl_sp] & HL_EM)
    write_string ("\\emph{");
}


void latex_end_hilite (void)
{
  write_string ("}");
}


void latex_end_env (void)
{
  switch (env_stack[env_sp].env)
    {
    case ENV_DESCRIPTION:
    case ENV_LIST:
      write_break ();
      write_line ("\\end{description}");
      break;
    case ENV_ENUMERATE:
      write_break ();
      write_line ("\\end{enumerate}");
      break;
    case ENV_ITEMIZE:
      write_break ();
      write_line ("\\end{itemize}");
      break;
    case ENV_TYPEWRITER:
    case ENV_INDENT:
      write_break ();
      write_line ("\\end{quote}");
      break;
    default:
      abort ();
    }
}


void latex_heading1 (void)
{
  write_nl ();
}


void latex_heading2 (const uchar *s)
{
  if (para_flag)
    write_nl ();
  switch (tg_level)
    {
    case 0:
      latex_output (s, FALSE);
      write_line ("\\break");
      break;
    case 1:
      write_string ("\\section{");
      latex_output (s, FALSE);
      write_line ("}");
      break;
    case 2:
      write_string ("\\subsection{");
      latex_output (s, FALSE);
      write_line ("}");
      break;
    case 3:
      write_string ("\\subsubsection{");
      latex_output (s, FALSE);
      write_line ("}");
      break;
    default:
      fatal ("Sorry, LaTeX supports only 3 levels of headings");
    }
  write_nl ();
}


void latex_description (void)
{
  write_break ();
  write_line ("\\begin{description}");
  write_nl ();
}


void latex_enumerate (void)
{
  write_break ();
  write_line ("\\begin{enumerate}");
  write_nl ();
}


void latex_itemize (void)
{
  write_break ();
  write_line ("\\begin{itemize}");
  write_nl ();
}


void latex_indent (void)
{
  write_break ();
  write_line ("\\begin{quote}");
}


void latex_list (void)
{
  write_break ();
  write_line ("\\begin{description}");
}


void latex_verbatim_start (enum tag tag_end)
{
  write_break ();
  switch (tag_end)
    {
    case TAG_ENDHEADERS:
      write_nl ();
      write_line ("\\subsubsection*{Headers}");
      break;
    case TAG_ENDSAMPLECODE:
      write_nl ();
      write_line ("\\subsubsection*{Example}");
      break;
    case TAG_ENDEXAMPLE:
      write_line ("\\begin{quote}");
      break;
    default:
      break;
    }
  write_break ();
  write_line ("\\begin{verbatim}");
}


void latex_verbatim_end (enum tag tag_end)
{
  write_line ("\\end{verbatim}");
  switch (tag_end)
    {
    case TAG_ENDEXAMPLE:
      write_line ("\\end{quote}");
      break;
    default:
      break;
    }
}


void latex_description_item (const uchar *s)
{
  write_break ();
  write_string ("\\item[");
  make_elements (s);
  latex_elements (STYLE_NORMAL, FALSE);
  write_line ("]");
}


void latex_enumerate_item (void)
{
  write_break ();
  write_line ("\\item");
}


void latex_itemize_item (void)
{
  write_break ();
  write_line ("\\item");
}


void latex_list_item (const uchar *s)
{
  write_break ();
  write_string ("\\item[");
  make_elements (s);
  latex_elements (STYLE_NORMAL, FALSE);
  write_line ("]");
}


void latex_copy (void)
{
  if (para_flag)
    {
      write_break ();
      write_nl ();
    }
  switch (env_stack[env_sp].env)
    {
    case ENV_TYPEWRITER:
      latex_elements (STYLE_NORMAL, TRUE);
      break;
    default:
      latex_elements (STYLE_NORMAL, FALSE);
      break;
    }
}

void latex_prototype_start (void)
{
  write_break ();
  write_fmt ("\\subsubsection*{Prototype%s}", function_count == 1 ? "" : "s");
  write_nl ();
}

void latex_prototype_end (uchar *compat)
{
  latex_elements (STYLE_NORMAL, TRUE);
  if (compat[0] != 0)
    {
      write_break ();
      write_line ("\\subsubsection*{Compatibility}");
      format_string (compat, STYLE_NORMAL, TRUE);
      compat[0] = 0;
    }
  write_line ("\\subsubsection*{Description}");

}

void latex_see_also_start (void)
{
  write_break ();
  write_line ("\\subsubsection*{See also}");
}

void latex_see_also_word (const uchar *word, const uchar *s)
{
  latex_output (word, FALSE);
  if (*s != 0)
    write_string (", ");
}

void latex_libref_section (const uchar *s)
{
  write_break ();
  write_fmt ("\\subsubsection*{%s}", s);
  write_nl ();
}

void latex_sample_file (const uchar *s)
{
  latex_libref_section ("Example");
  format_string ("See ", STYLE_NORMAL, FALSE);
  format_string (s, STYLE_TTY, FALSE);
  para_flag = TRUE;
}

void latex_function_start (const struct toc *tp)
{
  write_break ();
  write_string ("\\subsection{");
  latex_output (tp->title, TRUE);
  write_line ("}");
}

void latex_function_function (const uchar *s)
{
  if (index_word1 != NULL)
    latex_index (s, 2);
}

void latex_start (void)
{
  write_line ("\\documentclass{article}");
  write_line ("\\usepackage{emxdoc}");
  write_line ("\\usepackage{alltt}");
  if (output_encoding == ENC_CP850)
    write_line ("\\usepackage[cp850]{inputenc}");
  else if (output_encoding == ENC_ISO8859_1)
    write_line ("\\usepackage[latin1]{inputenc}");
  write_line ("\\usepackage[T1]{fontenc}");
  if (index_flag)
    write_line ("\\usepackage{makeidx}");
  if (title != NULL)
    {
      write_string ("\\title{");
      format_string (title, STYLE_NORMAL, FALSE);
      write_line ("}");
      write_line ("\\author{}");
    }
  if (index_flag)
    write_line ("\\makeindex");
  write_line ("\\begin{document}");
  if (title != NULL)
    write_line ("\\maketitle");
  write_line ("\\tableofcontents");
  write_line ("\\newpage");
}


void latex_end (void)
{
  if (out)
    {
      write_nl ();
      if (index_flag)
        write_line ("\\printindex");
      write_line ("\\end{document}");
    }
}

void latex_index (const uchar *s, int level)
{
  switch (level)
    {
    case 0:
      index_flag = TRUE;
      if (out)
        {
          write_string ("\\index{");
          latex_output (s, FALSE);
          write_string ("}");
        }
      break;
    case 1:
      if (*s == 0)
        index_word1 = NULL;
      else
        {
          struct word *wp = word_add (s);
          index_word1 = wp->str;
        }
      break;
    case 2:
      if (index_word1 == NULL)
        fatal ("%s:%d: %ci2 without %ci1", input_fname, line_no,
               escape, escape);
      index_flag = TRUE;
      if (out)
        {
          write_string ("\\index{");
          latex_output (index_word1, FALSE);
          write_string ("!");
          latex_output (s, FALSE);
          write_string ("}");
        }
      break;
    default:
      abort ();
    }
}
