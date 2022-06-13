/**************************************************************************
 *
 *  Copyright 2011, Roger Brown
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

#ifndef _RHBWINEX_H_
#	define _RHBWINEX_H_

#	if defined(_WIN32)

#		if defined(_MSC_VER) && (_MSC_VER < 1200)
			typedef	unsigned long			SIZE_T;
			typedef int						INT_PTR;
#		endif

#	endif

#	if defined(_WIN32) && !defined(_WIN64)

#		if defined(_MSC_VER) && (_MSC_VER < 1400) && !defined(GWLP_HINSTANCE)
#			define GetWindowLongPtr(x,y)	GetWindowLong(x,y)
#			define SetWindowLongPtr(x,y,z)	SetWindowLong(x,y,z)
#			define GWLP_HINSTANCE			GWL_HINSTANCE
#			define GWLP_USERDATA			GWL_USERDATA
#			define GWLP_WNDPROC				GWL_WNDPROC
#		endif

#	endif

#endif
