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

#ifndef SOM_Module_scdata_Source
#define SOM_Module_scdata_Source
#endif

#define SOMTDataEntryC_Class_Source

#define __PRIVATE__
#include <rhbopt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "rhbsc.h"

#include "scdata.xih"

SOM_Scope long SOMLINK somtSetSymbolsOnEntry(SOMTDataEntryC SOMSTAR somSelf,
	                                           /* in */ SOMTEmitC SOMSTAR emitter,
	                                           /* in */ string prefix)
{
  SOMTDataEntryCData *somThis = SOMTDataEntryCGetData(somSelf);
  SOMTDataEntryCMethodDebug("SOMTDataEntryC","somtSetSymbolsOnEntry");

  return 0;
}

SOM_Scope boolean SOMLINK _get_somtIsSelfRef(SOMTDataEntryC SOMSTAR somSelf)
{
  SOMTDataEntryCData *somThis = SOMTDataEntryCGetData(somSelf);
  SOMTDataEntryCMethodDebug("SOMTDataEntryC","_get_somtIsSelfRef");

  return FALSE;
}
