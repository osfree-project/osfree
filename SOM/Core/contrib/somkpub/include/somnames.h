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

/****************************************************
 * contains thunking definitions
 * and traditionally contains mapping to short names
 */

#ifndef somnames_h
#define somnames_h

#if defined(_WIN32) && defined(_MSC_VER) && defined(_M_IX86) && !defined(_WIN64)
#	ifndef SOM_METHOD_THUNKS
#		define SOM_METHOD_THUNKS	1
#	endif
#	ifndef SOM_DATA_THUNKS
#		define SOM_DATA_THUNKS		1
#	endif
#	ifndef SOM_METHOD_STUBS
#		define SOM_METHOD_STUBS		1
#	endif
#endif

#endif /* somnames_h */
