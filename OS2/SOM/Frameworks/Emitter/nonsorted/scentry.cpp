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

#ifndef SOM_Module_Source
#define SOM_Module_Source
#endif

#define SOMTEntryC_Class_Source

#define __PRIVATE__
#include <rhbopt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "rhbsc.h"


#include "scentry.xih"

SOM_Scope string  SOMLINK _get_somtEntryName(SOMTEntryC SOMSTAR somSelf)
{
  SOMTEntryCData *somThis = SOMTEntryCGetData(somSelf);
  SOMTEntryCMethodDebug("SOMTEntryC","_get_somtEntryName");
  
  return _somtEntryName;
}


SOM_Scope string SOMLINK _get_somtCScopedName(SOMTEntryC SOMSTAR somSelf)
{
  SOMTEntryCData *somThis = SOMTEntryCGetData(somSelf);
  SOMTEntryCMethodDebug("SOMTEntryC","_get_somtCScopedName");

  return _CScopedName;  // <modulename>_<interfacename>
}

SOM_Scope string SOMLINK _get_somtIDLScopedName(SOMTEntryC SOMSTAR somSelf)
{
  SOMTEntryCData *somThis = SOMTEntryCGetData(somSelf);
  SOMTEntryCMethodDebug("SOMTEntryC","_get_somtIDLScopedName");

  return _IDLScopedName;
}

SOM_Scope unsigned long SOMLINK _get_somtSourceLineNumber(SOMTEntryC SOMSTAR somSelf)
{
  SOMTEntryCData *somThis = SOMTEntryCGetData(somSelf);
  SOMTEntryCMethodDebug("SOMTEntryC","_get_somtSourceLineNumber");

  RHBelement * element=_es->data;
  
  return element->defined_line;
}

SOM_Scope TypeCode SOMLINK _get_somtTypeCode(SOMTEntryC SOMSTAR somSelf)
{
  SOMTEntryCData *somThis = SOMTEntryCGetData(somSelf);
  SOMTEntryCMethodDebug("SOMTEntryC","_get_somtTypeCode");

  return 0;
}

SOM_Scope string SOMLINK _get_somtEntryComment(SOMTEntryC SOMSTAR somSelf)
{
  SOMTEntryCData *somThis = SOMTEntryCGetData(somSelf);
  SOMTEntryCMethodDebug("SOMTEntryC","_get_somtEntryComment");

  return NULL;
}

SOM_Scope string SOMLINK _get_somtElementTypeName(SOMTEntryC SOMSTAR somSelf)
{
  SOMTEntryCData *somThis = SOMTEntryCGetData(somSelf);
  SOMTEntryCMethodDebug("SOMTEntryC","_get_somtElementTypeName");

  return NULL;
}

SOM_Scope boolean SOMLINK _get_somtIsReference(SOMTEntryC SOMSTAR somSelf)
{
  SOMTEntryCData *somThis = SOMTEntryCGetData(somSelf);
  SOMTEntryCMethodDebug("SOMTEntryC","_get_somtIsReference");

  return FALSE;
}

SOM_Scope long SOMLINK somtSetSymbolsOnEntry(SOMTEntryC SOMSTAR somSelf,
	                                                   /* in */ SOMTEmitC SOMSTAR emitter,
	                                                   /* in */ string prefix)
{
  char buf[250];
  char modbuf[25000];

  SOMTEntryCData *somThis = SOMTEntryCGetData(somSelf);
  SOMTEntryCMethodDebug("SOMTEntryC","somtSetSymbolsOnEntry");
  SOMTTemplateOutputC *templ = emitter->_get_somtTemplate();

  buf[0]=0;
  strcat(buf, prefix);
  templ->somtSetSymbolCopyBoth(strcat(buf, "Name"), somSelf->_get_somtEntryName());

  buf[0]=0;
  strcat(buf, prefix);
  templ->somtSetSymbolCopyBoth(strcat(buf, "IDSScopedName"), _IDLScopedName);

  buf[0]=0;
  strcat(buf, prefix);
  templ->somtSetSymbolCopyBoth(strcat(buf, "CScopedName"), _CScopedName);

  buf[0]=0;
  strcat(buf, prefix);
  templ->somtSetSymbolCopyBoth(strcat(buf, "Comment"), somSelf->_get_somtEntryComment());

  buf[0]=0;
  char buf2[20];
  strcat(buf, prefix);
  sprintf(buf2, "%d", somSelf->_get_somtSourceLineNumber());
  templ->somtSetSymbolCopyBoth(strcat(buf, "LineNumber"), buf2);

  buf[0]=0;
  modbuf[0]=0;
  strcat(buf, prefix);
  somSelf->somtGetModifierList(modbuf);
  templ->somtSetSymbolCopyBoth(strcat(buf, "Mods"), modbuf);
  
  return 1;
}

void get_c_name(RHBelement *element,char *buf,size_t buflen)
{
	if (element->parent())
	{
		get_c_name(element->parent(),buf,buflen);
		if (buf[0])
		{
			strncat(buf,"_",buflen);
		}
		strncat(buf,element->id,buflen);
	}
	else
	{
		if (element->id)
		{
			strncpy(buf,element->id,buflen);
		}
		else
		{
			buf[0]=0;
		}
	}
}

void get_ir_name(RHBelement *element,char *buf,size_t buflen)
{
	if (element->parent())
	{
		get_ir_name(element->parent(),buf,buflen);
		if (strcmp(buf,"::"))
		{
			strncat(buf,"::",buflen);
		}
		strncat(buf,element->id,buflen);
	}
	else
	{
		strncpy(buf,"::",buflen);
		if (element->id)
		{
			strncat(buf,element->id,buflen);
		}
	}
}

SOM_Scope void SOMLINK somtSetEntryStruct(SOMTEntryC SOMSTAR somSelf,
	                                                /* inout */ Entry *es)
{
  char buf[250];
  SOMTEntryCData *somThis = SOMTEntryCGetData(somSelf);
  SOMTEntryCMethodDebug("SOMTEntryC","somtSetEntryStruct");

  _es=es;

  RHBelement * element=_es->data;

// Store names for future usage

  _somtEntryName=es->name;
  get_c_name(element, buf, sizeof(buf));
  _CScopedName=(string) SOMMalloc(strlen(buf)+1);
  strncpy(_CScopedName, buf, strlen(buf));
  
  memset(buf, 0, sizeof(buf));
  get_ir_name(element, buf, sizeof(buf));
  _IDLScopedName=(string) SOMMalloc(strlen(buf)+1);
  strncpy(_IDLScopedName, buf, strlen(buf));

// Store modifiers in associative array
 
  char name[250];
  char value[2048];
  
 	RHBelement *e=element->children();
	while (e)
	{
    RHBmodifier *m=e->is_modifier();
    if (m) 
    {
			int i=0;
      
      name[0]=0;
      value[0]=0;
      
      strcat(name, m->id);

			while (m->modifier_data.get(i))
			{
        strcat(value, m->modifier_data.get(i));
				i++;
      }

      _Stab->somstAssociateCopyBoth(name, value);      
    }
		e=e->next();
	}

}

SOM_Scope long SOMLINK somtFormatModifier(SOMTEntryC SOMSTAR somSelf,
	                                                /* in */ string buffer,
	                                                /* in */ string name,
	                                                /* in */ string value)
{
  SOMTEntryCData *somThis = SOMTEntryCGetData(somSelf);
  SOMTEntryCMethodDebug("SOMTEntryC","somtFormatModifier");

  strcat(buffer, name);
  strcat(buffer, ": ");
  strcat(buffer, value);
  return strlen(buffer);
}

SOM_Scope long SOMLINK somtGetModifierList(SOMTEntryC SOMSTAR somSelf,
                                           /* in */ string buffer)
{
  char str[4096];
  
  SOMTEntryCData *somThis = SOMTEntryCGetData(somSelf);
  SOMTEntryCMethodDebug("SOMTEntryC","somtGetModifierList");
 
  unsigned long i=1;
  
  while(_Stab->somstGetIthKey(i))
  {
    str[0]=0;
    somSelf->somtFormatModifier(str, _Stab->somstGetIthKey(i), _Stab->somstGetIthValue(i));
    
    strcat(buffer, str);
    strcat(buffer, "\n");
    i++;
  }
 
  return i-1;
}

SOM_Scope boolean SOMLINK somtGetNextModifier(SOMTEntryC SOMSTAR somSelf,
	                                                    /* inout */ string *modifierName,
	                                                    /* inout */ string *modifierValue)
{
  SOMTEntryCData *somThis = SOMTEntryCGetData(somSelf);
  SOMTEntryCMethodDebug("SOMTEntryC","somtGetNextModifier");

  _iterator++;
  if (!_Stab->somstGetIthKey(_iterator)) return FALSE;
  
  *modifierName=_Stab->somstGetIthKey(_iterator);
  *modifierValue=_Stab->somstGetIthValue(_iterator);
  return TRUE;
}

SOM_Scope string SOMLINK somtGetModifierValue(SOMTEntryC SOMSTAR somSelf,
	                                                    /* in */ string modifierName)
{
  SOMTEntryCData *somThis = SOMTEntryCGetData(somSelf);
  SOMTEntryCMethodDebug("SOMTEntryC","somtGetModifierValue");

  return _Stab->somstGetAssociation(modifierName);
}

SOM_Scope boolean SOMLINK somtGetFirstModifier(SOMTEntryC SOMSTAR somSelf,
	                                                     /* inout */ string *modifierName,
	                                                     /* inout */ string *modifierValue)
{
  SOMTEntryCData *somThis = SOMTEntryCGetData(somSelf);
  SOMTEntryCMethodDebug("SOMTEntryC","somtGetFirstModifier");

  if (!_Stab->somstGetIthKey(1)) return FALSE;
  
  *modifierName=_Stab->somstGetIthKey(1);
  *modifierValue=_Stab->somstGetIthValue(1);
  _iterator=1;
  return TRUE;
}

SOM_Scope Entry *SOMLINK _get_somtEntryStruct(SOMTEntryC * somSelf)
{
  SOMTEntryCData *somThis = SOMTEntryCGetData(somSelf);
  SOMTEntryCMethodDebug("SOMTEntryC","_get_somtEntryStruct");

  return _es;
}

SOM_Scope void SOMLINK somDefaultInit(SOMTEntryC SOMSTAR somSelf, somInitCtrl *ctrl)
{
  SOMTEntryCData *somThis; // set by BeginInitializer 
  somInitCtrl globalCtrl;
  somBooleanVector myMask;
  SOMTEntryCMethodDebug("SOMTEntryC", "somDefaultInit");
  
  SOMTEntryC_BeginInitializer_somDefaultInit;
  SOMTEntryC_Init_SOMObject_somDefaultInit(somSelf, ctrl);

  _CScopedName=NULL;
  _IDLScopedName=NULL;
  _Stab = new SOMStringTableC();
  _iterator=0;
}

SOM_Scope void SOMLINK somDestruct(SOMTEntryC *somSelf, octet doFree, somDestructCtrl* ctrl)
{
  SOMTEntryCData *somThis; /* set by BeginDestructor */
  somDestructCtrl globalCtrl;
  somBooleanVector myMask;
  SOMTEntryCMethodDebug("SOMTEntryC","somDestruct");
  SOMTEntryC_BeginDestructor;
  /*
  * local SOMTEntryC deinitialization code added by programmer
  */
  if (_CScopedName) SOMFree(_CScopedName);
  if (_IDLScopedName) SOMFree(_IDLScopedName);
  delete _Stab;
  
  SOMTEntryC_EndDestructor;
}

// ??? Как быть с такими вещами? Рассинхронизимся...
const char * RHBmodifier_list::get(int i)
{
		if (i < _length)
		{
			return _buffer[i];
		}
		return 0;
}
