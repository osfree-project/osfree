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
* Description:  WHEN YOU FIGURE OUT WHAT THIS FILE DOES, PLEASE
*               DESCRIBE IT HERE!
*
****************************************************************************/


#include "variety.h"
#include "widechar.h"
#include "freeldr.h"
#include "stdarg.h"
//#include <limits.h>
#include "printf.h"


/*
 * mem_putc -- append a character to a string in memory
 */

static slib_callback_t mem_putc; // setup calling convention

static void __SLIB_CALLBACK mem_putc( SPECS __SLIB *specs, int op_char )
{
    *( specs->_o._dest++ ) = op_char;
    specs->_o._output_count++;
}

int __cdecl sprintf ( CHAR_TYPE *dest, const CHAR_TYPE *format, ... )
{
    auto    va_list         arg;
    slib_callback_t *tmp;
    register int len;

    va_start(arg, format);
        
    tmp = mem_putc;
    len = __prtf( dest, format, arg, tmp );
    dest[len] = NULLCHAR;
    return( len );
}

