/*! @file md.c
 *  @brief Markdown backend for emxdoc.
 *
 *  Emits a Markdown document: headings via #, ##, ###, inline
 *  styles via HTML tags <b>, <i>, <code>, <u>, code blocks via
 *  ```…```.  Escapes Markdown-significant characters with a
 *  backslash.
 *
 *  @copyright Copyright (C) 2026 osFree Project.
 *             License — see LICENSE in the project root.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "emxdoc.h"
#include "md.h"

/*!
 *  @brief Emits a line with Markdown special characters escaped.
 *  @param[in] p Pointer to a zero-terminated string.
 */
static void md_escape (const uchar *p)
{
  while (*p != 0)
    {
      uchar c = *p++;

      switch (c)
        {
        case '\\': case '`': case '*': case '_':
        case '[': case ']': case '<': case '>':
          write_string ("\\");
          break;
        case '#': case '+': case '-': case '.':
        case '!':
          if (output_x == 0)
            write_string ("\\");
          break;
        default:
          break;
        }
      write_nstring (&c, 1);
    }
}

/*!
 *  @brief Emits the highlight mask difference as HTML tags.
 *  @param[in] mask    New highlight mask.
 *  @param[in] prev    Previous mask.
 *  @param[in] opening Non-zero — opening tags, zero — closing tags.
 */
static void md_emit_hilite (int mask, int prev, int opening)
{
  int diff = mask & ~prev;
  int italic = (diff & HL_EM) || (diff & HL_SL);

  if (diff & HL_BF)
    write_string (opening ? "<b>" : "</b>");
  if (italic)
    write_string (opening ? "<i>" : "</i>");
  if (diff & HL_TT)
    write_string (opening ? "<code>" : "</code>");
  if (diff & HL_UL)
    write_string (opening ? "<u>" : "</u>");
}

/*! @brief Opens highlighting. */
void md_start_hilite (void)
{
  md_emit_hilite (hl_stack[hl_sp], hl_stack[hl_sp - 1], TRUE);
}

/*! @brief Closes highlighting. */
void md_end_hilite (void)
{
  md_emit_hilite (hl_stack[hl_sp], hl_stack[hl_sp - 1], FALSE);
}

/*! @brief Initializes Markdown output (no-op). */
void md_start (void) { }

/*! @brief Finalizes Markdown output (no-op). */
void md_end (void) { }

/*!
 *  @brief Emits a text fragment with Markdown escaping.
 *  @param[in] p         Pointer to a zero-terminated string.
 *  @param[in] may_break Non-zero if a line break is allowed.
 */
void md_output (const uchar *p, int may_break)
{
  (void)may_break;
  md_escape (p);
}

/*! @brief Starts a section heading. */
void md_heading1 (void)
{
  if (output_x > 0)
    write_nl ();
  switch (tg_level)
    {
    case 1:  write_string ("# ");   break;
    case 2:  write_string ("## ");  break;
    case 3:  write_string ("### "); break;
    default: write_string ("## ");  break;
    }
}

/*!
 *  @brief Finishes a section heading.
 *  @param[in] s Heading text.
 */
void md_heading2 (uchar *s)
{
  md_escape (s);
  write_nl ();
}

/*!
 *  @brief Emits one TOC line.
 *  @param[in] s  Section number.
 *  @param[in] tp TOC node.  Not NULL.
 */
void md_toc_line (const uchar *s, const struct toc *tp)
{
  write_string ("* ");
  write_string (s);
  md_escape (tp->title);
  write_nl ();
}

/*! @brief Starts the description environment. */
void md_description (void) { }

/*! @brief Starts the enumerate environment. */
void md_enumerate (void)   { }

/*! @brief Starts the itemize environment. */
void md_itemize (void)     { }

/*! @brief Starts the list environment. */
void md_list (void)        { }

/*! @brief Starts the indent environment. */
void md_indent (void)
{
  if (output_x > 0) write_nl ();
  write_string ("<blockquote>\n");
}

/*! @brief Closes the current environment. */
void md_end_env (void)
{
  switch (env_stack[env_sp].env)
    {
    case ENV_INDENT:
    case ENV_TYPEWRITER:
      if (output_x > 0) write_nl ();
      write_string ("</blockquote>\n");
      break;
    default:
      break;
    }
}

/*!
 *  @brief Emits indentation for a nesting level.
 *  @param[in] n Number of levels.
 */
static void md_indent_for (int n)
{
  int i;
  for (i = 0; i < n; ++i)
    write_string ("  ");
}

/*!
 *  @brief Starts a description-list item.
 *  @param[in] s Term.
 */
void md_description_item (const uchar *s)
{
  if (output_x > 0) write_nl ();
  md_indent_for (env_sp - 1);
  write_string ("* **");
  md_escape (s);
  write_string ("** ");
}

/*! @brief Starts an ordered-list item. */
void md_enumerate_item (void)
{
  int n = ++env_stack[env_sp].counter;

  if (output_x > 0) write_nl ();
  md_indent_for (env_sp - 1);
  write_fmt ("%d. ", n);
}

/*! @brief Starts an unordered-list item. */
void md_itemize_item (void)
{
  if (output_x > 0) write_nl ();
  md_indent_for (env_sp - 1);
  write_string ("* ");
}

/*!
 *  @brief Starts a description-list item (list environment).
 *  @param[in] s Term.
 */
void md_list_item (const uchar *s)
{
  if (output_x > 0) write_nl ();
  md_indent_for (env_sp - 1);
  write_string ("* **");
  md_escape (s);
  write_string ("** ");
}

/*!
 *  @brief Starts a verbatim/example/samplecode block.
 *  @param[in]  tag_end  End tag of the block.
 *  @param[out] ptmargin Receiver of the current margin.
 */
void md_verbatim_start (enum tag tag_end, int *ptmargin)
{
  (void)ptmargin;
  if (output_x > 0) write_nl ();
  if (tag_end == TAG_ENDSAMPLECODE)
    write_string ("**Example:**\n");
  write_string ("```\n");
}

/*!
 *  @brief Emits one verbatim-block line.
 *  @param[in] tag_end End tag of the block.
 *  @param[in] tmargin Current margin.
 *  @param[in] compat  Compatibility string.
 */
void md_verbatim_line (enum tag tag_end, int tmargin, uchar *compat)
{
  (void)tag_end; (void)tmargin; (void)compat;
  write_line (input);
}

/*!
 *  @brief Finishes a verbatim/example/samplecode block.
 *  @param[in] tag_end End tag of the block.
 */
void md_verbatim_end (enum tag tag_end)
{
  (void)tag_end;
  if (output_x > 0) write_nl ();
  write_string ("```\n");
  para_flag = TRUE;
}

/*!
 *  @brief Starts a prototype block.
 *  @param[in] compat Compatibility string.
 */
void md_prototype_start (uchar *compat)
{
  (void)compat;
  if (output_x > 0) write_nl ();
  write_string ("```c\n");
}

/*! @brief Finishes a prototype block. */
void md_prototype_end (void)
{
  md_copy ();
  if (output_x > 0) write_nl ();
  write_string ("```\n");
  para_flag = TRUE;
}

/*! @brief Emits the accumulated elements. */
void md_copy (void)
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

            if (style_sp == 0 || style_stack[style_sp] == STYLE_NORMAL)
              sty = (ep->wp->style != STYLE_NORMAL
                     ? ep->wp->style : STYLE_NORMAL);
            else
              sty = style_stack[style_sp];
            if (ep->wp->special != NULL && ep->wp->special->text != NULL)
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
void md_function (const struct toc *tp)
{
  if (output_x > 0) write_nl ();
  write_string ("## ");
  md_escape (tp->title);
  write_nl ();
}

/*! @brief Starts the "See also" block. */
void md_see_also_start (void)
{
  if (output_x > 0) write_nl ();
  write_string ("## See also\n\n");
}

/*!
 *  @brief Finishes the "See also" block.
 *  @param[in] s Ready-made string with the link list.
 */
void md_see_also_end (const uchar *s)
{
  md_escape (s);
  write_nl ();
  para_flag = TRUE;
}

/*!
 *  @brief Emits the reference to a sample file.
 *  @param[in] s File name.
 */
void md_sample_file (const uchar *s)
{
  if (para_flag) write_nl ();
  write_string ("**Example:** See ");
  md_escape (s);
  write_nl ();
}

/*!
 *  @brief Emits a libref section heading.
 *  @param[in] s Section title.
 */
void md_libref_section (const uchar *s)
{
  if (output_x > 0) write_nl ();
  write_string ("**");
  md_escape (s);
  write_string ("**\n\n");
  para_flag = TRUE;
}
