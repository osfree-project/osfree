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
* Description:  Macros for single source wide/narrow character code.
*
****************************************************************************/


#ifndef _WIDECHAR_H_INCLUDED
#define _WIDECHAR_H_INCLUDED

#include "variety.h"
//#include <stdlib.h>             /* for wchar_t and _atouni */

/*** Define some handy macros ***/
    #define DIR_TYPE            struct dirent
    #define CHAR_TYPE           char
    #define UCHAR_TYPE          unsigned char
    #define INTCHAR_TYPE        int
    #define NULLCHAR            '\0'
    #define _AToUni(p1,p2)      (p2)
    #define IS_ASCII(c)         ( 1 )
    #define TO_ASCII(c)         ( (unsigned char)c )
    #define __F_NAME(n1,n2)     n1
#define CHARSIZE                (sizeof( CHAR_TYPE ))

/* must be the larger of char and wchar_t */
#define MAX_CHAR_TYPE           wchar_t

#endif
