/**************************************************************************
 *
 *  Copyright 2015, Yuri Prokushev
 *
 *  This file is part of osFree project
 *
 *  This program is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License as published by the
 *  Free Software Foundation, either version 2 of the License, or (at your
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

#ifndef SOM_Module_scenumnm_Source
#define SOM_Module_scenumnm_Source
#endif

#define SOMTEnumNameEntryC_Class_Source

#include "scenumnm.xih"

SOM_Scope long SOMLINK somtSetSymbolsOnEntry(SOMTEnumNameEntryC SOMSTAR somSelf,
	                                           /* in */ SOMTEmitC SOMSTAR emitter,
	                                           /* in */ string prefix)
{
  return 0;
}

SOM_Scope SOMTEnumEntryC SOMSTAR SOMLINK _get_somtEnumPtr(SOMTEnumNameEntryC SOMSTAR somSelf)
{
  return NULL;
}

SOM_Scope unsigned long SOMLINK _get_somtEnumVal(SOMTEnumNameEntryC SOMSTAR somSelf)
{
  return 0;
}
