/* defines.h -- standard defines 

  COPYRIGHT NOTICE AND DISCLAIMER:

  Copyright (C) 2003 Gregory Pietsch

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA.
*/

#ifndef DEFINES_H
#define DEFINES_H

#if defined(__STDC__) || defined(STDC_HEADERS) || defined(HAVE_STDDEF_H)
#include <stddef.h>
#endif

typedef void fvoid_t ();

typedef enum capacity
{
  default_size,
  reserve
} capacity;

#ifndef NPOS
#define NPOS ((size_t)(-1))
#endif

void Nomemory ();

#endif

/* END OF FILE */
