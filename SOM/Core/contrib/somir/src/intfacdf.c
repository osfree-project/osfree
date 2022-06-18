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

#define InterfaceDef_Class_Source
#define M_InterfaceDef_Class_Source

#include <rhbsomir.h>
#include <intfacdf.ih>

SOM_Scope void SOMLINK intfacdf_somDumpSelf(
	InterfaceDef SOMSTAR somSelf,
	/* in */ long level)
{
	InterfaceDef_parent_Contained_somDumpSelf(somSelf,level);
}

SOM_Scope void SOMLINK intfacdf_somDumpSelfInt(
	InterfaceDef SOMSTAR somSelf,
	/* in */ long level)
{
	InterfaceDef_parent_Contained_somDumpSelfInt(somSelf,level);
	InterfaceDef_parent_Container_somDumpSelfInt(somSelf,level);
}
/*
SOM_Scope void SOMLINK intfacdf_somInit(
	InterfaceDef SOMSTAR somSelf)
{
	InterfaceDef_parent_Container_somInit(somSelf);
	InterfaceDef_parent_Contained_somInit(somSelf);
	SOM_IgnoreWarning(somSelf);
}
*/
SOM_Scope void SOMLINK intfacdf_somUninit(
	InterfaceDef SOMSTAR somSelf)
{
	InterfaceDef_parent_Container_somUninit(somSelf);
	InterfaceDef_parent_Contained_somUninit(somSelf);
}

/*SOM_Scope _IDL_SEQUENCE_Container SOMLINK intfacdf_within(
	InterfaceDef SOMSTAR somSelf,
	Environment *ev)
{
	return InterfaceDef_parent_Contained_within(somSelf,ev);
}*/

SOM_Scope void SOMLINK intfacdf__set_instanceData(
	InterfaceDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ TypeCode instanceData)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(instanceData);
}

SOM_Scope InterfaceDef_FullInterfaceDescription SOMLINK intfacdf_describe_interface(
	InterfaceDef SOMSTAR somSelf,
	Environment *ev)
{
	InterfaceDef_FullInterfaceDescription retVal={NULL,NULL,NULL,{0,0,NULL},{0,0,NULL}};
	somir_containedData *cnd=InterfaceDef__get_containedData(somSelf,ev);

	if (cnd)
	{
		SOMIR_LOCK

		retVal=cnd->lpVtbl->describe_interface(cnd,ev);

		SOMIR_UNLOCK

		cnd->lpVtbl->Release(cnd);

	}

	return retVal;
}

SOM_Scope void SOMLINK intfacdf__set_base_interfaces(
	InterfaceDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ _IDL_SEQUENCE_string *base_interfaces)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(base_interfaces);
}

SOM_Scope TypeCode SOMLINK intfacdf__get_instanceData(
	InterfaceDef SOMSTAR somSelf,
	Environment *ev)
{
	TypeCode retVal=NULL;
	somir_containedData *cnd=InterfaceDef__get_containedData(somSelf,ev);

	if (cnd)
	{
		retVal=cnd->description.interfaceDesc.instanceData;
		cnd->lpVtbl->Release(cnd);
	}

	return retVal;
}

SOM_Scope _IDL_SEQUENCE_string SOMLINK intfacdf__get_base_interfaces(
	InterfaceDef SOMSTAR somSelf,
	Environment *ev)
{
	_IDL_SEQUENCE_string retVal={0,0,NULL};
	somir_containedData *cnd=InterfaceDef__get_containedData(somSelf,ev);

	if (cnd)
	{
		retVal=cnd->description.interfaceDesc.base_interfaces;
		cnd->lpVtbl->Release(cnd);
	}

	return retVal;
}

struct intfacdf_lookup_name
{
	InterfaceDef SOMSTAR somSelf;
	Environment *ev;
	struct SOMIR_ContainedData *cnd;
	struct SOMIR_ContainerData *cnr;
	struct SOMIR_Repository *rep;
	InterfaceDef SOMSTAR base;
	long levels_to_search;
	_IDL_SEQUENCE_Contained seq;
};

RHBOPT_cleanup_begin(intfacdf_lookup_name_cleanup,pv)

struct intfacdf_lookup_name *data=pv;

	if (data->somSelf && data->seq._maximum)
	{
		Environment ev;
		SOM_InitEnvironment(&ev);
		Container_seqContained_free(data->somSelf,&ev,&data->seq);
		SOM_UninitEnvironment(&ev);
	}

	if (data->base)
	{
		InterfaceDef_somFree(data->base);
	}

	if (data->cnd)
	{
		data->cnd->lpVtbl->Release(data->cnd);
	}

RHBOPT_cleanup_end

SOM_Scope _IDL_SEQUENCE_Contained SOMLINK intfacdf_lookup_name(
	Container SOMSTAR somSelf,
	Environment *ev,
	/* in */ Identifier search_name,
	/* in */ long levels_to_search_arg,
	/* in */ Container_InterfaceName limit_type,
	/* in */ boolean exclude_inherited)
{
	_IDL_SEQUENCE_Contained retVal={0,0,NULL};
	struct intfacdf_lookup_name data={
		NULL,NULL,NULL,NULL,NULL,NULL,
		0,
		{0,0,NULL}};

	data.levels_to_search=levels_to_search_arg;

	RHBOPT_cleanup_push(intfacdf_lookup_name_cleanup,&data);

/*	somPrintf("lookup_name(%s,%s,%ld,%s,%d)\n",
		InterfaceDef__get_id(somSelf,ev),
		search_name,
		levels_to_search,
		limit_type,
		exclude_inherited);
*/

	data.seq=InterfaceDef_parent_Container_lookup_name(somSelf,ev,search_name,
		data.levels_to_search,
		limit_type,
		exclude_inherited);

	if (ev->_major==NO_EXCEPTION)
	{
		data.somSelf=somSelf;
		data.ev=ev;

		if ((!exclude_inherited) 
			&& (--data.levels_to_search))
		{
			_IDL_SEQUENCE_string bases=InterfaceDef__get_base_interfaces(somSelf,ev);

			if (bases._length && (ev->_major==NO_EXCEPTION))
			{
				unsigned long i=0;

				data.cnd=Contained__get_containedData(somSelf,ev);
				data.rep=data.cnd->defined_in->file->repository;

				SOMIR_LOCK

				while (i < bases._length)
				{
					char *id=bases._buffer[i];
					struct SOMIRfile *f=data.rep->first;

					while (f && (ev->_major==NO_EXCEPTION))
					{
						struct SOMIR_ContainedData *base=f->lpVtbl->Lookup(f,id);

						if (base)
						{
							data.base=data.rep->lpVtbl->Wrap(base,ev);

							base->lpVtbl->Release(base);

							break;
						}

						f=f->next;
					}

					if (data.base)
					{
						InterfaceDef SOMSTAR base=data.base;
						_IDL_SEQUENCE_Contained s2=InterfaceDef_lookup_name(
							base,
							ev,
							search_name,
							data.levels_to_search,
							limit_type,
							exclude_inherited);

						RHBOPT_ASSERT(InterfaceDef_somIsA(base,_InterfaceDef));

						if (ev->_major==NO_EXCEPTION)
						{
							unsigned long i=0;
							
							while (i < s2._length)
							{
								if (Container_seqContained_add(somSelf,ev,&data.seq,s2._buffer[i]))
								{
									s2._buffer[i]=NULL;
								}

								i++;
							}

							Container_seqContained_free(somSelf,ev,&s2);
						}

						data.base=NULL;

						InterfaceDef_somFree(base);
					}

					i++;
				}

				SOMIR_UNLOCK
			}
		}
	}

	if (ev->_major==NO_EXCEPTION)
	{
		retVal=data.seq;
		data.seq._length=0;
		data.seq._maximum=0;
		data.seq._buffer=NULL;
	}

	RHBOPT_cleanup_pop();

	return retVal;
}

SOM_Scope void SOMLINK intfacdf_somDestruct(InterfaceDef SOMSTAR somSelf,boolean doFree,
											somDestructCtrl *ctrl)
{
	somDestructCtrl globalCtrl;
	somBooleanVector myMask;
/*	InterfaceDefData *somThis; */

	InterfaceDef_BeginDestructor;

	intfacdf_somUninit(somSelf);

	InterfaceDef_EndDestructor;
}

SOM_Scope void SOMLINK intfacdf_somDefaultInit(InterfaceDef SOMSTAR somSelf,
											   somInitCtrl *ctrl)
{
	somInitCtrl globalCtrl;
	somBooleanVector myMask;
/*	InterfaceDefData *somThis;*/

	InterfaceDef_BeginInitializer_somDefaultInit

	InterfaceDef_Init_Contained_somDefaultInit(somSelf,ctrl)
	InterfaceDef_Init_Container_somDefaultInit(somSelf,ctrl)
}


SOM_Scope void SOMLINK m_intfacdf_somDestruct(M_InterfaceDef SOMSTAR somSelf,boolean doFree,
											somDestructCtrl *ctrl)
{
	somDestructCtrl globalCtrl;
	somBooleanVector myMask;
/*	M_InterfaceDefData *somThis;*/

	M_InterfaceDef_BeginDestructor;

	M_InterfaceDef_EndDestructor;
}

SOM_Scope void SOMLINK m_intfacdf_somDefaultInit(M_InterfaceDef SOMSTAR somSelf,
											   somInitCtrl *ctrl)
{
	somInitCtrl globalCtrl;
	somBooleanVector myMask;
/*	M_InterfaceDefData *somThis;*/

	M_InterfaceDef_BeginInitializer_somDefaultInit

	M_InterfaceDef_Init_M_Contained_somDefaultInit(somSelf,ctrl)
}


