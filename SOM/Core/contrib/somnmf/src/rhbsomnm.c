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
#include <rhbsomnm.h>
#include <rhbsomex.h>

SOMInitModule_begin(somnmf)

	SOMInitModule_new(CosNaming_NamingContext);
	SOMInitModule_new(CosNaming_NamingContextExt);
	SOMInitModule_new(CosNaming_BindingIterator);
	SOMInitModule_new(ExtendedNaming_ExtendedNamingContext);
	SOMInitModule_new(FileXNaming_FileENC);
	SOMInitModule_new(Biter_BINDITER_ONE);
	SOMInitModule_new(Biter_BINDITER_TWO);
	SOMInitModule_new(ExtendedNaming_PropertyBindingIterator);
	SOMInitModule_new(ExtendedNaming_PropertyIterator);
	SOMInitModule_new(ExtendedNaming_IndexIterator);
	SOMInitModule_new(FileXNaming_FileBindingIterator);
	SOMInitModule_new(FileXNaming_FPropertyBindingIterator);
	SOMInitModule_new(FileXNaming_FPropertyIterator);
	SOMInitModule_new(FileXNaming_FileIndexIterator);
	SOMInitModule_new(LName);
	SOMInitModule_new(LNameComponent);

SOMInitModule_end

#if defined(_PLATFORM_MACINTOSH_) && GENERATINGCFM
void SOMNMF_CFM_term(void);

RHBOPT_CFM_init(SOMNMF,initBlockPtr)
{
	if (initBlockPtr)
	{	
		return 0;
	}

	return -1;
}

void SOMNMF_CFM_term(void)
{
}
#endif

SOMEXTERN LName SOMSTAR SOMLINK  create_lname(void)
{
	return LNameNew();
}

SOMEXTERN LNameComponent SOMSTAR SOMLINK create_lname_component(void)
{
	return LNameComponentNew();
}

