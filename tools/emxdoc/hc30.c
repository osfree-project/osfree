/*!
 * @file hc30.c
 * @brief HC30 (WinHelp) output emitter for emxdoc.
 *
 * Generates an RTF file for the Microsoft Windows Help Compiler
 * (HC30.EXE).  The companion HPJ project file is written by hand
 * and is not produced by this emitter; the RTF file is the only
 * generated artifact.
 *
 * WinHelp conventions implemented here:
 *   - Each topic starts with a page break (\page).
 *   - Topic metadata is carried in RTF footnotes:
 *       *  build tag (optional)
 *       #  context string (required for every topic)
 *       $  topic title (shown in history/search)
 *       K  keywords for the index (semicolon-separated)
 *       A  ALink keywords (for context-sensitive help)
 *       +  browse sequence (sequence-name:sequence-number)
 *       !  topic-entry macro
 *       >  secondary window name
 *   - Hyperlinks are double-underlined text (\uldb) immediately
 *     followed by hidden text (\v) containing the context string.
 *     The context string is derived deterministically from the
 *     reference number as "topic_<ref>"; the same rule is applied
 *     when a topic's # footnote is emitted.
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
#include "hc30.h"

/* ------------------------------------------------------------------ */
/*  State                                                             */
/* ------------------------------------------------------------------ */

/*! @brief Context string for the current topic. */
static uchar hc30_context[256];
/*! @brief Title for the current topic. */
static uchar hc30_title[256];
/*! @brief Keywords for the current topic (semicolon-separated). */
static uchar hc30_keywords[1024];
/*! @brief ALink keywords for the current topic. */
static uchar hc30_alink[512];
/*! @brief Browse sequence for the current topic. */
static uchar hc30_browse[128];
/*! @brief Topic-entry macro for the current topic. */
static uchar hc30_macro[256];
/*! @brief Secondary window name for the current topic. */
static uchar hc30_window[64];
/*! @brief Build tag for the current topic. */
static uchar hc30_buildtag[64];
/*! @brief Non-zero if we are inside a topic body. */
static int hc30_in_topic;
/*! @brief Non-zero if a page break is pending. */
static int hc30_need_page_break;
/*! @brief Current heading level for the topic. */
static int hc30_topic_level;
/*! @brief Reference number of the current topic. */
static int hc30_topic_ref;

/* ------------------------------------------------------------------ */
/*  Helpers                                                           */
/* ------------------------------------------------------------------ */

/*!
 * @brief Derive a context string from a reference number.
 *
 * The rule is fixed: "topic_<ref>".  The same rule is used when
 * a topic's # footnote is emitted and when a hyperlink is expanded
 * in hc30_copy(), so the two always agree without any shared table.
 *
 * @param[out] dst  Destination buffer.  Not NULL.
 * @param[in]  size Size of @p dst in bytes.  Must be > 0.
 * @param[in]  ref  Reference number.
 */
static void hc30_ref_context (uchar *dst, size_t size, int ref)
{
  snprintf ((char *)dst, size, "topic_%d", ref);
}

/*!
 * @brief Build a context string from a title.
 *
 * Used when a topic has no reference number (for example, an
 * anonymous heading).  Only alphanumerics and underscores are
 * kept; spaces become underscores.
 *
 * @param[out] dst  Destination buffer.  Not NULL.
 * @param[in]  src  Source string.  Not NULL.
 * @param[in]  size Size of @p dst in bytes.  Must be > 0.
 */
static void hc30_make_context (uchar *dst, const uchar *src, size_t size)
{
  size_t i, n = 0;
  for (i = 0; src[i] != 0 && n < size - 1; ++i)
    {
      uchar c = src[i];
      if (isalnum (c) || c == '_')
        dst[n++] = c;
      else if (c == ' ' && n > 0 && dst[n-1] != '_')
        dst[n++] = '_';
    }
  dst[n] = 0;
  if (n == 0)
    strcpy ((char *)dst, "topic");
}

/* ------------------------------------------------------------------ */
/*  Low-level RTF output                                              */
/* ------------------------------------------------------------------ */

/*!
 * @brief Write a single character with RTF escaping.
 *
 * @param[in] c Character to write.
 */
static void hc30_rtf_char (uchar c)
{
  switch (c)
    {
    case '\\':
    case '{':
    case '}':
      write_string ("\\");
      write_nstring (&c, 1);
      break;
    case '\n':
      write_string ("\\par ");
      break;
    case '\r':
      break;
    case '\t':
      write_string ("\\tab ");
      break;
    default:
      if (c >= 0x80)
        write_fmt ("\\'%02x", c);
      else
        write_nstring (&c, 1);
      break;
    }
}

/*!
 * @brief Write a string with RTF escaping.
 *
 * @param[in] p String to write.  Not NULL.
 */
static void hc30_rtf_string (const uchar *p)
{
  while (*p != 0)
    hc30_rtf_char (*p++);
}

/*!
 * @brief Emit a footnote group for the current topic.
 *
 * @param[in] tag  Footnote character ('#', '$', 'K', 'A', '+', '!', '>', '*').
 * @param[in] text Footnote text.  Not NULL.
 */
static void hc30_footnote (char tag, const uchar *text)
{
  write_string ("{\\footnote ");
  write_nstring ((const uchar *)&tag, 1);
  hc30_rtf_string (text);
  write_string ("}");
}

/* ------------------------------------------------------------------ */
/*  Highlighting                                                      */
/* ------------------------------------------------------------------ */

/*! @brief Opens highlighting. */
void hc30_start_hilite (void)
{
  int diff = hl_stack[hl_sp] & ~hl_stack[hl_sp - 1];

  if (diff & HL_BF) write_string ("\\b ");
  if (diff & HL_SL) write_string ("\\i ");
  if (diff & HL_UL) write_string ("\\ul ");
  if (diff & HL_TT) write_string ("\\f1 ");
}

/*! @brief Closes highlighting. */
void hc30_end_hilite (void)
{
  write_string ("\\plain ");
}

/* ------------------------------------------------------------------ */
/*  Document prologue / epilogue                                      */
/* ------------------------------------------------------------------ */

/*! @brief Initializes HC30 output, writes the RTF header. */
void hc30_start (void)
{
  write_line ("{\\rtf1\\ansi\\ansicpg1252\\deff0");
  write_line ("{\\fonttbl");
  write_line ("{\\f0\\froman Times New Roman;}");
  write_line ("{\\f1\\fmodern Courier New;}");
  write_line ("}");
  write_line ("{\\colortbl;\\red0\\green0\\blue0;}");
  write_line ("\\viewkind4\\uc1\\pard\\lang1033\\f0\\fs20");

  hc30_context[0] = 0;
  hc30_title[0] = 0;
  hc30_keywords[0] = 0;
  hc30_alink[0] = 0;
  hc30_browse[0] = 0;
  hc30_macro[0] = 0;
  hc30_window[0] = 0;
  hc30_buildtag[0] = 0;
  hc30_in_topic = 0;
  hc30_need_page_break = 0;
  hc30_topic_level = 0;
  hc30_topic_ref = 0;
}

/*! @brief Finalizes HC30 output, closes the RTF group. */
void hc30_end (void)
{
  write_line ("}");
}

/* ------------------------------------------------------------------ */
/*  Text output                                                       */
/* ------------------------------------------------------------------ */

/*!
 * @brief Emit a text fragment with RTF escaping.
 *
 * @param[in] p         String.  Not NULL.
 * @param[in] may_break Non-zero if a line break may be inserted.
 */
void hc30_output (const uchar *p, int may_break)
{
  (void)may_break;
  hc30_rtf_string (p);
}

/* ------------------------------------------------------------------ */
/*  Headings                                                          */
/* ------------------------------------------------------------------ */

/*!
 * @brief Emit a heading prologue (RTF paragraph style).
 *
 * @param[in] level  Heading level.
 * @param[in] ref    Reference number.
 * @param[in] global Non-zero if global (unused).
 * @param[in] flags  Heading flags (unused).
 */
void hc30_heading1 (int level, int ref, int global, unsigned int flags)
{
  (void)global;
  (void)flags;

  if (hc30_need_page_break)
    {
      write_line ("\\page");
      hc30_need_page_break = 0;
    }

  hc30_topic_level = level;
  hc30_topic_ref = ref;

  switch (level)
    {
    case 1:  write_string ("\\pard\\sb240\\sa120\\b\\fs28 "); break;
    case 2:  write_string ("\\pard\\sb240\\sa120\\b\\fs24 "); break;
    case 3:  write_string ("\\pard\\sb240\\sa120\\b\\fs22 "); break;
    default: write_string ("\\pard\\sb240\\sa120\\b\\fs20 "); break;
    }

  hc30_in_topic = 0;
}

/*!
 * @brief Emit a heading body and start the topic.
 *
 * @param[in] s Heading text.  Not NULL.
 */
void hc30_heading2 (const uchar *s)
{
  strncpy ((char *)hc30_title, (const char *)s, sizeof (hc30_title) - 1);
  hc30_title[sizeof (hc30_title) - 1] = 0;

  hc30_rtf_string (s);
  write_string ("\\b0\\fs20 ");

  if (hc30_topic_ref != 0)
    hc30_ref_context (hc30_context, sizeof (hc30_context), hc30_topic_ref);
  else
    hc30_make_context (hc30_context, s, sizeof (hc30_context));

  if (hc30_buildtag[0] != 0)
    {
      hc30_footnote ('*', hc30_buildtag);
      hc30_buildtag[0] = 0;
    }

  hc30_footnote ('#', hc30_context);
  hc30_footnote ('$', hc30_title);

  if (hc30_keywords[0] != 0)
    {
      hc30_footnote ('K', hc30_keywords);
      hc30_keywords[0] = 0;
    }

  if (hc30_alink[0] != 0)
    {
      hc30_footnote ('A', hc30_alink);
      hc30_alink[0] = 0;
    }

  if (hc30_browse[0] != 0)
    {
      hc30_footnote ('+', hc30_browse);
      hc30_browse[0] = 0;
    }

  if (hc30_macro[0] != 0)
    {
      hc30_footnote ('!', hc30_macro);
      hc30_macro[0] = 0;
    }

  if (hc30_window[0] != 0)
    {
      hc30_footnote ('>', hc30_window);
      hc30_window[0] = 0;
    }

  write_line ("\\par");
  hc30_in_topic = 1;
  hc30_need_page_break = 1;
}

/* ------------------------------------------------------------------ */
/*  Environments (lists)                                              */
/* ------------------------------------------------------------------ */

/*! @brief Begins a description environment (no-op for HC30). */
void hc30_description (void)   { }

/*! @brief Begins an enumerate environment (no-op for HC30). */
void hc30_enumerate (void)     { }

/*! @brief Begins an itemize environment (no-op for HC30). */
void hc30_itemize (void)       { }

/*! @brief Begins an indent environment (no-op for HC30). */
void hc30_indent (void)        { }

/*! @brief Begins a list environment (no-op for HC30). */
void hc30_list (void)          { }

/*! @brief Closes the current environment. */
void hc30_end_env (void)
{
  write_string ("\\pard ");
}

/*!
 * @brief Emit a description-list item (term).
 *
 * @param[in] s Term.  Not NULL.
 */
void hc30_description_item (const uchar *s)
{
  write_string ("\\pard\\li720\\fi-720\\b ");
  hc30_rtf_string (s);
  write_string ("\\b0\\tab ");
}

/*! @brief Emit an enumerate-list item. */
void hc30_enumerate_item (void)
{
  write_string ("\\pard\\li720\\fi-360 ");
}

/*! @brief Emit an itemize-list item. */
void hc30_itemize_item (void)
{
  write_string ("\\pard\\li720\\fi-360 \\bullet\\tab ");
}

/*!
 * @brief Emit a list item (list environment).
 *
 * @param[in] s Term.  Not NULL.
 */
void hc30_list_item (const uchar *s)
{
  write_string ("\\pard\\li720\\fi-720\\b ");
  hc30_rtf_string (s);
  write_string ("\\b0\\tab ");
}

/* ------------------------------------------------------------------ */
/*  Paragraph (copy) with hyperlink support                           */
/* ------------------------------------------------------------------ */

/*!
 * @brief Emit the accumulated elements as an RTF paragraph.
 *
 * Handles inline styles, spaces, breaks, and hyperlinks.  A word
 * with ep->n != 0 is a reference: its context string is derived
 * from ep->wp->ref by hc30_ref_context(), which is the same rule
 * used when the # footnote is emitted in hc30_heading2().
 */
void hc30_copy (void)
{
  const struct element *ep;
  int style_sp = 0;
  enum style style_stack[STYLE_STACK_SIZE];

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

            if (ep->n != 0)
              {
                uchar ctx[64];
                hc30_ref_context (ctx, sizeof (ctx), ep->wp->ref);
                write_string ("{\\uldb ");
                hc30_rtf_string (ep->wp->str);
                write_string ("}{\\v ");
                hc30_rtf_string (ctx);
                write_string ("}");
              }
            else if (sty != STYLE_NORMAL)
              {
                switch (sty)
                  {
                  case STYLE_BOLD:      write_string ("{\\b "); break;
                  case STYLE_SLANTED:   write_string ("{\\i "); break;
                  case STYLE_UNDERLINE: write_string ("{\\ul "); break;
                  case STYLE_TTY:       write_string ("{\\f1 "); break;
                  default:              break;
                  }
                hc30_rtf_string (ep->wp->str);
                write_string ("}");
              }
            else
              hc30_rtf_string (ep->wp->str);
          }
          break;

        case EL_SPACE:
          {
            int n;
            for (n = 0; n < ep->n; ++n)
              write_string (" ");
          }
          break;

        case EL_BREAK:
          if (ep->n)
            write_line ("\\par");
          else
            write_string (" ");
          break;

        case EL_STYLE:
          if (style_sp + 1 < STYLE_STACK_SIZE)
            style_stack[++style_sp] = ep->n;
          break;

        case EL_ENDSTYLE:
          if (style_sp > 0)
            --style_sp;
          break;

        default:
          break;
        }
    }
  write_line ("\\par");
}

/* ------------------------------------------------------------------ */
/*  Verbatim / prototype                                              */
/* ------------------------------------------------------------------ */

/*!
 * @brief Starts a verbatim block.
 *
 * @param[in] tag_end  End tag of the block (unused).
 * @param[in] ptmargin Margin pointer (unused).
 */
void hc30_verbatim_start (enum tag tag_end, int *ptmargin)
{
  (void)tag_end;
  (void)ptmargin;
  write_string ("\\pard\\f1\\fs18 ");
}

/*!
 * @brief Emits one verbatim-block line.
 *
 * @param[in] tag_end End tag of the block (unused).
 * @param[in] tmargin Current margin (unused).
 * @param[in] compat  Compatibility string (unused).
 */
void hc30_verbatim_line (enum tag tag_end, int tmargin, uchar *compat)
{
  (void)tag_end;
  (void)tmargin;
  (void)compat;
  hc30_rtf_string (input);
  write_line ("\\par");
}

/*!
 * @brief Ends a verbatim block.
 *
 * @param[in] tag_end End tag of the block (unused).
 */
void hc30_verbatim_end (enum tag tag_end)
{
  (void)tag_end;
  write_string ("\\f0\\fs20 ");
  para_flag = TRUE;
}

/*!
 * @brief Starts a prototype block.
 *
 * @param[in] compat Compatibility string (unused).
 */
void hc30_prototype_start (uchar *compat)
{
  (void)compat;
  write_string ("\\pard\\f1\\fs18 ");
}

/*!
 * @brief Ends a prototype block.
 *
 * @param[in] compat Compatibility string (unused).
 */
void hc30_prototype_end (uchar *compat)
{
  (void)compat;
  hc30_copy ();
  write_string ("\\f0\\fs20 ");
  para_flag = TRUE;
}

/* ------------------------------------------------------------------ */
/*  TOC / minitoc                                                     */
/* ------------------------------------------------------------------ */

/*! @brief Begin the table of contents (no-op). */
void hc30_toc_start (void) { }

/*!
 * @brief Emit one TOC line (no-op; WinHelp builds its own TOC).
 *
 * @param[in] s  Section number (unused).
 * @param[in] tp TOC node (unused).
 */
void hc30_toc_line (const uchar *s, const struct toc *tp)
{
  (void)s;
  (void)tp;
}

/*! @brief End the table of contents (no-op). */
void hc30_toc_end (void) { }

/*!
 * @brief Emit a mini table of contents (no-op).
 *
 * @param[in] tp Current TOC entry (unused).
 */
void hc30_minitoc (const struct toc *tp) { (void)tp; }

/* ------------------------------------------------------------------ */
/*  Functions                                                         */
/* ------------------------------------------------------------------ */

/*!
 * @brief Begin a function block (no-op).
 *
 * @param[in] tp TOC node (unused).
 */
void hc30_function_start (const struct toc *tp)
{
  (void)tp;
}

/*!
 * @brief Emit one function name (no-op).
 *
 * @param[in] tp TOC node (unused).
 * @param[in] s  Function name (unused).
 */
void hc30_function_function (const struct toc *tp, const uchar *s)
{
  (void)tp;
  (void)s;
}

/* ------------------------------------------------------------------ */
/*  Index / keywords                                                  */
/* ------------------------------------------------------------------ */

/*!
 * @brief Register a keyword for the current topic.
 *
 * @param[in] tp    TOC entry (unused).
 * @param[in] s     Keyword text.  Not NULL.
 * @param[in] level Index level (unused).
 */
void hc30_index (const struct toc *tp, const uchar *s, int level)
{
  (void)tp;
  (void)level;
  if (hc30_keywords[0] != 0)
    strcat ((char *)hc30_keywords, ";");
  strncat ((char *)hc30_keywords, (const char *)s,
           sizeof (hc30_keywords) - strlen ((char *)hc30_keywords) - 1);
}

/* ------------------------------------------------------------------ */
/*  See also / sample / libref                                        */
/* ------------------------------------------------------------------ */

/*! @brief Starts the "See also" block. */
void hc30_see_also_start (void)
{
  write_string ("\\pard\\b See also: \\b0 ");
}

/*!
 * @brief Emit one "See also" reference.
 *
 * @param[in] word Reference text.  Not NULL.
 * @param[in] s    Remaining list text (unused).
 */
void hc30_see_also_word (const uchar *word, const uchar *s)
{
  (void)s;
  hc30_rtf_string (word);
}

/*!
 * @brief Finishes the "See also" block.
 *
 * @param[in] s Accumulated reference list (unused).
 */
void hc30_see_also_end (const uchar *s)
{
  (void)s;
  write_line ("\\par");
}

/*!
 * @brief Emit the reference to a sample file.
 *
 * @param[in] s File name.  Not NULL.
 */
void hc30_sample_file (const uchar *s)
{
  write_string ("\\pard\\b Example: \\b0 See ");
  hc30_rtf_string (s);
  write_line ("\\par");
}

/*!
 * @brief Emit a libref section heading.
 *
 * @param[in] s Section title.  Not NULL.
 */
void hc30_libref_section (const uchar *s)
{
  write_string ("\\pard\\b ");
  hc30_rtf_string (s);
  write_string (":\\b0 ");
}

/* ------------------------------------------------------------------ */
/*  Tables (HC30 does not support RTF tables)                         */
/* ------------------------------------------------------------------ */

/*!
 * @brief Begin a table block (no-op).
 *
 * @param[in] do_indent Non-zero to indent (unused).
 * @param[in] widths    Column widths (unused).
 * @param[in] wn        Number of columns (unused).
 */
void hc30_table_start (int do_indent, int *widths, int wn)
{
  (void)do_indent;
  (void)widths;
  (void)wn;
}

/*!
 * @brief Emit one table row (no-op).
 *
 * @param[in] s  Row text (unused).
 * @param[in] wn Number of columns (unused).
 */
void hc30_table_line (const uchar *s, int wn)
{
  (void)s;
  (void)wn;
}

/*!
 * @brief End a table block (no-op).
 *
 * @param[in] do_indent Non-zero if the table was indented (unused).
 */
void hc30_table_end (int do_indent)
{
  (void)do_indent;
}

/*!
 * @brief Emit an HTML fragment anchor (no-op for HC30).
 *
 * @param[in] s Anchor name (unused).
 */
void hc30_html_fragment (const uchar *s)
{
  (void)s;
}
