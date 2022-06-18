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

#define SOM_Module_rhbsomus_Source

#define RHBSOMUS_CDRStreamIO_Class_Source

#include <rhbsomex.h>
#include <somref.h>
#include <rhbsomut.h>
#include <somdtype.h>
#include <somd.h>
#include <rhbsomid.h>

#ifdef Contained
	/* this is to cope with Apple SOM */
	#ifndef CORBA_Contained
		#define CORBA_Contained  SOMObject
	#endif
	#ifndef CORBA_InterfaceDef
		#define CORBA_InterfaceDef  SOMObject
	#endif
#endif

#include <corba.h>
#include <rhbsomus.ih>

#define CHECK_WRITE			 \
	if (!ev) return;	\
	if (ev->_major) return; \
	if (!somThis->output_stream) { RHBOPT_throw_StExcep(ev,MARSHAL,UnexpectedNULL,MAYBE); return ; }


#define CHECK_READ(__rv__)   \
	if (!ev) return __rv__; \
	if (ev->_major) return __rv__; \
	if (!somThis->input_stream) { RHBOPT_throw_StExcep(ev,MARSHAL,UnexpectedNULL,MAYBE); return __rv__; }

SOM_Scope void SOMLINK rhbsomus_cdrstrio_write_octet(
		RHBSOMUS_CDRStreamIO SOMSTAR somSelf,
		Environment *ev,octet value)
{
	RHBSOMUS_CDRStreamIOData *somThis=RHBSOMUS_CDRStreamIOGetData(somSelf);

	CHECK_WRITE

	CORBA_DataOutputStream_write_octet(somThis->output_stream,ev,value);
}

SOM_Scope void SOMLINK rhbsomus_cdrstrio_write_string(
		RHBSOMUS_CDRStreamIO SOMSTAR somSelf,
		Environment *ev,corbastring value)
{
	RHBSOMUS_CDRStreamIOData *somThis=RHBSOMUS_CDRStreamIOGetData(somSelf);

	CHECK_WRITE

	CORBA_DataOutputStream_write_string(somThis->output_stream,ev,value);
}


SOM_Scope void SOMLINK rhbsomus_cdrstrio_write_long(
		RHBSOMUS_CDRStreamIO SOMSTAR somSelf,
		Environment *ev,long value)
{
	RHBSOMUS_CDRStreamIOData *somThis=RHBSOMUS_CDRStreamIOGetData(somSelf);
	CHECK_WRITE
	CORBA_DataOutputStream_write_long(somThis->output_stream,ev,value);
}



SOM_Scope void SOMLINK rhbsomus_cdrstrio_write_short(
		RHBSOMUS_CDRStreamIO SOMSTAR somSelf,
		Environment *ev,short value)
{
	RHBSOMUS_CDRStreamIOData *somThis=RHBSOMUS_CDRStreamIOGetData(somSelf);
	CHECK_WRITE
	CORBA_DataOutputStream_write_short(somThis->output_stream,ev,value);
}

SOM_Scope void SOMLINK rhbsomus_cdrstrio_write_float(
		RHBSOMUS_CDRStreamIO SOMSTAR somSelf,
		Environment *ev,float value)
{
	RHBSOMUS_CDRStreamIOData *somThis=RHBSOMUS_CDRStreamIOGetData(somSelf);
	CHECK_WRITE
	CORBA_DataOutputStream_write_float(somThis->output_stream,ev,value);
}


SOM_Scope void SOMLINK rhbsomus_cdrstrio_write_boolean(
		RHBSOMUS_CDRStreamIO SOMSTAR somSelf,
		Environment *ev,boolean value)
{
	RHBSOMUS_CDRStreamIOData *somThis=RHBSOMUS_CDRStreamIOGetData(somSelf);
	CHECK_WRITE
	CORBA_DataOutputStream_write_boolean(somThis->output_stream,ev,value);
}

SOM_Scope void SOMLINK rhbsomus_cdrstrio_write_unsigned_long(
		RHBSOMUS_CDRStreamIO SOMSTAR somSelf,
		Environment *ev,unsigned long value)
{
	RHBSOMUS_CDRStreamIOData *somThis=RHBSOMUS_CDRStreamIOGetData(somSelf);
	CHECK_WRITE
	CORBA_DataOutputStream_write_ulong(somThis->output_stream,ev,value);
}


SOM_Scope void SOMLINK rhbsomus_cdrstrio_write_double(
		RHBSOMUS_CDRStreamIO SOMSTAR somSelf,
		Environment *ev,double value)
{
	RHBSOMUS_CDRStreamIOData *somThis=RHBSOMUS_CDRStreamIOGetData(somSelf);
	CHECK_WRITE
	CORBA_DataOutputStream_write_double(somThis->output_stream,ev,value);
}

SOM_Scope void SOMLINK rhbsomus_cdrstrio_write_unsigned_short(
		RHBSOMUS_CDRStreamIO SOMSTAR somSelf,
		Environment *ev,unsigned short value)
{
	RHBSOMUS_CDRStreamIOData *somThis=RHBSOMUS_CDRStreamIOGetData(somSelf);
	CHECK_WRITE
	CORBA_DataOutputStream_write_ushort(somThis->output_stream,ev,value);
}


SOM_Scope void SOMLINK rhbsomus_cdrstrio_write_char(
		RHBSOMUS_CDRStreamIO SOMSTAR somSelf,
		Environment *ev,char value)
{
	RHBSOMUS_CDRStreamIOData *somThis=RHBSOMUS_CDRStreamIOGetData(somSelf);
	CHECK_WRITE
	CORBA_DataOutputStream_write_char(somThis->output_stream,ev,value);
}

SOM_Scope void SOMLINK rhbsomus_cdrstrio_write_object(
		RHBSOMUS_CDRStreamIO SOMSTAR somSelf,
		Environment *ev,CosStream_Streamable SOMSTAR value)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(value)

	RHBOPT_throw_StExcep(ev,MARSHAL,NotImplemented,MAYBE);
}

SOM_Scope SOMObject SOMSTAR SOMLINK rhbsomus_cdrstrio_read_object(
		RHBSOMUS_CDRStreamIO SOMSTAR somSelf,
		Environment *ev,
		CosLifeCycle_FactoryFinder SOMSTAR ff,
		CosStream_Streamable SOMSTAR obj)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ff)
	RHBOPT_unused(obj)

	RHBOPT_throw_StExcep(ev,MARSHAL,NotImplemented,MAYBE);

	return NULL;
}

SOM_Scope char * SOMLINK rhbsomus_cdrstrio_read_string(
		RHBSOMUS_CDRStreamIO SOMSTAR somSelf,
		Environment *ev)
{
	RHBSOMUS_CDRStreamIOData *somThis=RHBSOMUS_CDRStreamIOGetData(somSelf);

	CHECK_READ(NULL)

	return CORBA_DataInputStream_read_string(somThis->input_stream,ev);
}

SOM_Scope unsigned long SOMLINK rhbsomus_cdrstrio_read_unsigned_long(
		RHBSOMUS_CDRStreamIO SOMSTAR somSelf,
		Environment *ev)
{
	RHBSOMUS_CDRStreamIOData *somThis=RHBSOMUS_CDRStreamIOGetData(somSelf);

	CHECK_READ(0)

	return CORBA_DataInputStream_read_ulong(somThis->input_stream,ev);
}

SOM_Scope unsigned short SOMLINK rhbsomus_cdrstrio_read_unsigned_short(
		RHBSOMUS_CDRStreamIO SOMSTAR somSelf,
		Environment *ev)
{
	RHBSOMUS_CDRStreamIOData *somThis=RHBSOMUS_CDRStreamIOGetData(somSelf);
	CHECK_READ(0)
	return CORBA_DataInputStream_read_ushort(somThis->input_stream,ev);
}

SOM_Scope  long SOMLINK rhbsomus_cdrstrio_read_long(
		RHBSOMUS_CDRStreamIO SOMSTAR somSelf,
		Environment *ev)
{
	RHBSOMUS_CDRStreamIOData *somThis=RHBSOMUS_CDRStreamIOGetData(somSelf);
	CHECK_READ(0)
	return CORBA_DataInputStream_read_long(somThis->input_stream,ev);
}

SOM_Scope short SOMLINK rhbsomus_cdrstrio_read_short(
		RHBSOMUS_CDRStreamIO SOMSTAR somSelf,
		Environment *ev)
{
	RHBSOMUS_CDRStreamIOData *somThis=RHBSOMUS_CDRStreamIOGetData(somSelf);
	CHECK_READ(0)
	return CORBA_DataInputStream_read_short(somThis->input_stream,ev);
}

SOM_Scope boolean SOMLINK rhbsomus_cdrstrio_read_boolean(
		RHBSOMUS_CDRStreamIO SOMSTAR somSelf,
		Environment *ev)
{
	RHBSOMUS_CDRStreamIOData *somThis=RHBSOMUS_CDRStreamIOGetData(somSelf);
	CHECK_READ(0)
	return CORBA_DataInputStream_read_boolean(somThis->input_stream,ev);
}

SOM_Scope octet SOMLINK rhbsomus_cdrstrio_read_octet(
		RHBSOMUS_CDRStreamIO SOMSTAR somSelf,
		Environment *ev)
{
	RHBSOMUS_CDRStreamIOData *somThis=RHBSOMUS_CDRStreamIOGetData(somSelf);
	CHECK_READ(0)
	return CORBA_DataInputStream_read_octet(somThis->input_stream,ev);
}

SOM_Scope float SOMLINK rhbsomus_cdrstrio_read_float(
		RHBSOMUS_CDRStreamIO SOMSTAR somSelf,
		Environment *ev)
{
	RHBSOMUS_CDRStreamIOData *somThis=RHBSOMUS_CDRStreamIOGetData(somSelf);
	CHECK_READ(((float)0))
	return CORBA_DataInputStream_read_float(somThis->input_stream,ev);
}

SOM_Scope double SOMLINK rhbsomus_cdrstrio_read_double(
		RHBSOMUS_CDRStreamIO SOMSTAR somSelf,
		Environment *ev)
{
	RHBSOMUS_CDRStreamIOData *somThis=RHBSOMUS_CDRStreamIOGetData(somSelf);
	CHECK_READ(0)
	return CORBA_DataInputStream_read_double(somThis->input_stream,ev);
}


SOM_Scope char SOMLINK rhbsomus_cdrstrio_read_char(
		RHBSOMUS_CDRStreamIO SOMSTAR somSelf,
		Environment *ev)
{
	RHBSOMUS_CDRStreamIOData *somThis=RHBSOMUS_CDRStreamIOGetData(somSelf);
	CHECK_READ(0)
	return CORBA_DataInputStream_read_char(somThis->input_stream,ev);
}

static char *RHBSOMUS_get_foreign_marshal_info(TypeCode tc,Environment *ev)
{
	any a=TypeCode_parameter(tc,ev,1);
	char *p;

	if (ev->_major) return 0;

	p=RHBCDR_cast_any_to_string(ev,&a);

	if (ev->_major) return 0;

	/* get past language bits */

	while (*p)
	{
		if (*p++==',')
		{
			break;
		}
	}

	while (*p)
	{
		int i=(int)strlen(p);

		if (i >= 6)
		{
			if (!memcmp(p,"struct",6))
			{
				if (p[6]==0) return 0;

				if (p[6]==',')
				{
					p+=7;
					continue;
				}
			}
		}

		if (i >= 7)
		{
			if (!memcmp(p,"pointer",7))
			{
				if (p[7]==0) return 0;

				if (p[7]==',')
				{
					p+=8;
					continue;
				}
			}
		}

		return p;
	}

	return 0;
}

static void RHBCDR_call_foreign_marshaler(
	Environment *ev,
	char *method,
	void *data,
	unsigned long function,
	CosStream_StreamIO SOMSTAR stream)
{
	int i=(int)strlen(method);
	SOMD_marshaling_direction_t direction=0;

	if (i >= 6)
	{
		if (!memcmp("opaque",method,6))
		{
			RHBOPT_ASSERT(!"deal with opaque foreigns\n");
			return;
		}

		if (!memcmp("static",method,6))
		{
			/* deal with statics */
			void (SOMLINK *func)(
					void *,char *,
					SOMD_marshaling_direction_t,
					SOMD_marshaling_op_t,
					CosStream_StreamIO SOMSTAR,
					Environment *)=NULL;

			method+=6;
			
			if (!strcmp(method,"(somIdMarshal,0)"))
			{
				func=somIdMarshal;
			}

			if (func)
			{
				func(0,data,direction,function,stream,ev);
			}
			else
			{
				RHBOPT_throw_StExcep(ev,MARSHAL,UnexpectedNULL,NO);
			}

			return;
		}
	}

	somPrintf("failed trying to deal with %s\n",method);
	RHBOPT_ASSERT(!"marshal foreign\n");
}

SOMEXTERN unsigned long SOMLINK RHBCDR_call_foreign_handler(
				Environment *ev,
				CosStream_StreamIO SOMSTAR stream,
				void *_value,TypeCode _type,
				unsigned long function)
{
	/* deals with foreign data types */
	char *info=0;

	if (ev->_major) return SOMDERROR_BadEnvironment;

	if (tk_foreign != TypeCode_kind(_type,ev))
	{
		RHBOPT_throw_StExcep(ev,MARSHAL,BadTypeCode,MAYBE);

		return SOMDERROR_BadTypeCode;
	}

	info=RHBSOMUS_get_foreign_marshal_info(_type,ev);

	if (!info)
	{
		RHBOPT_throw_StExcep(ev,MARSHAL,BadTypeCode,MAYBE);

		return SOMDERROR_BadTypeCode;
	}

	RHBCDR_call_foreign_marshaler(
			ev,
			info,
			_value,
			function,
			stream);

	if (ev->_major) return SOMDERROR_Marshal;

	return 0;
}

SOM_Scope SOMObject SOMSTAR SOMLINK rhbsomus_cdrstrio__get_output_stream(
		RHBSOMUS_CDRStreamIO SOMSTAR somSelf,
		Environment *ev)
{
	RHBSOMUS_CDRStreamIOData *somThis=RHBSOMUS_CDRStreamIOGetData(somSelf);
	SOM_IgnoreWarning(ev);
	return somThis->output_stream;
}


SOM_Scope SOMObject SOMSTAR SOMLINK rhbsomus_cdrstrio__get_input_stream(
		RHBSOMUS_CDRStreamIO SOMSTAR somSelf,
		Environment *ev)
{
	RHBSOMUS_CDRStreamIOData *somThis=RHBSOMUS_CDRStreamIOGetData(somSelf);
	SOM_IgnoreWarning(ev);
	return somThis->input_stream;
}


SOM_Scope void SOMLINK rhbsomus_cdrstrio__set_input_stream(
		RHBSOMUS_CDRStreamIO SOMSTAR somSelf,
		Environment *ev,
		SOMObject SOMSTAR input_stream)
{
	RHBSOMUS_CDRStreamIOData *somThis=RHBSOMUS_CDRStreamIOGetData(somSelf);
	SOM_IgnoreWarning(ev);
	somThis->input_stream=input_stream;
}


SOM_Scope void SOMLINK rhbsomus_cdrstrio__set_output_stream(
		RHBSOMUS_CDRStreamIO SOMSTAR somSelf,
		Environment *ev,
		SOMObject SOMSTAR output_stream)
{
	RHBSOMUS_CDRStreamIOData *somThis=RHBSOMUS_CDRStreamIOGetData(somSelf);
	SOM_IgnoreWarning(ev);
	somThis->output_stream=output_stream;
}

#ifdef RHBSOMUS_CDRStreamIO_BeginInitializer_somDefaultInit
SOM_Scope void SOMLINK rhbsomus_cdrstrio_somDefaultInit(
		RHBSOMUS_CDRStreamIO SOMSTAR somSelf,
		somInitCtrl *ctrl)
{
	somInitCtrl globalCtrl;
	RHBSOMUS_CDRStreamIOData *somThis;
	somBooleanVector myMask;
	RHBSOMUS_CDRStreamIO_BeginInitializer_somDefaultInit
	RHBSOMUS_CDRStreamIO_Init_CosStream_StreamIO_somDefaultInit(somSelf,ctrl);
	RHBOPT_unused(somThis);
}
#else
SOM_Scope void SOMLINK rhbsomus_cdrstrio_somInit(
 		RHBSOMUS_CDRStreamIO SOMSTAR somSelf)
{
	RHBSOMUS_CDRStreamIO_parent_CosStream_StreamIO_somInit(somSelf);
	SOM_IgnoreWarning(somSelf);
}
#endif

#ifdef RHBSOMUS_CDRStreamIO_BeginDestructor
SOM_Scope void SOMLINK rhbsomus_cdrstrio_somDestruct(
 		RHBSOMUS_CDRStreamIO SOMSTAR somSelf,
		boolean doFree,
		somDestructCtrl *ctrl)
{
	somDestructCtrl globalCtrl;
	RHBSOMUS_CDRStreamIOData *somThis;
	somBooleanVector myMask;
	RHBSOMUS_CDRStreamIO_BeginDestructor
	RHBOPT_ASSERT(!somThis->output_stream)
	RHBOPT_ASSERT(!somThis->input_stream)
	RHBOPT_unused(somThis);
	RHBSOMUS_CDRStreamIO_EndDestructor
}
#else
SOM_Scope void SOMLINK rhbsomus_cdrstrio_somUninit(
 		RHBSOMUS_CDRStreamIO SOMSTAR somSelf)
{
#ifdef _DEBUG
	RHBSOMUS_CDRStreamIOData *somThis=RHBSOMUS_CDRStreamIOGetData(somSelf);
	RHBOPT_ASSERT(!somThis->output_stream)
	RHBOPT_ASSERT(!somThis->input_stream)
#endif
	RHBSOMUS_CDRStreamIO_parent_CosStream_StreamIO_somUninit(somSelf);
}
#endif
