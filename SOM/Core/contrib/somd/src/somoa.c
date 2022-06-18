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

#define SOMOA_Class_Source

#include <rhbopt.h>
#include <rhbsomd.h>
#include <signal.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifndef SOM_Module_somoa_Source
#define SOM_Module_somoa_Source
#endif

#define ImplementationDef__acquire_c_impl(a,b)  ImplementationDef__get_c_impl(a,b)

struct somoa_blocked
{
	RHBORB *orb;
	boolean enabled;
	struct somoa_blocked *chain;
	struct SOMOAPrivateData *somThis;
	boolean blocked,locked;
	ORBStatus interrupt_status;
#ifdef USE_THREADS
	#ifdef USE_PTHREADS
		pthread_cond_t condition;
	#else
		HANDLE event;
	#endif
#else
#endif
};

typedef struct SOMOAPrivateData
{
	RHBORB *c_orb;
	struct somoa_blocked *blocked;
	RHBServerData *servers;
} SOMOAPrivateData;

#include "somoa.ih"

struct somoa_execute_next_request
{
	Environment *ev;
	boolean enabled;
	Flags waitFlag;
	RHBORB *orb;
};

static void rhbServerData_AddRef(RHBServerData *somThis)
{
	somd_atomic_inc(&somThis->lUsage);
}

static void rhbServerData_Release(RHBServerData *somThis)
{
	if (!somd_atomic_dec(&somThis->lUsage))
	{
		RHBImplementationDef *listener=somThis->listener;

		if (somThis->impl_id._buffer)
		{
			SOMFree(somThis->impl_id._buffer);
		}

		SOMFree(somThis);

		if (listener)
		{
			RHBImplementationDef_Release(listener);
		}
	}
}

static struct RHBServerDataVtbl rhbServerDataVtbl={
	rhbServerData_AddRef,
	rhbServerData_Release
};

RHBOPT_cleanup_begin(somoa_execute_next_request_cleanup,pv)

struct somoa_execute_next_request *data=pv;

	RHBORB_set_server_requests_enabled(data->orb,data->enabled);

RHBOPT_cleanup_end

SOM_Scope ORBStatus  SOMLINK somoa_execute_next_request(
		SOMOA SOMSTAR somSelf, 
		Environment *ev, 
		Flags waitFlag)
{
	/* look for SOMD_WAIT or SOMD_NO_WAIT in waitFlag */
    SOMOAData *somThis = SOMOAGetData(somSelf);
	struct somoa_execute_next_request data={NULL,0,0,NULL};

	data.ev=ev;
	data.waitFlag=waitFlag;
	data.orb=somThis->waiter.c_orb;

	RHBOPT_cleanup_push(somoa_execute_next_request_cleanup,&data);

	data.enabled=RHBORB_set_server_requests_enabled(data.orb,1);

#ifdef USE_THREADS
	RHBOPT_unused(waitFlag)
#else
	if (waitFlag==SOMD_NO_WAIT)
	{
		RHBORB_ProcessEvents(somThis->waiter.c_orb,ev);
	}
	else
	{
		RHBORB_ProcessEvents(somThis->waiter.c_orb,ev);

		if (somThis->waiter.c_orb->events.user_eman)
		{
			SOMEEMan_someProcessEventInterval(
					somThis->waiter.c_orb->events.user_eman,
					ev,
					(unsigned long)~0L,
					10000000,
					0);
		}
		else
		{
			RHBORB_idle(somThis->waiter.c_orb,ev,0);
		}

		RHBORB_ProcessEvents(somThis->waiter.c_orb,ev);
	}
#endif

	RHBOPT_cleanup_pop();

    return SOMDERROR_NoMessages;
}

RHBOPT_cleanup_begin(somoa_execute_request_loop_cleanup,pv)

struct somoa_blocked *data=pv;

	if (!data->locked)
	{
		RHBORB_guard(data->orb);
	}

	RHBORB_set_server_requests_enabled(data->orb,data->enabled);

	if (data->somThis->blocked==data)
	{
		data->somThis->blocked=data->chain;
	}
	else
	{
		struct somoa_blocked *p=data->somThis->blocked;

		while (p)
		{
			if (p->chain==data)
			{
				p->chain=data->chain;
				break;
			}

			p=p->chain;
		}
	}

#ifdef USE_THREADS
	#ifdef USE_PTHREADS
		pthread_cond_destroy(&data->condition);
	#else
		CloseHandle(data->event);
	#endif
#endif

	RHBORB_unguard(data->orb);

RHBOPT_cleanup_end

SOM_Scope ORBStatus  SOMLINK somoa_execute_request_loop(
		SOMOA SOMSTAR somSelf, 
		Environment *ev, 
		Flags waitFlag)
{
SOMOAData *somThis=SOMOAGetData(somSelf);
struct somoa_blocked data;

	if (ev->_major) return SOMDERROR_BadEnvironment;

	if (somThis->waiter.blocked && (waitFlag!=SOMD_NO_WAIT))
	{
		RHBOPT_throw_StExcep(ev,OBJ_ADAPTER,Busy,NO);

		return SOMDERROR_Busy;
	}

	RHBORB_guard(orb)

	data.locked=1;
	data.somThis=&somThis->waiter;
	data.interrupt_status=0;
	data.blocked=1;
	data.orb=somThis->waiter.c_orb;
	data.chain=NULL;
	data.enabled=0;

#ifdef USE_THREADS
	#ifdef USE_PTHREADS
		pthread_cond_init(&data.condition,RHBOPT_pthread_condattr_default);
	#else	
		data.event=CreateEvent(NULL,0,0,NULL);
	#endif
#endif

	RHBOPT_cleanup_push(somoa_execute_request_loop_cleanup,&data);

	data.chain=somThis->waiter.blocked;
	somThis->waiter.blocked=&data;

	data.enabled=RHBORB_set_server_requests_enabled(data.orb,1);

#ifdef USE_THREADS
	while (data.blocked)
	{
	#if defined(USE_PTHREADS)
		somd_wait(&data.condition,__FILE__,__LINE__);
	#else
		somd_wait(data.event,__FILE__,__LINE__);
	#endif
	}
#else
	if (waitFlag==SOMD_NO_WAIT)
	{
		while ((!data.interrupt_status)&&!ev->_major)
		{
			data.interrupt_status=SOMOA_execute_next_request(somSelf,ev,waitFlag);
		} 
	}
	else
	{
		if (data.orb->events.user_eman)
		{
			SOMEEMan_someProcessEvents(data.orb->events.user_eman,ev);
		}
		else
		{
			while (data.blocked && !ev->_major)
			{
				RHBORB_idle(data.orb,ev,0);
				RHBORB_ProcessEvents(data.orb,ev);
			}
		}
	}
#endif

	RHBOPT_cleanup_pop();

    return data.interrupt_status;
}

SOM_Scope void  SOMLINK somoa_change_id(
		SOMOA SOMSTAR somSelf,
		Environment *ev, 
		SOMDObject SOMSTAR objref, 
		ReferenceData* id)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(objref)
	RHBOPT_unused(id)

	RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,NotImplemented,NO)
}

struct somoa_create_SOM_ref
{
	RHBImplementationDef *implDef;
	RHBInterfaceDef *iface;
	char *impl_id;
	char *iface_id;
};

RHBOPT_cleanup_begin(somoa_create_SOM_ref_cleanup,pv)

struct somoa_create_SOM_ref *data=pv;

	if (data->implDef) RHBImplementationDef_Release(data->implDef);
	if (data->iface) RHBInterfaceDef_Release(data->iface);
	if (data->impl_id) SOMFree(data->impl_id);
	if (data->iface_id) SOMFree(data->iface_id);

RHBOPT_cleanup_end

SOM_Scope SOMDObject SOMSTAR  SOMLINK somoa_create_SOM_ref(
		SOMOA SOMSTAR somSelf, 
		Environment *ev, 
		SOMObject SOMSTAR somobj, 
		ImplementationDef SOMSTAR impl)
{
	struct somoa_create_SOM_ref data={NULL,NULL,NULL,NULL};
	SOMDObject SOMSTAR RHBOPT_volatile ref=NULL;

	RHBOPT_unused(somSelf)

	RHBOPT_cleanup_push(somoa_create_SOM_ref_cleanup,&data);

	if (ev)
	{
		if (ev->_major) somobj=NULL;
	}
	else
	{
		somobj=NULL;
	}

	if (!somobj) 
	{
		RHBOPT_throw_StExcep(ev,INV_OBJREF,BadObjref,MAYBE)
	}
	else
	{
		if (!impl) 
		{
			RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,BadObjref,MAYBE)
		}
		else
		{
			data.implDef=ImplementationDef__acquire_c_impl(impl,ev);

			if (!data.implDef)
			{
				RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,BadObjref,MAYBE)
			}
			else
			{
				RHBORB *orb=ORB__get_c_orb(SOMD_ORBObject,ev);

				data.impl_id=ImplementationDef__get_impl_id(impl,ev);

				if (data.impl_id && !ev->_major) 
				{
					octet buf[OBJECTKEY_PREFIX_LEN+sizeof(somobj)];
					_IDL_SEQUENCE_octet seq={sizeof(buf),sizeof(buf),NULL};
					seq._buffer=buf;

					buf[0]=3;
					memcpy(buf+1,data.impl_id,OBJECTKEY_PREFIX_LEN-2);
					buf[OBJECTKEY_PREFIX_LEN-1]=2;
					memcpy(buf+OBJECTKEY_PREFIX_LEN,&somobj,sizeof(somobj));

					data.iface_id=RHBORB_convert_class_to_interface(orb,somobj->mtab->classObject);

					data.iface=RHBORB_get_interface(orb,ev,data.iface_id);

					ref=RHBORB_get_object(orb,ev,orb->somClass_SOMDObject_ref,data.implDef,data.iface,&seq,0);
				}
			}
		}
	}

	RHBOPT_cleanup_pop();

    return ref;
}

SOM_Scope SOMObject SOMSTAR  SOMLINK somoa_get_SOM_object(
		SOMOA SOMSTAR somSelf, 
		Environment *ev, 
		SOMDObject SOMSTAR somref)
{
	RHBObject *obj=SOMDObject__get_c_proxy(somref,ev);

	RHBOPT_unused(somSelf)

	return RHBObject_get_server_object(obj,ev,&obj->object_key);
}

struct somoa_activate_impl_failed
{
	char *impl_id;
};

RHBOPT_cleanup_begin(somoa_activate_impl_failed_cleanup,pv)

	struct somoa_activate_impl_failed *data=pv;

	if (data->impl_id) SOMFree(data->impl_id);

RHBOPT_cleanup_end

SOM_Scope void  SOMLINK somoa_activate_impl_failed(
		SOMOA SOMSTAR somSelf, 
		Environment *ev, 
		ImplementationDef SOMSTAR impl, 
		ORBStatus rc)
{
	struct somoa_activate_impl_failed data={NULL};

	if (SOMD_ORBObject && !ev->_major)
	{
/*		RHBORB *orb=ORB__get_c_orb(SOMD_ORBObject,ev);*/
		
		RHBOPT_cleanup_push(somoa_activate_impl_failed_cleanup,&data);

		if (impl)
		{
			data.impl_id=ImplementationDef__get_impl_id(impl,ev);
		}

		SOMOA_notify_somdd(somSelf,ev,-1,data.impl_id,0,rc);

		RHBOPT_cleanup_pop();
	}

	RHBOPT_unused(somSelf)
}

SOM_Scope void SOMLINK somoa_interrupt_server(
		SOMOA SOMSTAR somSelf, 
		Environment *ev)
{
	SOMOA_interrupt_server_status(somSelf,ev,SOMDERROR_ServerInterrupt);
}

SOM_Scope void  SOMLINK somoa_interrupt_server_status(
		SOMOA SOMSTAR somSelf, 
		Environment *ev,
		ORBStatus status)
{
	SOMOAData *somThis=SOMOAGetData(somSelf);

	RHBORB_guard(orb)

	if (somThis->waiter.blocked)
	{
		somThis->waiter.blocked->blocked=0;

		somThis->waiter.blocked->interrupt_status=status;
	
#ifdef USE_THREADS
		RHBOPT_unused(ev)
	#ifdef USE_PTHREADS
		pthread_cond_signal(&(somThis->waiter.blocked->condition));
	#else
		if (!SetEvent(somThis->waiter.blocked->event))
		{
			SOMD_bomb("somoa_interrupt_server_status");
		}
	#endif
#else
		if (somThis->waiter.c_orb->events.user_eman)
		{
			SOMEEMan_someShutdown(somThis->waiter.c_orb->events.user_eman,ev);
		}
#endif
	}

	RHBORB_unguard(orb)
}

#ifdef SOMOA_somDefaultInit
SOM_Scope void SOMLINK somoa_somDefaultInit(
	SOMOA SOMSTAR somSelf,
	somInitCtrl *ctrl)
{
	somInitCtrl globalCtrl;
	somBooleanVector myMask;
	SOMOAData *somThis;

	SOMOA_BeginInitializer_somDefaultInit
	SOMOA_Init_BOA_somDefaultInit(somSelf,ctrl);
#else
SOM_Scope void  SOMLINK somoa_somInit(SOMOA SOMSTAR somSelf)
{
	SOMOAData *somThis=SOMOAGetData(somSelf);
    SOMOA_parent_BOA_somInit(somSelf);
#endif

	somThis->waiter.blocked=NULL;

	if (SOMD_ORBObject)
	{
		Environment ev;
		SOM_InitEnvironment(&ev);
		somThis->waiter.c_orb=ORB__get_c_orb(SOMD_ORBObject,&ev);
		SOM_UninitEnvironment(&ev);
	}
}

#ifdef SOMOA_somDestruct
SOM_Scope void SOMLINK somoa_somDestruct(
	SOMOA SOMSTAR somSelf,
	boolean doFree,
	somDestructCtrl *ctrl)
{
	somDestructCtrl globalCtrl;
	somBooleanVector myMask;
	SOMOAData *somThis;

	SOMOA_BeginDestructor
#else
SOM_Scope void  SOMLINK somoa_somUninit(SOMOA SOMSTAR somSelf)
{
#	ifdef _DEBUG
	SOMOAData *somThis=SOMOAGetData(somSelf);
#	endif
#endif

	RHBOPT_unused(somThis);

	{
		Environment ev;

		SOM_InitEnvironment(&ev);

		RHBORB_guard(0)

		if (SOMD_SOMOAObject==somSelf) 
		{
			SOMD_SOMOAObject=NULL;
		}

		RHBOPT_ASSERT(!somThis->waiter.blocked)

		RHBORB_unguard(0)

		if (SOMMSingleInstanceClassData.sommSingleInstanceFreed)
		{
			SOMMSingleInstance_sommSingleInstanceFreed(somSelf->mtab->classObject,&ev,somSelf);
		}

		SOM_UninitEnvironment(&ev);
	}


#ifdef SOMOA_somDestruct
	SOMOA_EndDestructor
#else
    SOMOA_parent_BOA_somUninit(somSelf);
#endif
}

struct somoa_create
{
	ImplementationDef SOMSTAR somdd_impldef;
	char *iface_id;
	char *impl_uuid;
	RHBInterfaceDef *ifaceDef;
	RHBImplementationDef *implDef;
	_IDL_SEQUENCE_octet key;
};

RHBOPT_cleanup_begin(somoa_create_cleanup,pv)

struct somoa_create *data=pv;

	if (data->iface_id) SOMFree(data->iface_id);
	if (data->impl_uuid) SOMFree(data->impl_uuid);
	if (data->key._buffer) SOMFree(data->key._buffer);
	if (data->ifaceDef) RHBInterfaceDef_Release(data->ifaceDef);
	if (data->implDef) RHBInterfaceDef_Release(data->implDef);
	if (data->somdd_impldef) somReleaseObjectReference(data->somdd_impldef);

RHBOPT_cleanup_end

SOM_Scope SOMDObject SOMSTAR  SOMLINK somoa_create(
		SOMOA SOMSTAR somSelf,
		Environment *ev, 
		ReferenceData* id, 
		InterfaceDef SOMSTAR intf, 
		ImplementationDef SOMSTAR impl)
{
	struct somoa_create data={NULL,NULL,NULL,NULL,NULL,{0,0,NULL}};
	RHBOPT_volatile long l=0;
	SOMDObject SOMSTAR RHBOPT_volatile ref=NULL;

	RHBOPT_unused(somSelf)

	RHBOPT_cleanup_push(somoa_create_cleanup,&data);

	if (id) l=id->_length;

	data.iface_id=SOMOA_contained__get_id(somSelf,ev,intf);

	data.impl_uuid=ImplementationDef__get_impl_id(impl,ev);

	if (data.iface_id && data.impl_uuid)
	{
		RHBORB *orb=ORB__get_c_orb(SOMD_ORBObject,ev);

		data.key._length=OBJECTKEY_PREFIX_LEN+l;
		data.key._buffer=SOMMalloc(data.key._length);
		data.key._buffer[0]=3;
		memcpy(data.key._buffer+1,data.impl_uuid,strlen(data.impl_uuid));
		data.key._buffer[OBJECTKEY_PREFIX_LEN-1]=0;

		if (l)
		{
			memcpy(data.key._buffer+OBJECTKEY_PREFIX_LEN,
					id->_buffer,l);
		}

		data.key._maximum=data.key._length;

		data.implDef=ImplementationDef__acquire_c_impl(impl,ev);

		data.ifaceDef=RHBORB_get_interface(orb,ev,data.iface_id);

		ref=RHBORB_get_object(orb,
					ev,
					orb->somClass_SOMDObject_ref,
					data.implDef,
					data.ifaceDef,
					&data.key,
					0);
	}

	RHBOPT_cleanup_pop();

    return ref;
}

SOM_Scope SOMDObject SOMSTAR  SOMLINK somoa_create_constant(
		SOMOA SOMSTAR somSelf, 
		Environment *ev, 
		ReferenceData* id, 
		InterfaceDef SOMSTAR intf, 
		ImplementationDef SOMSTAR impl_arg)
{
	/* this means create object with address of SOMDD,
		this needs a fast way of getting the IOR of
		SOMDD */

	ImplementationDef SOMSTAR RHBOPT_volatile impl=impl_arg;
	struct somoa_create data={NULL,NULL,NULL,NULL,NULL,{0,0,NULL}};
	long RHBOPT_volatile l=0;
	SOMDObject SOMSTAR RHBOPT_volatile ref=NULL;

	RHBOPT_unused(somSelf)

	RHBOPT_cleanup_push(somoa_create_cleanup,&data);

	if (id) l=id->_length;

	data.iface_id=SOMOA_contained__get_id(somSelf,ev,intf);

	data.impl_uuid=ImplementationDef__get_impl_id(impl,ev);

	if (data.iface_id && data.impl_uuid)
	{
		RHBORB *orb=ORB__get_c_orb(SOMD_ORBObject,ev);

		data.key._length=OBJECTKEY_PREFIX_LEN+l;
		data.key._buffer=SOMMalloc(data.key._length);
		data.key._buffer[0]=3;
		memcpy(data.key._buffer+1,data.impl_uuid,strlen(data.impl_uuid));
		data.key._buffer[OBJECTKEY_PREFIX_LEN-1]=0;

		if (l)
		{
			memcpy(data.key._buffer+OBJECTKEY_PREFIX_LEN,
					id->_buffer,l);
		}

		data.key._maximum=data.key._length;

		data.somdd_impldef=ImplRepository__get_SOMDD_ImplDefObject(
				SOMD_ImplRepObject,ev);

		if (data.somdd_impldef)
		{
			impl=data.somdd_impldef;
		}

		data.implDef=ImplementationDef__acquire_c_impl(impl,ev);

		data.ifaceDef=RHBORB_get_interface(orb,ev,data.iface_id);

		ref=RHBORB_get_object(orb,
					ev,
					orb->somClass_SOMDObject_ref,
					data.implDef,
					data.ifaceDef,
					&data.key,
					0);
	}

	RHBOPT_cleanup_pop();

    return ref;
}

SOM_Scope void  SOMLINK somoa_dispose(
		SOMOA SOMSTAR somSelf,  
		Environment *ev, 
		SOMDObject SOMSTAR obj)
{
    SOMOA_parent_BOA_dispose(somSelf, ev, obj);
}

SOM_Scope void  SOMLINK somoa_change_implementation(
		SOMOA SOMSTAR somSelf, 
		Environment *ev, 
		SOMDObject SOMSTAR obj, 
		ImplementationDef SOMSTAR impl)
{
/*    SOMOA_parent_BOA_change_implementation(somSelf, ev, obj, 
                                           impl);*/

	RHBOPT_ASSERT(ev)
	RHBOPT_ASSERT(obj)
	RHBOPT_ASSERT(impl)

	if (obj && impl)
	{
		RHBImplementationDef *new_impl;
		RHBImplementationDef *old_impl;
		RHBObject *object;
		RHBORB_guard(0)
		object=SOMDObject__get_c_proxy(obj,ev);

		old_impl=object->impl;

		new_impl=ImplementationDef__acquire_c_impl(impl,ev);

		object->impl=new_impl;

		RHBORB_unguard(0)

		if (old_impl) RHBImplementationDef_Release(old_impl);
	}

	RHBOPT_unused(somSelf)
}

SOM_Scope ReferenceData  SOMLINK somoa_get_id(
		SOMOA SOMSTAR somSelf, 
		Environment *ev, 
		SOMDObject SOMSTAR obj)
{
	ReferenceData data={0,0,NULL};
	_IDL_SEQUENCE_octet object_key=SOMDObject__get_somd_id(obj,ev);

/* if this fails because it's a SOM_ref,
	then it should return 

	BAD_OPERATION,SOMDERROR_WrongRefType 
		::StExcep::BAD_OPERATION,30089
  */

	if (!ev->_major)
	{
		if (object_key._length > OBJECTKEY_PREFIX_LEN)
		{
			data._length=object_key._length-OBJECTKEY_PREFIX_LEN;
			data._buffer=SOMMalloc(data._length);

			if (data._buffer)
			{
				octet *op=object_key._buffer+OBJECTKEY_PREFIX_LEN;
				data._maximum=data._length;
				memcpy(data._buffer,op,data._length);
			}
			else
			{
				RHBOPT_throw_StExcep(ev,NO_MEMORY,NoMemory,MAYBE);
				data._length=0;
			}
		}
	}

	RHBOPT_unused(somSelf)
	return data;
}

SOM_Scope Principal SOMSTAR SOMLINK somoa_get_principal(
		SOMOA SOMSTAR somSelf, 
		Environment *ev, 
		SOMDObject SOMSTAR obj, 
		Environment* req_ev)
{
	RHBServerRequest *request;
	RHBORB_ThreadTask *thread=RHBSOMD_get_thread();

	if (thread)
	{
		request=thread->server_request;

		if (request)
		{
			if (request->header.response_expected & 1)
			{
				if (request->cancelled)
				{
					RHBOPT_throw_StExcep(ev,NO_RESPONSE,RequestNotFound,MAYBE);

					return 0;
				}

				if (request->impl)
				{
					if (!request->impl->connection.connected)
					{
						RHBOPT_throw_StExcep(ev,COMM_FAILURE,SocketError,MAYBE);

						return 0;
					}
				}
			}

			return RHBServerRequest_get_principal(request,ev);
		}
	}

	RHBOPT_unused(somSelf)
	RHBOPT_unused(obj)
	RHBOPT_unused(req_ev)

	return 0;
}

SOM_Scope void  SOMLINK somoa_set_exception(
		SOMOA SOMSTAR somSelf,
		Environment *ev, 
		exception_type major, 
		corbastring userid,
		void* param)
{
	RHBORB_ThreadTask *thread=RHBSOMD_get_thread();
	RHBOPT_unused(somSelf)

	if (SOM_TraceLevel)
	{
		somPrintf("somSetException(%s) in %s:%d\n",
				userid,__FILE__,__LINE__);
	}

	somSetException(ev,major,userid,param);

	if (thread)
	{
		RHBServerRequest *request=thread->server_request;
	
		if (request)
		{
			any a={NULL,NULL};
			a._type=TC_void;
			RHBServerRequest_AddRef(request);
			RHBServerRequest_set_result(request,ev,&a);
			RHBServerRequest_Release(request);

			somdExceptionFree(ev);
		}
	}
}

SOM_Scope void  SOMLINK somoa_impl_is_ready(
		SOMOA SOMSTAR somSelf,
		Environment *ev, 
		ImplementationDef SOMSTAR impl)
{
	RHBImplementationDef *implDef=NULL;
	SOMOAData *somThis=SOMOAGetData(somSelf);

	if (ev->_major)
	{
		return;
	}

	implDef=ImplementationDef__acquire_c_impl(impl,ev);

	if (ev->_major)
	{
		return;
	}

	if (implDef)
	{
		if (implDef->server.data)
		{
			RHBImplementationDef_Release(implDef);

			return;
		}
	}

	if (implDef)
	{
		char *impl_id=ImplementationDef__get_impl_id(impl,ev);
		RHBServerData *data=SOMMalloc(sizeof(*data));

		data->lpVtbl=&rhbServerDataVtbl;
		data->lUsage=2;
		data->somdServer=NULL;
		data->somoa=somSelf;
		data->next=somThis->waiter.servers;
		data->deactivating=0;
		data->is_ready=0;
		data->listener_list=NULL;
		data->listener=implDef;
		data->implDef=ImplementationDef_somDuplicateReference(impl);

		data->impl_id._length=(long)strlen(impl_id);
		data->impl_id._maximum=data->impl_id._length;
		data->impl_id._buffer=SOMMalloc(data->impl_id._length);
		memcpy(data->impl_id._buffer,impl_id,data->impl_id._length);

		RHBImplementationDef_AddRef(implDef);

		somThis->waiter.servers=data;
		implDef->server.data=data;

		RHBImplementationDef_begin_listen(implDef,ev,impl_id);

		if (impl_id) SOMFree(impl_id);

		if (!ev->_major)
		{
			/* now set up the SOMDServer */
			SOMDServer SOMSTAR server=NULL;
			char *p;
			SOMClass SOMSTAR serverClass=NULL;

			if (!SOMD_ImplDefObject)
			{
				SOMD_ImplDefObject=impl;
			}

			p=ImplementationDef__get_impl_server_class(impl,ev);
			
			if (!ev->_major)
			{
				if (p)
				{
					if (!*p) 
					{
						SOMFree(p); p=NULL;
					}
				}

				if (p)
				{
					somId id=somIdFromString(p);
					SOMClassMgr SOMSTAR mgr=somGetClassManagerReference();

					serverClass=SOMClassMgr_somFindClass(
							mgr,
							id,0,0);

					SOMFree(id);
					SOMFree(p);

					somReleaseClassManagerReference(mgr);
				}
				else
				{
					serverClass=
#ifdef SOMClass_somDuplicateReference
						SOMClass_somDuplicateReference
							(implDef->orb->somClass_SOMDServer_ref);
#else 
							(implDef->orb->somClass_SOMDServer_ref);
#endif
				}
			}

			if (serverClass)
			{
				server=SOMClass_somNew(serverClass);

				somReleaseClassReference(serverClass);
			}

			if (server)
			{
				SOMDServer__set_ImplDefObject(server,ev,impl);

				data->somdServer=server;

				if (!SOMD_ServerObject)
				{
					if (impl==SOMD_ImplDefObject)
					{
						SOMD_ServerObject=server;
					}
				}

				RHBImplementationDef_notify_somdd(implDef,ev,1);
			}
			else
			{
				RHBOPT_throw_StExcep(ev,OBJ_ADAPTER,SOMDDNotRunning,NO);
			}

			/*	if communication with 'somdd' fails then fail
					only if the impl has no 'alias' */
			/* if we don't have a server then fall through with
				the deactivate_impl due to the above exception */

			if (ev->_major && server)
			{
				Environment ev2;
				char *p;
				SOM_InitEnvironment(&ev2);

				p=ImplementationDef__get_impl_alias(impl,&ev2);

				if (!ev2._major)
				{
					if (p)
					{
						/* continue with the error */
						SOMFree(p);
					}
					else
					{
						/* clear exception and continue
							with ephemeral implementation */
						somExceptionFree(ev);
					}
				}

				SOM_UninitEnvironment(&ev2);
			}
		}

		if (ev->_major)
		{
			Environment ev2;
			SOM_InitEnvironment(&ev2);
			SOMOA_deactivate_impl(somSelf,&ev2,impl);
			SOM_UninitEnvironment(&ev2);
		}
		else
		{
			data->is_ready=1;
		}

		RHBORB_server_state_changed(implDef->orb);

		RHBImplementationDef_Release(implDef);

		data->lpVtbl->Release(data);
	}
	else
	{
		RHBOPT_throw_StExcep(ev,OBJ_ADAPTER,SOMDDNotRunning,NO);
	}
}

SOM_Scope void  SOMLINK somoa_deactivate_impl(
		SOMOA SOMSTAR somSelf, 
		Environment *ev, 
		ImplementationDef SOMSTAR impl)
{
	SOMOAData *somThis=SOMOAGetData(somSelf);
	RHBORB *c_orb=NULL;
	RHBImplementationDef *implDef=NULL,*others=NULL,*listener=NULL;
	SOMObject SOMSTAR obj=NULL;
	RHBServerData *data=NULL;

	RHBSOM_Trace("somoa_deactivate_impl - begin");

	if (ev->_major) 
	{
		RHBSOM_Trace("somoa_deactivate_impl - exception already raised");
		return;
	}

	c_orb=ORB__get_c_orb(SOMD_ORBObject,ev);

	if (!c_orb) 
	{
		RHBSOM_Trace("somoa_deactivate_impl - no ORB");
		return;
	}

	implDef=ImplementationDef__acquire_c_impl(impl,ev);

	if (ev->_major) 
	{
		RHBSOM_Trace("somoa_deactivate_impl - acquire_c_impl failed");
		return;
	}

	if (!implDef) 
	{
		RHBSOM_Trace("somoa_deactivate_impl - acquire_c_impl returned NULL");
		return;
	}

	RHBORB_guard(implDef)

	data=implDef->server.data;

	if (data)
	{
		data->is_ready=0;
		data->deactivating=1;
	}

	RHBORB_unguard(implDef)

	RHBSOM_Trace("somoa_deactivate_impl - notify somdd");

	RHBImplementationDef_notify_somdd(implDef,ev,0);

	RHBSOM_Trace("somoa_deactivate_impl - notified somdd");

	somExceptionFree(ev);

	RHBSOM_Trace("somoa_deactivate_impl - calling end_listen");

	RHBImplementationDef_end_listen(implDef,ev);

	RHBSOM_Trace("somoa_deactivate_impl - called end_listen");

	somExceptionFree(ev);

	RHBORB_guard(implDef)

	data=implDef->server.data;
	implDef->server.data=NULL;

	if (data)
	{
		somd_atomic_inc(&data->deactivating);
	}

	RHBSOM_Trace("somoa_deactivate_impl - state changed");

	RHBORB_server_state_changed(implDef->orb);

	RHBSOM_Trace("somoa_deactivate_impl - state changed okay");

	if (data)
	{
		others=c_orb->impls;

		while (others)
		{
			RHBSOM_Trace("somoa_deactivate_impl - others to deal with");

			if ((others!=implDef) && (others->server.data==data))
			{
				others->server.data=NULL;

				data->lpVtbl->Release(data);

				if (others->outstanding_request_list ||
					others->transmitting.head ||
					others->connection.fd)
				{
					RHBImplementationDef_AddRef(others);

					while (others->outstanding_request_list ||
						others->transmitting.head)
					{
	#if defined(_WIN32) || defined(_DEBUG)
						debug_somPrintf(("Waiting for server %p for (%s:%d) to finish it's requests\n",
								others,
								inet_ntoa(others->server.client.addr.u.ipv4.sin_addr),
								(int)ntohs(others->server.client.addr.u.ipv4.sin_port)));
	#endif

	#ifdef USE_THREADS
		#if defined(USE_PTHREADS)
						somd_wait(&others->p_EventSem,__FILE__,__LINE__);
		#else
			#ifdef _WIN32
						somd_wait(others->h_EventSem,__FILE__,__LINE__);
			#else
						somd_wait(&others->m_EventSem,__FILE__,__LINE__);
			#endif
		#endif
	#else
			/*			SOMD_bomb("never imagined it would get here");*/
						RHBORB_idle(c_orb,ev,0);
	#endif

					}

					RHBORB_unguard(implDef)
					RHBImplementationDef_interrupt_closed(others,ev);
					RHBImplementationDef_remove_client_references(others);
					RHBImplementationDef_Release(others);
					RHBORB_guard(implDef)

					others=c_orb->impls;
				}
				else
				{
					others=others->next;
				}
			}
			else
			{
				others=others->next;
			}
		}

		RHBSOM_Trace("somoa_deactivate_impl - dealt with others");
	}

	if (data)
	{
		listener=data->listener;
		data->listener=NULL;
		obj=data->somdServer;
		data->somdServer=NULL;

		RHBSOM_Trace("somoa_deactivate_impl - _set_ImplDefObject to NULL");

		if (obj) 
		{
			SOMDServer__set_ImplDefObject(obj,ev,NULL);
		}
	}

	RHBORB_unguard(implDef)

	if (obj) 
	{
		somReleaseObjectReference(obj);
	}

	RHBSOM_Trace("somoa_deactivate_impl - releasing impldef");

	RHBImplementationDef_Release(implDef);

	RHBSOM_Trace("somoa_deactivate_impl - released impldef");

	RHBOPT_unused(somSelf)

	if (ev->_major) 
	{
		somExceptionFree(ev);
	}

	if (data)
	{
		ImplementationDef SOMSTAR active=NULL;
		RHBORB_guard(0)
		active=data->implDef;
		data->implDef=NULL;
		RHBORB_unguard(0)

		RHBSOM_Trace("somoa_deactivate_impl - clearing up waiters");
		
		somd_atomic_dec(&data->deactivating);

		if (somThis->waiter.servers==data)
		{
			somThis->waiter.servers=data->next;
		}
		else
		{
			RHBServerData *p=somThis->waiter.servers;

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

		data->lpVtbl->Release(data);

		RHBSOM_Trace("somoa_deactivate_impl - done with waiters");

		if (active)
		{
			somReleaseObjectReference(active);
		}

	}

	RHBSOM_Trace("somoa_deactivate_impl - releasing listener");

	if (listener)
	{
		RHBImplementationDef_Release(listener);
	}

	RHBSOM_Trace("somoa_deactivate_impl - finished");
}

SOM_Scope void  SOMLINK somoa_obj_is_ready(
		SOMOA SOMSTAR somSelf,
		Environment *ev, 
		SOMDObject SOMSTAR obj,
		ImplementationDef SOMSTAR impl)
{
    SOMOA_parent_BOA_obj_is_ready(somSelf, ev, obj, impl);
}

SOM_Scope void  SOMLINK somoa_deactivate_obj(
		SOMOA SOMSTAR somSelf, 
		Environment *ev, 
		SOMDObject SOMSTAR obj)
{
    SOMOA_parent_BOA_deactivate_obj(somSelf, ev, obj);
}

SOM_Scope IIOP_SOMDServiceContextList SOMLINK somoa_get_service_contexts(
	SOMOA SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMDObject SOMSTAR obj)
{
	IIOP_SOMDServiceContextList seq={0,0,NULL};
	RHBServerRequest *request=NULL;
	RHBORB_ThreadTask *thread=RHBSOMD_get_thread();

	if (!thread) 
	{
		RHBOPT_throw_StExcep(ev,INTERNAL,BadThread,NO);

		return seq;
	}

	request=thread->server_request;

	if (request)
	{
		RHBSOM_Trace("Look at service_context")

		return request->somd_contexts;
	}

	RHBOPT_unused(somSelf)
	RHBOPT_unused(obj)

	return seq;
}

struct somoa_notify_somdd
{
	SOMDServer SOMSTAR somdd;
	Request SOMSTAR req;
	somId id;
};

RHBOPT_cleanup_begin(somoa_notify_somdd_cleanup,pv)

	struct somoa_notify_somdd *data=pv;

	if (data->id) SOMFree(data->id);
	if (data->req) somReleaseObjectReference(data->req);
	if (data->somdd) somReleaseObjectReference(data->somdd);

RHBOPT_cleanup_end

SOM_Scope void SOMLINK somoa_notify_somdd(SOMOA SOMSTAR somSelf,
										  Environment *ev,
										  short transition,
										  corbastring impl_id,
										  corbastring impl_server_ior,
										  ORBStatus rc)
{
	SOMOAData *somThis=SOMOAGetData(somSelf);
	RHBORB *c_orb=somThis->waiter.c_orb;
	struct somoa_notify_somdd data={NULL,NULL,NULL};
	char * RHBOPT_volatile message=NULL;
#ifdef _PLATFORM_MACINTOSH_
	long my_pid=0;
#else
	#ifdef _WIN32
		long my_pid=GetCurrentProcessId();
	#else
		long my_pid=getpid();
	#endif
#endif

	switch (transition)
	{
	case 0: message="impl_down"; break;
	case 1: message="impl_up"; break;
	case -1: message="impl_failed"; break;
	}

	if (!message)
	{
		RHBOPT_throw_StExcep(ev,BAD_PARAM,BadParm,MAYBE);
		return;
	}

	if (ev->_major) return;

#ifdef _PLATFORM_MACINTOSH_
	/* do activation with sockets to prove
		that comms work */

	if (transition!=1)
	{
		RHBSOMD_notify_somdd_impl_using_ipc(
				c_orb,ev,
				transition,
				impl_id,impl_server_ior,rc);

		return;
	}
#endif

	RHBOPT_cleanup_push(somoa_notify_somdd_cleanup,&data);

	data.somdd=ImplRepository__get_SOMDD_ServerObject(SOMD_ImplRepObject,ev);

	if (!data.somdd)
	{
		RHBOPT_throw_StExcep(ev,OBJ_ADAPTER,UnexpectedNULL,NO);
	}

	if (!ev->_major)
	{
		if (RHBORB_isSOMDObject(c_orb,data.somdd))
		{
			NamedValue nv=RHBSOMD_Init_NamedValue;

			nv.argument._type=TC_void;

			SOMDObject_create_request(data.somdd,ev,
					0,message,0,&nv,&data.req,0);

			if (!ev->_major)
			{
				Request_add_arg(data.req,ev,
						"impl_id",
						TC_string,
						&impl_id,
						0,
						ARG_IN);

				switch (transition)
				{
				case 1:
					Request_add_arg(data.req,ev,
						"impl_ior",
						TC_string,
						&impl_server_ior,
						0,
						ARG_IN);
					break;
				case -1:
					Request_add_arg(data.req,ev,
						"pid",
						TC_long,
						&my_pid,
						0,
						ARG_IN);
					Request_add_arg(data.req,ev,
						"rc",
						TC_long,
						&rc,
						0,
						ARG_IN);
					break;
				}

				Request_invoke(data.req,ev,0);
			}
		}
		else
		{
			somToken r=NULL;
			data.id=somIdFromString(message);
			if (data.id)
			{
				switch (transition)
				{
				case 1:
					somva_SOMObject_somDispatch(data.somdd,
						&r,data.id,
						data.somdd,ev,impl_id,impl_server_ior);
					break;
				case 0:
					somva_SOMObject_somDispatch(data.somdd,
						&r,data.id,
						data.somdd,ev,impl_id);
					break;
				case -1:
					somva_SOMObject_somDispatch(data.somdd,
						&r,data.id,
						data.somdd,ev,impl_id,my_pid,rc);
					break;
				}
			}
		}
	}

	RHBOPT_cleanup_pop();
}

SOM_Scope void SOMLINK somoa__set_default_SOMDServer(SOMOA SOMSTAR somSelf,
	Environment *ev,
	SOMDServer SOMSTAR server)
{
}

SOM_Scope SOMDServer SOMSTAR SOMLINK somoa__get_default_SOMDServer(SOMOA SOMSTAR somSelf,
	Environment *ev)
{
	SOMDServer SOMSTAR ref=NULL;
	RHBORB_guard(0)
	if ((!ev->_major) && SOMD_ServerObject)
	{
		ref=SOMDServer_somDuplicateReference(SOMD_ServerObject);
	}
	RHBORB_unguard(0)
	return ref;
}

static long get_modifier_long(_IDL_SEQUENCE_somModifier *mods,const char *name,long defaultValue)
{
	long value=defaultValue;
	unsigned long len=mods->_length;
	if (len)
	{
		somModifier *mod=mods->_buffer;

		while (len--)
		{
			if (!strcmp(mod->name,name))
			{
				value=atol(mod->value);

				break;
			}

			mod++;
		}
	}

	return value;
}

struct somd_mid_request
{
	char *id;
	_IDL_SEQUENCE_somModifier mods;
	unsigned short describe_flags;
};

RHBOPT_cleanup_begin(somd_mid_request_cleanup,pv)

struct somd_mid_request *data=pv;
	Environment ev;

	SOM_InitEnvironment(&ev);

	if (data->describe_flags==ORB_DESCRIBE_FREE)
	{
		if (data->id) SOMFree(data->id);

		SOMD_FreeType(&ev,&data->mods,somdTC_sequence_somModifier);
	}

	SOM_UninitEnvironment(&ev);

RHBOPT_cleanup_end

SOM_Scope char * SOMLINK somoa_contained__get_id(
	SOMOA SOMSTAR somSelf,
	Environment *ev,
	SOMObject SOMSTAR obj)
{
	SOMOAData *somThis=SOMOAGetData(somSelf);
	char * RHBOPT_volatile id=NULL;
	struct somd_mid_request data={NULL,{0,0,NULL},0};

	RHBOPT_cleanup_push(somd_mid_request_cleanup,&data);

	ORB_contained_describe(SOMD_ORBObject,ev,
			obj,&data.describe_flags,
			NULL,&data.mods,NULL,&data.id);

	if (!ev->_major)
	{
		if (data.id && (data.id[0]==':'))
		{
			/* need to create a pretty version */
			long majorVersion=get_modifier_long(&data.mods,"majorversion",0);
			long minorVersion=get_modifier_long(&data.mods,"minorversion",0);

			id=RHBORB_abs_to_id(somThis->waiter.c_orb,
								data.id,
								majorVersion,
								minorVersion);
		}
		else
		{
			if (data.describe_flags == ORB_DESCRIBE_FREE)
			{
				id=data.id;
				data.id=NULL;
			}
			else
			{
				id=somd_dupl_string(data.id);
			}
		}
	}

	RHBOPT_cleanup_pop();

	return id;
}
