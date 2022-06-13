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

#define SOMCDR_CDROutputStream_Class_Source
#define SOMCDR_CDRInputStream_Class_Source

#include <rhbsomex.h>
#include <rhbsomut.h>

#include <stexcep.h>
#include <rhbsomue.h>
#include <somtc.h>
#include <somtcnst.h>
#include <corba.h>
#include <somcdr.ih>

/* should be all write functions followed by all read fns */

#define throw_marshal(ev)  f_throw_marshal(__FILE__,__LINE__,ev)

#ifdef throw_marshal
static void f_throw_marshal(char *f,int l,Environment *ev)
#else
static void throw_marshal(Environment *ev)
#endif
{
#ifdef throw_marshal
	somPrintf("throw_marshal(%s,%d)\n",f,l);
#endif

	RHBOPT_throw_StExcep(ev,MARSHAL,Marshal,MAYBE);
}

static void throw_NULL(Environment *ev)
{
	RHBOPT_throw_StExcep(ev,MARSHAL,UnexpectedNULL,MAYBE);
}

static void stream_write(SOMCDR_marshalling_stream *st,Environment *ev,void *pv,unsigned long len)
{
	if (len && !ev->_major)
	{
		if (st->stream_io)
		{
			st->stream_offset+=st->stream_io(st->stream_data,ev,pv,len);
		}
		else
		{
			throw_marshal(ev);
		}
	}
}

static void stream_write_align(SOMCDR_marshalling_stream *st,Environment *ev,short off)
{
	if (ev->_major) return;

	if (off > 1)
	{
		int i=RHBCDR_align_diff(st->stream_offset,off);
		if (i)
		{
			static octet zeroes[8]={0,0,0,0,0,0,0,0};
			stream_write(st,ev,zeroes,i);
		}
	}
}

static void stream_write_octet(SOMCDR_marshalling_stream *st,Environment *ev,GIOP_octet value)
{
	stream_write(st,ev,&value,sizeof(value));
}

static void stream_write_float(SOMCDR_marshalling_stream *st,Environment *ev,GIOP_float value)
{
	stream_write_align(st,ev,4);
	stream_write(st,ev,&value,sizeof(value));
}

static void stream_write_double(SOMCDR_marshalling_stream *st,Environment *ev,GIOP_double value)
{
	stream_write_align(st,ev,8);
	stream_write(st,ev,&value,sizeof(value));
}

static void stream_write_ushort(SOMCDR_marshalling_stream *st,Environment *ev,GIOP_ushort value)
{
	stream_write_align(st,ev,2);
	stream_write(st,ev,&value,sizeof(value));
}

static void stream_write_short(SOMCDR_marshalling_stream *st,Environment *ev,GIOP_short value)
{
	stream_write_align(st,ev,2);
	stream_write(st,ev,&value,sizeof(value));
}

static void stream_write_boolean(SOMCDR_marshalling_stream *st,Environment *ev,GIOP_boolean value)
{
	stream_write(st,ev,&value,sizeof(value));
}

static void stream_write_char(SOMCDR_marshalling_stream *st,Environment *ev,GIOP_char value)
{
	stream_write(st,ev,&value,sizeof(value));
}

static void stream_write_ulong(SOMCDR_marshalling_stream *st,Environment *ev,GIOP_ulong value)
{
	stream_write_align(st,ev,4);
	stream_write(st,ev,&value,sizeof(value));
}

static void stream_write_long(SOMCDR_marshalling_stream *st,Environment *ev,GIOP_long value)
{
	stream_write_align(st,ev,4);
	stream_write(st,ev,&value,sizeof(value));
}

static void stream_write_string(SOMCDR_marshalling_stream *st,Environment *ev,char *value)
{
	if (value)
	{
		unsigned long l=(unsigned long)strlen(value)+1;

		stream_write_ulong(st,ev,l);
		stream_write(st,ev,value,l);
	}
	else
	{
		stream_write_ulong(st,ev,0);
	}
}

static void stream_write_value(SOMCDR_marshalling_stream *st,Environment *ev,void *pv,TypeCode type)
{
#define WRITE_VALUE(x)   case tk_##x: stream_write_##x(st,ev,*((x *)pv)); break;

	typedef unsigned long ulong;
	typedef unsigned short ushort;

	switch (TypeCode_kind(type,ev))
	{
	WRITE_VALUE(boolean);
	WRITE_VALUE(long);
	WRITE_VALUE(short);
	WRITE_VALUE(char);
	WRITE_VALUE(ulong);
	WRITE_VALUE(ushort);
	WRITE_VALUE(octet);
	case tk_enum:
		{
			CORBA_Enum e=*((CORBA_Enum *)pv);
			unsigned long ul=RHBCDR_IDLenumToGIOPenum(e,ev,type);
			stream_write_ulong(st,ev,ul);
		}
		break;
	case tk_string:
		{
			char **h=pv;
			stream_write_string(st,ev,*h);
		}
		break;
	default:
#ifdef _DEBUG
		somPrintf("can't write value ");
		TypeCode_print(type,ev);
#endif
		RHBOPT_throw_StExcep(ev,MARSHAL,BadTypeCode,MAYBE);
	}
}

static unsigned long SOMLINK encap_write_io(void *pv,Environment *ev,octet *buf,unsigned long len)
{
	RHBCDR_encapsulation_write *encap=pv;
	unsigned long r=0;

/*	somPrintf("encap_write_io(%d/%ld/%ld)\n",len,
		encap->stream_data._length,
		encap->stream_data._maximum);
*/
	if (len && !ev->_major)
	{
		unsigned long room=encap->stream_data._maximum-encap->stream_data._length;
		if (room < len)
		{
			octet *p=encap->stream_data._buffer;
			room=512;
			if ((encap->stream_data._maximum+room)
				<
				(encap->stream_data._length+len))
			{
				room=encap->stream_data._maximum-(encap->stream_data._length+len);
			}

			encap->stream_data._maximum+=room;
			encap->stream_data._buffer=SOMMalloc(encap->stream_data._maximum);
			if (encap->stream_data._buffer)
			{
				if (encap->stream_data._length)
				{
					memcpy(encap->stream_data._buffer,
						p,
						encap->stream_data._length);
				}
			}
			else
			{
				encap->stream_data._length=0;
				encap->stream_data._maximum=0;
				throw_marshal(ev);
			}

			if (p) SOMFree(p);
		}

		if (!ev->_major)
		{
			memcpy(encap->stream_data._buffer+encap->stream_data._length,
				buf,len);
			encap->stream_data._length+=len;
			r+=len;
		}
	}

	return r;
}

static RHBCDR_encapsulation_write *__RHBCDR_begin_write_encap(
					RHBCDR_encapsulation_write *encap,
					RHBCDR_encapsulation_write *ctx,
					SOMCDR_marshalling_stream *st,
					Environment *ev,TCKind kind)
{
	encap->ev=ev;
	encap->kind=kind;
	encap->parent_ctx=ctx;
	encap->stack.st=st;
	encap->stream_data._length=0;
	encap->stream_data._maximum=0;
	encap->stream_data._buffer=NULL;
	encap->stream.stream.stream_io=encap_write_io;
	encap->stream.stream.stream_data=encap;
	encap->stream.stream.stream_offset=0;
	encap->rep_id=NULL;
	encap->name=NULL;

	if (!ev->_major)
	{
		encap->stream_data._maximum=1024;
		encap->stream_data._buffer=SOMMalloc(encap->stream_data._maximum);

		if (encap->stream_data._buffer)
		{
			octet o=RHBCDR_machine_type();
			/* add the byte ordering first... */
			stream_write(&encap->stream.stream,ev,&o,1);
		}
		else
		{
			encap->stream_data._maximum=0;
			RHBOPT_throw_StExcep(ev,NO_MEMORY,NoMemory,MAYBE);
		}
	}

	/* this should be long aligned as TCKind should have been written */

	RHBOPT_ASSERT(!(st->stream_offset & 3))

	encap->buffer_start=st->stream_offset+4;  /* four is the length of the 
										long to start this encapsulation,
										which has not been written yet */

	if (ctx)
	{
		/* add to the parents start point */
		encap->buffer_start+=ctx->buffer_start;
	}

	if (ev->_major) return NULL;

	return encap;
}

RHBOPT_cleanup_begin(__RHBCDR_end_write_encap,pv)

	RHBCDR_encapsulation_write *encap=pv;

	if (encap->stream_data._buffer) SOMFree(encap->stream_data._buffer);
	encap->stream_data._buffer=NULL;
	encap->stream_data._length=0;
	encap->stream_data._maximum=0;

RHBOPT_cleanup_end

static void RHBCDR_write_rep_id_then_name(
			SOMCDR_marshalling_stream *ot,
			Environment *ev,
			any *a)
{
char *name=RHBCDR_cast_any_to_string(ev,a);

	/* only write rep_id if leads with '::' */

	if (name)
	{
		char *id=NULL;
		char *q=name;

		/* it's an ID if it has a colon in it */

		while (*q)
		{
			if (*q++==':')
			{
				id=name;
				break;
			}
		}

		stream_write_string(ot,ev,id);

		if (!(ev->_major))
		{
			char buf[256];
			char *p=name;

			if (id)
			{
				/* only write tail part of name */

				if (!memcmp(name,"::",2))
				{
					p+=strlen(p);

					while (p > name)
					{
						p--;
						if (*p==':')
						{
							p++;
							break;
						}
					}
				}
				else
				{
					if (!memcmp(name,"IDL:",4))
					{
						int i=0,j=(int)strlen(p);
						while (j)
						{
							j--;
							if (p[j]==':') break;
						}
						i=j;
						while (i)
						{
							i--;
							if ((p[i]==':')||(p[i]=='/')) { i++; break; }
						}
						j-=i;
						if (j > 0)
						{
							memcpy(buf,p+i,j);
							buf[j]=0;
							p=buf;
						}
					}
				}
			}

			stream_write_string(ot,ev,p);
		}
	}
	else
	{
		RHBOPT_throw_StExcep(ev,MARSHAL,UnexpectedNULL,MAYBE);
	}
}

static void __RHBCDR_commit_write_encap(RHBCDR_encapsulation_write *encap)
{
	Environment *ev=encap->ev;
	SOMCDR_marshalling_stream *ot=encap->stack.st;

	if (!ev->_major)
	{
		unsigned long len=encap->stream_data._length;

		/* write length to original stream */

/*		somPrintf("end_write_encap(%ld)\n",len);*/

		stream_write_ulong(ot,ev,len);

		if (len && !ev->_major)
		{
			/* append data to original stream */

			stream_write(ot,ev,encap->stream_data._buffer,len);
		}
	}
}

static void RHBCDR_stream_write_tc(
		RHBCDR_encapsulation_write *current_ctx,
		Environment *ev,
		SOMCDR_marshalling_stream *ot,
		TypeCode t)
{
	if (!ev->_major)
	{
		TCKind kind=TypeCode_kind(t,ev);

		stream_write_align(ot,ev,4);

		if (ev->_major) return;

		if (kind != tk_self)
		{
			/* only write long if this is not a tk_self */

			stream_write_ulong(ot,ev,RHBCDR_IDLenumToGIOPenum(kind,ev,0));
		}

		if (ev->_major) return;

		switch (kind)
		{
		case tk_null:
		case tk_void:
		case tk_short:
		case tk_long:
		case tk_ushort:
		case tk_ulong:
		case tk_float:
		case tk_double:
		case tk_boolean:
		case tk_char:
		case tk_octet:
		case tk_any:
		case tk_TypeCode:
		case tk_Principal:

			break;
		case tk_struct:
			/* corbastring, repository id, 
				corbastring, name,
				ulong count,

			*/
			{
				RHBCDR_begin_write_encap(ot,ev,kind)
				any any_name=TypeCode_parameter(t,ev,0);
				int i=TypeCode_param_count(t,ev),j=1;
				CORBA_unsigned_long ul=((i-1)>>1); /* count */
				current_ctx->name=RHBCDR_cast_any_to_string(ev,&any_name);

				RHBCDR_write_rep_id_then_name(ot,ev,&any_name);

				stream_write_long(ot,ev,ul);

				while (ul-- && !ev->_major)
				{
					any b;
					any a=TypeCode_parameter(t,ev,j);
					char *name=RHBCDR_cast_any_to_string(ev,&a);

					j++;

					/* member name */

					stream_write_string(ot,ev,name);

					if (ev->_major) break;

					/* member type */

					b=TypeCode_parameter(t,ev,j);

					j++;

					RHBCDR_stream_write_tc(current_ctx,ev,ot,
							RHBCDR_cast_any_to_TypeCode(ev,&b));
				}

				RHBCDR_end_write_encap
			}
			break;

		case tk_sequence:
			{
				RHBCDR_begin_write_encap(ot,ev,kind)
				any b=TypeCode_parameter(t,ev,0);   /* typecode */

				RHBCDR_stream_write_tc(current_ctx,ev,ot,
						RHBCDR_cast_any_to_TypeCode(ev,&b));

				if (!ev->_major)
				{
					CORBA_any a=TypeCode_parameter(t,ev,1);
					CORBA_unsigned_long ul=RHBCDR_cast_any_to_ulong(ev,&a);

					stream_write_ulong(ot,ev,ul);
				}

				RHBCDR_end_write_encap
			}
			break;

		case tk_string:
			{
				CORBA_any a=TypeCode_parameter(t,ev,0);	/* corbastring length */
				CORBA_unsigned_long ul=RHBCDR_cast_any_to_ulong(ev,&a);
				stream_write_ulong(ot,ev,ul);
			}
			break;

		case tk_array:
			{
				RHBCDR_begin_write_encap(ot,ev,kind)
				any b=TypeCode_parameter(t,ev,0);   /* typecode */

				RHBCDR_stream_write_tc(current_ctx,ev,ot,
						RHBCDR_cast_any_to_TypeCode(ev,&b));

				if (!ev->_major)
				{
					CORBA_any a=TypeCode_parameter(t,ev,1);
					CORBA_unsigned_long ul=RHBCDR_cast_any_to_ulong(ev,&a);

					stream_write_ulong(ot,ev,ul);
				}

				RHBCDR_end_write_encap
			}
			break;

		case tk_enum:
			{
				RHBCDR_begin_write_encap(ot,ev,kind)
				int i=1;
				any name=TypeCode_parameter(t,ev,0);
				CORBA_unsigned_long count=TypeCode_param_count(t,ev);

				/* repository id,
				   name,
				   count,
				   member name */

				SOM_IgnoreWarning(current_ctx);


				RHBCDR_write_rep_id_then_name(ot,ev,&name);

				count--;	/* don't include name */

				stream_write_ulong(ot,ev,count);

				while (count-- && !ev->_major)
				{
					any a=TypeCode_parameter(t,ev,i);
					char *p=RHBCDR_cast_any_to_string(ev,&a);
					i++;
					stream_write_string(ot,ev,p);
				}

				RHBCDR_end_write_encap
			}
			break;
		case tk_objref:
			{
				RHBCDR_begin_write_encap(ot,ev,kind)
				any name=TypeCode_parameter(t,ev,0);
				SOM_IgnoreWarning(current_ctx);
				RHBCDR_write_rep_id_then_name(ot,ev,&name);
				RHBCDR_end_write_encap
			}
			break;

		case tk_foreign:
			{
				RHBCDR_begin_write_encap(ot,ev,kind)
				SOM_IgnoreWarning(current_ctx);
				{
					any a=TypeCode_parameter(t,ev,0);	/* name */
					stream_write_string(ot,ev,RHBCDR_cast_any_to_string(ev,&a));
					a=TypeCode_parameter(t,ev,1);	/* impctx */
					stream_write_string(ot,ev,RHBCDR_cast_any_to_string(ev,&a));
					a=TypeCode_parameter(t,ev,2);	/* length */
					stream_write_long(ot,ev,RHBCDR_cast_any_to_long(ev,&a));
				}
				RHBCDR_end_write_encap
			}
			break;
		case tk_pointer:
			{
				RHBCDR_begin_write_encap(ot,ev,kind)
				any a=TypeCode_parameter(t,ev,0);
				TypeCode tc=RHBCDR_cast_any_to_TypeCode(ev,&a);
				RHBCDR_stream_write_tc(current_ctx,ev,ot,tc);	/* TypeCode */
				RHBCDR_end_write_encap
			}
			break;
		case tk_union:
			{
				RHBCDR_begin_write_encap(ot,ev,kind)
				long default_index=-1;
				int total_items=TypeCode_param_count(t,ev);
				unsigned long elements=0;
				any name=TypeCode_parameter(t,ev,0);
				any discriminator_type=TypeCode_parameter(t,ev,1);
				int k=0,j=2;

				while (j < total_items)
				{
					any case_id=TypeCode_parameter(t,ev,j);
					if (TypeCode_kind(case_id._type,ev)==tk_octet)
					{
						octet *op=case_id._value;
						if (op[0]==0)
						{
							default_index=k;
						}
					}

					k++;
					j+=3;
					elements++;
				}

				j=2;

				/* rep_id */

				stream_write_string(ot,ev,RHBCDR_cast_any_to_string(ev,&name));

				/* name */
				stream_write_string(ot,ev,RHBCDR_cast_any_to_string(ev,&name));

				/* descriminator type code */
				RHBCDR_stream_write_tc(current_ctx,ev,ot,
						RHBCDR_cast_any_to_TypeCode(ev,&discriminator_type));

				/* default used */

				stream_write_long(ot,ev,default_index);

				/* element count */

				stream_write_long(ot,ev,elements);

				k=0;

				while (elements--)
				{
	static			unsigned long zero=0;
				/*	any case_id=TypeCode_parameter(t,ev,j);*/
					any label_value=TypeCode_parameter(t,ev,j++);
					any member_name=TypeCode_parameter(t,ev,j++);
					any member_type=TypeCode_parameter(t,ev,j++);

					if (k==default_index)
					{
						/* label value for default item is always zero for SOM we think */
						stream_write_value(ot,ev,&zero,RHBCDR_cast_any_to_TypeCode(ev,&discriminator_type));
					}
					else
					{
						/* label value */
						stream_write_value(ot,ev,label_value._value,label_value._type);
					}

					/* member name */
					stream_write_value(ot,ev,member_name._value,member_name._type);

					/* member type */
					RHBCDR_stream_write_tc(current_ctx,ev,ot,
						RHBCDR_cast_any_to_TypeCode(ev,&member_type));

					k++;
				}
				RHBCDR_end_write_encap
			}
			break;
		case tk_self:
			if (current_ctx)
			{
				any a=TypeCode_parameter(t,ev,0);
				char *name=RHBCDR_cast_any_to_string(ev,&a);

				if (name && !ev->_major)
				{
					RHBCDR_encapsulation_write *p=current_ctx;

					while (p)
					{
						if (p->name)
						{
							if (!strcmp(p->name,name))
							{
								stream_write_long(ot,ev,-1);

								if (!ev->_major)
								{
									/* eight is for encap length and TCKind */
									long l=(p->buffer_start - 8)-(ot->stream_offset+current_ctx->buffer_start);
									
							/*		somPrintf("writing %ld\n",l);*/

									stream_write_long(ot,ev,l);
								}

								return;
							}
						}

						p=p->parent_ctx;
					}
				}
			}

			tcPrint(t,ev);
			somPrintf("\n");
			RHBOPT_ASSERT(!"can't handle this TypeCode\n");
			RHBOPT_throw_StExcep(ev,BAD_TYPECODE,BadTypeCode,MAYBE);
			break;
		default:
			{
				somPrintf("Can't stream_write_tc ");
				tcPrint(t,ev);
				RHBOPT_ASSERT(!"can't handle this TypeCode\n");
				RHBOPT_throw_StExcep(ev,BAD_TYPECODE,BadTypeCode,MAYBE);
			}
		}
	}
}

RHBOPT_cleanup_begin(__RHBCDR_end_read_encap,pv)

	RHBCDR_encapsulation_read *encap=pv;

/*   if (encap->encap_length)
   {
        somPrintf("%ld bytes left over\n",encap->encap_length);

		while (encap->encap_length)
	    {
            octet o=0;

            stream_read(&encap->stream,ev,&o,1);
            somPrintf("%02X ",o);
        }

        somPrintf("\n");
   }
*/
	if (encap->rep_id)
	{
		SOMFree(encap->rep_id);
	}

	if (encap->name)
	{
		SOMFree(encap->name);
	}

	if (encap->struct_data)
	{
		Environment ev;
		unsigned long i=encap->struct_data->_length;
		SOM_InitEnvironment(&ev);

		while (i--)
		{
			if (encap->struct_data->_buffer[i].name)
			{
				SOMFree(encap->struct_data->_buffer[i].name);
			}
		}

		i=encap->struct_data->_length;

		while (i--)
		{
			if (encap->struct_data->_buffer[i].type)
			{
				TypeCode_free(encap->struct_data->_buffer[i].type,&ev);
			}
		}

		if (encap->struct_data->_buffer)
		{
			SOMFree(encap->struct_data->_buffer);
		}

		SOM_UninitEnvironment(&ev);
	}

	if (encap->union_data)
	{
		Environment ev;
		unsigned long i=encap->union_data->_length;
		SOM_InitEnvironment(&ev);

		while (i--)
		{
			if (encap->union_data->_buffer[i].label._value)
			{
				SOMFree(encap->union_data->_buffer[i].label._value);
			}

			if (encap->union_data->_buffer[i].name)
			{
				SOMFree(encap->union_data->_buffer[i].name);
			}
		}

		i=encap->union_data->_length;

		while (i--)
		{
			if (encap->union_data->_buffer[i].type)
			{
				TypeCode_free(encap->union_data->_buffer[i].type,&ev);
			}
		}

		if (encap->union_data->_buffer)
		{
			SOMFree(encap->union_data->_buffer);
		}

		SOM_UninitEnvironment(&ev);
	}

	if (encap->enum_data)
	{
		Environment ev;
		unsigned long i=encap->enum_data->_length;

		SOM_InitEnvironment(&ev);

		while (i--)
		{
			if (encap->enum_data->_buffer[i])
			{
				SOMFree(encap->enum_data->_buffer[i]);
			}
		}

		if (encap->enum_data->_buffer)
		{
			SOMFree(encap->enum_data->_buffer);
		}

		SOM_UninitEnvironment(&ev);
	}

	if (encap->encap_tc)
	{
		Environment ev;
		SOM_InitEnvironment(&ev);

		TypeCode_free(encap->encap_tc,&ev);

		SOM_UninitEnvironment(&ev);
	}

RHBOPT_cleanup_end

#define ARRAY_BOUNDS_CHECK(ev,seq,off,len)  \
			if (!seq) { throw_NULL(ev); return; } \
			if (seq->_length > seq->_maximum) { throw_marshal(ev); return; } \
			if (off > seq->_length) { throw_marshal(ev); return; } \
			if ((off+len) > seq->_length) { throw_marshal(ev); return; } \
			if (!seq->_buffer) { throw_NULL(ev); return; } 


#define data_remaining(st)   ((st)->data._length -  (st)->offset)

SOM_Scope void SOMLINK cdrostr_completed(
			SOMCDR_CDROutputStream SOMSTAR somSelf,
			Environment *ev)
{
	SOMCDR_CDROutputStreamData *somThis=SOMCDR_CDROutputStreamGetData(somSelf);
/*	SOMCDR_StreamData ret=somThis->data;*/

	somThis->data.stream_data=NULL;
	somThis->data.stream_io=NULL;

	SOM_IgnoreWarning(ev);
}

SOM_Scope void SOMLINK cdrostr_stream_init(
			SOMCDR_CDROutputStream SOMSTAR somSelf,
			Environment *ev,
			SOMCDR_StreamData *data)
{
	SOMCDR_CDROutputStreamData *somThis=SOMCDR_CDROutputStreamGetData(somSelf);
	if (!ev->_major)
	{
		somThis->data=*data;
	}
}

SOM_Scope void SOMLINK cdrostr_somInit(
	SOMCDR_CDROutputStream SOMSTAR somSelf)
{
	SOMCDR_CDROutputStream_parent_CORBA_DataOutputStream_somInit(somSelf);
	SOM_IgnoreWarning(somSelf);
}

SOM_Scope void SOMLINK cdrostr_somUninit(
	SOMCDR_CDROutputStream SOMSTAR somSelf)
{
	SOMCDR_CDROutputStreamData *somThis=SOMCDR_CDROutputStreamGetData(somSelf);
	SOMCDR_StreamData data=somThis->data;
	somThis->data.stream_io=NULL;
	somThis->data.stream_data=NULL;
	if (data.stream_io) 
	{
		Environment ev;
		SOM_InitEnvironment(&ev);
		data.stream_io(data.stream_data,&ev,NULL,0);
		SOM_UninitEnvironment(&ev);
	}
}

SOM_Scope void SOMLINK cdrostr_write_ushort(
	SOMCDR_CDROutputStream SOMSTAR somSelf,
	Environment *ev,
	unsigned short value)
{
	SOMCDR_CDROutputStreamData *somThis=SOMCDR_CDROutputStreamGetData(somSelf);

	stream_write_ushort(&somThis->data,ev,value);
}

SOM_Scope void SOMLINK cdrostr_write_double_array(
	SOMCDR_CDROutputStream SOMSTAR somSelf,
	Environment *ev,
	CORBA_DoubleSeq *seq,
	unsigned long offset,
	unsigned long length)
{
	ARRAY_BOUNDS_CHECK(ev,seq,offset,length)
	while (length-- && !ev->_major)
	{
		CORBA_DataOutputStream_write_double(somSelf,ev,seq->_buffer[offset]);
		offset++;
	}
}

SOM_Scope void SOMLINK cdrostr_write_any(
	SOMCDR_CDROutputStream SOMSTAR somSelf,
	Environment *ev,
	any *value)
{
	if (!ev->_major)
	{
		if (value)
		{
			CORBA_DataOutputStream_write_TypeCode(somSelf,ev,value->_type);

			RHBCDR_marshal(
					SOMCDR_CDROutputStream__get_marshal_filter(somSelf,ev),
					ev,
					value->_value,
					value->_type,somSelf);
		}
		else
		{
			RHBOPT_throw_StExcep(ev,MARSHAL,UnexpectedNULL,MAYBE);
		}
	}
}

SOM_Scope void SOMLINK cdrostr_write_any_array(
	SOMCDR_CDROutputStream SOMSTAR somSelf,
	Environment *ev,
	CORBA_AnySeq *seq,
	unsigned long offset,
	unsigned long length)
{
	ARRAY_BOUNDS_CHECK(ev,seq,offset,length)
	while (length-- && !ev->_major)
	{
		CORBA_DataOutputStream_write_any(somSelf,ev,seq->_buffer+offset);
		offset++;
	}
}


SOM_Scope void SOMLINK cdrostr_write_char(
	SOMCDR_CDROutputStream SOMSTAR somSelf,
	Environment *ev,
	char value)
{
	SOMCDR_CDROutputStreamData *somThis=SOMCDR_CDROutputStreamGetData(somSelf);

	stream_write_char(&somThis->data,ev,value);
}

SOM_Scope void SOMLINK cdrostr_write_octet(
	SOMCDR_CDROutputStream SOMSTAR somSelf,
	Environment *ev,
	octet value)
{
	SOMCDR_CDROutputStreamData *somThis=SOMCDR_CDROutputStreamGetData(somSelf);

	stream_write_octet(&somThis->data,ev,value);
}

SOM_Scope void SOMLINK cdrostr_write_long(
	SOMCDR_CDROutputStream SOMSTAR somSelf,
	Environment *ev,
	long value)
{
	SOMCDR_CDROutputStreamData *somThis=SOMCDR_CDROutputStreamGetData(somSelf);

	stream_write_long(&somThis->data,ev,value);
}

SOM_Scope void SOMLINK cdrostr_write_ulong(
	SOMCDR_CDROutputStream SOMSTAR somSelf,
	Environment *ev,
	unsigned long value)
{
	SOMCDR_CDROutputStreamData *somThis=SOMCDR_CDROutputStreamGetData(somSelf);

	stream_write_ulong(&somThis->data,ev,value);
}

SOM_Scope void SOMLINK cdrostr_write_float(
	SOMCDR_CDROutputStream SOMSTAR somSelf,
	Environment *ev,
	float value)
{
	SOMCDR_CDROutputStreamData *somThis=SOMCDR_CDROutputStreamGetData(somSelf);

	stream_write_float(&somThis->data,ev,value);
}

SOM_Scope void SOMLINK cdrostr_write_double(
	SOMCDR_CDROutputStream SOMSTAR somSelf,
	Environment *ev,
	double value)
{
	SOMCDR_CDROutputStreamData *somThis=SOMCDR_CDROutputStreamGetData(somSelf);

	stream_write_double(&somThis->data,ev,value);
}

SOM_Scope void SOMLINK cdrostr_write_short(
	SOMCDR_CDROutputStream SOMSTAR somSelf,
	Environment *ev,
	short value)
{
	SOMCDR_CDROutputStreamData *somThis=SOMCDR_CDROutputStreamGetData(somSelf);

	stream_write_short(&somThis->data,ev,value);
}

SOM_Scope void SOMLINK cdrostr_write_boolean(
	SOMCDR_CDROutputStream SOMSTAR somSelf,
	Environment *ev,
	boolean value)
{
	SOMCDR_CDROutputStreamData *somThis=SOMCDR_CDROutputStreamGetData(somSelf);

	stream_write_boolean(&somThis->data,ev,value);
}

SOM_Scope void SOMLINK cdrostr_write_char_array(
	SOMCDR_CDROutputStream SOMSTAR somSelf,
	Environment *ev,
	CORBA_CharSeq *seq,
	unsigned long offset,
	unsigned long length)
{
	SOMCDR_CDROutputStreamData *somThis=SOMCDR_CDROutputStreamGetData(somSelf);

	ARRAY_BOUNDS_CHECK(ev,seq,offset,length)

	if (seq)
	{
		if (seq->_length >= (offset+length))
		{
			stream_write(&somThis->data,ev,seq->_buffer+offset,length);

			return;
		}
	}

	throw_marshal(ev);
}

SOM_Scope void SOMLINK cdrostr_write_octet_array(
	SOMCDR_CDROutputStream SOMSTAR somSelf,
	Environment *ev,
	CORBA_OctetSeq *seq,
	unsigned long offset,
	unsigned long length)
{
	SOMCDR_CDROutputStreamData *somThis=SOMCDR_CDROutputStreamGetData(somSelf);

	ARRAY_BOUNDS_CHECK(ev,seq,offset,length)

	if (seq)
	{
		if (seq->_length >= (offset+length))
		{
			stream_write(&somThis->data,ev,seq->_buffer+offset,length);

			return;
		}
	}

	throw_marshal(ev);
}

SOM_Scope void SOMLINK cdrostr_write_float_array(
	SOMCDR_CDROutputStream SOMSTAR somSelf,
	Environment *ev,
	CORBA_FloatSeq *seq,
	unsigned long offset,
	unsigned long length)
{
	ARRAY_BOUNDS_CHECK(ev,seq,offset,length)
	while (length-- && !ev->_major)
	{
		CORBA_DataOutputStream_write_float(somSelf,ev,seq->_buffer[offset]);
		offset++;
	}
}

SOM_Scope void SOMLINK cdrostr_write_ushort_array(
	SOMCDR_CDROutputStream SOMSTAR somSelf,
	Environment *ev,
	CORBA_UShortSeq *seq,
	unsigned long offset,
	unsigned long length)
{
	ARRAY_BOUNDS_CHECK(ev,seq,offset,length)
	while (length-- && !ev->_major)
	{
		CORBA_DataOutputStream_write_ushort(somSelf,ev,seq->_buffer[offset]);
		offset++;
	}
}

SOM_Scope void SOMLINK cdrostr_write_short_array(
	SOMCDR_CDROutputStream SOMSTAR somSelf,
	Environment *ev,
	CORBA_ShortSeq *seq,
	unsigned long offset,
	unsigned long length)
{
	ARRAY_BOUNDS_CHECK(ev,seq,offset,length)
	while (length-- && !ev->_major)
	{
		CORBA_DataOutputStream_write_short(somSelf,ev,seq->_buffer[offset]);
		offset++;
	}
}


SOM_Scope void SOMLINK cdrostr_write_ulong_array(
	SOMCDR_CDROutputStream SOMSTAR somSelf,
	Environment *ev,
	CORBA_ULongSeq *seq,
	unsigned long offset,
	unsigned long length)
{
	ARRAY_BOUNDS_CHECK(ev,seq,offset,length)
	while (length-- && !ev->_major)
	{
		CORBA_DataOutputStream_write_ulong(somSelf,ev,seq->_buffer[offset]);
		offset++;
	}
}

SOM_Scope void SOMLINK cdrostr_write_long_array(
	SOMCDR_CDROutputStream SOMSTAR somSelf,
	Environment *ev,
	CORBA_LongSeq *seq,
	unsigned long offset,
	unsigned long length)
{
	ARRAY_BOUNDS_CHECK(ev,seq,offset,length)
	while (length-- && !ev->_major)
	{
		CORBA_DataOutputStream_write_long(somSelf,ev,seq->_buffer[offset]);
		offset++;
	}
}

SOM_Scope void SOMLINK cdrostr_write_boolean_array(
	SOMCDR_CDROutputStream SOMSTAR somSelf,
	Environment *ev,
	CORBA_BooleanSeq *seq,
	unsigned long offset,
	unsigned long length)
{
	SOMCDR_CDROutputStreamData *somThis=SOMCDR_CDROutputStreamGetData(somSelf);

	ARRAY_BOUNDS_CHECK(ev,seq,offset,length)

	if (seq)
	{
		if (seq->_length >= (offset+length))
		{
			stream_write(&somThis->data,ev,seq->_buffer+offset,length);

			return;
		}
	}

	throw_marshal(ev);
}

SOM_Scope void SOMLINK cdrostr_write_TypeCode(
	SOMCDR_CDROutputStream SOMSTAR somSelf,
	Environment *ev,
	TypeCode value)
{
	SOMCDR_CDROutputStreamData *somThis=SOMCDR_CDROutputStreamGetData(somSelf);

	RHBCDR_stream_write_tc(NULL,ev,&somThis->data,value);
}

SOM_Scope void SOMLINK cdrostr_write_string(
	SOMCDR_CDROutputStream SOMSTAR somSelf,
	Environment *ev,
	char *value)
{
	SOMCDR_CDROutputStreamData *somThis=SOMCDR_CDROutputStreamGetData(somSelf);

	stream_write_string(&somThis->data,ev,value);

}

SOM_Scope void SOMLINK cdrostr_write_align(
			SOMCDR_CDROutputStream SOMSTAR somSelf,
			Environment *ev,
			unsigned long align)
{
	SOMCDR_CDROutputStreamData *somThis=SOMCDR_CDROutputStreamGetData(somSelf);

	stream_write_align(&somThis->data,ev,(unsigned short)align);
}

SOM_Scope SOMCDR_marshal_filter * SOMLINK cdrostr__get_marshal_filter(
		SOMCDR_CDROutputStream SOMSTAR somSelf,
		Environment *ev)
{
	SOMCDR_CDROutputStreamData *somThis=SOMCDR_CDROutputStreamGetData(somSelf);
	SOM_IgnoreWarning(ev);
	return somThis->marshal_filter;
}

SOM_Scope void SOMLINK cdrostr__set_marshal_filter(
		SOMCDR_CDROutputStream SOMSTAR somSelf,
		Environment *ev,
		SOMCDR_marshal_filter *filter)
{
	SOMCDR_CDROutputStreamData *somThis=SOMCDR_CDROutputStreamGetData(somSelf);

#ifdef _DEBUG
	if (filter)
	{
		RHBOPT_ASSERT(!somThis->marshal_filter)
	}
#endif

	somThis->marshal_filter=filter;
	SOM_IgnoreWarning(ev);
}

static unsigned long stream_read(SOMCDR_CDRInputStream_StreamState *st,Environment *ev,void *ptr,unsigned long len)
{
	if (!ev->_major)
	{
		unsigned long r=st->stream.stream_io(st->stream.stream_data,ev,ptr,len);

		if (!ev->_major)
		{
			if (r != len)
			{
				throw_marshal(ev);
			}
			else
			{
				st->stream.stream_offset+=r;
			}
		}

		return r;
	}

	return 0;
}

static void stream_read_align(SOMCDR_CDRInputStream_StreamState *st,Environment *ev,short off)
{
	if (!ev->_major)
	{
		if (off > 8) 
		{
			throw_marshal(ev);
		}
		else
		{
			if (off > 1)
			{
				int i=RHBCDR_align_diff(st->stream.stream_offset,off);
				if (i)
				{
					octet scrap[8];

					stream_read(st,ev,scrap,i);
				}
			}
		}
	}
}

static GIOP_octet stream_read_octet(SOMCDR_CDRInputStream_StreamState *st,Environment *ev)
{
	GIOP_octet o=0;

	if (ev->_major) return 0;

	stream_read(st,ev,&o,1); if (ev->_major) return 0;

	return o;
}

static GIOP_ushort stream_read_ushort(SOMCDR_CDRInputStream_StreamState *st,Environment *ev)
{
	GIOP_ushort l=0;

	if (ev->_major) return 0;

	stream_read_align(st,ev,2);
	stream_read(st,ev,&l,2); if (ev->_major) return 0;

	if (st->swap) RHBCDR_swap_bytes(&l,sizeof(l));

	return l;
}

static short stream_read_short(SOMCDR_CDRInputStream_StreamState *st,Environment *ev)
{
	GIOP_short l=0;

	if (ev->_major) return 0;

	stream_read_align(st,ev,2);

	stream_read(st,ev,&l,2); if (ev->_major) return 0;

	if (st->swap) RHBCDR_swap_bytes(&l,sizeof(l));

	return l;
}

static GIOP_long stream_read_long(SOMCDR_CDRInputStream_StreamState *st,Environment *ev)
{
	GIOP_long l=0;

	if (!ev->_major)
	{
		stream_read_align(st,ev,4);
		
		if (!ev->_major)
		{
			stream_read(st,ev,&l,4); 
			
			if (!ev->_major)
			{
				if (st->swap) RHBCDR_swap_bytes(&l,sizeof(l));
			}
		}
	}

	return l;
}

struct stream_read_string_data
{
	Environment *ev;
	boolean cleanup;
	char *str;
};

RHBOPT_cleanup_begin(stream_read_string_cleanup,pv)

	struct stream_read_string_data *data=pv;

	if (data->ev->_major || data->cleanup)
	{
		if (data->str) SOMFree(data->str);
		data->str=NULL;
	}

RHBOPT_cleanup_end

static char *stream_read_string(SOMCDR_CDRInputStream_StreamState *st,Environment *ev)
{
	unsigned long ul=stream_read_long(st,ev);

	if (ul && !ev->_major)
	{
		struct stream_read_string_data data={NULL,1,NULL};
		data.ev=ev;
		data.str=SOMMalloc(ul+1);

		if (data.str)
		{
			data.str[ul]=0;

			if (ul)	
			{
				RHBOPT_cleanup_push(stream_read_string_cleanup,&data);

				stream_read(st,ev,data.str,ul);

				data.cleanup=0;

				RHBOPT_cleanup_pop();
			}
		}

		if (!data.str) throw_marshal(ev);

		return data.str;
	}

	return NULL;
}

static unsigned long SOMLINK encap_read(void *pv,Environment *ev,octet *buf,unsigned long len)
{
	RHBCDR_encapsulation_read *somThis=pv;
	unsigned long r=0;

/*	somPrintf("encap_read(%ld/%ld)\n",len,somThis->encap_length);*/

	while (len && !ev->_major)
	{
		unsigned long room=somThis->encap_length;
		if (room > len)
		{
			room=len;
		}
		if (room)
		{
			room=stream_read(somThis->stack.st,
				ev,
				buf,
				room);
			r+=room;
			buf+=room;
			len-=room;
			somThis->encap_length-=room;
		}
		else
		{
			throw_marshal(ev);
		}
	}

	return r;
}

static RHBCDR_encapsulation_read *__RHBCDR_begin_read_encap(
			RHBCDR_encapsulation_read *encap,
			RHBCDR_encapsulation_read *parent_ctx,
			Environment *ev,
			SOMCDR_CDRInputStream_StreamState *st,TCKind kind)
{
	if (parent_ctx)
	{
		encap->nest=parent_ctx->nest+1;
	}
	else
	{
		encap->nest=0;
	}

/*	somPrintf("begin_read_encap(%ld)\n",encap->nest);*/

	encap->kind=kind;
	encap->parent_ctx=parent_ctx;

	encap->stack.st=st;

	/* get the root stream for this encapsulation */
	while (parent_ctx)
	{
		st=parent_ctx->stack.st;
		parent_ctx=parent_ctx->parent_ctx;
	}

	/* at this point, we have read the TCKind, so go back 4 to point to it */

	encap->tc_start_offset=st->stream.stream_offset;

/*	somPrintf("begin_read_encap(%ld)\n",encap->tc_start_offset);*/

	encap->stream.stream.stream_io=encap_read;
	encap->stream.stream.stream_data=encap;
	encap->stream.stream.stream_offset=0;
	encap->stream.swap=0;

	encap->rep_id=NULL;
	encap->name=NULL;

	encap->struct_data=NULL;
	encap->union_data=NULL;
	encap->encap_tc=NULL;
	encap->enum_data=NULL;

	encap->encap_length=stream_read_long(encap->stack.st,ev);

	if (encap->encap_length && !ev->_major)
	{
		octet o=0;

		stream_read(&encap->stream,ev,&o,1);
	
		encap->stream.swap=(o != RHBCDR_machine_type());
	}
	else
	{
		RHBOPT_throw_StExcep(ev,NO_MEMORY,NoMemory,MAYBE);
	}

	return encap;
}

static TypeCode RHBCDR_create_union_tc(Environment *ev,
				SOMCDR_RepositoryId id,
				SOMCDR_Identifier name,
				SOMCDR_VersionSpec version,
				TypeCode switch_type,
				SOMCDR_UnionMemberSeq *members)
{
	TypeCode tc=NULL;

	RHBSOMUT_unused(version)
	RHBSOMUT_unused(id)

	if (!ev->_major)
	{
		void *vabuf=somVaBuf_create(0,0);

		if (vabuf)
		{
			va_list args;
			void *p_null=NULL;
			unsigned long k=members->_length;
			SOMCDR_UnionMember *p=members->_buffer;
			TCKind kind=tk_union;

/*			if (id)
			{
				somVaBuf_add(vabuf,&id,tk_string);
				somVaBuf_add(vabuf,&kind,tk_enum);
				kind=tk_id;
			}
*/
			tc=TypeCode_copy(switch_type,ev);

			somVaBuf_add(vabuf,&name,tk_pointer);
			somVaBuf_add(vabuf,&tc,tk_pointer);

			while (k--)
			{
				long l=TCREGULAR_CASE;
				
				tc=TypeCode_copy(p->type,ev);

				if (tk_octet==TypeCode_kind(p->label._type,ev))
				{
					octet *op=p->label._value;
					if (!*op)
					{
						l=TCDEFAULT_CASE;
					}
				}

				somVaBuf_add(vabuf,&l,tk_long);

				if (l==TCREGULAR_CASE)
				{
					somVaBuf_add(vabuf,
							p->label._value,
							TypeCode_kind(p->label._type,ev));
				}
				else
				{
					l=0;
					somVaBuf_add(vabuf,&l,tk_long);
				}

				somVaBuf_add(vabuf,&p->name,tk_pointer);
				somVaBuf_add(vabuf,&tc,tk_pointer);

				p++;
			}

			somVaBuf_add(vabuf,&p_null,tk_pointer);

			somVaBuf_get_valist(vabuf,&args);

			tc=tcNewVL(kind,args);

			somVaBuf_destroy(vabuf);
		}
		else
		{
			RHBOPT_throw_StExcep(ev,NO_MEMORY,NoMemory,MAYBE);
		}
	}

	return tc;
}

static TypeCode RHBCDR_create_tk_self_from_ctx(
	   RHBCDR_encapsulation_read *ctx
	   )
{
	char *p=ctx->name;

	if (!p) p="";

	if (!p[0])
	{
		p=ctx->rep_id;
		}

	if (p) if (p[0]) return TypeCodeNew(tk_self,p);

	return 0;
}

static TypeCode RHBCDR_create_tk_self(RHBCDR_encapsulation_read *ctx,
			unsigned long posn)
{
	if (ctx)
	{
#ifdef _DEBUG
		RHBCDR_encapsulation_read *dbg=ctx;
#endif
		/* have read the long at this point so drop back by four */
/*		octet *start_pos=ctx->stream.data._buffer+ctx->stream.offset+posn-4;*/

		unsigned long start_offset=ctx->tc_start_offset+posn+4;
		RHBCDR_encapsulation_read *root=ctx;
		SOMCDR_CDRInputStream_StreamState *st=NULL;
		while (root)
		{
			st=root->stack.st;
			root=root->parent_ctx;
		}

		start_offset=st->stream.stream_offset+posn;

#ifdef _DEBUG_TK_SELF
		somPrintf("create tk_self from position %ld, %ld, %ld\n",
				posn,start_offset,st->stream.stream_offset);
#endif

		while (ctx)
		{
#ifdef _DEBUG_TK_SELF
			somPrintf("found tc at %ld, %s, %s\n",
					ctx->tc_start_offset,
					ctx->rep_id ? ctx->rep_id : "<unknown>",
					ctx->name ? ctx->name : "<unknown>");
#endif

			if (ctx->tc_start_offset == start_offset)
			{
				TypeCode tc=RHBCDR_create_tk_self_from_ctx(ctx);
				if (tc) return tc;
			}

/*			if (ctx->stream.data._buffer==start_pos)
			{
				TypeCode tc;
				somPrintf("warning, I think this is wrong\n");

				tc=RHBCDR_create_tk_self_from_ctx(ctx);
				if (tc) return tc;
			}
*/
			ctx=ctx->parent_ctx;
		}

#ifdef _DEBUG
		ctx=dbg;
	
		somPrintf("%s:%d, failed to find back-tracked TC at posn %ld\n",
				__FILE__,__LINE__,posn);

		while (ctx)
		{
			somPrintf("%p, %ld",
				ctx,
				ctx->tc_start_offset);

			if (ctx->rep_id) 
			{
				if (ctx->rep_id[0]) somPrintf(",%s",ctx->rep_id);
			}

			if (ctx->name) 
			{
				if (ctx->name[0]) somPrintf(",%s",ctx->name);
			}

			somPrintf("\n");
	
			if (!ctx->parent_ctx)
			{
				break;
			}

			ctx=ctx->parent_ctx;
		}

		RHBOPT_ASSERT(!"look at this\n");
#endif
	}

	return 0;
}

static TypeCode RHBCDR_stream_read_tc(
	RHBCDR_encapsulation_read *current_ctx_arg,
	Environment *ev,
	SOMCDR_CDRInputStream_StreamState *st_arg)
{
	RHBCDR_encapsulation_read * RHBOPT_volatile current_ctx=current_ctx_arg;
	SOMCDR_CDRInputStream_StreamState * RHBOPT_volatile st=st_arg;
	CORBA_long k;
	TCKind kind=0;
	TypeCode RHBOPT_volatile result=NULL;

	if (ev->_major) return result;

	k=stream_read_long(st,ev);

	if (ev->_major) return result;

	if (k==-1)
	{
		/* this is a special TypeCode in CDR encoding */
/*		TypeCode t=0;*/
		long offset=0;

		offset=stream_read_long(st,ev);

		if (ev->_major) return NULL;

		if (offset >= -4)
		{
			somPrintf("%s:%d, offset =%ld\n",__FILE__,__LINE__,offset);

			RHBOPT_ASSERT(!"cant have an offset larger than -4\n");

			RHBOPT_throw_StExcep(ev,BAD_TYPECODE,BadTypeCode,NO);

			return NULL;
		}

/*		somPrintf("Need backref %ld, pos=%ld\n",offset,*pos);

		{
			octet *op=st->_buffer+*pos-4+offset;
			somPrintf("typecode buffer should start at %p\n",
					op);
		}*/

		result=RHBCDR_create_tk_self(current_ctx,offset);

		if (result) return result;

		RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,NotImplemented,MAYBE);

		return NULL;
	}

	kind=RHBCDR_GIOPenumToIDLenum(k,ev,0);

	switch (kind)
	{
	case tk_null:
	case tk_void:
	case tk_short:
	case tk_long:
	case tk_ushort:
	case tk_ulong:
	case tk_float:
	case tk_double:
	case tk_boolean:
	case tk_char:
	case tk_octet:
	case tk_any:
	case tk_TypeCode:
	case tk_Principal:
		result=TypeCodeNew(kind);
		if (!result) throw_marshal(ev);
		break;
	case tk_struct:
		{
			CORBA_unsigned_long cnt=0;
			SOMCDR_StructMemberSeq seq={0,0,NULL};
			int i=0;
			RHBCDR_begin_read_encap(ev,st,kind)
			TCKind kind=tk_struct;

			current_ctx->rep_id=stream_read_string(st,ev);
			current_ctx->name=stream_read_string(st,ev);
			current_ctx->struct_data=&seq;

			cnt=stream_read_long(st,ev);

			seq._length=cnt;
			seq._maximum=cnt;

			if (cnt && !ev->_major)
			{
				seq._buffer=SOMCalloc(sizeof(*seq._buffer),cnt);

				if (!seq._buffer)
				{
					RHBOPT_throw_StExcep(ev,NO_MEMORY,NoMemory,MAYBE);
				}
			}
			else
			{
				seq._buffer=0;
			}

			i=0;

			while (cnt-- && !ev->_major)
			{
				seq._buffer[i].name=stream_read_string(st,ev);

				if (!ev->_major)
				{
					seq._buffer[i].type=RHBCDR_stream_read_tc(current_ctx,ev,st);
				}

				i++;
			}

			if (!ev->_major)
			{
				void *va_buf=somVaBuf_create(0,0);
				int j=0;
				char *id=current_ctx->name;
				if (id) if (!id[0]) id=NULL;
				if (!id)
				{
					id=current_ctx->rep_id;
					if (id) if (!id[0]) id="__unknown__";
				}

/*				if (current_ctx->rep_id)
				{
					somVaBuf_add(va_buf,&(current_ctx->rep_id),tk_string);
					somVaBuf_add(va_buf,&kind,tk_enum);
					kind=tk_id;
				}*/

/*				result=RHBCDR_create_struct_tc(ev,rep_id,name,RHBCDR_get_version(),&seq);*/

				somVaBuf_add(va_buf,&id,tk_string);

				while (j < i)
				{
					if ((!seq._buffer[j].name) || (!seq._buffer[j].type))
					{
						RHBOPT_throw_StExcep(ev,BAD_TYPECODE,UnexpectedNULL,MAYBE);
						break;
					}

					somVaBuf_add(va_buf,&seq._buffer[j].name,tk_string);
					somVaBuf_add(va_buf,&seq._buffer[j].type,tk_pointer);

					j++;
				}

				if (!ev->_major)
				{
					va_list va_args;

					id=NULL;

					somVaBuf_add(va_buf,&id,tk_pointer);

					somVaBuf_get_valist(va_buf,&va_args);

					result=tcNewVL(kind,va_args);

					if (result)
					{
						/* typecodes are now owned by the created typecode */
						while (j--)
						{
							seq._buffer[j].type=NULL;
						}

/*						tcPrint(result,ev);
						somPrintf("\n");*/
					}
					else
					{
						RHBOPT_throw_StExcep(ev,BAD_TYPECODE,UnexpectedNULL,MAYBE);
					}
				}

				somVaBuf_destroy(va_buf);
			}

/*	these get free'd in end of encap
			if (rep_id) SOMFree(rep_id);
			if (name) SOMFree(name);

			if (i && seq._buffer)
			{
				while (i--)
				{
					if (seq._buffer[i].name)
					{
						SOMFree(seq._buffer[i].name);
					}

					if (seq._buffer[i].type)
					{
						Environment ev2;
						SOM_InitEnvironment(&ev2);
						TypeCode_free(seq._buffer[i].type,&ev2);
						SOM_UninitEnvironment(&ev2);
					}
				}

				SOMFree(seq._buffer);
			}
*/
			RHBCDR_end_read_encap
		}
		break;
	case tk_array:
		{
			CORBA_unsigned_long ul=0;
			RHBCDR_begin_read_encap(ev,st,kind);

			current_ctx->encap_tc=RHBCDR_stream_read_tc(0,ev,st);

			if (!ev->_major)
			{
				ul=stream_read_long(st,ev);

	/*			result=RHBCDR_create_array_tc(ev,ul,of);

				TypeCode_free(of,ev);
	*/
				if (!ev->_major)
				{
					result=TypeCodeNew(tk_array,current_ctx->encap_tc,ul);

					if (result) 
					{
						current_ctx->encap_tc=NULL;
					}
					else
					{
						throw_marshal(ev);
					}
				}
			}

			RHBCDR_end_read_encap
		}
		break;
	case tk_sequence:
		{
			RHBCDR_begin_read_encap(ev,st,kind);

			current_ctx->encap_tc=RHBCDR_stream_read_tc(current_ctx,ev,st);

			if (!ev->_major)
			{
				CORBA_unsigned_long ul=stream_read_long(st,ev);

				if (!ev->_major)
				{
					result=TypeCodeNew(tk_sequence,current_ctx->encap_tc,ul);

					if (result) 
					{
						current_ctx->encap_tc=NULL;
					}
					else
					{
						throw_marshal(ev);
					}
				}
			}

			RHBCDR_end_read_encap
		}
		break;
	case tk_string:
		{
			CORBA_unsigned_long ul=stream_read_long(st,ev);

/*			result=RHBCDR_create_string_tc(ev,ul);*/

			if (!ev->_major)
			{
				result=TypeCodeNew(tk_string,ul);

				if (!result) throw_marshal(ev);
			}
		}
		break;
	case tk_enum:
		{
			SOMCDR_EnumMemberSeq seq={0,0,NULL};
			RHBOPT_volatile CORBA_unsigned_long ul=0;
			RHBOPT_volatile int i=0;
			RHBCDR_begin_read_encap(ev,st,kind);

			current_ctx->rep_id=stream_read_string(st,ev);
			current_ctx->name=stream_read_string(st,ev);
			current_ctx->enum_data=&seq;

			if (!ev->_major)
			{
				ul=stream_read_long(st,ev);

				seq._maximum=ul;
				seq._length=ul;

				seq._buffer=(corbastring *)SOMMalloc(ul*(sizeof(corbastring)));
			}

			while (ul--)
			{
				seq._buffer[i]=stream_read_string(st,ev);
				i++;
			}

/*			result=RHBCDR_create_enum_tc(ev,rep_id,name,RHBCDR_get_version(),&seq);*/

			if (!ev->_major)
			{
				void *va_buf=somVaBuf_create(0,0);
				va_list va_args;
				TCKind kind=tk_enum;
				char *p=current_ctx->name;
/*				if (current_ctx->rep_id)
				{
					somVaBuf_add(va_buf,&(current_ctx->rep_id),tk_string);
					somVaBuf_add(va_buf,&kind,tk_enum);
					kind=tk_id;
				}*/
				somVaBuf_add(va_buf,&p,tk_string);
				ul=0;
				while (ul < seq._length)
				{
					p=seq._buffer[ul];
					somVaBuf_add(va_buf,&p,tk_string);
					ul++;
				}
				p=NULL;
				somVaBuf_add(va_buf,&p,tk_string);

				somVaBuf_get_valist(va_buf,&va_args);

				result=tcNewVL(kind,va_args);

				somVaBuf_destroy(va_buf);
			}

/*			while (i--)
			{
				if (seq._buffer[i]) SOMFree(seq._buffer[i]);
			}

			if (seq._buffer) SOMFree(seq._buffer);
*/
			RHBCDR_end_read_encap
		}
		break;

	case tk_objref:
		{
			RHBCDR_begin_read_encap(ev,st,kind);

			current_ctx->rep_id=stream_read_string(st,ev);
			current_ctx->name=stream_read_string(st,ev);

			if (!ev->_major)
			{
				char *p=NULL;

/*			result=RHBCDR_create_interface_tc(ev,rep_id,name);*/
				if (current_ctx->rep_id)
				{
					if ((current_ctx->rep_id[0]==':')&&
						(current_ctx->rep_id[1]==':'))
					{
						p=current_ctx->rep_id;
					}
				}

				if (!p)
				{
					if (current_ctx->name)
					{
						if (current_ctx->name[0])
						{
							p=current_ctx->name;
						}
					}
				}

				if (!p)
				{
					p=current_ctx->rep_id;
				}

				if (!p)
				{
					throw_marshal(ev);
				}
				else
				{
/*					if (current_ctx->name && current_ctx->rep_id)
					{
						result=TypeCodeNew(tk_id,current_ctx->rep_id,
											(TCKind)tk_name,current_ctx->name,
											(TCKind)tk_objref,p);
					}
					else*/
					{
						result=TypeCodeNew(tk_objref,p);
					}

					if (!result) throw_marshal(ev);
				}
			}

/*			if (rep_id) SOMFree(rep_id);
			if (name) SOMFree(name);
*/
			RHBCDR_end_read_encap
		}
		break;

	case tk_self:
		{
			RHBCDR_begin_read_encap(ev,st,kind)
			current_ctx->name=stream_read_string(st,ev);
			if (!ev->_major)
			{
				result=tcNew(tk_self,current_ctx->name);
			}
			RHBCDR_end_read_encap
		}
		break;
	case tk_pointer:
		{
			RHBCDR_begin_read_encap(ev,st,kind);
		
			current_ctx->encap_tc=RHBCDR_stream_read_tc(current_ctx,ev,st);

			if (!ev->_major)
			{
				result=tcNew(tk_pointer,current_ctx->encap_tc);

				if (result)
				{
					current_ctx->encap_tc=NULL;
				}
				else
				{
					throw_marshal(ev);
				}
			}

			RHBCDR_end_read_encap
		}
		break;

	case tk_foreign:
		{
			long len=0;
			RHBCDR_begin_read_encap(ev,st,kind);

			current_ctx->name=stream_read_string(st,ev);
			current_ctx->rep_id=stream_read_string(st,ev);
			len=stream_read_long(st,ev);

			if (!ev->_major)
			{
				result=tcNew(tk_foreign,current_ctx->name,current_ctx->rep_id,len);
			}

			RHBCDR_end_read_encap
		}
		break;
	case tk_union:
		{
			unsigned long count=0;
			long default_used=-1;
			SOMCDR_UnionMemberSeq seq={0,0,NULL};
			RHBCDR_begin_read_encap(ev,st,kind);

			current_ctx->rep_id=stream_read_string(st,ev);
			current_ctx->name=stream_read_string(st,ev);
			current_ctx->union_data=&seq;

			current_ctx->encap_tc=RHBCDR_stream_read_tc(current_ctx,ev,st);

			default_used=stream_read_long(st,ev);

			count=stream_read_long(st,ev);

			if (count && !ev->_major)
			{
				unsigned int k=count;
				SOMCDR_UnionMember *p=SOMCalloc(count,sizeof(*p));
				int i=TypeCode_size(current_ctx->encap_tc,ev);

				seq._buffer=p;
				seq._length=count;
				seq._maximum=count;

				while (k--)
				{
					TCKind kind=TypeCode_kind(current_ctx->encap_tc,ev);

/*					RHBCDR_stream_cursor a;*/

					p->label._type=current_ctx->encap_tc;
					p->label._value=SOMMalloc(i);
					memset(p->label._value,0,i);

				/*	a._type=disc_type;
					a._value=p->label._value;
				*/

/*					RHBCDR_stream_read(impl,ev,&st->data,st->swap,&st->offset,&a);*/

					switch (kind)
					{
					case tk_octet:
					case tk_boolean:
					case tk_char:
						(*((char *)p->label._value))=stream_read_octet(st,ev);
						break;
					case tk_short:
					case tk_ushort:
						(*((short *)p->label._value))=stream_read_short(st,ev);
						break;
					case tk_long:
					case tk_ulong:
						(*((long *)p->label._value))=stream_read_long(st,ev);
						break;
					case tk_enum:
						(*((CORBA_Enum *)p->label._value))=
									RHBCDR_GIOPenumToIDLenum(stream_read_long(st,ev),
												ev,current_ctx->encap_tc);
						break;
					default:
						RHBOPT_ASSERT(!"unsupported kind in union\n");
					}

					p->name=stream_read_string(st,ev);
					p->type=RHBCDR_stream_read_tc(current_ctx,ev,st);

					p++;
				}
			}

			if (default_used >= 0)
			{
				unsigned long ul=(unsigned long)default_used;

				if (ul < count)
				{
					SOMCDR_UnionMember *p=&seq._buffer[ul];
					Environment ev2;
					octet *op;
					SOM_InitEnvironment(&ev2);
					RHBCDR_free_data(&ev2,NULL,p->label._type,p->label._value);
					SOMFree(p->label._value);
					SOM_UninitEnvironment(&ev2);
					op=SOMMalloc(sizeof(octet));
					*op=0;
					p->label._type=TC_octet;
					p->label._value=op;
				}
			}

			result=RHBCDR_create_union_tc(
					ev,
					current_ctx->rep_id,
					current_ctx->name,
					NULL,
					current_ctx->encap_tc,
					&seq);

/*			if (seq._buffer)
			{
				unsigned int k=seq._length;
				CORBA_UnionMember *p=seq._buffer;

				while (k--)
				{
					if (p->label._value)
					{
						Environment ev2;
						SOM_InitEnvironment(&ev2);
						RHBCDR_free_data(&ev2,p->label._type,p->label._value);
						SOMFree(p->label._value);
						p->label._value=0;
						SOM_UninitEnvironment(&ev2);
					}

					if (p->name)
					{
						SOMFree(p->name);
						p->name=0;
					}

					if (p->type)
					{
						Environment ev2;
						SOM_InitEnvironment(&ev2);
						TypeCode_free(p->type,&ev2);
						SOM_UninitEnvironment(&ev2);
						p->type=0;
					}

					p++;
				}

				SOMFree(seq._buffer);
			}*/

/*			if (rep_id) SOMFree(rep_id);
			if (name) SOMFree(name);
*/
/*			if (disc_type)
			{
				Environment ev2;
				SOM_InitEnvironment(&ev2);
				TypeCode_free(disc_type,&ev2);
				SOM_UninitEnvironment(&ev2);
			}*/

			RHBCDR_end_read_encap
		}
		break;
	default:
		{
			somPrintf("can't decode TypeCode(%ld)\n",(long)RHBCDR_IDLenumToGIOPenum(kind,ev,0));

			RHBOPT_ASSERT(!"RHBCDR_stream_read_tc");
		}
	}

	if (!result) 
	{
		RHBOPT_throw_StExcep(ev,MARSHAL,BadTypeCode,MAYBE);
	}

	return result;
}

SOM_Scope void SOMLINK cdristr_stream_init(
			SOMCDR_CDRInputStream SOMSTAR somSelf,
			Environment *ev,
			SOMCDR_unmarshal_filter *filter,
			SOMCDR_CDRInputStream_StreamState *state,
			boolean do_free)
{
	SOMCDR_CDRInputStreamData *somThis=SOMCDR_CDRInputStreamGetData(somSelf);

	somThis->filter=filter;

	if (state)
	{
		somThis->state=*state;
	}
	else
	{
		memset(&somThis->state,0,sizeof(somThis->state));
	}

	somThis->cleanup_flag=do_free;
	SOM_IgnoreWarning(ev);
}

#ifdef SOMCDR_CDRInputStream_BeginInitializer_somDefaultInit
SOM_Scope void SOMLINK cdristr_somDefaultInit(
	SOMCDR_CDRInputStream SOMSTAR somSelf,
	somInitCtrl *ctrl)
{
	somInitCtrl globalCtrl;
	somBooleanVector myMask;
	SOMCDR_CDRInputStreamData *somThis;
	SOMCDR_CDRInputStream_BeginInitializer_somDefaultInit
	SOMCDR_CDRInputStream_Init_CORBA_DataInputStream_somDefaultInit(somSelf,ctrl);
	RHBOPT_unused(somThis);
}
#else
SOM_Scope void SOMLINK cdristr_somInit(
	SOMCDR_CDRInputStream SOMSTAR somSelf)
{
	SOMCDR_CDRInputStream_parent_CORBA_DataInputStream_somInit(somSelf);
	SOM_IgnoreWarning(somSelf);
}
#endif

SOM_Scope void SOMLINK cdristr_somUninit(
	SOMCDR_CDRInputStream SOMSTAR somSelf)
{
	SOMCDR_CDRInputStreamData *somThis=SOMCDR_CDRInputStreamGetData(somSelf);
	if (somThis->state.stream.stream_data) 
	{
/*		somPrintf("leftover input stream buffer %p\n",
			somThis->state.data._buffer);*/
		if (somThis->cleanup_flag)
		{
			Environment ev;
			SOM_InitEnvironment(&ev);
			somThis->state.stream.stream_io(
				somThis->state.stream.stream_data,
				&ev,
				NULL,
				0);
			SOM_UninitEnvironment(&ev);
		}
		somThis->state.stream.stream_data=NULL;
		somThis->state.stream.stream_io=NULL;
	}
	somThis->cleanup_flag=0;
	SOMCDR_CDRInputStream_parent_CORBA_DataInputStream_somUninit(somSelf);
}

SOM_Scope void SOMLINK cdristr_read_float_array(
	SOMCDR_CDRInputStream SOMSTAR somSelf,
	Environment *ev,
	CORBA_FloatSeq *seq,
	unsigned long offset,
	unsigned long length)
{
	ARRAY_BOUNDS_CHECK(ev,seq,offset,length);
	while (length-- && !ev->_major)
	{
		seq->_buffer[offset++]=CORBA_DataInputStream_read_float(somSelf,ev);
	}
}

SOM_Scope void SOMLINK cdristr_read_double_array(
	SOMCDR_CDRInputStream SOMSTAR somSelf,
	Environment *ev,
	CORBA_DoubleSeq *seq,
	unsigned long offset,
	unsigned long length)
{
	ARRAY_BOUNDS_CHECK(ev,seq,offset,length);
	while (length-- && !ev->_major)
	{
		seq->_buffer[offset++]=CORBA_DataInputStream_read_double(somSelf,ev);
	}
}

SOM_Scope void SOMLINK cdristr_read_boolean_array(
	SOMCDR_CDRInputStream SOMSTAR somSelf,
	Environment *ev,
	CORBA_BooleanSeq *seq,
	unsigned long offset,
	unsigned long length)
{
	SOMCDR_CDRInputStreamData *somThis=SOMCDR_CDRInputStreamGetData(somSelf);

	ARRAY_BOUNDS_CHECK(ev,seq,offset,length);

/*	if (data_remaining(&somThis->state) < length)
	{
		throw_marshal(ev); return; 
	}
*/
	stream_read(&somThis->state,ev,&seq->_buffer[offset],length);
}

SOM_Scope void SOMLINK cdristr_read_char_array(
	SOMCDR_CDRInputStream SOMSTAR somSelf,
	Environment *ev,
	CORBA_CharSeq *seq,
	unsigned long offset,
	unsigned long length)
{
	SOMCDR_CDRInputStreamData *somThis=SOMCDR_CDRInputStreamGetData(somSelf);

	ARRAY_BOUNDS_CHECK(ev,seq,offset,length);

/*	if (data_remaining(&somThis->state) < length)
	{
		throw_marshal(ev); return; 
	}
*/
	stream_read(&somThis->state,ev,&seq->_buffer[offset],length);
}

SOM_Scope void SOMLINK cdristr_read_ushort_array(
	SOMCDR_CDRInputStream SOMSTAR somSelf,
	Environment *ev,
	CORBA_UShortSeq *seq,
	unsigned long offset,
	unsigned long length)
{
	ARRAY_BOUNDS_CHECK(ev,seq,offset,length);
	while (length-- && !ev->_major)
	{
		seq->_buffer[offset++]=CORBA_DataInputStream_read_ushort(somSelf,ev);
	}
}

SOM_Scope void SOMLINK cdristr_read_short_array(
	SOMCDR_CDRInputStream SOMSTAR somSelf,
	Environment *ev,
	CORBA_ShortSeq *seq,
	unsigned long offset,
	unsigned long length)
{
	ARRAY_BOUNDS_CHECK(ev,seq,offset,length);
	while (length-- && !ev->_major)
	{
		seq->_buffer[offset++]=CORBA_DataInputStream_read_short(somSelf,ev);
	}
}

SOM_Scope void SOMLINK cdristr_read_long_array(
	SOMCDR_CDRInputStream SOMSTAR somSelf,
	Environment *ev,
	CORBA_LongSeq *seq,
	unsigned long offset,
	unsigned long length)
{
	ARRAY_BOUNDS_CHECK(ev,seq,offset,length);
	while (length-- && !ev->_major)
	{
		seq->_buffer[offset++]=CORBA_DataInputStream_read_long(somSelf,ev);
	}
}

SOM_Scope void SOMLINK cdristr_read_ulong_array(
	SOMCDR_CDRInputStream SOMSTAR somSelf,
	Environment *ev,
	CORBA_ULongSeq *seq,
	unsigned long offset,
	unsigned long length)
{
	ARRAY_BOUNDS_CHECK(ev,seq,offset,length);
	while (length-- && !ev->_major)
	{
		seq->_buffer[offset++]=CORBA_DataInputStream_read_ulong(somSelf,ev);
	}
}

SOM_Scope void SOMLINK cdristr_read_octet_array(
	SOMCDR_CDRInputStream SOMSTAR somSelf,
	Environment *ev,
	CORBA_OctetSeq *seq,
	unsigned long offset,
	unsigned long length)
{
	SOMCDR_CDRInputStreamData *somThis=SOMCDR_CDRInputStreamGetData(somSelf);

	ARRAY_BOUNDS_CHECK(ev,seq,offset,length);

/*	if (data_remaining(&somThis->state) < length)
	{
		throw_marshal(ev); return; 
	}
*/
	stream_read(&somThis->state,ev,&seq->_buffer[offset],length);
}

SOM_Scope void SOMLINK cdristr_read_any_array(
	SOMCDR_CDRInputStream SOMSTAR somSelf,
	Environment *ev,
	CORBA_AnySeq *seq,
	unsigned long offset,
	unsigned long length)
{
	ARRAY_BOUNDS_CHECK(ev,seq,offset,length);
	while (length-- && !ev->_major)
	{
		seq->_buffer[offset++]=CORBA_DataInputStream_read_any(somSelf,ev);
	}
}

SOM_Scope boolean SOMLINK cdristr_read_boolean(
	SOMCDR_CDRInputStream SOMSTAR somSelf,
	Environment *ev)
{
	SOMCDR_CDRInputStreamData *somThis=SOMCDR_CDRInputStreamGetData(somSelf);

	return stream_read_octet(&somThis->state,ev);
}

SOM_Scope octet SOMLINK cdristr_read_octet(
	SOMCDR_CDRInputStream SOMSTAR somSelf,
	Environment *ev)
{
	SOMCDR_CDRInputStreamData *somThis=SOMCDR_CDRInputStreamGetData(somSelf);

	return stream_read_octet(&somThis->state,ev);
}

SOM_Scope char SOMLINK cdristr_read_char(
	SOMCDR_CDRInputStream SOMSTAR somSelf,
	Environment *ev)
{
	SOMCDR_CDRInputStreamData *somThis=SOMCDR_CDRInputStreamGetData(somSelf);

	return stream_read_octet(&somThis->state,ev);
}

SOM_Scope unsigned long SOMLINK cdristr_read_ulong(
	SOMCDR_CDRInputStream SOMSTAR somSelf,
	Environment *ev)
{
	SOMCDR_CDRInputStreamData *somThis=SOMCDR_CDRInputStreamGetData(somSelf);

	return stream_read_long(&somThis->state,ev);
}

SOM_Scope long SOMLINK cdristr_read_long(
	SOMCDR_CDRInputStream SOMSTAR somSelf,
	Environment *ev)
{
	SOMCDR_CDRInputStreamData *somThis=SOMCDR_CDRInputStreamGetData(somSelf);

	return stream_read_long(&somThis->state,ev);
}

SOM_Scope double SOMLINK cdristr_read_double(
	SOMCDR_CDRInputStream SOMSTAR somSelf,
	Environment *ev)
{
	SOMCDR_CDRInputStreamData *somThis=SOMCDR_CDRInputStreamGetData(somSelf);
	double __result=0.0;

	if (ev->_major) return __result;

	if (sizeof(__result)!=8)
	{
		throw_marshal(ev);

		return __result;
	}

	stream_read_align(&somThis->state,ev,8);
	stream_read(&somThis->state,ev,&__result,8);

	if (somThis->state.swap) RHBCDR_swap_bytes(&__result,sizeof(__result));

	return __result;
}

SOM_Scope float SOMLINK cdristr_read_float(
	SOMCDR_CDRInputStream SOMSTAR somSelf,
	Environment *ev)
{
	SOMCDR_CDRInputStreamData *somThis=SOMCDR_CDRInputStreamGetData(somSelf);
	float __result=(float)0;

	if (ev->_major) return __result;

	if (sizeof(__result)!=4)
	{
		throw_marshal(ev);

		return __result;
	}

	stream_read_align(&somThis->state,ev,4);
	stream_read(&somThis->state,ev,&__result,4);

	if (somThis->state.swap) RHBCDR_swap_bytes(&__result,sizeof(__result));

	return __result;
}

SOM_Scope char * SOMLINK cdristr_read_string(
	SOMCDR_CDRInputStream SOMSTAR somSelf,
	Environment *ev)
{
	SOMCDR_CDRInputStreamData *somThis=SOMCDR_CDRInputStreamGetData(somSelf);

	return stream_read_string(&somThis->state,ev);
}

SOM_Scope short SOMLINK cdristr_read_short(
	SOMCDR_CDRInputStream SOMSTAR somSelf,
	Environment *ev)
{
	SOMCDR_CDRInputStreamData *somThis=SOMCDR_CDRInputStreamGetData(somSelf);

	return stream_read_short(&somThis->state,ev);
}

SOM_Scope unsigned short SOMLINK cdristr_read_ushort(
	SOMCDR_CDRInputStream SOMSTAR somSelf,
	Environment *ev)
{
	SOMCDR_CDRInputStreamData *somThis=SOMCDR_CDRInputStreamGetData(somSelf);

	return stream_read_ushort(&somThis->state,ev);
}

SOM_Scope TypeCode SOMLINK cdristr_read_TypeCode(
	SOMCDR_CDRInputStream SOMSTAR somSelf,
	Environment *ev)
{
	SOMCDR_CDRInputStreamData *somThis=SOMCDR_CDRInputStreamGetData(somSelf);

	if (ev->_major) return NULL;

	return RHBCDR_stream_read_tc(NULL,ev,&somThis->state);
}

SOM_Scope any SOMLINK cdristr_read_any(
	SOMCDR_CDRInputStream SOMSTAR somSelf,
	Environment *ev)
{
	SOMCDR_CDRInputStreamData *somThis=SOMCDR_CDRInputStreamGetData(somSelf);
	any a={NULL,NULL};

	RHBCDR_unmarshal(somThis->filter,ev,&a,TC_any,somSelf);

	return a;
}

SOM_Scope SOMObject SOMSTAR SOMLINK cdristr_read_Object(
	SOMCDR_CDRInputStream SOMSTAR somSelf,
	Environment *ev)
{
	SOM_IgnoreWarning(somSelf);
	throw_marshal(ev);
	return NULL;
}

SOM_Scope void SOMLINK cdristr_read_align(SOMCDR_CDRInputStream SOMSTAR somSelf,Environment *ev,unsigned long align)
{
	SOMCDR_CDRInputStreamData *somThis=SOMCDR_CDRInputStreamGetData(somSelf);

	stream_read_align(&somThis->state,ev,(short)align);
}

SOM_Scope void SOMLINK cdristr_somDestruct(
		SOMCDR_CDRInputStream somSelf,
		boolean doFree,
		somDestructCtrl *ctrl)
{
	somDestructCtrl globalCtrl;
	somBooleanVector myMask;
	SOMCDR_CDRInputStreamData *somThis;

	SOMCDR_CDRInputStream_BeginDestructor

	RHBOPT_unused(somThis);
	cdristr_somUninit(somSelf);

	SOMCDR_CDRInputStream_EndDestructor
}

SOM_Scope void SOMLINK cdrostr_somDestruct(
		SOMCDR_CDROutputStream somSelf,
		boolean doFree,
		somDestructCtrl *ctrl)
{
	somDestructCtrl globalCtrl;
	somBooleanVector myMask;
	SOMCDR_CDROutputStreamData *somThis;

	SOMCDR_CDROutputStream_BeginDestructor

	RHBOPT_unused(somThis);
	cdrostr_somUninit(somSelf);

	SOMCDR_CDROutputStream_EndDestructor
}
