/*
 *  The Regina Rexx Interpreter
 *  Copyright (C) 2000  Mark Hessling <M.Hessling@qut.edu.au>
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
 * $Id: rexxbif.h,v 1.2 2003/12/11 04:43:17 prokushev Exp $
 */

#ifndef _REXXBIF_H_INCLUDED
#define _REXXBIF_H_INCLUDED

#if defined(HAVE_CONFIG_H)
# include "config.h"
#else
# include "configur.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#if defined(VMS) || defined(MAC)
# include <types.h>
#else
# include <sys/types.h>
#endif

#if defined(TIME_WITH_SYS_TIME)
# include <sys/time.h>
# include <time.h>
#else
# if defined(HAVE_SYS_TIME_H)
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#if defined(WIN32) && defined(__BORLANDC__)
# include <mem.h>
#endif
#include <string.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifdef EXTERNAL_TO_REGINA
typedef struct tsdtype { /* FGC: This will lead to severe troubles. Imagine
                          * some called routines which are compiled with the
                          * other typedef and linked with a routine using this
                          * kind. Strange things MUST happen. To avoid the
                          * ugliest errors ALWAYS add a scratch space of
                          * at least 4KB. This is a heuristic value.
                          * This is one of the most errorneous programming
                          * techniques to import features and functions from
                          * different programs.
                          * MH. This is included just in case there is an
                          * overlap problem when rexxbif.h is included in the
                          * base Regina code. When used in programs independent
                          * from Regina, the filler is not required.
                          */
   char filler_never_touch[0x1000];
   int called_from_saa;
} dummy_tsd_t ;

# define tsd_t dummy_tsd_t
# define MAKESTRENG( size )    MakeStreng( size )
# define DROPSTRENG( x )       DropStreng( x )
# define REXX_RIGHT( s,l,p )   Rexx_right( NULL,s,l,p )
# define REXX_X2D( x,err )     Rexx_x2d( NULL,x,err )
# define REXX_D2X( x )         Rexx_d2x( NULL,x )
# ifndef STRENG_TYPEDEFED
/*
 * typedef a streng type
 */
typedef struct strengtype {
   int len, max;
#if 0
   char *value;
#else
   char value[4];
#endif
} streng ;
# endif
#else
/*
 * We are including this from within the Regina Interpreter
 */
# include "rexx.h"
# define MAKESTRENG( size )    Str_makeTSD( size )
# define DROPSTRENG( x )       FreeTSD( x )
# define REXX_RIGHT( s,l,p )   Rexx_right( TSD,s,l,p )
# define REXX_X2D( x,err )     Rexx_x2d( TSD,x,err )
# define REXX_D2X( x )         Rexx_d2x( TSD,x )
#endif

extern streng *Rexx_right( const tsd_t *TSD, streng *str, int length, char padch );
extern int Rexx_x2d( const tsd_t *TSD, const streng *hex, int *error );
extern streng *Rexx_d2x( const tsd_t *TSD, int num );

#ifdef EXTERNAL_TO_REGINA
extern void DropStreng( streng *str );
extern streng *MakeStreng( int num );
#endif

#define PSTRENGLEN(x)           ((x)->value ? (x)->len : 0)
#define PSTRENGVAL(x)           ((x)->value)

#if 0
/*
 * Define the RXSTRING type, but only if we haven't already included
 * a SAA header file, which #defines RXSTRLEN()
 */
#ifndef RXSTRLEN
# define MAKERXSTRING(x,c,l)   ((x).strptr=(c),(x).strlength=(l))
# define RXNULLSTRING(x)       (!(x).strptr)
# define RXSTRLEN(x)           ((x).strptr ? (x).strlength : 0UL)
# define RXSTRPTR(x)           ((x).strptr)

#endif /* RXSTRLEN */
#endif

#endif
