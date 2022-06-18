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

#ifndef SOM_Module_eman_Source
#define SOM_Module_eman_Source
#endif
#define SOMEEMan_Class_Source

#include <rhbemani.h>
#include <snglicls.h>
#include <emtypes.h>
#include <event.h>
#include <eventmsk.h>
#include <eman.ih>
#include <rhbeman.h>
#include <somderr.h>
#include <stexcep.h>

#ifdef _PLATFORM_UNIX_
	#include <sys/types.h>
	#include <sys/ioctl.h>
	#include <sys/time.h>
	#include <unistd.h>
	#include <errno.h>
	#include <fcntl.h>
#endif

#ifdef _PLATFORM_MACINTOSH_
	#include <Events.h>
	#include <rhbmdisp.h>
	#include <rhbmtask.h>
	#include <AppleEvents.h>
#endif

#ifdef HAVE_STRINGS_H
#	include <strings.h>
#endif

#ifndef USE_THREADS
#undef SOMEEMan_someGetEManSem
#undef SOMEEMan_someReleaseEManSem
#endif

#ifndef SOMEEMan_someWait
/* macintosh specific normally */
#define SOMEEMan_someWait   eman_someWait
SOM_Scope void SOMLINK eman_someWait(SOMEEMan SOMSTAR somSelf,Environment *ev,unsigned long interval);
#endif

struct eman_blocking
{
	boolean locked;
	SOMEEManPrivateData *somThis;
	SOMEEMan SOMSTAR somSelf;
	struct eman_blocking *next;
#ifdef USE_THREADS
	#ifdef USE_PTHREADS
		pthread_mutex_t *must_unlock;
		pthread_t tid;
	#else
		DWORD tid;
	#endif
#else
	#ifdef _PLATFORM_MACINTOSH_
		ProcessSerialNumber psn;
	#else
		#ifdef _WIN32
			DWORD tid;
		#else
			int pid;
		#endif
	#endif
#endif
};

#ifndef _PLATFORM_MACINTOSH_
static void rhbeman_Enqueue(QElemPtr el,QHdrPtr hdr)
{
	el->qLink=NULL;

	if (hdr->qTail)
	{
		hdr->qTail->qLink=el;
		hdr->qTail=el;
	}
	else
	{
		hdr->qTail=hdr->qHead=el;
	}
}

static int rhbeman_Dequeue(QElemPtr el,QHdrPtr hdr)
{
	if (hdr->qHead)
	{
		if (hdr->qHead==el)
		{
			hdr->qHead=el->qLink;

			if (!(hdr->qHead))
			{
				hdr->qTail=NULL;
			}

			return 0;
		}
		else
		{
			QElemPtr p=hdr->qHead;
			while (p->qLink != el)
			{
				p=p->qLink;
				if (!p) break;
			}
			if (p)
			{
				p->qLink=el->qLink;
				if (!(p->qLink))
				{
					hdr->qTail=p;
				}

				return 0;
			}
		}
	}

	return -1;
}
#endif

static RHBEMan_Entry * RHBEMan_allocate(SOMEEMan SOMSTAR somSelf,SOMEEManPrivateData *somThis)
{
	RHBEMan_Entry *e;
	long token;

	#ifdef USE_THREADS
		#ifdef USE_PTHREADS
			RHBOPT_ASSERT(somThis->eman_current_nesting)
			RHBOPT_ASSERT(pthread_equal(somThis->eman_current,pthread_self()))
		#endif
	#endif

	token=++somThis->last_token;

	e=somThis->list;

	while (e)
	{
		if (e->token==token)
		{
			token++;
			e=somThis->list;
		}
		else
		{
			if (token==0)
			{
				token=1;
				e=somThis->list;
			}
			else
			{
				e=e->next;
			}
		}
	}

	somThis->last_token=token;

	e=SOMMalloc(sizeof(*e));

	if (e)
	{
		memset(e,0,sizeof(*e));

		e->token=token;
		e->next=somThis->list;
		somThis->list=e;

		return e;
	}

	return 0;
}

static RHBEMan_Entry * RHBEMan_find(SOMEEManPrivateData *somThis,long token)
{
	if (token > 0)
	{
		RHBEMan_Entry *e=somThis->list;

		while (e)
		{
			if (token==e->token)
			{
				return e;
			}

			e=e->next;
		}
	}

	return 0;
}

static long time_in_ms(void)
{
#ifdef _PLATFORM_WIN32_
	return GetTickCount();
#else
	#ifdef _PLATFORM_UNIX_
		struct timeval tv;
		long l;
		tv.tv_usec=0;
		tv.tv_sec=0;
		gettimeofday(&tv,NULL);
		l=tv.tv_sec*1000;
		l+=(tv.tv_usec/1000);
		return l;
	#else
		#ifdef _PLATFORM_MACINTOSH_
			return TickCount() * (1000/60);
		#else
			#error time_in_ms not for this platform
		#endif
	#endif
#endif
}

static void RHBEMan_add(SOMEEManQueue *q,RHBEMan_Entry *e)
{
	e->queue=q;
	e->element.qLink=NULL;

	if (q->queue_type==EMTimerEvent)
	{
		RHBEMan_Entry *f=(void *)q->queue.qHead;

		if (f)
		{
			if (f->specifics.timer.t_when > e->specifics.timer.t_when)
			{
				/* put this before the first entry */

				e->element.qLink=&f->element;
				q->queue.qHead=&e->element;
			}
			else
			{
				while (f->element.qLink)
				{
					/* check if should go before next entry */

					if (((RHBEMan_Entry *)f->element.qLink)->specifics.timer.t_when <= e->specifics.timer.t_when)
					{
						f=(void *)f->element.qLink;
					}
					else
					{
						break;
					}
				}

				e->element.qLink=f->element.qLink;
				f->element.qLink=&e->element;

				if (!e->element.qLink)
				{
					q->queue.qTail=&e->element;
				}
			}

			return;
		}
	}

	rhbeman_Enqueue(&e->element,&q->queue);
}

static void RHBEMan_remove(SOMEEManQueue *q,RHBEMan_Entry *e)
{
	if (q == e->queue)
	{
		rhbeman_Dequeue(&e->element,&q->queue);
	}

	e->queue=NULL;
}

static void RHBEMan_delete(SOMEEMan SOMSTAR eman,SOMEEManPrivateData *somThis,RHBEMan_Entry *e)
{
#ifdef USE_THREADS
	#ifdef USE_PTHREADS
		RHBOPT_ASSERT(somThis->eman_current_nesting)
		RHBOPT_ASSERT(pthread_equal(somThis->eman_current,pthread_self()))
	#endif
#endif

	if (e==somThis->list)
	{
		somThis->list=e->next;
	}
	else
	{
		struct RHBEMan_Entry *p=somThis->list;

		while (p)
		{
			if (p->next==e)
			{
				p->next=e->next;
				break;
			}
			else
			{
				p=p->next;
			}
		}
	}

	if (e->queue)
	{
		long queue_type=e->queue->queue_type;

		RHBEMan_remove(e->queue,e);

		switch (queue_type)
		{
		case EMTimerEvent:
			break;
		case EMSignalEvent:
			break;
		case EMWorkProcEvent:
			break;
		case EMSinkEvent:
			break;
		case EMClientEvent:
			{
				char *p=e->specifics.client_type.ct_type;
				e->specifics.client_type.ct_type=NULL;

				if (p) SOMFree(p);
			}
			break;
		case EMMsgQEvent:
			break;
		default:
			RHBOPT_ASSERT(!"unknown type\n");
		}
	}

	SOMFree(e);
}

static void RHBEMan_fill(
		SOMEEManPrivateData *priv,
		Environment *ev,RHBEMan_Entry *e,SOMEEMRegisterData SOMSTAR data)
{
	SOMEEManQueue *q=NULL;

	if (e->queue) RHBEMan_remove(e->queue,e);

	switch (SOMEEMRegisterData_someGetRegDataEventMask(data,ev))
	{
	case EMSinkEvent:
		e->specifics.sink.s_fd=SOMEEMRegisterData_someGetRegDataSink(data,ev);
		e->specifics.sink.s_mask=SOMEEMRegisterData_someGetRegDataSinkMask(data,ev);

		q=&priv->sink_queue;
		break;
	case EMTimerEvent:
		{
			long now=time_in_ms();

			e->specifics.timer.t_interval=SOMEEMRegisterData_someGetRegDataTimerInterval(data,ev);
			e->specifics.timer.t_count=SOMEEMRegisterData_someGetRegDataTimerCount(data,ev);
			e->specifics.timer.t_when=now+e->specifics.timer.t_interval;

			q=&priv->timer_queue;
		}
		break;
	case EMClientEvent:
		{
			corbastring p=SOMEEMRegisterData_someGetRegDataClientType(data,ev);

			q=&priv->client_types_queue;
			e->specifics.client_type.ct_type=SOMEEMRegisterData_dupl_string(ev,p);
		}
		break;
	case EMWorkProcEvent:
		{
			q=&priv->workproc_queue;
		}
		break;
	default:
		RHBOPT_ASSERT(!"what do we do with this emtype\n");
		break;
	}

	if (q) 
	{
		RHBEMan_add(q,e);
	}
	else
	{
		RHBOPT_ASSERT(!"RHBEMan_fill");
	}
}

struct RHBEMan_call_user
{
	boolean doFree;
	SOMEEvent SOMSTAR event;
};

RHBOPT_cleanup_begin(RHBEMan_call_user_cleanup,pv)

struct RHBEMan_call_user *data=pv;

	if (data->doFree && data->event)
	{
		somReleaseObjectReference(data->event);
	}

RHBOPT_cleanup_end

static void RHBEMan_call_user(RHBEMan_Entry *entry,SOMEEvent SOMSTAR event,boolean doFree)
{
struct RHBEMan_call_user data={0,NULL};

	data.doFree=doFree;
	data.event=event;

	RHBOPT_cleanup_push(RHBEMan_call_user_cleanup,&data);

	switch (entry->eman_callback_type)
	{
	case eman_cb_proc:
		entry->eman_callback.eman_proc.pfn(
			event,entry->eman_callback_token);
		break;
	case eman_cb_event:
		entry->eman_callback.eman_event.pfn(
			entry->eman_callback.eman_event.target,
			event,
			entry->eman_callback_token);
		break;
	case eman_cb_event_ev:
		entry->eman_callback.eman_event_ev.pfn(
			entry->eman_callback.eman_event_ev.target,
			entry->eman_callback.eman_event_ev.ev,
			event,
			entry->eman_callback_token);
		break;
	default:
		RHBOPT_ASSERT(!"eman callback error\n");
	}

	RHBOPT_cleanup_pop();
}

static void eman_blocking_unhook(struct eman_blocking *data)
{
	if (data->somThis->eman_blocking==data)
	{
		data->somThis->eman_blocking=data->next;
	}
	else
	{
		struct eman_blocking *p=data->somThis->eman_blocking;

		while (p)
		{
			if (p->next==data)
			{
				p->next=data->next;
				break;
			}
			else
			{
				p=p->next;
			}
		}
	}
}

RHBOPT_cleanup_begin(RHBEMan_doOne_cleanup,pv)

	struct eman_blocking *data=pv;
	Environment ev;
	SOM_InitEnvironment(&ev);

#ifdef USE_THREADS
	#ifdef USE_PTHREADS
		if (data->must_unlock)
		{
			pthread_mutex_unlock(data->must_unlock);
		}
	#else
	#endif
#endif

	if (!data->locked)
	{
#ifdef USE_THREADS
		SOMEEMan_someGetEManSem(data->somSelf,&ev);
#endif
		data->locked=1;
	}

	eman_blocking_unhook(data);

	if (data->locked)
	{
#ifdef USE_THREADS
		SOMEEMan_someReleaseEManSem(data->somSelf,&ev);
#endif
	}
	SOM_UninitEnvironment(&ev);

RHBOPT_cleanup_end

static boolean RHBEMan_doOne(SOMEEMan SOMSTAR somSelf,
						  SOMEEManPrivateData *somThis,
						  Environment *ev,
						  long msDelay_arg,
						  unsigned long mask)
{
	RHBOPT_volatile long msDelay=msDelay_arg;
	struct eman_blocking blocking={0,NULL,NULL,NULL};
	RHBOPT_volatile boolean result=0;

	if (ev->_major) return result;

#ifdef USE_THREADS
	#ifdef USE_PTHREADS
		blocking.must_unlock=NULL;
		blocking.tid=pthread_self();
		somThis->mainThread=blocking.tid;
	#else
		blocking.tid=GetCurrentThreadId();
	#endif
#else
	#ifdef _PLATFORM_MACINTOSH_
		GetCurrentProcess(&blocking.psn);
	#else
		#ifdef _WIN32
			blocking.tid=GetCurrentThreadId();
		#else
			#ifdef _WIN16
				blocking.tid=GetCurrentTask();
			#else
				blocking.pid=getpid();
			#endif
		#endif
	#endif
#endif

	RHBOPT_cleanup_push(RHBEMan_doOne_cleanup,&blocking);

#ifdef USE_THREADS
	SOMEEMan_someGetEManSem(somSelf,ev);
#endif

	blocking.locked=1;
	blocking.next=somThis->eman_blocking;
	somThis->eman_blocking=&blocking;
	blocking.somThis=somThis;
	blocking.somSelf=somSelf;

	while (somThis->cl_events.qHead && blocking.locked)
	{
		SOMEClientEventQL *q=(void *)somThis->cl_events.qHead;

		if (q)
		{
			if (0==rhbeman_Dequeue(&q->qElem,&somThis->cl_events))
			{
				SOMEEvent SOMSTAR z=q->event;
				char *p=SOMEClientEvent_somevGetEventClientType(z,ev);
				RHBEMan_Entry *t=(void *)somThis->client_types_queue.queue.qHead;
				q->header=NULL;

				while (t)
				{
					if (0==strcmp(p,t->specifics.client_type.ct_type))
					{
						eman_blocking_unhook(&blocking);

						blocking.locked=0;
	#ifdef USE_THREADS
						SOMEEMan_someReleaseEManSem(somSelf,ev);
	#endif
						RHBEMan_call_user(t,z,0);

						t=NULL;

						result=1;
						
						break;
					}
					else
					{
						t=(void *)t->element.qLink;
					}
				}
			}
		}
	}

	if (blocking.locked) /* means did not exec any client events */
	{
		int j=0;
		long now_time=time_in_ms();
		long earliest_timer=now_time+msDelay;
		RHBEMan_Entry *e=NULL;
		long number_timers=0;
	#ifdef RHBEMAN_SINKS
		fd_set fd_read;
		fd_set fd_write;
		fd_set fd_except;
		SOCKET max_fd=INVALID_SOCKET;
		long number_reads=0,number_writes=0,number_excepts=0;

		FD_ZERO(&fd_read);
		FD_ZERO(&fd_write);
		FD_ZERO(&fd_except);

		e=(void *)somThis->sink_queue.queue.qHead;

		while (e)
		{
			if (e->specifics.sink.s_mask & (EMInputReadMask|EMInputWriteMask|EMInputExceptMask))
			{
				if (e->specifics.sink.s_mask & EMInputReadMask)
				{
					FD_SET(e->specifics.sink.s_fd,&fd_read);
					number_reads++;

/*somPrintf("[R-%d]",e->specifics.sink.s_fd);*/
				}

				if (e->specifics.sink.s_mask & EMInputWriteMask)
				{
					FD_SET(e->specifics.sink.s_fd,&fd_write);
					number_writes++;
/*somPrintf("[W-%d]",e->specifics.sink.s_fd);*/
				}

				if (e->specifics.sink.s_mask & EMInputExceptMask)
				{
					FD_SET(e->specifics.sink.s_fd,&fd_except);
					number_excepts++;
/*somPrintf("[E-%d]",e->specifics.sink.s_fd);*/
				}

				if (max_fd==INVALID_SOCKET)
				{
					max_fd=e->specifics.sink.s_fd;
				}
				else
				{
					if (max_fd < e->specifics.sink.s_fd)
					{
						max_fd=e->specifics.sink.s_fd;
					}
				}
			}

			e=(void *)e->element.qLink;
		}
	#endif /* RHBEMAN_SINKS */

		e=(void *)somThis->timer_queue.queue.qHead;

		if (e)
		{
			number_timers++;

			if (e->specifics.timer.t_when < earliest_timer)
			{
				earliest_timer=e->specifics.timer.t_when;
			}
		}

		if (somThis->workproc_queue.queue.qHead)
		{
			msDelay=0;
		}
		else
		{
			if (number_timers)
			{
				long d=earliest_timer - now_time;

				if (d > 0)
				{
					msDelay=d;
				}
				else
				{
					msDelay=0;
				}
			}
			else
			{
				RHBSOM_Trace("no timers")
			}
		}

	/*	printf("tv_usec=%ld,tv_sec=%ld\n",tv->tv_usec,tv->tv_sec);*/

	#ifdef USE_EMAN_SOCKETPAIR
		if (somThis->eman_sock_read != INVALID_SOCKET)
		{
/*			somPrintf("eman_sock_read %d\n",somThis->eman_sock_read);*/

			FD_SET(somThis->eman_sock_read,&fd_read);
			number_reads++;

			if (somThis->eman_sock_read > max_fd) max_fd=somThis->eman_sock_read;
		}
	#endif /* USE_EMAN_SOCKETPAIR */

		if (msDelay 
	#ifdef RHBEMAN_SINKS
			| number_reads 
			| number_writes 
			| number_excepts
	#endif /* RHBEMAN_SINKS */
			)
		{
			blocking.locked=0;

	#ifdef USE_THREADS
			SOMEEMan_someReleaseEManSem(somSelf,ev);
	#endif /* !USE_THREADS */

	#ifdef _PLATFORM_MACINTOSH_
#error
			SOMEEMan_someWait(somSelf,ev,msDelay);
	#else /* _PLATFORM_MACINTOSH_ */
			{
				struct timeval tv={0,0};

				tv.tv_sec=(msDelay>>10);

				if (tv.tv_sec)
				{
					tv.tv_usec=0;
				}
				else
				{
					tv.tv_usec=msDelay << 10;
					if (tv.tv_usec > 999999)
					{
						tv.tv_usec=0;
						tv.tv_sec=1;
					}
				}

/*				somPrintf("max_fd=%d\n",max_fd);*/

		#ifdef RHBEMAN_SINKS
				if (number_reads|number_writes|number_excepts)
				{
					j=select((int)(max_fd+1),
			#ifdef __HPUX__
						number_reads ? ((void *)&fd_read) : NULL ,
						number_writes ? ((void *)&fd_write) : NULL,
						number_excepts ? ((void *)&fd_except) : NULL,
			#else /* __HPUX__ */
						number_reads ? &fd_read : NULL ,
						number_writes ? &fd_write : NULL,
						number_excepts ? &fd_except : NULL,
			#endif /* !__HPUX__ */
						&tv);
				}
				else
				{
					SOMEEMan_someWait(somSelf,ev,msDelay);
				}

			#ifdef _WIN32
				if (j<0)
				{
					int e=WSAGetLastError();
					RHBOPT_ASSERT(0)
					somPrintf("select error %d,%d\n",j,e);
					RHBOPT_throw_StExcep(ev,TRANSIENT,SocketSelect,NO);
				}
			#else /* _WIN32 */
				if (j < 0)
				{
					int e=errno;

					if (e != EINTR)
					{
						somPrintf("select error %d,%d at %s,%d\n",j,e,__FILE__,__LINE__);
						RHBOPT_throw_StExcep(ev,TRANSIENT,SocketSelect,NO);
					}
				}
			#endif /* !_WIN32 */
/*				somPrintf("select returned %d\n",j);*/

				number_reads=0;
				number_writes=0;
				number_excepts=0;
		#else /* RHBEMAN_SINKS */
				SOMEEMan_someWait(somSelf,ev,msDelay);
				j=0;
#error
		#endif /* !RHBEMAN_SINKS */
			}
	#endif /* _PLATFORM_MACINTOSH_ */

		#ifdef USE_THREADS
			SOMEEMan_someGetEManSem(somSelf,ev);
		#endif

			blocking.locked=1;
		}
	#ifdef RHBEMAN_SINKS
		else
		{
			j=0;
		}
	#endif /* RHBEMAN_SINKS */

		if (blocking.locked)
		{
			eman_blocking_unhook(&blocking);
		}

		number_timers=0;

	#ifdef RHBEMAN_SINKS
		if (j > 0)
		{
		#ifdef USE_EMAN_SOCKETPAIR
			if (somThis->eman_sock_read != INVALID_SOCKET)
			{
				if (FD_ISSET(somThis->eman_sock_read,&fd_read))
				{
					char buf[1];
					int i;

					FD_CLR(somThis->eman_sock_read,&fd_read);

					blocking.locked=0;

			#ifdef USE_THREADS
					SOMEEMan_someReleaseEManSem(somSelf,ev);
			#endif /* !USE_THREADS */

/*					somPrintf("read blip socket\n");*/

			#ifdef _WIN32
					i=recv(somThis->eman_sock_read,buf,1,0);
			#else /* _WIN32 */
					i=read(somThis->eman_sock_read,buf,1);
			#endif /* !_WIN32 */

					(void)i;

/*					somPrintf("read blip socket returned %d,%02X\n",i,(octet)buf[0]);*/

			#ifdef USE_THREADS
				#ifdef USE_PTHREADS
					pthread_mutex_lock(&somThis->eman_mtx);

					blocking.must_unlock=&somThis->eman_mtx;

					while (somThis->eman_kick_on_release)
					{
						pthread_cond_wait(&somThis->eman_event,&somThis->eman_mtx);
					} 

					blocking.must_unlock=NULL;

					pthread_mutex_unlock(&somThis->eman_mtx);
				#else /* USE_PTHREADS */
					WaitForSingleObjectEx(somThis->eman_event,INFINITE,TRUE);
				#endif /* !USE_PTHREADS */
			#endif /* !USE_THREADS */

			#ifdef USE_THREADS
					SOMEEMan_someGetEManSem(somSelf,ev);
			#endif /* !USE_THREADS */
					blocking.locked=1;

					j=0;
				}
			}
		#endif /* USE_EMAN_SOCKETPAIR */

			e=(void *)somThis->sink_queue.queue.qHead;

/*			somPrintf("j=%d, e=%p\n",j,e);*/

			while ((j>0)&&e)
			{
				long m=0;

				if (e->specifics.sink.s_mask & EMInputReadMask)
				{
					if (FD_ISSET(e->specifics.sink.s_fd,&fd_read))
					{
						m|=EMInputReadMask;

						FD_CLR(e->specifics.sink.s_fd,&fd_read);

						if (j) j--;

						number_reads++;
					}
				}

				if (e->specifics.sink.s_mask & EMInputWriteMask)
				{
					if (FD_ISSET(e->specifics.sink.s_fd,&fd_write))
					{
						m|=EMInputWriteMask;

						FD_CLR(e->specifics.sink.s_fd,&fd_write);

						if (j) j--;

						number_writes++;
					}
				}

				if (e->specifics.sink.s_mask & EMInputExceptMask)
				{
					if (FD_ISSET(e->specifics.sink.s_fd,&fd_except))
					{
						m|=EMInputExceptMask;

						FD_CLR(e->specifics.sink.s_fd,&fd_except);

						if (j) j--;

						number_excepts++;
					}
				}

				if (m)
				{
					SOMESinkEvent SOMSTAR sink=

		#ifdef somNewObject
					somNewObject(SOMESinkEvent);
		#else
					SOMESinkEventNew();
		#endif
					if (sink)
					{
						SOMESinkEvent_somevSetEventSink(sink,ev,(long)e->specifics.sink.s_fd);
						SOMESinkEvent_somevSetEventType(sink,ev,EMSinkEvent);
					}

					blocking.locked=0;

		#ifdef USE_THREADS
					SOMEEMan_someReleaseEManSem(somSelf,ev);
		#endif

/*					somPrintf("sink event, fd=%d, bitmask %d\n",
						e->specifics.sink.s_fd,m);*/

					RHBEMan_call_user(e,sink,1);

		#ifdef USE_THREADS
					SOMEEMan_someGetEManSem(somSelf,ev);
		#endif
					blocking.locked=1;

						/* this is used to loop round list
							again incase any were moved */

					e=(void *)somThis->sink_queue.queue.qHead;
					
					result=1;
				}
				else
				{
					e=(void *)e->element.qLink;
				}
			}
		}
		else
	#endif /* RHBEMAN_SINKS */
		{
			now_time=time_in_ms();

			e=(void *)somThis->timer_queue.queue.qHead;

			while (e)
			{
				if (e->specifics.timer.t_when <= now_time)
				{
					SOMETimerEvent SOMSTAR tim=

	#ifdef somNewObject
					somNewObject(SOMETimerEvent);
	#else
					SOMETimerEventNew();
	#endif

					SOMETimerEvent_somevSetEventInterval(tim,ev,e->specifics.timer.t_interval);
					SOMETimerEvent_somevSetEventType(tim,ev,EMTimerEvent);
					SOMETimerEvent_somevSetEventTime(tim,ev,now_time);
							
					e->specifics.timer.t_when+=e->specifics.timer.t_interval;

					RHBEMan_remove(e->queue,e);

					if (e->specifics.timer.t_count)
					{
						e->specifics.timer.t_count--;

						if (e->specifics.timer.t_count==0)
						{
	/*						somPrintf("Stopped timer %d\n",
									(int)e->token);*/

							RHBEMan_add(&somThis->dead_timer_queue,e);
						}
					}

					if (!e->queue)
					{
						RHBEMan_add(&somThis->timer_queue,e);
					}

					number_timers++;

					blocking.locked=0;
	#ifdef USE_THREADS
					SOMEEMan_someReleaseEManSem(somSelf,ev);
	#endif
					RHBEMan_call_user(e,tim,1);

	#ifdef USE_THREADS
					SOMEEMan_someGetEManSem(somSelf,ev);
	#endif
					blocking.locked=1;

					e=(void *)somThis->timer_queue.queue.qHead;

					result=1;
				}
				else
				{
					e=NULL;
				}
			}
		}

		if (somThis->workproc_queue.queue.qHead
			&& !(
			number_timers
	#ifdef RHBEMAN_SINKS
			|number_reads|number_writes|number_excepts
	#endif /* RHBEMAN_SINKS */
			)
		)
		{
			SOMEWorkProcEvent SOMSTAR wp=
	#ifdef somNewObject
			somNewObject(SOMEWorkProcEvent);
	#else
			SOMEWorkProcEventNew();
	#endif

			SOMEWorkProcEvent_somevSetEventType(wp,ev,EMWorkProcEvent);

			e=(void *)somThis->workproc_queue.queue.qHead;

			if (e != (void *)somThis->workproc_queue.queue.qTail)
			{
				/* this puts it as the last one */

				RHBEMan_remove(e->queue,e);

				RHBEMan_add(&somThis->workproc_queue,e);
			}

			blocking.locked=0;

	#ifdef USE_THREADS
			SOMEEMan_someReleaseEManSem(somSelf,ev);
	#endif
			RHBEMan_call_user(e,wp,1);
			
			result=1;
		}
	}

	RHBOPT_cleanup_pop();
	
	return result;
}

SOM_Scope void  SOMLINK eman_someGetEManSem(SOMEEMan SOMSTAR somSelf, 
                                             Environment *ev)
{
#ifdef USE_THREADS
	SOMEEManData *somThis=SOMEEManGetData(somSelf);
	#ifdef USE_PTHREADS
		pthread_t current=pthread_self();

		if (somThis->privdata.eman_current_nesting)
		{
			if (pthread_equal(current,somThis->privdata.eman_current))
			{
				somThis->privdata.eman_current_nesting++;
				return;
			}
		}

		if (pthread_equal(current,somThis->privdata.mainThread))
		{
/*			somPrintf("someGetEManSem in mainThread\n");*/
		}
		else
		{
			if (somThis->privdata.eman_sock_write != INVALID_SOCKET)
			{
				pthread_mutex_lock(&somThis->privdata.eman_mtx);
				somThis->privdata.eman_kick_on_release++;
				pthread_mutex_unlock(&somThis->privdata.eman_mtx);
				send(somThis->privdata.eman_sock_write,"0",1,0);
			}
		}

		pthread_mutex_lock(&somThis->privdata.eman_sem);
		somThis->privdata.eman_current=current;
		somThis->privdata.eman_current_nesting=1;
	#else
		if (WAIT_OBJECT_0!=WaitForSingleObject(
				somThis->privdata.eman_sem,INFINITE))
		{
			RHBOPT_throw_StExcep(ev,INTERNAL,UnknownError,NO);
		}
		else
		{
			InterlockedIncrement(&somThis->privdata.eman_current_nesting);
		}
	#endif
#endif
}

SOM_Scope void  SOMLINK eman_someReleaseEManSem(
		SOMEEMan SOMSTAR somSelf, 
		Environment *ev)
{
#ifdef USE_THREADS
	SOMEEManData *somThis=SOMEEManGetData(somSelf);
	#ifdef USE_PTHREADS
		RHBOPT_ASSERT(somThis->privdata.eman_current_nesting)

		RHBOPT_ASSERT(pthread_equal(pthread_self(),somThis->privdata.eman_current))

		if (somThis->privdata.eman_current_nesting > 1)
		{
			somThis->privdata.eman_current_nesting--;
			return;
		}

		somThis->privdata.eman_current_nesting=0;
	/*	eman_current=0;*/
		pthread_mutex_lock(&somThis->privdata.eman_mtx);
		if (somThis->privdata.eman_kick_on_release)
		{
			somThis->privdata.eman_kick_on_release--;
			pthread_cond_signal(&somThis->privdata.eman_event);
		}
		pthread_mutex_unlock(&somThis->privdata.eman_mtx);
		pthread_mutex_unlock(&somThis->privdata.eman_sem);
	#else
		DWORD dw=InterlockedDecrement(&somThis->privdata.eman_current_nesting);
		if (!ReleaseMutex(somThis->privdata.eman_sem))
		{
		#ifdef _M_IX86
			__asm int 3;
		#endif
			RHBOPT_throw_StExcep(ev,INTERNAL,UnknownError,NO);
		}
		if (!dw) 
		{
			if (!SetEvent(somThis->privdata.eman_event))
			{
		#ifdef _M_IX86
				__asm int 3;
		#endif
			}
		}
	#endif
#endif
}

SOM_Scope void  SOMLINK eman_someChangeRegData(
		SOMEEMan SOMSTAR somSelf, 
		Environment *ev, 
		long registrationId, 
		SOMEEMRegisterData SOMSTAR registerData)
{
	RHBEMan_Entry *e;
	SOMEEManData *somThis=SOMEEManGetData(somSelf);

#ifdef USE_THREADS
	SOMEEMan_someGetEManSem(somSelf,ev);
#endif

	e=RHBEMan_find(&somThis->privdata,registrationId);

	if (e)
	{
		RHBEMan_fill(&somThis->privdata,ev,e,registerData);
	}

#ifdef USE_THREADS
	SOMEEMan_someReleaseEManSem(somSelf,ev);
#endif
}

SOM_Scope void  SOMLINK eman_someProcessEvent(SOMEEMan SOMSTAR somSelf, 
                                               Environment *ev, 
                                              unsigned long mask)
{
	SOMEEManData *somThis=SOMEEManGetData(somSelf);
	RHBEMan_doOne(somSelf,&somThis->privdata,ev,0,mask);
}

struct eman_running
{
	void *hQuitThread;
	int quitting;
	SOMEEManData *somThis;
	struct eman_running *next;
};

RHBOPT_cleanup_begin(eman_someProcessEvents_cleanup,pv)

struct eman_running *data=pv;

#ifdef USE_THREADS
	SOMRequestMutexSem(data->somThis->privdata.running_list_mutex);
#endif

	if (data->somThis->privdata.eman_running==data)
	{
		data->somThis->privdata.eman_running=data->next;
	}
	else
	{
		struct eman_running *p=data->somThis->privdata.eman_running;

		while (p)
		{
			if (p->next==data)
			{
				p->next=data->next;
				break;
			}
			p=p->next;
		}
	}

#ifdef USE_THREADS
	SOMReleaseMutexSem(data->somThis->privdata.running_list_mutex);
#endif

#ifdef _WIN32
	if (data->hQuitThread)
	{
		WaitForSingleObject(data->hQuitThread,INFINITE);
		CloseHandle(data->hQuitThread);
	}
#endif

RHBOPT_cleanup_end

SOM_Scope void  SOMLINK eman_someProcessEvents(SOMEEMan SOMSTAR somSelf, 
                                                Environment *ev)
{
	if (!ev->_major)
	{
		struct eman_running data={NULL,0,NULL,NULL};
		data.somThis=SOMEEManGetData(somSelf);

		RHBOPT_cleanup_push(eman_someProcessEvents_cleanup,&data);

#ifdef USE_THREADS
		SOMRequestMutexSem(data.somThis->privdata.running_list_mutex);
#endif

		data.next=data.somThis->privdata.eman_running;
		data.somThis->privdata.eman_running=&data;

#ifdef USE_THREADS
		SOMReleaseMutexSem(data.somThis->privdata.running_list_mutex);
#endif

		while ((!data.quitting)&&(!ev->_major))
		{
			SOMEEMan_someProcessEventInterval(somSelf,ev,(unsigned long)-1L,(60*5000),0);
		}

		RHBOPT_cleanup_pop();
	}
}

SOM_Scope void SOMLINK eman_someProcessEventInterval(
	SOMEEMan SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned long mask,
	/* in */ unsigned long interval,
	/* in */ unsigned long inst)
{
	SOMEEManData *somThis=SOMEEManGetData(somSelf);
	RHBEMan_doOne(somSelf,&somThis->privdata,ev,interval,mask);
}

#ifdef _WIN32
#ifdef USE_EMAN_SOCKETPAIR
static DWORD CALLBACK blip_fd(void *pv)
{
		SOMEEManData *somThis=pv;
		int i=send(somThis->privdata.eman_sock_write,"1",1,0);
		switch (i)
		{
		case 1:
			return 0;
		case 0: 
			return (DWORD)~0;
		default:
			break;
		}
		return WSAGetLastError();
}
#endif
#endif

static void cev_dequeue(SOMEClientEventQL *ql)
{
	rhbeman_Dequeue(&ql->qElem,ql->header);
}

/* this needs to be able to be called from a signal handler
	in a non-threaded environment, so no memory allocs
	allowed,
	in a non-threaded environment the somStartCriticalSection
	will use sigprocmask(SIG_BLOCK) to disable an appropriate
	set of signals,
	in a multithreaded environment signal handlers should not
	call this directly but use a thread that used sigwait to
	retrieve the signal
 */

SOM_Scope void SOMLINK eman_someQueueEvent(
		SOMEEMan SOMSTAR somSelf, 
		Environment *ev, 
		SOMEClientEvent SOMSTAR event)
{
	if (event && somSelf)
	{
		SOMEEManData *somThis=SOMEEManGetData(somSelf);
		SOMEClientEventQL *ql=SOMEClientEvent_somevGetClientEventQL(event,ev);

#ifdef USE_THREADS
		SOMEEMan_someGetEManSem(somSelf,ev);
#endif

		if ((!ql->header))
		{
			ql->dequeue=cev_dequeue;
			ql->header=&somThis->privdata.cl_events;
			rhbeman_Enqueue(&ql->qElem,&somThis->privdata.cl_events);

#ifdef _PLATFORM_MACINTOSH_
			if (somThis->privdata.task)
			{
				somThis->privdata.task->lpVtbl->Notify(somThis->privdata.task);
			}
			WakeUpProcess(&somThis->privdata.psn);
#else
			/* write a blip to the signal socket */
#ifdef USE_EMAN_SOCKETPAIR
			if (somThis->privdata.eman_sock_write!=INVALID_SOCKET)
#endif
			{
#ifdef _WIN32
	#ifdef _WIN32S
				/* need to unblock the magic select statement */
				if (somThis->privdata.eman_blocking)
				{
					if (WSAIsBlocking())
					{
						WSACancelBlockingCall();
					}
				}
	#else
				/* need to do this is in a separate thread
				*/
				if (somThis->privdata.eman_blocking)
				{
#ifdef USE_PTHREADS
					pthread_t tid=pthread_self();
					if (pthread_equal(tid,somThis->privdata.eman_blocking->tid))
#else
					DWORD tid=GetCurrentThreadId();
					if (tid==somThis->privdata.eman_blocking->tid)
#endif
					{
						DWORD tid2=0;
						HANDLE h=CreateThread(NULL,0,blip_fd,somThis,0,&tid2);
						if (h)
						{
							__try
							{
								WaitForSingleObjectEx(h,10000,TRUE);
							}
							__finally
							{
								CloseHandle(h);
							}
						}
					}
					else
					{
						blip_fd(somThis);
					}
				}
				else
				{
					blip_fd(somThis);
				}
	#endif
#else
	#ifdef _WIN16
				if (somThis->privdata.eman_blocking)
				{
					if (WSAIsBlocking())
					{
						WSACancelBlockingCall();
					}
				}
	#else
				write(somThis->privdata.eman_sock_write,"2",1);
	#endif
#endif
			}
#endif
		}

#ifdef USE_THREADS
		SOMEEMan_someReleaseEManSem(somSelf,ev);
#endif
	}
}

SOM_Scope long SOMLINK eman_someRegister(
		SOMEEMan SOMSTAR somSelf, 
		Environment *ev, 
		SOMEEMRegisterData SOMSTAR registerData, 
		SOMObject SOMSTAR targetObject, 
		corbastring targetMethod, 
		void* targetData)
{
	SOMEEManData *somThis=SOMEEManGetData(somSelf);
	RHBEMan_Entry *e;

#ifdef USE_THREADS
	SOMEEMan_someGetEManSem(somSelf,ev);
#endif

	e=RHBEMan_allocate(somSelf,&somThis->privdata);

	if (e)
	{
		long token=e->token;

		e->eman_callback_type=eman_cb_event;
		e->eman_callback.eman_event.target=targetObject;
		e->eman_callback.eman_event.pfn=(EMMethodProc *)somResolveByName(targetObject,targetMethod);
		e->eman_callback_token=targetData;

		RHBEMan_fill(&somThis->privdata,ev,e,registerData);

#ifdef USE_THREADS
		SOMEEMan_someReleaseEManSem(somSelf,ev);
#endif

		return token;
	}

#ifdef USE_THREADS
	SOMEEMan_someReleaseEManSem(somSelf,ev);
#endif

	return 0;
}

SOM_Scope long  SOMLINK eman_someRegisterEv(
		SOMEEMan SOMSTAR somSelf, 
		Environment *ev, 
		SOMEEMRegisterData SOMSTAR registerData, 
		SOMObject SOMSTAR targetObject, 
		Environment* callbackEv, 
		corbastring targetMethod, 
		void* targetData)
{
	SOMEEManData *somThis=SOMEEManGetData(somSelf);
	RHBEMan_Entry *e;

#ifdef USE_THREADS
	SOMEEMan_someGetEManSem(somSelf,ev);
#endif

	e=RHBEMan_allocate(somSelf,&somThis->privdata);

	if (e)
	{
		long token=e->token;

		e->eman_callback_type=eman_cb_event_ev;
		e->eman_callback.eman_event_ev.target=targetObject;
		e->eman_callback.eman_event_ev.pfn=(EMMethodProcEv *)somResolveByName(targetObject,targetMethod);
		e->eman_callback.eman_event_ev.ev=callbackEv;
		e->eman_callback_token=targetData;

		RHBEMan_fill(&somThis->privdata,ev,e,registerData);

#ifdef USE_THREADS
		SOMEEMan_someReleaseEManSem(somSelf,ev);
#endif

		return token;
	}

#ifdef USE_THREADS
	SOMEEMan_someReleaseEManSem(somSelf,ev);
#endif

	return 0;
}

SOM_Scope long  SOMLINK eman_someRegisterProc(
		SOMEEMan SOMSTAR somSelf, 
		Environment *ev, 
		SOMEEMRegisterData SOMSTAR registerData, 
		EMRegProc* targetProcedure, 
		void* targetData)
{
	SOMEEManData *somThis=SOMEEManGetData(somSelf);
	RHBEMan_Entry *e;

#ifdef USE_THREADS
	SOMEEMan_someGetEManSem(somSelf,ev);
#endif

	e=RHBEMan_allocate(somSelf,&somThis->privdata);

	if (e)
	{
		long token=e->token;

		e->eman_callback_type=eman_cb_proc;
		e->eman_callback.eman_proc.pfn=targetProcedure;
		e->eman_callback_token=targetData;

		RHBEMan_fill(&somThis->privdata,ev,e,registerData);

#ifdef USE_THREADS
		SOMEEMan_someReleaseEManSem(somSelf,ev);
#endif

		return token;
	}

#ifdef USE_THREADS
	SOMEEMan_someReleaseEManSem(somSelf,ev);
#endif

	return 0;
}

SOM_Scope void  SOMLINK eman_someShutdown(
		SOMEEMan SOMSTAR somSelf, 
		Environment *ev)
{
	SOMEEManData *somThis=SOMEEManGetData(somSelf);

/*	somPrintf("eman_someShutdown\n");*/

	if (somThis->privdata.eman_running)
	{
		if (!somThis->privdata.eman_running->quitting)
		{
#ifdef USE_EMAN_SOCKETPAIR
			if (somThis->privdata.eman_sock_write != INVALID_SOCKET)
			{
#ifdef _WIN32
				if (somThis->privdata.eman_running->hQuitThread)
				{
					RHBOPT_throw_StExcep(ev,TRANSIENT,Busy,MAYBE);
				}
				else
				{
					DWORD tid=0;
					somThis->privdata.eman_running->hQuitThread=CreateThread(NULL,0,blip_fd,somThis,0,&tid);
					if (!somThis->privdata.eman_running->hQuitThread)
					{
						RHBOPT_throw_StExcep(ev,TRANSIENT,CouldNotStartThread,MAYBE);
					}
				}
#else
				send(somThis->privdata.eman_sock_write,"3",1,0);
#endif
			}
#endif
			somThis->privdata.eman_running->quitting++;
		}
	}
}

SOM_Scope void  SOMLINK eman_someUnRegister(
		SOMEEMan SOMSTAR somSelf, 
		Environment *ev, 
		long registrationId)
{
	RHBEMan_Entry *e;
	SOMEEManData *somThis=SOMEEManGetData(somSelf);

#ifdef USE_THREADS
	SOMEEMan_someGetEManSem(somSelf,ev);
#endif

	e=RHBEMan_find(&somThis->privdata,registrationId);

	if (e)
	{
		RHBEMan_delete(somSelf,&somThis->privdata,e);
	}

#ifdef USE_THREADS
	SOMEEMan_someReleaseEManSem(somSelf,ev);
#endif
}

#ifdef RHBEMAN_SINKS
static int set_non_blocking(SOCKET fd)
{
	#ifdef _WIN32
		BOOL ul=1;
		return ioctlsocket(fd,FIONBIO,&ul);
	#else
		#ifdef HAVE_FCNTL_F_SETFL_O_NDELAY
			int i=fcntl(fd,F_SETFL,O_NDELAY);
		#else
			int ul=1;
			int i=ioctl(fd,FIONBIO,(char *)&ul);
		#endif
			if (!i)
			{
		#if defined(F_SETFD) && defined(FD_CLOEXEC)
				i=fcntl(fd,F_SETFD,FD_CLOEXEC);
		#else
			#ifdef FIOCLEX
				/* Cygwin does not cope with this! */
				int k=1;
				i=ioctl(fd,FIOCLEX,(char *)&k);
			#else
				i=fcntl(fd,F_SETFD,1);
			#endif
		#endif
			}
			return i;
	#endif
}
#endif

#if defined(_WIN32) && !defined(USE_SELECT) && !defined(USE_PTHREADS)
#include <sockpair.h>
#endif

SOM_Scope void SOMLINK eman_someWait(SOMEEMan SOMSTAR somSelf,Environment *ev,unsigned long interval)
{
#ifdef _PLATFORM_MACINTOSH_
	EventRecord event; /* 1000/60 ==16.666 */;
	WaitNextEvent(everyEvent,&event,interval>>4,NULL);
	EventRecord_dispatch(&event);
#else
	#ifdef _WIN32
		SleepEx(interval,TRUE);
	#else
		{
			struct timeval tv;
			tv.tv_sec=0;
			tv.tv_usec=0;
			if (interval >= 1000)
			{
				tv.tv_sec=interval >> 10;
			}
			else
			{
				tv.tv_usec=interval << 10;
			}
			select(0,NULL,NULL,NULL,&tv);
		}
	#endif
#endif
}

#ifdef _PLATFORM_MACINTOSH_
static void eman_task(void *pv)
{
#define AEDESC_NULL   {typeNull,NULL}
	SOMEEManData *somThis=pv;
	AppleEvent reply=AEDESC_NULL;
	AppleEvent data=AEDESC_NULL;
	AEDesc address=AEDESC_NULL;
	OSErr err=AECreateDesc(typeProcessSerialNumber,
			&somThis->privdata.psn,
			sizeof(somThis->privdata.psn),
			&address);
	
	if (!err)
	{
		err=AECreateAppleEvent('SOMD','EMAN',&address,kAutoGenerateReturnID,kAnyTransactionID,&data);
	}	

	if (!err)
	{
		err=AESend(&data,&reply,kAENoReply,kAENormalPriority,kNoTimeOut,0,0);
	}
	
	AEDisposeDesc(&address);
	AEDisposeDesc(&data);
	AEDisposeDesc(&reply);
	
	WakeUpProcess(&somThis->privdata.psn);
}
static pascal OSErr eman_ae(const AppleEvent *a,AppleEvent *reply,long c)
{
	SOMEEMan SOMSTAR somSelf=(void *)c;
	SOMEEManData *somThis=SOMEEManGetData(somSelf);
	long num=0;
	boolean b=0;
	
/*	somPrintf("eman_ae event(%p)-begin...\n",somSelf);
	
	SOMEEMan_somDumpSelf(somSelf,0);
	*/
	
	do
	{
		Environment ev;
		SOM_InitEnvironment(&ev);
		b=RHBEMan_doOne(somSelf,&somThis->privdata,&ev,0,0);
		SOM_UninitEnvironment(&ev);
		num+=b;
	} while (b);
	
/*	somPrintf("eman_ae event(%p)-end %ld\n",somSelf,num);*/

	return 0;
}
#endif

SOM_Scope void  SOMLINK eman_somInit(SOMEEMan SOMSTAR somSelf)
{
	SOMEEManData *somThis=SOMEEManGetData(somSelf);

#ifdef USE_THREADS
#ifdef USE_PTHREADS
	somThis->privdata.mainThread=pthread_self();
#endif
	SOMCreateMutexSem(&(somThis->privdata.running_list_mutex));
#endif

	somThis->privdata.timer_queue.queue_type=EMTimerEvent;
	somThis->privdata.dead_timer_queue.queue_type=EMTimerEvent;
	somThis->privdata.sink_queue.queue_type=EMSinkEvent;
	somThis->privdata.client_types_queue.queue_type=EMClientEvent;
	somThis->privdata.workproc_queue.queue_type=EMWorkProcEvent;
#ifdef USE_EMAN_SOCKETPAIR
	somThis->privdata.eman_sock_write=INVALID_SOCKET;
	somThis->privdata.eman_sock_read=INVALID_SOCKET;
#endif

#ifdef _PLATFORM_MACINTOSH_
	GetCurrentProcess(&somThis->privdata.psn);
	somThis->privdata.task_upp=NewAEEventHandlerProc(eman_ae);
	somThis->privdata.task=rhbmtask_new(eman_task,somThis);
	if (somThis->privdata.task)
	{
		if (AEInstallEventHandler('SOMD','EMAN',
			somThis->privdata.task_upp,(long)somSelf,0))
		{
			somThis->privdata.task_upp=NULL;
		}
	}
#endif

#ifdef SOMEEMan_parent_SOMRefObject_somInit
    SOMEEMan_parent_SOMRefObject_somInit(somSelf);
#else
    SOMEEMan_parent_SOMObject_somInit(somSelf);
#endif

#ifdef _WIN32
	{
		WSADATA wsd;
		WSAStartup(0x101,&wsd);
	}
#endif

#ifdef USE_EMAN_SOCKETPAIR
	{
		SOCKET sockfd[2]={INVALID_SOCKET,INVALID_SOCKET};
#if defined(_WIN32)
		if (!socketpair(AF_UNIX,SOCK_STREAM,0,sockfd))
#else
		if (!pipe(sockfd))
#endif
		{
			somThis->privdata.eman_sock_read=sockfd[0];
			somThis->privdata.eman_sock_write=sockfd[1];

			set_non_blocking(somThis->privdata.eman_sock_read);
			set_non_blocking(somThis->privdata.eman_sock_write);
		}
	}
#endif

#ifdef USE_THREADS
	#ifdef USE_PTHREADS
		pthread_mutex_init(&somThis->privdata.eman_mtx,RHBOPT_pthread_mutexattr_default);
		pthread_mutex_init(&somThis->privdata.eman_sem,RHBOPT_pthread_mutexattr_default);
		pthread_cond_init(&somThis->privdata.eman_event,RHBOPT_pthread_condattr_default);
	#else
		somThis->privdata.eman_sem=CreateMutex(NULL,0,NULL);
		somThis->privdata.eman_event=CreateEvent(NULL,0,0,NULL);
	#endif
#endif
}

SOM_Scope void  SOMLINK eman_somUninit(SOMEEMan SOMSTAR somSelf)
{
	SOMEEManData *somThis=SOMEEManGetData(somSelf);

	while (somThis->privdata.cl_events.qHead)
	{
		SOMEClientEventQL *ql=(void *)somThis->privdata.cl_events.qHead;
		rhbeman_Dequeue(&ql->qElem,&somThis->privdata.cl_events);
		ql->header=NULL;
	}

	RHBOPT_ASSERT(!somThis->privdata.cl_events.qHead)
	RHBOPT_ASSERT(!somThis->privdata.timer_queue.queue.qHead)
	RHBOPT_ASSERT(!somThis->privdata.dead_timer_queue.queue.qHead)
	RHBOPT_ASSERT(!somThis->privdata.sink_queue.queue.qHead)
	RHBOPT_ASSERT(!somThis->privdata.client_types_queue.queue.qHead)
	RHBOPT_ASSERT(!somThis->privdata.workproc_queue.queue.qHead)

#ifdef USE_EMAN_SOCKETPAIR
	if (somThis->privdata.eman_sock_write!=INVALID_SOCKET)
	{
		shutdown(somThis->privdata.eman_sock_write,1);
	}
#endif

#ifdef USE_THREADS
	#ifdef USE_PTHREADS
		pthread_mutex_lock(&somThis->privdata.eman_sem);
		while (somThis->privdata.eman_current_nesting)
		{
			pthread_cond_wait(&somThis->privdata.eman_event,&somThis->privdata.eman_sem);
		}
		pthread_mutex_unlock(&somThis->privdata.eman_sem);
		pthread_mutex_destroy(&somThis->privdata.eman_mtx);
		pthread_mutex_destroy(&somThis->privdata.eman_sem);
		pthread_cond_destroy(&somThis->privdata.eman_event);
	#else
		while (somThis->privdata.eman_current_nesting)
		{
			WaitForSingleObject(somThis->privdata.eman_event,INFINITE);
		}
		CloseHandle(somThis->privdata.eman_sem);
		CloseHandle(somThis->privdata.eman_event);
	#endif
#endif
#ifdef _WIN32
	#define BLAT_SOCKET(x)  if (x!=INVALID_SOCKET) { SOCKET s=x; x=INVALID_SOCKET; closesocket(s); }
#else
	#define BLAT_SOCKET(x)  if (x!=INVALID_SOCKET) { SOCKET s=x; x=INVALID_SOCKET; close(s); }
#endif
#ifdef USE_EMAN_SOCKETPAIR
	BLAT_SOCKET(somThis->privdata.eman_sock_write);
	BLAT_SOCKET(somThis->privdata.eman_sock_read);
#endif
#ifdef _WIN32
	WSACleanup();
#endif

#ifdef _PLATFORM_MACINTOSH_
	if (somThis->privdata.task)
	{
		somThis->privdata.task->lpVtbl->Release(somThis->privdata.task);
	}
	if (somThis->privdata.task_upp)
	{
		AERemoveEventHandler('SOMD','EMAN',somThis->privdata.task_upp,0);
		somThis->privdata.task_upp=0;
	}
#endif

	{
		Environment ev;
/*		SOMMSingleInstanceClassDataStructure *cds2=&SOMMSingleInstanceClassData;
		somPrintf("cds2=%p\n",cds2);*/
		SOM_InitEnvironment(&ev);
		SOMMSingleInstance_sommSingleInstanceFreed(somSelf->mtab->classObject,&ev,somSelf);
		SOM_UninitEnvironment(&ev);
	}

	while (somThis->privdata.list)
	{
		struct RHBEMan_Entry *e=somThis->privdata.list;
		RHBEMan_delete(somSelf,&somThis->privdata,e);
	}

#ifdef USE_THREADS
	SOMDestroyMutexSem(somThis->privdata.running_list_mutex);
#endif

#ifdef SOMEEMan_parent_SOMRefObject_somUninit
    SOMEEMan_parent_SOMRefObject_somUninit(somSelf);
#else
    SOMEEMan_parent_SOMObject_somUninit(somSelf);
#endif
}
