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

#define SOM_DONTIMPORT_SOMTC

#define M_SOMAPI_CPP

#include <rhbopt.h>

#ifndef SOM_TYPECODE
#	define SOM_TYPECODE	TypeCode_somtc
typedef union SOM_TYPECODE *  TypeCode;
#endif

/* in order to stop SOMTCNST.H defining stuff we need to block it out */

#define somtcnst_h

#include <rhbsomex.h>
#include <somref.h>

#ifdef SOMDLLEXPORT
	#define SOM_IMPORTEXPORT_somtc  SOMDLLEXPORT
#else
	#define SOM_IMPORTEXPORT_somtc  
#endif

#include <somir.h>
#include <somtc.h>
#include <somtcint.h>
#include <stdio.h>
#include <somd.h>
#include <rhbsomex.h>

#define SHOW_CONSTANT_TC

#define Alignment(x)	 ((int)(size_t)&((struct { char _d; x z; } *)0)->z)

/* these are used to haul-in the various types */

#include <somdobj.h>
#include <containd.h>
#include <containr.h>
#include <paramdef.h>
#include <excptdef.h>
#include <operatdf.h>
#include <attribdf.h>
#include <intfacdf.h>
#include <repostry.h>
#include <typedef.h>
#include <moduledf.h>
#include <constdef.h>
#include <rhbsomex.h>

#ifdef SOMRealloc
	#undef SOMRealloc
#endif

#define SOMRealloc dont_use_this
#define printf  dont_use_printf_use_somPrintf

#define RHBSOMTC_unused(x)		if (x) { ; }

#ifdef TypeCode_kind
	#undef TypeCode_kind
	#define TypeCode_kind(t,ev)		t->tag
#endif

typedef unsigned short unsigned_short;
typedef unsigned long unsigned_long;

#ifdef _PLATFORM_MACINTOSH_
	#if powerc
		#pragma options align=mac68k
	#endif /* powerc */
#endif /* _PLATFORM_MACINTOSH_ */

typedef struct byte_struct { char b; } byte_struct;

#define TC__ParameterDef_ParameterDescription   TC__ParameterDescription
#define TC__OperationDef_OperationDescription   TC__OperationDescription
#define TC__AttributeDef_AttributeDescription   TC__AttributeDescription
#define TC__ExceptionDef_ExceptionDescription   TC__ExceptionDescription

#ifdef _WIN32
	/* makes the TypeCodes part of the CODE segment
	   and hence read-only */
	#ifdef __WATCOMC__
		#pragma data_seg("_TEXT","CODE")
	#else /* __WATCOMC__ */
		#pragma data_seg(".text","CODE")
	#endif /* !__WATCOMC__ */
#endif /* _WIN32 */

#define SOMTC_Scope SOM_IMPORTEXPORT_somtc

#include <somtcdat.tc>

#ifndef __TCD_TC__Object_generated
#define __TCD_TC__Object_generated
/* special case so that we don't get preceeding '::' */
SOMTC_Scope struct TypeCode_objref TC__Object={
	tk_objref,Alignment(SOMObject SOMSTAR),
	SOMTC_VERSION_1,0,
	"SOMObject"};
#endif /* __TCD_TC__Object_generated */

#undef SOMTC_EXTERN

#ifdef _WIN32
	#pragma data_seg()
#endif /* _WIN32 */

#ifdef _PLATFORM_MACINTOSH_
	#if powerc
		#pragma options align=reset
	#endif /* powerc */
#endif /* _PLATFORM_MACINTOSH_ */

/* these deliberately not been defined by SOM because of the #defined somtcnst_h */

#ifdef TC_long
#undef TC_long
#undef TC_string
#undef TC_TypeCode
#undef TC_ulong
#undef TC_octet
#endif

#define TC_long      ((TypeCode)(void *)&TC__long)
#define TC_string	 ((TypeCode)(void *)&TC__string)
#define TC_TypeCode  ((TypeCode)(void *)&TC__TypeCode)
#define TC_ulong     ((TypeCode)(void *)&TC__ulong)
#define TC_octet     ((TypeCode)(void *)&TC__octet)

/* alignment for various structs */

typedef struct TypeCode_enum  TypeCode_enum;
typedef struct TypeCode_foreign TypeCode_foreign;
typedef struct TypeCode_struct TypeCode_struct;
typedef struct TypeCode_union   TypeCode_union;
typedef struct TypeCode_struct_member TypeCode_struct_member;
typedef struct TypeCode_union_member   TypeCode_union_member;

#ifdef AlignmentHelper
	AlignmentHelper(TypeCode_enum)
	AlignmentHelper(TypeCode_struct)
	AlignmentHelper(TypeCode_union)
	AlignmentHelper(TypeCode_foreign)
	AlignmentHelper(TypeCode_union_member)
	AlignmentHelper(TypeCode_struct_member)
#endif

static void SOMTC_align_pos(long *pos,short by)
{
	if (by > 1)
	{
		short i=(short)(by-1);
		short d=(short)(i & *pos);

		if (d)
		{
			d=(short)(by-d);

			(*pos)+=d;
		}
	}
}

typedef struct { octet _d; union { octet o; } _u; } SOMTC_min_union_align_t;
typedef struct { octet _d; struct {	octet o; } _s; } SOMTC_min_struct_align_t;

#define SOMTC_min_union_align   ((int)(size_t)&(((SOMTC_min_union_align_t *)NULL)->_u))
#define SOMTC_min_struct_align  ((int)(size_t)&(((SOMTC_min_struct_align_t *)NULL)->_s))

static long SOMTC_cast_any_to_long(any *a)
{
	switch (TypeCode_peek(a->_type)->tag)
	{
	case tk_long:
		return *((long *)a->_value);
	case tk_ulong:
		return *((unsigned long *)a->_value);
	case tk_enum:
		return *((TCKind *)a->_value);
	case tk_short:
		return *((short *)a->_value);
	case tk_ushort:
		return *((unsigned short *)a->_value);
	case tk_char:
		return *((char *)a->_value);
	case tk_octet:
		return *((octet *)a->_value);
	case tk_boolean:
		return *((boolean *)a->_value);
	}

	RHBOPT_ASSERT(!a)

	return 0;
}

static char * SOMTC_cast_any_to_string(any *a)
{
	RHBOPT_ASSERT(a);
	RHBOPT_ASSERT(a->_type)
	RHBOPT_ASSERT(TypeCode_peek(a->_type)->tag==tk_string)

	if (tk_string==TypeCode_peek(a->_type)->tag)
	{
		return *((char **)a->_value);
	}

	return NULL;
}

static TypeCode SOMTC_cast_any_to_TypeCode(any *a)
{
	RHBOPT_ASSERT(a);
	RHBOPT_ASSERT(a->_type)
	RHBOPT_ASSERT(TypeCode_peek(a->_type)->tag==tk_TypeCode)

	if (tk_TypeCode==TypeCode_peek(a->_type)->tag)
	{
		return *((TypeCode *)a->_value);
	}

	return NULL;

}

static void * SOMTC_serial_alloc(_IDL_SEQUENCE_octet *seq,size_t size,short alignment)
{
	long x=seq->_length;
	long w=(alignment-1);
	long q=(x & w);

	if (q)
	{
		x+=(alignment-q);
	}

	seq->_length=(long)(x+size);

	if (seq->_buffer)
	{
		RHBOPT_ASSERT(seq->_length <= seq->_maximum)

		return seq->_buffer+x;
	}

	seq->_maximum=seq->_length;

	return 0;
}

static void *SOMTC_serial_init(Environment *ev,_IDL_SEQUENCE_octet *seq)
{
	if (seq)
	{
		seq->_buffer=SOMMalloc(seq->_length);
		if (seq->_buffer)
		{
			seq->_length=0;
#ifdef _WIN32
#ifdef _DEBUG
			memset(seq->_buffer,0xDD,seq->_length);
#endif
#endif
			return seq->_buffer;
		}
	}

	if (ev)
	{
		RHBOPT_throw_StExcep(ev,NO_MEMORY,UnexpectedNULL,MAYBE);
	}

	return NULL;
}

short SOMLINK tcAlignment(TypeCode t,Environment *ev)
{
	RHBOPT_ASSERT(ev) 
	RHBOPT_ASSERT(ev->_major==NO_EXCEPTION)

	if ((ev)&&(ev->_major==NO_EXCEPTION)&&(t))
	{
#define SOMTC_CASE_ALIGN(x)    case tk_##x: return Alignment(x);
		switch (TypeCode_peek(t)->tag)
		{
		SOMTC_CASE_ALIGN(char)
		SOMTC_CASE_ALIGN(short)
		SOMTC_CASE_ALIGN(long)
		SOMTC_CASE_ALIGN(double)
		SOMTC_CASE_ALIGN(float)
		SOMTC_CASE_ALIGN(boolean)
		SOMTC_CASE_ALIGN(octet)
		SOMTC_CASE_ALIGN(any)
		SOMTC_CASE_ALIGN(TypeCode)
#undef SOMTC_CASE_ALIGN
		case tk_ulong: return Alignment(unsigned_long);
		case tk_ushort: return Alignment(unsigned_short);
		case tk_sequence:
			return Alignment(GENERIC_SEQUENCE);
		case tk_string:
		case tk_pointer:
		case tk_Principal:
		case tk_objref:
			/* all these are pointer types */
			return Alignment(string);
		case tk_null:
			return 0;
		case tk_void:
			return 0;
		case tk_enum:
			/* TCKind must be same as an IDL generated enum */
			return Alignment(TCKind);
		case tk_struct:
			{
				short m=Alignment(byte_struct);
				/* 0==name, 1==elname, 2=type */
				long i=((struct TypeCode_struct *)t)->rptCount;
				struct TypeCode_struct_member *mb=((struct TypeCode_struct *)t)->mbrs;

				while (i--)
				{
					short n=tcAlignment(mb->mbrTc,ev);

					if (n > m) m=n;

					mb++;
				}

				return m;
			}
			break;
		case tk_union:
			{
				short m=Alignment(byte_struct);
				long len=TypeCode_param_count(t,ev);
				long i=4;

				/* 0==name, 1==discriminator, [ label, name, Type ] */

				/* tcPrint(t,ev); somPrintf("\n");*/

				{
					any a=tcParameter(t,ev,1);
					TypeCode *tp=a._value;
					short n=tcAlignment(*tp,ev);
					if (n > m) m=n;
				}

				while (i < len)
				{
					any a=tcParameter(t,ev,i);
					TypeCode *tp=a._value;
					short n=tcAlignment(*tp,ev);

	/*				tcPrint(a._type,ev); somPrintf("\n");*/

					RHBOPT_ASSERT(tcKind(a._type,ev) == tk_TypeCode)

					if (n > m) m=n;

					i+=3;
				}

				return m;
			}
			break;
		case tk_array:
			{
				any a=tcParameter(t,ev,0);
				TypeCode *tp=a._value;
				RHBOPT_ASSERT(tcKind(a._type,ev) == tk_TypeCode)
				return tcAlignment(*tp,ev);
			}
			break;
		case tk_self:
		case tk_foreign:
			return TypeCode_peek(t)->align;
		}

#if defined(_WIN32) && defined(_DEBUG)
		tcPrint(t,ev);
		RHBOPT_ASSERT(!t)
#endif

		RHBOPT_throw_StExcep(ev,BAD_TYPECODE,BadTypeCode,MAYBE);
	}

	return 0;
}

static boolean SOMTC_foreign_is_struct(char *p)
{
	/* encoding for tk_foreign is...

		language,option-sclass,marshaler-spec

		where language might be 'C' or 'SOM'
		optional-sclass might be 'pointer' or 'struct'

		pointer is default

		then marshalling spec is

		opaque				== sequence of bytes
		dynamic(marshallerclass,latent-param-name)
		static(function-name,C-initializer-statement)
	*/
	char *q="struct";
	int j=(int)strlen(q);
	int i;

	if (!p) return 0;
	if (!p[0]) return 0;

	while (p[0])
	{
		if (*p++==',')
		{
			break;
		}
	}

	i=(int)strlen(p);

	if (i < j)
	{
		return 0;
	}

	if (memcmp(p,q,j))
	{
		return 0;
	}

	p+=j;

	if (*p==0) return 1;
	if (*p==',') return 1;

	return 0;
}

TypeCode SOMLINK tcCopy(TypeCode tc,Environment *ev)
{
	RHBOPT_ASSERT(ev) 

	if (tc && ev && (ev->_major==NO_EXCEPTION))
	{
		switch (TypeCode_peek(tc)->tag)
		{
		case tk_void: return (TypeCode)(void *)&TC__void;
		case tk_null: return (TypeCode)(void *)&TC__null;
		case tk_char: return (TypeCode)(void *)&TC__char;
		case tk_octet: return (TypeCode)(void *)&TC__octet;
		case tk_boolean: return (TypeCode)(void *)&TC__boolean;
		case tk_short: return (TypeCode)(void *)&TC__short;
		case tk_ushort: return (TypeCode)(void *)&TC__ushort;
		case tk_long: return (TypeCode)(void *)&TC__long;
		case tk_ulong: return (TypeCode)(void *)&TC__ulong;
		case tk_float: return (TypeCode)(void *)&TC__float;
		case tk_double: return (TypeCode)(void *)&TC__double;
		case tk_any: return (TypeCode)(void *)&TC__any;
		case tk_TypeCode: return (TypeCode)(void *)&TC__TypeCode;
		case tk_Principal: return (TypeCode)(void *)&TC__Principal;
		case tk_objref:	return tcNew(tk_objref,
							 ((struct TypeCode_objref *)tc)->interfaceId);
		case tk_sequence:return tcNew(tk_sequence,
							tcCopy(((struct TypeCode_sequence *)tc)->tc,ev),
							((struct TypeCode_sequence *)tc)->maxBound);
		case tk_array: return tcNew(tk_array,
								tcCopy(((struct TypeCode_array *)tc)->tc,ev),
								((struct TypeCode_array *)tc)->maxBound);
		case tk_struct:
			{
				struct TypeCode_struct *src=(struct TypeCode_struct *)tc;
				_IDL_SEQUENCE_octet all={0,0,0};
				int nameLen=(int)(1+strlen(src->structName));
				int i=src->rptCount;
				int memberLen=sizeof(src->mbrs[0])*src->rptCount;

				SOMTC_serial_alloc(&all,sizeof(*src),Alignment(TypeCode_struct));

				SOMTC_serial_alloc(&all,memberLen,Alignment(TypeCode_struct_member));

				SOMTC_serial_alloc(&all,nameLen,1);

				while (i--)
				{
					SOMTC_serial_alloc(&all,1+strlen(src->mbrs[i].mbrName),1);
				}

				if (SOMTC_serial_init(ev,&all))
				{
					struct TypeCode_struct *tgt=SOMTC_serial_alloc(&all,
								sizeof(*tgt),
								Alignment(TypeCode_struct));
					tgt->mbrs=SOMTC_serial_alloc(&all,memberLen,
								Alignment(TypeCode_struct_member));
					tgt->structName=SOMTC_serial_alloc(&all,nameLen,1);
					memcpy(tgt->structName,src->structName,nameLen);

					tgt->align=tcAlignment(tc,ev);
					tgt->tag=tk_struct;
					tgt->flags=SOMTC_FLAGS_MALLOC;
					tgt->version=SOMTC_VERSION_1;
					tgt->rptCount=src->rptCount;

					i=src->rptCount;
					
					while (i--)
					{
						char *p=src->mbrs[i].mbrName;
						int len=(int)(1+strlen(p));
						tgt->mbrs[i].mbrName=SOMTC_serial_alloc(&all,len,1);
						memcpy(tgt->mbrs[i].mbrName,p,len);
						tgt->mbrs[i].mbrTc=tcCopy(src->mbrs[i].mbrTc,ev);
					}

					return (void *)tgt;
				}

			}
			break;
		case tk_self: return tcNew(tk_self,((struct TypeCode_self *)tc)->selfName);
		case tk_enum:
			{
				_IDL_SEQUENCE_octet all={0,0,NULL};
				const struct TypeCode_enum *t=(struct TypeCode_enum *)tc;
				struct TypeCode_enum *tr=NULL;
				int i=0;
				int nameLen=(int)(1+strlen(t->enumName));
				long enumLen=sizeof(t->enumId[0])*t->rptCount;

				{
					SOMTC_serial_alloc(&all,sizeof(*t),Alignment(TypeCode_enum));
				}
				SOMTC_serial_alloc(&all,enumLen,Alignment(string));
				SOMTC_serial_alloc(&all,nameLen,1);

				while (i < t->rptCount)
				{
					SOMTC_serial_alloc(&all,1+strlen(t->enumId[i]),1);
					i++;
				}
				if (SOMTC_serial_init(ev,&all))
				{
					{
						tr=SOMTC_serial_alloc(&all,sizeof(*tr),Alignment(TypeCode_enum));
					}
					tr->enumId=SOMTC_serial_alloc(&all,enumLen,Alignment(string));
					tr->enumName=SOMTC_serial_alloc(&all,nameLen,1);
					memcpy(tr->enumName,t->enumName,nameLen);
					while (i--)
					{
						char *p=t->enumId[i];
						int len=(int)(1+strlen(p));
						tr->enumId[i]=SOMTC_serial_alloc(&all,len,1);
						memcpy(tr->enumId[i],p,len);
					}
					tr->tag=t->tag;
					tr->rptCount=t->rptCount;
					tr->flags=(unsigned char)((t->flags&SOMTC_FLAGS_ZENUM)|SOMTC_FLAGS_MALLOC);
					tr->version=SOMTC_VERSION_1;
					tr->align=t->align;
				}
				return (TypeCode)tr;
			}
			break;
		case tk_string:
			return (((struct TypeCode_string *)tc)->maxBound) ?
				tcNew(tk_string,((struct TypeCode_string *)tc)->maxBound)
				:
				(TC_string);
		case tk_union:
			{
				struct TypeCode_union *src=(void *)tc;
				_IDL_SEQUENCE_octet all={0,0,0};
				int nameLen=(int)(1+strlen(src->unionName));
				int i=src->rptCount;
				int memberLen=i ? (i*sizeof(src->mbrs[0])) : 0;

				SOMTC_serial_alloc(&all,sizeof(*src),Alignment(TypeCode_union));

				if (memberLen)
				{
					SOMTC_serial_alloc(&all,memberLen,Alignment(TypeCode_union_member));
				}

				SOMTC_serial_alloc(&all,nameLen,1);

				while (i--)
				{
					SOMTC_serial_alloc(&all,1+strlen(src->mbrs[i].mbrName),1);
				}

				if (SOMTC_serial_init(ev,&all))
				{
					struct TypeCode_union *tgt=SOMTC_serial_alloc(&all,
								sizeof(*tgt),
								Alignment(TypeCode_union));

					tgt->mbrs=memberLen ? SOMTC_serial_alloc(&all,memberLen,
						Alignment(TypeCode_union_member)) : NULL;

					tgt->unionName=SOMTC_serial_alloc(&all,nameLen,1);
					memcpy(tgt->unionName,src->unionName,nameLen);

					tgt->align=tcAlignment(tc,ev);
					tgt->tag=tk_union;
					tgt->flags=SOMTC_FLAGS_MALLOC;
					tgt->version=SOMTC_VERSION_1;
					tgt->rptCount=src->rptCount;
					tgt->swTc=tcCopy(src->swTc,ev);

					i=src->rptCount;

					if (i)
					{
						/* this copies the labels */
						memcpy(tgt->mbrs,src->mbrs,i*sizeof(tgt->mbrs[0]));
					}

					while (i--)
					{
						char *p=src->mbrs[i].mbrName;
						int len=(int)(1+strlen(p));
						tgt->mbrs[i].mbrName=SOMTC_serial_alloc(&all,len,1);
						memcpy(tgt->mbrs[i].mbrName,p,len);
						tgt->mbrs[i].mbrTc=tcCopy(src->mbrs[i].mbrTc,ev);
					}

					return (TypeCode)tgt;
				}
			}
			break;
		case tk_foreign:
			{
				struct TypeCode_foreign *t=(void *)tc;
				return tcNew(tk_foreign,t->typeName,t->context,t->length);
			}
			break;
		case tk_pointer:
			{
				struct TypeCode_pointer *t=(void *)tc;
				return tcNew(tk_pointer,tcCopy(t->tc,ev));
			}
			break;
		default:
#if defined(_WIN32) && defined(_DEBUG)
			tcPrint(tc,ev);
			RHBOPT_ASSERT(!tc)
#endif
			RHBOPT_throw_StExcep(ev,BAD_TYPECODE,BadTypeCode,NO);
			break;
		}
	}

	return NULL;
}

boolean SOMLINK tcEqual(TypeCode tc,Environment *ev,TypeCode t)
{
	long i;

	RHBOPT_ASSERT(ev) 

	if (ev->_major != NO_EXCEPTION)
	{
		return 0;
	}

	if (tc==t)
	{
		return 1;
	}

	if ((!tc)||(!t))
	{
		RHBOPT_throw_StExcep(ev,BAD_TYPECODE,BadTypeCode,MAYBE);

		return 0;
	}

	if (TypeCode_peek(tc)->tag!=TypeCode_peek(t)->tag)
	{
		return 0;
	}

	i=TypeCode_param_count(t,ev);

	if (i != TypeCode_param_count(tc,ev))
	{
		return 0;
	}

	if (i)
	{
		while (i--)
		{
			any a=TypeCode_parameter(t,ev,i);
			TCKind kind=TypeCode_peek(a._type)->tag;
			any b=TypeCode_parameter(tc,ev,i);

			if (!tcEqual(a._type,ev,b._type))
			{
				return 0;
			}

			RHBOPT_ASSERT(a._value)
			RHBOPT_ASSERT(b._value)

			switch (kind)
			{
			case tk_ulong:
			case tk_long:
				if (*((long *)a._value) != *((long *)b._value))
				{
					return 0;
				}
				break;
			case tk_octet:
			case tk_boolean:
			case tk_char:
				if (*((octet *)a._value) != *((octet *)b._value))
				{
					return 0;
				}
				break;

			case tk_ushort:
			case tk_short:
				if (*((short *)a._value) != *((short *)b._value))
				{
					return 0;
				}
				break;

			case tk_TypeCode:
				if (!tcEqual(*(TypeCode *)a._value,ev,*(TypeCode *)b._value))
				{
					return 0;
				}
				break;
			case tk_string:
				if (strcmp(*(char **)a._value,*(char **)b._value))
				{
					return 0;
				}
				break;
			case tk_enum:
				if (*((TCKind *)a._value) != *((TCKind *)b._value))
				{
					return 0;
				}
				break;
			default:
				tcPrint(a._type,ev);
				RHBOPT_ASSERT(!a._type);
			}
		}
	}

	return 1;
}

void SOMLINK tcFree(TypeCode tc,Environment *ev)
{
	RHBOPT_ASSERT(ev)
	RHBOPT_ASSERT(!ev->_major)
	RHBOPT_ASSERT(tc)

	if (TypeCode_peek(tc)->flags & SOMTC_FLAGS_MALLOC)
	{
		switch (TypeCode_peek(tc)->tag)
		{
			case tk_pointer:
				{
					tcFree(((struct TypeCode_pointer *)tc)->tc,ev);
				}
				break;
			case tk_sequence:
				{
					tcFree(((struct TypeCode_sequence *)tc)->tc,ev);
				}
				break;
			case tk_array:
				{
					tcFree(((struct TypeCode_array *)tc)->tc,ev);
				}
				break;
			case tk_struct:
				{
					struct TypeCode_struct *ts=(void *)tc;
					long i=ts->rptCount;
					struct TypeCode_struct_member *mem=ts->mbrs;
					while (i--)
					{
						tcFree(mem->mbrTc,ev);
						mem++;
					}
				}
				break;
			case tk_union:
				{
					struct TypeCode_union *tu=(void *)tc;
					long i=tu->rptCount;
					struct TypeCode_union_member *mem=tu->mbrs;
					tcFree(tu->swTc,ev);
					while (i--)
					{
						tcFree(mem->mbrTc,ev);
						mem++;
					}
				}
				break;
		}

		SOMFree(tc);
	}
}

TCKind SOMLINK tcKind(TypeCode tc,Environment *ev)
{
	RHBOPT_ASSERT(ev) 
	RHBOPT_ASSERT(!ev->_major)

	return (ev->_major==NO_EXCEPTION) ?
	(
		tc ? TypeCode_peek(tc)->tag : TypeCode_tk_null
	)
	: 0;
}

long SOMLINK tcParmCount(TypeCode tc,Environment *ev)
{
	RHBOPT_ASSERT(ev)
	RHBOPT_ASSERT(!ev->_major)

	if ((ev->_major==NO_EXCEPTION)&&(tc))
	{
		switch (TypeCode_peek(tc)->tag)
		{
		case tk_void:
		case tk_null:
		case tk_long:
		case tk_short:
		case tk_ulong:
		case tk_ushort:
		case tk_char:
		case tk_boolean:
		case tk_octet:
		case tk_any:
		case tk_TypeCode:
		case tk_Principal:
		case tk_float:
		case tk_double:
			return 0;
		case tk_objref:
		case tk_string:
		case tk_pointer:
		case tk_self:
			return 1;
		case tk_array:
		case tk_sequence:
			return 2;
		case tk_foreign:
			return 3;
		case tk_struct:
			{
				struct TypeCode_struct *ts=(void *)tc;
				return (ts->rptCount << 1)+1;
			}
			break;
		case tk_enum:
			{
				struct TypeCode_enum *te=(void *)tc;
				return (te->rptCount)+1;
			}
			break;
		case tk_union:
			{
				struct TypeCode_union *tu=(void *)tc;
				return (tu->rptCount *3)+2;
			}
			break;
		default:
			RHBOPT_throw_StExcep(ev,BAD_TYPECODE,BadTypeCode,MAYBE);
			RHBOPT_ASSERT(!tc);
		}
	}

	return 0;
}

static void SOMTC_throw_BadParm(Environment *ev)
{
	RHBOPT_throw_StExcep(ev,BAD_PARAM,BadParm,NO);
}

any SOMLINK tcParameter(TypeCode tc,Environment *ev,long index)
{
	any b={NULL,NULL};

	b._type=(TypeCode)(void *)&TC__null;

	RHBOPT_ASSERT(ev)
	RHBOPT_ASSERT(!ev->_major)
	RHBOPT_ASSERT(tc)

	if ((!ev) || (ev->_major!=NO_EXCEPTION))
	{
		return b;
	}

	if ((index < 0)||(!tc))
	{
		SOMTC_throw_BadParm(ev);

		return b;
	}

	switch (TypeCode_peek(tc)->tag)
	{
	case tk_void:
	case tk_null:
	case tk_short:
	case tk_long:
	case tk_ushort:
	case tk_ulong:
		SOMTC_throw_BadParm(ev);
		return b;
	case tk_string:
		if (index==0)
		{
			struct TypeCode_string *ts=(void *)tc;
			b._value=&(ts->maxBound);
			b._type=TC_long;
		}
		else
		{
			SOMTC_throw_BadParm(ev);
		}
		return b;
	case tk_objref:
		if (index==0)
		{
			struct TypeCode_objref *to=(void *)tc;
			b._value=&(to->interfaceId);
			b._type=TC_string;
		}
		else
		{
			SOMTC_throw_BadParm(ev);
		}
		return b;
	case tk_struct:
		if (index==0)
		{
			struct TypeCode_struct *ts=(void *)tc;
			char **h=&(ts->structName);
			b._value=h;
			b._type=TC_string;
		}
		else
		{
			struct TypeCode_struct *ts=(void *)tc;
			int i=(index-1)>>1;
			if (i < ts->rptCount)
			{
				struct TypeCode_struct_member *me=&(ts->mbrs[i]);
				index-=1;
				if (index & 1)
				{
					TypeCode *h=&(me->mbrTc);
					b._type=TC_TypeCode;
					b._value=h;
				}
				else
				{
					char **h=&(me->mbrName);
					b._type=TC_string;
					b._value=h;
				}
			}
			else
			{
				SOMTC_throw_BadParm(ev);
			}
		}
		return b;
	case tk_sequence:
		{
			struct TypeCode_sequence *ts=(void *)tc;

			switch (index)
			{
			case 0:
				{
					TypeCode *h=&(ts->tc);
					b._value=h;
					b._type=TC_TypeCode;
				}
				break;
			case 1:
				{
					long *h=&(ts->maxBound);
					b._value=h;
					b._type=TC_long;
				}
				break;
			default:
				SOMTC_throw_BadParm(ev);
			}
		}
		return b;
	case tk_array:
		{
			struct TypeCode_array *ta=(void *)tc;

			switch (index)
			{
			case 0:
				{
					TypeCode *h=&(ta->tc);
					b._value=h;
					b._type=TC_TypeCode;
				}
				break;
			case 1:
				{
					long *h=&(ta->maxBound);
					b._value=h;
					b._type=TC_long;
				}
				break;
			default:
				SOMTC_throw_BadParm(ev);
			}
		}
		return b;
	case tk_pointer:
		{
			struct TypeCode_pointer *tp=(void *)tc;

			switch (index)
			{
			case 0:
				b._type=TC_TypeCode;
				b._value=&(tp->tc);
				break;
			default:
				SOMTC_throw_BadParm(ev);
			}
		}
		return b;
	case tk_foreign:
		{
			struct TypeCode_foreign *tf=(void *)tc;
			switch (index)
			{
			case 0:
				b._type=TC_string;
				b._value=&(tf->typeName);
				break;
			case 1:
				b._type=TC_string;
				b._value=&(tf->context);
				break;
			case 2:
				b._type=TC_long;
				b._value=&(tf->length);
				break;
			default:
				SOMTC_throw_BadParm(ev);
			}
		}
		return b;
	case tk_self:
		{
			struct TypeCode_self *ts=(void *)tc;
			switch (index)
			{
			case 0:
				b._type=TC_string;
				b._value=&(ts->selfName);
				break;
			default:
				SOMTC_throw_BadParm(ev);
			}
		}
		return b;
	case tk_enum:
		{
			struct TypeCode_enum *te=(void *)tc;

			if (index==0)
			{
				b._type=TC_string;
				b._value=&(te->enumName);

				return b;
			}

			index--;

			if (index < te->rptCount)
			{
				char **h;
				b._type=TC_string;
				h=te->enumId+index;
				b._value=h;
			}
			else
			{
				SOMTC_throw_BadParm(ev);
			}
		}
		return b;
	case tk_union:
		{
			struct TypeCode_union *tu=(void *)tc;

			switch (index)
			{
			case 0:
				b._type=TC_string;
				b._value=&(tu->unionName);
				return b;
			case 1:
				b._type=TC_TypeCode;
				b._value=&(tu->swTc);
				return b;
			default:
				index-=2;
				{
					int i=0;
					int j=0;
					struct TypeCode_union_member *m=tu->mbrs;

					while (index--)
					{
						j++;
						if (j==3)
						{
							i++;
							j=0;
							if (i >= tu->rptCount)
							{
								SOMTC_throw_BadParm(ev);
								return b;
							}
							m++;
						}
					}

					switch (j)
					{
					case 0:
						switch (m->labelFlag)
						{
						case TCREGULAR_CASE:
							b._type=tu->swTc;
							break;
						case TCDEFAULT_CASE:
							b._type=TC_octet;
							break;
						default:
							RHBOPT_ASSERT(!m);
						}
						b._value=&(m->labelValue);
						return b;
					case 1:
						b._type=TC_string;
						b._value=&(m->mbrName);
						return b;
					case 2:
						b._type=TC_TypeCode;
						b._value=&(m->mbrTc);
						return b;
					}

				}
				SOMTC_throw_BadParm(ev);
			}
		}
		break;
	default:
		RHBOPT_ASSERT(!tc);
		SOMTC_throw_BadParm(ev);
	}

	return b;
}

static void SOMLINK tcPrint_inner(TypeCode t,Environment *ev)
{
	if (t && ev && (ev->_major==NO_EXCEPTION))
	{
		int nulOnEnd=0;
		int i=TypeCode_param_count(t,ev);

		somPrintf("TypeCodeNew (");

#	ifdef SHOW_CONSTANT_TC
		if (!(TypeCode_peek(t)->flags & SOMTC_FLAGS_MALLOC))
		{
			somPrintf("/*constant*/ ");
		}
#	endif

		switch (TypeCode_peek(t)->tag)
		{
			case tk_octet:
				somPrintf("tk_octet");
				break;
			case tk_longdouble:
				somPrintf("tk_longdouble");
				break;
			case tk_longlong:
				somPrintf("tk_longlong");
				break;
			case tk_ulonglong:
				somPrintf("tk_ulonglong");
				break;
			case tk_wchar:
				somPrintf("tk_wchar");
				break;
			case tk_boolean:
				somPrintf("tk_boolean");
				break;
			case tk_long:
				somPrintf("tk_long");
				break;
			case tk_short:
				somPrintf("tk_short");
				break;
			case tk_ulong:
				somPrintf("tk_ulong");
				break;
			case tk_ushort:
				somPrintf("tk_ushort");
				break;
			case tk_char:
				somPrintf("tk_char");
				break;
			case tk_float:
				somPrintf("tk_float");
				break;
			case tk_double:
				somPrintf("tk_double");
				break;
			case tk_struct:
				somPrintf("tk_struct");
				nulOnEnd=1;
				break;
			case tk_any:
				somPrintf("tk_any");
				break;
			case tk_array:
				somPrintf("tk_array");
				break;
			case tk_sequence:
				somPrintf("tk_sequence");
				break;
			case tk_enum:
				somPrintf("tk_enum");
				nulOnEnd=1;
				break;
			case tk_objref:
				somPrintf("tk_objref");
				break;
			case tk_string:
				somPrintf("tk_string");
				break;
			case tk_wstring:
				somPrintf("tk_wstring");
				break;
			case tk_fixed:
				somPrintf("tk_fixed");
				break;
			case tk_union:
				somPrintf("tk_union");
				nulOnEnd=1;
				break;
			case tk_TypeCode:
				somPrintf("tk_TypeCode");
				break;
			case tk_Principal:
				somPrintf("tk_Principal");
				break;
			case tk_void:
				somPrintf("tk_void");
				break;
			case tk_null:
				somPrintf("tk_null");
				break;
			case tk_pointer:
				somPrintf("tk_pointer");
				break;
			case tk_foreign:
				somPrintf("tk_foreign");
				break;
			case tk_self:
				somPrintf("tk_self");
				break;
			default:
				somPrintf("%d",(int)(TypeCode_peek(t)->tag));
		}

		if (i)
		{
			int j=0;
			int e=-2;
			long l;

			while (j < i)
			{
				any a=TypeCode_parameter(t,ev,j);

				if (TypeCode_peek(t)->tag==tk_union)
				{
					if (e==0)
					{
						const char *p="TCREGULAR_CASE";

						if (tk_octet==TypeCode_peek(a._type)->tag)
						{
							octet *op=a._value;
							if (op[0]==0)
							{
								p="TCDEFAULT_CASE";
							}
						}

						somPrintf(", %s",p);
					}
					else
					{
						if (e==2)
						{
							e=-1;
						}
					}
				}

				switch (TypeCode_peek(a._type)->tag)
				{
				case tk_string:
					somPrintf(", \042%s\042",SOMTC_cast_any_to_string(&a));
					break;
				case tk_TypeCode:
					somPrintf(", ");
					tcPrint_inner(SOMTC_cast_any_to_TypeCode(&a),ev);
					break;
				case tk_long:
				case tk_ulong:
					l=SOMTC_cast_any_to_long(&a);

					if (TypeCode_peek(t)->tag==tk_fixed)
					{
						somPrintf(", %ld",(long)l);
					}
					else
					{
						somPrintf(", (long)%ld",(long)l);
					}

					break;
				case tk_short:
				case tk_ushort:
				case tk_char:
				case tk_octet:
				case tk_boolean:
					l=SOMTC_cast_any_to_long(&a);
					somPrintf(", %ld",(long)l);
					break;
				case tk_enum:
					l=SOMTC_cast_any_to_long(&a);
					if (a._type)
					{
						any b;
						corbastring p;
						b=tcParameter(a._type,ev,0);
						p=SOMTC_cast_any_to_string(&b);
						if (p)
						{
							somPrintf(", %s_",p);
						}
						else
						{
							somPrintf(", ");
						}

						b=tcParameter(a._type,ev,l);
						p=SOMTC_cast_any_to_string(&b);
						somPrintf("%s",p);
					}
					break;
				default:
					RHBOPT_ASSERT(!t);
				}

				e++;
				j++;
			}
		}

		if (nulOnEnd) somPrintf(", NULL");

		somPrintf(")");
	}
	else
	{
		somPrintf("NULL");
	}
}

void SOMLINK tcPrint(TypeCode t,Environment *ev)
{
	tcPrint_inner(t,ev);
	somPrintf("\n");
}

void SOMLINK tcSetAlignment(TypeCode t,Environment *ev, short a)
{
	RHBSOMTC_unused(ev)

	RHBOPT_ASSERT(TypeCode_peek(t)->align == a)

	TypeCode_peek(t)->align=a;
}

long SOMLINK tcSize(TypeCode t,Environment *ev)
{
	long size=0;

	if ((ev->_major==NO_EXCEPTION)&&(t))
	{
		typedef unsigned long ulong;
		typedef unsigned short ushort;
#define SOMTC_CASE_SIZE(x)    case tk_##x: size=sizeof(x); break;
		RHBSOMTC_unused(ev)

		switch (TypeCode_peek(t)->tag)
		{
		case tk_void:
		case tk_null:
			break;
		SOMTC_CASE_SIZE(char)
		SOMTC_CASE_SIZE(short)
		SOMTC_CASE_SIZE(long)
		SOMTC_CASE_SIZE(octet)
		SOMTC_CASE_SIZE(boolean)
		SOMTC_CASE_SIZE(TypeCode)
		SOMTC_CASE_SIZE(any)
		SOMTC_CASE_SIZE(float)
		SOMTC_CASE_SIZE(double)
		SOMTC_CASE_SIZE(ushort)
		SOMTC_CASE_SIZE(ulong)
#undef SOMTC_CASE_SIZE
		case tk_enum:
			size=sizeof(TCKind);
			break;
		case tk_sequence:
			size=sizeof(GENERIC_SEQUENCE);
			break;
		case tk_pointer:
		case tk_objref:
		case tk_Principal:
		case tk_string:
			size=sizeof(void *);
			break;
		case tk_struct:
			{
				struct TypeCode_struct *ts=(void *)t;
				int i=ts->rptCount;
				struct TypeCode_struct_member *tm=ts->mbrs;
				short align=Alignment(byte_struct);
				while (i--)
				{
					TypeCode t2=tm->mbrTc;
					short al=tcAlignment(t2,ev);
					long len=tcSize(t2,ev);
					SOMTC_align_pos(&size,al);
					size+=len;
					if (al > align) align=al;
					tm++;
				}
				SOMTC_align_pos(&size,align);
			}
			break;
		case tk_union:
			{
				short align=tcAlignment(t,ev);
				any adesc=TypeCode_parameter(t,ev,1);
				int len=TypeCode_param_count(t,ev);
				int i=4;
				long maxel=0;
				short ba=Alignment(byte_struct);
				TypeCode *tp=adesc._value;

				size=tcSize(*tp,ev);	/* descriminator is first */

				SOMTC_align_pos(&size,ba); /* items are in a struct */

				while (i < len)
				{
					any el=TypeCode_parameter(t,ev,i);
					TypeCode *elp=el._value;
					TypeCode elt=*elp;
					long els=tcSize(elt,ev);
					short ela=tcAlignment(elt,ev);

					if (els > maxel)
					{
						maxel=els;
					}

					if (ela > ba)
					{
						ba=ela;
					}

					i+=3;
				}

				SOMTC_align_pos(&size,ba);

				size+=maxel;

				SOMTC_align_pos(&size,align);
			}
			break;
		case tk_array:
			{
				any a=TypeCode_parameter(t,ev,0);
				TypeCode *tp=a._value;
				any b=TypeCode_parameter(t,ev,1);
				long *lp=b._value;

				size=tcSize(*tp,ev)*(*lp);
			}
			break;
		case tk_foreign:
			{
				any a=TypeCode_parameter(t,ev,2);
				size=SOMTC_cast_any_to_long(&a);
			}
			break;
		case tk_self:
			break;
		default:
			RHBOPT_ASSERT(t);
			RHBOPT_ASSERT(!t);
			RHBOPT_throw_StExcep(ev,BAD_TYPECODE,BadTypeCode,NO);
			break;
		}
	}
	return size;
}

TypeCode SOMLINK tcNewVL(TCKind tag,va_list pop)
{
	TypeCode tcRetVal=NULL;
	char *p=NULL;
	long l=0;

	RHBOPT_ASSERT(tag);

	switch (tag)
	{
	case tk_void: tcRetVal=(TypeCode)(void *)&TC__void; break;
	case tk_null: tcRetVal=(TypeCode)(void *)&TC__null; break;
	case tk_char: tcRetVal=(TypeCode)(void *)&TC__char; break;
	case tk_octet: tcRetVal=(TypeCode)(void *)&TC__octet; break;
	case tk_boolean: tcRetVal=(TypeCode)(void *)&TC__boolean; break;
	case tk_short: tcRetVal=(TypeCode)(void *)&TC__short; break;
	case tk_ushort: tcRetVal=(TypeCode)(void *)&TC__ushort; break;
	case tk_long: tcRetVal=(TypeCode)(void *)&TC__long; break;
	case tk_ulong: tcRetVal=(TypeCode)(void *)&TC__ulong; break;
	case tk_float: tcRetVal=(TypeCode)(void *)&TC__float; break;
	case tk_double: tcRetVal=(TypeCode)(void *)&TC__double; break;
	case tk_any: tcRetVal=(TypeCode)(void *)&TC__any; break;
	case tk_TypeCode: tcRetVal=(TypeCode)(void *)&TC__TypeCode; break;
	case tk_Principal: tcRetVal=(TypeCode)(void *)&TC__Principal; break;

/*	case tk_longlong: t=&TC__longlong; break;
	case tk_ulonglong: t=&TC__ulonglong; break;
	case tk_wchar: t=&TC__wchar; break;
	case tk_longdouble: t=&TC__longdouble; break;
*/

	case tk_struct:
		{
			_IDL_SEQUENCE_octet all={
					sizeof(struct TypeCode_struct),
					sizeof(struct TypeCode_struct),NULL};
			long rptCount=0;
			struct TypeCode_struct *ts;
			size_t structNameLen;
			size_t stringLengths=0;
			va_list args;
			short align=Alignment(octet);
			Environment ev;
			SOM_InitEnvironment(&ev);

			/* TypeCode_struct
			   TypeCode_struct_element[rptCount]
			   char structName[structNameLen]
			   char elemNames[....]
			*/

			va_copy(args,pop);

			p=va_arg(args,char *);
			structNameLen=1+strlen(p);
			stringLengths=structNameLen;

			do
			{
				p=va_arg(args,char *);
				if (p)
				{
					size_t n=strlen(p)+1;
					TypeCode tc=va_arg(args,TypeCode);
					short al=tcAlignment(tc,&ev);

					if (al > align) align=al;

					stringLengths+=n;
					rptCount++;
				}
			} while (p);

			if (rptCount)
			{
				SOMTC_serial_alloc(&all,
					rptCount*sizeof(ts->mbrs[0]),
					Alignment(struct TypeCode_struct_member));
			}

			SOMTC_serial_alloc(&all,stringLengths,1);

			if (SOMTC_serial_init(&ev,&all))
			{
				ts=SOMTC_serial_alloc(&all,sizeof(*ts),Alignment(struct TypeCode_struct));
				ts->tag=tk_struct;
				ts->align=align;
				ts->flags=SOMTC_FLAGS_MALLOC;
				ts->version=SOMTC_VERSION_1;
				ts->rptCount=rptCount;
				ts->mbrs=rptCount ?
						SOMTC_serial_alloc(&all,
							rptCount*sizeof(ts->mbrs[0]),
							Alignment(struct TypeCode_struct_member)) : NULL;

				va_copy(args,pop);

				p=va_arg(args,char *);

				ts->structName=SOMTC_serial_alloc(&all,structNameLen,1);

				memcpy(ts->structName,p,structNameLen);

				rptCount=0;

				while (rptCount < ts->rptCount)
				{
					char *n=va_arg(args,char *);
					TypeCode tc=va_arg(args,TypeCode);
					size_t nlen=strlen(n)+1;

					RHBOPT_ASSERT(n);
					RHBOPT_ASSERT(tc);

					ts->mbrs[rptCount].mbrTc=tc;
					ts->mbrs[rptCount].mbrName=SOMTC_serial_alloc(&all,nlen,1);

					memcpy(ts->mbrs[rptCount].mbrName,n,nlen);

					rptCount++;
				}

				tcRetVal=(TypeCode)ts;
			}
			
			SOM_UninitEnvironment(&ev);
		}
		break;

	case tk_enum:
		{
			_IDL_SEQUENCE_octet mem={
					sizeof(struct TypeCode_enum),
					sizeof(struct TypeCode_enum),NULL};
			long rptCount=0;
			char *p;
			char *enumName;
			struct TypeCode_enum *te;
			size_t enumNameLen;

			va_list va_enum;
			va_copy(va_enum,pop);

			enumName=va_arg(va_enum,corbastring);
			enumNameLen=1+strlen(enumName);

			do
			{
				p=va_arg(va_enum,corbastring);

				if (p)
				{
					rptCount++;
					mem._maximum+=(long)sizeof(te->enumId[0]);
					mem._maximum+=(long)(1+strlen(p));
				}

			} while (p);

			mem._maximum+=(long)enumNameLen;

			mem._buffer=SOMMalloc(mem._maximum);
			te=(void *)mem._buffer;

			va_copy(va_enum,pop);

			p=va_arg(va_enum,corbastring);

			RHBOPT_ASSERT(p==enumName);

			te->tag=tk_enum;
			te->align=Alignment(TCKind);
			te->flags=SOMTC_FLAGS_MALLOC;
			te->version=SOMTC_VERSION_1;
			te->rptCount=rptCount;
			te->enumId=SOMTC_serial_alloc(&mem,rptCount*sizeof(te->enumId),Alignment(corbastring));
			te->enumName=SOMTC_serial_alloc(&mem,enumNameLen,1);
			memcpy(te->enumName,enumName,enumNameLen);

			rptCount=0;

			while (rptCount < te->rptCount)
			{
				char *e=va_arg(va_enum,corbastring);
				size_t el=strlen(e)+1;

				p=SOMTC_serial_alloc(&mem,el,1);

				memcpy(p,e,el);

				te->enumId[rptCount]=p;

				rptCount++;
			}

			p=va_arg(va_enum,corbastring);

			RHBOPT_ASSERT(!p);

			tcRetVal=(TypeCode)te;
		}
		break;

	case tk_union:
		{
			short align=SOMTC_min_union_align;
			_IDL_SEQUENCE_octet all={
					sizeof(struct TypeCode_union),
					sizeof(struct TypeCode_union),NULL};
			long rptCount=0;
			struct TypeCode_union *ts;
			size_t unionNameLen;
			size_t stringLengths=0;
			va_list args;
			TypeCode tcSw;
			Environment ev;
			short al;
			SOM_InitEnvironment(&ev);


			/* TypeCode_union
			   TypeCode_union_element[rptCount]
			   char unionName[unionNameLen]
			   char elemNames[....]
			*/

			va_copy(args,pop);

			p=va_arg(args,char *);
			unionNameLen=1+strlen(p);
			stringLengths=unionNameLen;

			tcSw=va_arg(args,TypeCode);
			al=tcAlignment(tcSw,&ev);

			if (al > align) align=al;

			do
			{
				long labelFlag=va_arg(args,long);

				if (labelFlag)
				{
					TypeCode tc;
					size_t n;
					long labelValue=0;

					switch (TypeCode_peek(tcSw)->tag)
					{
					case tk_octet:
					case tk_char:
					case tk_boolean:
					case tk_short:
					case tk_ushort:
						labelValue=va_arg(args,int);
						break;
					case tk_enum:
						labelValue=va_arg(args,TCKind);
						break;
					default:
						labelValue=va_arg(args,long);
						break;
					}

					(void)labelValue;

					p=va_arg(args,char *);
					n=strlen(p)+1;
					tc=va_arg(args,TypeCode);
					al=tcAlignment(tc,&ev);

					if (al > align) align=al;

					stringLengths+=n;
					rptCount++;
				}
				else
				{
					break;
				}

			} while (ev._major==NO_EXCEPTION);

			if (rptCount)
			{
				SOMTC_serial_alloc(&all,
					rptCount*sizeof(ts->mbrs[0]),
					Alignment(struct TypeCode_union_member));
			}

			SOMTC_serial_alloc(&all,stringLengths,1);

			if (SOMTC_serial_init(&ev,&all))
			{
				ts=SOMTC_serial_alloc(&all,sizeof(*ts),Alignment(struct TypeCode_struct));
				ts->tag=tk_union;
				ts->align=align;
				ts->flags=SOMTC_FLAGS_MALLOC;
				ts->version=SOMTC_VERSION_1;
				ts->rptCount=rptCount;
				ts->mbrs=rptCount ?
						SOMTC_serial_alloc(&all,
							rptCount*sizeof(ts->mbrs[0]),
							Alignment(struct TypeCode_union_member)) : NULL;

				va_copy(args,pop);

				p=va_arg(args,char *);

				ts->unionName=SOMTC_serial_alloc(&all,unionNameLen,1);

				memcpy(ts->unionName,p,unionNameLen);

				ts->swTc=va_arg(args,TypeCode);

				rptCount=0;

				while (rptCount < ts->rptCount)
				{
					size_t n;

					ts->mbrs[rptCount].labelFlag=va_arg(args,long);

					RHBOPT_ASSERT(ts->mbrs[rptCount].labelFlag);

					switch (TypeCode_peek(tcSw)->tag)
					{
					case tk_octet:
					case tk_char:
					case tk_boolean:
						ts->mbrs[rptCount].labelValue.c=(char)va_arg(args,int);
						break;
					case tk_short:
					case tk_ushort:
						ts->mbrs[rptCount].labelValue.s=(short)va_arg(args,int);
						break;
					case tk_enum:
						ts->mbrs[rptCount].labelValue.l=va_arg(args,TCKind);
						break;
					default:
						ts->mbrs[rptCount].labelValue.l=va_arg(args,long);
						break;
					}

					p=va_arg(args,char *);
					n=strlen(p)+1;

					RHBOPT_ASSERT(n);

					ts->mbrs[rptCount].mbrName=SOMTC_serial_alloc(&all,n,1);

					memcpy(ts->mbrs[rptCount].mbrName,p,n);

					ts->mbrs[rptCount].mbrTc=va_arg(args,TypeCode);

					RHBOPT_ASSERT(ts->mbrs[rptCount].mbrTc);

					rptCount++;
				}

				tcRetVal=(TypeCode)ts;
			}
			
			SOM_UninitEnvironment(&ev);
		}
		break;

	case tk_string:
		l=va_arg(pop,long);
		if (l)
		{
			struct TypeCode_string *ts=SOMMalloc(sizeof(*ts));
			if (ts)
			{
				ts->tag=tk_string;
				ts->align=Alignment(string);
				ts->version=SOMTC_VERSION_1;
				ts->flags=SOMTC_FLAGS_MALLOC;
				ts->maxBound=l;
			}

			tcRetVal=(TypeCode)ts;
		}
		else
		{
			tcRetVal=(TypeCode)(void *)&TC__string;
		}

		break;

	case tk_sequence:
		{
			TypeCode tc=va_arg(pop,TypeCode);
			long length=va_arg(pop,long);
			struct TypeCode_sequence *ts=SOMMalloc(sizeof(*ts));
			if (ts)
			{
				ts->tag=tk_sequence;
				ts->align=Alignment(GENERIC_SEQUENCE);
				ts->version=SOMTC_VERSION_1;
				ts->flags=SOMTC_FLAGS_MALLOC;
				ts->tc=tc;
				ts->maxBound=length;
			}
			tcRetVal=(TypeCode)ts;
		}
		break;
	case tk_array:
		{
			TypeCode tc=va_arg(pop,TypeCode);
			long length=va_arg(pop,long);
			struct TypeCode_array *ta=SOMMalloc(sizeof(*ta));
			if (ta)
			{
				Environment ev;
				SOM_InitEnvironment(&ev);
				ta->tag=tk_array;
				ta->align=tcAlignment(tc,&ev);
				ta->version=SOMTC_VERSION_1;
				ta->flags=SOMTC_FLAGS_MALLOC;
				ta->tc=tc;
				ta->maxBound=length;
				SOM_UninitEnvironment(&ev);
			}
			tcRetVal=(TypeCode)ta;
		}
		break;
	case tk_self:
		p=va_arg(pop,corbastring);
		if (p && p[0])
		{
			size_t len=strlen(p)+1;
			struct TypeCode_self *to=SOMMalloc(sizeof(*to)+len);
			if (to)
			{
				to->selfName=(void *)&to[1];
				memcpy(to->selfName,p,len);
				to->tag=tk_self;
				to->align=0;
				to->version=SOMTC_VERSION_1;
				to->flags=SOMTC_FLAGS_MALLOC;
			}

			tcRetVal=(void *)to;
		}
		break;
	case tk_foreign:
		{
			char *typeName=va_arg(pop,corbastring);
			char *context=va_arg(pop,corbastring);
			size_t nameLen=strlen(typeName)+1;
			size_t contextLen=strlen(context)+1;
			long length=va_arg(pop,long);
			struct TypeCode_foreign *tf=SOMMalloc(sizeof(*tf)+nameLen+contextLen);

			if (tf)
			{
				tf->align=0;
				tf->tag=tk_foreign;
				tf->flags=SOMTC_FLAGS_MALLOC;
				tf->version=SOMTC_VERSION_1;
				tf->typeName=(void *)(tf+1);
				tf->context=tf->typeName+nameLen;
				tf->length=length;

				if (SOMTC_foreign_is_struct(context))
				{
					tf->align=SOMTC_min_struct_align; /* ONE */
				}
				else
				{
					tf->align=Alignment(string);
				}

				memcpy(tf->typeName,typeName,nameLen);
				memcpy(tf->context,context,contextLen);
			}

			tcRetVal=(TypeCode)tf;
		}
		break;
	case tk_pointer:
		{
			struct TypeCode_pointer *tp=SOMMalloc(sizeof(*tp));
			if (tp)
			{
				tp->tag=tk_pointer;
				tp->align=Alignment(string);
				tp->version=SOMTC_VERSION_1;
				tp->flags=SOMTC_FLAGS_MALLOC;
				tp->tc=va_arg(pop,TypeCode);
			}
			tcRetVal=(TypeCode)tp;
		}
		break;
	case tk_objref:
		p=va_arg(pop,corbastring);
		if (!p || !p[0])
		{
			tcRetVal=(TypeCode)(void *)&TC__Object;
		}
		else
		{
			size_t lenId=strlen(p)+1;
			struct TypeCode_objref *to=SOMMalloc(sizeof(*to)+lenId);
			if (to)
			{
				to->interfaceId=(void *)&to[1];
				memcpy(to->interfaceId,p,lenId);
				to->tag=tk_objref;
				to->align=Alignment(string);
				to->version=SOMTC_VERSION_1;
				to->flags=SOMTC_FLAGS_MALLOC;
			}

			tcRetVal=(TypeCode)to;
		}
		break;
	default:
		somPrintf("tcNewVL(): can't make typecode %d yet\n",(int)tag);
		RHBOPT_ASSERT(!tag);
	}

	return tcRetVal;
}

SOMEXTERN TypeCode SOMLINK tcNew(TCKind tag, ...)
{
	TypeCode t;
	va_list pop;

	va_start(pop,tag);

	t=tcNewVL(tag,pop);

	va_end(pop);

	return t;
}

SOMEXTERN long SOMLINK tcSetZeroOriginEnum(TypeCode t,Environment *ev,boolean value)
{
	if (!t)
	{
		RHBOPT_throw_StExcep(ev,BAD_TYPECODE,BadTypeCode,NO);

		return SOMDERROR_BadTypeCode;
	}

	if (TypeCode_peek(t)->tag != tk_enum)
	{
		RHBOPT_throw_StExcep(ev,BAD_TYPECODE,BadTypeCode,NO);

		return SOMDERROR_BadTypeCode;
	}

	if (value)
	{
		TypeCode_peek(t)->flags|=SOMTC_FLAGS_ZENUM;
	}
	else
	{
		TypeCode_peek(t)->flags&=~SOMTC_FLAGS_ZENUM;
	}

	return 0;
}

SOMEXTERN boolean SOMLINK tcGetZeroOriginEnum(TypeCode t,Environment *ev)
{
	if (!t)
	{
		RHBOPT_throw_StExcep(ev,BAD_TYPECODE,BadTypeCode,NO);

		return 0;
	}

	if (TypeCode_peek(t)->tag != tk_enum)
	{
		RHBOPT_throw_StExcep(ev,BAD_TYPECODE,BadTypeCode,NO);

		return 0;
	}

	return (boolean)((TypeCode_peek(t)->flags & SOMTC_FLAGS_ZENUM) ? 1 : 0);
}


#ifdef SOM_RESOLVE_DATA
	#define SOM_PUBLIC_DATA(x) TypeCode SOMLINK resolve_##x(void) { return (TypeCode)(void *)&x; }

	SOM_PUBLIC_DATA(TC__null)
	SOM_PUBLIC_DATA(TC__void)
	SOM_PUBLIC_DATA(TC__char)
	SOM_PUBLIC_DATA(TC__octet)
	SOM_PUBLIC_DATA(TC__Principal)
	SOM_PUBLIC_DATA(TC__Object)
	SOM_PUBLIC_DATA(TC__long)
	SOM_PUBLIC_DATA(TC__short)
	SOM_PUBLIC_DATA(TC__string)
	SOM_PUBLIC_DATA(TC__ushort)
	SOM_PUBLIC_DATA(TC__ulong)
	SOM_PUBLIC_DATA(TC__float)
	SOM_PUBLIC_DATA(TC__double)
	SOM_PUBLIC_DATA(TC__any)
	SOM_PUBLIC_DATA(TC__boolean)
	SOM_PUBLIC_DATA(TC__TypeCode)
	SOM_PUBLIC_DATA(TC__NamedValue)
	SOM_PUBLIC_DATA(TC__InterfaceDescription)
	SOM_PUBLIC_DATA(TC__FullInterfaceDescription)
	SOM_PUBLIC_DATA(TC__ConstDescription)
	SOM_PUBLIC_DATA(TC__RepositoryDescription)
	SOM_PUBLIC_DATA(TC__ParameterDescription)
	SOM_PUBLIC_DATA(TC__OperationDescription)
	SOM_PUBLIC_DATA(TC__AttributeDescription)
	SOM_PUBLIC_DATA(TC__TypeDescription)
	SOM_PUBLIC_DATA(TC__ModuleDescription)
	SOM_PUBLIC_DATA(TC__ExceptionDescription)

	#undef SOM_PUBLIC_DATA
#endif

#ifdef _WIN32
	#ifndef WM_USER
		#include <windows.h>
	#endif
__declspec(dllexport) BOOL CALLBACK DllMain(HMODULE hInst,DWORD reason,LPVOID extra)
{
 	switch (reason)
    	{
        	case DLL_PROCESS_ATTACH:
				return DisableThreadLibraryCalls(hInst);
	        case DLL_PROCESS_DETACH:
           		return 1;
	}
    
   	return 1;
}
#endif

