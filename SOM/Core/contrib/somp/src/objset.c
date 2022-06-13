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
/* code generater for objset.idl */
#define SOM_Module_objset_Source
#define SOMPObjectSet_Class_Source
#include <rhbsomp.h>
#include <objset.ih>
/* overridden methods for ::SOMPObjectSet */
/* overridden method ::SOMObject::somInit */
SOM_Scope void SOMLINK objset_somInit(
	SOMPObjectSet SOMSTAR somSelf)
{
	RHBOPT_unused(somSelf)
	SOMPObjectSet_parent_SOMObject_somInit(somSelf);
}
/* overridden method ::SOMObject::somUninit */
SOM_Scope void SOMLINK objset_somUninit(
	SOMPObjectSet SOMSTAR somSelf)
{
	RHBOPT_unused(somSelf)
	SOMPObjectSet_parent_SOMObject_somUninit(somSelf);
}
/* overridden method ::SOMObject::somDumpSelfInt */
SOM_Scope void SOMLINK objset_somDumpSelfInt(
	SOMPObjectSet SOMSTAR somSelf,
	/* in */ long level)
{
	SOMPObjectSet_parent_SOMObject_somDumpSelfInt(somSelf,level);
}
/* introduced methods for ::SOMPObjectSet */
/* introduced method ::SOMPObjectSet::sompSetGetNth */
SOM_Scope SOMObject SOMSTAR SOMLINK objset_sompSetGetNth(
	SOMPObjectSet SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned long n)
{
	SOMObject SOMSTAR __result=NULL;
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)
	return __result;
}
/* introduced method ::SOMPObjectSet::sompFindByID */
SOM_Scope SOMObject SOMSTAR SOMLINK objset_sompFindByID(
	SOMPObjectSet SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMPPersistentId SOMSTAR lookfor)
{
	SOMObject SOMSTAR __result=NULL;
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(lookfor)
	return __result;
}
/* introduced method ::SOMPObjectSet::sompFindByAddress */
SOM_Scope long SOMLINK objset_sompFindByAddress(
	SOMPObjectSet SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMObject SOMSTAR lookfor)
{
	long __result=0;
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(lookfor)
	return __result;
}
/* introduced method ::SOMPObjectSet::sompAddToSet */
SOM_Scope void SOMLINK objset_sompAddToSet(
	SOMPObjectSet SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMObject SOMSTAR thisObject)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(thisObject)
}
/* introduced method ::SOMPObjectSet::sompSetDeleteNth */
SOM_Scope void SOMLINK objset_sompSetDeleteNth(
	SOMPObjectSet SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned long n)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(n)
}
/* introduced method ::SOMPObjectSet::sompGetSetSize */
SOM_Scope long SOMLINK objset_sompGetSetSize(
	SOMPObjectSet SOMSTAR somSelf,
	Environment *ev)
{
	long __result=0;
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	return __result;
}
/* introduced method ::SOMPObjectSet::sompObjEqualsID */
SOM_Scope boolean SOMLINK objset_sompObjEqualsID(
	SOMPObjectSet SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMObject SOMSTAR cmpObj,
	/* in */ SOMPPersistentId SOMSTAR lookfor)
{
	boolean __result=0;
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(cmpObj)
	RHBOPT_unused(lookfor)
	return __result;
}
