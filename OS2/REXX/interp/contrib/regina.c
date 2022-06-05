/*
 *  The Regina Rexx Interpreter
 *  Copyright (C) 1992-1994  Anders Christensen <anders@pvv.unit.no>
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

#include "regina_c.h"

#if defined(WIN32) && defined(__IBMC__)
# include <windows.h>
# pragma warning(default: 4115 4201 4214)
#else
# define APIENTRY
# if defined(WIN32) && (defined(__WATCOMC__) || defined(__BORLANDC__))
#  undef APIENTRY
#  include <windows.h>
# endif
# if defined(_MSC_VER)
#  undef APIENTRY
#  if _MSC_VER >= 1100
/* Stupid MSC can't compile own headers without warning at least in VC 5.0 */
#    pragma warning(disable: 4115 4201 4214 4514)
#  endif
#  include <windows.h>
#  if _MSC_VER >= 1100
#    pragma warning(default: 4115 4201 4214)
#  endif
#endif
#endif

#include "rexxsaa.h"
#define DONT_TYPEDEF_PFN
#ifndef RXLIB  /* need __regina_faked_main which is known with RXLIB only */
# define RXLIB
#endif
#include "rexx.h"

int main(int argc, char *argv[])
{
   int rc;
   rc = __regina_faked_main(argc,argv);
   return(rc);
}
