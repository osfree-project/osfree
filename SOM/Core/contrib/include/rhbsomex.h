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
 
/**************************************************************************
 * standardised SOM environment
 * - make a portable way of doing SOMInitModule
 * - fix differences between Apple's SOM and IBM's SOM
 * - add some tracing and assert macros
 * - add an exception mechanism
 */

#ifndef __RHBSOMEX_H__
#define __RHBSOMEX_H__

#ifndef RHBOPT_SOMDPORT
#	define RHBOPT_SOMDPORT  3053	/* as defined by IANA, 'dsom-server' */
#endif

#if defined(_WIN32) && !defined(SOM_TYPECODE) && defined(_DEBUG)
#	define SOM_TYPECODE		TypeCode_any
		typedef union SOM_TYPECODE *TypeCode;
#endif

#ifdef SOM_Resolve
#	error use rhbsomex.h instead of som.h 
#endif

/* this speeds up compilation be reducing preprocessor macros */
#ifdef SOM_DONT_USE_SHORT_NAMES
#	error dont define SOM_DONT_USE_SHORT_NAMES beforehand
#else
#	define SOM_DONT_USE_SHORT_NAMES
#	ifdef __cplusplus
#		include <som.xh>
#	else
#		include <som.h>
#	endif
#endif

#ifdef USE_APPLE_SOM
#	define SOMInitModule_proto(modname)  SOMEXTERN void SOMLINK modname##_SOMInitModule(long major,long minor,char *classname)
#	define SOMInitModule_call(modname,major,minor,cls)    modname##_SOMInitModule(major,minor,cls)

#	define SOMInitModule_begin(modname)  \
		SOMEXTERN void SOMLINK SOMInitModule_proto(modname); \
		SOMEXTERN void SOMLINK SOMInitModule_proto(modname) {

#	define	SOMInitModule_new(cls)	somNewClassByTT( (somClassDataStructure *) & cls##ClassData)

#	define SOMInitModule_end			}
#else
#	define somNewClassReference(cls)  (cls##ClassData.classObject ? \
			cls##ClassData.classObject : \
			cls##NewClass(cls##_MajorVersion,cls##_MinorVersion))

#	ifdef PORT_FOR_MAC
#		ifdef SOMObject_somFree
#			undef SOMObject_somFree
#		endif
#	endif

#	if defined(BUILD_STATIC)
#		define SOMInitModule_export(modname)
#	else
#		ifdef SOMDLLEXPORT
#			define SOMInitModule_export(modname) SOMEXTERN SOMDLLEXPORT void SOMLINK SOMInitModule \
				(long major,long minor,char *cls) { SOMInitModule_call(modname,major,minor,cls); } 
#		else
#			define SOMInitModule_export(modname) SOMEXTERN void SOMLINK SOMInitModule \
				(long major,long minor,char *cls) { SOMInitModule_call(modname,major,minor,cls); } 
#		endif
#	endif

#	define SOMInitModule_proto(modname)  SOMEXTERN void SOMLINK modname##_SOMInitModule(long major,long minor,char *classname)
#	define SOMInitModule_call(modname,major,minor,cls)    modname##_SOMInitModule(major,minor,cls)
#	define SOMInitModule_begin(modname)  SOMInitModule_proto(modname); \
										  SOMInitModule_export(modname) \
										  SOMInitModule_proto(modname) {

#	define SOMInitModule_new(cls)	cls##NewClass(cls##_MajorVersion,cls##_MinorVersion)

#	define SOMInitModule_end			}

#	ifndef RHBOPT_ASSERT_FAILED
#		define RHBOPT_ASSERT_FAILED(a,b,c)  somAssert(0,SOM_Fatal,a,b,c)
#	endif
#endif

#ifdef USE_THREADS
#else
#	if defined(SOM_TRY) || defined(SOM_CATCH_ALL) || defined(SOM_ENDTRY) || defined(NO_SOM_TRY)
#	else
#		define SOM_TRY				TRY
#		define SOM_CATCH_ALL		CATCH_ALL
#		define SOM_ENDTRY			ENDTRY	
#	endif
#endif

#if defined(USE_APPLE_SOM) && defined(SOMSTAR)
/* this the internal layout as used by SOMobjects for MacOS 2.0.8 */
/* it appears to the same as used by other platforms */
struct somMethodTabStruct 
{
	SOMClass		SOMSTAR	classObject;
	somClassInfo			*classInfo;
	char					*className;
};
#endif

#ifdef _DEBUG_X
#	define  RHBSOM_Trace(x)     { const char *__SOM_Trace_p; __SOM_Trace_p=x; \
							somPrintf("%s:%d,%s\n",__FILE__,__LINE__,__SOM_Trace_p); }
#	if 0
#		undef RHBOPT_Trace
#		define  RHBOPT_Trace(x)     RHBSOM_Trace(x)
#	endif
#else
#	define RHBSOM_Trace(x) 
#endif

#if !defined(USE_APPLE_SOM) && !defined(SOM_NO_OBJECTS) && !defined(BUILD_SOM) && !defined(BUILD_SOMREF) && !defined(BUILD_SOMSTUBS)
#	ifdef __cplusplus
#		include <somref.xh>
#	else
#		include <somref.h>
#	endif
#endif

#ifndef RHBOPT_throw_StExcep
#	define RHBOPT_throw_StExcep(ev,what,m,c) \
		if (NO_EXCEPTION==(ev)->_major) { const char *id=ex_StExcep_##what; \
				StExcep_##what *pv=(StExcep_##what *)SOMMalloc(sizeof(*pv)); \
				if (pv) { pv->minor=SOMDERROR_##m; pv->completed=c; } \
				if (SOM_TraceLevel) { \
					somPrintf("somSetException(%s) at %s:%d\n",id,__FILE__,__LINE__); } \
			somSetException(ev,SYSTEM_EXCEPTION,id,pv);	}
#endif

#ifndef RHBOPT_throw_ODException
#	define RHBOPT_throw_ODException(ev,what) ODSetSOMException(ev,kODErr##what,NULL)
#endif

#ifndef RHBOPT_throw_ODExceptionMessage
#	define RHBOPT_throw_ODExceptionMessage(ev,what,m) ODSetSOMException(ev,kODErr##what,m)
#endif

#ifdef _PLATFORM_MACINTOSH_
#	include <MacTypes.h>
#		ifdef __cplusplus
			extern "C" {
#		endif
		/* implemented in rhbpkern */
		extern void somStartCriticalSection(void);
		extern void somEndCriticalSection(void);

		/* not in Mac headers, implemented in stdc.c */
		extern int strcasecmp(const char *,const char *);

#		ifdef BUILD_RHBPKERN
			#pragma export list somStartCriticalSection,somEndCriticalSection
#		else
			#pragma import list somStartCriticalSection,somEndCriticalSection
#		endif
#		ifdef __cplusplus
			}
#		endif
#	define RHBOPT_CFM_init(shlb_name,param_name)  \
			OSErr shlb_name##_CFM_init(CFragInitBlockPtr); \
			OSErr shlb_name##_CFM_init(CFragInitBlockPtr param_name)
#	define RHBOPT_CFM_term(shlb_name) \
			void shlb_name##_CFM_term(void); \
			void shlb_name##_CFM_term(void)
#endif

#ifndef RHBOPT_ASSERT_FAILED
#	define RHBOPT_ASSERT_FAILED(a,b,c)  somAssert(0,SOM_Fatal,a,b,c)
#endif

#endif /* __RHBSOMEX_H__ */
