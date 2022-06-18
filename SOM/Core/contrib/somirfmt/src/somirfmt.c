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

#ifdef _PLATFORM_WIN32_
#include <windows.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_FCNTL_H
#include <fcntl.h>
#endif

#include <rhbsomir.h>
#include <somtcint.h>

#define DUPL_EMPTY_SEQUENCE

static char root_repository_id[]="::";

static struct SOMIR_ContainedData *container_Acquire(
		struct SOMIR_ContainerData *defined_in,
		long rootOffset,
		long type);

static long read_long(struct SOMIR_read_data *data)
{
	octet buf[4];
	size_t n=data->stream->lpVtbl->read(data->stream,buf,sizeof(buf));

	return (n==sizeof(buf)) ? (long)(
			(buf[0]<<24)|
			(buf[1]<<16)|
			(buf[2]<<8)|
			(buf[3]) ) : -1L;
}

static char *read_string(struct SOMIR_read_data *data)
{
	long len=data->lpVtbl->read_long(data);
	char *p=NULL;

	if (!++len)
	{
		data->stream->lpVtbl->read(data->stream,NULL,1);
	}
	else
	{
		p=data->allocator->lpVtbl->alloc(data->allocator,len,1);
		data->stream->lpVtbl->read(data->stream,p,len);
	}

	return p;
}

static TypeCode somir_get_constant_base_tc(TCKind kind)
{
	TypeCode tc=NULL;

	switch (kind)
	{
	case tk_void: tc=TC_void; break;
	case tk_null: tc=TC_null; break;
	case tk_short: tc=TC_short; break;
	case tk_ushort: tc=TC_ushort; break;
	case tk_long: tc=TC_long; break;
	case tk_ulong: tc=TC_ulong; break;
	case tk_octet: tc=TC_octet; break;
	case tk_char: tc=TC_char; break;
	case tk_boolean: tc=TC_boolean; break;
	case tk_double: tc=TC_double; break;
	case tk_float: tc=TC_float; break;
	case tk_Principal: tc=TC_Principal; break;
	case tk_TypeCode: tc=TC_TypeCode; break;
	case tk_any: tc=TC_any; break;
		break;
	}

	return tc;
}

static TypeCode read_TypeCode(struct SOMIR_read_data *data)
{
	long kind=data->lpVtbl->read_long(data);
	TypeCode tc=NULL;

	if (kind!=-1)
	{
		long n2=data->lpVtbl->read_long(data);
#ifdef _DEBUG
		long n3=
#endif
			data->lpVtbl->read_long(data);
#ifdef _DEBUG
		long n4=
#endif
			data->lpVtbl->read_long(data);

		RHBOPT_ASSERT(!n2)
		RHBOPT_ASSERT(n3==SOMTC_VERSION_1)
		RHBOPT_ASSERT(!n4)

		tc=somir_get_constant_base_tc(kind);

#if 0
		if (tc)
		{
			/* either use constant, or allocate a simple copy */

			struct TypeCode_base *tb=data->allocator->lpVtbl->alloc(data->allocator,sizeof(*tb),
						alignment_of(struct TypeCode_base));

			if (tb)
			{
				*tb=tc->any;
			}

			tc=(somToken)tb;
		}
		else
#else
		/* reuse the constants from somtc.dll */
		if (!tc)
#endif
		{
			switch (kind)
			{
			case tk_string:
				{
					struct TypeCode_string *tc2=data->allocator->lpVtbl->alloc(data->allocator,
							sizeof(*tc2),alignment_of(struct TypeCode_string));
					long maxBound=data->lpVtbl->read_long(data);

					if (tc2)
					{
						tc2->align=alignment_of(void *);
						tc2->maxBound=maxBound;
					}

					tc=(somToken)tc2;
				}
				break;
			case tk_objref:
				{
					struct TypeCode_objref *tc2=data->allocator->lpVtbl->alloc(data->allocator,
							sizeof(*tc2),alignment_of(struct TypeCode_objref));
					char *interfaceId=data->lpVtbl->read_string(data);

					if (tc2)
					{
						tc2->align=alignment_of(void *);
						tc2->interfaceId=interfaceId;
					}

					tc=(somToken)tc2;
				}
				break;
			case tk_foreign:
				{
					struct TypeCode_foreign *tc2=data->allocator->lpVtbl->alloc(data->allocator,
							sizeof(*tc2),alignment_of(struct TypeCode_foreign));
					char *typeName=data->lpVtbl->read_string(data);
					char *context=data->lpVtbl->read_string(data);
					long length=data->lpVtbl->read_long(data);

					if (tc2)
					{
						tc2->align=alignment_of(void *);
						tc2->typeName=typeName;
						tc2->context=context;
						tc2->length=length;
					}

					tc=(somToken)tc2;
				}
				break;
			case tk_pointer:
				{
					struct TypeCode_pointer *tc2=data->allocator->lpVtbl->alloc(data->allocator,
							sizeof(*tc2),alignment_of(struct TypeCode_pointer));

					tc=data->lpVtbl->read_TypeCode(data);

					if (tc2)
					{
						tc2->align=alignment_of(void *);
						tc2->tc=tc;
					}

					tc=(somToken)tc2;
				}
				break;
			case tk_sequence:
				{
					struct TypeCode_sequence *tc2=data->allocator->lpVtbl->alloc(data->allocator,
							sizeof(*tc2),alignment_of(struct TypeCode_sequence));

					tc=data->lpVtbl->read_TypeCode(data);
					n2=data->lpVtbl->read_long(data);

					if (tc2)
					{
						tc2->align=alignment_of(GENERIC_SEQUENCE);
						tc2->tc=tc;
						tc2->maxBound=n2;
					}

					tc=(somToken)tc2;
				}
				break;
			case tk_array:
				{
					struct TypeCode_array *tc2=data->allocator->lpVtbl->alloc(data->allocator,
							sizeof(*tc2),alignment_of(struct TypeCode_array));

					tc=data->lpVtbl->read_TypeCode(data);
					n2=data->lpVtbl->read_long(data);

					if (tc2)
					{
						tc2->align=(short)(tc ? (((struct TypeCode_base *)tc)->align) : 0);
						tc2->tc=tc;
						tc2->maxBound=n2;
					}

					tc=(somToken)tc2;
				}
				break;
			case tk_enum:
				{
					struct TypeCode_enum *tc2=data->allocator->lpVtbl->alloc(data->allocator,
							sizeof(*tc2),alignment_of(struct TypeCode_enum));
					char *enumName=data->lpVtbl->read_string(data);
					long rptCount=data->lpVtbl->read_long(data);
					char **ids=rptCount ? 
							data->allocator->lpVtbl->alloc(data->allocator,
								sizeof(char *)*rptCount,
								alignment_of(char *)) : NULL;
					long i=0;

					if (tc2)
					{
						tc2->align=alignment_of(TCKind);
						tc2->rptCount=rptCount;
						tc2->enumName=enumName;
						tc2->enumId=ids;
					}

					while (i < rptCount)
					{
						char *enumId=data->lpVtbl->read_string(data);

						if (ids)
						{
							ids[i]=enumId;
						}

						i++;
					}

					tc=(somToken)tc2;
				}
				break;
			case tk_self:
				{
					struct TypeCode_self *tc2=data->allocator->lpVtbl->alloc(data->allocator,
							sizeof(*tc2),alignment_of(struct TypeCode_self));
					char *selfName=data->lpVtbl->read_string(data);

					if (tc2)
					{
						tc2->align=0;
						tc2->selfName=selfName;
					}

					tc=(somToken)tc2;
				}
				break;
			case tk_struct:
				{
					struct TypeCode_struct *tc2=data->allocator->lpVtbl->alloc(data->allocator,
							sizeof(*tc2),alignment_of(struct TypeCode_struct));
					char *structName=data->lpVtbl->read_string(data);
					long rptCount=data->lpVtbl->read_long(data);
					struct TypeCode_struct_member *mbrs=rptCount ?
							data->allocator->lpVtbl->alloc(data->allocator,
							sizeof(*mbrs)*rptCount,alignment_of(struct TypeCode_struct_member))
							: NULL;
					long i=0;
					short align=0;

					if (tc2)
					{
						tc2->rptCount=rptCount;
						tc2->structName=structName;
						tc2->mbrs=mbrs;
					}

					while (i < rptCount)
					{
						char *mbrName=data->lpVtbl->read_string(data);
						TypeCode mbrTc=data->lpVtbl->read_TypeCode(data);

						if (mbrs)
						{
							if (mbrTc)
							{
								if (TypeCode_peek(mbrTc)->align > align)
								{
									align=TypeCode_peek(mbrTc)->align;
								}
							}

							mbrs[i].mbrName=mbrName;
							mbrs[i].mbrTc=mbrTc;
						}

						i++;
					}

					if (tc2)
					{
						tc2->align=align;
					}

					tc=(somToken)tc2;
				}
				break;
			case tk_union:
				{
					struct TypeCode_union *tc2=data->allocator->lpVtbl->alloc(data->allocator,
							sizeof(*tc2),alignment_of(struct TypeCode_union));
					char *unionName=data->lpVtbl->read_string(data);
					TypeCode swTc=data->lpVtbl->read_TypeCode(data);
					long rptCount=data->lpVtbl->read_long(data);
					struct TypeCode_union_member *mbrs=rptCount ?
							data->allocator->lpVtbl->alloc(data->allocator,
							sizeof(*mbrs)*rptCount,alignment_of(struct TypeCode_union_member))
							: NULL;
					long i=0;
					short align=(short)(swTc ? TypeCode_peek(swTc)->align : 0);

					if (tc2)
					{
						tc2->rptCount=rptCount;
						tc2->unionName=unionName;
						tc2->swTc=swTc;
						tc2->mbrs=mbrs;
					}

					while (i < rptCount)
					{
						struct TypeCode_union_member mbr={0,{0},NULL,NULL};

						mbr.labelFlag=data->lpVtbl->read_long(data);

						switch (mbr.labelFlag)
						{
						case TCREGULAR_CASE:
							{
								long value=data->lpVtbl->read_long(data);

								if (swTc)
								{
									switch (((struct TypeCode_base *)swTc)->tag)
									{
									case tk_octet:
									case tk_char:
									case tk_boolean:
										mbr.labelValue.c=(char)value;
										break;
									case tk_short:
									case tk_ushort:
										mbr.labelValue.s=(unsigned short)value;
										break;
										break;
									default:
										mbr.labelValue.l=(unsigned long)value;
										break;
									}
								}
							}
							break;
						case TCDEFAULT_CASE:
							break;
						default:
							RHBOPT_ASSERT(!tc2)
							RHBOPT_ASSERT(tc2)
							break;
						}

						mbr.mbrName=data->lpVtbl->read_string(data);
						mbr.mbrTc=data->lpVtbl->read_TypeCode(data);

						if (mbr.mbrTc)
						{
							if (align < ((struct TypeCode_base *)mbr.mbrTc)->align)
							{
								align=((struct TypeCode_base *)mbr.mbrTc)->align;						
							}
						}

						if (mbrs)
						{
							mbrs[i]=mbr;
						}

						i++;
					}

					if (tc2)
					{
						tc2->align=align;
					}

					tc=(somToken)tc2;
				}
				break;
			default:
				RHBOPT_ASSERT(kind);
				RHBOPT_ASSERT(!kind);
				break;
			}

			if (tc)
			{
				TypeCode_peek(tc)->tag=kind;
				TypeCode_peek(tc)->version=SOMTC_VERSION_1;
				TypeCode_peek(tc)->flags=0;
			}
		}
	}

	return (TypeCode)tc;
}

static _IDL_SEQUENCE_string read_strings(struct SOMIR_read_data *data)
{
	_IDL_SEQUENCE_string seq={0,0,NULL};

	seq._maximum=data->lpVtbl->read_long(data);

	if (seq._maximum)
	{
		seq._buffer=data->allocator->lpVtbl->alloc(data->allocator,
					seq._maximum*sizeof(seq._buffer[0]),
					alignment_of(char *));

		while (seq._length < seq._maximum)
		{
			char *p=data->lpVtbl->read_string(data);

			if (seq._buffer)
			{
				seq._buffer[seq._length]=p;
			}

			seq._length++;
		}
	}

	return seq;
}

static char *somir_dupl_string(char *str,struct SOMIR_alloc *alloc)
{
	char *retVal=NULL;

	if (str)
	{
		size_t len=strlen(str)+1;
		retVal=alloc->lpVtbl->alloc(alloc,len,1);
		if (retVal)
		{
			memcpy(retVal,str,len);
		}
	}

	return retVal;
}

static TypeCode SOMIR_dupl_TypeCode(TypeCode tcIn,Environment *ev,struct SOMIR_alloc *alloc)
{
	/*******************************************************
	 *
	 * linearise a real TypeCode into a single block of memory
	 *
	 */

	TypeCode tc=NULL;
	
	if (ev->_major==NO_EXCEPTION)
	{
		if (tcIn)
		{
			TCKind kind=TypeCode_kind(tcIn,ev); 

			if (ev->_major==NO_EXCEPTION)
			{
				unsigned char flags=0;

				tc=somir_get_constant_base_tc(kind);

				if (tc)
				{
					struct TypeCode_base *tc2=alloc->lpVtbl->alloc(alloc,sizeof(tc2[0]),
									alignment_of(struct TypeCode_base));

					RHBOPT_ASSERT(kind != tk_string);

					if (tc2)
					{
						tc2->tag=kind;
						tc2->flags=0;
						tc2->version=SOMTC_VERSION_1;
						tc2->align=TypeCode_alignment(tc,ev);
					}

					tc=(somToken)tc2;
				}
				else
				{
					switch (kind)
					{
					case tk_string:
						{
							struct TypeCode_string *tc2=alloc->lpVtbl->alloc(alloc,sizeof(tc2[0]),
									alignment_of(struct TypeCode_string));

							if (tc2)
							{
								any a=TypeCode_parameter(tcIn,ev,0);
								long n=*(long *)a._value;

								RHBOPT_ASSERT(TypeCode_equal(a._type,ev,TC_long));

								tc2->align=alignment_of(char *);
								tc2->maxBound=n;

								tc=(somToken)tc2;
							}
						}
						break;
					case tk_objref:
						{
							any name=TypeCode_parameter(tcIn,ev,0);
							struct TypeCode_objref *tc2=alloc->lpVtbl->alloc(alloc,sizeof(tc2[0]),
									alignment_of(struct TypeCode_objref));
							char *interfaceId=somir_dupl_string(*(char **)name._value,alloc);

							RHBOPT_ASSERT(TypeCode_equal(name._type,ev,TC_string));

							if (tc2)
							{
								tc2->align=alignment_of(SOMObject SOMSTAR);
								tc2->interfaceId=interfaceId;

								tc=(somToken)tc2;
							}
						}
						break;
					case tk_struct:
						{
							long count=TypeCode_param_count(tcIn,ev)>>1;
							long index=0;
							any name=TypeCode_parameter(tcIn,ev,index++);
							struct TypeCode_struct *tc2=alloc->lpVtbl->alloc(alloc,sizeof(tc2[0]),
									alignment_of(struct TypeCode_struct));
							struct TypeCode_struct_member *mbrs=(count > 0) ? alloc->lpVtbl->alloc(alloc,
								count*sizeof(*mbrs),alignment_of(struct TypeCode_struct_member)) : NULL;
							char *structName=somir_dupl_string(*(char **)name._value,alloc);
							long i=0;
							short align=0;

							RHBOPT_ASSERT(TypeCode_equal(name._type,ev,TC_string));

							while (i < count)
							{
								any id=TypeCode_parameter(tcIn,ev,index++);
								any type=TypeCode_parameter(tcIn,ev,index++);
								char *mbrName=somir_dupl_string(*(char **)id._value,alloc);
								TypeCode mbrTc=SOMIR_dupl_TypeCode(*(TypeCode *)type._value,ev,alloc);

								RHBOPT_ASSERT(TypeCode_equal(id._type,ev,TC_string));
								RHBOPT_ASSERT(TypeCode_equal(type._type,ev,TC_TypeCode));

								if (mbrs)
								{
									short al=TypeCode_alignment(mbrTc,ev);
									if (align < al) align=al;
									mbrs[i].mbrName=mbrName;
									mbrs[i].mbrTc=mbrTc;
								}

								i++;
							}

							if (tc2)
							{
								tc2->align=align;
								tc2->structName=structName;
								tc2->rptCount=count;
								tc2->mbrs=mbrs;

								tc=(TypeCode)tc2;
							}
						}
						break;
					case tk_union:
						{
							long count=(TypeCode_param_count(tcIn,ev)-2)/3;
							long index=0;
							any name=TypeCode_parameter(tcIn,ev,index++);
							any switchType=TypeCode_parameter(tcIn,ev,index++);
							struct TypeCode_union *tc2=alloc->lpVtbl->alloc(alloc,sizeof(tc2[0]),
									alignment_of(struct TypeCode_union));
							struct TypeCode_union_member *mbrs=(count > 0) ? alloc->lpVtbl->alloc(alloc,
								count*sizeof(*mbrs),alignment_of(struct TypeCode_union_member)) : NULL;
							char *unionName=somir_dupl_string(*(char **)name._value,alloc);
							long i=0;
							short align=TypeCode_alignment(*(TypeCode *)switchType._value,ev);
							TypeCode swTc=SOMIR_dupl_TypeCode(*(TypeCode *)switchType._value,ev,alloc);

							RHBOPT_ASSERT(TypeCode_equal(name._type,ev,TC_string));
							RHBOPT_ASSERT(TypeCode_equal(switchType._type,ev,TC_TypeCode));

							while (i < count)
							{
								any flag=TypeCode_parameter(tcIn,ev,index++);
								any id=TypeCode_parameter(tcIn,ev,index++);
								any type=TypeCode_parameter(tcIn,ev,index++);
								char *mbrName=somir_dupl_string(*(char **)id._value,alloc);
								TypeCode mbrTc=SOMIR_dupl_TypeCode(*(TypeCode *)type._value,ev,alloc);

								RHBOPT_ASSERT(TypeCode_equal(id._type,ev,TC_string));
								RHBOPT_ASSERT(TypeCode_equal(type._type,ev,TC_TypeCode));

								if (mbrs)
								{
									short al=TypeCode_alignment(mbrTc,ev);
									if (align < al) align=al;
									mbrs[i].mbrName=mbrName;
									mbrs[i].mbrTc=mbrTc;

									switch (TypeCode_kind(flag._type,ev))
									{
									case tk_octet:
										mbrs[i].labelFlag=TCDEFAULT_CASE;
										mbrs[i].labelValue.c=*(char *)flag._value;
										break;
									case tk_char:
									case tk_boolean:
										mbrs[i].labelFlag=TCREGULAR_CASE;
										mbrs[i].labelValue.c=*(char *)flag._value;
										break;
									case tk_short:
									case tk_ushort:
										mbrs[i].labelFlag=TCREGULAR_CASE;
										mbrs[i].labelValue.s=*(short *)flag._value;
										break;
									case tk_long:
									case tk_ulong:
										mbrs[i].labelFlag=TCREGULAR_CASE;
										mbrs[i].labelValue.l=*(long *)flag._value;
										break;
									case tk_enum:
										mbrs[i].labelFlag=TCREGULAR_CASE;
										mbrs[i].labelValue.l=*(TCKind *)flag._value;
										break;
									default:
										RHBOPT_ASSERT(tc);
										break;
									}
								}

								i++;
							}

							if (tc2)
							{
								tc2->align=align;
								tc2->unionName=unionName;
								tc2->rptCount=count;
								tc2->mbrs=mbrs;
								tc2->swTc=swTc;

								tc=(TypeCode)tc2;
							}
						}
						break;
					case tk_enum:
						{
							long count=TypeCode_param_count(tcIn,ev)-1;
							any name=TypeCode_parameter(tcIn,ev,0);
							struct TypeCode_enum *tc2=alloc->lpVtbl->alloc(alloc,sizeof(tc2[0]),
									alignment_of(struct TypeCode_enum));
							char **enumId=(count > 0) ? alloc->lpVtbl->alloc(alloc,
								count*sizeof(*enumId),alignment_of(char *)) : NULL;
							char *enumName=somir_dupl_string(*(char **)name._value,alloc);
							long i=0;

							RHBOPT_ASSERT(TypeCode_equal(name._type,ev,TC_string));

							while (i < count)
							{
								any id=TypeCode_parameter(tcIn,ev,1+i);
								char *p=somir_dupl_string(*(char **)id._value,alloc);

								RHBOPT_ASSERT(TypeCode_equal(id._type,ev,TC_string));

								if (enumId)
								{
									enumId[i]=p;
								}

								i++;
							}

							if (tc2)
							{
								tc2->align=alignment_of(TCKind);
								tc2->enumName=enumName;
								tc2->rptCount=count;
								tc2->enumId=enumId;

								if (tcGetZeroOriginEnum(tcIn,ev))
								{
									flags|=SOMTC_FLAGS_ZENUM;
								}

								tc=(somToken)tc2;
							}
						}
						break;
					case tk_foreign:
						{
							struct TypeCode_foreign *tc2=alloc->lpVtbl->alloc(alloc,sizeof(tc2[0]),
									alignment_of(struct TypeCode_foreign));
							any a=TypeCode_parameter(tcIn,ev,0);
							any b=TypeCode_parameter(tcIn,ev,1);
							any c=TypeCode_parameter(tcIn,ev,2);

							RHBOPT_ASSERT(TypeCode_equal(a._type,ev,TC_string));
							RHBOPT_ASSERT(TypeCode_equal(b._type,ev,TC_string));
							RHBOPT_ASSERT(TypeCode_equal(c._type,ev,TC_long));

							if (ev->_major==NO_EXCEPTION)
							{
								char *typeName=somir_dupl_string(*(char **)a._value,alloc);
								char *context=somir_dupl_string(*(char **)b._value,alloc);
								long len=*(long *)c._value;

								if (tc2)
								{
									tc2->align=TypeCode_alignment(tcIn,ev);
									tc2->typeName=typeName;
									tc2->context=context;
									tc2->length=len;

									tc=(somToken)tc2;
								}
							}
						}
						break;
					case tk_self:
						{
							any name=TypeCode_parameter(tcIn,ev,0);
							struct TypeCode_self *tc2=alloc->lpVtbl->alloc(alloc,sizeof(tc2[0]),
									alignment_of(struct TypeCode_self));
							char *selfName=somir_dupl_string(*(char **)name._value,alloc);

							RHBOPT_ASSERT(TypeCode_equal(name._type,ev,TC_string));

							if (tc2)
							{
								tc2->align=TypeCode_alignment(tcIn,ev);
								tc2->selfName=selfName;

								tc=(somToken)tc2;
							}
						}
						break;
					case tk_pointer:
						{
							struct TypeCode_pointer *tc2=alloc->lpVtbl->alloc(alloc,sizeof(tc2[0]),
									alignment_of(struct TypeCode_pointer));
							any a=TypeCode_parameter(tcIn,ev,0);

							RHBOPT_ASSERT(ev->_major==NO_EXCEPTION);
							RHBOPT_ASSERT(TypeCode_equal(a._type,ev,TC_TypeCode));

							if (ev->_major==NO_EXCEPTION)
							{
								TypeCode ptrOf=SOMIR_dupl_TypeCode(*((TypeCode *)a._value),ev,alloc);

								if (tc2)
								{
									tc2->align=alignment_of(void *);
									tc2->tc=ptrOf;

									tc=(somToken)tc2;
								}
							}
						}
						break;
					case tk_sequence:
						{
							struct TypeCode_sequence *tc2=alloc->lpVtbl->alloc(alloc,sizeof(tc2[0]),
									alignment_of(struct TypeCode_sequence));
							any a=TypeCode_parameter(tcIn,ev,0);
							any b=TypeCode_parameter(tcIn,ev,1);

							RHBOPT_ASSERT(ev->_major==NO_EXCEPTION);
							RHBOPT_ASSERT(TypeCode_equal(a._type,ev,TC_TypeCode));
							RHBOPT_ASSERT(TypeCode_equal(b._type,ev,TC_long));

							if (ev->_major==NO_EXCEPTION)
							{
								TypeCode arrayOf=SOMIR_dupl_TypeCode(*((TypeCode *)a._value),ev,alloc);

								if (tc2)
								{
									tc2->align=alignment_of(GENERIC_SEQUENCE);
									tc2->tc=arrayOf;
									tc2->maxBound=*(long *)b._value;

									tc=(somToken)tc2;
								}
							}
						}
						break;
					case tk_array:
						{
							struct TypeCode_array *tc2=alloc->lpVtbl->alloc(alloc,sizeof(tc2[0]),
									alignment_of(struct TypeCode_array));
							any a=TypeCode_parameter(tcIn,ev,0);
							any b=TypeCode_parameter(tcIn,ev,1);

							RHBOPT_ASSERT(ev->_major==NO_EXCEPTION);
							RHBOPT_ASSERT(TypeCode_equal(a._type,ev,TC_TypeCode));
							RHBOPT_ASSERT(TypeCode_equal(b._type,ev,TC_long));

							if (ev->_major==NO_EXCEPTION)
							{
								TypeCode arrayOf=SOMIR_dupl_TypeCode(*((TypeCode *)a._value),ev,alloc);

								if (tc2)
								{
									tc2->align=TypeCode_alignment(arrayOf,ev);
									tc2->tc=arrayOf;
									tc2->maxBound=*(long *)b._value;

									tc=(somToken)tc2;
								}
							}
						}
						break;
					default:
						RHBOPT_ASSERT(tc);
						break;
					}

					if (tc)
					{
						TypeCode_peek(tc)->tag=kind;
						TypeCode_peek(tc)->version=SOMTC_VERSION_1;
						TypeCode_peek(tc)->flags=flags;
					}
				}
			}
		}
	}

	return tc;
}

static void SOMIR_dupl_data(void *tgt,
					 Environment *ev,
					 TypeCode tcIn,
					 const void *dataIn,
					 struct SOMIR_alloc *alloc)
{
	/************************************************
	 *
	 * the purpose of this is to duplicate any data as a single
	 * linear block of memory which can be free'd with a single
	 * SOMFree
	 *
	 */

	if (ev->_major==NO_EXCEPTION)
	{
		TCKind kind=TypeCode_kind(tcIn,ev);

		if (ev->_major==NO_EXCEPTION)
		{
			switch (kind)
			{
			case tk_null:

			case tk_void:
				break;

#define DUPL_BY_TYPE(tk,tv)   case tk: if (tgt) \
								{ *(tv *)tgt=*(const tv *)dataIn; } \
										break;

			DUPL_BY_TYPE(tk_enum,TCKind)
			DUPL_BY_TYPE(tk_octet,octet)
			DUPL_BY_TYPE(tk_boolean,boolean)
			DUPL_BY_TYPE(tk_char,char)
			DUPL_BY_TYPE(tk_short,short)
			DUPL_BY_TYPE(tk_ushort,unsigned short)
			DUPL_BY_TYPE(tk_long,long)
			DUPL_BY_TYPE(tk_ulong,unsigned long)
			DUPL_BY_TYPE(tk_float,float)
			DUPL_BY_TYPE(tk_double,double)

			case tk_any:
				{
					const any *anyIn=(const any *)dataIn;
					TypeCode type=SOMIR_dupl_TypeCode(anyIn->_type,ev,alloc);
					void *value=NULL;

					switch (TypeCode_kind(anyIn->_type,ev))
					{
					case tk_void:
					case tk_null:
						break;
					default:
						{
							value=alloc->lpVtbl->alloc(alloc,
									TypeCode_size(anyIn->_type,ev),
									TypeCode_alignment(anyIn->_type,ev));

							SOMIR_dupl_data(
								value,
								ev,
								anyIn->_type,
								anyIn->_value,
								alloc);
						}
						break;
					}

					if (tgt)
					{
						any *anyOut=(any *)tgt;
						anyOut->_type=type;
						anyOut->_value=value;
					}
				}
				break;

			case tk_sequence:
				{
					const GENERIC_SEQUENCE *seq=dataIn;
					any a=TypeCode_parameter(tcIn,ev,0);
					TypeCode elType=*(TypeCode *)a._value;
					short elAlign=TypeCode_alignment(elType,ev);
					size_t elSize=TypeCode_size(elType,ev);
					size_t mask=elAlign-1;
					void *value=NULL;
					unsigned long count=seq->_length;

					RHBOPT_ASSERT(TypeCode_kind(a._type,ev)==tk_TypeCode);

					if (elAlign && (elSize & mask))
					{
						elSize=(elSize+elAlign)&(size_t)~mask;
					}

					if (count)
					{
						unsigned long offset=0;

						value=alloc->lpVtbl->alloc(alloc,elSize*count,elAlign);

						while (count--)
						{
							SOMIR_dupl_data(
								value ? (((octet *)value)+offset) : NULL,
								ev,
								elType,
								((const octet *)seq->_buffer)+offset,
								alloc);

							offset+=(unsigned long)elSize;
						}
					}
#ifdef DUPL_EMPTY_SEQUENCE
					else
					{
						value=alloc->lpVtbl->alloc(alloc,0,0);
					}
#endif

					if (tgt)
					{
						GENERIC_SEQUENCE *out=tgt;

						out->_length=out->_maximum=seq->_length;
						out->_buffer=value;
					}
				}
				break;

			case tk_TypeCode:
				{
					TypeCode tc=*(TypeCode *)dataIn;
					TypeCode result=SOMIR_dupl_TypeCode(tc,ev,alloc);

					if (tgt)
					{
						*(TypeCode *)tgt=result;
					}
				}
				break;

			case tk_string:
				{
					const char *p=*(const char **)dataIn;
					char *q=NULL;

					if (p)
					{
						size_t len=1+strlen(p);
						q=alloc->lpVtbl->alloc(alloc,len,1);

						if (q)
						{
							memcpy(q,p,len);
						}
					}

					if (tgt)
					{
						*(char **)tgt=q;
					}
				}
				break;

			case tk_struct:
				{
					size_t offset=0;
					octet *pTgt=tgt;
					const octet *pSrc=dataIn;

					long num_elements=TypeCode_param_count(tcIn,ev)>>1;
					long index=2;

					while (num_elements--)
					{
						any a=TypeCode_parameter(tcIn,ev,index);
						TypeCode tcElement=*(TypeCode *)a._value;
						long elSize=TypeCode_size(tcElement,ev);
						short elAlign=TypeCode_alignment(tcElement,ev);

						RHBOPT_ASSERT(ev->_major==NO_EXCEPTION);
						RHBOPT_ASSERT(TypeCode_equal(a._type,ev,TC_TypeCode));

						if (elAlign)
						{
							size_t mask=elAlign-1;
							if (offset & mask)
							{
								offset=(offset+elAlign)&~mask;
							}
						}

						SOMIR_dupl_data(
							tgt ? pTgt+offset : NULL,
							ev,
							tcElement,
							pSrc+offset,
							alloc);

						offset+=elSize;

						index+=2;
					}
				}
				break;
			default:
				TypeCode_print(tcIn,ev);

				RHBOPT_ASSERT(!alloc);
				RHBOPT_ASSERT(!kind);
				RHBOPT_ASSERT(!tcIn);
				break;
			}
		}
	}
}

static struct SOMIR_name_id_pair read_nip(struct SOMIR_read_data *data,char *defined_in)
{
	struct SOMIR_name_id_pair nip={NULL,NULL};
	size_t defined_in_len=strlen(defined_in);
	long name_len=1+data->lpVtbl->read_long(data);

	RHBOPT_ASSERT(defined_in);

	if (defined_in_len==2)
	{
		if ((defined_in[0]==':')&&
			(defined_in[1]==':'))
		{
			defined_in_len=0;
		}
	}

	RHBOPT_ASSERT(name_len > 0);

	nip.id=data->allocator->lpVtbl->alloc(data->allocator,name_len+2+defined_in_len,1);
	
	if (nip.id)
	{
		nip.name=nip.id+2+defined_in_len;

		if (defined_in_len)
		{
			memcpy(nip.id,defined_in,defined_in_len);
		}

		nip.id[defined_in_len]=':';
		nip.id[defined_in_len+1]=':';
	}

	data->stream->lpVtbl->read(data->stream,nip.name,name_len);

	return nip;
}

static struct SOMIR_read_data_vtbl readDataVtbl={
	read_long,
	read_string,
	read_strings,
	read_TypeCode,
	read_nip
};

static void *somir_alloc(struct SOMIR_alloc *somThis,size_t len,short align)
{
	void *pv=NULL;

	if (align)
	{
		unsigned long mask=(unsigned long)(long)(align-1);

		if (somThis->data._length & mask)
		{
			somThis->data._length=(somThis->data._length+align)&~mask;
		}
	}

	if (somThis->data._buffer)
	{
		RHBOPT_ASSERT((somThis->data._length+len) <= somThis->data._maximum);

		pv=somThis->data._buffer+somThis->data._length;

	}

	somThis->data._length+=(unsigned long)len;

	return pv;
}

static void * somir_realise(struct SOMIR_alloc *somThis)
{
	if (somThis->data._length)
	{
		somThis->data._maximum=somThis->data._length;
		somThis->data._length=0;
		somThis->data._buffer=SOMMalloc(somThis->data._maximum);
	}

	return somThis->data._buffer;
}

static struct SOMIR_alloc_vtbl alloc_vtbl={
	somir_alloc,
	somir_realise
};

static size_t read_buffer_read(struct SOMIR_read_stream *somThis,
							   void *pv,
							   size_t len)
{
struct SOMIR_read_stream_buffer *data=(void *)somThis;

	RHBOPT_ASSERT(somThis==&data->stream);

	RHBOPT_ASSERT((data->data._length+len)<=(data->data._maximum));

	if (pv && data->data._buffer && len)
	{
		memcpy(pv,data->data._buffer+data->data._length,len);
	}

	data->data._length+=(unsigned long)len;

	return len;
}

static struct SOMIR_read_stream_vtbl buffer_read_vtbl={
	read_buffer_read
};

static long read_record(
	struct SOMIR_read_stream_buffer *buffer,
	struct SOMIRfile *file,
	long record)
{
	struct SOMIR_read_data reader={NULL,NULL,NULL};

	RHBOPT_ASSERT(buffer);
	RHBOPT_ASSERT(file);

	buffer->data._length=0;

	if (!buffer->stream.lpVtbl)
	{
		long length;

		buffer->stream.lpVtbl=&buffer_read_vtbl;

		reader.lpVtbl=&readDataVtbl;
		reader.stream=&file->stream;

		file->lpVtbl->Seek(file,record);

		length=reader.lpVtbl->read_long(&reader);

		if (length >= 4)
		{
			buffer->data._maximum=length;
			buffer->data._buffer=SOMMalloc(length);

			/* offset by four as have already read length
				which we will treat as inclusive */

			file->stream.lpVtbl->read(&file->stream,buffer->data._buffer+4,
								  length-4);

			buffer->data._buffer[0]=(octet)(length >> 24);
			buffer->data._buffer[1]=(octet)(length >> 16);
			buffer->data._buffer[2]=(octet)(length >> 8);
			buffer->data._buffer[3]=(octet)length;
		}
	}

	return buffer->data._maximum;
}

static void containedData_AddRef(struct SOMIR_ContainedData *somThis)
{
	RHBOPT_ASSERT(somThis);
	RHBOPT_ASSERT(somThis->usage > 0);

	rhbatomic_inc(&somThis->usage);
}

static struct SOMIR_ContainerData *containedData_remove(struct SOMIR_ContainedData *somThis)
{
	struct SOMIR_ContainerData *defined_in=somThis->defined_in;

	SOMIR_ASSERT_LOCKED

	if (defined_in)
	{
		somThis->defined_in=NULL;

		RHBOPT_ASSERT(defined_in);

		if (somThis->prev)
		{
			RHBOPT_ASSERT(somThis->prev->next==somThis);

			somThis->prev->next=somThis->next;

			if (somThis->next)
			{
				RHBOPT_ASSERT(somThis->next->prev==somThis)

				somThis->next->prev=somThis->prev;
			}
			else
			{
				/* it's the last item, but not only */
				RHBOPT_ASSERT(defined_in->last==somThis);

				defined_in->last=somThis->prev;
			}
		}
		else
		{
			/* it's the first item */
			RHBOPT_ASSERT(defined_in->first==somThis)

			defined_in->first=somThis->next;

			if (somThis->next)
			{
				RHBOPT_ASSERT(somThis->next->prev==somThis)
				somThis->next->prev=NULL;
			}
			else
			{
				RHBOPT_ASSERT(defined_in->last==somThis);
				/* it's the only one */
				defined_in->last=NULL;
			}
		}
	}

	return defined_in;
}

static void containedData_Release(struct SOMIR_ContainedData *somThis)
{
	struct SOMIR_ContainerData *defined_in=NULL;

	/* it would be nice to have the lock only while deleting
	   but synchronisation with aquire is tricky, thats work for another day */

	RHBOPT_ASSERT(somThis);
	RHBOPT_ASSERT(somThis->usage > 0);

	if (!rhbatomic_dec(&somThis->usage))
	{
		SOMIR_LOCK

		defined_in=containedData_remove(somThis);

		SOMIR_UNLOCK

		switch (somThis->type)
		{
		case kSOMIR_OperationDef:
			if (somThis->description.operationDesc.args)
			{
				SOMFree(somThis->description.operationDesc.args);
			}
			break;
		}
		
		SOMFree(somThis);
	}

	if (defined_in)
	{
		defined_in->lpVtbl->Release(defined_in);
	}
}

static Contained_Description containedData_describe(
		struct SOMIR_ContainedData *somThis,
		Environment *ev)
{
Contained_Description desc={NULL,{NULL,NULL}};

	if (ev->_major==NO_EXCEPTION)
	{
		switch (somThis->type)
		{
		case kSOMIR_InterfaceDef:
			desc.value._type=TC_InterfaceDescription;
			break;
		case kSOMIR_OperationDef:
			desc.value._type=TC_OperationDescription;
			break;
		case kSOMIR_AttributeDef:
			desc.value._type=TC_AttributeDescription;
			break;
		case kSOMIR_ExceptionDef:
			desc.value._type=TC_ExceptionDescription;
			break;
		case kSOMIR_ParameterDef:
			desc.value._type=TC_ParameterDescription;
			break;
		case kSOMIR_ConstantDef:
			desc.value._type=TC_ConstantDescription;
			break;
		case kSOMIR_TypeDef:
			desc.value._type=TC_TypeDescription;
			break;
		case kSOMIR_ModuleDef:
			desc.value._type=TC_ModuleDescription;
			break;
		}

		if (desc.value._type)
		{
			any a=TypeCode_parameter(desc.value._type,ev,0);
			size_t size=TypeCode_size(desc.value._type,ev);
			short align=TypeCode_alignment(desc.value._type,ev);
			struct SOMIR_alloc alloc={NULL,{0,0,NULL}};
			desc.name=*(char **)a._value;

			alloc.lpVtbl=&alloc_vtbl;

			do
			{
				desc.value._value=alloc.lpVtbl->alloc(&alloc,size,align);

				SOMIR_dupl_data(
					desc.value._value,
					ev,
					desc.value._type,
					&somThis->description,
					&alloc);

				if (desc.value._value) break;

			} while (alloc.lpVtbl->realise(&alloc));
		}

		switch (somThis->type)
		{
		case kSOMIR_OperationDef:
			{
				/* special fixup to match SOM */

				OperationDef_OperationDescription *dp=desc.value._value;

				if (!dp->parameter._length)
				{
					RHBOPT_ASSERT(dp->parameter._buffer);

					dp->parameter._buffer=NULL;
				}
			}
			break;
		}
	}

	return desc;	
}

static void full_sequence(
		struct SOMIR_ContainedData *somThis,
		Environment *ev,
		short type,
		TypeCode tc,
		GENERIC_SEQUENCE *seq)
{
	struct SOMIR_ContainerData *cnr=&somThis->description.interfaceDesc.containerData;
	unsigned long i=cnr->itemSeq._length;
	struct SOMIR_ContainerItem *p=cnr->itemSeq._buffer;
	unsigned long count=0;
	long size=TypeCode_size(tc,ev);
	short align=TypeCode_alignment(tc,ev);

	while (i--)
	{
		if (p->type==type) 
		{
			count++;
		}

		p++;
	}

	if (count)
	{
		struct
		{
			unsigned long _length,_maximum;
			struct SOMIR_ContainedData **_buffer;
		} itemSeq={0,0,NULL};

		p=cnr->itemSeq._buffer;
		i=cnr->itemSeq._length;

		itemSeq._maximum=count;
		itemSeq._buffer=SOMMalloc(count*sizeof(itemSeq._buffer[0]));

		while (i--)
		{
			if (p->type==type)
			{
				struct SOMIR_ContainedData *cnd=cnr->lpVtbl->Acquire(cnr,p->offset,p->type);

				if (cnd)
				{
					itemSeq._buffer[itemSeq._length++]=cnd;
				}
			}

			p++;
		}

		if (itemSeq._length)
		{
			struct SOMIR_alloc alloc={NULL,{0,0,NULL}};
			long totalBuffer=size*itemSeq._length;
			alloc.lpVtbl=&alloc_vtbl;

			seq->_maximum=itemSeq._length;

			do
			{
				long offset=0;
				seq->_length=0;
				seq->_buffer=alloc.lpVtbl->alloc(&alloc,totalBuffer,align);

				i=0;

				while (i < itemSeq._length)
				{
					struct SOMIR_ContainedData *cnd=itemSeq._buffer[i];

					RHBOPT_ASSERT(cnd);

					SOMIR_dupl_data(
						seq->_buffer ? ((char *)seq->_buffer)+offset : NULL,
						ev,
						tc,
						&cnd->description,
						&alloc);

					seq->_length++;
					offset+=size;

					if (seq->_length==seq->_maximum) 
					{
						break;
					}

					i++;
				}

				if (seq->_buffer) break;

			} while (alloc.lpVtbl->realise(&alloc));

			while (itemSeq._length--)
			{
				struct SOMIR_ContainedData *cnd=itemSeq._buffer[itemSeq._length];
				cnd->lpVtbl->Release(cnd);
			}
		}

		SOMFree(itemSeq._buffer);
	}
}

static InterfaceDef_FullInterfaceDescription containedData_interface(
		struct SOMIR_ContainedData *somThis,
		Environment *ev)
{
	InterfaceDef_FullInterfaceDescription retVal=
		{NULL,NULL,NULL,{0,0,NULL},{0,0,NULL}};

	RHBOPT_ASSERT(somThis->type==kSOMIR_InterfaceDef);

	/* three allocation blocks,
		1. ids
		2. operations
		3. attributes */

	{
		struct SOMIR_alloc alloc={NULL,{0,0,NULL}};

		alloc.lpVtbl=&alloc_vtbl;

		do
		{
			SOMIR_dupl_data(&retVal.name,ev,
						    TC_string,
							&somThis->description.interfaceDesc.desc.name,
							&alloc);
			SOMIR_dupl_data(&retVal.id,ev,
							TC_string,
							&somThis->description.interfaceDesc.desc.id,
							&alloc);
			SOMIR_dupl_data(&retVal.defined_in,ev,
							TC_string,
							&somThis->description.interfaceDesc.desc.defined_in,
							&alloc);

			if (retVal.name || retVal.id || retVal.defined_in) break;

		} while (alloc.lpVtbl->realise(&alloc));
	}

	full_sequence(somThis,ev,kSOMIR_OperationDef,TC_OperationDescription,(void *)&retVal.operation);
	full_sequence(somThis,ev,kSOMIR_AttributeDef,TC_AttributeDescription,(void *)&retVal.attributes);

	return retVal;
}

static struct SOMIR_ContainerData *containedData_IsContainer(
		struct SOMIR_ContainedData *somThis)
{
	struct SOMIR_ContainerData *retVal;

	switch (somThis->type)
	{
	case kSOMIR_OperationDef:
		retVal=&somThis->description.operationDesc.containerData;
		break;
	case kSOMIR_InterfaceDef:
		retVal=&somThis->description.interfaceDesc.containerData;
		break;
	case kSOMIR_ModuleDef:
		retVal=&somThis->description.moduleDesc.containerData;
		break;
	default:
		retVal=NULL;
		break;
	}
	return retVal;
}

static void container_AddRef(struct SOMIR_ContainerData *somThis)
{
	struct SOMIR_ContainedData *cnd=somThis->defined_in;

	RHBOPT_ASSERT(cnd);

	cnd->lpVtbl->AddRef(cnd);
}

static void container_Release(struct SOMIR_ContainerData *somThis)
{
	struct SOMIR_ContainedData *cnd=somThis->defined_in;

	RHBOPT_ASSERT(cnd);

	cnd->lpVtbl->Release(cnd);
}

static void container_Detach(struct SOMIR_ContainerData *somThis)
{
	RHBOPT_ASSERT(!somThis);
}

static struct SOMIR_ContainerDataVtbl containerVtbl={
	container_AddRef,
	container_Release,
	container_Detach,
	container_Acquire
};

static struct SOMIR_ContainedDataVtbl containedDataVtbl={
	containedData_AddRef,
	containedData_Release,
	containedData_describe,
	containedData_interface,
	containedData_IsContainer};

struct SOMIR_Acquire
{
	struct SOMIR_read_stream_buffer mainRecord,
									modifiersRecord,
									containerRecord,
									descendantsRecord;
	struct SOMIR_alloc allocator;
	struct SOMIR_ContainerData *containerData;
	struct
	{
		unsigned long _length,_maximum;
		struct SOMIR_ContainedData **_buffer;
	} refs;
};

RHBOPT_cleanup_begin(SOMIR_Acquire_cleanup,pv)
struct SOMIR_Acquire *data=pv;

	if (data->mainRecord.data._buffer)
	{
		SOMFree(data->mainRecord.data._buffer);
	}

	if (data->modifiersRecord.data._buffer)
	{
		SOMFree(data->modifiersRecord.data._buffer);
	}

	if (data->containerRecord.data._buffer)
	{
		SOMFree(data->containerRecord.data._buffer);
	}

	if (data->descendantsRecord.data._buffer)
	{
		SOMFree(data->descendantsRecord.data._buffer);
	}

	if (data->allocator.data._buffer)
	{
		SOMFree(data->allocator.data._buffer);
	}

	if (data->refs._buffer)
	{
		unsigned long i=data->refs._length;

		while (i--)
		{
			struct SOMIR_ContainedData *r=data->refs._buffer[i];

			if (r)
			{
				r->lpVtbl->Release(r);
			}
		}

		SOMFree(data->refs._buffer);
	}

RHBOPT_cleanup_end

static struct SOMIR_ContainedData *container_Acquire(
		struct SOMIR_ContainerData *defined_in,
		long rootOffset,
		long type)
{
	struct SOMIR_ContainedData * RHBOPT_volatile somThis=NULL;

	SOMIR_ASSERT_LOCKED

	if (defined_in)
	{
		somThis=defined_in->first;

		while (somThis)
		{
			if (somThis->usage)
			{
				if (somThis->containedOffset==rootOffset)
				{
					RHBOPT_ASSERT(type==somThis->type);

					somThis->lpVtbl->AddRef(somThis);

					break;
				}

				somThis=somThis->next;
			}
			else
			{
				struct SOMIR_ContainerData *di;
				/* marked for removal */

				RHBOPT_ASSERT(defined_in==somThis->defined_in);

				di=containedData_remove(somThis);

				RHBOPT_ASSERT(defined_in==di);

				if (di)
				{
					di->lpVtbl->Release(di);
				}

				somThis=defined_in->first;
			}
		}
	}

	if (!somThis)
	{
		struct SOMIR_Acquire data={
				{{NULL},{0,0,NULL}},
				{{NULL},{0,0,NULL}},
				{{NULL},{0,0,NULL}},
				{{NULL},{0,0,NULL}},
				{NULL,{0,0,NULL}},
				NULL,
				{0,0,NULL}};

		size_t len=(long)&(((struct SOMIR_ContainedData *)NULL)->description);
		size_t RHBOPT_volatile descLen=sizeof(somThis->description.moduleDesc);
		struct SOMIR_read_data reader={NULL,NULL,NULL};
		char * RHBOPT_volatile defined_in_id=defined_in->defined_in ?
							defined_in->defined_in->description.moduleDesc.desc.id :
							root_repository_id;

		RHBOPT_cleanup_push(SOMIR_Acquire_cleanup,&data);

		reader.lpVtbl=&readDataVtbl;
		reader.allocator=&data.allocator;

		read_record(&data.mainRecord,defined_in->file,rootOffset);

		data.allocator.lpVtbl=&alloc_vtbl;

		switch (type)
		{
		case kSOMIR_InterfaceDef: descLen=sizeof(somThis->description.interfaceDesc); break;
		case kSOMIR_OperationDef: descLen=sizeof(somThis->description.operationDesc); break;
		case kSOMIR_ExceptionDef: descLen=sizeof(somThis->description.exceptionDesc); break;
		case kSOMIR_ParameterDef: descLen=sizeof(somThis->description.parameterDesc); break;
		case kSOMIR_TypeDef: descLen=sizeof(somThis->description.typeDesc); break;
		case kSOMIR_ConstantDef: descLen=sizeof(somThis->description.constantDesc); break;
		case kSOMIR_AttributeDef: descLen=sizeof(somThis->description.attributeDesc); break;
		case kSOMIR_ModuleDef: descLen=sizeof(somThis->description.moduleDesc); break;
		default:
			RHBOPT_ASSERT(!defined_in->file)
			break;
		}

		do 
		{
			char *constantValue=NULL;
			struct SOMIR_ContainedData **exceptions=NULL;
			long recLength,
				 recType,
				 containerLength=0,
				 containerType=0,
				 containerCount=0,
				 modifiersLength=0,
				 modifiersType=0,
				 modifiersOwner=0,
				 zero1,
				 definedInOffset,
				 descendantsOffset=0,
				 descendantsCount=0,
				 modifiersOffset=0,
				 contentsOffset=0,
				 operationMode=0,
				 attributeMode=0,
				 parameterMode=0;
			struct SOMIR_DescendantItem *descendants=NULL;
			TypeCode tc=NULL;
			_IDL_SEQUENCE_somModifier somModifiers={0,0,NULL};
			_IDL_SEQUENCE_string contexts={0,0,NULL},
								 raiseSeq={0,0,NULL},
								 base_interfaces={0,0,NULL};
			_IDL_SEQUENCE_ParameterDef_ParameterDescription paramSeq={0,0,NULL};
			_IDL_SEQUENCE_ExceptionDef_ExceptionDescription exceptSeq={0,0,NULL};
			struct SOMIR_name_id_pair nip;

			somThis=data.allocator.lpVtbl->alloc(&data.allocator,len+descLen,
					alignment_of(struct SOMIR_ContainedData));

			reader.stream=&data.mainRecord.stream;
			data.mainRecord.data._length=0;

			recLength=reader.lpVtbl->read_long(&reader);
			recType=reader.lpVtbl->read_long(&reader);

			RHBOPT_ASSERT(recType==type);
			RHBOPT_unused(recType);
			RHBOPT_unused(recLength);

			nip=reader.lpVtbl->read_nip(&reader,defined_in_id);

			zero1=reader.lpVtbl->read_long(&reader);

			RHBOPT_ASSERT(!zero1);
			RHBOPT_unused(zero1);

			definedInOffset=reader.lpVtbl->read_long(&reader);
			descendantsOffset=reader.lpVtbl->read_long(&reader);
			modifiersOffset=reader.lpVtbl->read_long(&reader);

			RHBOPT_ASSERT(defined_in->defined_in ? 
					(definedInOffset==defined_in->defined_in->containedOffset) 
					: !definedInOffset);

			RHBOPT_ASSERT((!descendantsOffset)||(type==kSOMIR_InterfaceDef));

			RHBOPT_ASSERT(modifiersOffset);

			switch (type)
			{
			case kSOMIR_InterfaceDef:
			case kSOMIR_OperationDef:
			case kSOMIR_ModuleDef:
				contentsOffset=reader.lpVtbl->read_long(&reader);
				break;
			}

			if (somThis)
			{
				switch (type)
				{
				case kSOMIR_OperationDef:
					data.containerData=&somThis->description.operationDesc.containerData;
					break;
				case kSOMIR_InterfaceDef:
					data.containerData=&somThis->description.interfaceDesc.containerData;
					break;
				case kSOMIR_ModuleDef:
					data.containerData=&somThis->description.moduleDesc.containerData;
					break;
				}

				if (data.containerData)
				{
					data.containerData->lpVtbl=&containerVtbl;
					data.containerData->first=NULL;
					data.containerData->last=NULL;
					data.containerData->defined_in=somThis;
					data.containerData->itemSeq._length=0;
					data.containerData->itemSeq._maximum=0;
					data.containerData->itemSeq._buffer=NULL;
					data.containerData->file=defined_in->file;
				}
			}

			switch (type)
			{
			case kSOMIR_InterfaceDef:
				break;

			case kSOMIR_OperationDef:
				reader.lpVtbl->read_long(&reader);
				operationMode=reader.lpVtbl->read_long(&reader);

				RHBOPT_ASSERT((operationMode==OperationDef_NORMAL)||
							  (operationMode==OperationDef_ONEWAY));
				break;

			case kSOMIR_ModuleDef:
				break;

			case kSOMIR_AttributeDef:
				attributeMode=reader.lpVtbl->read_long(&reader);

				RHBOPT_ASSERT((attributeMode==AttributeDef_NORMAL)||
							  (attributeMode==AttributeDef_READONLY));

				break;

			case kSOMIR_ParameterDef:
				parameterMode=reader.lpVtbl->read_long(&reader);

				RHBOPT_ASSERT((parameterMode==ParameterDef_IN)||
							  (parameterMode==ParameterDef_OUT)||
							  (parameterMode==ParameterDef_INOUT));

				break;
			}

			RHBOPT_ASSERT(data.mainRecord.data._length==data.mainRecord.data._maximum);

			if (definedInOffset)
			{
				RHBOPT_ASSERT(defined_in->defined_in->containedOffset==definedInOffset);
			}
			else
			{
				RHBOPT_ASSERT(!defined_in->defined_in);
			}
					
			if (contentsOffset)
			{
				struct SOMIR_ContainerItem *items=NULL;
				read_record(&data.containerRecord,defined_in->file,contentsOffset);

				reader.stream=&data.containerRecord.stream;

				containerLength=reader.lpVtbl->read_long(&reader);
				containerType=reader.lpVtbl->read_long(&reader);
				containerCount=reader.lpVtbl->read_long(&reader);

				RHBOPT_ASSERT(containerType==kSOMIR_Container)
				RHBOPT_unused(containerType);
				RHBOPT_unused(containerLength);

				/* OperationDef with no parameters has no contents */

				if (somThis)
				{
					RHBOPT_ASSERT(data.containerData);

					data.containerData->itemSeq._length=containerCount;
					data.containerData->itemSeq._maximum=containerCount;
				}

				if (containerCount)
				{
					long i=0;

					items=data.allocator.lpVtbl->alloc(&data.allocator,
						sizeof(*items)*containerCount,
						alignment_of(struct SOMIR_ContainerItem));

					while (i < containerCount)
					{
						long offset=reader.lpVtbl->read_long(&reader);
						long type=reader.lpVtbl->read_long(&reader);
						char *name=reader.lpVtbl->read_string(&reader);

						if (items)
						{
							items[i].offset=offset;
							items[i].type=(short)type;
							items[i].name=name;
							items[i].namelen=(short)(name ? strlen(name) : 0);
						}

						i++;
					}

					if (type==kSOMIR_OperationDef)
					{
						paramSeq._length=0;
						paramSeq._maximum=containerCount;
						paramSeq._buffer=data.allocator.lpVtbl->alloc(&data.allocator,
							sizeof(paramSeq._buffer[0])*containerCount,
							alignment_of(ParameterDef_ParameterDescription));

						if (paramSeq._buffer)
						{
							while (paramSeq._length < paramSeq._maximum)
							{
								paramSeq._buffer[paramSeq._length].id=NULL;
								paramSeq._buffer[paramSeq._length].name=items[paramSeq._length].name;
								paramSeq._buffer[paramSeq._length].defined_in=nip.id;
								paramSeq._buffer[paramSeq._length].mode=0;
								paramSeq._buffer[paramSeq._length].type=NULL;

								paramSeq._length++;
							}
						}
					}
				}

				if (data.containerData)
				{
					data.containerData->itemSeq._buffer=items;
				}

				RHBOPT_ASSERT(data.containerRecord.data._length==
							  data.containerRecord.data._maximum);
			}

			if (modifiersOffset)
			{
				long zero;

				read_record(&data.modifiersRecord,defined_in->file,modifiersOffset);

				reader.stream=&data.modifiersRecord.stream;
				modifiersLength=reader.lpVtbl->read_long(&reader);
				modifiersType=reader.lpVtbl->read_long(&reader);

				RHBOPT_ASSERT(modifiersType==kSOMIR_Contained)
				RHBOPT_unused(modifiersType);

				modifiersOwner=reader.lpVtbl->read_long(&reader);

				RHBOPT_ASSERT(modifiersOwner==rootOffset);
				RHBOPT_unused(modifiersOwner);
				RHBOPT_unused(modifiersLength);

				zero=reader.lpVtbl->read_long(&reader);

				RHBOPT_ASSERT(!zero);
				RHBOPT_unused(zero);

				somModifiers._maximum=reader.lpVtbl->read_long(&reader);

				if (somModifiers._maximum)
				{
					somModifiers._buffer=
							data.allocator.lpVtbl->alloc(&data.allocator,
							sizeof(somModifiers._buffer[0])*somModifiers._maximum,
							alignment_of(somModifier));

					while (somModifiers._length < somModifiers._maximum)
					{
						somModifier RHBOPT_volatile mod;

						mod.name=reader.lpVtbl->read_string(&reader);
						mod.value=reader.lpVtbl->read_string(&reader);

						if (somModifiers._buffer)
						{
							somModifiers._buffer[somModifiers._length]=mod;
						}

						somModifiers._length++;
					}
				}

				switch (type)
				{
				case kSOMIR_InterfaceDef:
					base_interfaces=reader.lpVtbl->read_strings(&reader);
					tc=reader.lpVtbl->read_TypeCode(&reader);
					break;
				case kSOMIR_OperationDef:
					tc=reader.lpVtbl->read_TypeCode(&reader);
					contexts=reader.lpVtbl->read_strings(&reader);
					raiseSeq=reader.lpVtbl->read_strings(&reader);

					if (raiseSeq._length)
					{
						exceptSeq._maximum=raiseSeq._length;
						exceptSeq._buffer=data.allocator.lpVtbl->alloc(&data.allocator,
								sizeof(exceptSeq._buffer[0])*exceptSeq._maximum,
								alignment_of(ExceptionDef_ExceptionDescription));

						exceptions=data.allocator.lpVtbl->alloc(&data.allocator,
								sizeof(exceptions[0])*exceptSeq._maximum,
								alignment_of(struct SOMIR_ContainedData *));

						if (exceptSeq._buffer)
						{
							/* this can get filled in some time later when it's
								actually needed */

							while (exceptSeq._length < exceptSeq._maximum)
							{
								exceptSeq._buffer[exceptSeq._length].id=
									raiseSeq._buffer[exceptSeq._length];
								exceptSeq._buffer[exceptSeq._length].name=NULL;
								exceptSeq._buffer[exceptSeq._length].defined_in=NULL;
								exceptSeq._buffer[exceptSeq._length].type=NULL;

								exceptSeq._length++;
							}
						}

						if (exceptions)
						{
							unsigned long i=exceptSeq._maximum;
							while (i--)
							{
								exceptions[i]=NULL;
							}
						}
					}
					break;
				case kSOMIR_AttributeDef:
				case kSOMIR_ParameterDef:
				case kSOMIR_ExceptionDef:
				case kSOMIR_TypeDef:
					tc=reader.lpVtbl->read_TypeCode(&reader);
					break;
				case kSOMIR_ConstantDef:
					tc=reader.lpVtbl->read_TypeCode(&reader);
					constantValue=reader.lpVtbl->read_string(&reader);
					break;
				case kSOMIR_ModuleDef:
					break;
				default:
					RHBOPT_ASSERT(somThis)
					RHBOPT_ASSERT(!somThis)
					break;
				}

				RHBOPT_ASSERT(data.modifiersRecord.data._length==
							  data.modifiersRecord.data._maximum);
			}

			if (descendantsOffset)
			{
				long descendantsLength,descendantsType;

				read_record(&data.descendantsRecord,defined_in->file,descendantsOffset);

				reader.stream=&data.descendantsRecord.stream;
				descendantsLength=reader.lpVtbl->read_long(&reader);
				descendantsType=reader.lpVtbl->read_long(&reader);

				RHBOPT_ASSERT(descendantsType==kSOMIR_Descendant)
				RHBOPT_unused(descendantsType);
				RHBOPT_unused(descendantsLength);

				descendantsCount=reader.lpVtbl->read_long(&reader);

				if (descendantsCount)
				{
					long i=0;

					descendants=data.allocator.lpVtbl->alloc(&data.allocator,
								sizeof(descendants[0])*descendantsCount,
								alignment_of(struct SOMIR_DescendantItem));

					while (i < descendantsCount)
					{
						long descendantOffset=reader.lpVtbl->read_long(&reader);

						if (descendants)
						{
							descendants[i].offset=descendantOffset;
							descendants[i].containedData=NULL;
							descendants[i].lockCount=0;
						}

						i++;
					}
				}

				RHBOPT_ASSERT(data.descendantsRecord.data._length==
							  data.descendantsRecord.data._maximum);
			}

			if (somThis)
			{
				somThis->lpVtbl=&containedDataVtbl;
				somThis->usage=1;
				somThis->next=NULL;
				somThis->prev=NULL;
				somThis->containedOffset=rootOffset;
				somThis->defined_in=NULL;
				somThis->type=(short)type;
				somThis->modifiers=somModifiers;

				RHBOPT_ASSERT(nip.name);
				RHBOPT_ASSERT(nip.id);
				RHBOPT_ASSERT(defined_in_id);

				somThis->description.moduleDesc.desc.name=nip.name;
				somThis->description.moduleDesc.desc.id=nip.id;
				somThis->description.moduleDesc.desc.defined_in=defined_in_id;

				switch (type)
				{
				case kSOMIR_AttributeDef:
					somThis->description.attributeDesc.type=tc;
					somThis->description.attributeDesc.mode=attributeMode;
					break;

				case kSOMIR_ParameterDef:
					somThis->description.parameterDesc.type=tc;
					somThis->description.parameterDesc.mode=parameterMode;
					break;

				case kSOMIR_ConstantDef:
					somThis->description.constantDesc.desc.type=tc;
					somThis->description.constantDesc.desc.value._type=TC_string;
					somThis->description.constantDesc.desc.value._value=
							&somThis->description.constantDesc.value;
					somThis->description.constantDesc.value=constantValue;
					break;

				case kSOMIR_TypeDef:
					somThis->description.typeDesc.type=tc;
					break;

				case kSOMIR_ExceptionDef:
					somThis->description.exceptionDesc.type=tc;
					break;

				case kSOMIR_ModuleDef:
					break;

				case kSOMIR_InterfaceDef:
					somThis->description.interfaceDesc.base_interfaces=base_interfaces;
					somThis->description.interfaceDesc.instanceData=tc;
					somThis->description.interfaceDesc.descendants._length=descendantsCount;
					somThis->description.interfaceDesc.descendants._maximum=descendantsCount;
					somThis->description.interfaceDesc.descendants._buffer=descendants;
					break;

				case kSOMIR_OperationDef:
					somThis->description.operationDesc.desc.result=tc;
					somThis->description.operationDesc.desc.mode=operationMode;
					somThis->description.operationDesc.desc.contexts=contexts;
					somThis->description.operationDesc.desc.parameter=paramSeq;
					somThis->description.operationDesc.desc.exceptions=exceptSeq;
					somThis->description.operationDesc.raiseSeq=raiseSeq;
					somThis->description.operationDesc.exceptions=exceptions;
					somThis->description.operationDesc.args=NULL;
					break;

				default:
					RHBOPT_ASSERT(!somThis);
					break;
				}

				RHBOPT_ASSERT(data.allocator.data._length==data.allocator.data._maximum);

				break;
			}

		} while (data.allocator.lpVtbl->realise(&data.allocator));

		if (type==kSOMIR_OperationDef)
		{
			/* really want to have a single block
				starting with parameter _buffer
				that contains all parameters, then all exception data
				then don't need to worry about usage counts and the like
			*/

			unsigned long references=somThis->description.operationDesc.desc.parameter._length+
									 somThis->description.operationDesc.desc.exceptions._length;
			if (references)
			{
				unsigned long params=0;
				unsigned long raises=0;
				struct SOMIR_alloc alloc={NULL,{0,0,NULL}};
				Environment ev;
				SOM_InitEnvironment(&ev);

				data.refs._maximum=references;
				data.refs._buffer=SOMCalloc(sizeof(data.refs._buffer),references);

				RHBOPT_ASSERT(somThis->description.operationDesc.desc.exceptions._length==
							  somThis->description.operationDesc.raiseSeq._length);

				if (data.containerData)
				{
					RHBOPT_ASSERT(!data.containerData->first);

					while (params < data.containerData->itemSeq._length)
					{
						RHBOPT_ASSERT(data.containerData->itemSeq._buffer[params].type==
									kSOMIR_ParameterDef);

						data.refs._buffer[data.refs._length]=
								data.containerData->lpVtbl->Acquire(data.containerData,
								data.containerData->itemSeq._buffer[params].offset,
								data.containerData->itemSeq._buffer[params].type);

						RHBOPT_ASSERT(data.refs._buffer[data.refs._length]);

						data.refs._length++;
						params++;
					}
				}

				if (somThis->description.operationDesc.raiseSeq._length)
				{
					while (raises < somThis->description.operationDesc.raiseSeq._length)
					{
						char *ex_id=somThis->description.operationDesc.raiseSeq._buffer[raises];
						struct SOMIR_ContainedData *cnd=
								defined_in->file->lpVtbl->Lookup(
									defined_in->file,ex_id);

						RHBOPT_ASSERT(ex_id);
						RHBOPT_ASSERT(ex_id[0]==':');
						RHBOPT_ASSERT(cnd);

						data.refs._buffer[data.refs._length]=cnd;

						data.refs._length++;
						raises++;
					}
				}

				alloc.lpVtbl=&alloc_vtbl;

				do
				{
					unsigned long i=0;
					struct SOMIR_ContainedData **pop=data.refs._buffer;
					while (i < params)
					{
						struct SOMIR_ContainedData *param=*pop++;

						if (param)
						{
							ParameterDef_ParameterDescription *pd=somThis->description.operationDesc.desc.parameter._buffer+i;

							RHBOPT_ASSERT(param->type==kSOMIR_ParameterDef);
							RHBOPT_ASSERT(!strcmp(param->description.parameterDesc.name,pd->name));

							SOMIR_dupl_data(&(pd->id),&ev,TC_string,
											&(param->description.parameterDesc.id),
											&alloc);

							SOMIR_dupl_data(&(pd->type),&ev,TC_TypeCode,
											&(param->description.parameterDesc.type),
											&alloc);

							pd->mode=param->description.parameterDesc.mode;
						}

						i++;
					}

					i=0;

					while (i < raises)
					{
						struct SOMIR_ContainedData *ex=*pop++;

						if (ex)
						{
							ExceptionDef_ExceptionDescription *ed=somThis->description.operationDesc.desc.exceptions._buffer+i;

							RHBOPT_ASSERT(ex->type==kSOMIR_ExceptionDef);
							RHBOPT_ASSERT(!strcmp(ex->description.parameterDesc.id,ed->id));

							SOMIR_dupl_data(&(ed->name),&ev,TC_string,
											&(ex->description.exceptionDesc.name),
											&alloc);

							SOMIR_dupl_data(&(ed->defined_in),&ev,TC_string,
											&(ex->description.exceptionDesc.defined_in),
											&alloc);

							SOMIR_dupl_data(&(ed->type),&ev,TC_TypeCode,
											&(ex->description.exceptionDesc.type),
											&alloc);
						}

						i++;
					}

					if (alloc.data._buffer) break;

				} while (alloc.lpVtbl->realise(&alloc));

				somThis->description.operationDesc.args=alloc.data._buffer;

				SOM_UninitEnvironment(&ev);
			}
		}

		data.allocator.data._buffer=NULL;
		data.containerData=NULL;

		RHBOPT_cleanup_pop();

		RHBOPT_ASSERT(defined_in)

		defined_in->lpVtbl->AddRef(defined_in);

		somThis->defined_in=defined_in;

		if (defined_in->last)
		{
			defined_in->last->next=somThis;
			somThis->prev=defined_in->last;
			defined_in->last=somThis;
		}
		else
		{
			somThis->prev=NULL;
			defined_in->first=somThis;
			defined_in->last=somThis;
		}
	}

	return somThis;
}

static void irDetach(struct SOMIRfile *somThis)
{
	if (somThis->repository)
	{
		struct SOMIR_Repository *repository=somThis->repository;

		somThis->repository=NULL;

		if (somThis->next)
		{
			RHBOPT_ASSERT(somThis->next->prev==somThis);

			somThis->next->prev=somThis->prev;

			if (somThis->prev)
			{
				RHBOPT_ASSERT(somThis->prev->next==somThis);

				somThis->prev->next=somThis->next;
			}
			else
			{
				RHBOPT_ASSERT(repository->first==somThis);

				repository->first=somThis->next;
			}
		}
		else
		{
			RHBOPT_ASSERT(repository->last==somThis);

			repository->last=somThis->prev;

			if (somThis->prev)
			{
				RHBOPT_ASSERT(somThis->prev->next==somThis);

				repository->last->next=NULL;
			}
			else
			{
				RHBOPT_ASSERT(repository->first==somThis);

				repository->first=NULL;
			}
		}
	}
}

static void irDestroy(struct SOMIRfile *somThis)
{
	irDetach(somThis);

#ifdef _WIN32
	if (somThis->fd!=INVALID_HANDLE_VALUE)
	{
		BOOL b=CloseHandle(somThis->fd);
		RHBOPT_ASSERT(b);
	}
#else
	if (somThis->fd!=-1)
	{
		close(somThis->fd);
	}
#endif

	if (somThis->containerData.itemSeq._buffer)
	{
		SOMFree(somThis->containerData.itemSeq._buffer);
	}

	SOMFree(somThis);
}

static void repository_AddRef(struct SOMIR_ContainerData *somThis)
{
	rhbatomic_inc(&(somThis->file->usage));
}

static void repository_Release(struct SOMIR_ContainerData *somThis)
{
	if (!rhbatomic_dec(&(somThis->file->usage)))
	{
		irDestroy(somThis->file);
	}
}

static void repository_Detach(struct SOMIR_ContainerData *somThis)
{
	irDetach(somThis->file);
}

static struct SOMIR_ContainerDataVtbl repositoryVtbl={
	repository_AddRef,
	repository_Release,
	repository_Detach,
	container_Acquire
};

struct SOMIR_RepositoryInit
{
	struct SOMIR_read_stream_buffer mainRecord;
	struct SOMIR_alloc allocator;
	octet record_data[32];
};

RHBOPT_cleanup_begin(SOMIR_RepositoryInit_cleanup,pv)

struct SOMIR_RepositoryInit *data=pv;

	if (data->mainRecord.data._buffer)
	{
		SOMFree(data->mainRecord.data._buffer);
	}

RHBOPT_cleanup_end

static boolean SOMIR_RepositoryInit(struct SOMIRfile *file,struct SOMIR_ContainerData *container)
{
	struct SOMIR_RepositoryInit data={{{NULL},{0,0,NULL}},{NULL,{0,0,NULL}}};
	struct SOMIR_read_data reader={NULL,NULL,NULL};
	boolean RHBOPT_volatile result=0;

	file->usage=1;

	container->lpVtbl=&repositoryVtbl;

	file->lpVtbl->Seek(file,0);
	
	if (sizeof(data.record_data)==
			file->stream.lpVtbl->read(
				&file->stream,
				data.record_data,
				sizeof(data.record_data)))
	{
		reader.lpVtbl=&readDataVtbl;
		reader.allocator=&data.allocator;
		reader.stream=&data.mainRecord.stream;
		data.mainRecord.stream.lpVtbl=&buffer_read_vtbl;

		RHBOPT_ASSERT(file);

		container->file=file;

		data.mainRecord.data._buffer=data.record_data;
		data.mainRecord.data._maximum=sizeof(data.record_data);
		data.allocator.lpVtbl=&alloc_vtbl;

		file->header.magic1=reader.lpVtbl->read_long(&reader);
		file->header.magic2=reader.lpVtbl->read_long(&reader);
		file->header.masterOffset=reader.lpVtbl->read_long(&reader);
		file->header.freeList=reader.lpVtbl->read_long(&reader);
		file->header.unknown1=reader.lpVtbl->read_long(&reader);
		file->header.unknown2=reader.lpVtbl->read_long(&reader);
		file->header.timeStamp=reader.lpVtbl->read_long(&reader);
		file->header.zero2=reader.lpVtbl->read_long(&reader);

		if ((file->header.magic1==kSOMIR_Magic1)
			&&(file->header.magic2==kSOMIR_Magic2)
			&&file->header.masterOffset
			)
		{
			result=1;

			data.mainRecord.data._buffer=NULL;
			data.mainRecord.data._length=0;
			data.mainRecord.data._maximum=0;
			data.mainRecord.stream.lpVtbl=NULL;

			RHBOPT_cleanup_push(SOMIR_RepositoryInit_cleanup,&data);

			do
			{
				long length;
				long type;
				long count;

				read_record(&data.mainRecord,file,file->header.masterOffset);

				length=reader.lpVtbl->read_long(&reader);
				type=reader.lpVtbl->read_long(&reader);
				count=reader.lpVtbl->read_long(&reader);

				RHBOPT_ASSERT(type==kSOMIR_Container);
				RHBOPT_unused(type);
				RHBOPT_unused(length);

				container->itemSeq._length=0;
				container->itemSeq._maximum=count;

				if (!count) break;

				container->itemSeq._buffer=data.allocator.lpVtbl->alloc(
						&data.allocator,
						sizeof(*container->itemSeq._buffer)*count,
						alignment_of(struct SOMIR_ContainerItem));

				while (container->itemSeq._length <
					   container->itemSeq._maximum)
				{
					long offset=reader.lpVtbl->read_long(&reader);
					long itemType=reader.lpVtbl->read_long(&reader);
					char *name=reader.lpVtbl->read_string(&reader);

					if (container->itemSeq._buffer)
					{
						container->itemSeq._buffer[container->itemSeq._length].offset=offset;
						container->itemSeq._buffer[container->itemSeq._length].type=(short)itemType;
						container->itemSeq._buffer[container->itemSeq._length].name=name;
						container->itemSeq._buffer[container->itemSeq._length].namelen=
							(short)(name ? strlen(name) : 0);
					}

					container->itemSeq._length++;
				}

				if (container->itemSeq._buffer) break;

			} while (data.allocator.lpVtbl->realise(&data.allocator));

			RHBOPT_cleanup_pop();
		}
	}

	return result;
}

static size_t read_file(struct SOMIR_read_stream *somThis,void *buf,size_t len)
{
	struct SOMIRfile *file=SOMIR_cast_self(struct SOMIRfile,somThis,stream);
#ifdef _WIN32
	DWORD dw=(DWORD)len;
	RHBOPT_ASSERT(somThis==&file->stream)
	if (buf)
	{
		if (ReadFile(file->fd,buf,(DWORD)len,&dw,NULL)) 
		{
#if defined(_DEBUG) && defined(_M_IX86)
			if (!dw) __asm int 3;
#endif

			file->fileOffset+=dw;
			return dw;
		}
#if defined(_DEBUG) && defined(_M_IX86)
		__asm int 3;
#endif
	}
	else
	{
		LONG dw=0;
		
		dw=SetFilePointer(file->fd,(DWORD)len,&dw,FILE_CURRENT);

		if (dw != 0xffffffff)
		{
			file->fileOffset=dw;

			return len;
		}
	}
	return (size_t)-1;
#else
	if (buf)
	{
		int i=read(file->fd,buf,len);
		if (i>0)
		{
			file->fileOffset+=i;

			return (size_t)i;
		}
	}
	else
	{
		long posn=lseek(file->fd,len,SEEK_CUR);

		if (posn >= 0)
		{
			file->fileOffset=posn;

			return len;
		}
	}
	return (size_t)-1;
#endif
}

static void irSeek(struct SOMIRfile *somThis,long len)
{
	if (len != somThis->fileOffset)
	{
#ifdef _WIN32
		if (somThis->fd==INVALID_HANDLE_VALUE)
		{
			wchar_t wch[512];
			int len=(int)strlen(somThis->name);
			int i=MultiByteToWideChar(CP_ACP,0,somThis->name,len,wch,(sizeof(wch)/sizeof(wch[0]))-1);
			wch[i]=0;

			somThis->fd=CreateFileW(wch,
				GENERIC_READ,
				FILE_SHARE_READ,
				NULL,
				OPEN_EXISTING,
				FILE_FLAG_RANDOM_ACCESS,NULL);
		}

		if (somThis->fd==INVALID_HANDLE_VALUE)
		{
			somPrintf("SOMIR: ERROR: %s\n",somThis->name);
		}
		else
		{
			LONG high=0;
			DWORD dw=SetFilePointer(somThis->fd,len & 0xffffffff,&high,FILE_BEGIN);

			if (dw != 0xFFFFFFFFUL)
			{
				RHBOPT_ASSERT(dw == (DWORD)len);

				somThis->fileOffset=dw;
			}
		}
#else
		if (somThis->fd==-1)
		{
			somThis->fd=open(somThis->name,O_RDONLY);
		}

		if (somThis->fd!=-1)
		{
			long n=lseek(somThis->fd,len,SEEK_SET);
			if (n != -1L)
			{
				somThis->fileOffset=n;
			}
		}
#endif
	}
}

static struct SOMIR_ContainedData *container_lookup(struct SOMIR_ContainerData *somThis,const char *id)
{
	struct SOMIR_ContainedData *retVal=NULL;
	long nesting=0;

	SOMIR_ASSERT_LOCKED

	while (id[0])
	{
		short segLen=0;
		struct SOMIR_ContainerData *cnr=NULL;

		while (*id==':') id++;

		while (id[segLen] && (id[segLen]!=':')) segLen++;

		if (segLen)
		{
			unsigned long i=somThis->itemSeq._length;
			struct SOMIR_ContainerItem *p=somThis->itemSeq._buffer;

			while (i--)
			{
				if (p->namelen==segLen)
				{
					if (!memcmp(p->name,id,segLen))
					{
						retVal=somThis->lpVtbl->Acquire(
							somThis,
							p->offset,
							p->type);

						break;
					}
				}

				p++;
			}
		}

		if (!retVal) break;

		id+=segLen;

		if (!id[0]) break;

		cnr=retVal->lpVtbl->IsContainer(retVal);

		if (!cnr)
		{
			retVal->lpVtbl->Release(retVal);
			retVal=NULL;

			break;
		}

		somThis=cnr;

		nesting++;

		retVal=NULL;
	}

	while (nesting--)
	{
		struct SOMIR_ContainedData *cnd=somThis->defined_in;
		somThis=cnd->defined_in;
		cnd->lpVtbl->Release(cnd);
	}

	return retVal;
}

static struct SOMIR_ContainedData *irLookup(struct SOMIRfile *file,const char *id)
{
	return container_lookup(&file->containerData,id);
}

static void irRelease(struct SOMIRfile *file)
{
	file->containerData.lpVtbl->Release(&(file->containerData));
}

static void irClose(struct SOMIRfile *file)
{
#ifdef _PLATFORM_WIN32_
	HANDLE fd=file->fd;

	if (fd!=INVALID_HANDLE_VALUE)
	{
		BOOL b;
		file->fd=INVALID_HANDLE_VALUE;
		b=CloseHandle(fd);

		RHBOPT_ASSERT(b);

		file->fileOffset=-1L;
	}
#else
	int fd=file->fd;

	if (fd != -1)
	{
		file->fd=-1;
		close(fd);
		file->fileOffset=-1L;
	}
#endif
}

static struct SOMIRfileVtbl irfileVtbl=
{
	irRelease,
	irDetach,
	irSeek,
	irLookup,
	irClose
};

static struct SOMIR_read_stream_vtbl somir_file_stream_vtbl={
	read_file
};

struct SOMIRfile *SOMIRopen(struct SOMIR_Repository *repository,
							const char *filename)
{
	int k=(int)strlen(filename);
	struct SOMIRfile *somThis=NULL;
#ifdef _WIN32
	HANDLE fd=INVALID_HANDLE_VALUE;
	wchar_t wch[512];
	int i=MultiByteToWideChar(CP_ACP,0,filename,k,wch,(sizeof(wch)/sizeof(wch[0]))-1);
	wch[i]=0;
	fd=CreateFileW(wch,
		GENERIC_READ,
		FILE_SHARE_READ,
		NULL,
		OPEN_EXISTING,
		FILE_FLAG_RANDOM_ACCESS,NULL);
	if (fd!=INVALID_HANDLE_VALUE)
#else
	int fd=open(filename,O_RDONLY);
	if (fd!=-1)
#endif
	{
		somThis=SOMCalloc(sizeof(*somThis)+k,1);

		if (somThis)
		{
			memcpy(somThis->name,filename,k+1);
			somThis->lpVtbl=&irfileVtbl;
			somThis->fd=fd;
			somThis->fileOffset=0;
			somThis->stream.lpVtbl=&somir_file_stream_vtbl;
		}
		else
		{
#ifdef _WIN32
			BOOL b=CloseHandle(fd);
			RHBOPT_ASSERT(b);
#else
			close(fd);
#endif
		}

		if (somThis)
		{
			if (SOMIR_RepositoryInit(somThis,&somThis->containerData))
			{
				somThis->repository=repository;

				if (repository->last)
				{
					somThis->prev=repository->last;
					somThis->prev->next=somThis;
					repository->last=somThis;
				}
				else
				{
					repository->first=somThis;
					repository->last=somThis;
				}
			}
			else
			{
				somThis->lpVtbl->Release(somThis);

				somThis=NULL;
			}
		}
	}

	return somThis;
}

