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
 * get the linkages correct based on the platform 
 */

#ifndef somplatf_h
#define somplatf_h

#ifdef _PLATFORM_MACINTOSH_
#	if GENERATINGCFM
#		ifdef USE_APPLE_SOM
#			error put the somincl folder last in include chain
#		endif
#	else
#		define SOM_RESOLVE_DATA
#	endif
#else
#	if defined(_WIN32S) || defined(_WIN16)
#		define SOM_RESOLVE_DATA
#	endif
#endif

#ifndef SOMLINK
#	ifdef _WIN32
#		if 1
#			define SOMLINK  __stdcall
#		else
#			define SOMLINK  __cdecl
#		endif
#	else
#		ifdef __OS2__
#			if defined(__BORLANDC__) || defined(__BCPLUSPLUS__))
#				define SOMLINK __syscall
#			else
#				define SOMLINK _System
#			endif
#		else
#			define SOMLINK
#		endif
#	endif
#endif

#ifndef SOMDLINK
#	ifdef _WIN32
#		define SOMDLINK  
#	else
#		define SOMDLINK
#	endif
#endif

#ifndef SOMEXTERN
#	ifdef __cplusplus
#		define SOMEXTERN extern "C"
#	else
#		define SOMEXTERN extern
#	endif
#endif

#ifndef SOMDLLIMPORT
#	if defined(_WIN32) || defined(HAVE_DECLSPEC_DLLIMPORT)
#		define SOMDLLIMPORT  __declspec(dllimport)
#	endif
#endif

#ifndef SOMDLLEXPORT
#	if defined(_WIN32) || defined(HAVE_DECLSPEC_DLLEXPORT)
#		define SOMDLLEXPORT  __declspec(dllexport)
#	endif
#endif

#if defined(_WIN32) && defined(_MSC_VER)
/* warning C4116: unnamed type definition in parentheses */
#	pragma warning ( disable : 4116 )
#endif

#endif /* somplatf_h */
