/**************************************************************************
 *
 *  Copyright 2018 Yuri Prokushev
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

#ifndef SOM_Module_scusrtyp_Source
#define SOM_Module_scusrtyp_Source
#endif

#define SOMTStringEntryC_Class_Source

#define __PRIVATE__
#include <rhbopt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "rhbsc.h"

#define SOMTUserDefinedTypeEntryC_Class_Source

#include "scusrtyp.xih"

SOM_Scope long SOMLINK somtSetSymbolsOnEntry(SOMTUserDefinedTypeEntryC SOMSTAR somSelf,
	                                           /* in */ SOMTEmitC SOMSTAR emitter,
	                                           /* in */ string prefix)
{
  SOMTUserDefinedTypeEntryCData *somThis = SOMTUserDefinedTypeEntryCGetData(somSelf);
  SOMTUserDefinedTypeEntryCMethodDebug("SOMTUserDefinedTypeEntryC","somtSetSymbolsOnEntry");
  
  return 1;
}


SOM_Scope void SOMLINK somDumpSelfInt(SOMTUserDefinedTypeEntryC * somSelf,
                                      /* in */ long level)
{
}

SOM_Scope SOMTEntryC SOMSTAR SOMLINK _get_somtBaseTypeObj(SOMTUserDefinedTypeEntryC SOMSTAR somSelf)
{
  return NULL;
}

SOM_Scope SOMTTypedefEntryC SOMSTAR SOMLINK _get_somtOriginalTypedef(SOMTUserDefinedTypeEntryC SOMSTAR somSelf)
{
  return NULL;
}

SOM_Scope SOMTEntryC SOMSTAR SOMLINK _get_somtTypeObj(SOMTUserDefinedTypeEntryC SOMSTAR somSelf)
{
  return NULL;
}
