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
#include <rhbsomex.h>
#include <somtc.h>

/* #define SOM_Module_dynany_Source */

#define DynamicAny_DynAny_Class_Source
#define DynamicAny_DynAnyFactory_Class_Source
#define DynamicAny_DynArray_Class_Source
#define DynamicAny_DynEnum_Class_Source
#define DynamicAny_DynFixed_Class_Source
#define DynamicAny_DynUnion_Class_Source
#define DynamicAny_DynStruct_Class_Source
#define DynamicAny_DynSequence_Class_Source
#define DynamicAny_DynValue_Class_Source

#include <dynany.ih>

#ifdef _type
	#undef _type
#endif

#ifdef _value
	#undef _value
#endif

static void throw_InvalidValue(Environment *ev);
static void throw_TypeMismatch(Environment *ev);

/* overridden methods for ::DynamicAny::DynAny */
/* overridden method ::SOMObject::somInit */
SOM_Scope void SOMLINK dynany_somInit(
	DynamicAny_DynAny SOMSTAR somSelf)
{
	SOM_IgnoreWarning(somSelf);
	DynamicAny_DynAny_parent_SOMObject_somInit(somSelf);
}
/* overridden method ::SOMObject::somUninit */
SOM_Scope void SOMLINK dynany_somUninit(
	DynamicAny_DynAny SOMSTAR somSelf)
{
	SOM_IgnoreWarning(somSelf);
	DynamicAny_DynAny_parent_SOMObject_somUninit(somSelf);
}
/* introduced methods for ::DynamicAny::DynAny */
/* introduced method ::DynamicAny::DynAny::type */
SOM_Scope TypeCode SOMLINK dynany_type(
	DynamicAny_DynAny SOMSTAR somSelf,
	Environment *ev)
{
	TypeCode __result=NULL;
	SOM_IgnoreWarning(somSelf);
	throw_TypeMismatch(ev);
	return __result;
}
/* introduced method ::DynamicAny::DynAny::assign */
SOM_Scope void SOMLINK dynany_assign(
	DynamicAny_DynAny SOMSTAR somSelf,
	Environment *ev,
	/* in */ DynamicAny_DynAny SOMSTAR dyn_any)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(dyn_any);
	throw_TypeMismatch(ev);
}
/* introduced method ::DynamicAny::DynAny::from_any */
SOM_Scope void SOMLINK dynany_from_any(
	DynamicAny_DynAny SOMSTAR somSelf,
	Environment *ev,
	/* in */ any *value)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(value);
	throw_TypeMismatch(ev);
}
/* introduced method ::DynamicAny::DynAny::to_any */
SOM_Scope any SOMLINK dynany_to_any(
	DynamicAny_DynAny SOMSTAR somSelf,
	Environment *ev)
{
	any __result={NULL,NULL};
	SOM_IgnoreWarning(somSelf);
	throw_TypeMismatch(ev);
	return __result;
}
/* introduced method ::DynamicAny::DynAny::equal */
SOM_Scope boolean SOMLINK dynany_equal(
	DynamicAny_DynAny SOMSTAR somSelf,
	Environment *ev,
	/* in */ DynamicAny_DynAny SOMSTAR dyn_any)
{
	boolean __result=0;
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(dyn_any);
	throw_TypeMismatch(ev);
	return __result;
}
/* introduced method ::DynamicAny::DynAny::destroy */
SOM_Scope void SOMLINK dynany_destroy(
	DynamicAny_DynAny SOMSTAR somSelf,
	Environment *ev)
{
	SOM_IgnoreWarning(somSelf);
	throw_TypeMismatch(ev);
}
/* introduced method ::DynamicAny::DynAny::copy */
SOM_Scope DynamicAny_DynAny SOMSTAR SOMLINK dynany_copy(
	DynamicAny_DynAny SOMSTAR somSelf,
	Environment *ev)
{
	DynamicAny_DynAny SOMSTAR __result=NULL;
	SOM_IgnoreWarning(somSelf);
	throw_TypeMismatch(ev);
	return __result;
}
/* introduced method ::DynamicAny::DynAny::insert_boolean */
SOM_Scope void SOMLINK dynany_insert_boolean(
	DynamicAny_DynAny SOMSTAR somSelf,
	Environment *ev,
	/* in */ boolean value)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(value);
	throw_TypeMismatch(ev);
}
/* introduced method ::DynamicAny::DynAny::insert_octet */
SOM_Scope void SOMLINK dynany_insert_octet(
	DynamicAny_DynAny SOMSTAR somSelf,
	Environment *ev,
	/* in */ octet value)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(value);
	throw_TypeMismatch(ev);
}
/* introduced method ::DynamicAny::DynAny::insert_char */
SOM_Scope void SOMLINK dynany_insert_char(
	DynamicAny_DynAny SOMSTAR somSelf,
	Environment *ev,
	/* in */ char value)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(value);
	throw_TypeMismatch(ev);
}
/* introduced method ::DynamicAny::DynAny::insert_short */
SOM_Scope void SOMLINK dynany_insert_short(
	DynamicAny_DynAny SOMSTAR somSelf,
	Environment *ev,
	/* in */ short value)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(value);
	throw_TypeMismatch(ev);
}
/* introduced method ::DynamicAny::DynAny::insert_ushort */
SOM_Scope void SOMLINK dynany_insert_ushort(
	DynamicAny_DynAny SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned short value)
{
	throw_TypeMismatch(ev);
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(value);
}
/* introduced method ::DynamicAny::DynAny::insert_long */
SOM_Scope void SOMLINK dynany_insert_long(
	DynamicAny_DynAny SOMSTAR somSelf,
	Environment *ev,
	/* in */ long value)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(value);
	/* if enum, check bounds and throw if out of range */
	throw_InvalidValue(ev);

	/* else */
	throw_TypeMismatch(ev);
}
/* introduced method ::DynamicAny::DynAny::insert_ulong */
SOM_Scope void SOMLINK dynany_insert_ulong(
	DynamicAny_DynAny SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned long value)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(value);
	/* if enum, check bounds and throw if out of range */
	throw_InvalidValue(ev);

	throw_TypeMismatch(ev);
}
/* introduced method ::DynamicAny::DynAny::insert_float */
SOM_Scope void SOMLINK dynany_insert_float(
	DynamicAny_DynAny SOMSTAR somSelf,
	Environment *ev,
	/* in */ float value)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(value);
	throw_TypeMismatch(ev);
}
/* introduced method ::DynamicAny::DynAny::insert_double */
SOM_Scope void SOMLINK dynany_insert_double(
	DynamicAny_DynAny SOMSTAR somSelf,
	Environment *ev,
	/* in */ double value)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(value);
	throw_TypeMismatch(ev);
}
/* introduced method ::DynamicAny::DynAny::insert_string */
SOM_Scope void SOMLINK dynany_insert_string(
	DynamicAny_DynAny SOMSTAR somSelf,
	Environment *ev,
	/* in */ char * value)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(value);
	throw_TypeMismatch(ev);
}
/* introduced method ::DynamicAny::DynAny::insert_reference */
SOM_Scope void SOMLINK dynany_insert_reference(
	DynamicAny_DynAny SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMObject SOMSTAR value)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(value);
	throw_TypeMismatch(ev);
}
/* introduced method ::DynamicAny::DynAny::insert_typecode */
SOM_Scope void SOMLINK dynany_insert_typecode(
	DynamicAny_DynAny SOMSTAR somSelf,
	Environment *ev,
	/* in */ TypeCode value)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(value);
	throw_TypeMismatch(ev);
}
/* introduced method ::DynamicAny::DynAny::insert_any */
SOM_Scope void SOMLINK dynany_insert_any(
	DynamicAny_DynAny SOMSTAR somSelf,
	Environment *ev,
	/* in */ any *value)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(value);
	throw_TypeMismatch(ev);
}
/* introduced method ::DynamicAny::DynAny::insert_dyn_any */
SOM_Scope void SOMLINK dynany_insert_dyn_any(
	DynamicAny_DynAny SOMSTAR somSelf,
	Environment *ev,
	/* in */ DynamicAny_DynAny SOMSTAR value)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(value);
	throw_TypeMismatch(ev);
}
/* introduced method ::DynamicAny::DynAny::get_boolean */
SOM_Scope boolean SOMLINK dynany_get_boolean(
	DynamicAny_DynAny SOMSTAR somSelf,
	Environment *ev)
{
	boolean __result=0;
	SOM_IgnoreWarning(somSelf);
	throw_TypeMismatch(ev);
	return __result;
}
/* introduced method ::DynamicAny::DynAny::get_octet */
SOM_Scope octet SOMLINK dynany_get_octet(
	DynamicAny_DynAny SOMSTAR somSelf,
	Environment *ev)
{
	octet __result=0;
	SOM_IgnoreWarning(somSelf);
	throw_TypeMismatch(ev);
	return __result;
}
/* introduced method ::DynamicAny::DynAny::get_char */
SOM_Scope char SOMLINK dynany_get_char(
	DynamicAny_DynAny SOMSTAR somSelf,
	Environment *ev)
{
	char __result=0;
	SOM_IgnoreWarning(somSelf);
	throw_TypeMismatch(ev);
	return __result;
}
/* introduced method ::DynamicAny::DynAny::get_short */
SOM_Scope short SOMLINK dynany_get_short(
	DynamicAny_DynAny SOMSTAR somSelf,
	Environment *ev)
{
	short __result=0;
	SOM_IgnoreWarning(somSelf);
	throw_TypeMismatch(ev);
	return __result;
}
/* introduced method ::DynamicAny::DynAny::get_ushort */
SOM_Scope unsigned short SOMLINK dynany_get_ushort(
	DynamicAny_DynAny SOMSTAR somSelf,
	Environment *ev)
{
	unsigned short __result=0;
	SOM_IgnoreWarning(somSelf);
	throw_TypeMismatch(ev);
	return __result;
}
/* introduced method ::DynamicAny::DynAny::get_long */
SOM_Scope long SOMLINK dynany_get_long(
	DynamicAny_DynAny SOMSTAR somSelf,
	Environment *ev)
{
	long __result=0;
	SOM_IgnoreWarning(somSelf);
	throw_TypeMismatch(ev);
	return __result;
}
/* introduced method ::DynamicAny::DynAny::get_ulong */
SOM_Scope unsigned long SOMLINK dynany_get_ulong(
	DynamicAny_DynAny SOMSTAR somSelf,
	Environment *ev)
{
	unsigned long __result=0;
	SOM_IgnoreWarning(somSelf);
	throw_TypeMismatch(ev);
	return __result;
}
/* introduced method ::DynamicAny::DynAny::get_float */
SOM_Scope float SOMLINK dynany_get_float(
	DynamicAny_DynAny SOMSTAR somSelf,
	Environment *ev)
{
	float __result=(float)0;
	SOM_IgnoreWarning(somSelf);
	throw_TypeMismatch(ev);
	return __result;
}
/* introduced method ::DynamicAny::DynAny::get_double */
SOM_Scope double SOMLINK dynany_get_double(
	DynamicAny_DynAny SOMSTAR somSelf,
	Environment *ev)
{
	double __result=0;
	SOM_IgnoreWarning(somSelf);
	throw_TypeMismatch(ev);
	return __result;
}
/* introduced method ::DynamicAny::DynAny::get_string */
SOM_Scope char * SOMLINK dynany_get_string(
	DynamicAny_DynAny SOMSTAR somSelf,
	Environment *ev)
{
	char * __result=0;
	SOM_IgnoreWarning(somSelf);
	throw_TypeMismatch(ev);
	return __result;
}
/* introduced method ::DynamicAny::DynAny::get_reference */
SOM_Scope SOMObject SOMSTAR SOMLINK dynany_get_reference(
	DynamicAny_DynAny SOMSTAR somSelf,
	Environment *ev)
{
	SOMObject SOMSTAR __result=NULL;
	SOM_IgnoreWarning(somSelf);
	throw_TypeMismatch(ev);
	return __result;
}
/* introduced method ::DynamicAny::DynAny::get_typecode */
SOM_Scope TypeCode SOMLINK dynany_get_typecode(
	DynamicAny_DynAny SOMSTAR somSelf,
	Environment *ev)
{
	TypeCode __result=NULL;
	SOM_IgnoreWarning(somSelf);
	throw_TypeMismatch(ev);
	return __result;
}
/* introduced method ::DynamicAny::DynAny::get_any */
SOM_Scope any SOMLINK dynany_get_any(
	DynamicAny_DynAny SOMSTAR somSelf,
	Environment *ev)
{
	any __result={0,0};
	SOM_IgnoreWarning(somSelf);
	throw_TypeMismatch(ev);
	return __result;
}
/* introduced method ::DynamicAny::DynAny::get_dyn_any */
SOM_Scope DynamicAny_DynAny SOMSTAR SOMLINK dynany_get_dyn_any(
	DynamicAny_DynAny SOMSTAR somSelf,
	Environment *ev)
{
	DynamicAny_DynAny SOMSTAR __result=0;
	SOM_IgnoreWarning(somSelf);
	throw_TypeMismatch(ev);
	return __result;
}
/* introduced method ::DynamicAny::DynAny::seek */
SOM_Scope boolean SOMLINK dynany_seek(
	DynamicAny_DynAny SOMSTAR somSelf,
	Environment *ev,
	/* in */ long index)
{
	boolean __result=0;
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(index);
	throw_TypeMismatch(ev);
	return __result;
}
/* introduced method ::DynamicAny::DynAny::rewind */
SOM_Scope void SOMLINK dynany_rewind(
	DynamicAny_DynAny SOMSTAR somSelf,
	Environment *ev)
{
	SOM_IgnoreWarning(somSelf);
	throw_TypeMismatch(ev);
}
/* introduced method ::DynamicAny::DynAny::next */
SOM_Scope boolean SOMLINK dynany_next(
	DynamicAny_DynAny SOMSTAR somSelf,
	Environment *ev)
{
	boolean __result=0;
	SOM_IgnoreWarning(somSelf);
	throw_TypeMismatch(ev);
	return __result;
}
/* introduced method ::DynamicAny::DynAny::component_count */
SOM_Scope unsigned long SOMLINK dynany_component_count(
	DynamicAny_DynAny SOMSTAR somSelf,
	Environment *ev)
{
	unsigned long __result=0;
	SOM_IgnoreWarning(somSelf);
	throw_TypeMismatch(ev);
	return __result;
}
/* introduced method ::DynamicAny::DynAny::current_component */
SOM_Scope DynamicAny_DynAny SOMSTAR SOMLINK dynany_current_component(
	DynamicAny_DynAny SOMSTAR somSelf,
	Environment *ev)
{
	DynamicAny_DynAny SOMSTAR __result=0;

	SOM_IgnoreWarning(somSelf);
	throw_TypeMismatch(ev);
	
	return __result;
}

static void throw_InvalidValue(Environment *ev)
{
	if (ev) somSetException(ev,USER_EXCEPTION,ex_DynamicAny_DynAny_InvalidValue,NULL);
}

static void throw_TypeMismatch(Environment *ev)
{
	if (ev) somSetException(ev,USER_EXCEPTION,ex_DynamicAny_DynAny_TypeMismatch,NULL);
}

SOM_Scope any SOMLINK dynany__get_value(
			DynamicAny_DynAny SOMSTAR somSelf,
			Environment *ev)
{
DynamicAny_DynAnyData *somThis=DynamicAny_DynAnyGetData(somSelf);

	if (ev->_major)
	{
		any a={NULL,NULL};
		return a;
	}

	return somThis->value;
}

SOM_Scope void SOMLINK dynany__set_value(
			DynamicAny_DynAny SOMSTAR somSelf,
			Environment *ev,
			any *value)
{
DynamicAny_DynAnyData *somThis=DynamicAny_DynAnyGetData(somSelf);

	if (ev->_major) return;

	if (somThis->value._type)
	{
		if (!TypeCode_equal(value->_type,ev,somThis->value._type))
		{
			if (!ev->_major)
			{
				throw_TypeMismatch(ev);
			}

			return;
		}

		if (ev->_major) return;
	}

	somThis->value=*value;
}

/* overridden methods for ::DynamicAny::DynAnyFactory */
/* overridden method ::SOMObject::somInit */
SOM_Scope void SOMLINK dynanyfactory_somInit(
	DynamicAny_DynAnyFactory SOMSTAR somSelf)
{
	DynamicAny_DynAnyFactory_parent_SOMObject_somInit(somSelf);
	SOM_IgnoreWarning(somSelf);
}
/* overridden method ::SOMObject::somUninit */
SOM_Scope void SOMLINK dynanyfactory_somUninit(
	DynamicAny_DynAnyFactory SOMSTAR somSelf)
{
	DynamicAny_DynAnyFactory_parent_SOMObject_somUninit(somSelf);
	SOM_IgnoreWarning(somSelf);
}
/* introduced methods for ::DynamicAny::DynAnyFactory */
/* introduced method ::DynamicAny::DynAnyFactory::create_dyn_any */
SOM_Scope DynamicAny_DynAny SOMSTAR SOMLINK dynanyfactory_create_dyn_any(
	DynamicAny_DynAnyFactory SOMSTAR somSelf,
	Environment *ev,
	/* in */ any *value)
{
	DynamicAny_DynAny SOMSTAR __result=NULL;
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(value);
	return __result;
}
/* introduced method ::DynamicAny::DynAnyFactory::create_dyn_any_from_type_code */
SOM_Scope DynamicAny_DynAny SOMSTAR SOMLINK dynanyfactory_create_dyn_any_from_type_code(
	DynamicAny_DynAnyFactory SOMSTAR somSelf,
	Environment *ev,
	/* in */ TypeCode type)
{
	DynamicAny_DynAny SOMSTAR __result=NULL;
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(type);
	return __result;
}
/* overridden methods for ::DynamicAny::DynArray */
/* overridden method ::SOMObject::somInit */
SOM_Scope void SOMLINK dynarray_somInit(
	DynamicAny_DynArray SOMSTAR somSelf)
{
	DynamicAny_DynArray_parent_DynamicAny_DynAny_somInit(somSelf);
	SOM_IgnoreWarning(somSelf);
}
/* overridden method ::SOMObject::somUninit */
SOM_Scope void SOMLINK dynarray_somUninit(
	DynamicAny_DynArray SOMSTAR somSelf)
{
	DynamicAny_DynArray_parent_DynamicAny_DynAny_somUninit(somSelf);
	SOM_IgnoreWarning(somSelf);
}
/* introduced methods for ::DynamicAny::DynArray */
/* introduced method ::DynamicAny::DynArray::get_elements */
SOM_Scope DynamicAny_AnySeq SOMLINK dynarray_get_elements(
	DynamicAny_DynArray SOMSTAR somSelf,
	Environment *ev)
{
	DynamicAny_AnySeq __result={0,0,NULL};

	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(somSelf);

	return __result;
}
/* introduced method ::DynamicAny::DynArray::set_elements */
SOM_Scope void SOMLINK dynarray_set_elements(
	DynamicAny_DynArray SOMSTAR somSelf,
	Environment *ev,
	/* in */ DynamicAny_AnySeq *value)
{
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(value);
}

/* introduced method ::DynamicAny::DynArray::get_elements_as_dyn_any */
SOM_Scope DynamicAny_DynAnySeq SOMLINK dynarray_get_elements_as_dyn_any(
	DynamicAny_DynArray SOMSTAR somSelf,
	Environment *ev)
{
	DynamicAny_DynAnySeq __result={0,0,NULL};
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(somSelf);
	return __result;
}

/* introduced method ::DynamicAny::DynArray::set_elements_as_dyn_any */
SOM_Scope void SOMLINK dynarray_set_elements_as_dyn_any(
	DynamicAny_DynArray SOMSTAR somSelf,
	Environment *ev,
	/* in */ DynamicAny_DynAnySeq *value)
{
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(value);
}


/* overridden methods for ::DynamicAny::DynEnum */
/* overridden method ::SOMObject::somInit */
SOM_Scope void SOMLINK dynenum_somInit(
	DynamicAny_DynEnum SOMSTAR somSelf)
{
	DynamicAny_DynEnum_parent_DynamicAny_DynAny_somInit(somSelf);
	SOM_IgnoreWarning(somSelf);
}
/* overridden method ::SOMObject::somUninit */
SOM_Scope void SOMLINK dynenum_somUninit(
	DynamicAny_DynEnum SOMSTAR somSelf)
{
	DynamicAny_DynEnum_parent_DynamicAny_DynAny_somUninit(somSelf);
	SOM_IgnoreWarning(somSelf);
}
/* introduced methods for ::DynamicAny::DynEnum */
/* introduced method ::DynamicAny::DynEnum::get_as_string */
SOM_Scope char * SOMLINK dynenum_get_as_string(
	DynamicAny_DynEnum SOMSTAR somSelf,
	Environment *ev)
{
	char * __result=NULL;

	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);

	return __result;
}
/* introduced method ::DynamicAny::DynEnum::set_as_string */
SOM_Scope void SOMLINK dynenum_set_as_string(
	DynamicAny_DynEnum SOMSTAR somSelf,
	Environment *ev,
	/* in */ char * value)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(value);
}
/* introduced method ::DynamicAny::DynEnum::get_as_ulong */
SOM_Scope unsigned long SOMLINK dynenum_get_as_ulong(
	DynamicAny_DynEnum SOMSTAR somSelf,
	Environment *ev)
{
	unsigned long __result=0;
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	return __result;
}
/* introduced method ::DynamicAny::DynEnum::set_as_ulong */
SOM_Scope void SOMLINK dynenum_set_as_ulong(
	DynamicAny_DynEnum SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned long value)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(value);
}


/* overridden methods for ::DynamicAny::DynFixed */
/* overridden method ::SOMObject::somInit */
SOM_Scope void SOMLINK dynfixed_somInit(
	DynamicAny_DynFixed SOMSTAR somSelf)
{
	DynamicAny_DynFixed_parent_DynamicAny_DynAny_somInit(somSelf);
	SOM_IgnoreWarning(somSelf);
}
/* overridden method ::SOMObject::somUninit */
SOM_Scope void SOMLINK dynfixed_somUninit(
	DynamicAny_DynFixed SOMSTAR somSelf)
{
	DynamicAny_DynFixed_parent_DynamicAny_DynAny_somUninit(somSelf);
	SOM_IgnoreWarning(somSelf);
}
/* introduced methods for ::DynamicAny::DynFixed */
/* introduced method ::DynamicAny::DynFixed::get_value */
SOM_Scope char * SOMLINK dynfixed_get_value(
	DynamicAny_DynFixed SOMSTAR somSelf,
	Environment *ev)
{
	char * __result=NULL;
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	return __result;
}
/* introduced method ::DynamicAny::DynFixed::set_value */
SOM_Scope boolean SOMLINK dynfixed_set_value(
	DynamicAny_DynFixed SOMSTAR somSelf,
	Environment *ev,
	/* in */ char * val)
{
	boolean __result=0;
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(val);
	return __result;
}


/* overridden methods for ::DynamicAny::DynUnion */
/* overridden method ::SOMObject::somInit */
SOM_Scope void SOMLINK dynunion_somInit(
	DynamicAny_DynUnion SOMSTAR somSelf)
{
	DynamicAny_DynUnion_parent_DynamicAny_DynAny_somInit(somSelf);
	SOM_IgnoreWarning(somSelf);
}
/* overridden method ::SOMObject::somUninit */
SOM_Scope void SOMLINK dynunion_somUninit(
	DynamicAny_DynUnion SOMSTAR somSelf)
{
	SOM_IgnoreWarning(somSelf);
	DynamicAny_DynUnion_parent_DynamicAny_DynAny_somUninit(somSelf);
}
/* introduced methods for ::DynamicAny::DynUnion */
/* introduced method ::DynamicAny::DynUnion::get_discriminator */
SOM_Scope DynamicAny_DynAny SOMSTAR SOMLINK dynunion_get_discriminator(
	DynamicAny_DynUnion SOMSTAR somSelf,
	Environment *ev)
{
	DynamicAny_DynAny SOMSTAR __result=NULL;
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	return __result;
}
/* introduced method ::DynamicAny::DynUnion::set_discriminator */
SOM_Scope void SOMLINK dynunion_set_discriminator(
	DynamicAny_DynUnion SOMSTAR somSelf,
	Environment *ev,
	/* in */ DynamicAny_DynAny SOMSTAR d)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(d);
}
/* introduced method ::DynamicAny::DynUnion::set_to_default_member */
SOM_Scope void SOMLINK dynunion_set_to_default_member(
	DynamicAny_DynUnion SOMSTAR somSelf,
	Environment *ev)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
}
/* introduced method ::DynamicAny::DynUnion::set_to_no_active_member */
SOM_Scope void SOMLINK dynunion_set_to_no_active_member(
	DynamicAny_DynUnion SOMSTAR somSelf,
	Environment *ev)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
}
/* introduced method ::DynamicAny::DynUnion::has_no_active_member */
SOM_Scope boolean SOMLINK dynunion_has_no_active_member(
	DynamicAny_DynUnion SOMSTAR somSelf,
	Environment *ev)
{
	boolean __result=0;
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	return __result;
}
/* introduced method ::DynamicAny::DynUnion::discriminator_kind */
SOM_Scope TCKind SOMLINK dynunion_discriminator_kind(
	DynamicAny_DynUnion SOMSTAR somSelf,
	Environment *ev)
{
	TCKind __result=tk_null;
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	return __result;
}
/* introduced method ::DynamicAny::DynUnion::member */
SOM_Scope DynamicAny_DynAny SOMSTAR SOMLINK dynunion_member(
	DynamicAny_DynUnion SOMSTAR somSelf,
	Environment *ev)
{
	DynamicAny_DynAny SOMSTAR __result=NULL;
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	return __result;
}
/* introduced method ::DynamicAny::DynUnion::member_name */
SOM_Scope DynamicAny_FieldName SOMLINK dynunion_member_name(
	DynamicAny_DynUnion SOMSTAR somSelf,
	Environment *ev)
{
	DynamicAny_FieldName __result=NULL;
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	return __result;
}
/* introduced method ::DynamicAny::DynUnion::member_kind */
SOM_Scope TCKind SOMLINK dynunion_member_kind(
	DynamicAny_DynUnion SOMSTAR somSelf,
	Environment *ev)
{
	TCKind __result=0;
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	return __result;
}


/* overridden methods for ::DynamicAny::DynStruct */
/* overridden method ::SOMObject::somInit */
SOM_Scope void SOMLINK dynstruct_somInit(
	DynamicAny_DynStruct SOMSTAR somSelf)
{
	SOM_IgnoreWarning(somSelf);
	DynamicAny_DynStruct_parent_DynamicAny_DynAny_somInit(somSelf);
}
/* overridden method ::SOMObject::somUninit */
SOM_Scope void SOMLINK dynstruct_somUninit(
	DynamicAny_DynStruct SOMSTAR somSelf)
{
	SOM_IgnoreWarning(somSelf);
	DynamicAny_DynStruct_parent_DynamicAny_DynAny_somUninit(somSelf);
}
/* introduced methods for ::DynamicAny::DynStruct */
/* introduced method ::DynamicAny::DynStruct::current_member_name */
SOM_Scope DynamicAny_FieldName SOMLINK dynstruct_current_member_name(
	DynamicAny_DynStruct SOMSTAR somSelf,
	Environment *ev)
{
	DynamicAny_FieldName __result=NULL;
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(somSelf);
	return __result;
}
/* introduced method ::DynamicAny::DynStruct::current_member_kind */
SOM_Scope TCKind SOMLINK dynstruct_current_member_kind(
	DynamicAny_DynStruct SOMSTAR somSelf,
	Environment *ev)
{
	TCKind __result=tk_null;
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(somSelf);
	return __result;
}
/* introduced method ::DynamicAny::DynStruct::get_members */
SOM_Scope DynamicAny_NameValuePairSeq SOMLINK dynstruct_get_members(
	DynamicAny_DynStruct SOMSTAR somSelf,
	Environment *ev)
{
	DynamicAny_NameValuePairSeq __result={0,0,NULL};
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(somSelf);
	return __result;
}
/* introduced method ::DynamicAny::DynStruct::set_members */
SOM_Scope void SOMLINK dynstruct_set_members(
	DynamicAny_DynStruct SOMSTAR somSelf,
	Environment *ev,
	/* in */ DynamicAny_NameValuePairSeq *value)
{
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(value);
}
/* introduced method ::DynamicAny::DynStruct::get_members_as_dyn_any */
SOM_Scope DynamicAny_NameDynAnyPairSeq SOMLINK dynstruct_get_members_as_dyn_any(
	DynamicAny_DynStruct SOMSTAR somSelf,
	Environment *ev)
{
	DynamicAny_NameDynAnyPairSeq __result={0,0,NULL};
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(somSelf);
	return __result;
}
/* introduced method ::DynamicAny::DynStruct::set_members_as_dyn_any */
SOM_Scope void SOMLINK dynstruct_set_members_as_dyn_any(
	DynamicAny_DynStruct SOMSTAR somSelf,
	Environment *ev,
	/* in */ DynamicAny_NameDynAnyPairSeq *value)
{
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(value);
}

/* overridden methods for ::DynamicAny::DynSequence */
/* overridden method ::SOMObject::somInit */
SOM_Scope void SOMLINK dynseq_somInit(
	DynamicAny_DynSequence SOMSTAR somSelf)
{
	DynamicAny_DynSequence_parent_DynamicAny_DynAny_somInit(somSelf);
	SOM_IgnoreWarning(somSelf);
}
/* overridden method ::SOMObject::somUninit */
SOM_Scope void SOMLINK dynseq_somUninit(
	DynamicAny_DynSequence SOMSTAR somSelf)
{
	DynamicAny_DynSequence_parent_DynamicAny_DynAny_somUninit(somSelf);
	SOM_IgnoreWarning(somSelf);
}
/* introduced methods for ::DynamicAny::DynSequence */
/* introduced method ::DynamicAny::DynSequence::get_length */
SOM_Scope unsigned long SOMLINK dynseq_get_length(
	DynamicAny_DynSequence SOMSTAR somSelf,
	Environment *ev)
{
	unsigned long __result=0;
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(somSelf);
	return __result;
}
/* introduced method ::DynamicAny::DynSequence::set_length */
SOM_Scope void SOMLINK dynseq_set_length(
	DynamicAny_DynSequence SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned long len)
{
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(len);
}
/* introduced method ::DynamicAny::DynSequence::get_elements */
SOM_Scope DynamicAny_AnySeq SOMLINK dynseq_get_elements(
	DynamicAny_DynSequence SOMSTAR somSelf,
	Environment *ev)
{
	DynamicAny_AnySeq __result={0,0,NULL};
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(somSelf);
	return __result;
}
/* introduced method ::DynamicAny::DynSequence::set_elements */
SOM_Scope void SOMLINK dynseq_set_elements(
	DynamicAny_DynSequence SOMSTAR somSelf,
	Environment *ev,
	/* in */ DynamicAny_AnySeq *value)
{
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(value);
}
/* introduced method ::DynamicAny::DynSequence::get_elements_as_dyn_any */
SOM_Scope DynamicAny_DynAnySeq SOMLINK dynseq_get_elements_as_dyn_any(
	DynamicAny_DynSequence SOMSTAR somSelf,
	Environment *ev)
{
	DynamicAny_DynAnySeq __result={0,0,NULL};
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(somSelf);
	return __result;
}
/* introduced method ::DynamicAny::DynSequence::set_elements_as_dyn_any */
SOM_Scope void SOMLINK dynseq_set_elements_as_dyn_any(
	DynamicAny_DynSequence SOMSTAR somSelf,
	Environment *ev,
	/* in */ DynamicAny_DynAnySeq *value)
{
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(value);
}


/* overridden methods for ::DynamicAny::DynValue */
/* overridden method ::SOMObject::somInit */
SOM_Scope void SOMLINK dynvalue_somInit(
	DynamicAny_DynValue SOMSTAR somSelf)
{
	DynamicAny_DynValue_parent_DynamicAny_DynAny_somInit(somSelf);
	SOM_IgnoreWarning(somSelf);
}
/* overridden method ::SOMObject::somUninit */
SOM_Scope void SOMLINK dynvalue_somUninit(
	DynamicAny_DynValue SOMSTAR somSelf)
{
	DynamicAny_DynValue_parent_DynamicAny_DynAny_somUninit(somSelf);
	SOM_IgnoreWarning(somSelf);
}
/* introduced methods for ::DynamicAny::DynValue */
/* introduced method ::DynamicAny::DynValue::current_member_name */
SOM_Scope DynamicAny_FieldName SOMLINK dynvalue_current_member_name(
	DynamicAny_DynValue SOMSTAR somSelf,
	Environment *ev)
{
	DynamicAny_FieldName __result=NULL;
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	return __result;
}
/* introduced method ::DynamicAny::DynValue::current_member_kind */
SOM_Scope TCKind SOMLINK dynvalue_current_member_kind(
	DynamicAny_DynValue SOMSTAR somSelf,
	Environment *ev)
{
	TCKind __result=tk_null;
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	return __result;
}
/* introduced method ::DynamicAny::DynValue::get_members */
SOM_Scope DynamicAny_NameValuePairSeq SOMLINK dynvalue_get_members(
	DynamicAny_DynValue SOMSTAR somSelf,
	Environment *ev)
{
	DynamicAny_NameValuePairSeq __result={0,0,NULL};
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	return __result;
}
/* introduced method ::DynamicAny::DynValue::set_members */
SOM_Scope void SOMLINK dynvalue_set_members(
	DynamicAny_DynValue SOMSTAR somSelf,
	Environment *ev,
	/* in */ DynamicAny_NameValuePairSeq *value)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(value);
}
/* introduced method ::DynamicAny::DynValue::get_members_as_dyn_any */
SOM_Scope DynamicAny_NameDynAnyPairSeq SOMLINK dynvalue_get_members_as_dyn_any(
	DynamicAny_DynValue SOMSTAR somSelf,
	Environment *ev)
{
	DynamicAny_NameDynAnyPairSeq __result={0,0,NULL};
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	return __result;
}
/* introduced method ::DynamicAny::DynValue::set_members_as_dyn_any */
SOM_Scope void SOMLINK dynvalue_set_members_as_dyn_any(
	DynamicAny_DynValue SOMSTAR somSelf,
	Environment *ev,
	/* in */ DynamicAny_NameDynAnyPairSeq *value)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(value);
}
