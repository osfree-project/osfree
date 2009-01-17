/*
 *  Generic Call Interface for Rexx
 *  Copyright © 2003-2004, Florian Groﬂe-Coosmann
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
 *
 * ----------------------------------------------------------------------------
 *
 * This file configures the number converting system. Have a look at
 * gci_convert.c for the requirements. This file must include gci.h.
 */

#ifndef _GNU_SOURCE
# define _GNU_SOURCE
#endif
#include <limits.h>
#include <float.h>
#include "gci.h"

#define NEED_STRTOBIGL
#define NEED_STRTOBIGUL

#define GCI_Ir(s,p,b) strtobigl( hidden, s, p, b )
#define GCI_Iw(s,v)   sprintf( s, "%Ld", v )
#define GCI_I_1       signed char
#define GCI_I_1m      SCHAR_MIN
#define GCI_I_1M      SCHAR_MAX
#define GCI_I_2       signed short
#define GCI_I_2m      SHRT_MIN
#define GCI_I_2M      SHRT_MAX
#define GCI_I_4       signed int
#define GCI_I_4m      INT_MIN
#define GCI_I_4M      INT_MAX
#define GCI_I_8       signed __int64
#define GCI_I_8m      LONGLONG_MIN
#define GCI_I_8M      LONGLONG_MAX

#define GCI_Ur(s,p,b) strtobigul( hidden, s, p, b )
#define GCI_Uw(s,v)   sprintf( s, "%Lu", v )
#define GCI_U_1       unsigned char
#define GCI_U_1M      UCHAR_MAX
#define GCI_U_2       unsigned short
#define GCI_U_2M      USHRT_MAX
#define GCI_U_4       unsigned
#define GCI_U_4M      UINT_MAX
#define GCI_U_8       unsigned __int64
#define GCI_U_8M      ULONGLONG_MAX

#define GCI_Fr        strtod
#define GCI_Fw(s,v)   sprintf( s, "%.*lE", LDBL_MANT_DIG/3, v )
#define GCI_F_4       float
#define GCI_F_4m      (-FLT_MAX)
#define GCI_F_4M      FLT_MAX
#define GCI_F_8       double
#define GCI_F_8m      (-DBL_MAX)
#define GCI_F_8M      DBL_MAX

/*
 ******************************************************************************
 */

/*
 * GCI_STACK_ELEMENT sets integral type of a stack element. This is typically
 * an unsigned or int.
 */
#define GCI_STACK_ELEMENT unsigned

/*
 * GCI_LITTLE_ENDIAN must be set to 1 or 0 depending on whether little endian
 * or big endian is the machine's representation.
 * In doubt, select 1 for Intel compatibles and 0 for others.
 */
#define GCI_LITTLE_ENDIAN 1

/*
 * GCI_ARGS shall be the maximum number of GCI_STACK_ELEMENTs which shall
 * be passed on the stack. This is usually the base type of maximum width
 * (e.g. long long or long double) / sizeof(unsigned) * GCI_REXX_ARGS.
 * But you can't use sizeof(), therefore you have to compute it by hand.
 * 4 * GCI_REXX_ARGS is an upper maximum for all useful systems I can imagine.
 */
#define GCI_ARGS 50   /* 25 full sized arguments */

/*
 * GCI_PASSARGS is a macro which enumerates GCI_ARGS args of an array which
 * is passed to the macro. I don't know a good ANSI macro for this purpose.
 * Feel free to provide it!
 */
#define GCI_PASSARGS(a) a[0],a[1],a[2],a[3],a[4],a[5],a[6],a[7],a[8],a[9],\
                        a[10],a[11],a[12],a[13],a[14],a[15],a[16],a[17],a[18],\
                        a[19],a[20],a[21],a[22],a[23],a[24],a[25],a[26],a[27],\
                        a[28],a[29],a[30],a[31],a[32],a[33],a[34],a[35],a[36],\
                        a[37],a[38],a[39],a[40],a[41],a[42],a[43],a[44],a[45],\
                        a[46],a[47],a[48],a[49]
