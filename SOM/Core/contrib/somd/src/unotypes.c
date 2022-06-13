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

#define IOP_Codec_Class_Source
#define IOP_CodecFactory_Class_Source

#include <rhbsomd.h>
#include <unotypes.ih>

static unsigned long SOMLINK codec_decode_io(
		void *pv,Environment *ev,octet *buf,unsigned long len);

SOM_Scope _IDL_SEQUENCE_octet SOMLINK codec_encode_value(
		IOP_Codec SOMSTAR somSelf,
		Environment *ev,
		any *value)
{
	IOP_CodecData *somThis=IOP_CodecGetData(somSelf);
	_IDL_SEQUENCE_octet seq={0,0,NULL};

	RHBOPT_ASSERT(somThis->c_orb)

	if (!ev->_major)
	{
#ifdef somNewObject
		RHBORBStream_GIOPOutput SOMSTAR stream=somNewObject(RHBORBStream_GIOPOutput);
#else
		RHBORBStream_GIOPOutput SOMSTAR stream=RHBORBStream_GIOPOutputNew();
#endif

		if (!stream)
		{
			RHBOPT_throw_StExcep(ev,MARSHAL,UnexpectedNULL,MAYBE);
			return seq;
		}

		if (ev->_major)
		{
			somReleaseObjectReference(stream);

			return seq;
		}

		RHBORBStream_GIOPOutput__set_c_orb(stream,ev,somThis->c_orb);
		RHBORBStream_GIOPOutput__set_streamed_data(stream,ev,&seq);

		RHBORBStream_GIOPOutput_write_octet(stream,ev,
					RHBCDR_machine_type());

		RHBCDR_marshal(NULL,ev,
					value->_value,
					value->_type,
					stream);

		seq=RHBORBStream_GIOPOutput__get_streamed_data(stream,ev);

		somReleaseObjectReference(stream);

		if (ev->_major)
		{
			if (seq._buffer)
			{
				SOMFree(seq._buffer);
			}

			seq._buffer=NULL;
			seq._length=0;
			seq._maximum=0;
		}

	}

	return seq;
}

SOM_Scope _IDL_SEQUENCE_octet SOMLINK codec_encode(
		IOP_Codec SOMSTAR somSelf,
		Environment *ev,
		any *value)
{
	any a={NULL,NULL};
	a._value=value;
	a._type=TC_any;

	return IOP_Codec_encode_value(somSelf,ev,&a);
}

SOM_Scope any SOMLINK codec_decode(
		IOP_Codec SOMSTAR somSelf,
		Environment *ev,
		_IDL_SEQUENCE_octet *seq)
{
	return IOP_Codec_decode_value(somSelf,ev,seq,TC_any);
}

struct codec_decode_data
{
	RHBORBStream_GIOPInput SOMSTAR stream;
	unsigned long length;
	octet *ptr;
	any result;
};

static unsigned long SOMLINK codec_decode_io(
		void *pv,Environment *ev,octet *buf,unsigned long len)
{
struct codec_decode_data *data=pv;

	if (len && !ev->_major)
	{
		if ((len > data->length) || !buf)
		{
			RHBOPT_throw_StExcep(ev,MARSHAL,Marshal,MAYBE);
		}
		else
		{
			memcpy(buf,data->ptr,len);
			data->ptr+=len;
			data->length-=len;
		}
	}

	return len;
}

static void SOMLINK codec_read_object(
		struct SOMCDR_unmarshal_filter *filter,
		Environment *ev,
		void *value,
		TypeCode type,
		RHBORBStream_GIOPInput SOMSTAR stream)
{
	switch (TypeCode_kind(type,ev))
	{
	case tk_Principal:
		((Principal SOMSTAR *)value)[0]=RHBORBStream_GIOPInput_read_Principal(stream,ev);
		break;
	case tk_objref:
		((SOMObject SOMSTAR *)value)[0]=RHBORBStream_GIOPInput_read_SOMObject(stream,ev,type);
		break;
	default:
		RHBOPT_throw_StExcep(ev,MARSHAL,BadTypeCode,MAYBE);
		break;
	}
}

RHBOPT_cleanup_begin(codec_decode_value_cleanup,pv)

	struct codec_decode_data *data=pv;

	if (data->result._value && !data->result._type)
	{
		SOMFree(data->result._value);
		data->result._value=NULL;
	}

	if (data->stream)
	{
		somReleaseObjectReference(data->stream);
	}

RHBOPT_cleanup_end

SOM_Scope any SOMLINK codec_decode_value(
		IOP_Codec SOMSTAR somSelf,
		Environment *ev,
		_IDL_SEQUENCE_octet *seq,
		TypeCode tc)
{
	IOP_CodecData *somThis=IOP_CodecGetData(somSelf);
	SOMCDR_CDRInputStream_StreamState state;
	unsigned long len=TypeCode_size(tc,ev);
	struct codec_decode_data data={NULL,0,NULL,{NULL,NULL}};
	octet order=0;
	struct SOMCDR_unmarshal_filter filter;

	filter.read_object=codec_read_object;

	RHBOPT_ASSERT(somThis->c_orb)

	state.stream.stream_io=codec_decode_io;
	state.stream.stream_data=&data;
	state.stream.stream_offset=0;
	state.swap=0;

	RHBOPT_cleanup_push(codec_decode_value_cleanup,&data);

	if (!ev->_major
		&&
		len
		&&
		seq
		&&
		seq->_length)
	{
		data.length=seq->_length;
		data.ptr=seq->_buffer;

		state.stream.stream_offset+=
			state.stream.stream_io(
				state.stream.stream_data,ev,&order,sizeof(order));

		state.swap=(octet)(order != RHBCDR_machine_type());

		if (len)
		{
			data.result._value=SOMMalloc(len);
		}

		if (!data.result._value)
		{
			RHBOPT_throw_StExcep(ev,NO_MEMORY,UnexpectedNULL,MAYBE);
		}
		else
		{
			memset(data.result._value,0,len);

			data.stream=RHBORBStream_GIOPInputNew();

			RHBORBStream_GIOPInput__set_c_orb(data.stream,ev,somThis->c_orb);
			RHBORBStream_GIOPInput_stream_init(data.stream,ev,&filter,&state,0);

			RHBCDR_unmarshal(&filter,ev,data.result._value,tc,data.stream);

			if (!ev->_major)
			{
				data.result._type=TypeCode_copy(tc,ev);
			}
		}
	}
	else
	{
		RHBOPT_throw_StExcep(ev,MARSHAL,UnexpectedNULL,MAYBE);
	}

	RHBOPT_cleanup_pop();

	return data.result;
}

SOM_Scope IOP_Codec SOMSTAR SOMLINK codecf_create_codec(
	IOP_CodecFactory SOMSTAR somSelf,
	Environment *ev,
	IOP_Encoding *enc)
{
	IOP_CodecFactoryData *somThis=IOP_CodecFactoryGetData(somSelf);
	IOP_Codec SOMSTAR result=NULL;

	if (!ev->_major)
	{
#ifdef somNewObject
		result=somNewObject(IOP_Codec);
#else
		result=IOP_CodecNew();
#endif
		if (result)
		{
			IOP_Codec__set_c_orb(result,ev,somThis->c_orb);
			IOP_Codec__set_encoding(result,ev,enc);

			if (ev->_major)
			{
				somReleaseObjectReference(result);
				result=NULL;
			}
		}
		else
		{
			RHBOPT_throw_StExcep(ev,NO_MEMORY,NoMemory,MAYBE);
		}
	}
	
	return result;
}

SOM_Scope void SOMLINK codec__set_encoding(
		IOP_Codec SOMSTAR somSelf,
		Environment *ev,
		IOP_Encoding *encoding)
{
	IOP_CodecData *somThis=IOP_CodecGetData(somSelf);
	somThis->encoding=*encoding;
}

SOM_Scope IOP_Encoding SOMLINK codec__get_encoding(
		IOP_Codec SOMSTAR somSelf,
		Environment *ev)
{
	IOP_CodecData *somThis=IOP_CodecGetData(somSelf);
	return somThis->encoding;
}

SOM_Scope void SOMLINK codecf__set_c_orb(
	IOP_CodecFactory SOMSTAR somSelf,
	Environment *ev,
	RHBORB *orb)
{
	IOP_CodecFactoryData *somThis=IOP_CodecFactoryGetData(somSelf);

	somThis->c_orb=orb;
}

SOM_Scope RHBORB * SOMLINK codecf__get_c_orb(
	IOP_CodecFactory SOMSTAR somSelf,
	Environment *ev)
{
	IOP_CodecFactoryData *somThis=IOP_CodecFactoryGetData(somSelf);

	return somThis->c_orb;
}

SOM_Scope void SOMLINK codec__set_c_orb(
	IOP_Codec SOMSTAR somSelf,
	Environment *ev,
	RHBORB *orb)
{
	IOP_CodecData *somThis=IOP_CodecGetData(somSelf);

	somThis->c_orb=orb;
}


SOM_Scope RHBORB * SOMLINK codec__get_c_orb(
	IOP_Codec SOMSTAR somSelf,
	Environment *ev)
{
	IOP_CodecData *somThis=IOP_CodecGetData(somSelf);

	return somThis->c_orb;
}
