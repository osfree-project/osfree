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

/* This contains the client 'Request'  */

struct SOMCDR_marshal_filter_data
{
	RHBRequest *request;
	NamedValue *nv;
	SOMDServer SOMSTAR somdServer;
};

struct RHBRequestMap
{
	SOMObject SOMSTAR obj;
	SOMDObject SOMSTAR somdref;
};

static void request_release_map(RHBRequest *somThis)
{
	struct RHBRequestMap *map=somThis->in_params._buffer;
	unsigned long l=somThis->in_params._length;

	while (l--)
	{
		SOMDObject SOMSTAR dobj=map->somdref;
		map->somdref=NULL;
		if (dobj)
		{
			somReleaseObjectReference(dobj);
		}
		map++;
	}

	somThis->in_params._length=0;
	somThis->in_params._maximum=0;
	map=somThis->in_params._buffer;
	somThis->in_params._buffer=NULL;
	if (map) SOMFree(map);
}

static void SOMLINK request_read_object(
		SOMCDR_unmarshal_filter *filter,
		Environment *ev,
		void *value,
		TypeCode tc,
		RHBORBStream_GIOPInput SOMSTAR stream)
{
	switch (TypeCode_kind(tc,ev))
	{
	case tk_Principal:
		((Principal SOMSTAR *)value)[0]=RHBORBStream_GIOPInput_read_Principal(stream,ev);
		break;
	case tk_objref:
		((SOMObject SOMSTAR *)value)[0]=RHBORBStream_GIOPInput_read_SOMObject(stream,ev,tc);
		break;
	default:
		RHBOPT_throw_StExcep(ev,MARSHAL,BadTypeCode,MAYBE);
		break;
	}
}


struct RHBRequest_decode_exception_data
{
	char *ex_id;
	char *rep_id;
	any ex_val;
	RHBContained *ex;
	any desc_value;
	Repository SOMSTAR rep;
	Contained SOMSTAR cnd;
	InterfaceDef SOMSTAR iface;
	_IDL_SEQUENCE_somModifier mods;
};

RHBOPT_cleanup_begin(RHBRequest_decode_exception_cleanup,pv)

	struct RHBRequest_decode_exception_data *data=pv;
	Environment ev;
	SOM_InitEnvironment(&ev);
	if (data->rep) somReleaseObjectReference(data->rep);
	if (data->cnd) somReleaseObjectReference(data->cnd);
	if (data->iface) somReleaseObjectReference(data->iface);
	if (data->ex) RHBContained_Release(data->ex);
	if (data->ex_id) SOMFree(data->ex_id);
	if (data->rep_id) SOMFree(data->rep_id);
	SOMD_FreeType(&ev,&data->ex_val,TC_any);
	SOMD_FreeType(&ev,&data->desc_value,TC_any);
	SOMD_FreeType(&ev,&data->mods,somdTC_sequence_somModifier);
	SOM_UninitEnvironment(&ev);

RHBOPT_cleanup_end

static void RHBRequest_decode_exception(
			RHBRequest *somThis,
			Environment *ev,
			SOMCDR_unmarshal_filter *filter,
			CORBA_DataInputStream SOMSTAR reply_stream)
{
	/* need to convert exception name in IDL: format to ::' format */
	struct RHBRequest_decode_exception_data data;

	if (ev->_major) return;

	memset(&data,0,sizeof(data));

	if (!reply_stream)
	{
		RHBOPT_throw_StExcep(ev,UNKNOWN,UnknownError,NO);

		return;
	}

	RHBOPT_cleanup_push(RHBRequest_decode_exception_cleanup,&data);

	data.rep_id=CORBA_DataInputStream_read_string(reply_stream,ev);

	if (somThis->reply.reply_status==GIOP_SYSTEM_EXCEPTION)
	{
		if (data.rep_id) 
		{
			if (!memcmp(data.rep_id,"::",2))
			{
				data.ex_id=somd_dupl_string(data.rep_id);
			}
		}
	}

	if (!data.ex_id)
	{
		RHBORB *orb=somThis->orb;

		somExceptionFree(ev);

		data.ex=RHBORB_get_contained(orb,ev,data.rep_id);

		if (data.ex && !ev->_major)
		{
			char *p=RHBContained_get_id(data.ex,ev);

			if (p && !ev->_major)
			{
				data.ex_id=somd_dupl_string(p);
			}
		}

		if (!data.ex_id)
		{
			somPrintf("unknown exception %s\n",data.rep_id);
		}
	}

	if (data.ex_id && !ev->_major)
	{
		if (!data.ex_val._type)
		{
			switch (somThis->reply.reply_status)
			{
			case GIOP_SYSTEM_EXCEPTION:
				data.ex_val._type=RHBORB_get_exception_tc(
					somThis->request.impl->orb,
					SYSTEM_EXCEPTION,
					data.ex_id);
				break;
			case GIOP_USER_EXCEPTION:
				data.ex_val._type=RHBORB_get_exception_tc(
					somThis->request.impl->orb,
					USER_EXCEPTION,
					data.ex_id);
				break;
			}
		}

		if (data.ex_val._type)
		{
			int i=TypeCode_size(data.ex_val._type,ev);

			if (ev->_major==NO_EXCEPTION)
			{
				if (i)
				{
					data.ex_val._value=SOMMalloc(i);

					if (data.ex_val._value)
					{
						memset(data.ex_val._value,0,i);

						RHBCDR_unmarshal(filter,ev,data.ex_val._value,data.ex_val._type,reply_stream);
					}
					else
					{
						RHBOPT_throw_StExcep(ev,MARSHAL,NoMemory,MAYBE);
					}
				}

				if (ev->_major==NO_EXCEPTION)
				{
					switch (somThis->reply.reply_status)
					{
						case GIOP_SYSTEM_EXCEPTION:
							if (SOM_TraceLevel)
							{
								somPrintf("somSetException(%s) at %s:%d\n",
									data.ex_id,
									__FILE__,__LINE__);
							}
							somSetException(
									ev,
									SYSTEM_EXCEPTION,
									data.ex_id,
									data.ex_val._value);
							data.ex_val._value=NULL;
							break;
						case GIOP_USER_EXCEPTION:
							if (SOM_TraceLevel)
							{
								somPrintf("somSetException(%s) at %s:%d\n",
									data.ex_id,
									__FILE__,__LINE__);
							}
							somSetException(
									ev,
									USER_EXCEPTION,
									data.ex_id,
									data.ex_val._value);
							data.ex_val._value=NULL;
							break;
					}
				}
			}
		}
	}

	if (!ev->_major)
	{
		RHBOPT_throw_StExcep(ev,UNKNOWN,UnknownError,NO);
	}

	RHBOPT_cleanup_pop();
}

struct rhbRequest_exec_local
{
	RHBRequest *somThis;
	SOMObject SOMSTAR local_target;
	Context SOMSTAR ctx;
	somId id;
	SOMDServer SOMSTAR server;
	NVList SOMSTAR args;
};

RHBOPT_cleanup_begin(rhbRequest_exec_local_cleanup,pv)

struct rhbRequest_exec_local *data=pv;
Environment ev;

	SOM_InitEnvironment(&ev);

	if (data->ctx) somReleaseObjectReference(data->ctx);
	if (data->server) somReleaseObjectReference(data->server);
	if (data->id) SOMFree(data->id);
	if (data->args) somReleaseObjectReference(data->args);

	RHBRequest_SetComplete(data->somThis,&ev);

	if (data->local_target)
	{
#ifdef SOMObject_somRelease
		somReleaseObjectReference(data->local_target);
#else
		if (SOMObject_somIsA(data->local_target,_SOMRefObject))
		{
			SOMRefObject_somRelease(data->local_target);
		}
#endif
	}

	SOM_UninitEnvironment(&ev);

	RHBRequest_Release(data->somThis);

RHBOPT_cleanup_end

static void SOMLINK rhbRequest_exec_local(
		void *pv,
		Environment *ev)
{
struct rhbRequest_exec_local data={NULL,NULL,NULL,NULL,NULL,NULL};
RHBRequest *somThis=pv;

	data.somThis=somThis;

	somThis->was_local_target=1;
	somThis->reply.reply_status=GIOP_NO_EXCEPTION;

	ev=&somThis->local.ev;

	RHBOPT_cleanup_push(rhbRequest_exec_local_cleanup,&data);

	data.local_target=SOMDClientProxy__get_sommProxyTarget(somThis->request.somobj);

	if (ev->_major || !data.local_target)
	{
		RHBOPT_throw_StExcep(ev,INV_OBJREF,BadObjref,NO);
	}
	else
	{
		if (!strcmp(somThis->request.operation,"somGetClassName"))
		{
			char **h=somThis->request.result->argument._value;
			char *p=SOMObject_somGetClassName(data.local_target);
			*h=somd_dupl_string(p);
		}
		else 
		{
			data.server=RHBImplementationDef_acquire_somdServer(somThis->request.impl,ev);

			if (ev->_major || !data.server) 
			{
				RHBOPT_throw_StExcep(ev,
					NO_IMPLEMENT,
					ServerNotActive,NO);
			}
			else
			{
				data.id=somIdFromString(somThis->request.operation);

				if (!SOMObject_somRespondsTo(data.local_target,data.id))
				{
					RHBOPT_throw_StExcep(ev,BAD_OPERATION,BadMethodName,NO);
				}
				else
				{
					if (somThis->request.send_context)
					{
						unsigned int i=0;

						data.ctx=SOMClass_somNew(somThis->orb->somClass_Context_ref);

						while (i < somThis->request.context_values._length)
						{
							char *name=somThis->request.context_values._buffer[i++];
							char *value=somThis->request.context_values._buffer[i++];

							Context_set_one_value(data.ctx,ev,name,value);
						}
					}

					if (!ev->_major)
					{
						if (somThis->request.arg_list)
						{
							data.args=NVList_somDuplicateReference(somThis->request.arg_list);
						}

#ifndef USE_THREADS
						SOM_TRY
#endif

						SOMDServer_somdDispatchArgs(
									data.server,
									ev,
									data.ctx,
									data.local_target,
									data.id,
									&somThis->request.result->argument,
									data.args);
#ifndef USE_THREADS
						SOM_CATCH_ALL
						RHBOPT_throw_StExcep(ev,
								INTERNAL,
								DispatchError,
								MAYBE);
						SOM_ENDTRY
#endif
					}
				}
			}

		}
	}

	RHBOPT_cleanup_pop();
}

#ifdef USE_THREADS
static void SOMLINK rhbRequest_exec_failed(void *pv)
{
	RHBRequest *somThis=pv;
	Environment ev;
	SOM_InitEnvironment(&ev);
	RHBOPT_throw_StExcep((&ev),INTERNAL,DispatchError,NO);
	RHBRequest_SetComplete(somThis,&ev);
	SOM_UninitEnvironment(&ev);
	RHBRequest_Release(somThis);
}
#endif

static ORBStatus rhbRequest_send_local(
		    RHBRequest *somThis,
		    Environment *ev,
			Flags flags,
			boolean bInvoke)
{
	RHBRequest_AddRef(somThis);

#ifdef USE_THREADS
	if (bInvoke)
	{
#endif
		rhbRequest_exec_local(somThis,ev);
#ifdef USE_THREADS
	}
	else
	{
		/* spawn a thread to run the command in */

		RHBORB *orb=somThis->orb;
		somThis->local.task.start=rhbRequest_exec_local;
		somThis->local.task.failed=rhbRequest_exec_failed;
		somThis->local.task.param=somThis;
		RHBORB_guard(orb)
		if (somThis->pending)
		{
			RHBORB_StartThreadTask(orb,&somThis->local.task);
		}
		RHBORB_unguard(orb)
	}
#endif

	return 0;
}

static void SOMLINK RHBRequest_write_object(
	struct SOMCDR_marshal_filter *filter,
	Environment *ev,
	any *value,
	RHBORBStream_GIOPOutput SOMSTAR marshal_stream)
{
#ifdef USE_THREADS
	RHBOPT_ASSERT(!somd_is_guarded());
#endif

	switch (TypeCode_kind(value->_type,ev))
	{
	case tk_Principal:
		RHBORBStream_GIOPOutput_write_Principal(marshal_stream,ev,
				*((Principal SOMSTAR *)(value->_value)));
		break;
	case tk_objref:
		if (value->_value)
		{
			SOMObject SOMSTAR obj=*((SOMObject SOMSTAR *)(value->_value));
			RHBRequest *somThis=filter->data->request;

			if (obj && !(RHBORB_isSOMDObject(somThis->orb,obj)))
			{
				SOMDObject SOMSTAR dobj=NULL;
				struct RHBRequestMap *map=somThis->in_params._buffer;
				unsigned long i=somThis->in_params._length;

				while (i--)
				{
					if (map->obj==obj)
					{
						dobj=map->somdref;
						break;
					}
					map++;
				}

				if ((!dobj) && (filter->data->somdServer))
				{
					dobj=SOMDServer_somdRefFromSOMObj(filter->data->somdServer,ev,obj);

					if (dobj && !ev->_major)
					{
					/* need to look up mapping for converting a local
						object to a SOMDObject for transmission in 
						the request,

						if as response to a server request, can use that SOMDServer
						to map the object, else use SOMD_ServerObject if it exists

						should maintain any mapping for the life of the request
						so if the request gets forwarded, we will use the same
						SOMDObject

						once request is complete, release all the SOMDObjects used
						in the mapping, this includes
							(a) if the request was cancelled
							(b) completion of the request
							(c) receipt of an exception for the request
					*/
						if (somThis->in_params._length==somThis->in_params._maximum)
						{
							unsigned long k=somThis->in_params._maximum+16;
							void *pv=somThis->in_params._buffer;
							i=somThis->in_params._length;
							map=SOMMalloc(k * sizeof(map[0]));
							if (i)
							{
								memcpy(map,pv,i*sizeof(map[0]));
							}
							somThis->in_params._buffer=map;
							if (pv) SOMFree(pv);
							somThis->in_params._maximum=k;
						}

						somThis->in_params._buffer[somThis->in_params._length].obj=obj;
						somThis->in_params._buffer[somThis->in_params._length].somdref=dobj;
						somThis->in_params._length++;
					}
				}

				obj=dobj;

				if (!obj)
				{
					RHBOPT_throw_StExcep(ev,MARSHAL,BadObjref,NO);
				}
#ifdef _DEBUG
				else
				{
					char *p=filter->data->nv->name;
					somPrintf("param for %s swizzled to...\n",p);
					SOMDObject_somDumpSelf(obj,0);
				}
#endif
			}

			if (!ev->_major)
			{
				RHBORBStream_GIOPOutput_write_Object(marshal_stream,ev,obj);
			}
		}
		else
		{
			RHBOPT_throw_StExcep(ev,MARSHAL,UnexpectedNULL,MAYBE);
		}
		break;
	default:
		RHBOPT_throw_StExcep(ev,MARSHAL,BadTypeCode,MAYBE);
		break;
	}
}

static ORBStatus rhbRequest_marshal_request(
	  RHBRequest *somThis,
	  Environment *ev,
	  RHBORBStream_GIOPOutput SOMSTAR marshal_stream,
	  SOMDServer SOMSTAR somdServer)
{
	GIOP_RequestHeader_1_0 rq;
	struct SOMCDR_marshal_filter_data data;
	struct SOMCDR_marshal_filter filter;

	/* this should maintain a list of non-SOMDObjects 
		mapped against a SOMDObject used to marshal
		an object in a request
		this mapping needs to be created only during
		the initial send and not as a result of forwarding,
		as the forwarding may occur at an 'inconvenient time'
		and calling out to a user SOMDObject::somdRefFromSOMObject
		is not desireable

		the normal send should occur first, the mapping
		then held in the request and reused during the forwarding,
		if a mapping does not exist while forwarding this then counts
		as a marshalling error
    */

	data.request=somThis;
	data.nv=NULL;
	data.somdServer=somdServer;

	filter.data=&data;
	filter.write_object=RHBRequest_write_object;

	if (somThis->marshalled_data._length)
	{
		SOMD_bomb("this should not be set");
	}

	rq.service_context._length=0;
	rq.service_context._maximum=0;
	rq.service_context._buffer=NULL;

	rq.request_id=RHBImplementationDef_allocate_sequence(somThis->request.impl);

	somThis->request.lSequence=rq.request_id;

	if (somThis->request.send_flags & INV_NO_RESPONSE)
	{
		rq.response_expected=0;
	}
	else
	{
		rq.response_expected=1;
	}

	rq.object_key._length=somThis->request.object_key._length;
	rq.object_key._maximum=somThis->request.object_key._maximum;
	rq.object_key._buffer=somThis->request.object_key._buffer;

	rq.operation=somThis->request.operation;
	rq.requesting_principal=somThis->request.principal;

	RHBCDR_marshal(&filter,ev,
				&rq,somdTC_GIOP_RequestHeader_1_0,
				marshal_stream);

	/* now actually write the IN and INOUT arguments into osRequest */

	if (somThis->request.arg_list)
	{
		long count=0;

		if (!NVList_get_count(somThis->request.arg_list,ev,&count))
		{
			long j=0;

			while ((j < count) && !ev->_major)
			{
				NamedValue nv=RHBSOMD_Init_NamedValue;

				if (!NVList_get_item(somThis->request.arg_list,ev,j++,
						&nv.name,
						&nv.argument._type,
						&nv.argument._value,
						&nv.len,
						&nv.arg_modes))
				{
					if (nv.arg_modes & (ARG_IN | ARG_INOUT))
					{
						data.nv=&nv;

						RHBCDR_marshal(&filter,
									ev,
									nv.argument._value,
									nv.argument._type,
									marshal_stream);
					}
				}
			}
		}
	}

	/* now write out the context list if any as on page 12-14 */

	/*	write out as sequence<string>
		where strings are pairs name:value,
		eg must be even number of strings */

	if (somThis->request.send_context && !ev->_major)
	{	
		unsigned int i=0;
		unsigned long l=somThis->request.context_values._length;

		CORBA_DataOutputStream_write_ulong(marshal_stream,ev,l);

		while ((i < l) && !ev->_major)
		{
			char *p=somThis->request.context_values._buffer[i++];

			CORBA_DataOutputStream_write_string(marshal_stream,ev,p);
		}
	}

	if (ev->_major) return SOMDERROR_Marshal;

	return 0;
}

struct rhbRequest_send
{
	RHBORBStream_GIOPOutput SOMSTAR stream;
	ORBStatus r;
	Environment *ev;
	RHBRequest *somThis;
	SOMDServer SOMSTAR somdServer;
};

RHBOPT_cleanup_begin(rhbRequest_send_cleanup,pv)

struct rhbRequest_send *data=pv;

	if (data->stream) 
	{
		if (data->r || data->ev->_major)
		{
			RHBORBStream_GIOPOutput_completed(data->stream,data->ev);
			RHBRequest_SetComplete(data->somThis,data->ev);
		}

		somReleaseObjectReference(data->stream);
	}

	if (data->somdServer)
	{
		somReleaseObjectReference(data->somdServer);
	}

RHBOPT_cleanup_end

static ORBStatus rhbRequest_send(
	   RHBRequest *somThis,
	   Environment *ev,
	   Flags flags,
	   boolean bInvoke,
	   SOMDServer SOMSTAR somdServer)
{
	struct rhbRequest_send data={NULL,SOMDERROR_Marshal,NULL,NULL,NULL};
	/* this should just pass the request to the ImplementationDef
	   to put in it's queue ready to shove out of the door
	*/

	data.r=SOMDERROR_BadEnvironment;

	RHBOPT_ASSERT(ev);
	RHBOPT_ASSERT(ev->_major==NO_EXCEPTION);

	RHBOPT_ASSERT(!(somThis->cancelled));
	RHBOPT_ASSERT(!(somThis->deleting));

	if (ev && (ev->_major==NO_EXCEPTION))
	{
		data.ev=ev;
		data.somThis=somThis;

		RHBOPT_unused(flags)

		somThis->pending=1;
		somThis->request.send_flags=flags;

	/*	somPrintf("request(%s)\n",somThis->request.operation);*/

		if (somThis->request.impl->connection.is_listener)
		{
			/* this is a local send */

			data.r=rhbRequest_send_local(somThis,ev,flags,bInvoke);
		}
		else
		{
		/* this linearised the arguments */

			if (somdServer)
			{
				data.somdServer=SOMDServer_somDuplicateReference(somdServer);
			}

			RHBOPT_cleanup_push(rhbRequest_send_cleanup,&data);

	#ifdef somNewObject
			data.stream=somNewObject(RHBORBStream_GIOPOutput);
	#else
			data.stream=RHBORBStream_GIOPOutputNew();
	#endif

			RHBORBStream_GIOPOutput__set_c_orb(data.stream,ev,somThis->orb);
			RHBORBStream_GIOPOutput__set_streamed_data(data.stream,ev,NULL);

			data.r=rhbRequest_marshal_request(somThis,ev,data.stream,data.somdServer);

			if (data.r || ev->_major)
			{
				debug_somPrintf(("%s:%d, marshal_request failed with %ld\n",
					__FILE__,__LINE__,data.r));

				RHBOPT_throw_StExcep(ev,MARSHAL,Marshal,NO)
			}
			else
			{
				somThis->marshalled_data=RHBORBStream_GIOPOutput__get_streamed_data(data.stream,ev);

				/* this schedules the request for transmission */

				data.r=RHBImplementationDef_queue_request(somThis->request.impl,ev,somThis);
			}

			RHBOPT_cleanup_pop();
		}
	}

	return data.r;
}

struct rhbRequest_invoke
{
	RHBRequest *request;
};

RHBOPT_cleanup_begin(rhbRequest_invoke_cleanup,pv)

struct rhbRequest_invoke *data=pv;

	RHBRequest_Release(data->request);

RHBOPT_cleanup_end

static ORBStatus rhbRequest_invoke(
		 RHBRequest *somThis,
		 Environment *ev,
		 Flags flags,
		 SOMDServer SOMSTAR somdServer)
{
	ORBStatus RHBOPT_volatile r=SOMDERROR_BadEnvironment;

	RHBOPT_ASSERT(ev);
	RHBOPT_ASSERT(ev->_major==NO_EXCEPTION);

	if (ev && (ev->_major==NO_EXCEPTION))
	{
		struct rhbRequest_invoke data={NULL};
		data.request=somThis;

		RHBRequest_AddRef(somThis);

		RHBOPT_cleanup_push(rhbRequest_invoke_cleanup,&data);

		r=RHBRequest_send(somThis,ev,flags,1,somdServer);

		if (r) 
		{
			if (!ev->_major)
			{
				r=RHBRequest_get_response(somThis,ev,0);
			}
		}
		else
		{
			r=RHBRequest_get_response(somThis,ev,flags);
		}

		RHBOPT_cleanup_pop();
	}


	return r;
}

static void rhbRequest_send_marshalled(
		RHBRequest *somThis,
		Environment *ev,
		RHBImplementationDef *impl)
{
	RHBOPT_unused(impl)

	if (somThis->marshalled_data._length)
	{
		RHBImplementationDef_write_GIOP_msg(
			somThis->request.impl,
			ev,
			&somThis->protocol_version,
			GIOP_Request,
			&somThis->marshalled_data);
	}
}

static void SOMLINK rhbRequest_delete(RHBRequest *somThis)
{
	RHBImplementationDef *impl;
	SOMObject SOMSTAR object=NULL;
	Principal SOMSTAR principal=NULL;
	CORBA_DataInputStream SOMSTAR in_stream=NULL;
	NVList SOMSTAR arg_list=NULL;

	if (somThis->deleting)
	{
		debug_somPrintf(("already deleting RHBRequest(%p)",somThis));

		return;
	}

	RHBORB_guard(somThis->orb)

	if (somThis->deleting)
	{
		debug_somPrintf(("already deleting RHBRequest(%p)",somThis));

		RHBORB_unguard(somThis->orb)

		return;
	}

	if (somThis->pending)
	{
		debug_somPrintf(("cant delete RHBRequest(%p) while pending",somThis));

		somThis->deleting=0;

		RHBORB_unguard(somThis->orb);

		return;
	}

	somThis->deleting=1;

	if (somThis->lUsage)
	{
		debug_somPrintf(("cant delete RHBRequest(%p) with a count",somThis));

		somThis->deleting=0;

		RHBORB_unguard(somThis->orb);

		return;
	}

	if (somThis->container)
	{
		RHBRequestList_remove(somThis->container,somThis);
	}

	if (somThis->marshalled_data._buffer)
	{
		SOMFree(somThis->marshalled_data._buffer);
		somThis->marshalled_data._buffer=0;
	}

	arg_list=somThis->request.arg_list;
	somThis->request.arg_list=NULL;

	principal=somThis->request.principal;
	somThis->request.principal=NULL;

	object=somThis->request.somobj;
	somThis->request.somobj=NULL;

	if (somThis->request.object_key._buffer)
	{
		SOMFree(somThis->request.object_key._buffer);
		somThis->request.object_key._buffer=0;
	}

	in_stream=somThis->reply.stream;
	somThis->reply.stream=NULL;

	impl=somThis->request.impl;
	somThis->request.impl=NULL;

	RHBORB_unguard(somThis->orb)

	if (impl) RHBImplementationDef_Release(impl);

	if (object) somReleaseObjectReference(object);

	if (principal) somReleaseObjectReference(principal);

	if (in_stream) somReleaseObjectReference(in_stream);

	if (arg_list) somReleaseObjectReference(arg_list);

	RHBORB_Release(somThis->orb);

	if (somThis->request.context_values._length)
	{
		unsigned int i=somThis->request.context_values._length;

		while (i--)
		{
			char *p=somThis->request.context_values._buffer[i];
			if (p) SOMFree(p);
		}
	}

	if (somThis->request.context_values._maximum)
	{
		SOMFree(somThis->request.context_values._buffer);
	}

	if (somThis->request.operation)
	{
		SOMFree(somThis->request.operation);
	}

	request_release_map(somThis);

	RHBDebug_deleted(RHBRequest,somThis)

	somdExceptionFree(&somThis->local.ev);

	SOMFree(somThis);
}

#ifdef USE_THREADS
typedef struct RHBRequest_Waiting
{
#ifdef USE_PTHREADS
	pthread_cond_t pEvent;
#else 
	HANDLE hEvent;
#endif
/*	RHBORB *orb;*/
} RHBRequest_Waiting;

static void RHBRequest_Trigger(RHBRequest *rq,void *pv)
{
	RHBRequest_Waiting *w=pv;

	RHBOPT_unused(rq)

	RHBORB_guard(rq->orb)
#ifdef USE_PTHREADS
	pthread_cond_signal(&w->pEvent);
#else
	if (!SetEvent(w->hEvent)) 
	{
#ifdef _M_IX86
		__asm int 3;
#endif
	}
#endif
	RHBORB_unguard(rq->orb)
}
#endif

#ifdef USE_THREADS
/*static void SOMLINK somDispatch_wake_up(void *x,Environment *ev)
{
	struct RHBRequest *req=x;
	debug_somPrintf(("somDispatch_wakeup(%p)\n",x));
	RHBRequest_cancel(req);
	RHBOPT_unused(ev)
}*/

struct RHBRequest_WaitCompleteData
{
	RHBRequest *somThis;
	boolean locked;
	RHBRequest_Waiting waiting;
};

RHBOPT_cleanup_begin(RHBRequest_WaitComplete_cleanup,pv)

	struct RHBRequest_WaitCompleteData *data=pv;

	RHBRequest_SetCompletionProc(data->somThis,NULL,NULL);

	if (data->locked)
	{
		RHBORB_unguard(orb);
		data->locked=0;
	}

	RHBRequest_Release(data->somThis);

#ifdef USE_PTHREADS
	pthread_cond_destroy(&data->waiting.pEvent);
#else
	CloseHandle(data->waiting.hEvent);
#endif
RHBOPT_cleanup_end

static void RHBRequest_WaitComplete(RHBRequest *somThis)
{
struct RHBRequest_WaitCompleteData data={NULL,0};
	
/*	somPrintf("RHBRequest_WaitComplete - enter\n");*/

	RHBRequest_AddRef(somThis);

	data.somThis=somThis;

	RHBORB_guard(orb)

	data.locked=1;

#ifdef USE_PTHREADS
	pthread_cond_init(&data.waiting.pEvent,RHBOPT_pthread_condattr_default);
#else
	data.waiting.hEvent=CreateEvent(NULL,0,0,NULL);
#endif

	RHBRequest_SetCompletionProc(somThis,RHBRequest_Trigger,&data.waiting);

	RHBOPT_cleanup_push(RHBRequest_WaitComplete_cleanup,&data);

	if (somThis->pending)
	{
		RHBServerRequest *sr=NULL;
		RHBORB_ThreadTask *thread=RHBSOMD_get_thread();

		if (thread)
		{
			sr=thread->server_request;
		}

		if (sr)
		{
			if (sr->cancelled)
			{
				RHBORB_unguard(orb)
				data.locked=0;
				RHBRequest_cancel(somThis);
				data.locked=1;
				RHBORB_guard(orb)
			}
			else
			{					
				if (somThis->pending)
				{
#if defined(USE_PTHREADS)
					somd_wait(&data.waiting.pEvent,__FILE__,__LINE__);
#else
					somd_wait(data.waiting.hEvent,__FILE__,__LINE__);
#endif
				}
			}
		}
		else
		{
#if defined(USE_PTHREADS)
			somd_wait(&data.waiting.pEvent,__FILE__,__LINE__);
#else
			somd_wait(data.waiting.hEvent,__FILE__,__LINE__);
#endif
		}

#ifdef _DEBUG
		if (somThis->pending)
		{
			debug_somPrintf(("Still pending...%s ",somThis->request.operation));
	/*		if (somThis->pending) bomb("wait not complete");*/
#ifdef RHBSocket_dump
			if (somThis->request.impl)
			{
				if (somThis->request.impl->connection.fd_ptr)
				{
					RHBSocket_dump(somThis->request.impl->connection.fd_ptr);
				}
				else
				{
					debug_somPrintf(("NULL fd_ptr\n"));
				}
			}
			else
			{
				debug_somPrintf(("No Implementation\n"));
			}
#else
			debug_somPrintf(("\n"));
#endif
		}
#endif
	}

	RHBOPT_cleanup_pop();
/*	somPrintf("RHBRequest_WaitComplete - leave\n");*/
}
#endif

struct rhbRequest_get_response_data
{
	RHBRequest *somThis;
	CORBA_DataInputStream SOMSTAR reply_stream;
	Environment *ev;
	unsigned long args_read;
	boolean retval_read;
};

RHBOPT_cleanup_begin(rhbRequest_get_response_cleanup,pv)

	struct rhbRequest_get_response_data *data=pv;

	if (data->reply_stream) somReleaseObjectReference(data->reply_stream);

	if (data->ev->_major)
	{
		Environment ev;
		SOM_InitEnvironment(&ev);
		if (data->retval_read)
		{
			NamedValue *ret=data->somThis->request.result;

			SOMD_FreeType(&ev,ret->argument._value,ret->argument._type);
		}
		if (data->args_read)
		{
			NVList SOMSTAR arg_list=data->somThis->request.arg_list;
			if (arg_list)
			{
				unsigned long i=data->args_read;
				while (i--)
				{
					NamedValue nv=RHBSOMD_Init_NamedValue;
					if (!NVList_get_item(arg_list,&ev,i,
							&nv.name,
							&nv.argument._type,
							&nv.argument._value,
							&nv.len,
							&nv.arg_modes))
					{
						if (nv.arg_modes & (ARG_INOUT | ARG_OUT))
						{
							SOMD_FreeType(&ev,nv.argument._value,nv.argument._type);
						}
					}
				}
			}
		}
		SOM_UninitEnvironment(&ev);
	}

	RHBRequest_Release(data->somThis);

RHBOPT_cleanup_end

static ORBStatus rhbRequest_get_response(
	RHBRequest *somThis,
	Environment *arg_ev,Flags flags)
{
	Environment * RHBOPT_volatile ev=arg_ev;
	ORBStatus RHBOPT_volatile status=0;
	struct rhbRequest_get_response_data data;

	if (!ev)
	{
		SOMD_bomb("bad environment passed to get_response");

		return SOMDERROR_BadEnvironment;
	}

	if (ev->_major)
	{
		SOMD_bomb("exception already raised on entry to get_response");

		return SOMDERROR_BadEnvironment;
	}

	RHBRequest_AddRef(somThis);
	data.somThis=somThis;
	data.reply_stream=NULL;
	data.ev=ev;
	data.args_read=0;
	data.retval_read=0;

	RHBOPT_cleanup_push(rhbRequest_get_response_cleanup,&data);

	while (somThis->pending)
	{
		if (flags & RESP_NO_WAIT)
		{
			status=SOMDERROR_NoMessages;
			break;
		}

#ifdef USE_THREADS
		RHBRequest_WaitComplete(somThis);
#else
		RHBORB_idle(somThis->request.impl->orb,ev,flags);
#endif
	}

	if (!status)
	{
		if (somThis->has_got_response)
		{
			status=SOMDERROR_RequestNotFound;
		}
		else
		{
			somThis->has_got_response=1;

			/* don't need the in_params map any more */

			request_release_map(somThis);

			/* should really free up all 'inout' arguments now that
				the request data has come back */

			if (somThis->request.arg_list 
				&& 
				!somThis->was_local_target)
			{
				long count=0;

				if (!NVList_get_count(somThis->request.arg_list,ev,&count))
				{
					while (count-- && !ev->_major)
					{
						NamedValue nv=RHBSOMD_Init_NamedValue;

						if (!NVList_get_item(somThis->request.arg_list,ev,count,
								&nv.name,
								&nv.argument._type,
								&nv.argument._value,
								&nv.len,
								&nv.arg_modes))
						{
							if ((nv.arg_modes & ARG_INOUT)==ARG_INOUT)
							{
								SOMD_FreeType(ev,nv.argument._value,nv.argument._type);
							}
						}
					}
				}
			}

			if (!ev->_major)
			{
				SOMCDR_unmarshal_filter filter;

				filter.read_object=request_read_object;

				data.reply_stream=somThis->reply.stream;
				somThis->reply.stream=NULL;

				if (somThis->reply.reply_status != GIOP_NO_EXCEPTION)
				{
					RHBRequest_decode_exception(somThis,ev,
						&filter,data.reply_stream);
				}
				else
				{
					if (somThis->was_local_target)
					{
						/* return the exception from the locally dispatched
							object */
						Environment noEv;
						SOM_InitEnvironment(&noEv);
						*ev=somThis->local.ev;
						somThis->local.ev=noEv;
					}
					else
					{
						/* decode return parameter then out,inout arguments */

						if (somThis->request.result && !ev->_major)
						{
							any a=somThis->request.result->argument;

							if (a._type)
							{
								if (!TypeCode_equal(a._type,ev,TC_void))
								{
									RHBCDR_unmarshal(
											&filter,
											ev,
											a._value,
											a._type,
											data.reply_stream);

									if (!ev->_major)
									{
										data.retval_read=1;
									}
								}
							}

						}

						if (somThis->request.arg_list)
						{
							long count=0;

							if (!NVList_get_count(somThis->request.arg_list,ev,&count))
							{
								NamedValue nv=RHBSOMD_Init_NamedValue;
								long i=0;

								while (count--)
								{
									if (!NVList_get_item(somThis->request.arg_list,ev,i++,
											&nv.name,
											&nv.argument._type,
											&nv.argument._value,
											&nv.len,
											&nv.arg_modes))
									{
										if (ev->_major) break;

										/* it needs to be either ARG_INOUT or ARG_OUT */

										if (nv.arg_modes & (ARG_INOUT | ARG_OUT))
										{
											RHBCDR_unmarshal(
												&filter,
												ev,
												nv.argument._value,
												nv.argument._type,data.reply_stream);
										}

										if (!ev->_major)
										{
											data.args_read++;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	RHBOPT_cleanup_pop();

	return status;
}

static ORBStatus rhbRequest_add_arg(
		RHBRequest *somThis,
		Environment *ev,
		Identifier name,
		TypeCode _type,
		void *_value,
		long length /* unused length */,
		unsigned long flags)
{
	RHBOPT_ASSERT(ev);
	RHBOPT_ASSERT(ev->_major==NO_EXCEPTION);

	if (ev->_major) return SOMDERROR_BadEnvironment;

	if (!somThis->request.arg_list)
	{
		somThis->request.arg_list=NVListNew();
	}

	return NVList_add_item(somThis->request.arg_list,ev,
				name,
				_type,
				_value,
				length,
				flags|CLIENT_SIDE);
}

static void rhbRequest_forward(
			RHBRequest *somThis,
			Environment *ev,
			SOMDObject SOMSTAR newTarget)
{
	RHBOPT_ASSERT(ev);
	RHBOPT_ASSERT(ev->_major==NO_EXCEPTION);

	if (somThis->pending == 1)
	{
		RHBImplementationDef *i=NULL;
		RHBObject *object=SOMDObject__get_c_proxy(newTarget,ev);

		RHBORB_guard(somThis->request.impl)

		if (somThis->orb->closing)
		{
			RHBORB_unguard(somThis->request.impl)

			debug_somPrintf(("Cancelling forward as ORB is closing\n"));

			RHBRequest_SetComplete(somThis,ev);

			return;
		}

		i=somThis->request.impl;
		somThis->request.impl=NULL;

		if (somThis->marshalled_data._maximum)
		{
			SOMFree(somThis->marshalled_data._buffer);
			somThis->marshalled_data._buffer=0;
			somThis->marshalled_data._maximum=0;
			somThis->marshalled_data._length=0;
		}

		if (somThis->request.somobj && !ev->_major)
		{
/*			somPrintf("change_location(%p,%p)\n",
					somThis->request.somobj,newTarget);*/
			RHBObject *cobj=SOMDObject__get_c_proxy(somThis->request.somobj,ev);
			RHBObject_change_location(
					cobj,
					ev,
					object);
		}
		else
		{
			Environment ev2;

			SOM_InitEnvironment(&ev2);

			debug_somPrintf(("not resending %s\n",somThis->request.operation));

			somThis->request.impl=i;

			if (!ev->_major)
			{
				RHBOPT_throw_StExcep(&ev2,INV_OBJREF,UnexpectedNULL,MAYBE);

				ev=&ev2;
			}

			RHBORB_unguard(i)

			RHBRequest_SetComplete(somThis,ev);

			SOM_UninitEnvironment(&ev2);

			return;
		}
		
		SOMFree(somThis->request.object_key._buffer);

		somThis->request.object_key._length=object->object_key._length;
		somThis->request.object_key._maximum=object->object_key._maximum;
		somThis->request.object_key._buffer=SOMMalloc(object->object_key._length);

		memcpy(somThis->request.object_key._buffer,
			object->object_key._buffer,
			object->object_key._length);

		/* change actual implementation */

		somThis->request.impl=object->impl;

		RHBImplementationDef_AddRef(somThis->request.impl);

		if (somThis->container)
		{
			SOMD_bomb("still in somebodies container");
		}

		/* this is the old implementation */
		RHBORB_unguard(i)
		
		RHBRequest_send(somThis,ev,somThis->request.send_flags,0,NULL);

		RHBImplementationDef_Release(i);
	}
	else
	{
		debug_somPrintf(("This is not pending!!!!!\n"));
	}
}

static void rhbRequest_reply_received(
	RHBRequest *somThis,
	Environment *ev,
	GIOP_MessageHeader *header,
	GIOP_ReplyHeader_1_0 *reply_ptr,
	CORBA_DataInputStream SOMSTAR input)
{
	RHBOPT_unused(header)

	switch (reply_ptr->reply_status)
	{
		case GIOP_NO_EXCEPTION:
		case GIOP_USER_EXCEPTION:
		case GIOP_SYSTEM_EXCEPTION:
			somThis->reply.reply_status=reply_ptr->reply_status;
			somThis->reply.stream=input;
			input=NULL;

			RHBRequest_SetComplete(somThis,ev);

			break;

		case GIOP_LOCATION_FORWARD:
			{
				SOMDObject SOMSTAR forward=NULL;
/*				NamedValue nv=RHBSOMD_Init_NamedValue;*/

/*				somPrintf("GIOP_LOCATION_FORWARD\n");*/

				RHBSOM_Trace("GIOP_LOCATION_FORWARD\n")

/*				nv.argument._value=&forward;
				nv.argument._type=TC_Object;
				nv.arg_modes=ARG_OUT|CLIENT_SIDE;*/

				forward=CORBA_DataInputStream_read_Object(input,ev);

				if (forward && !ev->_major)
				{
					rhbRequest_forward(somThis,ev,forward);

					SOMDObject_somRelease(forward);
				}
				else
				{
					RHBOPT_throw_StExcep(ev,
							INV_OBJREF,UnexpectedNULL,MAYBE);

					RHBRequest_SetComplete(somThis,ev);
				}
			}
			break;

		default:
			debug_somPrintf(("unknown status %ld\n",reply_ptr->reply_status));
			SOMD_bomb("dont know what this was");
	}

	if (input) RHBORBStream_GIOPInput_somRelease(input);
}

static CORBA_DataInputStream SOMSTAR RHBRequest_encode_exception(
		RHBRequest *somThis,
		Environment *evEx)
{
	CORBA_DataInputStream SOMSTAR result=NULL;
	Environment ev;
	SOM_InitEnvironment(&ev);

	switch (evEx->_major)
	{
	case SYSTEM_EXCEPTION:
		{
			_IDL_SEQUENCE_octet so={0,0,NULL};
			RHBORBStream_GIOPOutput SOMSTAR output=
#ifdef somNewObject
				somNewObject(RHBORBStream_GIOPOutput);
#else
				RHBORBStream_GIOPOutputNew();
#endif
			RHBORBStream_GIOPOutput__set_c_orb(output,&ev,somThis->orb);
			RHBORBStream_GIOPOutput__set_streamed_data(output,&ev,NULL);
			RHBORBStream_GIOPOutput_write_string(output,&ev,somExceptionId(evEx));
			if (evEx->exception._params)
			{
				TypeCode tc=RHBORB_get_exception_tc(somThis->orb,evEx->_major,somExceptionId(evEx));
				RHBCDR_marshal(
							NULL,
							&ev,
							evEx->exception._params,
							tc,
							output);
				TypeCode_free(tc,&ev);
			}
			so=RHBORBStream_GIOPOutput__get_streamed_data(output,&ev);
			somReleaseObjectReference(output);
#ifdef somNewObject
			result=somNewObject(RHBORBStream_GIOPInput);
#else
			result=RHBORBStream_GIOPInputNew();
#endif
			RHBORBStream_GIOPInput__set_c_orb(result,&ev,somThis->orb);
			RHBORBStream_GIOPInput_stream_init_data(result,&ev,&so,0);
		}
		break;
	default:
		RHBOPT_ASSERT(!somThis)
		break;
	}
	SOM_UninitEnvironment(&ev);
	return result;
}

static void rhbRequest_SetComplete(
	  RHBRequest *somThis,
	  Environment *ev)
{
	if (somThis->pending == 1)
	{
		RHBImplementationDef *i;
		void *pv;
		RHBRequest_Complete ptr;

		RHBORB_guard(somThis->orb)

		i=somThis->request.impl;

		RHBImplementationDef_AddRef(i);

		if (!somThis->reply.reply_status)
		{
			if (!somThis->reply.stream)
			{
				if (!somThis->was_local_target)
				{
					if (!(somThis->request.send_flags & INV_NO_RESPONSE))
					{
						switch (ev->_major)
						{
						case SYSTEM_EXCEPTION:
						case USER_EXCEPTION:
							switch (ev->_major)
							{
							case SYSTEM_EXCEPTION:
								somThis->reply.reply_status=GIOP_SYSTEM_EXCEPTION;
								break;
							case USER_EXCEPTION:
								somThis->reply.reply_status=GIOP_USER_EXCEPTION;
								break;
							default:
								break;
							}
							somThis->reply.stream=RHBRequest_encode_exception(somThis,ev);
							break;
						default:
							{
								Environment ev2;
								SOM_InitEnvironment(&ev2);
								RHBOPT_throw_StExcep(&ev2,COMM_FAILURE,UnknownError,MAYBE);
								somThis->reply.reply_status=GIOP_SYSTEM_EXCEPTION;
								somThis->reply.stream=RHBRequest_encode_exception(somThis,&ev2);
								SOM_UninitEnvironment(&ev2);
							}
						}
					}
				}
			}
		}

		RHBOPT_ASSERT(!somThis->container)

		somThis->pending=0;

		ptr=somThis->completionPtr;
		pv=somThis->refCon;
		somThis->completionPtr=NULL;

		if (ptr)
		{
			ptr(somThis,pv);
		}

		RHBORB_unguard(somThis->orb)

		RHBImplementationDef_Release(i);
	}
	else
	{
#if defined(_WIN32) && defined(_DEBUG) && defined(_M_IX86)
		__asm int 3
#endif
		debug_somPrintf(("This is not pending!!!!!\n"));
	}
}

static void rhbRequest_SetCompletionProc(
			RHBRequest *somThis,
			RHBRequest_Complete pfn,
			void *refCon)
{
	somThis->refCon=refCon;
	somThis->completionPtr=pfn;
}

static void rhbRequest_AddRef(RHBRequest *somThis)
{
	somd_atomic_inc(&somThis->lUsage);
}

static void rhbRequest_Release(RHBRequest *somThis)
{
	if (!somd_atomic_dec(&somThis->lUsage))
	{
		rhbRequest_delete(somThis);
	}
}

struct rhbRequest_cancel
{
	RHBORBStream_GIOPOutput SOMSTAR stream;
	RHBGIOPRequestStream *impl;
	_IDL_SEQUENCE_octet cancel_data;
	Environment ev;
};

RHBOPT_cleanup_begin(rhbRequest_cancel_cleanup,pv)

struct rhbRequest_cancel *data=pv;

	RHBORB_unguard(NULL)

	if (data->cancel_data._buffer) SOMFree(data->cancel_data._buffer);

	if (data->stream)
	{
		somReleaseObjectReference(data->stream);
	}

	if (data->impl)
	{
		RHBImplementationDef_Release(data->impl);
	}

	SOM_UninitEnvironment(&data->ev);

RHBOPT_cleanup_end

static void rhbRequest_cancel(
			RHBRequest *somThis)
{
struct rhbRequest_cancel data={NULL,NULL,{0,0,NULL}};

	SOM_InitEnvironment(&data.ev);

	RHBORB_guard(somThis->request.impl)

	RHBOPT_cleanup_push(rhbRequest_cancel_cleanup,&data);

	if (!somThis->cancelled)
	{
		somThis->cancelled=1;

		if (somThis->pending)
		{
			boolean b=0;

			data.impl=somThis->request.impl;

			if (data.impl)
			{
				RHBImplementationDef_AddRef(data.impl);
			}

			/* was from 'executing list' of the impl */

			if (somThis->container)
			{
				b=RHBRequestList_remove(somThis->container,somThis);
			}

			if (b)
			{
				GIOP_CancelRequestHeader rq;
				any a={NULL,NULL};
				data.stream=
	#ifdef somNewObject
				somNewObject(RHBORBStream_GIOPOutput);
	#else
				RHBORBStream_GIOPOutputNew();
	#endif

				RHBORBStream_GIOPOutput__set_c_orb(data.stream,&data.ev,somThis->orb);
				RHBORBStream_GIOPOutput__set_streamed_data(data.stream,&data.ev,&data.cancel_data);

				rq.request_id=somThis->request.lSequence;

				a._value=&rq;
				a._type=somdTC_GIOP_CancelRequestHeader;

				RHBCDR_marshal(NULL,
						&data.ev,
						a._value,a._type,
						data.stream);

				data.cancel_data=RHBORBStream_GIOPOutput__get_streamed_data(data.stream,&data.ev);
			}

			RHBOPT_throw_StExcep(&data.ev,NO_RESPONSE,RequestNotFound,MAYBE);

			RHBRequest_SetComplete(somThis,&data.ev);

			if (b && data.impl && data.impl->connection.connected)
			{
				somdExceptionFree(&data.ev);

				RHBImplementationDef_write_GIOP_msg(
						data.impl,
						&data.ev,
						&somThis->protocol_version,
						GIOP_CancelRequest,
						&data.cancel_data);
			}
		}
	}

	RHBOPT_cleanup_pop();
}


static struct RHBRequestVtbl rhbRequestVtbl=
{
	rhbRequest_AddRef,
	rhbRequest_Release,
	rhbRequest_cancel,
	rhbRequest_add_arg,
	rhbRequest_invoke,
	rhbRequest_send,
	rhbRequest_get_response,
	rhbRequest_SetComplete,
	rhbRequest_SetCompletionProc,
	rhbRequest_reply_received,
	rhbRequest_send_marshalled
};

SOMEXTERN RHBRequest * SOMLINK RHBRequestNew(
				Environment *ev,
				RHBObject *object,
				SOMDObject SOMSTAR somobj,
				_IDL_SEQUENCE_string * ctx,
				Identifier operation,
				NVList SOMSTAR args,
				NamedValue *result,
				Flags flags
				)
{
	RHBRequest *somThis;
	RHBServerRequest *server_request=NULL;
	RHBORB_ThreadTask *thread=RHBSOMD_get_thread();

	if (result)
	{
#ifdef _DEBUG_X
		if (result->arg_modes) 
		{
			debug_somPrintf(("result->arg_modes for operation %s are set\n",operation));

			SOMD_bomb("we'll bomb here for the moment");
		}
		if (result->name)
		{
			debug_somPrintf(("result->name for operation %s is set\n",operation));

			SOMD_bomb("result should be nameless");
		}
#endif

		result->arg_modes|=(CLIENT_SIDE | ARG_OUT | RESULT_OUT);
	}

	somThis=(RHBRequest *)SOMMalloc(sizeof(*somThis));

	if (!somThis)
	{
		RHBOPT_throw_StExcep(ev,NO_MEMORY,NoMemory,MAYBE);

		return 0;
	}

	somThis->lpVtbl=&rhbRequestVtbl;
	somThis->protocol_version.major=1;
	somThis->protocol_version.minor=0;
	somThis->was_local_target=0;
	somThis->has_got_response=0;
	somThis->in_params._length=0;
	somThis->in_params._maximum=0;
	somThis->in_params._buffer=NULL;
	somThis->pending=0;
	somThis->cancelled=0;
	somThis->completionPtr=0;

	SOM_InitEnvironment(&somThis->local.ev);

	somThis->pool=NULL;		 /* not in any request pool, yet. */
	somThis->container=NULL;
	somThis->deleting=0;

	somThis->lUsage=1;
	somThis->marshalled_data._length=0;
	somThis->marshalled_data._maximum=0;
	somThis->marshalled_data._buffer=NULL;
	somThis->request.send_flags=0;

	somThis->reply.reply_status=0;

	if (args)
	{
		somThis->request.arg_list=NVList_somDuplicateReference(args);
	}
	else
	{
		somThis->request.arg_list=NULL;
	}

	somThis->request.somobj=NULL;

	if (somobj)
	{
		somThis->request.somobj=SOMDObject_somDuplicateReference(somobj);
	}

	somThis->request.object_key._length=object->object_key._length;
	somThis->request.object_key._maximum=object->object_key._maximum;
	somThis->request.object_key._buffer=SOMMalloc(object->object_key._length);
	memcpy(somThis->request.object_key._buffer,
			object->object_key._buffer,
			object->object_key._length);

	if (ctx)
	{
		somThis->request.send_context=1;
		somThis->request.context_values=*ctx;
		ctx->_length=0;
		ctx->_maximum=0;
		ctx->_buffer=NULL;
	}
	else
	{
		somThis->request.send_context=0;
		somThis->request.context_values._length=0;
		somThis->request.context_values._maximum=0;
		somThis->request.context_values._buffer=NULL;
	}

	somThis->request.operation=somd_dupl_string(operation);
	somThis->request.result=result;
	somThis->request.flags=flags;
	somThis->request.impl=object->impl;

	RHBImplementationDef_AddRef(somThis->request.impl);

	somThis->orb=object->impl->orb;

	RHBDebug_created(RHBRequest,somThis)

	RHBORB_AddRef(somThis->orb);

	if (thread)
	{
		server_request=thread->server_request;
	}

	if (server_request)
	{
		somThis->request.principal=RHBServerRequest_get_principal(server_request,ev);
	}
	else
	{
		somThis->request.principal=RHBORB_get_principal(somThis->request.impl->orb,ev);
	}

	somThis->reply.stream=NULL;

	return somThis;
}
