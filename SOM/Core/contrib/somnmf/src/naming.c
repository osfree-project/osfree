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

#define CosNaming_BindingIterator_Class_Source
#define CosNaming_NamingContext_Class_Source
#define CosNaming_NamingContextExt_Class_Source

#include <rhbsomnm.h>

#include <naming.ih>

SOM_Scope void SOMLINK naming_NamingContext_bind(
	CosNaming_NamingContext SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n,
	/* in */ SOMObject SOMSTAR obj)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)
	RHBOPT_unused(obj)
}

SOM_Scope void SOMLINK naming_NamingContext_destroy(
	CosNaming_NamingContext SOMSTAR somSelf,
	Environment *ev)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
}

SOM_Scope CosNaming_NamingContext SOMSTAR SOMLINK naming_NamingContext_bind_new_context(
	CosNaming_NamingContext SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)

	return 0;
}

SOM_Scope CosNaming_NamingContext SOMSTAR SOMLINK naming_NamingContext_new_context(
	CosNaming_NamingContext SOMSTAR somSelf,
	Environment *ev)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)

	return 0;
}

SOM_Scope void SOMLINK naming_NamingContext_unbind(
	CosNaming_NamingContext SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)
}

SOM_Scope void SOMLINK naming_NamingContext_bind_context(
	CosNaming_NamingContext SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n,
	/* in */ CosNaming_NamingContext SOMSTAR nc)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)
	RHBOPT_unused(nc)
}

SOM_Scope void SOMLINK naming_NamingContext_rebind_context(
	CosNaming_NamingContext SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n,
	/* in */ CosNaming_NamingContext SOMSTAR nc)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(nc)
	RHBOPT_unused(n)
}

SOM_Scope SOMObject SOMSTAR SOMLINK naming_NamingContext_resolve(
	CosNaming_NamingContext SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)

	return 0;
}

SOM_Scope void SOMLINK naming_NamingContext_rebind(
	CosNaming_NamingContext SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n,
	/* in */ SOMObject SOMSTAR obj)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)
	RHBOPT_unused(obj)
}


SOM_Scope void SOMLINK naming_NamingContext_list(
	CosNaming_NamingContext SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned long how_many,
	/* out */ CosNaming_BindingList *bl,
	/* out */ CosNaming_BindingIterator SOMSTAR *bi)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(how_many)
	RHBOPT_unused(bl)
	RHBOPT_unused(bi)
}


SOM_Scope boolean SOMLINK naming_BindingIterator_next_n(
	CosNaming_BindingIterator SOMSTAR somSelf,
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

SOM_Scope boolean SOMLINK naming_BindingIterator_next_one(
	CosNaming_BindingIterator SOMSTAR somSelf,
	Environment *ev,
	/* out */ CosNaming_Binding *b)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(b)

	return 0;
}


SOM_Scope void SOMLINK naming_BindingIterator_destroy(
	CosNaming_BindingIterator SOMSTAR somSelf,
	Environment *ev)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
}

SOM_Scope CosNaming_Name SOMLINK naming_NamingContextExt_to_name(
	CosNaming_NamingContextExt SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_NamingContextExt_StringName sn)
{
	CosNaming_Name name={0,0,0};

	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(sn);

	return name;
}

SOM_Scope SOMObject SOMSTAR SOMLINK naming_NamingContextExt_resolve_str(
	CosNaming_NamingContextExt SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_NamingContextExt_StringName n)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(n);
	return 0;
}

SOM_Scope CosNaming_NamingContextExt_URLString SOMLINK naming_NamingContextExt_to_url(
	CosNaming_NamingContextExt SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_NamingContextExt_Address addr,
	/* in */ CosNaming_NamingContextExt_StringName sn)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(sn);
	SOM_IgnoreWarning(addr);
	return 0;
}

SOM_Scope CosNaming_NamingContextExt_StringName SOMLINK naming_NamingContextExt_to_string(
	CosNaming_NamingContextExt SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(n);
	return 0;
}

