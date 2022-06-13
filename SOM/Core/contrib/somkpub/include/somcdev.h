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

#ifndef somcdev_h
#define somcdev_h

#define SOM_Ok		 0
#define SOM_Warn	 1
#define SOM_Ignore	 2
#define SOM_Template 5
#define SOM_Fatal    9

#define SOM_Assert(condition,ecode)    (somAssert((condition) ? 1 : 0, ecode, __FILE__, __LINE__, # condition))

#ifndef SOM_Trace
#	define SOM_Trace(c,m)
#endif

#ifndef SOM_IgnoreWarning
#	define SOM_IgnoreWarning(x)      ((void)x)
#endif

#ifndef SOMMethodDebug
#	define SOMMethodDebug(c,m) 
#endif

#define SOM_ResolveD(o,oc,cc,m)    \
	((somTD_##cc##_##m)somResolve((SOMObject SOMSTAR)(void *)o,cc##ClassData.m))

#ifndef SOM_Resolve
#	ifdef SOM_METHOD_THUNKS
#		define SOM_Resolve(o,cc,m)    ((somTD_##cc##_##m)(cc##ClassData.m))
#	else
#		define SOM_Resolve(o,cc,m)    \
			((somTD_##cc##_##m)somResolve((SOMObject SOMSTAR)(void *)o,cc##ClassData.m))
#	endif
#endif

SOMEXTERN void 
#	ifdef SOMDLLEXPORT
		SOMDLLEXPORT
#	endif
		SOMLINK SOMInitModule(long,long,char *);

#endif /* somcdev_h */

