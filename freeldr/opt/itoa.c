/****************************************************************************
*
*                            Open Watcom Project
*
*    Portions Copyright (c) 1983-2002 Sybase, Inc. All Rights Reserved.
*
*  ========================================================================
*
*    This file contains Original Code and/or Modifications of Original
*    Code as defined in and that are subject to the Sybase Open Watcom
*    Public License version 1.0 (the 'License'). You may not use this file
*    except in compliance with the License. BY USING THIS FILE YOU AGREE TO
*    ALL TERMS AND CONDITIONS OF THE LICENSE. A copy of the License is
*    provided with the Original Code and Modifications, and is also
*    available at www.sybase.com/developer/opensource.
*
*    The Original Code and all software distributed under the License are
*    distributed on an 'AS IS' basis, WITHOUT WARRANTY OF ANY KIND, EITHER
*    EXPRESS OR IMPLIED, AND SYBASE AND ALL CONTRIBUTORS HEREBY DISCLAIM
*    ALL SUCH WARRANTIES, INCLUDING WITHOUT LIMITATION, ANY WARRANTIES OF
*    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE, QUIET ENJOYMENT OR
*    NON-INFRINGEMENT. Please see the License for the specific language
*    governing rights and limitations under the License.
*
*  ========================================================================
*
* Description:  Implementation of itoa() and utoa().
*
****************************************************************************/


#include "variety.h"
#include "widechar.h"


#define MAX_DIGITS 9

unsigned int __cdecl itoa10(char* p, unsigned int n);

unsigned int __cdecl itoa10(char* p, unsigned int n)
{
    char tmp[MAX_DIGITS + 1];
    char* s = tmp + MAX_DIGITS;
    *s = 0;

    do
    {
        *--s = "0123456789"[n % 10];
        n /= 10;
    } while (n > 0);

    strcpy(p, s);

    return tmp + MAX_DIGITS - s;
}

extern const char __based(__segname("_CODE")) __Alphabet[];

unsigned __cdecl __udiv( unsigned, unsigned _WCNEAR * );
    #pragma aux __udiv = \
        "xor dx,dx" \
        "div word ptr [bx]" \
        "mov [bx],ax" \
        parm caller [ax] [bx] \
        modify exact [ax dx] \
        value [dx];

CHAR_TYPE * __cdecl utoa( unsigned value, CHAR_TYPE *buffer, int radix )
    {
        CHAR_TYPE *p = buffer;
        char *q;
        unsigned rem;
        unsigned quot;
        auto char buf[34];      // only holds ASCII so 'char' is OK

        buf[0] = '\0';
        q = &buf[1];
        do {
                quot = radix;
                rem = __udiv( value, (unsigned _WCNEAR *) &quot );
            *q = __Alphabet[ rem ];
            ++q;
            value = quot;
        } while( value != 0 );
        while( *p++ = (CHAR_TYPE)*--q );
        return( buffer );
    }


CHAR_TYPE * __cdecl itoa( int value, CHAR_TYPE *buffer, int radix )
    {
        register CHAR_TYPE *p = buffer;

        if( radix == 10 ) {
            if( value < 0 ) {
                *p++ = '-';
                value = - value;
            }
        }
        utoa( value, p, radix );
        return( buffer );
    }
