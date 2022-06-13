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

#define SOM_Module_somtdm_Source

#define TypecodeDataManipulator_Class_Source

#include <rhbsomu2.h>
#include <somtdm.ih>

SOM_Scope void SOMLINK somtdm_somUninit(
	TypecodeDataManipulator SOMSTAR somSelf)
{
	TypecodeDataManipulator_parent_SOMObject_somUninit(somSelf);
}

SOM_Scope void SOMLINK somtdm_somInit(
	TypecodeDataManipulator SOMSTAR somSelf)
{
	TypecodeDataManipulator_parent_SOMObject_somInit(somSelf);
}
