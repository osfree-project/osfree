/**************************************************************************
 *
 *  Copyright 2015, Yuri Prokushev
 *
 *  This file is part of osFree project
 *
 *  This program is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License as published by the
 *  Free Software Foundation, either version 2 of the License, or (at your
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
  return NULL;
}

SOM_Scope boolean SOMLINK _get_somtIsVarargs(SOMTMethodEntryC * somSelf)
{
  return NULL;
}

SOM_Scope SOMTParameterEntryC * SOMLINK somtGetNextParameter(SOMTMethodEntryC * somSelf)
{
  return NULL;
}

SOM_Scope SOMTEntryC * SOMLINK _get_somtMethodGroup(SOMTMethodEntryC * somSelf)
{
  return NULL;
}

SOM_Scope boolean SOMLINK _get_somtIsOneway(SOMTMethodEntryC * somSelf)
{
  return FALSE;
}

SOM_Scope SOMTParameterEntryC * SOMLINK somtGetNthParameter(SOMTMethodEntryC * somSelf,
                                                            /* in */ short n)
{
  return NULL;
}

SOM_Scope string SOMLINK somtGetShortParamNameList(SOMTMethodEntryC * somSelf,
                                                   /* in */ string buffer,
                                                   /* in */ string selfParm,
                                                   /* in */ string varargsParm)
{
  return NULL;
}

SOM_Scope string SOMLINK somtGetIDLParamList(SOMTMethodEntryC * somSelf,
                                             /* in */ string buffer)
{
  return NULL;
}

SOM_Scope string *SOMLINK _get_somtContextArray(SOMTMethodEntryC * somSelf)
{
  return NULL;
}

SOM_Scope SOMTParameterEntryC * SOMLINK somtGetFirstParameter(SOMTMethodEntryC * somSelf)
{
  return NULL;
}

SOM_Scope long SOMLINK somtSetSymbolsOnEntry(SOMTMethodEntryC * somSelf,
                                             /* in */ SOMTEmitC * emitter,
                                             /* in */ string prefix)
{
  return 0;
}

SOM_Scope string SOMLINK somtGetFullCParamList(SOMTMethodEntryC * somSelf,
                                               /* in */ string buffer,
                                               /* in */ string varargsParm)
{
  return NULL;
}

SOM_Scope void SOMLINK somDumpSelfInt(SOMTMethodEntryC * somSelf,
                                      /* in */ long level)
{

}

SOM_Scope string SOMLINK somtGetShortCParamList(SOMTMethodEntryC * somSelf,
                                                /* in */ string buffer,
                                                /* in */ string selfParm,
                                                /* in */ string varargsParm)
{
  return NULL;
}

SOM_Scope boolean SOMLINK _get_somtIsPrivateMethod(SOMTMethodEntryC * somSelf)
{
  return FALSE;
}

SOM_Scope SOMTStructEntryC * SOMLINK somtGetNextException(SOMTMethodEntryC * somSelf)
{
  return NULL;
}

SOM_Scope short SOMLINK _get_somtArgCount(SOMTMethodEntryC * somSelf)
{
  return 0;
}

SOM_Scope string SOMLINK _get_somtCReturnType(SOMTMethodEntryC * somSelf)
{
  return NULL;
}

SOM_Scope SOMTStructEntryC * SOMLINK somtGetFirstException(SOMTMethodEntryC * somSelf)
{
  return NULL;
}

SOM_Scope SOMTClassEntryC * SOMLINK _get_somtOriginalClass(SOMTMethodEntryC * somSelf)
{
  return NULL;
}

SOM_Scope string SOMLINK somtGetFullParamNameList(SOMTMethodEntryC * somSelf,
                                                  /* in */ string buffer,
                                                  /* in */ string varargsParm)
{
  return NULL;
}

