/*
 *  The Regina Rexx Interpreter
 *  Copyright (C) 1992  Anders Christensen <anders@solan.unit.no>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

/*
 * $Id: utsname.h,v 1.3 2001/07/29 07:36:21 mark Exp $
 */

#ifndef _UTSNAME_ALREADY_DEFINED_
#define _UTSNAME_ALREADY_DEFINED_

#define _UTSNAME_LENGTH 20

struct utsname
{
  char sysname[_UTSNAME_LENGTH];
  char nodename[_UTSNAME_LENGTH];
  char release[_UTSNAME_LENGTH];
  char version[_UTSNAME_LENGTH];
  char machine[_UTSNAME_LENGTH];
};

#if defined(__cplusplus)
extern "C"
{
#endif
int uname (struct utsname *name);
#if defined(__cplusplus)
}
#endif

#endif /* _UTSNAME_ALREADY_INCLUDED_ */
