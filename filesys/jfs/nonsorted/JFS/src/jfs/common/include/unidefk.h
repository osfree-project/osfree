/* $Id: unidefk.h,v 1.1.1.1 2003/05/21 13:36:00 pasha Exp $ */

/* static char *SCCSID = "@(#)1.2  12/8/98 13:57:02 src/jfs/common/include/unidefk.h, sysjfs, w45.fs32, 990417.1";*/
/*
 *   Copyright (c) International Business Machines  Corp., 2000
 *
 *   This program is free software;  you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 2 of the License, or 
 *   (at your option) any later version.
 * 
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY;  without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See
 *   the GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program;  if not, write to the Free Software 
 *   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
 * unidefk.h
 *
 * Function:
 *     ULS Kernel API prototypes and typedefs.
 *
  * Notes:
 */

#ifndef _ULS_UNIDEFK
#define _ULS_UNIDEFK

/*
 * UniChar Type
 */
#ifndef _ULS_UNICHAR_DEFINED
    typedef unsigned  short  UniChar ;
    #define _ULS_UNICHAR_DEFINED
#endif

/*
 * Unicode versions of C runtime library functions
 */
UniChar * UniStrcat  ( UniChar * ucs1, const UniChar * ucs2 );
UniChar * UniStrchr  ( const UniChar * ucs, UniChar uc );
int       UniStrcmp  ( const UniChar * ucs1, const UniChar * ucs2 );
UniChar * UniStrcpy  ( UniChar * ucs1, const UniChar * ucs2 );
size_t    UniStrlen  ( const UniChar * ucs1 );
UniChar * UniStrlwr  ( UniChar * ucs );
UniChar * UniStrncat ( UniChar * ucs1, const UniChar * ucs2, size_t n );
int       UniStrncmp ( const UniChar * ucs1, const UniChar * ucs2, size_t n );
UniChar * UniStrncpy ( UniChar * ucs1, const UniChar * ucs2, size_t n );
UniChar * UniStrstr  ( const UniChar *ucs1, const UniChar *ucs2 );
UniChar * UniStrupr  ( UniChar * ucs );
UniChar   UniTolower ( UniChar uc );
UniChar   UniToupper ( UniChar uc );

#endif
