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

#ifndef SOM_Module_somdfenc_Source
	#define SOM_Module_somdfenc_Source
#endif

#define FENC_FactoryENC_Class_Source

#include "somdfenc.ih"

SOM_Scope SOMObject SOMSTAR SOMLINK somdfenc_find_any_from_proxy(
		FENC_FactoryENC SOMSTAR somSelf,
		Environment *ev,
		SOMObject SOMSTAR proxy)
{
	return NULL;
}

SOM_Scope SOMObject SOMSTAR SOMLINK somdfenc_resolve_from_proxy(
	FENC_FactoryENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMObject SOMSTAR proxy)
{
	return NULL;
}

SOM_Scope CosNaming_NamingContext SOMSTAR SOMLINK somdfenc_bind_new_context(
	FENC_FactoryENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n)
{
	return NULL;
}

SOM_Scope void SOMLINK somdfenc_destroy(
	FENC_FactoryENC SOMSTAR somSelf,
	Environment *ev)
{
}

SOM_Scope SOMObject SOMSTAR SOMLINK somdfenc_resolve(
	FENC_FactoryENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n)
{
	return NULL;
}

SOM_Scope void SOMLINK somdfenc_list(
	FENC_FactoryENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned long how_many,
	/* out */ CosNaming_BindingList *bl,
	/* out */ CosNaming_BindingIterator SOMSTAR *bi)
{
}

SOM_Scope void SOMLINK somdfenc_rebind(
	FENC_FactoryENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n,
	/* in */ SOMObject SOMSTAR obj)
{
}

SOM_Scope CosNaming_NamingContext SOMSTAR SOMLINK somdfenc_new_context(
	FENC_FactoryENC SOMSTAR somSelf,
	Environment *ev)
{
	return NULL;
}

SOM_Scope void SOMLINK somdfenc_bind_context(
	FENC_FactoryENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n,
	/* in */ CosNaming_NamingContext SOMSTAR nc)
{
}

SOM_Scope void SOMLINK somdfenc_bind(
	FENC_FactoryENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n,
	/* in */ SOMObject SOMSTAR obj)
{
}

SOM_Scope void SOMLINK somdfenc_unbind(
	FENC_FactoryENC SOMSTAR somSelf,
	Environment *ev,
	/* in */ CosNaming_Name *n)
{
}



