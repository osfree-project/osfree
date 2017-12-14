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

#ifndef SOM_Module_scconst_Source
#define SOM_Module_scconst_Source
#endif

#define SOMTConstEntryC_Class_Source

#define __PRIVATE__
#include <rhbopt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "rhbsc.h"

#include "scconst.xih"

SOM_Scope long SOMLINK somtSetSymbolsOnEntry(SOMTConstEntryC SOMSTAR somSelf,
	                                           /* in */ SOMTEmitC SOMSTAR emitter,
	                                           /* in */ string prefix)
{
  char buf[250];
  
  SOMTConstEntryCData *somThis = SOMTConstEntryCGetData(somSelf);
  SOMTConstEntryCMethodDebug("SOMTConstEntryC","somtSetSymbolsOnEntry");

  SOMTTemplateOutputC *templ = emitter->_get_somtTemplate();

  SOMTConstEntryC_parent_SOMTEntryC_somtSetSymbolsOnEntry(somSelf, emitter, prefix);
  RHBconstant *c=somSelf->_get_somtEntryStruct()->data->is_constant();
  
  buf[0]=0;
  strcat(buf, prefix);
  templ->somtSetSymbolCopyBoth(strcat(buf, "Type"), c->constant_type->id);
  
  buf[0]=0;
  strcat(buf, prefix);
  if (!(c->value_string))
  {
    templ->somtSetSymbolCopyBoth(strcat(buf, "ValueEvaluated"), c->const_val->value_string);
  } else {
    templ->somtSetSymbolCopyBoth(strcat(buf, "ValueEvaluated"), c->value_string);
  }
  return 1;
}

SOM_Scope boolean SOMLINK _get_somtConstIsNegative(SOMTConstEntryC SOMSTAR somSelf)
{
  return FALSE;
}

SOM_Scope unsigned long SOMLINK _get_somtConstNumVal(SOMTConstEntryC SOMSTAR somSelf)
{
  return 0;
}

SOM_Scope SOMTEntryC SOMSTAR SOMLINK _get_somtConstTypeObj(SOMTConstEntryC SOMSTAR somSelf)
{
  return FALSE;
}

SOM_Scope string SOMLINK _get_somtConstStringVal(SOMTConstEntryC SOMSTAR somSelf)
{
  return NULL;
}

SOM_Scope long SOMLINK _get_somtConstNumNegVal(SOMTConstEntryC SOMSTAR somSelf)
{
  return 0;
}

SOM_Scope string SOMLINK _get_somtConstVal(SOMTConstEntryC SOMSTAR somSelf)
{
  return NULL;
}

SOM_Scope string SOMLINK _get_somtConstType(SOMTConstEntryC SOMSTAR somSelf)
{
  return NULL;
}
