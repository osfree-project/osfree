/**************************************************************************
 *
 *  Copyright 2022, Yuri Prokushev
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

#ifndef sctypes_h
#define sctypes_h

#ifdef __SOMIDL__
/*
    interface SOMTEntryC;
    interface SOMTEmitC;
    interface SOMTMetaClassEntryC;
    interface SOMTAttributeEntryC;
    interface SOMTTypedefEntryC;
    interface SOMTBaseClassEntryC;
    interface SOMTPassthruEntryC;
    interface SOMTDataEntryC;
    interface SOMTMethodEntryC;
    interface SOMTClassEntryC;
    interface SOMTModuleEntryC;
    interface SOMTParameterEntryC;
    interface SOMTStructEntryC;
    interface SOMTUnionEntryC;
    interface SOMTEnumEntryC;
    interface SOMTConstEntryC;
    interface SOMTSequenceEntryC;
    interface SOMTStringEntryC;
    interface SOMTEnumNameEntryC;
    interface SOMTCommonEntryC;
    interface SOMTUserDefinedTypeEntryC;*/
#else
    #ifdef __cplusplus
    #undef __STR__
    #endif
    #include <stdio.h>
    #include <string.h>
    #include <som.h>
#endif  /* __SOMIDL__ */

#include <sm.h>

#define SOMT_MAX_STRING_LENGTH 10000
#define SOMT_MAX_SMALL_STRING 1000

enum SOMTTargetTypeT {
    somtPrivateE,
    somtPublicE,
    somtImplementationE,
    somtAllE
};

#ifdef __SOMIDL__
typedef unsigned long SOMTTypes;
#else
typedef enum SOMTTargetTypeT SOMTTargetTypeT;
#endif


#ifndef __SOMIDL__

#if defined(_OS2) && defined(__IBMC__)
#pragma linkage(somtGetObjectWrapper, 	system)
#pragma linkage(printEntry, 		system)
#pragma linkage(dumpEntry, 		system)
#pragma linkage(somtShowEntry, 		system)
#pragma linkage(somtStrDup, 		system)
#pragma linkage(somtStrCat, 		system)
#pragma linkage(somtMakeIncludeStr, 	system)
#pragma linkage(somtNewSymbol, 		system)
#pragma linkage(somtGetFileStem, 	system)
#pragma linkage(somtEntryTypeName, 	system)
#endif

#ifdef SOM_STRICT_IDL
//SOMEXTERN void SOMLINK printEntry(SOMTEntryC entry, int level);
//SOMEXTERN void SOMLINK dumpEntry(SOMTEntryC entry, int level);
#else
//SOMEXTERN void SOMLINK printEntry(SOMTEntryC * entry, int level);
//SOMEXTERN void SOMLINK dumpEntry(SOMTEntryC * entry, int level);
#endif
SOMEXTERN void SOMLINK somtShowEntry(Entry * ep);
SOMEXTERN char * SOMLINK somtStrDup(char *str);
SOMEXTERN char * SOMLINK somtStrCat(int count,...);
SOMEXTERN char * SOMLINK somtMakeIncludeStr(boolean local, char *stem, char *suffix);
SOMEXTERN char * SOMLINK somtNewSymbol(char *prefix, char *stem);
SOMEXTERN char * SOMLINK somtGetFileStem(char *fullName);
SOMEXTERN char * SOMLINK somtEntryTypeName(SOMTTypes type);

#endif /* __SOMIDL__ */


#endif /* sctypes_h */
