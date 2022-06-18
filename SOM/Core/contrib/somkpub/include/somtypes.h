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
 * standard types for som
 */

#ifndef somtypes_h
#define somtypes_h

#include <stdarg.h>
#include <sombtype.h>

#ifdef	__cplusplus
	class SOMObject;
	class SOMClass;
	class SOMClassMgr;

	#ifndef SOMSTAR
		#define SOMSTAR    *
	#endif /* SOMSTAR */

	#if defined(_MSC_VER)
		#if (_MSC_VER < 1200)
			#define SOM_EXTERN_C_STATIC
		#endif
	#else
		#define SOM_EXTERN_C_STATIC
		#ifdef __GNUC__
		#else
			#define SOM_AVOID_DOUBLE_UNDERSCORES
		#endif
	#endif
#else
	typedef struct 
	{
		struct somMethodTabStruct * mtab;
	} SOMAny,*SOMAnyPtr;

	#ifdef SOM_STRICT_IDL
		typedef SOMAny *  SOMObject;
	#else
		typedef SOMAny     SOMObject;
	#endif

	typedef SOMObject  SOMClass;
	typedef SOMObject  SOMClassMgr; 

	#define _IDL_SOMObject_defined
	#define _IDL_SOMClass_defined
	#define _IDL_SOMClassMgr_defined

	#ifndef SOMSTAR
		#ifdef SOM_STRICT_IDL
			#define SOMSTAR    
		#else
			#define SOMSTAR    *
		#endif
	#endif  /* !SOMSTAR */
#endif /* __cplusplus */


#ifdef __cplusplus
extern "C" {
#endif
	typedef void SOMLINK somTP_somClassInitFunc(SOMClass SOMSTAR);
#ifdef __cplusplus
}
#endif


#endif /* somtypes_h */
