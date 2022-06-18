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
/* code generater for nsma.idl */
#define SOM_Module_nsma_Source
#define SOMPNameSpaceMgr_Class_Source
#include <rhbsomp.h>
#include <nsma.ih>
/* overridden methods for ::SOMPNameSpaceMgr */
/* overridden method ::SOMObject::somInit */
SOM_Scope void SOMLINK nsma_somInit(
	SOMPNameSpaceMgr SOMSTAR somSelf)
{
	RHBOPT_unused(somSelf)
	SOMPNameSpaceMgr_parent_SOMObject_somInit(somSelf);
}
/* overridden method ::SOMObject::somUninit */
SOM_Scope void SOMLINK nsma_somUninit(
	SOMPNameSpaceMgr SOMSTAR somSelf)
{
	RHBOPT_unused(somSelf)
	SOMPNameSpaceMgr_parent_SOMObject_somUninit(somSelf);
}
/* overridden method ::SOMObject::somDumpSelfInt */
SOM_Scope void SOMLINK nsma_somDumpSelfInt(
	SOMPNameSpaceMgr SOMSTAR somSelf,
	/* in */ long level)
{
	SOMPNameSpaceMgr_parent_SOMObject_somDumpSelfInt(somSelf,level);
}
/* introduced methods for ::SOMPNameSpaceMgr */
