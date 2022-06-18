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

#define BUILD_RHBSOMUE

#include <rhbopt.h>
#include <rhbsomex.h>
#include <rhbsomut.h>
#include <somtc.h>
#include <somtcnst.h>
#include <somcdr.h>
#include <stexcep.h>
#include <somdtype.h>
#include <naming.h>
#include <rhbsomue.h>
#include <rhbunion.h>
#include <corba.h>

#ifndef _DEBUG
	#define RHBCDR_cast_any_to_TypeCode(ev,a)    (*((TypeCode *)((a)->_value)))
	#define RHBCDR_cast_any_to_string(ev,a)    (*((corbastring *)((a)->_value)))
#endif

#define RHBCDR_unmarshal_data(t)    struct RHBCDR_unmarshal_##t
#define RHBCDR_unmarshal_push(t)	data.filter=filter; RHBOPT_cleanup_push(RHBCDR_unmarshal_cleanup_##t,&data)
#define RHBCDR_unmarshal_pop()		if (!ev->_major) data.cleanup=0; RHBOPT_cleanup_pop()
#define RHBCDR_unmarshal_cleanup_begin(t) RHBOPT_cleanup_begin(RHBCDR_unmarshal_cleanup_##t,pv) RHBCDR_unmarshal_data(t) *data=pv;
#define RHBCDR_unmarshal_cleanup_end   RHBOPT_cleanup_end

RHBCDR_unmarshal_data(struct)
{
	boolean cleanup;
	SOMCDR_unmarshal_filter *filter;
	TypeCode type;
	octet *struct_base;
	unsigned long elements_read;
};

RHBCDR_unmarshal_data(sequence)
{
	boolean cleanup;
	SOMCDR_unmarshal_filter *filter;
	unsigned long element_size;
	TypeCode element_type;
	_IDL_SEQUENCE_octet *seqPtr;
};

RHBCDR_unmarshal_data(array)
{
	boolean cleanup;
	SOMCDR_unmarshal_filter *filter;
	unsigned long element_size;
	unsigned long elements_read;
	TypeCode element_type;
	octet *array_base;
};

RHBCDR_unmarshal_data(foreign)
{
	boolean cleanup;
	SOMCDR_unmarshal_filter *filter;
	void *value;
	TypeCode type;
};

RHBCDR_unmarshal_data(any)
{
	boolean cleanup;
	SOMCDR_unmarshal_filter *filter;
	any *result;
};

RHBCDR_unmarshal_cleanup_begin(any)

	if (data->cleanup)
	{
		void *pv=data->result->_value;
		TypeCode tc=data->result->_type;
		data->result->_value=NULL;
		data->result->_type=NULL;
		if (pv) SOMFree(pv);
		if (tc)
		{
			Environment ev;
			SOM_InitEnvironment(&ev);
			TypeCode_free(tc,&ev);
			SOM_UninitEnvironment(&ev);
		}
	}

RHBCDR_unmarshal_cleanup_end

RHBCDR_unmarshal_cleanup_begin(array)

	if (data->cleanup)
	{
		unsigned long len=data->elements_read;

		if (len)
		{
			Environment ev;
			octet *po=data->array_base;

			SOM_InitEnvironment(&ev);

			switch (TypeCode_kind(data->element_type,&ev))
			{
			case tk_null:
			case tk_void:
			case tk_double:
			case tk_long:
			case tk_ulong:
			case tk_short:
			case tk_ushort:
			case tk_float:
			case tk_octet:
			case tk_char:
			case tk_boolean:
				break;
			default:
				while (len--)
				{
					RHBCDR_free_data(&ev,data->filter,data->element_type,po);
					po+=data->element_size;
				}
			}

			SOM_UninitEnvironment(&ev);
		}
	}

RHBCDR_unmarshal_cleanup_end

RHBCDR_unmarshal_cleanup_begin(sequence)

	if (data->cleanup)
	{
		unsigned long len=data->seqPtr->_length;

		data->seqPtr->_length=0;
		data->seqPtr->_maximum=0;

		if (data->seqPtr->_buffer)
		{
			Environment ev;
			void *pv=data->seqPtr->_buffer;
			octet *po=pv;

			data->seqPtr->_buffer=NULL;

			SOM_InitEnvironment(&ev);

			switch (TypeCode_kind(data->element_type,&ev))
			{
			case tk_null:
			case tk_void:
			case tk_double:
			case tk_long:
			case tk_ulong:
			case tk_short:
			case tk_ushort:
			case tk_float:
			case tk_octet:
			case tk_char:
			case tk_boolean:
				break;
			default:
				while (len--)
				{
					RHBCDR_free_data(&ev,data->filter,data->element_type,po);
					po+=data->element_size;
				}
			}

			SOM_UninitEnvironment(&ev);

			SOMFree(pv);
		}
	}

RHBCDR_unmarshal_cleanup_end

RHBCDR_unmarshal_cleanup_begin(foreign)


	if (data->cleanup)
	{
		Environment ev;
		SOM_InitEnvironment(&ev);

		data->filter->free_object(data->filter,&ev,data->value,data->type);

/*		RHBCDR_call_foreign_handler(&ev,
			data->io,data->value,data->type,SOMD_OpFreeStorage);*/

		SOM_UninitEnvironment(&ev);
	}


RHBCDR_unmarshal_cleanup_end

RHBCDR_unmarshal_cleanup_begin(struct)

	if (data->cleanup && data->elements_read)
	{
		Environment ev;
		int i=2;
		int j=data->elements_read;
		unsigned long struct_offset=0;
		TypeCode type=data->type;

		SOM_InitEnvironment(&ev);

		while (j--)
		{
			any	b=TypeCode_parameter(type,&ev,i);
			TypeCode t2=RHBCDR_cast_any_to_TypeCode(&ev,&b);
			short align=TypeCode_alignment(t2,&ev);

			i+=2;

			RHBCDR_align_pos(&struct_offset,align);

			RHBCDR_free_data(&ev,data->filter,t2,data->struct_base+struct_offset);

			if (ev._major) break;

			struct_offset+=TypeCode_size(t2,&ev);
		}

		SOM_UninitEnvironment(&ev);
	}

RHBCDR_unmarshal_cleanup_end

SOMEXTERN void SOMLINK RHBCDR_unmarshal(
				SOMCDR_unmarshal_filter *filter,
				Environment *ev,
				void *value,
				TypeCode type,
				CORBA_DataInputStream SOMSTAR stream)
{
	if (ev->_major)
	{
		somPrintf("%s:%d, exception %s already raised\n",
			__FILE__,
			__LINE__,
			somExceptionId(ev));

		RHBOPT_ASSERT(!"exception already raised\n");

		return;
	}

	if (!value)
	{
		RHBOPT_throw_StExcep(ev,MARSHAL,UnexpectedNULL,MAYBE);

		return;
	}

	if (ev->_major) return;

	switch (TypeCode_kind(type,ev))
	{
	case tk_objref:
	case tk_Principal:
		filter->read_object(filter,ev,value,type,stream);
		break;
	case tk_TypeCode:
		{
			TypeCode *pt=value;
			*pt=CORBA_DataInputStream_read_TypeCode(stream,ev);
		}
		break;
	case tk_struct:
		{
			RHBCDR_unmarshal_data(struct) data={1,NULL,NULL,0};
			int RHBOPT_volatile i=2;
			int RHBOPT_volatile params=TypeCode_param_count(type,ev);
			unsigned long struct_offset=0;

			data.struct_base=value;
			data.type=type;
			params=(params-1)>>1;

			RHBCDR_unmarshal_push(struct);

			while (params-- && !ev->_major)
			{
				any b=TypeCode_parameter(type,ev,i);
				TypeCode t2=RHBCDR_cast_any_to_TypeCode(ev,&b);
				short align=TypeCode_alignment(t2,ev);

				i+=2;

				RHBCDR_align_pos(&struct_offset,align);

				RHBCDR_unmarshal(filter,ev,
						data.struct_base+struct_offset,
						t2,
						stream);


				if (ev->_major) break;

				struct_offset+=TypeCode_size(t2,ev);

				data.elements_read++;
			}

			RHBCDR_unmarshal_pop();
		}

		break;
	case tk_sequence:
		{
			RHBCDR_unmarshal_data(sequence) data={1,NULL,0,NULL,NULL};
			RHBOPT_volatile unsigned long len,maxLen;
			any t=TypeCode_parameter(type,ev,0);

			data.seqPtr=value;
			data.seqPtr->_length=0;
			data.seqPtr->_maximum=0;
			data.seqPtr->_buffer=NULL;
			data.element_type=RHBCDR_cast_any_to_TypeCode(ev,&t);
			data.element_size=TypeCode_size(data.element_type,ev);

			t=TypeCode_parameter(type,ev,1);
			maxLen=RHBCDR_cast_any_to_ulong(ev,&t);

			len=CORBA_DataInputStream_read_ulong(stream,ev);

			RHBOPT_ASSERT(!(len & 0xff000000))

			if (maxLen && (len > maxLen) && !ev->_major)
			{
				RHBOPT_throw_StExcep(ev,MARSHAL,Boundary,MAYBE);
			}

			if (len && !ev->_major)
			{
				data.seqPtr->_maximum=len;

				RHBCDR_unmarshal_push(sequence);
				
				switch (TypeCode_kind(data.element_type,ev))
				{
				case tk_char:
				case tk_octet:
				case tk_boolean:
					data.seqPtr->_buffer=SOMMalloc(len);
					data.seqPtr->_length=len;

					if (data.seqPtr->_buffer)
					{
						CORBA_DataInputStream_read_octet_array(stream,ev,data.seqPtr,0,len);
					}
					else
					{
						RHBOPT_throw_StExcep(ev,MARSHAL,NoMemory,MAYBE);
					}

					break;
				default:
					data.seqPtr->_buffer=SOMMalloc(data.element_size * len);

					if (data.seqPtr->_buffer)
					{
						octet *readPtr=data.seqPtr->_buffer;

						while (len-- && !ev->_major)
						{
							RHBCDR_unmarshal(filter,ev,readPtr,data.element_type,stream);

							if (ev->_major) break;

							readPtr+=data.element_size;
							data.seqPtr->_length++;
						}
					}
					else
					{
						RHBOPT_throw_StExcep(ev,MARSHAL,NoMemory,MAYBE);
					}
				}

				RHBCDR_unmarshal_pop();
			}
		}
		break;

	case tk_short:
		{
			CORBA_short *lp=value;

			*lp=CORBA_DataInputStream_read_short(stream,ev);
		}
		break;
	case tk_ushort:
		{
			CORBA_unsigned_short *lp=value;

			*lp=CORBA_DataInputStream_read_ushort(stream,ev);
		}
		break;

	case tk_enum:
		{
			CORBA_Enum *ep=value;
			GIOP_enum l;

			l=CORBA_DataInputStream_read_ulong(stream,ev);

			*ep=RHBCDR_GIOPenumToIDLenum(l,ev,type);
		}
		break;

	case tk_ulong:
		{
			CORBA_unsigned_long *lp=value;

			*lp=CORBA_DataInputStream_read_ulong(stream,ev);
		}
		break;
	case tk_long:
		{
			CORBA_long *lp=value;

			*lp=CORBA_DataInputStream_read_long(stream,ev);
		}
		break;
	case tk_char:
	case tk_octet:
	case tk_boolean:
		{
			octet *p=value;

			*p=CORBA_DataInputStream_read_octet(stream,ev);
		}
		break;
	case tk_array:
		{
			RHBCDR_unmarshal_data(array) data={1,NULL,0,0,NULL,NULL};
			any b=TypeCode_parameter(type,ev,0);
			long RHBOPT_volatile l=0;

			data.array_base=value;

			data.element_type=RHBCDR_cast_any_to_TypeCode(ev,&b);
			data.element_size=TypeCode_size(data.element_type,ev);

			b=TypeCode_parameter(type,ev,1);

			l=RHBCDR_cast_any_to_long(ev,&b);

			if (l)
			{
				switch (TypeCode_kind(data.element_type,ev))
				{
				case tk_octet:
				case tk_boolean:
				case tk_char:
					{
						_IDL_SEQUENCE_octet ot;
						ot._length=l;
						ot._maximum=l;
						ot._buffer=value;
						CORBA_DataInputStream_read_octet_array(
									stream,
									ev,
									&ot,
									0,
									l);
					}
					break;
				default:
					{
						octet * RHBOPT_volatile op=data.array_base;

						RHBCDR_unmarshal_push(array);

						while (l--)
						{
							RHBCDR_unmarshal(filter,ev,op,data.element_type,stream);

							if (ev->_major) break;

							op+=data.element_size;
							data.elements_read++;
						}

						RHBCDR_unmarshal_pop();
					}
				}
			}
		}
		break;
	case tk_string:
		{
			corbastring *hp=value;

			*hp=CORBA_DataInputStream_read_string(stream,ev);

			if (*hp && !ev->_major)
			{
				any a=TypeCode_parameter(type,ev,0);
				unsigned long ul=RHBCDR_cast_any_to_ulong(ev,&a);
				if (!ev->_major)
				{
					if (ul)
					{
						unsigned long sl=(unsigned long)strlen(*hp);
						if (sl > ul)
						{
							char *p=*hp;
							*hp=NULL;
							SOMFree(p);
							RHBOPT_throw_StExcep(ev,MARSHAL,Boundary,MAYBE);
						}
					}
				}
			}
		}
		break;
	case tk_any:
		{
			RHBCDR_unmarshal_data(any) data={1,NULL};
			data.result=value;
			data.result->_type=NULL;
			data.result->_value=NULL;

			RHBCDR_unmarshal_push(any);

			data.result->_type=CORBA_DataInputStream_read_TypeCode(stream,ev);

			if (!ev->_major)
			{
				int i=TypeCode_size(data.result->_type,ev);

				if (i)
				{
					data.result->_value=SOMMalloc(i);

					if (data.result->_value)
					{
						memset(data.result->_value,0,i);

						RHBCDR_unmarshal(filter,ev,data.result->_value,data.result->_type,stream);
					}
					else
					{
						RHBOPT_throw_StExcep(ev,MARSHAL,NoMemory,MAYBE);
					}
				}
			}

			RHBCDR_unmarshal_pop();
		}
		break;

	case tk_void:
		break;
	case tk_pointer:
		{
			void **hp=value;
			_IDL_SEQUENCE_octet val={0,0,NULL};
			RHBCDR_unmarshal_data(sequence) data={1,NULL,0,NULL,NULL};

			data.seqPtr=&val;

			val._maximum=CORBA_DataInputStream_read_ulong(stream,ev);

			if (val._maximum && !ev->_major)
			{
				any at=TypeCode_parameter(type,ev,0);
				data.element_type=RHBCDR_cast_any_to_TypeCode(ev,&at);
				data.element_size=TypeCode_size(data.element_type,ev);

				if (data.element_size && !ev->_major)
				{
					val._buffer=SOMMalloc(data.element_size * val._maximum);

					RHBCDR_unmarshal_push(sequence);

					if (val._buffer)
					{
						octet *op=val._buffer;

						while (val._length < val._maximum)
						{
							RHBCDR_unmarshal(filter,ev,op,data.element_type,stream);

							if (ev->_major)
							{
								break;
							}
							else
							{
								val._length++;
								op+=data.element_size;
							}
						}
					}
					else
					{
						RHBOPT_throw_StExcep(ev,MARSHAL,NoMemory,MAYBE);
					}

					RHBCDR_unmarshal_pop();
				}
			}

			*hp=val._buffer;
		}
		break;
	case tk_float:
		{
			CORBA_float *fp=value;
			*fp=CORBA_DataInputStream_read_float(stream,ev);
		}
		break;
	case tk_double:
		{
			CORBA_double *dp=value;
			*dp=CORBA_DataInputStream_read_double(stream,ev);
		}
		break;
	case tk_union:
		{
			/* first read discriminator */
			any discriminator=RHBUNION_get_switch(ev,type,value);

			RHBCDR_unmarshal(
					filter,
					ev,
					discriminator._value,
					discriminator._type,
					stream);

			if (!ev->_major)
			{
				any val=RHBUNION_get_value(ev,type,value);

				RHBCDR_unmarshal(filter,ev,val._value,val._type,stream);
			}
		}
		break;
	case tk_foreign:
		{
			RHBCDR_unmarshal_data(foreign) data={1,NULL,NULL,NULL};

			data.value=value;
			data.type=type;

			RHBCDR_unmarshal_push(foreign);

			data.filter->read_object(data.filter,ev,data.value,data.type,stream);

			RHBCDR_unmarshal_pop();
		}
		break;
	default:
		somPrintf("can't stream_read...");
		tcPrint(type,ev);
/*		bomb("RHBCDR_stream_read");*/
		RHBOPT_throw_StExcep(ev,MARSHAL,Marshal,NO);
	}
}

SOMEXTERN unsigned long SOMLINK RHBCDR_allocate_octets(
	_IDL_SEQUENCE_octet *a,
	Environment *ev,
	unsigned long length)
{
	unsigned long new_max;

/* #define ALLOC_GULP_SIZE 0   */

#ifndef ALLOC_GULP_SIZE
	#define ALLOC_GULP_SIZE  0x200
#endif

	if (ev->_major) return SOMDERROR_BadEnvironment;

	RHBOPT_ASSERT(a->_length <= a->_maximum)

	new_max=a->_length+length;

	if (new_max > a->_maximum)
	{
		if (new_max < (a->_maximum+ALLOC_GULP_SIZE))
		{
			new_max=a->_maximum+ALLOC_GULP_SIZE;
		}

		if (a->_maximum)
		{
			octet *oldPtr;

			oldPtr=a->_buffer;

			a->_buffer=(octet *)SOMMalloc(new_max);

			if (a->_buffer) 
			{
				memcpy(a->_buffer,oldPtr,a->_maximum);

				memset(&a->_buffer[a->_maximum],0,new_max - a->_maximum);
			}

			SOMFree(oldPtr);

			a->_maximum=new_max;
		}
		else
		{
			a->_buffer=(octet *)SOMMalloc(new_max);

			if (a->_buffer)
			{
				a->_maximum=new_max;
				memset(a->_buffer,0,new_max);
			}
		}
	}

	if (a->_buffer) return 0;

	RHBOPT_throw_StExcep(ev,NO_MEMORY,NoMemory,MAYBE);

	return SOMDERROR_NoMemory;
}

SOMEXTERN void SOMLINK RHBCDR_marshal(
				SOMCDR_marshal_filter *filter,
				Environment *ev,
				void *value,
				TypeCode type,
				CORBA_DataOutputStream SOMSTAR stream)
{
	if (ev->_major) 
	{
#ifdef DEBUG_MARSHAL
		somPrintf("RHBCDR_marshal, exception already raised %s\n",somExceptionId(ev));
#endif

		return;
	}

/*	SOMCDR_CDROutputStream__set_marshal_filter(stream,ev,filter);*/

	switch(TypeCode_kind(type,ev))
	{
	case tk_void:
		/* not much point sending anything for this */
		break;

	case tk_TypeCode:
		{
			TypeCode *pt=value;
			CORBA_DataOutputStream_write_TypeCode(stream,ev,*pt);
		}
		break;

	case tk_struct:
		{
			long params=TypeCode_param_count(type,ev);
#ifdef DEBUG_MARSHAL
			any struct_name=TypeCode_parameter(type,ev,0);
			char *p=RHBCDR_cast_any_to_string(ev,&struct_name);
#endif
			int i=1;
			octet *struct_base=value;
			unsigned long struct_offset=0;
/*			long struct_len=TypeCode_size(type,ev);*/

			/* align a->_value to start of structure... */

#ifdef DEBUG_MARSHAL			
			somPrintf("started writing struct '%s'\n",p);
#endif

			while (i < params)
			{
#ifdef DEBUG_MARSHAL
				any el_name=TypeCode_parameter(type,i);
				char *p=RHBCDR_cast_any_to_string(ev,&el_name);
#endif
				any el_type=TypeCode_parameter(type,ev,++i);
				TypeCode c_type=RHBCDR_cast_any_to_TypeCode(ev,&el_type);
				short align=TypeCode_alignment(c_type,ev);
				size_t size=TypeCode_size(c_type,ev);
#ifdef DEBUG_MARSHAL
				somPrintf("writing element '%s'\n",p);
#endif

				if (ev->_major)
				{
#ifdef DEBUG_MARSHAL
					somPrintf("%s:%d, exception=%s\n",__FILE__,__LINE__,somExceptionId(ev));
#endif

					break;
				}

				i++;

				RHBCDR_align_pos(&struct_offset,align);

				if (ev->_major)
				{
#ifdef DEBUG_MARSHAL
					somPrintf("%s:%d, exception=%s\n",__FILE__,__LINE__,somExceptionId(ev));
#endif
					break;
				}

				RHBCDR_marshal(filter,ev,struct_base+struct_offset,c_type,stream);

				if (ev->_major) 
				{
#ifdef DEBUG_MARSHAL
					somPrintf("RHBCDR_marshal, exception while marshalling struct %s\n",somExceptionId(ev));
#endif

					break;
				}

				struct_offset+=(unsigned long)size;
			}

		
#ifdef DEBUG_MARSHAL
			d=TypeCode_parameter(type,ev,0);
			p=RHBCDR_cast_any_to_string(ev,&d);
			somPrintf("finished writing struct '%s'\n",p);
#endif
		}
		break;
	case tk_string:
		{
			char **hp=value;
			CORBA_DataOutputStream_write_string(stream,ev,*hp);
		}
		break;
	case tk_array:
		{
			any b1=TypeCode_parameter(type,ev,1);
			unsigned long element_count=RHBCDR_cast_any_to_long(ev,&b1);
			any b2=TypeCode_parameter(type,ev,0);
			TypeCode element_type=RHBCDR_cast_any_to_TypeCode(ev,&b2);
			unsigned long element_len=TypeCode_size(element_type,ev);

			switch (TypeCode_kind(element_type,ev))
			{
			case tk_octet:
			case tk_boolean:
			case tk_char:
				{
					_IDL_SEQUENCE_octet seq;
					seq._length=element_count;
					seq._maximum=element_count;
					seq._buffer=value;
					CORBA_DataOutputStream_write_octet_array(stream,ev,&seq,0,element_count);
				}
				break;
			default:
				while (element_count-- && !ev->_major)
				{
					RHBCDR_marshal(filter,ev,value,element_type,stream);
					value=(((octet *)value)+element_len);
				}
			}
		}
		break;

	case tk_any:
		{
			any *b=value;
			CORBA_DataOutputStream_write_any(stream,ev,b);
		}
		break;

	case tk_sequence:
		{
			_IDL_SEQUENCE_octet *os=value;

			CORBA_DataOutputStream_write_ulong(stream,ev,os->_length);

			if (os->_length)
			{
				_IDL_SEQUENCE_octet seq=*os;
				any d=TypeCode_parameter(type,ev,0);
				TypeCode sequence_of=RHBCDR_cast_any_to_TypeCode(ev,&d);
				unsigned long ul=seq._length;
				unsigned long element_len=TypeCode_size(sequence_of,ev);

				switch (TypeCode_kind(sequence_of,ev))
				{
				case tk_octet:
				case tk_char:
				case tk_boolean:
					{
						CORBA_DataOutputStream_write_octet_array(stream,ev,&seq,0,seq._length);
					}
					break;
				default:
					while (ul-- && !ev->_major)
					{
						RHBCDR_marshal(filter,ev,seq._buffer,sequence_of,stream);
	
						seq._buffer+=element_len;
					}
				}
			}
		}
		break;

	case tk_ulong:
		{
			CORBA_unsigned_long *lp=value;
			CORBA_DataOutputStream_write_ulong(stream,ev,*lp);
		}
		break;
	case tk_long:
		{
			CORBA_long *lp=value;
			CORBA_DataOutputStream_write_long(stream,ev,*lp);
		}
		break;

	case tk_enum:
		{
			CORBA_Enum *lp=value;
			CORBA_Enum l=RHBCDR_IDLenumToGIOPenum(*lp,ev,type);

			if (!ev->_major)
			{
				CORBA_DataOutputStream_write_ulong(stream,ev,l);
			}
		}
		break;

	case tk_Principal:
		{
			any a;
			a._type=type;
			a._value=value;
			filter->write_object(filter,ev,&a,stream);
		}
		break;

	case tk_ushort:
		{
			CORBA_unsigned_short *p=value;
			CORBA_DataOutputStream_write_ushort(stream,ev,*p);
		}
		break;
	case tk_short:
		{
			CORBA_short *p=value;
			CORBA_DataOutputStream_write_short(stream,ev,*p);
		}
		break;

	case tk_octet:
		{
			octet *p=value;
			CORBA_DataOutputStream_write_octet(stream,ev,*p);
		}
		break;
	case tk_boolean:
		{
			octet *p=value;
			CORBA_DataOutputStream_write_boolean(stream,ev,*p);
		}
		break;
	case tk_char:
		{
			char *p=value;
			CORBA_DataOutputStream_write_char(stream,ev,*p);
		}
		break;

	case tk_objref:
		if (filter)
		{
			any a;
			a._value=value;
			a._type=type;
			filter->write_object(filter,ev,&a,stream);
		}
		else
		{
			SOMObject SOMSTAR *ho=value;
			CORBA_DataOutputStream_write_Object(stream,ev,*ho);
		}
		break;
	case tk_pointer:
		{
			/* recommend write a long zero if no pointer,
				else write a long one then write the value,
				hence encoding is exactly the same as a sequence<1> */
			any at=TypeCode_parameter(type,ev,0);
			void **hp=value;
			TypeCode atype=RHBCDR_cast_any_to_TypeCode(ev,&at);
			if (*hp)
			{
				CORBA_DataOutputStream_write_ulong(stream,ev,1);
				RHBCDR_marshal(filter,ev,*hp,atype,stream);
			}
			else
			{
				CORBA_DataOutputStream_write_ulong(stream,ev,0);
			}
		}
		break;
	case tk_double:
		{
			CORBA_double *dp=value;
			CORBA_DataOutputStream_write_double(stream,ev,*dp);
		}
		break;
	case tk_float:
		{
			CORBA_float *dp=value;
			CORBA_DataOutputStream_write_float(stream,ev,*dp);
		}
		break;
	case tk_union:
		{
			any discriminator=RHBUNION_get_switch(ev,type,value);
			any union_val=RHBUNION_get_value(ev,type,value);
			/* a union is encoded by writing out the
				switcher, followed by that case's data
			*/

			RHBCDR_marshal(filter,ev,discriminator._value,discriminator._type,stream);
			RHBCDR_marshal(filter,ev,union_val._value,union_val._type,stream);
		}
		break;
	case tk_foreign:
		{
			any data;
			data._type=type;
			data._value=value;

			filter->write_object(filter,ev,&data,stream);
		}
		break;
	default:
		{
			somPrintf("writing failed...");
			tcPrint(type,ev);
			RHBOPT_ASSERT(!"RHBCDR_stream_write");
		}

		RHBOPT_throw_StExcep(ev,BAD_TYPECODE,BadTypeCode,MAYBE);
	}
}

SOMEXTERN void SOMLINK RHBCDR_swap_bytes(void *pv,unsigned int len)
{
	octet *p;
	int i,j;

	p=(octet *)pv;

	i=len>>1;

	len--;

	j=0;

	while (i--)
	{
		octet c;

		c=p[j];
		p[j]=p[len];
		p[len]=c;

		j++;
		len--;
	}
}
static void enum_bomb(TypeCode tc,Environment *ev,long x)
{
	any a=TypeCode_parameter(tc,ev,0);
	char **h=a._value;
	char *p=*h;
#if defined(_WIN32) && defined(_DEBUG)
/*	__asm int 3;*/
#endif
	somPrintf("enum value out of bounds, TypeCode='%s',value=%ld\n",p,x);
	RHBOPT_throw_StExcep(ev,DATA_CONVERSION,Marshal,NO);

#ifdef _DEBUG
/*	bomb("sort out what is going on here...\n");*/
#endif
}

SOMEXTERN unsigned long SOMLINK RHBCDR_GIOPenumToIDLenum(GIOP_enum x,Environment *ev,TypeCode tc)
{
	/* this is done because enums generated by the SOM compiler start
		at one rather than zero as the CORBA 2.0 spec says,
		the CORBA spec says that tk_null is 0, SOM generates it as 1,
		so add this when converting from GIOP
	*/

	if (((GIOP_long)x) < 0) 
	{
		enum_bomb(tc,ev,x);

		x=(GIOP_enum)-tk_null;
	}
	else
	{
		if (tc) 
		{
			unsigned long elements=(unsigned long)(TypeCode_param_count(tc,ev)-1);

			if (TypeCode_getZeroOriginEnum(tc,ev))
			{
				if (x >= elements) 
				{
					enum_bomb(tc,ev,x);
				}

				return x;
			}

			if ((x+tk_null) > elements) 
			{
				enum_bomb(tc,ev,x);
			}
		}
	}

	return x+tk_null;
}

SOMEXTERN GIOP_enum SOMLINK RHBCDR_IDLenumToGIOPenum(
			unsigned long x,
			Environment *ev,
			TypeCode tc)
{
	/* this is done because enums generated by the SOM compiler start
		at one rather than zero as the CORBA 2.0 spec says,
		the CORBA spec says that tk_null is 0, SOM generates it as 1,
		so subtract this when converting to GIOP
	*/

	if (tc) 
	{
		unsigned long elements=(unsigned long)(TypeCode_param_count(tc,ev)-1);

		if (TypeCode_getZeroOriginEnum(tc,ev))
		{
			if (elements <= x) 
			{
				enum_bomb(tc,ev,x);
			}

			return x;
		}

		if (elements <= (x-tk_null)) 
		{
			enum_bomb(tc,ev,x);
		}
	}

	return x-tk_null;
}

