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
#define Contained_Class_Source
#define M_Contained_Class_Source
#include <rhbsomir.h>
#include <containd.ih>

SOM_Scope void SOMLINK containd_somDumpSelf(
	Contained SOMSTAR somSelf,
	/* in */ long level)
{
	ContainedData *somThis=ContainedGetData(somSelf);
	char *id=somThis->containedData ? somThis->containedData->description.moduleDesc.desc.id : NULL;

	somPrefixLevel(level);
	somPrintf("%s \042%s\042\n",
		SOMObject_somGetClassName(somSelf),
		id);

	SOMObject_somDumpSelfInt(somSelf,level);
}

static void print_enum(long level,TypeCode tc,Environment *ev,long index,long value)
{
	any name=TypeCode_parameter(tc,ev,1+(index << 1));
	any type=TypeCode_parameter(tc,ev,2+(index << 1));
/*	any label=TypeCode_parameter(*(TypeCode *)type._value,ev,0);*/
	any val=TypeCode_parameter(*(TypeCode *)type._value,ev,value);
	char *label_string=*(char **)name._value;
	char *label_value=*(char **)val._value;

	somPrefixLevel(level);

	RHBOPT_ASSERT(TypeCode_equal(name._type,ev,TC_string));
	RHBOPT_ASSERT(TypeCode_equal(type._type,ev,TC_TypeCode));
/*	RHBOPT_ASSERT(TypeCode_equal(label._type,ev,TC_string));*/

	somPrintf("%s: %s\n",label_string,label_value);
}

SOM_Scope void SOMLINK containd_somDumpSelfInt(
	Contained SOMSTAR somSelf,
	/* in */ long level)
{
	ContainedData *somThis=ContainedGetData(somSelf);
	somir_containedData *cnd=somThis->containedData;

	if (cnd)
	{
		_IDL_SEQUENCE_somModifier *mods=&cnd->modifiers;
		unsigned long i=mods->_length;
		char *plural=i ? "s" : "";
		somModifier *mb=mods->_buffer;
		Environment ev;
		
		SOM_InitEnvironment(&ev);

		somPrefixLevel(level);
		somPrintf("id: %s\n",cnd->description.moduleDesc.desc.id);

		somPrefixLevel(level);
		somPrintf("%ld modifier%s:\n",i,plural);
		while (i--)
		{
			char *value=mb->value;
			somPrefixLevel(level+1);
			if (value)
			{
				somPrintf("%s = %s\n",mb->name,value);
			}
			else
			{
				somPrintf("%s\n",mb->name);
			}

			mb++;
		}

		switch (cnd->type)
		{
		case kSOMIR_ExceptionDef:
			RHBOPT_ASSERT(Contained_somIsA(somSelf,_ExceptionDef));
			somPrefixLevel(level+1);
			somPrintf("type: "); 
			TypeCode_print(cnd->description.exceptionDesc.type,&ev);
			break;
		case kSOMIR_OperationDef:
			RHBOPT_ASSERT(Contained_somIsA(somSelf,_OperationDef));

			print_enum(level,
					TC_OperationDescription,
					&ev,
					4,
					cnd->description.operationDesc.desc.mode);

			if (cnd->description.operationDesc.desc.result)
			{
				somPrefixLevel(level);
				somPrintf("result: ");
				TypeCode_print(cnd->description.operationDesc.desc.result,&ev);
			}
			break;
		case kSOMIR_TypeDef:
			RHBOPT_ASSERT(Contained_somIsA(somSelf,_TypeDef));
			if (cnd->description.typeDesc.type)
			{
				somPrefixLevel(level);
				somPrintf("type: "); 
				TypeCode_print(cnd->description.typeDesc.type,&ev);
			}
			break;
		case kSOMIR_ConstantDef:
			RHBOPT_ASSERT(Contained_somIsA(somSelf,_ConstantDef));

			if (cnd->description.constantDesc.value)
			{
				char *p=cnd->description.constantDesc.value;
				somPrefixLevel(level);
				somPrintf("value: \"%s\"\n",p);
			}

			if (cnd->description.constantDesc.desc.type)
			{
				somPrefixLevel(level);
				somPrintf("type: "); 
				TypeCode_print(cnd->description.constantDesc.desc.type,&ev);
			}
			break;
		case kSOMIR_InterfaceDef:
			RHBOPT_ASSERT(Contained_somIsA(somSelf,_InterfaceDef));
			break;
		case kSOMIR_ModuleDef:
			RHBOPT_ASSERT(Contained_somIsA(somSelf,_ModuleDef));
			break;
		case kSOMIR_AttributeDef:
			RHBOPT_ASSERT(Contained_somIsA(somSelf,_AttributeDef));
			print_enum(level,
					TC_AttributeDescription,
					&ev,
					4,
					cnd->description.attributeDesc.mode);
			if (cnd->description.attributeDesc.type)
			{
				somPrefixLevel(level);
				somPrintf("type: "); 
				TypeCode_print(cnd->description.attributeDesc.type,&ev);
			}
			break;
		case kSOMIR_ParameterDef:
			RHBOPT_ASSERT(Contained_somIsA(somSelf,_ParameterDef));
			print_enum(level,
					TC_ParameterDescription,
					&ev,
					4,
					cnd->description.parameterDesc.mode);
			if (cnd->description.parameterDesc.type)
			{
				somPrefixLevel(level);
				somPrintf("type: "); 
				TypeCode_print(cnd->description.parameterDesc.type,&ev);
			}
			break;
		default:
			RHBOPT_ASSERT(!somSelf);
			break;
		}

		SOM_UninitEnvironment(&ev);
	}

#ifdef Contained_parent_SOMRefObject_somDumpSelfInt
	Contained_parent_SOMRefObject_somDumpSelfInt(somSelf,level);
#else
	Contained_parent_SOMObject_somDumpSelfInt(somSelf,level);
#endif
}

SOM_Scope Contained_Description SOMLINK containd_describe(
	Contained SOMSTAR somSelf,
	Environment *ev)
{
	ContainedData *somThis=ContainedGetData(somSelf);
	Contained_Description ret={0,{NULL,NULL}};
	somir_containedData *cnd=somThis->containedData;

	if (cnd && (ev->_major==NO_EXCEPTION))
	{
		return cnd->lpVtbl->describe(cnd,ev);
	}

	return ret;
}

SOM_Scope _IDL_SEQUENCE_Container SOMLINK containd_within(
	Contained SOMSTAR somSelf,
	Environment *ev)
{
/*	ContainedData *somThis=ContainedGetData(somSelf);*/
	_IDL_SEQUENCE_Container seq={0,0,NULL};
	return seq;
}

SOM_Scope void SOMLINK containd__set_somModifiers(
	Contained SOMSTAR somSelf,
	Environment *ev,
	/* in */ _IDL_SEQUENCE_somModifier *somModifiers)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(somModifiers);
}

SOM_Scope _IDL_SEQUENCE_somModifier SOMLINK containd__get_somModifiers(
	Contained SOMSTAR somSelf,
	Environment *ev)
{
	ContainedData *somThis=ContainedGetData(somSelf);
	_IDL_SEQUENCE_somModifier seq={0,0,NULL};

	if (somThis->containedData)
	{
		return somThis->containedData->modifiers;
	}

	return seq;
}

SOM_Scope void SOMLINK containd__set_defined_in(
	Contained SOMSTAR somSelf,
	Environment *ev,
	/* in */ RepositoryId defined_in)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(defined_in);
}

SOM_Scope char * SOMLINK containd__get_defined_in(
	Contained SOMSTAR somSelf,
	Environment *ev)
{
	ContainedData *somThis=ContainedGetData(somSelf);
	char *p=somThis->containedData ?
		somThis->containedData->description.moduleDesc.desc.defined_in : NULL;

	return p;
}

SOM_Scope RepositoryId SOMLINK containd__get_id(
	Contained SOMSTAR somSelf,
	Environment *ev)
{
	ContainedData *somThis=ContainedGetData(somSelf);
	char *p=somThis->containedData ?
		somThis->containedData->description.moduleDesc.desc.id : NULL;

	return p;
}

SOM_Scope void SOMLINK containd__set_id(
	Contained SOMSTAR somSelf,
	Environment *ev,
	/* in */ RepositoryId id)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(id);
}

SOM_Scope void SOMLINK containd__set_name(
	Contained SOMSTAR somSelf,
	Environment *ev,
	/* in */ Identifier name)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(name);
}

SOM_Scope Identifier SOMLINK containd__get_name(
	Contained SOMSTAR somSelf,
	Environment *ev)
{
	ContainedData *somThis=ContainedGetData(somSelf);
	char *p=somThis->containedData ?
		somThis->containedData->description.moduleDesc.desc.name : NULL;

	return p;
}

SOM_Scope void SOMLINK containd_somInit(
	Contained SOMSTAR somSelf)
{
	ContainedData *somThis=ContainedGetData(somSelf);

	somThis->containedData=NULL;

/*	somThis->name=0;*/
/*	somThis->id=0;*/
/*	somThis->defined_in=0;*/
/*	somThis->absolute_name=0;*/

#ifdef Contained_parent_SOMRefObject_somInit
	Contained_parent_SOMRefObject_somInit(somSelf);
#else
	Contained_parent_SOMObject_somInit(somSelf);
#endif
}

static void contained_somUninitData(ContainedData *somThis)
{
	struct SOMIR_ContainedData *cnd=somThis->containedData;

	somThis->containedData=NULL;

	if (cnd)
	{
		cnd->lpVtbl->Release(cnd);
	}
}

#define is_of_class(obj,cls)  (somTestCls(obj,cls,__FILE__,__LINE__)?1:0)

#ifdef Contained_somDestruct
SOM_Scope void SOMLINK containd_somDestruct(
	Contained SOMSTAR somSelf,
	boolean doFree,
	somDestructCtrl *ctrl)
{
	ContainedData *somThis;
	somDestructCtrl globalCtrl;
	somBooleanVector myMask;

	Contained_BeginDestructor

	contained_somUninitData(somThis);

	Contained_EndDestructor
}
#else
SOM_Scope void SOMLINK containd_somUninit(
	Contained SOMSTAR somSelf)
{
	contained_somUninitData(ContainedGetData(somSelf));

#ifdef Contained_parent_SOMRefObject_somUninit
	Contained_parent_SOMRefObject_somUninit(somSelf);
#else
	Contained_parent_SOMObject_somUninit(somSelf);
#endif
}
#endif

SOM_Scope void SOMLINK containd_somDefaultInit(
	Contained SOMSTAR somSelf,
	somInitCtrl *ctrl)
{
	somInitCtrl globalCtrl;
	somBooleanVector myMask;
	ContainedData *somThis;

	Contained_BeginInitializer_somDefaultInit

	Contained_Init_SOMRefObject_somDefaultInit(somSelf,ctrl);

	RHBOPT_unused(somThis);
	containd_somInit(somSelf);
}

SOM_Scope void SOMLINK m_containd_somDestruct(
	M_Contained SOMSTAR somSelf,
	boolean doFree,
	somDestructCtrl *ctrl)
{
/*	M_ContainedData *somThis;*/
	somDestructCtrl globalCtrl;
	somBooleanVector myMask;

	M_Contained_BeginDestructor

	M_Contained_EndDestructor
}

SOM_Scope void SOMLINK m_containd_somDefaultInit(
	M_Contained SOMSTAR somSelf,
	somInitCtrl *ctrl)
{
	somInitCtrl globalCtrl;
	somBooleanVector myMask;
/*	M_ContainedData *somThis */

	M_Contained_BeginInitializer_somDefaultInit

	M_Contained_Init_SOMClass_somDefaultInit(somSelf,ctrl);
}

SOM_Scope somir_containedData * SOMLINK containd__get_containedData(
	Contained SOMSTAR somSelf,
	Environment *ev)
{
	somir_containedData *retValue=ContainedGetData(somSelf)->containedData;

	if (retValue)
	{
		retValue->lpVtbl->AddRef(retValue);
	}
	else
	{
		RHBOPT_throw_StExcep(ev,INTF_REPOS,IRNotFound,NO);
	}

	return retValue;
}

SOM_Scope void SOMLINK containd__set_containedData(
	Contained SOMSTAR somSelf,
	Environment *ev,
	somir_containedData *containedData)
{
	ContainedData *somThis=ContainedGetData(somSelf);
	
	if (somThis->containedData!=containedData)
	{
		somir_containedData *old=somThis->containedData;
		if (containedData) containedData->lpVtbl->AddRef(containedData);
		somThis->containedData=containedData;
		if (old) old->lpVtbl->Release(old);
	}
}

static void SOMLINK m_containd_clsInit(SOMClass SOMSTAR somSelf)
{
	SOMIR_DLL_INIT
}

