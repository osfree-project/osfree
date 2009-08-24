/* dynamic string function header

  AUTHOR: Gregory Pietsch

  DESCRIPTION:

  This file contains dynamic string function prototypes for an 
  edlin-style line editor.

  COPYRIGHT NOTICE AND DISCLAIMER:

  Copyright (C) 2003 Gregory Pietsch

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.
*/

#ifndef DYNSTR_H
#define DYNSTR_H

#include "defines.h"

typedef struct STRING_T
{
  char *ptr;
  size_t len, res;
} STRING_T;

/* exported functions */

void DSctor (STRING_T * this);
void DSctor_with_size (STRING_T * this, size_t n, capacity c);
void DScopy_ctor (STRING_T *this, STRING_T *x);
void DSdtor (STRING_T * this);
STRING_T *DScreate (void);
STRING_T *DScreate_with_size (size_t n, capacity c);
STRING_T *DScreate_copy (STRING_T *x);
void DSdestroy (STRING_T * this);
STRING_T *DSappendchar (STRING_T * this, int c, size_t nr);
STRING_T *DSappendcstr (STRING_T * this, char *s, size_t ns);
STRING_T *DSappend (STRING_T * this, STRING_T * str, size_t pos, size_t ns);
STRING_T *DSassignchar (STRING_T * this, int c, size_t n);
STRING_T *DSassigncstr (STRING_T * this, char *s, size_t n);
STRING_T *DSassign (STRING_T * this, STRING_T * str, size_t pos, size_t ns);
STRING_T *DSinsertchar (STRING_T * this, size_t p0, int c, size_t nr);
STRING_T *DSinsertcstr (STRING_T * this, size_t p0, char *s, size_t ns);
STRING_T *DSinsert (STRING_T * this, size_t p0, STRING_T * str, size_t pos,
                    size_t ns);
STRING_T *DSremove (STRING_T * this, size_t p0, size_t nr);
STRING_T *DSreplacechar (STRING_T * this, size_t p0, size_t n0, int c,
                         size_t nr);
STRING_T *DSreplacecstr (STRING_T * this, size_t p0, size_t n0, char *s,
                         size_t ns);
STRING_T *DSreplace (STRING_T * this, size_t p0, size_t n0, STRING_T * str,
                     size_t pos, size_t ns);
int DSget_at (STRING_T * this, size_t p0);
void DSput_at (STRING_T * this, size_t p0, int c);
size_t DScopy (STRING_T * this, char *s, size_t n, size_t p0);
size_t DSfind (STRING_T * this, char *s, size_t p0, size_t n);
size_t DSrfind (STRING_T * this, char *s, size_t p0, size_t n);
size_t DSfind_first_of (STRING_T * this, char *s, size_t p0, size_t n);
size_t DSfind_last_of (STRING_T * this, char *s, size_t p0, size_t n);
size_t DSfind_first_not_of (STRING_T * this, char *s, size_t p0, size_t n);
size_t DSfind_last_not_of (STRING_T * this, char *s, size_t p0, size_t n);
int DScomparechar (STRING_T * this, int c, size_t p0, size_t ns);
int DScomparecstr (STRING_T * this, char *s, size_t p0, size_t ns);
int DScompare (STRING_T * this, STRING_T * str, size_t p0, size_t ns);
char *DScstr (STRING_T * this);
size_t DSlength (STRING_T * this);
void DSresize (STRING_T * this, size_t n, int c);
size_t DSreserve (STRING_T * this);
void DSset_reserve (STRING_T * this, size_t n);
STRING_T *DSsubstr (STRING_T * this, size_t p, size_t n);

#define T               STRING_T
#define TS              DAS
#define Tassign(x,y)    DSassign(x,y,0,NPOS)
#define Tctor(x)        DSctor(x)
#define Tdtor(x)        DSdtor(x)
#define PROTOS_ONLY
#include "dynarray.h"
#undef  T
#undef  TS
#undef  Tassign
#undef  Tctor
#undef  Tdtor
#undef  PROTOS_ONLY

#endif

/* END OF FILE */
