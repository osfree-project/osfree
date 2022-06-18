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

#ifdef _WIN32
#	include <windows.h>
#endif

#ifdef HAVE_UNISTD_H
#	include <unistd.h>
#endif

#include <rhbmtut.h>

#ifdef USE_PTHREADS
#	ifdef RHBOPT_PTHREAD_MUTEX_INITIALIZER
static pthread_mutex_t gMutex=RHBOPT_PTHREAD_MUTEX_INITIALIZER;
#	define 	RHBMTUT_GLOBAL_LOCK_INIT
#	else
static pthread_mutex_t gMutex;
static pthread_once_t gMutex_once=PTHREAD_ONCE_INIT;
static void gMutex_init(void)
{
	pthread_mutex_init(&gMutex,RHBOPT_pthread_mutexattr_default);
}
#	define 	RHBMTUT_GLOBAL_LOCK_INIT   pthread_once(&gMutex_once,gMutex_init);
#	endif
#else
#	define 	RHBMTUT_GLOBAL_LOCK_INIT
#endif

#if defined(USE_PTHREADS) && defined(USE_THREADS)
#	define RHBMTUT_GLOBAL_LOCK			pthread_mutex_lock(&gMutex);
#	define RHBMTUT_GLOBAL_UNLOCK		pthread_mutex_unlock(&gMutex);
#else
#	if !defined(USE_THREADS)
#		define RHBMTUT_GLOBAL_LOCK			sigset_t newS,oldS; \
											sigfillset(&newS); \
											sigprocmask(SIG_BLOCK,&newS,&oldS);
#		define RHBMTUT_GLOBAL_UNLOCK		sigprocmask(SIG_SETMASK,&oldS,NULL);
#	endif
#endif


RHBMTUTAPI_(rhbatomic_t) rhbatomic_inc(rhbatomic_t *pl)
{
#if defined(RHBMTUT_GLOBAL_LOCK) && defined(RHBMTUT_GLOBAL_UNLOCK)
	register int x;
	RHBMTUT_GLOBAL_LOCK_INIT
	RHBMTUT_GLOBAL_LOCK
	x=++(*pl);
	RHBMTUT_GLOBAL_UNLOCK
	return x;
#else
	return InterlockedIncrement(pl);
#endif
}

RHBMTUTAPI_(rhbatomic_t) rhbatomic_dec(rhbatomic_t *pl)
{
#if defined(RHBMTUT_GLOBAL_LOCK) && defined(RHBMTUT_GLOBAL_UNLOCK)
	register int x;
	RHBMTUT_GLOBAL_LOCK_INIT
	RHBMTUT_GLOBAL_LOCK
	x=--(*pl);
	RHBMTUT_GLOBAL_UNLOCK
	return x;
#else
	return InterlockedDecrement(pl);
#endif
}

#if defined(_WIN32) && 0
__declspec(dllexport) BOOL CALLBACK DllMain(HINSTANCE hInstance,
											DWORD reason,
											LPVOID args)
{
	return 1;
}
#endif

