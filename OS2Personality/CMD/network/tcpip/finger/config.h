/* config.h -- GNU Finger configuration

   Copyright (c) 1990, 1991, 1992  Free Software Foundation, Inc.

   This file is part of GNU Finger

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.  */

#ifndef _FINGER_CONFIG_H
#define _FINGER_CONFIG_H
/* #define __GNUC__ */
/* AIX... sigh. This must go first in the file. */
#if defined(__GNUC__)
#define alloca __builtin_alloca
#else  /* !__GNUC__ */
#if defined(HAVE_ALLOCA_H)
#include <alloca.h>
#else  /* !HAVE_ALLOCA_H */
#ifdef _AIX
 #pragma alloca
#else  /* !_AIX */
char *alloca ();
#endif /* !_AIX */
#endif /* !HAVE_ALLOCA_H */
#endif /* !__GNUC__ */


#if !__STDC__
#define volatile
#endif


/* This file contains defines which control the configuration of GNU
   Finger. */

/* Don't change */
#define VERSION_STRING version_string
extern char version_string[];

/* Define if you want --info to be the default, rather than --brief. */
/* #define INFO_IS_DEFAULT  1 */

#include <sys/types.h>

#if defined(DIRENT) || defined(_POSIX_VERSION)
#include <dirent.h>
#define NLENGTH(dirent) (strlen ((dirent)->d_name))
#else  /* !DIRENT && !_POSIX_VERSION */
#define dirent direct
#define NLENGTH(dirent) ((dirent)->d_namlen)
#ifdef USG
#ifdef SYSNDIR
#include <sys/ndir.h>
#else  /* !SYSNDIR */
#include <ndir.h>
#endif /* !SYSNDIR */
#else  /* !USG */
#include <sys/dir.h>
#endif /* !USG */
#endif /* !DIRENT && !_POSIX_VERSION */

#if defined(USG) || defined (STDC_HEADERS)
#include <string.h>
#define index strchr
#define rindex strrchr
#define bcopy(s, d, n)  (memcpy ((d), (s), (n)))
#define bcmp(s1, s2, n)  (memcmp ((s1), (s2), (n)))
#define bzero(d, n)  (memset ((d), 0, (n)))
#else
#include <strings.h>
#endif

#if !defined (RETSIGTYPE)
#define RETSIGTYPE void
#endif

/* Assume utmpx always has ut_host */
#if defined (HAVE_UTMPX_H) && !defined (HAVE_UT_HOST)
#define HAVE_UT_HOST 1
#endif

#ifndef MAX
#define MAX(A,B) ((A) > (B) ? (A) : (B))
#endif
#ifdef __EMX__
#include <unistd.h>
#endif
#endif /* !_FINGER_CONFIG_H */
