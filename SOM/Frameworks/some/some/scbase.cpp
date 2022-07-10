/**************************************************************************
 *
 *  Copyright 2015, 2017 Yuri Prokushev
 *
 *  This file is part of osFree project
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

#ifndef SOM_Module_scbase_Source
#define SOM_Module_scbase_Source
#endif

#define SOMTBaseClassEntryC_Class_Source

#include "scbase.xih"

SOM_Scope SOMTClassEntryC SOMSTAR SOMLINK _get_somtBaseClassDef(SOMTBaseClassEntryC SOMSTAR somSelf)
{
  SOMTBaseClassEntryCData *somThis = SOMTBaseClassEntryCGetData(somSelf);
  SOMTBaseClassEntryCMethodDebug("SOMTBaseClassEntryC","_get_somtBaseClassDef");

  return NULL;
}

SOM_Scope long SOMLINK somtSetSymbolsOnEntry(SOMTBaseClassEntryC SOMSTAR somSelf,
	                                           /* in */ SOMTEmitC SOMSTAR emitter,
	                                           /* in */ string prefix)
{
  SOMTBaseClassEntryCData *somThis = SOMTBaseClassEntryCGetData(somSelf);
  SOMTBaseClassEntryCMethodDebug("SOMTBaseClassEntryC","somtSetSymbolsOnEntry");

  return 0;
}
