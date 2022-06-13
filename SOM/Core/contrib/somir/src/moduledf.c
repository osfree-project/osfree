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

#define ModuleDef_Class_Source
#define M_ModuleDef_Class_Source

#include <rhbsomir.h>
#include <moduledf.ih>

SOM_Scope void SOMLINK moduledf_somDumpSelf(
	ModuleDef SOMSTAR somSelf,
	/* in */ long level)
{
	ModuleDef_parent_Contained_somDumpSelf(somSelf,level);
}

SOM_Scope void SOMLINK moduledf_somDumpSelfInt(
	ModuleDef SOMSTAR somSelf,
	/* in */ long level)
{
	ModuleDef_parent_Contained_somDumpSelfInt(somSelf,level);
	ModuleDef_parent_Container_somDumpSelfInt(somSelf,level);
}

/*
SOM_Scope void SOMLINK moduledf_somInit(
	ModuleDef SOMSTAR somSelf)
{
	ModuleDef_parent_Container_somInit(somSelf);
	ModuleDef_parent_Contained_somInit(somSelf);
	SOM_IgnoreWarning(somSelf);
}
*/
/*
SOM_Scope void SOMLINK moduledf_somUninit(
	ModuleDef SOMSTAR somSelf)
{
	SOM_IgnoreWarning(somSelf);
	ModuleDef_parent_Container_somUninit(somSelf);
	ModuleDef_parent_Contained_somUninit(somSelf);
}
*/

SOM_Scope void SOMLINK moduledf_somDestruct(
		ModuleDef SOMSTAR somSelf,
		boolean doFree,
		somDestructCtrl *ctrl)
{
	somDestructCtrl globalCtrl;
	somBooleanVector myMask;

	ModuleDef_BeginDestructor

	ModuleDef_EndDestructor
}

SOM_Scope void SOMLINK m_moduledf_somDestruct(M_ModuleDef SOMSTAR somSelf,boolean doFree,
											somDestructCtrl *ctrl)
{
	somDestructCtrl globalCtrl;
	somBooleanVector myMask;
/*	M_ModuleDef *somThis;*/

	M_ModuleDef_BeginDestructor;

	M_ModuleDef_EndDestructor;
}

SOM_Scope void SOMLINK m_moduledf_somDefaultInit(M_ModuleDef SOMSTAR somSelf,
											   somInitCtrl *ctrl)
{
	somInitCtrl globalCtrl;
	somBooleanVector myMask;
/*	M_ModuleDef *somThis;*/

	M_ModuleDef_BeginInitializer_somDefaultInit

	M_ModuleDef_Init_M_Contained_somDefaultInit(somSelf,ctrl)
}
