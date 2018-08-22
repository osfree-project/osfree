/*    
	buffer.c	1.4
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

 */

#include <stdlib.h>
#include "rc.h"
#include "buffer.h"
#include "string.h"

/* NOTE: NULL happens to be a resource id */
#define IS_RESID(x) (((unsigned long)(x) & 0xFFFF0000L) == 0)

strp str_alloc(unsigned size)
{
  return get_mem(size);
}

void str_free(strp s)
{
  if (!IS_RESID(s)) /* NULL is a resource id, so it won't be freed */
    free_mem(s);
}

strp str_dup(const char *s1)
{
  strp s;
  if (IS_RESID(s1))
    return (strp)s1;
  s = str_alloc(strlen(s1) + 1);
  strcpy(s, s1);
  return s;
}

buffp buff_alloc(unsigned size)
{
  buffp b;
  ALLOC(b, struct binarybuffer);
  b->alloc = size;
  b->len = 0;
  b->buffer = get_mem(size);
  return b;
}

void buff_free(buffp b)
{
  if (!b)
    return;

  if (b->buffer)
    free_mem(b->buffer);
  free_mem(b);
}

void *buff_ind(buffp b)
{
  void *p;
  if (!b)
    return NULL;

  p = b->buffer;
  free_mem(b);
  return p;
}

buffp buff_unind(void *p, unsigned size)
{
  buffp b;
  ALLOC(b, struct binarybuffer);
  b->alloc = b->len = size;
  b->buffer = p;
  return b;
}

buffp buff_realloc(buffp b, unsigned size)
{
  void *p;

  if (!b || !(b->buffer))
    return buff_alloc(size);

  if (!size)
    {
      buff_free(b);
      return NULL;
    }

  p = get_mem(size);
  if (!p)
    {
      buff_free(b);
      return NULL;
    }

  if (size > b->alloc)
    memcpy(p, b->buffer, b->alloc);
  else
    memcpy(p, b->buffer, size);
  free_mem(b->buffer);
  b->buffer = p;
  b->alloc = size;
  return b;
}

buffp buff_append_byte(buffp b, char c)
{
  if (!b)
    b = buff_alloc(ALLOCSIZE);

  if (b->len >= b->alloc)
    buff_realloc(b, b->alloc + ALLOCSIZE);

  ((char *)(b->buffer))[b->len++] = c;
  return b;
}

buffp buff_cat(buffp b1, buffp b2)
{
  buffp b;

  if (!b1 || !(b1->buffer))
    return buff_dup(b2);

  if (!b2 || !(b2->buffer))
    return buff_dup(b1);

  b = buff_alloc(b1->alloc + b2->alloc);
  memcpy(b->buffer, b1->buffer, b1->len);
  memcpy(((void *)(((char *)b->buffer) + b1->len)), b2->buffer, b2->len);
  b->len = b1->len + b2->len;
  return b;
}

buffp buff_dup(buffp b1)
{
  buffp b;
  if (!b1)
    return NULL;

  b = buff_alloc(b1->alloc);
  b->len = b1->len;
  memcpy(b->buffer, b1->buffer, b1->len);
  return b;
}

resp res_new(void)
{
  resp r;
  ALLOC(r, RESOURCE);
  r->type = NULL;
  r->name = NULL;
  r->b = NULL;
  r->flags = 0;
  return r;
}

void res_free(resp r)
{
  buff_free(r->b);
  free_mem(r);
}
