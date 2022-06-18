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
/* code generater for pid.idl */
#define SOM_Module_pid_Source
#define SOMPPersistentId_Class_Source

#include <rhbsomp.h>

#include <somsid.h>
#include <somida.h>
#include <pid.ih>
/* overridden methods for ::SOMPPersistentId */
/* overridden method ::SOMUTStringId::somutSetIdString */
SOM_Scope long SOMLINK pid_somutSetIdString(
	SOMPPersistentId SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring IdString)
{
	long __result;
	__result=SOMPPersistentId_parent_SOMUTStringId_somutSetIdString(somSelf,ev,IdString);
	return __result;}
/* overridden method ::SOMUTStringId::somutGetIdString */
SOM_Scope corbastring SOMLINK pid_somutGetIdString(
	SOMPPersistentId SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring toBuffer)
{
	corbastring __result;
	__result=SOMPPersistentId_parent_SOMUTStringId_somutGetIdString(somSelf,ev,toBuffer);
	return __result;}
/* overridden method ::SOMUTStringId::somutGetIdStringLen */
SOM_Scope long SOMLINK pid_somutGetIdStringLen(
	SOMPPersistentId SOMSTAR somSelf,
	Environment *ev)
{
	long __result;
	__result=SOMPPersistentId_parent_SOMUTStringId_somutGetIdStringLen(somSelf,ev);
	return __result;}
/* overridden method ::SOMUTStringId::somutEqualsString */
SOM_Scope boolean SOMLINK pid_somutEqualsString(
	SOMPPersistentId SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring IdString)
{
	boolean __result;
	__result=SOMPPersistentId_parent_SOMUTStringId_somutEqualsString(somSelf,ev,IdString);
	return __result;}
/* overridden method ::SOMUTStringId::somutCompareString */
SOM_Scope short SOMLINK pid_somutCompareString(
	SOMPPersistentId SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring IdString)
{
	short __result;
	__result=SOMPPersistentId_parent_SOMUTStringId_somutCompareString(somSelf,ev,IdString);
	return __result;}
/* overridden method ::SOMUTId::somutSetIdId */
SOM_Scope void SOMLINK pid_somutSetIdId(
	SOMPPersistentId SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMUTId SOMSTAR otherId)
{
	SOMPPersistentId_parent_SOMUTStringId_somutSetIdId(somSelf,ev,otherId);
}
/* overridden method ::SOMUTId::somutEqualsId */
SOM_Scope boolean SOMLINK pid_somutEqualsId(
	SOMPPersistentId SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMUTId SOMSTAR otherId)
{
	boolean __result;
	__result=SOMPPersistentId_parent_SOMUTStringId_somutEqualsId(somSelf,ev,otherId);
	return __result;}
/* overridden method ::SOMUTId::somutCompareId */
SOM_Scope short SOMLINK pid_somutCompareId(
	SOMPPersistentId SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMUTId SOMSTAR otherId)
{
	short __result;
	__result=SOMPPersistentId_parent_SOMUTStringId_somutCompareId(somSelf,ev,otherId);
	return __result;}
/* overridden method ::SOMUTId::somutHashId */
SOM_Scope unsigned long SOMLINK pid_somutHashId(
	SOMPPersistentId SOMSTAR somSelf,
	Environment *ev)
{
	unsigned long __result;
	__result=SOMPPersistentId_parent_SOMUTStringId_somutHashId(somSelf,ev);
	return __result;}
/* overridden method ::SOMObject::somInit */
SOM_Scope void SOMLINK pid_somInit(
	SOMPPersistentId SOMSTAR somSelf)
{
	RHBOPT_unused(somSelf)
	SOMPPersistentId_parent_SOMUTStringId_somInit(somSelf);
}
/* overridden method ::SOMObject::somUninit */
SOM_Scope void SOMLINK pid_somUninit(
	SOMPPersistentId SOMSTAR somSelf)
{
	RHBOPT_unused(somSelf)
	SOMPPersistentId_parent_SOMUTStringId_somUninit(somSelf);
}
/* overridden method ::SOMObject::somDumpSelfInt */
SOM_Scope void SOMLINK pid_somDumpSelfInt(
	SOMPPersistentId SOMSTAR somSelf,
	/* in */ long level)
{
	SOMPPersistentId_parent_SOMUTStringId_somDumpSelfInt(somSelf,level);
}
/* introduced methods for ::SOMPPersistentId */
/* introduced method ::SOMPPersistentId::sompSetIOGroupMgrClassName */
SOM_Scope void SOMLINK pid_sompSetIOGroupMgrClassName(
	SOMPPersistentId SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring newName)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(newName)
}
/* introduced method ::SOMPPersistentId::sompGetIOGroupMgrClassName */
SOM_Scope corbastring SOMLINK pid_sompGetIOGroupMgrClassName(
	SOMPPersistentId SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring toBuffer)
{
	corbastring __result=NULL;
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(toBuffer)
	return __result;
}
/* introduced method ::SOMPPersistentId::sompGetIOGroupMgrClassNameLen */
SOM_Scope short SOMLINK pid_sompGetIOGroupMgrClassNameLen(
	SOMPPersistentId SOMSTAR somSelf,
	Environment *ev)
{
	short __result=0;
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	return __result;
}
/* introduced method ::SOMPPersistentId::sompSetIOGroupName */
SOM_Scope void SOMLINK pid_sompSetIOGroupName(
	SOMPPersistentId SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring newName)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(newName)
}
/* introduced method ::SOMPPersistentId::sompGetIOGroupName */
SOM_Scope corbastring SOMLINK pid_sompGetIOGroupName(
	SOMPPersistentId SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring toBuffer)
{
	corbastring __result=NULL;
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(toBuffer)
	return __result;
}
/* introduced method ::SOMPPersistentId::sompGetIOGroupNameLen */
SOM_Scope short SOMLINK pid_sompGetIOGroupNameLen(
	SOMPPersistentId SOMSTAR somSelf,
	Environment *ev)
{
	short __result=0;
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	return __result;
}
/* introduced method ::SOMPPersistentId::sompSetGroupOffset */
SOM_Scope void SOMLINK pid_sompSetGroupOffset(
	SOMPPersistentId SOMSTAR somSelf,
	Environment *ev,
	/* in */ long offset)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(offset)
}
/* introduced method ::SOMPPersistentId::sompGetGroupOffset */
SOM_Scope long SOMLINK pid_sompGetGroupOffset(
	SOMPPersistentId SOMSTAR somSelf,
	Environment *ev)
{
	long __result=0;
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	return __result;
}
/* introduced method ::SOMPPersistentId::sompEqualsIOGroupName */
SOM_Scope boolean SOMLINK pid_sompEqualsIOGroupName(
	SOMPPersistentId SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMPPersistentId SOMSTAR id)
{
	boolean __result=0;
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(id)
	return __result;
}
