/* dynarray.h - dynamic array header 

  DESCRIPTION:

  The functions in this header can be used to create a system for
  handling dynamic arrays with just about any type as the base type.

  The following macros may be predefined:

  PROTOS_ONLY - Return prototypes only.
  Tctor - Use this function to construct a type T (Initialize pointers, etc.)
  Tdtor - Use this function to give an instance of type T its last rites
          (deallocate memory, etc.)
  Tassign - Use this function to assign an instance of a type T to another.
            (If pointers are involved, assigning may have some additional
                         steps beyond just using operator=.)
  
  The following macros must be predefined:

  T - the base type for the dynamic array
  TS - a "slug" prefix for all the functions in the header.

  Because this file may be needed more than once or #included more than once
  in the same file with different parameters, it does not have #include
  guards.

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

#include <stdlib.h>
#include <stdio.h>
#include "defines.h"

#define _NM(y,x)                _VAL(y,x)
#define _VAL(y,x)               y ## x

#define _TS_ARRAY_T             _NM(TS, _ARRAY_T)
#define _TS_Grow                _NM(TS, _Grow)
#define _TS_Tidy                _NM(TS, _Tidy)
#define _TS_Xinv                _NM(TS, _Xinv)
#define _TS_Xlen                _NM(TS, _Xlen)
#define _TS_Xran                _NM(TS, _Xran)
#define _TS_append              _NM(TS, _append)
#define _TS_assign              _NM(TS, _assign)
#define _TS_base                _NM(TS, _base)
#define _TS_copy_ctor           _NM(TS, _copy_ctor)
#define _TS_create              _NM(TS, _create)
#define _TS_create_copy         _NM(TS, _create_copy)
#define _TS_create_from_ptr     _NM(TS, _create_from_ptr)
#define _TS_create_with_size    _NM(TS, _create_with_size)
#define _TS_ctor                _NM(TS, _ctor)
#define _TS_ctor_from_ptr       _NM(TS, _ctor_from_ptr)
#define _TS_ctor_with_size      _NM(TS, _ctor_with_size)
#define _TS_destroy             _NM(TS, _destroy)
#define _TS_dtor                _NM(TS, _dtor)
#define _TS_get_at              _NM(TS, _get_at)
#define _TS_insert              _NM(TS, _insert)
#define _TS_length              _NM(TS, _length)
#define _TS_put_at              _NM(TS, _put_at)
#define _TS_remove              _NM(TS, _remove)
#define _TS_reserve             _NM(TS, _reserve)
#define _TS_resize              _NM(TS, _resize)
#define _TS_set_reserve         _NM(TS, _set_reserve)
#define _TS_subarray            _NM(TS, _subarray)
#define _TS_swap                _NM(TS, _swap)

#ifdef PROTOS_ONLY

/* types */

/* _ARRAY_T: The array type.  */
typedef struct _TS_ARRAY_T
{
  T *_Ptr;
  size_t _Len, _Res;
} _TS_ARRAY_T;

/* Function prototypes.  */
_TS_ARRAY_T *_TS_append (_TS_ARRAY_T *, T *, size_t, size_t);
_TS_ARRAY_T *_TS_assign (_TS_ARRAY_T *, T *, size_t, size_t);
T *_TS_base (_TS_ARRAY_T *);
void _TS_copy_ctor (_TS_ARRAY_T *, _TS_ARRAY_T *);
_TS_ARRAY_T *_TS_create (void);
_TS_ARRAY_T *_TS_create_copy (_TS_ARRAY_T *);
_TS_ARRAY_T *_TS_create_from_ptr (T *, size_t);
_TS_ARRAY_T *_TS_create_with_size (size_t, capacity);
void _TS_ctor (_TS_ARRAY_T *);
void _TS_ctor_from_ptr (_TS_ARRAY_T *, T *, size_t);
void _TS_ctor_with_size (_TS_ARRAY_T *, size_t, capacity);
void _TS_destroy (_TS_ARRAY_T *);
void _TS_dtor (_TS_ARRAY_T *);
T *_TS_get_at (_TS_ARRAY_T *, size_t);
_TS_ARRAY_T *_TS_insert (_TS_ARRAY_T *, size_t, T *, size_t, size_t);
size_t _TS_length (_TS_ARRAY_T *);
void _TS_put_at (_TS_ARRAY_T *, size_t, T *);
_TS_ARRAY_T *_TS_remove (_TS_ARRAY_T *, size_t, size_t);
size_t _TS_reserve (_TS_ARRAY_T *);
void _TS_resize (_TS_ARRAY_T *, size_t, T *);
void _TS_set_reserve (_TS_ARRAY_T *, size_t);
_TS_ARRAY_T *_TS_subarray (_TS_ARRAY_T *, _TS_ARRAY_T *, size_t, size_t);
void _TS_swap (_TS_ARRAY_T *, _TS_ARRAY_T *);

#else

#ifndef Tctor
#define Tctor(x)
#endif
#ifndef Tdtor
#define Tdtor(x)
#endif
#ifndef Tassign
#define Tassign(x,y)    (*(x) = *(y))
#endif

/* functions */

static void
_TS_Xinv (void)
{
  fputs ("Invalid dynamic array argument\n", stderr);
  abort ();
}

static void
_TS_Xlen (void)
{
  fputs ("Length error: dynamic array too long\n", stderr);
  abort ();
}

static void
_TS_Xran (void)
{
  fputs ("Out of range: invalid dynamic array position\n", stderr);
  abort ();
}

/* _Tidy: Tidy up an array either when it's initially constructed
   or right before it is freed.  */
static void
_TS_Tidy (_TS_ARRAY_T * this, int constructed)
{
  size_t i;

  if (constructed && this->_Ptr != 0)
    {
      for (i = 0; i < this->_Len; i++)
        Tdtor (this->_Ptr + i);
      free (this->_Ptr);
    }
  this->_Len = 0;
  this->_Ptr = 0;
  this->_Res = 0;
}

/* _Grow: Reallocate an array to make it size n.  Initialize the unused
   elements in the array to s.  If trim is nonzero, resize the array
   even if the new size is less than the old one. */
static void
_TS_Grow (_TS_ARRAY_T * this, size_t n, T * s, int trim)
{
  size_t os = this->_Ptr == 0 ? 0 : this->_Res;
  size_t i, m, r;
  T *np;

  if (n == 0)
    {
      if (trim)
        _TS_Tidy (this, 1);
    }
  else if (n == os || (n < os && !trim));
  else
    {
      m = this->_Ptr == 0 && n < this->_Res ? this->_Res : n;
      np = calloc (m, sizeof (T));
      if (np == 0)
        Nomemory ();            /* no memory */
      for (i = 0; i < m; ++i)
        Tctor (np + i);
      r = m;
      m = n < this->_Len ? n : this->_Len;
      for (i = 0; i < m; ++i)
        Tassign (np + i, this->_Ptr + i);
      if (s != 0)
        for (; i < this->_Res; ++i)
          Tassign (np + i, s);
      _TS_Tidy (this, 1);
      this->_Ptr = np;
      this->_Res = r;
    }
  this->_Len = n;
}

/* Exported functions */

/* _create: Create an array out of thin air.  */
_TS_ARRAY_T *
_TS_create (void)
{
  _TS_ARRAY_T *x = malloc (sizeof (_TS_ARRAY_T));

  if (x == 0)
    Nomemory ();
  _TS_Tidy (x, 0);
  return x;
}

/* _destroy: Destroy all elements in the array and free the array.  */
void
_TS_destroy (_TS_ARRAY_T * x)
{
  _TS_Tidy (x, 1);
  free (x);
}

/* _ctor: Construct a newly-allocated array.  */
void
_TS_ctor (_TS_ARRAY_T * this)
{
  _TS_Tidy (this, 0);
}

/* _ctor_with_size: Construct a newly-allocated array, given a size and
   whether the new elements are a default_size or should be in reserve.  */
void
_TS_ctor_with_size (_TS_ARRAY_T * this, size_t n, capacity c)
{
  _TS_Tidy (this, 0);
  this->_Res = n;
  if (c == default_size)
    _TS_Grow (this, n, 0, 0);
}

/* _create_with_size: Allocates an array and calls _ctor_with_size on it.  */
_TS_ARRAY_T *
_TS_create_with_size (size_t n, capacity c)
{
  _TS_ARRAY_T *x = malloc (sizeof (_TS_ARRAY_T));

  if (x == 0)
    Nomemory ();
  _TS_ctor_with_size (x, n, c);
  return x;
}

/* _copy_ctor: Construct a newly-allocated array based on the elements of an
   existing array.  */
void
_TS_copy_ctor (_TS_ARRAY_T * this, _TS_ARRAY_T * x)
{
  size_t i;

  _TS_Tidy (this, 0);
  _TS_Grow (this, _TS_length (x), 0, 0);
  for (i = 0; i < this->_Len; ++i)
    Tassign (this->_Ptr + i, x->_Ptr + i);
}

/* _create_copy: Allocates an array and calls _copy_ctor on it.  */
_TS_ARRAY_T *
_TS_create_copy (_TS_ARRAY_T * y)
{
  _TS_ARRAY_T *x = malloc (sizeof (_TS_ARRAY_T));

  if (x == 0)
    Nomemory ();
  _TS_copy_ctor (x, y);
  return x;
}

/* _ctor_from_ptr: Construct an array based on the elements of a normal C array
   (decayed to a pointer).  The n parameter is the length.  */
void
_TS_ctor_from_ptr (_TS_ARRAY_T * this, T * s, size_t n)
{
  if (s == 0)
    _TS_Xinv ();
  _TS_Tidy (this, 0);
  _TS_assign (this, s, n, 1);
}

/* _create_from_ptr: Allocates an array and calls _ctor_from_ptr on it.  */
_TS_ARRAY_T *
_TS_create_from_ptr (T * s, size_t n)
{
  _TS_ARRAY_T *x = malloc (sizeof (_TS_ARRAY_T));

  if (x == 0)
    Nomemory ();
  _TS_ctor_from_ptr (x, s, n);
  return x;
}

/* _dtor: Give an array its last rites before free()ing it.  */
void
_TS_dtor (_TS_ARRAY_T * this)
{
  _TS_Tidy (this, 1);
}

/* _append: Append new elements to an existing array.  The d parameter is 1 
   if the new elements are in an array, 0 if there is only one element that 
   should be propagated through the new space.  */
_TS_ARRAY_T *
_TS_append (_TS_ARRAY_T * this, T * s, size_t n, size_t d)
{
  size_t i;

  if (NPOS - this->_Len <= n)
    _TS_Xlen ();
  i = this->_Len;
  for (_TS_Grow (this, n += i, 0, 0); i < n; ++i, s += d)
    Tassign (this->_Ptr + i, s);
  return this;
}

/* _assign: Similar to _append, except that the array is assigned to
   instead of appended to.  */
_TS_ARRAY_T *
_TS_assign (_TS_ARRAY_T * this, T * s, size_t n, size_t d)
{
  size_t i;

  _TS_Grow (this, n, 0, 1);
  for (i = 0; i < n; ++i, s += d)
    Tassign (this->_Ptr + i, s);
  return this;
}

/* _insert: Insert n elements from s into this at position p. The d parameter
   is 1 if s should be incremented after the new element is assigned to, 0 if 
   s should not be incremented.  */
_TS_ARRAY_T *
_TS_insert (_TS_ARRAY_T * this, size_t p, T * s, size_t n, size_t d)
{
  size_t i;

  if (this->_Len < p)
    _TS_Xran ();
  if (NPOS - this->_Len <= n)
    _TS_Xlen ();
  if (0 < n)
    {
      i = this->_Len - p;
      for (_TS_Grow (this, n + this->_Len, 0, 0); 0 < i;)
        {
          --i;
          Tassign (this->_Ptr + (p + n + i), this->_Ptr + (p + i));
        }
      for (i = 0; i < n; ++i, s += d)
        Tassign (this->_Ptr + (p + i), s);
    }
  return this;
}

/* _remove: Remove and destroy the n elements starting at this[p].  */
_TS_ARRAY_T *
_TS_remove (_TS_ARRAY_T * this, size_t p, size_t n)
{
  size_t i, m;

  if (this->_Len < p)
    _TS_Xran ();
  if (this->_Len - p < n)
    n = this->_Len - p;
  if (0 < n)
    {
      m = this->_Len - p - n;
      for (i = 0; i < m; ++i)
        Tassign (this->_Ptr + (p + i), this->_Ptr + (p + i + n));
      _TS_Grow (this, this->_Len - n, 0, 0);
    }
  return this;
}

/* _subarray: Assign the n elements starting at this[p] to x.  It's okay
   if this and x are the same array.  */
_TS_ARRAY_T *
_TS_subarray (_TS_ARRAY_T * this, _TS_ARRAY_T * x, size_t p, size_t n)
{
  if (this->_Len < p)
    _TS_Xran ();
  if (this->_Len - p < n)
    n = this->_Len - p;
  return (this == x) ? (_TS_remove (this, p + n, NPOS),
                        _TS_remove (this, 0, p))
    : _TS_assign (x, this->_Ptr + p, n, 1);
}

/* _Swap: swap the contents of two arrays.  */
void
_TS_swap (_TS_ARRAY_T * this, _TS_ARRAY_T * x)
{
  T *tp;
  size_t t;

  tp = this->_Ptr;
  this->_Ptr = x->_Ptr;
  x->_Ptr = tp;
  t = this->_Len;
  this->_Len = x->_Len;
  x->_Len = t;
  t = this->_Res;
  this->_Res = x->_Res;
  x->_Res = t;
}

/* _get_at: Return the element at this[i].  */
T *
_TS_get_at (_TS_ARRAY_T * this, size_t i)
{
  if (this->_Len <= i)
    _TS_Xran ();
  return this->_Ptr + i;
}

/* _put_at: Assign the element x to this[i].  */
void
_TS_put_at (_TS_ARRAY_T * this, size_t i, T * x)
{
  if (this->_Len < i)
    _TS_Xran ();
  else if (this->_Len == i)
    _TS_append (this, x, 1, 1);
  else
    Tassign (this->_Ptr + i, x);
}

/* _base: Return the base pointer, or a null pointer if there aren't
   any elements in the array. */
T *
_TS_base (_TS_ARRAY_T * this)
{
  return this->_Len != 0 ? this->_Ptr : 0;
}

/* _length: Return how many elements are there in the array */
size_t
_TS_length (_TS_ARRAY_T * this)
{
  return this->_Len;
}

/* _resize: Resize an array to _N elements, using _X for new elements
   (if we're growing the array).  */
void
_TS_resize (_TS_ARRAY_T * this, size_t n, T * x)
{
  _TS_Grow (this, n, x, 1);
}

/* _reserve: Return how many elements there are in reserve.  */
size_t
_TS_reserve (_TS_ARRAY_T * this)
{
  return this->_Res;
}

/* _set_reserve: Sets how many elements there are in reserve.  Only works if
   the array is empty.  */
void
_TS_set_reserve (_TS_ARRAY_T * this, size_t r)
{
  if (this->_Ptr == 0)
    this->_Res = r;
}

#endif

#undef _NM
#undef _VAL

#undef _TS_ARRAY_T
#undef _TS_Grow
#undef _TS_Tidy
#undef _TS_Xinv
#undef _TS_Xlen
#undef _TS_Xran
#undef _TS_append
#undef _TS_assign
#undef _TS_base
#undef _TS_copy_ctor
#undef _TS_create
#undef _TS_create_copy
#undef _TS_create_from_ptr
#undef _TS_create_with_size
#undef _TS_ctor
#undef _TS_ctor_from_ptr
#undef _TS_ctor_with_size
#undef _TS_destroy
#undef _TS_dtor
#undef _TS_get_at
#undef _TS_insert
#undef _TS_length
#undef _TS_put_at
#undef _TS_remove
#undef _TS_reserve
#undef _TS_resize
#undef _TS_set_reserve
#undef _TS_subarray
#undef _TS_swap

/* END OF FILE */
