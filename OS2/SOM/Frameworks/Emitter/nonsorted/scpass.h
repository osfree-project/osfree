/* generated from scpass.idl */
/* internal conditional is SOM_Module_scpass_Source */
#ifndef SOM_Module_scpass_Header_h
	#define SOM_Module_scpass_Header_h 1
	#include <som.h>
	#include <scentry.h>
	#include <somobj.h>
	#include <sctypes.h>
	#include <sm.h>
	#include <somstrt.h>
	#ifndef _IDL_SOMTPassthruEntryC_defined
		#define _IDL_SOMTPassthruEntryC_defined
		typedef SOMObject SOMTPassthruEntryC;
	#endif /* _IDL_SOMTPassthruEntryC_defined */
	#ifndef _IDL_SEQUENCE_SOMTPassthruEntryC_defined
		#define _IDL_SEQUENCE_SOMTPassthruEntryC_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMTPassthruEntryC SOMSTAR ,sequence(SOMTPassthruEntryC));
	#endif /* _IDL_SEQUENCE_SOMTPassthruEntryC_defined */
	#ifdef __IBMC__
		typedef string (somTP_SOMTPassthruEntryC__get_somtPassthruBody)(
			SOMTPassthruEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTPassthruEntryC__get_somtPassthruBody,system)
		typedef somTP_SOMTPassthruEntryC__get_somtPassthruBody *somTD_SOMTPassthruEntryC__get_somtPassthruBody;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_SOMTPassthruEntryC__get_somtPassthruBody)(
			SOMTPassthruEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_SOMTPassthruEntryC__get_somtPassthruLanguage)(
			SOMTPassthruEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTPassthruEntryC__get_somtPassthruLanguage,system)
		typedef somTP_SOMTPassthruEntryC__get_somtPassthruLanguage *somTD_SOMTPassthruEntryC__get_somtPassthruLanguage;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_SOMTPassthruEntryC__get_somtPassthruLanguage)(
			SOMTPassthruEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_SOMTPassthruEntryC__get_somtPassthruTarget)(
			SOMTPassthruEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTPassthruEntryC__get_somtPassthruTarget,system)
		typedef somTP_SOMTPassthruEntryC__get_somtPassthruTarget *somTD_SOMTPassthruEntryC__get_somtPassthruTarget;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_SOMTPassthruEntryC__get_somtPassthruTarget)(
			SOMTPassthruEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef boolean (somTP_SOMTPassthruEntryC_somtIsBeforePassthru)(
			SOMTPassthruEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTPassthruEntryC_somtIsBeforePassthru,system)
		typedef somTP_SOMTPassthruEntryC_somtIsBeforePassthru *somTD_SOMTPassthruEntryC_somtIsBeforePassthru;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_SOMTPassthruEntryC_somtIsBeforePassthru)(
			SOMTPassthruEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifndef SOMTPassthruEntryC_MajorVersion
		#define SOMTPassthruEntryC_MajorVersion   2
	#endif /* SOMTPassthruEntryC_MajorVersion */
	#ifndef SOMTPassthruEntryC_MinorVersion
		#define SOMTPassthruEntryC_MinorVersion   1
	#endif /* SOMTPassthruEntryC_MinorVersion */
	typedef struct SOMTPassthruEntryCClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken _get_somtPassthruBody;
		somMToken _get_somtPassthruTarget;
		somMToken _get_somtPassthruLanguage;
		somMToken somtIsBeforePassthru;
	} SOMTPassthruEntryCClassDataStructure;
	typedef struct SOMTPassthruEntryCCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} SOMTPassthruEntryCCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_scpass_Source) || defined(SOMTPassthruEntryC_Class_Source)
			SOMEXTERN struct SOMTPassthruEntryCClassDataStructure _SOMTPassthruEntryCClassData;
			#ifndef SOMTPassthruEntryCClassData
				#define SOMTPassthruEntryCClassData    _SOMTPassthruEntryCClassData
			#endif /* SOMTPassthruEntryCClassData */
		#else
			SOMEXTERN struct SOMTPassthruEntryCClassDataStructure * SOMLINK resolve_SOMTPassthruEntryCClassData(void);
			#ifndef SOMTPassthruEntryCClassData
				#define SOMTPassthruEntryCClassData    (*(resolve_SOMTPassthruEntryCClassData()))
			#endif /* SOMTPassthruEntryCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_scpass_Source) || defined(SOMTPassthruEntryC_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_scpass_Source || SOMTPassthruEntryC_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_scpass_Source || SOMTPassthruEntryC_Class_Source */
		struct SOMTPassthruEntryCClassDataStructure SOMDLINK SOMTPassthruEntryCClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_scpass_Source) || defined(SOMTPassthruEntryC_Class_Source)
			SOMEXTERN struct SOMTPassthruEntryCCClassDataStructure _SOMTPassthruEntryCCClassData;
			#ifndef SOMTPassthruEntryCCClassData
				#define SOMTPassthruEntryCCClassData    _SOMTPassthruEntryCCClassData
			#endif /* SOMTPassthruEntryCCClassData */
		#else
			SOMEXTERN struct SOMTPassthruEntryCCClassDataStructure * SOMLINK resolve_SOMTPassthruEntryCCClassData(void);
			#ifndef SOMTPassthruEntryCCClassData
				#define SOMTPassthruEntryCCClassData    (*(resolve_SOMTPassthruEntryCCClassData()))
			#endif /* SOMTPassthruEntryCCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_scpass_Source) || defined(SOMTPassthruEntryC_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_scpass_Source || SOMTPassthruEntryC_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_scpass_Source || SOMTPassthruEntryC_Class_Source */
		struct SOMTPassthruEntryCCClassDataStructure SOMDLINK SOMTPassthruEntryCCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_scpass_Source) || defined(SOMTPassthruEntryC_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_scpass_Source || SOMTPassthruEntryC_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_scpass_Source || SOMTPassthruEntryC_Class_Source */
	SOMClass SOMSTAR SOMLINK SOMTPassthruEntryCNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_SOMTPassthruEntryC (SOMTPassthruEntryCClassData.classObject)
	#ifndef SOMGD_SOMTPassthruEntryC
		#if (defined(_SOMTPassthruEntryC) || defined(__SOMTPassthruEntryC))
			#undef _SOMTPassthruEntryC
			#undef __SOMTPassthruEntryC
			#define SOMGD_SOMTPassthruEntryC 1
		#else
			#define _SOMTPassthruEntryC _SOMCLASS_SOMTPassthruEntryC
		#endif /* _SOMTPassthruEntryC */
	#endif /* SOMGD_SOMTPassthruEntryC */
	#define SOMTPassthruEntryC_classObj _SOMCLASS_SOMTPassthruEntryC
	#define _SOMMTOKEN_SOMTPassthruEntryC(method) ((somMToken)(SOMTPassthruEntryCClassData.method))
	#ifndef SOMTPassthruEntryCNew
		#define SOMTPassthruEntryCNew() ( _SOMTPassthruEntryC ? \
			(SOMClass_somNew(_SOMTPassthruEntryC)) : \
			( SOMTPassthruEntryCNewClass( \
				SOMTPassthruEntryC_MajorVersion, \
				SOMTPassthruEntryC_MinorVersion), \
			SOMClass_somNew(_SOMTPassthruEntryC))) 
	#endif /* NewSOMTPassthruEntryC */
	#ifndef SOMTPassthruEntryC__get_somtPassthruBody
		#define SOMTPassthruEntryC__get_somtPassthruBody(somSelf) \
			SOM_Resolve(somSelf,SOMTPassthruEntryC,_get_somtPassthruBody)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtPassthruBody
				#if defined(__get_somtPassthruBody)
					#undef __get_somtPassthruBody
					#define SOMGD___get_somtPassthruBody
				#else
					#define __get_somtPassthruBody SOMTPassthruEntryC__get_somtPassthruBody
				#endif
			#endif /* SOMGD___get_somtPassthruBody */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTPassthruEntryC__get_somtPassthruBody */
	#ifndef SOMTPassthruEntryC__get_somtPassthruLanguage
		#define SOMTPassthruEntryC__get_somtPassthruLanguage(somSelf) \
			SOM_Resolve(somSelf,SOMTPassthruEntryC,_get_somtPassthruLanguage)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtPassthruLanguage
				#if defined(__get_somtPassthruLanguage)
					#undef __get_somtPassthruLanguage
					#define SOMGD___get_somtPassthruLanguage
				#else
					#define __get_somtPassthruLanguage SOMTPassthruEntryC__get_somtPassthruLanguage
				#endif
			#endif /* SOMGD___get_somtPassthruLanguage */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTPassthruEntryC__get_somtPassthruLanguage */
	#ifndef SOMTPassthruEntryC__get_somtPassthruTarget
		#define SOMTPassthruEntryC__get_somtPassthruTarget(somSelf) \
			SOM_Resolve(somSelf,SOMTPassthruEntryC,_get_somtPassthruTarget)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtPassthruTarget
				#if defined(__get_somtPassthruTarget)
					#undef __get_somtPassthruTarget
					#define SOMGD___get_somtPassthruTarget
				#else
					#define __get_somtPassthruTarget SOMTPassthruEntryC__get_somtPassthruTarget
				#endif
			#endif /* SOMGD___get_somtPassthruTarget */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTPassthruEntryC__get_somtPassthruTarget */
	#ifndef SOMTPassthruEntryC_somtIsBeforePassthru
		#define SOMTPassthruEntryC_somtIsBeforePassthru(somSelf) \
			SOM_Resolve(somSelf,SOMTPassthruEntryC,somtIsBeforePassthru)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtIsBeforePassthru
				#if defined(_somtIsBeforePassthru)
					#undef _somtIsBeforePassthru
					#define SOMGD__somtIsBeforePassthru
				#else
					#define _somtIsBeforePassthru SOMTPassthruEntryC_somtIsBeforePassthru
				#endif
			#endif /* SOMGD__somtIsBeforePassthru */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTPassthruEntryC_somtIsBeforePassthru */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define SOMTPassthruEntryC_somInit SOMObject_somInit
#define SOMTPassthruEntryC_somUninit SOMObject_somUninit
#define SOMTPassthruEntryC_somFree SOMObject_somFree
#define SOMTPassthruEntryC_somGetClass SOMObject_somGetClass
#define SOMTPassthruEntryC_somGetClassName SOMObject_somGetClassName
#define SOMTPassthruEntryC_somGetSize SOMObject_somGetSize
#define SOMTPassthruEntryC_somIsA SOMObject_somIsA
#define SOMTPassthruEntryC_somIsInstanceOf SOMObject_somIsInstanceOf
#define SOMTPassthruEntryC_somRespondsTo SOMObject_somRespondsTo
#define SOMTPassthruEntryC_somDispatch SOMObject_somDispatch
#define SOMTPassthruEntryC_somClassDispatch SOMObject_somClassDispatch
#define SOMTPassthruEntryC_somCastObj SOMObject_somCastObj
#define SOMTPassthruEntryC_somResetObj SOMObject_somResetObj
#define SOMTPassthruEntryC_somPrintSelf SOMObject_somPrintSelf
#define SOMTPassthruEntryC_somDumpSelf SOMObject_somDumpSelf
#define SOMTPassthruEntryC_somDumpSelfInt SOMObject_somDumpSelfInt
#define SOMTPassthruEntryC_somDefaultInit SOMObject_somDefaultInit
#define SOMTPassthruEntryC_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define SOMTPassthruEntryC_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define SOMTPassthruEntryC_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define SOMTPassthruEntryC_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define SOMTPassthruEntryC_somDefaultAssign SOMObject_somDefaultAssign
#define SOMTPassthruEntryC_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define SOMTPassthruEntryC_somDefaultVAssign SOMObject_somDefaultVAssign
#define SOMTPassthruEntryC_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define SOMTPassthruEntryC_somDestruct SOMObject_somDestruct
#define SOMTPassthruEntryC__get_somtEntryStruct SOMTEntryC__get_somtEntryStruct
#define SOMTPassthruEntryC__set_somtEntryName SOMTEntryC__set_somtEntryName
#define SOMTPassthruEntryC__get_somtEntryName SOMTEntryC__get_somtEntryName
#define SOMTPassthruEntryC__set_somtElementType SOMTEntryC__set_somtElementType
#define SOMTPassthruEntryC__get_somtElementType SOMTEntryC__get_somtElementType
#define SOMTPassthruEntryC__get_somtElementTypeName SOMTEntryC__get_somtElementTypeName
#define SOMTPassthruEntryC__get_somtEntryComment SOMTEntryC__get_somtEntryComment
#define SOMTPassthruEntryC__get_somtSourceLineNumber SOMTEntryC__get_somtSourceLineNumber
#define SOMTPassthruEntryC__get_somtTypeCode SOMTEntryC__get_somtTypeCode
#define SOMTPassthruEntryC__get_somtIsReference SOMTEntryC__get_somtIsReference
#define SOMTPassthruEntryC__get_somtIDLScopedName SOMTEntryC__get_somtIDLScopedName
#define SOMTPassthruEntryC__get_somtCScopedName SOMTEntryC__get_somtCScopedName
#define SOMTPassthruEntryC_somtGetModifierValue SOMTEntryC_somtGetModifierValue
#define SOMTPassthruEntryC_somtGetFirstModifier SOMTEntryC_somtGetFirstModifier
#define SOMTPassthruEntryC_somtGetNextModifier SOMTEntryC_somtGetNextModifier
#define SOMTPassthruEntryC_somtFormatModifier SOMTEntryC_somtFormatModifier
#define SOMTPassthruEntryC_somtGetModifierList SOMTEntryC_somtGetModifierList
#define SOMTPassthruEntryC_somtSetSymbolsOnEntry SOMTEntryC_somtSetSymbolsOnEntry
#define SOMTPassthruEntryC_somtSetEntryStruct SOMTEntryC_somtSetEntryStruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_scpass_Header_h */
