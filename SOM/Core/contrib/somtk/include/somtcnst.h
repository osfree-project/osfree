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

#ifndef somtcnst_h
#define somtcnst_h

#ifdef __cplusplus
	#include <som.xh>
#else
	#include <som.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef SOM_RESOLVE_DATA
	#define SOMTCNST_DEF_DATA(x)		SOMEXTERN TypeCode SOMLINK resolve_##x(void);
	#define SOMTCNST_REF_DATA(x)		(resolve_##x())
#else
	#define SOMTCNST_REF_DATA(x)		((TypeCode)(void *)&x)

	#if !defined(SOMDLLIMPORT) || defined(SOM_DONTIMPORT_SOMTC)
		#define SOMTCNST_DEF_DATA(x)	SOMEXTERN somToken SOMDLINK x;
	#else
		#define SOMTCNST_DEF_DATA(x)	SOMEXTERN SOMDLLIMPORT somToken SOMDLINK x;
	#endif
#endif

#define TC_null                     SOMTCNST_REF_DATA(TC__null)
#define TC_void                     SOMTCNST_REF_DATA(TC__void)
#define TC_short                    SOMTCNST_REF_DATA(TC__short)
#define TC_long                     SOMTCNST_REF_DATA(TC__long)
#define TC_ushort                   SOMTCNST_REF_DATA(TC__ushort)
#define TC_ulong                    SOMTCNST_REF_DATA(TC__ulong)
#define TC_float                    SOMTCNST_REF_DATA(TC__float)
#define TC_double                   SOMTCNST_REF_DATA(TC__double)
#define TC_boolean                  SOMTCNST_REF_DATA(TC__boolean)
#define TC_char                     SOMTCNST_REF_DATA(TC__char)
#define TC_octet                    SOMTCNST_REF_DATA(TC__octet)
#define TC_any                      SOMTCNST_REF_DATA(TC__any)
#define TC_TypeCode                 SOMTCNST_REF_DATA(TC__TypeCode)
#define TC_Principal                SOMTCNST_REF_DATA(TC__Principal)
#define TC_Object                   SOMTCNST_REF_DATA(TC__Object)
#define TC_string                   SOMTCNST_REF_DATA(TC__string)
#define TC_NamedValue               SOMTCNST_REF_DATA(TC__NamedValue)
#define TC_InterfaceDescription     SOMTCNST_REF_DATA(TC__InterfaceDescription)
#define TC_OperationDescription     SOMTCNST_REF_DATA(TC__OperationDescription)
#define TC_AttributeDescription     SOMTCNST_REF_DATA(TC__AttributeDescription)
#define TC_ParameterDescription     SOMTCNST_REF_DATA(TC__ParameterDescription)
#define TC_RepositoryDescription    SOMTCNST_REF_DATA(TC__RepositoryDescription)
#define TC_ModuleDescription        SOMTCNST_REF_DATA(TC__ModuleDescription)
#define TC_ConstDescription         SOMTCNST_REF_DATA(TC__ConstDescription)
#define TC_ConstantDescription      SOMTCNST_REF_DATA(TC__ConstDescription)
#define TC_ExceptionDescription     SOMTCNST_REF_DATA(TC__ExceptionDescription)
#define TC_TypeDescription          SOMTCNST_REF_DATA(TC__TypeDescription)
#define TC_FullInterfaceDescription SOMTCNST_REF_DATA(TC__FullInterfaceDescription)

SOMTCNST_DEF_DATA(TC__null)
SOMTCNST_DEF_DATA(TC__void)
SOMTCNST_DEF_DATA(TC__short)
SOMTCNST_DEF_DATA(TC__long)
SOMTCNST_DEF_DATA(TC__ushort)
SOMTCNST_DEF_DATA(TC__ulong)
SOMTCNST_DEF_DATA(TC__float)
SOMTCNST_DEF_DATA(TC__double)
SOMTCNST_DEF_DATA(TC__boolean)
SOMTCNST_DEF_DATA(TC__char)
SOMTCNST_DEF_DATA(TC__octet)
SOMTCNST_DEF_DATA(TC__any)
SOMTCNST_DEF_DATA(TC__TypeCode)
SOMTCNST_DEF_DATA(TC__Principal)
SOMTCNST_DEF_DATA(TC__Object)
SOMTCNST_DEF_DATA(TC__string)
SOMTCNST_DEF_DATA(TC__NamedValue)
SOMTCNST_DEF_DATA(TC__InterfaceDescription)
SOMTCNST_DEF_DATA(TC__OperationDescription)
SOMTCNST_DEF_DATA(TC__AttributeDescription)
SOMTCNST_DEF_DATA(TC__ParameterDescription)
SOMTCNST_DEF_DATA(TC__RepositoryDescription)
SOMTCNST_DEF_DATA(TC__ModuleDescription)
SOMTCNST_DEF_DATA(TC__ConstDescription)
SOMTCNST_DEF_DATA(TC__ExceptionDescription)
SOMTCNST_DEF_DATA(TC__TypeDescription)
SOMTCNST_DEF_DATA(TC__FullInterfaceDescription)

#undef SOMTCNST_DEF_DATA

#ifndef SOM_DONTIMPORT_SOMTC
	#ifndef SOM_RESOLVE_DATA
		#ifdef PRAGMA_IMPORT_SUPPORTED
			#if PRAGMA_IMPORT_SUPPORTED
				#ifdef BUILD_SOMTC
					#pragma export list TC__null,TC__void,TC__short,TC__long,\
						TC__ushort,TC__ulong,TC__float,TC__double,TC__boolean,\
						TC__char,TC__octet,TC__any,TC__TypeCode,TC__Principal,\
						TC__Object,TC__string,TC__NamedValue,\
						TC__InterfaceDescription,TC__OperationDescription,\
						TC__AttributeDescription,TC__ParameterDescription,\
						TC__RepositoryDescription,TC__ModuleDescription,\
						TC__ConstDescription,TC__ExceptionDescription,\
						TC__TypeDescription,TC__FullInterfaceDescription
				#else
					#pragma import list TC__null,TC__void,TC__short,TC__long,\
						TC__ushort,TC__ulong,TC__float,TC__double,TC__boolean,\
						TC__char,TC__octet,TC__any,TC__TypeCode,TC__Principal,\
						TC__Object,TC__string,TC__NamedValue,\
						TC__InterfaceDescription,TC__OperationDescription,\
						TC__AttributeDescription,TC__ParameterDescription,\
						TC__RepositoryDescription,TC__ModuleDescription,\
						TC__ConstDescription,TC__ExceptionDescription,\
						TC__TypeDescription,TC__FullInterfaceDescription
				#endif
			#endif
		#endif
	#endif
#endif

#ifdef __cplusplus
}
#endif

#endif /* somtcnst_h */
