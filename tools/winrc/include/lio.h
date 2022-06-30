/*  io.h	1.4
    Copyright 1997 Willows Software, Inc. 

This library is free software; you can redistribute it and/or
modify it under the terms of the GNU Library General Public License as
published by the Free Software Foundation; either version 2 of the
License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Library General Public License for more details.

You should have received a copy of the GNU Library General Public
License along with this library; see the file COPYING.LIB.  If
not, write to the Free Software Foundation, Inc., 675 Mass Ave,
Cambridge, MA 02139, USA.

The maintainer of the Willows TWIN Libraries may be reached (Email) 
at the address twin@willows.com	

*/

#ifndef IO_H
#define	IO_H

/*
 *  Pick up definition of "time_t".
 */
#include <time.h>

/*
 *  Get various file I/O function definitions.
 */
#include <unistd.h>
#include <fcntl.h>

/*
 *  ftime structure used by getftime()/setftime().
 */
struct ftime
{
    unsigned ft_tsec  : 5;   /* 2-second intervals */
    unsigned ft_min   : 6;   /* minutes */
    unsigned ft_hour  : 5;   /* hours */
    unsigned ft_day   : 5;   /* days */
    unsigned ft_month : 4;   /* months */
    unsigned ft_year  : 7;   /* year */
};

/*
 * _fsize_t is used by some MSVC items, including _finddata_t struct.
 */
typedef unsigned long _fsize_t;

/*
 *  _finddata_t structure is used by MSVC's _findfirst()/_findnext()
 *  functions.
 */
struct _finddata_t
{
    unsigned attrib;
    time_t time_create;
    time_t time_access;
    time_t time_write;
    _fsize_t size;
    char name[260];
};

/*
 *  Define some prototypes and synonyms for various Microsoft and
 *  Borland library calls we support.
 */

#ifdef __cplusplus
extern "C" {
#endif

long _filelength(int handle);
long filelength(int handle);
int getftime(int handle, struct ftime *ftimep);
int setftime(int handle, struct ftime *ftimep);
long _findfirst(const char *filespec, struct _finddata_t *fileinfo);
int _findnext(long handle, struct _finddata_t *fileinfo);

#ifdef __cplusplus
}
#endif

#endif /* IO_H */

