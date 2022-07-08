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

#ifndef SOM_Module_scstring_Source
#define SOM_Module_scstring_Source
#endif

#define SOMTStringEntryC_Class_Source

#define __PRIVATE__
#include <rhbopt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "rhbsc.h"

#include "scstring.xih"

SOM_Scope long SOMLINK somtSetSymbolsOnEntry(SOMTStringEntryC SOMSTAR somSelf,
	                                           /* in */ SOMTEmitC SOMSTAR emitter,
	                                           /* in */ string prefix)
{
  char buf[250];
  
  SOMTStringEntryCData *somThis = SOMTStringEntryCGetData(somSelf);
  SOMTStringEntryCMethodDebug("SOMTStringEntryC","somtSetSymbolsOnEntry");
  
SOMTTemplateOutputC *templ = emitter->_get_somtTemplate();

  SOMTStringEntryC_parent_SOMTEntryC_somtSetSymbolsOnEntry(somSelf, emitter, prefix);
//  RHBattribute *c=somSelf->_get_somtEntryStruct()->data->is_attribute();
  
//  RHBtype *t=c->attribute_type->is_base_type();

//  buf[0]=0;
//  strcat(buf, prefix);
//  templ->somtSetSymbolCopyBoth(strcat(buf, "BaseType"), c->attribute_type->id);

// здесь надо отработать деклараторы  
  return 1;
}


SOM_Scope void SOMLINK somDumpSelfInt(SOMTStringEntryC * somSelf,
                                      /* in */ long level)
{
  SOMTStringEntryCData *somThis = SOMTStringEntryCGetData(somSelf);
  SOMTStringEntryCMethodDebug("SOMTStringEntryC","somDumpSelfInt");


}

SOM_Scope long SOMLINK _get_somtStringLength(SOMTStringEntryC * somSelf)
{
  SOMTStringEntryCData *somThis = SOMTStringEntryCGetData(somSelf);
  SOMTStringEntryCMethodDebug("SOMTStringEntryC","_get_somtStringLength");
 
  return 0; 
}
