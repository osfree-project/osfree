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
/* code generater for corba.idl */
#include <rhbsomex.h>

#define CORBA_Current_Class_Source
#define CORBA_Policy_Class_Source
#define CORBA_IRObject_Class_Source
#define CORBA_Contained_Class_Source
#define CORBA_Container_Class_Source
#define CORBA_IDLType_Class_Source
#define CORBA_Repository_Class_Source
#define CORBA_ModuleDef_Class_Source
#define CORBA_ConstantDef_Class_Source
#define CORBA_TypedefDef_Class_Source
#define CORBA_StructDef_Class_Source
#define CORBA_UnionDef_Class_Source
#define CORBA_EnumDef_Class_Source
#define CORBA_AliasDef_Class_Source
#define CORBA_NativeDef_Class_Source
#define CORBA_PrimitiveDef_Class_Source
#define CORBA_StringDef_Class_Source
#define CORBA_WstringDef_Class_Source
#define CORBA_FixedDef_Class_Source
#define CORBA_SequenceDef_Class_Source
#define CORBA_ArrayDef_Class_Source
#define CORBA_ExceptionDef_Class_Source
#define CORBA_AttributeDef_Class_Source
#define CORBA_OperationDef_Class_Source
#define CORBA_InterfaceDef_Class_Source
#define CORBA_ValueMemberDef_Class_Source
#define CORBA_ValueDef_Class_Source
#define CORBA_ValueBoxDef_Class_Source
#define CORBA_AbstractInterfaceDef_Class_Source
#define CORBA_DataOutputStream_Class_Source
#define CORBA_DataInputStream_Class_Source
#define CORBA_Object_Class_Source

#include <somref.h>
#include <rhbcorba.h>
#include <corba.ih>
#include <stexcep.h>
#include <somderr.h>

/* overridden methods for ::CORBA::IRObject */
/* introduced methods for ::CORBA::IRObject */
/* introduced method ::CORBA::IRObject::_get_def_kind */
SOM_Scope CORBA_DefinitionKind SOMLINK ir2_irobject__get_def_kind(
	CORBA_IRObject SOMSTAR somSelf,
	Environment *ev)
{
	CORBA_DefinitionKind __result=0;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::IRObject::destroy */
SOM_Scope void SOMLINK ir2_irobject_destroy(
	CORBA_IRObject SOMSTAR somSelf,
	Environment *ev)
{
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
}
/* overridden methods for ::CORBA::Contained */
/* introduced methods for ::CORBA::Contained */
/* introduced method ::CORBA::Contained::_set_id */
SOM_Scope void SOMLINK ir2_containd__set_id(
	CORBA_Contained SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_RepositoryId id)
{
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
}
/* introduced method ::CORBA::Contained::_get_id */
SOM_Scope CORBA_RepositoryId SOMLINK ir2_containd__get_id(
	CORBA_Contained SOMSTAR somSelf,
	Environment *ev)
{
	CORBA_RepositoryId __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::Contained::_set_name */
SOM_Scope void SOMLINK ir2_containd__set_name(
	CORBA_Contained SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_Identifier name)
{
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
}
/* introduced method ::CORBA::Contained::_get_name */
SOM_Scope CORBA_Identifier SOMLINK ir2_containd__get_name(
	CORBA_Contained SOMSTAR somSelf,
	Environment *ev)
{
	CORBA_Identifier __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::Contained::_set_version */
SOM_Scope void SOMLINK ir2_containd__set_version(
	CORBA_Contained SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_VersionSpec version)
{
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
}
/* introduced method ::CORBA::Contained::_get_version */
SOM_Scope CORBA_VersionSpec SOMLINK ir2_containd__get_version(
	CORBA_Contained SOMSTAR somSelf,
	Environment *ev)
{
	CORBA_VersionSpec __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::Contained::_get_defined_in */
SOM_Scope CORBA_Container SOMSTAR SOMLINK ir2_containd__get_defined_in(
	CORBA_Contained SOMSTAR somSelf,
	Environment *ev)
{
	CORBA_Container SOMSTAR __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::Contained::_get_absolute_name */
SOM_Scope CORBA_ScopedName SOMLINK ir2_containd__get_absolute_name(
	CORBA_Contained SOMSTAR somSelf,
	Environment *ev)
{
	CORBA_ScopedName __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::Contained::_get_containing_repository */
SOM_Scope CORBA_Repository SOMSTAR SOMLINK ir2_containd__get_containing_repository(
	CORBA_Contained SOMSTAR somSelf,
	Environment *ev)
{
	CORBA_Repository SOMSTAR __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::Contained::describe */
SOM_Scope CORBA_Contained_Description SOMLINK ir2_containd_describe(
	CORBA_Contained SOMSTAR somSelf,
	Environment *ev)
{
	CORBA_Contained_Description __result={0,{NULL,NULL}};
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::Contained::move */
SOM_Scope void SOMLINK ir2_containd_move(
	CORBA_Contained SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_Container SOMSTAR new_container,
	/* in */ CORBA_Identifier new_name,
	/* in */ CORBA_VersionSpec new_version)
{
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
}
/* overridden methods for ::CORBA::Container */
/* introduced methods for ::CORBA::Container */
/* introduced method ::CORBA::Container::lookup */
SOM_Scope CORBA_Contained SOMSTAR SOMLINK ir2_containr_lookup(
	CORBA_Container SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_ScopedName search_name)
{
	CORBA_Contained SOMSTAR __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::Container::contents */
SOM_Scope CORBA_ContainedSeq SOMLINK ir2_containr_contents(
	CORBA_Container SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_DefinitionKind limit_type,
	/* in */ boolean exclude_inherited)
{
	CORBA_ContainedSeq __result={0,0,NULL};
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::Container::lookup_name */
SOM_Scope CORBA_ContainedSeq SOMLINK ir2_containr_lookup_name(
	CORBA_Container SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_Identifier search_name,
	/* in */ long levels_to_search,
	/* in */ CORBA_DefinitionKind limit_type,
	/* in */ boolean exclude_inherited)
{
	CORBA_ContainedSeq __result={0,0,NULL};
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::Container::describe_contents */
SOM_Scope CORBA_Container_DescriptionSeq SOMLINK ir2_containr_describe_contents(
	CORBA_Container SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_DefinitionKind limit_type,
	/* in */ boolean exclude_inherited,
	/* in */ long max_returned_objs)
{
	CORBA_Container_DescriptionSeq __result={0,0,NULL};
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::Container::create_module */
SOM_Scope CORBA_ModuleDef SOMSTAR SOMLINK ir2_containr_create_module(
	CORBA_Container SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_RepositoryId id,
	/* in */ CORBA_Identifier name,
	/* in */ CORBA_VersionSpec version)
{
	CORBA_ModuleDef SOMSTAR __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::Container::create_constant */
SOM_Scope CORBA_ConstantDef SOMSTAR SOMLINK ir2_containr_create_constant(
	CORBA_Container SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_RepositoryId id,
	/* in */ CORBA_Identifier name,
	/* in */ CORBA_VersionSpec version,
	/* in */ CORBA_IDLType SOMSTAR type,
	/* in */ any *value)
{
	CORBA_ConstantDef SOMSTAR __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::Container::create_struct */
SOM_Scope CORBA_StructDef SOMSTAR SOMLINK ir2_containr_create_struct(
	CORBA_Container SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_RepositoryId id,
	/* in */ CORBA_Identifier name,
	/* in */ CORBA_VersionSpec version,
	/* in */ CORBA_StructMemberSeq *members)
{
	CORBA_StructDef SOMSTAR __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::Container::create_union */
SOM_Scope CORBA_UnionDef SOMSTAR SOMLINK ir2_containr_create_union(
	CORBA_Container SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_RepositoryId id,
	/* in */ CORBA_Identifier name,
	/* in */ CORBA_VersionSpec version,
	/* in */ CORBA_IDLType SOMSTAR discriminator_type,
	/* in */ CORBA_UnionMemberSeq *members)
{
	CORBA_UnionDef SOMSTAR __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::Container::create_enum */
SOM_Scope CORBA_EnumDef SOMSTAR SOMLINK ir2_containr_create_enum(
	CORBA_Container SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_RepositoryId id,
	/* in */ CORBA_Identifier name,
	/* in */ CORBA_VersionSpec version,
	/* in */ CORBA_EnumMemberSeq *members)
{
	CORBA_EnumDef SOMSTAR __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::Container::create_alias */
SOM_Scope CORBA_AliasDef SOMSTAR SOMLINK ir2_containr_create_alias(
	CORBA_Container SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_RepositoryId id,
	/* in */ CORBA_Identifier name,
	/* in */ CORBA_VersionSpec version,
	/* in */ CORBA_IDLType SOMSTAR original_type)
{
	CORBA_AliasDef SOMSTAR __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::Container::create_interface */
SOM_Scope CORBA_InterfaceDef SOMSTAR SOMLINK ir2_containr_create_interface(
	CORBA_Container SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_RepositoryId id,
	/* in */ CORBA_Identifier name,
	/* in */ CORBA_VersionSpec version,
	/* in */ CORBA_InterfaceDefSeq *base_interfaces)
{
	CORBA_InterfaceDef SOMSTAR __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::Container::create_value */
SOM_Scope CORBA_ValueDef SOMSTAR SOMLINK ir2_containr_create_value(
	CORBA_Container SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_RepositoryId id,
	/* in */ CORBA_Identifier name,
	/* in */ CORBA_VersionSpec version,
	/* in */ boolean is_custom,
	/* in */ boolean is_abstract,
	/* in */ CORBA_ValueDef SOMSTAR base_value,
	/* in */ boolean is_truncatable,
	/* in */ CORBA_ValueDefSeq *abstract_base_values,
	/* in */ CORBA_InterfaceDefSeq *supported_interfaces,
	/* in */ CORBA_InitializerSeq *initializers)
{
	CORBA_ValueDef SOMSTAR __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::Container::create_value_box */
SOM_Scope CORBA_ValueBoxDef SOMSTAR SOMLINK ir2_containr_create_value_box(
	CORBA_Container SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_RepositoryId id,
	/* in */ CORBA_Identifier name,
	/* in */ CORBA_VersionSpec version,
	/* in */ CORBA_IDLType SOMSTAR original_type_def)
{
	CORBA_ValueBoxDef SOMSTAR __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::Container::create_exception */
SOM_Scope CORBA_ExceptionDef SOMSTAR SOMLINK ir2_containr_create_exception(
	CORBA_Container SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_RepositoryId id,
	/* in */ CORBA_Identifier name,
	/* in */ CORBA_VersionSpec version,
	/* in */ CORBA_StructMemberSeq *members)
{
	CORBA_ExceptionDef SOMSTAR __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::Container::create_native */
SOM_Scope CORBA_NativeDef SOMSTAR SOMLINK ir2_containr_create_native(
	CORBA_Container SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_RepositoryId id,
	/* in */ CORBA_Identifier name,
	/* in */ CORBA_VersionSpec version)
{
	CORBA_NativeDef SOMSTAR __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::Container::create_abstract_interface */
SOM_Scope CORBA_AbstractInterfaceDef SOMSTAR SOMLINK ir2_containr_create_abstract_interface(
	CORBA_Container SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_RepositoryId id,
	/* in */ CORBA_Identifier name,
	/* in */ CORBA_VersionSpec version,
	/* in */ CORBA_AbstractInterfaceDefSeq *base_interfaces)
{
	CORBA_AbstractInterfaceDef SOMSTAR __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* overridden methods for ::CORBA::IDLType */
/* introduced methods for ::CORBA::IDLType */
/* introduced method ::CORBA::IDLType::_get_type */
SOM_Scope TypeCode SOMLINK ir2_idltype__get_type(
	CORBA_IDLType SOMSTAR somSelf,
	Environment *ev)
{
	TypeCode __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* overridden methods for ::CORBA::Repository */
/* introduced methods for ::CORBA::Repository */
/* introduced method ::CORBA::Repository::lookup_id */
SOM_Scope CORBA_Contained SOMSTAR SOMLINK ir2_repostry_lookup_id(
	CORBA_Repository SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_RepositoryId search_id)
{
	CORBA_Contained SOMSTAR __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::Repository::get_canonical_typecode */
SOM_Scope TypeCode SOMLINK ir2_repostry_get_canonical_typecode(
	CORBA_Repository SOMSTAR somSelf,
	Environment *ev,
	/* in */ TypeCode tc)
{
	TypeCode __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::Repository::get_primitive */
SOM_Scope CORBA_PrimitiveDef SOMSTAR SOMLINK ir2_repostry_get_primitive(
	CORBA_Repository SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_PrimitiveKind kind)
{
	CORBA_PrimitiveDef SOMSTAR __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::Repository::create_string */
SOM_Scope CORBA_StringDef SOMSTAR SOMLINK ir2_repostry_create_string(
	CORBA_Repository SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned long bound)
{
	CORBA_StringDef SOMSTAR __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::Repository::create_wstring */
SOM_Scope CORBA_WstringDef SOMSTAR SOMLINK ir2_repostry_create_wstring(
	CORBA_Repository SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned long bound)
{
	CORBA_WstringDef SOMSTAR __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::Repository::create_sequence */
SOM_Scope CORBA_SequenceDef SOMSTAR SOMLINK ir2_repostry_create_sequence(
	CORBA_Repository SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned long bound,
	/* in */ CORBA_IDLType SOMSTAR element_type)
{
	CORBA_SequenceDef SOMSTAR __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::Repository::create_array */
SOM_Scope CORBA_ArrayDef SOMSTAR SOMLINK ir2_repostry_create_array(
	CORBA_Repository SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned long length,
	/* in */ CORBA_IDLType SOMSTAR element_type)
{
	CORBA_ArrayDef SOMSTAR __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::Repository::create_fixed */
SOM_Scope CORBA_FixedDef SOMSTAR SOMLINK ir2_repostry_create_fixed(
	CORBA_Repository SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned short digits,
	/* in */ short scale)
{
	CORBA_FixedDef SOMSTAR __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* overridden methods for ::CORBA::ModuleDef */
/* introduced methods for ::CORBA::ModuleDef */
/* overridden methods for ::CORBA::ConstantDef */
/* introduced methods for ::CORBA::ConstantDef */
/* introduced method ::CORBA::ConstantDef::_get_type */
SOM_Scope TypeCode SOMLINK ir2_constdef__get_type(
	CORBA_ConstantDef SOMSTAR somSelf,
	Environment *ev)
{
	TypeCode __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::ConstantDef::_set_type_def */
SOM_Scope void SOMLINK ir2_constdef__set_type_def(
	CORBA_ConstantDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_IDLType SOMSTAR type_def)
{
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
}
/* introduced method ::CORBA::ConstantDef::_get_type_def */
SOM_Scope CORBA_IDLType SOMSTAR SOMLINK ir2_constdef__get_type_def(
	CORBA_ConstantDef SOMSTAR somSelf,
	Environment *ev)
{
	CORBA_IDLType SOMSTAR __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::ConstantDef::_set_value */
SOM_Scope void SOMLINK ir2_constdef__set_value(
	CORBA_ConstantDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ any *value)
{
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
}
/* introduced method ::CORBA::ConstantDef::_get_value */
SOM_Scope any SOMLINK ir2_constdef__get_value(
	CORBA_ConstantDef SOMSTAR somSelf,
	Environment *ev)
{
	any __result={NULL,NULL};
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* overridden methods for ::CORBA::TypedefDef */
/* introduced methods for ::CORBA::TypedefDef */
/* overridden methods for ::CORBA::StructDef */
/* introduced methods for ::CORBA::StructDef */
/* introduced method ::CORBA::StructDef::_set_members */
SOM_Scope void SOMLINK ir2_structdf__set_members(
	CORBA_StructDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_StructMemberSeq *members)
{
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
}
/* introduced method ::CORBA::StructDef::_get_members */
SOM_Scope CORBA_StructMemberSeq SOMLINK ir2_structdf__get_members(
	CORBA_StructDef SOMSTAR somSelf,
	Environment *ev)
{
	CORBA_StructMemberSeq __result={0,0,NULL};
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* overridden methods for ::CORBA::UnionDef */
/* introduced methods for ::CORBA::UnionDef */
/* introduced method ::CORBA::UnionDef::_get_discriminator_type */
SOM_Scope TypeCode SOMLINK ir2_uniondef__get_discriminator_type(
	CORBA_UnionDef SOMSTAR somSelf,
	Environment *ev)
{
	TypeCode __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::UnionDef::_set_discriminator_type_def */
SOM_Scope void SOMLINK ir2_uniondef__set_discriminator_type_def(
	CORBA_UnionDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_IDLType SOMSTAR discriminator_type_def)
{
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
}
/* introduced method ::CORBA::UnionDef::_get_discriminator_type_def */
SOM_Scope CORBA_IDLType SOMSTAR SOMLINK ir2_uniondef__get_discriminator_type_def(
	CORBA_UnionDef SOMSTAR somSelf,
	Environment *ev)
{
	CORBA_IDLType SOMSTAR __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::UnionDef::_set_members */
SOM_Scope void SOMLINK ir2_uniondef__set_members(
	CORBA_UnionDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_UnionMemberSeq *members)
{
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
}
/* introduced method ::CORBA::UnionDef::_get_members */
SOM_Scope CORBA_UnionMemberSeq SOMLINK ir2_uniondef__get_members(
	CORBA_UnionDef SOMSTAR somSelf,
	Environment *ev)
{
	CORBA_UnionMemberSeq __result={0,0,NULL};
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* overridden methods for ::CORBA::EnumDef */
/* introduced methods for ::CORBA::EnumDef */
/* introduced method ::CORBA::EnumDef::_set_members */
SOM_Scope void SOMLINK ir2_enumdef__set_members(
	CORBA_EnumDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_EnumMemberSeq *members)
{
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
}
/* introduced method ::CORBA::EnumDef::_get_members */
SOM_Scope CORBA_EnumMemberSeq SOMLINK ir2_enumdef__get_members(
	CORBA_EnumDef SOMSTAR somSelf,
	Environment *ev)
{
	CORBA_EnumMemberSeq __result={0,0,NULL};
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* overridden methods for ::CORBA::AliasDef */
/* introduced methods for ::CORBA::AliasDef */
/* introduced method ::CORBA::AliasDef::_set_original_type_def */
SOM_Scope void SOMLINK ir2_aliasdef__set_original_type_def(
	CORBA_AliasDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_IDLType SOMSTAR original_type_def)
{
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
}
/* introduced method ::CORBA::AliasDef::_get_original_type_def */
SOM_Scope CORBA_IDLType SOMSTAR SOMLINK ir2_aliasdef__get_original_type_def(
	CORBA_AliasDef SOMSTAR somSelf,
	Environment *ev)
{
	CORBA_IDLType SOMSTAR __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* overridden methods for ::CORBA::NativeDef */
/* introduced methods for ::CORBA::NativeDef */
/* overridden methods for ::CORBA::PrimitiveDef */
/* introduced methods for ::CORBA::PrimitiveDef */
/* introduced method ::CORBA::PrimitiveDef::_get_kind */
SOM_Scope CORBA_PrimitiveKind SOMLINK ir2_primitiv__get_kind(
	CORBA_PrimitiveDef SOMSTAR somSelf,
	Environment *ev)
{
	CORBA_PrimitiveKind __result=0;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* overridden methods for ::CORBA::StringDef */
/* introduced methods for ::CORBA::StringDef */
/* introduced method ::CORBA::StringDef::_set_bound */
SOM_Scope void SOMLINK ir2_stringdf__set_bound(
	CORBA_StringDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned long bound)
{
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
}
/* introduced method ::CORBA::StringDef::_get_bound */
SOM_Scope unsigned long SOMLINK ir2_stringdf__get_bound(
	CORBA_StringDef SOMSTAR somSelf,
	Environment *ev)
{
	unsigned long __result=0;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* overridden methods for ::CORBA::WstringDef */
/* introduced methods for ::CORBA::WstringDef */
/* introduced method ::CORBA::WstringDef::_set_bound */
SOM_Scope void SOMLINK ir2_wstringd__set_bound(
	CORBA_WstringDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned long bound)
{
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
}
/* introduced method ::CORBA::WstringDef::_get_bound */
SOM_Scope unsigned long SOMLINK ir2_wstringd__get_bound(
	CORBA_WstringDef SOMSTAR somSelf,
	Environment *ev)
{
	unsigned long __result=0;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* overridden methods for ::CORBA::FixedDef */
/* introduced methods for ::CORBA::FixedDef */
/* introduced method ::CORBA::FixedDef::_set_digits */
SOM_Scope void SOMLINK ir2_fixeddef__set_digits(
	CORBA_FixedDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned short digits)
{
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
}
/* introduced method ::CORBA::FixedDef::_get_digits */
SOM_Scope unsigned short SOMLINK ir2_fixeddef__get_digits(
	CORBA_FixedDef SOMSTAR somSelf,
	Environment *ev)
{
	unsigned short __result=0;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::FixedDef::_set_scale */
SOM_Scope void SOMLINK ir2_fixeddef__set_scale(
	CORBA_FixedDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ short scale)
{
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
}
/* introduced method ::CORBA::FixedDef::_get_scale */
SOM_Scope short SOMLINK ir2_fixeddef__get_scale(
	CORBA_FixedDef SOMSTAR somSelf,
	Environment *ev)
{
	short __result=0;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* overridden methods for ::CORBA::SequenceDef */
/* introduced methods for ::CORBA::SequenceDef */
/* introduced method ::CORBA::SequenceDef::_set_bound */
SOM_Scope void SOMLINK ir2_seqdef__set_bound(
	CORBA_SequenceDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned long bound)
{
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
}
/* introduced method ::CORBA::SequenceDef::_get_bound */
SOM_Scope unsigned long SOMLINK ir2_seqdef__get_bound(
	CORBA_SequenceDef SOMSTAR somSelf,
	Environment *ev)
{
	unsigned long __result=0;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::SequenceDef::_get_element_type */
SOM_Scope TypeCode SOMLINK ir2_seqdef__get_element_type(
	CORBA_SequenceDef SOMSTAR somSelf,
	Environment *ev)
{
	TypeCode __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::SequenceDef::_set_element_type_def */
SOM_Scope void SOMLINK ir2_seqdef__set_element_type_def(
	CORBA_SequenceDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_IDLType SOMSTAR element_type_def)
{
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
}
/* introduced method ::CORBA::SequenceDef::_get_element_type_def */
SOM_Scope CORBA_IDLType SOMSTAR SOMLINK ir2_seqdef__get_element_type_def(
	CORBA_SequenceDef SOMSTAR somSelf,
	Environment *ev)
{
	CORBA_IDLType SOMSTAR __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* overridden methods for ::CORBA::ArrayDef */
/* introduced methods for ::CORBA::ArrayDef */
/* introduced method ::CORBA::ArrayDef::_set_length */
SOM_Scope void SOMLINK ir2_arraydef__set_length(
	CORBA_ArrayDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned long length)
{
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
}
/* introduced method ::CORBA::ArrayDef::_get_length */
SOM_Scope unsigned long SOMLINK ir2_arraydef__get_length(
	CORBA_ArrayDef SOMSTAR somSelf,
	Environment *ev)
{
	unsigned long __result=0;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::ArrayDef::_get_element_type */
SOM_Scope TypeCode SOMLINK ir2_arraydef__get_element_type(
	CORBA_ArrayDef SOMSTAR somSelf,
	Environment *ev)
{
	TypeCode __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::ArrayDef::_set_element_type_def */
SOM_Scope void SOMLINK ir2_arraydef__set_element_type_def(
	CORBA_ArrayDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_IDLType SOMSTAR element_type_def)
{
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
}
/* introduced method ::CORBA::ArrayDef::_get_element_type_def */
SOM_Scope CORBA_IDLType SOMSTAR SOMLINK ir2_arraydef__get_element_type_def(
	CORBA_ArrayDef SOMSTAR somSelf,
	Environment *ev)
{
	CORBA_IDLType SOMSTAR __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* overridden methods for ::CORBA::ExceptionDef */
/* introduced methods for ::CORBA::ExceptionDef */
/* introduced method ::CORBA::ExceptionDef::_get_type */
SOM_Scope TypeCode SOMLINK ir2_exceptdf__get_type(
	CORBA_ExceptionDef SOMSTAR somSelf,
	Environment *ev)
{
	TypeCode __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::ExceptionDef::_set_members */
SOM_Scope void SOMLINK ir2_exceptdf__set_members(
	CORBA_ExceptionDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_StructMemberSeq *members)
{
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
}
/* introduced method ::CORBA::ExceptionDef::_get_members */
SOM_Scope CORBA_StructMemberSeq SOMLINK ir2_exceptdf__get_members(
	CORBA_ExceptionDef SOMSTAR somSelf,
	Environment *ev)
{
	CORBA_StructMemberSeq __result={0,0,NULL};
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* overridden methods for ::CORBA::AttributeDef */
/* introduced methods for ::CORBA::AttributeDef */
/* introduced method ::CORBA::AttributeDef::_get_type */
SOM_Scope TypeCode SOMLINK ir2_attribdf__get_type(
	CORBA_AttributeDef SOMSTAR somSelf,
	Environment *ev)
{
	TypeCode __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::AttributeDef::_set_type_def */
SOM_Scope void SOMLINK ir2_attribdf__set_type_def(
	CORBA_AttributeDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_IDLType SOMSTAR type_def)
{
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
}
/* introduced method ::CORBA::AttributeDef::_get_type_def */
SOM_Scope CORBA_IDLType SOMSTAR SOMLINK ir2_attribdf__get_type_def(
	CORBA_AttributeDef SOMSTAR somSelf,
	Environment *ev)
{
	CORBA_IDLType SOMSTAR __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::AttributeDef::_set_mode */
SOM_Scope void SOMLINK ir2_attribdf__set_mode(
	CORBA_AttributeDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_AttributeMode mode)
{
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
}
/* introduced method ::CORBA::AttributeDef::_get_mode */
SOM_Scope CORBA_AttributeMode SOMLINK ir2_attribdf__get_mode(
	CORBA_AttributeDef SOMSTAR somSelf,
	Environment *ev)
{
	CORBA_AttributeMode __result=0;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* overridden methods for ::CORBA::OperationDef */
/* introduced methods for ::CORBA::OperationDef */
/* introduced method ::CORBA::OperationDef::_get_result */
SOM_Scope TypeCode SOMLINK ir2_operatdf__get_result(
	CORBA_OperationDef SOMSTAR somSelf,
	Environment *ev)
{
	TypeCode __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::OperationDef::_set_result_def */
SOM_Scope void SOMLINK ir2_operatdf__set_result_def(
	CORBA_OperationDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_IDLType SOMSTAR result_def)
{
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
}
/* introduced method ::CORBA::OperationDef::_get_result_def */
SOM_Scope CORBA_IDLType SOMSTAR SOMLINK ir2_operatdf__get_result_def(
	CORBA_OperationDef SOMSTAR somSelf,
	Environment *ev)
{
	CORBA_IDLType SOMSTAR __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::OperationDef::_set_params */
SOM_Scope void SOMLINK ir2_operatdf__set_params(
	CORBA_OperationDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_ParDescriptionSeq *params)
{
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
}
/* introduced method ::CORBA::OperationDef::_get_params */
SOM_Scope CORBA_ParDescriptionSeq SOMLINK ir2_operatdf__get_params(
	CORBA_OperationDef SOMSTAR somSelf,
	Environment *ev)
{
	CORBA_ParDescriptionSeq __result={0,0,NULL};
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::OperationDef::_set_mode */
SOM_Scope void SOMLINK ir2_operatdf__set_mode(
	CORBA_OperationDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_OperationMode mode)
{
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
}
/* introduced method ::CORBA::OperationDef::_get_mode */
SOM_Scope CORBA_OperationMode SOMLINK ir2_operatdf__get_mode(
	CORBA_OperationDef SOMSTAR somSelf,
	Environment *ev)
{
	CORBA_OperationMode __result=0;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::OperationDef::_set_contexts */
SOM_Scope void SOMLINK ir2_operatdf__set_contexts(
	CORBA_OperationDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_ContextIdSeq *contexts)
{
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
}
/* introduced method ::CORBA::OperationDef::_get_contexts */
SOM_Scope CORBA_ContextIdSeq SOMLINK ir2_operatdf__get_contexts(
	CORBA_OperationDef SOMSTAR somSelf,
	Environment *ev)
{
	CORBA_ContextIdSeq __result={0,0,NULL};
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::OperationDef::_set_exceptions */
SOM_Scope void SOMLINK ir2_operatdf__set_exceptions(
	CORBA_OperationDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_ExceptionDefSeq *exceptions)
{
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
}
/* introduced method ::CORBA::OperationDef::_get_exceptions */
SOM_Scope CORBA_ExceptionDefSeq SOMLINK ir2_operatdf__get_exceptions(
	CORBA_OperationDef SOMSTAR somSelf,
	Environment *ev)
{
	CORBA_ExceptionDefSeq __result={0,0,NULL};
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* overridden methods for ::CORBA::InterfaceDef */
/* introduced methods for ::CORBA::InterfaceDef */
/* introduced method ::CORBA::InterfaceDef::_set_base_interfaces */
SOM_Scope void SOMLINK ir2_intfacdf__set_base_interfaces(
	CORBA_InterfaceDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_InterfaceDefSeq *base_interfaces)
{
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
}
/* introduced method ::CORBA::InterfaceDef::_get_base_interfaces */
SOM_Scope CORBA_InterfaceDefSeq SOMLINK ir2_intfacdf__get_base_interfaces(
	CORBA_InterfaceDef SOMSTAR somSelf,
	Environment *ev)
{
	CORBA_InterfaceDefSeq __result={0,0,NULL};
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::InterfaceDef::is_a */
SOM_Scope boolean SOMLINK ir2_intfacdf_is_a(
	CORBA_InterfaceDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_RepositoryId interface_id)
{
	boolean __result=0;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::InterfaceDef::describe_interface */
SOM_Scope CORBA_InterfaceDef_FullInterfaceDescription SOMLINK ir2_intfacdf_describe_interface(
	CORBA_InterfaceDef SOMSTAR somSelf,
	Environment *ev)
{
	CORBA_InterfaceDef_FullInterfaceDescription __result={NULL,NULL,NULL,NULL,{0,0,NULL},{0,0,NULL},{0,0,NULL},NULL};
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::InterfaceDef::create_attribute */
SOM_Scope CORBA_AttributeDef SOMSTAR SOMLINK ir2_intfacdf_create_attribute(
	CORBA_InterfaceDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_RepositoryId id,
	/* in */ CORBA_Identifier name,
	/* in */ CORBA_VersionSpec version,
	/* in */ CORBA_IDLType SOMSTAR type,
	/* in */ CORBA_AttributeMode mode)
{
	CORBA_AttributeDef SOMSTAR __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::InterfaceDef::create_operation */
SOM_Scope CORBA_OperationDef SOMSTAR SOMLINK ir2_intfacdf_create_operation(
	CORBA_InterfaceDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_RepositoryId id,
	/* in */ CORBA_Identifier name,
	/* in */ CORBA_VersionSpec version,
	/* in */ CORBA_IDLType SOMSTAR result,
	/* in */ CORBA_OperationMode mode,
	/* in */ CORBA_ParDescriptionSeq *params,
	/* in */ CORBA_ExceptionDefSeq *exceptions,
	/* in */ CORBA_ContextIdSeq *contexts)
{
	CORBA_OperationDef SOMSTAR __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* overridden methods for ::CORBA::ValueMemberDef */
/* introduced methods for ::CORBA::ValueMemberDef */
/* introduced method ::CORBA::ValueMemberDef::_get_type */
SOM_Scope TypeCode SOMLINK ir2_valmemdf__get_type(
	CORBA_ValueMemberDef SOMSTAR somSelf,
	Environment *ev)
{
	TypeCode __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::ValueMemberDef::_set_type_def */
SOM_Scope void SOMLINK ir2_valmemdf__set_type_def(
	CORBA_ValueMemberDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_IDLType SOMSTAR type_def)
{
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
}
/* introduced method ::CORBA::ValueMemberDef::_get_type_def */
SOM_Scope CORBA_IDLType SOMSTAR SOMLINK ir2_valmemdf__get_type_def(
	CORBA_ValueMemberDef SOMSTAR somSelf,
	Environment *ev)
{
	CORBA_IDLType SOMSTAR __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::ValueMemberDef::_set_access */
SOM_Scope void SOMLINK ir2_valmemdf__set_access(
	CORBA_ValueMemberDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_Visibility access)
{
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
}
/* introduced method ::CORBA::ValueMemberDef::_get_access */
SOM_Scope CORBA_Visibility SOMLINK ir2_valmemdf__get_access(
	CORBA_ValueMemberDef SOMSTAR somSelf,
	Environment *ev)
{
	CORBA_Visibility __result=0;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* overridden methods for ::CORBA::ValueDef */
/* introduced methods for ::CORBA::ValueDef */
/* introduced method ::CORBA::ValueDef::_set_supported_interfaces */
SOM_Scope void SOMLINK ir2_valuedef__set_supported_interfaces(
	CORBA_ValueDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_InterfaceDefSeq *supported_interfaces)
{
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
}
/* introduced method ::CORBA::ValueDef::_get_supported_interfaces */
SOM_Scope CORBA_InterfaceDefSeq SOMLINK ir2_valuedef__get_supported_interfaces(
	CORBA_ValueDef SOMSTAR somSelf,
	Environment *ev)
{
	CORBA_InterfaceDefSeq __result={0,0,NULL};
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::ValueDef::_set_initializers */
SOM_Scope void SOMLINK ir2_valuedef__set_initializers(
	CORBA_ValueDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_InitializerSeq *initializers)
{
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
}
/* introduced method ::CORBA::ValueDef::_get_initializers */
SOM_Scope CORBA_InitializerSeq SOMLINK ir2_valuedef__get_initializers(
	CORBA_ValueDef SOMSTAR somSelf,
	Environment *ev)
{
	CORBA_InitializerSeq __result={0,0,NULL};
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::ValueDef::_set_base_value */
SOM_Scope void SOMLINK ir2_valuedef__set_base_value(
	CORBA_ValueDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_ValueDef SOMSTAR base_value)
{
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
}
/* introduced method ::CORBA::ValueDef::_get_base_value */
SOM_Scope CORBA_ValueDef SOMSTAR SOMLINK ir2_valuedef__get_base_value(
	CORBA_ValueDef SOMSTAR somSelf,
	Environment *ev)
{
	CORBA_ValueDef SOMSTAR __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::ValueDef::_set_abstract_base_values */
SOM_Scope void SOMLINK ir2_valuedef__set_abstract_base_values(
	CORBA_ValueDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_ValueDefSeq *abstract_base_values)
{
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
}
/* introduced method ::CORBA::ValueDef::_get_abstract_base_values */
SOM_Scope CORBA_ValueDefSeq SOMLINK ir2_valuedef__get_abstract_base_values(
	CORBA_ValueDef SOMSTAR somSelf,
	Environment *ev)
{
	CORBA_ValueDefSeq __result={0,0,NULL};
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::ValueDef::_set_is_abstract */
SOM_Scope void SOMLINK ir2_valuedef__set_is_abstract(
	CORBA_ValueDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ boolean is_abstract)
{
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
}
/* introduced method ::CORBA::ValueDef::_get_is_abstract */
SOM_Scope boolean SOMLINK ir2_valuedef__get_is_abstract(
	CORBA_ValueDef SOMSTAR somSelf,
	Environment *ev)
{
	boolean __result=0;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::ValueDef::_set_is_custom */
SOM_Scope void SOMLINK ir2_valuedef__set_is_custom(
	CORBA_ValueDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ boolean is_custom)
{
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
}
/* introduced method ::CORBA::ValueDef::_get_is_custom */
SOM_Scope boolean SOMLINK ir2_valuedef__get_is_custom(
	CORBA_ValueDef SOMSTAR somSelf,
	Environment *ev)
{
	boolean __result=0;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::ValueDef::_set_is_truncatable */
SOM_Scope void SOMLINK ir2_valuedef__set_is_truncatable(
	CORBA_ValueDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ boolean is_truncatable)
{
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
}
/* introduced method ::CORBA::ValueDef::_get_is_truncatable */
SOM_Scope boolean SOMLINK ir2_valuedef__get_is_truncatable(
	CORBA_ValueDef SOMSTAR somSelf,
	Environment *ev)
{
	boolean __result=0;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::ValueDef::is_a */
SOM_Scope boolean SOMLINK ir2_valuedef_is_a(
	CORBA_ValueDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_RepositoryId id)
{
	boolean __result=0;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::ValueDef::describe_value */
SOM_Scope CORBA_ValueDef_FullValueDescription SOMLINK ir2_valuedef_describe_value(
	CORBA_ValueDef SOMSTAR somSelf,
	Environment *ev)
{
	CORBA_ValueDef_FullValueDescription __result={NULL,NULL,0,0,NULL,NULL,{0,0,NULL},{0,0,NULL},{0,0,NULL},{0,0,NULL},{0,0,NULL},{0,0,NULL},0,NULL,NULL};
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::ValueDef::create_value_member */
SOM_Scope CORBA_ValueMemberDef SOMSTAR SOMLINK ir2_valuedef_create_value_member(
	CORBA_ValueDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_RepositoryId id,
	/* in */ CORBA_Identifier name,
	/* in */ CORBA_VersionSpec version,
	/* in */ CORBA_IDLType SOMSTAR type,
	/* in */ CORBA_Visibility access)
{
	CORBA_ValueMemberDef SOMSTAR __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::ValueDef::create_attribute */
SOM_Scope CORBA_AttributeDef SOMSTAR SOMLINK ir2_valuedef_create_attribute(
	CORBA_ValueDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_RepositoryId id,
	/* in */ CORBA_Identifier name,
	/* in */ CORBA_VersionSpec version,
	/* in */ CORBA_IDLType SOMSTAR type,
	/* in */ CORBA_AttributeMode mode)
{
	CORBA_AttributeDef SOMSTAR __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* introduced method ::CORBA::ValueDef::create_operation */
SOM_Scope CORBA_OperationDef SOMSTAR SOMLINK ir2_valuedef_create_operation(
	CORBA_ValueDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_RepositoryId id,
	/* in */ CORBA_Identifier name,
	/* in */ CORBA_VersionSpec version,
	/* in */ CORBA_IDLType SOMSTAR result,
	/* in */ CORBA_OperationMode mode,
	/* in */ CORBA_ParDescriptionSeq *params,
	/* in */ CORBA_ExceptionDefSeq *exceptions,
	/* in */ CORBA_ContextIdSeq *contexts)
{
	CORBA_OperationDef SOMSTAR __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* overridden methods for ::CORBA::ValueBoxDef */
/* introduced methods for ::CORBA::ValueBoxDef */
/* introduced method ::CORBA::ValueBoxDef::_set_original_type_def */
SOM_Scope void SOMLINK ir2_valboxdf__set_original_type_def(
	CORBA_ValueBoxDef SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_IDLType SOMSTAR original_type_def)
{
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
}
/* introduced method ::CORBA::ValueBoxDef::_get_original_type_def */
SOM_Scope CORBA_IDLType SOMSTAR SOMLINK ir2_valboxdf__get_original_type_def(
	CORBA_ValueBoxDef SOMSTAR somSelf,
	Environment *ev)
{
	CORBA_IDLType SOMSTAR __result=NULL;
	if (!ev->_major) {
		StExcep *ex=SOMMalloc(sizeof(ex));
		if (ex) { ex->minor=0; ex->completed=NO; }
		somSetException(ev,SYSTEM_EXCEPTION,"CORBA::NO_IMPLEMENT",ex);
	}
	return __result;
}
/* overridden methods for ::CORBA::AbstractInterfaceDef */
/* introduced methods for ::CORBA::AbstractInterfaceDef */


SOM_Scope void SOMLINK policy_destroy(
		CORBA_Policy SOMSTAR somSelf,
		Environment *ev)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
}

SOM_Scope CORBA_Policy SOMSTAR SOMLINK policy_copy(
		CORBA_Policy SOMSTAR somSelf,
		Environment *ev)
{
	SOM_IgnoreWarning(somSelf);
	RHBOPT_throw_StExcep(ev,NO_PERMISSION,AccessDenied,NO);
	return NULL;
}

SOM_Scope CORBA_PolicyType SOMLINK policy__get_policy_type(
		CORBA_Policy SOMSTAR somSelf,
		Environment *ev)
{
	SOM_IgnoreWarning(somSelf);
	RHBOPT_throw_StExcep(ev,NO_PERMISSION,AccessDenied,NO);
	return 0;
}


static void f_throw_marshal(char *,int,Environment *ev);
#define throw_marshal(ev)  f_throw_marshal(__FILE__,__LINE__,ev)

/* overridden methods for ::CORBA::DataOutputStream */
/* introduced methods for ::CORBA::DataOutputStream */
/* introduced method ::CORBA::DataOutputStream::write_any */
SOM_Scope void SOMLINK doutstr_write_any(
	CORBA_DataOutputStream SOMSTAR somSelf,
	Environment *ev,
	/* in */ any *value)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(value);

	throw_marshal(ev);
}
/* introduced method ::CORBA::DataOutputStream::write_boolean */
SOM_Scope void SOMLINK doutstr_write_boolean(
	CORBA_DataOutputStream SOMSTAR somSelf,
	Environment *ev,
	/* in */ boolean value)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(value);
	throw_marshal(ev);
}
/* introduced method ::CORBA::DataOutputStream::write_char */
SOM_Scope void SOMLINK doutstr_write_char(
	CORBA_DataOutputStream SOMSTAR somSelf,
	Environment *ev,
	/* in */ char value)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(value);
	throw_marshal(ev);
}
/* introduced method ::CORBA::DataOutputStream::write_octet */
SOM_Scope void SOMLINK doutstr_write_octet(
	CORBA_DataOutputStream SOMSTAR somSelf,
	Environment *ev,
	/* in */ octet value)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(value);
	throw_marshal(ev);
}
/* introduced method ::CORBA::DataOutputStream::write_short */
SOM_Scope void SOMLINK doutstr_write_short(
	CORBA_DataOutputStream SOMSTAR somSelf,
	Environment *ev,
	/* in */ short value)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(value);
	throw_marshal(ev);
}
/* introduced method ::CORBA::DataOutputStream::write_ushort */
SOM_Scope void SOMLINK doutstr_write_ushort(
	CORBA_DataOutputStream SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned short value)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(value);
	throw_marshal(ev);
}
/* introduced method ::CORBA::DataOutputStream::write_long */
SOM_Scope void SOMLINK doutstr_write_long(
	CORBA_DataOutputStream SOMSTAR somSelf,
	Environment *ev,
	/* in */ long value)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(value);
	throw_marshal(ev);
}
/* introduced method ::CORBA::DataOutputStream::write_ulong */
SOM_Scope void SOMLINK doutstr_write_ulong(
	CORBA_DataOutputStream SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned long value)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(value);
	throw_marshal(ev);
}
/* introduced method ::CORBA::DataOutputStream::write_float */
SOM_Scope void SOMLINK doutstr_write_float(
	CORBA_DataOutputStream SOMSTAR somSelf,
	Environment *ev,
	/* in */ float value)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(value);
	throw_marshal(ev);
}
/* introduced method ::CORBA::DataOutputStream::write_double */
SOM_Scope void SOMLINK doutstr_write_double(
	CORBA_DataOutputStream SOMSTAR somSelf,
	Environment *ev,
	/* in */ double value)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(value);
	throw_marshal(ev);
}
/* introduced method ::CORBA::DataOutputStream::write_string */
SOM_Scope void SOMLINK doutstr_write_string(
	CORBA_DataOutputStream SOMSTAR somSelf,
	Environment *ev,
	/* in */ char * value)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(value);
	throw_marshal(ev);
}
/* introduced method ::CORBA::DataOutputStream::write_Object */
SOM_Scope void SOMLINK doutstr_write_Object(
	CORBA_DataOutputStream SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMObject SOMSTAR value)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(value);
	throw_marshal(ev);
}
/* introduced method ::CORBA::DataOutputStream::write_Abstract */
SOM_Scope void SOMLINK doutstr_write_Abstract(
	CORBA_DataOutputStream SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_AbstractBase SOMSTAR value)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(value);
	throw_marshal(ev);
}
/* introduced method ::CORBA::DataOutputStream::write_Value */
SOM_Scope void SOMLINK doutstr_write_Value(
	CORBA_DataOutputStream SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_ValueBase SOMSTAR value)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(value);
	throw_marshal(ev);
}
/* introduced method ::CORBA::DataOutputStream::write_TypeCode */
SOM_Scope void SOMLINK doutstr_write_TypeCode(
	CORBA_DataOutputStream SOMSTAR somSelf,
	Environment *ev,
	/* in */ TypeCode value)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(value);
	throw_marshal(ev);
}
/* introduced method ::CORBA::DataOutputStream::write_any_array */
SOM_Scope void SOMLINK doutstr_write_any_array(
	CORBA_DataOutputStream SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_AnySeq *seq,
	/* in */ unsigned long offset,
	/* in */ unsigned long length)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(seq);
	SOM_IgnoreWarning(offset);
	SOM_IgnoreWarning(length);
	throw_marshal(ev);
}
/* introduced method ::CORBA::DataOutputStream::write_boolean_array */
SOM_Scope void SOMLINK doutstr_write_boolean_array(
	CORBA_DataOutputStream SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_BooleanSeq *seq,
	/* in */ unsigned long offset,
	/* in */ unsigned long length)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(seq);
	SOM_IgnoreWarning(offset);
	SOM_IgnoreWarning(length);
	throw_marshal(ev);
}
/* introduced method ::CORBA::DataOutputStream::write_char_array */
SOM_Scope void SOMLINK doutstr_write_char_array(
	CORBA_DataOutputStream SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_CharSeq *seq,
	/* in */ unsigned long offset,
	/* in */ unsigned long length)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(seq);
	SOM_IgnoreWarning(offset);
	SOM_IgnoreWarning(length);
	throw_marshal(ev);
}
/* introduced method ::CORBA::DataOutputStream::write_octet_array */
SOM_Scope void SOMLINK doutstr_write_octet_array(
	CORBA_DataOutputStream SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_OctetSeq *seq,
	/* in */ unsigned long offset,
	/* in */ unsigned long length)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(seq);
	SOM_IgnoreWarning(offset);
	SOM_IgnoreWarning(length);
	throw_marshal(ev);
}
/* introduced method ::CORBA::DataOutputStream::write_short_array */
SOM_Scope void SOMLINK doutstr_write_short_array(
	CORBA_DataOutputStream SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_ShortSeq *seq,
	/* in */ unsigned long offset,
	/* in */ unsigned long length)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(seq);
	SOM_IgnoreWarning(offset);
	SOM_IgnoreWarning(length);
	throw_marshal(ev);
}
/* introduced method ::CORBA::DataOutputStream::write_ushort_array */
SOM_Scope void SOMLINK doutstr_write_ushort_array(
	CORBA_DataOutputStream SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_UShortSeq *seq,
	/* in */ unsigned long offset,
	/* in */ unsigned long length)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(seq);
	SOM_IgnoreWarning(offset);
	SOM_IgnoreWarning(length);
	throw_marshal(ev);
}
/* introduced method ::CORBA::DataOutputStream::write_long_array */
SOM_Scope void SOMLINK doutstr_write_long_array(
	CORBA_DataOutputStream SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_LongSeq *seq,
	/* in */ unsigned long offset,
	/* in */ unsigned long length)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(seq);
	SOM_IgnoreWarning(offset);
	SOM_IgnoreWarning(length);

	throw_marshal(ev);
}
/* introduced method ::CORBA::DataOutputStream::write_ulong_array */
SOM_Scope void SOMLINK doutstr_write_ulong_array(
	CORBA_DataOutputStream SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_ULongSeq *seq,
	/* in */ unsigned long offset,
	/* in */ unsigned long length)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(seq);
	SOM_IgnoreWarning(offset);
	SOM_IgnoreWarning(length);
	throw_marshal(ev);
}
/* introduced method ::CORBA::DataOutputStream::write_float_array */
SOM_Scope void SOMLINK doutstr_write_float_array(
	CORBA_DataOutputStream SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_FloatSeq *seq,
	/* in */ unsigned long offset,
	/* in */ unsigned long length)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(seq);
	SOM_IgnoreWarning(offset);
	SOM_IgnoreWarning(length);
	throw_marshal(ev);
}
/* introduced method ::CORBA::DataOutputStream::write_double_array */
SOM_Scope void SOMLINK doutstr_write_double_array(
	CORBA_DataOutputStream SOMSTAR somSelf,
	Environment *ev,
	/* in */ CORBA_DoubleSeq *seq,
	/* in */ unsigned long offset,
	/* in */ unsigned long length)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(seq);
	SOM_IgnoreWarning(offset);
	SOM_IgnoreWarning(length);
	throw_marshal(ev);
}

static void f_throw_marshal(char *file,int line,Environment *ev)
{
#ifdef DEBUG_MARSHAL
	somPrintf("%s:%d, throw marshal\n",file,line);
#endif
	SOM_IgnoreWarning(file);
	SOM_IgnoreWarning(line);

	RHBOPT_throw_StExcep(ev,MARSHAL,Marshal,MAYBE);
}


/* overridden methods for ::CORBA::DataInputStream */
/* introduced methods for ::CORBA::DataInputStream */
/* introduced method ::CORBA::DataInputStream::read_any */
SOM_Scope any SOMLINK dinstr_read_any(
	CORBA_DataInputStream SOMSTAR somSelf,
	Environment *ev)
{
	any __result={NULL,NULL};
	throw_marshal(ev);

	SOM_IgnoreWarning(somSelf);

	return __result;
}
/* introduced method ::CORBA::DataInputStream::read_boolean */
SOM_Scope boolean SOMLINK dinstr_read_boolean(
	CORBA_DataInputStream SOMSTAR somSelf,
	Environment *ev)
{
	boolean __result=0;
	SOM_IgnoreWarning(somSelf);
	throw_marshal(ev);
	return __result;
}
/* introduced method ::CORBA::DataInputStream::read_char */
SOM_Scope char SOMLINK dinstr_read_char(
	CORBA_DataInputStream SOMSTAR somSelf,
	Environment *ev)
{
	char __result=0;
	SOM_IgnoreWarning(somSelf);
	throw_marshal(ev);
	return __result;
}
/* introduced method ::CORBA::DataInputStream::read_octet */
SOM_Scope octet SOMLINK dinstr_read_octet(
	CORBA_DataInputStream SOMSTAR somSelf,
	Environment *ev)
{
	octet __result=0;
	SOM_IgnoreWarning(somSelf);
	throw_marshal(ev);
	return __result;
}
/* introduced method ::CORBA::DataInputStream::read_short */
SOM_Scope short SOMLINK dinstr_read_short(
	CORBA_DataInputStream SOMSTAR somSelf,
	Environment *ev)
{
	short __result=0;
	SOM_IgnoreWarning(somSelf);
	throw_marshal(ev);
	return __result;
}
/* introduced method ::CORBA::DataInputStream::read_ushort */
SOM_Scope unsigned short SOMLINK dinstr_read_ushort(
	CORBA_DataInputStream SOMSTAR somSelf,
	Environment *ev)
{
	unsigned short __result=0;
	SOM_IgnoreWarning(somSelf);
	throw_marshal(ev);
	return __result;
}
/* introduced method ::CORBA::DataInputStream::read_long */
SOM_Scope long SOMLINK dinstr_read_long(
	CORBA_DataInputStream SOMSTAR somSelf,
	Environment *ev)
{
	long __result=0;
	SOM_IgnoreWarning(somSelf);
	throw_marshal(ev);
	return __result;
}
/* introduced method ::CORBA::DataInputStream::read_ulong */
SOM_Scope unsigned long SOMLINK dinstr_read_ulong(
	CORBA_DataInputStream SOMSTAR somSelf,
	Environment *ev)
{
	unsigned long __result=0;
	SOM_IgnoreWarning(somSelf);
	throw_marshal(ev);
	return __result;
}
/* introduced method ::CORBA::DataInputStream::read_float */
SOM_Scope float SOMLINK dinstr_read_float(
	CORBA_DataInputStream SOMSTAR somSelf,
	Environment *ev)
{
	float __result=(float)0.0;
	SOM_IgnoreWarning(somSelf);
	throw_marshal(ev);
	return __result;
}
/* introduced method ::CORBA::DataInputStream::read_double */
SOM_Scope double SOMLINK dinstr_read_double(
	CORBA_DataInputStream SOMSTAR somSelf,
	Environment *ev)
{
	double __result=0;
	SOM_IgnoreWarning(somSelf);
	throw_marshal(ev);
	return __result;
}
/* introduced method ::CORBA::DataInputStream::read_string */
SOM_Scope char * SOMLINK dinstr_read_string(
	CORBA_DataInputStream SOMSTAR somSelf,
	Environment *ev)
{
	char * __result=0;
	SOM_IgnoreWarning(somSelf);
	throw_marshal(ev);
	return __result;
}
/* introduced method ::CORBA::DataInputStream::read_Object */
SOM_Scope SOMObject SOMSTAR SOMLINK dinstr_read_Object(
	CORBA_DataInputStream SOMSTAR somSelf,
	Environment *ev)
{
	SOM_IgnoreWarning(somSelf);
	throw_marshal(ev);
	return NULL;
}

/* introduced method ::CORBA::DataInputStream::read_Abstract */
SOM_Scope CORBA_AbstractBase SOMSTAR SOMLINK dinstr_read_Abstract(
	CORBA_DataInputStream SOMSTAR somSelf,
	Environment *ev)
{
	CORBA_AbstractBase SOMSTAR __result=0;
	SOM_IgnoreWarning(somSelf);
	throw_marshal(ev);
	return __result;
}
/* introduced method ::CORBA::DataInputStream::read_Value */
SOM_Scope CORBA_ValueBase SOMSTAR SOMLINK dinstr_read_Value(
	CORBA_DataInputStream SOMSTAR somSelf,
	Environment *ev)
{
	CORBA_ValueBase SOMSTAR __result=0;
	SOM_IgnoreWarning(somSelf);
	throw_marshal(ev);
	return __result;
}
/* introduced method ::CORBA::DataInputStream::read_TypeCode */
SOM_Scope TypeCode SOMLINK dinstr_read_TypeCode(
	CORBA_DataInputStream SOMSTAR somSelf,
	Environment *ev)
{
	TypeCode __result=NULL;
	SOM_IgnoreWarning(somSelf);
	throw_marshal(ev);
	return __result;
}
/* introduced method ::CORBA::DataInputStream::read_any_array */
SOM_Scope void SOMLINK dinstr_read_any_array(
	CORBA_DataInputStream SOMSTAR somSelf,
	Environment *ev,
	/* inout */ CORBA_AnySeq *seq,
	/* in */ unsigned long offset,
	/* in */ unsigned long length)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(seq);
	SOM_IgnoreWarning(offset);
	SOM_IgnoreWarning(length);
	throw_marshal(ev);
}
/* introduced method ::CORBA::DataInputStream::read_boolean_array */
SOM_Scope void SOMLINK dinstr_read_boolean_array(
	CORBA_DataInputStream SOMSTAR somSelf,
	Environment *ev,
	/* inout */ CORBA_BooleanSeq *seq,
	/* in */ unsigned long offset,
	/* in */ unsigned long length)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(seq);
	SOM_IgnoreWarning(offset);
	SOM_IgnoreWarning(length);
	throw_marshal(ev);
}
/* introduced method ::CORBA::DataInputStream::read_char_array */
SOM_Scope void SOMLINK dinstr_read_char_array(
	CORBA_DataInputStream SOMSTAR somSelf,
	Environment *ev,
	/* inout */ CORBA_CharSeq *seq,
	/* in */ unsigned long offset,
	/* in */ unsigned long length)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(seq);
	SOM_IgnoreWarning(offset);
	SOM_IgnoreWarning(length);
	throw_marshal(ev);
}
/* introduced method ::CORBA::DataInputStream::read_octet_array */
SOM_Scope void SOMLINK dinstr_read_octet_array(
	CORBA_DataInputStream SOMSTAR somSelf,
	Environment *ev,
	/* inout */ CORBA_OctetSeq *seq,
	/* in */ unsigned long offset,
	/* in */ unsigned long length)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(seq);
	SOM_IgnoreWarning(offset);
	SOM_IgnoreWarning(length);
	throw_marshal(ev);
}
/* introduced method ::CORBA::DataInputStream::read_short_array */
SOM_Scope void SOMLINK dinstr_read_short_array(
	CORBA_DataInputStream SOMSTAR somSelf,
	Environment *ev,
	/* inout */ CORBA_ShortSeq *seq,
	/* in */ unsigned long offset,
	/* in */ unsigned long length)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(seq);
	SOM_IgnoreWarning(offset);
	SOM_IgnoreWarning(length);
	throw_marshal(ev);
}
/* introduced method ::CORBA::DataInputStream::read_ushort_array */
SOM_Scope void SOMLINK dinstr_read_ushort_array(
	CORBA_DataInputStream SOMSTAR somSelf,
	Environment *ev,
	/* inout */ CORBA_UShortSeq *seq,
	/* in */ unsigned long offset,
	/* in */ unsigned long length)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(seq);
	SOM_IgnoreWarning(offset);
	SOM_IgnoreWarning(length);
	throw_marshal(ev);
}
/* introduced method ::CORBA::DataInputStream::read_long_array */
SOM_Scope void SOMLINK dinstr_read_long_array(
	CORBA_DataInputStream SOMSTAR somSelf,
	Environment *ev,
	/* inout */ CORBA_LongSeq *seq,
	/* in */ unsigned long offset,
	/* in */ unsigned long length)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(seq);
	SOM_IgnoreWarning(offset);
	SOM_IgnoreWarning(length);
	throw_marshal(ev);
}
/* introduced method ::CORBA::DataInputStream::read_ulong_array */
SOM_Scope void SOMLINK dinstr_read_ulong_array(
	CORBA_DataInputStream SOMSTAR somSelf,
	Environment *ev,
	/* inout */ CORBA_ULongSeq *seq,
	/* in */ unsigned long offset,
	/* in */ unsigned long length)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(seq);
	SOM_IgnoreWarning(offset);
	SOM_IgnoreWarning(length);
	throw_marshal(ev);
}
/* introduced method ::CORBA::DataInputStream::read_float_array */
SOM_Scope void SOMLINK dinstr_read_float_array(
	CORBA_DataInputStream SOMSTAR somSelf,
	Environment *ev,
	/* inout */ CORBA_FloatSeq *seq,
	/* in */ unsigned long offset,
	/* in */ unsigned long length)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(seq);
	SOM_IgnoreWarning(offset);
	SOM_IgnoreWarning(length);

	throw_marshal(ev);
}
/* introduced method ::CORBA::DataInputStream::read_double_array */
SOM_Scope void SOMLINK dinstr_read_double_array(
	CORBA_DataInputStream SOMSTAR somSelf,
	Environment *ev,
	/* inout */ CORBA_DoubleSeq *seq,
	/* in */ unsigned long offset,
	/* in */ unsigned long length)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(seq);
	SOM_IgnoreWarning(offset);
	SOM_IgnoreWarning(length);
	throw_marshal(ev);
}

#define GENERIC_CONSTRUCTOR(fn,cls)  \
SOM_Scope void SOMLINK fn##somDestruct(cls SOMSTAR somSelf,boolean doFree,somDestructCtrl *ctrl) \
{ somDestructCtrl globalCtrl; somBooleanVector myMask;  \
cls##_BeginDestructor  cls##_EndDestructor } \
SOM_Scope void SOMLINK fn##somDefaultInit(cls SOMSTAR somSelf,somInitCtrl *ctrl) \
{ somInitCtrl globalCtrl; somBooleanVector myMask; \
cls##_BeginInitializer_somDefaultInit
     
GENERIC_CONSTRUCTOR(ir2_containr_,CORBA_Container)
	CORBA_Container_Init_CORBA_IRObject_somDefaultInit(somSelf,ctrl);
}


GENERIC_CONSTRUCTOR(ir2_structdf_,CORBA_StructDef)
	CORBA_StructDef_Init_CORBA_TypedefDef_somDefaultInit(somSelf,ctrl);
	CORBA_StructDef_Init_CORBA_Container_somDefaultInit(somSelf,ctrl);
}

GENERIC_CONSTRUCTOR(ir2_typedef_,CORBA_TypedefDef)
	CORBA_TypedefDef_Init_CORBA_Contained_somDefaultInit(somSelf,ctrl);
	CORBA_TypedefDef_Init_CORBA_IDLType_somDefaultInit(somSelf,ctrl);
}

GENERIC_CONSTRUCTOR(ir2_containd_,CORBA_Contained)
	CORBA_Contained_Init_CORBA_IRObject_somDefaultInit(somSelf,ctrl);
}

GENERIC_CONSTRUCTOR(ir2_idltype_,CORBA_IDLType)
	CORBA_IDLType_Init_CORBA_IRObject_somDefaultInit(somSelf,ctrl);
}

GENERIC_CONSTRUCTOR(ir2_absintdf_,CORBA_AbstractInterfaceDef)
	CORBA_AbstractInterfaceDef_Init_CORBA_InterfaceDef_somDefaultInit(somSelf,ctrl);
}

GENERIC_CONSTRUCTOR(ir2_seqdef_,CORBA_SequenceDef)
	CORBA_SequenceDef_Init_CORBA_IDLType_somDefaultInit(somSelf,ctrl);
}

GENERIC_CONSTRUCTOR(ir2_constdef_,CORBA_ConstantDef)
	CORBA_ConstantDef_Init_CORBA_Contained_somDefaultInit(somSelf,ctrl);
}

GENERIC_CONSTRUCTOR(ir2_arraydef_,CORBA_ArrayDef)
	CORBA_ArrayDef_Init_CORBA_IDLType_somDefaultInit(somSelf,ctrl);
}

GENERIC_CONSTRUCTOR(ir2_primitiv_,CORBA_PrimitiveDef)
	CORBA_PrimitiveDef_Init_CORBA_IDLType_somDefaultInit(somSelf,ctrl);
}

GENERIC_CONSTRUCTOR(ir2_valuedef_,CORBA_ValueDef)
	CORBA_ValueDef_Init_CORBA_Container_somDefaultInit(somSelf,ctrl);
	CORBA_ValueDef_Init_CORBA_Contained_somDefaultInit(somSelf,ctrl);
	CORBA_ValueDef_Init_CORBA_IDLType_somDefaultInit(somSelf,ctrl);
}

GENERIC_CONSTRUCTOR(ir2_valboxdf_,CORBA_ValueBoxDef)
	CORBA_ValueBoxDef_Init_CORBA_TypedefDef_somDefaultInit(somSelf,ctrl);
}

GENERIC_CONSTRUCTOR(ir2_uniondef_,CORBA_UnionDef)
	CORBA_UnionDef_Init_CORBA_TypedefDef_somDefaultInit(somSelf,ctrl);
	CORBA_UnionDef_Init_CORBA_Container_somDefaultInit(somSelf,ctrl);
}

GENERIC_CONSTRUCTOR(ir2_attribdf_,CORBA_AttributeDef)
	CORBA_AttributeDef_Init_CORBA_Contained_somDefaultInit(somSelf,ctrl);
}

GENERIC_CONSTRUCTOR(ir2_moduledf_,CORBA_ModuleDef)
	CORBA_ModuleDef_Init_CORBA_Container_somDefaultInit(somSelf,ctrl);
	CORBA_ModuleDef_Init_CORBA_Contained_somDefaultInit(somSelf,ctrl);
}

GENERIC_CONSTRUCTOR(ir2_wstringd_,CORBA_WstringDef)
	CORBA_WstringDef_Init_CORBA_IDLType_somDefaultInit(somSelf,ctrl);
}

GENERIC_CONSTRUCTOR(ir2_exceptdf_,CORBA_ExceptionDef)
	CORBA_ExceptionDef_Init_CORBA_Contained_somDefaultInit(somSelf,ctrl);
	CORBA_ExceptionDef_Init_CORBA_Container_somDefaultInit(somSelf,ctrl);

}

GENERIC_CONSTRUCTOR(ir2_stringdf_,CORBA_StringDef)
	CORBA_StringDef_Init_CORBA_IDLType_somDefaultInit(somSelf,ctrl);
}

GENERIC_CONSTRUCTOR(ir2_valmemdf_,CORBA_ValueMemberDef)
	CORBA_ValueMemberDef_Init_CORBA_Contained_somDefaultInit(somSelf,ctrl);
}

GENERIC_CONSTRUCTOR(ir2_fixeddef_,CORBA_FixedDef)
	CORBA_FixedDef_Init_CORBA_IDLType_somDefaultInit(somSelf,ctrl);
}

GENERIC_CONSTRUCTOR(ir2_operatdf_,CORBA_OperationDef)
	CORBA_OperationDef_Init_CORBA_Contained_somDefaultInit(somSelf,ctrl);
}

GENERIC_CONSTRUCTOR(ir2_repostry_,CORBA_Repository)
	CORBA_Repository_Init_CORBA_Container_somDefaultInit(somSelf,ctrl);
}

GENERIC_CONSTRUCTOR(ir2_aliasdef_,CORBA_AliasDef)
	CORBA_AliasDef_Init_CORBA_TypedefDef_somDefaultInit(somSelf,ctrl);
}

GENERIC_CONSTRUCTOR(ir2_irobject_,CORBA_IRObject)
	CORBA_IRObject_Init_SOMRefObject_somDefaultInit(somSelf,ctrl);
}

GENERIC_CONSTRUCTOR(ir2_intfacdf_,CORBA_InterfaceDef)
	CORBA_InterfaceDef_Init_CORBA_Container_somDefaultInit(somSelf,ctrl);
	CORBA_InterfaceDef_Init_CORBA_Contained_somDefaultInit(somSelf,ctrl);
	CORBA_InterfaceDef_Init_CORBA_IDLType_somDefaultInit(somSelf,ctrl);
}

GENERIC_CONSTRUCTOR(ir2_nativedf_,CORBA_NativeDef)
	CORBA_NativeDef_Init_CORBA_TypedefDef_somDefaultInit(somSelf,ctrl);
}

GENERIC_CONSTRUCTOR(ir2_enumdef_,CORBA_EnumDef)
	CORBA_EnumDef_Init_CORBA_TypedefDef_somDefaultInit(somSelf,ctrl);
}


GENERIC_CONSTRUCTOR(policy_,CORBA_Policy)
	CORBA_Policy_Init_SOMRefObject_somDefaultInit(somSelf,ctrl);
}


GENERIC_CONSTRUCTOR(dinstr_,CORBA_DataInputStream)
	CORBA_DataInputStream_Init_SOMRefObject_somDefaultInit(somSelf,ctrl);
}

GENERIC_CONSTRUCTOR(doutstr_,CORBA_DataOutputStream)
	CORBA_DataOutputStream_Init_SOMRefObject_somDefaultInit(somSelf,ctrl);
}

GENERIC_CONSTRUCTOR(object_,CORBA_Object)
	CORBA_Object_Init_SOMRefObject_somDefaultInit(somSelf,ctrl);
}

GENERIC_CONSTRUCTOR(currnt_,CORBA_Current)
	CORBA_Current_Init_SOMRefObject_somDefaultInit(somSelf,ctrl);
}

