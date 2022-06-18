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

#define somAttributePersistence_Persist_Class_Source

#include <rhbsomos.h>
#include <somap.ih>

SOM_Scope SOMObject SOMSTAR SOMLINK somap_init_for_object_creation(
	somAttributePersistence_Persist SOMSTAR somSelf,
	Environment *ev)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	return 0;
}

SOM_Scope void SOMLINK somap_reinit(
	somAttributePersistence_Persist SOMSTAR somSelf,
	Environment *ev,
	/* in */ somOS_ServiceBase_metastate_t *meta_data)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(meta_data)
}

SOM_Scope SOMObject SOMSTAR SOMLINK somap_init_for_object_copy(
	somAttributePersistence_Persist SOMSTAR somSelf,
	Environment *ev)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	return 0;
}

SOM_Scope SOMObject SOMSTAR SOMLINK somap_init_for_object_reactivation(
	somAttributePersistence_Persist SOMSTAR somSelf,
	Environment *ev)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	return 0;
}

SOM_Scope void SOMLINK somap_capture(
	somAttributePersistence_Persist SOMSTAR somSelf,
	Environment *ev,
	/* inout */ somOS_ServiceBase_metastate_t *meta_data)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(meta_data)
}

SOM_Scope void SOMLINK somap_load_attribute(
	somAttributePersistence_Persist SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring attribute_name,
	/* out */ void **buffer,
	/* in */ TypeCode attribute_tc)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(buffer)
	RHBOPT_unused(attribute_name)
	RHBOPT_unused(attribute_tc)
}

SOM_Scope void SOMLINK somap_store_attribute(
	somAttributePersistence_Persist SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring attribute_name,
	/* in */ void *buffer,
	/* in */ TypeCode attribute_tc)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(buffer)
	RHBOPT_unused(attribute_name)
	RHBOPT_unused(attribute_tc)
}

SOM_Scope void SOMLINK somap_uninit_for_object_move(
	somAttributePersistence_Persist SOMSTAR somSelf,
	Environment *ev)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
}

SOM_Scope void SOMLINK somap_uninit_for_object_passivation(
	somAttributePersistence_Persist SOMSTAR somSelf,
	Environment *ev)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
}

SOM_Scope void SOMLINK somap_uninit_for_object_destruction(
	somAttributePersistence_Persist SOMSTAR somSelf,
	Environment *ev)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
}










