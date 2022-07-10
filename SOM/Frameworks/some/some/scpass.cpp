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

#ifndef SOM_Module_scpass_Source
#define SOM_Module_scpass_Source
#endif

#define SOMTPassthruEntryC_Class_Source

#include "scpass.xih"

SOM_Scope long SOMLINK somtSetSymbolsOnEntry(SOMTPassthruEntryC SOMSTAR somSelf,
	                                           /* in */ SOMTEmitC SOMSTAR emitter,
	                                           /* in */ string prefix)
{
  SOMTPassthruEntryCData *somThis = SOMTPassthruEntryCGetData(somSelf);
  SOMTPassthruEntryCMethodDebug("SOMTPassthruEntryC","somtSetSymbolsOnEntry");

  return 0;
}

SOM_Scope string SOMLINK _get_somtPassthruTarget(SOMTPassthruEntryC SOMSTAR somSelf)
{
  SOMTPassthruEntryCData *somThis = SOMTPassthruEntryCGetData(somSelf);
  SOMTPassthruEntryCMethodDebug("SOMTPassthruEntryC","_get_somtPassthruTarget");

  return NULL;
}

SOM_Scope string SOMLINK _get_somtPassthruBody(SOMTPassthruEntryC SOMSTAR somSelf)
{
  SOMTPassthruEntryCData *somThis = SOMTPassthruEntryCGetData(somSelf);
  SOMTPassthruEntryCMethodDebug("SOMTPassthruEntryC","_get_somtPassthruBody");

  return NULL;
}

SOM_Scope string SOMLINK _get_somtPassthruLanguage(SOMTPassthruEntryC SOMSTAR somSelf)
{
  SOMTPassthruEntryCData *somThis = SOMTPassthruEntryCGetData(somSelf);
  SOMTPassthruEntryCMethodDebug("SOMTPassthruEntryC","_get_somtPassthruLanguage");

  return NULL;
}

SOM_Scope boolean SOMLINK somtIsBeforePassthru(SOMTPassthruEntryC SOMSTAR somSelf)
{
  SOMTPassthruEntryCData *somThis = SOMTPassthruEntryCGetData(somSelf);
  SOMTPassthruEntryCMethodDebug("SOMTPassthruEntryC","somtIsBeforePassthru");

  return FALSE;
}
