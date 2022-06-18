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

#if (!defined(_PLATFORM_BIG_ENDIAN_)) && \
	(!defined(_PLATFORM_LITTLE_ENDIAN_)) && \
	defined(HAVE_SYS_PARAM_H) 
/* 
 *	first try and determine from sys/param.h 
 */
#	include <sys/param.h>
#	if defined(_BYTE_ORDER) && defined(_LITTLE_ENDIAN) && defined(_BIG_ENDIAN)
#		if (_BIG_ENDIAN != _LITTLE_ENDIAN)
#			if (_BYTE_ORDER==_BIG_ENDIAN)
#				define _PLATFORM_BIG_ENDIAN_
#			else
#				if (_BYTE_ORDER==_LITTLE_ENDIAN)
#					define _PLATFORM_LITTLE_ENDIAN_
#				endif
#			endif
#		endif
#	endif
#endif

#if (!defined(_PLATFORM_BIG_ENDIAN_)) && \
	(!defined(_PLATFORM_LITTLE_ENDIAN_))
/* 
 * second, try a couple of simple agnostic macros
 */
#	if defined(_BIG_ENDIAN) && !defined(_LITTLE_ENDIAN)
#		define _PLATFORM_BIG_ENDIAN_
#	endif
#	if defined(_LITTLE_ENDIAN) && !defined(_BIG_ENDIAN)
#		define _PLATFORM_LITTLE_ENDIAN_
#	endif
#endif

#if (!defined(_PLATFORM_BIG_ENDIAN_)) && (!defined(_PLATFORM_LITTLE_ENDIAN_))
/* 
 * thirdly, look for processor specific flags and go from there
 * mips and ia64 could be either endian
 */
#	if  defined(__ppc__) || \
		defined(__m68k__) || \
		defined(__sparc__) || \
		defined(__sparc) || \
		defined(__hppa) || \
		((defined(__mips__)||defined(__mips))&&( defined(__MIPSEB__)||defined(MIPSEB))) || \
        (defined(__ia64__)&&defined(__GNUC__)&&defined(__BIG_ENDIAN__))
#		define _PLATFORM_BIG_ENDIAN_
#	endif
#	if  defined(__i386__) || \
		defined(__i386) || \
		defined(__alpha) || \
		defined(__arm__) || \
		defined(_M_IX86) || \
		defined(__x86_64__) || \
		((defined(__mips__)||defined(__mips))&&(defined(__MIPSEL__)||defined(MIPSEL))) || \
	    (defined(__ia64__)&&defined(__GNUC__)&&!defined(__BIG_ENDIAN__)) 
#		define _PLATFORM_LITTLE_ENDIAN_
#	endif
#endif

#if (!defined(_PLATFORM_BIG_ENDIAN_)) && (!defined(_PLATFORM_LITTLE_ENDIAN_))
	#error could not determine endianness, not enough clues
#endif

#if defined(_PLATFORM_BIG_ENDIAN_) && defined(_PLATFORM_LITTLE_ENDIAN_)
	#error could not determine endianness, result confusing
#endif
