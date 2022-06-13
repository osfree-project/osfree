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

#define ConstantDef_Class_Source
#define M_ConstantDef_Class_Source

#include <rhbsomir.h>
#include <constdef.ih>

#ifdef _type
#undef _type
#endif

#ifdef _value
#undef _value
#endif

SOM_Scope void SOMLINK constdef_somDumpSelf(
	ConstantDef SOMSTAR somSelf,
	/* in */ long level)
{
	ConstantDef_parent_Contained_somDumpSelf(somSelf,level);
}

SOM_Scope void SOMLINK constdef_somDumpSelfInt(
	ConstantDef SOMSTAR somSelf,
	/* in */ long level)
{
	ConstantDef_parent_Contained_somDumpSelfInt(somSelf,level);
}

SOM_Scope void SOMLINK constdef_somInit(
	ConstantDef SOMSTAR somSelf)
{
	SOM_IgnoreWarning(somSelf);
	ConstantDef_parent_Contained_somInit(somSelf);
}

SOM_Scope void SOMLINK constdef_somUninit(
	ConstantDef SOMSTAR somSelf)
{
}

SOM_Scope TypeCode SOMLINK constdef__get_type(
	ConstantDef SOMSTAR somSelf,
	Environment *ev)
{
	TypeCode retVal=NULL;
	somir_containedData *cnd=ConstantDef__get_containedData(somSelf,ev);

	if (cnd)
	{
		retVal=cnd->description.constantDesc.desc.type;
		cnd->lpVtbl->Release(cnd);
	}

	return retVal;
}

SOM_Scope void SOMLINK constdef__set_type(
	ConstantDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ TypeCode type)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(type);
}

SOM_Scope any SOMLINK constdef__get_value(
	ConstantDef SOMSTAR somSelf,
	Environment *ev)
{
/*	ConstantDefData *somThis=ConstantDefGetData(somSelf); */
	any retVal={NULL,NULL};
	somir_containedData *cnd=ConstantDef__get_containedData(somSelf,ev);

	if (cnd)
	{
		retVal=cnd->description.constantDesc.desc.value;
		cnd->lpVtbl->Release(cnd);
	}

	return retVal;
}

SOM_Scope void SOMLINK constdef__set_value(
	ConstantDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ any *value)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(value);
}

SOM_Scope void SOMLINK constdef_somDestruct(
	ConstantDef SOMSTAR somSelf,
	boolean doFree,
	somDestructCtrl *ctrl)
{
/*	ConstantDefData *somThis;*/
	somDestructCtrl globalCtrl;
	somBooleanVector myMask;

	ConstantDef_BeginDestructor

	constdef_somUninit(somSelf);

	ConstantDef_EndDestructor
}

SOM_Scope void SOMLINK m_constdef_somDestruct(M_ConstantDef SOMSTAR somSelf,boolean doFree,
											somDestructCtrl *ctrl)
{
	somDestructCtrl globalCtrl;
	somBooleanVector myMask;
/*	M_ConstantDef *somThis;*/

	M_ConstantDef_BeginDestructor;

	M_ConstantDef_EndDestructor;
}

SOM_Scope void SOMLINK m_constdef_somDefaultInit(M_ConstantDef SOMSTAR somSelf,
											   somInitCtrl *ctrl)
{
	somInitCtrl globalCtrl;
	somBooleanVector myMask;
/*	M_ConstantDef *somThis;*/

	M_ConstantDef_BeginInitializer_somDefaultInit

	M_ConstantDef_Init_M_Contained_somDefaultInit(somSelf,ctrl)
}
