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

#ifndef SOM_Module_request_Source
#define SOM_Module_request_Source
#endif
#define Request_Class_Source

#include "request.ih"

SOM_Scope ORBStatus SOMLINK request_add_arg(
			Request SOMSTAR somSelf, 
			Environment *ev, 
			Identifier name, 
			TypeCode arg_type, 
			void* value, long len, 
			Flags arg_flags)
{
    RequestData *somThis = RequestGetData(somSelf);

    return RHBRequest_add_arg(somThis->c_request,
			ev,
			name,
			arg_type,
			value,
			len,
			arg_flags);
}
/*
static void SOMLINK release_oneway_final(void *pv)
{
	RHBRequest *request=pv;

	RHBRequest_Release(request);
}

static void SOMLINK release_oneway_thread(void *pv,Environment *ev)
{
	RHBRequest *request=pv;
	RHBOPT_unused(ev)

	RHBRequest_Release(request);
}*/

static void request_oneway_released(RHBRequest *request,void *pv)
{
	RHBOPT_unused(pv)

#if defined(USE_THREADS) && 0
	RHBORB_StartThreadTask(request->orb,
				release_oneway_thread,
				request,
				release_oneway_final);
#else
	RHBRequest_Release(request);
#endif

	debug_somPrintf(("Finished one way request\n"));
}

static ORBStatus request_send_oneway(
		RequestData *somThis,
		Environment *ev,
		Flags invoke_flags,
		SOMDServer SOMSTAR somdServer)
{
	RHBRequest *request=somThis->c_request;

	somThis->c_request=NULL;

	if (request)
	{
		RHBRequest_SetCompletionProc(request,request_oneway_released,0);

	    return RHBRequest_send(request,ev,invoke_flags,0,somdServer);
	}

	return SOMDERROR_RequestNotFound;
}

SOM_Scope ORBStatus  SOMLINK request_invoke(
		Request SOMSTAR somSelf, 
		Environment *ev, 
		Flags invoke_flags)
{
    RequestData *somThis=RequestGetData(somSelf);
	RHBORB_ThreadTask *thread=RHBSOMD_get_thread();
	SOMDServer SOMSTAR somdServer=(thread && thread->server_request) ? 
		thread->server_request->somdServer :
		SOMD_ServerObject;

	if (invoke_flags & INV_NO_RESPONSE)
	{
		return request_send_oneway(somThis,ev,invoke_flags,somdServer);
	}

	return RHBRequest_invoke(somThis->c_request,ev,invoke_flags,somdServer);
}

SOM_Scope ORBStatus  SOMLINK request_send(
		Request SOMSTAR somSelf,  
		Environment *ev, 
		Flags invoke_flags)
{
    RequestData *somThis = RequestGetData(somSelf);
	RHBORB_ThreadTask *thread=RHBSOMD_get_thread();
	SOMDServer SOMSTAR somdServer=(thread && thread->server_request) ? 
		thread->server_request->somdServer :
		SOMD_ServerObject;

	if (invoke_flags & INV_NO_RESPONSE)
	{
		return request_send_oneway(somThis,ev,invoke_flags,somdServer);
	}

    return RHBRequest_send(somThis->c_request,ev,invoke_flags,0,somdServer);
}

struct request_get_response
{
	RHBRequest *request;
	ORBStatus status;
};

RHBOPT_cleanup_begin(request_get_response_cleanup,pv)

struct request_get_response *data=pv;

	if (data->request)
	{
		RHBRequest_Release(data->request);
	}

RHBOPT_cleanup_end

SOM_Scope ORBStatus  SOMLINK request_get_response(
		Request SOMSTAR somSelf, 
		Environment *ev, 
		Flags response_flags)
{
	struct request_get_response data={NULL,SOMDERROR_RequestNotFound};
    RequestData *somThis = RequestGetData(somSelf); 

	RHBOPT_cleanup_push(request_get_response_cleanup,&data);

	RHBORB_guard(somThis->c_request->orb)

	data.request=somThis->c_request;

	if (data.request)
	{
		RHBRequest_AddRef(data.request);
	}

	RHBORB_unguard(somThis->c_request->orb)
	
	if (data.request)
	{
	    data.status=RHBRequest_get_response(data.request,ev,response_flags);
	}

	RHBOPT_cleanup_pop();

	return data.status;
}

struct request_destroy
{
	RHBRequest *request;
};

RHBOPT_cleanup_begin(request_destroy_cleanup,pv)

struct request_destroy *data=pv;

	if (data->request)
	{
		RHBRequest_Release(data->request);
	}

RHBOPT_cleanup_end

SOM_Scope ORBStatus  SOMLINK request_destroy(
		Request SOMSTAR somSelf, 
		Environment *ev)
{
struct request_destroy data={NULL};
RequestData *somThis=RequestGetData(somSelf);

	RHBOPT_cleanup_push(request_destroy_cleanup,&data);

	RHBORB_guard(somThis->c_request->orb)
	data.request=somThis->c_request;

	if (data.request)
	{
		RHBRequest_AddRef(data.request);
	}

	RHBORB_unguard(somThis->c_request->orb)

	if (data.request)
	{
		RHBRequest_cancel(data.request);
	}

	RHBOPT_unused(ev)

	RHBOPT_cleanup_pop();

    return 0;
}

#ifdef Request_BeginInitializer_somDefaultInit
SOM_Scope void SOMLINK request_somDefaultInit(
		Request SOMSTAR somSelf,
		somInitCtrl *ctrl)
{
    RequestData *somThis;
	somInitCtrl globalCtrl;
	somBooleanVector myMask;

	Request_BeginInitializer_somDefaultInit
#else
SOM_Scope void  SOMLINK request_somInit(
		Request SOMSTAR somSelf)
{
    RequestData *somThis = RequestGetData(somSelf);
#endif

	somThis->c_request=NULL;

#ifdef Request_Init_SOMRefObject_somDefaultInit
	Request_Init_SOMRefObject_somDefaultInit(somSelf,ctrl);
#else
	#ifdef Request_parent_SOMRefObject_somInit
		Request_parent_SOMRefObject_somInit(somSelf);
	#else
		Request_parent_SOMObject_somInit(somSelf);
	#endif
#endif
}

#ifdef Request_BeginDestructor
SOM_Scope void SOMLINK request_somDestruct(
		Request SOMSTAR somSelf,
		boolean doFree,
		somDestructCtrl *ctrl)
{
    RequestData *somThis;
	somDestructCtrl globalCtrl;
	somBooleanVector myMask;

	Request_BeginDestructor
#else
SOM_Scope void  SOMLINK request_somUninit(
		Request SOMSTAR somSelf)
{
    RequestData *somThis = RequestGetData(somSelf);
#endif

	if (somThis->c_request)
	{
		RHBRequest *r;
		RHBORB_guard(somThis->c_request->orb)
		r=somThis->c_request;
		somThis->c_request=NULL;
		RHBORB_unguard(somThis->c_request->orb)
		if (r)
		{
			RHBRequest_cancel(r);
			RHBRequest_Release(r);
		}
	}

#ifdef Request_EndDestructor
	Request_EndDestructor
#else
	#ifdef Request_parent_SOMRefObject_somUninit
		Request_parent_SOMRefObject_somUninit(somSelf);
	#else
		Request_parent_SOMObject_somUninit(somSelf);
	#endif
#endif
}

SOM_Scope void SOMLINK request__set_c_request(
			Request SOMSTAR somSelf,
			Environment *ev,
			RHBRequest *request)
{
	RequestData *somThis = RequestGetData(somSelf);

	RHBOPT_unused(ev)

	somThis->c_request=request;
}

SOM_Scope RHBRequest * SOMLINK request__get_c_request(Request SOMSTAR somSelf,Environment *ev)
{
	RHBRequest *request=RequestGetData(somSelf)->c_request;
	if (request)
	{
		RHBRequest_AddRef(request);
		return request;
	}
	RHBOPT_unused(ev)
	return NULL;
}

SOM_Scope void SOMLINK request_somDumpSelfInt(Request SOMSTAR somSelf,long level)
{
	RequestData *somThis=RequestGetData(somSelf);
	RHBRequest *request=somThis->c_request;
	if (request)
	{
		Environment ev;

		SOM_InitEnvironment(&ev);

		somPrefixLevel(level);
		dump_somPrintf(("operation : %s\n",request->request.operation));

		somPrefixLevel(level);
		dump_somPrintf(("pending   : %s\n",
			request->pending ? "yes" : "no"));

/*		somPrefixLevel(level);
		dump_somPrintf(("arguments : %ld\n",
				request->request.arg_list._length));
*/
		if (request->request.arg_list)
		{
			long i=0;
			long count=0;

			NVList_get_count(request->request.arg_list,&ev,&count);
				
			while (i < count)
			{
				NamedValue nv=RHBSOMD_Init_NamedValue;

				NVList_get_item(request->request.arg_list,&ev,i,
						&nv.name,
						&nv.argument._type,
						&nv.argument._value,
						&nv.len,
						&nv.arg_modes);

				somPrefixLevel(level+1);
				dump_somPrintf(("arg[%ld]",i));
				dump_somPrintf((" name=%s",nv.name));

				if (nv.arg_modes & ARG_IN) dump_somPrintf((" ARG_IN"));
				if (nv.arg_modes & ARG_OUT) dump_somPrintf((" ARG_OUT"));
				if (nv.arg_modes & ARG_INOUT) dump_somPrintf((" ARG_INOUT"));

				dump_somPrintf((" "));
				TypeCode_print(nv.argument._type,&ev);

				i++;
			}
		}

/*		somPrefixLevel(level);
		dump_somPrintf(("response  : "));
		switch (request->reply.ev._major)
		{
		case NO_EXCEPTION:
			dump_somPrintf(("NO_EXCEPTION\n"));
			break;
		case SYSTEM_EXCEPTION:
			dump_somPrintf(("SYSTEM_EXCEPTION, %s\n",
					somExceptionId(&request->reply.ev)));
			break;
		case USER_EXCEPTION:
			dump_somPrintf(("USER_EXCEPTION, %s\n",
					somExceptionId(&request->reply.ev)));
			break;
		}
*/
		SOM_UninitEnvironment(&ev);
	}
}
