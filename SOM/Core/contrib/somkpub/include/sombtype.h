/**************************************************************************
 *
 *  Copyright 2008, Roger Brown
 *
 *  This file is part of Roger Brown's Toolkit.
 *
 *  This program is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License as published by the
 *  Free Software Foundation, either version 3 of the License, or (at your
 *  option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 */

/*
 * $Id$
 */

 /**********************************************
 * base types for som
 */

#ifndef sombtype_h
#define sombtype_h

#include <limits.h>

#ifdef __cplusplus
extern "C" {
#endif
	#ifdef __IBMC__
		typedef void (somMethodProc)(void);
		#pragma linkage(somMethodProc,system)
		typedef somMethodProc *somMethodPtr;
	#else
		typedef void SOMLINK somMethodProc(void);
		typedef void (SOMLINK *somMethodPtr)(void);
	#endif /* __IBMC__ */

	/* should have the definition for somTP_somClassInitFunc,
	    but this really takes a SOMClass as an argument 
		rather than a void *
		*/

#ifdef __cplusplus
}
#endif

/*******************************
 * the integer4 type is needed for the XXXNewClass functions
 */

#if INT_MAX > 65536
	typedef int integer4;
	typedef unsigned int uinteger4;
#else
	#error non 32 bit platform
	typedef long integer4;
	typedef unsigned long uinteger4;
#endif

typedef char **somId;
typedef void *somToken;

#define IN
#define OUT
#define INOUT

#endif
