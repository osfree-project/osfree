/* xref.h -- Manage cross references
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


void write_keyword (const uchar *p);
struct word *define_label (const uchar *p, int ref, const char *msg);
struct word *use_reference (const uchar *p);
void make_global (const uchar *name);
void keywords_keyword (const uchar *s);
void keywords_start (const char *fname);
void read_xref (const char *fname, struct toc *th);
