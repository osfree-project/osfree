/* tcp.h -- What we need to finger with TCP protocol. */

/* Copyright (C) 1988, 1990, 1992 Free Software Foundation, Inc.

   This file is part of GNU Finger.

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

#if !defined (_TCP_H_)
#define _TCP_H_

/* Don't forget to include sys/types.h before sys/socket.h. */

/* #include <sys/types.h> */
#include <sys/socket.h>
#ifdef _AIX
#include <netinet/in.h>
#endif /* _AIX */

#ifdef __STDC__
extern int host_cmp (char *, char *);
#else
extern int host_cmp ();
#endif

#endif /* _TCP_H_ */
