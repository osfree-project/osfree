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
#include <stdlib.h>
#include <stdio.h>
#ifdef _WIN32
#	include <windows.h>
#else
#	include <unistd.h>
#endif
#include <rhbmtut.h>

#if defined(_DEBUG) && defined(USE_THREADS)
#	undef RHBOPT_ASSERT_FAILED
#	define RHBOPT_ASSERT_FAILED(a,b,c) rhbmtuxp_failed(a,b,c)
static void rhbmtuxp_failed(char *file,int line,char *comment)
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
	int i=snprintf(buf,sizeof(buf),"rhbmtuxp_failed(%s,%d): %s\n",
		file,line,comment);
#		else
	int i=sprintf(buf,"rhbmtuxp_failed(%s,%d): %s\n",
		file,line,comment);
#		endif
	write(2,buf,i);
#	endif
}
#endif
#ifdef USE_THREADS

#define RHBMTUTIMPL_(x)   static x RHBMTUTCALLTYPE
#define RHBMTUTIMPL   RHBMTUTIMPL_(RHBMTUT_RESULT)

#define pool_alloc(x)        malloc(x)
#define pool_free(x)         free(x)

struct rhbThreadList
{
	struct rhbThread *first;
	struct rhbThread *last;
	int count;
};

#define init_list(x)   (x).first=NULL; (x).last=NULL; (x).count=0;

struct rhbThread
{
	struct rhbThreadList *list;
	struct rhbThread *next;
	struct rhbThread *prev;
	unsigned char alive,died,cancel;
	struct rhbThreadPool *pool;
	RHBThreadTask *task;
#ifdef USE_PTHREADS
	pthread_t tid;
	pthread_cond_t cond;
#else
	DWORD tid;
	HANDLE hThread,hEvent;
#endif
};

struct rhbThreadPool
{
	RHBThreadPool comobj;
	rhbatomic_t usage;

	struct rhbThreadList running,		/* running */
						 waiting,		/* tasked, but no thread */
						 idle,			/* thread but no task */
						 limbo,			/* told to die but hasn't yet */
						 obituary;		/* waiting for cleanup */

#ifdef USE_PTHREADS
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	pthread_t runner;
#else
	CRITICAL_SECTION crit;
	HANDLE hEvent,hRunner;
	DWORD tid;
#endif
	struct rhbThreadPoolClosing *closing;
	unsigned char is_running;
};

struct rhbThreadPoolClosing
{
#ifdef USE_PTHREADS
	pthread_cond_t cond;
#else
	HANDLE hEvent;
#endif
};

#ifdef USE_PTHREADS
#	define POOL_LOCK(x)			pthread_mutex_lock(&x->mutex)
#	define POOL_UNLOCK(x)		pthread_mutex_unlock(&x->mutex)
#else
#	define POOL_LOCK(x)			EnterCriticalSection(&x->crit)
#	define POOL_UNLOCK(x)		LeaveCriticalSection(&x->crit)
#endif

static void list_add(struct rhbThreadList *list,struct rhbThread *thr)
{
	RHBOPT_ASSERT(!(thr->list))

	thr->list=list;
	thr->next=NULL;

	if (list->last)
	{
		thr->prev=list->last;
		thr->prev->next=thr;
		list->last=thr;
	}
	else
	{
		thr->prev=NULL;
		list->first=thr;
		list->last=thr;
	}

	list->count++;
}

static struct rhbThread *list_remove(struct rhbThread *thr)
{
	struct rhbThreadList *list=thr->list;

	RHBOPT_ASSERT(thr->list)

	if (thr->next)
	{
		thr->next->prev=thr->prev;
	}

	if (thr->prev)
	{
		thr->prev->next=thr->next;
	}

	if (thr==list->first)
	{
		list->first=thr->next;
	}

	if (thr==list->last)
	{
		list->last=thr->prev;
	}

	list->count--;

	return thr;
}

RHBMTUTIMPL tp_QI(RHBThreadPool *self,void *riid,void **ppvObject)
{
/*	struct rhbThreadPool *pself=(void *)self;

	RHBOPT_ASSERT(self==&pself->comobj)
*/
	return -1;
}

RHBMTUTIMPL_(long) tp_AddRef(RHBThreadPool *self)
{
	struct rhbThreadPool *pself=(void *)self;

	RHBOPT_ASSERT(self==&pself->comobj)

	return rhbatomic_inc(&pself->usage);
}

RHBMTUTIMPL_(long) tp_Release(RHBThreadPool *self)
{
	struct rhbThreadPool *pself=(void *)self;

	RHBOPT_ASSERT(self==&pself->comobj)

	if (!rhbatomic_dec(&pself->usage))
	{
#ifdef USE_PTHREADS
		pthread_mutex_destroy(&pself->mutex);
#else
		DeleteCriticalSection(&pself->crit);
#endif

		pool_free(pself);
	}

	return 0;
}

RHBMTUTIMPL tp_Create(RHBThreadPool *self,RHBThreadTask *task)
{
	struct rhbThreadPool *pself=(void *)self;
	struct rhbThread *thr=NULL;

	RHBOPT_ASSERT(self==&pself->comobj)

	task->lpVtbl->AddRef(task);

	POOL_LOCK(pself);

	if (pself->idle.count)
	{
		thr=list_remove(pself->idle.first);
		thr->task=task;
		list_add(&pself->running,thr);
#ifdef USE_PTHREADS
		pthread_cond_signal(&thr->cond);
#else
		SetEvent(thr->hEvent);
#endif
	}
	else
	{
		thr=pool_alloc(sizeof(*thr));
		thr->alive=0;
		thr->died=0;
		thr->cancel=0;
		thr->task=task;
		thr->pool=pself;
#ifdef USE_PTHREADS
		pthread_cond_init(&thr->cond,RHBOPT_pthread_condattr_default);
#else
		thr->hEvent=CreateEvent(NULL,FALSE,FALSE,NULL);
#endif
		list_add(&pself->waiting,thr);
	}

	POOL_UNLOCK(pself);

	return -1;
}

static void reaper(struct rhbThreadPool *pool)
{
	while (pool->obituary.count)
	{
		struct rhbThread *thr=list_remove(pool->obituary.first);
	
		POOL_UNLOCK(pool);

#ifdef USE_PTHREADS
		{	/* make sure thread is truely gone and all 
				cleanups have been called */

			void *pv=NULL;

			pthread_join(thr->tid,&pv);
		}
#else
		WaitForSingleObject(thr->hThread,INFINITE);
		CloseHandle(thr->hThread);
#endif

		pool_free(thr);

		POOL_LOCK(pool);
	}
}

RHBMTUTIMPL tp_Close(RHBThreadPool *self)
{
	struct rhbThreadPool *pself=(void *)self;

	if (pself->is_running)
	{
		struct rhbThreadPoolClosing closing;

#ifdef USE_PTHREADS
		pthread_cond_init(&closing.cond,RHBOPT_pthread_condattr_default);
#else
		closing.hEvent=CreateEvent(NULL,FALSE,FALSE,NULL);
#endif

		RHBOPT_ASSERT(self==&pself->comobj)

		POOL_LOCK(pself);

		pself->closing=&closing;

		while (pself->is_running)
		{
#ifdef USE_PTHREADS
			pthread_cond_signal(&pself->cond);
			pthread_cond_wait(&closing.cond,&pself->mutex);
#else
			SetEvent(pself->hEvent);
			POOL_UNLOCK(pself);
#	ifdef HAVE_WAITFORSINGLEOBJECTEX
			WaitForSingleObjectEx(closing.hEvent,INFINITE,TRUE);
#	else
			WaitForSingleObject(closing.hEvent,INFINITE);
#	endif
			POOL_LOCK(pself);
#endif
		}

		pself->closing=NULL;

		POOL_UNLOCK(pself);

#ifdef USE_PTHREADS
		pthread_cond_destroy(&closing.cond);
#else
		CloseHandle(closing.hEvent);
#endif

#ifdef USE_PTHREADS
		{
			void *pv=NULL;
			pthread_join(pself->runner,&pv);
		}
#else
		WaitForSingleObject(pself->hRunner,INFINITE);
		CloseHandle(pself->hRunner);
#endif
	}

	return 0;
}

static void thread_delete(struct rhbThread *thr)
{
	if (thr->task) 
	{
		thr->task->lpVtbl->Release(thr->task);
	}

#ifdef USE_PTHREADS
	pthread_cond_destroy(&thr->cond);
#else
	CloseHandle(thr->hEvent);
#endif

	pool_free(thr);
}

struct thread_data
{
	int locked;
	struct rhbThread *thr;
	RHBThreadTask *task;
};

RHBOPT_cleanup_begin(thread_cleanup,pv)

struct thread_data *data=pv;
struct rhbThread *thr=data->thr;
struct rhbThreadPool *pool=thr->pool;
RHBThreadTask *task=data->task;

	if (!data->locked)
	{
		POOL_LOCK(pool);

		data->locked=1;
	}

	if (!task)
	{
		task=thr->task;

		thr->task=NULL;
	}

	if (thr->list)
	{
		list_remove(thr);
	}

	list_add(&pool->obituary,thr);

#ifdef USE_PTHREADS
	pthread_cond_signal(&pool->cond);
#else
	SetEvent(pool->hEvent);
#endif

	POOL_UNLOCK(pool);

	if (task) 
	{
		task->lpVtbl->Release(task);
	}

RHBOPT_cleanup_end

static 
#ifdef USE_PTHREADS
	void *
#else
	DWORD CALLBACK
#endif
	thread_running(void *pv)
{
	struct thread_data data={0,NULL,NULL};
	struct rhbThread *thr=pv;
	struct rhbThreadPool *pool=thr->pool;

	data.thr=pv;

	RHBOPT_cleanup_push(thread_cleanup,&data);

	POOL_LOCK(pool);

	data.locked=1;

	while (!pool->closing)
	{
		data.task=thr->task;
		thr->task=NULL;

		if (data.task)
		{
			RHBThreadTask *task=data.task;
			data.locked=0;

			POOL_UNLOCK(pool);

			data.task->lpVtbl->Run(data.task);

			data.task=NULL;

			task->lpVtbl->Release(task);

			POOL_LOCK(pool);

			data.locked=1;
		}
	}

	RHBOPT_cleanup_pop();

#ifdef USE_PTHREADS
	return NULL;
#else
	return 0;
#endif
}

static 
#ifdef USE_PTHREADS
	void *
#else
	DWORD CALLBACK
#endif
	pool_runner(void *pv)
{
	struct rhbThreadPool *pool=pv;

	POOL_LOCK(pool);

	while (1)
	{
		if (pool->obituary.count)
		{
			reaper(pool);
		}
		else
		{
			if (pool->waiting.count)
			{
				struct rhbThread *thr=list_remove(pool->waiting.first);

				if (pool->closing)
				{
					/* delete queued thread */

					POOL_UNLOCK(pool);

					thread_delete(thr);

					POOL_LOCK(pool);
				}
				else
				{
					/* create thread */
#ifdef USE_PTHREADS
					int err=pthread_create(&thr->tid,
											RHBOPT_pthread_attr_default,
											thread_running,
											thr);
#else
					int err=0;
					DWORD tid=0;
					thr->hThread=CreateThread(NULL,0,thread_running,thr,0,&tid);
					if (!thr->hThread)
					{
						err=GetLastError();
					}
#endif

					if (err)
					{
					}
					else
					{
						list_add(&pool->running,thr);
					}
				}
			}
			else
			{
				if (pool->closing && (!pool->running.count) && (!pool->limbo.count))
				{
					if (pool->idle.count)
					{
						while (pool->idle.count)
						{
							struct rhbThread *thr=list_remove(pool->idle.first);

							list_add(&pool->limbo,thr);

#ifdef USE_PTHREADS
							pthread_cond_signal(&thr->cond);
#else
							SetEvent(thr->hEvent);
#endif
						}
					}
					else
					{
						break;
					}
				}
				else
				{
#ifdef USE_PTHREADS
					pthread_cond_wait(&pool->cond,&pool->mutex);
#else
					POOL_UNLOCK(pool);
#	ifdef HAVE_WAITFORSINGLEOBJECTEX
					WaitForSingleObjectEx(pool->hEvent,INFINITE,TRUE);
#	else
					WaitForSingleObject(pool->hEvent,INFINITE);
#	endif
					POOL_LOCK(pool);
#endif
				}
			}
		}
	}

	if (pool->closing)
	{
#ifdef USE_PTHREADS
		pthread_cond_signal(&(pool->closing->cond));
#else
		SetEvent(pool->closing->hEvent);
#endif
	}

	pool->is_running=0;

	POOL_UNLOCK(pool);

#ifdef USE_PTHREADS
	return NULL;
#else
	return 0;
#endif
}

static struct RHBThreadPoolVtbl tpVtbl={
	tp_QI,tp_AddRef,tp_Release,
	tp_Create,tp_Close
};

RHBMTUTAPI_(RHBMTUT_RESULT) RHBThreadPoolNew(RHBThreadPool **pPool)
{
	struct rhbThreadPool *pool=pool_alloc(sizeof(*pool));

	pool->comobj.lpVtbl=&tpVtbl;
	pool->usage=1;
	pool->closing=NULL;

#ifdef USE_PTHREADS
	pthread_mutex_init(&pool->mutex,RHBOPT_pthread_mutexattr_default);
	pthread_cond_init(&pool->cond,RHBOPT_pthread_condattr_default);
#else
	InitializeCriticalSection(&pool->crit);
	pool->hEvent=CreateEvent(NULL,FALSE,FALSE,NULL);
#endif

	init_list(pool->running)
	init_list(pool->waiting)
	init_list(pool->idle)
	init_list(pool->limbo)
	init_list(pool->obituary)

	pool->is_running=1;

#ifdef USE_PTHREADS
	pthread_create(&pool->runner,
					RHBOPT_pthread_attr_default,
					pool_runner,
					pool);
#else
	pool->hRunner=CreateThread(NULL,0,pool_runner,pool,0,&pool->tid);
#endif

	*pPool=&pool->comobj;

	return 0;
}
#else
#	if defined(_WIN32)
/************************** 
 *
 * blank stubs to support def file 
 *
 */
void RHBThreadPoolNew(void) {}
#	endif
#endif /* USE_THREADS */
