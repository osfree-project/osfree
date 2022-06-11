#ifndef sctypes_h
#define sctypes_h

#ifdef __SOMIDL__
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
    interface SOMTUserDefinedTypeEntryC;
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

#ifdef SOM_STRICT_IDL
SOMEXTERN SOMTEntryC SOMLINK somtGetObjectWrapper(Entry *ep);
SOMEXTERN void SOMLINK printEntry(SOMTEntryC entry, int level);
SOMEXTERN void SOMLINK dumpEntry(SOMTEntryC entry, int level);
#else
SOMEXTERN SOMTEntryC * SOMLINK somtGetObjectWrapper(Entry * ep);
SOMEXTERN void SOMLINK printEntry(SOMTEntryC * entry, int level);
SOMEXTERN void SOMLINK dumpEntry(SOMTEntryC * entry, int level);
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
