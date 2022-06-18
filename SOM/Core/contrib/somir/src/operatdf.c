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

#define OperationDef_Class_Source
#define M_OperationDef_Class_Source

#include <rhbsomir.h>
#include <operatdf.ih>

#ifdef _params
#undef _params
#endif

SOM_Scope void SOMLINK operatdf_somDumpSelf(
	OperationDef SOMSTAR somSelf,
	/* in */ long level)
{
	OperationDef_parent_Contained_somDumpSelf(somSelf,level);
}

SOM_Scope void SOMLINK operatdf_somDumpSelfInt(
	OperationDef SOMSTAR somSelf,
	/* in */ long level)
{
	OperationDef_parent_Contained_somDumpSelfInt(somSelf,level);
	OperationDef_parent_Container_somDumpSelfInt(somSelf,level);
}

SOM_Scope void SOMLINK operatdf_somInit(
	OperationDef SOMSTAR somSelf)
{
	OperationDef_parent_Container_somInit(somSelf);
	OperationDef_parent_Contained_somInit(somSelf);
	SOM_IgnoreWarning(somSelf);
}

SOM_Scope void SOMLINK operatdf_somUninit(
	OperationDef SOMSTAR somSelf)
{
	OperationDef_parent_Container_somUninit(somSelf);
	OperationDef_parent_Contained_somUninit(somSelf);
}

SOM_Scope void SOMLINK operatdf__set_contexts(
	OperationDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ _IDL_SEQUENCE_string *contexts)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(contexts);
}

SOM_Scope void SOMLINK operatdf__set_mode(
	OperationDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ OperationDef_OperationMode mode)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(mode);
}

SOM_Scope void SOMLINK operatdf__set_result(
	OperationDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ TypeCode result)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(result);
}

SOM_Scope _IDL_SEQUENCE_string SOMLINK operatdf__get_contexts(
	OperationDef SOMSTAR somSelf,
	Environment *ev)
{
	_IDL_SEQUENCE_string retVal={0,0,NULL};
	somir_containedData *cnd=OperationDef__get_containedData(somSelf,ev);

	if (cnd)
	{
		retVal=cnd->description.operationDesc.desc.contexts;
		cnd->lpVtbl->Release(cnd);
	}

	return retVal;
}

SOM_Scope OperationDef_OperationMode SOMLINK operatdf__get_mode(
	OperationDef SOMSTAR somSelf,
	Environment *ev)
{
	OperationDef_OperationMode retVal=0;
	somir_containedData *cnd=OperationDef__get_containedData(somSelf,ev);

	if (cnd)
	{
		retVal=cnd->description.operationDesc.desc.mode;
		cnd->lpVtbl->Release(cnd);
	}

	return retVal;
}

SOM_Scope TypeCode SOMLINK operatdf__get_result(
	OperationDef SOMSTAR somSelf,
	Environment *ev)
{
	TypeCode retVal=NULL;
	somir_containedData *cnd=OperationDef__get_containedData(somSelf,ev);

	if (cnd)
	{
		retVal=cnd->description.operationDesc.desc.result;
		cnd->lpVtbl->Release(cnd);
	}

	return retVal;
}

SOM_Scope void SOMLINK operatdf__set_raiseSeq(
	OperationDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ _IDL_SEQUENCE_string *exceptions)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(exceptions);
}

SOM_Scope _IDL_SEQUENCE_string SOMLINK operatdf__get_raiseSeq(
	OperationDef SOMSTAR somSelf,
	Environment *ev)
{
	_IDL_SEQUENCE_string retVal={0,0,NULL};
	somir_containedData *cnd=OperationDef__get_containedData(somSelf,ev);

	if (cnd)
	{
		retVal=cnd->description.operationDesc.raiseSeq;

		cnd->lpVtbl->Release(cnd);
	}

	return retVal;
}

SOM_Scope void SOMLINK operatdf_somDestruct(OperationDef SOMSTAR somSelf,
											boolean doFree,
											somDestructCtrl *ctrl)
{
/*	OperationDefData *somThis;*/
	somDestructCtrl globalCtrl;
	somBooleanVector myMask;

	OperationDef_BeginDestructor;

	operatdf_somUninit(somSelf);

	OperationDef_EndDestructor;
}

SOM_Scope void SOMLINK m_operatdf_somDestruct(M_OperationDef SOMSTAR somSelf,boolean doFree,
											somDestructCtrl *ctrl)
{
	somDestructCtrl globalCtrl;
	somBooleanVector myMask;
/*	M_OperationDef *somThis;*/

	M_OperationDef_BeginDestructor;

	M_OperationDef_EndDestructor;
}

SOM_Scope void SOMLINK m_operatdf_somDefaultInit(M_OperationDef SOMSTAR somSelf,
											   somInitCtrl *ctrl)
{
	somInitCtrl globalCtrl;
	somBooleanVector myMask;
/*	M_OperationDef *somThis;*/

	M_OperationDef_BeginInitializer_somDefaultInit

	M_OperationDef_Init_M_Contained_somDefaultInit(somSelf,ctrl)
}


