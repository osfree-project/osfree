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

#ifndef SOM_Module_scstruct_Source
#define SOM_Module_scstruct_Source
#endif

#define SOMTStructEntryC_Class_Source

#include "scstruct.xih"
#include <scemit.xh>
#include <sctdef.xh>
#include <scclass.xh>

SOM_Scope long SOMLINK somtSetSymbolsOnEntry(SOMTStructEntryC SOMSTAR somSelf,
	                                           /* in */ SOMTEmitC SOMSTAR emitter,
	                                           /* in */ string prefix)
{
  SOMTStructEntryCData *somThis = SOMTStructEntryCGetData(somSelf);
  SOMTStructEntryCMethodDebug("SOMTStructEntryC","somtSetSymbolsOnEntry");

  return 0;
}

SOM_Scope SOMTTypedefEntryC SOMSTAR SOMLINK somtGetNextMember(SOMTStructEntryC SOMSTAR somSelf)
{
  SOMTStructEntryCData *somThis = SOMTStructEntryCGetData(somSelf);
  SOMTStructEntryCMethodDebug("SOMTStructEntryC","somtGetNextMember");

  return NULL;
}

SOM_Scope boolean SOMLINK _get_somtIsException(SOMTStructEntryC SOMSTAR somSelf)
{
  SOMTStructEntryCData *somThis = SOMTStructEntryCGetData(somSelf);
  SOMTStructEntryCMethodDebug("SOMTStructEntryC","_get_somtIsException");

  return FALSE;
}

SOM_Scope SOMTTypedefEntryC SOMSTAR SOMLINK somtGetFirstMember(SOMTStructEntryC SOMSTAR somSelf)
{
  SOMTStructEntryCData *somThis = SOMTStructEntryCGetData(somSelf);
  SOMTStructEntryCMethodDebug("SOMTStructEntryC","somtGetFirstMember");

  return NULL;
}

SOM_Scope SOMTClassEntryC SOMSTAR SOMLINK _get_somtStructClass(SOMTStructEntryC SOMSTAR somSelf)
{
  SOMTStructEntryCData *somThis = SOMTStructEntryCGetData(somSelf);
  SOMTStructEntryCMethodDebug("SOMTStructEntryC","_get_somtStructClass");

  return NULL;
}
