/*!
 * @file ed.c
 * @brief emxdoc (source) emitter for emxdoc.
 *
 * Reconstructs canonical emxdoc source from the parsed
 * representation.  Uses only the public emxdoc.h interface:
 * elements[] for paragraphs and prototypes, tg_* and struct toc
 * for tags and headings.
 *
 * @copyright Copyright (C) 2026 osFree Project.
 *            License - see LICENSE in the project root.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "emxdoc.h"
#include "emitter.h"
#include "ed.h"

/*!
 * @brief Emits the escape character.
 */
static void ed_esc (void)
{
  write_nstring (&escape, 1);
}

/*!
 * @brief Emits a canonical tag line.
 *
 * @param[in] name Tag name (e.g. "h1", "item", "title").
 * @param[in] arg  Argument, or NULL.  Leading whitespace is trimmed.
 */
static void ed_line (const char *name, const uchar *arg)
{
  ed_esc ();
  write_string ((const uchar *)name);
  if (arg != NULL)
    {
      while (isspace (*arg))
        ++arg;
      if (*arg != 0)
        {
          write_string (" ");
          write_string (arg);
        }
    }
  write_nl ();
}

/*!
 * @brief Emits the opening delimiter of an inline style.
 *
 * @param[in] sty Style to open.
 */
static void ed_open_style (enum style sty)
{
  ed_esc ();
  switch (sty)
    {
    case STYLE_BOLD:      write_string ("bf{"); break;
    case STYLE_SLANTED:   write_string ("sl{"); break;
    case STYLE_UNDERLINE: write_string ("ul{"); break;
    case STYLE_TTY:       write_string ("tt{"); break;
    case STYLE_EMPHASIZE: write_string ("em{"); break;
    case STYLE_SYNTAX:    write_string ("sy{"); break;
    case STYLE_PARAM:     write_string ("pa{"); break;
    default: break;
    }
}

/*!
 * @brief Renders elements[] as canonical emxdoc text, without a
 *        trailing newline.
 *
 * The element list is produced by make_elements() by the caller, so
 * this function never sees raw input lines.  Inline styles are
 * emitted directly from EL_STYLE / EL_ENDSTYLE, so the whole styled
 * run is reproduced as a single %xx{...} block.
 */
static void ed_emit (void)
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
          write_string (ep->wp->str);
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
          ed_open_style ((enum style)ep->n);
          break;

        case EL_ENDSTYLE:
          if (style_sp == 0)
            fatal ("%s:%d: Style stack underflow", input_fname, line_no);
          --style_sp;
          write_string ("}");
          break;

        case EL_BREAK:
          if (ep->n)
            write_nl ();
          else
            write_string (" ");
          break;

        default:
          break;
        }
    }
}

/*! @brief Initializes emxdoc-source output, emitting the title tag. */
void ed_start (void)
{
  write_string (".\\\" Re-emitted by emxdoc (emxdoc emitter)\n");
  if (title != NULL)
    ed_line ("title", title);
}

/*! @brief Finalizes emxdoc-source output. */
void ed_end (void)
{
}

/*!
 * @brief Emits text verbatim.
 *
 * @param[in] p         String.  Not NULL.
 * @param[in] may_break Unused.
 */
void ed_output (const uchar *p, int may_break)
{
  (void)may_break;
  write_string (p);
}

/*!
 * @brief Opens an inline style according to hl_stack.
 *
 * ed_emit() emits inline styles directly from the element list, so
 * this hook is not normally reached.  It is provided for
 * completeness when format_string() is used on styled text.
 */
void ed_start_hilite (void)
{
  int cur = hl_stack[hl_sp];
  int prev = (hl_sp > 0) ? hl_stack[hl_sp - 1] : 0;
  int diff = cur & ~prev;

  if (diff == 0)
    return;
  if (diff & HL_BF)                       ed_open_style (STYLE_BOLD);
  else if (diff & HL_TT)                  ed_open_style (STYLE_TTY);
  else if (diff & HL_SL)                  ed_open_style (STYLE_SLANTED);
  else if (diff & HL_UL)                  ed_open_style (STYLE_UNDERLINE);
  else if (diff & HL_EM)                  ed_open_style (STYLE_EMPHASIZE);
}

/*!
 * @brief Closes an inline style.
 */
void ed_end_hilite (void)
{
  int cur = hl_stack[hl_sp];
  int prev = (hl_sp > 0) ? hl_stack[hl_sp - 1] : 0;
  int removed = cur & ~prev;

  if (removed == 0)
    return;
  write_string ("}");
}

/*!
 * @brief Emits a canonical %hN / %h= / %h- prologue.
 *
 * @param[in] level  Heading level (unused here; ed_heading2 has it).
 * @param[in] ref    Reference number (unused).
 * @param[in] global Non-zero if global (unused).
 * @param[in] flags  HF_* flags (unused).
 */
void ed_heading1 (int level, int ref, int global, unsigned int flags)
{
  (void)level;
  (void)ref;
  (void)global;
  (void)flags;
}

/*!
 * @brief Emits a canonical %hN / %h= / %h- body.
 *
 * The section-number prefix carried by @p s is stripped; the tag
 * name is rebuilt from tg_level, tg_underline, tg_flags.
 *
 * @param[in] s Heading text.  Not NULL.
 */
void ed_heading2 (const uchar *s)
{
  const uchar *title = s;
  char name[8];
  int n = 0;

  if (tg_level > 0 && !(tg_flags & HF_UNNUMBERED))
    {
      const uchar *p = s;
      while (*p != 0 && *p != ' ')
        ++p;
      while (*p == ' ')
        ++p;
      if (*p != 0)
        title = p;
    }

  name[n++] = 'h';
  if (tg_level == 0)
    name[n++] = (char)tg_underline;
  else
    name[n++] = (char)('0' + tg_level);
  if (tg_flags & HF_UNNUMBERED)  name[n++] = 'u';
  if (tg_flags & HF_HIDE)        name[n++] = 'h';
  name[n] = 0;

  ed_line (name, title);
}

/*! @brief Emits %description. */
void ed_description (void)  { ed_line ("description",  NULL); }
/*! @brief Emits %enumerate. */
void ed_enumerate   (void)  { ed_line ("enumerate",    NULL); }
/*! @brief Emits %itemize. */
void ed_itemize     (void)  { ed_line ("itemize",      NULL); }
/*! @brief Emits %indent. */
void ed_indent      (void)  { ed_line ("indent",       NULL); }
/*! @brief Emits %list. */
void ed_list        (void)  { ed_line ("list",         NULL); }

/*!
 * @brief Emits the closing tag of the current environment.
 */
void ed_end_env (void)
{
  if (env_sp == 0)
    return;
  switch (env_stack[env_sp].env)
    {
    case ENV_DESCRIPTION: ed_line ("enddescription", NULL); break;
    case ENV_ENUMERATE:   ed_line ("endenumerate",   NULL); break;
    case ENV_ITEMIZE:     ed_line ("enditemize",     NULL); break;
    case ENV_LIST:        ed_line ("endlist",        NULL); break;
    case ENV_INDENT:      ed_line ("endindent",      NULL); break;
    case ENV_TYPEWRITER:  ed_line ("endtypewriter",  NULL); break;
    default: break;
    }
}

/*!
 * @brief Emits a %item tag with a term.
 * @param[in] s Term.  Not NULL.
 */
void ed_description_item (const uchar *s)  { ed_line ("item", s); }

/*! @brief Emits a %item tag without a term. */
void ed_enumerate_item   (void)            { ed_line ("item", NULL); }

/*! @brief Emits a %item tag without a term. */
void ed_itemize_item     (void)            { ed_line ("item", NULL); }

/*!
 * @brief Emits a %item tag with a term (list environment).
 * @param[in] s Term.  Not NULL.
 */
void ed_list_item (const uchar *s)         { ed_line ("item", s); }

/*!
 * @brief Emits the current paragraph (elements[]) with a newline.
 */
void ed_copy (void)
{
  ed_emit ();
  write_nl ();
}

/*! @brief Emits the opening of the see-also list. */
void ed_see_also_start (void)
{
  ed_esc ();
  write_string ("seealso");
}

/*!
 * @brief Emits one see-also reference word.
 * @param[in] word Reference word.  Not NULL.
 * @param[in] s    Remaining list text (unused).
 */
void ed_see_also_word (const uchar *word, const uchar *s)
{
  (void)s;
  write_string (" ");
  write_string (word);
}

/*!
 * @brief Emits the closing of the see-also list.
 * @param[in] s Ignored.
 */
void ed_see_also_end (const uchar *s)
{
  (void)s;
  write_nl ();
}

/*!
 * @brief Emits a %samplefile reference.
 * @param[in] s Sample file name.  Not NULL.
 */
void ed_sample_file (const uchar *s)  { ed_line ("samplefile", s); }

/*!
 * @brief Emits a libref section heading tag.
 * @param[in] s Section title.  Not NULL.
 */
void ed_libref_section (const uchar *s)
{
  if      (strcmp ((const char *)s, "Restrictions") == 0)
    ed_line ("restrictions", NULL);
  else if (strcmp ((const char *)s, "Implementation-defined behavior") == 0)
    ed_line ("implementation", NULL);
  else if (strcmp ((const char *)s, "Bugs") == 0)
    ed_line ("bugs", NULL);
  else if (strcmp ((const char *)s, "Errors") == 0)
    ed_line ("errors", NULL);
  else if (strcmp ((const char *)s, "Hints") == 0)
    ed_line ("hints", NULL);
  else if (strcmp ((const char *)s, "Return value") == 0)
    ed_line ("returnvalue", NULL);
}

/*!
 * @brief Emits an index tag.
 * @param[in] tp    TOC entry (unused).
 * @param[in] s     Index entry text.  Not NULL.
 * @param[in] level 0 for %index, 1 for %i1, 2 for %i2.
 */
void ed_index (const struct toc *tp, const uchar *s, int level)
{
  (void)tp;
  if (!out)
    return;
  switch (level)
    {
    case 0: ed_line ("index", s); break;
    case 1: ed_line ("i1",    s); break;
    case 2: ed_line ("i2",    s); break;
    default: break;
    }
}

/*! @brief Emits a %toc tag. */
void ed_toc_start (void)
{
  ed_line ("toc", NULL);
}

/*!
 * @brief Emits one TOC entry (no-op; %toc emitted by ed_toc_start).
 * @param[in] s  Section number.  Not NULL.
 * @param[in] tp TOC node.  Not NULL.
 */
void ed_toc_line (const uchar *s, const struct toc *tp)
{
  (void)s;
  (void)tp;
}

/*! @brief Emits the closing of the %toc tag (no-op). */
void ed_toc_end (void)
{
}

/*!
 * @brief Emits a %minitoc tag.
 * @param[in] tp Current table-of-contents entry (unused).
 */
void ed_minitoc (const struct toc *tp)
{
  (void)tp;
  ed_line ("minitoc", NULL);
}

/*!
 * @brief Emits the opening tag of a verbatim block.
 * @param[in] tag_end  End tag of the block.
 * @param[in] ptmargin Receiver of the current margin (unused).
 */
void ed_verbatim_start (enum tag tag_end, int *ptmargin)
{
  const char *name;

  (void)ptmargin;
  switch (tag_end)
    {
    case TAG_ENDEXAMPLE:    name = "example";    break;
    case TAG_ENDSAMPLECODE: name = "samplecode"; break;
    case TAG_ENDHEADERS:    name = "headers";    break;
    case TAG_ENDVERBATIM:   name = "verbatim";   break;
    default:                name = "verbatim";   break;
    }
  ed_line (name, NULL);
}

/*!
 * @brief Emits one verbatim line, re-parsed through make_elements.
 *
 * @param[in] tag_end End tag of the block (unused).
 * @param[in] tmargin Current margin (unused).
 * @param[in] compat  Compatibility string (unused).
 */
void ed_verbatim_line (enum tag tag_end, int tmargin, uchar *compat)
{
  (void)tag_end;
  (void)tmargin;
  (void)compat;
  make_elements (input);
  ed_emit ();
  write_nl ();
}

/*!
 * @brief Emits the closing tag of a verbatim block.
 * @param[in] tag_end End tag of the block.
 */
void ed_verbatim_end (enum tag tag_end)
{
  const char *name;
  switch (tag_end)
    {
    case TAG_ENDEXAMPLE:    name = "endexample";    break;
    case TAG_ENDSAMPLECODE: name = "endsamplecode"; break;
    case TAG_ENDHEADERS:    name = "endheaders";    break;
    case TAG_ENDVERBATIM:   name = "endverbatim";   break;
    default:                name = "endverbatim";   break;
    }
  ed_line (name, NULL);
}

/*!
 * @brief Emits %prototype.
 *
 * The compatibility note is emitted by ed_prototype_end, so this
 * function only emits the opening tag.
 *
 * @param[in] compat Compatibility string.  Not NULL.
 */
void ed_prototype_start (uchar *compat)
{
  (void)compat;
  ed_line ("prototype", NULL);
}

/*!
 * @brief Emits the prototype body, %compat and %endprototype.
 *
 * @param[in] compat Compatibility string.  Not NULL.
 */
void ed_prototype_end (uchar *compat)
{
  ed_emit ();
  if (compat[0] != 0)
    {
      ed_line ("compat", compat);
      compat[0] = 0;
    }
  ed_line ("endprototype", NULL);
}

/*!
 * @brief Emits a canonical %function line.
 *
 * All function names are already collected in tp->title by the
 * first pass (see do_function in emxdoc.c), so one call emits the
 * whole line.
 *
 * @param[in] tp TOC node.  Not NULL.
 */
void ed_function_start (const struct toc *tp)
{
  ed_line ("function", tp->title);
}

/*!
 * @brief Per-name hook.  No-op; the names are already in tp->title.
 *
 * @param[in] tp TOC node (unused).  Not NULL.
 * @param[in] s  Function name (unused).  Not NULL.
 */
void ed_function_function (const struct toc *tp, const uchar *s)
{
  (void)tp;
  (void)s;
}

/*!
 * @brief Emits a canonical %table line.
 * @param[in] do_indent Non-zero to indent.
 * @param[in] widths    Column widths.
 * @param[in] wn        Number of columns.
 */
void ed_table_start (int do_indent, int *widths, int wn)
{
  int i;

  ed_esc ();
  write_string ("table");
  if (do_indent)
    write_string (" indent");
  for (i = 0; i < wn; ++i)
    write_fmt (" %d", widths[i]);
  write_nl ();
}

/*!
 * @brief Emits one table row, re-parsed through make_elements.
 *
 * @param[in] s  Row text with 0xb3 cell separators.  Not NULL.
 * @param[in] wn Number of columns (unused).
 */
void ed_table_line (const uchar *s, int wn)
{
  (void)wn;
  make_elements (s);
  ed_emit ();
  write_nl ();
}

/*! @brief Emits %endtable. */
void ed_table_end (int do_indent)
{
  (void)do_indent;
  ed_line ("endtable", NULL);
}

/*! @brief Emit an HTML fragment anchor (no-op for emxdoc). */
void ed_html_fragment (const uchar *s)
{
  (void)s;
}
