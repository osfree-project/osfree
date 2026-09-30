/*!
 * @file hc30.h
 * @brief HC30 (WinHelp) output emitter for emxdoc.
 *
 * Generates an RTF file for the Microsoft Windows Help Compiler
 * (HC30.EXE) and a corresponding HPJ project file.
 *
 * WinHelp conventions implemented here:
 *   - Each topic starts with a page break (\page).
 *   - Topic metadata is carried in RTF footnotes:
 *       #  context string (required for every topic)
 *       $  topic title (shown in history/search)
 *       K  keywords for the index (semicolon-separated)
 *       +  browse sequence (sequence-name:sequence-number)
 *       A  ALink keywords (for context-sensitive help)
 *       !  topic-entry macro
 *       >  secondary window name
 *       *  build tag
 *   - Hyperlinks are double-underlined text (\uldb) immediately
 *     followed by hidden text (\v) containing the context string.
 *   - Popup links use single underline (\ul) instead of \uldb.
 *
 * @copyright Copyright (C) 2026 osFree Project.
 *            License - see LICENSE in the project root.
 */

void hc30_start (void);
void hc30_end (void);
void hc30_output (const uchar *p, int may_break);
void hc30_start_hilite (void);
void hc30_end_hilite (void);
void hc30_heading1 (int level, int ref, int global, unsigned int flags);
void hc30_heading2 (const uchar *s);
void hc30_description (void);
void hc30_enumerate (void);
void hc30_itemize (void);
void hc30_indent (void);
void hc30_list (void);
void hc30_end_env (void);
void hc30_description_item (const uchar *s);
void hc30_enumerate_item (void);
void hc30_itemize_item (void);
void hc30_list_item (const uchar *s);
void hc30_copy (void);
void hc30_verbatim_start (enum tag tag_end, int *ptmargin);
void hc30_verbatim_line (enum tag tag_end, int tmargin, uchar *compat);
void hc30_verbatim_end (enum tag tag_end);
void hc30_prototype_start (uchar *compat);
void hc30_prototype_end (uchar *compat);
void hc30_toc_start (void);
void hc30_toc_line (const uchar *s, const struct toc *tp);
void hc30_toc_end (void);
void hc30_minitoc (const struct toc *tp);
void hc30_function_start (const struct toc *tp);
void hc30_function_function (const struct toc *tp, const uchar *s);
void hc30_index (const struct toc *tp, const uchar *s, int level);
void hc30_see_also_start (void);
void hc30_see_also_word (const uchar *word, const uchar *s);
void hc30_see_also_end (const uchar *s);
void hc30_sample_file (const uchar *s);
void hc30_libref_section (const uchar *s);
void hc30_table_start (int do_indent, int *widths, int wn);
void hc30_table_line (const uchar *s, int wn);
void hc30_table_end (int do_indent);
void hc30_html_fragment (const uchar *s);
