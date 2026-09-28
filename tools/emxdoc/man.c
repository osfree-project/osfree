/*! @file man.c
 *  @brief man (roff) output emitter for emxdoc.
 *
 *  Emits a man page using plain roff macros (.TH, .SH, .SS, .TP,
 *  .IP, .RS/.RE, .nf/.fi, .TS/.TE).  Escapes backslash, hyphen,
 *  and a leading period or apostrophe.
 *
 *  @copyright Copyright (C) 2026 osFree Project.
 *             License - see LICENSE in the project root.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "emxdoc.h"
#include "emitter.h"
#include "xref.h"
#include "man.h"

/*!
 * @brief Number of collected top-level index entries.
 */
static int man_index_count;
/*!
 * @brief Buffer used while collecting entries for one level.
 */
static struct word **man_index_v;
/*!
 * @brief Number of entries currently stored in man_index_v.
 */
static int man_index_n;
/*!
 * @brief Current top-level index word (set by %i1).
 */
static struct word *man_index_wp1;

/* ------------------------------------------------------------------ */
/*  Low-level output                                                  */
/* ------------------------------------------------------------------ */

/*!
 * @brief Emits a string with roff special characters escaped.
 *
 * Backslash, hyphen, and a leading period or apostrophe are
 * escaped.  A space triggers a line break when the line exceeds
 * 60 columns and @p may_break is non-zero.
 *
 * @param[in] p         Zero-terminated string.  Not NULL.
 * @param[in] may_break Non-zero if a line break may be inserted.
 */
static void man_escape (const uchar *p, int may_break)
{
  while (*p != 0)
    {
      uchar c = *p++;

      switch (c)
        {
        case '\\':
          write_string ("\\\\");
          break;
        case '-':
          write_string ("\\-");
          break;
        case ' ':
          if (output_x >= 60 && may_break)
            write_nl ();
          else
            write_nstring (&c, 1);
          break;
        case '.':
        case '\'':
          if (output_x == 0)
            write_string ("\\&");
          write_nstring (&c, 1);
          break;
        default:
          write_nstring (&c, 1);
          break;
        }
    }
}

/* ------------------------------------------------------------------ */
/*  Highlighting                                                      */
/* ------------------------------------------------------------------ */

/*! @brief Opens highlighting according to hl_stack. */
void man_start_hilite (void)
{
  int hl = hl_stack[hl_sp];

  if (hl & HL_BF)                         write_string ("\\fB");
  else if (hl & HL_TT)                    write_string ("\\f(CW");
  else if (hl & (HL_SL | HL_EM | HL_UL))  write_string ("\\fI");
  else                                    write_string ("\\fP");
}

/*! @brief Closes highlighting according to hl_stack. */
void man_end_hilite (void)
{
  int hl;

  if (hl_sp == 0)
    return;
  hl = hl_stack[hl_sp - 1];

  if (hl & HL_BF)                         write_string ("\\fB");
  else if (hl & HL_TT)                    write_string ("\\f(CW");
  else if (hl & (HL_SL | HL_EM | HL_UL))  write_string ("\\fI");
  else                                    write_string ("\\fP");
}

/* ------------------------------------------------------------------ */
/*  Page prologue / epilogue                                          */
/* ------------------------------------------------------------------ */

/*!
 * @brief Builds the man-page name from the input file name.
 *
 * @param[out] out   Receiver.  Not NULL.
 * @param[in]  outsz Size of the buffer.
 */
static void man_th_name (char *out, size_t outsz)
{
  const char *base = input_fname ? input_fname : "DOC";
  const char *s = base + strlen (base);
  size_t n = 0;

  while (s > base && s[-1] != '/' && s[-1] != '\\')
    --s;
  while (s[n] != 0 && s[n] != '.' && n + 1 < outsz)
    {
      out[n] = (char)toupper ((unsigned char)s[n]);
      ++n;
    }
  out[n] = 0;
  if (n == 0)
    strcpy (out, "DOC");
}

/*!
 * @brief qsort callback comparing two index entries by strcmp.
 *
 * @param[in] p1 Pointer to first entry.
 * @param[in] p2 Pointer to second entry.
 *
 * @return Negative, zero, or positive, following the qsort convention.
 */
static int man_index_cmp (const void *p1, const void *p2)
{
  const struct word *wp1 = *(const struct word **)p1;
  const struct word *wp2 = *(const struct word **)p2;
  return strcmp ((const char *)wp1->str, (const char *)wp2->str);
}

/*!
 * @brief Callback: collect index words that have entries.
 *
 * @param[in] wp Word to examine.  Not NULL.
 *
 * @return Always 0.
 */
static int man_index_add (struct word *wp)
{
  if (wp->idx != 0 || wp->subidx != NULL)
    {
      assert (man_index_n < man_index_count);
      man_index_v[man_index_n++] = wp;
    }
  return 0;
}

/*!
 * @brief Recursively emit an index table.
 *
 * @param[in] wt    Word table to emit.  Not NULL.
 * @param[in] level Current nesting level.
 */
static void man_index_recurse (struct word_table *wt, int level)
{
  struct word **v;
  int i, n;

  if (level != 0)
    man_index_count = wt_count (wt);
  n = man_index_count;
  v = xmalloc ((size_t)n * sizeof (*v));
  man_index_v = v;
  man_index_n = 0;
  wt_walk (wt, man_index_add);
  assert (man_index_n == n);
  man_index_v = NULL;
  man_index_count = 0;

  qsort (v, (size_t)n, sizeof (*v), man_index_cmp);
  for (i = 0; i < n; ++i)
    {
      struct word *wp = v[i];
      if (level > 0 && i == 0)
        write_string (".RS 4\n");
      write_string (".TP\n\\fB");
      man_escape (wp->str, FALSE);
      write_string ("\\fP\n");
      if (wp->subidx != NULL)
        man_index_recurse (wp->subidx, level + 1);
    }
  if (level > 0 && n > 0)
    write_string (".RE\n");
  free (v);
}

/*! @brief Initializes man-page output, prints .TH and .SH NAME. */
void man_start (void)
{
  char name[64];

  man_th_name (name, sizeof (name));
  write_string (".\\\" Generated by emxdoc (man emitter)\n");
  write_fmt (".TH \"%s\" 1 \"\" \"\" \"\"\n", name);
  write_string (".nh\n");
  write_string (".ad l\n");

  write_string (".SH NAME\n");
  man_escape ((const uchar *)name, FALSE);
  write_string (" \\- ");
  if (title != NULL)
    man_escape (title, FALSE);
  else
    write_string ("manual page");
  write_nl ();

  man_index_wp1 = NULL;
  man_index_count = 0;
}

/*! @brief Finalizes man-page output, emitting the index. */
void man_end (void)
{
  if (out && man_index_count != 0)
    {
      write_string (".SH INDEX\n");
      man_index_recurse (word_top, 0);
    }
}

/* ------------------------------------------------------------------ */
/*  Inline output                                                     */
/* ------------------------------------------------------------------ */

/*!
 * @brief Emits a text fragment with roff escaping.
 *
 * @param[in] p         Zero-terminated string.  Not NULL.
 * @param[in] may_break Non-zero if a line break is allowed.
 */
void man_output (const uchar *p, int may_break)
{
  man_escape (p, may_break);
}

/* ------------------------------------------------------------------ */
/*  Headings                                                          */
/* ------------------------------------------------------------------ */

/*!
 * @brief Starts a section heading (.SH or .SS).
 *
 * @param[in] level  Heading level (0 for %h= / %h-).
 * @param[in] ref    Reference number (unused).
 * @param[in] global Non-zero if the section is global (unused).
 * @param[in] flags  HF_* flags (unused).
 */
void man_heading1 (int level, int ref, int global, unsigned flags)
{
  (void)ref;
  (void)global;
  (void)flags;
  if (output_x > 0)
    write_nl ();
  if (level >= 2)
    write_string (".SS ");
  else
    write_string (".SH ");
}

/*!
 * @brief Finishes a section heading.
 *
 * @param[in] s Heading text with section number prefix.  Not NULL.
 */
void man_heading2 (const uchar *s)
{
  man_escape (s, FALSE);
  write_nl ();
}

/*!
 * @brief Emits one TOC line.
 *
 * @param[in] s  Section number with alignment padding.
 * @param[in] tp TOC node.  Not NULL.
 */
void man_toc_line (const uchar *s, const struct toc *tp)
{
  write_string (".TP\n\\fB");
  man_escape (s, FALSE);
  write_string ("\\fP\n");
  man_escape (tp->title, FALSE);
  write_nl ();
}

/*!
 * @brief Emits the function-section heading.
 *
 * @param[in] tp TOC node.  Not NULL.
 */
void man_function_start (const struct toc *tp)
{
  if (output_x > 0)
    write_nl ();
  write_string (".SS ");
  man_escape (tp->title, FALSE);
  write_nl ();
}

/*!
 * @brief Emits one function name within a function block.
 *
 * Registers the function name in the sub-index of the current %i1.
 *
 * @param[in] tp TOC node of the enclosing function (unused).
 * @param[in] s  Function name.  Not NULL.
 */
void man_function_function (const struct toc *tp, const uchar *s)
{
  (void)tp;
  if (man_index_wp1 != NULL)
    man_index (NULL, s, 2);
}

/* ------------------------------------------------------------------ */
/*  Environments                                                      */
/* ------------------------------------------------------------------ */

/*! @brief Starts the description environment. */
void man_description (void)
{
  if (output_x > 0) write_nl ();
  write_string (".RS 6\n");
}

/*! @brief Starts the enumerate environment. */
void man_enumerate (void)
{
  if (output_x > 0) write_nl ();
  write_string (".RS 4\n");
}

/*! @brief Starts the itemize environment. */
void man_itemize (void)
{
  if (output_x > 0) write_nl ();
  write_string (".RS 4\n");
}

/*! @brief Starts the indent environment. */
void man_indent (void)
{
  if (output_x > 0) write_nl ();
  write_string (".RS 4\n");
}

/*! @brief Starts the list environment. */
void man_list (void)
{
  if (output_x > 0) write_nl ();
  write_string (".RS 6\n");
}

/*! @brief Closes the current environment. */
void man_end_env (void)
{
  if (env_sp == 0)
    return;
  switch (env_stack[env_sp].env)
    {
    case ENV_DESCRIPTION:
    case ENV_ENUMERATE:
    case ENV_ITEMIZE:
    case ENV_LIST:
    case ENV_INDENT:
    case ENV_TYPEWRITER:
      if (output_x > 0) write_nl ();
      write_string (".RE\n");
      break;
    default:
      break;
    }
}

/* ------------------------------------------------------------------ */
/*  List items                                                        */
/* ------------------------------------------------------------------ */

/*!
 * @brief Starts a description-list item.
 *
 * @param[in] s Term.  Not NULL.
 */
void man_description_item (const uchar *s)
{
  if (output_x > 0) write_nl ();
  write_string (".TP\n");
  man_escape (s, FALSE);
  write_nl ();
}

/*! @brief Starts an ordered-list item. */
void man_enumerate_item (void)
{
  int n = ++env_stack[env_sp].counter;

  if (output_x > 0) write_nl ();
  write_fmt (".IP \"%d.\" 4\n", n);
}

/*! @brief Starts an unordered-list item. */
void man_itemize_item (void)
{
  if (output_x > 0) write_nl ();
  write_string (".IP \\(bu 2\n");
}

/*!
 * @brief Starts a description-list item (list environment).
 *
 * @param[in] s Term.  Not NULL.
 */
void man_list_item (const uchar *s)
{
  if (output_x > 0) write_nl ();
  write_string (".TP\n");
  man_escape (s, FALSE);
  write_nl ();
}

/* ------------------------------------------------------------------ */
/*  Verbatim / prototype                                              */
/* ------------------------------------------------------------------ */

/*!
 * @brief Starts a verbatim/example/samplecode/headers block.
 *
 * @param[in]  tag_end  End tag of the block.
 * @param[out] ptmargin Receiver of the current margin.  Not NULL.
 */
void man_verbatim_start (enum tag tag_end, int *ptmargin)
{
  (void)ptmargin;
  if (output_x > 0) write_nl ();
  switch (tag_end)
    {
    case TAG_ENDHEADERS:
      write_string (".PP\n\\fBHeaders:\\fP\n");
      break;
    case TAG_ENDSAMPLECODE:
      write_string (".PP\n\\fBExample:\\fP\n");
      break;
    case TAG_ENDEXAMPLE:
      write_string (".RS 4\n");
      break;
    default:
      break;
    }
  write_string (".nf\n");
}

/*!
 * @brief Emits one verbatim-block line.
 *
 * @param[in] tag_end End tag of the block (unused).
 * @param[in] tmargin Current margin (unused).
 * @param[in] compat  Compatibility string (unused).
 */
void man_verbatim_line (enum tag tag_end, int tmargin, uchar *compat)
{
  (void)tag_end;
  (void)tmargin;
  (void)compat;
  man_escape (input, FALSE);
  write_nl ();
}

/*!
 * @brief Finishes a verbatim/example/samplecode block.
 *
 * @param[in] tag_end End tag of the block.
 */
void man_verbatim_end (enum tag tag_end)
{
  if (output_x > 0) write_nl ();
  write_string (".fi\n");
  if (tag_end == TAG_ENDEXAMPLE)
    write_string (".RE\n");
  para_flag = TRUE;
}

/*!
 * @brief Starts a prototype block.
 *
 * @param[in] compat Compatibility string (may be empty).  Not NULL.
 */
void man_prototype_start (uchar *compat)
{
  (void)compat;
  if (output_x > 0) write_nl ();
  write_string (".nf\n");
}

/*!
 * @brief Finishes a prototype block and emits COMPATIBILITY.
 *
 * @param[in] compat Compatibility string (may be empty).  Not NULL.
 */
void man_prototype_end (uchar *compat)
{
  man_copy ();
  if (output_x > 0) write_nl ();
  write_string (".fi\n");
  if (compat[0] != 0)
    {
      write_string (".SH COMPATIBILITY\n");
      man_escape (compat, TRUE);
      write_nl ();
      compat[0] = 0;
    }
  para_flag = TRUE;
}

/* ------------------------------------------------------------------ */
/*  Element list                                                      */
/* ------------------------------------------------------------------ */

/*! @brief Emits the accumulated elements. */
void man_copy (void)
{
  enum style style_stack[STYLE_STACK_SIZE];
  int style_sp = 0;
  const struct element *ep;

  style_stack[0] = STYLE_NORMAL;
  for (ep = elements; ep->el != EL_END; ++ep)
    {
      switch (ep->el)
        {
        case EL_WORD:
        case EL_PUNCT:
          {
            enum style sty;

            if (ep->n != 0)
              {
                /* Cross-reference to a section. */
                write_string ("\\fB");
                man_escape (ep->wp->str, FALSE);
                write_string ("\\fP");
                break;
              }
            if (style_sp == 0 || style_stack[style_sp] == STYLE_NORMAL)
              sty = (ep->wp->style != STYLE_NORMAL
                     ? ep->wp->style : STYLE_NORMAL);
            else
              sty = style_stack[style_sp];
            if (ep->wp->special != NULL && ep->wp->special->man != NULL)
              format_output (ep->wp->special->man, FALSE);
            else if (ep->wp->special != NULL && ep->wp->special->text != NULL)
              format_output (ep->wp->special->text, FALSE);
            else
              format_string (ep->wp->str, sty, FALSE);
          }
          break;

        case EL_SPACE:
          {
            int n;
            for (n = 0; n < ep->n; ++n)
              {
                if (output_x >= 60)
                  write_nl ();
                else
                  write_string (" ");
              }
          }
          break;

        case EL_STYLE:
          if (style_sp + 1 >= STYLE_STACK_SIZE)
            fatal ("%s:%d: Style stack overflow", input_fname, line_no);
          style_stack[++style_sp] = (enum style)ep->n;
          break;

        case EL_ENDSTYLE:
          if (style_sp == 0)
            fatal ("%s:%d: Style stack underflow", input_fname, line_no);
          --style_sp;
          break;

        case EL_BREAK:
          if (ep->n) write_nl ();
          else       write_string (" ");
          break;

        default:
          abort ();
        }
    }
}

/* ------------------------------------------------------------------ */
/*  See also                                                          */
/* ------------------------------------------------------------------ */

/*! @brief Starts the "See also" block. */
void man_see_also_start (void)
{
  if (output_x > 0) write_nl ();
  write_string (".SH SEE ALSO\n");
}

/*!
 * @brief Emits one "See also" reference.
 *
 * @param[in] word Reference text.  Not NULL.
 * @param[in] s    Remaining list text.  Not NULL.
 */
void man_see_also_word (const uchar *word, const uchar *s)
{
  struct word *wp;

  wp = use_reference (word);
  write_string ("\\fB");
  man_escape (word, FALSE);
  write_string ("\\fP");
  if (wp != NULL)
    write_string (" (1)");
  if (*s != 0)
    {
      write_string (",");
      if (output_x >= 60)
        write_nl ();
      else
        write_string (" ");
    }
}

/*!
 * @brief Finishes the "See also" block.
 *
 * @param[in] s Accumulated reference list.  Not NULL.
 */
void man_see_also_end (const uchar *s)
{
  (void)s;
  write_nl ();
  para_flag = TRUE;
}

/* ------------------------------------------------------------------ */
/*  Sample file / libref                                              */
/* ------------------------------------------------------------------ */

/*!
 * @brief Emits the reference to a sample file.
 *
 * @param[in] s File name.  Not NULL.
 */
void man_sample_file (const uchar *s)
{
  if (para_flag) write_nl ();
  write_string ("\\fBExample:\\fP See ");
  man_escape (s, FALSE);
  write_nl ();
  para_flag = TRUE;
}

/*!
 * @brief Emits a libref section heading.
 *
 * @param[in] s Section title.  Not NULL.
 */
void man_libref_section (const uchar *s)
{
  if (output_x > 0) write_nl ();
  write_string (".PP\n\\fB");
  man_escape (s, FALSE);
  write_string (":\\fP\n");
  para_flag = TRUE;
}

/* ------------------------------------------------------------------ */
/*  Index                                                             */
/* ------------------------------------------------------------------ */

/*!
 * @brief Registers an index entry.
 *
 * @param[in] tp    TOC entry the entry refers to (unused).
 * @param[in] s     Index entry text.  Not NULL.
 * @param[in] level 0 for %index, 1 for %i1, 2 for %i2.
 */
void man_index (const struct toc *tp, const uchar *s, int level)
{
  struct word *wp;

  (void)tp;
  if (!out)
    return;
  switch (level)
    {
    case 0:
      wp = word_add (s);
      if (wp->idx == 0)
        {
          wp->idx = 1;
          if (wp->subidx == NULL)
            ++man_index_count;
        }
      break;
    case 1:
      if (*s == 0)
        man_index_wp1 = NULL;
      else
        man_index_wp1 = word_add (s);
      break;
    case 2:
      if (man_index_wp1 == NULL)
        fatal ("%s:%d: %ci2 without %ci1", input_fname, line_no,
               escape, escape);
      if (man_index_wp1->subidx == NULL)
        {
          man_index_wp1->subidx = wt_new (37);
          if (man_index_wp1->idx == 0)
            ++man_index_count;
        }
      wp = wt_add (man_index_wp1->subidx, s);
      if (wp->idx == 0)
        wp->idx = 1;
      break;
    default:
      abort ();
    }
}

/* ------------------------------------------------------------------ */
/*  Minitoc                                                           */
/* ------------------------------------------------------------------ */

/*!
 * @brief Emits a mini table of contents.
 *
 * @param[in] tp Current table-of-contents entry.  Not NULL.
 */
void man_minitoc (const struct toc *tp)
{
  int level;

  if (tp == NULL)
    fatal ("%s:%d: Cannot build minitoc before the first heading",
           input_fname, line_no);
  if (output_x > 0) write_nl ();
  level = tp->level;
  tp = tp->next;
  while (tp != NULL && tp->level >= level + 1)
    {
      if (tp->level == level + 1 && !(tp->flags & HF_HIDE))
        {
          if (tp->number != NULL)
            {
              write_string ("\\fB");
              man_escape (tp->number, FALSE);
              write_string ("\\fP ");
            }
          man_escape (tp->title, FALSE);
          write_nl ();
        }
      tp = tp->next;
    }
  para_flag = TRUE;
}

/* ------------------------------------------------------------------ */
/*  Table (tbl preprocessor)                                          */
/* ------------------------------------------------------------------ */

/*!
 * @brief Begins a table block (.TS).
 *
 * @param[in] do_indent Non-zero to indent the table.
 * @param[in] widths    Column widths (unused; tbl auto-sizes).
 * @param[in] wn        Number of columns.
 */
void man_table_start (int do_indent, int *widths, int wn)
{
  int wi;

  (void)widths;
  if (output_x > 0) write_nl ();
  if (do_indent) write_string (".RS 4\n");
  write_string (".TS\ntab(|) ;\n");
  if (wn <= 0) wn = 1;
  for (wi = 0; wi < wn; ++wi)
    {
      if (wi != 0) write_string (" ");
      write_string ("l");
    }
  write_string (".\n");
}

/*!
 * @brief Emits one table row.
 *
 * Cells in @p s are separated by 0xb3 (VBAR).
 *
 * @param[in] s  Row text.  Not NULL.
 * @param[in] wn Expected number of cells.
 */
void man_table_line (const uchar *s, int wn)
{
  uchar word[512], *d;
  const uchar *p;
  int wi;

  if (wn <= 0) wn = 1;
  for (wi = 0; wi < wn; ++wi)
    {
      if (wi != 0)
        write_string ("|");
      d = word;
      p = s;
      while (*p != 0 && *p != 0xb3)
        *d++ = *p++;
      while (d != word && isspace (d[-1]))
        --d;
      *d = 0;
      s = p;
      if (*s == 0xb3) ++s;
      make_elements (word);
      man_copy ();
    }
  write_nl ();
}

/*!
 * @brief Ends a table block (.TE).
 *
 * @param[in] do_indent Non-zero if the table was indented.
 */
void man_table_end (int do_indent)
{
  write_string (".TE\n");
  if (do_indent) write_string (".RE\n");
  para_flag = TRUE;
}
