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
/* code generater for iogma.idl */
#define SOM_Module_iogma_Source
#define SOMPIOGroupMgrAbstract_Class_Source
#include <rhbsomp.h>
#include <iogma.ih>
/* overridden methods for ::SOMPIOGroupMgrAbstract */
/* introduced methods for ::SOMPIOGroupMgrAbstract */
/* introduced method ::SOMPIOGroupMgrAbstract::sompNewMediaInterface */
SOM_Scope void SOMLINK iogma_sompNewMediaInterface(
	SOMPIOGroupMgrAbstract SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring IOInfo)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(IOInfo)
}
/* introduced method ::SOMPIOGroupMgrAbstract::sompGetMediaInterface */
SOM_Scope SOMPMediaInterfaceAbstract SOMSTAR SOMLINK iogma_sompGetMediaInterface(
	SOMPIOGroupMgrAbstract SOMSTAR somSelf,
	Environment *ev)
{
	SOMPMediaInterfaceAbstract SOMSTAR __result=NULL;
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	return __result;
}
/* introduced method ::SOMPIOGroupMgrAbstract::sompFreeMediaInterface */
SOM_Scope void SOMLINK iogma_sompFreeMediaInterface(
	SOMPIOGroupMgrAbstract SOMSTAR somSelf,
	Environment *ev)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
}
/* introduced method ::SOMPIOGroupMgrAbstract::sompInstantiateMediaInterface */
SOM_Scope SOMPMediaInterfaceAbstract SOMSTAR SOMLINK iogma_sompInstantiateMediaInterface(
	SOMPIOGroupMgrAbstract SOMSTAR somSelf,
	Environment *ev)
{
	SOMPMediaInterfaceAbstract SOMSTAR __result=NULL;
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	return __result;
}
/* introduced method ::SOMPIOGroupMgrAbstract::sompWriteGroup */
SOM_Scope boolean SOMLINK iogma_sompWriteGroup(
	SOMPIOGroupMgrAbstract SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMPPersistentObject SOMSTAR thisPo)
{
	boolean __result=0;
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(thisPo)
	return __result;
}
/* introduced method ::SOMPIOGroupMgrAbstract::sompReadGroup */
SOM_Scope SOMPPersistentObject SOMSTAR SOMLINK iogma_sompReadGroup(
	SOMPIOGroupMgrAbstract SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMPPersistentId SOMSTAR objectID)
{
	SOMPPersistentObject SOMSTAR __result=NULL;
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(objectID)
	return __result;
}
/* introduced method ::SOMPIOGroupMgrAbstract::sompReadObjectData */
SOM_Scope void SOMLINK iogma_sompReadObjectData(
	SOMPIOGroupMgrAbstract SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMPPersistentObject SOMSTAR thisPo)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(thisPo)
}
/* introduced method ::SOMPIOGroupMgrAbstract::sompGroupExists */
SOM_Scope boolean SOMLINK iogma_sompGroupExists(
	SOMPIOGroupMgrAbstract SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring IOInfo)
{
	boolean __result=0;
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(IOInfo)
	return __result;
}
/* introduced method ::SOMPIOGroupMgrAbstract::sompObjectInGroup */
SOM_Scope boolean SOMLINK iogma_sompObjectInGroup(
	SOMPIOGroupMgrAbstract SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMPPersistentId SOMSTAR objectID)
{
	boolean __result=0;
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(objectID)
	return __result;
}
/* introduced method ::SOMPIOGroupMgrAbstract::sompMediaFormatOk */
SOM_Scope boolean SOMLINK iogma_sompMediaFormatOk(
	SOMPIOGroupMgrAbstract SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring mediaFormatName)
{
	boolean __result=0;
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(mediaFormatName)
	return __result;
}
/* introduced method ::SOMPIOGroupMgrAbstract::sompDeleteObjectFromGroup */
SOM_Scope void SOMLINK iogma_sompDeleteObjectFromGroup(
	SOMPIOGroupMgrAbstract SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMPPersistentId SOMSTAR objectID)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(objectID)
}
