/*! @file dokuwiki.c
 *  @brief DokuWiki backend for emxdoc.
 *
 *  Emits DokuWiki text: headings via =====, =====, inline styles
 *  via **, //, '', __, code blocks via <code>...</code>.  Words
 *  containing DokuWiki markup sequences are wrapped in %%...%% for
 *  protection.  Bare URLs are wrapped in [[...]] so that an
 *  enclosing //...// italic run cannot swallow the closing marker.
 *  Block-level tags (<code>, <blockquote>) are preceded by a blank
 *  line, as required by the DokuWiki parser.
 *
 *  @copyright Copyright (C) 2026 osFree Project.
 *             License - see LICENSE in the project root.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "emxdoc.h"
#include "emitter.h"
#include "dokuwiki.h"

/*!
 *  @brief Emits a line with backslash escaped.
 *  @param[in] p Pointer to a zero-terminated string.
 */
static void dw_escape (const uchar *p)
{
  while (*p != 0)
    {
      uchar c = *p++;

      if (c == '\\')
        write_string ("\\\\");
      else
        write_nstring (&c, 1);
    }
}

/*!
 *  @brief Checks whether the string is a bare URL.
 *  @param[in] p Pointer to a zero-terminated string.
 *
 *  @return Non-zero if the string is a URL.
 *
 *  @retval TRUE   The string starts with a known URL scheme.
 *  @retval FALSE  Otherwise.
 *
 *  URLs are wrapped in [[...]] by dw_text() so that an enclosing
 *  //...// italic run cannot swallow the closing marker as part of
 *  the URL.
 */
static int dw_is_url (const uchar *p)
{
  return (strncmp ((const char *)p, "http://",   7) == 0
          || strncmp ((const char *)p, "https://",  8) == 0
          || strncmp ((const char *)p, "ftp://",    6) == 0
          || strncmp ((const char *)p, "ftps://",   7) == 0
          || strncmp ((const char *)p, "gopher://", 9) == 0
          || strncmp ((const char *)p, "mailto:",   7) == 0);
}

/*!
 *  @brief Checks whether the string contains DokuWiki markup sequences.
 *  @param[in] p Pointer to a zero-terminated string.
 *
 *  @return Non-zero if the string needs a %%...%% wrapper.
 *
 *  @retval TRUE   The string needs a %%...%% wrapper.
 *  @retval FALSE  The string does not need a wrapper.
 *
 *  Only paired markers and characters that DokuWiki interprets
 *  unconditionally trigger nowiki.  Single '*', '/', '_' and
 *  apostrophe are ordinary text and must not be wrapped.
 */
static int dw_needs_nowiki (const uchar *p)
{
  while (*p != 0)
    {
      uchar c = *p;
      uchar n = p[1];

      /* Paired markup: **, //, __, '' */
      if ((c == '*' || c == '/' || c == '_' || c == '\'') && c == n)
        return TRUE;

      /* Bracketed markup: [[...]], {{...}} */
      if ((c == '[' && n == '[') || (c == '{' && n == '{'))
        return TRUE;

      /* Deleted text: ~~ */
      if (c == '~' && n == '~')
        return TRUE;

      /* HTML-ish and table markup, nowiki itself */
      if (c == '<' || c == '>' || c == '&' || c == '|' || c == '%')
        return TRUE;

      ++p;
    }
  return FALSE;
}

/*!
 *  @brief Emits a string with DokuWiki escaping.
 *  @param[in] p Pointer to a zero-terminated string.
 *
 *  A bare URL is wrapped in [[...]] so that an enclosing italic
 *  run (//...//) cannot misinterpret the scheme's "//" as its
 *  closing marker.  Everything else that contains paired markup
 *  or HTML-ish characters is wrapped in %%...%%.
 */
static void dw_text (const uchar *p)
{
  if (dw_is_url (p))
    {
      write_string ("[[");
      write_string (p);
      write_string ("]]");
      return;
    }

  if (dw_needs_nowiki (p))
    {
      write_string ("%%");
      dw_escape (p);
      write_string ("%%");
    }
  else
    dw_escape (p);
}

/*!
 *  @brief Emits the highlight mask difference as DokuWiki markup.
 *  @param[in] mask New highlight mask.
 *  @param[in] prev Previous mask.
 */
static void dw_emit_hilite (int mask, int prev)
{
  int diff = mask & ~prev;

  if (diff & HL_BF) write_string ("**");
  if ((diff & HL_EM) || (diff & HL_SL)) write_string ("//");
  if (diff & HL_TT) write_string ("''");
  if (diff & HL_UL) write_string ("__");
}

/*! @brief Opens highlighting. */
void dw_start_hilite (void)
{
  dw_emit_hilite (hl_stack[hl_sp], hl_stack[hl_sp - 1]);
}

/*! @brief Closes highlighting. */
void dw_end_hilite (void)
{
  dw_emit_hilite (hl_stack[hl_sp], hl_stack[hl_sp - 1]);
}

/*! @brief Initializes DokuWiki output (no-op). */
void dw_start (void) { }

/*! @brief Finalizes DokuWiki output (no-op). */
void dw_end (void) { }

/*!
 *  @brief Emits a text fragment with DokuWiki escaping.
 *  @param[in] p         Pointer to a zero-terminated string.
 *  @param[in] may_break Non-zero if a line break is allowed.
 */
void dw_output (const uchar *p, int may_break)
{
  (void)may_break;
  dw_text (p);
}

/*!
 *  @brief Starts a section heading.
 *  @param[in] level  Heading level.
 *  @param[in] ref    Reference number (unused).
 *  @param[in] global Non-zero if the section is global (unused).
 *  @param[in] flags  Heading flags (unused).
 */
void dw_heading1 (int level, int ref, int global, unsigned flags)
{
  int n;

  (void)ref;
  (void)global;
  (void)flags;
  write_break ();
  write_nl ();
  switch (level)
    {
    case 1:  n = 6; break;
    case 2:  n = 5; break;
    case 3:  n = 4; break;
    default: n = 5; break;
    }
  while (n-- > 0) write_string ("=");
  write_string (" ");
}

/*!
 *  @brief Finishes a section heading.
 *  @param[in] s Heading text. Not NULL.
 */
void dw_heading2 (const uchar *s)
{
  int n;

  dw_text (s);
  write_string (" ");
  switch (tg_level)
    {
    case 1:  n = 6; break;
    case 2:  n = 5; break;
    case 3:  n = 4; break;
    default: n = 5; break;
    }
  while (n-- > 0) write_string ("=");
  write_nl ();
}

/*!
 *  @brief Emits one TOC line.
 *  @param[in] s  Section number.
 *  @param[in] tp TOC node.  Not NULL.
 */
void dw_toc_line (const uchar *s, const struct toc *tp)
{
  write_string ("  * ");
  write_string (s);
  dw_text (tp->title);
  write_nl ();
}

/*! @brief Starts the description environment. */
void dw_description (void) { }

/*! @brief Starts the enumerate environment. */
void dw_enumerate (void)   { }

/*! @brief Starts the itemize environment. */
void dw_itemize (void)     { }

/*! @brief Starts the list environment. */
void dw_list (void)        { }

/*! @brief Starts the indent environment. */
void dw_indent (void)
{
  write_break ();
  write_nl ();
  write_line ("<blockquote>");
}

/*! @brief Closes the current environment. */
void dw_end_env (void)
{
  switch (env_stack[env_sp].env)
    {
    case ENV_INDENT:
    case ENV_TYPEWRITER:
      write_break ();
      write_line ("</blockquote>");
      break;
    default:
      break;
    }
}

/*!
 *  @brief Emits indentation for a nesting level.
 *  @param[in] n Number of levels.
 */
static void dw_indent_for (int n)
{
  int i;
  for (i = 0; i < n; ++i)
    write_string ("  ");
}

/*!
 *  @brief Starts a description-list item.
 *  @param[in] s Term. Not NULL.
 */
void dw_description_item (const uchar *s)
{
  if (output_x > 0) write_nl ();
  dw_indent_for (env_sp - 1);
  write_string ("  * **");
  dw_text (s);
  write_string ("**: ");
}

/*! @brief Starts an ordered-list item. */
void dw_enumerate_item (void)
{
  if (output_x > 0) write_nl ();
  dw_indent_for (env_sp - 1);
  write_string ("  - ");
}

/*! @brief Starts an unordered-list item. */
void dw_itemize_item (void)
{
  if (output_x > 0) write_nl ();
  dw_indent_for (env_sp - 1);
  write_string ("  * ");
}

/*!
 *  @brief Starts a description-list item (list environment).
 *  @param[in] s Term. Not NULL.
 */
void dw_list_item (const uchar *s)
{
  if (output_x > 0) write_nl ();
  dw_indent_for (env_sp - 1);
  write_string ("  * **");
  dw_text (s);
  write_string ("**: ");
}

/*!
 *  @brief Starts a verbatim/example/samplecode block.
 *  @param[in]  tag_end  End tag of the block.
 *  @param[out] ptmargin Receiver of the current margin (unused).
 */
void dw_verbatim_start (enum tag tag_end, int *ptmargin)
{
  (void)ptmargin;
  write_break ();
  write_nl ();
  if (tag_end == TAG_ENDSAMPLECODE)
    write_line ("**Example:**");
  write_line ("<code>");
}

/*!
 *  @brief Emits one verbatim-block line.
 *  @param[in] tag_end End tag of the block (unused).
 *  @param[in] tmargin Current margin (unused).
 *  @param[in] compat  Compatibility string (unused).
 */
void dw_verbatim_line (enum tag tag_end, int tmargin, uchar *compat)
{
  (void)tag_end; (void)tmargin; (void)compat;
  write_line (input);
}

/*!
 *  @brief Finishes a verbatim/example/samplecode block.
 *  @param[in] tag_end End tag of the block (unused).
 */
void dw_verbatim_end (enum tag tag_end)
{
  (void)tag_end;
  if (output_x > 0) write_nl ();
  write_line ("</code>");
  para_flag = TRUE;
}

/*!
 *  @brief Starts a prototype block.
 *  @param[in] compat Compatibility string (unused).
 */
void dw_prototype_start (uchar *compat)
{
  (void)compat;
  write_break ();
  write_nl ();
  write_line ("<code c>");
}

/*!
 *  @brief Finishes a prototype block.
 *  @param[in] compat Compatibility string (unused here; DokuWiki
 *                    handles compat inline in dw_prototype_start).
 */
void dw_prototype_end (uchar *compat)
{
  (void)compat;
  dw_copy ();
  if (output_x > 0) write_nl ();
  write_line ("</code>");
  para_flag = TRUE;
}

/*! @brief Emits the accumulated elements. */
void dw_copy (void)
{
  enum style style_stack[STYLE_STACK_SIZE];
  int style_sp = 0;
  const struct element *ep;

  if (para_flag)
    {
      write_break ();
      write_nl ();
    }

  style_stack[0] = STYLE_NORMAL;
  for (ep = elements; ep->el != EL_END; ++ep)
    {
      switch (ep->el)
        {
        case EL_WORD:
        case EL_PUNCT:
          {
            enum style sty;

            if (style_sp == 0 || style_stack[style_sp] == STYLE_NORMAL)
              sty = (ep->wp->style != STYLE_NORMAL
                     ? ep->wp->style : STYLE_NORMAL);
            else
              sty = style_stack[style_sp];
            if (ep->wp->special != NULL && ep->wp->special->dw != NULL)
              format_output (ep->wp->special->dw, FALSE);
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
              write_string (" ");
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

/*!
 *  @brief Emits the function-section heading.
 *  @param[in] tp TOC node.  Not NULL.
 */
void dw_function_start (const struct toc *tp)
{
  if (output_x > 0) write_nl ();
  write_string ("===== ");
  dw_text (tp->title);
  write_string (" =====");
  write_nl ();
}

/*!
 *  @brief Emits one function name within a function block (no-op).
 *  @param[in] tp TOC node. Not NULL.
 *  @param[in] s  Function name. Not NULL.
 */
void dw_function_function (const struct toc *tp, const uchar *s)
{
  (void)tp;
  (void)s;
}

/*! @brief Starts the "See also" block. */
void dw_see_also_start (void)
{
  if (output_x > 0) write_nl ();
  write_line ("===== See also =====");
  write_nl ();
}

/*!
 *  @brief Emits one "See also" reference (no-op; the accumulated
 *         list is emitted by dw_see_also_end).
 *  @param[in] word Reference text. Not NULL.
 *  @param[in] s    Remaining list text. Not NULL.
 */
void dw_see_also_word (const uchar *word, const uchar *s)
{
  (void)word;
  (void)s;
}

/*!
 *  @brief Finishes the "See also" block.
 *  @param[in] s Ready-made string with the link list. Not NULL.
 */
void dw_see_also_end (const uchar *s)
{
  dw_text (s);
  write_nl ();
  para_flag = TRUE;
}

/*!
 *  @brief Emits the reference to a sample file.
 *  @param[in] s File name. Not NULL.
 */
void dw_sample_file (const uchar *s)
{
  if (para_flag) write_nl ();
  write_string ("**Example:** See ");
  dw_text (s);
  write_nl ();
}

/*!
 *  @brief Emits a libref section heading.
 *  @param[in] s Section title. Not NULL.
 */
void dw_libref_section (const uchar *s)
{
  if (output_x > 0) write_nl ();
  write_string ("**");
  dw_text (s);
  write_string ("**");
  write_nl ();
  write_nl ();
  para_flag = TRUE;
}

/*! @brief Begin the table of contents (no-op). */
void dw_toc_start (void) { }

/*! @brief End the table of contents (no-op). */
void dw_toc_end (void) { }

/*!
 *  @brief Emits a mini table of contents (no-op).
 *  @param[in] tp Current table-of-contents entry. Not NULL.
 */
void dw_minitoc (const struct toc *tp)
{
  (void)tp;
}

/*!
 *  @brief Register an index entry (no-op; DokuWiki has no index).
 *  @param[in] tp    TOC entry, or NULL.
 *  @param[in] s     Index entry text. Not NULL.
 *  @param[in] level 0, 1 or 2.
 */
void dw_index (const struct toc *tp, const uchar *s, int level)
{
  (void)tp;
  (void)s;
  (void)level;
}

/*! @brief Begin a table block (no-op; DokuWiki has no tables). */
void dw_table_start (int do_indent, int *widths, int wn)
{
  (void)do_indent;
  (void)widths;
  (void)wn;
}

/*!
 *  @brief Emit one table row (no-op).
 *  @param[in] s  Row text. Not NULL.
 *  @param[in] wn Number of columns.
 */
void dw_table_line (const uchar *s, int wn)
{
  (void)s;
  (void)wn;
}

/*!
 *  @brief End a table block (no-op).
 *  @param[in] do_indent Non-zero if the table was indented.
 */
void dw_table_end (int do_indent)
{
  (void)do_indent;
}

/*!
 *  @brief Emit an HTML fragment anchor (no-op for DokuWiki).
 *  @param[in] s Anchor name. Not NULL.
 */
void dw_html_fragment (const uchar *s)
{
  (void)s;
}
