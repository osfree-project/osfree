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

#ifndef SOM_Module_sccommon_Source
#define SOM_Module_sccommon_Source
#endif

#define SOMTCommonEntryC_Class_Source

#define __PRIVATE__
#include <rhbopt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "rhbsc.h"

#include "sccommon.xih"

SOM_Scope long SOMLINK somtSetSymbolsOnEntry(SOMTCommonEntryC SOMSTAR somSelf,
	                                           /* in */ SOMTEmitC SOMSTAR emitter,
	                                           /* in */ string prefix)
{
  SOMTCommonEntryCData *somThis = SOMTCommonEntryCGetData(somSelf);
  SOMTCommonEntryCMethodDebug("SOMTCommonEntryC","somtSetSymbolsOnEntry");

  return 0;
}

SOM_Scope string SOMLINK _get_somtArrayDimsString(SOMTCommonEntryC SOMSTAR somSelf)
{
  SOMTCommonEntryCData *somThis = SOMTCommonEntryCGetData(somSelf);
  SOMTCommonEntryCMethodDebug("SOMTCommonEntryC","_get_somtArrayDimsString");

  return NULL;
}

SOM_Scope SOMTEntryC SOMSTAR SOMLINK _get_somtTypeObj(SOMTCommonEntryC SOMSTAR somSelf)
{
  SOMTCommonEntryCData *somThis = SOMTCommonEntryCGetData(somSelf);
  SOMTCommonEntryCMethodDebug("SOMTCommonEntryC","_get_somtTypeObj");

  return NULL;
}

SOM_Scope somtVisibilityT SOMLINK _get_somtVisibility(SOMTCommonEntryC SOMSTAR somSelf)
{
  SOMTCommonEntryCData *somThis = SOMTCommonEntryCGetData(somSelf);
  SOMTCommonEntryCMethodDebug("SOMTCommonEntryC","_get_somtVisibility");

  return (somtVisibilityT)NULL;
}

SOM_Scope unsigned long SOMLINK somtGetFirstArrayDimension(SOMTCommonEntryC SOMSTAR somSelf)
{
  SOMTCommonEntryCData *somThis = SOMTCommonEntryCGetData(somSelf);
  SOMTCommonEntryCMethodDebug("SOMTCommonEntryC","somtGetFirstArrayDimension");

  return 0;
}

SOM_Scope string SOMLINK _get_somtType(SOMTCommonEntryC SOMSTAR somSelf)
{
  SOMTCommonEntryCData *somThis = SOMTCommonEntryCGetData(somSelf);
  SOMTCommonEntryCMethodDebug("SOMTCommonEntryC","_get_somtType");

  return NULL;
}

SOM_Scope string SOMLINK _get_somtPtrs(SOMTCommonEntryC SOMSTAR somSelf)
{
  SOMTCommonEntryCData *somThis = SOMTCommonEntryCGetData(somSelf);
  SOMTCommonEntryCMethodDebug("SOMTCommonEntryC","_get_somtPtrs");

  return NULL;
}

SOM_Scope boolean SOMLINK somtIsPointer(SOMTCommonEntryC SOMSTAR somSelf)
{
  SOMTCommonEntryCData *somThis = SOMTCommonEntryCGetData(somSelf);
  SOMTCommonEntryCMethodDebug("SOMTCommonEntryC","somtIsPointer");

  return FALSE;
}

SOM_Scope unsigned long SOMLINK somtGetNextArrayDimension(SOMTCommonEntryC SOMSTAR somSelf)
{
  SOMTCommonEntryCData *somThis = SOMTCommonEntryCGetData(somSelf);
  SOMTCommonEntryCMethodDebug("SOMTCommonEntryC","somtGetNextArrayDimension");

  return 0;
}

SOM_Scope boolean SOMLINK somtIsArray(SOMTCommonEntryC SOMSTAR somSelf,
	                                    /* out */ long *size)
{
  SOMTCommonEntryCData *somThis = SOMTCommonEntryCGetData(somSelf);
  SOMTCommonEntryCMethodDebug("SOMTCommonEntryC","somtIsArray");

  return FALSE;
}

