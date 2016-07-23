/* generated from scbase.idl */
/* internal conditional is SOM_Module_scbase_Source */
#ifndef SOM_Module_scbase_Header_h
	#define SOM_Module_scbase_Header_h 1
	#include <som.h>
	#include <scentry.h>
	#include <somobj.h>
	#include <sctypes.h>
	#include <sm.h>
	#include <somstrt.h>
	#ifndef _IDL_SOMTClassEntryC_defined
		#define _IDL_SOMTClassEntryC_defined
		typedef SOMObject SOMTClassEntryC;
	#endif /* _IDL_SOMTClassEntryC_defined */
	#ifndef _IDL_SEQUENCE_SOMTClassEntryC_defined
		#define _IDL_SEQUENCE_SOMTClassEntryC_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMTClassEntryC SOMSTAR ,sequence(SOMTClassEntryC));
	#endif /* _IDL_SEQUENCE_SOMTClassEntryC_defined */
	#ifndef _IDL_SOMTBaseClassEntryC_defined
		#define _IDL_SOMTBaseClassEntryC_defined
		typedef SOMObject SOMTBaseClassEntryC;
	#endif /* _IDL_SOMTBaseClassEntryC_defined */
	#ifndef _IDL_SEQUENCE_SOMTBaseClassEntryC_defined
		#define _IDL_SEQUENCE_SOMTBaseClassEntryC_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMTBaseClassEntryC SOMSTAR ,sequence(SOMTBaseClassEntryC));
	#endif /* _IDL_SEQUENCE_SOMTBaseClassEntryC_defined */
	#ifdef __IBMC__
		typedef SOMTClassEntryC SOMSTAR (somTP_SOMTBaseClassEntryC__get_somtBaseClassDef)(
			SOMTBaseClassEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTBaseClassEntryC__get_somtBaseClassDef,system)
		typedef somTP_SOMTBaseClassEntryC__get_somtBaseClassDef *somTD_SOMTBaseClassEntryC__get_somtBaseClassDef;
	#else /* __IBMC__ */
		typedef SOMTClassEntryC SOMSTAR (SOMLINK * somTD_SOMTBaseClassEntryC__get_somtBaseClassDef)(
			SOMTBaseClassEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifndef SOMTBaseClassEntryC_MajorVersion
		#define SOMTBaseClassEntryC_MajorVersion   2
	#endif /* SOMTBaseClassEntryC_MajorVersion */
	#ifndef SOMTBaseClassEntryC_MinorVersion
		#define SOMTBaseClassEntryC_MinorVersion   1
	#endif /* SOMTBaseClassEntryC_MinorVersion */
	typedef struct SOMTBaseClassEntryCClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken _get_somtBaseClassDef;
	} SOMTBaseClassEntryCClassDataStructure;
	typedef struct SOMTBaseClassEntryCCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} SOMTBaseClassEntryCCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_scbase_Source) || defined(SOMTBaseClassEntryC_Class_Source)
			SOMEXTERN struct SOMTBaseClassEntryCClassDataStructure _SOMTBaseClassEntryCClassData;
			#ifndef SOMTBaseClassEntryCClassData
				#define SOMTBaseClassEntryCClassData    _SOMTBaseClassEntryCClassData
			#endif /* SOMTBaseClassEntryCClassData */
		#else
			SOMEXTERN struct SOMTBaseClassEntryCClassDataStructure * SOMLINK resolve_SOMTBaseClassEntryCClassData(void);
			#ifndef SOMTBaseClassEntryCClassData
				#define SOMTBaseClassEntryCClassData    (*(resolve_SOMTBaseClassEntryCClassData()))
			#endif /* SOMTBaseClassEntryCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_scbase_Source) || defined(SOMTBaseClassEntryC_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_scbase_Source || SOMTBaseClassEntryC_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_scbase_Source || SOMTBaseClassEntryC_Class_Source */
		struct SOMTBaseClassEntryCClassDataStructure SOMDLINK SOMTBaseClassEntryCClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_scbase_Source) || defined(SOMTBaseClassEntryC_Class_Source)
			SOMEXTERN struct SOMTBaseClassEntryCCClassDataStructure _SOMTBaseClassEntryCCClassData;
			#ifndef SOMTBaseClassEntryCCClassData
				#define SOMTBaseClassEntryCCClassData    _SOMTBaseClassEntryCCClassData
			#endif /* SOMTBaseClassEntryCCClassData */
		#else
			SOMEXTERN struct SOMTBaseClassEntryCCClassDataStructure * SOMLINK resolve_SOMTBaseClassEntryCCClassData(void);
			#ifndef SOMTBaseClassEntryCCClassData
				#define SOMTBaseClassEntryCCClassData    (*(resolve_SOMTBaseClassEntryCCClassData()))
			#endif /* SOMTBaseClassEntryCCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_scbase_Source) || defined(SOMTBaseClassEntryC_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_scbase_Source || SOMTBaseClassEntryC_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_scbase_Source || SOMTBaseClassEntryC_Class_Source */
		struct SOMTBaseClassEntryCCClassDataStructure SOMDLINK SOMTBaseClassEntryCCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_scbase_Source) || defined(SOMTBaseClassEntryC_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_scbase_Source || SOMTBaseClassEntryC_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_scbase_Source || SOMTBaseClassEntryC_Class_Source */
	SOMClass SOMSTAR SOMLINK SOMTBaseClassEntryCNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_SOMTBaseClassEntryC (SOMTBaseClassEntryCClassData.classObject)
	#ifndef SOMGD_SOMTBaseClassEntryC
		#if (defined(_SOMTBaseClassEntryC) || defined(__SOMTBaseClassEntryC))
			#undef _SOMTBaseClassEntryC
			#undef __SOMTBaseClassEntryC
			#define SOMGD_SOMTBaseClassEntryC 1
		#else
			#define _SOMTBaseClassEntryC _SOMCLASS_SOMTBaseClassEntryC
		#endif /* _SOMTBaseClassEntryC */
	#endif /* SOMGD_SOMTBaseClassEntryC */
	#define SOMTBaseClassEntryC_classObj _SOMCLASS_SOMTBaseClassEntryC
	#define _SOMMTOKEN_SOMTBaseClassEntryC(method) ((somMToken)(SOMTBaseClassEntryCClassData.method))
	#ifndef SOMTBaseClassEntryCNew
		#define SOMTBaseClassEntryCNew() ( _SOMTBaseClassEntryC ? \
			(SOMClass_somNew(_SOMTBaseClassEntryC)) : \
			( SOMTBaseClassEntryCNewClass( \
				SOMTBaseClassEntryC_MajorVersion, \
				SOMTBaseClassEntryC_MinorVersion), \
			SOMClass_somNew(_SOMTBaseClassEntryC))) 
	#endif /* NewSOMTBaseClassEntryC */
	#ifndef SOMTBaseClassEntryC__get_somtBaseClassDef
		#define SOMTBaseClassEntryC__get_somtBaseClassDef(somSelf) \
			SOM_Resolve(somSelf,SOMTBaseClassEntryC,_get_somtBaseClassDef)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtBaseClassDef
				#if defined(__get_somtBaseClassDef)
					#undef __get_somtBaseClassDef
					#define SOMGD___get_somtBaseClassDef
				#else
					#define __get_somtBaseClassDef SOMTBaseClassEntryC__get_somtBaseClassDef
				#endif
			#endif /* SOMGD___get_somtBaseClassDef */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTBaseClassEntryC__get_somtBaseClassDef */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define SOMTBaseClassEntryC_somInit SOMObject_somInit
#define SOMTBaseClassEntryC_somUninit SOMObject_somUninit
#define SOMTBaseClassEntryC_somFree SOMObject_somFree
#define SOMTBaseClassEntryC_somGetClass SOMObject_somGetClass
#define SOMTBaseClassEntryC_somGetClassName SOMObject_somGetClassName
#define SOMTBaseClassEntryC_somGetSize SOMObject_somGetSize
#define SOMTBaseClassEntryC_somIsA SOMObject_somIsA
#define SOMTBaseClassEntryC_somIsInstanceOf SOMObject_somIsInstanceOf
#define SOMTBaseClassEntryC_somRespondsTo SOMObject_somRespondsTo
#define SOMTBaseClassEntryC_somDispatch SOMObject_somDispatch
#define SOMTBaseClassEntryC_somClassDispatch SOMObject_somClassDispatch
#define SOMTBaseClassEntryC_somCastObj SOMObject_somCastObj
#define SOMTBaseClassEntryC_somResetObj SOMObject_somResetObj
#define SOMTBaseClassEntryC_somPrintSelf SOMObject_somPrintSelf
#define SOMTBaseClassEntryC_somDumpSelf SOMObject_somDumpSelf
#define SOMTBaseClassEntryC_somDumpSelfInt SOMObject_somDumpSelfInt
#define SOMTBaseClassEntryC_somDefaultInit SOMObject_somDefaultInit
#define SOMTBaseClassEntryC_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define SOMTBaseClassEntryC_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define SOMTBaseClassEntryC_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define SOMTBaseClassEntryC_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define SOMTBaseClassEntryC_somDefaultAssign SOMObject_somDefaultAssign
#define SOMTBaseClassEntryC_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define SOMTBaseClassEntryC_somDefaultVAssign SOMObject_somDefaultVAssign
#define SOMTBaseClassEntryC_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define SOMTBaseClassEntryC_somDestruct SOMObject_somDestruct
#define SOMTBaseClassEntryC__get_somtEntryStruct SOMTEntryC__get_somtEntryStruct
#define SOMTBaseClassEntryC__set_somtEntryName SOMTEntryC__set_somtEntryName
#define SOMTBaseClassEntryC__get_somtEntryName SOMTEntryC__get_somtEntryName
#define SOMTBaseClassEntryC__set_somtElementType SOMTEntryC__set_somtElementType
#define SOMTBaseClassEntryC__get_somtElementType SOMTEntryC__get_somtElementType
#define SOMTBaseClassEntryC__get_somtElementTypeName SOMTEntryC__get_somtElementTypeName
#define SOMTBaseClassEntryC__get_somtEntryComment SOMTEntryC__get_somtEntryComment
#define SOMTBaseClassEntryC__get_somtSourceLineNumber SOMTEntryC__get_somtSourceLineNumber
#define SOMTBaseClassEntryC__get_somtTypeCode SOMTEntryC__get_somtTypeCode
#define SOMTBaseClassEntryC__get_somtIsReference SOMTEntryC__get_somtIsReference
#define SOMTBaseClassEntryC__get_somtIDLScopedName SOMTEntryC__get_somtIDLScopedName
#define SOMTBaseClassEntryC__get_somtCScopedName SOMTEntryC__get_somtCScopedName
#define SOMTBaseClassEntryC_somtGetModifierValue SOMTEntryC_somtGetModifierValue
#define SOMTBaseClassEntryC_somtGetFirstModifier SOMTEntryC_somtGetFirstModifier
#define SOMTBaseClassEntryC_somtGetNextModifier SOMTEntryC_somtGetNextModifier
#define SOMTBaseClassEntryC_somtFormatModifier SOMTEntryC_somtFormatModifier
#define SOMTBaseClassEntryC_somtGetModifierList SOMTEntryC_somtGetModifierList
#define SOMTBaseClassEntryC_somtSetSymbolsOnEntry SOMTEntryC_somtSetSymbolsOnEntry
#define SOMTBaseClassEntryC_somtSetEntryStruct SOMTEntryC_somtSetEntryStruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_scbase_Header_h */
