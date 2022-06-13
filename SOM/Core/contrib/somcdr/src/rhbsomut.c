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

#ifdef _WIN32
	#include <windows.h>
#endif

#include <rhbsomex.h>
#include <rhbsomut.h>
#include <stdio.h>
#include <somcdr.h>
#include <somtc.h>
#include <somtcnst.h>
#include <stexcep.h>
#include <somdtype.h>
#include <dynany.h>
#include <corba.h>

#ifdef _type
	#undef _type
#endif

#ifdef _value
	#undef _value
#endif

/* #define FREE_DATA_DEBUG */

#ifdef FREE_DATA_DEBUG
static long freeDataDebug;
#endif

#ifdef _WIN32
__declspec(dllexport) BOOL CALLBACK DllMain(HMODULE hInst,DWORD reason,LPVOID extra)
{
	SOM_IgnoreWarning(hInst);
	SOM_IgnoreWarning(extra);

 	switch (reason)
    	{
        	case DLL_PROCESS_ATTACH:
/*				{
					char buf[1024];
					GetModuleFileName(hInst,buf,sizeof(buf));
                    somPrintf("DllMain(%s) called\n",buf);
            		return 1;
				}*/
				return 1;
	        case DLL_PROCESS_DETACH:
            	return 1;
	}

   	return 1;
}
#endif

SOMInitModule_begin(somcdr)

	SOMInitModule_new(SOMCDR_CDROutputStream);
	SOMInitModule_new(SOMCDR_CDRInputStream);

SOMInitModule_end

#ifndef RHBCDR_align_diff
SOMEXTERN unsigned short SOMLINK RHBCDR_align_diff(unsigned long pos,unsigned short by)
{
	if (by > 1)
	{
		unsigned short d=(unsigned short)(pos & (by-1));
		unsigned short result=d ? ((unsigned short)(by-d)) : 0;

		RHBOPT_ASSERT(RHBCDR_align_diff(pos,by)==result)

		return result;
	}

	return 0;
}
#endif

#ifndef RHBCDR_align_pos
SOMEXTERN void SOMLINK RHBCDR_align_pos(unsigned long *pos,short by)
{
	(*pos)+=RHBCDR_align_diff((*pos),by);
}
#endif

SOMEXTERN corbastring SOMLINK RHBCDR_cast_any_to_string(Environment *ev,any *a)
{
	switch (TypeCode_kind(a->_type,ev))
	{
		case tk_string:
			return *((corbastring *)a->_value);
		case tk_enum:
			{
				CORBA_Enum e=*((CORBA_Enum *)a->_value);

				e+=tcGetZeroOriginEnum(a->_type,ev);

				if (e < (CORBA_Enum)tcParmCount(a->_type,ev))
				{
					any b=tcParameter(a->_type,ev,e);

					return RHBCDR_cast_any_to_string(ev,&b);
				}
			}
	}

	TypeCode_print(a->_type,ev);

	RHBOPT_ASSERT(!"cast_any_to_string");

	return NULL;
}

SOMEXTERN long SOMLINK RHBCDR_cast_any_to_long(Environment *ev,any *a)
{
	switch (TypeCode_kind(a->_type,ev))
	{
	case tk_long:
		return *((CORBA_long *)a->_value);
	case tk_short:
		return *((CORBA_short *)a->_value);
	case tk_ushort:
		return *((CORBA_unsigned_short *)a->_value);
	case tk_ulong:
		return *((CORBA_unsigned_long *)a->_value);
	case tk_enum:
		return *((CORBA_Enum *)a->_value);
	case tk_octet:
		return *((CORBA_octet *)a->_value);
	}

	TypeCode_print(a->_type,ev);

	RHBOPT_ASSERT(!"cast_any_to_long");

	return 0;
}

SOMEXTERN unsigned long SOMLINK RHBCDR_cast_any_to_ulong(Environment *ev,any *a)
{
	switch (TypeCode_kind(a->_type,ev))
	{
	case tk_short:
		return *((CORBA_short *)a->_value);
	case tk_ushort:
		return *((CORBA_unsigned_short *)a->_value);
	case tk_long:
		return *((CORBA_long *)a->_value);
	case tk_ulong:
		return *((CORBA_unsigned_long *)a->_value);
	case tk_enum:
		return *((CORBA_Enum *)a->_value);
	}

	RHBOPT_ASSERT(!"cast_any_to_ulong");

	return 0;
}

SOMEXTERN CORBA_Enum SOMLINK RHBCDR_cast_any_to_enum(Environment *ev,any *a)
{
	switch (TypeCode_kind(a->_type,ev))
	{
	case tk_short:
		return *((CORBA_short *)a->_value);
	case tk_ushort:
		return *((CORBA_unsigned_short *)a->_value);
	case tk_long:
		return *((CORBA_long *)a->_value);
	case tk_ulong:
		return *((CORBA_unsigned_long *)a->_value);
	case tk_enum:
		return *((CORBA_Enum *)a->_value);
	}

	RHBOPT_ASSERT(!"cast_any_to_enum");

	return 0;
}

CORBA_TypeCode SOMLINK RHBCDR_cast_any_to_TypeCode(Environment *ev,any *a)
{
	if (tk_TypeCode==TypeCode_kind(a->_type,ev))
	{
		return *((CORBA_TypeCode *)a->_value);
	}

	RHBOPT_ASSERT(!"cast_any_to_TypeCode");

	return NULL;

}

SOMObject SOMSTAR SOMLINK RHBCDR_cast_any_to_SOMObject(Environment *ev,any *a)
{
	if (tk_objref==TypeCode_kind(a->_type,ev))
	{
		return *((SOMObject SOMSTAR *)a->_value);
	}

	RHBOPT_ASSERT(!"cast_any_to_SOMObject");

	return NULL;
}


/* #define FREE_DATA_DEBUG */

#include <rhbunion.h>
SOMEXTERN unsigned long SOMLINK RHBCDR_free_data(
		Environment *ev,
		SOMCDR_unmarshal_filter *filter,
		TypeCode _type,
		void *_value)
{
	if (!ev)
	{
		RHBOPT_ASSERT(!"no ev in RHBCDR_free_data\n");

		return SOMDERROR_BadEnvironment;
	}

	if (ev->_major)
	{
		RHBOPT_ASSERT(!"ev->_major set in RHBCDR_free_data\n");

		return SOMDERROR_BadEnvironment;
	}

	if (_value)
	{
		TCKind kind=TypeCode_kind(_type,ev);

		switch (kind)
		{
		case tk_octet:
		case tk_ulong:
		case tk_long:
		case tk_char:
		case tk_boolean:
		case tk_short:
		case tk_ushort:
		case tk_void:
		case tk_null:
		case tk_float:
		case tk_double:
		case tk_enum:
			break;
		case tk_struct:
			{
				int i=TypeCode_param_count(_type,ev);
				int j=1;
				octet *struct_base=_value;	
				unsigned long struct_offset=0;

#ifdef FREE_DATA_DEBUG
				{
					any a=TypeCode_parameter(_type,ev,0);
					somPrefixLevel(freeDataDebug);
					somPrintf("struct==%s\n",*((corbastring *)a._value));
					InterlockedIncrement(&freeDataDebug);
				}
#endif

				while (j < i)
				{

#ifdef FREE_DATA_DEBUG
					any a=TypeCode_parameter(_type,ev,j);
					int n=somPrintf("element==%s\n",*((corbastring *)a._value));
#else
					any
#endif
					a=TypeCode_parameter(_type,ev,++j);
					TypeCode type=RHBCDR_cast_any_to_TypeCode(ev,&a);
					short align=TypeCode_alignment(type,ev);
					size_t k=TypeCode_size(type,ev);

					RHBCDR_align_pos(&struct_offset,align);

					if (k >= sizeof(void *))
					{
						/* only free data if might contain a pointer */
						RHBCDR_free_data(ev,filter,type,struct_base+struct_offset);
					}

					struct_offset+=(unsigned long)k;

					j++;
				}

#ifdef FREE_DATA_DEBUG
				{
					any a=TypeCode_parameter(_type,ev,0);
					somPrefixLevel(freeDataDebug);
					somPrintf("end of struct %s\n",*((corbastring *)a._value));
					InterlockedDecrement(&freeDataDebug);
				}
#endif
			}

			break;
		case tk_sequence:
			{
				any a=TypeCode_parameter(_type,ev,0);
				_IDL_SEQUENCE_octet *value=_value;
				_IDL_SEQUENCE_octet op=*value;
				unsigned int i=op._length;
				TypeCode element_type=RHBCDR_cast_any_to_TypeCode(ev,&a);

				value->_length=0;
				value->_maximum=0;
				value->_buffer=NULL;

				if (i)
				{
					unsigned int k=TypeCode_size(element_type,ev);

					if (k >= sizeof(void *))
					{
						octet *o=op._buffer;
						/* only free up data if it might contain a pointer */

						while ((i--) && !ev->_major)
						{
							RHBCDR_free_data(ev,filter,element_type,o);

							o+=k;
						}
					}
				}

				if (op._maximum)
				{
					SOMFree(op._buffer);
				}
			}

			break;

		case tk_array:
			{
				any a0=TypeCode_parameter(_type,ev,0);  /* type of thing */
				any a1=TypeCode_parameter(_type,ev,1);
				TypeCode element_type=RHBCDR_cast_any_to_TypeCode(ev,&a0);
				long len=RHBCDR_cast_any_to_long(ev,&a1);
				unsigned int k=TypeCode_size(element_type,ev);

				if (k >= sizeof(void *))
				{
					octet *o=_value;
					while ((len--) && !ev->_major)
					{
						RHBCDR_free_data(ev,filter,element_type,o);
						o+=k;
					}
				}
			}

			break;

		case tk_TypeCode:
			{
				TypeCode *tcp=(TypeCode *)_value;
				TypeCode tc=*tcp;
				*tcp=NULL;
				if (tc) TypeCode_free(tc,ev);
			}
			break;
		case tk_string:
			{
				corbastring *sp=(corbastring *)_value;
				corbastring s=*sp;
				*sp=NULL;
				if (s) SOMFree(s);
			}
			break;
		case tk_any:
			{
				any *pany=(any *)_value;

				if (pany)
				{
					any value=*pany;
					pany->_type=NULL;
					pany->_value=NULL;
					
					if (value._type)
					{
						if (value._value)
						{
							RHBCDR_free_data(ev,filter,value._type,value._value);

							SOMFree(value._value);
						}

						TypeCode_free(value._type,ev);
					}
				}
			}

			break;
		case tk_Principal:
		case tk_objref:
			{
				SOMObject SOMSTAR *sp=(SOMObject SOMSTAR *)_value;
				SOMObject SOMSTAR obj=*sp;
				*sp=NULL;
				if (obj)
				{
#ifdef USE_APPLE_SOM
					if (obj) somReleaseObjectReference(obj);
#else
					if (obj) SOMObject_somFree(obj);
#endif
				}
			}
			break;
		case tk_pointer:
			{
				RHBOPT_ASSERT(!"have not written free pointer yet");
			}
			break;
		case tk_union:
			{
				any a=RHBUNION_get_value(ev,_type,_value);

				return RHBCDR_free_data(ev,filter,a._type,a._value);
			}
			break;
		case tk_foreign:
			filter->free_object(filter,ev,_value,_type);
			break;
		default:
			{
				somPrintf("Can't free data of type %d\n",(int)kind);
				TypeCode_print(_type,ev);

				return SOMDERROR_BadTypeCode;
			}
		}
	}

	return 0;
}

RHBOPT_cleanup_begin(free_id_cleanup,pv)

	somId *data=pv;
	if (*data) SOMFree(*data);

RHBOPT_cleanup_end

SOMEXTERN void SOMLINK RHBCDR_copy_from(TypeCode type,Environment *ev,void *target,void *source)
{
#define COPY_ONE(x)    { ((x *)target)[0]=((x *)source)[0]; } 

	switch (TypeCode_kind(type,ev))
	{
	case tk_octet:		COPY_ONE(octet)				break;
	case tk_char:		COPY_ONE(char)				break;
	case tk_boolean:	COPY_ONE(boolean)			break;
	case tk_float:		COPY_ONE(float)				break;
	case tk_double:		COPY_ONE(double)			break;
	case tk_long:		COPY_ONE(long)				break;
	case tk_short:		COPY_ONE(short)				break;
	case tk_ulong:		COPY_ONE(unsigned long)		break;
	case tk_ushort:		COPY_ONE(unsigned short)	break;
	case tk_TypeCode:
		{
			TypeCode *tp=source;
			TypeCode *tp2=target;

			*tp2=TypeCode_copy(*tp,ev);
		}
		break;
	case tk_string:
		{
			corbastring *pstr1=source;
			corbastring *pstr2=target;

			if (*pstr1)
			{
				size_t l=strlen(*pstr1)+1;
				*pstr2=SOMMalloc(l);
				memcpy(*pstr2,*pstr1,l);
			}
			else
			{
				*pstr2=NULL;
			}
		}
		break;
	case tk_any:
		{
			any *tgt=target;
			any *src=source;

			tgt->_value=NULL;
			tgt->_type=TypeCode_copy(src->_type,ev);

			if (src->_value)
			{
				size_t n=TypeCode_size(tgt->_type,ev);
				if (n)
				{
					tgt->_value=SOMCalloc(n,1);
				}
				RHBCDR_copy_from(tgt->_type,ev,tgt->_value,src->_value);
			}
		}
		break;
	case tk_struct:
		{
			long count=TypeCode_param_count(type,ev);
			int i=0;
#ifdef _DEBUG
			any src,tgt;
#endif
			octet *source_p=source;
			octet *target_p=target;
			unsigned long offset=0;

#ifdef _DEBUG
			src._value=source;
			src._type=type;
			tgt._value=target;
			tgt._type=type;
#endif

			count--;
			count>>=1;
			
			while (i < count)
			{
				any etype=TypeCode_parameter(type,ev,(i << 1)+2);
				TypeCode etc=((TypeCode *)etype._value)[0];
#ifdef _DEBUG
/*				unsigned short al=TypeCode_alignment(etc,ev);*/
				any name=TypeCode_parameter(type,ev,(i << 1)+1);
				char *p=RHBCDR_cast_any_to_string(ev,&name);
				any from=RHBCDR_get_named_element(ev,&src,p);
				any to=RHBCDR_get_named_element(ev,&tgt,p);
#endif
				short align=TypeCode_alignment(etc,ev);
				RHBCDR_align_pos(&offset,align);

				RHBOPT_ASSERT(to._value==(target_p+offset))
				RHBOPT_ASSERT(from._value==(source_p+offset))
				RHBOPT_ASSERT(TypeCode_equal(etc,ev,to._type))

				RHBCDR_copy_from(etc,ev,target_p+offset,source_p+offset);

				offset+=TypeCode_size(etc,ev);

				i++;
			}
		}

		break;
	case tk_sequence:
		{
			_IDL_SEQUENCE_octet *from=source;
			_IDL_SEQUENCE_octet *to=target;
			any a=TypeCode_parameter(type,ev,0);
			TypeCode tc=RHBCDR_cast_any_to_TypeCode(ev,&a);
			size_t n=TypeCode_size(tc,ev);
			to->_buffer=NULL;
			to->_length=from->_length;
			to->_maximum=from->_length;
			if (to->_length)
			{
				size_t m=to->_length,o=0;
				to->_buffer=SOMCalloc(to->_length,n);
				while ((m--) && !ev->_major)
				{
					RHBCDR_copy_from(tc,ev,to->_buffer+o,from->_buffer+o);
					o+=n;
				}
			}
		}
		break;
	case tk_objref:
	case tk_Principal:
		{
			SOMObject SOMSTAR *to=target;
			SOMObject SOMSTAR *from=source;
			SOMObject SOMSTAR src=*from;

			*to=NULL;
			if (src)
			{
#ifdef SOMObject_somDuplicateReference
				*to=SOMObject_somDuplicateReference(src);
#else
				if (SOMObject_somIsA(src,_SOMRefObject))
				{
					*to=SOMRefObject_somDuplicateReference(src);
				}
				else
				{
					somId id=somIdFromString("duplicate");
					RHBOPT_cleanup_push(free_id_cleanup,&id);

					somva_SOMObject_somDispatch(src,target,id,src,ev);

					RHBOPT_cleanup_pop();
				}
#endif
				if (!to)
				{
					RHBOPT_throw_StExcep(ev,DATA_CONVERSION,
						UnexpectedNULL,MAYBE);
				}
			}
		}
		break;
	default:
		{
			char *p=NULL;
			somPrintf("Can't copy data..:");
			TypeCode_print(type,ev);
			somPrintf("\n");

			*p=0;
		}
		break;
	}
}

SOMEXTERN any SOMLINK RHBCDR_get_named_element(Environment *ev,any *a,char *str)
{
	any res={NULL,NULL};

	RHBOPT_ASSERT(ev->_major==NO_EXCEPTION);
	RHBOPT_ASSERT(a);
	RHBOPT_ASSERT(str);

	if (TypeCode_kind(a->_type,ev)==tk_struct)
	{
		long l=TypeCode_param_count(a->_type,ev);
		long j=1; /* dont need name of struct */
		octet *struct_base=a->_value;
		unsigned long struct_offset=0;

		while (j < l)
		{
			any name_any=TypeCode_parameter(a->_type,ev,j++);
			char *element_name=RHBCDR_cast_any_to_string(ev,&name_any);
			any type_any=TypeCode_parameter(a->_type,ev,j++);
			TypeCode type=RHBCDR_cast_any_to_TypeCode(ev,&type_any);
			short align=TypeCode_alignment(type,ev);

			RHBCDR_align_pos(&struct_offset,align);

			/* or should this be stricmp/strcasecmp ? */

			if (0==strcmp(element_name,str))
			{
				/* this is the one we want... */

				/* align it */

				res._type=type;
				res._value=struct_base+struct_offset;

				break;
			}

			/* then add the length of this parameter */

			struct_offset+=TypeCode_size(type,ev);
		}
	}
	else
	{
		somPrintf("Can't extract '%s' from ",str);
		tcPrint(a->_type,ev);
	}

	return res;
}

SOMEXTERN any SOMLINK RHBCDR_get_nth_element(Environment *ev,any *a,int num)
{
	any res;
	TCKind kind;

	res._type=TC_void;
	res._value=0;

	kind=TypeCode_kind(a->_type,ev);

	switch (kind)
	{
	case tk_sequence:
		{
			_IDL_SEQUENCE_octet *o;
			long i,j;

			res=TypeCode_parameter(a->_type,ev,0);

			res._type=RHBCDR_cast_any_to_TypeCode(ev,&res);

			i=TypeCode_size(res._type,ev);

			o=(_IDL_SEQUENCE_octet *)a->_value;

			j=i * num;

			res._value=&o->_buffer[j];
		}
		break;
	case tk_array:
		break;
	}

	return res;
}

SOMEXTERN int SOMLINK RHBCDR_count_elements(Environment *ev,any *a)
{
	switch (TypeCode_kind(a->_type,ev))
	{
	case tk_array:
		{
			any b=TypeCode_parameter(a->_type,ev,2);

			return RHBCDR_cast_any_to_long(ev,&b);
		}
		break;
	case tk_sequence:
		{
			_IDL_SEQUENCE_octet *o=(_IDL_SEQUENCE_octet *)a->_value;

			return o->_length;
		}
	}

	return 0;
}

