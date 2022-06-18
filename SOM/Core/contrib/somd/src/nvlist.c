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

#ifndef SOM_Module_nvlist_Source
#define SOM_Module_nvlist_Source
#endif
#define NVList_Class_Source

#include "nvlist.ih"

static void NamedValue_set(
		 NamedValue *somThis,
		 Environment *ev,
         Identifier item_name, 
         TypeCode item_type, 
         void* value, long value_len, 
         Flags item_flags)
{
	TypeCode old_type=somThis->argument._type;

	if (somThis->name != item_name)
	{
		if (somThis->name) SOMFree(somThis->name);
		somThis->name=somd_dupl_string(item_name);
	}

	if (item_type != old_type)
	{
		if (item_type)
		{
			somThis->argument._type=TypeCode_copy(item_type,ev);
		}
		else
		{
			somThis->argument._type=NULL;
		}
	}

	if (somThis->arg_modes & IN_COPY_VALUE)
	{
		SOMD_FreeType(ev,somThis->argument._value,old_type);
		SOMFree(somThis->argument._value);
	}

	if (item_flags & IN_COPY_VALUE)
	{
		size_t i=tcSize(somThis->argument._type,ev);

		somThis->argument._value=SOMMalloc(i);
		memset(somThis->argument._value,0,i);

		RHBCDR_copy_from(somThis->argument._type,ev,somThis->argument._value,value);
	}
	else
	{
		somThis->argument._value=value;
	}

	somThis->len=value_len;
	somThis->arg_modes=item_flags;

	if (old_type) 
	{
		if (item_type != old_type)
		{
			TypeCode_free(old_type,ev);
		}
	}
}
	
static void NVList_set_count(
		NVList SOMSTAR somSelf,
		Environment *ev,
		unsigned long _length)
{
    NVListData *somThis = NVListGetData(somSelf);

	RHBOPT_unused(ev)

	if (_length > somThis->contents._length)
	{
		if (_length > somThis->contents._maximum)
		{
			unsigned int i=0;
			NamedValue *_buffer=SOMMalloc(sizeof(*_buffer) * _length);

			while (i < somThis->contents._length)
			{
				_buffer[i].name=somThis->contents._buffer[i].name;
				_buffer[i].argument._type=somThis->contents._buffer[i].argument._type;
				_buffer[i].argument._value=somThis->contents._buffer[i].argument._value;
				_buffer[i].len=somThis->contents._buffer[i].len;
				_buffer[i].arg_modes=somThis->contents._buffer[i].arg_modes;
				i++;
			}

			while (i < _length)
			{
				_buffer[i].name=NULL;
				_buffer[i].argument._type=NULL;
				_buffer[i].argument._value=NULL;
				_buffer[i].len=0;
				_buffer[i].arg_modes=0;
				i++;
			}

			if (somThis->contents._maximum)
			{
				SOMFree(somThis->contents._buffer);
			}

			somThis->contents._maximum=_length;

			somThis->contents._buffer=_buffer;
		}

		somThis->contents._length=_length;
	}
	else
	{
		if (_length < somThis->contents._length)
		{
			somThis->contents._length=_length;
		}
	}
}

SOM_Scope ORBStatus  SOMLINK nvlist_add_item(
		NVList SOMSTAR somSelf, 
		Environment *ev, 
		Identifier item_name, 
		TypeCode item_type, 
		void* value, long value_len, 
		Flags item_flags)
{
    NVListData *somThis = NVListGetData(somSelf);
	unsigned int i=somThis->contents._length;

	NVList_set_count(somSelf,ev,i+1);

	NamedValue_set(&somThis->contents._buffer[i],
			ev,
			item_name,
			item_type,
			value,
			value_len,
			item_flags);

    return 0;
}

SOM_Scope ORBStatus  SOMLINK nvlist_free_memory(
		NVList SOMSTAR somSelf, 
		Environment *ev)
{
    NVListData *somThis = NVListGetData(somSelf);

	if (somThis->contents._maximum)
	{
		NamedValue *p=somThis->contents._buffer;
		unsigned long i=somThis->contents._length;

		while (i--)
		{
			NamedValue_set(p,ev,0,0,0,0,0);
			p++;
		}

		SOMFree(somThis->contents._buffer);
		somThis->contents._buffer=NULL;
		somThis->contents._maximum=0;
		somThis->contents._length=0;
	}

    return 0;
}

SOM_Scope ORBStatus  SOMLINK nvlist_free(
		NVList SOMSTAR somSelf,  Environment *ev)
{
	ORBStatus status=nvlist_free_memory(somSelf,ev);

    somReleaseObjectReference(somSelf);

    return status;
}

SOM_Scope ORBStatus  SOMLINK nvlist_get_count(
		NVList SOMSTAR somSelf, 
		Environment *ev, 
		long* count)
{
    NVListData *somThis = NVListGetData(somSelf);

	RHBOPT_unused(ev)

   *count=(long)somThis->contents._length;

   return 0;
}

SOM_Scope ORBStatus  SOMLINK nvlist_set_item(
		NVList SOMSTAR somSelf, 
		Environment *ev, 
		long item_number, 
		Identifier item_name, 
		TypeCode item_type, 
		void* value, long value_len, 
		Flags item_flags)
{
    NVListData *somThis = NVListGetData(somSelf);

	if (((unsigned long)item_number) >= somThis->contents._length)
	{
		NVList_set_count(somSelf,ev,item_number+1);
	}

	NamedValue_set(&somThis->contents._buffer[item_number],
			ev,
			item_name,
			item_type,
			value,
			value_len,
			item_flags);

    return 0;
}

SOM_Scope ORBStatus  SOMLINK nvlist_get_item(
		NVList SOMSTAR somSelf, 
		Environment *ev, 
		long item_number, 
		Identifier* item_name, 
		TypeCode* item_type, 
		void** value, long* value_len, 
		Flags* item_flags)
{
    NVListData *somThis = NVListGetData(somSelf);

	RHBOPT_unused(ev)

	if (((unsigned long)item_number) >= somThis->contents._length)
	{
		return SOMDERROR_NVIndexError;
	}

	*item_name=somThis->contents._buffer[item_number].name;
	*item_type=somThis->contents._buffer[item_number].argument._type;
	*value=somThis->contents._buffer[item_number].argument._value;
	*value_len=somThis->contents._buffer[item_number].len;
	*item_flags=somThis->contents._buffer[item_number].arg_modes;

    return 0;
}

#ifdef NVList_BeginInitializer_somDefaultInit
SOM_Scope void SOMLINK nvlist_somDefaultInit(
	NVList SOMSTAR somSelf,
	somInitCtrl *ctrl)
{
	somInitCtrl globalCtrl;
	somBooleanVector myMask;
	NVListData *somThis;

	NVList_BeginInitializer_somDefaultInit
	NVList_Init_SOMRefObject_somDefaultInit(somSelf,ctrl);

	somThis->contents._length=0;
	somThis->contents._maximum=0;
	somThis->contents._buffer=NULL;
}
#else
SOM_Scope void  SOMLINK nvlist_somInit(NVList SOMSTAR somSelf)
{
    NVListData *somThis = NVListGetData(somSelf);

#ifdef NVList_parent_SOMRefObject_somInit
    NVList_parent_SOMRefObject_somInit(somSelf);
#else
	NVList_parent_SOMObject_somInit(somSelf);
#endif

	somThis->contents._length=0;
	somThis->contents._maximum=0;
	somThis->contents._buffer=NULL;
}
#endif

#ifdef NVList_BeginDestructor
SOM_Scope void SOMLINK nvlist_somDestruct(
	NVList SOMSTAR somSelf,
	boolean doFree,
	somDestructCtrl *ctrl)
{
	Environment ev;
	somDestructCtrl globalCtrl;
	somBooleanVector myMask;
	NVListData *somThis;

	NVList_BeginDestructor

	RHBOPT_unused(somThis);
	SOM_InitEnvironment(&ev);
	nvlist_free_memory(somSelf,&ev);
	SOM_UninitEnvironment(&ev);

	NVList_EndDestructor
}
#else
SOM_Scope void  SOMLINK nvlist_somUninit(NVList SOMSTAR somSelf)
{
	Environment ev;
	SOM_InitEnvironment(&ev);
	nvlist_free_memory(somSelf,&ev);
#ifdef NVList_parent_SOMRefObject_somUninit
    NVList_parent_SOMRefObject_somUninit(somSelf);
#else
    NVList_parent_SOMObject_somUninit(somSelf);
#endif
	SOM_UninitEnvironment(&ev);
}
#endif

static struct 
{
	long arg_modes;
	const char *name;
}
arg_modes[]=
{
	{ARG_IN,"ARG_IN"},
	{ARG_OUT,"ARG_OUT"},
	{ARG_INOUT,"ARG_INOUT"},
	{IN_COPY_VALUE,"IN_COPY_VALUE"},
	{DEPENDENT_LIST,"DEPENDENT_LIST"},
	{OBJ_OWNED,"OBJ_OWNED"},
	{CALLER_OWNED,"CALLER_OWNED"},
	{RESULT_OUT,"RESULT_OUT"},
	{ENV_IN_MSG,"ENV_IN_MSG"},
	{CTX_IN_MSG,"CTX_IN_MSG"},
	{CLIENT_SIDE,"CLIENT_SIDE"},
	{SERVER_SIDE,"SERVER_SIDE"}
};

SOM_Scope void SOMLINK nvlist_somDumpSelfInt(
	NVList SOMSTAR somSelf,
	/* in */ long level)
{
    NVListData *somThis = NVListGetData(somSelf);
	unsigned int i=0;
	Environment ev;

	SOM_InitEnvironment(&ev);

	somPrefixLevel(level);
	dump_somPrintf(("count = %ld\n",(long)somThis->contents._length));

	while (i < somThis->contents._length)
	{
		long m=somThis->contents._buffer[i].arg_modes;
		long j=0;

		somPrefixLevel(level+1);

		dump_somPrintf(("%s : ",somThis->contents._buffer[i].name));

		while (j < (sizeof(arg_modes)/sizeof(arg_modes[0])))
		{
			if (m & arg_modes[j].arg_modes) 
			{
				dump_somPrintf(("%s : ",arg_modes[j].name));
			}

			j++;
		}

		tcPrint(somThis->contents._buffer[i].argument._type,&ev);
		dump_somPrintf(("\n"));
		i++;
	}

	SOM_UninitEnvironment(&ev);
}

