//
// $Header: /cur/cvsroot/boot/muFSD/microfsd.h,v 1.1.1.1 2006/11/23 08:17:26 valerius Exp $
//

// 32 bits Linux ext2 file system driver for OS/2 WARP - Allows OS/2 to
// access your Linux ext2fs partitions as normal drive letters.
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

#ifndef __vsprintf_h
#define __vsprintf_h

//#include <os2/types.h>

extern void _pascal video_output(char *msg, unsigned short msglen);
extern void _pascal video_crlf(void);
extern void _pascal video_init(void);

int strcmp (const char *s1, const char *s2);
unsigned int strlen (const char *str);

int sprintf(char * buf, const char *fmt, ...);

unsigned long simple_strtoul(const char *cp,char **endp,unsigned int base);
extern int printk(const char *fmt, ...);

#endif /* __vsprintf_h */
