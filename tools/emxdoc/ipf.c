/* ipf.c -- IPF output
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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "emxdoc.h"
#include "ipf.h"
#include "xref.h"

#define HBAR   0xc4
#define VBAR   0xb3

static int idx_refid;


void ipf_hilite (int new)
{
  int diff, font_changed;

  if (new & HL_EM)
    new = (new & ~HL_EM) | HL_SL;
  if (hl_ipf == new)
    return;
  font_changed = FALSE;
  diff = new & ~hl_ipf;
  if (diff & HL_TT)
    {
      write_string (":font");
      write_space ();
      write_string ("facename=Courier");
      write_space ();
      write_string ("size=16x9.");
      font_changed = TRUE;
    }
  diff = hl_ipf & ~new;
  if (diff & HL_TT)
    {
      write_string (":font");
      write_space ();
      write_string ("facename=default");
      write_space ();
      write_string ("size=0x0.");
      font_changed = TRUE;
    }

  if ((hl_ipf & (HL_BF|HL_SL|HL_UL))
      != (new & (HL_BF|HL_SL|HL_UL)) || font_changed)
    {
      if (hl_ipf_no != 0)
        write_fmt (":ehp%d.", hl_ipf_no);
      if (opt_c && (new & HL_SL))
        hl_ipf_no = 4;
      else
        switch (new & (HL_BF|HL_SL|HL_UL))
          {
          case 0:
            hl_ipf_no = 0;
            break;
          case HL_SL:
            hl_ipf_no = 1;
            break;
          case HL_SL|HL_BF:
            hl_ipf_no = 3;
            break;
          case HL_SL|HL_UL:
          case HL_SL|HL_BF|HL_UL:
            hl_ipf_no = 6;
            break;
          case HL_BF:
            hl_ipf_no = 2;
            break;
          case HL_BF|HL_UL:
            hl_ipf_no = 7;
            break;
          case HL_UL:
            hl_ipf_no = 5;
            break;
          default:
            abort ();
          }
      if (hl_ipf_no != 0)
        write_fmt (":hp%d.", hl_ipf_no);
    }

  hl_ipf = new;
}


static void ipf_string (const uchar *p, int may_break)
{
  const uchar *q;

  while ((q = strpbrk (p, ":&. ")) != NULL)
    {
      write_nstring (p, q - p);
      switch (*q)
        {
        case ':':
          write_string ("&colon.");
          break;
        case '&':
          write_string ("&amp.");
          break;
        case ' ':
          if (output_x >= 60 && may_break)
            write_nl ();
          else
            write_string (" ");
          break;
        case '.':
          if (output_x == 0)
            write_string ("&per.");
          else
            write_string (".");
          break;
        default:
          abort ();
        }
      p = q + 1;
    }
  write_string (p);
}


void ipf_output (const uchar *p, int may_break)
{
  ipf_hilite (hl_stack[hl_sp]);
  ipf_string (p, may_break);
}


void ipf_para (void)
{
  write_break ();
  write_line (":p.");
}


void ipf_margin (int margin)
{
  if (out && mode == 'I')
    {
      write_fmt (":lm margin=%d.", margin);
      write_nl ();
    }
}


void ipf_env_margin (int sp)
{
  ipf_margin (env_stack[sp].imargin);
}


void ipf_elements (enum style style)
{
  const struct element *ep;
  enum style style_stack[STYLE_STACK_SIZE];
  int style_sp;

  style_sp = 0;
  style_stack[style_sp] = style;
  for (ep = elements; ep->el != EL_END; ++ep)
    switch (ep->el)
      {
      case EL_WORD:
      case EL_PUNCT:
        if (ep->n != 0)
          {
            if (ep->wp->database == NULL)
              ipf_begin_link (NULL, ep->wp->ref);
            else
              ipf_begin_link (ep->wp->database->str, ep->wp->ref);
            format_output (ep->wp->str, FALSE);
            ipf_end_link ();
          }
        else if (ep->wp->special != NULL && ep->wp->special->ipf != NULL)
          write_string (ep->wp->special->ipf);
        else if (style_sp == 0 || style_stack[style_sp] == STYLE_NORMAL)
          format_string (ep->wp->str, (ep->wp->style != STYLE_NORMAL
                                       ? ep->wp->style : style), FALSE);
        else
          format_string (ep->wp->str, style_stack[style_sp], FALSE);
        break;
      case EL_SPACE:
        format_spaces (ep->n, style_stack[style_sp], TRUE);
        break;
      case EL_BREAK:
        write_break ();
        write_line (".br");
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
}


void ipf_begin_link (const uchar *database, int ref)
{
  ipf_hilite (hl_stack[hl_sp]);
  write_string (":link");
  write_space ();
  write_string ("reftype=hd");
  write_space ();
  if (database != NULL && !opt_a)
    {
      write_space ();
      write_fmt ("refid=%d database='%s'", ref, database);
    }
  else
    write_fmt ("res=%d", ref);
  write_string (".");
}


void ipf_end_link (void)
{
  write_string (":elink.");
}


void ipf_end_env (void)
{
  switch (env_stack[env_sp].env)
    {
    case ENV_DESCRIPTION:
      write_break ();
      write_line (":edl.");
      break;
    case ENV_ENUMERATE:
      write_break ();
      write_line (":eol.");
      break;
    case ENV_ITEMIZE:
      write_break ();
      write_line (":eul.");
      break;
    case ENV_LIST:
    case ENV_INDENT:
    case ENV_TYPEWRITER:
      write_break ();
      ipf_env_margin (env_sp-1);
      break;
    default:
      abort ();
    }
}


void ipf_toc_start (void)
{
  write_line (":lines align=left.");
}


void ipf_toc_line (const uchar *s, const struct toc *tp)
{
  format_string (s, STYLE_NORMAL, FALSE);
  ipf_begin_link (NULL, tp->ref);
  format_output (tp->title, FALSE);
  ipf_end_link ();
  write_nl ();
}


void ipf_toc_end (void)
{
  write_line (":elines.");
}


void ipf_heading1 (int level, int ref, int global, unsigned flags)
{
  if (hl_sp != 0)
    fatal ("%s:%d: Hilighting must be off at section heading",
           input_fname, line_no);
  ipf_hilite (hl_stack[hl_sp]);
  write_break ();
  write_fmt (":h%d res=%d", level, ref);
  if (global && !opt_a)
    write_fmt (" id=%d global", ref);
  if (flags & HF_HIDE)
    write_string (" hide");
  write_string (".");
}


void ipf_heading2 (const uchar *s)
{
  if (tg_level == 0)
    {
      if (para_flag)
        ipf_para ();
      ipf_para ();
      start_hilite (HL_BF);
      format_output (s, FALSE);
      end_hilite ();
    }
  else
    format_output (s, FALSE);
  write_nl ();
  ipf_env_margin (0);
}


void ipf_description (void)
{
  write_fmt (":dl break=fit tsize=%d.", IPF_DESCRIPTION_INDENT);
  write_nl ();
}


void ipf_enumerate (void)
{
  write_line (":ol.");
}


void ipf_itemize (void)
{
  write_line (":ul.");
}


void ipf_verbatim_start (enum tag tag_end)
{
  switch (tag_end)
    {
    case TAG_ENDHEADERS:
      format_string ("Headers:", STYLE_BOLD, FALSE);
      break;
    case TAG_ENDSAMPLECODE:
      ipf_para ();
      format_string ("Example:", STYLE_BOLD, FALSE);
      break;
    case TAG_ENDEXAMPLE:
      ipf_margin (env_stack[env_sp].imargin + 4);
      break;
    default:
      break;
    }
  write_break ();
  ipf_hilite (hl_stack[hl_sp]); /* Don't switch font after :cgraphic */
  write_line (":cgraphic.");
}


void ipf_verbatim_line (void)
{
  format_output (input, FALSE);
  write_nl ();
}


void ipf_verbatim_end (enum tag tag_end)
{
  write_line (":ecgraphic.");
  switch (tag_end)
    {
    case TAG_ENDEXAMPLE:
      ipf_env_margin (env_sp);
      write_line (".br");
      para_flag = FALSE;
      break;
    default:
      para_flag = TRUE;
      break;
    }
}


void ipf_description_item (const uchar *s)
{
  write_break ();
  write_string (":dt.");
  make_elements (s);
  ipf_elements (STYLE_NORMAL);
  write_break ();
  write_string (":dd.");
}


void ipf_enumerate_item (void)
{
  write_break ();
  ipf_hilite (hl_stack[hl_sp]); /* Use correct font for number */
  write_string (":li.");
}


void ipf_itemize_item (void)
{
  write_break ();
  ipf_hilite (hl_stack[hl_sp]); /* Use correct font for mark */
  write_string (":li.");
}


void ipf_list_item (const uchar *s)
{
  ipf_env_margin (env_sp-1);
  ipf_para ();
  make_elements (s);
  ipf_elements (STYLE_NORMAL);
  ipf_para ();
  ipf_env_margin (env_sp);
}


void ipf_prototype_start (void)
{
  if (para_flag)
    ipf_para ();
  format_string (function_count == 1 ? "Prototype:" : "Prototypes:",
                 STYLE_BOLD, FALSE);
  write_break ();
  ipf_para ();
}


void ipf_prototype_end (uchar *compat)
{
  ipf_elements (STYLE_TTY);
  ipf_para ();
  if (compat[0] != 0)
    {
      format_string ("Compatibility:", STYLE_BOLD, FALSE);
      ipf_para ();
      format_string (compat, STYLE_NORMAL, FALSE);
      ipf_para ();
      compat[0] = 0;
    }
  format_string ("Description:", STYLE_BOLD, FALSE);
}


void ipf_start (void)
{
  write_line (":userdoc.");
  write_line (":prolog.");
  write_line (":docprof toc=123.");
  if (title != NULL)
    {
      write_string (":title.");
      format_string (title, STYLE_NORMAL, FALSE);
      write_nl ();
    }
  write_line (":eprolog.");
  write_line (":body.");
}


void ipf_table_start (int do_indent, int *widths, int wn)
{
  int wi;

  if (do_indent)
    ipf_margin (env_stack[env_sp].imargin + 4);
  write_break ();
  write_string (":table");
  if (wn != 0)
    {
      write_string (" cols='");
      for (wi = 0; wi < wn; ++wi)
        {
          if (wi != 0)
            write_string (" ");
          write_fmt ("%d", widths[wi]);
        }
      write_string ("'");
    }
  write_line (".");
}


void ipf_table_line (const uchar *s, int wn)
{
  int wi;
  uchar word[512], *d;

  if (*s != HBAR)
    {
      write_line (":row.");
      wi = 0;
      while (*s != 0)
        {
          ++wi;
          write_string (":c.");
          d = word;
          while (*s != VBAR && *s != 0)
            *d++ = *s++;
          while (d != word && isspace (d[-1]))
            --d;
          *d = 0;
          make_elements (word);
          ipf_elements (STYLE_NORMAL);
          write_break ();
          if (*s != 0)
            {
              ++s;
              while (isspace (*s))
                ++s;
            }
        }
      if (wi != wn)
        fatal ("%s:%d: Wrong number of columns", input_fname, line_no);
    }
}


void ipf_table_end (int do_indent)
{
  write_line (":etable.");
  if (do_indent)
    {
      ipf_env_margin (env_sp);
      write_line (".br");
      para_flag = FALSE;
    }
  else
    para_flag = TRUE;
}


void ipf_index (const uchar *s)
{
  struct word *wp;

  if (!out)
    return;
  switch (tg_level)
    {
    case 0:
      write_string (":i1.");
      ipf_string (s, FALSE);
      write_nl ();
      break;
    case 1:
      if (*s == 0)
        idx_refid = 0;
      else
        {
          wp = word_add (s);
          if (wp->idx == 0)
            {
              idx_refid = wp->idx = ++idx_no;
              write_fmt (":i1 id=%d.", idx_refid);
              ipf_string (s, FALSE);
              write_nl ();
            }
          else
            idx_refid = wp->idx;
        }
      break;
    case 2:
      if (idx_refid == 0)
        fatal ("%s:%d: %ci2 without %ci1", input_fname, line_no,
               escape, escape);
      write_fmt (":i2 refid=%d.", idx_refid);
      ipf_string (s, FALSE);
      write_nl ();
      break;
    default:
      abort ();
    }
}


void ipf_see_also_start (void)
{
  if (para_flag)
    ipf_para ();
  format_string ("See also: ", STYLE_BOLD, FALSE);
}


void ipf_see_also_word (const uchar *word, const uchar *s)
{
  struct word *wp;

  wp = use_reference (word);
  if (wp != NULL)
    {
      if (wp->database == NULL)
        ipf_begin_link (NULL, wp->ref);
      else
        ipf_begin_link (wp->database->str, wp->ref);
      ipf_string (word, FALSE);
      ipf_end_link ();
    }
  if (*s != 0)
    {
      write_string (",");
      if (output_x >= 60)
        write_nl ();
      else
        write_string (" ");
    }
}


void ipf_sample_file (const uchar *s)
{
  if (para_flag)
    ipf_para ();
  format_string ("Example: ", STYLE_BOLD, FALSE);
  format_string ("See ", STYLE_NORMAL, FALSE);
  format_string (s, STYLE_TTY, FALSE);
  para_flag = TRUE;
}


void ipf_libref_section (const uchar *s)
{
  if (para_flag)
    ipf_para ();
  format_string (s, STYLE_BOLD, FALSE);
  write_string ("&colon.");
  para_flag = TRUE;
}


void ipf_function_start (const struct toc *tp)
{
  ipf_heading1 (2, tp->ref, tp->global, 0);
  ipf_string (tp->title, TRUE);
  if (idx_refid == 0)
    fatal ("%s:%d: %cfunction without %ci1",
           input_fname, line_no, escape, escape);
  ipf_margin (1);
}


void ipf_function_function (const uchar *s)
{
  if (out)
    {
      write_break ();
      write_fmt (":i2 refid=%d.", idx_refid);
      ipf_string (s, FALSE);
      write_nl ();
    }
}


void ipf_copy (void)
{
  if (para_flag)
    ipf_para ();
  switch (env_stack[env_sp].env)
    {
    case ENV_TYPEWRITER:
      ipf_elements (STYLE_TTY);
      break;
    default:
      ipf_elements (STYLE_NORMAL);
      break;
    }
}


void ipf_minitoc (const struct toc *tp)
{
  int level, any;

  if (tp == NULL)
    fatal ("%s:%d: Cannot build minitoc before the first heading",
           input_fname, line_no);
  if (para_flag)
    ipf_para ();
  level = tp->level;
  tp = tp->next;
  any = FALSE;
  while (tp != NULL && tp->level >= level + 1)
    {
      if (tp->level == level + 1)
        {
          if (!any)
            {
              write_line (":lines align=left.");
              any = TRUE;
            }
          ipf_begin_link (NULL, tp->ref);
          format_output (tp->title, TRUE);
          ipf_end_link ();
          write_nl ();
        }
      tp = tp->next;
    }
  if (any)
    {
      write_line (":elines.");
      para_flag = TRUE;
    }
}


void ipf_end (void)
{
  if (out)
    {
      ipf_hilite (0);
      write_line (":euserdoc.");
    }
}
