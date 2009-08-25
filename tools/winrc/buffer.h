/*************************************************************************

	buffer.h	1.3
	binarybuffer and other support structures for rc
  
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

/* I can't write this in C++... :,(
 *
 * Many of the structures in this file have corresponding functions that
 * go with them.  Generally, they take a pointer to the associated
 * structure (occassionally known as 'this', but you'll never "C" that
 * in here).  We try to track ownership through the yacc script...
 */

/* Some general things tend to happen quite frequently with many of these:
 *
 * foo_alloc: allocate a new foo.  Returns a foop (pointer to foo).
 * foo_free: free a foo.  Takes a foop, returns void.
 * foo_dup: make a copy of a foo.  The new foo is independent of the old one.
 *            Takes a foop, returns a different foop.
 * foo_cat: puts two foos together.  Takes two foops, returns one independent
 *            foop.
 */

typedef char *strp;

strp str_alloc(unsigned size);
void str_free(strp s);
strp str_dup(const char *s);

typedef struct binarybuffer
{
  unsigned len;
  unsigned alloc;
  void *buffer;
} *buffp;

/* Things to work with binary buffers */
#define ALLOCSIZE 32
buffp buff_alloc(unsigned size);
void buff_free(buffp b);
void *buff_ind(buffp b); /* Releases binarybuffer struct but leaves buffer intact */
buffp buff_unind(void *p, unsigned size); /* Makes buffp from arbitrary memory */
buffp buff_realloc(buffp b, unsigned size);

buffp buff_append_byte(buffp b, char c);
buffp buff_cat(buffp b1, buffp b2);
buffp buff_dup(buffp b1);
buffp buff_load(char *filename, int lookHere);

#define BUFF_STRUCT(b, v, t) (b) = buff_alloc(sizeof(t)); (b)->len = sizeof(t); (v) = (t*)(b)->buffer;

typedef struct tagRESOURCE
{
  const char *type;
  const char *name;
  buffp b;
  unsigned flags;
} RESOURCE, *resp;

resp res_new(void);
void res_free(resp);

struct maskvals
{
  int and;
  int or;
};

struct xyvals
{
  int x;
  int y;
};

struct dimvals
{
  struct xyvals corner;
  struct xyvals extent;
};

enum endianvals{little, big};

struct binflagvals
{
  int align;
  enum endianvals endian;
};

extern struct binflagvals defbinflags;

enum userrestype {shortint, longint, string, binary};

struct userresbuffer
{
  struct userresbuffer *next;
  enum userrestype t;
  union userresidata
  {
    unsigned long i;
    strp s;
    buffp b;
  } b;
};
