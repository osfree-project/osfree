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
/* code generater for defedidl.idl */
#define SOMPAttrEncoderDecoder_Class_Source
#include <rhbsomp.h>
#include <defedidl.ih>
/* overridden methods for ::SOMPAttrEncoderDecoder */
/* overridden method ::SOMPEncoderDecoderAbstract::sompEDWrite */
SOM_Scope void SOMLINK defedidl_sompEDWrite(
	SOMPAttrEncoderDecoder SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMPMediaInterfaceAbstract SOMSTAR thisMedia,
	/* in */ SOMPPersistentObject SOMSTAR thisObject)
{
	SOMPAttrEncoderDecoder_parent_SOMPEncoderDecoderAbstract_sompEDWrite(somSelf,ev,thisMedia,thisObject);
}
/* overridden method ::SOMPEncoderDecoderAbstract::sompEDRead */
SOM_Scope void SOMLINK defedidl_sompEDRead(
	SOMPAttrEncoderDecoder SOMSTAR somSelf,
	Environment *ev,
	/* in */ SOMPMediaInterfaceAbstract SOMSTAR thisMedia,
	/* in */ SOMPPersistentObject SOMSTAR thisObject)
{
	SOMPAttrEncoderDecoder_parent_SOMPEncoderDecoderAbstract_sompEDRead(somSelf,ev,thisMedia,thisObject);
}
/* overridden method ::SOMObject::somInit */
SOM_Scope void SOMLINK defedidl_somInit(
	SOMPAttrEncoderDecoder SOMSTAR somSelf)
{
	RHBOPT_unused(somSelf)

	SOMPAttrEncoderDecoder_parent_SOMPEncoderDecoderAbstract_somInit(somSelf);
}
/* introduced methods for ::SOMPAttrEncoderDecoder */
