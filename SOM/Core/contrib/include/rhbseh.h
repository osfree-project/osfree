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
 *
 * the purpose is to provide a common exception handling mechanism using
 * TRY/ENDTRY etc macros, this can then be mapped to native structured exception
 * handling on Win32/OS2, and to an abort()/signal(SIGABRT) scheme on UNIX
 *
 * where a platform supports pthreads, expect normal cleanup callbacks be used
 *
 */

#ifndef __RHBSEH__
#define __RHBSEH__

/* recommend _POSIX_SOURCE for _PLATFORM_AUX_ */

#ifdef HAVE_SETJMPEX_H
#	include <setjmpex.h>
#else
#		include <setjmp.h>
#endif

#ifdef _WIN32
#	ifdef _WIN32S
		typedef jmp_buf sigjmp_buf;
#		define USE_NATIVE_SEH
#	else
#		if defined(USE_PTHREADS) || defined(USE_SELECT)
#			include <rhbposix.h>
#		else
			typedef jmp_buf sigjmp_buf;
#			define USE_NATIVE_SEH
#		endif
#	endif
#	define RHBSEHLINK		__stdcall

#	ifndef _MT
		#error build using multithreading
#	endif

#	if !defined(_DLL) && !defined(_WIN32_WCE)
		#error build using DLL CRT
#	endif
#else
#	define RHBSEHLINK
#endif

#ifdef HAVE_SIGNAL_H
#	include <signal.h>
#endif

#ifdef __OS2__
	typedef jmp_buf sigjmp_buf;
#	define USE_NATIVE_SEH
#endif

#ifdef __cplusplus
extern "C" {
#endif

#if defined(USE_NATIVE_SEH)
#	else	
#	if defined(__STDC__) || defined(_WIN32)
#	   define RHBSEH_VOID                  void
#		define RHBSEH_PROTO(param_list)		param_list
#	else
#		define RHBSEH_VOID					
#		define RHBSEH_PROTO(param_list)		()
#	endif
struct __exception
{
	struct __exception *chain;
	char *file; 
	int line;
	int signum;
	char caught,pushed;
	int signal_count;
	int *signals;
	struct sigaction *actions;
	sigjmp_buf jumper;
};

#	if defined(HAVE_DECLSPEC_DLLEXPORT) && defined(BUILD_RHBSEH)
#		define RHBSEHEXTERN __declspec(dllexport) extern
#	else
#		if defined(HAVE_DECLSPEC_DLLIMPORT) && !defined(BUILD_RHBSEH)
#			define RHBSEHEXTERN __declspec(dllimport) extern
#		else
#			define RHBSEHEXTERN extern
#		endif
#	endif

	RHBSEHEXTERN int RHBSEHLINK rhbseh_push RHBSEH_PROTO((struct __exception *));
	RHBSEHEXTERN int RHBSEHLINK rhbseh_pop RHBSEH_PROTO((struct __exception *));
	RHBSEHEXTERN int RHBSEHLINK rhbseh_raise RHBSEH_PROTO((int));

#	undef RHBSEH_PROTO
#	undef RHBSEH_VOID
#endif


#ifdef _WIN32
#	if !defined(BUILD_RHBSEH2)
#		pragma comment(lib,"rhbseh2.lib")
#	endif
struct rhbseh_guardian
{
	struct rhbseh_guardian_vtbl *vtbl;
	unsigned long except_code;
	unsigned char reraise_flag;
};
struct rhbseh_guardian_vtbl
{
	int (__stdcall * except_function)(struct rhbseh_guardian *,void *);
	void (__stdcall * except_final)(struct rhbseh_guardian *);
};
extern void __stdcall rhbseh_guardian(struct rhbseh_guardian *);
#define RHBSEH_GUARDIAN_BEGIN { struct rhbseh_guardian __rhbseh_guardian; \
				rhbseh_guardian(&__rhbseh_guardian); __try { \

#define RHBSEH_GUARDIAN_END  } __except(__rhbseh_guardian.vtbl->except_function(&__rhbseh_guardian,_exception_info())) { } \
						     __rhbseh_guardian.vtbl->except_final(&__rhbseh_guardian); }
#endif

#if defined(USE_NATIVE_SEH)
#	define TRY				{ __try {
#	define EXCEPT(x)		} __except(x) {
#	define CATCH(y)			} __except(_exception_code()==(y)) {
#	define CATCH_ALL		} __except(1) {
#	define FINALLY			} __finally {
#	define ENDTRY			} }
#	ifdef _WIN32
#		define RAISE(x)		RaiseException(x,EXCEPTION_NONCONTINUABLE,0,NULL)
#	else
#		define RAISE(x)		{ EXCEPTIONREPORTRECORD __record={x,EH_NONCONTINUABLE,NULL,NULL,0}; \
								DosRaiseException(&__record); }
#	endif
#else
	/*****************************************
	 * handlers that are installed for each try level, basically
	 * the list of synchronous signals 
     */
#	ifndef TRY_SIGMASK
#		ifdef SIGBUS
#			define RHBSEH_SIGBUS		,SIGBUS
#		else
#			define RHBSEH_SIGBUS		
#		endif
#		ifdef SIGSYS
#			define RHBSEH_SIGSYS		,SIGSYS
#		else
#			define RHBSEH_SIGSYS		
#		endif
#		define TRY_SIGMASK			SIGSEGV,SIGFPE,SIGILL,SIGABRT,SIGPIPE RHBSEH_SIGBUS RHBSEH_SIGSYS
#	endif

#	define TRY_FOR(exList)		{	static int rhbseh_signums[]={exList}; 	\
								struct sigaction __rhbseh_sigactions		\
									[sizeof(rhbseh_signums)/				\
										sizeof(rhbseh_signums[0])]; 		\
								struct __exception __exception; 			\
								__exception.chain=NULL; 					\
								__exception.file=__FILE__; 					\
								__exception.line=__LINE__; 					\
								__exception.signum=0; 						\
								__exception.caught=0; 						\
								__exception.pushed=0;						\
								__exception.signal_count=					\
										sizeof(rhbseh_signums)/				\
											sizeof(rhbseh_signums[0]);		\
								__exception.signals=rhbseh_signums;			\
								__exception.actions=__rhbseh_sigactions; 	\
								rhbseh_push(&__exception);  				\
								if (!sigsetjmp(__exception.jumper,1)) { {

#	define TRY				TRY_FOR(TRY_SIGMASK)

#	define EXCEPT(exBool)	} } else if (__exception.signum && (exBool)) {   \
								__exception.caught=1; rhbseh_pop(&__exception); {

#	define CATCH(exCode)	EXCEPT(__exception.signum==(exCode))

#	define CATCH_ALL		EXCEPT(1)

#	define FINALLY			} } rhbseh_pop(&__exception); { {

#	define ENDTRY			} } rhbseh_pop(&__exception);  \
							if (__exception.signum && !__exception.caught) rhbseh_raise(__exception.signum); }

#	define RAISE(x)			rhbseh_raise(x)
#endif

#ifdef __cplusplus
}
#endif

#endif
