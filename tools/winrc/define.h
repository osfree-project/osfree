/*************************************************************************

	define.h	1.2
	prototypes and definitions for rc #define support

  
    Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.

The maintainer of the Willows TWIN Libraries may be reached (Email) 
at the address twin@willows.com	

**************************************************************************/

typedef struct tagDEFINESTRUCT
{
  struct tagDEFINESTRUCT *less;
  struct tagDEFINESTRUCT *more;
  char *key;
  char *value;
} DEFINESTRUCT;

extern DEFINESTRUCT **defineTop;

DEFINESTRUCT *create_define(char *key);
char *match_define(char *key);
void define_define(char *key, char *value);
void undefine_define(char *key);
void delete_define(DEFINESTRUCT *pDefine);
void clean_defines(void);
