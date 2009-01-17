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

#include "rexx_charset.h"

#include "rexxsaa.h"
#define DONT_TYPEDEF_PFN
#ifndef RXLIB  /* need __regina_faked_main which is known with RXLIB only */
# define RXLIB
#endif
#include "rexx.h"

int main(int argc, char *argv[])
{
   /* The following value won't change and is const */
   static char *args[] = { "regina", "-c", argv[1], argv[2], NULL } ;
   int rc;

   if (argc!=2) return -1;

   rc = __regina_faked_main(4,args);
   return(rc);
}
