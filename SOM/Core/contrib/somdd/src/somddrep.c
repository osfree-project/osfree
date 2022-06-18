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

#ifdef _WIN32
	#define SOMDLLEXPORT
#endif

#define SOMDDREP_DynamicRepository_Class_Source

#include <rhbsomdd.h>
#include <somddrep.ih>

SOM_Scope void SOMLINK somddrep_dynrep_somUninit(
		SOMDDREP_DynamicRepository SOMSTAR somSelf)
{
	SOMDDREP_DynamicRepository_parent_PortableServer_DynamicImpl_somUninit(somSelf);
}

SOM_Scope void SOMLINK somddrep_dynrep_somInit(
		SOMDDREP_DynamicRepository SOMSTAR somSelf)
{
	SOMDDREP_DynamicRepository_parent_PortableServer_DynamicImpl_somInit(somSelf);
}

SOM_Scope char *SOMLINK somddrep_dynrep_primary_interface(
		SOMDDREP_DynamicRepository SOMSTAR somSelf,
		Environment *ev,
		_IDL_SEQUENCE_octet *key,
		SOMObject SOMSTAR poa)
{
	return NULL;
}

SOM_Scope void SOMLINK somddrep_dynrep_invoke(
		SOMDDREP_DynamicRepository SOMSTAR somSelf,
		Environment *ev,
		ServerRequest SOMSTAR req)
{
}

SOM_Scope SOMObject SOMSTAR SOMLINK somddrep_dynrep_lookup_id(
		SOMDDREP_DynamicRepository SOMSTAR somSelf,
		Environment *ev,char *id)
{
	return NULL;
}
