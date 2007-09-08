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
* Description:  __prtf() - low level string formatter.
*
****************************************************************************/



#include "variety.h"
#include "widechar.h"
//#include <stdio.h>
#include "stdarg.h"
//#include <stdlib.h>
//#include <string.h>
//#include <ctype.h>
#include "ftos.h"
#include "farsupp.h"
#include "printf.h"
#include "fixpoint.h"
#include "myvalist.h"

char * __cdecl itoa( int __value, char *__buf, int __radix );
unsigned int __cdecl itoa10(char* p, unsigned int n);

  #define BUF_SIZE 72       /* 64-bit ints formatted as binary can get big */
  #define TRUE 1
  #define FALSE 0

  #define PASCAL_STRING           'S'             /* for Novell */



typedef int             bool;

#define EFG_PRINTF (*__EFG_printf)
extern FAR_STRING EFG_PRINTF( char *buffer, my_va_list *args, _mbcs_SPECS __SLIB *specs );


  #define _FAR_OTHER_STRING             FAR_UNI_STRING

    #define CVT_NEAR(p)     ((void *)(p))


/* forward references */
static const CHAR_TYPE * __cdecl evalflags( const CHAR_TYPE *, SPECS __SLIB * );
static FAR_STRING __cdecl formstring( CHAR_TYPE *, my_va_list *, SPECS __SLIB *, CHAR_TYPE * );
static const CHAR_TYPE * __cdecl getprintspecs( const CHAR_TYPE *, my_va_list *, SPECS __SLIB * );


int __cdecl __prtf(     void __SLIB *dest,              /* parm for use by out_putc */
                const CHAR_TYPE *format,                /* pointer to format string */
                va_list args,                   /* pointer to pointer to args*/
                slib_callback_t *out_putc       /* char output routine */
              )
{
    auto CHAR_TYPE buffer[ BUF_SIZE ];
    auto CHAR_TYPE null_char = '\0';
    register CHAR_TYPE *a;
    FAR_STRING arg;
    const CHAR_TYPE *ctl;
    SPECS specs;

    specs._o._dest = dest;
    specs._flags = 0;
    specs._version = SPECS_VERSION;
    specs._o._output_count = 0;
    ctl = format;
    while( *ctl != NULLCHAR ) {
        if( *ctl != '%' ) {
            (out_putc)( &specs, *ctl++ );
        } else {
            ++ctl;
            {
                my_va_list pargs;
                pargs = MY_VA_LIST( args );
                ctl = getprintspecs( ctl, &pargs, &specs );
                MY_VA_LIST( args ) = pargs;
            }


            specs._o._character = *ctl++;
            if( specs._o._character == NULLCHAR )
                break;        /* 05-jan-89 */

            if( specs._o._character == 'n' ) {
                if( specs._flags & SPF_LONG ) {
  #if defined( __FAR_SUPPORT__ )
                    if( specs._flags & SPF_FAR ) {
                        *va_arg( args, long int _WCFAR *) = specs._o._output_count;
                    } else if( specs._flags & SPF_NEAR ) {
                        a =  CVT_NEAR( va_arg( args, long int _WCNEAR * ) );
                        *(long int *)a = specs._o._output_count;
                    } else {
                        *va_arg( args, long int *) = specs._o._output_count;
                    }
  #else
                    *va_arg( args, long int * ) = specs._o._output_count;
  #endif
                } else if( specs._flags & SPF_SHORT ) {     /* JBS 92/02/12 */
  #if defined( __FAR_SUPPORT__ )
                    if( specs._flags & SPF_FAR ) {
                        *va_arg( args, short int _WCFAR *) = specs._o._output_count;
                    } else if( specs._flags & SPF_NEAR ) {
                        a =  CVT_NEAR( va_arg( args, short int _WCNEAR * ) );
                        *(short int *)a = specs._o._output_count;
                    } else {
                        *va_arg( args, short int *) = specs._o._output_count;
                    }
  #else
                    *va_arg( args, short int * ) = specs._o._output_count;
  #endif
                } else {
  #if defined( __FAR_SUPPORT__ )
                    if( specs._flags & SPF_FAR ) {
                        *va_arg( args, int _WCFAR *) = specs._o._output_count;
                    } else if( specs._flags & SPF_NEAR ) {
                        a =  CVT_NEAR( va_arg( args, long int _WCNEAR * ) );
                        *(int *)a = specs._o._output_count;
                    } else {
                        *va_arg( args, int *) = specs._o._output_count;
                    }
  #else
                    *va_arg( args, int *) = specs._o._output_count;
  #endif
                }
            } else {
                {
                    my_va_list pargs;
                    pargs = MY_VA_LIST( args );
                    arg = formstring( buffer, &pargs, &specs, &null_char );
                    MY_VA_LIST( args ) = pargs;
                }
                specs._o._fld_width -= specs._n0  +
                                       specs._nz0 +
                                       specs._n1  +
                                       specs._nz1 +
                                       specs._n2  +
                                       specs._nz2;
                if( !(specs._flags & SPF_LEFT_ADJUST) ) {
                    if( specs._o._pad_char == ' ' ) {
                        while( specs._o._fld_width > 0 ) {
                            (out_putc)( &specs, ' ' );
                            --specs._o._fld_width;
                        }
                    }
                }
                a = buffer;
                while( specs._n0 > 0 ) {
                    (out_putc)( &specs, *a );
                    ++a;
                    --specs._n0;
                }
                while( specs._nz0 > 0 ) {
                    (out_putc)( &specs, '0' );
                    --specs._nz0;
                }
                if( specs._o._character == 's' ) {
                    {
                        while( specs._n1 > 0 ) {
                            (out_putc)( &specs, *arg++ );
                            --specs._n1;
                        }
                    }
                }
                {
                    while( specs._n1 > 0 ) {
                        (out_putc)( &specs, *arg++ );
                        --specs._n1;
                    }
                }
                while( specs._nz1 > 0 ) {
                    (out_putc)( &specs, '0' );
                    --specs._nz1;
                }
                while( specs._n2 > 0 ) {
                    (out_putc)( &specs, *arg );
                    ++arg;
                    --specs._n2;
                }
                while( specs._nz2 > 0 ) {
                    (out_putc)( &specs, '0' );
                    --specs._nz2;
                }
                if( specs._flags & SPF_LEFT_ADJUST ) {
                    while( specs._o._fld_width > 0 ) {
                        (out_putc)( &specs, ' ' );
                        --specs._o._fld_width;
                    }
                }
            }
        }
    }
    return( specs._o._output_count );
}

static const CHAR_TYPE * __cdecl getprintspecs( const CHAR_TYPE *ctl,
                                    my_va_list *pargs,
                                    SPECS __SLIB *specs )
{
    specs->_o._pad_char = ' ';
    ctl = evalflags( ctl, specs );
    specs->_o._fld_width = 0;
    if( *ctl == '*' ) {
        specs->_o._fld_width = va_arg( pargs->v, int );
        if( specs->_o._fld_width < 0 ) {
            specs->_o._fld_width = - specs->_o._fld_width;
            specs->_flags |= SPF_LEFT_ADJUST;
        }
        ctl++;
    } else {
        while( *ctl >= '0'  &&  *ctl <= '9' ) {
            specs->_o._fld_width = specs->_o._fld_width * 10 + ( *ctl++ - '0' );
        }
    }
    specs->_o._prec = -1;
    if( *ctl == '.' ) {
        specs->_o._prec = 0;
        ctl++;
        if( *ctl == '*' ) {
            specs->_o._prec = va_arg( pargs->v, int );
            if( specs->_o._prec < 0 )
                specs->_o._prec = -1;    /* 19-jul-90 */
            ctl++;
        } else {
            while( *ctl >= '0'  &&  *ctl <= '9' ) {
                specs->_o._prec = specs->_o._prec * 10 + ( *ctl++ - '0' );
            }
        }
        /*
        "For b, d, i, o, u, x, X, e, E, f, g and G conversions, leading
        zeros (following any indication of sign or base) are used to
        pad the field width; no space padding is performed. If the 0
        or - flags both appear, the 0 flag is ignored.  For b, d, i, o,
        u, x or X conversions, if a precision is specified, the 0 flag
        is ignored. For other conversions, the behaviour is undefined."
        */
//      if( specs->_o._prec != -1 )  specs->_o._pad_char = ' '; /* 30-jul-95 *//*removed by JBS*/
    }
    switch( *ctl ) {
    case 'l':
        /* fall through */
    case 'w':
        specs->_flags |= SPF_LONG;
        ctl++;
        break;
    case 'h':
        specs->_flags |= SPF_SHORT;
        ctl++;
        break;
    case 'L':
        specs->_flags |= SPF_LONG_DOUBLE;
        ctl++;
        break;
  #if defined( __FAR_SUPPORT__ )
    case 'F':                   /* 8086 specific flag for FAR pointer */
        specs->_flags |= SPF_FAR;
        ctl++;
        break;
    case 'N':                   /* 8086 specific flag for NEAR pointer */
        specs->_flags |= SPF_NEAR;
        ctl++;
        break;
  #endif
    }
    return( ctl );
}


static const CHAR_TYPE * __cdecl evalflags( const CHAR_TYPE *ctl, SPECS __SLIB *specs )
{
    specs->_flags = 0;
    for(;; ctl++ ) {
        if( *ctl == '-' ) {
            specs->_flags |= SPF_LEFT_ADJUST;
        } else
        if( *ctl == '#' ) {
            specs->_flags |= SPF_ALT;
        } else
        if( *ctl == '+' ) {
            specs->_flags |= SPF_FORCE_SIGN;
            specs->_flags &= ~SPF_BLANK;
        } else
        if( *ctl == ' ' ) {
            if( ( specs->_flags & SPF_FORCE_SIGN ) == 0 ) {
                specs->_flags |= SPF_BLANK;
            }
        } else
        if( *ctl == '0' ) {
            specs->_o._pad_char = '0';
        } else {
            break;
        }
    }
    return( ctl );
}


static int __cdecl far_strlen( FAR_STRING s, int precision )
{
    int len;

    len = 0;
    while( len != precision && *s++ != NULLCHAR )
        ++len;

    return( len );
}

static void __cdecl fmt4hex( unsigned value, CHAR_TYPE *buf, int maxlen )
{
    register int i, len;

    itoa( value, buf, 16 );
    len = __F_NAME(strlen,wcslen)( buf );
    for( i = maxlen - 1; len; --i ) {
        --len;
        buf[i] = buf[len];
    }
    while( i >= 0 ) {
        buf[i] = '0';
        --i;
    }
    buf[maxlen] = NULLCHAR;
}


static void __cdecl FixedPoint_Format( CHAR_TYPE *buf, long value, SPECS __SLIB *specs )
{
    T32         at;
    int         i;
    CHAR_TYPE   *bufp;

    at.sWhole = value;
    if( at.sWhole < 0 ) {
        at.sWhole = - at.sWhole;
        *buf++ = '-';
    }
    if( specs->_o._prec == -1 )
        specs->_o._prec = 4;

    itoa10(buf, at.wd.hi);
    bufp = buf;         /* remember start address of buffer */
    while( *buf )
        ++buf;

    if( specs->_o._prec != 0 ) {
        *buf++ = '.';
        for( i = 0; i < specs->_o._prec; i++ ) {
            at.wd.hi = 0;
            at.uWhole *= 10;
            *buf++ = at.bite.b3 + '0';
        }
        *buf = NULLCHAR;
    }
    if( at.wd.lo & 0x8000 ) {   /* fraction >= .5, need to round */
        for(;;) {                               /* 22-dec-91 */
            if( buf == bufp ) {
                *buf++ = '1';
                while( *buf == '0' )
                    ++buf;

                if( *buf == '.' ) {
                    *buf++ = '0';
                    *buf++ = '.';
                    while( *buf == '0' )
                        ++buf;
                }
                *buf++ = '0';
                *buf = NULLCHAR;
                break;
            }
            --buf;
            if( *buf == '.' )
                --buf;

            if( *buf != '9' ) {
                ++ *buf;
                break;
            }
            *buf = '0';
        }
    }
}

static void __cdecl SetZeroPad( SPECS __SLIB *specs )
{
    int         n;

    if( !(specs->_flags & SPF_LEFT_ADJUST) ) {
        if( specs->_o._pad_char == '0' ) {
            n = specs->_o._fld_width - specs->_n0 - specs->_nz0 -
                         specs->_n1 - specs->_nz1 - specs->_n2 - specs->_nz2;
            if( n > 0 ) {
                specs->_nz0 += n;
            }
        }
    }
}



static FAR_STRING __cdecl formstring( CHAR_TYPE *buffer, my_va_list *pargs,
                              SPECS __SLIB *specs, CHAR_TYPE *null_string )
{
    FAR_STRING          arg;
    int                 length;
    int                 radix;
    unsigned long  long_value;
    unsigned int   int_value;
    unsigned int   seg_value;

    arg = buffer;

    specs->_n0 = specs->_nz0 =
    specs->_n1 = specs->_nz1 =
    specs->_n2 = specs->_nz2 = 0;

    if( ( specs->_o._character == 'b' ) ||
        ( specs->_o._character == 'o' ) ||
        ( specs->_o._character == 'u' ) ||
        ( specs->_o._character == 'x' ) ||
        ( specs->_o._character == 'X' ) ) {
        if( specs->_flags & SPF_LONG ) {
            long_value = va_arg( pargs->v, unsigned long );
        } else {
            long_value = va_arg( pargs->v, unsigned );
            if( specs->_flags & SPF_SHORT ) {    /* JBS 92/02/12 */
                long_value = (unsigned short) long_value;
            }
        }
    } else

    if( ( specs->_o._character == 'd' ) ||
        ( specs->_o._character == 'i' ) ) {

        if( specs->_flags & SPF_LONG ) {
            long_value = va_arg( pargs->v, long );
        } else {
            long_value = va_arg( pargs->v, int );
            if( specs->_flags & SPF_SHORT ) {    /* JBS 92/02/12 */
                long_value = (short) long_value;
            }
        }
        {
            int negative = FALSE;

            if( (long)long_value < 0 ) {
                negative = TRUE;
            }
            if( negative ) {
                buffer[specs->_n0++] = '-';

                long_value = - long_value;
            } else
            if( specs->_flags & SPF_FORCE_SIGN ) {
                buffer[specs->_n0++] = '+';
            } else
            if( specs->_flags & SPF_BLANK ) {
                buffer[specs->_n0++] = ' ';
            }
        }
    }

    radix  = 10;                        /* base 10 for 'd', 'i' and 'u' */
    
    switch( specs->_o._character ) {

    case 's':
        CHAR_TYPE *temp;
        
        temp = va_arg( pargs->v, CHAR_TYPE * );
        // arg has been initialized to point to buffer
        // set buffer[0] to a null character assuming pointer will be NULL
        // If pointer is not null, then arg will be assigned the pointer
        buffer[0] = '\0';                       // assume null pointer

        if( temp ) arg = temp;

        length = far_strlen( arg, specs->_o._prec );

        specs->_n1 = length;
        if( specs->_o._prec >= 0  &&  specs->_o._prec < length ) {
            specs->_n1 = specs->_o._prec;
        }
        break;

    case 'x':
    case 'X':
        if( specs->_flags & SPF_ALT ) {
            if( long_value != 0 ) {
                buffer[specs->_n0++] = '0';
                buffer[specs->_n0++] = specs->_o._character;
            }
        }
        radix = 16;                     /* base 16 */
        goto processNumericTypes;

    case 'b':           /* CDH 2003 Apr 23 *//* Add binary mode */
        radix = 2;                  /* base 2 */
        goto checkFlags;

    case 'd':
    case 'i':
    case 'u':
//       'x' and 'X' jumps here

processNumericTypes:
        if( specs->_o._prec != -1 )
            specs->_o._pad_char = ' '; /* 30-jul-95, 11-may-99 */

        /* radix contains the base; 8 for 'o', 10 for 'd' and 'i' and 'u',
           16 for 'x' and 'X', and 2 for 'b' */

        arg = &buffer[ specs->_n0 ];

        if( specs->_o._prec == 0  &&  long_value == 0 ) {
            *arg = '\0';
            length = 0;
        } else {
            ultoa( long_value, &buffer[specs->_n0], radix );
            if (radix==10) itoa10(&buffer[specs->_n0], long_value);
            if( specs->_o._character == 'X' ) {
                strupr( buffer );
            }
            length = far_strlen( arg, -1 );
        }
        specs->_n1 = length;
        if( specs->_n1 < specs->_o._prec ) {
            specs->_nz0 = specs->_o._prec - specs->_n1;
        }
        if( specs->_o._prec == -1 ) {
            SetZeroPad( specs );
        }
        break;

    case 'p':
    case 'P':
        if( specs->_o._fld_width == 0 ) {
            specs->_o._fld_width = sizeof(unsigned)*2;
        }
        specs->_flags &= ~( SPF_BLANK | SPF_FORCE_SIGN );
        int_value = va_arg( pargs->v, unsigned );               /* offset */
            seg_value = va_arg( pargs->v, unsigned ) & 0xFFFF; /* segment */
            /* use "unsigned short" for 386 instead of "unsigned" 21-jul-89 */
            fmt4hex( seg_value, buffer, sizeof(unsigned short)*2 );
            buffer[sizeof(unsigned short)*2] = ':';
            fmt4hex( int_value, buffer + sizeof(unsigned short)*2 + 1,
                       sizeof(unsigned)*2 );
        if( specs->_o._character == 'P' ) {
            __F_NAME(strupr,_wcsupr)( buffer );
        }
        specs->_n0 = far_strlen( arg, -1 );
        break;

    case 'c':
        specs->_n0 = 1;
        buffer[0] = va_arg( pargs->v, int );
        break;

    default:
        specs->_o._fld_width = 0;
        buffer[ 0 ] = specs->_o._character;
        specs->_n0 = 1;
        break;
    }
    return( arg );
}


