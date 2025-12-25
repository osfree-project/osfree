/**************************************************************************
 *
 *  Copyright 1998-2010, Roger Brown
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

 /********************************************************
 * Portability layer
 *
 * platform configurations tested on
 *
 * Operating Systems:
 *
 *	  Solaris, 6, 7, 8, 9, 10 
 *	  SunOS 4.1.1U, 4.1.4, 
 *	  IRIX 6.5.22m, 
 *	  AIX 5.1, 
 *	  Tru64 5.1,
 *	  HP/UX 11.11,
 *	  A/UX, 3.1.1
 *	  NetBSD 1.3.3, 1.6.2, 2.0, 3.0.1, 
 *	  OpenBSD, 3.8
 *	  FreeBSD 6.0, 
 *	  DragonFly,
 *	  Darwin, 6, 7, 8
 *	  Linux 1.2.13, 2.0.35, 2.2.*, 2.4.*, 2.6.*, 
 *	  Win32, 98, NT 3.51, 4.0, 2000, XP, Vista, Win7, 2003, 2008
 *    WinCE
 *	  Cygwin
 *	  SFU 3.5
 *    BeOS 5
 *    QNX 6.3.2
 * 
 * CPUs:
 *	  powerpc, 601, 603, 604, G3, POWER3
 *	  i386, et al...
 *	  m68k, 68000, 68020, 68030, 68040
 *	  sparc, v7, v8, v9
 *	  alpha,
 *	  mips, R4600, R4400, R10000, R12000
 *	  ia64,
 *	  x86_64,
 *	  pa-risc, pa11_32, pa20_32, pa20_64
 *
 * Compilers:
 *	  gcc
 *	  MIPSpro
 *	  SUNWspro
 *	  Digital C
 *	  Microsoft Visual C++
 *	  HP aCC
 *	  BorlandC
 *	  xlc
 */

#ifndef __RHBOPT_H__
#define __RHBOPT_H__

#if defined(__cplusplus) && defined(HAVE_CONFIG_HPP)
#	include <config.hpp>
#else
#	ifdef HAVE_CONFIG_H
#		include <config.h>
#	else
#		ifdef _WIN32
#			define HAVE_DECLSPEC_DLLIMPORT
#			define HAVE_DECLSPEC_DLLEXPORT
#		endif
#	endif
#endif

#if defined(_WIN32) && defined(_MSC_VER)

#	ifdef WM_QUIT
		#error do not include windows.h till after rhbopt.h
#	endif

	#pragma warning ( disable : 4786 )
	#pragma warning ( disable : 4214 )
	#pragma warning ( disable : 4201 )
	/* unused inline expansion */
	#pragma warning ( disable : 4514 )
	/* unreferenced formal parameter...*/
	#pragma warning ( disable : 4100 )
	/*  unamed type definition in parentheses [sic] */
	#pragma warning ( disable : 4116 )
	/* method ptr -> data ptr*/
	#pragma warning ( disable : 4054 )
	/* data ptr -> method ptr*/
	#pragma warning ( disable : 4055 )
#endif

#ifdef _DEBUG_X
#	define  RHBOPT_Trace(x)     { const char *__RHBOPT_Trace_p; __RHBOPT_Trace_p=x; \
							fprintf(stderr,"%s:%d,%s\n",__FILE__,__LINE__,__RHBOPT_Trace_p); \
							fflush(stderr); }
#else
#	define RHBOPT_Trace(x) 
#endif

/* for Windows, 
	the MSVC compilers always generates '_WIN32'
	additionally, when compiling multi-threaded, it adds '_MT'
	and when the runtime library is a DLL it adds '_DLL'
	*/

#ifdef _WIN32
/* WINDOWS OPTIONS - start */
#	if ((!defined(_MT)) || (!defined(_DLL))) && (!defined(_WIN32_WCE))
		#error Use the multithreading DLL libraries
#	endif

#	ifdef AF_INET
		#error include winsock[2].h after rhbopt.h
#	endif

#	ifndef _PLATFORM_WIN32_
#		define _PLATFORM_WIN32_
#	endif /* _PLATFORM_WIN32_ */

#	ifndef WIN32_LEAN_AND_MEAN
		#error use WIN32_LEAN_AND_MEAN
#	endif

#	ifndef _PLATFORM_LITTLE_ENDIAN_
#		define _PLATFORM_LITTLE_ENDIAN_
#	endif

#	if (_MSC_VER < 1200)
	/* compiling with below MSVC 6 */
#		ifdef _WIN32S
#			define RHBOPT_SHARED_DATA
#		else
#			if 1
#				define USE_THREADS
#			endif

#			ifdef USE_THREADS
#				ifdef HAVE_PTHREAD_H
#					define USE_PTHREADS
#				else
#					define USE_PTH		/* this to be ignored */ 
#				endif
#			endif
#		endif
#	else
	/* compiling with MSVC 6 */
#		define USE_THREADS  
#		if defined(_DEBUG) && !defined(_ARM_)
#			include <crtdbg.h>
#		endif
#	endif

#	ifndef USE_THREADS
#		define USE_SELECT
#	endif

#	ifdef __cplusplus
#		ifndef HAVE_CONFIG_HPP
			#error have not defined HAVE_CONFIG_HPP
#		endif
#	else
#		ifndef HAVE_CONFIG_H
			#error have not defined HAVE_CONFIG_H
#		endif
#	endif

	/* WINDOWS OPTIONS - end */
#else   /* now not _WIN32 */
#	ifdef _PLATFORM_MACINTOSH_
#		include <MacTypes.h>
#		ifndef GENERATINGCFM
#			if defined(powerc) || defined(__powerc) || defined(__CFM68K__)
#				ifndef __ASLM__
#					define GENERATINGCFM   1
#				endif
#			endif
#		endif
#		if GENERATINGCFM
			typedef long integer4;
			typedef unsigned long uinteger4;
#			define USE_THREADS
#			define USE_PTHREADS
#			define USE_APPLE_SOM
#			include <CodeFragments.h>
#		else
#			define USE_SELECT
#			define USE_ASLM
#			define RHBOPT_SHARED_DATA
#			include <LibraryManager.h>
#		endif
#	else /* now not _PLATFORM_MACINTOSH_ */
#		ifdef __OS2__
#			ifndef _PLATFORM_OS2_
#				define _PLATFORM_OS2_
#			endif
#			define USE_THREADS
#		else
#			if defined(_REENTRANT) && (defined(HAVE_PTHREAD_H)||defined(HAVE_PTH_H))
#				define USE_THREADS
#			else   /* now not _REENTRANT */
#				define USE_SELECT
#			endif  /* _REENTRANT */
#			ifdef _PLATFORM_UNIX_
#				ifndef _PLATFORM_X11_
/*					#error currently require X11 for UNIX build */
#				endif
#				ifdef USE_THREADS
#					ifdef HAVE_PTHREAD_H
#						define USE_PTHREADS
#					else
#						error no other supported threading model
#					endif
#				endif /* USE_THREADS */
#			else
				#error unknown platform, not __OS2__, _PLATFORM_UNIX_, _PLATFORM_MACINTOSH_ or _WIN32
#			endif /* not _PLATFORM_UNIX_ */
#		endif /* not __OS2__ */
#	endif /* not _PLATFORM_MACINTOSH_ */
#endif  /* not _WIN32 */

#ifndef SOM_STRICT_IDL
#	ifndef __cplusplus
#		if (!defined(_PLATFORM_MACINTOSH_)) && (!defined(__OS2__))
#			define SOM_STRICT_IDL 
#		endif
#	endif
#endif

#ifndef USE_SELECT
#	ifndef USE_THREADS
		#error Neither USE_SELECT or USE_THREADS has been choosen
#	endif
#endif

#if defined(__cplusplus) || defined(__STDC__) || defined(_WIN32)
#	define RHBOPT_PROTOTYPES		1
#	define RHBOPT_PROTOTYPE(x)		x
#else
#	define RHBOPT_PROTOTYPE(x)		()
#endif

#ifdef USE_THREADS
#	ifdef USE_PTHREADS
#		include <pthread.h>
#		ifdef HAVE_PTHREAD_ATTR_DEFAULT
#			define RHBOPT_pthread_attr_default			pthread_attr_default
#		else
#			define RHBOPT_pthread_attr_default			NULL
#		endif
#		ifdef HAVE_PTHREAD_CONDATTR_DEFAULT
#			define RHBOPT_pthread_condattr_default     pthread_condattr_default
#		else
#			define RHBOPT_pthread_condattr_default     NULL
#		endif
#		ifdef HAVE_PTHREAD_MUTEXATTR_DEFAULT
#			define RHBOPT_pthread_mutexattr_default    pthread_mutexattr_default
#		else
#			define RHBOPT_pthread_mutexattr_default    NULL
#		endif
#		ifdef USE_PTHREAD_ONCE_INIT_BRACES1
#			define RHBOPT_PTHREAD_ONCE_INIT		{PTHREAD_ONCE_INIT}
#		else
#			ifdef USE_PTHREAD_ONCE_INIT_BRACES2
#				define RHBOPT_PTHREAD_ONCE_INIT		{{PTHREAD_ONCE_INIT}}
#			else
#				define RHBOPT_PTHREAD_ONCE_INIT		PTHREAD_ONCE_INIT
#			endif
#		endif
#		ifdef USE_PTHREAD_MUTEX_INITIALIZER_BRACES1
#			define RHBOPT_PTHREAD_MUTEX_INITIALIZER		{PTHREAD_MUTEX_INITIALIZER}
#		else
#			ifdef USE_PTHREAD_MUTEX_INITIALIZER_BRACES2
#				define RHBOPT_PTHREAD_MUTEX_INITIALIZER		{{PTHREAD_MUTEX_INITIALIZER}}
#			else
#				ifdef USE_PTHREAD_MUTEX_INITIALIZER_BRACES0
#					define RHBOPT_PTHREAD_MUTEX_INITIALIZER		PTHREAD_MUTEX_INITIALIZER
#				endif
#			endif
#		endif
#		ifdef USE_PTHREAD_COND_INITIALIZER_BRACES1
#			define RHBOPT_PTHREAD_COND_INITIALIZER		{PTHREAD_COND_INITIALIZER}
#		else
#			ifdef USE_PTHREAD_COND_INITIALIZER_BRACES2
#				define RHBOPT_PTHREAD_COND_INITIALIZER		{{PTHREAD_COND_INITIALIZER}}
#			else
#				ifdef USE_PTHREAD_COND_INITIALIZER_BRACES0
#					define RHBOPT_PTHREAD_COND_INITIALIZER		PTHREAD_COND_INITIALIZER
#				endif
#			endif
#		endif
#		ifdef PTHREAD_CANCELED
#			define RHBOPT_PTHREAD_CANCELED		(void *)PTHREAD_CANCELED
#		else
#			define RHBOPT_PTHREAD_CANCELED		(void *)-1L
#		endif
#	else
	/*
	 * some other form of threads than pthreads
	 */
#	endif
#else
#	if defined(_WIN32) && !defined(_WIN32S)
#		include <rhbposix.h>
#	endif
#endif

#ifndef RHBOPT_unused
#	define RHBOPT_unused(x)   (void)(x);
#endif

#ifndef RHBOPT_volatile
#	if defined(__STDC__) || defined(__cplusplus) || defined(_WIN32)
#		define RHBOPT_volatile   volatile
#	else
#		define RHBOPT_volatile   
#	endif
#endif

/* thread cleanup, exception, signal handler unwinding etc */

#ifdef USE_THREADS
#	ifdef USE_PTHREADS
#		define RHBOPT_cleanup_push(r,p)	pthread_cleanup_push(r,p); RHBOPT_Trace("RHBOPT_cleanup_push")
#		define RHBOPT_cleanup_pop()		RHBOPT_Trace("RHBOPT_cleanup_pop"); pthread_cleanup_pop(1)
#		define RHBOPT_cleanup_proto(_scope,_pfn,_pv)  _scope void _pfn(void *_pv) 
#		define _RHBOPT_cleanup_begin(_scope,_pfn,_pv)  RHBOPT_cleanup_proto(_scope,_pfn,_pv) \
						{ int __cancel; pthread_setcancelstate(PTHREAD_CANCEL_DISABLE,&__cancel); \
						     RHBOPT_Trace("RHBOPT_cleanup_begin"); { 
#		define RHBOPT_cleanup_begin(_pfn,_pv)  _RHBOPT_cleanup_begin(static,_pfn,_pv) 
#		define RHBOPT_cleanup_end	} RHBOPT_Trace("RHBOPT_cleanup_end"); \
							pthread_setcancelstate(__cancel,NULL); }
#		define RHBOPT_cleanup_call(_pfn,_pv)	_pfn(_pv)
#	else
#		include <rhbseh.h>
		struct __RHBOPT_cleanup	{ void (__stdcall *  routine)(void *,int); void *arg; };
#		define RHBOPT_cleanup_push(r,p)  { RHBOPT_volatile struct __RHBOPT_cleanup __RHBOPT_cleanup={NULL,NULL}; \
					__RHBOPT_cleanup.routine=r; \
					__RHBOPT_cleanup.arg=p; __try {
#		define RHBOPT_cleanup_pop()		} __finally { __RHBOPT_cleanup.routine(__RHBOPT_cleanup.arg,_abnormal_termination()); } }
#		define RHBOPT_cleanup_proto(_scope,_pfn,_pv)  _scope void __stdcall _pfn(void *_pv,int abnormalTermination) 
#		define _RHBOPT_cleanup_begin(_scope,_pfn,_pv)  RHBOPT_cleanup_proto(_scope,_pfn,_pv) { RHBSEH_GUARDIAN_BEGIN
#		define RHBOPT_cleanup_begin(_pfn,_pv)  _RHBOPT_cleanup_begin(static,_pfn,_pv) 
#		define RHBOPT_cleanup_end		RHBSEH_GUARDIAN_END }
#		define RHBOPT_cleanup_call(_pfn,_pv)	_pfn(_pv,0)
#	endif
#else
#	if !defined(_PLATFORM_MACINTOSH_)
#		include <rhbseh.h>
#	endif
#	if defined(__STDC__) || defined(__cplusplus) || defined(_WIN32)
	typedef void *__RHBOPT_cleanup_ptr;
	typedef void (*__RHBOPT_cleanup_proc)(__RHBOPT_cleanup_ptr);
#	define RHBOPT_cleanup_proto(_scope,_pfn,_pv)  _scope void _pfn(__RHBOPT_cleanup_ptr _pv) 
#	define _RHBOPT_cleanup_begin(_scope,_pfn,_pv)  RHBOPT_cleanup_proto(_scope,_pfn,_pv) {
#	else
	typedef (*__RHBOPT_cleanup_proc)();
	typedef char *__RHBOPT_cleanup_ptr;
#	define _RHBOPT_cleanup_begin(_scope,_pfn,_pv)  _scope _pfn(_pv) __RHBOPT_cleanup_ptr _pv; {
#	endif
	struct __RHBOPT_cleanup { __RHBOPT_cleanup_proc routine; __RHBOPT_cleanup_ptr arg; };
#	define RHBOPT_cleanup_push(r,p) 	{ 									\
				RHBOPT_volatile struct __RHBOPT_cleanup __RHBOPT_cleanup; 	\
				__RHBOPT_cleanup.routine=r; 								\
				__RHBOPT_cleanup.arg=(__RHBOPT_cleanup_ptr)p; TRY { 
#	define RHBOPT_cleanup_pop()	} FINALLY __RHBOPT_cleanup.routine(__RHBOPT_cleanup.arg); ENDTRY }
#	define RHBOPT_cleanup_begin(_pfn,_pv)  _RHBOPT_cleanup_begin(static,_pfn,_pv) 
#	define RHBOPT_cleanup_end	}
#	define RHBOPT_cleanup_call(_pfn,_pv)	_pfn(_pv)
#endif

#if defined(_WIN32) && !defined(BUILD_STDC)
#	include <string.h>
#	ifdef HAVE__STRICMP
#		define strcasecmp	_stricmp
#	else
#		define strcasecmp	stricmp
#	endif
#	if defined(HAVE__SNPRINTF) && !defined(HAVE_SNPRINTF) 
#		define HAVE_SNPRINTF
#		define snprintf		 _snprintf
#	endif
#	if defined(HAVE__VSNPRINTF) && !defined(HAVE_VSNPRINTF)
#		define HAVE_VSNPRINTF
#		define vsnprintf	 _vsnprintf
#	endif
#endif

#if defined(_DEBUG)
#	ifndef RHBOPT_ASSERT
#		define RHBOPT_ASSERT(assCond) { if (!(assCond)) RHBOPT_ASSERT_FAILED(__FILE__,__LINE__,#assCond); }
#	endif
#else
#	define RHBOPT_ASSERT(x)
#endif

#ifdef HAVE_STDARG_H
#	include <stdarg.h>
#else
#	include <varargs.h>
#endif

#if !defined(HAVE_VA_COPY) && !defined(va_copy)
#	ifdef HAVE_VA_LIST_OVERFLOW_ARG_AREA
#		define va_copy(t,s)		\
			(t)[0].gpr=(s)[0].gpr; \
			(t)[0].fpr=(s)[0].fpr; \
			(t)[0].overflow_arg_area=(s)[0].overflow_arg_area; \
			(t)[0].reg_save_area=(s)[0].reg_save_area;
#	else
#		if defined(HAVE_VA_LIST_ASSIGN) || defined(HAVE_VA_LIST_SCALAR)
#			define va_copy(t,s)		(t)=(s)
#		else
#			ifdef HAVE_CONFIG_H
				#error can not 2nd guess va_copy
#			endif
#		endif
#	endif
#endif

#ifndef HAVE_SOCKLEN_T
#	define HAVE_SOCKLEN_T
	typedef int socklen_t;
#endif

#if defined(_PLATFORM_LITTLE_ENDIAN_) && defined(_PLATFORM_BIG_ENDIAN_)
	#error cannot be both big and little endian
#endif

#ifdef _WIN32
	struct IRpcStubBuffer;
	struct HINSTANCE__;

#	ifdef __cplusplus
		extern "C"
#	else
		extern
#	endif
	__declspec(dllexport) int __stdcall DllMain(
#	ifdef STRICT
		struct HINSTANCE__ *
#	else
		void *
#	endif
		,unsigned long,void *);
#endif /* _WIN32 */

#if (!defined(_PLATFORM_MACINTOSH_))
#	ifdef _WIN32
#		if !defined(_WIN32S)
#			define USE_SIGNALS
#		endif
#	else
#		if !defined(_WIN16)
#			define USE_SIGNALS
#		endif
#	endif

#	ifdef USE_SIGNALS
#		if defined(USE_PTHREADS) || !defined(USE_THREADS)
#			define USE_SIGACTION
#			define USE_POSIX_SIGNALS
#		endif
#	endif
#endif

#if defined(_DEBUG) && 0
/* included so that tracing works... */
#	include <stdio.h>
#	include <string.h>

#	define  sprintf		do not use
#	define	strcat		do not use
#	define	strcpy		do not use
#endif

#endif /* __RHBOPT_H__ */

