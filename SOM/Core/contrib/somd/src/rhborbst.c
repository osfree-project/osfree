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

#define RHBORBStream_GIOPInput_Class_Source
#define RHBORBStream_GIOPOutput_Class_Source

#include <rhbsomd.h>
#include <rhborbst.ih>

static void throw_marshal(Environment *ev)
{
	RHBOPT_throw_StExcep(ev,MARSHAL,Marshal,MAYBE);
}

struct rhborbsi_read_Principal
{
	_IDL_SEQUENCE_octet seq;
	Principal SOMSTAR result;
};

RHBOPT_cleanup_begin(rhborbsi_read_Principal_cleanup,pv)

struct rhborbsi_read_Principal *data=pv;

	if (data->seq._buffer) SOMFree(data->seq._buffer);
	if (data->result) somReleaseObjectReference(data->result);

RHBOPT_cleanup_end

SOM_Scope Principal SOMSTAR SOMLINK rhborbsi_read_Principal(
				RHBORBStream_GIOPInput SOMSTAR somSelf,
				Environment *ev)
{
	RHBORBStream_GIOPInputData *somThis=RHBORBStream_GIOPInputGetData(somSelf);
	struct rhborbsi_read_Principal data={{0,0,NULL},NULL};
	Principal SOMSTAR RHBOPT_volatile result=NULL;
	RHBORB *c_orb=somThis->c_orb;

	if (ev->_major || !c_orb)
	{
#ifdef _DEBUG
		RHBOPT_ASSERT(c_orb && !ev->_major )
#endif
		throw_marshal(ev);

		return NULL;
	}

	RHBOPT_cleanup_push(rhborbsi_read_Principal_cleanup,&data);

	RHBCDR_unmarshal(&somThis->filter,ev,&data.seq,somdTC_sequence_octet,somSelf);

	if (!ev->_major)
	{
		data.result=SOMClass_somNew(c_orb->somClass_Principal_ref);

		Principal_read_Principal(data.result,ev,&data.seq);

		if (!ev->_major)
		{
			result=data.result;
			data.result=NULL;
		}
	}

	RHBOPT_cleanup_pop();

	return result;
}

struct rhborbsi_read_SOMObject
{
	IOP_IOR ior;
	SOMObject SOMSTAR dobj;
	SOMDServer SOMSTAR server;
};

RHBOPT_cleanup_begin(rhborbsi_read_SOMObject_cleanup,pv)

struct rhborbsi_read_SOMObject *data=pv;
Environment ev;

	SOM_InitEnvironment(&ev);

	SOMD_FreeType(&ev,&data->ior,somdTC_IOP_IOR);

	if (data->dobj) somReleaseObjectReference(data->dobj);
	if (data->server) somReleaseObjectReference(data->server);

	SOM_UninitEnvironment(&ev);

RHBOPT_cleanup_end

SOM_Scope SOMObject SOMSTAR SOMLINK rhborbsi_read_SOMObject(
				RHBORBStream_GIOPInput SOMSTAR somSelf,
				Environment *ev,
				TypeCode hint)
{
	RHBORBStream_GIOPInputData *somThis=RHBORBStream_GIOPInputGetData(somSelf);
	struct rhborbsi_read_SOMObject data={{NULL,{0,0,NULL}},NULL,NULL};
	SOMObject SOMSTAR RHBOPT_volatile result=NULL;
	RHBORB *c_orb=somThis->c_orb;
	RHBImplementationDef * RHBOPT_volatile c_impl=NULL;

	if (somThis->read_ctx)
	{
		c_impl=somThis->read_ctx->c_impl;
	}

	if (!c_orb)
	{
#ifdef _DEBUG
		RHBOPT_ASSERT(c_orb)
#endif
		throw_marshal(ev);

		return NULL;
	}

	RHBOPT_cleanup_push(rhborbsi_read_SOMObject_cleanup,&data);

	RHBCDR_unmarshal(&somThis->filter,ev,&data.ior,somdTC_IOP_IOR,somSelf);

	result=RHBORB_get_object_from_ior(
			c_orb,
			ev,
			&data.ior,
			c_impl,
			0,
			hint);

	if (result && !ev->_major)
	{
		if (!SOMDObject_is_proxy(result,ev))
		{
			RHBObject *o=SOMDObject__get_c_proxy(result,ev);

			data.dobj=result;
			result=NULL;

			data.server=RHBImplementationDef_acquire_somdServer(o->impl,ev);

			if (data.server && !ev->_major)
			{
				result=SOMDServer_somdSOMObjFromRef(data.server,ev,data.dobj);
			}
			else
			{
				throw_marshal(ev);
			}
		}
	}

	RHBOPT_cleanup_pop();

	return result;
}

RHBOPT_cleanup_begin(rhborbso_write_Principal_cleanup,pv)

	_IDL_SEQUENCE_octet *data=pv;
	if (data->_buffer) SOMFree(data->_buffer);

RHBOPT_cleanup_end

SOM_Scope void SOMLINK rhborbso_write_Principal(
				RHBORBStream_GIOPOutput SOMSTAR somSelf,
				Environment *ev,
				Principal SOMSTAR value)
{
	if (!ev->_major)
	{
		if (value)
		{
			_IDL_SEQUENCE_octet seq={0,0,NULL};
			
			RHBOPT_cleanup_push(rhborbso_write_Principal_cleanup,&seq);

			seq=Principal_write_Principal(value,ev);

			if (!ev->_major)
			{
				RHBORBStream_GIOPOutput_write_ulong(somSelf,ev,seq._length);

				if (seq._length && !ev->_major)
				{
					RHBORBStream_GIOPOutput_write_octet_array(somSelf,ev,&seq,
							0,seq._length);
				}
			}

			RHBOPT_cleanup_pop();
		}
		else
		{
			RHBORBStream_GIOPOutput_write_ulong(somSelf,ev,0);
		}
	}
}

SOM_Scope void SOMLINK rhborbso_write_Object(
				RHBORBStream_GIOPOutput SOMSTAR somSelf,
				Environment *ev,
				SOMObject SOMSTAR value)
{
	RHBORBStream_GIOPOutputData *somThis=RHBORBStream_GIOPOutputGetData(somSelf);
	RHBORB *c_orb=somThis->c_orb;

	if (ev->_major || !c_orb) 
	{
#ifdef _DEBUG
		RHBOPT_ASSERT(c_orb && !ev->_major)
#endif
		throw_marshal(ev);
	}
	else
	{
		if (value)
		{
			if (SOMObject_somIsA(value,c_orb->somClass_SOMDObject_ref))
			{
				SOMDObject_somdMarshal(value,ev,somSelf);
			}
			else
			{
				RHBOPT_throw_StExcep(ev,MARSHAL,BadObjref,MAYBE);
			}
		}
		else
		{
			IOP_IOR ior={NULL,{0,0,NULL}};

			RHBCDR_marshal(NULL,ev,&ior,somdTC_IOP_IOR,somSelf);
		}
	}
}

SOM_Scope RHBORBStream_MarshalContext * SOMLINK rhborbsi__get_read_ctx(
				RHBORBStream_GIOPInput SOMSTAR somSelf,
				Environment *ev)
{
		RHBORBStream_GIOPInputData *somThis=RHBORBStream_GIOPInputGetData(somSelf);
		return somThis->read_ctx;
}

SOM_Scope void SOMLINK rhborbsi__set_read_ctx(
				RHBORBStream_GIOPInput SOMSTAR somSelf,
				Environment *ev,
				RHBORBStream_MarshalContext *mctx)
{
		RHBORBStream_GIOPInputData *somThis=RHBORBStream_GIOPInputGetData(somSelf);
		somThis->read_ctx=mctx;
}

SOM_Scope RHBORBStream_MarshalContext * SOMLINK rhborbso__get_write_ctx(
				RHBORBStream_GIOPOutput SOMSTAR somSelf,
				Environment *ev)
{
		RHBORBStream_GIOPOutputData *somThis=RHBORBStream_GIOPOutputGetData(somSelf);
		return somThis->write_ctx;
}

SOM_Scope void SOMLINK rhborbso__set_write_ctx(
				RHBORBStream_GIOPOutput SOMSTAR somSelf,
				Environment *ev,
				RHBORBStream_MarshalContext *mctx)
{
		RHBORBStream_GIOPOutputData *somThis=RHBORBStream_GIOPOutputGetData(somSelf);
		somThis->write_ctx=mctx;
}

static unsigned long SOMLINK decode_io(void *pv,Environment *ev,octet *buf,unsigned long len)
{
	unsigned long r=0;
	RHBORBStream_GIOPInputData *somThis=pv;

	if (len && !ev->_major)
	{
		if (len > somThis->read_length)
		{
			RHBOPT_throw_StExcep(ev,MARSHAL,UnexpectedNULL,MAYBE);
		}
		else
		{
			memcpy(buf,somThis->read_ptr,len);
			somThis->read_ptr+=len;
			somThis->read_length-=len;
			r=len;
		}
	}

	return r;
}

SOM_Scope void SOMLINK rhborbsi_stream_init_data(
				RHBORBStream_GIOPInput SOMSTAR somSelf,
				Environment *ev,
				_IDL_SEQUENCE_octet *data,
				boolean doSwap)
{
		RHBORBStream_GIOPInputData *somThis=RHBORBStream_GIOPInputGetData(somSelf);
		SOMCDR_CDRInputStream_StreamState ctx;

/*		if (doSwap) __asm int 3;*/

		somThis->data=*data;

		data->_length=0;
		data->_buffer=NULL;
		data->_maximum=0;
		somThis->read_ptr=somThis->data._buffer;
		somThis->read_length=somThis->data._length;

		ctx.swap=doSwap;
		ctx.stream.stream_io=decode_io;
		ctx.stream.stream_data=somThis;
		ctx.stream.stream_offset=0;

		RHBORBStream_GIOPInput_stream_init(somSelf,ev,&somThis->filter,&ctx,1);
}

static unsigned long SOMLINK marshal_octet_io(
		void *pv,
		Environment *ev,
		octet *buf,
		unsigned long len)
{
	unsigned long r=0;
	RHBORBStream_GIOPOutputData *somThis=pv;
/*	somPrintf("marshal_octet_io(%p,%d,%d,%d)\n",
			ot->_buffer,
			ot->_length,
			ot->_maximum,
			len);
*/
	if (len && !ev->_major)
	{
		unsigned long room=somThis->streamed_data._maximum-somThis->streamed_data._length;
		if (room < len)
		{
			octet *op=somThis->streamed_data._buffer;
			somThis->streamed_data._maximum+=1024;

			if ((somThis->streamed_data._length+len) > somThis->streamed_data._maximum)
			{
				somThis->streamed_data._maximum=somThis->streamed_data._length+len;
			}

			somThis->streamed_data._buffer=SOMMalloc(somThis->streamed_data._maximum);

			if (somThis->streamed_data._buffer)
			{
				if (somThis->streamed_data._length)
				{
					memcpy(somThis->streamed_data._buffer,op,somThis->streamed_data._length);
				}
			}
			else
			{
#ifdef _WIN32
				RHBOPT_ASSERT(somThis->streamed_data._buffer);
#endif
				RHBOPT_throw_StExcep(ev,NO_MEMORY,NoMemory,MAYBE);
				somThis->streamed_data._length=0;
				somThis->streamed_data._maximum=0;
			}
			
			if (op)	{ SOMFree(op);	}
		}

		if (!ev->_major)
		{
			RHBOPT_ASSERT((somThis->streamed_data._length+len)<=somThis->streamed_data._maximum)

			memcpy(somThis->streamed_data._buffer+somThis->streamed_data._length,buf,len);
			somThis->streamed_data._length+=len;
			r=len;
		}
	}

	return r;
}

SOM_Scope void SOMLINK rhborbso__set_streamed_data(
		RHBORBStream_GIOPOutput SOMSTAR somSelf,
		Environment *ev,
		_IDL_SEQUENCE_octet *data)
{
		RHBORBStream_GIOPOutputData *somThis=RHBORBStream_GIOPOutputGetData(somSelf);
		SOMCDR_StreamData ctx;

		if (somThis->streamed_data._buffer) SOMFree(somThis->streamed_data._buffer);

		if (data)
		{
			somThis->streamed_data=*data;
			data->_length=0;
			data->_maximum=0;
			data->_buffer=NULL;
		}
		else
		{
			somThis->streamed_data._length=0;
			somThis->streamed_data._maximum=0;
			somThis->streamed_data._buffer=NULL;
		}

		ctx.stream_io=marshal_octet_io;
		ctx.stream_data=somThis;
		ctx.stream_offset=somThis->streamed_data._length;

		RHBORBStream_GIOPOutput_stream_init(somSelf,ev,&ctx);
}

SOM_Scope _IDL_SEQUENCE_octet SOMLINK rhborbso__get_streamed_data(
		RHBORBStream_GIOPOutput SOMSTAR somSelf,
		Environment *ev)
{
		RHBORBStream_GIOPOutputData *somThis=RHBORBStream_GIOPOutputGetData(somSelf);
		_IDL_SEQUENCE_octet ret=somThis->streamed_data;
		somThis->streamed_data._buffer=NULL;
		somThis->streamed_data._maximum=0;
		somThis->streamed_data._length=0;
		return ret;
}

SOM_Scope void SOMLINK rhborbsi_somUninit(RHBORBStream_GIOPInput SOMSTAR somSelf)
{
		RHBORBStream_GIOPInputData *somThis=RHBORBStream_GIOPInputGetData(somSelf);

		if (somThis->data._buffer)
		{
			SOMFree(somThis->data._buffer);
		}
}

SOM_Scope void SOMLINK rhborbso_somUninit(RHBORBStream_GIOPOutput SOMSTAR somSelf)
{
		RHBORBStream_GIOPOutputData *somThis=RHBORBStream_GIOPOutputGetData(somSelf);
		
		if (somThis->streamed_data._buffer)
		{
			SOMFree(somThis->streamed_data._buffer);
		}
}

SOM_Scope void SOMLINK rhborbso__set_c_orb(
		RHBORBStream_GIOPOutput SOMSTAR somSelf,
		Environment *ev,
		RHBORB *orb)
{
	RHBORBStream_GIOPOutputData *somThis=RHBORBStream_GIOPOutputGetData(somSelf);
	somThis->c_orb=orb;
}

SOM_Scope RHBORB * SOMLINK rhborbso__get_c_orb(
		RHBORBStream_GIOPOutput SOMSTAR somSelf,
		Environment *ev)
{
	RHBORBStream_GIOPOutputData *somThis=RHBORBStream_GIOPOutputGetData(somSelf);
	return somThis->c_orb;
}

SOM_Scope void SOMLINK rhborbsi__set_c_orb(
		RHBORBStream_GIOPInput SOMSTAR somSelf,
		Environment *ev,
		RHBORB * orb)
{
	RHBORBStream_GIOPInputData *somThis=RHBORBStream_GIOPInputGetData(somSelf);
	somThis->c_orb=orb;
}

SOM_Scope RHBORB * SOMLINK rhborbsi__get_c_orb(
		RHBORBStream_GIOPInput SOMSTAR somSelf,
		Environment *ev)
{
	RHBORBStream_GIOPInputData *somThis=RHBORBStream_GIOPInputGetData(somSelf);
	return somThis->c_orb;
}

static void SOMLINK rhborbsi_read_object(
		SOMCDR_unmarshal_filter *filter,
		Environment *ev,
		void *value,
		TypeCode tc,
		SOMObject SOMSTAR stream)
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

SOM_Scope void SOMLINK rhborbsi_somInit(RHBORBStream_GIOPInput SOMSTAR somSelf)
{
	RHBORBStream_GIOPInputData *somThis=RHBORBStream_GIOPInputGetData(somSelf);
	somThis->filter.read_object=rhborbsi_read_object;
	RHBORBStream_GIOPInput_parent_SOMCDR_CDRInputStream_somInit(somSelf);
}

SOM_Scope SOMObject SOMSTAR SOMLINK rhborbsi_read_Object(
 		RHBORBStream_GIOPInput SOMSTAR somSelf,
		Environment *ev)
{
	return RHBORBStream_GIOPInput_read_SOMObject(somSelf,ev,TC_Object);
}

SOM_Scope void SOMLINK rhborbso_somDestruct(
		RHBORBStream_GIOPOutput SOMSTAR somSelf,
		boolean doFree,
		somDestructCtrl *ctrl)
{
	somDestructCtrl globalCtrl;
	somBooleanVector myMask;
	RHBORBStream_GIOPOutputData *somThis;

	RHBORBStream_GIOPOutput_BeginDestructor

	RHBOPT_unused(somThis);

	rhborbso_somUninit(somSelf);

	RHBORBStream_GIOPOutput_EndDestructor
}

SOM_Scope void SOMLINK rhborbsi_somDestruct(
		RHBORBStream_GIOPInput SOMSTAR somSelf,
		boolean doFree,
		somDestructCtrl *ctrl)
{
	somDestructCtrl globalCtrl;
	somBooleanVector myMask;
	RHBORBStream_GIOPInputData *somThis;

	RHBORBStream_GIOPInput_BeginDestructor

	RHBOPT_unused(somThis);
	rhborbsi_somUninit(somSelf);

	RHBORBStream_GIOPInput_EndDestructor
}
