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
/* code generater for fsgm.idl */
#define SOM_Module_fsgm_Source
#define SOMPBinary_Class_Source
#include <rhbsomp.h>
#include <fsgm.ih>
/* overridden methods for ::SOMPBinary */
/* overridden method ::SOMPIOGroupMgrAbstract::sompInstantiateMediaInterface */
SOM_Scope SOMPMediaInterfaceAbstract SOMSTAR SOMLINK fsgm_sompInstantiateMediaInterface(
	SOMPBinary SOMSTAR somSelf,
	Environment *ev)
{
	SOMPMediaInterfaceAbstract SOMSTAR __result;
	__result=SOMPBinary_parent_SOMPAscii_sompInstantiateMediaInterface(somSelf,ev);
	return __result;}
/* introduced methods for ::SOMPBinary */
