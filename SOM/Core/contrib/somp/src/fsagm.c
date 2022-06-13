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
/* code generater for fsagm.idl */
#define SOM_Module_fsagm_Source
#define SOMPAscii_Class_Source
#include <rhbsomp.h>
#include <fsagm.ih>
/* overridden methods for ::SOMPAscii */
/* overridden method ::SOMPIOGroupMgrAbstract::sompNewMediaInterface */
SOM_Scope void SOMLINK fsagm_sompNewMediaInterface(
	SOMPAscii SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring IOInfo)
{
	SOMPAscii_parent_SOMPIOGroupMgrAbstract_sompNewMediaInterface(somSelf,ev,IOInfo);
}
/* overridden method ::SOMPIOGroupMgrAbstract::sompGetMediaInterface */
SOM_Scope SOMPMediaInterfaceAbstract SOMSTAR SOMLINK fsagm_sompGetMediaInterface(
	SOMPAscii SOMSTAR somSelf,
	Environment *ev)
{
	SOMPMediaInterfaceAbstract SOMSTAR __result;
	__result=SOMPAscii_parent_SOMPIOGroupMgrAbstract_sompGetMediaInterface(somSelf,ev);
	return __result;}
/* overridden method ::SOMPIOGroupMgrAbstract::sompFreeMediaInterface */
SOM_Scope void SOMLINK fsagm_sompFreeMediaInterface(
	SOMPAscii SOMSTAR somSelf,
	Environment *ev)
{
	SOMPAscii_parent_SOMPIOGroupMgrAbstract_sompFreeMediaInterface(somSelf,ev);
}
/* overridden method ::SOMPIOGroupMgrAbstract::sompInstantiateMediaInterface */
SOM_Scope SOMPMediaInterfaceAbstract SOMSTAR SOMLINK fsagm_sompInstantiateMediaInterface(
	SOMPAscii SOMSTAR somSelf,
	Environment *ev)
{
	SOMPMediaInterfaceAbstract SOMSTAR __result;
	__result=SOMPAscii_parent_SOMPIOGroupMgrAbstract_sompInstantiateMediaInterface(somSelf,ev);
	return __result;}
/* overridden method ::SOMPIOGroupMgrAbstract::sompWriteGroup */
SOM_Scope boolean SOMLINK fsagm_sompWriteGroup(
	SOMPAscii SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMPPersistentObject SOMSTAR thisPo)
{
	boolean __result;
	__result=SOMPAscii_parent_SOMPIOGroupMgrAbstract_sompWriteGroup(somSelf,ev,thisPo);
	return __result;}
/* overridden method ::SOMPIOGroupMgrAbstract::sompReadGroup */
SOM_Scope SOMPPersistentObject SOMSTAR SOMLINK fsagm_sompReadGroup(
	SOMPAscii SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMPPersistentId SOMSTAR objectID)
{
	SOMPPersistentObject SOMSTAR __result;
	__result=SOMPAscii_parent_SOMPIOGroupMgrAbstract_sompReadGroup(somSelf,ev,objectID);
	return __result;}
/* overridden method ::SOMPIOGroupMgrAbstract::sompReadObjectData */
SOM_Scope void SOMLINK fsagm_sompReadObjectData(
	SOMPAscii SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMPPersistentObject SOMSTAR thisPo)
{
	SOMPAscii_parent_SOMPIOGroupMgrAbstract_sompReadObjectData(somSelf,ev,thisPo);
}
/* overridden method ::SOMPIOGroupMgrAbstract::sompGroupExists */
SOM_Scope boolean SOMLINK fsagm_sompGroupExists(
	SOMPAscii SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring IOInfo)
{
	boolean __result;
	__result=SOMPAscii_parent_SOMPIOGroupMgrAbstract_sompGroupExists(somSelf,ev,IOInfo);
	return __result;}
/* overridden method ::SOMPIOGroupMgrAbstract::sompObjectInGroup */
SOM_Scope boolean SOMLINK fsagm_sompObjectInGroup(
	SOMPAscii SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMPPersistentId SOMSTAR objectID)
{
	boolean __result;
	__result=SOMPAscii_parent_SOMPIOGroupMgrAbstract_sompObjectInGroup(somSelf,ev,objectID);
	return __result;}
/* overridden method ::SOMPIOGroupMgrAbstract::sompMediaFormatOk */
SOM_Scope boolean SOMLINK fsagm_sompMediaFormatOk(
	SOMPAscii SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring mediaFormatName)
{
	boolean __result;
	__result=SOMPAscii_parent_SOMPIOGroupMgrAbstract_sompMediaFormatOk(somSelf,ev,mediaFormatName);
	return __result;}
/* overridden method ::SOMPIOGroupMgrAbstract::sompDeleteObjectFromGroup */
SOM_Scope void SOMLINK fsagm_sompDeleteObjectFromGroup(
	SOMPAscii SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMPPersistentId SOMSTAR objectID)
{
	SOMPAscii_parent_SOMPIOGroupMgrAbstract_sompDeleteObjectFromGroup(somSelf,ev,objectID);
}
/* overridden method ::SOMObject::somInit */
SOM_Scope void SOMLINK fsagm_somInit(
	SOMPAscii SOMSTAR somSelf)
{
	RHBOPT_unused(somSelf)
	SOMPAscii_parent_SOMPIOGroupMgrAbstract_somInit(somSelf);
}
/* overridden method ::SOMObject::somUninit */
SOM_Scope void SOMLINK fsagm_somUninit(
	SOMPAscii SOMSTAR somSelf)
{
	RHBOPT_unused(somSelf)
	SOMPAscii_parent_SOMPIOGroupMgrAbstract_somUninit(somSelf);
}
/* introduced methods for ::SOMPAscii */
