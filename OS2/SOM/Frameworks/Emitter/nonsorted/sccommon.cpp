/**************************************************************************
 *
 *  Copyright 2015, Yuri Prokushev
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
  return 0;
}

SOM_Scope string SOMLINK _get_somtArrayDimsString(SOMTCommonEntryC SOMSTAR somSelf)
{
  return NULL;
}

SOM_Scope SOMTEntryC SOMSTAR SOMLINK _get_somtTypeObj(SOMTCommonEntryC SOMSTAR somSelf)
{
  return NULL;
}

SOM_Scope somtVisibilityT SOMLINK _get_somtVisibility(SOMTCommonEntryC SOMSTAR somSelf)
{
  return (somtVisibilityT)NULL;
}

SOM_Scope unsigned long SOMLINK somtGetFirstArrayDimension(SOMTCommonEntryC SOMSTAR somSelf)
{
  return 0;
}

SOM_Scope string SOMLINK _get_somtType(SOMTCommonEntryC SOMSTAR somSelf)
{
  return NULL;
}

SOM_Scope string SOMLINK _get_somtPtrs(SOMTCommonEntryC SOMSTAR somSelf)
{
  return NULL;
}

SOM_Scope boolean SOMLINK somtIsPointer(SOMTCommonEntryC SOMSTAR somSelf)
{
  return FALSE;
}

SOM_Scope unsigned long SOMLINK somtGetNextArrayDimension(SOMTCommonEntryC SOMSTAR somSelf)
{
  return 0;
}

SOM_Scope boolean SOMLINK somtIsArray(SOMTCommonEntryC SOMSTAR somSelf,
	                                    /* out */ long *size)
{
  return FALSE;
}

