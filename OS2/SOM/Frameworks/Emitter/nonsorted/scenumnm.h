/* generated from scenumnm.idl */
/* internal conditional is SOM_Module_scenumnm_Source */
#ifndef SOM_Module_scenumnm_Header_h
	#define SOM_Module_scenumnm_Header_h 1
	#include <som.h>
	#include <scentry.h>
	#include <somobj.h>
	#include <sctypes.h>
	#include <sm.h>
	#include <somstrt.h>
	#ifndef _IDL_SOMTEnumEntryC_defined
		#define _IDL_SOMTEnumEntryC_defined
		typedef SOMObject SOMTEnumEntryC;
	#endif /* _IDL_SOMTEnumEntryC_defined */
	#ifndef _IDL_SEQUENCE_SOMTEnumEntryC_defined
		#define _IDL_SEQUENCE_SOMTEnumEntryC_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMTEnumEntryC SOMSTAR ,sequence(SOMTEnumEntryC));
	#endif /* _IDL_SEQUENCE_SOMTEnumEntryC_defined */
	#ifndef _IDL_SOMTEnumNameEntryC_defined
		#define _IDL_SOMTEnumNameEntryC_defined
		typedef SOMObject SOMTEnumNameEntryC;
	#endif /* _IDL_SOMTEnumNameEntryC_defined */
	#ifndef _IDL_SEQUENCE_SOMTEnumNameEntryC_defined
		#define _IDL_SEQUENCE_SOMTEnumNameEntryC_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMTEnumNameEntryC SOMSTAR ,sequence(SOMTEnumNameEntryC));
	#endif /* _IDL_SEQUENCE_SOMTEnumNameEntryC_defined */
	#ifdef __IBMC__
		typedef SOMTEnumEntryC SOMSTAR (somTP_SOMTEnumNameEntryC__get_somtEnumPtr)(
			SOMTEnumNameEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTEnumNameEntryC__get_somtEnumPtr,system)
		typedef somTP_SOMTEnumNameEntryC__get_somtEnumPtr *somTD_SOMTEnumNameEntryC__get_somtEnumPtr;
	#else /* __IBMC__ */
		typedef SOMTEnumEntryC SOMSTAR (SOMLINK * somTD_SOMTEnumNameEntryC__get_somtEnumPtr)(
			SOMTEnumNameEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef unsigned long (somTP_SOMTEnumNameEntryC__get_somtEnumVal)(
			SOMTEnumNameEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTEnumNameEntryC__get_somtEnumVal,system)
		typedef somTP_SOMTEnumNameEntryC__get_somtEnumVal *somTD_SOMTEnumNameEntryC__get_somtEnumVal;
	#else /* __IBMC__ */
		typedef unsigned long (SOMLINK * somTD_SOMTEnumNameEntryC__get_somtEnumVal)(
			SOMTEnumNameEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifndef SOMTEnumNameEntryC_MajorVersion
		#define SOMTEnumNameEntryC_MajorVersion   2
	#endif /* SOMTEnumNameEntryC_MajorVersion */
	#ifndef SOMTEnumNameEntryC_MinorVersion
		#define SOMTEnumNameEntryC_MinorVersion   1
	#endif /* SOMTEnumNameEntryC_MinorVersion */
	typedef struct SOMTEnumNameEntryCClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken _get_somtEnumPtr;
		somMToken _get_somtEnumVal;
	} SOMTEnumNameEntryCClassDataStructure;
	typedef struct SOMTEnumNameEntryCCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} SOMTEnumNameEntryCCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_scenumnm_Source) || defined(SOMTEnumNameEntryC_Class_Source)
			SOMEXTERN struct SOMTEnumNameEntryCClassDataStructure _SOMTEnumNameEntryCClassData;
			#ifndef SOMTEnumNameEntryCClassData
				#define SOMTEnumNameEntryCClassData    _SOMTEnumNameEntryCClassData
			#endif /* SOMTEnumNameEntryCClassData */
		#else
			SOMEXTERN struct SOMTEnumNameEntryCClassDataStructure * SOMLINK resolve_SOMTEnumNameEntryCClassData(void);
			#ifndef SOMTEnumNameEntryCClassData
				#define SOMTEnumNameEntryCClassData    (*(resolve_SOMTEnumNameEntryCClassData()))
			#endif /* SOMTEnumNameEntryCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_scenumnm_Source) || defined(SOMTEnumNameEntryC_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_scenumnm_Source || SOMTEnumNameEntryC_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_scenumnm_Source || SOMTEnumNameEntryC_Class_Source */
		struct SOMTEnumNameEntryCClassDataStructure SOMDLINK SOMTEnumNameEntryCClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_scenumnm_Source) || defined(SOMTEnumNameEntryC_Class_Source)
			SOMEXTERN struct SOMTEnumNameEntryCCClassDataStructure _SOMTEnumNameEntryCCClassData;
			#ifndef SOMTEnumNameEntryCCClassData
				#define SOMTEnumNameEntryCCClassData    _SOMTEnumNameEntryCCClassData
			#endif /* SOMTEnumNameEntryCCClassData */
		#else
			SOMEXTERN struct SOMTEnumNameEntryCCClassDataStructure * SOMLINK resolve_SOMTEnumNameEntryCCClassData(void);
			#ifndef SOMTEnumNameEntryCCClassData
				#define SOMTEnumNameEntryCCClassData    (*(resolve_SOMTEnumNameEntryCCClassData()))
			#endif /* SOMTEnumNameEntryCCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_scenumnm_Source) || defined(SOMTEnumNameEntryC_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_scenumnm_Source || SOMTEnumNameEntryC_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_scenumnm_Source || SOMTEnumNameEntryC_Class_Source */
		struct SOMTEnumNameEntryCCClassDataStructure SOMDLINK SOMTEnumNameEntryCCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_scenumnm_Source) || defined(SOMTEnumNameEntryC_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_scenumnm_Source || SOMTEnumNameEntryC_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_scenumnm_Source || SOMTEnumNameEntryC_Class_Source */
	SOMClass SOMSTAR SOMLINK SOMTEnumNameEntryCNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_SOMTEnumNameEntryC (SOMTEnumNameEntryCClassData.classObject)
	#ifndef SOMGD_SOMTEnumNameEntryC
		#if (defined(_SOMTEnumNameEntryC) || defined(__SOMTEnumNameEntryC))
			#undef _SOMTEnumNameEntryC
			#undef __SOMTEnumNameEntryC
			#define SOMGD_SOMTEnumNameEntryC 1
		#else
			#define _SOMTEnumNameEntryC _SOMCLASS_SOMTEnumNameEntryC
		#endif /* _SOMTEnumNameEntryC */
	#endif /* SOMGD_SOMTEnumNameEntryC */
	#define SOMTEnumNameEntryC_classObj _SOMCLASS_SOMTEnumNameEntryC
	#define _SOMMTOKEN_SOMTEnumNameEntryC(method) ((somMToken)(SOMTEnumNameEntryCClassData.method))
	#ifndef SOMTEnumNameEntryCNew
		#define SOMTEnumNameEntryCNew() ( _SOMTEnumNameEntryC ? \
			(SOMClass_somNew(_SOMTEnumNameEntryC)) : \
			( SOMTEnumNameEntryCNewClass( \
				SOMTEnumNameEntryC_MajorVersion, \
				SOMTEnumNameEntryC_MinorVersion), \
			SOMClass_somNew(_SOMTEnumNameEntryC))) 
	#endif /* NewSOMTEnumNameEntryC */
	#ifndef SOMTEnumNameEntryC__get_somtEnumPtr
		#define SOMTEnumNameEntryC__get_somtEnumPtr(somSelf) \
			SOM_Resolve(somSelf,SOMTEnumNameEntryC,_get_somtEnumPtr)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtEnumPtr
				#if defined(__get_somtEnumPtr)
					#undef __get_somtEnumPtr
					#define SOMGD___get_somtEnumPtr
				#else
					#define __get_somtEnumPtr SOMTEnumNameEntryC__get_somtEnumPtr
				#endif
			#endif /* SOMGD___get_somtEnumPtr */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTEnumNameEntryC__get_somtEnumPtr */
	#ifndef SOMTEnumNameEntryC__get_somtEnumVal
		#define SOMTEnumNameEntryC__get_somtEnumVal(somSelf) \
			SOM_Resolve(somSelf,SOMTEnumNameEntryC,_get_somtEnumVal)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtEnumVal
				#if defined(__get_somtEnumVal)
					#undef __get_somtEnumVal
					#define SOMGD___get_somtEnumVal
				#else
					#define __get_somtEnumVal SOMTEnumNameEntryC__get_somtEnumVal
				#endif
			#endif /* SOMGD___get_somtEnumVal */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTEnumNameEntryC__get_somtEnumVal */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define SOMTEnumNameEntryC_somInit SOMObject_somInit
#define SOMTEnumNameEntryC_somUninit SOMObject_somUninit
#define SOMTEnumNameEntryC_somFree SOMObject_somFree
#define SOMTEnumNameEntryC_somGetClass SOMObject_somGetClass
#define SOMTEnumNameEntryC_somGetClassName SOMObject_somGetClassName
#define SOMTEnumNameEntryC_somGetSize SOMObject_somGetSize
#define SOMTEnumNameEntryC_somIsA SOMObject_somIsA
#define SOMTEnumNameEntryC_somIsInstanceOf SOMObject_somIsInstanceOf
#define SOMTEnumNameEntryC_somRespondsTo SOMObject_somRespondsTo
#define SOMTEnumNameEntryC_somDispatch SOMObject_somDispatch
#define SOMTEnumNameEntryC_somClassDispatch SOMObject_somClassDispatch
#define SOMTEnumNameEntryC_somCastObj SOMObject_somCastObj
#define SOMTEnumNameEntryC_somResetObj SOMObject_somResetObj
#define SOMTEnumNameEntryC_somPrintSelf SOMObject_somPrintSelf
#define SOMTEnumNameEntryC_somDumpSelf SOMObject_somDumpSelf
#define SOMTEnumNameEntryC_somDumpSelfInt SOMObject_somDumpSelfInt
#define SOMTEnumNameEntryC_somDefaultInit SOMObject_somDefaultInit
#define SOMTEnumNameEntryC_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define SOMTEnumNameEntryC_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define SOMTEnumNameEntryC_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define SOMTEnumNameEntryC_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define SOMTEnumNameEntryC_somDefaultAssign SOMObject_somDefaultAssign
#define SOMTEnumNameEntryC_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define SOMTEnumNameEntryC_somDefaultVAssign SOMObject_somDefaultVAssign
#define SOMTEnumNameEntryC_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define SOMTEnumNameEntryC_somDestruct SOMObject_somDestruct
#define SOMTEnumNameEntryC__get_somtEntryStruct SOMTEntryC__get_somtEntryStruct
#define SOMTEnumNameEntryC__set_somtEntryName SOMTEntryC__set_somtEntryName
#define SOMTEnumNameEntryC__get_somtEntryName SOMTEntryC__get_somtEntryName
#define SOMTEnumNameEntryC__set_somtElementType SOMTEntryC__set_somtElementType
#define SOMTEnumNameEntryC__get_somtElementType SOMTEntryC__get_somtElementType
#define SOMTEnumNameEntryC__get_somtElementTypeName SOMTEntryC__get_somtElementTypeName
#define SOMTEnumNameEntryC__get_somtEntryComment SOMTEntryC__get_somtEntryComment
#define SOMTEnumNameEntryC__get_somtSourceLineNumber SOMTEntryC__get_somtSourceLineNumber
#define SOMTEnumNameEntryC__get_somtTypeCode SOMTEntryC__get_somtTypeCode
#define SOMTEnumNameEntryC__get_somtIsReference SOMTEntryC__get_somtIsReference
#define SOMTEnumNameEntryC__get_somtIDLScopedName SOMTEntryC__get_somtIDLScopedName
#define SOMTEnumNameEntryC__get_somtCScopedName SOMTEntryC__get_somtCScopedName
#define SOMTEnumNameEntryC_somtGetModifierValue SOMTEntryC_somtGetModifierValue
#define SOMTEnumNameEntryC_somtGetFirstModifier SOMTEntryC_somtGetFirstModifier
#define SOMTEnumNameEntryC_somtGetNextModifier SOMTEntryC_somtGetNextModifier
#define SOMTEnumNameEntryC_somtFormatModifier SOMTEntryC_somtFormatModifier
#define SOMTEnumNameEntryC_somtGetModifierList SOMTEntryC_somtGetModifierList
#define SOMTEnumNameEntryC_somtSetSymbolsOnEntry SOMTEntryC_somtSetSymbolsOnEntry
#define SOMTEnumNameEntryC_somtSetEntryStruct SOMTEntryC_somtSetEntryStruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_scenumnm_Header_h */
