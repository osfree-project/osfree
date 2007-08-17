/*
 *  The Regina Rexx Interpreter
 *  Copyright (C) 2003  Florian Groﬂe-Coosmann <florian@grosse-coosmann.de>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public
 *  License along with this library; if not, write to the Free
 *  Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * $Id: rexx_charset.h,v 1.1 2003/12/11 05:06:00 prokushev Exp $
 */

/*
 * This header file is needed by some systems which include ctype.h in
 * system header files. Some of these must be included before rexx.h can be
 * included. This will lead to warnings or error since this file replaces
 * the functionality of ctype.h for a seamless support by Regina's own
 * routines.
 */

#ifndef REXX_CHARSET_H_INCLUDED
#define REXX_CHARSET_H_INCLUDED

#define RX_ISLOWER  0x001
#define RX_ISUPPER  0x002
#define RX_ISALPHA  0x004
#define RX_ISALNUM  0x008
#define RX_ISDIGIT  0x010
#define RX_ISXDIGIT 0x020
#define RX_ISPUNCT  0x040
#define RX_ISSPACE  0x080
#define RX_ISPRINT  0x100
#define RX_ISGRAPH  0x200
#define RX_ISCNTRL  0x400

#ifndef NO_CTYPE_REPLACEMENT

#include "wrappers.h"

/*
 * Emulate the inclusion of <ctype.h> by defining popular values.
 */
# define _CTYPE_H_INCLUDED
# define _CTYPE_H
# define _INC_CTYPE
# define _CTYPE_DEFINED
# define is_expand( c, bit, func ) ( ( char_info[256] & bit ) ?               \
                         ( char_info[(unsigned char) c] & bit ) :  func( c ) )

# define islower( c )  is_expand( c, RX_ISLOWER , Islower  )
# define isupper( c )  is_expand( c, RX_ISUPPER , Isupper  )
# define isalpha( c )  is_expand( c, RX_ISALPHA , Isalpha  )
# define isalnum( c )  is_expand( c, RX_ISALNUM , Isalnum  )
# define isdigit( c )  is_expand( c, RX_ISDIGIT , Isdigit  )
# define isxdigit( c ) is_expand( c, RX_ISXDIGIT, Isxdigit )
# define ispunct( c )  is_expand( c, RX_ISPUNCT , Ispunct  )
# define isspace( c )  is_expand( c, RX_ISSPACE , Isspace  )
# define isprint( c )  is_expand( c, RX_ISPRINT , Isprint  )
# define isgraph( c )  is_expand( c, RX_ISGRAPH , Isgraph  )
# define iscntrl( c )  is_expand( c, RX_ISCNTRL , Iscntrl  )

# define toupper( c ) ( ( char_info[256] & RX_ISUPPER ) ?           \
                         l_to_u[(unsigned char) c] : Toupper( c ) )

# define tolower( c ) ( ( char_info[256] & RX_ISLOWER ) ?           \
                         u_to_l[(unsigned char) c] : Tolower( c ) )
int Islower( int c );
int Isupper( int c );
int Isalpha( int c );
int Isalnum( int c );
int Isdigit( int c );
int Isxdigit( int c );
int Ispunct( int c );
int Isspace( int c );
int Isprint( int c );
int Isgraph( int c );
int Iscntrl( int c );
int Toupper( int c );
int Tolower( int c );

# ifdef WIN32
#  include <wchar.h>
# endif
#endif

#endif /* REXX_CHARSET_H_INCLUDED */
