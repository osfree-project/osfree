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

#ifndef SOM_Module_scattrib_Source
#define SOM_Module_scattrib_Source
#endif

#define SOMTAttributeEntryC_Class_Source

#define __PRIVATE__
#include <rhbopt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "rhbsc.h"

#include "scattrib.xih"

SOM_Scope long SOMLINK somtSetSymbolsOnEntry(SOMTAttributeEntryC SOMSTAR somSelf,
	                                           /* in */ SOMTEmitC SOMSTAR emitter,
	                                           /* in */ string prefix)
{
  char buf[250];
  
  SOMTAttributeEntryCData *somThis = SOMTAttributeEntryCGetData(somSelf);
  SOMTAttributeEntryCMethodDebug("SOMTAttributeEntryC","somtSetSymbolsOnEntry");
  

//attributeDeclarators
  SOMTTemplateOutputC *templ = emitter->_get_somtTemplate();

  SOMTAttributeEntryC_parent_SOMTEntryC_somtSetSymbolsOnEntry(somSelf, emitter, prefix);
  RHBattribute *c=somSelf->_get_somtEntryStruct()->data->is_attribute();
  /*
	RHBtype *is_type();
	void build(RHBPreprocessor *pp);
	virtual RHBenum *is_enum();
	virtual RHBstruct *is_struct();
	virtual RHBtype_def *is_typedef();
	virtual RHBstruct_element *is_struct_element();
	virtual RHBarray *is_array();
	virtual RHBpointer_type * is_pointer();
	virtual RHBstring_type *is_string();
	virtual RHBqualified_type *is_qualified();
	virtual RHBsequence_type *is_sequence();
	virtual RHBunion *is_union();
	virtual RHBunion_element *is_union_element();
	virtual RHBbase_type *is_base_type();
	virtual RHBany_type *is_any();
	virtual RHBTypeCode_type *is_TypeCode();
*/
  
  RHBtype *t=c->attribute_type->is_base_type();
    if (t) 
    {
      printf("111\n");
    } else {
      printf("222\n");
      
    }

  buf[0]=0;
  strcat(buf, prefix);
  templ->somtSetSymbolCopyBoth(strcat(buf, "BaseType"), c->attribute_type->id);
  printf("666\n");

// здесь надо отработать деклараторы  
  return 1;
}

SOM_Scope SOMTMethodEntryC SOMSTAR SOMLINK somtGetFirstSetMethod(SOMTAttributeEntryC SOMSTAR somSelf)
{
  return NULL;
}

SOM_Scope SOMTEntryC SOMSTAR SOMLINK _get_somtAttribType(SOMTAttributeEntryC SOMSTAR somSelf)
{
  return NULL;
}

SOM_Scope SOMTMethodEntryC SOMSTAR SOMLINK somtGetFirstGetMethod(SOMTAttributeEntryC SOMSTAR somSelf)
{
  return NULL;
}

SOM_Scope SOMTDataEntryC SOMSTAR SOMLINK somtGetFirstAttributeDeclarator(SOMTAttributeEntryC SOMSTAR somSelf)
{
  return NULL;
}

SOM_Scope SOMTMethodEntryC SOMSTAR SOMLINK somtGetNextGetMethod(SOMTAttributeEntryC SOMSTAR somSelf)
{
  return NULL;
}

SOM_Scope SOMTDataEntryC SOMSTAR SOMLINK somtGetNextAttributeDeclarator(SOMTAttributeEntryC SOMSTAR somSelf)
{
  return NULL;
}

SOM_Scope boolean SOMLINK _get_somtIsReadonly(SOMTAttributeEntryC SOMSTAR somSelf)
{
  SOMTAttributeEntryCData *somThis = SOMTAttributeEntryCGetData(somSelf);
  SOMTAttributeEntryCMethodDebug("SOMTAttributeEntryC","_get_somtIsReadonly");
  
  RHBattribute *c=somSelf->_get_somtEntryStruct()->data->is_attribute();
  
  return c->readonly;
}

SOM_Scope SOMTMethodEntryC SOMSTAR SOMLINK somtGetNextSetMethod(SOMTAttributeEntryC SOMSTAR somSelf)
{
  return NULL;
}
