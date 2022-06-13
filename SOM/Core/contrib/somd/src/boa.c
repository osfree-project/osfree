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

#ifndef SOM_Module_boa_Source
#define SOM_Module_boa_Source
#endif
#define BOA_Class_Source

#include "boa.ih"

/* would be nice to have a simple BOA to do activation
	of implementations with raw object_keys,
	then have SOMOA supplement the impl_id prefix */

SOM_Scope SOMDObject SOMSTAR  SOMLINK boa_create(
		BOA SOMSTAR somSelf,  
		Environment *ev, 
		ReferenceData* id, 
		InterfaceDef SOMSTAR intf, 
		ImplementationDef SOMSTAR impl)
{
	RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,NotImplemented,NO);
	RHBOPT_unused(somSelf)
	RHBOPT_unused(id)
	RHBOPT_unused(intf)
	RHBOPT_unused(impl)
	return NULL;
}

SOM_Scope void  SOMLINK boa_dispose(
		BOA SOMSTAR somSelf,
		Environment *ev, 
		SOMDObject SOMSTAR obj)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)

	if (obj && !ev->_major) SOMDObject_somFree(obj);
}

SOM_Scope ReferenceData  SOMLINK boa_get_id(
		BOA SOMSTAR somSelf, 
		Environment *ev, 
		SOMDObject SOMSTAR obj)
{
	ReferenceData ref={0,0,NULL};

	RHBOPT_unused(somSelf)
	RHBOPT_unused(obj)

	RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,NotImplemented,NO);

    return ref;
}

SOM_Scope Principal SOMSTAR  SOMLINK boa_get_principal(
	BOA SOMSTAR somSelf, 
	Environment *ev, 
	SOMDObject SOMSTAR obj, 
	Environment* req_ev)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(obj)
	RHBOPT_unused(req_ev)

	RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,NotImplemented,NO);

    return NULL;
}

SOM_Scope void  SOMLINK boa_set_exception(
		BOA SOMSTAR somSelf,
		Environment *ev, 
		exception_type major, 
		corbastring userid,
		void* param)
{
	RHBOPT_unused(somSelf)
	RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,NotImplemented,NO);
	RHBOPT_unused(userid)
	RHBOPT_unused(param)
	RHBOPT_unused(major)
}

SOM_Scope void  SOMLINK boa_impl_is_ready(
		BOA SOMSTAR somSelf,
		Environment *ev, 
		ImplementationDef SOMSTAR impl)
{
	RHBOPT_unused(somSelf)
	RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,NotImplemented,NO);
	RHBOPT_unused(impl)
}

SOM_Scope void  SOMLINK boa_deactivate_impl(
		BOA SOMSTAR somSelf,
		Environment *ev, 
		ImplementationDef SOMSTAR impl)
{
	RHBOPT_unused(somSelf)
	RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,NotImplemented,NO);
	RHBOPT_unused(impl)
}

SOM_Scope void  SOMLINK boa_change_implementation(
		BOA SOMSTAR somSelf, 
		Environment *ev, 
		SOMDObject SOMSTAR obj, 
		ImplementationDef SOMSTAR impl)
{
	RHBOPT_unused(somSelf)
	RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,NotImplemented,NO);
	RHBOPT_unused(obj)
	RHBOPT_unused(impl)
}

SOM_Scope void  SOMLINK boa_obj_is_ready(
		BOA SOMSTAR somSelf,
		Environment *ev, 
		SOMDObject SOMSTAR obj,
		ImplementationDef SOMSTAR impl)
{
	RHBOPT_unused(somSelf)
	RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,NotImplemented,NO);
	RHBOPT_unused(obj)
	RHBOPT_unused(impl)
}

SOM_Scope void SOMLINK boa_deactivate_obj(
		BOA SOMSTAR somSelf,
		Environment *ev, 
		SOMDObject SOMSTAR obj)
{
	RHBOPT_unused(somSelf)
	RHBOPT_throw_StExcep(ev,NO_IMPLEMENT,NotImplemented,NO);
	RHBOPT_unused(obj)
}

#ifdef BOA_somDefaultInit
SOM_Scope void SOMLINK boa_somDefaultInit(
	BOA SOMSTAR somSelf,
	somInitCtrl *ctrl)
{
	somInitCtrl globalCtrl;
	somBooleanVector myMask;
/*	BOAData *somThis; */

	BOA_BeginInitializer_somDefaultInit
	BOA_Init_SOMRefObject_somDefaultInit(somSelf,ctrl);
#else
SOM_Scope void  SOMLINK boa_somInit(BOA SOMSTAR somSelf)
{
	BOAData *somThis=BOAGetData(somSelf);
    BOA_parent_BOA_somInit(somSelf);
#endif
}

#ifdef SOMOA_somDestruct
SOM_Scope void SOMLINK boa_somDestruct(
	SOMOA SOMSTAR somSelf,
	boolean doFree,
	somDestructCtrl *ctrl)
{
	somDestructCtrl globalCtrl;
	somBooleanVector myMask;
/*	BOAData *somThis; */

	BOA_BeginDestructor
#else
SOM_Scope void SOMLINK boa_somUninit(
		BOA SOMSTAR somSelf)
{
#endif

	if (SOMMSingleInstanceClassData.sommSingleInstanceFreed)
	{
		Environment ev;

		SOM_InitEnvironment(&ev);

		SOMMSingleInstance_sommSingleInstanceFreed(somSelf->mtab->classObject,&ev,somSelf);

		SOM_UninitEnvironment(&ev);
	}

#ifdef BOA_somDestruct
	BOA_EndDestructor
#else
#	ifdef BOA_parent_SOMObject_somUninit
	BOA_parent_SOMObject_somUninit(somSelf);
#	else
	BOA_parent_SOMRefObject_somUninit(somSelf);
#	endif
#endif
}
