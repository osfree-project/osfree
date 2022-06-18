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

#ifndef SOM_Module_om_Source
#define SOM_Module_om_Source
#endif
#define ObjectMgr_Class_Source

#include "om.ih"

SOM_Scope SOMObject SOMSTAR SOMLINK om_somdNewObject(
		ObjectMgr SOMSTAR somSelf, 
		Environment *ev, 
		Identifier objclass, 
		corbastring hints)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(objclass)
	RHBOPT_unused(hints)

    return 0;
}

SOM_Scope corbastring  SOMLINK om_somdGetIdFromObject(
		ObjectMgr SOMSTAR somSelf, 
		Environment *ev, 
		SOMObject SOMSTAR obj)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(obj)

    return 0;
}

SOM_Scope SOMObject SOMSTAR  SOMLINK om_somdGetObjectFromId(
		ObjectMgr SOMSTAR somSelf, 
		Environment *ev, 
		corbastring id)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(id)

    return 0;
}

SOM_Scope void  SOMLINK om_somdReleaseObject(
		ObjectMgr SOMSTAR somSelf, 
		Environment *ev, 
		SOMObject SOMSTAR obj)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(obj)
}

SOM_Scope void  SOMLINK om_somdDestroyObject(
		ObjectMgr SOMSTAR somSelf, 
		Environment *ev, 
		SOMObject SOMSTAR obj)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(obj)
}

