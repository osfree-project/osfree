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

/* contains ServerRequest */

struct RHBServerRequest_write_object
{
	SOMDObject SOMSTAR somdRef;
};

RHBOPT_cleanup_begin(RHBServerRequest_write_object_cleanup,pv)

struct RHBServerRequest_write_object *data=pv;

	if (data->somdRef) somReleaseObjectReference(data->somdRef);

RHBOPT_cleanup_end

struct SOMCDR_marshal_filter_data
{
	RHBServerRequest *request;
	unsigned long arg_modes;
};

static void SOMLINK RHBServerRequest_write_object(
			struct SOMCDR_marshal_filter *filter,
			Environment *ev,
			any *value,
			CORBA_DataOutputStream SOMSTAR stream)
{
	if (!ev->_major)
	{
		if (value->_value)
		{
			SOMObject SOMSTAR *objptr=value->_value;
			SOMObject SOMSTAR valObj=*objptr;


			switch (TypeCode_kind(value->_type,ev))
			{
			case tk_Principal:
				/* servers should not be replying with
					principals... */
				RHBOPT_ASSERT(!value)
				RHBOPT_throw_StExcep(ev,MARSHAL,BadResultType,MAYBE);
				break;
			case tk_objref:
				if (valObj)
				{
					RHBServerRequest *somThis=filter->data->request;

					if (SOMObject_somIsA(valObj,somThis->orb->somClass_SOMDObject_ref))
					{
						CORBA_DataOutputStream_write_Object(stream,ev,valObj);
					}
					else
					{
						struct RHBServerRequest_write_object data={NULL};

						RHBOPT_cleanup_push(RHBServerRequest_write_object_cleanup,&data);

						if (!ev->_major)
						{
							data.somdRef=SOMDServer_somdRefFromSOMObj(
								somThis->somdServer,ev,valObj);
						}

						if (!ev->_major)
						{
							if (SOMDObject_is_SOM_ref(data.somdRef,ev))
							{
								SOMObject SOMSTAR ref=NULL;

	#ifdef SOMObject_somDuplicateReference
								ref=SOMObject_somDuplicateReference(valObj);
	#else
								if (SOMObject_somIsA(valObj,SOMRefObjectClassData.classObject))
								{
									ref=SOMRefObject_somDuplicateReference(valObj);
								}
								else
								{
									*objptr=NULL; /* we have rescued it from
													being cleared until
													this client connection
												dies */
									ref=valObj;
									valObj=NULL;
								}
	#endif
								if (ref)
								{
									RHBImplementationDef_add_client_ref(somThis->impl,ref);
								}
							}

							CORBA_DataOutputStream_write_Object(stream,ev,data.somdRef);
						}

						RHBOPT_cleanup_pop();
					}
				}
				else
				{
					CORBA_DataOutputStream_write_Object(stream,ev,valObj);
				}
				break;
			default:
				RHBOPT_throw_StExcep(ev,MARSHAL,BadTypeCode,MAYBE);
				break;
			}
		}
		else
		{
			RHBOPT_throw_StExcep(ev,MARSHAL,UnexpectedNULL,MAYBE);
		}
	}
}

static ORBStatus RHBServerRequest_write_exception(
		RHBServerRequest *somThis,
		Environment *evLocal,
		CORBA_DataOutputStream SOMSTAR out_stream,
		Environment *evResult,
		any *exceptionValue)
{
	char *p=NULL;
	RHBContained *ex=NULL;
	char *ex_id=NULL;
	struct SOMCDR_marshal_filter filter;
	struct SOMCDR_marshal_filter_data filter_data={NULL,
				SERVER_SIDE|RESULT_OUT};
	any return_ex={NULL,NULL};

	if (evResult->exception._params)
	{
		return_ex._value=evResult->exception._params;
	}
	else
	{
		return_ex._value=exceptionValue->_value;
	}

	RHBOPT_ASSERT(exceptionValue)
	RHBOPT_ASSERT(evResult->_major)

	filter_data.request=somThis;
	filter.data=&filter_data;
	filter.write_object=RHBServerRequest_write_object;

	if (evLocal->_major) 
	{
		return SOMDERROR_Marshal;
	}

	ex_id=somExceptionId(evResult);

	if ((!ex_id)||(!exceptionValue->_type))
	{
		RHBOPT_throw_StExcep(evLocal,MARSHAL,UnexpectedNULL,MAYBE);
		return SOMDERROR_Marshal;
	}
	else
	{
		if (evResult->_major==SYSTEM_EXCEPTION)
		{
			if (!return_ex._value)
			{
				RHBOPT_throw_StExcep(evLocal,MARSHAL,UnexpectedNULL,MAYBE);

				return SOMDERROR_Marshal;
			}

			return_ex._type=somdTC_StExcep;
		}
		else
		{
			return_ex._type=exceptionValue->_type;

			if (!return_ex._type)
			{
				RHBOPT_throw_StExcep(evLocal,MARSHAL,BadTypeCode,MAYBE);

				return SOMDERROR_Marshal;
			}
		}
	}

	ex=RHBORB_get_contained(somThis->orb,
			evLocal,
			ex_id);

	if (evLocal->_major || !ex) 
	{
		debug_somPrintf(("problem returning exception %s\n",somExceptionId(evResult)));

		return SOMDERROR_Marshal;
	}

	p=RHBContained_get_id(ex,evLocal);

	if (!p)
	{
		RHBContained_Release(ex);

		RHBOPT_throw_StExcep(evLocal,MARSHAL,UnexpectedNULL,MAYBE);

		return SOMDERROR_Marshal;
	}

	CORBA_DataOutputStream_write_string(out_stream,evLocal,p);

	if (return_ex._value)
	{
		Environment evLocal2;

		SOM_InitEnvironment(&evLocal2);

		RHBCDR_marshal(&filter,
			evLocal,
			return_ex._value,
			return_ex._type,
			out_stream);

		evResult->exception._params=NULL;
		exceptionValue->_value=NULL;

		SOMD_FreeType(&evLocal2,
			return_ex._value,
			return_ex._type);

		SOMFree(return_ex._value);

		SOM_UninitEnvironment(&evLocal2);
	}

	somExceptionFree(evResult);

	RHBContained_Release(ex);

	if (evLocal->_major) return SOMDERROR_Marshal;

	return 0;
}



static ORBStatus RHBServerRequest_return_relocate(
			RHBServerRequest *somThis,
			Environment *ev,
			SOMDObject SOMSTAR obj,
			boolean release_flag)
{
	ORBStatus status=0;
	CORBA_DataInputStream SOMSTAR in_stream=NULL;

	RHBORB_guard(somThis->impl)

	somPrintf("RHBServerRequest_return_relocate(%p,%d)\n",obj,release_flag);
	if (obj)
	{
		if (SOMObject_somIsA(obj,_SOMDObject))
		{
			SOMObject_somDumpSelf(obj,0);
		}
		else
		{
			SOMObject_somPrintSelf(obj);
		}
	}

	if (somThis->impl)
	{
		_IDL_SEQUENCE_octet reply_head={0,0,NULL};
		struct SOMCDR_marshal_filter filter;
		struct SOMCDR_marshal_filter_data filter_data={NULL,
				RESULT_OUT|SERVER_SIDE};
/*		boolean exception_returned=0;*/
		GIOP_ReplyHeader_1_0 reply;
#ifdef somNewObject
		RHBORBStream_GIOPOutput SOMSTAR reply_stream=somNewObject(RHBORBStream_GIOPOutput);
#else
		RHBORBStream_GIOPOutput SOMSTAR reply_stream=RHBORBStream_GIOPOutputNew();
#endif

		RHBORBStream_GIOPOutput__set_c_orb(reply_stream,ev,somThis->orb);
		RHBORBStream_GIOPOutput__set_streamed_data(reply_stream,ev,&reply_head);

		somThis->finished=2;

		/* act of writing will drop reference count */

/*		if (!release_flag)
		{
			obj=SOMDObject_somDuplicateReference(obj);
		}
*/
		filter_data.request=somThis;
		filter.data=&filter_data;
		filter.write_object=RHBServerRequest_write_object;

		memset(&reply,0,sizeof(reply));

		/* if exception
			write reply, corbastring repid of exeception,
			else
			write return, and out/inout parameters */

		reply.service_context._length=0;
		reply.request_id=somThis->header.request_id;

		/* this looks like the wrong way round, but for exceptions 
			start from zero, but reply_status is an 'enum' */

		reply.reply_status=GIOP_LOCATION_FORWARD;

/*		a._type=RHBORB_get_GIOP_ReplyHeader_tc(somThis->orb,ev);
		a._value=&reply;
*/
		RHBCDR_marshal(&filter,
					ev,
					&reply,
					somdTC_GIOP_ReplyHeader_1_0,
					reply_stream);

/*		status=RHBGIOPORB_stream_write(&wrapper,ev,&reply_head,&a);*/

/*		a._type=TC_Object;
		a._value=&obj;

		status=RHBGIOPORB_stream_write(&wrapper,ev,&reply_head,&a);
*/
		RHBORBStream_GIOPOutput_write_Object(reply_stream,ev,obj);

		if (somThis->impl->connection.connected)
		{
			if (!somThis->cancelled)
			{
				reply_head=RHBORBStream_GIOPOutput__get_streamed_data(reply_stream,ev);

				RHBImplementationDef_write_GIOP_msg(
					somThis->impl,
					ev,
					&somThis->protocol_version,
					GIOP_Reply,&reply_head);
			}
		}

		if (release_flag)
		{
			if (obj) 
			{
				SOMObject_somFree(obj);
			}
			else
			{
				SOMD_bomb("return_relocate error, obj gone, I was supposed to release it\n");
			}
		}
		else
		{
			if (!obj) SOMD_bomb("return_relocate error, obj gone\n");
		}

		if (reply_head._buffer) 
		{
			SOMFree(reply_head._buffer);
			reply_head._buffer=NULL;
		}

		reply_head._maximum=0;
		reply_head._length=0;

		somReleaseObjectReference(reply_stream);
	}

	in_stream=somThis->in_stream;
	somThis->in_stream=NULL;

	RHBImplementationDef_remove_outstanding(somThis->impl,somThis);

	RHBORB_unguard(somThis->impl)

	if (in_stream) CORBA_DataInputStream_somFree(in_stream);

	return status;
}

struct rhbServerRequest_set_result
{
	RHBImplementationDef *impl;
	RHBORBStream_GIOPOutput SOMSTAR reply_stream;
	Environment evLocal;
	GIOP_ReplyHeader_1_0 reply;
};

RHBOPT_cleanup_begin(rhbServerRequest_set_result_cleanup,pv)

struct rhbServerRequest_set_result *data=pv;

	if (data->impl)
	{
		RHBImplementationDef_Release(data->impl);
	}

	if (data->reply_stream)
	{
		somReleaseObjectReference(data->reply_stream);
	}

	if (data->evLocal._major)
	{
		somdExceptionFree(&data->evLocal);
	}

RHBOPT_cleanup_end

static ORBStatus rhbServerRequest_set_result(
		RHBServerRequest *somThis,
		Environment *evResult,
		any *ret_val)
{
	struct rhbServerRequest_set_result data={NULL,NULL};
	ORBStatus RHBOPT_volatile status=0;
	struct SOMCDR_marshal_filter filter;
	struct SOMCDR_marshal_filter_data filter_data={NULL,RESULT_OUT|SERVER_SIDE};

	SOM_InitEnvironment(&data.evLocal);

	RHBOPT_ASSERT(evResult)
	RHBOPT_ASSERT(ret_val)

	if (somThis->finished)
	{
		somdExceptionFree(evResult);

		return status;
	}

	if (evResult->_major==USER_EXCEPTION)
	{
		char *ex=somExceptionId(evResult);

		if (ex)
		{
			if (!strcmp(ex,ex_PortableServer_ForwardRequest))
			{
				PortableServer_ForwardRequest *fwd=evResult->exception._params;

				evResult->exception._params=NULL;

				if (!fwd)
				{
					fwd=ret_val->_value;
					ret_val->_value=NULL;
				}

				if (fwd)
				{
					SOMObject SOMSTAR obj=fwd->forward_reference;
					fwd->forward_reference=NULL;
					SOMFree(fwd);

					if (obj)
					{
						somExceptionFree(evResult);

						return RHBServerRequest_return_relocate(somThis,evResult,obj,1);
					}
				}
			}
		}
	}

	if (!ret_val)
	{
		if (!evResult->_major)
		{
			RHBOPT_throw_StExcep(evResult,UNKNOWN,BadResultType,MAYBE)
		}
	}

	RHBORB_guard(somThis->impl)

	filter_data.request=somThis;
	filter.data=&filter_data;
	filter.write_object=RHBServerRequest_write_object;

	if (somThis->finished)
	{
		RHBORB_unguard(somThis->impl);

		somdExceptionFree(evResult);

		return 0;
	}

	RHBOPT_cleanup_push(rhbServerRequest_set_result_cleanup,&data);

	somThis->finished=1;

	/* write out even if not connected, as writing
		out process can release some objects,
		however,dont transmit if broken */

	RHBOPT_ASSERT(somThis->impl)

	if (somThis->impl)
	{
		/* need to write a GIOP_MessageHeader followed by a GIOP_RequestReply */

		RHBORBStream_MarshalContext marshal_context;
		boolean exception_returned=0;

#ifdef somNewObject
		data.reply_stream=somNewObject(RHBORBStream_GIOPOutput);
#else
		data.reply_stream=RHBORBStream_GIOPOutputNew();
#endif

/*		marshal_context.c_orb=somThis->orb;*/
		marshal_context.c_impl=somThis->impl;
		marshal_context.nv=NULL;

		RHBORBStream_GIOPOutput__set_c_orb(data.reply_stream,&data.evLocal,somThis->orb);
		RHBORBStream_GIOPOutput__set_streamed_data(data.reply_stream,&data.evLocal,NULL);
		RHBORBStream_GIOPOutput__set_write_ctx(data.reply_stream,&data.evLocal,&marshal_context);
		RHBORBStream_GIOPOutput__set_marshal_filter(data.reply_stream,&data.evLocal,&filter);

		memset(&data.reply,0,sizeof(data.reply));

		/* if exception
			write reply, corbastring repid of exeception,
			else
			write return, and out/inout parameters */

		data.reply.service_context._length=0;
		data.reply.request_id=somThis->header.request_id;

		/* this looks like the wrong way round, but for exceptions 
			start from zero, but reply_status is an 'enum' */

		data.reply.reply_status=RHBCDR_GIOPenumToIDLenum(evResult->_major,&data.evLocal,0);

		RHBCDR_marshal(&filter,&data.evLocal,
					&data.reply,
					somdTC_GIOP_ReplyHeader_1_0,
					data.reply_stream);

		/* now write the parameters */

		RHBORB_unguard(somThis->impl)

		if (evResult->_major)
		{
			exception_returned=1;

			status=RHBServerRequest_write_exception(somThis,&data.evLocal,data.reply_stream,evResult,ret_val);
		}
		else
		{
			if (ret_val && (!data.evLocal._major))
			{
				NamedValue result=RHBSOMD_Init_NamedValue;

				result.arg_modes=SERVER_SIDE|ARG_OUT;
				result.argument=*ret_val;

				marshal_context.nv=&result;

				RHBCDR_marshal(&filter,&data.evLocal,
					ret_val->_value,
					ret_val->_type,
					data.reply_stream);
			}

			/* write return value, then all out and inout parameters */

			if (somThis->param_seq && !data.evLocal._major)
			{
				NVList SOMSTAR params=somThis->param_seq[0];
				if (params)
				{
					long count=0;
					if (!NVList_get_count(params,&data.evLocal,&count))
					{
						long l=0;

						while ((l < count) && (!data.evLocal._major))
						{
							NamedValue nv=RHBSOMD_Init_NamedValue;

							NVList_get_item(params,&data.evLocal,
									l++,
									&nv.name,
									&nv.argument._type,
									&nv.argument._value,
									&nv.len,
									&nv.arg_modes);

							marshal_context.nv=&nv;
							filter_data.arg_modes=nv.arg_modes;

#ifdef _DEBUG
							if (!(nv.arg_modes & SERVER_SIDE))
							{
								SOMD_bomb("bomb here");
							}
#endif

							if ((nv.arg_modes & (ARG_OUT | ARG_INOUT)) 
								&& !data.evLocal._major)
							{
								RHBCDR_marshal(&filter,&data.evLocal,
										nv.argument._value,
										nv.argument._type,
										data.reply_stream);
							}
						}
					}
				}
			}

			marshal_context.nv=NULL;
			filter_data.arg_modes=0;
		}

		RHBORB_guard(somThis->impl)
			/* end of write parameters */

		if ((somThis->header.response_expected&1) | exception_returned)
		{
			if (somThis->impl->connection.connected)
			{
				if (!somThis->cancelled)
				{
					int reply_attempts=0;
					_IDL_SEQUENCE_octet stream_octets=
						RHBORBStream_GIOPOutput__get_streamed_data(
							data.reply_stream,&data.evLocal);		

					if (!data.evLocal._major)
					{
						RHBImplementationDef_write_GIOP_msg(
								somThis->impl,
								&data.evLocal,
								&somThis->protocol_version,
								GIOP_Reply,
								&stream_octets);
					}

					if (stream_octets._buffer)
					{
						SOMFree(stream_octets._buffer);
						stream_octets._length=0;
						stream_octets._maximum=0;
						stream_octets._buffer=NULL;
					}
					
					while (data.evLocal._major && 
							(reply_attempts < 4) &&
							somThis->impl->connection.connected)
					{
						any ex_value={NULL,NULL};
						Environment evLocal2;

						SOM_InitEnvironment(&evLocal2);

						RHBORBStream_GIOPOutput__set_streamed_data(data.reply_stream,&evLocal2,NULL);

						reply_attempts++;

						debug_somPrintf(("marshalling reply failed, %s\n",
								somExceptionId(&evLocal)));

						/* need to return local error */

						/* change reply to say are returning exception */
						data.reply.reply_status=RHBCDR_GIOPenumToIDLenum(data.evLocal._major,&evLocal2,0);

						RHBCDR_marshal(&filter,&evLocal2,
									&data.reply,
									somdTC_GIOP_ReplyHeader_1_0,
									data.reply_stream);

						if (data.evLocal._major==SYSTEM_EXCEPTION)
						{
							/* don't know it correctly completed, so always
								assume maybe */

							StExcep_UNKNOWN *ex=data.evLocal.exception._params;;

							if (ex)
							{
								ex->completed=MAYBE;
							}

							ex_value._type=somdTC_StExcep;
						}

						RHBORB_unguard(somThis->impl)

						status=RHBServerRequest_write_exception(somThis,&evLocal2,data.reply_stream,&data.evLocal,&ex_value);

						RHBORB_guard(somThis->impl)

						stream_octets=RHBORBStream_GIOPOutput__get_streamed_data(data.reply_stream,&evLocal2);

						if (!status)
						{
							if (!somThis->cancelled)
							{
								RHBImplementationDef_write_GIOP_msg(
									somThis->impl,
									&evLocal2,
									&somThis->protocol_version,
									GIOP_Reply,
									&stream_octets);
							}
							else
							{
								RHBSOM_Trace("not return reply to cancelled message\n");
							}
						}

						if (stream_octets._buffer)
						{
							SOMFree(stream_octets._buffer);
							stream_octets._buffer=NULL;
						}

						stream_octets._maximum=0;
						stream_octets._length=0;

						somdExceptionFree(&data.evLocal);

						if (evLocal2._major || status)
						{
							debug_somPrintf(("serious marshalling reply failed(%d), %s\n",
								reply_attempts,
								somExceptionId(&evLocal2)));
/*							bomb("help needed here..final reply marshalling\n");*/

							somExceptionFree(&evLocal2);

							RHBOPT_throw_StExcep(&data.evLocal,
									MARSHAL,
									Marshal,
									MAYBE);
						}
					}

					if (stream_octets._buffer) 
					{
						SOMFree(stream_octets._buffer);
						stream_octets._length=0;
						stream_octets._maximum=0;
						stream_octets._buffer=NULL;
					}
				}
			}
		}
	}

	RHBImplementationDef_remove_outstanding(somThis->impl,somThis);

	data.impl=somThis->impl;
	somThis->impl=NULL;

	RHBORB_unguard(impl)

	RHBOPT_cleanup_pop();

	return status;
}

static void RHBServerRequest_delete(RHBServerRequest *somThis)
{
	RHBORB *orb=somThis->orb;
	SOMDServer SOMSTAR somdServer=somThis->somdServer;
	somThis->orb=NULL;

	if (somThis->lUsage)
	{
		SOMD_bomb("Should use 'Release'");
	}

	if (somThis->impl)
	{
		SOMD_bomb("should use RHBServerRequest::Close()");
	}

	RHBDebug_deleted(RHBServerRequest,somThis)

	SOMFree(somThis);

	if (somdServer) somReleaseObjectReference(somdServer);
	if (orb) RHBORB_Release(orb);
}

static void rhbServerRequest_AddRef(RHBServerRequest *somThis)
{
	if (somThis->lUsage==0) SOMD_bomb("should not have a zero usage\n");

	somd_atomic_inc(&somThis->lUsage);
}

static void rhbServerRequest_Release(RHBServerRequest *somThis)
{
	Environment ev;
	Principal SOMSTAR principal=NULL;
	CORBA_DataInputStream SOMSTAR in_stream=NULL;
	RHBGIOPRequestStream *impl=NULL;

	SOM_InitEnvironment(&ev);

	RHBORB_guard(somThis->impl)

	if (somd_atomic_dec(&somThis->lUsage)) 
	{
		RHBORB_unguard(somThis->impl)

		return;
	}

	if (somThis->closing) 
	{
		SOMD_bomb("how did this happen?");

		RHBORB_unguard(somThis->impl)

		return;
	}

	RHBOPT_ASSERT(somThis->finished)

	somThis->closing=1;

	if (somThis->lUsage)
	{
		RHBORB_unguard(somThis->impl)

		SOMD_bomb("how did this happen?");
	}
	else
	{
		RHBOPT_ASSERT(!somThis->executing_thread);

		SOMD_FreeType(
				&ev,
				&somThis->header,
				somdTC_GIOP_RequestHeader_1_0);

		principal=somThis->principal;
		somThis->principal=NULL;

		in_stream=somThis->in_stream;
		somThis->in_stream=NULL;

		if (somThis->somd_contexts._buffer)
		{
			SOMFree(somThis->somd_contexts._buffer);
			somThis->somd_contexts._buffer=NULL;
		}

		if (somThis->impl)
		{
			RHBImplementationDef_remove_outstanding(somThis->impl,somThis);
			impl=somThis->impl;
			somThis->impl=NULL;
		}

		RHBORB_unguard(somThis->impl)

		if (!somThis->lUsage)
		{
			RHBServerRequest_delete(somThis);
		}
		else
		{
			somThis->closing=0;
		}
	}

	if (in_stream) CORBA_DataInputStream_somFree(in_stream);
	if (principal) Principal_somFree(principal);
	if (impl) RHBImplementationDef_Release(impl);

	SOM_UninitEnvironment(&ev);
}

static void rhbServerRequest_push_current(
	RHBServerRequest *somThis,
	RHBServerRequest **prev)
{
	RHBORB_ThreadTask *thread=RHBSOMD_get_thread();

	*prev=thread->server_request;

	RHBOPT_ASSERT(&somThis->executing_thread);
	somThis->executing_thread=thread;

	thread->server_request=somThis;

	somThis->metrics.execute_chain=prev;

	if (*prev)
	{
#ifdef USE_THREADS
		SOMD_bomb("should only have one server request per thread\n");
#else
		if ((*prev)->impl==somThis->impl)
		{
			SOMD_bomb("should only have one server request active per user connection\n");
		}
#endif
	}

#ifdef USE_SELECT
	if (somThis->impl->active_server_request)
	{
		SOMD_bomb("this implementation already processing\n");
	}
	somThis->impl->active_server_request=somThis;
#endif
}

static void rhbServerRequest_pop_current(
	RHBServerRequest *somThis,
	RHBServerRequest **prev)
{
	RHBORB_ThreadTask *thread=RHBSOMD_get_thread();

	RHBOPT_ASSERT(thread->server_request==somThis);
	RHBOPT_ASSERT(somThis->executing_thread==thread);

	somThis->executing_thread=NULL;

#ifdef USE_SELECT
	if (somThis->impl)
	{
		if (somThis->impl->active_server_request!=somThis)
		{
			SOMD_bomb("not this current active request");
		}
		somThis->impl->active_server_request=NULL;
	}
	else
	{
		boolean dump_stat=0;

		if (somThis->orb)
		{
			RHBImplementationDef *impl;

			impl=somThis->orb->impls;

			while (impl)
			{
				if (impl->active_server_request==somThis)
				{
					break;
				}

				impl=impl->next;
			}

			if (impl)
			{
				impl->active_server_request=NULL;
			}
			else
			{
				debug_somPrintf(("This server request has been mislaid\n"));
				dump_stat=1;
			}
		}
		else
		{
			debug_somPrintf(("This server request has got a bit lost\n"));
			dump_stat=1;
		}

		if (dump_stat)
		{
			if (somThis->closing) debug_somPrintf(("--closing\n"));
			if (somThis->cancelled) debug_somPrintf(("--cancelled\n"));
			if (somThis->finished) debug_somPrintf(("--finished\n"));
		}
	}
#endif

	thread->server_request=*prev;
	*prev=NULL;

	somThis->metrics.execute_chain=NULL;
}

struct RHBServerRequest_Execute_data
{
	SOMObject SOMSTAR target;
	SOMDObject SOMSTAR dobj;
	ServerRequest SOMSTAR servreq;
	SOMDServerStub SOMSTAR dynimpl;
	SOMDServer SOMSTAR somdServer;
	boolean target_somRelease;
	RHBServerRequest *previous;
	RHBServerRequest *request;
	boolean guarded;
};

RHBOPT_cleanup_begin(RHBServerRequest_Execute_cleanup,pv)

	struct RHBServerRequest_Execute_data *local=pv;

	if (local->guarded)
	{
		RHBORB_unguard(local->orb);

		local->guarded=0;
	}


	if (local->target_somRelease) 
	{
/*		RHBServerRequest_free_object(0,&ev,TC_Object,local->target);*/
		if (local->target)
		{
			somReleaseObjectReference(local->target);
			local->target=NULL;
		}
	}

	if ((!local->request->finished)&&(!local->servreq))
	{
		Environment ev;
		any a={NULL,NULL};
		SOM_InitEnvironment(&ev);

		RHBOPT_throw_StExcep(&ev,INTERNAL,DispatchError,MAYBE);

		a._type=somdTC_StExcep;
		a._value=ev.exception._params;
		ev.exception._params=NULL;

		RHBServerRequest_set_result(local->request,&ev,&a);

		SOM_UninitEnvironment(&ev);
	}

	RHBServerRequest_pop_current(local->request,&local->previous);

	RHBServerRequest_Release(local->request);

	if (local->servreq)
	{
		somReleaseObjectReference(local->servreq);
	}

	if (local->dynimpl)
	{
		somReleaseObjectReference(local->dynimpl);
	}

	if (local->somdServer)
	{
		somReleaseObjectReference(local->somdServer);
	}

RHBOPT_cleanup_end

static void SOMLINK rhbServerRequest_Execute(
	RHBServerRequest *somThis,
	Environment *ev)
{
	struct RHBServerRequest_Execute_data local;

	local.target=NULL;
	local.dobj=NULL;
	local.target_somRelease=0;
	local.previous=NULL;
	local.request=somThis;
	local.guarded=0;
	local.servreq=NULL;
	local.dynimpl=NULL;
	local.somdServer=NULL;

	RHBServerRequest_push_current(somThis,&local.previous);
	RHBOPT_cleanup_push(RHBServerRequest_Execute_cleanup,&local);

	RHBOPT_ASSERT(somThis->impl)

	if (!somThis->impl)
	{
		somThis->cancelled=1;
	}
	else
	{
		if (!somThis->impl->connection.connected)
		{
			somThis->cancelled=1;
		}
		else
		{
#ifdef USE_THREADS
			RHBORB_guard(orb)

			local.guarded=1;

			while (somThis->impl->server.data)
			{
				if (somThis->impl->server.data->is_ready)
				{
					break;
				}

				if (somThis->impl->server.data->deactivating)
				{
					somThis->cancelled=1;

					break;
				}

				if (!somThis->impl->connection.connected)
				{
					somThis->cancelled=1;
					break;
				}

				RHBORB_wait_server_state_changed(somThis->impl->orb);
			}

			local.guarded=0;

			RHBORB_unguard(orb)
#endif
		}
	}

	if (somThis->cancelled)
	{
		any a={NULL,NULL};
		a._type=TC_void;
		RHBServerRequest_set_result(somThis,ev,&a);
	}
	else
	{
		RHBServerRequest_state(somThis,"RHBImplementationDef_get_target_object")

		local.target=RHBImplementationDef_get_target_object(somThis->impl,ev,&somThis->header.object_key,&somThis->somdServer);

		if (!local.target)
		{
			any a={NULL,NULL};
			
			a._type=TC_void;

			RHBOPT_throw_StExcep(ev,OBJ_ADAPTER,ObjectNotFound,NO);

			RHBServerRequest_set_result(somThis,ev,&a);
		}
		else
		{
			RHBServerRequest_state(somThis,"RHBORB_isObject")

			local.dobj=RHBORB_isSOMDObject(somThis->orb,local.target);

			if (local.dobj)
			{
				boolean andFree=0;

	/*		somPrintf("looks like an object in another process... bounce it\n");*/

			/* the server object can actually be nabbed under foot */

				RHBServerRequest_state(somThis,"RHBImplementationDef_acquire_somdServer")
	
				local.somdServer=RHBImplementationDef_acquire_somdServer(somThis->impl,ev);

				if (local.somdServer)
				{
					andFree=(boolean)!SOMDServer_somdObjReferencesCached(local.somdServer,ev);
				}
				else
				{
					SOMD_bomb("where has the server gone");
					andFree=0;
				}

				if (!(somThis->header.response_expected & 1))
				{
					somPrintf("%s, cannot return relocate for a oneway message\n",
						somThis->header.operation);
					SOMObject_somDumpSelf(local.dobj,0);
				}

				RHBServerRequest_return_relocate(somThis,ev,local.dobj,andFree);
			}
			else
			{
	#ifdef SOMObject_somRelease
				local.target_somRelease=1;
	#else
				local.target_somRelease=SOMObject_somIsA(local.target,SOMRefObjectClassData.classObject);
	#endif
				if (!strcmp(somThis->header.operation,"somFree"))
				{
					any a={NULL,NULL};
					a._type=TC_void;

					RHBImplementationDef_remove_client_ref(somThis->impl,local.target,0);

					/* I think this should be bounced to the 
					SOMDServer::somdDeleteObj */

					somReleaseObjectReference(local.target);

					RHBServerRequest_set_result(somThis,ev,&a);
				}
				else
				{
					if (!strcmp(somThis->header.operation,"somRelease"))
					{
						any a={NULL,NULL};

						a._type=TC_void;

						if (RHBImplementationDef_remove_client_ref(somThis->impl,local.target,0))
						{
							if (local.target_somRelease) 
							{
								somReleaseObjectReference(local.target);
							}
						}

						RHBServerRequest_set_result(somThis,ev,&a);
					}
					else
					{
						int invoked=0;

						if (SOMObject_somIsA(local.target,
								somThis->orb->somClass_PortableServer_DynamicImpl_ref))
						{
							local.dynimpl=PortableServer_DynamicImpl_somDuplicateReference(local.target);
						}

						if (!local.dynimpl)
						{
							if (!local.somdServer)
							{
								local.somdServer=RHBImplementationDef_acquire_somdServer(somThis->impl,ev);
							}

							if (!ev->_major)
							{
								local.dynimpl=SOMClass_somNew(somThis->orb->somClass_SOMDServerStub_ref);

								SOMDServerStub__set_somdServant(local.dynimpl,ev,local.target);
								SOMDServerStub__set_somdServer(local.dynimpl,ev,local.somdServer);
							}
						}

						if (local.dynimpl && !ev->_major)
						{
							local.servreq=SOMClass_somNew(somThis->orb->somClass_ServerRequest_ref);

							if (local.servreq && !ev->_major)
							{
								ServerRequest__set_c_request(local.servreq,ev,somThis);

								if (!ev->_major)
								{
									PortableServer_DynamicImpl_invoke(
										local.dynimpl,
										ev,
										local.servreq);

									invoked=1;
								}
							}
						}

						if (!invoked)
						{
							any a={NULL,NULL};

							a._type=TC_void;

							if (!ev->_major)
							{
								RHBOPT_throw_StExcep(ev,INTERNAL,DispatchError,NO);
							}

							RHBServerRequest_set_result(somThis,ev,&a);
						}
					}
				}
			}
		}
	}

	RHBServerRequest_state(somThis,"completed")

	RHBOPT_cleanup_pop();

	somdExceptionFree(ev);
}

static Context SOMSTAR rhbServerRequest_ctx(
			RHBServerRequest *somThis,
			Environment *ev)
{
	Context SOMSTAR ctx=SOMClass_somNew(somThis->orb->somClass_Context_ref);
	unsigned long l=CORBA_DataInputStream_read_ulong(somThis->in_stream,ev);

	if (l && !ev->_major)
	{
		l=l>>1;
		while (l--)
		{
			char *name=CORBA_DataInputStream_read_string(somThis->in_stream,ev);
			char *value=CORBA_DataInputStream_read_string(somThis->in_stream,ev);

			Context_set_one_value(ctx,ev,name,value);

			if (name) SOMFree(name);
			if (value) SOMFree(value);
		}
	}

	return ctx;
}

static void SOMLINK server_request_read_object(
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
	}
}

static ORBStatus rhbServerRequest_params(
			RHBServerRequest *somThis,
			Environment *ev,
			NVList SOMSTAR *args)
{
	long count=0,i=0;
	RHBORBStream_MarshalContext wrapper;
	ORBStatus status=0;
	NVList SOMSTAR list=args ? args[0] : NULL;
	SOMCDR_unmarshal_filter filter={NULL,NULL};

	filter.read_object=server_request_read_object;

/*	wrapper.c_orb=somThis->orb;*/
	wrapper.c_impl=somThis->impl;

/*	somPrintf("decoding %d parameters\n",args->_length);*/

	somThis->param_seq=args;	/* capture this as need if for the reply */

	if (list)
	{
		status=NVList_get_count(list,ev,&count);
	}

	wrapper.nv=NULL;

	RHBORBStream_GIOPInput__set_read_ctx(
			somThis->in_stream,
			ev,
			&wrapper);

	while ((i < count) && (!status) && (!ev->_major))
	{
		NamedValue nv=RHBSOMD_Init_NamedValue;

		status=NVList_get_item(list,ev,i,
			&nv.name,
			&nv.argument._type,
			&nv.argument._value,
			&nv.len,
			&nv.arg_modes);

		wrapper.nv=&nv;
		nv.arg_modes|=SERVER_SIDE;

		if (nv.arg_modes & (ARG_IN | ARG_INOUT))
		{
			if (!nv.argument._value)
			{
				size_t l=TypeCode_size(nv.argument._type,ev);
				if (l)
				{
					nv.argument._value=SOMMalloc(l);
					memset(nv.argument._value,0,l);
				}
			}

			NVList_set_item(list,ev,i,
					nv.name,
					nv.argument._type,
					nv.argument._value,
					nv.len,
					nv.arg_modes);

			RHBCDR_unmarshal(&filter,ev,
					wrapper.nv->argument._value,
					wrapper.nv->argument._type,
					somThis->in_stream);
		}
		else
		{
/*			somPrintf("not decoding argument '%s' as it is out only\n",args->_buffer[i].name);*/

			size_t l=TypeCode_size(wrapper.nv->argument._type,ev);

/*			somPrintf("buf allocating %ld bytes\n",l);*/

			if (l)
			{
				wrapper.nv->argument._value=SOMMalloc(l);
				memset(wrapper.nv->argument._value,0,l);
			}

			NVList_set_item(list,ev,i,
					nv.name,
					nv.argument._type,
					nv.argument._value,
					nv.len,
					nv.arg_modes);
		}

		i++;
	}

	wrapper.nv=NULL;

	RHBORBStream_GIOPInput__set_read_ctx(
			somThis->in_stream,
			ev,
			NULL);

	return status;
}

static Principal SOMSTAR rhbServerRequest_get_principal(
			RHBServerRequest *somThis,
			Environment *ev)
{
	if (!somThis->principal)
	{
		somThis->principal=SOMClass_somNew(somThis->orb->somClass_Principal_ref);
	}

	return Principal_somDuplicateReference(somThis->principal);
}


static void rhbServerRequest_cancel(RHBServerRequest *somThis,
											   Environment *ev)
{
	if (!somThis->cancelled)
	{
		if (!somThis->finished)
		{
			if (somThis->header.response_expected & 1)
			{
				RHBSOM_Trace("calling cancel server request")

				somThis->cancelled=1;
			}
			else
			{
				debug_somPrintf(("attempt to cancel 'oneway' server request for %s\n",
					somThis->header.operation));
			}
		}
	}
}

#ifdef USE_SELECT
void RHBServerRequest_add_to_impl_queue(
			RHBServerRequest *somThis,
			Environment *ev,
			RHBServerRequest_Process pfn)
{
	RHBServerRequest *o=somThis->impl->queued_request_list;
	somThis->qel.next=NULL;
	somThis->qel.process_request=pfn;

	if (o)
	{
		while (o->qel.next)
		{
			o=o->qel.next;
		}

		o->qel.next=somThis;
	}
	else
	{
		somThis->impl->queued_request_list=somThis;
	}
}
#endif

#ifdef _DEBUG
void RHBServerRequest_dump_chain(RHBServerRequest *somThis)
{
	int j=0;
	while (somThis)
	{
		char buf[256];
		unsigned int i=somThis->header.object_key._length;
		if (i >= sizeof(buf))
		{
			i=sizeof(buf)-1;
		}
		memcpy(buf,somThis->header.object_key._buffer,i);
		buf[i]=0;
		j++;
		dump_somPrintf(("request(%d,%p,%s,%d)\n operation=%s, key=%s\n",
			j,
			somThis,
			somThis->metrics.state_file,
			somThis->metrics.state_line,
			somThis->header.operation,
			buf));

		if (somThis->metrics.execute_chain)
		{
			somThis=*(somThis->metrics.execute_chain);
		}
		else
		{
			somThis=NULL;
		}
	}
}
#endif

#ifdef USE_THREADS
static void SOMLINK rhbServerRequest_exec_mt(void *r,Environment *ev)
{
	rhbServerRequest_Execute(r,ev);
}
#endif

#ifdef USE_THREADS
struct rhbServerRequest_exec_mt_failed
{
	RHBServerRequest *request;
	Environment ev;
};
RHBOPT_cleanup_begin(rhbServerRequest_exec_mt_failed_cleanup,pv)
struct rhbServerRequest_exec_mt_failed *data=pv;

	RHBServerRequest_Release(data->request);
	somdExceptionFree(&data->ev);

RHBOPT_cleanup_end

static void SOMLINK rhbServerRequest_exec_mt_failed(void *pv)
{
struct rhbServerRequest_exec_mt_failed data;
	any a={NULL,NULL};
	data.request=pv;
	SOM_InitEnvironment(&data.ev);

	RHBOPT_cleanup_push(rhbServerRequest_exec_mt_failed_cleanup,&data);

	a._type=TC_void;

	debug_somPrintf(("exec request failed to spawn thread.\n"));

	RHBOPT_throw_StExcep(&data.ev,NO_RESOURCES,CouldNotStartThread,NO);

	RHBServerRequest_set_result(data.request,&data.ev,&a);

	/* it's now not supposed to be in a list of some kind, of it's impl? */

	RHBOPT_cleanup_pop();
}
#endif

static struct RHBServerRequestVtbl rhbServerRequestVtbl=
{
	rhbServerRequest_AddRef,
	rhbServerRequest_Release,
	rhbServerRequest_cancel,
	rhbServerRequest_push_current,
	rhbServerRequest_pop_current,
	rhbServerRequest_set_result,
	rhbServerRequest_params,
	rhbServerRequest_ctx,
	rhbServerRequest_get_principal
};

RHBServerRequest * RHBServerRequestNew(
	Environment *ev,
	RHBImplementationDef *impl_ptr,
	GIOP_MessageHeader *message_ptr,
	GIOP_RequestHeader_1_0 *header_ptr,
	CORBA_DataInputStream SOMSTAR input)
{
	RHBServerRequest *somThis=NULL;

	if (ev->_major) return NULL;

	if (!impl_ptr->connection.connected) 
	{
		SOMD_bomb("follow");

		return NULL;
	}

	if (impl_ptr->is_closing) 
	{
		SOMD_bomb("follow");

		return NULL;
	}

	if (impl_is_deactivating(impl_ptr))
	{
		SOMD_bomb("follow");

		return NULL;
	}

	somThis=SOMMalloc(sizeof(*somThis));

	somThis->lpVtbl=&rhbServerRequestVtbl;

	somThis->executing_thread=NULL;

#ifdef USE_THREADS
	somThis->task.start=rhbServerRequest_exec_mt,
	somThis->task.failed=rhbServerRequest_exec_mt_failed;
	somThis->task.param=somThis;
#else
	somThis->exec_st=rhbServerRequest_Execute;
#endif

	somThis->next=NULL;
	somThis->somdServer=NULL;
	somThis->orb=impl_ptr->orb;
	RHBORB_AddRef(somThis->orb);

	somThis->param_seq=0;
	somThis->lUsage=1;
	somThis->closing=0;
	somThis->cancelled=0;
	somThis->finished=0;

	if (message_ptr)
	{
		somThis->protocol_version=message_ptr->protocol_version;
	}

	somThis->metrics.execute_chain=NULL;

	RHBServerRequest_state(somThis,"RHBServerRequestNew")

	somThis->impl=impl_ptr;

	RHBImplementationDef_AddRef(somThis->impl);

	if (header_ptr)
	{
		somThis->principal=header_ptr->requesting_principal;
		header_ptr->requesting_principal=NULL;

		somThis->header=*header_ptr;

		/* say we have grabbed the header... */

		header_ptr->object_key._length=0;
		header_ptr->object_key._maximum=0;
		header_ptr->object_key._buffer=NULL;
		header_ptr->operation=NULL;
		header_ptr->requesting_principal=NULL;
		header_ptr->service_context._length=0;
		header_ptr->service_context._maximum=0;
		header_ptr->service_context._buffer=NULL;
	}
	else
	{
		somThis->principal=NULL;

		somThis->header.object_key._length=0;
		somThis->header.object_key._maximum=0;
		somThis->header.object_key._buffer=NULL;
		somThis->header.operation=NULL;
		somThis->header.requesting_principal=NULL;
		somThis->header.service_context._length=0;
		somThis->header.service_context._maximum=0;
		somThis->header.service_context._buffer=NULL;
	}

	if (input)
	{
		input=CORBA_DataInputStream_somDuplicateReference(input);
	}

	somThis->in_stream=input;

	somThis->somd_contexts._buffer=NULL;
	somThis->somd_contexts._length=somThis->header.service_context._length;
	somThis->somd_contexts._maximum=somThis->header.service_context._length;

	if (somThis->header.service_context._length)
	{
		unsigned int i;

		somThis->somd_contexts._buffer=
				SOMMalloc(
					somThis->header.service_context._length
						*
						sizeof(*somThis->somd_contexts._buffer));

		i=somThis->header.service_context._length;

		while (i--)
		{
			somThis->somd_contexts._buffer[i].context_tag=
				somThis->header.service_context._buffer[i].context_id;
			somThis->somd_contexts._buffer[i].context_data=
				somThis->header.service_context._buffer[i].context_data;

			somThis->somd_contexts._buffer[i].context_body=0;
		}
	}

	/* say we have grabbed the data buffer */

	RHBDebug_created(RHBServerRequest,somThis)

	RHBImplementationDef_add_outstanding(somThis->impl,somThis);

	return somThis;
}
