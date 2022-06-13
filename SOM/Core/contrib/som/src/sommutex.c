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

/* $Id$ */

#include <rhbopt.h>

#ifdef _WIN32
	#include <windows.h>
#endif

#include <somkern.h>
#include <rhbmtut.h>

static unsigned long SOMLINK SOMKERN_getThreadId(void)
{
#ifdef USE_PTHREADS
	#ifdef HAVE_PTHREAD_T_SCALAR
		return (unsigned long)pthread_self();
	#else
		union
		{
			pthread_t self;
			unsigned long value;
		} u;
		u.self=pthread_self();
		return u.value;
	#endif
#else
		#ifdef _PLATFORM_WIN32_
			return GetCurrentThreadId();
		#else
			return 0;
		#endif
#endif
}

static unsigned long SOMLINK SOMKERN_createMutexSem(somToken *sem)
{
#ifdef USE_THREADS
	#if defined(USE_PTHREADS)
	struct rhbmutex_t *mutex=SOMMalloc(sizeof(*mutex));
		RHBMUTEX_INIT(mutex);
		*sem=mutex;
	#else
		*sem=CreateMutex(NULL,0,NULL);
	#endif
	return 0;
#else
	return (unsigned long)-1;
#endif
}

static unsigned long SOMLINK SOMKERN_destroyMutexSem(somToken sem)
{
#ifdef USE_THREADS
	#if defined(USE_PTHREADS)
		struct rhbmutex_t *s=sem;
		RHBOPT_ASSERT(s)
		RHBMUTEX_UNINIT(s);
		SOMFree(s);
	#else
		if (!CloseHandle(sem)) 
		{
		#ifdef _M_IX86
			__asm int 3;
		#endif
		}
	#endif
	return 0;
#else
	return (unsigned long)-1;
#endif
}

static unsigned long SOMLINK SOMKERN_requestMutexSem(somToken sem)
{
#ifdef USE_THREADS
	#if defined(USE_PTHREADS)
		RHBMUTEX_LOCK((struct rhbmutex_t *)sem)
	#else
		WaitForSingleObject(sem,INFINITE);
	#endif
	return 0;
#else
	return (unsigned long)-1;
#endif
}

static unsigned long SOMLINK SOMKERN_releaseMutexSem(somToken sem)
{
#ifdef USE_THREADS
	#if defined(USE_PTHREADS)
		struct rhbmutex_t *s=sem;
		#ifdef USE_PTHREADS
			RHBOPT_ASSERT(pthread_equal(pthread_self(),s->tid))
		#else
			RHBOPT_ASSERT(pth_self()==s->tid)
		#endif

		RHBOPT_ASSERT(s->count)

		RHBMUTEX_UNLOCK(s)
	#else
		ReleaseMutex(sem);
	#endif
	return 0;
#else
	return (unsigned long)-1;
#endif
}

SOM_IMPORTEXPORT_som somTD_SOMCreateMutexSem  * SOMDLINK SOMCreateMutexSem=SOMKERN_createMutexSem;
SOM_IMPORTEXPORT_som somTD_SOMDestroyMutexSem * SOMDLINK SOMDestroyMutexSem=SOMKERN_destroyMutexSem;
SOM_IMPORTEXPORT_som somTD_SOMRequestMutexSem * SOMDLINK SOMRequestMutexSem=SOMKERN_requestMutexSem;
SOM_IMPORTEXPORT_som somTD_SOMReleaseMutexSem * SOMDLINK SOMReleaseMutexSem=SOMKERN_releaseMutexSem;
SOM_IMPORTEXPORT_som somTD_SOMGetThreadId     * SOMDLINK SOMGetThreadId=SOMKERN_getThreadId;
