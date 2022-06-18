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

#include <rhbopt.h>

/* the purpose of this module is to provide a common 
	exception handling framework

	in order to have a thread unwind correctly it needs
	to maintain a chain of the exception frames pushed
	onto the stack, these need to be popped off
	in the correct order.

	the goal is to allow a thread to unwind and have
	either the exceptions caught by the correct exception
	handler or give that thread a chance to do the correct
	cleanup and pass the exception on to the parent

	in order to comply with UNIX programming signal
	handlers should be maintained by the application
	not by libraries

  */

#include <rhbseh.h>
#include <errno.h>
#include <string.h>

#ifdef _WIN32	
#	ifndef WIN32_LEAN_AND_MEAN
		#error define WIN32_LEAN_AND_MEAN
#	endif
#	include <windows.h>
#	include <process.h>
#	include <winsock.h>
#	include <winerror.h>
#else
#	include <unistd.h>
#	include <stdio.h>
#	include <stdlib.h>
#   include <signal.h>
#   if !defined(HAVE_RAISE) && !defined(raise)
#       define raise(s)     kill(getpid(),(s))
#   endif
#endif

#ifdef USE_THREADS
#	ifdef USE_PTHREADS
#		ifndef PTHREAD_CANCELED
#			define PTHREAD_CANCELED  ((void *)-1)
#		endif
		static pthread_key_t rhbseh_exception_chain_key;
		static pthread_once_t rhbseh_once_init=RHBOPT_PTHREAD_ONCE_INIT;
		static void rhbseh_finally(void *pv) { }
		static void rhbseh_once(void)
		{
			int err=
#ifdef HAVE_PTHREAD_KEY_CREATE
			pthread_key_create(&rhbseh_exception_chain_key,rhbseh_finally);
#else
			pthread_keycreate(&rhbseh_exception_chain_key,rhbseh_finally);
#endif
			if (err) pthread_exit(RHBOPT_PTHREAD_CANCELED);
		}
#	else
		static __declspec(thread) struct __exception *rhbseh_exception_chain=NULL;
#	endif
#else
#	ifndef USE_NATIVE_SEH
#		ifdef _WIN32
			static __declspec(thread) struct __exception *rhbseh_exception_chain=NULL;
#		else
			static struct __exception *rhbseh_exception_chain=NULL;
#		endif
#	endif
#endif

#ifdef _DEBUG
#	define FATAL rhbseh_fatal(__FILE__,__LINE__);
		static void rhbseh_fatal(char *file,int line)
		{
#	ifdef _WIN32
			HANDLE h=GetStdHandle(STD_ERROR_HANDLE);
			if (h!=INVALID_HANDLE_VALUE)
			{
				char buf[256];
				int i=wsprintf(buf,"fatal error at %s:%d\n",file,line);
				ULONG ul=i;
				WriteFile(h,buf,ul,&ul,NULL);
			}

			RaiseException(ERROR_INVALID_THREAD_ID,EXCEPTION_NONCONTINUABLE,0,NULL);
#	else
			char buf[256];
#ifdef HAVE_SNPRINTF
			int i=snprintf(buf,sizeof(buf),"fatal error at %s:%d\n",file,line);
#else
			int i=sprintf(buf,"fatal error at %s:%d\n",file,line);
#endif
			write(2,buf,i);
			exit(1);

#	endif
		}
#endif

static
#if defined(__STDC__) || defined(_WIN32)
		void rhbseh_signal(int sig)
#else
#	ifdef HAVE_SA_HANDLER_VOID_KANDR
		void
#	endif
		rhbseh_signal(sig)
		int sig;
#endif
{
#ifdef USE_THREADS
#	ifdef USE_PTHREADS
#		ifdef HAVE_PTHREAD_GETSPECIFIC_STD
			struct __exception *rhbseh_exception_chain
			=pthread_getspecific(rhbseh_exception_chain_key);
#		else
			pthread_addr_t keyval;
			struct __exception *rhbseh_exception_chain=NULL;
			if (!pthread_getspecific(rhbseh_exception_chain_key,&keyval))
			{
				rhbseh_exception_chain=(struct __exception *)keyval;
			}
#		endif
#	endif
#endif

#if defined(_WIN32) && defined(_DEBUG)
	{
		HANDLE h=GetStdHandle(STD_ERROR_HANDLE);
		if (h!=INVALID_HANDLE_VALUE)
		{
			char buf[256];
			int i=wsprintf(buf,"raised(%d)\n",sig);
			ULONG ul=i;
			WriteFile(h,buf,ul,&ul,NULL);
		}
	}
#endif

#ifdef USE_NATIVE_SEH
	RaiseException(sig,EXCEPTION_NONCONTINUABLE,0,0);
#else
	if (rhbseh_exception_chain)
	{
		rhbseh_exception_chain->signum=sig;
		siglongjmp(rhbseh_exception_chain->jumper,sig);
	}
#endif

	/* don't know where else to go so pass it up
		to the SIGSYS handler or SIGABRT if they exist
	*/

#ifdef SIGSYS
	raise(SIGSYS);
#endif

#ifdef SIGABRT
	raise(SIGABRT);
#endif

	/* they failed to leap us out of the situation
		so exit */

#ifdef USE_THREADS
#	ifdef USE_PTHREADS
		pthread_exit(RHBOPT_PTHREAD_CANCELED);
#	else
		RaiseException(ERROR_CANCELLED,EXCEPTION_NONCONTINUABLE,0,NULL);
#	endif
#else
	_exit(1);
#endif

	/* this is really bad... */
}

#ifdef USE_NATIVE_SEH
int RHBSEHLINK rhbseh_push(ex)
struct __exception *ex;
{
	return -1;
}
int RHBSEHLINK rhbseh_pop(ex)
struct __exception *ex;
{
	return -1;
}
#else
int RHBSEHLINK rhbseh_push(ex)
struct __exception *ex;
{
#ifdef USE_THREADS
#	ifdef USE_PTHREADS
			pthread_once(&rhbseh_once_init,rhbseh_once);
#		ifdef HAVE_PTHREAD_GETSPECIFIC_STD
			ex->chain=pthread_getspecific(rhbseh_exception_chain_key);
			pthread_setspecific(rhbseh_exception_chain_key,ex);
#		else
			{
				pthread_addr_t keyval;
				if (pthread_getspecific(rhbseh_exception_chain_key,&keyval))
				{
					ex->chain=NULL;
				}
				else
				{
					ex->chain=(struct __exception *)keyval;
				}
			}
			pthread_setspecific(rhbseh_exception_chain_key,(pthread_addr_t)ex);
#		endif
#	else
			ex->chain=rhbseh_exception_chain;
			rhbseh_exception_chain=ex;
#	endif
#else
	int i=ex->signal_count;
	ex->chain=rhbseh_exception_chain;
	rhbseh_exception_chain=ex;
	if (i)
	{
		int *p=ex->signals;
		struct sigaction *a=ex->actions;
		struct sigaction action;
		memset(&action,0,sizeof(action));
#ifdef SA_RESTART
		action.sa_flags=SA_RESTART;
#endif
		action.sa_handler=rhbseh_signal;
		while (i--)
		{
			int s=*p++;
			sigaction(s,&action,a);
			a++;
		}
	}
#endif
	ex->pushed=1;

	return 1;
}

int RHBSEHLINK rhbseh_pop(ex)
struct __exception *ex;
{
	if (ex->pushed)
	{		
#ifdef _DEBUG
#	ifdef USE_THREADS
#		ifdef USE_PTHREADS
#			ifdef HAVE_PTHREAD_GETSPECIFIC_STD
				if (ex!=pthread_getspecific(rhbseh_exception_chain_key)) FATAL
#			else
				pthread_addr_t addr=NULL;
				if (pthread_getspecific(rhbseh_exception_chain_key,&addr)) FATAL
				if (addr != (pthread_addr_t)ex) FATAL
#			endif
#		endif
#	else
		if (ex != rhbseh_exception_chain) FATAL
#	endif
#endif

#ifdef USE_THREADS
#	ifdef USE_PTHREADS
#		ifdef HAVE_PTHREAD_ADDR_T
			pthread_setspecific(rhbseh_exception_chain_key,(pthread_addr_t)ex->chain);
#		else
			pthread_setspecific(rhbseh_exception_chain_key,ex->chain);
#		endif
#	else
		rhbseh_exception_chain=ex->chain;
#	endif
#else
		if (ex->signal_count)
		{
			/* uninstall in reverse order */
			int i=ex->signal_count;
			struct sigaction *a=ex->actions+i;
			int *p=ex->signals+i;
			while (i--)
			{
				int s=*--p;
				--a;
				sigaction(s,a,NULL);
			}
		}

		rhbseh_exception_chain=ex->chain;
#endif
		ex->pushed=0;

		return 1;
	}

	return 0;
}
#endif /* !USE_NATIVE_SEH */

#ifdef _WIN32
BOOL CALLBACK DllMain(HINSTANCE hModule,DWORD dw,LPVOID pv)
{
	switch (dw)
	{
	case DLL_PROCESS_ATTACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}

	return 1;
}
#endif

int RHBSEHLINK rhbseh_raise(sig)
int sig;
{
#if defined(_DEBUG) && !defined(USE_NATIVE_SEH)
	sigset_t s;
	sigfillset(&s);
	if (!sigismember(&s,sig))
	{
		FATAL
	}
#endif
#ifdef USE_THREADS
	rhbseh_signal(sig);
#else
	raise(sig);
#endif
	return -1;
}

int rhbseh_init()
{
	return 0;
}

#ifdef RHBSEH_NETBSDELF1_CRT0
/* this is needed to link with -shared -Wl,-z,defs on NetBSD 1.6.2
	my guess is on all pre-2.0 ELF based NetBSD systems
	the problem being that libc.so expects to find these provided
	by the application
	don't get this problem with >2.0 or with a.out, eg 1.3.3
  */
char **environ;
char *__progname;
#endif
