/*    
	define.c	1.3
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


For more information about the Willows Twin Libraries.

	http://www.willows.com	

To send email to the maintainer of the Willows Twin Libraries.

	mailto:twin@willows.com 

*	rc support for #define macros

 */


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "define.h"
#include "rc.h"
#include "buffer.h"

DEFINESTRUCT *dTop = NULL;
DEFINESTRUCT **defineTop = &dTop;

DEFINESTRUCT **internal_find_define(char *key, DEFINESTRUCT **start);

DEFINESTRUCT **internal_find_define(char *key, DEFINESTRUCT **start)
{
  register int i;

  if (!*start)
    return start;

  if (!(i = strcmp((*start)->key, key)))
    return start;

  if (i > 0)
    return internal_find_define(key, &((*start)->less));
  else
    return internal_find_define(key, &((*start)->more));
}

DEFINESTRUCT *create_define(char *key)
{
  DEFINESTRUCT *pNew;
  DEFINESTRUCT **pInsert = internal_find_define(key, defineTop);

  if (*pInsert)
    return *pInsert;

  pNew = (DEFINESTRUCT *)get_mem(sizeof(DEFINESTRUCT));
  pNew->key = str_dup(key);
  pNew->less = NULL;
  pNew->more = NULL;
  pNew->value = NULL;
  *pInsert = pNew;
  return pNew;
}

char *match_define(char *key)
{
  DEFINESTRUCT **pMatch = internal_find_define(key, defineTop);
  if (*pMatch)
    return (*pMatch)->value;
  return NULL;
}

void define_define(char *key, char *value)
{
  DEFINESTRUCT *pDefine = create_define(key);
  if (value)
    pDefine->value = str_dup(value);
  else
    pDefine->value = str_dup("");
}

void undefine_define(char *key)
{
  DEFINESTRUCT **pMatch = internal_find_define(key, defineTop);

  if (!*pMatch)
    return;

  str_free((*pMatch)->value);
  (*pMatch)->value = NULL;
}

void delete_define(DEFINESTRUCT *pDefine)
{
  if (!pDefine)
    return;

  delete_define(pDefine->less);
  delete_define(pDefine->more);
  str_free(pDefine->value);
  str_free(pDefine->key);
  free_mem(pDefine);
}

void clean_defines(void)
{
  delete_define(dTop);
  dTop = NULL;
}
