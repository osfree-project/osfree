
// 32 bits OS/2 device driver and IFS support. Provides 32 bits kernel
// services (DevHelp) and utility functions to 32 bits OS/2 ring 0 code
// (device drivers and installable file system drivers).
// Copyright (C) 1995, 1996, 1997  Matthieu WILLM (willm@ibm.net)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

/* From :            */
/* ctype.h (emx+gcc) */

#if !defined (_CTYPE_H)
#define _CTYPE_H

#if defined (__cplusplus)
extern "C" {
#endif

extern unsigned char _ctype[];

#define _UPPER  0x01
#define _LOWER  0x02
#define _DIGIT  0x04
#define _XDIGIT 0x08
#define _CNTRL  0x10
#define _SPACE  0x20
#define _PUNCT  0x40
#define _PRINT  0x80

#define isalnum(c)  ((_ctype+1)[c] & (_UPPER|_LOWER|_DIGIT))
#define isalpha(c)  ((_ctype+1)[c] & (_UPPER|_LOWER))
#define iscntrl(c)  ((_ctype+1)[c] & (_CNTRL))
#define isdigit(c)  ((_ctype+1)[c] & (_DIGIT))
#define isgraph(c)  ((_ctype+1)[c] & (_PUNCT|_UPPER|_LOWER|_DIGIT))
#define islower(c)  ((_ctype+1)[c] & (_LOWER))
#define isprint(c)  ((_ctype+1)[c] & (_PRINT))
#define ispunct(c)  ((_ctype+1)[c] & (_PUNCT))
#define isspace(c)  ((_ctype+1)[c] & (_SPACE))
#define isupper(c)  ((_ctype+1)[c] & (_UPPER))
#define isxdigit(c) ((_ctype+1)[c] & (_XDIGIT))

#if !defined (_CTYPE_FUN)
#ifndef __IBMC__
static __inline__ int _toupper (int _c) { return (_c-'a'+'A'); }
static __inline__ int _tolower (int _c) { return (_c-'A'+'a'); }
static __inline__ int toupper(int _c)
  {return (islower(_c) ? _toupper(_c) : _c);}
static __inline__ int tolower(int _c)
  {return (isupper(_c) ? _tolower(_c) : _c);}
  #else

INLINE            int _toupper (int _c) { return (_c-'a'+'A'); }
INLINE            int _tolower (int _c) { return (_c-'A'+'a'); }
INLINE            int toupper(int _c)
  {return (islower(_c) ? _toupper(_c) : _c);}
INLINE            int tolower(int _c)
  {return (isupper(_c) ? _tolower(_c) : _c);}
  #endif
#endif

#if !defined (__STRICT_ANSI__) && !defined (_POSIX_SOURCE)

#define isascii(c)  ((unsigned)(c) <= 0x7f)
#define toascii(c)  ((c) & 0x7f)

#endif


#if defined (__cplusplus)
}
#endif

#endif /* !defined (_CTYPE_H) */
