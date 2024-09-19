/* html.h -- HTML output
   Copyright (c) 1999 Eberhard Mattes

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


void html_output (const uchar *p, int may_break);
void html_elements (enum style style);
void html_start_hilite (void);
void html_end_hilite (void);
void html_end_env (void);
void html_toc_start (void);
void html_toc_line (const uchar *s, const struct toc *tp);
void html_toc_end (void);
void html_heading1 (int ref);
void html_heading2 (const uchar *s);
void html_description (void);
void html_enumerate (void);
void html_itemize (void);
void html_indent (void);
void html_list (void);
void html_verbatim_start (enum tag tag_end);
void html_verbatim_line (void);
void html_verbatim_end (enum tag tag_end);
void html_description_item (const uchar *s);
void html_enumerate_item (void);
void html_itemize_item (void);
void html_list (void);
void html_list_item (const uchar *s);
void html_copy (void);
void html_start (void);
void html_end (void);
void html_minitoc (const struct toc *tp);
void html_prototype_start (void);
void html_prototype_end (uchar *compat);
void html_index (const struct toc *tp, const uchar *s, int level);
void html_see_also_start (void);
void html_see_also_word (const uchar *word, const uchar *s);
void html_sample_file (const uchar *s);
void html_libref_section (const uchar *s);
void html_function_start (const struct toc *tp);
void html_function_function (const struct toc *tp, const uchar *s);
void html_fragment (const uchar *s);
