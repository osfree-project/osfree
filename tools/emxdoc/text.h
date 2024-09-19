/* text.h -- Text output
   Copyright (c) 1993-1995 Eberhard Mattes

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


void text_output (const uchar *p, int may_break);
void text_heading1 (void);
void text_heading2 (uchar *s);
void text_see_also_start (void);
void text_see_also_end (const uchar *s);
void text_description_item (const uchar *s);
void text_enumerate_item (void);
void text_itemize_item (void);
void text_list_item (const uchar *s);
void text_copy (void);
void text_verbatim_start (enum tag tag_end, int *ptmargin);
void text_verbatim_line (enum tag tag_end, int tmargin, uchar *compat);
void text_function (void);
void text_prototype_start (uchar *compat);
void text_prototype_end (void);
void text_toc_line (const uchar *s, const struct toc *tp);
void text_table_start (int do_indent, int *ptmargin);
void text_table_line (const uchar *s, int tmargin);
void text_sample_file (const uchar *s);
void text_libref_section (const uchar *s);
void text_hyphenation (const char *name);
