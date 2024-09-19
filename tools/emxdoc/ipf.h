/* ipf.h -- IPF output
   Copyright (c) 1993-1998 Eberhard Mattes

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


#define IPF_DESCRIPTION_INDENT  8

EXTERN int hl_ipf;
EXTERN int hl_ipf_no;

void ipf_begin_link (const uchar *database, int ref);
void ipf_end_link (void);
void ipf_para (void);
void ipf_env_margin (int sp);
void ipf_elements (enum style style);
void ipf_end_env (void);
void ipf_toc_start (void);
void ipf_toc_line (const uchar *s, const struct toc *tp);
void ipf_toc_end (void);
void ipf_heading1 (int level, int ref, int global, unsigned flags);
void ipf_heading2 (const uchar *s);
void ipf_output (const uchar *p, int may_break);
void ipf_margin (int margin);
void ipf_hilite (int new);
void ipf_description (void);
void ipf_enumerate (void);
void ipf_itemize (void);
void ipf_verbatim_start (enum tag tag_end);
void ipf_verbatim_line (void);
void ipf_verbatim_end (enum tag tag_end);
void ipf_description_item (const uchar *s);
void ipf_enumerate_item (void);
void ipf_itemize_item (void);
void ipf_list_item (const uchar *s);
void ipf_prototype_start (void);
void ipf_prototype_end (uchar *compat);
void ipf_start (void);
void ipf_table_start (int do_indent, int *widths, int wn);
void ipf_table_line (const uchar *s, int wn);
void ipf_table_end (int do_indent);
void ipf_index (const uchar *s);
void ipf_see_also_start (void);
void ipf_see_also_word (const uchar *word, const uchar *s);
void ipf_sample_file (const uchar *s);
void ipf_libref_section (const uchar *s);
void ipf_function_start (const struct toc *tp);
void ipf_function_function (const uchar *s);
void ipf_copy (void);
void ipf_minitoc (const struct toc *tp);
void ipf_end (void);
