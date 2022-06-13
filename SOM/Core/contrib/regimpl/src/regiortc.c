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

#include <rhbopt.h>
#include <rhbsomex.h>
#include <somtc.h>

#if 0
#	include <somtcint.h>
#	define AlignmentHelper(x)  typedef struct x##AlignmentHelper { char q; x z; } x##AlignmentHelper;
#	define Alignment(x)        ((int)&(((struct x##AlignmentHelper *)0)->z))
#endif

#if 0
#ifdef _PLATFORM_MACINTOSH_
	typedef char *string;
#endif
typedef unsigned short unsigned_short;
typedef unsigned long unsigned_long;
#endif

#ifndef _IDL_SEQUENCE_octet_defined
	#define _IDL_SEQUENCE_octet_defined
	SOM_SEQUENCE_TYPEDEF(octet);
#endif

#include <unotypes.h>

#define SOMTC_Scope

#include <regiortc.tc>
