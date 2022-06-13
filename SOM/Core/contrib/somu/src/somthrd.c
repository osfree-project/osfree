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
#include <som.h>
#include <rhbsomu.h>

#ifndef BUILD_SOMU
	#error define BUILD_SOM for this part
#endif

#include <somthrd.h>
#include <somderr.h>

#ifndef USE_PTHREADS
	#ifdef _WIN32
		#include <windows.h>
	#endif
#endif

#ifdef USE_THREADS
	static unsigned long SOMLINK RHBSOMU_StartThread(somToken * thrd,
							   somTD_SOMThreadProc *proc,
							   void * data, 
							   unsigned long datasz,
							   unsigned long stacksz);
	static unsigned long SOMLINK RHBSOMU_EndThread(void);
	static unsigned long SOMLINK RHBSOMU_KillThread(somToken thrd);
	static unsigned long SOMLINK RHBSOMU_YieldThread(void);
	static unsigned long SOMLINK RHBSOMU_GetThreadHandle(somToken *thrd);
#endif

#ifdef USE_THREADS
	somTD_SOMStartThread     * SOMDLINK SOMStartThread=RHBSOMU_StartThread;
	somTD_SOMEndThread       * SOMDLINK SOMEndThread=RHBSOMU_EndThread;	
	somTD_SOMKillThread      * SOMDLINK SOMKillThread=RHBSOMU_KillThread;
	somTD_SOMYieldThread     * SOMDLINK SOMYieldThread=RHBSOMU_YieldThread;
	somTD_SOMGetThreadHandle * SOMDLINK SOMGetThreadHandle=RHBSOMU_GetThreadHandle;
#else
	somTD_SOMStartThread     * SOMDLINK SOMStartThread;
	somTD_SOMEndThread       * SOMDLINK SOMEndThread;
	somTD_SOMKillThread      * SOMDLINK SOMKillThread;
	somTD_SOMYieldThread     * SOMDLINK SOMYieldThread;
	somTD_SOMGetThreadHandle * SOMDLINK SOMGetThreadHandle;
#endif

#ifdef USE_THREADS
	struct somu_thread_data
	{
#ifdef USE_PTHREADS
		pthread_t thread;
#else
		HANDLE handle;
		DWORD dwId;
#endif
		somTD_SOMThreadProc *proc;
		void *data;
		octet data_bytes[1];
	};
	#if defined(USE_PTHREADS)
		#ifndef HAVE_PTHREAD_CANCEL
			#if defined(PTHREAD_CANCELED) || defined(PTHREAD_CANCEL_ENABLE)
				#define HAVE_PTHREAD_CANCEL
			#endif
		#endif

			static pthread_once_t somu_once=RHBOPT_PTHREAD_ONCE_INIT;
			static pthread_key_t somu_thread_key;

		static void somu_thread_clearup(void *pv)
		{
			if (pv)
			{
				pthread_setspecific(somu_thread_key,NULL);

				SOMFree(pv);
			}
		}
			static void somu_thread_init(void)
		{
#ifdef HAVE_PTHREAD_KEY_CREATE
			pthread_key_create(&somu_thread_key,somu_thread_clearup);
#else
			pthread_keycreate(&somu_thread_key,somu_thread_clearup);
#endif
		}
		static void *somu_thread_starter(void *pv)
		{
			struct somu_thread_data *somThis=pv;

#ifdef HAVE_PTHREAD_ADDR_T
			pthread_setspecific(somu_thread_key,(pthread_addr_t)(void *)somThis);
#else
			pthread_setspecific(somu_thread_key,somThis);
#endif

			RHBOPT_cleanup_push(somu_thread_clearup,somThis);

			somThis->proc(somThis->data);

			RHBOPT_cleanup_pop();

			return NULL;
		}
	#else
		static DWORD __stdcall somu_thread_starter(void *);
		static void __stdcall somu_stop_thread(DWORD dw);
		static DWORD somu_thread_key;
		typedef DWORD (__stdcall *ptrQueueUserAPC)(void (__stdcall *)(DWORD),HANDLE,DWORD);
		static ptrQueueUserAPC pfnQueueUserAPC;
	#endif
static unsigned long SOMLINK RHBSOMU_StartThread(somToken * thrd,
						   somTD_SOMThreadProc *proc,
						   void * data, 
						   unsigned long datasz,
						   unsigned long stacksz)
{
	struct somu_thread_data *somThis=SOMMalloc(sizeof(*somThis)+datasz);
	unsigned long err=0;

	SOM_IgnoreWarning(stacksz);

	if (thrd) *thrd=NULL;

	if (!somThis) return SOMDERROR_NoMemory;

	somThis->proc=proc;

	if (datasz)
	{
		memcpy(somThis->data_bytes,data,datasz);
		somThis->data=somThis->data_bytes;
	}
	else
	{
		somThis->data=data;
	}

#ifdef USE_PTHREADS
	pthread_once(&somu_once,somu_thread_init);

	if (pthread_create(
				&somThis->thread,
				RHBOPT_pthread_attr_default,
				somu_thread_starter,
				somThis))
	{
		err=SOMDERROR_CouldNotStartThread;
	}
	else
	{
		pthread_detach(somThis->thread);
	}
#else
	somThis->handle=CreateThread(
			0,
			0,
			somu_thread_starter,
			somThis,
			0,
			&somThis->dwId);
	if (!somThis->handle)
	{
		err=SOMDERROR_CouldNotStartThread;
	}
#endif

	if (err)
	{
		SOMFree(somThis);
	}
	else
	{
		if (thrd) *thrd=somThis;
	}

	return err;
}
static unsigned long SOMLINK RHBSOMU_EndThread(void)
{
#ifdef USE_PTHREADS
	pthread_exit(0);
#else
	somu_stop_thread(0);
#endif

	return 0xffffffff;
}
static unsigned long SOMLINK RHBSOMU_KillThread(somToken thrd)
{
	struct somu_thread_data *somThis=thrd;
#ifdef USE_PTHREADS
	#ifdef HAVE_PTHREAD_CANCEL
		return pthread_cancel(somThis->thread);
	#else
		#ifdef _DEBUG
			somPrintf("%s:%d,does not support pthread_cancel()\n",__FILE__,__LINE__);
		#endif
		return 0xffffffff;
	#endif
#else
	if (pfnQueueUserAPC)
	{
		return pfnQueueUserAPC(somu_stop_thread,somThis->handle,(DWORD)somThis);
	}

	return 0xffffffff;
#endif
}
static unsigned long SOMLINK RHBSOMU_YieldThread(void)
{
#ifdef USE_PTHREADS
	return 0;
#else
	Sleep(0);
	return 0;
#endif
}
static unsigned long SOMLINK RHBSOMU_GetThreadHandle(somToken *thrd)
{
#ifdef USE_PTHREADS
	struct somu_thread_data *somThis=NULL;

	#ifdef HAVE_PTHREAD_GETSPECIFIC_STD
		somThis=pthread_getspecific(somu_thread_key);
	#else
		pthread_addr_t addr;
		if (!pthread_getspecific(somu_thread_key,&addr))
		{
			somThis=(void *)addr;
			return 0;
		}
	#endif

	if (somThis)
	{
		*thrd=somThis;
		return 0;
	}

	return 0xffffffff;
#else
	struct somu_thread_data *somThis=TlsGetValue(somu_thread_key);

	if (!somThis)
	{
		somThis=SOMMalloc(sizeof(*somThis));
		somThis->handle=NULL;
		somThis->dwId=GetCurrentThreadId();
		somThis->proc=0;
		somThis->data=0;
		DuplicateHandle(GetCurrentProcess(),
			GetCurrentThread(),
			GetCurrentProcess(),
			&somThis->handle,
			DUPLICATE_SAME_ACCESS,
			0,
			DUPLICATE_SAME_ACCESS);

		TlsSetValue(somu_thread_key,somThis);
	}
	*thrd=somThis;
	return 0;
#endif
}
#endif

#ifdef _WIN32
	void SOMTHRD_init(void)
	{
	#ifdef USE_THREADS
		#ifndef USE_PTHREADS
			somu_thread_key=TlsAlloc();
			pfnQueueUserAPC=(ptrQueueUserAPC)
						GetProcAddress(
							GetModuleHandle("KERNEL32"),
						"QueueUserAPC");
		#endif
	#endif
	}
	void SOMTHRD_term(void)
	{
	#ifdef USE_THREADS
		#ifndef USE_PTHREADS
			TlsFree(somu_thread_key);
		#endif
	#endif
	}

	void SOMTHRD_thread_dead(void)
	{
	#ifdef USE_THREADS
		#ifndef USE_PTHREADS
			struct somu_thread_data *somThis=TlsGetValue(somu_thread_key);

			if (somThis)
			{
				CloseHandle(somThis->handle);
				SOMFree(somThis);
			}
		#endif
	#endif
	}

	#ifdef USE_THREADS
		#ifndef USE_PTHREADS
			static DWORD __stdcall somu_thread_starter(void *pv)
			{
				struct somu_thread_data *somThis=pv;

				TlsSetValue(somu_thread_key,somThis);

				__try
				{
					__try
					{
						somThis->proc(somThis->data);
					}
					__except((_exception_code()==ERROR_OPERATION_ABORTED) ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH)
					{
					}
				}

				__finally
				{
					TlsSetValue(somu_thread_key,0);
					CloseHandle(somThis->handle);
					SOMFree(somThis);
				}

				return 0;
			}
			static void __stdcall somu_stop_thread(DWORD dw)
			{
				RaiseException(ERROR_OPERATION_ABORTED,EXCEPTION_NONCONTINUABLE,0,0);
			}
		#endif
	#endif
#endif
