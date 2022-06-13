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

/* code generater for somestio.idl */
#include <rhbopt.h>

#include <rhbsomex.h>

#ifdef USE_APPLE_SOM
#else
#	include <somref.h>
#endif

#define SOM_Module_somestio_Source
#define SOM_Module_omgestio_Source

#define somStream_StreamIO_Class_Source
#define somStream_MemoryStreamIO_Class_Source
#define somStream_Streamable_Class_Source

/* it is all very unfortunate that this module depends
	on SOMOS.DLL/somos.so/libsomos.sl etc,
	and that is caused by only one class, somStream_Streamable
		which actually has no implementation!!!  */

#include <somir.h>
#include <somd.h>
#include <rhbestrm.h>

#include <somestio.ih>

/* from CORBA externalization, page 8-29, section 8.11,
	'Standard Stream Format' */

#define tag_object		0xF0
#define tag_char		0xF1
#define tag_octet		0xF2
#define tag_ulong		0xF3
#define tag_ushort      0xF4
#define tag_long		0xF5
#define tag_short		0xF6
#define tag_float		0xF7
#define tag_double		0xF8
#define tag_boolean		0xF9
#define tag_string		0xFA
#define tag_object_ref  0x04
#define tag_nil_object  0x05

static boolean somestio_am_little_endian(void)
{
	short l=1;
	char *op=(void *)&l;

	return *op;
}

static void somestio_swap_bytes(void *data,unsigned long len)
{
	octet *p1,*p2;
	p1=data;
	p2=p1+len;

	len>>=1;

	while (len--)
	{
		octet o=*--p2;
		*p2=*p1;
		*p1++=o;
	}
}

static void somestio_bigendian_bytes(void *data,unsigned long len)
{
	if (somestio_am_little_endian())
	{
		somestio_swap_bytes(data,len);
	}
}

static void somestio_machine_order_bytes(void *data,unsigned long len)
{
	if (somestio_am_little_endian())
	{
		somestio_swap_bytes(data,len);
	}
}

static void somestio_append_data(
			_IDL_SEQUENCE_octet *stream,
			Environment *ev,
			void *data,
			unsigned long len)
{
	if (len && !ev->_major)
	{
		if ((stream->_length + len) > stream->_maximum)
		{
			unsigned long new_len=len+stream->_length+256;
			octet *buffer=SOMMalloc(new_len);
			if (stream->_length)
			{
				memcpy(buffer,stream->_buffer,stream->_length);
			}
			if (stream->_buffer) SOMFree(stream->_buffer);
			stream->_buffer=buffer;
			stream->_maximum=new_len;
		}

		memcpy(&stream->_buffer[stream->_length],data,len);
		stream->_length+=len;
	}
}

static boolean somestio_check_tag_len(Environment *ev,octet tag,unsigned long len)
{
	switch (tag)
	{
	case tag_long:
	case tag_ulong:
	case tag_float:
		if (len != 4)
		{
			RHBOPT_throw_StExcep(ev,INTERNAL,Marshal,NO);
			return 0;
		}
		break;
	case tag_char:
	case tag_boolean:
	case tag_octet:
		if (len != 1)
		{
			RHBOPT_throw_StExcep(ev,INTERNAL,Marshal,NO);
			return 0;
		}
		break;
	case tag_short:
	case tag_ushort:
		if (len != 2)
		{
			RHBOPT_throw_StExcep(ev,INTERNAL,Marshal,NO);
			return 0;
		}
		break;
	case tag_double:
		if (len != 8)
		{
			RHBOPT_throw_StExcep(ev,INTERNAL,Marshal,NO);
			return 0;
		}
		break;
	}
	return 1;
}

static void somestio_append_tagged_data(
			_IDL_SEQUENCE_octet *stream,
			Environment *ev,
			octet tag,
			void *data,
			unsigned long len)
{
	if (!somestio_check_tag_len(ev,tag,len))
	{
		return;
	}

	somestio_append_data(stream,ev,&tag,sizeof(tag));
	somestio_append_data(stream,ev,data,len);
}

static void somestio_zap_contents(somStream_MemoryStreamIOData *somThis)
{
	if (somThis->stream_buffer._buffer)
	{
		SOMFree(somThis->stream_buffer._buffer);
	}

	somThis->stream_buffer._length=0;
	somThis->stream_buffer._maximum=0;
	somThis->stream_buffer._buffer=0;
	somThis->stream_position=0;
}

static void somestio_read_data(
			_IDL_SEQUENCE_octet *stream,
			Environment *ev,
			unsigned long *pos,
			void *data,
			unsigned long len)
{
	if (!ev->_major)
	{
		if ((len+(*pos)) > (stream->_length))
		{
			somSetException(ev,USER_EXCEPTION,ex_CosStream_StreamDataFormatError,0);
		}
		else
		{
			memcpy(data,stream->_buffer+(*pos),len);
			(*pos)+=len;
		}
	}
}

static void somestio_read_tagged_data(
			_IDL_SEQUENCE_octet *stream,
			Environment *ev,
			unsigned long *pos,
			octet tag,
			void *data,
			unsigned long len)
{
	octet stag;

	if (!somestio_check_tag_len(ev,tag,len))
	{
		return;
	}

	if (ev->_major) return;

	somestio_read_data(stream,ev,pos,&stag,sizeof(stag));

	if (ev->_major) return;

	if (stag!=tag)
	{
		somSetException(ev,USER_EXCEPTION,ex_CosStream_StreamDataFormatError,0);
		return;
	}

	somestio_read_data(stream,ev,pos,data,len);
}

static char *somestio_read_string(
			_IDL_SEQUENCE_octet *stream,
			Environment *ev,
			unsigned long *pos)
{
	octet *p=stream->_buffer+(*pos);

	if (ev->_major) return 0;

	if ((stream->_length <= (*pos)))
	{
		somSetException(ev,USER_EXCEPTION,ex_CosStream_StreamDataFormatError,0);
		return 0;
	}

	if (*p)
	{
		unsigned long remainder=stream->_length-(*pos);
		octet *q;
		unsigned long act_len=1; /* include null terminator */
		char *r;

		q=p;

		while (*q)
		{
			if (!--remainder)
			{
				somSetException(ev,USER_EXCEPTION,ex_CosStream_StreamDataFormatError,0);
				return 0;
			}
			q++;
			act_len++;
		}

		(*pos)+=act_len;

		r=SOMMalloc(act_len);
		memcpy(r,p,act_len);
		return r;
	}
	
	(*pos)++;

	return 0;
}

/* overridden methods for ::somStream::StreamIO */
/* introduced methods for ::somStream::StreamIO */
/* introduced method ::somStream::StreamIO::already_streamed */
SOM_Scope boolean SOMLINK somestio_StreamIO_already_streamed(
	somStream_StreamIO SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMObject SOMSTAR obj,
	/* in */ SOMObject SOMSTAR class_obj)
{
	boolean __result=0;

	RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,NotImplemented,NO);

	return __result;
}
/* introduced method ::somStream::StreamIO::reset */
SOM_Scope void SOMLINK somestio_StreamIO_reset(
	somStream_StreamIO SOMSTAR somSelf,
	Environment *ev)
{
	RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,NotImplemented,NO);
}
/* introduced method ::somStream::StreamIO::set_buffer */
SOM_Scope void SOMLINK somestio_StreamIO_set_buffer(
	somStream_StreamIO SOMSTAR somSelf,
	Environment *ev,
	/* in */ somStream_seq_octet *buffer)
{
	RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,NotImplemented,NO);
}
/* introduced method ::somStream::StreamIO::get_buffer */
SOM_Scope somStream_seq_octet SOMLINK somestio_StreamIO_get_buffer(
	somStream_StreamIO SOMSTAR somSelf,
	Environment *ev)
{
	somStream_seq_octet __result={0,0,0};
	RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,NotImplemented,NO);
	return __result;
}
/* introduced method ::somStream::StreamIO::clear_buffer */
SOM_Scope void SOMLINK somestio_StreamIO_clear_buffer(
	somStream_StreamIO SOMSTAR somSelf,
	Environment *ev)
{
	RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,NotImplemented,NO);
}
/* overridden methods for ::somStream::MemoryStreamIO */
/* overridden method ::somStream::StreamIO::reset */
SOM_Scope void SOMLINK somestio_MemoryStreamIO_reset(
	somStream_MemoryStreamIO SOMSTAR somSelf,
	Environment *ev)
{
	somStream_MemoryStreamIOData *somThis=somStream_MemoryStreamIOGetData(somSelf);
	somThis->stream_position=0;
}
/* overridden method ::somStream::StreamIO::set_buffer */
SOM_Scope void SOMLINK somestio_MemoryStreamIO_set_buffer(
	somStream_MemoryStreamIO SOMSTAR somSelf,
	Environment *ev,
	/* in */ somStream_seq_octet *buffer)
{
	somStream_MemoryStreamIOData *somThis=somStream_MemoryStreamIOGetData(somSelf);
	somestio_zap_contents(somThis);

	if (buffer)
	{
		if (buffer->_length)
		{
			octet *op=SOMMalloc(buffer->_length);
			if (op)
			{
				memcpy(op,buffer->_buffer,buffer->_length);
				somThis->stream_buffer._buffer=op;
				somThis->stream_buffer._length=buffer->_length;
				somThis->stream_buffer._maximum=buffer->_length;
			}
			else
			{
				RHBOPT_throw_StExcep(ev,NO_MEMORY,NoMemory,NO);
			}
		}
	}
}
/* overridden method ::somStream::StreamIO::get_buffer */
SOM_Scope somStream_seq_octet SOMLINK somestio_MemoryStreamIO_get_buffer(
	somStream_MemoryStreamIO SOMSTAR somSelf,
	Environment *ev)
{
	somStream_seq_octet __result={0,0,0};
	somStream_MemoryStreamIOData *somThis=somStream_MemoryStreamIOGetData(somSelf);

	if (somThis->stream_buffer._length)
	{
		octet *op;

		op=SOMMalloc(somThis->stream_buffer._length);

		if (op)
		{
			__result._length=somThis->stream_buffer._length;
			__result._maximum=somThis->stream_buffer._length;
			__result._buffer=op;
			memcpy(op,somThis->stream_buffer._buffer,__result._length);
		}
		else
		{
			RHBOPT_throw_StExcep(ev,NO_MEMORY,NoMemory,NO);
		}
	}

	return __result;
}
/* overridden method ::somStream::StreamIO::clear_buffer */
SOM_Scope void SOMLINK somestio_MemoryStreamIO_clear_buffer(
	somStream_MemoryStreamIO SOMSTAR somSelf,
	Environment *ev)
{
	somStream_MemoryStreamIOData *somThis=somStream_MemoryStreamIOGetData(somSelf);

	somestio_zap_contents(somThis);
}
/* overridden method ::CosStream::StreamIO::write_string */
SOM_Scope void SOMLINK somestio_MemoryStreamIO_write_string(
	somStream_MemoryStreamIO SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring item)
{
	somStream_MemoryStreamIOData *somThis=somStream_MemoryStreamIOGetData(somSelf);

	if (item)
	{
		somestio_append_tagged_data(&somThis->stream_buffer,ev,tag_string,item,(int)strlen(item)+1);
	}
	else
	{
		somestio_append_tagged_data(&somThis->stream_buffer,ev,tag_string,"",1);
	}
}
/* overridden method ::CosStream::StreamIO::write_char */
SOM_Scope void SOMLINK somestio_MemoryStreamIO_write_char(
	somStream_MemoryStreamIO SOMSTAR somSelf,
	Environment *ev,
	/* in */ char item)
{
	somStream_MemoryStreamIOData *somThis=somStream_MemoryStreamIOGetData(somSelf);
	somestio_append_tagged_data(&somThis->stream_buffer,ev,tag_char,&item,sizeof(item));
}
/* overridden method ::CosStream::StreamIO::write_octet */
SOM_Scope void SOMLINK somestio_MemoryStreamIO_write_octet(
	somStream_MemoryStreamIO SOMSTAR somSelf,
	Environment *ev,
	/* in */ octet item)
{
	somStream_MemoryStreamIOData *somThis=somStream_MemoryStreamIOGetData(somSelf);
	somestio_append_tagged_data(&somThis->stream_buffer,ev,tag_octet,&item,sizeof(item));
}
/* overridden method ::CosStream::StreamIO::write_unsigned_long */
SOM_Scope void SOMLINK somestio_MemoryStreamIO_write_unsigned_long(
	somStream_MemoryStreamIO SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned long item)
{
	integer4 i4=item;
	somStream_MemoryStreamIOData *somThis=somStream_MemoryStreamIOGetData(somSelf);
	somestio_bigendian_bytes(&i4,sizeof(i4));
	somestio_append_tagged_data(&somThis->stream_buffer,ev,tag_ulong,&i4,sizeof(i4));
}
/* overridden method ::CosStream::StreamIO::write_unsigned_short */
SOM_Scope void SOMLINK somestio_MemoryStreamIO_write_unsigned_short(
	somStream_MemoryStreamIO SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned short item)
{
	unsigned short i2=item;
	somStream_MemoryStreamIOData *somThis=somStream_MemoryStreamIOGetData(somSelf);
	somestio_bigendian_bytes(&i2,sizeof(i2));
	somestio_append_tagged_data(&somThis->stream_buffer,ev,tag_ushort,&i2,sizeof(i2));
}
/* overridden method ::CosStream::StreamIO::write_long */
SOM_Scope void SOMLINK somestio_MemoryStreamIO_write_long(
	somStream_MemoryStreamIO SOMSTAR somSelf,
	Environment *ev,
	/* in */ long item)
{
	integer4 i4=item;
	somStream_MemoryStreamIOData *somThis=somStream_MemoryStreamIOGetData(somSelf);
	somestio_bigendian_bytes(&i4,sizeof(i4));
	somestio_append_tagged_data(&somThis->stream_buffer,ev,tag_long,&i4,sizeof(i4));
}
/* overridden method ::CosStream::StreamIO::write_short */
SOM_Scope void SOMLINK somestio_MemoryStreamIO_write_short(
	somStream_MemoryStreamIO SOMSTAR somSelf,
	Environment *ev,
	/* in */ short item)
{
	short i2=item;
	somStream_MemoryStreamIOData *somThis=somStream_MemoryStreamIOGetData(somSelf);
	somestio_bigendian_bytes(&i2,sizeof(i2));
	somestio_append_tagged_data(&somThis->stream_buffer,ev,tag_short,&i2,sizeof(i2));
}
/* overridden method ::CosStream::StreamIO::write_float */
SOM_Scope void SOMLINK somestio_MemoryStreamIO_write_float(
	somStream_MemoryStreamIO SOMSTAR somSelf,
	Environment *ev,
	/* in */ float item)
{
	float f4=item;
	somStream_MemoryStreamIOData *somThis=somStream_MemoryStreamIOGetData(somSelf);
	somestio_bigendian_bytes(&f4,sizeof(f4));
	somestio_append_tagged_data(&somThis->stream_buffer,ev,tag_float,&f4,sizeof(f4));
}
/* overridden method ::CosStream::StreamIO::write_double */
SOM_Scope void SOMLINK somestio_MemoryStreamIO_write_double(
	somStream_MemoryStreamIO SOMSTAR somSelf,
	Environment *ev,
	/* in */ double item)
{
	double f8=item;
	somStream_MemoryStreamIOData *somThis=somStream_MemoryStreamIOGetData(somSelf);
	somestio_bigendian_bytes(&f8,sizeof(f8));
	somestio_append_tagged_data(&somThis->stream_buffer,ev,tag_double,&f8,sizeof(f8));
}

/* overridden method ::CosStream::StreamIO::write_boolean */
SOM_Scope void SOMLINK somestio_MemoryStreamIO_write_boolean(
	somStream_MemoryStreamIO SOMSTAR somSelf,
	Environment *ev,
	/* in */ boolean item)
{
	somStream_MemoryStreamIOData *somThis=somStream_MemoryStreamIOGetData(somSelf);
	somestio_append_tagged_data(&somThis->stream_buffer,ev,tag_boolean,&item,sizeof(item));
}

/* overridden method ::CosStream::StreamIO::read_string */
SOM_Scope corbastring SOMLINK somestio_MemoryStreamIO_read_string(
	somStream_MemoryStreamIO SOMSTAR somSelf,
	Environment *ev)
{
	somStream_MemoryStreamIOData *somThis=somStream_MemoryStreamIOGetData(somSelf);
	octet tag=0;

	if (ev->_major) return NULL;
	
	somestio_read_data(&somThis->stream_buffer,ev,&somThis->stream_position,&tag,sizeof(tag));

	if (tag != tag_string)
	{
		somSetException(ev,USER_EXCEPTION,ex_CosStream_StreamDataFormatError,0);

		return NULL;
	}

	return somestio_read_string(&somThis->stream_buffer,ev,&somThis->stream_position);
}
/* overridden method ::CosStream::StreamIO::read_char */
SOM_Scope char SOMLINK somestio_MemoryStreamIO_read_char(
	somStream_MemoryStreamIO SOMSTAR somSelf,
	Environment *ev)
{
	char __result=0;
	somStream_MemoryStreamIOData *somThis=somStream_MemoryStreamIOGetData(somSelf);

	somestio_read_tagged_data(
			&somThis->stream_buffer,ev,&somThis->stream_position,
			tag_char,&__result,sizeof(__result));

	return __result;
}
/* overridden method ::CosStream::StreamIO::read_octet */
SOM_Scope octet SOMLINK somestio_MemoryStreamIO_read_octet(
	somStream_MemoryStreamIO SOMSTAR somSelf,
	Environment *ev)
{
	octet __result=0;
	somStream_MemoryStreamIOData *somThis=somStream_MemoryStreamIOGetData(somSelf);

	somestio_read_tagged_data(
			&somThis->stream_buffer,ev,&somThis->stream_position,
			tag_octet,&__result,sizeof(__result));
	return __result;
}
/* overridden method ::CosStream::StreamIO::read_unsigned_long */
SOM_Scope unsigned long SOMLINK somestio_MemoryStreamIO_read_unsigned_long(
	somStream_MemoryStreamIO SOMSTAR somSelf,
	Environment *ev)
{
	integer4 __result=0;
	somStream_MemoryStreamIOData *somThis=somStream_MemoryStreamIOGetData(somSelf);

	somestio_read_tagged_data(
			&somThis->stream_buffer,ev,&somThis->stream_position,
			tag_ulong,&__result,sizeof(__result));

	somestio_machine_order_bytes(&__result,sizeof(__result));

	return __result;
}
/* overridden method ::CosStream::StreamIO::read_unsigned_short */
SOM_Scope unsigned short SOMLINK somestio_MemoryStreamIO_read_unsigned_short(
	somStream_MemoryStreamIO SOMSTAR somSelf,
	Environment *ev)
{
	unsigned short __result=0;
	somStream_MemoryStreamIOData *somThis=somStream_MemoryStreamIOGetData(somSelf);

	somestio_read_tagged_data(
			&somThis->stream_buffer,ev,&somThis->stream_position,
			tag_ushort,&__result,sizeof(__result));

	somestio_machine_order_bytes(&__result,sizeof(__result));

	return __result;
}
/* overridden method ::CosStream::StreamIO::read_long */
SOM_Scope long SOMLINK somestio_MemoryStreamIO_read_long(
	somStream_MemoryStreamIO SOMSTAR somSelf,
	Environment *ev)
{
	integer4 __result=0;
	somStream_MemoryStreamIOData *somThis=somStream_MemoryStreamIOGetData(somSelf);

	somestio_read_tagged_data(
			&somThis->stream_buffer,ev,&somThis->stream_position,
			tag_long,&__result,sizeof(__result));

	somestio_machine_order_bytes(&__result,sizeof(__result));

	return __result;
}
/* overridden method ::CosStream::StreamIO::read_short */
SOM_Scope short SOMLINK somestio_MemoryStreamIO_read_short(
	somStream_MemoryStreamIO SOMSTAR somSelf,
	Environment *ev)
{
	short __result=0;
	somStream_MemoryStreamIOData *somThis=somStream_MemoryStreamIOGetData(somSelf);

	somestio_read_tagged_data(
			&somThis->stream_buffer,ev,&somThis->stream_position,
			tag_short,&__result,sizeof(__result));

	somestio_machine_order_bytes(&__result,sizeof(__result));

	return __result;
}
/* overridden method ::CosStream::StreamIO::read_float */
SOM_Scope float SOMLINK somestio_MemoryStreamIO_read_float(
	somStream_MemoryStreamIO SOMSTAR somSelf,
	Environment *ev)
{
	float __result;
	somStream_MemoryStreamIOData *somThis=somStream_MemoryStreamIOGetData(somSelf);

	somestio_read_tagged_data(
			&somThis->stream_buffer,ev,&somThis->stream_position,
			tag_float,&__result,sizeof(__result));

	somestio_machine_order_bytes(&__result,sizeof(__result));

	return __result;
}
/* overridden method ::CosStream::StreamIO::read_double */
SOM_Scope double SOMLINK somestio_MemoryStreamIO_read_double(
	somStream_MemoryStreamIO SOMSTAR somSelf,
	Environment *ev)
{
	double __result;
	somStream_MemoryStreamIOData *somThis=somStream_MemoryStreamIOGetData(somSelf);

	somestio_read_tagged_data(
			&somThis->stream_buffer,ev,&somThis->stream_position,
			tag_double,&__result,sizeof(__result));

	somestio_machine_order_bytes(&__result,sizeof(__result));

	return __result;
}
/* overridden method ::CosStream::StreamIO::read_boolean */
SOM_Scope boolean SOMLINK somestio_MemoryStreamIO_read_boolean(
	somStream_MemoryStreamIO SOMSTAR somSelf,
	Environment *ev)
{
	boolean __result;
	somStream_MemoryStreamIOData *somThis=somStream_MemoryStreamIOGetData(somSelf);

	somestio_read_tagged_data(
			&somThis->stream_buffer,ev,&somThis->stream_position,
			tag_boolean,&__result,sizeof(__result));

	return __result;
}

SOM_Scope void SOMLINK somestio_MemoryStreamIO_somInit(
	somStream_MemoryStreamIO SOMSTAR somSelf)
{
	somStream_MemoryStreamIOData *somThis=somStream_MemoryStreamIOGetData(somSelf);

	somThis->stream_buffer._length=0;
	somThis->stream_buffer._maximum=0;
	somThis->stream_buffer._buffer=0;
	somThis->stream_position=0;

	somStream_MemoryStreamIO_parent_somStream_StreamIO_somInit(somSelf);
}


SOM_Scope void SOMLINK somestio_MemoryStreamIO_somUninit(
	somStream_MemoryStreamIO SOMSTAR somSelf)
{
	somStream_MemoryStreamIOData *somThis=somStream_MemoryStreamIOGetData(somSelf);

	somestio_zap_contents(somThis);

	somStream_MemoryStreamIO_parent_somStream_StreamIO_somUninit(somSelf);
}


SOM_Scope CosStream_Streamable SOMSTAR SOMLINK somestio_MemoryStreamIO_read_object(
	somStream_MemoryStreamIO SOMSTAR somSelf,
	Environment *ev,
	CosLifeCycle_FactoryFinder SOMSTAR ff,
	CosStream_Streamable SOMSTAR obj)
{
	somStream_MemoryStreamIOData *somThis=somStream_MemoryStreamIOGetData(somSelf);
	CosLifeCycle_Key key={0,0,0};
	octet tag=0;
	octet i;

	if (ev->_major) return 0;

	somestio_read_data(
			&somThis->stream_buffer,
			ev,
			&somThis->stream_position,
			&tag,
			sizeof(tag));

	if (ev->_major) return 0;

	if (tag==tag_nil_object) return 0;

	if (tag!=tag_object)
	{
		somSetException(ev,USER_EXCEPTION,ex_CosStream_StreamDataFormatError,0);

		return 0;
	}

	somestio_read_data(
			&somThis->stream_buffer,
			ev,
			&somThis->stream_position,
			&i,
			sizeof(i));

	key._length=i;
	key._maximum=i;

	if (i)
	{
		key._buffer=SOMMalloc(i * sizeof(key._buffer[0]));

		i=0;

		while (i < key._length)
		{
			key._buffer[i].kind=0;
			key._buffer[i].id=somestio_read_string(
				&somThis->stream_buffer,ev,&somThis->stream_position);
			i++;
		}
	}

	CosStream_Streamable_internalize_from_stream(obj,ev,somSelf,ff);

	if (key._length)
	{
		while (key._length--)
		{
			char *p=key._buffer[key._length].id;
			if (p) SOMFree(p);
			p=key._buffer[key._length].kind;
			if (p) SOMFree(p);
		}
	}

	if (key._maximum)
	{
		SOMFree(key._buffer);
	}

	if (ev->_major) return 0;

	return obj;
}

SOM_Scope void SOMLINK somestio_MemoryStreamIO_write_object(
	somStream_MemoryStreamIO SOMSTAR somSelf,
	Environment *ev,
	CosStream_Streamable SOMSTAR obj)
{
	somStream_MemoryStreamIOData *somThis=somStream_MemoryStreamIOGetData(somSelf);

	if (ev->_major) return;

	if (obj)
	{
		octet tag=tag_object;
		unsigned long j;
		CosLifeCycle_Key key={0,0,0};
		octet i;

		key=CosStream_Streamable__get_external_form_id(obj,ev);

		if (ev->_major) return;

		somestio_append_data(&somThis->stream_buffer,ev,&tag,sizeof(tag));

		i=(octet)key._length;

		somestio_append_data(&somThis->stream_buffer,ev,&i,sizeof(i));

		j=key._length;

		while (j < key._length)
		{
			char *p=key._buffer[j].id;

			somestio_append_data(&somThis->stream_buffer,ev,p,(int)strlen(p)+1);

			SOMFree(p);

			p=key._buffer[j].kind;

			if (p) SOMFree(p);

			j++;
		}

		if (key._maximum)
		{
			SOMFree(key._buffer);
		}

		CosStream_Streamable_externalize_to_stream(obj,ev,somSelf);
	}
	else
	{
		octet tag=tag_nil_object;

		somestio_append_data(&somThis->stream_buffer,ev,&tag,sizeof(tag));
	}
}

