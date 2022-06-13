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
	/* this stops exports from exe */
	#define SOMDLLEXPORT
#endif

#define SOM_Module_implreps_Source
#define ImplRepositoryServer_Class_Source
#define DynamicServerObject_Class_Source

/* Apples som does not recognize SOM_DONT_USE_SHORT_NAME
	and BOA_get_id clashes with _get_id from DynamicServerObject */

#define SOMGD_get_id    1

#include <rhbsomdd.h>
#include <somdsvr.h>
#include <somestio.h>
#include <servreq.h>
#include <implreps.ih>
#include <stexcep.h>

#define USE_SEND_MULTIPLE

#define somStartCriticalSection()		dont use
#define somEndCriticalSection()			dont use


#ifdef _WIN32
static void implreps_trap(void)
{
#ifdef _M_IX86
	__asm int 3
#else
	((void **)0)[0]=0;
#endif
}
#endif

#ifdef USE_THREADS
	#ifdef USE_PTHREADS
		#define implreps_notify(x,r)  pthread_cond_signal(&(x)->p_event);	
	#else
		#define implreps_notify(x,r)  if (!SetEvent((x)->h_event)) implreps_trap();
	#endif
#else
	#define implreps_notify(x,r)
#endif

#ifdef USE_THREADS
RHBOPT_cleanup_begin(implreps_begin_cleanup,pv)

	SOMDD_MutexRef *data=pv;
	SOMDD_MutexRef ref=*data;
	*data=NULL;
	if (ref) 
	{
		SOMDD_ReleaseGlobalMutex(ref);
	}

RHBOPT_cleanup_end

RHBOPT_cleanup_begin(implreps_pause_cleanup,pv)

	SOMDD_MutexRef *data=pv;
	*data=SOMDD_AcquireGlobalMutex();

RHBOPT_cleanup_end

	#define SOMDD_MutexRef_begin	{ \
				SOMDD_MutexRef SOMDD_MutexRef_ref=SOMDD_AcquireGlobalMutex(); \
				RHBOPT_cleanup_push(implreps_begin_cleanup,&SOMDD_MutexRef_ref); {
	#define SOMDD_MutexRef_end   } RHBOPT_cleanup_pop(); }
	#define SOMDD_MutexRef_pause    RHBOPT_cleanup_call(implreps_begin_cleanup,&SOMDD_MutexRef_ref); \
				RHBOPT_cleanup_push(implreps_pause_cleanup,&SOMDD_MutexRef_ref);
	#define SOMDD_MutexRef_resume   RHBOPT_cleanup_pop();
	#define SOMDD_MutexRef_enter(m)	{ SOMDD_MutexRef SOMDD_MutexRef_ref=m; RHBOPT_ASSERT(m);
	#define SOMDD_MutexRef_leave(m)  RHBOPT_ASSERT((m==SOMDD_MutexRef_ref)); }
#else	
	#define SOMDD_MutexRef_begin
	#define SOMDD_MutexRef_end
	#define SOMDD_MutexRef_enter(x)
	#define SOMDD_MutexRef_leave(x)
	#define SOMDD_MutexRef_pause
	#define SOMDD_MutexRef_resume
	#define SOMDD_MutexRef_ref			0
#endif

char *SOMDD_dupl_string(const char *p)
{
	if (p)
	{
		int i=(int)strlen(p)+1;
		char *q=SOMMalloc(i);
		memcpy(q,p,i);
		return q;
	}

	return NULL;
}

static boolean SOMDD_has_string(_IDL_SEQUENCE_string *somThis,char *str)
{
	if (str)
	{
		unsigned long i=somThis->_length;

		while (i--)
		{
			if (!strcmp(str,somThis->_buffer[i]))
			{
				return 1;
			}
		}
	}

	return 0;
}

static void implreps_somFree(SOMObject SOMSTAR *ref)
{
	SOMObject SOMSTAR obj=ref[0];
	ref[0]=NULL;
	if (obj) SOMObject_somFree(obj);
}

static void RHBRequest_delete(struct RHBRequest *somThis,somToken mutex)
{
	Request SOMSTAR request=somThis->request;

	RHBOPT_ASSERT(!somThis->request_list);

	SOMDD_MutexRef_enter(mutex)

	somThis->request=NULL;

	if (somThis->request_list)
	{
		if (somThis==somThis->request_list->list)
		{
			somThis->request_list->list=NULL;
		}
		else
		{
			struct RHBRequest *p=somThis->request_list->list;

			while (p)
			{
				if (p->next==somThis)
				{
					p->next=somThis->next;
					break;
				}

				p=p->next;
			}
		}
	}

	if (request)
	{
		SOMDD_MutexRef_pause
		Request_somFree(request);
		SOMDD_MutexRef_resume
	}

	SOMDD_MutexRef_leave(mutex)
}

static void RHBServerRequest_change_state_r(
			struct RHBServerRequest *somThis,
			long state,
			const char *f,int n)
{
	long old_state=somThis->state;

	RHBOPT_ASSERT(state);
	RHBOPT_ASSERT(state!=old_state);
	RHBOPT_ASSERT(state>old_state);

	{
		somPrintf("%s:%d, state change of %p from %ld to %ld\n",
				f,n,
				somThis,
				old_state,state);

		somThis->state=state;
	}
}

#define RHBServerRequest_change_state(x,y) RHBServerRequest_change_state_r(x,y,__FILE__,__LINE__)

static void RHBServerRequest_delete(struct RHBServerRequest *somThis,somToken mutex)
{
	SOMDD_MutexRef_enter(mutex)

	RHBOPT_ASSERT(somThis->state==RHBServerRequest_state_end);

	RHBServerRequest_change_state(somThis,RHBServerRequest_state_delete);

	if (somThis->server_request_list)
	{
		if (somThis->server_request_list->list==somThis)
		{
			somThis->server_request_list->list=somThis->next;
		}
		else
		{
			struct RHBServerRequest *p=somThis->server_request_list->list;

			while (p->next != somThis)
			{
				p=p->next;
			}

			p->next=somThis->next;
		}
	}

	RHBRequest_delete(&somThis->_interface,SOMDD_MutexRef_ref);
	RHBRequest_delete(&somThis->_get_id,SOMDD_MutexRef_ref);
	RHBRequest_delete(&somThis->_get_somModifiers,SOMDD_MutexRef_ref);

	{
		SOMDD_MutexRef_pause

		implreps_somFree(&somThis->server_request);
		implreps_somFree(&somThis->dynobj);
		implreps_somFree(&somThis->somref);
		implreps_somFree(&somThis->codec);
		implreps_somFree(&somThis->client_proxy);
		implreps_somFree(&somThis->interface_def);

		if (somThis->object_key._buffer)
		{
			SOMFree(somThis->object_key._buffer);
			somThis->object_key._buffer=NULL;
		}

		if (somThis->type_id)
		{
			SOMFree(somThis->type_id);
			somThis->type_id=NULL;
		}

		if (somThis->mods._buffer)
		{
			unsigned long i=somThis->mods._length;
			somModifier *b=somThis->mods._buffer;
			while (i--)
			{
				if (b->name) SOMFree(b->name);
				if (b->value) SOMFree(b->value);
				b++;
			}
			SOMFree(somThis->mods._buffer);
		}

		somdExceptionFree(&somThis->ev);

		SOMFree(somThis);

		SOMDD_MutexRef_resume
	}

	SOMDD_MutexRef_leave(mutex)
}

static void RHBRequestList_add(struct RHBRequestList *list,
							   struct RHBRequest *req,
							   somToken ref)
{
	RHBOPT_ASSERT(!req->request_list);

	req->request_list=list;
	req->next=list->list;
	list->list=req;
	req->pending++;

	implreps_notify(list,ref);
}

static _IDL_SEQUENCE_somModifier SOMDD_dupl_somModifiers(
		_IDL_SEQUENCE_somModifier *mods)
{
	_IDL_SEQUENCE_somModifier seq={0,0,NULL};

	if (mods && mods->_length)
	{
		unsigned long i=mods->_length;
		seq._buffer=SOMCalloc(mods->_length,sizeof(seq._buffer[0]));
		seq._length=i;
		seq._maximum=i;
		while (i--)
		{
			seq._buffer[i].name=SOMDD_dupl_string(mods->_buffer[i].name);
			seq._buffer[i].value=SOMDD_dupl_string(mods->_buffer[i].value);
		}
	}

	return seq;
}

static long mod_long(_IDL_SEQUENCE_somModifier *m,const char *name)
{
	somModifier *b=m->_buffer;
	unsigned long i=m->_length;

	while (i--)
	{
		if (!strcmp(b->name,name))
		{
			if (b->value) return atol(b->value);

			break;
		}

		b++;
	}

	return 0;
}

static void adjust_type(struct RHBServerRequest *somThis)
{
	if (somThis->type_id && somThis->mods._length && !memcmp(somThis->type_id,"::",2))
	{
		char vers[30];
		int j=(int)strlen(somThis->type_id);
		char *id;
		const char *q=somThis->type_id;
		char *p;
		const char *fmt="%ld.%ld";
		int vlen=1
#ifdef HAVE_SNPRINTF
		+snprintf(vers,sizeof(vers),
				fmt,
				mod_long(&somThis->mods,"majorversion"),
				mod_long(&somThis->mods,"minorversion"));
#else
#	ifdef HAVE_PTR_SPRINTF
		+strlen(sprintf(vers,
				fmt,
				mod_long(&somThis->mods,"majorversion"),
				mod_long(&somThis->mods,"minorversion")));
#	else
		+sprintf(vers,
				fmt,
				mod_long(&somThis->mods,"majorversion"),
				mod_long(&somThis->mods,"minorversion"));
#	endif
#endif
		id=p=SOMMalloc(4+vlen+j);

		memcpy(p,"IDL:",4); p+=4;

		while (*q)
		{
			if (*q==':') 
			{
				while (*q==':') q++;
			}
			else
			{
				int l=0;
				while (q[l] && (q[l]!=':'))
				{
					l++;
				}
				if (l)
				{
					if (p[-1]!=':') 
					{
						*p++='/';
					}
					memcpy(p,q,l);
					q+=l;
					p+=l;
				}
			}
		}

		memcpy(p,vers,vlen);

		SOMFree(somThis->type_id);

		somThis->type_id=id;
	}
}

static void RHBServerRequest_reply(struct RHBServerRequest *somThis,somToken mutex)
{
	Environment ev;

	SOM_InitEnvironment(&ev);

	RHBOPT_ASSERT(somThis->state==RHBServerRequest_state_begin_reply);

	RHBServerRequest_change_state(somThis,RHBServerRequest_state_wait_reply);

	if ((somThis->type_id)&&(somThis->client_proxy))
	{
		adjust_type(somThis);

		SOMDClientProxy__set_type_id(somThis->client_proxy,&ev,somThis->type_id);

#if 0
		SOMObject_somDumpSelf(somThis->client_proxy,0);
#endif
	}

	if (somThis->client_proxy)
	{
		Environment evForward;
		PortableServer_ForwardRequest *ex=SOMMalloc(sizeof(*ex));
		ServerRequest SOMSTAR req=somThis->server_request;
		any a={NULL,NULL};
		somThis->server_request=NULL;
		SOM_InitEnvironment(&evForward);

		ex->forward_reference=SOMDObject_somDuplicateReference(somThis->client_proxy);
		somSetException(&evForward,USER_EXCEPTION,ex_PortableServer_ForwardRequest,ex);
		a._type=somddTC_PortableServer_ForwardRequest;

		ServerRequest_set_exception(req,&evForward,&a);

		if ((a._value==ex)||(evForward.exception._params==ex))
		{
			SOMObject SOMSTAR ref=ex->forward_reference;
			ex->forward_reference=NULL;
			if (ref) somReleaseObjectReference(ref);
		}

		somReleaseObjectReference(req);
		SOM_UninitEnvironment(&evForward);
	}
	else
	{
#if defined(_WIN32) && defined(_DEBUG)
		implreps_trap();
#endif
	}

	SOM_UninitEnvironment(&ev);

	switch (somThis->state)
	{
	case RHBServerRequest_state_wait_reply:
		RHBServerRequest_change_state(somThis,RHBServerRequest_state_end);
		RHBServerRequest_delete(somThis,mutex);
		break;
	}
}

static void RHBServerRequest_reply_error(struct RHBServerRequest *somThis,somToken mutex)
{
	ServerRequest SOMSTAR req=somThis->server_request;
	somThis->server_request=NULL;

	SOMDD_MutexRef_enter(mutex)

	if (req)
	{
		SOMDD_MutexRef_pause

		if (somThis->ev._major==NO_EXCEPTION)
		{
			RHBOPT_throw_StExcep(&somThis->ev,
				UNKNOWN,UnknownError,MAYBE);
		}

		ServerRequest_set_result(req,&somThis->ev,NULL);

		SOMDD_MutexRef_resume

		somReleaseObjectReference(req);
	}

	RHBServerRequest_change_state(somThis,RHBServerRequest_state_end);

	RHBServerRequest_delete(somThis,mutex);

	SOMDD_MutexRef_leave(mutex)
}

static void RHBServerRequest_describe_complete(struct RHBRequest *somThis,
												Environment *ev,
												somToken mutex)
{
	struct RHBServerRequest *sreq=somThis->server_request;

	RHBOPT_ASSERT(sreq->state==RHBServerRequest_state_wait_describe);

	SOMDD_MutexRef_enter(mutex) 

	if (ev->_major==NO_EXCEPTION)
	{
#if defined(_WIN32) && defined(_DEBUG)
		implreps_trap();
#endif
		somPrintf("this needs work to properly reply...%s:%d\n",__FILE__,__LINE__);
	}

	RHBServerRequest_change_state(sreq,RHBServerRequest_state_end);

	RHBServerRequest_delete(sreq,SOMDD_MutexRef_ref);

	SOMDD_MutexRef_leave(mutex)
}

static void begin_describe(struct RHBServerRequest *somThis,Environment *ev,somToken mutex)
{
#if 0
	SOMObject_somDumpSelf(somThis->interface_def,0);
#endif

	somThis->_get_id.result.argument._type=TC_string;
	somThis->_get_id.result.argument._value=&somThis->type_id;
	somThis->_get_id.server_request=somThis;
	somThis->_get_id.complete=RHBServerRequest_describe_complete;

	SOMDObject_create_request(somThis->client_proxy,
							  ev,0,
							  "_get_id",
							  0,
							  &somThis->_get_id.result,
							  &somThis->_get_id.request,0);
					
	RHBOPT_ASSERT(ev->_major==NO_EXCEPTION);

	if (ev->_major==NO_EXCEPTION)
	{
		/* invoke it async */

#ifdef USE_SEND_MULTIPLE
		if (send_multiple_requests(&somThis->_get_id.request,ev,1,0))
#else
		if (Request_send(somThis->_get_id.request,ev,0))
#endif
		{
#if defined(_WIN32) && defined(_DEBUG)
			implreps_trap();
#endif
		}
	}
}

static void RHBServerRequest_interface_complete(struct RHBRequest *somThis,
												Environment *ev,
												somToken mutex)
{
	SOMClass SOMSTAR clientProxyClass=somNewClassReference(SOMDClientProxy);
	struct RHBServerRequest *sreq=somThis->server_request;
	SOMObject SOMSTAR proxyFor=NULL;

	SOMDD_MutexRef_enter(mutex)

	RHBOPT_ASSERT(sreq->state==RHBServerRequest_state_wait_interface);

	if ((ev->_major==NO_EXCEPTION)&&
		(sreq->state==RHBServerRequest_state_wait_interface))
	{
		somId somId_get_id=somIdFromString("_get_id");
		somId somId_get_somModifiers=somIdFromString("_get_somModifiers");
		char *id=NULL;
		_IDL_SEQUENCE_somModifier mods={0,0,NULL};

		RHBServerRequest_change_state(sreq,RHBServerRequest_state_begin_describe);

		SOMDD_MutexRef_pause

#if 0
		SOMObject_somDumpSelf(sreq->interface_def,0);
#endif

		if (SOMObject_somIsA(sreq->interface_def,clientProxyClass))
		{
			proxyFor=SOMDClientProxy__get_sommProxyTarget(sreq->interface_def);

			if (proxyFor)
			{
				somva_SOMObject_somDispatch(proxyFor,
						(somToken *)(void *)&id,
						somId_get_id,
						proxyFor,ev);

				if ((ev->_major==NO_EXCEPTION)&&(id))
				{
					somva_SOMObject_somDispatch(proxyFor,
							(somToken *)(void *)&mods,
							somId_get_somModifiers,
							proxyFor,ev);
				}
			}
		}
		else
		{
			somva_SOMObject_somDispatch(sreq->interface_def,
					(somToken *)(void *)&id,
					somId_get_id,
					sreq->interface_def,ev);

			if ((ev->_major==NO_EXCEPTION)&&(id))
			{
				somva_SOMObject_somDispatch(sreq->interface_def,
						(somToken *)(void *)&mods,
						somId_get_somModifiers,
						sreq->interface_def,ev);
			}

			if (!id)
			{
				RHBOPT_throw_StExcep(ev,INTERNAL,UnexpectedNULL,MAYBE);
			}
		}

		SOMDD_MutexRef_resume

		switch (sreq->state)
		{
		case RHBServerRequest_state_begin_describe:
			if (id)
			{
				sreq->type_id=SOMDD_dupl_string(id);
				sreq->mods=SOMDD_dupl_somModifiers(&mods);
				RHBServerRequest_change_state(sreq,RHBServerRequest_state_begin_reply);
			}
			else
			{
				if (ev->_major==NO_EXCEPTION)
				{
					begin_describe(sreq,ev,mutex);

					if (sreq->_get_id.request)
					{
						RHBRequestList_add(sreq->request_list,&sreq->_get_id,SOMDD_MutexRef_ref);
						RHBServerRequest_change_state(sreq,RHBServerRequest_state_wait_describe);
					}
					else
					{
#if defined(_WIN32) && defined(_DEBUG)
						implreps_trap();
#endif
					}
				}
			}
			break;
		default:
#if defined(_WIN32) && defined(_DEBUG)
			implreps_trap();
#endif
			break;
		}

		SOMFree(somId_get_id);
		SOMFree(somId_get_somModifiers);
	}

	switch (sreq->state)
	{
	case RHBServerRequest_state_begin_reply:
		RHBServerRequest_reply(sreq,SOMDD_MutexRef_ref);
		break;
	case RHBServerRequest_state_wait_describe:
		break;
	default:
		if ((ev->_major!=NO_EXCEPTION)&&(sreq->ev._major==NO_EXCEPTION))
		{
			void *pv=ev->exception._params;
			ev->exception._params=NULL;

			somSetException(&sreq->ev,
					ev->_major,
					somExceptionId(ev),
					pv);
		}

		RHBServerRequest_reply_error(sreq,SOMDD_MutexRef_ref);
	}

	if (proxyFor)
	{
		SOMDD_MutexRef_pause
		somReleaseObjectReference(proxyFor);
		SOMDD_MutexRef_resume
	}

	if (clientProxyClass)
	{
		SOMDD_MutexRef_pause
		somReleaseClassReference(clientProxyClass);
		SOMDD_MutexRef_resume
	}

	somdExceptionFree(ev);

	SOMDD_MutexRef_leave(mutex)
}

static void RHBServerRequest_get_interface(struct RHBServerRequest *somThis,somToken mtx)
{
#ifdef USE_THREADS
	SOMDD_MutexRef SOMDD_MutexRef_ref=mtx;
#endif
	Environment ev;

	SOM_InitEnvironment(&ev);

	/* get the implementation from the somdsvr object */

	RHBOPT_ASSERT(somThis->impl->active_impl);
	RHBOPT_ASSERT(SOMObject_somIsA(somThis->impl->active_impl,_ImplementationDef));
	RHBOPT_ASSERT(somThis->codec);
	RHBOPT_ASSERT(!somThis->interface_def);
	RHBOPT_ASSERT(!somThis->_interface.request);
	RHBOPT_ASSERT(!somThis->type_id);
	RHBOPT_ASSERT(somThis->state==RHBServerRequest_state_begin_interface);

#if 0
	SOMObject_somDumpSelf(somThis->somref,0);
#endif
	/* change the implementation of the somref object */

	/* now have to make a client proxy */

	SOMOA_change_implementation(SOMD_SOMOAObject,&ev,
								somThis->somref,
								somThis->impl->active_impl);

	RHBOPT_ASSERT(ev._major==NO_EXCEPTION);
	
#if 0			
	SOMObject_somDumpSelf(somThis->somref,0);
#endif

	if (SOMObject_somIsA(somThis->somref,_SOMDClientProxy))
	{
		somThis->client_proxy=SOMDClientProxy_somDuplicateReference(somThis->somref);
	}
	else
	{
		any a={NULL,NULL};
		_IDL_SEQUENCE_octet ior={0,0,NULL};
		char IDL_CORBA_Object[32];
		long majorVersion=SOMObject_MajorVersion;
		long minorVersion=SOMObject_MajorVersion;
		const char *fmt="IDL:SOMObject:%ld.%ld";
#ifdef HAVE_SNPRINTF
		snprintf(IDL_CORBA_Object,sizeof(IDL_CORBA_Object),fmt,majorVersion,minorVersion);
#else
		sprintf(IDL_CORBA_Object,fmt,majorVersion,minorVersion);
#endif

		a._type=TC_Object;
		a._value=&somThis->somref;

		SOMDD_MutexRef_pause

		SOMDObject__set_type_id(somThis->somref,&ev,IDL_CORBA_Object);

		ior=IOP_Codec_encode_value(somThis->codec,&ev,&a);

		a._type=NULL;
		a._value=NULL;

		if (ev._major==NO_EXCEPTION)
		{
			a=IOP_Codec_decode_value(somThis->codec,&ev,&ior,TC_Object);

			SOMFree(ior._buffer);
			ior._buffer=NULL;
		}

		SOMDD_MutexRef_resume

		if (a._value && (ev._major==NO_EXCEPTION))
		{
			somThis->client_proxy=((SOMObject SOMSTAR *)a._value)[0];
			SOMFree(a._value);
		}

		if (a._type && (ev._major==NO_EXCEPTION))
		{
			TypeCode_free(a._type,&ev);
		}
	}

#if 0
	SOMObject_somDumpSelf(somThis->client_proxy,0);
#endif

	/* create a request to get the interface */

	somThis->_interface.result.argument._type=somddTC_CORBA_InterfaceDef;
	somThis->_interface.result.argument._value=&somThis->interface_def;
	somThis->_interface.server_request=somThis;
	somThis->_interface.complete=RHBServerRequest_interface_complete;

	SOMDObject_create_request(somThis->client_proxy,
		                      &ev,0,
							  "_interface",
							  0,
							  &somThis->_interface.result,
							  &somThis->_interface.request,0);
					
	RHBOPT_ASSERT(ev._major==NO_EXCEPTION);

	if (ev._major==NO_EXCEPTION)
	{
		/* invoke it async */

#ifdef USE_SEND_MULTIPLE
		if (!send_multiple_requests(&somThis->_interface.request,&ev,1,0))
#else
		if (!Request_send(somThis->_interface.request,&ev,0))
#endif
		{
			RHBRequestList_add(somThis->request_list,&somThis->_interface,mtx);

			RHBServerRequest_change_state(somThis,RHBServerRequest_state_wait_interface);
		}
	}

	SOM_UninitEnvironment(&ev);
}

static void RHBRequest_cancel(struct RHBRequest *somThis,somToken mutex)
{
	if (somThis->request && !somThis->cancelled)
	{
		Environment ev;
		somThis->cancelled=1;
		RHBOPT_ASSERT(somThis->request);
		somPrintf("cancelling request %p\n",somThis->request);
		SOM_InitEnvironment(&ev);
		Request_destroy(somThis->request,&ev);
		SOM_UninitEnvironment(&ev);
	}
}

static void implreps_changed(
		ImplRepositoryServerData *somThis,
		ImplRepositoryServer_ImplRepContents *k,
		somToken mtx)
{
#ifdef USE_THREADS
	SOMDD_MutexRef SOMDD_MutexRef_ref=mtx;
#endif

	while (k)
	{
		struct RHBServerRequest *req=somThis->server_request_list.list;

		while (req)
		{
			RHBOPT_ASSERT(req->impl);

			if (k==req->impl)
			{
				if (k->state==ImplRepositoryServer_Server_Ready)
				{
					if (req->state==RHBServerRequest_state_wait_for_up)
					{
						RHBServerRequest_change_state(req,RHBServerRequest_state_is_up);
						break;
					}
				}
				else
				{
					if (k->state==ImplRepositoryServer_Server_Activating)
					{
	#if defined(_WIN32) && defined(_DEBUG)
						somPrintf("already activating....\n");
	#endif
					}
					else
					{
						switch (req->state)
						{
						case RHBServerRequest_state_wait_interface:
							/* do the cancelation */
							somPrintf("cancelling %ld\n",req->state);
							RHBRequest_cancel(&req->_interface,mtx);
							break;
						case RHBServerRequest_state_wait_describe:
							somPrintf("cancelling %ld\n",req->state);
							/* do the cancelation */
							RHBRequest_cancel(&req->_get_id,mtx);
							RHBRequest_cancel(&req->_get_somModifiers,mtx);
							break;
						case RHBServerRequest_state_wait_for_up:
							RHBServerRequest_change_state(req,RHBServerRequest_state_end);
							RHBOPT_throw_StExcep(&req->ev,
								TRANSIENT,
								ServerNotActive,MAYBE);
							break;
						case RHBServerRequest_state_end:
							somPrintf("not changing state, already at end\n");
							break;
						case RHBServerRequest_state_begin_describe:
						case RHBServerRequest_state_begin_interface:
						case RHBServerRequest_state_begin_reply:
							somPrintf("not changing state, request is busy state %ld\n",req->state);
							break;
						default:
							somPrintf("not changing state, unknown at %ld\n",req->state);
		#if defined(_WIN32) && defined(_DEBUG)
							implreps_trap();
		#endif
							break;
						}
					}
				}
			}

			if (req->state==RHBServerRequest_state_end)
			{
				break;
			}

			req=req->next;
		}

		if (req)
		{
			switch (req->state)
			{
			case RHBServerRequest_state_end:
				RHBServerRequest_delete(req,SOMDD_MutexRef_ref);
				break;
			case RHBServerRequest_state_is_up:
				RHBServerRequest_change_state(req,RHBServerRequest_state_begin_interface);
				RHBServerRequest_get_interface(req,SOMDD_MutexRef_ref);
				break;
			}
		}
		else
		{
			break;
		}
	}
}

RHBOPT_cleanup_begin(SOMDD_read_strings_cleanup,pv)

	_IDL_SEQUENCE_string *seq=pv;

	if (seq->_buffer)
	{
		while (seq->_length--)
		{
			if (seq->_buffer[seq->_length])
			{
				SOMFree(seq->_buffer[seq->_length]);
			}
		}

		SOMFree(seq->_buffer);
	}

RHBOPT_cleanup_end

static void SOMDD_read_strings(
		_IDL_SEQUENCE_string * RHBOPT_volatile result,
		Environment *ev,
		CosStream_StreamIO SOMSTAR stream)
{
	_IDL_SEQUENCE_string seq={0,0,NULL};
	unsigned long RHBOPT_volatile m=CosStream_StreamIO_read_unsigned_long(stream,ev);

	RHBOPT_cleanup_push(SOMDD_read_strings_cleanup,&seq);

	if (m && !ev->_major)
	{
		seq._maximum=m;
		seq._buffer=
				SOMMalloc(sizeof(seq._buffer[0])*m);

		while (m-- && !ev->_major)
		{
			seq._buffer[seq._length++]=
					CosStream_StreamIO_read_string(stream,ev);
		}
	}

	if (result && !ev->_major)
	{
		*result=seq;
		seq._length=0;
		seq._maximum=0;
		seq._buffer=NULL;
	}

	RHBOPT_cleanup_pop();
}

static void SOMDD_add_string(_IDL_SEQUENCE_string *seq,char *str)
{
	unsigned int i=0;

	while (i < seq->_length)
	{
		if (!strcmp(seq->_buffer[i],str))
		{
			str=0;
			break;
		}
		i++;
	}

	if (str)
	{
		if (seq->_length==seq->_maximum)
		{
			char **h;
			seq->_maximum+=4;
			h=SOMMalloc(seq->_maximum * sizeof(*h));
			i=0;
			while (i < seq->_length)
			{
				h[i]=seq->_buffer[i];
				i++;
			}
			if (seq->_buffer)
			{
				SOMFree(seq->_buffer);
			}
			seq->_buffer=h;
		}
		seq->_buffer[seq->_length]=SOMDD_dupl_string(str);
		seq->_length++;
	}
}

static void SOMDD_remove_string(_IDL_SEQUENCE_string *seq,char *str)
{
	unsigned int i=0;
	while (i < seq->_length)
	{
		if (!strcmp(seq->_buffer[i],str))
		{
			SOMFree(seq->_buffer[i]);
			seq->_length--;
			if (i < seq->_length)
			{
				seq->_buffer[i]=seq->_buffer[seq->_length];
			}
			else
			{
				if (!seq->_length)
				{
					seq->_maximum=0;
					SOMFree(seq->_buffer);
					seq->_buffer=0;
				}
			}
		}
		else
		{
			i++;
		}
	}
}

static void SOMDD_write_strings(_IDL_SEQUENCE_string *somThis,Environment *ev,CosStream_StreamIO SOMSTAR stream)
{
	unsigned long i=somThis->_length,k=0;

	CosStream_StreamIO_write_unsigned_long(stream,ev,i);

	while (i-- && !ev->_major)
	{
		CosStream_StreamIO_write_string(stream,ev,somThis->_buffer[k]);
		k++;
	}
}

static ImplRepositoryServer_ImplRepContents *get_named_item(
		ImplRepositoryServerData *somThis,char *impl_id)
{
	RHBSOMUT_KeyDataPtr k=NULL;
	_IDL_SEQUENCE_octet seq={0,0,NULL};

	seq._buffer=(octet *)impl_id;
	seq._length=(unsigned long)strlen(impl_id);
	seq._maximum=(unsigned long)seq._length;

	k=RHBCDR_kds_find(&somThis->data_set,&seq);

	if (k)
	{
		return k->data._value;
	}

	return NULL;
}

SOM_Scope ImplementationDef SOMSTAR SOMLINK implreps_find_impldef(
	ImplRepositoryServer SOMSTAR somSelf,
	Environment *ev,
	/* in */ ImplId implid)
{
	ImplRepositoryServerData *somThis=ImplRepositoryServerGetData(somSelf);
	ImplementationDef SOMSTAR result=NULL;
	SOMDD_MutexRef_begin
	unsigned int i=RHBCDR_kds_count(&somThis->data_set);

	while (i-- && !ev->_major)
	{
		RHBSOMUT_KeyDataPtr k=RHBCDR_kds_get(&somThis->data_set,i);

		if (k)
		{
			ImplRepositoryServer_ImplRepContents *one=k->data._value;

			if (one->impl)
			{
				char *p=ImplementationDef__get_impl_id(one->impl,ev);

				if (p && !ev->_major)
				{
					if (!strcmp(p,implid))
					{
						result=ImplementationDef_somDuplicateReference(one->impl);
					}

					SOMFree(p);

					if (result) 
					{
						break;
					}
				}
			}
		}
	}

	if (!result) 
	{
		if (SOM_TraceLevel)
		{
			somPrintf("implreps_find_impldef(\"%s\") failed, %s:%d\n",
				implid,
				__FILE__,__LINE__);
		}

		RHBOPT_throw_StExcep(ev,INV_OBJREF,BadObjref,YES)
	}

	SOMDD_MutexRef_end

	return result;
}


SOM_Scope ImplementationDef SOMSTAR SOMLINK implreps_find_impldef_by_alias(
	ImplRepositoryServer SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring alias_name)
{
	ImplRepositoryServerData *somThis=ImplRepositoryServerGetData(somSelf);
	ImplementationDef SOMSTAR result=NULL;
	SOMDD_MutexRef_begin
	unsigned int i=RHBCDR_kds_count(&somThis->data_set);

	while (i-- && !ev->_major)
	{
		RHBSOMUT_KeyDataPtr k=RHBCDR_kds_get(&somThis->data_set,i);

		if (k)
		{
			ImplRepositoryServer_ImplRepContents *one=k->data._value;

			if (one->impl)
			{
				char *p=ImplementationDef__get_impl_alias(one->impl,ev);

				if (p && !ev->_major)
				{
					if (!strcmp(p,alias_name))
					{
						result=ImplementationDef_somDuplicateReference(one->impl);
					}

					SOMFree(p);

					if (result) break;
				}
			}
		}
	}

	if (!result) 
	{
		if (SOM_TraceLevel)
		{
			somPrintf("implreps_find_impldef_by_alias(\"%s\") failed, %s:%d\n",
				alias_name,
				__FILE__,__LINE__);
		}

		RHBOPT_throw_StExcep(ev,BAD_PARAM,BadParm,YES)
	}

	SOMDD_MutexRef_end

	return result;
}

RHBOPT_cleanup_begin(implreps_find_all_impldefs_cleanup,pv)

	_IDL_SEQUENCE_ImplementationDef *data=pv;

	if (data->_buffer)
	{
		while (data->_length--)
		{
			ImplementationDef SOMSTAR obj=data->_buffer[data->_length];
			data->_buffer[data->_length]=NULL;
			if (obj)
			{
				ImplementationDef_somRelease(obj);
			}
		}

		SOMFree(data->_buffer);
		data->_buffer=NULL;
	}

RHBOPT_cleanup_end

SOM_Scope ORBStatus SOMLINK implreps_find_all_impldefs(
	ImplRepositoryServer SOMSTAR somSelf,
	Environment *ev,
	/* out */ _IDL_SEQUENCE_ImplementationDef *outdefs)
{
	ImplRepositoryServerData *somThis=ImplRepositoryServerGetData(somSelf);
	_IDL_SEQUENCE_ImplementationDef * RHBOPT_volatile outimpldefs=outdefs;
	_IDL_SEQUENCE_ImplementationDef data={0,0,NULL};

	if (ev->_major) return SOMDERROR_UnknownError;

	SOMDD_MutexRef_begin
	RHBOPT_cleanup_push(implreps_find_all_impldefs_cleanup,&data);

	data._maximum=RHBCDR_kds_count(&somThis->data_set);

	if (data._maximum)
	{
		unsigned int i=data._maximum;
		data._buffer=SOMMalloc(i * sizeof(data._buffer[0]));
		
		while (i-- && !ev->_major)
		{
			RHBSOMUT_KeyDataPtr k=RHBCDR_kds_get(&somThis->data_set,i);

			if (k)
			{
				ImplRepositoryServer_ImplRepContents *one=k->data._value;

				if (one->impl)
				{
					ImplementationDef SOMSTAR o=ImplementationDef_somDuplicateReference(one->impl);
					char *alias=ImplementationDef__get_impl_alias(o,ev);

					if (alias && !ev->_major)
					{
						data._buffer[data._length++]=o;
						SOMFree(alias);
					}
					else
					{
						ImplementationDef_somRelease(o);
					}
				}
			}
		}
	}

	if (outimpldefs && data._length && !ev->_major)
	{
		*outimpldefs=data;
		data._length=0;
		data._buffer=NULL;
		data._maximum=0;
	}

	RHBOPT_cleanup_pop();
	SOMDD_MutexRef_end

	return 0;
}

static void delete_one_impl(ImplRepositoryServer_ImplRepContents *somThis)
{
	if (somThis->active_impl)
	{
		SOMObject SOMSTAR o=somThis->active_impl;
		somThis->active_impl=0;
		if (o) somReleaseObjectReference(o);
	}
	if (somThis->impl)
	{
		somReleaseObjectReference(somThis->impl);
		somThis->impl=NULL;
	}
	if (somThis->impl_id)
	{
		SOMFree(somThis->impl_id);
		somThis->impl_id=NULL;
	}
	if (somThis->classes._length)
	{
		while (somThis->classes._length--)
		{
			SOMFree(somThis->classes._buffer[somThis->classes._length]);
			somThis->classes._buffer[somThis->classes._length]=0;
		}
	}
	if (somThis->classes._buffer)
	{
		SOMFree(somThis->classes._buffer);
		somThis->classes._buffer=NULL;
	}
	SOMFree(somThis);
}

struct SOMDD_copy_from_remote_impl
{
	ORBStatus status;
	somStream_MemoryStreamIO SOMSTAR io;
	SOMDObject SOMSTAR ref;
};

RHBOPT_cleanup_begin(SOMDD_copy_from_remote_impl_cleanup,pv)

	struct SOMDD_copy_from_remote_impl *data=pv;
	Environment ev;
	SOM_InitEnvironment(&ev);
	if (data->ref) SOMDObject_somRelease(data->ref);
	if (data->io) SOMDServer_somdDeleteObj(SOMD_ServerObject,&ev,data->io);
	SOM_UninitEnvironment(&ev);

RHBOPT_cleanup_end

static ORBStatus SOMDD_copy_from_remote_impl(
		ImplementationDef SOMSTAR local,
		Environment *ev,
		SOMDObject SOMSTAR remote)
{
	struct SOMDD_copy_from_remote_impl data={0,NULL,NULL};
	/* first get a remoteable stream */

	RHBOPT_cleanup_push(SOMDD_copy_from_remote_impl_cleanup,&data);

	data.io=SOMDServer_somdCreateObj(SOMD_ServerObject,ev,"somStream::MemoryStreamIO",NULL);
	data.ref=SOMDServer_somdRefFromSOMObj(SOMD_ServerObject,ev,data.io);

	if (!ev->_major)
	{
		ImplementationDef_externalize_to_stream(remote,ev,data.ref);

		if (!ev->_major)
		{
			ImplementationDef_internalize_from_stream(local,ev,data.ref,0);
		}
	}

	if (ev->_major) data.status=SOMDERROR_BadEnvironment;

	RHBOPT_cleanup_pop();

	return data.status;
}

static ImplRepositoryServer_ImplRepContents * SOMDD_create_ImplRepContents(void)
{
	ImplRepositoryServer_ImplRepContents *somThis=SOMMalloc(sizeof(*somThis));

	memset(somThis,0,sizeof(*somThis));

	somThis->state=ImplRepositoryServer_Server_Passive;

	return somThis;
}

SOM_Scope ORBStatus SOMLINK implreps_add_impldef(
	ImplRepositoryServer SOMSTAR somSelf,
	Environment *ev,
	/* in */ ImplementationDef SOMSTAR impldef)
{
	ImplRepositoryServerData *somThis=ImplRepositoryServerGetData(somSelf);
	ImplRepositoryServer_ImplRepContents *item=NULL;
	SOMClass SOMSTAR somdClass=NULL;
	boolean remote=0;

	if (!impldef)
	{
		RHBOPT_throw_StExcep(ev,INV_OBJREF,BadObjref,NO);

		return SOMDERROR_BadParm;
	}

	SOMDD_MutexRef_begin
	somdClass=somNewClassReference(SOMDObject);

	remote=SOMObject_somIsA(impldef,somdClass);

	somReleaseClassReference(somdClass);

	if (remote)
	{
#ifdef somNewObject
		ImplementationDef SOMSTAR local=somNewObject(ImplementationDef);
#else
		ImplementationDef SOMSTAR local=ImplementationDefNew();
#endif
		SOMDD_copy_from_remote_impl(local,ev,impldef);

		if (ev->_major)
		{
			ImplementationDef_somRelease(local);

			return SOMDERROR_BadEnvironment;
		}

		impldef=local;
	}
	else
	{
		impldef=ImplementationDef_somDuplicateReference(impldef);
	}

	item=SOMDD_create_ImplRepContents();

	item->impl=impldef;
	item->impl_id=ImplementationDef__get_impl_id(impldef,ev);

	item->data.data._type=0;
	item->data.data._value=item;

	item->data.key._length=(unsigned long)strlen(item->impl_id);
	item->data.key._maximum=(unsigned long)strlen(item->impl_id);
	item->data.key._buffer=(octet *)item->impl_id;

	item->classes._length=0;
	item->classes._maximum=0;
	item->classes._buffer=NULL;

	if (RHBCDR_kds_add(&somThis->data_set,&item->data))
	{
		somThis->implrep_changed=1;
	}
	else
	{
/*		somPrintf("failed to add this one\n");*/

		delete_one_impl(item);

		RHBOPT_throw_StExcep(ev,INV_IDENT,BadParm,NO);
	}

	SOMDD_MutexRef_end

	return 0;
}

SOM_Scope ORBStatus SOMLINK implreps_update_impldef(
	ImplRepositoryServer SOMSTAR somSelf,
	Environment *ev,
	/* in */ ImplementationDef SOMSTAR impldef)
{
	ImplRepositoryServerData *somThis=ImplRepositoryServerGetData(somSelf);
	char *id=NULL;
	RHBSOMUT_KeyDataPtr k=NULL;
	_IDL_SEQUENCE_octet seq={0,0,NULL};

	if (ev->_major)
	{
		return SOMDERROR_BadEnvironment;
	}

	if (!impldef)
	{
		RHBOPT_throw_StExcep(ev,BAD_PARAM,UnexpectedNULL,NO);

		return SOMDERROR_UnexpectedNULL;
	}

	id=ImplementationDef__get_impl_id(impldef,ev);

	if (ev->_major)
	{
		return SOMDERROR_BadEnvironment;
	}

	if (!id)
	{
		RHBOPT_throw_StExcep(ev,BAD_PARAM,UnexpectedNULL,NO);

		return SOMDERROR_UnexpectedNULL;
	}

	seq._buffer=(octet *)id;
	seq._length=(unsigned long)strlen(id);
	seq._maximum=seq._length;

	k=RHBCDR_kds_find(&somThis->data_set,&seq);

	SOMFree(id);

	id=NULL;

	if (!k)
	{
		RHBOPT_throw_StExcep(ev,BAD_PARAM,UnexpectedNULL,NO);

		return SOMDERROR_UnexpectedNULL;
	}
	else
	{
		ImplRepositoryServer_ImplRepContents *one=k->data._value;
		_IDL_SEQUENCE_octet stream_data={0,0,NULL};
		somStream_StreamIO SOMSTAR stream=NULL;

#ifdef somNewObject
		stream=somNewObject(somStream_MemoryStreamIO);
#else
		stream=somStream_MemoryStreamIONew();
#endif
		
		if (!ev->_major)
		{
			ImplementationDef_externalize_to_stream(impldef,ev,stream);
		}

		if (!ev->_major)
		{
			stream_data=somStream_StreamIO_get_buffer(stream,ev);
		}

		somReleaseObjectReference(stream);

		if (!ev->_major)
		{
#ifdef somNewObject
			stream=somNewObject(somStream_MemoryStreamIO);
#else
			stream=somStream_MemoryStreamIONew();
#endif

			somStream_MemoryStreamIO_set_buffer(stream,ev,&stream_data);

			SOMFree(stream_data._buffer);

			if (!ev->_major)
			{
				if (one->impl)
				{
					ImplementationDef_internalize_from_stream(one->impl,ev,stream,NULL);

					somThis->implrep_changed=1;
				}
			}
		
			somReleaseObjectReference(stream);
		}
	}

	return 0;
}

SOM_Scope ORBStatus SOMLINK implreps_delete_impldef(
	ImplRepositoryServer SOMSTAR somSelf,
	Environment *ev,
	/* in */ ImplId implid)
{
	ImplRepositoryServerData *somThis=ImplRepositoryServerGetData(somSelf);
	RHBSOMUT_KeyDataPtr k=NULL;
	_IDL_SEQUENCE_octet seq={0,0,NULL};

	SOMDD_MutexRef_begin

	seq._buffer=(octet *)implid;
	seq._length=(unsigned long)strlen(implid);
	seq._maximum=seq._length;

	k=RHBCDR_kds_find(&somThis->data_set,&seq);

	if (k)
	{
		RHBCDR_kds_remove(&somThis->data_set,k);
		delete_one_impl(k->data._value);
		somThis->implrep_changed=1;
	}
	else
	{
		RHBOPT_throw_StExcep(ev,BAD_PARAM,BadParm,YES)
	}

	SOMDD_MutexRef_end

	return 0;
}

SOM_Scope _IDL_SEQUENCE_ImplementationDef SOMLINK implreps_find_impldef_by_class(
	ImplRepositoryServer SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring classname)
{
	ImplRepositoryServerData *somThis=ImplRepositoryServerGetData(somSelf);
	_IDL_SEQUENCE_ImplementationDef seq={0,0,NULL};
	SOMDD_MutexRef_begin
	unsigned int i=RHBCDR_kds_count(&somThis->data_set);

	seq._maximum=i;

	if (i)
	{
		seq._buffer=SOMMalloc(sizeof(*seq._buffer)*i);

		while (i--)
		{
			RHBSOMUT_KeyDataPtr k=RHBCDR_kds_get(&somThis->data_set,i);

			if (k)
			{
				ImplRepositoryServer_ImplRepContents *one=k->data._value;

				if (SOMDD_has_string(&one->classes,classname))
				{
					if (one->impl)
					{
						seq._buffer[seq._length]=
								ImplementationDef_somDuplicateReference(one->impl);
						seq._length++;
					}
				}
			}
		}

		if (!seq._length)
		{
			SOMFree(seq._buffer);
			seq._buffer=0;
			seq._maximum=0;
		}
	}

	SOMDD_MutexRef_end

	RHBOPT_unused(ev)

	return seq;
}

SOM_Scope void SOMLINK implreps_add_class_to_impldef(
	ImplRepositoryServer SOMSTAR somSelf,
	Environment *ev,
	/* in */ ImplId implid,
	/* in */ corbastring classname)
{
	ImplRepositoryServerData *somThis=ImplRepositoryServerGetData(somSelf);
	SOMDD_MutexRef_begin
	ImplRepositoryServer_ImplRepContents *one=get_named_item(somThis,implid);

	if (one && classname)
	{
		SOMDD_add_string(&one->classes,classname);

		somThis->implrep_changed=1;
	}
	else
	{
		RHBOPT_throw_StExcep(ev,BAD_PARAM,BadParm,YES);
	}

	SOMDD_MutexRef_end
}

SOM_Scope ORBStatus SOMLINK implreps_find_all_aliases(
	ImplRepositoryServer SOMSTAR somSelf,
	Environment *ev,
	/* out */ _IDL_SEQUENCE_string *impl_aliases)
{
	ImplRepositoryServerData *somThis=ImplRepositoryServerGetData(somSelf);

	SOMDD_MutexRef_begin

	impl_aliases->_length=0;
	impl_aliases->_buffer=0;
	impl_aliases->_maximum=RHBCDR_kds_count(&somThis->data_set);

	if (impl_aliases->_maximum)
	{
		unsigned int i=impl_aliases->_maximum;
		impl_aliases->_buffer=
			SOMMalloc(i * sizeof(*impl_aliases->_buffer));
		
		while (i--)
		{
			RHBSOMUT_KeyDataPtr k=RHBCDR_kds_get(&somThis->data_set,i);

			if (k)
			{
				ImplRepositoryServer_ImplRepContents *one=k->data._value;
				char *p=ImplementationDef__get_impl_alias(one->impl,ev);
				if ((ev->_major==NO_EXCEPTION)&&(p)&&(*p))
				{
					impl_aliases->_buffer[impl_aliases->_length++]=p;
				}
			}
		}

		/* clear return data if nothing returned or an error occured */

		if ((ev->_major!=NO_EXCEPTION) || (!impl_aliases->_length))
		{
			char **h=impl_aliases->_buffer;
			unsigned int i=impl_aliases->_length;

			impl_aliases->_length=0;
			impl_aliases->_maximum=0;
			impl_aliases->_buffer=NULL;

			while (i--)
			{
				char *p=h[i];
				if (p) SOMFree(p);
			}

			if (h) SOMFree(h);
		}
	}

	SOMDD_MutexRef_end

	return 0;
}

#ifdef USE_THREADS
RHBOPT_cleanup_begin(implreps_thread_cleanup,pv)

	RHBRequestList *data=pv;
	data->isAlive=0;
#ifdef USE_PTHREADS
	pthread_cond_signal(&data->p_dead);
#else
	if (!SetEvent(data->h_dead)) implreps_trap();
#endif
RHBOPT_cleanup_end

static 
#if defined(USE_PTHREADS) 
void *
#else
DWORD CALLBACK
#endif
implreps_thread(void *pv)
{
	RHBRequestList *data=pv;
	Environment ev;
	SOM_InitEnvironment(&ev);

	SOMDD_MutexRef_begin

	RHBOPT_cleanup_push(implreps_thread_cleanup,data);

	while (!data->doQuit)
	{
		if (data->list)
		{
			SOMDD_MutexRef_pause

/*			somPrintf("handling requests....\n");*/

			while (!ImplRepositoryServer_handle_requests(
				data->somSelf,&ev,0))
			{
/*				somPrintf("...and again...\n");*/
			}

			SOMDD_MutexRef_resume
		}
		else
		{
/*			somPrintf("waiting....\n");*/

#ifdef USE_PTHREADS
			pthread_cond_wait(&data->p_event,SOMDD_MutexRef_ref);
#else
			SOMDD_MutexRef_pause
			WaitForSingleObjectEx(data->h_event,INFINITE,1);
			SOMDD_MutexRef_resume
#endif
		}
	}

	RHBOPT_cleanup_pop();

	SOMDD_MutexRef_end

	return 0;
}
#endif

#if defined(_WIN32) && defined(_DEBUG) && defined(USE_THREADS)
static DWORD CALLBACK debug_thread(void *pv)
{
ImplRepositoryServerData *somThis=pv;

	while (somThis)
	{
		int id=MessageBox(GetDesktopWindow(),"Click OK to debug",__FILE__,MB_OKCANCEL);

		switch (id)
		{
		case IDOK:
			implreps_trap();
			break;
		default:
			return 0;
		}

		SOMDD_MutexRef_begin

		{
			struct RHBServerRequest *sr=somThis->server_request_list.list;

			while (sr)
			{
				sr=sr->next;
			}
		}
		
		{
			struct RHBRequest *r=somThis->request_list.list;

			while (r)
			{
				RHBOPT_ASSERT(r->request);

				SOMObject_somDumpSelf(r->request,0);

				r=r->next;
			}
		}

		SOMDD_MutexRef_end
	}

	return 0;
}
#endif

SOM_Scope void SOMLINK implreps_somInit(
	ImplRepositoryServer SOMSTAR somSelf)
{
	ImplRepositoryServerData *somThis=ImplRepositoryServerGetData(somSelf);
	Environment ev;
	IOP_CodecFactory SOMSTAR factory=NULL;

	ImplRepositoryServer_parent_ImplRepository_somInit(somSelf);

	SOM_InitEnvironment(&ev);
	RHBCDR_kds_init(&somThis->data_set);

#ifdef USE_THREADS
	somThis->request_list.somSelf=somSelf;
	somThis->request_list.isAlive=1;
	#ifdef USE_PTHREADS
		pthread_cond_init(&somThis->server_request_list.p_event,RHBOPT_pthread_condattr_default);
		pthread_cond_init(&somThis->request_list.p_event,RHBOPT_pthread_condattr_default);
		pthread_cond_init(&somThis->request_list.p_dead,RHBOPT_pthread_condattr_default);
		pthread_create(
				&somThis->request_list.tid,
				RHBOPT_pthread_attr_default,
				implreps_thread,
				&somThis->request_list);
	#else
		somThis->server_request_list.h_event=CreateEvent(NULL,0,0,NULL);
		somThis->request_list.h_event=CreateEvent(NULL,0,0,NULL);
		somThis->request_list.h_dead=CreateEvent(NULL,0,0,NULL);
		somThis->request_list.h_thread=CreateThread(NULL,0,
			implreps_thread,&somThis->request_list,0,
			&somThis->request_list.tid);
	#endif
#endif
				
	factory=ORB_resolve_initial_references(SOMD_ORBObject,&ev,
			ORB_initial_CodecFactory);

	if (factory && (ev._major==NO_EXCEPTION))
	{
		IOP_Encoding enc={1,0};
		somThis->codec=IOP_CodecFactory_create_codec(factory,&ev,&enc);
		somReleaseObjectReference(factory);
	}

	SOM_UninitEnvironment(&ev);

#if defined(_WIN32) && defined(_DEBUG) && defined(USE_THREADS)
	{
		DWORD tid;
		HANDLE h=CreateThread(NULL,0,debug_thread,somThis,0,&tid);
		if (h) CloseHandle(h);
	}
#endif
}

SOM_Scope void SOMLINK implreps_somUninit(
	ImplRepositoryServer SOMSTAR somSelf)
{
	Environment ev;
	ImplRepositoryServerData *somThis=ImplRepositoryServerGetData(somSelf);
	SOMDD_MutexRef_begin

#ifdef USE_THREADS
	while (somThis->waiting_room.list)
	{
		somPrintf("items still in waiting room\n");
		SOMDD_NotifyWaitingRoom(&somThis->waiting_room,SOMDD_MutexRef_ref);

		if (somThis->waiting_room.list)
		{	
			SOMDD_WaitInWaitingRoom(&somThis->waiting_room,SOMDD_MutexRef_ref);
		}
	}
#endif

#ifdef USE_THREADS
	somThis->request_list.doQuit=1;
#ifdef USE_PTHREADS
	pthread_cond_signal(&somThis->request_list.p_event);
#else
	if (!SetEvent(somThis->request_list.h_event)) implreps_trap();
#endif

	while (somThis->request_list.isAlive)
	{
#ifdef USE_PTHREADS
		pthread_cond_wait(&somThis->request_list.p_dead,SOMDD_MutexRef_ref);
#else
		SOMDD_MutexRef_pause
		WaitForSingleObjectEx(somThis->request_list.h_dead,INFINITE,1);
		SOMDD_MutexRef_resume
#endif
	}

#ifdef USE_PTHREADS
	{
		void *pv=NULL;
		pthread_join(somThis->request_list.tid,&pv);
		pthread_cond_destroy(&somThis->request_list.p_event);
		pthread_cond_destroy(&somThis->request_list.p_dead);
		pthread_cond_destroy(&somThis->server_request_list.p_event);
	}
#else
	WaitForSingleObjectEx(somThis->request_list.h_thread,INFINITE,1);
	if (!CloseHandle(somThis->request_list.h_thread)) implreps_trap();
	if (!CloseHandle(somThis->request_list.h_dead)) implreps_trap();
	if (!CloseHandle(somThis->request_list.h_event)) implreps_trap();
	if (!CloseHandle(somThis->server_request_list.h_event)) implreps_trap();
#endif

#endif

	SOM_InitEnvironment(&ev);

	if (SOMD_ImplRepObject==somSelf)
	{
		SOMD_ImplRepObject=NULL;
	}

	while (RHBCDR_kds_count(&somThis->data_set))
	{
		RHBSOMUT_KeyDataPtr p=RHBCDR_kds_get(&somThis->data_set,0);
		RHBCDR_kds_remove(&somThis->data_set,p);
		delete_one_impl(p->data._value);
	} 

	if (somThis->implrep_filename)
	{
		SOMFree(somThis->implrep_filename);
		somThis->implrep_filename=NULL;
	}

	SOMDD_MutexRef_end

	if (SOMMSingleInstanceClassData.sommSingleInstanceFreed)
	{
		SOMMSingleInstance_sommSingleInstanceFreed(somSelf->mtab->classObject,&ev,somSelf);
	}

	if (somThis->codec)
	{
		somReleaseObjectReference(somThis->codec);
		somThis->codec=NULL;
	}

	ImplRepositoryServer_parent_ImplRepository_somUninit(somSelf);

	SOM_UninitEnvironment(&ev);
}

SOM_Scope ORBStatus SOMLINK implreps_add_class_with_properties(
	ImplRepositoryServer SOMSTAR somSelf,
	Environment *ev,
	/* in */ ImplId implid,
	/* in */ corbastring classname,
	/* in */ ImplRepository_PVList *pvl)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(implid)
	RHBOPT_unused(classname)
	RHBOPT_unused(pvl)
	return 0;
}


SOM_Scope ORBStatus SOMLINK implreps_add_class_to_all(
	ImplRepositoryServer SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring classname)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(classname)
	return 0;
}


SOM_Scope _IDL_SEQUENCE_string SOMLINK implreps_find_classes_by_impldef(
	ImplRepositoryServer SOMSTAR somSelf,
	Environment *ev,
	/* in */ ImplId implid)
{
	_IDL_SEQUENCE_string seq={0,0,NULL};
	ImplRepositoryServerData *somThis=ImplRepositoryServerGetData(somSelf);
	SOMDD_MutexRef_begin
	ImplRepositoryServer_ImplRepContents *one=get_named_item(somThis,implid);

	if (one)
	{
		if (one->classes._length)
		{
			unsigned int i;

			seq._buffer=SOMMalloc(one->classes._length * sizeof(char *));

			i=0;

			while (i < one->classes._length)
			{
				seq._buffer[i]=SOMDD_dupl_string(one->classes._buffer[i]);
				i++;
			}
			seq._length=i;
			seq._maximum=i;
		}
	}
	else
	{
		RHBOPT_throw_StExcep(ev,BAD_PARAM,BadParm,YES);
	}

	SOMDD_MutexRef_end

	return seq;
}

SOM_Scope void SOMLINK implreps_remove_class_from_impldef(
	ImplRepositoryServer SOMSTAR somSelf,
	Environment *ev,
	/* in */ ImplId implid,
	/* in */ corbastring classname)
{
	ImplRepositoryServerData *somThis=ImplRepositoryServerGetData(somSelf);
	SOMDD_MutexRef_begin
	ImplRepositoryServer_ImplRepContents *one=get_named_item(somThis,implid);

	if (one && classname)
	{
		SOMDD_remove_string(&one->classes,classname);
		somThis->implrep_changed=1;
	}
	else
	{
		RHBOPT_throw_StExcep(ev,BAD_PARAM,BadParm,YES)
	}

	SOMDD_MutexRef_end
}

SOM_Scope void SOMLINK implreps_remove_class_from_all(
	ImplRepositoryServer SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring classname)
{
	ImplRepositoryServerData *somThis=ImplRepositoryServerGetData(somSelf);

	RHBOPT_unused(ev)

	if (classname)
	{
		SOMDD_MutexRef_begin
		unsigned int i=RHBCDR_kds_count(&somThis->data_set);
		while (i--)
		{
			RHBSOMUT_KeyDataPtr p;
			ImplRepositoryServer_ImplRepContents *one;

			p=RHBCDR_kds_get(&somThis->data_set,i);

			one=p->data._value;

			if (SOMDD_has_string(&one->classes,classname))
			{
				SOMDD_remove_string(&one->classes,classname);

				somThis->implrep_changed=1;
			}
		}

		SOMDD_MutexRef_end
	}
}


struct read_one_impl
{
	ImplRepositoryServer_ImplRepContents *somThis;
	SOMClass SOMSTAR cls;
	somId id;
	char *clsName;
};

RHBOPT_cleanup_begin(read_one_impl_cleanup,pv)

struct read_one_impl *data=pv;

	if (data->clsName) SOMFree(data->clsName);
	if (data->id) SOMFree(data->id);
	if (data->cls) somReleaseClassReference(data->cls);

	if (data->somThis)
	{
		if (data->somThis->impl)
		{
			ImplementationDef_somRelease(data->somThis->impl);
		}
		if (data->somThis->impl_id)
		{
			SOMFree(data->somThis->impl_id);
		}
		SOMFree(data->somThis);
	}

RHBOPT_cleanup_end

static ImplRepositoryServer_ImplRepContents * read_one_impl(
		SOMClassMgr SOMSTAR mgr,
		CosStream_StreamIO SOMSTAR str,
		Environment *ev)
{
struct read_one_impl data={NULL,NULL,NULL,NULL};
ImplRepositoryServer_ImplRepContents *RHBOPT_volatile result=NULL;

	RHBOPT_cleanup_push(read_one_impl_cleanup,&data);

	data.clsName=CosStream_StreamIO_read_string(str,ev);

	if (!ev->_major)
	{
		if (!data.clsName)
		{
			data.clsName=SOMDD_dupl_string("ImplementationDef");
		}

		data.id=somIdFromString(data.clsName);

		data.cls=SOMClassMgr_somFindClass(mgr,data.id,0,0);

		if (data.cls)
		{
			data.somThis=SOMDD_create_ImplRepContents();

			data.somThis->impl=SOMClass_somNew(data.cls);

			ImplementationDef_internalize_from_stream(
				data.somThis->impl,
				ev,
				str,
				0);

			if (!ev->_major)
			{
				char *p=NULL;

				data.somThis->data.data._type=NULL;
				data.somThis->data.data._value=data.somThis;

				p=ImplementationDef__get_impl_id(data.somThis->impl,ev);
				data.somThis->impl_id=p;

				data.somThis->data.key._length=(unsigned long)strlen(p);
				data.somThis->data.key._maximum=(unsigned long)strlen(p);
				data.somThis->data.key._buffer=(octet *)p;

				SOMDD_read_strings(&data.somThis->classes,ev,str);
			}
		}
	}

	if (!ev->_major)
	{
		result=data.somThis;
		data.somThis=NULL;
	}

	RHBOPT_cleanup_pop();

	return result;
}

SOM_Scope void SOMLINK implreps_init_from_stream(
	ImplRepositoryServer SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosStream_StreamIO SOMSTAR str)
{
	ImplRepositoryServerData *somThis=ImplRepositoryServerGetData(somSelf);
	SOMDD_MutexRef_begin
	unsigned long l=CosStream_StreamIO_read_unsigned_long(str,ev);

	if (l && !ev->_major)
	{
		SOMClassMgr SOMSTAR mgr=somGetClassManagerReference();

		while (l-- && !ev->_major)
		{
			ImplRepositoryServer_ImplRepContents *one=read_one_impl(mgr,str,ev);

			if (one && !ev->_major)
			{
				RHBCDR_kds_add(&somThis->data_set,&one->data);
			}
		}

		somReleaseClassManagerReference(mgr);
	}

	somThis->implrep_changed=0;

	SOMDD_MutexRef_end
}

SOM_Scope void SOMLINK implreps_save_to_stream(
	ImplRepositoryServer SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosStream_StreamIO SOMSTAR stream)
{
	ImplRepositoryServerData *somThis=ImplRepositoryServerGetData(somSelf);
	SOMDD_MutexRef_begin
	unsigned long l=RHBCDR_kds_count(&somThis->data_set);
	unsigned long al=0;

	while (l--)
	{
		RHBSOMUT_KeyDataPtr k=RHBCDR_kds_get(&somThis->data_set,l);

		if (k)
		{
			ImplRepositoryServer_ImplRepContents *one=k->data._value;

			if (!one->ephemeral)
			{
				al++;
			}
		}
	}

	CosStream_StreamIO_write_unsigned_long(stream,ev,al);

	l=0;

	while (al--)
	{
		RHBSOMUT_KeyDataPtr k=RHBCDR_kds_get(&somThis->data_set,l);

		l++;

		if (k)
		{
			ImplRepositoryServer_ImplRepContents *one=k->data._value;

			if (!one->ephemeral)
			{
				char *p=ImplementationDef_somGetClassName(one->impl);

				CosStream_StreamIO_write_string(stream,ev,p);

				if (p)
				{
					SOMClass SOMSTAR cls=somNewClassReference(SOMDClientProxy);

					if (SOMObject_somIsA(one->impl,cls))
					{
						SOMFree(p);
					}

					somReleaseClassReference(cls);
				}
				else
				{
					RHBOPT_ASSERT(!"can't save object in repository\n");
					SOMObject_somDumpSelf(one->impl,0);
				}

				ImplementationDef_externalize_to_stream(one->impl,ev,
						stream);

				SOMDD_write_strings(&one->classes,ev,stream);
			}
			else
			{
				CosStream_StreamIO_write_string(stream,ev,0);
			}
		}
	}

	somThis->implrep_changed=0;

	SOMDD_MutexRef_end
}

SOM_Scope boolean SOMLINK implreps__get_implrep_changed(
	ImplRepositoryServer SOMSTAR somSelf,
	Environment *ev)
{
	ImplRepositoryServerData *somThis=ImplRepositoryServerGetData(somSelf);
	RHBOPT_unused(ev)

	return somThis->implrep_changed;
}

struct implreps_get_active_impldef_data
{
	RHBSOMUT_KeyDataPtr k;
	ImplRepositoryServer_ImplRepContents *one;
	char *prog;
	char *alias;
	boolean started;
	boolean starting;
#ifdef USE_THREADS
	SOMDD_MutexRef m;
#endif
};

RHBOPT_cleanup_begin(impldefs_get_active_impldef_cleanup,pv)

	struct implreps_get_active_impldef_data *data=pv;

	if (!data->started)
	{
		if (data->one)
		{
			if (data->one->state==ImplRepositoryServer_Server_Activating)
			{
				if (data->starting)
				{
					RHBProcess *proc=data->one->child_process;
					data->one->child_process=NULL;
					data->one->state=ImplRepositoryServer_Server_Passive;
					if (proc) RHBProcess_Release(proc);
				}
			}
		}
	}

#ifdef USE_THREADS
	if (data->m) SOMDD_ReleaseGlobalMutex(data->m);
#endif
	if (data->alias) SOMFree(data->alias);
	if (data->prog) SOMFree(data->prog);

RHBOPT_cleanup_end

SOM_Scope ImplementationDef SOMSTAR SOMLINK implreps_get_active_impldef(
	ImplRepositoryServer SOMSTAR somSelf,
	Environment *ev,
	/* in */ char *impl_id,
	SOMObject SOMSTAR somref)
{
	ImplRepositoryServerData * RHBOPT_volatile somThis=ImplRepositoryServerGetData(somSelf);
	struct implreps_get_active_impldef_data data={NULL,NULL,NULL,NULL,0,0};
	ImplementationDef SOMSTAR RHBOPT_volatile retval=NULL;
#ifndef USE_THREADS
	static ImplRepositoryServer_ImplRepContents *any_starting;
#endif

/*	pthread_exit(0);*/

	if (ev->_major) return NULL;

#ifdef USE_THREADS
	data.m=SOMDD_AcquireGlobalMutex();
	if (!data.m) return NULL;
#endif

	RHBOPT_cleanup_push(impldefs_get_active_impldef_cleanup,&data);

	if (!SOMDD_running)
	{
		RHBOPT_throw_StExcep(ev,OBJ_ADAPTER,SOMDDNotRunning,NO);

		somPrintf("Not starting %s as am shutting down\n",impl_id);
	}
	else
	{
		unsigned long i=(unsigned long)strlen(impl_id);
		_IDL_SEQUENCE_octet seq;
		seq._length=seq._maximum=i;
		seq._buffer=(octet *)impl_id;
		data.k=RHBCDR_kds_find(&somThis->data_set,&seq);
	}

	if (data.k && !ev->_major)
	{
		data.one=data.k->data._value;

		if (data.one->impl)
		{
			if ((!data.one->active_impl)&&!retval)
			{
				if (data.one->ephemeral) 
				{
					RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,ImplNotFound,NO);
				}
				else
				{
					switch (data.one->state)
					{
					case ImplRepositoryServer_Server_Deactivating:
						somPrintf("Process for %s still shutting down ",impl_id);
						RHBProcess_Print(data.one->child_process);
						RHBOPT_throw_StExcep(ev,TRANSIENT,CouldNotStartProcess,NO);
						break;
					case ImplRepositoryServer_Server_Passive:
						{
#ifndef USE_THREADS
							if (any_starting)
							{
								/* already starting another program */

								RHBOPT_throw_StExcep(ev,TRANSIENT,CouldNotStartProcess,NO);
							}
#endif

							if (!ev->_major)
							{
								data.prog=ImplementationDef__get_impl_program(data.one->impl,ev);
								data.alias=ImplementationDef__get_impl_alias(data.one->impl,ev);

								data.one->state=ImplRepositoryServer_Server_Activating;
							}

							if (!ev->_major)
							{
								if (!data.prog) data.prog=SOMDD_dupl_string(sz_somdsvr);

								data.starting=1;

								data.one->child_process=RHBProcessNew(
									somThis->process_mgr,
									ev,
									data.prog,
									data.one->impl_id,
									data.alias
		#ifdef USE_THREADS
									,data.m
		#endif
									);

								if (ev->_major || !data.one->child_process)
								{
									data.one->state=ImplRepositoryServer_Server_Passive;
								}

								data.starting=0;

/*								pthread_exit(0);*/
							}

							if (!ev->_major)
							{
								data.started=1;
								somPrintf("%s started\n",data.one->impl_id);

/*								somPrintf("%s started in process ",data.one->impl_id);
								SOMDD_PrintProcess(data.one->child_process);*/
							}
						}
					}

					while ((data.one->state==ImplRepositoryServer_Server_Activating)&&(!ev->_major))
					{
/*						somPrintf("Waiting for %s to complete activation\n",
								impl_id);
*/
						if (SOMDD_running)
						{
	#ifdef USE_THREADS
							SOMDD_WaitInWaitingRoom(&somThis->waiting_room,data.m);
	#else
							{
								any_starting=data.one;
								SOMOA_execute_next_request(SOMD_SOMOAObject,ev,SOMD_WAIT);
								any_starting=NULL;
							}
	#endif
						}
						else
						{
							somPrintf("Stopped waiting for %s as am shutting down\n",impl_id);
							RHBOPT_throw_StExcep(ev,OBJ_ADAPTER,SOMDDNotRunning,NO);
						}
					}
				}
			}

			if (data.one->active_impl && !retval)
			{
				retval=ImplementationDef_somDuplicateReference(data.one->active_impl);
			}

			if ((!retval) && (!ev->_major))
			{
				RHBOPT_throw_StExcep(ev,
						NO_RESOURCES,
						CouldNotStartProcess,
						NO);
			}
		}
	}

	RHBOPT_cleanup_pop();

	if ((!retval) && (!ev->_major))
	{
		RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,ImplNotFound,NO);
#ifdef _DEBUG
		somPrintf("Returning %s for activation of '%s'\n",
			somExceptionId(ev),
			impl_id);
#endif
	}

	return retval;
}

SOM_Scope void SOMLINK implreps_obituary(
	ImplRepositoryServer SOMSTAR somSelf,
	Environment *ev,
	RHBProcess * process_id,
	long error,
	somToken mtx)
{
	RHBOPT_unused(ev)
	RHBOPT_unused(error)

	if (somSelf)
	{
		ImplRepositoryServerData *somThis=ImplRepositoryServerGetData(somSelf);
		unsigned int i=RHBCDR_kds_count(&somThis->data_set);

		while (i--)
		{
			RHBSOMUT_KeyDataPtr k=RHBCDR_kds_get(&somThis->data_set,i);

			if (k)
			{
				ImplRepositoryServer_ImplRepContents *one=k->data._value;

				if (RHBProcess_Equal(process_id,one->child_process))
				{
					ImplementationDef SOMSTAR active_impl=NULL;
					RHBProcess *process=one->child_process;
					one->child_process=NULL;

					somPrintf("obituary(%s,%ld)\n",
						one->impl_id,
						error);

					RHBProcess_Release(process);
				
					one->state=ImplRepositoryServer_Server_Passive;

					active_impl=one->active_impl;
					one->active_impl=NULL;
					if (active_impl) somReleaseObjectReference(active_impl);

					implreps_changed(somThis,one,mtx);
				}
			}
		}

	#ifdef USE_THREADS
		SOMDD_NotifyWaitingRoom(&somThis->waiting_room,mtx);
	#endif
	}
}

static long SOMDD_children_still_running(ImplRepositoryServerData *somThis
#ifdef USE_THREADS
				  ,SOMDD_MutexRef m
#endif
					)
{
	long z=0;
	unsigned int i=RHBCDR_kds_count(&somThis->data_set);
	while (i--)
	{
		RHBSOMUT_KeyDataPtr k=RHBCDR_kds_get(&somThis->data_set,i);

		if (k)
		{
			ImplRepositoryServer_ImplRepContents *one=k->data._value;

			if (!one->ephemeral)
			{
				if (one->state != ImplRepositoryServer_Server_Passive)
				{
					somPrintf("Server %s is at state %ld ",
						one->impl_id,(long)one->state);

					if (one->child_process)
					{
						RHBProcess_Print(one->child_process);

						z++;
					}
				}
			}
		}
	}
#ifdef USE_THREADS
	RHBOPT_unused(m)
#endif
	return z;
}

struct SOMDD_interrupt_server
{
	SOMDServer SOMSTAR server;
	SOMOA SOMSTAR somoa;
	Request SOMSTAR request_create;
	Request SOMSTAR request_interrupt;
	char *ior;
};

RHBOPT_cleanup_begin(SOMDD_interrupt_server_cleanup,pv)

struct SOMDD_interrupt_server *data=pv;

	if (data->ior) SOMFree(data->ior);
	if (data->request_create) somReleaseObjectReference(data->request_create);
	if (data->request_interrupt) somReleaseObjectReference(data->request_interrupt);
	if (data->server) somReleaseObjectReference(data->server);
	if (data->somoa) somReleaseObjectReference(data->somoa);

RHBOPT_cleanup_end

static boolean SOMDD_interrupt_server(ImplRepositoryServer_ImplRepContents *one,Environment *ev)
{
struct SOMDD_interrupt_server data={NULL,NULL,NULL,NULL,NULL};
_IDL_SEQUENCE_octet *op=NULL;

	if (!one->active_impl) return 0;
	if (ev->_major) return 0;

	RHBOPT_cleanup_push(SOMDD_interrupt_server_cleanup,&data);

	op=ImplementationDef__get_svr_objref(one->active_impl,ev);

	data.server=SOMDD_get_obj_from_ior(op,ev,somddTC_SOMDServer);

/*	data.ior=SOMDD_ior_from_data(op);*/

/*	if (data.ior)*/
	{
/*		data.server=ORB_string_to_object(SOMD_ORBObject,ev,data.ior);*/
	
		if (data.server && !ev->_major)
		{
			NamedValue result={NULL,{NULL,NULL},0,0};
			corbastring hint=NULL,cls="SOMOA";
			result.argument._type=TC_Object;
			result.argument._value=&data.somoa;

			if (!SOMDObject_create_request(data.server,ev,
				0,"somdCreateObj",
				0,&result,&data.request_create,0))
			{
				Request_add_arg(data.request_create,ev,
					"objclass",TC_string,&cls,0,ARG_IN);

				Request_add_arg(data.request_create,ev,
					"hint",TC_string,&hint,0,ARG_IN);

				Request_invoke(data.request_create,ev,0);
			}
		}

		if (data.somoa && !ev->_major)
		{
			NamedValue result={NULL,{NULL,NULL},0,0};
			result.argument._type=TC_void;

			if (!SOMDObject_create_request(data.somoa,ev,
				0,"interrupt_server",
				0,&result,&data.request_interrupt,0))
			{
				Request_invoke(data.request_interrupt,ev,0);
			}
		}
	}

	/* getting COMM failure might indicate line dropped due to shutdown */
	if (ev->_major)
	{
		somExceptionFree(ev);
	}

	RHBOPT_cleanup_pop();

	return 1;
}

SOM_Scope void SOMLINK implreps_kill_all(
	SOMObject SOMSTAR somSelf,
	Environment *ev)
{
	ImplRepositoryServerData *somThis=ImplRepositoryServerGetData(somSelf);
	unsigned int i=0;
	SOMDD_MutexRef_begin
#ifdef USE_THREADS
	SOMDD_NotifyWaitingRoom(&somThis->waiting_room,SOMDD_MutexRef_ref);
#endif
	i=RHBCDR_kds_count(&somThis->data_set);

	while (i--)
	{
		RHBSOMUT_KeyDataPtr k=RHBCDR_kds_get(&somThis->data_set,i);

		if (k)
		{
			ImplRepositoryServer_ImplRepContents *one=k->data._value;

			if (one->state != ImplRepositoryServer_Server_Passive) 
			{
				boolean dead=0;

				if (one->child_process)
				{
					RHBSOM_Trace("killing server...")
					RHBSOM_Trace(one->impl_id)
					
					somPrintf("somdd: killing process ");
					RHBProcess_Print(one->child_process);

					dead=
#ifdef USE_THREADS
					RHBProcess_Kill(one->child_process,SOMDD_MutexRef_ref);
#else
					RHBProcess_Kill(one->child_process);
#endif
				}

				if (!dead)
				{
					somPrintf("Killing server using SOMOA\n");
					SOMDD_MutexRef_pause

					dead=SOMDD_interrupt_server(one,ev);

					if (ev->_major) 
					{
						somPrintf("%s\n",somExceptionId(ev));
						somExceptionFree(ev);
					}
					SOMDD_MutexRef_resume
				}

				if (!dead)
				{
					if (one->child_process)
					{
	#ifdef USE_THREADS
		#if defined(USE_PTHREADS) 
						dead=RHBProcess_Kill(one->child_process,SOMDD_MutexRef_ref) ? 0 : 1;
		#else
						/* should not use terminate process */
						RHBOPT_ASSERT(!"killing server failed");
		#endif
	#else
						dead=RHBProcess_Kill(one->child_process) ? 0 : 1;
	#endif
						if (!dead) 
						{
							ImplementationDef SOMSTAR obj=NULL;
							RHBProcess *child_process=one->child_process;
							one->child_process=NULL;
							somPrintf("somdd: Failed to kill process ");
							RHBProcess_Print(child_process);
							/* if killing failed, then assume it's passive */
							RHBProcess_Release(child_process);

							obj=one->active_impl;
							one->active_impl=NULL;
							if (obj) somReleaseObjectReference(obj);
							one->state=ImplRepositoryServer_Server_Passive;
						}

					}
				}

				if (dead)
				{
					if (one->state==ImplRepositoryServer_Server_Ready)
					{
						one->state=ImplRepositoryServer_Server_Deactivating;
					}
				}
			}
		}
	}

#ifdef USE_THREADS
	i=0;

	if (SOMDD_children_still_running(somThis,SOMDD_MutexRef_ref))
	{
		int k;

		do
		{
			k=SOMDD_children_still_running(somThis,SOMDD_MutexRef_ref);

			if (k)
			{
				i++;
				somPrintf("Waiting for %d running server(s) to stop, #%d\n",
							k,i);

				SOMDD_WaitInWaitingRoom(&somThis->waiting_room,SOMDD_MutexRef_ref);

#ifdef _DEBUG
			/*	if (i > 30) bomb("waiting too long");*/
#endif
			}
		} while (k);
	}

	while (somThis->waiting_room.list)
	{
		somPrintf("Items still in waiting room\n");
		SOMDD_NotifyWaitingRoom(&somThis->waiting_room,SOMDD_MutexRef_ref);
	}
#else
	/* non threaded system should still wait for childprocesses */
	while (SOMDD_children_still_running(somThis))
	{
		somPrintf("waiting for children to die\n");

		SOMOA_execute_next_request(SOMD_SOMOAObject,ev,SOMD_WAIT);
	}
#endif

	SOMDD_MutexRef_end
}

static octet SOMDD_hex_value(char c)
{
	static char SOMDD_u_case[16]="0123456789ABCDEF";
	static char SOMDD_l_case[16]="0123456789abcdef";
	octet i=0;

	while (i < 16)
	{
		if (SOMDD_u_case[i]==c) return i;
		if (SOMDD_l_case[i]==c) return i;

		i++;
	}

	return 0;
}

static ImplementationDef_octet_seqP SOMDD_decode_ior(char *p)
{
	int i=0,j=0;
	ImplementationDef_octet_seqP seq=NULL;

	if (!p) return NULL;

	i=(int)strlen(p);

	if (i < 6)
	{
		return 0;
	}

	if (memcmp(p,"IOR:",4))
	{
		return 0;
	}

	p+=4;
	i-=4;

	seq=SOMMalloc(sizeof(*seq));

	if (!seq) return 0;

	i>>=1;
	seq->_length=i;
	seq->_maximum=i;
	seq->_buffer=SOMMalloc(i);

	if (!seq->_buffer)
	{
		SOMFree(seq);
		return 0;
	}

	while (i--)
	{
		octet o=SOMDD_hex_value(*p++)<<4;
		o+=SOMDD_hex_value(*p++);
		seq->_buffer[j++]=o;
	}

	return seq;
}

SOM_Scope void SOMLINK implreps_impl_up(
	ImplRepositoryServer SOMSTAR somSelf,
	Environment *ev,
	char *impl_id,
	char *impl_server_ior,
	somToken mutex)
{
	ImplRepositoryServerData *somThis=ImplRepositoryServerGetData(somSelf);
	RHBSOMUT_KeyDataPtr k=0;
#ifdef USE_THREADS
	SOMDD_MutexRef m=mutex;

	if (!m) 
	{
		RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,UnexpectedNULL,NO);

		return;
	}
#endif

	if (impl_id)
	{
		if (*impl_id)
		{
			_IDL_SEQUENCE_octet seq;
			seq._buffer=(octet *)impl_id;
			seq._length=(unsigned long)strlen(impl_id);
			seq._maximum=seq._length;
	
			k=RHBCDR_kds_find(&somThis->data_set,&seq);
		}
	}

	if (SOMDD_running && !k)
	{
		ImplRepositoryServer_ImplRepContents *one=SOMDD_create_ImplRepContents();

		one->ephemeral=1;
		one->impl_id=SOMDD_dupl_string(impl_id);

#ifdef somNewObject
		one->impl=somNewObject(ImplementationDef);
#else
		one->impl=ImplementationDefNew();
#endif

		ImplementationDef__set_impl_id(one->impl,ev,one->impl_id);
		one->data.data._type=0;
		one->data.data._value=one;
		one->data.key._length=(unsigned long)strlen(one->impl_id);
		one->data.key._maximum=(unsigned long)strlen(one->impl_id);
		one->data.key._buffer=(octet *)one->impl_id;
		RHBCDR_kds_add(&somThis->data_set,&one->data);
		k=&one->data;
	}

	if (k)
	{
		ImplRepositoryServer_ImplRepContents *one=k->data._value;
		ImplementationDef SOMSTAR old_impl=one->active_impl;
		one->active_impl=NULL;

		if (SOMDD_running)
		{
			ImplementationDef_octet_seqP iors=SOMDD_decode_ior(impl_server_ior);

			if (iors)
			{
#ifdef somNewObject
				ImplementationDef SOMSTAR active_impl=somNewObject(ImplementationDef);
#else
				ImplementationDef SOMSTAR active_impl=ImplementationDefNew();
#endif
				ImplementationDef__set_impl_id(active_impl,ev,one->impl_id);
				ImplementationDef__set_svr_objref(active_impl,ev,iors);


				one->active_impl=active_impl;

				one->state=ImplRepositoryServer_Server_Ready;
			}
		}

		if (old_impl) 
		{
			ImplementationDef_somRelease(old_impl);
		}

		if ((one->state==ImplRepositoryServer_Server_Activating)||
				ev->_major)
		{
			one->state=ImplRepositoryServer_Server_Deactivating;
			RHBOPT_throw_StExcep(ev,OBJ_ADAPTER,UnexpectedNULL,MAYBE);
		}

		implreps_changed(somThis,one,mutex);
	}

#ifdef USE_THREADS
	SOMDD_NotifyWaitingRoom(&somThis->waiting_room,m);
#endif

	if (ev->_major || (!SOMDD_running))
	{

		RHBOPT_throw_StExcep(ev,OBJ_ADAPTER,SOMDDNotRunning,MAYBE);

		somPrintf("impl_up(%s), returning exception(%s)\n",
				impl_id,somExceptionId(ev));
	}
}

SOM_Scope void SOMLINK implreps_impl_down(
	ImplRepositoryServer SOMSTAR somSelf,
	Environment *ev,
	char *impl_id,
	somToken mutex)
{
	ImplRepositoryServerData *somThis=ImplRepositoryServerGetData(somSelf);
	RHBSOMUT_KeyDataPtr k=0;
#ifdef USE_THREADS
	SOMDD_MutexRef m=mutex;

	if (!m) return;
#endif

	if (impl_id)
	{
		if (*impl_id)
		{
			_IDL_SEQUENCE_octet seq;
			seq._buffer=(octet *)impl_id;
			seq._length=(unsigned long)strlen(impl_id);
			seq._maximum=seq._length;
	
			k=RHBCDR_kds_find(&somThis->data_set,&seq);
		}
	}

	if (k)
	{
		ImplRepositoryServer_ImplRepContents *one=k->data._value;
		SOMObject SOMSTAR active_impl=one->active_impl;

		if (one->state!=ImplRepositoryServer_Server_Passive)
		{
			one->state=ImplRepositoryServer_Server_Deactivating;
		}

		one->active_impl=NULL;

		if (active_impl) somReleaseObjectReference(active_impl);

/*		want somdd to wait till all child processes
		have stopped correctly, deactivate_impl is not enough

		SOMDD_SetInvalidProcess(&one->process_info);*/

		if (one->impl)
		{
			/* see if it's ephemeral */

			if (one->ephemeral)
			{
				one->state=ImplRepositoryServer_Server_Passive;

				ImplRepositoryServer_delete_impldef(somSelf,ev,impl_id);
			}
			else
			{
				if (!one->child_process)
				{
					/* this means it was not spawned by 'somdd' */

					one->state=ImplRepositoryServer_Server_Passive;
				}
			}
		}

		implreps_changed(somThis,one,mutex);
	}
	else
	{
		RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,BadParm,NO);
	}

#ifdef USE_THREADS
	SOMDD_NotifyWaitingRoom(&somThis->waiting_room,m);
#endif
}

SOM_Scope boolean SOMLINK implreps_get_stable_state(
		ImplRepositoryServer SOMSTAR somSelf,
		Environment *ev,
		corbastring impl_id)
{
	ImplRepositoryServerData *somThis=ImplRepositoryServerGetData(somSelf);
	boolean result=0;
	SOMDD_MutexRef_begin

	while (!ev->_major)
	{
		_IDL_SEQUENCE_octet seq={0,0,NULL};
		RHBSOMUT_KeyDataPtr k;

		seq._buffer=(octet *)impl_id;
		seq._length=(unsigned long)strlen(impl_id);
		seq._maximum=seq._length;
	
		k=RHBCDR_kds_find(&somThis->data_set,&seq);

		if (k)
		{
			ImplRepositoryServer_ImplRepContents *one=k->data._value;

			if (one->active_impl) 
			{
				result=1;
				break;
			}

			if (!one->child_process)
			{
				switch (one->state)
				{
				case ImplRepositoryServer_Server_Activating:
				case ImplRepositoryServer_Server_Deactivating:
					break;
				case ImplRepositoryServer_Server_Ready:
					result=1;
					break;
				case ImplRepositoryServer_Server_Passive:
					result=0;
					break;
				}
			}

			if (result) break;
			if (one->state==ImplRepositoryServer_Server_Passive)
			{
				break;
			}
		}
		else
		{
			RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,BadParm,NO);
			break;
		}
		
#ifdef USE_THREADS
		/* in threaded environment wait till state has stabilised */
		SOMDD_WaitInWaitingRoom(&somThis->waiting_room,SOMDD_MutexRef_ref);
#else
		/* in none threaded environment bomb out... */
		RHBOPT_throw_StExcep(ev,TRANSIENT,UnknownError,NO);
#endif
	}

	SOMDD_MutexRef_end

	return result;
}

SOM_Scope void SOMLINK implreps_shutdown_server(
		ImplRepositoryServer SOMSTAR somSelf,
		Environment *ev,
		char *impl_id)
{
	ImplRepositoryServerData *somThis=ImplRepositoryServerGetData(somSelf);
	SOMDD_MutexRef_begin

	if (impl_id && !ev->_major)
	{
		_IDL_SEQUENCE_octet seq;
		RHBSOMUT_KeyDataPtr k;

		seq._buffer=(octet *)impl_id;
		seq._length=(unsigned long)strlen(impl_id);
		seq._maximum=seq._length;
	
		k=RHBCDR_kds_find(&somThis->data_set,&seq);

		if (k)
		{
			ImplRepositoryServer_ImplRepContents *one=k->data._value;

			if (one->active_impl) 
			{
				SOMDD_MutexRef_pause
				SOMDD_interrupt_server(one,ev);
				SOMDD_MutexRef_resume
				somExceptionFree(ev);

				while (one->state!=ImplRepositoryServer_Server_Passive)
				{
					somPrintf("Waiting for server process to end.\n");

#ifdef USE_THREADS
					SOMDD_WaitInWaitingRoom(&somThis->waiting_room,SOMDD_MutexRef_ref);
#else
					/* obituary will get through this... */
					SOMOA_execute_next_request(SOMD_SOMOAObject,ev,SOMD_WAIT);
#endif
				}
			}
		}
		else
		{
			RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,BadParm,NO);
		}
	}

	SOMDD_MutexRef_end
}

static ImplRepositoryServer_ImplRepContents *implreps_get_one(
	ImplRepositoryServer SOMSTAR somSelf,
	char *impl_id,
	somToken mutex)
{
	ImplRepositoryServerData *somThis=ImplRepositoryServerGetData(somSelf);
	
	if (impl_id)
	{
		if (impl_id[0])
		{
			RHBSOMUT_KeyDataPtr k=NULL;
			_IDL_SEQUENCE_octet seq={0,0,NULL};
			seq._buffer=(octet *)impl_id;
			seq._length=(unsigned long)strlen(impl_id);
			seq._maximum=seq._length;
	
			k=RHBCDR_kds_find(&somThis->data_set,&seq);
	
			if (k) return k->data._value;
		}
	}

	return NULL;
}

SOM_Scope void SOMLINK implreps_impl_failed(
	ImplRepositoryServer SOMSTAR somSelf,
	Environment *ev,
	char *impl_id,
	somToken proc_info_ref,
	ORBStatus rc,
	somToken mutex)
{
#ifdef USE_THREADS
	ImplRepositoryServerData * somThis=ImplRepositoryServerGetData(somSelf);
#endif
	RHBProcess *process_info=proc_info_ref;
	ImplRepositoryServer_ImplRepContents *one=implreps_get_one(somSelf,impl_id,mutex);
#ifdef USE_THREADS
	SOMDD_MutexRef m=mutex;
	if (!m) return;
#else
	somToken m=mutex;
#endif

	if (one)
	{
		somPrintf("%s failed with rc=%ld in ",impl_id,rc);
		RHBProcess_Print(process_info);
	}
	else
	{
		somPrintf("<unknown-impl> failed with rc=%ld in ",impl_id,rc);
		/* treat as obituary */
		ImplRepositoryServer_obituary(somSelf,ev,process_info,rc,m);
	}

	if (one)
	{
		SOMObject SOMSTAR active_impl=one->active_impl;

		if (one->state!=ImplRepositoryServer_Server_Passive)
		{
			one->state=ImplRepositoryServer_Server_Deactivating;
		}

		one->active_impl=0;

		if (active_impl) somReleaseObjectReference(active_impl);

		if (RHBProcess_Equal(one->child_process,process_info))
		{
			RHBProcess *proc=one->child_process;
			one->child_process=NULL;
			RHBProcess_Release(proc);
		}

		if (one->impl)
		{
			/* see if it's ephemeral */

			char *alias=ImplementationDef__get_impl_alias(one->impl,ev);

			if (alias)
			{
				if (!one->child_process)
				{
					/* this means it was not spawned by 'somdd' */

					somPrintf("impl now has invalid process\n");

					one->state=ImplRepositoryServer_Server_Passive;
				}

				SOMFree(alias);
			}
			else
			{
				one->state=ImplRepositoryServer_Server_Passive;
				ImplRepositoryServer_delete_impldef(somSelf,ev,impl_id);
			}
		}
	}
	else
	{
		RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,BadParm,NO);
	}

#ifdef USE_THREADS
	SOMDD_NotifyWaitingRoom(&somThis->waiting_room,m);
#endif
}

boolean SOMDD_is_impl_alive_in(char *impl_id,RHBProcess *process_info)
{
	boolean result=0;
	SOMDD_MutexRef_begin
	ImplRepositoryServer_ImplRepContents *one=NULL;

	if (SOMD_ImplRepObject)
	{
		one=implreps_get_one(SOMD_ImplRepObject,impl_id,SOMDD_MutexRef_ref);

		if (one->state==ImplRepositoryServer_Server_Ready)
		{
			if (RHBProcess_Equal(process_info,one->child_process))
			{
				result=1;
			}
		}
	}

	SOMDD_MutexRef_end

	return result;
}

SOM_Scope void SOMLINK implreps__set_implrep_filename(
		ImplRepositoryServer SOMSTAR somSelf,
		Environment *ev,
		char *implrep_filename)
{
	ImplRepositoryServerData *somThis=ImplRepositoryServerGetData(somSelf);
	char *p=SOMDD_dupl_string(implrep_filename);
	char *q=somThis->implrep_filename;
	somThis->implrep_filename=p;
	if (q) SOMFree(q);
	somPrintf("implreps__set_implrep_filename(%s)\n",p);
}

SOM_Scope char * SOMLINK implreps__get_implrep_filename(
		ImplRepositoryServer SOMSTAR somSelf,
		Environment *ev)
{
	ImplRepositoryServerData *somThis=ImplRepositoryServerGetData(somSelf);

	return SOMDD_dupl_string(somThis->implrep_filename);
}

SOM_Scope void SOMLINK implreps_implrep_update(
		ImplRepositoryServer SOMSTAR somSelf,
		Environment *ev)
{
	ImplRepositoryServerData *somThis=ImplRepositoryServerGetData(somSelf);

	if (!(somThis->implrep_filename)) 
	{
		RHBOPT_throw_StExcep(ev,INITIALIZE,UnexpectedNULL,NO);
	}

	if (ev->_major) return;

	if (somThis->implrep_changed)
	{
	
	}
}

SOM_Scope ORBStatus SOMLINK implreps_handle_requests(
	ImplRepositoryServer SOMSTAR somSelf,
	Environment *ev,
	long flags)
{
	ImplRepositoryServerData *somThis=ImplRepositoryServerGetData(somSelf);
	RHBRequestList *list=&somThis->request_list;
	ORBStatus status=SOMDERROR_NoMessages;

	if (!list->lNested)
	{
		list->lNested++;

		if (list->list)
		{
#ifdef USE_SEND_MULTIPLE
			while (ev->_major==NO_EXCEPTION)
			{
				Request SOMSTAR req=NULL;
				ORBStatus status=get_next_response(ev,flags,&req);

				if (status==SOMDERROR_NoMessages)
				{
					break;
				}

				somPrintf("get_next_response(),.. have response(%ld)....\n",status);

				if (status && (ev->_major==NO_EXCEPTION))
				{
					StExcep_UNKNOWN *ex=SOMMalloc(sizeof(*ex));

					if (ex)
					{
						ex->completed=MAYBE;
						ex->minor=status;
					}

					somSetException(ev,SYSTEM_EXCEPTION,ex_StExcep_UNKNOWN,ex);
				}

				SOMDD_MutexRef_begin

				{
					struct RHBRequest *r=list->list;
					struct RHBRequest *p=NULL;

					while (r)
					{
						if (r->request==req)
						{
							if (p)
							{
								p->next=r->next;
							}
							else
							{
								list->list=r->next;
							}

							r->pending=0;
							r->request_list=NULL;

							r->complete(r,ev,SOMDD_MutexRef_ref);

							break;
						}
						else
						{
							p=r;
							r=r->next;
						}
					}
				}

				SOMDD_MutexRef_end

				somdExceptionFree(ev);

				status=0;
			}
#else					
			{
				long lSequence=++(list->lSequence);
				struct RHBRequest *r=list->list;
				struct RHBRequest *p=NULL;

				while (r)
				{
					if (r->lSequence != lSequence)
					{
						Environment ev;
						SOM_InitEnvironment(&ev);

						if (p)
						{
							p->next=r->next;
						}
						else
						{
							list->list=r->next;
						}

						if (r->request)
						{
							if (!Request_get_response(r->request,&ev,flags))
							{
								r->complete(r,&ev);
								r=NULL;
							}

							status=0;
						}

						if (r)
						{
							r->lSequence=lSequence;
							r->next=list->list;
							list->list=r;
						}

						r=list->list;
						p=NULL;

						SOM_UninitEnvironment(&ev);
					}
					else
					{
						p=r;
						r=r->next;
					}
				}
			}
#endif
		}

		list->lNested--;
	}

	return status;
}

SOM_Scope ORBStatus SOMLINK implreps_close(
		ImplRepositoryServer SOMSTAR somSelf,
		Environment *ev,
		somToken mutex)
{
#ifdef USE_THREADS
	ImplRepositoryServerData *somThis=ImplRepositoryServerGetData(somSelf);
	SOMDD_MutexRef SOMDD_MutexRef_ref=mutex;

	somThis->request_list.doQuit=1;
#ifdef USE_PTHREADS
	pthread_cond_signal(&somThis->request_list.p_event);
#else
	if (!SetEvent(somThis->request_list.h_event)) implreps_trap();
#endif

	while (somThis->request_list.isAlive)
	{
#ifdef USE_PTHREADS
		pthread_cond_wait(&somThis->request_list.p_dead,SOMDD_MutexRef_ref);
#else
		SOMDD_MutexRef_pause
		WaitForSingleObjectEx(somThis->request_list.h_dead,INFINITE,1);
		SOMDD_MutexRef_resume
#endif
	}

	while (somThis->request_list.list ||
		   somThis->server_request_list.list)
	{
#ifdef USE_PTHREADS
		pthread_cond_wait(&somThis->server_request_list.p_event,SOMDD_MutexRef_ref);
#else
		SOMDD_MutexRef_pause
		WaitForSingleObjectEx(somThis->server_request_list.h_event,INFINITE,1);
		SOMDD_MutexRef_resume
#endif
	}
#endif

	return 0;
}

