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

#ifndef SOM_Module_scmethod_Source
#define SOM_Module_scmethod_Source
#endif

#define SOMTMethodEntryC_Class_Source

#include "scmethod.xih"

SOM_Scope SOMTMethodEntryC * SOMLINK _get_somtOriginalMethod(SOMTMethodEntryC * somSelf)
{
  SOMTMethodEntryCData *somThis = SOMTMethodEntryCGetData(somSelf);
  SOMTMethodEntryCMethodDebug("SOMTMethodEntryC","_get_somtOriginalMethod");

  return NULL;
}

SOM_Scope boolean SOMLINK _get_somtIsVarargs(SOMTMethodEntryC * somSelf)
{
  SOMTMethodEntryCData *somThis = SOMTMethodEntryCGetData(somSelf);
  SOMTMethodEntryCMethodDebug("SOMTMethodEntryC","_get_somtIsVarargs");

  return NULL;
}

SOM_Scope SOMTParameterEntryC * SOMLINK somtGetNextParameter(SOMTMethodEntryC * somSelf)
{
  SOMTMethodEntryCData *somThis = SOMTMethodEntryCGetData(somSelf);
  SOMTMethodEntryCMethodDebug("SOMTMethodEntryC","somtGetNextParameter");

  return NULL;
}

SOM_Scope SOMTEntryC * SOMLINK _get_somtMethodGroup(SOMTMethodEntryC * somSelf)
{
  SOMTMethodEntryCData *somThis = SOMTMethodEntryCGetData(somSelf);
  SOMTMethodEntryCMethodDebug("SOMTMethodEntryC","_get_somtMethodGroup");

  return NULL;
}

SOM_Scope boolean SOMLINK _get_somtIsOneway(SOMTMethodEntryC * somSelf)
{
  SOMTMethodEntryCData *somThis = SOMTMethodEntryCGetData(somSelf);
  SOMTMethodEntryCMethodDebug("SOMTMethodEntryC","_get_somtIsOneway");

  return FALSE;
}

SOM_Scope SOMTParameterEntryC * SOMLINK somtGetNthParameter(SOMTMethodEntryC * somSelf,
                                                            /* in */ short n)
{
  SOMTMethodEntryCData *somThis = SOMTMethodEntryCGetData(somSelf);
  SOMTMethodEntryCMethodDebug("SOMTMethodEntryC","somtGetNthParameter");

  return NULL;
}

SOM_Scope string SOMLINK somtGetShortParamNameList(SOMTMethodEntryC * somSelf,
                                                   /* in */ string buffer,
                                                   /* in */ string selfParm,
                                                   /* in */ string varargsParm)
{
  SOMTMethodEntryCData *somThis = SOMTMethodEntryCGetData(somSelf);
  SOMTMethodEntryCMethodDebug("SOMTMethodEntryC","somtGetShortParamNameList");

  return NULL;
}

SOM_Scope string SOMLINK somtGetIDLParamList(SOMTMethodEntryC * somSelf,
                                             /* in */ string buffer)
{
  SOMTMethodEntryCData *somThis = SOMTMethodEntryCGetData(somSelf);
  SOMTMethodEntryCMethodDebug("SOMTMethodEntryC","somtGetIDLParamList");

  return NULL;
}

SOM_Scope string *SOMLINK _get_somtContextArray(SOMTMethodEntryC * somSelf)
{
  SOMTMethodEntryCData *somThis = SOMTMethodEntryCGetData(somSelf);
  SOMTMethodEntryCMethodDebug("SOMTMethodEntryC","_get_somtContextArray");

  return NULL;
}

SOM_Scope SOMTParameterEntryC * SOMLINK somtGetFirstParameter(SOMTMethodEntryC * somSelf)
{
  SOMTMethodEntryCData *somThis = SOMTMethodEntryCGetData(somSelf);
  SOMTMethodEntryCMethodDebug("SOMTMethodEntryC","somtGetFirstParameter");

  return NULL;
}

SOM_Scope long SOMLINK somtSetSymbolsOnEntry(SOMTMethodEntryC * somSelf,
                                             /* in */ SOMTEmitC * emitter,
                                             /* in */ string prefix)
{
  SOMTMethodEntryCData *somThis = SOMTMethodEntryCGetData(somSelf);
  SOMTMethodEntryCMethodDebug("SOMTMethodEntryC","somtSetSymbolsOnEntry");

  return 0;
}

SOM_Scope string SOMLINK somtGetFullCParamList(SOMTMethodEntryC * somSelf,
                                               /* in */ string buffer,
                                               /* in */ string varargsParm)
{
  SOMTMethodEntryCData *somThis = SOMTMethodEntryCGetData(somSelf);
  SOMTMethodEntryCMethodDebug("SOMTMethodEntryC","somtGetFullCParamList");

  return NULL;
}

SOM_Scope void SOMLINK somDumpSelfInt(SOMTMethodEntryC * somSelf,
                                      /* in */ long level)
{
  SOMTMethodEntryCData *somThis = SOMTMethodEntryCGetData(somSelf);
  SOMTMethodEntryCMethodDebug("SOMTMethodEntryC","somDumpSelfInt");


}

SOM_Scope string SOMLINK somtGetShortCParamList(SOMTMethodEntryC * somSelf,
                                                /* in */ string buffer,
                                                /* in */ string selfParm,
                                                /* in */ string varargsParm)
{
  SOMTMethodEntryCData *somThis = SOMTMethodEntryCGetData(somSelf);
  SOMTMethodEntryCMethodDebug("SOMTMethodEntryC","somtGetShortCParamList");

  return NULL;
}

SOM_Scope boolean SOMLINK _get_somtIsPrivateMethod(SOMTMethodEntryC * somSelf)
{
  SOMTMethodEntryCData *somThis = SOMTMethodEntryCGetData(somSelf);
  SOMTMethodEntryCMethodDebug("SOMTMethodEntryC","_get_somtIsPrivateMethod");

  return FALSE;
}

SOM_Scope SOMTStructEntryC * SOMLINK somtGetNextException(SOMTMethodEntryC * somSelf)
{
  SOMTMethodEntryCData *somThis = SOMTMethodEntryCGetData(somSelf);
  SOMTMethodEntryCMethodDebug("SOMTMethodEntryC","somtGetNextException");

  return NULL;
}

SOM_Scope short SOMLINK _get_somtArgCount(SOMTMethodEntryC * somSelf)
{
  SOMTMethodEntryCData *somThis = SOMTMethodEntryCGetData(somSelf);
  SOMTMethodEntryCMethodDebug("SOMTMethodEntryC","_get_somtArgCount");

  return 0;
}

SOM_Scope string SOMLINK _get_somtCReturnType(SOMTMethodEntryC * somSelf)
{
  SOMTMethodEntryCData *somThis = SOMTMethodEntryCGetData(somSelf);
  SOMTMethodEntryCMethodDebug("SOMTMethodEntryC","_get_somtCReturnType");

  return NULL;
}

SOM_Scope SOMTStructEntryC * SOMLINK somtGetFirstException(SOMTMethodEntryC * somSelf)
{
  SOMTMethodEntryCData *somThis = SOMTMethodEntryCGetData(somSelf);
  SOMTMethodEntryCMethodDebug("SOMTMethodEntryC","somtGetFirstException");

  return NULL;
}

SOM_Scope SOMTClassEntryC * SOMLINK _get_somtOriginalClass(SOMTMethodEntryC * somSelf)
{
  SOMTMethodEntryCData *somThis = SOMTMethodEntryCGetData(somSelf);
  SOMTMethodEntryCMethodDebug("SOMTMethodEntryC","_get_somtOriginalClass");

  return NULL;
}

SOM_Scope string SOMLINK somtGetFullParamNameList(SOMTMethodEntryC * somSelf,
                                                  /* in */ string buffer,
                                                  /* in */ string varargsParm)
{
  SOMTMethodEntryCData *somThis = SOMTMethodEntryCGetData(somSelf);
  SOMTMethodEntryCMethodDebug("SOMTMethodEntryC","somtGetFullParamNameList");

  return NULL;
}

