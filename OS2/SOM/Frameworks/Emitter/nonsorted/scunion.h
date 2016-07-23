/* generated from scunion.idl */
/* internal conditional is SOM_Module_scunion_Source */
#ifndef SOM_Module_scunion_Header_h
	#define SOM_Module_scunion_Header_h 1
	#include <som.h>
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
	#ifndef _IDL_SOMTUnionEntryC_defined
		#define _IDL_SOMTUnionEntryC_defined
		typedef SOMObject SOMTUnionEntryC;
	#endif /* _IDL_SOMTUnionEntryC_defined */
	#ifndef _IDL_SEQUENCE_SOMTUnionEntryC_defined
		#define _IDL_SEQUENCE_SOMTUnionEntryC_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMTUnionEntryC SOMSTAR ,sequence(SOMTUnionEntryC));
	#endif /* _IDL_SEQUENCE_SOMTUnionEntryC_defined */
	#ifdef __IBMC__
		typedef SOMTEntryC SOMSTAR (somTP_SOMTUnionEntryC__get_somtSwitchType)(
			SOMTUnionEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTUnionEntryC__get_somtSwitchType,system)
		typedef somTP_SOMTUnionEntryC__get_somtSwitchType *somTD_SOMTUnionEntryC__get_somtSwitchType;
	#else /* __IBMC__ */
		typedef SOMTEntryC SOMSTAR (SOMLINK * somTD_SOMTUnionEntryC__get_somtSwitchType)(
			SOMTUnionEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifndef SOMTUnionEntryC_MajorVersion
		#define SOMTUnionEntryC_MajorVersion   2
	#endif /* SOMTUnionEntryC_MajorVersion */
	#ifndef SOMTUnionEntryC_MinorVersion
		#define SOMTUnionEntryC_MinorVersion   1
	#endif /* SOMTUnionEntryC_MinorVersion */
	typedef struct SOMTUnionEntryCClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken _get_somtSwitchType;
		somMToken somtGetFirstCaseEntry;
		somMToken somtGetNextCaseEntry;
	} SOMTUnionEntryCClassDataStructure;
	typedef struct SOMTUnionEntryCCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} SOMTUnionEntryCCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_scunion_Source) || defined(SOMTUnionEntryC_Class_Source)
			SOMEXTERN struct SOMTUnionEntryCClassDataStructure _SOMTUnionEntryCClassData;
			#ifndef SOMTUnionEntryCClassData
				#define SOMTUnionEntryCClassData    _SOMTUnionEntryCClassData
			#endif /* SOMTUnionEntryCClassData */
		#else
			SOMEXTERN struct SOMTUnionEntryCClassDataStructure * SOMLINK resolve_SOMTUnionEntryCClassData(void);
			#ifndef SOMTUnionEntryCClassData
				#define SOMTUnionEntryCClassData    (*(resolve_SOMTUnionEntryCClassData()))
			#endif /* SOMTUnionEntryCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_scunion_Source) || defined(SOMTUnionEntryC_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_scunion_Source || SOMTUnionEntryC_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_scunion_Source || SOMTUnionEntryC_Class_Source */
		struct SOMTUnionEntryCClassDataStructure SOMDLINK SOMTUnionEntryCClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_scunion_Source) || defined(SOMTUnionEntryC_Class_Source)
			SOMEXTERN struct SOMTUnionEntryCCClassDataStructure _SOMTUnionEntryCCClassData;
			#ifndef SOMTUnionEntryCCClassData
				#define SOMTUnionEntryCCClassData    _SOMTUnionEntryCCClassData
			#endif /* SOMTUnionEntryCCClassData */
		#else
			SOMEXTERN struct SOMTUnionEntryCCClassDataStructure * SOMLINK resolve_SOMTUnionEntryCCClassData(void);
			#ifndef SOMTUnionEntryCCClassData
				#define SOMTUnionEntryCCClassData    (*(resolve_SOMTUnionEntryCCClassData()))
			#endif /* SOMTUnionEntryCCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_scunion_Source) || defined(SOMTUnionEntryC_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_scunion_Source || SOMTUnionEntryC_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_scunion_Source || SOMTUnionEntryC_Class_Source */
		struct SOMTUnionEntryCCClassDataStructure SOMDLINK SOMTUnionEntryCCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_scunion_Source) || defined(SOMTUnionEntryC_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_scunion_Source || SOMTUnionEntryC_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_scunion_Source || SOMTUnionEntryC_Class_Source */
	SOMClass SOMSTAR SOMLINK SOMTUnionEntryCNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_SOMTUnionEntryC (SOMTUnionEntryCClassData.classObject)
	#ifndef SOMGD_SOMTUnionEntryC
		#if (defined(_SOMTUnionEntryC) || defined(__SOMTUnionEntryC))
			#undef _SOMTUnionEntryC
			#undef __SOMTUnionEntryC
			#define SOMGD_SOMTUnionEntryC 1
		#else
			#define _SOMTUnionEntryC _SOMCLASS_SOMTUnionEntryC
		#endif /* _SOMTUnionEntryC */
	#endif /* SOMGD_SOMTUnionEntryC */
	#define SOMTUnionEntryC_classObj _SOMCLASS_SOMTUnionEntryC
	#define _SOMMTOKEN_SOMTUnionEntryC(method) ((somMToken)(SOMTUnionEntryCClassData.method))
	#ifndef SOMTUnionEntryCNew
		#define SOMTUnionEntryCNew() ( _SOMTUnionEntryC ? \
			(SOMClass_somNew(_SOMTUnionEntryC)) : \
			( SOMTUnionEntryCNewClass( \
				SOMTUnionEntryC_MajorVersion, \
				SOMTUnionEntryC_MinorVersion), \
			SOMClass_somNew(_SOMTUnionEntryC))) 
	#endif /* NewSOMTUnionEntryC */
	#ifndef SOMTUnionEntryC__get_somtSwitchType
		#define SOMTUnionEntryC__get_somtSwitchType(somSelf) \
			SOM_Resolve(somSelf,SOMTUnionEntryC,_get_somtSwitchType)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtSwitchType
				#if defined(__get_somtSwitchType)
					#undef __get_somtSwitchType
					#define SOMGD___get_somtSwitchType
				#else
					#define __get_somtSwitchType SOMTUnionEntryC__get_somtSwitchType
				#endif
			#endif /* SOMGD___get_somtSwitchType */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTUnionEntryC__get_somtSwitchType */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define SOMTUnionEntryC_somInit SOMObject_somInit
#define SOMTUnionEntryC_somUninit SOMObject_somUninit
#define SOMTUnionEntryC_somFree SOMObject_somFree
#define SOMTUnionEntryC_somGetClass SOMObject_somGetClass
#define SOMTUnionEntryC_somGetClassName SOMObject_somGetClassName
#define SOMTUnionEntryC_somGetSize SOMObject_somGetSize
#define SOMTUnionEntryC_somIsA SOMObject_somIsA
#define SOMTUnionEntryC_somIsInstanceOf SOMObject_somIsInstanceOf
#define SOMTUnionEntryC_somRespondsTo SOMObject_somRespondsTo
#define SOMTUnionEntryC_somDispatch SOMObject_somDispatch
#define SOMTUnionEntryC_somClassDispatch SOMObject_somClassDispatch
#define SOMTUnionEntryC_somCastObj SOMObject_somCastObj
#define SOMTUnionEntryC_somResetObj SOMObject_somResetObj
#define SOMTUnionEntryC_somPrintSelf SOMObject_somPrintSelf
#define SOMTUnionEntryC_somDumpSelf SOMObject_somDumpSelf
#define SOMTUnionEntryC_somDumpSelfInt SOMObject_somDumpSelfInt
#define SOMTUnionEntryC_somDefaultInit SOMObject_somDefaultInit
#define SOMTUnionEntryC_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define SOMTUnionEntryC_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define SOMTUnionEntryC_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define SOMTUnionEntryC_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define SOMTUnionEntryC_somDefaultAssign SOMObject_somDefaultAssign
#define SOMTUnionEntryC_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define SOMTUnionEntryC_somDefaultVAssign SOMObject_somDefaultVAssign
#define SOMTUnionEntryC_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define SOMTUnionEntryC_somDestruct SOMObject_somDestruct
#define SOMTUnionEntryC__get_somtEntryStruct SOMTEntryC__get_somtEntryStruct
#define SOMTUnionEntryC__set_somtEntryName SOMTEntryC__set_somtEntryName
#define SOMTUnionEntryC__get_somtEntryName SOMTEntryC__get_somtEntryName
#define SOMTUnionEntryC__set_somtElementType SOMTEntryC__set_somtElementType
#define SOMTUnionEntryC__get_somtElementType SOMTEntryC__get_somtElementType
#define SOMTUnionEntryC__get_somtElementTypeName SOMTEntryC__get_somtElementTypeName
#define SOMTUnionEntryC__get_somtEntryComment SOMTEntryC__get_somtEntryComment
#define SOMTUnionEntryC__get_somtSourceLineNumber SOMTEntryC__get_somtSourceLineNumber
#define SOMTUnionEntryC__get_somtTypeCode SOMTEntryC__get_somtTypeCode
#define SOMTUnionEntryC__get_somtIsReference SOMTEntryC__get_somtIsReference
#define SOMTUnionEntryC__get_somtIDLScopedName SOMTEntryC__get_somtIDLScopedName
#define SOMTUnionEntryC__get_somtCScopedName SOMTEntryC__get_somtCScopedName
#define SOMTUnionEntryC_somtGetModifierValue SOMTEntryC_somtGetModifierValue
#define SOMTUnionEntryC_somtGetFirstModifier SOMTEntryC_somtGetFirstModifier
#define SOMTUnionEntryC_somtGetNextModifier SOMTEntryC_somtGetNextModifier
#define SOMTUnionEntryC_somtFormatModifier SOMTEntryC_somtFormatModifier
#define SOMTUnionEntryC_somtGetModifierList SOMTEntryC_somtGetModifierList
#define SOMTUnionEntryC_somtSetSymbolsOnEntry SOMTEntryC_somtSetSymbolsOnEntry
#define SOMTUnionEntryC_somtSetEntryStruct SOMTEntryC_somtSetEntryStruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_scunion_Header_h */
