/* generated from scdata.idl */
/* internal conditional is SOM_Module_scdata_Source */
#ifndef SOM_Module_scdata_Header_h
	#define SOM_Module_scdata_Header_h 1
	#include <som.h>
	#include <sccommon.h>
	#include <scentry.h>
	#include <somobj.h>
	#include <sctypes.h>
	#include <sm.h>
	#include <somstrt.h>
	#ifndef _IDL_SOMTDataEntryC_defined
		#define _IDL_SOMTDataEntryC_defined
		typedef SOMObject SOMTDataEntryC;
	#endif /* _IDL_SOMTDataEntryC_defined */
	#ifndef _IDL_SEQUENCE_SOMTDataEntryC_defined
		#define _IDL_SEQUENCE_SOMTDataEntryC_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMTDataEntryC SOMSTAR ,sequence(SOMTDataEntryC));
	#endif /* _IDL_SEQUENCE_SOMTDataEntryC_defined */
	#ifdef __IBMC__
		typedef boolean (somTP_SOMTDataEntryC__get_somtIsSelfRef)(
			SOMTDataEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTDataEntryC__get_somtIsSelfRef,system)
		typedef somTP_SOMTDataEntryC__get_somtIsSelfRef *somTD_SOMTDataEntryC__get_somtIsSelfRef;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_SOMTDataEntryC__get_somtIsSelfRef)(
			SOMTDataEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifndef SOMTDataEntryC_MajorVersion
		#define SOMTDataEntryC_MajorVersion   2
	#endif /* SOMTDataEntryC_MajorVersion */
	#ifndef SOMTDataEntryC_MinorVersion
		#define SOMTDataEntryC_MinorVersion   1
	#endif /* SOMTDataEntryC_MinorVersion */
	typedef struct SOMTDataEntryCClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken _get_somtIsSelfRef;
	} SOMTDataEntryCClassDataStructure;
	typedef struct SOMTDataEntryCCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} SOMTDataEntryCCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_scdata_Source) || defined(SOMTDataEntryC_Class_Source)
			SOMEXTERN struct SOMTDataEntryCClassDataStructure _SOMTDataEntryCClassData;
			#ifndef SOMTDataEntryCClassData
				#define SOMTDataEntryCClassData    _SOMTDataEntryCClassData
			#endif /* SOMTDataEntryCClassData */
		#else
			SOMEXTERN struct SOMTDataEntryCClassDataStructure * SOMLINK resolve_SOMTDataEntryCClassData(void);
			#ifndef SOMTDataEntryCClassData
				#define SOMTDataEntryCClassData    (*(resolve_SOMTDataEntryCClassData()))
			#endif /* SOMTDataEntryCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_scdata_Source) || defined(SOMTDataEntryC_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_scdata_Source || SOMTDataEntryC_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_scdata_Source || SOMTDataEntryC_Class_Source */
		struct SOMTDataEntryCClassDataStructure SOMDLINK SOMTDataEntryCClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_scdata_Source) || defined(SOMTDataEntryC_Class_Source)
			SOMEXTERN struct SOMTDataEntryCCClassDataStructure _SOMTDataEntryCCClassData;
			#ifndef SOMTDataEntryCCClassData
				#define SOMTDataEntryCCClassData    _SOMTDataEntryCCClassData
			#endif /* SOMTDataEntryCCClassData */
		#else
			SOMEXTERN struct SOMTDataEntryCCClassDataStructure * SOMLINK resolve_SOMTDataEntryCCClassData(void);
			#ifndef SOMTDataEntryCCClassData
				#define SOMTDataEntryCCClassData    (*(resolve_SOMTDataEntryCCClassData()))
			#endif /* SOMTDataEntryCCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_scdata_Source) || defined(SOMTDataEntryC_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_scdata_Source || SOMTDataEntryC_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_scdata_Source || SOMTDataEntryC_Class_Source */
		struct SOMTDataEntryCCClassDataStructure SOMDLINK SOMTDataEntryCCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_scdata_Source) || defined(SOMTDataEntryC_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_scdata_Source || SOMTDataEntryC_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_scdata_Source || SOMTDataEntryC_Class_Source */
	SOMClass SOMSTAR SOMLINK SOMTDataEntryCNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_SOMTDataEntryC (SOMTDataEntryCClassData.classObject)
	#ifndef SOMGD_SOMTDataEntryC
		#if (defined(_SOMTDataEntryC) || defined(__SOMTDataEntryC))
			#undef _SOMTDataEntryC
			#undef __SOMTDataEntryC
			#define SOMGD_SOMTDataEntryC 1
		#else
			#define _SOMTDataEntryC _SOMCLASS_SOMTDataEntryC
		#endif /* _SOMTDataEntryC */
	#endif /* SOMGD_SOMTDataEntryC */
	#define SOMTDataEntryC_classObj _SOMCLASS_SOMTDataEntryC
	#define _SOMMTOKEN_SOMTDataEntryC(method) ((somMToken)(SOMTDataEntryCClassData.method))
	#ifndef SOMTDataEntryCNew
		#define SOMTDataEntryCNew() ( _SOMTDataEntryC ? \
			(SOMClass_somNew(_SOMTDataEntryC)) : \
			( SOMTDataEntryCNewClass( \
				SOMTDataEntryC_MajorVersion, \
				SOMTDataEntryC_MinorVersion), \
			SOMClass_somNew(_SOMTDataEntryC))) 
	#endif /* NewSOMTDataEntryC */
	#ifndef SOMTDataEntryC__get_somtIsSelfRef
		#define SOMTDataEntryC__get_somtIsSelfRef(somSelf) \
			SOM_Resolve(somSelf,SOMTDataEntryC,_get_somtIsSelfRef)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtIsSelfRef
				#if defined(__get_somtIsSelfRef)
					#undef __get_somtIsSelfRef
					#define SOMGD___get_somtIsSelfRef
				#else
					#define __get_somtIsSelfRef SOMTDataEntryC__get_somtIsSelfRef
				#endif
			#endif /* SOMGD___get_somtIsSelfRef */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTDataEntryC__get_somtIsSelfRef */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define SOMTDataEntryC_somInit SOMObject_somInit
#define SOMTDataEntryC_somUninit SOMObject_somUninit
#define SOMTDataEntryC_somFree SOMObject_somFree
#define SOMTDataEntryC_somGetClass SOMObject_somGetClass
#define SOMTDataEntryC_somGetClassName SOMObject_somGetClassName
#define SOMTDataEntryC_somGetSize SOMObject_somGetSize
#define SOMTDataEntryC_somIsA SOMObject_somIsA
#define SOMTDataEntryC_somIsInstanceOf SOMObject_somIsInstanceOf
#define SOMTDataEntryC_somRespondsTo SOMObject_somRespondsTo
#define SOMTDataEntryC_somDispatch SOMObject_somDispatch
#define SOMTDataEntryC_somClassDispatch SOMObject_somClassDispatch
#define SOMTDataEntryC_somCastObj SOMObject_somCastObj
#define SOMTDataEntryC_somResetObj SOMObject_somResetObj
#define SOMTDataEntryC_somPrintSelf SOMObject_somPrintSelf
#define SOMTDataEntryC_somDumpSelf SOMObject_somDumpSelf
#define SOMTDataEntryC_somDumpSelfInt SOMObject_somDumpSelfInt
#define SOMTDataEntryC_somDefaultInit SOMObject_somDefaultInit
#define SOMTDataEntryC_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define SOMTDataEntryC_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define SOMTDataEntryC_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define SOMTDataEntryC_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define SOMTDataEntryC_somDefaultAssign SOMObject_somDefaultAssign
#define SOMTDataEntryC_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define SOMTDataEntryC_somDefaultVAssign SOMObject_somDefaultVAssign
#define SOMTDataEntryC_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define SOMTDataEntryC_somDestruct SOMObject_somDestruct
#define SOMTDataEntryC__get_somtEntryStruct SOMTEntryC__get_somtEntryStruct
#define SOMTDataEntryC__set_somtEntryName SOMTEntryC__set_somtEntryName
#define SOMTDataEntryC__get_somtEntryName SOMTEntryC__get_somtEntryName
#define SOMTDataEntryC__set_somtElementType SOMTEntryC__set_somtElementType
#define SOMTDataEntryC__get_somtElementType SOMTEntryC__get_somtElementType
#define SOMTDataEntryC__get_somtElementTypeName SOMTEntryC__get_somtElementTypeName
#define SOMTDataEntryC__get_somtEntryComment SOMTEntryC__get_somtEntryComment
#define SOMTDataEntryC__get_somtSourceLineNumber SOMTEntryC__get_somtSourceLineNumber
#define SOMTDataEntryC__get_somtTypeCode SOMTEntryC__get_somtTypeCode
#define SOMTDataEntryC__get_somtIsReference SOMTEntryC__get_somtIsReference
#define SOMTDataEntryC__get_somtIDLScopedName SOMTEntryC__get_somtIDLScopedName
#define SOMTDataEntryC__get_somtCScopedName SOMTEntryC__get_somtCScopedName
#define SOMTDataEntryC_somtGetModifierValue SOMTEntryC_somtGetModifierValue
#define SOMTDataEntryC_somtGetFirstModifier SOMTEntryC_somtGetFirstModifier
#define SOMTDataEntryC_somtGetNextModifier SOMTEntryC_somtGetNextModifier
#define SOMTDataEntryC_somtFormatModifier SOMTEntryC_somtFormatModifier
#define SOMTDataEntryC_somtGetModifierList SOMTEntryC_somtGetModifierList
#define SOMTDataEntryC_somtSetSymbolsOnEntry SOMTEntryC_somtSetSymbolsOnEntry
#define SOMTDataEntryC_somtSetEntryStruct SOMTEntryC_somtSetEntryStruct
#define SOMTDataEntryC__get_somtTypeObj SOMTCommonEntryC__get_somtTypeObj
#define SOMTDataEntryC__get_somtPtrs SOMTCommonEntryC__get_somtPtrs
#define SOMTDataEntryC__get_somtArrayDimsString SOMTCommonEntryC__get_somtArrayDimsString
#define SOMTDataEntryC_somtGetFirstArrayDimension SOMTCommonEntryC_somtGetFirstArrayDimension
#define SOMTDataEntryC_somtGetNextArrayDimension SOMTCommonEntryC_somtGetNextArrayDimension
#define SOMTDataEntryC__get_somtSourceText SOMTCommonEntryC__get_somtSourceText
#define SOMTDataEntryC__get_somtType SOMTCommonEntryC__get_somtType
#define SOMTDataEntryC__get_somtVisibility SOMTCommonEntryC__get_somtVisibility
#define SOMTDataEntryC_somtIsArray SOMTCommonEntryC_somtIsArray
#define SOMTDataEntryC_somtIsPointer SOMTCommonEntryC_somtIsPointer
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_scdata_Header_h */
