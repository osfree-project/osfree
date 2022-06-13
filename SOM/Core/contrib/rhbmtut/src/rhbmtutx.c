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

#if defined(_WIN32) && (!defined(USE_PTHREADS))
#	include <windows.h>
#endif

#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <rhbmtut.h>

#ifdef HAVE_ERRNO_H
#	include <errno.h>
#endif

#ifdef _WIN32
#else
#	include <unistd.h>
#endif

#if defined(_DEBUG) && defined(USE_THREADS)
#	undef RHBOPT_ASSERT_FAILED
#	define RHBOPT_ASSERT_FAILED(a,b,c) rhbmtux_failed(a,b,c)
static void rhbmtux_failed(char *file,int line,char *comment)
{
#	ifdef _WIN32
#		ifdef _M_IX86
		__asm int 3
#		else
			((int *)0)[0]=0;
#		endif
#	else
char buf[256];
#		ifdef HAVE_SNPRINTF
	int i=snprintf(buf,sizeof(buf),"rhbmtux_failed(%s,%d): %s\n",
		file,line,comment);
#		else
	int i=sprintf(buf,"rhbmtux_failed(%s,%d): %s\n",
		file,line,comment);
#		endif
	write(2,buf,i);
#	endif
}
#endif

#ifdef USE_THREADS
RHBMTUTAPI rhbmutex_lock(struct rhbmutex_t *mutex)
{
	RHBMUTEX_GETSELF

	RHBMUTEX_GUARD(mutex)

	if (RHBMUTEX_TEST(mutex))
	{
		mutex->count++;

		RHBMUTEX_UNGUARD(mutex)
	}
	else
	{
		RHBMUTEX_UNGUARD(mutex)

		RHBMUTEX_ACQUIRE(mutex)

		RHBMUTEX_GUARD(mutex)

		RHBOPT_ASSERT(!mutex->count);

		mutex->tid=rhbmutex_self;
		mutex->count=1;

		RHBMUTEX_UNGUARD(mutex)
	}
}

RHBMTUTAPI rhbmutex_unlock(struct rhbmutex_t *mutex)
{
	RHBOPT_ASSERT(rhbmutex_locked(mutex));

	RHBMUTEX_GUARD(mutex)

	if (!--(mutex->count))
	{
		RHBMUTEX_UNGUARD(mutex)

		RHBMUTEX_RELEASE(mutex)
	}
	else
	{
		RHBMUTEX_UNGUARD(mutex)
	}
}

RHBMTUTAPI rhbmutex_init(struct rhbmutex_t *mutex)
{
#ifdef RHBMUTEX_INIT_DATA
	struct rhbmutex_t init_tmp=RHBMUTEX_INIT_DATA;
	*mutex=init_tmp;
#else
	RHBMUTEX_INIT(mutex);
#endif
}

RHBMTUTAPI rhbmutex_uninit(struct rhbmutex_t *mutex)
{
	RHBOPT_ASSERT(!mutex->count);

#ifdef RHBMUTEX_UNINIT
	RHBMUTEX_UNINIT(mutex);
#endif
}

RHBMTUTAPI_(int) rhbmutex_locked(struct rhbmutex_t *mutex)
{
	RHBMUTEX_GETSELF

	return RHBMUTEX_TEST(mutex);
}

struct rhbmutex_timed_wait
{
	struct rhbmutex_t *mutex;
#ifdef USE_PTHREADS
	pthread_t tid;
	int count;
#else
	DWORD tid;
	LONG count;
#endif
};

RHBOPT_cleanup_begin(rhbmutex_wait_cleanup,pv)

struct rhbmutex_timed_wait *data=pv;
struct rhbmutex_t *mutex=data->mutex;

	mutex->tid=data->tid;
	mutex->count=data->count;

RHBOPT_cleanup_end

RHBMTUTAPI_(int) rhbmutex_wait(struct rhbmutex_t *mutex,
#ifdef USE_PTHREADS
		unsigned long timeOut,pthread_cond_t *cond
#else
		DWORD timeOut,HANDLE hEvent
#endif								 
	 )
{
struct rhbmutex_timed_wait data;
int rc=-1;
static unsigned long forever=~0UL;

	RHBOPT_ASSERT(rhbmutex_locked(mutex));

	data.mutex=mutex;
	data.tid=mutex->tid;
	data.count=mutex->count;
	
	mutex->count=0;

	RHBOPT_cleanup_push(rhbmutex_wait_cleanup,&data);

#ifdef USE_PTHREADS
	if (timeOut==forever)
	{
		RHBOPT_Trace("wait forever");
		rc=pthread_cond_wait(cond,&mutex->mutex.mutex);
		RHBOPT_ASSERT(rc!=EINVAL);
	}
	else
	{
		struct timespec ts;
		time_t now;
		time(&now);
		ts.tv_sec=now+timeOut;
		ts.tv_nsec=0;
		RHBOPT_Trace("wait with timeout");
		rc=pthread_cond_timedwait(cond,&mutex->mutex.mutex,&ts);
		RHBOPT_ASSERT(rc!=EINVAL);
	}
#else
		if (timeOut == forever) { timeOut=INFINITE; } else { timeOut<<=10; }

		__try
		{
			RHBMUTEX_RELEASE(mutex)

#	ifdef HAVE_WAITFORSINGLEOBJECTEX
			rc=WaitForSingleObjectEx(hEvent,timeOut,TRUE);
#	else
			rc=WaitForSingleObject(hEvent,timeOut);
#	endif
			switch (rc)
			{
			case -1:
				rc=GetLastError();
				break;
			case WAIT_OBJECT_0:
				rc=0;
				break;
			case WAIT_ABANDONED:
				rc=ERROR_SEM_OWNER_DIED;
				break;
			default:
				rc=ERROR_SEM_TIMEOUT;
				break;
			}
		}
		__finally
		{
			RHBMUTEX_ACQUIRE(mutex)
		}
#endif

	RHBOPT_cleanup_pop();

	return rc;
}
#else /* USE_THREADS */
#	if defined(_WIN32)
	/***********************
	 *
	 * blank entries for def file
	 *
	 */
	void rhbmutex_init(void) {}
	void rhbmutex_uninit(void) {}
	void rhbmutex_locked(void) {}
	void rhbmutex_wait(void) {}
	void rhbmutex_lock(void) {}
	void rhbmutex_unlock(void) {}
#	endif /* _WIN32 */
#endif /* USE_THREADS */

