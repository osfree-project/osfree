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

 
/*************************************
 *
 * multi-thread utilities
 *
 */

#ifndef __RHBMTUT_H__
#define __RHBMTUT_H__

#ifdef _WIN32
#	if defined(_DEBUG) && defined(USE_PTHREADS)
typedef short rhbatomic_t;
#	else
typedef long rhbatomic_t;
#	endif
#else
typedef int rhbatomic_t;
#endif

#ifdef USE_THREADS
struct rhbmutex_t
{
#ifdef USE_PTHREADS
	int RHBOPT_volatile count;
	struct {
		pthread_mutex_t mutex;
	} guardian;
	struct {
		pthread_mutex_t mutex;
	} mutex;
	pthread_t RHBOPT_volatile tid;
#else
	LONG RHBOPT_volatile count;
	DWORD RHBOPT_volatile tid;
	CRITICAL_SECTION mutex_crit,guardian_crit;
#endif
};
#endif

#ifdef USE_PTHREADS
#	ifdef RHBOPT_PTHREAD_MUTEX_INITIALIZER
#		define RHBMUTEX_INIT_DATA		{0, \
									{RHBOPT_PTHREAD_MUTEX_INITIALIZER}, \
									{RHBOPT_PTHREAD_MUTEX_INITIALIZER}}
#	endif
#	define RHBMUTEX_INIT(c)		{ (c)->count=0; (c)->tid=pthread_self(); \
								  pthread_mutex_init(&(c)->guardian.mutex,RHBOPT_pthread_mutexattr_default); \
								  pthread_mutex_init(&(c)->mutex.mutex,RHBOPT_pthread_mutexattr_default); }
#	define RHBMUTEX_GETSELF			pthread_t rhbmutex_self=pthread_self();
#	define RHBMUTEX_ISSELF(x)		pthread_equal(rhbmutex_self,(x)->tid)
#	define RHBMUTEX_ACQUIRE(x)		pthread_mutex_lock(&((x)->mutex.mutex));
#	define RHBMUTEX_RELEASE(x)		pthread_mutex_unlock(&((x)->mutex.mutex));
#	define RHBMUTEX_GUARD(x)		pthread_mutex_lock(&((x)->guardian.mutex));
#	define RHBMUTEX_UNGUARD(x)		pthread_mutex_unlock(&((x)->guardian.mutex));
#	define RHBMUTEX_UNINIT(x)		{ pthread_mutex_destroy(&((x)->mutex.mutex)); \
									pthread_mutex_destroy(&((x)->guardian.mutex)); }
#else
#	define RHBMUTEX_GETSELF			DWORD rhbmutex_self=GetCurrentThreadId();
#	define RHBMUTEX_ISSELF(x)		(rhbmutex_self==(x)->tid)
#	define RHBMUTEX_ACQUIRE(x)		EnterCriticalSection(&((x)->mutex_crit));
#	define RHBMUTEX_RELEASE(x)		LeaveCriticalSection(&((x)->mutex_crit));
#	define RHBMUTEX_GUARD(x)		EnterCriticalSection(&((x)->guardian_crit));
#	define RHBMUTEX_UNGUARD(x)		LeaveCriticalSection(&((x)->guardian_crit));
#	define RHBMUTEX_INIT(x)			{ (x)->count=0; (x)->tid=GetCurrentThreadId(); \
										InitializeCriticalSection(&((x)->mutex_crit)); \
										InitializeCriticalSection(&((x)->guardian_crit)); }
#	define RHBMUTEX_UNINIT(x)		{ DeleteCriticalSection(&((x)->mutex_crit)); \
										DeleteCriticalSection(&((x)->guardian_crit)); }
#endif

#define RHBMUTEX_TEST(x)		(RHBMUTEX_ISSELF(x) && ((x)->count))

#ifdef _DEBUG
	#define RHBMUTEX_LOCK(x)	rhbmutex_lock(x);
	#define RHBMUTEX_UNLOCK(x)	rhbmutex_unlock(x);
#else
	#define RHBMUTEX_LOCK(x)	{ RHBMUTEX_GETSELF RHBMUTEX_GUARD(x) \
								if (RHBMUTEX_TEST(x)) { (x)->count++; RHBMUTEX_UNGUARD(x) \
								} else { RHBMUTEX_UNGUARD(x) \
								RHBMUTEX_ACQUIRE(x) \
								RHBMUTEX_GUARD(x) \
								(x)->tid=rhbmutex_self; (x)->count=1; \
								RHBMUTEX_UNGUARD(x) } }

	#define RHBMUTEX_UNLOCK(x)	{ RHBMUTEX_GUARD(x) if (!--((x)->count)) { \
								  RHBMUTEX_UNGUARD(x) RHBMUTEX_RELEASE(x); } \
								  else { RHBMUTEX_UNGUARD(x) } }
#endif

#define RHBMUTEX_ASSERT_LOCKED(x)	\
	{   RHBMUTEX_GETSELF RHBMUTEX_GUARD(x) \
			if (!RHBMUTEX_TEST(x)) { void **h=NULL; *h=NULL; } \
		RHBMUTEX_UNGUARD(x); }

#ifdef _WIN32
#	define	RHBMTUTCALLTYPE	__stdcall
#else
#	define	RHBMTUTCALLTYPE
#endif

#ifdef BUILD_RHBMTUT
#	if defined(HAVE_DECLSPEC_DLLEXPORT) && !defined(_WIN32)
#		define RHBMTUTAPI_(type) __declspec(dllexport) type RHBMTUTCALLTYPE
#	else
#		define RHBMTUTAPI_(type) extern type RHBMTUTCALLTYPE
#	endif
#else
#	if defined(HAVE_DECLSPEC_DLLIMPORT) && !defined(_WIN32)
#		define RHBMTUTAPI_(type) __declspec(dllimport) type RHBMTUTCALLTYPE
#	else
#		define RHBMTUTAPI_(type) extern type RHBMTUTCALLTYPE
#	endif
#endif

#define RHBMTUT_RESULT	int

#define RHBMTUTAPI   RHBMTUTAPI_(void)    

#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32) && !defined(BUILD_RHBMTUT) && !defined(USE_PTHREADS)
#	define	rhbatomic_inc(x)		InterlockedIncrement(x)
#	define	rhbatomic_dec(x)		InterlockedDecrement(x)
#else
RHBMTUTAPI_(rhbatomic_t) rhbatomic_inc(rhbatomic_t *);
RHBMTUTAPI_(rhbatomic_t) rhbatomic_dec(rhbatomic_t *);
#endif

#if defined(USE_THREADS) || defined(_REENTRANT)
	RHBMTUTAPI rhbmutex_lock(struct rhbmutex_t *);
	RHBMTUTAPI rhbmutex_unlock(struct rhbmutex_t *);
	RHBMTUTAPI rhbmutex_init(struct rhbmutex_t *);
	RHBMTUTAPI rhbmutex_uninit(struct rhbmutex_t *);
	RHBMTUTAPI_(int) rhbmutex_locked(struct rhbmutex_t *);
	RHBMTUTAPI_(int) rhbmutex_wait(struct rhbmutex_t *,
	#ifdef USE_PTHREADS
			unsigned long,pthread_cond_t *
	#else
			DWORD,HANDLE
	#endif								 
										 );

typedef struct { struct RHBThreadTaskVtbl *lpVtbl; } RHBThreadTask;
typedef struct { struct RHBThreadPoolVtbl *lpVtbl; } RHBThreadPool;

#define RHBMTUTVtbl_QI(x)   \
		RHBMTUT_RESULT (RHBMTUTCALLTYPE * QueryInterface)(x *,void *,void **); \
		long (RHBMTUTCALLTYPE * AddRef)(x *); \
		long (RHBMTUTCALLTYPE * Release)(x *);

RHBMTUTAPI_(RHBMTUT_RESULT) RHBThreadPoolNew(RHBThreadPool **);

struct RHBThreadPoolVtbl
{
	RHBMTUTVtbl_QI(RHBThreadPool)
	RHBMTUT_RESULT (RHBMTUTCALLTYPE * Create)(RHBThreadPool *,RHBThreadTask *);
	RHBMTUT_RESULT (RHBMTUTCALLTYPE * Close)(RHBThreadPool *);
};

struct RHBThreadTaskVtbl
{
	RHBMTUTVtbl_QI(RHBThreadTask)
	RHBMTUT_RESULT (RHBMTUTCALLTYPE * Run)(RHBThreadTask *);
};
#endif /* USE_THREADS */

#ifdef __cplusplus
}
#endif

#endif /* __RHBMTUT_H__ */
