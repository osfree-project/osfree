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
/* code generater for po.idl */
#define SOM_Module_po_Source
#define M_SOMPPersistentObject_Class_Source
#define SOMPPersistentObject_Class_Source

#include <rhbsomp.h>

#include <po.ih>
/* overridden methods for ::M_SOMPPersistentObject */
/* overridden method ::SOMObject::somInit */
SOM_Scope void SOMLINK m_po_somInit(
	M_SOMPPersistentObject SOMSTAR somSelf)
{
	RHBOPT_unused(somSelf)
	M_SOMPPersistentObject_parent_SOMClass_somInit(somSelf);
}
/* introduced methods for ::M_SOMPPersistentObject */
/* introduced method ::M_SOMPPersistentObject::sompSetClassLevelEncoderDecoderName */
SOM_Scope void SOMLINK m_po_sompSetClassLevelEncoderDecoderName(
	M_SOMPPersistentObject SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring myName)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(myName)
}
/* introduced method ::M_SOMPPersistentObject::sompGetClassLevelEncoderDecoderName */
SOM_Scope corbastring SOMLINK m_po_sompGetClassLevelEncoderDecoderName(
	M_SOMPPersistentObject SOMSTAR somSelf,
	Environment *ev)
{
	corbastring __result=NULL;
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	return __result;
}
/* overridden methods for ::SOMPPersistentObject */
/* overridden method ::SOMObject::somInit */
SOM_Scope void SOMLINK po_somInit(
	SOMPPersistentObject SOMSTAR somSelf)
{
	RHBOPT_unused(somSelf)
	SOMPPersistentObject_parent_SOMObject_somInit(somSelf);
}
/* overridden method ::SOMObject::somUninit */
SOM_Scope void SOMLINK po_somUninit(
	SOMPPersistentObject SOMSTAR somSelf)
{
	RHBOPT_unused(somSelf)
	SOMPPersistentObject_parent_SOMObject_somUninit(somSelf);
}
/* overridden method ::SOMObject::somDumpSelfInt */
SOM_Scope void SOMLINK po_somDumpSelfInt(
	SOMPPersistentObject SOMSTAR somSelf,
	/* in */ long level)
{
	SOMPPersistentObject_parent_SOMObject_somDumpSelfInt(somSelf,level);
}
/* introduced methods for ::SOMPPersistentObject */
/* introduced method ::SOMPPersistentObject::sompInitNextAvail */
SOM_Scope void SOMLINK po_sompInitNextAvail(
	SOMPPersistentObject SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMPIdAssigner SOMSTAR thisAssigner)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(thisAssigner)
}
/* introduced method ::SOMPPersistentObject::sompInitNearObject */
SOM_Scope void SOMLINK po_sompInitNearObject(
	SOMPPersistentObject SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMPPersistentObject SOMSTAR nearObject)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(nearObject)
}
/* introduced method ::SOMPPersistentObject::sompInitGivenId */
SOM_Scope void SOMLINK po_sompInitGivenId(
	SOMPPersistentObject SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMPPersistentId SOMSTAR thisId)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(thisId)
}
/* introduced method ::SOMPPersistentObject::sompGetPersistentId */
SOM_Scope SOMPPersistentId SOMSTAR SOMLINK po_sompGetPersistentId(
	SOMPPersistentObject SOMSTAR somSelf,
	Environment *ev)
{
	SOMPPersistentId SOMSTAR __result=NULL;
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	return __result;
}
/* introduced method ::SOMPPersistentObject::sompGetPersistentIdString */
SOM_Scope corbastring SOMLINK po_sompGetPersistentIdString(
	SOMPPersistentObject SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring outBuff)
{
	corbastring __result=NULL;
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(outBuff)
	return __result;
}
/* introduced method ::SOMPPersistentObject::sompSetEncoderDecoderName */
SOM_Scope void SOMLINK po_sompSetEncoderDecoderName(
	SOMPPersistentObject SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring myName)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(myName)
}
/* introduced method ::SOMPPersistentObject::sompGetEncoderDecoderName */
SOM_Scope corbastring SOMLINK po_sompGetEncoderDecoderName(
	SOMPPersistentObject SOMSTAR somSelf,
	Environment *ev)
{
	corbastring __result=NULL;
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	return __result;
}
/* introduced method ::SOMPPersistentObject::sompEquals */
SOM_Scope boolean SOMLINK po_sompEquals(
	SOMPPersistentObject SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMPPersistentId SOMSTAR otherId)
{
	boolean __result=0;
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(otherId)
	return __result;
}
/* introduced method ::SOMPPersistentObject::sompPassivate */
SOM_Scope void SOMLINK po_sompPassivate(
	SOMPPersistentObject SOMSTAR somSelf,
	Environment *ev)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
}
/* introduced method ::SOMPPersistentObject::sompActivated */
SOM_Scope void SOMLINK po_sompActivated(
	SOMPPersistentObject SOMSTAR somSelf,
	Environment *ev)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
}
/* introduced method ::SOMPPersistentObject::sompGetRelativeIdString */
SOM_Scope corbastring SOMLINK po_sompGetRelativeIdString(
	SOMPPersistentObject SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMObject SOMSTAR ifNearThisObject,
	/* in */ corbastring relativeIdString)
{
	corbastring __result=NULL;
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(ifNearThisObject)
	RHBOPT_unused(relativeIdString)
	return __result;
}
/* introduced method ::SOMPPersistentObject::sompInitIOGroup */
SOM_Scope SOMPIOGroup SOMSTAR SOMLINK po_sompInitIOGroup(
	SOMPPersistentObject SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMObject SOMSTAR nearPersistentObj)
{
	SOMPIOGroup SOMSTAR __result=NULL;
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(nearPersistentObj)
	return __result;
}
/* introduced method ::SOMPPersistentObject::sompSetDirty */
SOM_Scope void SOMLINK po_sompSetDirty(
	SOMPPersistentObject SOMSTAR somSelf,
	Environment *ev)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
}
/* introduced method ::SOMPPersistentObject::sompIsDirty */
SOM_Scope boolean SOMLINK po_sompIsDirty(
	SOMPPersistentObject SOMSTAR somSelf,
	Environment *ev)
{
	boolean __result=0;
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	return __result;
}
/* introduced method ::SOMPPersistentObject::sompGetDirty */
SOM_Scope boolean SOMLINK po_sompGetDirty(
	SOMPPersistentObject SOMSTAR somSelf,
	Environment *ev)
{
	boolean __result=0;
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	return __result;
}
/* introduced method ::SOMPPersistentObject::sompMarkForCompaction */
SOM_Scope void SOMLINK po_sompMarkForCompaction(
	SOMPPersistentObject SOMSTAR somSelf,
	Environment *ev)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
}
/* introduced method ::SOMPPersistentObject::sompSetState */
SOM_Scope void SOMLINK po_sompSetState(
	SOMPPersistentObject SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned short state)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(state)
}
/* introduced method ::SOMPPersistentObject::sompCheckState */
SOM_Scope boolean SOMLINK po_sompCheckState(
	SOMPPersistentObject SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned short state)
{
	boolean __result=0;
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(state)
	return __result;
}
/* introduced method ::SOMPPersistentObject::sompClearState */
SOM_Scope void SOMLINK po_sompClearState(
	SOMPPersistentObject SOMSTAR somSelf,
	Environment *ev,
	/* in */ unsigned short state)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(state)
}
/* introduced method ::SOMPPersistentObject::sompGetEncoderDecoder */
SOM_Scope SOMPEncoderDecoderAbstract SOMSTAR SOMLINK po_sompGetEncoderDecoder(
	SOMPPersistentObject SOMSTAR somSelf,
	Environment *ev)
{
	SOMPEncoderDecoderAbstract SOMSTAR __result=NULL;
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	return __result;
}
/* introduced method ::SOMPPersistentObject::sompFreeEncoderDecoder */
SOM_Scope void SOMLINK po_sompFreeEncoderDecoder(
	SOMPPersistentObject SOMSTAR somSelf,
	Environment *ev)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
}
/* introduced method ::SOMPPersistentObject::sompGetIOGroup */
SOM_Scope SOMPIOGroup SOMSTAR SOMLINK po_sompGetIOGroup(
	SOMPPersistentObject SOMSTAR somSelf,
	Environment *ev)
{
	SOMPIOGroup SOMSTAR __result=NULL;
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	return __result;
}
