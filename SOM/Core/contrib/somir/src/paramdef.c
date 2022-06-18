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

#define ParameterDef_Class_Source
#define M_ParameterDef_Class_Source

#include <rhbsomir.h>
#include <paramdef.ih>

#ifdef _type
#undef _type
#endif

SOM_Scope void SOMLINK paramdef_somDumpSelf(
	ParameterDef SOMSTAR somSelf,
	/* in */ long level)
{
	ParameterDef_parent_Contained_somDumpSelf(somSelf,level);
}

SOM_Scope void SOMLINK paramdef_somDumpSelfInt(
	ParameterDef SOMSTAR somSelf,
	/* in */ long level)
{
	ParameterDef_parent_Contained_somDumpSelfInt(somSelf,level);
}

SOM_Scope void SOMLINK paramdef__set_mode(
	ParameterDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ ParameterDef_ParameterMode mode)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(mode);
}

SOM_Scope void SOMLINK paramdef__set_type(
	ParameterDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ TypeCode type)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(type);
}

SOM_Scope TypeCode SOMLINK paramdef__get_type(
	ParameterDef SOMSTAR somSelf,
	Environment *ev)
{
	TypeCode retVal=NULL;
	somir_containedData *cnd=ParameterDef__get_containedData(somSelf,ev);

	if (cnd)
	{
		retVal=cnd->description.parameterDesc.type;
		cnd->lpVtbl->Release(cnd);
	}

	return retVal;
}


SOM_Scope ParameterDef_ParameterMode SOMLINK paramdef__get_mode(
	ParameterDef SOMSTAR somSelf,
	Environment *ev)
{
	ParameterDef_ParameterMode retVal=0;
	somir_containedData *cnd=ParameterDef__get_containedData(somSelf,ev);

	if (cnd)
	{
		retVal=cnd->description.parameterDesc.mode;
		cnd->lpVtbl->Release(cnd);
	}

	return retVal;
}

SOM_Scope void SOMLINK paramdef_somUninit(
	ParameterDef SOMSTAR somSelf)
{
/*	ParameterDefData *somThis=ParameterDefGetData(somSelf);*/
	ParameterDef_parent_Contained_somUninit(somSelf);
}

SOM_Scope void SOMLINK paramdef_somDestruct(
	ParameterDef SOMSTAR somSelf,
	boolean doFree,
	somDestructCtrl *ctrl)
{
/*	ParameterDefData *somThis;*/
	somDestructCtrl globalCtrl;
	somBooleanVector myMask;

	ParameterDef_BeginDestructor

	paramdef_somUninit(somSelf);

	ParameterDef_EndDestructor
}

SOM_Scope void SOMLINK paramdef_somDefaultInit(ParameterDef SOMSTAR somSelf,
											   somInitCtrl *ctrl)
{
	somInitCtrl globalCtrl;
	somBooleanVector myMask;
/*	ParameterDefData *somThis;*/

	ParameterDef_BeginInitializer_somDefaultInit

	ParameterDef_Init_Contained_somDefaultInit(somSelf,ctrl)
}

/*
SOM_Scope void SOMLINK paramdef_somInit(
	ParameterDef SOMSTAR somSelf)
{
	SOM_IgnoreWarning(somSelf);
	ParameterDef_parent_Contained_somInit(somSelf);
}
*/


SOM_Scope void SOMLINK m_paramdef_somDestruct(M_ParameterDef SOMSTAR somSelf,boolean doFree,
											somDestructCtrl *ctrl)
{
	somDestructCtrl globalCtrl;
	somBooleanVector myMask;
/*	M_ParameterDef *somThis;*/

	M_ParameterDef_BeginDestructor;

	M_ParameterDef_EndDestructor;
}

SOM_Scope void SOMLINK m_paramdef_somDefaultInit(M_ParameterDef SOMSTAR somSelf,
											   somInitCtrl *ctrl)
{
	somInitCtrl globalCtrl;
	somBooleanVector myMask;
/*	M_ParameterDef *somThis;*/

	M_ParameterDef_BeginInitializer_somDefaultInit

	M_ParameterDef_Init_M_Contained_somDefaultInit(somSelf,ctrl)
}
