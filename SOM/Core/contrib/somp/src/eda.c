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
/* code generater for eda.idl */
#define SOM_Module_eda_Source
#define SOMPEncoderDecoderAbstract_Class_Source
#include <rhbsomp.h>
#include <eda.ih>
/* overridden methods for ::SOMPEncoderDecoderAbstract */
/* introduced methods for ::SOMPEncoderDecoderAbstract */
/* introduced method ::SOMPEncoderDecoderAbstract::sompEDWrite */
SOM_Scope void SOMLINK eda_sompEDWrite(
	SOMPEncoderDecoderAbstract SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMPMediaInterfaceAbstract SOMSTAR thisMedia,
	/* in */ SOMPPersistentObject SOMSTAR thisObject)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(thisMedia)
	RHBOPT_unused(thisObject)
}
/* introduced method ::SOMPEncoderDecoderAbstract::sompEDRead */
SOM_Scope void SOMLINK eda_sompEDRead(
	SOMPEncoderDecoderAbstract SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMPMediaInterfaceAbstract SOMSTAR thisMedia,
	/* in */ SOMPPersistentObject SOMSTAR thisObject)
{
	RHBOPT_unused(somSelf)
	RHBOPT_unused(ev)
	RHBOPT_unused(thisMedia)
	RHBOPT_unused(thisObject)
}
