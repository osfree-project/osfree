/* generated from sccommon.idl */
/* internal conditional is SOM_Module_sccommon_Source */
#ifndef SOM_Module_sccommon_Header_h
	#define SOM_Module_sccommon_Header_h 1
typedef enum somtVisibilityT { somtInternalVE, somtPublicVE, somtPrivateVE } somtVisibilityT;
	#include <som.h>
	#include <scentry.h>
	#include <somobj.h>
	#include <sctypes.h>
	#include <sm.h>
	#include <somstrt.h>
	#ifndef _IDL_SOMTTypedefEntryC_defined
		#define _IDL_SOMTTypedefEntryC_defined
		typedef SOMObject SOMTTypedefEntryC;
	#endif /* _IDL_SOMTTypedefEntryC_defined */
	#ifndef _IDL_SEQUENCE_SOMTTypedefEntryC_defined
		#define _IDL_SEQUENCE_SOMTTypedefEntryC_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMTTypedefEntryC SOMSTAR ,sequence(SOMTTypedefEntryC));
	#endif /* _IDL_SEQUENCE_SOMTTypedefEntryC_defined */
	#ifndef _IDL_SOMTCommonEntryC_defined
		#define _IDL_SOMTCommonEntryC_defined
		typedef SOMObject SOMTCommonEntryC;
	#endif /* _IDL_SOMTCommonEntryC_defined */
	#ifndef _IDL_SEQUENCE_SOMTCommonEntryC_defined
		#define _IDL_SEQUENCE_SOMTCommonEntryC_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMTCommonEntryC SOMSTAR ,sequence(SOMTCommonEntryC));
	#endif /* _IDL_SEQUENCE_SOMTCommonEntryC_defined */
	#ifdef __IBMC__
		typedef SOMTEntryC SOMSTAR (somTP_SOMTCommonEntryC__get_somtTypeObj)(
			SOMTCommonEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTCommonEntryC__get_somtTypeObj,system)
		typedef somTP_SOMTCommonEntryC__get_somtTypeObj *somTD_SOMTCommonEntryC__get_somtTypeObj;
	#else /* __IBMC__ */
		typedef SOMTEntryC SOMSTAR (SOMLINK * somTD_SOMTCommonEntryC__get_somtTypeObj)(
			SOMTCommonEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_SOMTCommonEntryC__get_somtPtrs)(
			SOMTCommonEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTCommonEntryC__get_somtPtrs,system)
		typedef somTP_SOMTCommonEntryC__get_somtPtrs *somTD_SOMTCommonEntryC__get_somtPtrs;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_SOMTCommonEntryC__get_somtPtrs)(
			SOMTCommonEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_SOMTCommonEntryC__get_somtArrayDimsString)(
			SOMTCommonEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTCommonEntryC__get_somtArrayDimsString,system)
		typedef somTP_SOMTCommonEntryC__get_somtArrayDimsString *somTD_SOMTCommonEntryC__get_somtArrayDimsString;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_SOMTCommonEntryC__get_somtArrayDimsString)(
			SOMTCommonEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef unsigned long (somTP_SOMTCommonEntryC_somtGetFirstArrayDimension)(
			SOMTCommonEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTCommonEntryC_somtGetFirstArrayDimension,system)
		typedef somTP_SOMTCommonEntryC_somtGetFirstArrayDimension *somTD_SOMTCommonEntryC_somtGetFirstArrayDimension;
	#else /* __IBMC__ */
		typedef unsigned long (SOMLINK * somTD_SOMTCommonEntryC_somtGetFirstArrayDimension)(
			SOMTCommonEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef unsigned long (somTP_SOMTCommonEntryC_somtGetNextArrayDimension)(
			SOMTCommonEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTCommonEntryC_somtGetNextArrayDimension,system)
		typedef somTP_SOMTCommonEntryC_somtGetNextArrayDimension *somTD_SOMTCommonEntryC_somtGetNextArrayDimension;
	#else /* __IBMC__ */
		typedef unsigned long (SOMLINK * somTD_SOMTCommonEntryC_somtGetNextArrayDimension)(
			SOMTCommonEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_SOMTCommonEntryC__get_somtSourceText)(
			SOMTCommonEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTCommonEntryC__get_somtSourceText,system)
		typedef somTP_SOMTCommonEntryC__get_somtSourceText *somTD_SOMTCommonEntryC__get_somtSourceText;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_SOMTCommonEntryC__get_somtSourceText)(
			SOMTCommonEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_SOMTCommonEntryC__get_somtType)(
			SOMTCommonEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTCommonEntryC__get_somtType,system)
		typedef somTP_SOMTCommonEntryC__get_somtType *somTD_SOMTCommonEntryC__get_somtType;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_SOMTCommonEntryC__get_somtType)(
			SOMTCommonEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef somtVisibilityT (somTP_SOMTCommonEntryC__get_somtVisibility)(
			SOMTCommonEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTCommonEntryC__get_somtVisibility,system)
		typedef somTP_SOMTCommonEntryC__get_somtVisibility *somTD_SOMTCommonEntryC__get_somtVisibility;
	#else /* __IBMC__ */
		typedef somtVisibilityT (SOMLINK * somTD_SOMTCommonEntryC__get_somtVisibility)(
			SOMTCommonEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef boolean (somTP_SOMTCommonEntryC_somtIsArray)(
			SOMTCommonEntryC SOMSTAR somSelf,
			/* out */ long *size);
		#pragma linkage(somTP_SOMTCommonEntryC_somtIsArray,system)
		typedef somTP_SOMTCommonEntryC_somtIsArray *somTD_SOMTCommonEntryC_somtIsArray;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_SOMTCommonEntryC_somtIsArray)(
			SOMTCommonEntryC SOMSTAR somSelf,
			/* out */ long *size);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef boolean (somTP_SOMTCommonEntryC_somtIsPointer)(
			SOMTCommonEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTCommonEntryC_somtIsPointer,system)
		typedef somTP_SOMTCommonEntryC_somtIsPointer *somTD_SOMTCommonEntryC_somtIsPointer;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_SOMTCommonEntryC_somtIsPointer)(
			SOMTCommonEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifndef SOMTCommonEntryC_MajorVersion
		#define SOMTCommonEntryC_MajorVersion   2
	#endif /* SOMTCommonEntryC_MajorVersion */
	#ifndef SOMTCommonEntryC_MinorVersion
		#define SOMTCommonEntryC_MinorVersion   1
	#endif /* SOMTCommonEntryC_MinorVersion */
	typedef struct SOMTCommonEntryCClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken _get_somtSourceText;
		somMToken _get_somtType;
		somMToken _get_somtVisibility;
		somMToken somtIsArray;
		somMToken somtIsPointer;
		somMToken somtIsArrayDominant;
		somMToken _get_somtTypeObj;
		somMToken _get_somtPtrs;
		somMToken _get_somtArrayDimsString;
		somMToken somtGetFirstArrayDimension;
		somMToken somtGetNextArrayDimension;
	} SOMTCommonEntryCClassDataStructure;
	typedef struct SOMTCommonEntryCCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} SOMTCommonEntryCCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_sccommon_Source) || defined(SOMTCommonEntryC_Class_Source)
			SOMEXTERN struct SOMTCommonEntryCClassDataStructure _SOMTCommonEntryCClassData;
			#ifndef SOMTCommonEntryCClassData
				#define SOMTCommonEntryCClassData    _SOMTCommonEntryCClassData
			#endif /* SOMTCommonEntryCClassData */
		#else
			SOMEXTERN struct SOMTCommonEntryCClassDataStructure * SOMLINK resolve_SOMTCommonEntryCClassData(void);
			#ifndef SOMTCommonEntryCClassData
				#define SOMTCommonEntryCClassData    (*(resolve_SOMTCommonEntryCClassData()))
			#endif /* SOMTCommonEntryCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_sccommon_Source) || defined(SOMTCommonEntryC_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_sccommon_Source || SOMTCommonEntryC_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_sccommon_Source || SOMTCommonEntryC_Class_Source */
		struct SOMTCommonEntryCClassDataStructure SOMDLINK SOMTCommonEntryCClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_sccommon_Source) || defined(SOMTCommonEntryC_Class_Source)
			SOMEXTERN struct SOMTCommonEntryCCClassDataStructure _SOMTCommonEntryCCClassData;
			#ifndef SOMTCommonEntryCCClassData
				#define SOMTCommonEntryCCClassData    _SOMTCommonEntryCCClassData
			#endif /* SOMTCommonEntryCCClassData */
		#else
			SOMEXTERN struct SOMTCommonEntryCCClassDataStructure * SOMLINK resolve_SOMTCommonEntryCCClassData(void);
			#ifndef SOMTCommonEntryCCClassData
				#define SOMTCommonEntryCCClassData    (*(resolve_SOMTCommonEntryCCClassData()))
			#endif /* SOMTCommonEntryCCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_sccommon_Source) || defined(SOMTCommonEntryC_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_sccommon_Source || SOMTCommonEntryC_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_sccommon_Source || SOMTCommonEntryC_Class_Source */
		struct SOMTCommonEntryCCClassDataStructure SOMDLINK SOMTCommonEntryCCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_sccommon_Source) || defined(SOMTCommonEntryC_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_sccommon_Source || SOMTCommonEntryC_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_sccommon_Source || SOMTCommonEntryC_Class_Source */
	SOMClass SOMSTAR SOMLINK SOMTCommonEntryCNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_SOMTCommonEntryC (SOMTCommonEntryCClassData.classObject)
	#ifndef SOMGD_SOMTCommonEntryC
		#if (defined(_SOMTCommonEntryC) || defined(__SOMTCommonEntryC))
			#undef _SOMTCommonEntryC
			#undef __SOMTCommonEntryC
			#define SOMGD_SOMTCommonEntryC 1
		#else
			#define _SOMTCommonEntryC _SOMCLASS_SOMTCommonEntryC
		#endif /* _SOMTCommonEntryC */
	#endif /* SOMGD_SOMTCommonEntryC */
	#define SOMTCommonEntryC_classObj _SOMCLASS_SOMTCommonEntryC
	#define _SOMMTOKEN_SOMTCommonEntryC(method) ((somMToken)(SOMTCommonEntryCClassData.method))
	#ifndef SOMTCommonEntryCNew
		#define SOMTCommonEntryCNew() ( _SOMTCommonEntryC ? \
			(SOMClass_somNew(_SOMTCommonEntryC)) : \
			( SOMTCommonEntryCNewClass( \
				SOMTCommonEntryC_MajorVersion, \
				SOMTCommonEntryC_MinorVersion), \
			SOMClass_somNew(_SOMTCommonEntryC))) 
	#endif /* NewSOMTCommonEntryC */
	#ifndef SOMTCommonEntryC__get_somtTypeObj
		#define SOMTCommonEntryC__get_somtTypeObj(somSelf) \
			SOM_Resolve(somSelf,SOMTCommonEntryC,_get_somtTypeObj)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtTypeObj
				#if defined(__get_somtTypeObj)
					#undef __get_somtTypeObj
					#define SOMGD___get_somtTypeObj
				#else
					#define __get_somtTypeObj SOMTCommonEntryC__get_somtTypeObj
				#endif
			#endif /* SOMGD___get_somtTypeObj */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTCommonEntryC__get_somtTypeObj */
	#ifndef SOMTCommonEntryC__get_somtPtrs
		#define SOMTCommonEntryC__get_somtPtrs(somSelf) \
			SOM_Resolve(somSelf,SOMTCommonEntryC,_get_somtPtrs)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtPtrs
				#if defined(__get_somtPtrs)
					#undef __get_somtPtrs
					#define SOMGD___get_somtPtrs
				#else
					#define __get_somtPtrs SOMTCommonEntryC__get_somtPtrs
				#endif
			#endif /* SOMGD___get_somtPtrs */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTCommonEntryC__get_somtPtrs */
	#ifndef SOMTCommonEntryC__get_somtArrayDimsString
		#define SOMTCommonEntryC__get_somtArrayDimsString(somSelf) \
			SOM_Resolve(somSelf,SOMTCommonEntryC,_get_somtArrayDimsString)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtArrayDimsString
				#if defined(__get_somtArrayDimsString)
					#undef __get_somtArrayDimsString
					#define SOMGD___get_somtArrayDimsString
				#else
					#define __get_somtArrayDimsString SOMTCommonEntryC__get_somtArrayDimsString
				#endif
			#endif /* SOMGD___get_somtArrayDimsString */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTCommonEntryC__get_somtArrayDimsString */
	#ifndef SOMTCommonEntryC_somtGetFirstArrayDimension
		#define SOMTCommonEntryC_somtGetFirstArrayDimension(somSelf) \
			SOM_Resolve(somSelf,SOMTCommonEntryC,somtGetFirstArrayDimension)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetFirstArrayDimension
				#if defined(_somtGetFirstArrayDimension)
					#undef _somtGetFirstArrayDimension
					#define SOMGD__somtGetFirstArrayDimension
				#else
					#define _somtGetFirstArrayDimension SOMTCommonEntryC_somtGetFirstArrayDimension
				#endif
			#endif /* SOMGD__somtGetFirstArrayDimension */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTCommonEntryC_somtGetFirstArrayDimension */
	#ifndef SOMTCommonEntryC_somtGetNextArrayDimension
		#define SOMTCommonEntryC_somtGetNextArrayDimension(somSelf) \
			SOM_Resolve(somSelf,SOMTCommonEntryC,somtGetNextArrayDimension)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetNextArrayDimension
				#if defined(_somtGetNextArrayDimension)
					#undef _somtGetNextArrayDimension
					#define SOMGD__somtGetNextArrayDimension
				#else
					#define _somtGetNextArrayDimension SOMTCommonEntryC_somtGetNextArrayDimension
				#endif
			#endif /* SOMGD__somtGetNextArrayDimension */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTCommonEntryC_somtGetNextArrayDimension */
	#ifndef SOMTCommonEntryC__get_somtSourceText
		#define SOMTCommonEntryC__get_somtSourceText(somSelf) \
			SOM_Resolve(somSelf,SOMTCommonEntryC,_get_somtSourceText)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtSourceText
				#if defined(__get_somtSourceText)
					#undef __get_somtSourceText
					#define SOMGD___get_somtSourceText
				#else
					#define __get_somtSourceText SOMTCommonEntryC__get_somtSourceText
				#endif
			#endif /* SOMGD___get_somtSourceText */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTCommonEntryC__get_somtSourceText */
	#ifndef SOMTCommonEntryC__get_somtType
		#define SOMTCommonEntryC__get_somtType(somSelf) \
			SOM_Resolve(somSelf,SOMTCommonEntryC,_get_somtType)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtType
				#if defined(__get_somtType)
					#undef __get_somtType
					#define SOMGD___get_somtType
				#else
					#define __get_somtType SOMTCommonEntryC__get_somtType
				#endif
			#endif /* SOMGD___get_somtType */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTCommonEntryC__get_somtType */
	#ifndef SOMTCommonEntryC__get_somtVisibility
		#define SOMTCommonEntryC__get_somtVisibility(somSelf) \
			SOM_Resolve(somSelf,SOMTCommonEntryC,_get_somtVisibility)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtVisibility
				#if defined(__get_somtVisibility)
					#undef __get_somtVisibility
					#define SOMGD___get_somtVisibility
				#else
					#define __get_somtVisibility SOMTCommonEntryC__get_somtVisibility
				#endif
			#endif /* SOMGD___get_somtVisibility */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTCommonEntryC__get_somtVisibility */
	#ifndef SOMTCommonEntryC_somtIsArray
		#define SOMTCommonEntryC_somtIsArray(somSelf,size) \
			SOM_Resolve(somSelf,SOMTCommonEntryC,somtIsArray)  \
				(somSelf,size)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtIsArray
				#if defined(_somtIsArray)
					#undef _somtIsArray
					#define SOMGD__somtIsArray
				#else
					#define _somtIsArray SOMTCommonEntryC_somtIsArray
				#endif
			#endif /* SOMGD__somtIsArray */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTCommonEntryC_somtIsArray */
	#ifndef SOMTCommonEntryC_somtIsPointer
		#define SOMTCommonEntryC_somtIsPointer(somSelf) \
			SOM_Resolve(somSelf,SOMTCommonEntryC,somtIsPointer)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtIsPointer
				#if defined(_somtIsPointer)
					#undef _somtIsPointer
					#define SOMGD__somtIsPointer
				#else
					#define _somtIsPointer SOMTCommonEntryC_somtIsPointer
				#endif
			#endif /* SOMGD__somtIsPointer */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTCommonEntryC_somtIsPointer */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define SOMTCommonEntryC_somInit SOMObject_somInit
#define SOMTCommonEntryC_somUninit SOMObject_somUninit
#define SOMTCommonEntryC_somFree SOMObject_somFree
#define SOMTCommonEntryC_somGetClass SOMObject_somGetClass
#define SOMTCommonEntryC_somGetClassName SOMObject_somGetClassName
#define SOMTCommonEntryC_somGetSize SOMObject_somGetSize
#define SOMTCommonEntryC_somIsA SOMObject_somIsA
#define SOMTCommonEntryC_somIsInstanceOf SOMObject_somIsInstanceOf
#define SOMTCommonEntryC_somRespondsTo SOMObject_somRespondsTo
#define SOMTCommonEntryC_somDispatch SOMObject_somDispatch
#define SOMTCommonEntryC_somClassDispatch SOMObject_somClassDispatch
#define SOMTCommonEntryC_somCastObj SOMObject_somCastObj
#define SOMTCommonEntryC_somResetObj SOMObject_somResetObj
#define SOMTCommonEntryC_somPrintSelf SOMObject_somPrintSelf
#define SOMTCommonEntryC_somDumpSelf SOMObject_somDumpSelf
#define SOMTCommonEntryC_somDumpSelfInt SOMObject_somDumpSelfInt
#define SOMTCommonEntryC_somDefaultInit SOMObject_somDefaultInit
#define SOMTCommonEntryC_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define SOMTCommonEntryC_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define SOMTCommonEntryC_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define SOMTCommonEntryC_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define SOMTCommonEntryC_somDefaultAssign SOMObject_somDefaultAssign
#define SOMTCommonEntryC_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define SOMTCommonEntryC_somDefaultVAssign SOMObject_somDefaultVAssign
#define SOMTCommonEntryC_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define SOMTCommonEntryC_somDestruct SOMObject_somDestruct
#define SOMTCommonEntryC__get_somtEntryStruct SOMTEntryC__get_somtEntryStruct
#define SOMTCommonEntryC__set_somtEntryName SOMTEntryC__set_somtEntryName
#define SOMTCommonEntryC__get_somtEntryName SOMTEntryC__get_somtEntryName
#define SOMTCommonEntryC__set_somtElementType SOMTEntryC__set_somtElementType
#define SOMTCommonEntryC__get_somtElementType SOMTEntryC__get_somtElementType
#define SOMTCommonEntryC__get_somtElementTypeName SOMTEntryC__get_somtElementTypeName
#define SOMTCommonEntryC__get_somtEntryComment SOMTEntryC__get_somtEntryComment
#define SOMTCommonEntryC__get_somtSourceLineNumber SOMTEntryC__get_somtSourceLineNumber
#define SOMTCommonEntryC__get_somtTypeCode SOMTEntryC__get_somtTypeCode
#define SOMTCommonEntryC__get_somtIsReference SOMTEntryC__get_somtIsReference
#define SOMTCommonEntryC__get_somtIDLScopedName SOMTEntryC__get_somtIDLScopedName
#define SOMTCommonEntryC__get_somtCScopedName SOMTEntryC__get_somtCScopedName
#define SOMTCommonEntryC_somtGetModifierValue SOMTEntryC_somtGetModifierValue
#define SOMTCommonEntryC_somtGetFirstModifier SOMTEntryC_somtGetFirstModifier
#define SOMTCommonEntryC_somtGetNextModifier SOMTEntryC_somtGetNextModifier
#define SOMTCommonEntryC_somtFormatModifier SOMTEntryC_somtFormatModifier
#define SOMTCommonEntryC_somtGetModifierList SOMTEntryC_somtGetModifierList
#define SOMTCommonEntryC_somtSetSymbolsOnEntry SOMTEntryC_somtSetSymbolsOnEntry
#define SOMTCommonEntryC_somtSetEntryStruct SOMTEntryC_somtSetEntryStruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_sccommon_Header_h */
