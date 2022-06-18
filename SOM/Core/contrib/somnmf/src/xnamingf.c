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

#define FileXNaming_FileENC_Class_Source
#define FileXNaming_FileBindingIterator_Class_Source
#define FileXNaming_FPropertyBindingIterator_Class_Source
#define FileXNaming_FileIndexIterator_Class_Source
#define FileXNaming_FPropertyIterator_Class_Source

#include <rhbsomnm.h>
#include <xnamingf.ih>

SOM_Scope ExtendedNaming_TypeCodeSeq SOMLINK xnamingf_FENC__get_allowed_property_types(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev)
{
	ExtendedNaming_TypeCodeSeq seq={0,0,0};
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)

	return seq;
}

SOM_Scope void SOMLINK xnamingf_FENC_bind_with_properties(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n,
	/* in */ SOMObject SOMSTAR obj,
	/* in */ ExtendedNaming_PropertyList *prop)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)
	RHBOPT_unused(prop)
	RHBOPT_unused(obj)
}

SOM_Scope void SOMLINK xnamingf_FENC_list_properties(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n,
	/* in */ unsigned long howMany,
	/* out */ ExtendedNaming_PropertyBindingList *pbl,
	/* out */ ExtendedNaming_PropertyBindingIterator SOMSTAR *rest)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)
	RHBOPT_unused(pbl)
	RHBOPT_unused(howMany)
	RHBOPT_unused(rest)
}

SOM_Scope void SOMLINK xnamingf_FENC_bind_context_with_properties(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n,
	/* in */ ExtendedNaming_ExtendedNamingContext SOMSTAR obj,
	/* in */ ExtendedNaming_PropertyList *props)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)
	RHBOPT_unused(props)
	RHBOPT_unused(obj)
}


SOM_Scope void SOMLINK xnamingf_FENC_rebind_with_properties(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n,
	/* in */ SOMObject SOMSTAR obj,
	/* in */ ExtendedNaming_PropertyList *prop)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)
	RHBOPT_unused(obj)
	RHBOPT_unused(prop)
}

SOM_Scope void SOMLINK xnamingf_FENC_bind_context(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n,
	/* in */ CosNaming_NamingContext SOMSTAR nc)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)
	RHBOPT_unused(nc)
}


SOM_Scope void SOMLINK xnamingf_FENC_bind(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n,
	/* in */ SOMObject SOMSTAR obj)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)
	RHBOPT_unused(obj)
}

SOM_Scope void SOMLINK xnamingf_FENC_remove_property(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n,
	/* in */ CosNaming_Istring prop)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)
	RHBOPT_unused(prop)
}

SOM_Scope void SOMLINK xnamingf_FENC_somDumpSelfInt(
	FileXNaming_FileENC SOMSTAR somSelf,
	/* in */ long level)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(level)
}

SOM_Scope boolean SOMLINK xnamingf_FBI_next_n(
	FileXNaming_FileBindingIterator SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned long how_many,
	/* out */ CosNaming_BindingList *bl)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(how_many)
	RHBOPT_unused(bl)

	return 0;
}

SOM_Scope void SOMLINK xnamingf_FENC_reinit(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ somOS_ServiceBase_metastate_t *meta_data)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(meta_data)
}

SOM_Scope void SOMLINK xnamingf_FBI_destroy(
	FileXNaming_FileBindingIterator SOMSTAR somSelf,
	Environment *ev)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
}

SOM_Scope void SOMLINK xnamingf_FENC_unshare_property(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n,
	/* in */ CosNaming_Istring property_name)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)
	RHBOPT_unused(property_name)
}


SOM_Scope SOMObject SOMSTAR SOMLINK xnamingf_FENC_resolve_with_all_properties(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n,
	/* in */ unsigned long howMany,
	/* out */ ExtendedNaming_PropertyList *props,
	/* out */ ExtendedNaming_PropertyIterator SOMSTAR *rest)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)
	RHBOPT_unused(howMany)
	RHBOPT_unused(props)
	RHBOPT_unused(rest)

	return 0;
}

SOM_Scope void SOMLINK xnamingf_FENC_remove_index(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ ExtendedNaming_IndexDescriptor *i)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(i)
}

SOM_Scope boolean SOMLINK xnamingf_II_next_one(
	FileXNaming_FileIndexIterator SOMSTAR somSelf,
	Environment *ev,
	/* out */ ExtendedNaming_IndexDescriptor *p)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(p)

	return 0;
}

SOM_Scope CosNaming_NamingContext SOMSTAR SOMLINK xnamingf_FENC_new_context(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	return 0;
}

SOM_Scope SOMObject SOMSTAR SOMLINK xnamingf_FENC_resolve_with_properties(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n,
	/* in */ unsigned long howMany,
	/* in */ ExtendedNaming_IList *inames,
	/* out */ ExtendedNaming_PropertyList *props,
	/* out */ ExtendedNaming_PropertyIterator SOMSTAR *rest)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)
	RHBOPT_unused(howMany)
	RHBOPT_unused(inames)
	RHBOPT_unused(props)
	RHBOPT_unused(rest)

	return 0;
}


SOM_Scope boolean SOMLINK xnamingf_FPB_next_one(
	FileXNaming_FPropertyBindingIterator SOMSTAR somSelf,
	Environment *ev,
	/* out */ ExtendedNaming_PropertyBinding *pb)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(pb)

	return 0;
}

SOM_Scope void SOMLINK xnamingf_FENC_share_properties(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *target_n,
	/* in */ ExtendedNaming_ExtendedNamingContext SOMSTAR source_enc,
	/* in */ CosNaming_Name *source_n,
	/* in */ ExtendedNaming_IList *pnames)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(target_n)
	RHBOPT_unused(source_n)
	RHBOPT_unused(pnames)
	RHBOPT_unused(source_enc)
}

SOM_Scope void SOMLINK xnamingf_FENC_share_property(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *target_n,
	/* in */ ExtendedNaming_ExtendedNamingContext SOMSTAR source_enc,
	/* in */ CosNaming_Name *source_n,
	/* in */ CosNaming_Istring pname)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(target_n)
	RHBOPT_unused(source_n)
	RHBOPT_unused(pname)
	RHBOPT_unused(source_enc)
}

SOM_Scope CosNaming_NamingContext SOMSTAR SOMLINK xnamingf_FENC_bind_new_context(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)

	return 0;
}

SOM_Scope void SOMLINK xnamingf_FENC_destroy(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
}

SOM_Scope SOMObject SOMSTAR SOMLINK xnamingf_FENC_init_for_object_creation(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	return 0;
}

SOM_Scope unsigned short SOMLINK xnamingf_FENC_get_features_supported(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	return 0;
}

SOM_Scope void SOMLINK xnamingf_FENC_get_property(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n,
	/* in */ CosNaming_Istring pn,
	/* out */ ExtendedNaming_Property *prop)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)
	RHBOPT_unused(pn)
	RHBOPT_unused(prop)
}

SOM_Scope boolean SOMLINK xnamingf_II_next_n(
	FileXNaming_FileIndexIterator SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned long howMany,
	/* out */ ExtendedNaming_IndexDescriptorList *il)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(howMany)
	RHBOPT_unused(il)

	return 0;
}

SOM_Scope void SOMLINK xnamingf_FENC_unshare_properties(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n,
	/* in */ ExtendedNaming_IList *pnames)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)
	RHBOPT_unused(pnames)
}

SOM_Scope boolean SOMLINK xnamingf_FBI_next_one(
	FileXNaming_FileBindingIterator SOMSTAR somSelf,
	Environment *ev,
	/* out */ CosNaming_Binding *b)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(b)

	return 0;
}


SOM_Scope void SOMLINK xnamingf_FENC_find_any_name_binding(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ ExtendedNaming_ExtendedNamingContext_Constraint c,
	/* in */ unsigned long distance,
	/* out */ CosNaming_Binding *bi)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(bi)
	RHBOPT_unused(c)
	RHBOPT_unused(distance)
}

SOM_Scope void SOMLINK xnamingf_FENC_capture(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev,
	/* inout */ somOS_ServiceBase_metastate_t *meta_data)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(meta_data)
}


SOM_Scope void SOMLINK xnamingf_FENC_uninit_for_object_destruction(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
}

SOM_Scope void SOMLINK xnamingf_FENC_get_all_properties(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n,
	/* in */ unsigned long howMany,
	/* out */ ExtendedNaming_PropertyList *props,
	/* out */ ExtendedNaming_PropertyIterator SOMSTAR *rest)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)
	RHBOPT_unused(howMany)
	RHBOPT_unused(props)
	RHBOPT_unused(rest)
}

SOM_Scope void SOMLINK xnamingf_FENC_rebind_context(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n,
	/* in */ CosNaming_NamingContext SOMSTAR nc)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)
	RHBOPT_unused(nc)
}

SOM_Scope SOMObject SOMSTAR SOMLINK xnamingf_FENC_resolve(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)

	return 0;
}

SOM_Scope void SOMLINK xnamingf_FENC_rebind_context_with_properties(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n,
	/* in */ ExtendedNaming_ExtendedNamingContext SOMSTAR obj,
	/* in */ ExtendedNaming_PropertyList *props)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)
	RHBOPT_unused(obj)
	RHBOPT_unused(props)
}

SOM_Scope void SOMLINK xnamingf_FENC_add_index(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ ExtendedNaming_IndexDescriptor *i)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(i)
}


SOM_Scope void SOMLINK xnamingf_FENC_rebind(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n,
	/* in */ SOMObject SOMSTAR obj)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(obj)
	RHBOPT_unused(n)
}

SOM_Scope void SOMLINK xnamingf_FBI_FileBindingIterator_Init1(
	FileXNaming_FileBindingIterator SOMSTAR somSelf,
	Environment *ev,
	/* inout */ somToken *ctrl,
	/* in */ FileXNaming_FileBindingIterator_stringSeq *key,
	/* in */ corbastring databaseFile)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(ctrl)
	RHBOPT_unused(key)
	RHBOPT_unused(databaseFile)
}


SOM_Scope void SOMLINK xnamingf_FBI_FileBindingIterator_Init2(
	FileXNaming_FileBindingIterator SOMSTAR somSelf,
	Environment *ev,
	/* inout */ somToken *ctrl,
	/* in */ CosNaming_BindingList *bl)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(ctrl)
	RHBOPT_unused(bl)
}

SOM_Scope SOMObject SOMSTAR SOMLINK xnamingf_FENC_resolve_with_property(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n,
	/* in */ CosNaming_Istring prop,
	/* out */ any *v)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)
	RHBOPT_unused(prop)
	RHBOPT_unused(v)

	return 0;
}

SOM_Scope void SOMLINK xnamingf_FENC_remove_properties(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n,
	/* in */ ExtendedNaming_IList *plist)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)
	RHBOPT_unused(plist)
}

SOM_Scope ExtendedNaming_ExtendedNamingContext_CosNaming_IstringSeq SOMLINK xnamingf_FENC__get_shared_property_names(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev)
{
	ExtendedNaming_ExtendedNamingContext_CosNaming_IstringSeq seq={0,0,0};
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)

	return seq;
}

SOM_Scope void SOMLINK xnamingf_FENC_add_properties(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n,
	/* in */ ExtendedNaming_PropertyList *props)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)
	RHBOPT_unused(props)
}


SOM_Scope void SOMLINK xnamingf_FENC_list_indexes(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned long howMany,
	/* out */ ExtendedNaming_IndexDescriptorList *il,
	/* out */ ExtendedNaming_IndexIterator SOMSTAR *rest)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(il)
	RHBOPT_unused(howMany)
	RHBOPT_unused(rest)
}

SOM_Scope void SOMLINK xnamingf_FENC_find_all(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ ExtendedNaming_ExtendedNamingContext_Constraint c,
	/* in */ unsigned long distance,
	/* in */ unsigned long howMany,
	/* out */ CosNaming_BindingList *bl,
	/* out */ CosNaming_BindingIterator SOMSTAR *bi)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(c)
	RHBOPT_unused(distance)
	RHBOPT_unused(bl)
	RHBOPT_unused(bi)
	RHBOPT_unused(howMany)
}

SOM_Scope void SOMLINK xnamingf_FENC_get_properties(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n,
	/* in */ unsigned long howMany,
	/* in */ ExtendedNaming_IList *inames,
	/* out */ ExtendedNaming_PropertyList *props,
	/* out */ ExtendedNaming_PropertyIterator SOMSTAR *rest)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)
	RHBOPT_unused(inames)
	RHBOPT_unused(howMany)
	RHBOPT_unused(props)
	RHBOPT_unused(rest)
}


SOM_Scope SOMObject SOMSTAR SOMLINK xnamingf_FENC_find_any(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ ExtendedNaming_ExtendedNamingContext_Constraint c,
	/* in */ unsigned long distance)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(c)
	RHBOPT_unused(distance)

	return 0;
}


SOM_Scope void SOMLINK xnamingf_FENC_list(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned long how_many,
	/* out */ CosNaming_BindingList *bl,
	/* out */ CosNaming_BindingIterator SOMSTAR *bi)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(bl)
	RHBOPT_unused(bi)
	RHBOPT_unused(how_many)
}

SOM_Scope ExtendedNaming_TypeCodeSeq SOMLINK xnamingf_FENC__get_allowed_object_types(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev)
{
	ExtendedNaming_TypeCodeSeq seq={0,0,0};
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)

	return seq;
}

SOM_Scope ExtendedNaming_TypeCodeSeq SOMLINK xnamingf_FENC__get_shared_property_types(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev)
{
	ExtendedNaming_TypeCodeSeq seq={0,0,0};
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)

	return seq;
}

SOM_Scope void SOMLINK xnamingf_FENC_remove_all_properties(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)
}

SOM_Scope boolean SOMLINK xnamingf_FPB_next_n(
	FileXNaming_FPropertyBindingIterator SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned long howMany,
	/* out */ ExtendedNaming_PropertyBindingList *il)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(howMany)
	RHBOPT_unused(il)
	return 0;
}

SOM_Scope void SOMLINK xnamingf_II_FileIndexIterator_Init(
	FileXNaming_FileIndexIterator SOMSTAR somSelf,
	Environment *ev,
	/* inout */ somToken *ctrl,
	/* in */ _IDL_SEQUENCE_octet *key,
	/* in */ corbastring databaseFile)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(ctrl)
	RHBOPT_unused(key)
	RHBOPT_unused(databaseFile)
}


SOM_Scope ExtendedNaming_ExtendedNamingContext_CosNaming_IstringSeq SOMLINK xnamingf_FENC__get_allowed_property_names(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev)
{
	ExtendedNaming_ExtendedNamingContext_CosNaming_IstringSeq seq={0,0,0};
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)

	return seq;
}

SOM_Scope void SOMLINK xnamingf_FPB_destroy(
	FileXNaming_FPropertyBindingIterator SOMSTAR somSelf,
	Environment *ev)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
}

SOM_Scope void SOMLINK xnamingf_II_destroy(
	FileXNaming_FileIndexIterator SOMSTAR somSelf,
	Environment *ev)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
}

SOM_Scope void SOMLINK xnamingf_FENC_add_property(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n,
	/* in */ ExtendedNaming_Property *prop)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)
	RHBOPT_unused(prop)
}

SOM_Scope void SOMLINK xnamingf_FENC_unbind(
	FileXNaming_FileENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)
}

SOM_Scope void SOMLINK xnamingf_FP_destroy(
	FileXNaming_FPropertyIterator SOMSTAR somSelf,
	Environment *ev)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
}

SOM_Scope boolean SOMLINK xnamingf_FP_next_one(
	FileXNaming_FPropertyIterator SOMSTAR somSelf,
	Environment *ev,
	/* out */ ExtendedNaming_Property *p)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(p)
	return 0;
}

SOM_Scope boolean SOMLINK xnamingf_FP_next_n(
	FileXNaming_FPropertyIterator SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned long howMany,
	/* out */ ExtendedNaming_PropertyList *pl)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(pl)
	RHBOPT_unused(howMany)

	return 0;
}






