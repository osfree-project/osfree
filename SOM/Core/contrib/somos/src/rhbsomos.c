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

#ifdef _PLATFORM_MACINTOSH_
	#include <CodeFragments.h>
#endif

#include <rhbsomos.h>

SOMInitModule_begin(somos)

	SOMInitModule_new(somOS_Server);
	SOMInitModule_new(somOS_ServiceBase);
	SOMInitModule_new(somOS_ServiceBasePRef);
	SOMInitModule_new(somOS_ServiceBaseCORBA);
	SOMInitModule_new(somOS_ServiceBasePRefCORBA);
	SOMInitModule_new(somAttributePersistence_Persist);

SOMInitModule_end

#ifdef _PLATFORM_MACINTOSH_
void SOMOS_CFM_term(void);

RHBOPT_CFM_init(SOMOS,initBlockPtr)
{
	if (initBlockPtr)
	{	
		return 0;
	}

	return -1;
}

void SOMOS_CFM_term(void)
{
}
#endif
