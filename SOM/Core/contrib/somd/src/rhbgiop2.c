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

struct RHBLocateRequest_set_result_data
{
	_IDL_SEQUENCE_octet reply_head;
	RHBORBStream_GIOPOutput SOMSTAR stream;
};

RHBOPT_cleanup_begin(RHBLocateRequest_set_result_cleanup,pv)

struct RHBLocateRequest_set_result_data *data=pv;

	if (data->reply_head._buffer)
	{
		SOMFree(data->reply_head._buffer);
	}

	if (data->stream)
	{
		somReleaseObjectReference(data->stream);
	}

RHBOPT_cleanup_end

static void RHBLocateRequest_set_result(
					RHBLocateRequest *somThis,
					Environment *ev,
					SOMDObject SOMSTAR somdobj)
{
	GIOP_LocateReplyHeader_1_0 reply;
	any RHBOPT_volatile a={NULL,NULL};
	struct RHBLocateRequest_set_result_data data={{0,0,NULL},NULL};

	if (somThis->cancelled || somThis->finished) return;
	if (!(somThis->impl->connection.connected)) return;

	RHBOPT_cleanup_push(RHBLocateRequest_set_result_cleanup,&data);

#ifdef somNewObject
	data.stream=somNewObject(RHBORBStream_GIOPOutput);
#else
	data.stream=RHBORBStream_GIOPOutputNew();
#endif

	RHBORBStream_GIOPOutput__set_c_orb(data.stream,ev,somThis->orb);
	RHBORBStream_GIOPOutput__set_streamed_data(data.stream,ev,NULL);

	memset(&reply,0,sizeof(reply));

	reply.request_id=somThis->header.request_id;

	if (somdobj)
	{
		if (SOMObject_somIsA(somdobj,somThis->impl->orb->somClass_SOMDObject_ref))
		{
			debug_somPrintf(("It's elsewhere\n"));

			reply.locate_status=GIOP_OBJECT_FORWARD;
		}
		else
		{
			/* it's here */

			debug_somPrintf(("It's here\n"));

			reply.locate_status=GIOP_OBJECT_HERE;
		}
	}
	else
	{
		debug_somPrintf(("Not here at all!!!!\n"));

		reply.locate_status=GIOP_UNKNOWN_OBJECT;
	}

	/* if exception
		write reply, string repid of exeception,
		else
		write return, and out/inout parameters */

	a._type=somdTC_GIOP_LocateReplyHeader_1_0;
	a._value=&reply;

/*	TypeCode_print(a._type,ev);*/

	RHBCDR_marshal(NULL,ev,a._value,a._type,data.stream);

/*	RHBGIOPORB_stream_write(&wrapper,ev,&reply_head,&a);*/

	if (reply.locate_status==GIOP_OBJECT_FORWARD)
	{
		CORBA_DataOutputStream_write_Object(data.stream,ev,somdobj);
	}

	data.reply_head=RHBORBStream_GIOPOutput__get_streamed_data(data.stream,ev);

	if (!somThis->cancelled)
	{
		RHBImplementationDef_write_GIOP_msg(
			somThis->impl,
			ev,
			&somThis->protocol_version,
			GIOP_LocateReply,
			&data.reply_head);

		somThis->finished=1;
	}

	RHBOPT_cleanup_pop();
}

struct rhbLocateRequest_exec_st
{
	SOMObject SOMSTAR target;
	RHBServerRequest *prev;
	RHBLocateRequest *request;
	boolean result;
};

RHBOPT_cleanup_begin(rhbLocateRequest_exec_st_cleanup,pv)

	struct rhbLocateRequest_exec_st *somThis=pv;

	RHBServerRequest_pop_current(somThis->request,&somThis->prev);

	if (somThis->target) somReleaseObjectReference(somThis->target);

	if (somThis->request) 
	{
		if (!somThis->result)
		{
			Environment ev;
			SOM_InitEnvironment(&ev);
			RHBLocateRequest_set_result(somThis->request,&ev,NULL);
			SOM_UninitEnvironment(&ev);
		}

		RHBServerRequest_Release(somThis->request);
	}

RHBOPT_cleanup_end

static void SOMLINK rhbLocateRequest_exec_st(RHBLocateRequest *somThis,Environment *ev)
{
	struct rhbLocateRequest_exec_st data={NULL,NULL,NULL,0};

	data.request=somThis;

	RHBOPT_cleanup_push(rhbLocateRequest_exec_st_cleanup,&data);

	RHBServerRequest_push_current(somThis,&data.prev);

	data.target=RHBImplementationDef_get_target_object(
		somThis->impl,
		ev,
		&somThis->header.object_key,
		&somThis->somdServer);

	if (ev->_major)
	{
		data.target=NULL;
		somdExceptionFree(ev);
	}

	RHBLocateRequest_set_result(somThis,ev,data.target);

	if (!ev->_major)
	{
		data.result=1;
	}

	RHBOPT_cleanup_pop();
}

#ifdef USE_THREADS
static void SOMLINK rhbLocateRequest_exec_mt(void *somThis,Environment *ev)
{
	rhbLocateRequest_exec_st(somThis,ev);
}
#endif

#ifdef USE_THREADS
struct rhbLocateRequest_exec_mt_failed
{
	RHBServerRequest *somThis;
	Environment ev;
};
RHBOPT_cleanup_begin(rhbLocateRequest_exec_mt_failed_cleanup,pv)
struct rhbLocateRequest_exec_mt_failed *data=pv;
	RHBServerRequest_Release(data->somThis);
	somdExceptionFree(&data->ev);
RHBOPT_cleanup_end

static void SOMLINK rhbLocateRequest_exec_mt_failed(void *pv)
{
struct rhbLocateRequest_exec_mt_failed data;
	data.somThis=pv;
	SOM_InitEnvironment(&data.ev);
	RHBOPT_cleanup_push(rhbLocateRequest_exec_mt_failed_cleanup,&data);
	RHBOPT_throw_StExcep(&data.ev,NO_RESOURCES,CouldNotStartThread,NO);
	RHBLocateRequest_set_result(data.somThis,&data.ev,NULL);
	RHBOPT_cleanup_pop();
}
#endif

RHBLocateRequest * RHBLocateRequestNew(
		Environment *ev,
		RHBImplementationDef *impl,
		GIOP_MessageHeader *message,
		GIOP_LocateRequestHeader *hdr,
		CORBA_DataInputStream SOMSTAR in_stream)
{
	RHBLocateRequest *somThis=RHBServerRequestNew(ev,impl,message,NULL,in_stream);

	somThis->header.request_id=hdr->request_id;
	somThis->header.object_key=hdr->object_key;

#ifdef USE_THREADS
	somThis->task.start=rhbLocateRequest_exec_mt,
	somThis->task.failed=rhbLocateRequest_exec_mt_failed;
	somThis->task.param=somThis;
#else
	somThis->exec_st=rhbLocateRequest_exec_st;
#endif

	return somThis;
}

