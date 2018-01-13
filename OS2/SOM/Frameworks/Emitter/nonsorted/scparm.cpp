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

#ifndef SOM_Module_scparm_Source
#define SOM_Module_scparm_Source
#endif

#define SOMTParameterEntryC_Class_Source

#define __PRIVATE__
#include <rhbopt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "rhbsc.h"

#include "scparm.xih"

SOM_Scope long SOMLINK somtSetSymbolsOnEntry(SOMTParameterEntryC SOMSTAR somSelf,
	                                           /* in */ SOMTEmitC SOMSTAR emitter,
	                                           /* in */ string prefix)
{
  char buf[250];
  
  SOMTParameterEntryCData *somThis = SOMTParameterEntryCGetData(somSelf);
  SOMTParameterEntryCMethodDebug("SOMTParameterEntryC","somtSetSymbolsOnEntry");
  
  SOMTTemplateOutputC *templ = emitter->_get_somtTemplate();

  SOMTParameterEntryC_parent_SOMTCommonEntryC_somtSetSymbolsOnEntry(somSelf, emitter, prefix);
//  RHBattribute *c=somSelf->_get_somtEntryStruct()->data->is_attribute();
  
//  RHBtype *t=c->attribute_type->is_base_type();

//  buf[0]=0;
//  strcat(buf, prefix);
//  templ->somtSetSymbolCopyBoth(strcat(buf, "BaseType"), c->attribute_type->id);

// здесь надо отработать деклараторы  
  return 1;
}


SOM_Scope void SOMLINK somDumpSelfInt(SOMTParameterEntryC * somSelf,
                                      /* in */ long level)
{
  SOMTParameterEntryCData *somThis = SOMTParameterEntryCGetData(somSelf);
  SOMTParameterEntryCMethodDebug("SOMTParameterEntryC","somDumpSelfInt");


}

SOM_Scope string SOMLINK _get_somtCParameterDeclaration(SOMTParameterEntryC SOMSTAR somSelf)
{
  return NULL;
}

SOM_Scope string SOMLINK _get_somtIDLParameterDeclaration(SOMTParameterEntryC SOMSTAR somSelf)
{
  return NULL;
}

SOM_Scope string SOMLINK _get_somtPascalParameterDeclaration(SOMTParameterEntryC SOMSTAR somSelf)
{
  return NULL;
}

SOM_Scope somtParameterDirectionT SOMLINK _get_somtParameterDirection(SOMTParameterEntryC SOMSTAR somSelf)
{
  return somtInOutE;  
}
