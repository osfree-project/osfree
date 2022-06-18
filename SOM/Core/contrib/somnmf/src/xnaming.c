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

#define ExtendedNaming_ExtendedNamingContext_Class_Source
#define ExtendedNaming_PropertyBindingIterator_Class_Source
#define ExtendedNaming_IndexIterator_Class_Source
#define ExtendedNaming_PropertyIterator_Class_Source

#include <rhbsomnm.h>
#include <xnaming.ih>

SOM_Scope boolean SOMLINK xnaming_PB_next_n(
	ExtendedNaming_PropertyBindingIterator SOMSTAR somSelf,
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

SOM_Scope boolean SOMLINK xnaming_PB_next_one(
	ExtendedNaming_PropertyBindingIterator SOMSTAR somSelf,
	Environment *ev,
	/* out */ ExtendedNaming_PropertyBinding *pb)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(pb)
	RHBOPT_unused(pb)

	return 0;
}

SOM_Scope void SOMLINK xnaming_PB_destroy(
	ExtendedNaming_PropertyBindingIterator SOMSTAR somSelf,
	Environment *ev)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)

}

SOM_Scope void SOMLINK xnaming_In_destroy(
	ExtendedNaming_IndexIterator SOMSTAR somSelf,
	Environment *ev)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)

}

SOM_Scope SOMObject SOMSTAR SOMLINK xnaming_ENC_resolve_with_property(
	ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n,
	/* in */ CosNaming_Istring prop,
	/* out */ any *v)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)
	RHBOPT_unused(v)
	RHBOPT_unused(prop)

	return 0;
}

SOM_Scope boolean SOMLINK xnaming_In_next_one(
	ExtendedNaming_IndexIterator SOMSTAR somSelf,
	Environment *ev,
	/* out */ ExtendedNaming_IndexDescriptor *p)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(p)

	return 0;
}

SOM_Scope boolean SOMLINK xnaming_In_next_n(
	ExtendedNaming_IndexIterator SOMSTAR somSelf,
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


SOM_Scope ExtendedNaming_TypeCodeSeq SOMLINK xnaming_ENC__get_allowed_object_types(
	ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
	Environment *ev)
{
	ExtendedNaming_TypeCodeSeq seq={0,0,0};
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)

	return seq;
}


SOM_Scope ExtendedNaming_TypeCodeSeq SOMLINK xnaming_ENC__get_allowed_property_types(
	ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
	Environment *ev)
{
	ExtendedNaming_TypeCodeSeq seq={0,0,0};
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)

	return seq;
}

SOM_Scope ExtendedNaming_ExtendedNamingContext_CosNaming_IstringSeq SOMLINK xnaming_ENC__get_allowed_property_names(
	ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
	Environment *ev)
{
	ExtendedNaming_ExtendedNamingContext_CosNaming_IstringSeq seq={0,0,0};
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)


	return seq;
}

SOM_Scope ExtendedNaming_TypeCodeSeq SOMLINK xnaming_ENC__get_shared_property_types(
	ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
	Environment *ev)
{
	ExtendedNaming_TypeCodeSeq seq={0,0,0};
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)


	return seq;
}

SOM_Scope ExtendedNaming_ExtendedNamingContext_CosNaming_IstringSeq SOMLINK xnaming_ENC__get_shared_property_names(
	ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
	Environment *ev)
{
	ExtendedNaming_ExtendedNamingContext_CosNaming_IstringSeq seq={0,0,0};
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)


	return seq;
}

SOM_Scope unsigned short SOMLINK xnaming_ENC_get_features_supported(
	ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
	Environment *ev)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)

	return 0;
}

SOM_Scope void SOMLINK xnaming_ENC_add_property(
	ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n,
	/* in */ ExtendedNaming_Property *prop)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)
	RHBOPT_unused(prop)
}

SOM_Scope void SOMLINK xnaming_ENC_add_properties(
	ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n,
	/* in */ ExtendedNaming_PropertyList *props)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)
	RHBOPT_unused(props)
}

SOM_Scope void SOMLINK xnaming_ENC_bind_with_properties(
	ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
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

SOM_Scope void SOMLINK xnaming_ENC_bind_context_with_properties(
	ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
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

SOM_Scope void SOMLINK xnaming_ENC_rebind_with_properties(
	ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
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

SOM_Scope void SOMLINK xnaming_ENC_rebind_context_with_properties(
	ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
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

SOM_Scope void SOMLINK xnaming_ENC_share_property(
	ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *target_n,
	/* in */ ExtendedNaming_ExtendedNamingContext SOMSTAR source_enc,
	/* in */ CosNaming_Name *source_n,
	/* in */ CosNaming_Istring pname)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(target_n)
	RHBOPT_unused(source_enc)
	RHBOPT_unused(source_n)
	RHBOPT_unused(pname)
}

SOM_Scope void SOMLINK xnaming_ENC_share_properties(
	ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
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

SOM_Scope void SOMLINK xnaming_ENC_find_any_name_binding(
	ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
	Environment *ev,
	/* in */ ExtendedNaming_ExtendedNamingContext_Constraint c,
	/* in */ unsigned long distance,
	/* out */ CosNaming_Binding *bi)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(c)
	RHBOPT_unused(distance)
	RHBOPT_unused(bi)
}

SOM_Scope void SOMLINK xnaming_ENC_list_properties(
	ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n,
	/* in */ unsigned long howMany,
	/* out */ ExtendedNaming_PropertyBindingList *pbl,
	/* out */ ExtendedNaming_PropertyBindingIterator SOMSTAR *rest)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)
	RHBOPT_unused(howMany)
	RHBOPT_unused(pbl)
	RHBOPT_unused(rest)
}

SOM_Scope SOMObject SOMSTAR SOMLINK xnaming_ENC_resolve_with_properties(
	ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
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


SOM_Scope void SOMLINK xnaming_ENC_list_indexes(
	ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned long howMany,
	/* out */ ExtendedNaming_IndexDescriptorList *il,
	/* out */ ExtendedNaming_IndexIterator SOMSTAR *rest)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(rest)
	RHBOPT_unused(il)
	RHBOPT_unused(howMany)
}

SOM_Scope void SOMLINK xnaming_ENC_add_index(
	ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
	Environment *ev,
	/* in */ ExtendedNaming_IndexDescriptor *i)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(i)
}

SOM_Scope void SOMLINK xnaming_ENC_remove_index(
	ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
	Environment *ev,
	/* in */ ExtendedNaming_IndexDescriptor *i)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(i)
}

SOM_Scope void SOMLINK xnaming_ENC_remove_all_properties(
	ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)
}

SOM_Scope void SOMLINK xnaming_ENC_remove_properties(
	ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n,
	/* in */ ExtendedNaming_IList *plist)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)
	RHBOPT_unused(plist)
}

SOM_Scope void SOMLINK xnaming_ENC_unshare_properties(
	ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n,
	/* in */ ExtendedNaming_IList *pnames)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)
	RHBOPT_unused(pnames)
}

SOM_Scope void SOMLINK xnaming_ENC_get_all_properties(
	ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
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

SOM_Scope void SOMLINK xnaming_ENC_remove_property(
	ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n,
	/* in */ CosNaming_Istring prop)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)
	RHBOPT_unused(prop)
}

SOM_Scope SOMObject SOMSTAR SOMLINK xnaming_ENC_resolve_with_all_properties(
	ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
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

SOM_Scope void SOMLINK xnaming_ENC_find_all(
	ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
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
	RHBOPT_unused(bl)
	RHBOPT_unused(bi)
	RHBOPT_unused(howMany)
	RHBOPT_unused(distance)
}

SOM_Scope SOMObject SOMSTAR SOMLINK xnaming_ENC_find_any(
	ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
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

SOM_Scope void SOMLINK xnaming_ENC_unshare_property(
	ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n,
	/* in */ CosNaming_Istring property_name)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)
	RHBOPT_unused(property_name)
}

SOM_Scope void SOMLINK xnaming_ENC_get_property(
	ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
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

SOM_Scope void SOMLINK xnaming_ENC_get_properties(
	ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
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
}

SOM_Scope void SOMLINK xnaming_P_somInit(
	ExtendedNaming_PropertyIterator SOMSTAR somSelf)
{
	RHBOPT_unused(somSelf)

#ifdef ExtendedNaming_PropertyIterator_parent_SOMRefObject_somInit
	ExtendedNaming_PropertyIterator_parent_SOMRefObject_somInit(somSelf);
#else
	ExtendedNaming_PropertyIterator_parent_SOMObject_somInit(somSelf);
#endif
}

SOM_Scope boolean SOMLINK xnaming_P_next_one(
	ExtendedNaming_PropertyIterator SOMSTAR somSelf,
	Environment *ev,
	/* out */ ExtendedNaming_Property *p)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(p)

	return 0;
}

SOM_Scope boolean SOMLINK xnaming_P_next_n(
	ExtendedNaming_PropertyIterator SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned long howMany,
	/* out */ ExtendedNaming_PropertyList *pl)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(howMany)
	RHBOPT_unused(pl)

	return 0;
}

SOM_Scope void SOMLINK xnaming_P_destroy(
	ExtendedNaming_PropertyIterator SOMSTAR somSelf,
	Environment *ev)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
}

