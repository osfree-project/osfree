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

#define DEFAULT_IIOP_PORT  1571	/* ORBIX port */

#include <rhbopt.h>
#include <rhbsomd.h>
#include <somestio.h>
#include <somuutil.h>

#ifdef _PLATFORM_UNIX_
#	include <pwd.h>
#endif

#ifdef HAVE_SIGNAL_H
#	include <signal.h>
#endif

#ifdef _PLATFORM_MACINTOSH_
#	include <Resources.h>
#	include <Processes.h>
#endif

#include <rhbgiops.h>

#if defined(BUILD_STATIC) && !defined(USE_THREADS)
#	include <tcpsock.h>
#endif

/*

1. contained_describe to return flag if local or remote
2. TypeCodeNew("::SOMDServer") needs to be constant
3. initialise a load of somId's which are liberally scattered to 
reduce calling somIdFromString

*/

static char szSOMD_TCPIP[]="[SOMD_TCPIP]",
			szHOSTNAME[]="HOSTNAME",
			szSOMDPORT[]="SOMDPORT",
			szLocalhost[]="localhost",
#ifdef _WIN32
			szUSER[]="USERNAME";
#else
			szUSER[]="USER";
#endif

struct somIdMap
{
	short offset;
	const char *value;
};

#define offset_of(x,y)     (short)(size_t)(void *)&(((x *)NULL)->y)
#define somIdMapEntry(p,q)   {offset_of(RHBORB,p),q}

#define somIdMapId(p,o)     *((somId *)(((char *)p)+o))

static struct somIdMap orbIdMap[]={
	somIdMapEntry(somId_get_impl_hostname,"_get_impl_hostname"),
	somIdMapEntry(somId_get_impl_id,"_get_impl_id"),
	somIdMapEntry(somId_get_somModifiers,"_get_somModifiers"),
	somIdMapEntry(somId_get_absolute_name,"_get_absolute_name"),
	somIdMapEntry(somId_get_id,"_get_id"),
	somIdMapEntry(somIddescribe,"describe"),
	somIdMapEntry(somIdRepository,"Repository")
};

static char *rhbORB_id_to_abs(RHBORB *somThis,const char *id)
{
	char *abs_name=NULL;

#ifdef DEBUG_ABS_ID
	somPrintf("id_to_abs(\"%s\")=",id);
#endif

	if (id && 
		(id[0]=='I') &&
		(id[1]=='D') &&
		(id[2]=='L') &&
		(id[3]==':'))
	{
		const char *p;
		unsigned short slashCount=0;
		size_t newLen;
		id+=4;
		p=id;
		while (*p)
		{
			if (*p==':') break;
			if (*p++=='/') slashCount++;
		}

		/* length, 2 for leading ::, 1 for null, slashCount for extra colons */
		newLen=(p-id)+2+1+slashCount;	

		abs_name=SOMMalloc(newLen);

		if (abs_name)
		{
			char *q=abs_name;

			while (id[0])
			{
				size_t segLen=0;
				p=id;

				while (p[0] && (p[0]!='/') && (p[0]!=':'))
				{
					p++;
					segLen++;
				}

				if (!segLen) break;

				*q++=':';
				*q++=':';

				memcpy(q,id,segLen);
				q+=segLen;
				id+=segLen;
			
				while (id[0]=='/')
				{
					id++;
				}
			}

			*q=0;

			RHBOPT_ASSERT((abs_name+newLen)==(q+1))
		}
	}

#ifdef DEBUG_ABS_ID
	somPrintf("\"%s\"\n",abs_name);
#endif

	return abs_name;
}

#ifdef USE_THREADS

static int thread_list_remove(struct RHBORB_ThreadTask *somThis,
							   struct RHBORB_ThreadTask **list)
{
	if (*list==somThis)
	{
		*list=somThis->next;

		return 1;
	}
	else
	{
		struct RHBORB_ThreadTask *p=*list;

		while (p)
		{
			if (p->next==somThis)
			{
				p->next=somThis->next;

				return 1;
			}

			p=p->next;
		}
	}

	return 0;
}

struct RHBORB_killer
{
	struct RHBORB_ThreadTask *pending;
#ifdef USE_PTHREADS
	pthread_cond_t pEvent;
#else
	HANDLE hEvent;
#endif
};

static void RHBORB_join(RHBORB *somThis)
{
	while (somThis->child.dead_list)
	{
#if defined(USE_PTHREADS) 
		void *pv=NULL;
#else
		DWORD dw=0;
#endif
		struct RHBORB_ThreadTask *task=somThis->child.dead_list;
		somThis->child.dead_list=task->next;

		RHBORB_unguard(somThis)

#ifdef USE_PTHREADS
		pthread_join(task->pThread,&pv);
#else
		WaitForSingleObjectEx(task->hThread,INFINITE,1);
		GetExitCodeThread(task->hThread,&dw);
		CloseHandle(task->hThread);
#endif
		SOMFree(task);

		RHBORB_guard(somThis)

		if (somThis->child.killer)
		{
#ifdef USE_PTHREADS
			pthread_cond_signal(&somThis->child.killer->pEvent);
#else
			if (!SetEvent(somThis->child.killer->hEvent))
			{
				RHBOPT_ASSERT(0)
			}
#endif
		}
	}
}

static struct RHBORB_thread_task *RHBORB_pop_task(RHBORB *somThis)
{
	if (somThis->child.tasks)
	{
		struct RHBORB_thread_task *task=somThis->child.tasks;
		somThis->child.tasks=task->next;
		if (somThis->child.killer)
		{
#ifdef USE_PTHREADS
			pthread_cond_signal(&somThis->child.killer->pEvent);
#else
			if (!SetEvent(somThis->child.killer->hEvent))
			{
				RHBOPT_ASSERT(0)
			}
#endif
		}
		return task;
	}
	return NULL;
}

static void RHBORB_CloseThreads(RHBORB *somThis)
{
	struct RHBORB_killer killer={NULL};

	RHBORB_guard(somThis)

#ifdef USE_PTHREADS
		pthread_cond_init(&killer.pEvent,RHBOPT_pthread_condattr_default);
#else
		killer.hEvent=CreateEvent(0,0,0,0);
#endif

	while (somThis->threads ||
			somThis->child.tasks ||
			killer.pending)
	{
		RHBORB_ThreadTask *t=somThis->threads;

		while (t)
		{
			RHBORB_ThreadTask *n=t->next;

			RHBOPT_ASSERT(!t->killer)

			if (!t->task)
			{
				t->task=RHBORB_pop_task(somThis);

				if (!t->task)
				{
					thread_list_remove(t,&somThis->threads);

					t->killer=&killer;
					t->next=killer.pending;
					killer.pending=t;
				}

		#ifdef USE_PTHREADS
				pthread_cond_signal(&t->pEvent);
		#else
				if (!SetEvent(t->hEvent)) { RHBOPT_ASSERT(0); }
		#endif
			}

			t=n;
		}

		if (somThis->child.tasks)
		{
#ifdef USE_PTHREADS
			pthread_cond_signal(&somThis->child.pEvent);
#else
			if (!SetEvent(somThis->child.hEvent))
			{
				RHBOPT_ASSERT(0)
			}
#endif
		}

		if (killer.pending || somThis->child.tasks || somThis->threads)
		{
			RHBOPT_ASSERT(!somThis->child.killer)

			somThis->child.killer=&killer;

#if defined(USE_PTHREADS)
			somd_wait(&killer.pEvent,__FILE__,__LINE__);
#else
			somd_wait(killer.hEvent,__FILE__,__LINE__);
#endif
			somThis->child.killer=NULL;
		}
	}

	RHBORB_unguard(somThis)

#ifdef USE_PTHREADS
	pthread_cond_destroy(&killer.pEvent);
#else
	if (!CloseHandle(killer.hEvent))
	{
		RHBOPT_ASSERT(0)
	}
#endif
}
#endif

#ifdef USE_THREADS
struct RHBORB_ThreadTask_local
{
	boolean guarded;
	RHBORB_ThreadTask *somThis;
	Environment ev;
};

RHBOPT_cleanup_begin(RHBORB_ThreadTaskCleanup,pv)

	struct RHBORB_ThreadTask_local *data=pv;
	RHBORB_ThreadTask *somThis=data->somThis;
	RHBORB *orb=somThis->orb_ptr;
	RHBSOMD_set_thread_specific(NULL);

#ifdef _DEBUG
	if (somThis->server_request)
	{
		SOMD_bomb("this should have been cleared");
	}
#endif

	RHBOPT_ASSERT(orb)

	if (!data->guarded)
	{
		RHBORB_guard(orb)

		data->guarded=1;
	}

	if (thread_list_remove(somThis,&(somThis->orb_ptr->threads)))
	{
		RHBOPT_ASSERT(!somThis->killer)

		somThis->killer=somThis->orb_ptr->child.killer;
	}

#ifdef USE_PTHREADS
	pthread_cond_destroy(&somThis->pEvent);
#else
	if (!CloseHandle(somThis->hEvent))
	{
		RHBOPT_ASSERT(0)
	}
#endif

	if (somThis->killer)
	{
		/* add self to obituary list of killed... */
		struct RHBORB_killer *killer=somThis->killer;

		thread_list_remove(somThis,&(killer->pending));

#ifdef USE_PTHREADS
		pthread_cond_signal(&killer->pEvent);
#else
		if (!SetEvent(killer->hEvent)) { RHBOPT_ASSERT(!"event wrong"); }
#endif
	}

	if (orb)
	{
		somThis->next=orb->child.dead_list;
		orb->child.dead_list=somThis;

#ifdef USE_PTHREADS
		pthread_cond_signal(&orb->child.pEvent);
#else
		if (!SetEvent(orb->child.hEvent))
		{
			RHBOPT_ASSERT(0)
		}
#endif
	}
	else
	{
	#ifdef USE_PTHREADS
		pthread_detach(somThis->pThread);
	#else
		CloseHandle(somThis->hThread);
	#endif
		SOMFree(somThis);
	}

	RHBORB_unguard(orb)

RHBOPT_cleanup_end

#if defined(USE_PTHREADS) 
	static void * RHBORB_ThreadTaskStarter(void *pv)
#else
	static int RHBSOMD_thread_filter(DWORD dw,DWORD *p)
	{
		*p=dw;
#ifdef _DEBUG
		if (dw != ERROR_OPERATION_ABORTED) return 0;
#endif
		return 1;
	}
	static DWORD CALLBACK RHBORB_ThreadTaskStarter(void *pv)
#endif
{
	struct RHBORB_ThreadTask_local data={0,NULL};
	RHBORB_ThreadTask *somThis=pv;

#ifdef USE_PTHREADS
#ifdef PTHREAD_CANCEL_DEFERRED
	{
		int type;
			pthread_setcanceltype(
				PTHREAD_CANCEL_DEFERRED,
				&type);
	}
#endif
#ifdef PTHREAD_CANCEL_ENABLE
	{
		int state;
		pthread_setcancelstate(
				PTHREAD_CANCEL_ENABLE,
				&state);
	}
#endif
#endif

#if defined(_WIN32) && !defined(USE_PTHREADS) 
	DWORD dwException=0;
	__try {
#endif

	data.somThis=somThis;

	RHBOPT_cleanup_push(RHBORB_ThreadTaskCleanup,&data);

	RHBSOMD_set_thread_specific(somThis);

	if (!somThis->orb_ptr) SOMD_bomb("why no ORB");

	RHBORB_guard(orb)

	data.guarded=1;

	while (somThis->task)
	{
		struct RHBORB_thread_task task;
		task=somThis->task[0];

		data.guarded=0;

		RHBOPT_ASSERT(!somThis->server_request);

		RHBORB_unguard(orb)

		if (task.start)
		{
#ifdef _WIN32
			void (SOMLINK * func)(void *,Environment *ev)=task.start;
#endif

			SOM_InitEnvironment(&data.ev);

			task.start(task.param,&data.ev);

			RHBOPT_ASSERT(!somThis->server_request);

			SOM_UninitEnvironment(&data.ev);
		}

		RHBORB_guard(orb)

		data.guarded=1;

		somThis->task=RHBORB_pop_task(somThis->orb_ptr);

		if (!somThis->task)
		{
			if (somThis->killer) break;
			if (somThis->orb_ptr->child.killer) break;

#if defined(USE_PTHREADS) 
			somd_timed_wait(
				&somThis->pEvent,
				30,__FILE__,__LINE__);
#else
			somd_timed_wait(
				somThis->hEvent,
				30,__FILE__,__LINE__);
#endif
		}
	}

	RHBOPT_cleanup_pop();

#if defined(_WIN32) && !defined(USE_PTHREADS) 
	} __except(RHBSOMD_thread_filter(_exception_code(),&dwException)) {
#ifdef _DEBUG
		somPrintf("# SOMD::EXCEPTION 0x%lx\n",dwException);
#endif
	}
#endif

	return 0;
}

static boolean RHBORB_queue_task(RHBORB *somThis,struct RHBORB_thread_task *task)
{
	RHBORB_ThreadTask *t=somThis->threads;

	RHBSOMD_must_be_guarded

	RHBOPT_ASSERT(!somThis->child.killer)

	RHBOPT_ASSERT(!somThis->closing)

	while (t)
	{
		if (t->task)
		{
			t=t->next;
		}
		else
		{
			t->task=task;

#ifdef USE_PTHREADS
			pthread_cond_signal(&t->pEvent);
#else
			if (!SetEvent(t->hEvent)) { RHBOPT_ASSERT(!"event wrong"); }
#endif

			return 1;
		}
	}

	return 0;
}

static boolean RHBORB_begin_task(RHBORB *somThis,
								 struct RHBORB_thread_task *task)
{
	boolean started=0;

	RHBSOMD_must_be_guarded

	if (task)
	{
		started=RHBORB_queue_task(somThis,task);

		if (started)
		{
			task=NULL;
		}
		else
		{
			RHBORB_ThreadTask *t=SOMMalloc(sizeof(*t));

			t->task=task;
			t->orb_ptr=somThis;
			t->killer=NULL;
			t->server_request=NULL;

		#ifdef USE_PTHREADS
			pthread_cond_init(
						&t->pEvent,
						RHBOPT_pthread_condattr_default);
			if (!pthread_create(
						&t->pThread,
						RHBOPT_pthread_attr_default,
						RHBORB_ThreadTaskStarter,t))
			{
				started=1;
			}
			else
			{
				pthread_cond_destroy(&t->pEvent);
			}
		#else
			t->hEvent=CreateEvent(NULL,0,0,NULL);
			if (t->hEvent)
			{
				t->hThread=CreateThread(0,0,RHBORB_ThreadTaskStarter,t,0,&t->dwThread);

				if (t->hThread)
				{
					started=1;
				}
				else
				{
					CloseHandle(t->hEvent);
					t->hEvent=NULL;
				}
			}
		#endif

			if (started)
			{
				t->next=somThis->threads;
				somThis->threads=t;
				task=NULL;
			}
			else
			{
				SOMFree(t);
			}
		}

		if (task)
		{
			task->next=somThis->child.tasks;
			somThis->child.tasks=task;
		}
	}

	return started;
}
#endif

#ifdef USE_THREADS

/* this child thread is used to spawn further child threads,
   it is created when ORB is created, and dies when ORB is dead
   it means any further task thread is spawned in same
   environment as when the ORB was initialised rather than
   any ad-hoc environment
*/

static 
#if defined(USE_PTHREADS) 
void *
#else
DWORD __stdcall
#endif
rhborb_child(void *pv)
{
	RHBORB *somThis=pv;
#if (defined(USE_PTHREADS) ) && !defined(_PLATFORM_MACINTOSH_)
	sigset_t sigs;
	sigemptyset(&sigs);

	/* don't want any of these signals delivered in the 
		child threads */

	sigaddset(&sigs,SIGINT);
	sigaddset(&sigs,SIGTERM);
	sigaddset(&sigs,SIGQUIT);
	sigaddset(&sigs,SIGHUP);
	sigaddset(&sigs,SIGPIPE);

#ifdef USE_PTHREADS
	pthread_sigmask(SIG_BLOCK,&sigs,NULL);
#else
	pth_sigmask(SIG_BLOCK,&sigs,NULL);
#endif
#endif

	RHBORB_guard(somThis)

	while (somThis->child.running)
	{
		struct RHBORB_thread_task *task=NULL;
		do
		{
			task=RHBORB_pop_task(somThis);

			if (task)
			{
				if (!RHBORB_begin_task(somThis,task))
				{
					break;
				}
			}

		} while (task);

		if (somThis->child.dead_list)
		{
			RHBORB_join(somThis);
		}
		else
		{
			if (task)
			{
				/* if there was a task we could not start then
					sleep for a while and reattempt */

	#if defined(USE_PTHREADS) 
				somd_timed_wait(&somThis->child.pEvent,
					10,
					__FILE__,__LINE__);
	#else
				somd_timed_wait(somThis->child.hEvent,10,__FILE__,__LINE__);
	#endif
			}
			else
			{
#if defined(USE_PTHREADS)
				somd_wait(&somThis->child.pEvent,__FILE__,__LINE__);
#else
				somd_wait(somThis->child.hEvent,__FILE__,__LINE__);
#endif
			}
		}
	}

	RHBORB_join(somThis);

	RHBORB_unguard(somThis)

	return 0;
}
#endif

/* This contains the 'ORB' class */

static void rhbContained_delete(RHBContained *somThis)
{
	Environment ev;
	SOM_InitEnvironment(&ev);

	if (somThis->orb)
	{
		RHBCDR_kds_remove(&somThis->orb->contained_by_abs,&somThis->key);
	}

	somThis->orb=NULL;

	switch (somThis->describeFlags)
	{
	case ORB_DESCRIBE_CONST:
		RHBOPT_ASSERT(!somThis->desc_value._value);
		break;
	case ORB_DESCRIBE_SOMIR:
		RHBOPT_ASSERT(somThis->desc_value._type);
		RHBOPT_ASSERT(somThis->desc_value._value);
		SOMFree(somThis->desc_value._value);
		break;
	case ORB_DESCRIBE_FREE:
		SOMD_FreeType(&ev,&somThis->desc_value,TC_any);
		break;
	}

	switch (somThis->modifierFlags)
	{
	case ORB_DESCRIBE_CONST:
		break;
	case ORB_DESCRIBE_SOMIR:
		RHBOPT_ASSERT(!somThis->modifiers._length);
		break;
	case ORB_DESCRIBE_FREE:
		SOMD_FreeType(&ev,&somThis->modifiers,somdTC_sequence_somModifier);
		break;
	}

	SOM_UninitEnvironment(&ev);

	RHBDebug_deleted(RHBContained,somThis)

	if (somThis->id)
	{
		SOMFree(somThis->id);
	}

	SOMFree(somThis);
}


static void rhbContained_Release(RHBContained *somThis)
{
	somd_atomic_dec(&somThis->lUsage);
}

static void rhbContained_AddRef(RHBContained *somThis)
{
	somd_atomic_inc(&somThis->lUsage);
}

static const char * get_modifier(_IDL_SEQUENCE_somModifier *mods,const char *name)
{
	unsigned long i=mods->_length;
	somModifier *b=mods->_buffer;
	while (i--)
	{
		if (!strcmp(b->name,name))
		{
			return b->value;
		}

		b++;
	}
	return NULL;
}

static const char *get_modifier_long(_IDL_SEQUENCE_somModifier *mods,const char *name,long *pv)
{
	const char *p=get_modifier(mods,name);
	if (p && pv) { *pv=atol(p); }
	return p;
}

static char * rhbContained_get_id(RHBContained *somThis,Environment *ev)
{
	char *id=somThis->id;

	RHBOPT_ASSERT(ev);
	RHBOPT_ASSERT(somThis);
	RHBOPT_ASSERT(ev->_major==NO_EXCEPTION);

	if (!id)
	{
		any a=RHBCDR_get_named_element(ev,&somThis->desc_value,"id");

		if (ev->_major==NO_EXCEPTION)
		{
			id=RHBCDR_cast_any_to_string(ev,&a);

			if (id && (id[0]==':') && (id[1]==':'))
			{
				long majorVersion=0,minorVersion=0;

				if (get_modifier_long(&somThis->modifiers,"majorversion",&majorVersion) &&
					get_modifier_long(&somThis->modifiers,"minorversion",&minorVersion))
				{
					char tail[32];
					int tailLen=1
#ifdef HAVE_SNPRINTF
					+snprintf(tail,sizeof(tail),":%ld.%ld",majorVersion,minorVersion);
#else
					;
					sprintf(tail,":%ld.%ld",majorVersion,minorVersion);
					tailLen+=strlen(tail);
#endif

					RHBORB_guard(somThis->orb);

					if (!somThis->id)
					{
						const char *q=id;
						int i=(int)strlen(id);
						char *id=SOMMalloc(tailLen+i+4);
						char *p=id;

						while (*q==':') 
						{ 
							q++; 
						}

						memcpy(p,"IDL:",4); p+=4;

						while (*q)
						{
							if (*q==':')
							{
								while (*q==':') 
								{ 
									q++; 
								}
								*p++='/';
							}
							else
							{
								while (*q && (*q!=':'))
								{
									*p++=*q++;
								}
							}
						}

						memcpy(p,tail,tailLen);

						somThis->id=id;
					}

					id=somThis->id;

					RHBORB_unguard(somThis->orb);
				}
			}
		}
	}

	return id;
}

RHBOPT_cleanup_begin(RHBORB_string_to_object_cleanup,pv)

	_IDL_SEQUENCE_octet *data=pv;
	if (data->_buffer) SOMFree(data->_buffer);

RHBOPT_cleanup_end

static SOMDObject SOMSTAR rhbORB_string_to_object(
		RHBORB *somThis,
		Environment *ev,
		corbastring str)
{
	if (str && !ev->_major)
	{
		long l=(long)strlen(str);

		if ((l > 4) && !memcmp(str,"IOR:",4))
		{
			SOMDObject SOMSTAR RHBOPT_volatile obj=NULL;
			_IDL_SEQUENCE_octet seq=RHBORB_read_hex(somThis,str+4);

			RHBOPT_cleanup_push(RHBORB_string_to_object_cleanup,&seq);

			if (seq._length)
			{
				any d=IOP_Codec_decode_value(somThis->iop_codec,ev,&seq,TC_Object);

				if (ev->_major==NO_EXCEPTION)
				{
					TCKind kind=tk_null;

					RHBOPT_ASSERT(d._type);

					if (d._type)
					{
						kind=TypeCode_kind(d._type,ev);

						TypeCode_free(d._type,ev);

						RHBOPT_ASSERT(kind==tk_objref);
					}

					RHBOPT_ASSERT(d._value);

					if (d._value) 
					{
						if (kind==tk_objref)
						{
							obj=*((SOMObject SOMSTAR *)d._value);
						}

						SOMFree(d._value);
					}

				}
			}

			RHBOPT_cleanup_pop();

			return obj;
		}
	}

	RHBOPT_throw_StExcep(ev,BAD_PARAM,BadParm,NO);

	return NULL;
}

struct RHBORB_get_object_from_ior
{
	RHBGIOPRequestStream *impl;
	RHBInterfaceDef *iface;
	any n;
};

RHBOPT_cleanup_begin(RHBORB_get_object_from_ior_cleanup,pv)

struct RHBORB_get_object_from_ior *data=pv;
RHBGIOPRequestStream *impl=data->impl;
RHBInterfaceDef *iface=data->iface;
Environment ev;

	SOM_InitEnvironment(&ev);

	data->impl=NULL;
	data->iface=NULL;

	SOMD_FreeType(&ev,&data->n,TC_any);

	if (impl) RHBImplementationDef_Release(impl);
	if (iface) RHBInterfaceDef_Release(iface);

	SOM_UninitEnvironment(&ev);

RHBOPT_cleanup_end

static SOMDObject SOMSTAR rhbORB_get_object_from_ior(
		RHBORB *somThis,
		Environment *ev,
		IOP_IOR *ior,
		RHBImplementationDef *suggestion_arg,
		SOMClass SOMSTAR cls_if_server_arg,
		TypeCode tc)
{
	RHBImplementationDef * RHBOPT_volatile suggestion=suggestion_arg;
	SOMClass SOMSTAR RHBOPT_volatile cls_if_server=cls_if_server_arg;
	unsigned int RHBOPT_volatile i=0;
	SOMDObject SOMSTAR RHBOPT_volatile obj=NULL;
	struct RHBORB_get_object_from_ior data={NULL,NULL,{NULL,NULL}};

	RHBOPT_cleanup_push(RHBORB_get_object_from_ior_cleanup,&data);

	while ((i < ior->profiles._length) 
			&& (!ev->_major)
			&& (!obj))
	{
		if (ior->profiles._buffer[i].tag==IOP_TAG_INTERNET_IOP)
		{
			data.n=IOP_Codec_decode_value(
					somThis->iop_codec,ev,
					&ior->profiles._buffer[i].profile_data,
					somdTC_IIOP_ProfileBody_1_0);

			if (!ev->_major)
			{
				IIOP_ProfileBody_1_0 *body=data.n._value;

				if (!body->port)
				{
					body->port=DEFAULT_IIOP_PORT;
					debug_somPrintf(("using default port of %d\n",(int)body->port));
	/*				bomb("why is this happening?");*/
				}

				if (suggestion)
				{
					if (suggestion->server.data)
					{
						if (suggestion->server.data->listener)
						{
							suggestion=suggestion->server.data->listener;
						}
					}

					if (!suggestion->deleting)
					{
						if (suggestion->address.port==body->port)
						{
							if (0==strcmp(body->host,suggestion->address.host))
							{
								RHBImplementationDef_AddRef(suggestion);

								data.impl=suggestion;

								if (!cls_if_server)
								{
									if (suggestion->connection.is_listener)
									{
										cls_if_server=somThis->somClass_SOMDObject_ref;
									}
								}
							}
						}
					}
				}

				if (!data.impl)
				{
					data.impl=RHBORB_get_impl(somThis,ev,body);
				}

				data.iface=RHBORB_get_interface(somThis,ev,ior->type_id);

				i=ior->profiles._length;

				obj=RHBORB_get_object(somThis,
						ev,
						cls_if_server,
						data.impl,
						data.iface,
						&(body->object_key),
						tc);
			}

			RHBOPT_cleanup_call(RHBORB_get_object_from_ior_cleanup,&data);

			if (obj)
			{
				if (ev->_major)
				{
					SOMD_bomb("Has object and exception??...\n");
				}

				break;
			}
		}

		i++;
	}

	RHBOPT_cleanup_pop();

	if (ev->_major) obj=NULL;

	return obj;
}

static char *RHBORB_create_proxy_className(char *className)
{
	char *buf=NULL;
	if (className)
	{
		size_t len=strlen(className);
		buf=SOMMalloc(len+8);
		if (buf)
		{
			memcpy(buf,className,len);
			memcpy(buf+len,"__Proxy",8);
		}
	}
	return buf;
}

struct RHBORB_get_proxy_class
{
	SOMClass SOMSTAR actual;
	SOMClassMgr SOMSTAR clsMgr;
	somId id;
	char *proxy_className;
	char *className;
};

RHBOPT_cleanup_begin(RHBORB_get_proxy_class_cleanup,pv)

	struct RHBORB_get_proxy_class *data=pv;

	if (data->actual) somReleaseClassReference(data->actual);
	if (data->clsMgr) somReleaseClassManagerReference(data->clsMgr);
	if (data->id) SOMFree(data->id);
	if (data->proxy_className) SOMFree(data->proxy_className);
	if (data->className) SOMFree(data->className);

RHBOPT_cleanup_end

static SOMClass SOMSTAR RHBORB_get_proxy_class_from_TypeCode(
		RHBORB *somThis,
		Environment *ev,
		TypeCode tc)
{
	SOMClass SOMSTAR RHBOPT_volatile proxy=NULL;

	if (TypeCode_equal(tc,ev,TC_Object))
	{
		proxy=SOMClass_somDuplicateReference(somThis->somClass_SOMDClientProxy_ref);
	}
	else
	{
		if (TypeCode_kind(tc,ev)==tk_objref)
		{
			any a=TypeCode_parameter(tc,ev,0);
			char *p=RHBCDR_cast_any_to_string(ev,&a);

			if (p)
			{
				struct RHBORB_get_proxy_class data={NULL,NULL,NULL,NULL,NULL};
				
				RHBOPT_cleanup_push(RHBORB_get_proxy_class_cleanup,&data);

				data.className=RHBORB_convert_interface_to_class(somThis,p);

				if (data.className)
				{				
					data.proxy_className=RHBORB_create_proxy_className(data.className);

					data.clsMgr=somGetClassManagerReference();

					data.id=somIdFromString(data.proxy_className);

					proxy=SOMClassMgr_somClassFromId(data.clsMgr,data.id);

					if (!proxy) 
					{
						SOMFree(data.id);

						data.id=somIdFromString(data.className);

						data.actual=SOMClassMgr_somFindClass(data.clsMgr,data.id,0,0 /*major,minor*/);

						if (data.actual)
						{
							if (SOMClass_somDescendedFrom(
								somThis->somClass_SOMDClientProxy_ref,
								data.actual))
							{
								proxy=SOMClass_somDuplicateReference(somThis->somClass_SOMDClientProxy_ref);
							}
							else
							{
								proxy=RHBORB_create_proxy_class(
									somThis,
									ev,
									data.clsMgr,
									somThis->somClass_SOMDClientProxy_ref,
									data.actual);
							}
						}
					}
				}

				RHBOPT_cleanup_pop();
			}
		}
	}

	return proxy;
}

struct RHBORB_get_object
{
	SOMClass SOMSTAR cls;
	boolean release;
};

RHBOPT_cleanup_begin(RHBORB_get_object_cleanup,pv)

	struct RHBORB_get_object *data=pv;

	if (data->cls && data->release)
	{
		somReleaseClassReference(data->cls);
	}

RHBOPT_cleanup_end

static SOMDObject SOMSTAR rhbORB_get_object(
				RHBORB *somThis,
				Environment *ev,
				SOMClass SOMSTAR cls,
				RHBImplementationDef *imp,
				RHBInterfaceDef *iface_def,
				_IDL_SEQUENCE_octet *key,
				TypeCode tc)
{
	SOMDObject SOMSTAR RHBOPT_volatile somobj=NULL;
	RHBObject *rhbobj=NULL;

	if (!ev->_major)
	{
		struct RHBORB_get_object data={NULL,0};

		RHBOPT_cleanup_push(RHBORB_get_object_cleanup,&data);

		data.cls=cls;

		if (!data.cls)
		{
			data.cls=RHBInterfaceDef_get_proxy_class(iface_def,ev);

			if (!data.cls)
			{
				somExceptionFree(ev);

				data.cls=RHBORB_get_proxy_class_from_TypeCode(somThis,ev,tc);

				if (!data.cls)
				{
					somExceptionFree(ev);

					data.cls=SOMClass_somDuplicateReference(somThis->somClass_SOMDClientProxy_ref);
				}
			}
		}

		if (ev->_major) somExceptionFree(ev);

		data.release=1;

#ifdef _DEBUG
		if (!SOMClass_somDescendedFrom(data.cls,
				somThis->somClass_SOMDObject_ref))
		{
			SOMD_bomb("RHB_get_object - class object failed");
		}
#endif

		somobj=SOMClass_somNew(data.cls);
		rhbobj=SOMDObject__get_c_proxy(somobj,ev);
		RHBObject_somInit(rhbobj,iface_def,imp,key);

		RHBOPT_cleanup_pop();
	}

	return somobj;
}

static RHBImplementationDef * rhbORB_get_impl(
		RHBORB *somThis,
		Environment *ev,
		IIOP_ProfileBody_1_0 *iop)
{
	RHBImplementationDef *def;

	if (ev->_major) return 0;

	if (!iop->port)
	{
		/* always return new one if zero 'port' */
		return RHBGIOPRequestStreamNew(somThis,iop);
	}

	if (somThis->impls)
	{
		RHBImplementationDef *i;

		RHBORB_guard(somThis)

		i=somThis->impls;

		while (i)
		{
			if (!i->deleting)
			{
				if (!i->is_closing)
				{
					if (i->lUsage)
					{
				/*		if (!i->server.accepted_from)*/
						{
							/* comment out the following line to enable 
								local access to local server object */
					/*		if (!i->connection.is_listener) */
							{
								if (iop->port==i->address.port)
								{
									if (!strcmp(iop->host,i->address.host))
									{
										if (i->server.data)
										{
											if (i->server.data->listener)
											{
												i=i->server.data->listener;
											}
										}

										RHBImplementationDef_AddRef(i);
	
										RHBORB_unguard(somThis)

										return i;
									}
								}
							}
						}
					}
				}
			}
			i=i->next;
		}

		RHBORB_unguard(somThis)
	}

	def=RHBGIOPRequestStreamNew(somThis,iop);

	return def;
}

static RHBInterfaceDef * rhbORB_get_interface(
	RHBORB *somThis,
	Environment *ev,corbastring s)
{
	RHBInterfaceDef *def;

	if (ev->_major) return 0;

	if (!s) return 0;
	if (!s[0]) return 0;

	RHBORB_guard(somThis)

	def=somThis->iface;

	while (def)
	{
		if (RHBInterfaceDef__get_usage(def))
		{
			if (0==strcmp(def->id,s))
			{
				break;
			}
		}

		def=def->next;
	}

	if (def)
	{
		RHBInterfaceDef_AddRef(def);
	}
	else
	{
		def=RHBInterfaceDefNew(somThis,s,1);
	}

	RHBORB_unguard(somThis)

	return def;
}

static char hex_char_list_upper[16]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
static char hex_char_list_lower[16]={'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f'};

static octet hex_nybble(char c)
{
	octet i=0;

	while (i < 16)
	{
		if (c==hex_char_list_upper[i]) return i;
		if (c==hex_char_list_lower[i]) return i;

		i++;
	}

	return 0;
}

static char rhbORB_hex_char(RHBORB *somThis,octet c)
{
	return hex_char_list_lower[c & 0xf];
}

static void rhbORB_dump_sequence(RHBORB *somThis,_IDL_SEQUENCE_octet *st)
{
	octet *p=st->_buffer;
	unsigned long l=st->_length;
	while (l--)
	{
		dump_somPrintf(("%c",(char)RHBORB_hex_char(somThis,(char)((*p >> 4)))));
		dump_somPrintf(("%c",(char)RHBORB_hex_char(somThis,(char)(*p))));
		p++;
	}
}


static _IDL_SEQUENCE_octet rhbORB_read_hex(RHBORB *somThis,char *p)
{
	_IDL_SEQUENCE_octet seq={0,0,NULL};
	RHBOPT_unused(somThis)
	if (p)
	{
		long l=(long)strlen(p)>>1;
		if (l)
		{
			octet *q=seq._buffer=SOMMalloc(l);
			seq._maximum=l;

			while (l-- && *p)
			{
				octet w;

				w=hex_nybble(*p++);
				w<<=4;
				w|=hex_nybble(*p++);

				*q++=w;
			}

			seq._length=(long)(q-seq._buffer);
		}
	}

	return seq;
}

static void rhbORB_AddRef(RHBORB *somThis)
{
	somd_atomic_inc(&somThis->lUsage);
}

static void rhbORB_Release(RHBORB *somThis)
{
	if (!somd_atomic_dec(&somThis->lUsage)) 
	{
		if (!somThis->closing)
		{
			Environment ev;
			SOM_InitEnvironment(&ev);
			RHBORB_Close(somThis,&ev);
			SOM_UninitEnvironment(&ev);
		}
	}
}

static void RHBORB_delete(RHBORB *somThis)
{
	struct somIdMap *idp=orbIdMap;
	int i=sizeof(orbIdMap)/sizeof(orbIdMap[0]);
#ifdef USE_THREADS
	boolean doJoin=0;
#endif
	RHBProtocol *protocol=somThis->protocol;
	somThis->protocol=NULL;

	if (protocol)
	{
		RHBProtocol_Release(protocol);
	}

	RHBORB_guard(somThis)

	if (somThis->globals)
	{
		if (somThis->globals->orb==somThis)
		{
			somThis->globals->orb=NULL;
		}
	}

#ifdef USE_THREADS
	#ifdef USE_PTHREADS
		pthread_cond_signal(&somThis->globals->p_event);
	#else
		if (somThis->globals && somThis->globals->h_event)
		{
			if (!SetEvent(somThis->globals->h_event)) { RHBOPT_ASSERT(!"event wrong"); }
		}
	#endif
	if (somThis->child.running)
	{
		somThis->child.running=0;
#ifdef USE_PTHREADS
		pthread_cond_signal(&somThis->child.pEvent);
#else
		if (!SetEvent(somThis->child.hEvent)) { RHBOPT_ASSERT(!"event wrong"); }
#endif
		doJoin=1;
	}
#endif

	RHBORB_unguard(somThis)

	RHBDebug_deleted(RHBORB,somThis)

#ifdef USE_THREADS
	if (doJoin)
	{
	#ifdef USE_PTHREADS
		void *pv=NULL;
		pthread_join(somThis->child.pThread,&pv);
		pthread_cond_destroy(&somThis->child.pEvent);
	#else
		WaitForSingleObject(somThis->child.hThread,INFINITE);
		CloseHandle(somThis->child.hThread);
		CloseHandle(somThis->child.hEvent);
	#endif
	}
#endif

	while (i--)
	{
		somId id=somIdMapId(somThis,idp->offset);
		somIdMapId(somThis,idp->offset)=NULL;
		if (id) SOMFree(id);
		idp++;
	}

	SOMFree(somThis);
}

static void rhbORB_shutdown(
		RHBORB *somThis,
		Environment *ev)
{
	if (somThis->closing) return;

	RHBORB_guard(somThis)

	if (somThis->impls)
	{
		RHBImplementationDef *i=somThis->impls;

		while (i)
		{
			if (
				(i->connection.connected)||
				(i->connection.connecting)||
				(i->connection.fd))
			{
				RHBImplementationDef_AddRef(i);

#ifdef _DEBUG
/*				somPrintf("Tearing down %p(%s:%d) on RHBORB_shutdown(usage=%ld)\n",
						i,i->address.host,(int)i->address.port,i->lUsage);
				if (i->connection.connected) somPrintf("it's still connected\n");
				if (i->connection.connecting) somPrintf("it's still connecting\n");
				if (i->connection.fd) somPrintf("it still has an 'fd'\n");*/
#endif

				RHBORB_unguard(somThis)

				if (ev->_major) somExceptionFree(ev);

				RHBImplementationDef_interrupt_closed(i,ev);

				if (ev->_major) somExceptionFree(ev);

				RHBImplementationDef_Release(i);

				RHBORB_guard(somThis)

				i=somThis->impls;
			}
			else
			{
				i=i->next;
			}
		}
	}

	RHBORB_unguard(somThis)
}

static void rhbORB_Close(
	RHBORB *somThis,Environment *ev)
{
	SOMObject SOMSTAR ref;

	if (somThis->closing) return;

	RHBORB_guard(somThis)

	if (somThis->closing) 
	{
		RHBORB_unguard(somThis)

		return;
	}

	somThis->closing=1;

	RHBORB_unguard(somThis)

	RHBORB_shutdown(somThis,ev);

#ifdef USE_THREADS
	RHBSOM_Trace("Closing threads")
	RHBORB_CloseThreads(somThis);
#endif

	if (somThis->protocol) 			
	{
		RHBSOM_Trace("Closing protocol")
		RHBProtocol_Close(somThis->protocol);
	}

	RHBSOM_Trace("looking to release repository reference")

	RHBSOM_Trace("checking Principal")

	RHBORB_guard(somThis)

	ref=somThis->principal;
	somThis->principal=NULL;

	if (ref)
	{
		RHBSOM_Trace("clearing up Principal")

		Principal_somRelease(ref);
	}

	RHBSOM_Trace("checking implementations")

	if (somThis->impls)
	{
		RHBSOM_Trace("Impls left over...")

		if (somThis->impls)
		{
			RHBImplementationDef *i;

			RHBSOM_Trace("Impls are left over")

			i=somThis->impls;

			while (i)
			{
				if (i->lUsage)
				{
					RHBImplementationDef_debug_printf(i,ev);

					i=i->next;
				}
				else
				{
					RHBImplementationDef_AddRef(i);

/*					somPrintf("closing a left over impl(%p) with lUsage==0\n",i);*/

					RHBORB_unguard(somThis)

					RHBImplementationDef_Release(i);

					RHBORB_guard(somThis)

					i=somThis->impls;
				}
			}
		}
	}

/*	somPrintf("clearing up operations");*/

	while (RHBCDR_kds_count(&somThis->contained_by_abs))
	{
		RHBSOMUT_KeyDataPtr ptr=RHBCDR_kds_get(&somThis->contained_by_abs,0);
		if (ptr)
		{
			rhbContained_delete(ptr->data._value);
		}
	}

	RHBSOM_Trace("finished clearing up operations")

#define ZAP_OBJECT(x)  if (x) { SOMObject SOMSTAR y=x; x=NULL; if (y) { somReleaseObjectReference(y); } }
	ZAP_OBJECT(somThis->iop_codec);
	ZAP_OBJECT(somThis->iop_codecFactory);
#undef ZAP_OBJECT

#define ZAP_CLASS(x)   if (x) { SOMClass SOMSTAR y=x; x=NULL; if (y) { somReleaseClassReference(y); } }
	ZAP_CLASS(somThis->somClass_ImplementationDef_ref);
	ZAP_CLASS(somThis->somClass_Request_ref);
	ZAP_CLASS(somThis->somClass_SOMDObject_ref);
	ZAP_CLASS(somThis->somClass_SOMOA_ref);
	ZAP_CLASS(somThis->somClass_SOMDClientProxy_ref);
	ZAP_CLASS(somThis->somClass_SOMDServer_ref);
	ZAP_CLASS(somThis->somClass_Context_ref);
	ZAP_CLASS(somThis->somClass_Principal_ref);
	ZAP_CLASS(somThis->somClass_NVList_ref);
	ZAP_CLASS(somThis->somClass_somStream_MemoryStreamIO_ref);
	ZAP_CLASS(somThis->somClass_PortableServer_DynamicImpl_ref);
	ZAP_CLASS(somThis->somClass_SOMDServerStub_ref);
	ZAP_CLASS(somThis->somClass_ServerRequest_ref);
#undef ZAP_CLASS


	RHBSOM_Trace("finished zapping classes");

	if (somThis->iface)
	{
		RHBInterfaceDef *i;
		dump_somPrintf(("Interfaces still left over\n"));

		i=somThis->iface;

		while (i)
		{
			dump_somPrintf(("%s\n",i->id));
			i=i->next;
		}
	}

/*	if (somThis->StExcep_tc)
	{
		TypeCode_free(somThis->StExcep_tc,ev);
		somThis->StExcep_tc=0;
	}
*/
	RHBSOM_Trace("finished closing");

	if (somThis->lUsage)
	{
		somThis->closing=0;

		RHBORB_unguard(somThis)
	}
	else
	{
		RHBSOM_Trace("calling RHBORB_delete");
		RHBORB_unguard(somThis)
		RHBORB_delete(somThis);
	}
}

static corbastring rhbORB_object_to_string(
		RHBORB *somThis,
		Environment *ev,SOMDObject SOMSTAR object)
{
	if (ev->_major) return NULL;

	if (object)
	{
		_IDL_SEQUENCE_octet e={0,0,NULL};
		char *p=NULL;
		any value;

		value._type=TC_Object;
		value._value=&object;
		e=IOP_Codec_encode_value(somThis->iop_codec,ev,&value);

		if (!ev->_major)
		{
			if (e._buffer)
			{
				unsigned int i=0;
				char *q=p=(corbastring)SOMMalloc((e._length << 1)+5);

				*q++='I';
				*q++='O';
				*q++='R';
				*q++=':';

				while (i < e._length)
				{
					char c=(char)(e._buffer[i] >> 4);

					*q++=RHBORB_hex_char(somThis,c);

					c=(char)(e._buffer[i] & 0xf);

					*q++=RHBORB_hex_char(somThis,c);
					i++;
				}

				SOMFree(e._buffer);

				*q=0;
			}

			if (!p)
			{
				RHBOPT_throw_StExcep(ev,INV_OBJREF,UnexpectedNULL,MAYBE);
			}
		}

		return p;
	}

	return NULL;
}

static void rhbORB_add_impl(RHBORB *somThis,RHBImplementationDef *im)
{
	if (im)
	{
		RHBORB_guard(somThis)

		im->next=somThis->impls;

		somThis->impls=im;

		RHBORB_unguard(somThis)
	}
}

static void rhbORB_remove_impl(
		RHBORB *somThis,
		RHBImplementationDef *im)
{
	if (im)
	{
		RHBORB_guard(somThis)

		if (somThis->impls==im)
		{
			somThis->impls=im->next;
		}
		else
		{
			RHBImplementationDef *s=somThis->impls;

			if (s)
			{
				while (s->next != im)
				{
					s=s->next;

					if (!s) break;
				}

				if (s)
				{
					s->next=im->next;
				}
				else
				{
					debug_somPrintf(("Implementation not listed\n"));
				}
			}
		}

		RHBORB_unguard(somThis)
	}
}

static void rhbORB_add_interface(
		RHBORB *somThis,
		RHBInterfaceDef *ifnew,
		boolean orb_locked)
{
	if (ifnew)
	{
#ifdef USE_THREADS
		if (!orb_locked)
		{
			RHBORB_guard(somThis)
		}
#endif
		ifnew->next=somThis->iface;
		somThis->iface=ifnew;

#ifdef USE_THREADS
		if (!orb_locked)
		{
			RHBORB_unguard(somThis)
		}
#endif
	}
}

static void rhbORB_remove_interface(
		RHBORB *somThis,
		RHBInterfaceDef *ifold,
		boolean orb_locked)
{
	if (ifold)
	{
		RHBInterfaceDef *o;

#ifdef USE_THREADS
		if (!orb_locked)
		{
			RHBORB_guard(somThis)
		}
#endif
		o=somThis->iface;

		if (o==ifold)
		{
			somThis->iface=ifold->next;
		}
		else
		{
			while (o)
			{
				if (o->next == ifold)
				{
					break;
				}
				o=o->next;
			}

			if (o) o->next=ifold->next;
		}

#ifdef USE_THREADS
		if (!orb_locked)
		{
			RHBORB_unguard(somThis)
		}
#endif
	}
}

struct RHBORB_acquire_interface_repository
{
	Repository SOMSTAR rep;
	SOMClassMgr SOMSTAR mgr;
	SOMClass SOMSTAR cls;
};

RHBOPT_cleanup_begin(RHBORB_acquire_interface_repository_cleanup,pv)

	struct RHBORB_acquire_interface_repository *data=pv;

	if (data->rep)
	{
		somReleaseObjectReference(data->rep);
	}

	if (data->cls)
	{
		somReleaseClassReference(data->cls);
	}

	if (data->mgr)
	{
		somReleaseClassManagerReference(data->mgr);
	}

RHBOPT_cleanup_end

static Repository SOMSTAR rhbORB_acquire_interface_repository(
			RHBORB *somThis,
			Environment *ev,char *why)
{
	SOMObject SOMSTAR RHBOPT_volatile result=NULL;

	if (!(ev->_major))
	{
		struct RHBORB_acquire_interface_repository data={NULL,NULL,NULL};

		RHBOPT_cleanup_push(RHBORB_acquire_interface_repository_cleanup,&data);

		data.mgr=somGetClassManagerReference();

		if (data.mgr)
		{
			data.rep=SOMClassMgr__get_somInterfaceRepository(data.mgr);

#	if defined(_PLATFORM_MACINTOSH_)||defined(_WIN32)
			if (!data.rep)
			{
				data.cls=SOMClassMgr_somFindClass(data.mgr,
					somThis->somIdRepository,0,0);

				if (!data.cls)
				{
					data.cls=SOMClassMgr_somFindClsInFile(data.mgr,
						somThis->somIdRepository,0,0,"SOMIR");
				}

				if (data.cls)
				{
					data.rep=SOMClass_somNew(data.cls);

					if (data.rep)
					{
						SOMClassMgr__set_somInterfaceRepository(data.mgr,data.rep);
					}
				}
			}
#	endif
		}

		if (!(ev->_major))
		{
			if (data.rep)
			{
				result=data.rep;

				data.rep=NULL;
			}
			else
			{
				RHBOPT_throw_StExcep(ev,INTF_REPOS,ClassNotFound,NO);
			}
		}

		RHBOPT_cleanup_pop();
	}

	return result;
}

struct RHBGIOPORB_create_proxy_class
{
	RHBORB *orb;
	somId id;
	char *name;
	boolean locked;
};

RHBOPT_cleanup_begin(RHBGIOPORB_create_proxy_class_cleanup,pv)

	struct RHBGIOPORB_create_proxy_class *data=pv;

	if (data->id) SOMFree(data->id);
	if (data->name) SOMFree(data->name);

	if (data->locked)
	{
		RHBORB_unguard(data->orb);
	}

RHBOPT_cleanup_end

static SOMClass SOMSTAR rhbORB_create_proxy_class(
				RHBORB *orb,
				Environment *ev,
				SOMClassMgr SOMSTAR clsMgr,
				SOMClass SOMSTAR proxy_class,
				SOMClass SOMSTAR local_class)
{
	SOMClass SOMSTAR RHBOPT_volatile newClass=NULL;

	if (clsMgr && local_class && clsMgr && orb)
	{
		struct RHBGIOPORB_create_proxy_class data={NULL,NULL,NULL,0};

		data.orb=orb;
		RHBOPT_cleanup_push(RHBGIOPORB_create_proxy_class_cleanup,&data);

		RHBOPT_unused(ev)

		data.name=RHBORB_create_proxy_className(SOMClass_somGetName(local_class));
		data.id=somIdFromString(data.name);

		RHBORB_guard(orb)

		data.locked=1;

		newClass=SOMClassMgr_somClassFromId(clsMgr,data.id);

		if (!newClass)
		{
			newClass=SOMMProxyFor_sommMakeProxyClass(proxy_class,local_class,data.name);
		}

		RHBOPT_cleanup_pop();
	}

	return newClass;
}


static SOMClass SOMSTAR rhbORB_get_proxy_class(
			RHBORB *somThis,
			Environment *ev,
			char *name_arg)
{
	char * RHBOPT_volatile name=name_arg;
	SOMClass SOMSTAR RHBOPT_volatile proxy=NULL;

	if (name)
	{
		if (name[0])
		{
			struct RHBORB_get_proxy_class data={NULL,NULL,NULL,NULL,NULL};

			RHBOPT_cleanup_push(RHBORB_get_proxy_class_cleanup,&data);

			if ( ( name[0] != ':' ) || ( name[1] != ':' ) )
			{
				if (strlen(name) > 4)
				{
					if (!memcmp(name,"IDL:",4))
					{
						if (name[4] != ':')
						{
							SOMD_bomb("this should have been converted to a class name");
						}
					}
				}
			}
			else
			{
				name+=2;
			}

			data.id=somIdFromString(name);

			data.clsMgr=somGetClassManagerReference();

	/* first look in class using class by ID,
		if this fails check we have loaded the IR */
	
			data.actual=SOMClassMgr_somClassFromId(data.clsMgr,data.id);

			if (!data.actual)
			{
				data.actual=SOMClassMgr_somFindClass(data.clsMgr,data.id,0,0);
			}

			if (data.actual)
			{
				if (somThis->somClass_SOMDClientProxy_ref==data.actual)
				{
					proxy=data.actual;
					data.actual=NULL;
				}
				else
				{
					if (SOMClass_somDescendedFrom(somThis->somClass_SOMDClientProxy_ref,data.actual))
					{
						proxy=SOMClass_somDuplicateReference(somThis->somClass_SOMDClientProxy_ref);
					}
				}
			}

			if (data.actual && !proxy)
			{
				if (data.id) 
				{
					SOMFree(data.id);
					data.id=NULL;
				}

				if (!data.id)
				{
					char *buf=RHBORB_create_proxy_className(SOMClass_somGetName(data.actual));
					if (buf)
					{
						data.id=somIdFromString(buf);
						SOMFree(buf);
					}
				}

				if (data.id) proxy=SOMClassMgr_somClassFromId(data.clsMgr,data.id);
			}
			
			if (data.actual && !proxy)
			{
				/* so now need to create the proxy class */

				proxy=RHBORB_create_proxy_class(
					somThis,
					ev,
					data.clsMgr,
					somThis->somClass_SOMDClientProxy_ref,
					data.actual);
			}

			RHBOPT_cleanup_pop();
		}
	}

	return proxy;
}

struct RHBORB_convert_interface_to_class
{
	RHBInterfaceDef *def;
};

RHBOPT_cleanup_begin(RHBORB_convert_interface_to_class_cleanup,pv)

struct RHBORB_convert_interface_to_class *data=pv;

	if (data->def) RHBInterfaceDef_Release(data->def);

RHBOPT_cleanup_end

static char * rhbORB_convert_interface_to_class(RHBORB *somThis,char *interface_name)
{
	char * RHBOPT_volatile className=NULL;

	if (interface_name)
	{
		if (interface_name[0])
		{
			if ((interface_name[0]==':')&&(interface_name[1]==':'))
			{
				className=somd_dupl_string(interface_name+2);
			}
			else
			{
				struct RHBORB_convert_interface_to_class data={NULL};
				Environment ev;

				RHBOPT_cleanup_push(RHBORB_convert_interface_to_class_cleanup,&data);

				SOM_InitEnvironment(&ev);

				data.def=RHBORB_get_interface(somThis,&ev,interface_name);

				if (data.def)
				{
					char *p=RHBInterfaceDef_get_absolute_name(data.def,&ev);

					if (p) 
					{
						if (*p)
						{
							if (p[0]==':')
							{
								if (p[1]==':')
								{
									p+=2;
								}
							}

							className=somd_dupl_string(p);
						}
					}
				}

				SOM_UninitEnvironment(&ev);

				RHBOPT_cleanup_pop();
			}
		}
	}

	return className;
}

static char * rhbORB_abs_to_id(RHBORB *somThis,const char *absId,long majorVersion,long minorVersion)
{
	char *idl_name=NULL;

#ifdef DEBUG_ABS_ID
	somPrintf("abs_to_id(\"%s\",%ld,%ld)=",absId,majorVersion,minorVersion);
#endif

	if (absId)
	{
		/* IDL:module/class:major.minor */

		char numBuf[22];
		size_t numLen;
		int doubleColon=0;
		const char *p;
		char *q;
		size_t absLen;

		while (absId[0]==':')
		{
			absId++;
		}

		p=absId;

		while (*p)
		{
			/* count pairs of colons */

			if (*p++==':')
			{
				RHBOPT_ASSERT(*p==':');

				while (*p==':')
				{
					doubleColon++;

					p++;
				}
			}
		}

		absLen=(p-absId);

#ifdef HAVE_SNPRINTF
		numLen=snprintf(numBuf,sizeof(numBuf),"%ld.%ld",majorVersion,minorVersion);
#else
		sprintf(numBuf,"%ld.%ld",majorVersion,minorVersion);
		numLen=strlen(numBuf);
#endif

		idl_name=SOMMalloc(6+absLen-doubleColon+numLen);

		memcpy(idl_name,"IDL:",4); /* 3=='IDL', 1=':', 1=':', 1='\000' */

		q=idl_name+4;

		while (absId[0])
		{
			size_t segLen=0;

			while (absId[segLen] && (absId[segLen]!=':'))
			{
				segLen++;
			}

			if (segLen)
			{
				memcpy(q,absId,segLen);
				q+=segLen;
				absId+=segLen;
			}
			
			while (absId[0]==':')
			{
				absId++;
			}

			if (segLen && absId[0])
			{
				*q++='/';
			}
		}

		*q++=':';

		memcpy(q,numBuf,numLen+1);
	}

#ifdef DEBUG_ABS_ID
	somPrintf("\"%s\"\n",idl_name);
#endif

	return idl_name;
}

static char * rhbORB_convert_class_to_interface(RHBORB *somThis,SOMClass SOMSTAR cls)
{
	char *idl_name=NULL;

	if (cls && somThis)
	{
		char *cls_name=SOMClass_somGetName(cls);

		if (cls_name)
		{
			long majorVersion=0,minorVersion=0;
			SOMClass_somGetVersionNumbers(cls,&majorVersion,&minorVersion);

			idl_name=RHBORB_abs_to_id(somThis,cls_name,majorVersion,minorVersion);
		}
	}

	return idl_name;
}

/* this is typically used with OperationDescription::defined_in */

static boolean rhbORB_is_oidl(RHBORB *somThis,char *name)
{
/*	Repository SOMSTAR rep=NULL; */
	Environment ev;
	RHBContained *c;

	RHBOPT_ASSERT(somThis);
	RHBOPT_ASSERT(name);

	if (name[0] != ':') 
	{
		debug_somPrintf(("RHBORB_is_oidl(%s)\n",name));
		SOMD_bomb("not id");
	}

	if (name[1] != ':') SOMD_bomb("not id");

	SOM_InitEnvironment(&ev);

	c=RHBORB_get_contained(somThis,&ev,name);

	if (c)
	{
		boolean b=0;
		unsigned int i=0;

		while (i < c->modifiers._length)
		{
			if (!strcmp(c->modifiers._buffer[i].name,"callstyle"))
			{
				if (!strcmp(c->modifiers._buffer[i].value,"oidl"))
				{
					b=1;
					break;
				}

				i=c->modifiers._length;
			}

			i++;
		}

		RHBContained_Release(c);

		return b;
	}

	if (ev._major) somExceptionFree(&ev);

	if (name)
	{
		if (name[0]==':')
		{
			if (name[1]==':')
			{
				name+=2;
			}
		}

		if (name[0] != 'S') return 0;
		if (name[1] != 'O') return 0;
		if (name[2] != 'M') return 0;

		if (strcmp(name,"SOMObject"))
			if (strcmp(name,"SOMClass"))
				if (strcmp(name,"SOMClassMgr"))
					if (strcmp(name,"SOMRefObject"))
		{
			return 0;
		}

		return 1;
	}

	return 0;
}

#ifdef USE_SELECT
SOMEXTERN RHBServerRequest * RHBORB_pop_first_queued_request(
		RHBORB *somThis)
{
	RHBImplementationDef *im=somThis->impls;
	RHBImplementationDef *p=NULL;

	while (im)
	{
		if (!im->active_server_request)
		{
			if (im->queued_request_list)
			{
				RHBServerRequest *n=im->queued_request_list;

				im->queued_request_list=n->qel.next;

				if (im->next)
				{
					/* put this implementation on the end now */

					if (p)
					{
						p->next=im->next;
						im->next=NULL;
					}
					else
					{
						somThis->impls=im->next;
						im->next=NULL;
						p=somThis->impls;
					}

					while (p->next)
					{
						p=p->next;
					}

					p->next=im;
				}

				return n;
			}
		}

		p=im;
		im=im->next;
	}

	return NULL;
}

void RHBORB_ProcessEvents(RHBORB *somThis,Environment *ev)
{
	if (somThis)
	{
		RHBServerRequest *req=NULL;

		do
		{
			req=RHBORB_pop_first_queued_request(somThis);

			if (req) 
			{
/*				if (nesting) 
				{
					somPrintf("RHBORB_ProcessEvents(%d,%d,%s)\n",
							getpid(),
							nesting,
							req->header.operation);
				}*/

				SOM_TRY

				req->qel.process_request(req,ev);

				SOM_CATCH_ALL

				somPrintf("request exception caught,%s:%d\n",__FILE__,__LINE__);

				SOM_ENDTRY

				if (ev->_major) somExceptionFree(ev);
			}
		
		} while (req);
	}
}
#endif

static char * rhbORB_leading_colons(RHBORB *somThis,char *p)
{
	char *q;
	unsigned int i;

	if (p[0]==':') if (p[1]==':') return p;

	i=(unsigned int)strlen(p);

	q=SOMMalloc(i+4);

	q[0]=':';
	q[1]=':';

	memcpy(&q[2],p,i+1);

	SOMFree(p);

	return q;
}

static struct RHBContainedVtbl rhbContainedVtbl=
{
	rhbContained_AddRef,
	rhbContained_Release,
	rhbContained_get_id
};

static RHBContained * RHBContainedNew(
		Environment *ev,
		RHBORB *orb,
		corbastring name,
		any *desc_value,
		unsigned short describeFlags,
		_IDL_SEQUENCE_somModifier *modifiers,
		unsigned short modifierFlags,
		RHBORB_operation_handler handler)
{
	RHBContained *op=NULL;
	int id_len=(int)strlen(name);

/*	if (!strcmp(name,"::SOMDServer::somdCreateObj"))
	{
		somPrintf("%s created\n",name);
	}
*/
	RHBOPT_ASSERT(ev);
	RHBOPT_ASSERT(ev->_major==NO_EXCEPTION);
	RHBOPT_ASSERT(name);
	RHBOPT_ASSERT(!memcmp(name,"::",2));

	if (ev->_major)
	{
		debug_somPrintf(("exception %s at %s:%d while registering %s\n",
				somExceptionId(ev),
				__FILE__,__LINE__,
				name));

		return NULL;
	}

	op=(RHBContained *)SOMMalloc(sizeof(*op)+id_len);

	if (op)
	{
		static _IDL_SEQUENCE_somModifier null_mods={0,0,NULL};
		static any null_any={NULL,NULL};

		op->describeFlags=describeFlags;
		op->modifierFlags=modifierFlags;
		op->lpVtbl=&rhbContainedVtbl;
		op->desc_value=*desc_value;
		op->modifiers=*modifiers;
		op->handler=handler;
		op->id=NULL;

		desc_value->_type=NULL;
		desc_value->_value=NULL;

#ifdef _DEBUG
		{
			any a=op->desc_value;
			int tcConst=((a._type==TC_InterfaceDescription) ||
						(a._type==TC_OperationDescription)  ||
						(a._type==TC_ExceptionDescription));
			char *p=a._value;
			InterfaceDef_InterfaceDescription *desc=a._value;
			size_t len=TypeCode_size(a._type,ev);
			p+=len;

			switch (describeFlags)
			{
			case ORB_DESCRIBE_SOMIR:
				RHBOPT_ASSERT(tcConst);
				RHBOPT_ASSERT(desc->name==p);
				break;
			case ORB_DESCRIBE_FREE:
				RHBOPT_ASSERT(!tcConst);
				RHBOPT_ASSERT(desc->name!=p);
				break;
			}
		}
#endif

		if (modifierFlags==ORB_DESCRIBE_FREE)
		{
			*modifiers=null_mods;
		}

		*desc_value=null_any;

		memcpy(op->absolute_name,name,id_len+1);

		op->lUsage=1;

		op->orb=orb;

		if (!op->modifiers._length)
		{
			if (!strcmp(name,"::SOMObject::somGetClassName"))
			{
			/* cheat to make sure it has the correct modifiers */
				/* should have 'dual_owned_result' */

				RHBOPT_ASSERT(!name)
		
				debug_somPrintf(("missing modifiers for %s at %s:%d\n",
						op->absolute_name,
						__FILE__,
						__LINE__));

				op->modifiers._length=1;
				op->modifiers._maximum=1;
				op->modifiers._buffer=SOMMalloc(sizeof(*op->modifiers._buffer));
				op->modifiers._buffer[0].name=somd_dupl_string("dual_owned_result");
				op->modifiers._buffer[0].value=NULL;
			}
		}

		/*
	#ifdef USE_THREADS
		key=RHBORB_guard(orb,__FILE__,__LINE__);
	#endif

		op->next=orb->operations;
		orb->operations=op;

	#ifdef USE_THREADS
		RHBORB_unguard(orb,key);
	#endif
	*/
		op->key.count=0;
		op->key.data._value=op;
		op->key.data._type=TC_string;
		op->key.key._length=(long)strlen(op->absolute_name);
		op->key.key._maximum=op->key.key._length;
		op->key.key._buffer=(octet *)op->absolute_name;

		RHBDebug_created(RHBContained,op)

		if (RHBCDR_kds_add(&orb->contained_by_abs,&op->key))
		{
			return op;
		}

		op->lUsage--;
		op->orb=NULL;	/* so don't try and remove it */

		if (!op->lUsage) 
		{
			rhbContained_delete(op);
		}

		return RHBORB_get_contained(orb,ev,name);
	}

	return NULL;
}

#ifdef USE_THREADS
static void rhbORB_StartThreadTask(
		RHBORB *somThis,
		struct RHBORB_thread_task *t)
{
	boolean started=0;
	
	RHBORB_guard(somThis)

	if (!somThis->closing)
	{
		t->next=NULL;

		started=RHBORB_queue_task(somThis,t);

		if (!started)
		{
			if (somThis->child.tasks)
			{
				struct RHBORB_thread_task *p=somThis->child.tasks;
				while (p->next) 
				{
					p=p->next; 
				}
				p->next=t;
			}
			else
			{
				somThis->child.tasks=t;
			}

	#ifdef USE_PTHREADS
			if (!pthread_cond_signal(&somThis->child.pEvent))
	#else
			if (SetEvent(somThis->child.hEvent))
	#endif
			{
				started=1;
			}
			else
			{
				somThis->child.tasks=t->next;
			}
		}
	}

	RHBORB_unguard(somThis)

	if (!started)
	{
		if (t->failed)
		{
			t->failed(t->param);
		}
	}
}
#endif

static char * rhbORB_create_hex_string(RHBORB *somThis,void *v,size_t i)
{
	octet *j=v;
	char *p=SOMMalloc((i<<1)+1);
	char *o=p;

	while (i--)
	{
		*o++=RHBORB_hex_char(somThis,(char )(0xf & ((*j)>>4)));
		*o++=RHBORB_hex_char(somThis,(char )((*j)&0xf));
		j++;
	}
	*o=0;

	return p;
}

struct rhbORB_get_principal
{
	Principal SOMSTAR retval;
};

RHBOPT_cleanup_begin(rhbORB_get_principalCleanup,pv)

	struct rhbORB_get_principal *data=pv;

	RHBORB_unguard(orb)

	if (data->retval) somReleaseObjectReference(data->retval);

RHBOPT_cleanup_end

static Principal SOMSTAR rhbORB_get_principal(
		RHBORB *somThis,Environment *ev)
{
	Principal SOMSTAR RHBOPT_volatile retval=NULL;
	struct rhbORB_get_principal data={NULL};

	RHBORB_guard(orb)
	RHBOPT_cleanup_push(rhbORB_get_principalCleanup,&data);

	if (somThis->principal)
	{
		data.retval=Principal_somDuplicateReference(somThis->principal);
	}
	else
	{
		data.retval=SOMClass_somNew(somThis->somClass_Principal_ref);

		if (data.retval)
		{
			char *userName=somutgetshellenv(szUSER,szSOMD_TCPIP);
			char *hostName=somutgetshellenv(szHOSTNAME,szSOMD_TCPIP);

			Principal__set_userName(data.retval,ev,userName);
			Principal__set_hostName(data.retval,ev,hostName);
		}
	}

	if (data.retval && ev && !ev->_major)
	{
		retval=data.retval;
		data.retval=NULL;
		if (retval && !somThis->principal)
		{
			somThis->principal=Principal_somDuplicateReference(retval);
		}
	}

	RHBOPT_cleanup_pop();

	return retval;
}

struct rhbORB_get_exception_tc
{
	RHBContained *cnd;
	Environment ev;
};

RHBOPT_cleanup_begin(rhbORB_get_exception_tc_cleanup,pv)

struct rhbORB_get_exception_tc *data=pv;

	if (data->cnd)
	{
		RHBContained_Release(data->cnd);
	}

	SOM_UninitEnvironment(&data->ev);

RHBOPT_cleanup_end

static TypeCode rhbORB_get_exception_tc(
		RHBORB *orb,
		exception_type type,
		char *id)
{
	TypeCode RHBOPT_volatile tc=NULL;

	if (type==SYSTEM_EXCEPTION)
	{
		tc=somdTC_StExcep;
	}
	else
	{
		if (orb && id)
		{
			struct rhbORB_get_exception_tc data={NULL};

			SOM_InitEnvironment(&data.ev);

			RHBOPT_cleanup_push(rhbORB_get_exception_tc_cleanup,&data);

			data.cnd=RHBORB_get_contained(orb,&data.ev,id);

			if (data.cnd) 
			{
				any a=RHBCDR_get_named_element(
						&data.ev,
						&data.cnd->desc_value,
						"type");

				if (a._type && !data.ev._major)
				{
					if (tcEqual(a._type,&data.ev,TC_TypeCode))
					{
						tc=*((TypeCode *)a._value);

						if (tc) tc=TypeCode_copy(tc,&data.ev);
					}
				}
			}

			RHBOPT_cleanup_pop();
		}
	}

	return tc;
}


struct RHBORB_contained_describe
{
	Contained SOMSTAR op_def;
	unsigned short describe_flags;
	_IDL_SEQUENCE_somModifier *modifiers; /* moved to avoid clobbering */
	_IDL_SEQUENCE_somModifier mods;
};

RHBOPT_cleanup_begin(RHBORB_contained_describe_cleanup,pv)

struct RHBORB_contained_describe *data=pv;

	if (data->op_def) somReleaseObjectReference(data->op_def);

RHBOPT_cleanup_end

static boolean RHBORB_contained_describe(
		RHBORB *somThis,
		Environment *ev,
		char *name,
		any *description_value,
		unsigned short *flagsDesc,
		_IDL_SEQUENCE_somModifier *modifiers_arg,
		unsigned short *flagsMods,
		RHBORB_operation_handler *handler)
{
	struct RHBORB_contained_describe data={NULL,0,NULL,{0,0,NULL}};
	boolean RHBOPT_volatile success=0;
static any null_any={NULL,NULL};

	RHBOPT_cleanup_push(RHBORB_contained_describe_cleanup,&data);

	data.modifiers=modifiers_arg;

	*flagsMods=ORB_DESCRIBE_CONST;
	*flagsDesc=ORB_DESCRIBE_FREE;

	*description_value=null_any;

	if (data.modifiers)
	{
		data.modifiers[0]=data.mods;
	}

	data.op_def=ORB_lookup_id(SOMD_ORBObject,ev,NULL,name);

	if (data.op_def && !ev->_major)
	{
		ORB_contained_describe(SOMD_ORBObject,ev,data.op_def,
				&data.describe_flags,
				description_value,
				data.modifiers ? &data.mods : data.modifiers,
				NULL,NULL);

		if (!ev->_major) 
		{
			success=1;

			*flagsDesc=data.describe_flags;
			*flagsMods=data.describe_flags;

			if (data.mods._length && data.modifiers)
			{
				if (data.describe_flags==ORB_DESCRIBE_FREE)
				{
					*data.modifiers=data.mods;
				}
				else
				{
					RHBCDR_copy_from(somdTC_sequence_somModifier,ev,
							data.modifiers,&data.mods);

					*flagsMods=ORB_DESCRIBE_FREE;
				}

				data.mods._length=0;
				data.mods._maximum=0;
				data.mods._buffer=NULL;
			}
		}
	}

	RHBOPT_cleanup_pop();

	return success;
}

struct RHBORB_get_contained
{
	any desc_value;
	_IDL_SEQUENCE_somModifier modifiers;
	boolean locked;
	unsigned short flagsMods,flagsDesc;
};

RHBOPT_cleanup_begin(RHBORB_get_contained_cleanup,pv)

	struct RHBORB_get_contained *data=pv;
	Environment ev;
	SOM_InitEnvironment(&ev);

	if (data->locked) 
	{
		RHBORB_unguard(x);
	}

	switch (data->flagsDesc)
	{
	case ORB_DESCRIBE_SOMIR:
		if (data->desc_value._value)
		{
			SOMFree(data->desc_value._value);
		}
		break;
	case ORB_DESCRIBE_FREE:
		SOMD_FreeType(&ev,&data->desc_value,TC_any);
		break;
	}

	switch (data->flagsMods)
	{
	case ORB_DESCRIBE_FREE:
		SOMD_FreeType(&ev,&data->modifiers,somdTC_sequence_somModifier);
		break;
	}
	SOM_UninitEnvironment(&ev);

RHBOPT_cleanup_end

static RHBContained * rhbORB_get_contained(
		RHBORB *somThis,
		Environment *ev,
		corbastring name)
{
	RHBContained * RHBOPT_volatile op=NULL;
	struct RHBORB_get_contained data={{NULL,NULL},{0,0,NULL},0,0,0};

	if (ev->_major) return NULL;

	if (!name) 
	{
		RHBOPT_throw_StExcep(ev,INTF_REPOS,UnknownReposId,NO);

		return NULL;
	}

	RHBORB_guard(somThis)

	if (*name != ':')
	{
/*		buf[0]=':';
		buf[1]=':';
		strcpy(&buf[2],name);
		name=buf;*/

		debug_somPrintf(("RHBORB_get_contained(%s) error\n",name));
/*		bomb("should be ::\n");*/

		RHBORB_unguard(somThis)

		return 0;
	}

	if (RHBCDR_kds_count(&somThis->contained_by_abs))
	{
		_IDL_SEQUENCE_octet o={0,0,NULL};
		RHBSOMUT_KeyDataPtr ptr=NULL;

		o._length=(long)strlen(name);
		o._buffer=(octet *)name;
		o._maximum=o._length;

		ptr=RHBCDR_kds_find(&somThis->contained_by_abs,&o);

		if (ptr)
		{
			op=ptr->data._value;

			RHBContained_AddRef(op);

			RHBORB_unguard(somThis)

			return op;
		}
	}

	RHBORB_unguard(somThis)

	if (name)
	{
		RHBORB_operation_handler handler=NULL;

		RHBOPT_cleanup_push(RHBORB_get_contained_cleanup,&data);

		if (RHBORB_contained_describe(
				somThis,
				ev,
				name,
				&data.desc_value,
				&data.flagsDesc,
				&data.modifiers,
				&data.flagsMods,
				&handler))
		{
			RHBORB_guard(somThis)

			data.locked=1;

			if (!ev->_major)
			{
				op=RHBContainedNew(ev,somThis,
					name,
					&data.desc_value,
					data.flagsDesc,
					&data.modifiers,
					data.flagsMods,
					handler);
			}
		}
		else
		{
			if (!ev->_major)
			{
				SOMD_bomb("RHBORB_contained_describe did not return exception\n");
			}
		}

		RHBOPT_cleanup_pop();
	}

	if (ev->_major)
	{
		if (op)
		{
			RHBContained_Release(op);
			op=NULL;
		}
	}

	return op;
}

static RHBImplementationDef *rhbORB_get_server_impl(
		RHBORB *somThis,
		Environment *ev,
		char *impl_id)
{
	RHBImplementationDef *impl=NULL;

	if ((!ev->_major) &&
		impl_id &&
		impl_id[0])
	{
		RHBORB_guard(somThis)

		{
			unsigned int i=(unsigned int)strlen(impl_id);
			impl=somThis->impls;

			while (impl)
			{
				if (impl->server.data)
				{
					if (impl->server.data->impl_id._length==i)
					{
						if (!memcmp(impl_id,impl->server.data->impl_id._buffer,i))
						{
							if (impl->server.data->listener)
							{
								impl=impl->server.data->listener;
							}

							RHBImplementationDef_AddRef(impl);

							break;
						}

					}
				}

				impl=impl->next;
			}
		}

		RHBORB_unguard(somThis)
	}

	return impl;
}

static SOMDServer SOMSTAR rhbORB_get_local_server(
	RHBORB *somThis,
	Environment *ev,
	char *impl_id)
{
	RHBImplementationDef *impl=NULL;

	if (ev->_major) return NULL;

	impl=RHBORB_get_server_impl(somThis,ev,impl_id);

	if (ev->_major) return NULL;

	if (impl)
	{
		SOMDServer SOMSTAR server=NULL;

		RHBORB_guard(somThis)

		if (impl->server.data)
		{
			server=impl->server.data->somdServer;
		
			if (server)
			{
				server=SOMDServer_somDuplicateReference(server);
			}
		}

		RHBORB_unguard(somThis)

		RHBImplementationDef_Release(impl);

		return server;
	}

	return NULL;
}

struct rhbORB_is_method_oidl
{
	somId method,desc;
	char *defined_in;
};

RHBOPT_cleanup_begin(rhbORB_is_method_oidl_cleanup,pv)

struct rhbORB_is_method_oidl *data=pv;

	if (data->method) SOMFree(data->method);
#ifdef USE_APPLE_SOM
	if (data->desc) SOMFree(data->desc);
#endif
	if (data->defined_in) SOMFree(data->defined_in);

RHBOPT_cleanup_end

static boolean rhbORB_is_method_oidl(
		RHBORB *orb,
		SOMObject SOMSTAR target,
		char *id_arg)
{
	char * RHBOPT_volatile id=id_arg;
	boolean RHBOPT_volatile is=0;
	struct rhbORB_is_method_oidl data={NULL,NULL,NULL};

	RHBOPT_cleanup_push(rhbORB_is_method_oidl_cleanup,&data);

	if (id)
	{
		if (id[0]==':') if (id[1]==':') id+=2;

		data.method=somIdFromString(id);

		if (target)
		{
#ifdef USE_APPLE_SOM
			somKernelId kid=SOMClass_somGetMethodDescriptor(target->mtab->classObject,data.method);
			if (kid) data.desc=somConvertAndFreeKernelId(kid);
#else
			data.desc=SOMClass_somGetMethodDescriptor(target->mtab->classObject,data.method);
#endif

			if (data.desc)
			{
				char *p=data.defined_in=RHBORB_leading_colons(orb,somMakeStringFromId(data.desc));

				while (*p)
				{
					p++;
				}

				while (p > data.defined_in)
				{
					p--;

					if (*p==':')
					{
						p--;
						if (*p==':')
						{
							*p=0;
							break;
						}
					}
				} 

				is=RHBORB_is_oidl(orb,data.defined_in);
			}
		}
	}

	RHBOPT_cleanup_pop();

	return is;
}


static boolean rhbORB_any_client_references(
		RHBORB *orb,SOMObject SOMSTAR obj)
{
	if (orb)
	{
		if (obj)
		{
			RHBImplementationDef *impls=orb->impls;

			while (impls)
			{
				unsigned int i=impls->client_references._length;

				while (i--)
				{
					if (impls->client_references._buffer[i].somobj==obj)
					{
						return 1;
					}
				}

				impls=impls->next;
			}
		}
	}

	return 0;
}

struct RHBORB_ThreadTask * SOMLINK RHBSOMD_get_thread(void)
{
#ifdef USE_THREADS
	RHBORB_ThreadTask *thread=NULL;
	RHBSOMD_get_thread_specific(thread);
	return thread;
#else
	RHBORB_ThreadTask *r=NULL;
	Environment ev;
	ORB SOMSTAR orb=SOMD_ORBObject;
	if (orb)
	{	
		SOM_InitEnvironment(&ev);
		{
			RHBORB *c_orb=ORB__get_c_orb(orb,&ev);
			r=&c_orb->thread_globals;
		}
		SOM_UninitEnvironment(&ev);
	}
	return r;
#endif
}

static boolean rhbORB_set_server_requests_enabled(
		RHBORB *somThis,boolean enabled)
{
	boolean previous=somThis->state.request_loop_ready;

	if (previous != enabled)
	{
		somThis->state.request_loop_ready=enabled;

		RHBORB_server_state_changed(somThis);
	}

	return previous;
}

#ifdef USE_THREADS
struct RHBORB_server_state_waiter
{
	struct RHBORB_server_state_waiter *next;
	RHBORB *orb;
#ifdef USE_PTHREADS
	pthread_cond_t event;
#else
	HANDLE h_event;
#endif
};
#endif

static void rhbORB_server_state_changed(
		RHBORB *somThis)
{
#ifdef USE_THREADS
	struct RHBORB_server_state_waiter *w;

	RHBORB_guard(somThis)

	w=somThis->state.change_waiters;

	while (w)
	{
#ifdef USE_PTHREADS
		pthread_cond_signal(&w->event);
#else
		if (!SetEvent(w->h_event)) { RHBOPT_ASSERT(!"event wrong"); }
#endif

		w=w->next;
	}

	RHBORB_unguard(somThis)
#endif
}

#ifdef USE_THREADS
RHBOPT_cleanup_begin(RHBORB_wait_server_state_changed_cleanup,pv)

	struct RHBORB_server_state_waiter *w=pv;
	RHBORB *somThis=w->orb;

	if (somThis->state.change_waiters==w)
	{
		somThis->state.change_waiters=w->next;
	}
	else
	{
		struct RHBORB_server_state_waiter *p=somThis->state.change_waiters;

		while (p->next != w)
		{
			p=p->next;
		}

		p->next=w->next;
	}

#ifdef USE_PTHREADS
	pthread_cond_destroy(&w->event);
#else
	if (!CloseHandle(w->h_event)) 
	{
#ifdef _M_IX86
		__asm int 3;
#endif
	}
#endif
RHBOPT_cleanup_end

static void rhbORB_wait_server_state_changed(RHBORB *somThis)
{
	struct RHBORB_server_state_waiter w;

	w.orb=somThis;

#ifdef USE_PTHREADS
	pthread_cond_init(&w.event,RHBOPT_pthread_condattr_default);
#else
	w.h_event=CreateEvent(NULL,0,0,NULL);
#endif

	RHBOPT_cleanup_push(RHBORB_wait_server_state_changed_cleanup,&w);

	w.next=somThis->state.change_waiters;
	somThis->state.change_waiters=&w;

#if defined(USE_PTHREADS) 
	somd_wait(&w.event,__FILE__,__LINE__);
#else
	somd_wait(w.h_event,__FILE__,__LINE__);
#endif

	RHBOPT_cleanup_pop();
}
#endif

static char *rhbORB_get_SOMDServer_type_id(RHBORB *somThis,Environment *ev)
{
/*	char type_id[256];
	SOMClass SOMSTAR somdServerClass=somNewClassReference(SOMDServer);
	const char *somdServerName=SOMClass_somGetName(somdServerClass);
	long somdServerMajor=SOMDServer_MajorVersion;
	long somdServerMinor=SOMDServer_MinorVersion;

	const char *fmt="IDL:%s:%ld.%ld";

	SOMClass_somGetVersionNumbers(somdServerClass,
						&somdServerMajor,
						&somdServerMinor);

	snprintf(type_id,sizeof(type_id),fmt,
		somdServerName,
		somdServerMajor,
		somdServerMinor);

	somReleaseClassReference(somdServerClass);
*/
	RHBContained *cnd=RHBORB_get_contained(somThis,ev,"::SOMDServer");
	char *id=NULL;

	if (cnd)
	{
		id=RHBContained_get_id(cnd,ev);

		if (id) 
		{
			id=somd_dupl_string(id);
		}

		RHBContained_Release(cnd);
	}

	return id;
}

static char * rhbORB_get_HOSTNAME(RHBORB *somThis)
{
	char *p=somutgetshellenv(szHOSTNAME,szSOMD_TCPIP);

	return p ? p : szLocalhost;
}


static unsigned short rhbORB_get_SOMDPORT(RHBORB *somThis)
{
	unsigned short s=0;
	char *p=somutgetshellenv(szSOMDPORT,szSOMD_TCPIP);

	if (p)
	{
		s=(unsigned short)atol(p);
	}

#ifdef RHBOPT_SOMDPORT
	if (!s)
	{
		s=RHBOPT_SOMDPORT;
	}
#endif

	return s;
}

static char *rhbORB_get_somdd_implid(RHBORB *somThis)
{
	return "6034ed68-09a7d111-9a-d5-000094687ad6";
}

static struct RHBORBVtbl rhbORBVtbl={
	rhbORB_AddRef,
	rhbORB_Release,
	rhbORB_Close,
	rhbORB_shutdown,
	rhbORB_add_interface,
	rhbORB_remove_interface,
	rhbORB_get_object,
	rhbORB_get_object_from_ior,
	rhbORB_acquire_interface_repository,
	rhbORB_get_local_server,
	rhbORB_get_server_impl,
	rhbORB_get_exception_tc,
	rhbORB_get_contained,
	rhbORB_get_interface,
	rhbORB_get_impl,
	rhbORB_get_principal,
	rhbORB_get_SOMDServer_type_id,
	rhbORB_object_to_string,
	rhbORB_string_to_object,
	rhbORB_create_proxy_class,
	rhbORB_hex_char,
	rhbORB_create_hex_string,
	rhbORB_dump_sequence,
	rhbORB_read_hex,
	rhbORB_add_impl,
	rhbORB_remove_impl,
	rhbORB_get_proxy_class,
	rhbORB_convert_class_to_interface,
	rhbORB_convert_interface_to_class,
	rhbORB_set_server_requests_enabled,
	rhbORB_server_state_changed,
	rhbORB_is_oidl,
	rhbORB_is_method_oidl,
	rhbORB_any_client_references,
	rhbORB_leading_colons,
	rhbORB_id_to_abs,
	rhbORB_abs_to_id,
	rhbORB_get_HOSTNAME,
	rhbORB_get_SOMDPORT,
	rhbORB_get_somdd_implid
#ifdef USE_THREADS
	,rhbORB_wait_server_state_changed,
	rhbORB_StartThreadTask
#else
#endif
};

SOMEXTERN RHBORB * SOMLINK RHBORBNew(
		struct SOMD_Globals *globals,
		Environment *ev)
{
	RHBORB *somThis=SOMMalloc(sizeof(*somThis));
	struct somIdMap *idp=orbIdMap;
	int i=sizeof(orbIdMap)/sizeof(orbIdMap[0]);

	somThis->globals=globals;
	somThis->lpVtbl=&rhbORBVtbl;

	RHBEventManager_init(&somThis->events,somThis);
	RHBRequestList_init(&somThis->pool.completed);

	somThis->pool.waiting=0;
	somThis->pool.outstanding=0;

	somThis->state.request_loop_ready=0;
#ifdef USE_THREADS
	somThis->pool.event=NULL;
	somThis->state.change_waiters=NULL;
	somThis->threads=NULL;
#else
	somThis->thread_globals.server_request=NULL;
#endif

	RHBCDR_kds_init(&somThis->contained_by_abs);
	RHBCDR_kds_init(&somThis->ifaces_by_id);
	RHBCDR_kds_init(&somThis->ifaces_by_abs);

	while (i--)
	{
		somIdMapId(somThis,idp->offset)=somIdFromString((char *)idp->value);
		idp++;
	}

	somThis->principal=NULL;

	somThis->lUsage=1;
	somThis->impls=NULL;
	somThis->iface=NULL;
	somThis->closing=0;

	somThis->somClass_ImplementationDef_ref=somNewClassReference(ImplementationDef);
	somThis->somClass_Request_ref=somNewClassReference(Request);
	somThis->somClass_SOMDObject_ref=somNewClassReference(SOMDObject);
	somThis->somClass_SOMDClientProxy_ref=somNewClassReference(SOMDClientProxy);
	somThis->somClass_SOMOA_ref=somNewClassReference(SOMOA);
	somThis->somClass_SOMDServer_ref=somNewClassReference(SOMDServer);
	somThis->somClass_Principal_ref=somNewClassReference(Principal);
	somThis->somClass_Context_ref=somNewClassReference(Context);
	somThis->somClass_NVList_ref=somNewClassReference(NVList);
	somThis->somClass_somStream_MemoryStreamIO_ref=somNewClassReference(somStream_MemoryStreamIO);
	somThis->somClass_ServerRequest_ref=somNewClassReference(ServerRequest);
	somThis->somClass_SOMDServerStub_ref=somNewClassReference(SOMDServerStub);
	somThis->somClass_PortableServer_DynamicImpl_ref=somNewClassReference(PortableServer_DynamicImpl);

#ifdef somNewObject
	somThis->iop_codecFactory=somNewObject(IOP_CodecFactory);
#else
	somThis->iop_codecFactory=IOP_CodecFactoryNew();
#endif
	{
		Environment ev;
static IOP_Encoding encoding={IOP_ENCODING_CDR_ENCAPS,1,0};
		SOM_InitEnvironment(&ev);
		IOP_CodecFactory__set_c_orb(somThis->iop_codecFactory,&ev,somThis);
		somThis->iop_codec=IOP_CodecFactory_create_codec(somThis->iop_codecFactory,&ev,&encoding);
		SOM_UninitEnvironment(&ev);
	}

	somThis->protocol=NULL;

#ifdef USE_THREADS
	somThis->child.running=0;
	somThis->child.tasks=NULL;
	somThis->child.killer=NULL;
	somThis->child.dead_list=NULL;

	{
		RHBORB_guard(somThis)

		somThis->child.running=1;
#ifdef USE_PTHREADS
		pthread_cond_init(&somThis->child.pEvent,RHBOPT_pthread_condattr_default);
		if (pthread_create(&somThis->child.pThread,RHBOPT_pthread_attr_default,rhborb_child,somThis))
#else
		somThis->child.hEvent=CreateEvent(NULL,0,0,NULL);
		if (somThis->child.hEvent)
		{
			somThis->child.hThread=CreateThread(NULL,0,rhborb_child,somThis,0,&somThis->child.tid);
		}
		else
		{
			somThis->child.hThread=NULL;
		}
		if (!somThis->child.hThread)
#endif
		{
			somThis->child.running=0;
#ifdef USE_PTHREADS
			pthread_cond_destroy(&somThis->child.pEvent);
#else
			CloseHandle(somThis->child.hEvent);
			somThis->child.hEvent=NULL;
#endif
		}

		RHBORB_unguard(somThis)

		if (!somThis->child.running)
		{
			RHBOPT_throw_StExcep(ev,NO_RESOURCES,CouldNotStartThread,NO);
		}
	}
#endif

#ifdef USE_SELECT
	if ((!SOMD_SocketObject) && (ev->_major==NO_EXCEPTION) && somThis)
	{
		char *p=somutgetshellenv("SOMSOCKETS","[somd]");

#ifdef BUILD_STATIC
		{
			SOMClass SOMSTAR clsSockets=
				TCPIPSocketsNewClass(TCPIPSockets_MajorVersion,TCPIPSockets_MinorVersion);

			if (clsSockets && !p)
			{
				p=SOMClass_somGetName(clsSockets);
			}
		}
#endif

		if (p)
		{
			SOMClassMgr SOMSTAR mgr=SOMClassMgrObject;
			somId id=somIdFromString(p);
			SOMClass SOMSTAR cls=SOMClassMgr_somFindClass(mgr,id,0,0);

			SOMFree(id);

			id=0;

			if (cls)
			{
				if (_Sockets && SOMClass_somDescendedFrom(cls,_Sockets))
				{
					SOMD_SocketObject=SOMClass_somNew(cls);
				}
								
				if (SOMD_SocketObject)
				{
					somThis->events.socketObject=SOMD_SocketObject;
				}
				else
				{
					RHBOPT_throw_StExcep(ev,INITIALIZE,NoSocketsClass,NO);
				}
			}
			else
			{
				RHBOPT_throw_StExcep(ev,INITIALIZE,NoSocketsClass,NO);
			}
		}
		else
		{
			RHBOPT_throw_StExcep(ev,INITIALIZE,InvalidSOMSOCKETS,NO);
		}
	}
#endif

#ifdef USE_THREADS
	if (somThis->child.running && (ev->_major==NO_EXCEPTION))
#else
	if (somThis && (ev->_major==NO_EXCEPTION))
#endif
	{
		globals->orb=somThis;

		RHBDebug_created(RHBORB,somThis)


#ifdef _PLATFORM_MACINTOSH_
	/* this loads OpenTransport for the Mac */
		#if GENERATINGCFM

		somThis->protocol=RHBSOMD_load_rhbnetot_protocol();

		#endif
#endif
	
		if (!somThis->protocol)
		{
			somThis->protocol=RHBORB_create_protocol(somThis);
		}

		if (!somThis->protocol)
		{
			dump_somPrintf(("%s:%d, Failed to create network protocol handler\n",__FILE__,__LINE__));
		}

		if (somThis->protocol)
		{
			return somThis;
		}
	}

	RHBORB_Release(somThis);

	return NULL;
}

