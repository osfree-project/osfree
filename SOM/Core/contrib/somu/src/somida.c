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


#define SOMUTId_Class_Source

#include <somida.h>
#include <somida.ih>

SOM_Scope unsigned long SOMLINK somida_somutHashId(
	SOMUTId SOMSTAR somSelf,
	Environment *ev)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);

	return 0;
}

SOM_Scope short SOMLINK somida_somutCompareId(
	SOMUTId SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMUTId SOMSTAR otherId)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(otherId);

	return 0;
}

SOM_Scope boolean SOMLINK somida_somutEqualsId(
	SOMUTId SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMUTId SOMSTAR otherId)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(otherId);

	return 0;
}

SOM_Scope void SOMLINK somida_somutSetIdId(
	SOMUTId SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMUTId SOMSTAR otherId)
{
	SOM_IgnoreWarning(somSelf);
	SOM_IgnoreWarning(ev);
	SOM_IgnoreWarning(otherId);
}
