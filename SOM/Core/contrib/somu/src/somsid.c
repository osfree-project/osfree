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

#define SOM_Module_somida_Source
#define SOM_Module_somsid_Source

#define SOMUTStringId_Class_Source

#include <rhbsomex.h>

#include <somida.h>
#include <somsid.h>
#include <somsid.ih>

SOM_Scope unsigned long SOMLINK somsid_somutHashId(
	SOMUTStringId SOMSTAR somSelf,
	Environment *ev)
{
	return SOMUTStringId_parent_SOMUTId_somutHashId(somSelf,ev);
}

SOM_Scope short SOMLINK somsid_somutCompareId(
	SOMUTStringId SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMUTId SOMSTAR otherId)
{
	return SOMUTStringId_parent_SOMUTId_somutCompareId(somSelf,ev,otherId);
}

SOM_Scope boolean SOMLINK somsid_somutEqualsId(
	SOMUTStringId SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMUTId SOMSTAR otherId)
{
	return SOMUTStringId_parent_SOMUTId_somutEqualsId(somSelf,ev,otherId);
}

SOM_Scope void SOMLINK somsid_somutSetIdId(
	SOMUTStringId SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMUTId SOMSTAR otherId)
{
	SOMUTStringId_parent_SOMUTId_somutSetIdId(somSelf,ev,otherId);
}

SOM_Scope boolean SOMLINK somsid_somutEqualsString(
	SOMUTStringId SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring IdString)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(IdString);

	return 0;
}

SOM_Scope short SOMLINK somsid_somutCompareString(
	SOMUTStringId SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring IdString)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(IdString);

	return 0;
}

SOM_Scope corbastring SOMLINK somsid_somutGetIdString(
	SOMUTStringId SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring toBuffer)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(toBuffer);

	return toBuffer;
}

SOM_Scope long SOMLINK somsid_somutGetIdStringLen(
	SOMUTStringId SOMSTAR somSelf,
	Environment *ev)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);

	return 0;
}

SOM_Scope long SOMLINK somsid_somutSetIdString(
	SOMUTStringId SOMSTAR somSelf,
	Environment *ev,
	/* in */ corbastring IdString)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(IdString);

	return 0;
}

