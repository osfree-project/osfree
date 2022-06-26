
// 32 bits OS/2 device driver and IFS support. Provides 32 bits kernel
// services (DevHelp) and utility functions to 32 bits OS/2 ring 0 code
// (device drivers and installable file system drivers).
// Copyright (C) 1995, 1996, 1997  Matthieu WILLM (willm@ibm.net)
//
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

/* From:               */
/* fnmatch.h (emx+gcc) */

#if !defined (_FNMATCH_H)
#define _FNMATCH_H

#if defined (__cplusplus)
extern "C" {
#endif

/* POSIX.2 */

#define FNM_NOMATCH        1

#define FNM_NOESCAPE       16
#define FNM_PATHNAME       32
#define FNM_PERIOD         64

#if !defined (_POSIX_SOURCE) || defined (_WITH_UNDERSCORE)

/* emx extensions */

#define _FNM_MATCH         0
#define _FNM_ERR           2

#define _FNM_STYLE_MASK    15

#define _FNM_POSIX         0
#define _FNM_OS2           1
#define _FNM_DOS           2

#define _FNM_IGNORECASE    128
#define _FNM_PATHPREFIX    256

int _fnmatch (__const__ char *, __const__ char *, int);

#endif


#if defined (__cplusplus)
}
#endif

#endif /* !defined (_FNMATCH_H) */
