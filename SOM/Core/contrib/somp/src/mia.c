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
/* code generater for mia.idl */
#define SOM_Module_mia_Source
#define SOMPMediaInterfaceAbstract_Class_Source
#include <rhbsomp.h>
#include <mia.ih>
/* overridden methods for ::SOMPMediaInterfaceAbstract */
/* introduced methods for ::SOMPMediaInterfaceAbstract */
/* introduced method ::SOMPMediaInterfaceAbstract::sompOpen */
SOM_Scope void SOMLINK mia_sompOpen(
	SOMPMediaInterfaceAbstract SOMSTAR somSelf,
	Environment *ev)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
}
/* introduced method ::SOMPMediaInterfaceAbstract::sompClose */
SOM_Scope void SOMLINK mia_sompClose(
	SOMPMediaInterfaceAbstract SOMSTAR somSelf,
	Environment *ev)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
}
