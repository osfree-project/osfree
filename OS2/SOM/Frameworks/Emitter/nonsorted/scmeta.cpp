/**************************************************************************
 *
 *  Copyright 2014, 2017 Yuri Prokushev
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

#ifndef SOM_Module_scmeta_Source
#define SOM_Module_scmeta_Source
#endif

#define SOMTMetaClassEntryC_Class_Source

#include "scmeta.xih"

SOM_Scope SOMTClassEntryC * SOMLINK _get_somtMetaClassDef(SOMTMetaClassEntryC * somSelf)
{
  SOMTMetaClassEntryCData *somThis = SOMTMetaClassEntryCGetData(somSelf);
  SOMTMetaClassEntryCMethodDebug("SOMTMetaClassEntryC","_get_somtMetaClassDef");
  
  return NULL;
}

SOM_Scope string SOMLINK _get_somtMetaFile(SOMTMetaClassEntryC * somSelf)
{
  SOMTMetaClassEntryCData *somThis = SOMTMetaClassEntryCGetData(somSelf);
  SOMTMetaClassEntryCMethodDebug("SOMTMetaClassEntryC","_get_somtMetaFile");
  
  return NULL;
}
