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
#include <rhbsomex.h>
#include <rhbdynam.h>
#include <dynany.h>

SOMInitModule_begin(somany)

	SOMInitModule_new(DynamicAny_DynAny);
	SOMInitModule_new(DynamicAny_DynFixed);
	SOMInitModule_new(DynamicAny_DynUnion);
	SOMInitModule_new(DynamicAny_DynStruct);
	SOMInitModule_new(DynamicAny_DynArray);
	SOMInitModule_new(DynamicAny_DynSequence);
	SOMInitModule_new(DynamicAny_DynValue);
	SOMInitModule_new(DynamicAny_DynEnum);
	SOMInitModule_new(DynamicAny_DynAnyFactory);

SOMInitModule_end

#if defined(_PLATFORM_MACINTOSH_) && GENERATINGCFM
void SOMANY_CFM_term(void);
RHBOPT_CFM_init(SOMANY,initBlockPtr)
{
	return 0;
}
void SOMANY_CFM_term(void)
{
}
#endif
