/* latex.h -- LaTeX output
   Copyright (c) 1993-1999 Eberhard Mattes

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


void latex_output (const uchar *p, int may_break);
void latex_start_hilite (void);
void latex_end_hilite (void);
void latex_end_env (void);
void latex_heading1 (void);
void latex_heading2 (const uchar *s);
void latex_description (void);
void latex_enumerate (void);
void latex_itemize (void);
void latex_indent (void);
void latex_list (void);
void latex_verbatim_start (enum tag tag_end);
void latex_verbatim_end (enum tag tag_end);
void latex_description_item (const uchar *s);
void latex_enumerate_item (void);
void latex_itemize_item (void);
void latex_list_item (const uchar *s);
void latex_prototype_start (void);
void latex_prototype_end (uchar *compat);
void latex_index (const uchar *s, int level);
void latex_see_also_start (void);
void latex_see_also_word (const uchar *word, const uchar *s);
void latex_sample_file (const uchar *s);
void latex_libref_section (const uchar *s);
void latex_function_start (const struct toc *tp);
void latex_function_function (const uchar *s);
void latex_copy (void);
void latex_start (void);
void latex_end (void);
