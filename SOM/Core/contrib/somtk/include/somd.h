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

#ifndef __SOMD_H__
#define __SOMD_H__

#ifdef __cplusplus
	#include <som.xh>
#else
	#include <som.h>
#endif

#ifdef _PLATFORM_MACINTOSH_
	#ifdef USE_APPLE_SOM
		typedef struct { long highWord; unsigned long lowWord; } IDL_longlong;
		typedef struct { unsigned long highWord; unsigned long lowWord; } IDL_ulonglong;
		typedef long double IDL_longdouble;
		typedef unsigned short IDL_wchar;
		typedef IDL_wchar *IDL_wstring;
	#endif
	#ifndef TypeCode_tk_alias
		#define TypeCode_tk_alias	  22
		#define TypeCode_tk_except	  23
		#define TypeCode_tk_longlong  24
		#define TypeCode_tk_ulonglong 25
		#define TypeCode_tk_longdouble 26
		#define TypeCode_tk_wchar	  27
		#define TypeCode_tk_wstring   28
		#define TypeCode_tk_fixed     29
	#endif
	#ifndef tk_alias
		#define tk_alias	  TypeCode_tk_alias
		#define tk_except	  TypeCode_tk_except
		#define tk_longlong   TypeCode_tk_longlong
		#define tk_ulonglong  TypeCode_tk_ulonglong
		#define tk_longdouble TypeCode_tk_longdouble
		#define tk_wchar      TypeCode_tk_wchar
		#define tk_wstring    TypeCode_tk_wstring
		#define tk_fixed	  TypeCode_tk_fixed
	#endif
#endif

#ifdef __cplusplus
	#include <somir.xh>
	#include <somdtype.xh>
	#include <unotypes.xh>
	#include <naming.xh>
	#include <xnaming.xh>
	#include <snglicls.xh>
	#include <nvlist.xh>
	#include <somdobj.xh>
	#include <cntxt.xh>
	#include <orb.xh>
	#include <request.xh>
	#include <omgidobj.xh>
	#include <omgestio.xh>
	#include <impldef.xh>
	#include <boa.xh>
	#include <somoa.xh>
	#include <somdserv.xh>
	#include <om.xh>
	#include <somdom.xh>
	#include <somproxy.xh>
	#include <somdcprx.xh>
	#include <implrep.xh>
	#include <stexcep.xh>
	#include <principl.xh>
	#include <servmgr.xh>
#else
	#ifdef _PLATFORM_MACINTOSH_
	#else
		#define SOMDServer_VA_EXTERN
		#define SOMMProxyForObject_VA_EXTERN
	#endif
	#include <somir.h>
	#include <somdtype.h>
	#include <unotypes.h>
	#include <naming.h>
	#include <xnaming.h>
	#include <snglicls.h>
	#include <nvlist.h>
	#include <somdobj.h>
	#include <cntxt.h>
	#include <orb.h>
	#include <request.h>
	#include <omgidobj.h>
	#include <omgestio.h>
	#include <impldef.h>
	#include <boa.h>
	#include <somoa.h>
	#include <somdserv.h>
	#include <om.h>
	#include <somdom.h>
	#include <somproxy.h>
	#include <somdcprx.h>
	#include <implrep.h>
	#include <stexcep.h>
	#include <principl.h>
	#include <servmgr.h>
#endif

#include <somdext.h>
#include <somderr.h>

#ifndef _IDL_SEQUENCE_string_defined
	#ifdef _IDL_SEQUENCE_corbastring_defined
		#define _IDL_SEQUENCE_string_defined
		typedef _IDL_SEQUENCE_corbastring _IDL_SEQUENCE_string;
	#endif
#endif

#endif /* __SOMD_H__ */
