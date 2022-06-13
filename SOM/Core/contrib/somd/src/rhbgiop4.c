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
#include <rhbsomd.h>
#include <rhbgiops.h>

#ifdef USE_SELECT
	#include <sinkev.h>
	#include <emregdat.h>
#endif

#ifdef HAVE_ERRNO_H
	#include <errno.h>
#endif

#ifdef _PLATFORM_MACINTOSH_
	#include <Processes.h>
	#ifdef __ASLM__
		#include <LibraryManagerUtilities.h>
	#endif
	#include <rhbmdisp.h>
#endif

/* this contains 'EventManager'  */

#ifdef USE_SELECT
	static void SOMLINK RHBEventManager_EmanCallback_read(SOMEEvent SOMSTAR a,void *e);
	static void SOMLINK RHBEventManager_EmanCallback_write(SOMEEvent SOMSTAR a,void *e);
	static void SOMLINK RHBEventManager_EmanCallback_except(SOMEEvent SOMSTAR a,void *e);
#endif

static void rhbEventManager_uninit(
	RHBEventManager *somThis)
{
	RHBOPT_unused(somThis)
}

#ifdef USE_SELECT
static RHBSocketEvent * rhbEventManager_add(
		RHBEventManager *somThis,
		SOCKET fd,
		RHBEventCallback cb,
		void *pv)
{
	RHBSocketEvent *t;

#ifdef USE_THREADS
	getMutex();
#endif

	t=(RHBSocketEvent *)SOMMalloc(sizeof(*t));

	t->fd=fd;
	t->pfn=cb;
	t->refCon=pv;
	t->event_mask=EMInputReadMask;
	t->read_token=0;
	t->write_token=0;
	t->except_token=0;

	t->c_orb=somThis->orb;

#ifdef _PLATFORM_MACINTOSH_
	GetCurrentProcess(&t->psn);
#endif

	t->next=somThis->eventList;
	somThis->eventList=t;

	t->regData=0;

	if (somThis->user_eman)
	{
#ifdef somNewObject
		t->regData=somNewObject(SOMEEMRegisterData);
#else
		t->regData=SOMEEMRegisterDataNew();
#endif

		if (t->regData)
		{
			Environment ev;

			SOM_InitEnvironment(&ev);
			SOMEEMRegisterData_someClearRegData(t->regData,&ev);
			somva_SOMEEMRegisterData_someSetRegDataEventMask(t->regData,&ev,EMSinkEvent,NULL);
			SOMEEMRegisterData_someSetRegDataSink(t->regData,&ev,fd);
			SOMEEMRegisterData_someSetRegDataSinkMask(t->regData,&ev,t->event_mask);

			t->read_token=SOMEEMan_someRegisterProc(
						somThis->user_eman,
						&ev,
						t->regData,
						RHBEventManager_EmanCallback_read,
						t);

			SOM_UninitEnvironment(&ev);
		}
	}

#ifdef USE_THREADS
	releaseMutex();
#endif

	return t;
}
#endif

#ifdef USE_SELECT
static void rhbEventManager_move_to_end(
		RHBEventManager *somThis,
		RHBSocketEvent *event)
{
	RHBSocketEvent *p;

	if (event->next)
	{
		p=somThis->eventList;

		if (p==event)
		{
			somThis->eventList=p->next;
		}
		else
		{
			while (p->next != event)
			{
				p=p->next;
			}

			p->next=event->next;
		}

		event->next=0;

		if (somThis->eventList)
		{
			p=somThis->eventList;

			while (p->next)
			{
				p=p->next;
			}

			p->next=event;
		}
		else
		{
			somThis->eventList=event;
		}
	}
}
#endif

#ifdef USE_SELECT
static void rhbEventManager_set_mask(
	RHBEventManager *somThis,
	RHBSocketEvent *event,
	int mask)
{
	if (event->event_mask != mask)
	{
		int old_mask=event->event_mask;
		int diff=old_mask ^ mask;

		event->event_mask=mask;

		if (event->regData)
		{
			Environment ev;
			SOM_InitEnvironment(&ev);

			if (diff & EMInputReadMask)
			{
				if (old_mask & EMInputReadMask)
				{
					long token=event->read_token;
					event->read_token=0;
					SOMEEMan_someUnRegister(somThis->user_eman,&ev,token);
				}
				else
				{
					SOMEEMRegisterData_someSetRegDataSinkMask(event->regData,&ev,EMInputReadMask);

					event->read_token=SOMEEMan_someRegisterProc(
								somThis->user_eman,
								&ev,
								event->regData,
								RHBEventManager_EmanCallback_read,
								event);
				}
			}

			if (diff & EMInputWriteMask)
			{
				if (old_mask & EMInputWriteMask)
				{
					long token=event->write_token;
					event->write_token=0;
					SOMEEMan_someUnRegister(somThis->user_eman,&ev,token);
				}
				else
				{
					SOMEEMRegisterData_someSetRegDataSinkMask(event->regData,&ev,EMInputWriteMask);

					event->write_token=SOMEEMan_someRegisterProc(
								somThis->user_eman,
								&ev,
								event->regData,
								RHBEventManager_EmanCallback_write,
								event);
				}
			}

			if (diff & EMInputExceptMask)
			{
				if (old_mask & EMInputWriteMask)
				{
					long token=event->except_token;
					event->except_token=0;
					SOMEEMan_someUnRegister(somThis->user_eman,&ev,token);
				}
				else
				{
					SOMEEMRegisterData_someSetRegDataSinkMask(event->regData,&ev,EMInputExceptMask);

					event->except_token=SOMEEMan_someRegisterProc(
								somThis->user_eman,
								&ev,
								event->regData,
								RHBEventManager_EmanCallback_except,
								event);
				}
			}

			SOM_UninitEnvironment(&ev);
		}
	}
}
#endif

#ifdef USE_SELECT
static void rhbEventManager_remove(
		RHBEventManager *somThis,
		RHBSocketEvent *event)
{
	RHBSocketEvent *p;

	p=somThis->eventList;

	if (p==event)
	{
		somThis->eventList=p->next;
	}
	else
	{
		while (p->next != event)
		{
			p=p->next;
		}

		p->next=event->next;
	}

	if (event->regData)
	{
		rhbEventManager_set_mask(somThis,event,0);

		somReleaseObjectReference(event->regData);
	}

	SOMFree(event);
}
#endif

#ifdef USE_SELECT
static void rhbEventManager_handle_one(
		RHBEventManager *somThis,
		Environment *ev,
		unsigned long flags)
{
#ifdef _PLATFORM_MACINTOSH_
	struct RHBAsyncEvent *async=NULL;
	char *p=(void *)somThis->osQ.qHead;
	if (p)
	{
		int i=(int)(long)&(((struct RHBAsyncEvent *)0)->qElem);
		async=(void *)(p-i);
		i=Dequeue(&async->qElem,&somThis->osQ);
		if (!i)
		{
			async->qHdr=NULL;
			async->callback(async);
		}
		else
		{
			somPrintf("Dequeue(%p,%d)->failed\n",async,i);
			async=NULL;
		}
	}

	if (!async) /* only yield if no queued task */
#endif
	{
		struct timeval tv={0,0};

		if (!(flags & RESP_NO_WAIT))
		{
			tv.tv_sec=6000;
		}

		if (somThis->user_eman)
		{
			Environment ev;
			long ms=(tv.tv_usec/1000)+(tv.tv_sec*1000);

			SOM_InitEnvironment(&ev);

			SOMEEMan_someProcessEventInterval(
					somThis->user_eman,&ev,
					EMProcessTimerEvent | EMProcessSinkEvent | EMClientEvent,
					ms,0);

			SOM_UninitEnvironment(&ev);
		}
		else
		{
			Sockets SOMSTAR socks=somThis->socketObject;
			RHBSocketEvent *event=somThis->eventList;
			fd_set fd_read;
			fd_set fd_write;
			fd_set fd_except;
			int i=0;
			boolean any_reads=0,any_writes=0,any_excepts=0;

			FD_ZERO(&fd_read);
			FD_ZERO(&fd_write);
			FD_ZERO(&fd_except);

			while (event)
			{
				if (event->event_mask & EMInputReadMask)
				{
					FD_SET(event->fd,&fd_read);

		/*			somPrintf("sel(%d) for reading\n",event->fd);*/

					if (event->fd >= (SOCKET)i) i=event->fd+1;

					any_reads=1;
				}

				if (event->event_mask & EMInputWriteMask)
				{
					FD_SET(event->fd,&fd_write);

		/*			somPrintf("sel(%d) for writing\n",event->fd);*/

					if (event->fd >= (SOCKET)i) i=event->fd+1;

					any_writes=1;
				}

				if (event->event_mask & EMInputExceptMask)
				{
					FD_SET(event->fd,&fd_except);

		/*			somPrintf("sel(%d) for except\n",event->fd);*/

					if (event->fd >= (SOCKET)i) i=event->fd+1;

					any_excepts=1;
				}

				event=event->next;
			}

			if (tv.tv_usec >= 1000000L)
			{
				tv.tv_sec+=(tv.tv_usec / 1000000L);
				tv.tv_usec=(tv.tv_usec % 1000000L);
			}

	/*		bomb("why select here and not in eman?");*/

	/*		somPrintf("[%d]select(maxfd=%d,tv_sec=%ld,tv_usec=%d,file=%s,line=%d)\n",
					getpid(),
					(int)i,
					(long)tv.tv_sec,
					(long)tv.tv_usec,
					__FILE__,
					__LINE__);*/

			RHBOPT_ASSERT(socks);

			if (socks)
			{
				Environment ev;
				SOM_InitEnvironment(&ev);

/*				somPrintf("using Sockets_somsSelect(%d,{%ld,%ld})\n",
						i,tv.tv_sec,tv.tv_usec);*/

				i=Sockets_somsSelect(socks,&ev,i,
					any_reads ? &fd_read : NULL,
					any_writes ? &fd_write : NULL,
					any_excepts ? &fd_except : NULL,
					&tv);

				SOM_UninitEnvironment(&ev);
			}
			else
			{
				i=-1;
			}

			if (i > 0)
			{
				event=somThis->eventList;

				while (event && i)
				{
					int mask=0;

					if (FD_ISSET(event->fd,&fd_read))
					{
						FD_CLR(event->fd,&fd_read);
						mask|=EMInputReadMask;
					}

					if (FD_ISSET(event->fd,&fd_write))
					{
						FD_CLR(event->fd,&fd_write);
						mask|=EMInputWriteMask;
					}

					if (FD_ISSET(event->fd,&fd_except))
					{
						FD_CLR(event->fd,&fd_except);
						mask|=EMInputExceptMask;
					}

					if (mask)
					{
						RHBEventManager_move_to_end(somThis,event);

						event->pfn(ev,event->fd,mask,event->refCon);

						i--;

						if (i)
						{
							event=somThis->eventList;
						}
						else
						{
							event=NULL;
						}
					}
					else
					{
						event=event->next;
					}
				}
			}
		}
	}

	while (somThis->orb)
	{
		RHBServerRequest *req=RHBORB_pop_first_queued_request(somThis->orb);

		if (req) 
		{
			SOM_TRY

			req->qel.process_request(req,ev);

			SOM_CATCH_ALL

			somPrintf("request exception caught,%s:%d\n",__FILE__,__LINE__);

			SOM_ENDTRY

			somExceptionFree(ev);
		}
		else
		{
			break;
		}
	}
}
#endif


#ifdef USE_SELECT
static void SOMLINK RHBEventManager_EmanCallback_read(SOMEEvent SOMSTAR a,void *f)
{
	RHBOPT_ASSERT(SOMObject_somIsA(a,SOMESinkEventClassData.classObject));

	{
		RHBSocketEvent *e=f;
		Environment ev;
		RHBORB *c_orb=e->c_orb;

		SOM_InitEnvironment(&ev);

		e->pfn(&ev,e->fd,EMInputReadMask,e->refCon);

		SOM_UninitEnvironment(&ev);

		if (c_orb->events.user_callback)
		{
			c_orb->events.user_callback(a,f);
		}
	}
}

static void SOMLINK RHBEventManager_EmanCallback_write(SOMEEvent SOMSTAR a,void *f)
{
	RHBOPT_ASSERT(SOMObject_somIsA(a,SOMESinkEventClassData.classObject));

	{
		RHBSocketEvent *e=f;
		Environment ev;
		RHBORB *c_orb=e->c_orb;

		SOM_InitEnvironment(&ev);

		e->pfn(&ev,e->fd,EMInputWriteMask,e->refCon);

		SOM_UninitEnvironment(&ev);

		if (c_orb->events.user_callback)
		{
			c_orb->events.user_callback(a,f);
		}
	}
}

static void SOMLINK RHBEventManager_EmanCallback_except(SOMEEvent SOMSTAR a,void *f)
{
	RHBOPT_ASSERT(SOMObject_somIsA(a,SOMESinkEventClassData.classObject));

	{
		RHBSocketEvent *e=f;
		Environment ev;
		RHBORB *c_orb=e->c_orb;

		SOM_InitEnvironment(&ev);

		e->pfn(&ev,e->fd,EMInputExceptMask,e->refCon);

		SOM_UninitEnvironment(&ev);

		if (c_orb->events.user_callback)
		{
			c_orb->events.user_callback(a,f);
		}
	}
}

static void rhbAsyncEvent_uninit(struct RHBAsyncEvent *ae)
{
	if (ae->clientEvent)
	{
		SOMObject SOMSTAR o=ae->clientEvent;
		ae->clientEvent=NULL;
		SOMObject_somFree(o);
	}

#ifdef _PLATFORM_MACINTOSH_
	if (ae->qHdr)
	{
		Dequeue(&ae->qElem,ae->qHdr);
		ae->qHdr=NULL;
	}
#endif
}

static void rhbAsyncEvent_post(struct RHBAsyncEvent *ae)
{
	RHBORB *orb=ae->orb;
	if (orb)
	{
		SOMEEMan SOMSTAR emanObj=orb->events.user_eman;

		if (emanObj)
		{
			/* dont want environment at interrupt time */
			SOMEEMan_someQueueEvent(emanObj,NULL,ae->clientEvent);
		}
		else
		{
#ifdef _PLATFORM_MACINTOSH_
			WakeUpProcess(&orb->events.psn);

			if (!ae->qHdr)
			{
				ae->qHdr=&orb->events.osQ;
				Enqueue(&ae->qElem,&orb->events.osQ);
			}
#endif
		}
	}
}

static struct RHBAsyncEventVtbl rhbAsyncEventVtbl={
	rhbAsyncEvent_post,
	rhbAsyncEvent_uninit};

static SOMObject SOMSTAR rhbEventManager_AsyncEvent_init(
		RHBEventManager *somThis,
		Environment *ev,
		struct RHBAsyncEvent *ae)
{
	SOMEClientEvent SOMSTAR e=SOMEClientEventNew();
	RHBORB *orb=somThis->orb;

	ae->lpVtbl=&rhbAsyncEventVtbl;

	SOMEClientEvent_somevSetEventClientData(e,ev,ae);
	SOMEClientEvent_somevSetEventClientType(e,ev,somThis->async_proc_type);

	ae->clientEvent=e;
	ae->orb=orb;
#ifdef _PLATFORM_MACINTOSH_
	ae->qHdr=NULL;
#endif
	return e;
}
#endif

static struct RHBEventManagerVtbl rhbEventManagerVtbl={
	rhbEventManager_uninit
#ifdef USE_SELECT
	,rhbEventManager_handle_one,
	rhbEventManager_add,
	rhbEventManager_remove,
	rhbEventManager_set_mask,
	rhbEventManager_move_to_end,
	rhbEventManager_AsyncEvent_init
#endif
};

SOMEXTERN void SOMLINK RHBEventManager_init(RHBEventManager *somThis,RHBORB *the_orb)
{
#ifdef USE_THREADS
#else
static char async_proc_type[]="ecfeb170-1d0b-11d6-8bc4-0080c8c1ab83";
#endif

	somThis->orb=the_orb;
	somThis->lpVtbl=&rhbEventManagerVtbl;

#ifdef USE_THREADS
#else
	somThis->eventList=NULL;
	somThis->socketObject=NULL;
	somThis->user_eman=NULL;
	somThis->user_callback=NULL;
	somThis->async_proc_token=0;
	somThis->async_proc_type=async_proc_type;
#ifdef _PLATFORM_MACINTOSH_
	memset(&somThis->osQ,0,sizeof(somThis->osQ));
	somThis->osQ.qHead=NULL;
	somThis->osQ.qTail=NULL;
	GetCurrentProcess(&somThis->psn);
#endif
#endif
}

SOM_Scope int RHBLINK RHBSocketListener_QueryInterface(RHBSocketSink *somSelf,void *iid,void **ppv)
{
	return -1;
}

SOM_Scope int RHBLINK RHBSocketListener_AddRef(RHBSocketSink *somSelf)
{
	struct RHBSocketListener *somThis=(void *)somSelf;

	somd_atomic_inc(&somThis->lUsage);

	return 0;
}

SOM_Scope int RHBLINK RHBSocketListener_Release(RHBSocketSink *somSelf)
{
	struct RHBSocketListener *somThis=(void *)somSelf;

	if (!somd_atomic_dec(&somThis->lUsage))
	{
		RHBOPT_ASSERT(!somThis->fd)
		RHBOPT_ASSERT(!somThis->server)

		SOMFree(somThis);
	}
	return 0;
}

SOM_Scope void RHBLINK RHBSocketListener_Notify(RHBSocketSink *somSelf,struct RHBSocket *sock,short what,long err)
{
	struct RHBSocketListener *somThis=(void *)somSelf;
	if (somThis->server)
	{
		switch (what)
		{
		case RHBSocket_notify_close:
			/* MacTCP/OpenTransport will send this if provider changes */
			if (somThis->server->somoa)
			{
				SOMOA SOMSTAR somoa=SOMOA_somDuplicateReference(somThis->server->somoa);
				Environment ev;
				SOM_InitEnvironment(&ev);
				SOMOA_interrupt_server_status(somoa,&ev,SOMDERROR_SocketClose);
				SOM_UninitEnvironment(&ev);
				somReleaseObjectReference(somoa);
			}
			break;
		case RHBSocket_notify_accept:
			if (somThis->server->listener)
			{
				RHBSocketSink *sink=&somThis->server->listener->sink.vtbl;
				(*sink)->Notify(sink,sock,what,err);
			}
			break;
		}
	}
}

static boolean RHBSocketListener_begin_listen(
		struct RHBSocketListener *somThis,
		struct RHBServerData *server,
		short family,
		struct sockaddr *addr,int addrlen)
{
	RHBSocketRef fd=RHBProtocol_CreateStream(somThis->orb->protocol,family);
	boolean rc=0;

	if (fd)
	{
		RHBSocketError listenError=0;
		RHBORB_sockaddr retaddr;
		RHBSocketLength retaddrlen=sizeof(retaddr);

		RHBSocket_StartListen(fd,&listenError,
				addr,
				(RHBSocketLength)addrlen,
				&retaddr.u.generic,
				&retaddrlen);

		if (!listenError)
		{
			switch (retaddr.u.generic.sa_family)
			{
			case AF_INET:
				somThis->port=ntohs(retaddr.u.ipv4.sin_port);
/*				somPrintf("listen4(%d)\n",somThis->port); */
				break;
#ifdef HAVE_SOCKADDR_IN6
			case AF_INET6:
				somThis->port=ntohs(retaddr.u.ipv6.sin6_port);
/*				somPrintf("listen6(%d)\n",somThis->port); */
				break;
#endif
			}

			RHBORB_guard(0)

			somThis->next=server->listener_list;
			server->listener_list=somThis;
			somThis->server=server;

			somThis->fd=fd;

			RHBORB_unguard(0)

			RHBSocket_IsNonBlocking(fd,&somThis->vtbl);				

			rc=1;
		}
		else
		{
			RHBSocket_Release(fd);
		}
	}

	return rc;
}

static void RHBSocketListener_end_listen(
		struct RHBSocketListener *somThis)
{
	RHBSocketRef fd=NULL;

	RHBSOM_Trace("end_listen - begin");
		
	RHBORB_guard(0)

	fd=somThis->fd;
	somThis->fd=NULL;

	RHBSOM_Trace("end_listen - unhooking");

	if (somThis->server)
	{
		if (somThis->server->listener_list==somThis)
		{
			somThis->server->listener_list=somThis->next;
		}
		else
		{
			struct RHBSocketListener *p=somThis->server->listener_list;
			while (p->next != somThis)
			{
				p=p->next;
			}
			p->next=somThis->next;
		}

		somThis->server=NULL;
	}

	RHBSOM_Trace("end_listen - unhooked");

	RHBORB_unguard(0)

	if (fd)
	{
		RHBSOM_Trace("end_listen - set non-blocking NULL");
		RHBSocket_IsNonBlocking(fd,NULL);
		RHBSOM_Trace("end_listen - close socket");
		RHBSocket_Close(fd);
		RHBSOM_Trace("end_listen - release socket");
		RHBSocket_Release(fd);
	}

	RHBSOM_Trace("end_listen - end");
}

static struct RHBSocketSinkJumpTable RHBSocketListenerSinkVtbl={
	RHBSocketListener_QueryInterface,
	RHBSocketListener_AddRef,
	RHBSocketListener_Release,
	RHBSocketListener_Notify
};

static struct RHBSocketListenerVtbl RHBSocketListenerVtbl={
	RHBSocketListener_begin_listen,
	RHBSocketListener_end_listen
};

struct RHBSocketListener *RHBSocketListenerNew(RHBORB *orb)
{
	struct RHBSocketListener *somThis=SOMMalloc(sizeof(*somThis));
	somThis->lUsage=1;
	somThis->fd=NULL;
	somThis->next=NULL;
	somThis->server=NULL;
	somThis->vtbl=&RHBSocketListenerSinkVtbl;
	somThis->vtbl2=&RHBSocketListenerVtbl;
	somThis->port=0;
	somThis->orb=orb;
	return somThis;
}

