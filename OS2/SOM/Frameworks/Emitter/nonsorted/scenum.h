/* generated from scenum.idl */
/* internal conditional is SOM_Module_scenum_Source */
#ifndef SOM_Module_scenum_Header_h
	#define SOM_Module_scenum_Header_h 1
	#include <som.h>
	#include <scentry.h>
	#include <somobj.h>
	#include <sctypes.h>
	#include <sm.h>
	#include <somstrt.h>
	#ifndef _IDL_SOMTEnumNameEntryC_defined
		#define _IDL_SOMTEnumNameEntryC_defined
		typedef SOMObject SOMTEnumNameEntryC;
	#endif /* _IDL_SOMTEnumNameEntryC_defined */
	#ifndef _IDL_SEQUENCE_SOMTEnumNameEntryC_defined
		#define _IDL_SEQUENCE_SOMTEnumNameEntryC_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMTEnumNameEntryC SOMSTAR ,sequence(SOMTEnumNameEntryC));
	#endif /* _IDL_SEQUENCE_SOMTEnumNameEntryC_defined */
	#ifndef _IDL_SOMTEnumEntryC_defined
		#define _IDL_SOMTEnumEntryC_defined
		typedef SOMObject SOMTEnumEntryC;
	#endif /* _IDL_SOMTEnumEntryC_defined */
	#ifndef _IDL_SEQUENCE_SOMTEnumEntryC_defined
		#define _IDL_SEQUENCE_SOMTEnumEntryC_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMTEnumEntryC SOMSTAR ,sequence(SOMTEnumEntryC));
	#endif /* _IDL_SEQUENCE_SOMTEnumEntryC_defined */
	#ifdef __IBMC__
		typedef SOMTEnumNameEntryC SOMSTAR (somTP_SOMTEnumEntryC_somtGetFirstEnumName)(
			SOMTEnumEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTEnumEntryC_somtGetFirstEnumName,system)
		typedef somTP_SOMTEnumEntryC_somtGetFirstEnumName *somTD_SOMTEnumEntryC_somtGetFirstEnumName;
	#else /* __IBMC__ */
		typedef SOMTEnumNameEntryC SOMSTAR (SOMLINK * somTD_SOMTEnumEntryC_somtGetFirstEnumName)(
			SOMTEnumEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMTEnumNameEntryC SOMSTAR (somTP_SOMTEnumEntryC_somtGetNextEnumName)(
			SOMTEnumEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTEnumEntryC_somtGetNextEnumName,system)
		typedef somTP_SOMTEnumEntryC_somtGetNextEnumName *somTD_SOMTEnumEntryC_somtGetNextEnumName;
	#else /* __IBMC__ */
		typedef SOMTEnumNameEntryC SOMSTAR (SOMLINK * somTD_SOMTEnumEntryC_somtGetNextEnumName)(
			SOMTEnumEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifndef SOMTEnumEntryC_MajorVersion
		#define SOMTEnumEntryC_MajorVersion   2
	#endif /* SOMTEnumEntryC_MajorVersion */
	#ifndef SOMTEnumEntryC_MinorVersion
		#define SOMTEnumEntryC_MinorVersion   1
	#endif /* SOMTEnumEntryC_MinorVersion */
	typedef struct SOMTEnumEntryCClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken somtGetFirstEnumName;
		somMToken somtGetNextEnumName;
	} SOMTEnumEntryCClassDataStructure;
	typedef struct SOMTEnumEntryCCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} SOMTEnumEntryCCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_scenum_Source) || defined(SOMTEnumEntryC_Class_Source)
			SOMEXTERN struct SOMTEnumEntryCClassDataStructure _SOMTEnumEntryCClassData;
			#ifndef SOMTEnumEntryCClassData
				#define SOMTEnumEntryCClassData    _SOMTEnumEntryCClassData
			#endif /* SOMTEnumEntryCClassData */
		#else
			SOMEXTERN struct SOMTEnumEntryCClassDataStructure * SOMLINK resolve_SOMTEnumEntryCClassData(void);
			#ifndef SOMTEnumEntryCClassData
				#define SOMTEnumEntryCClassData    (*(resolve_SOMTEnumEntryCClassData()))
			#endif /* SOMTEnumEntryCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_scenum_Source) || defined(SOMTEnumEntryC_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_scenum_Source || SOMTEnumEntryC_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_scenum_Source || SOMTEnumEntryC_Class_Source */
		struct SOMTEnumEntryCClassDataStructure SOMDLINK SOMTEnumEntryCClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_scenum_Source) || defined(SOMTEnumEntryC_Class_Source)
			SOMEXTERN struct SOMTEnumEntryCCClassDataStructure _SOMTEnumEntryCCClassData;
			#ifndef SOMTEnumEntryCCClassData
				#define SOMTEnumEntryCCClassData    _SOMTEnumEntryCCClassData
			#endif /* SOMTEnumEntryCCClassData */
		#else
			SOMEXTERN struct SOMTEnumEntryCCClassDataStructure * SOMLINK resolve_SOMTEnumEntryCCClassData(void);
			#ifndef SOMTEnumEntryCCClassData
				#define SOMTEnumEntryCCClassData    (*(resolve_SOMTEnumEntryCCClassData()))
			#endif /* SOMTEnumEntryCCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_scenum_Source) || defined(SOMTEnumEntryC_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_scenum_Source || SOMTEnumEntryC_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_scenum_Source || SOMTEnumEntryC_Class_Source */
		struct SOMTEnumEntryCCClassDataStructure SOMDLINK SOMTEnumEntryCCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_scenum_Source) || defined(SOMTEnumEntryC_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_scenum_Source || SOMTEnumEntryC_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_scenum_Source || SOMTEnumEntryC_Class_Source */
	SOMClass SOMSTAR SOMLINK SOMTEnumEntryCNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_SOMTEnumEntryC (SOMTEnumEntryCClassData.classObject)
	#ifndef SOMGD_SOMTEnumEntryC
		#if (defined(_SOMTEnumEntryC) || defined(__SOMTEnumEntryC))
			#undef _SOMTEnumEntryC
			#undef __SOMTEnumEntryC
			#define SOMGD_SOMTEnumEntryC 1
		#else
			#define _SOMTEnumEntryC _SOMCLASS_SOMTEnumEntryC
		#endif /* _SOMTEnumEntryC */
	#endif /* SOMGD_SOMTEnumEntryC */
	#define SOMTEnumEntryC_classObj _SOMCLASS_SOMTEnumEntryC
	#define _SOMMTOKEN_SOMTEnumEntryC(method) ((somMToken)(SOMTEnumEntryCClassData.method))
	#ifndef SOMTEnumEntryCNew
		#define SOMTEnumEntryCNew() ( _SOMTEnumEntryC ? \
			(SOMClass_somNew(_SOMTEnumEntryC)) : \
			( SOMTEnumEntryCNewClass( \
				SOMTEnumEntryC_MajorVersion, \
				SOMTEnumEntryC_MinorVersion), \
			SOMClass_somNew(_SOMTEnumEntryC))) 
	#endif /* NewSOMTEnumEntryC */
	#ifndef SOMTEnumEntryC_somtGetFirstEnumName
		#define SOMTEnumEntryC_somtGetFirstEnumName(somSelf) \
			SOM_Resolve(somSelf,SOMTEnumEntryC,somtGetFirstEnumName)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetFirstEnumName
				#if defined(_somtGetFirstEnumName)
					#undef _somtGetFirstEnumName
					#define SOMGD__somtGetFirstEnumName
				#else
					#define _somtGetFirstEnumName SOMTEnumEntryC_somtGetFirstEnumName
				#endif
			#endif /* SOMGD__somtGetFirstEnumName */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTEnumEntryC_somtGetFirstEnumName */
	#ifndef SOMTEnumEntryC_somtGetNextEnumName
		#define SOMTEnumEntryC_somtGetNextEnumName(somSelf) \
			SOM_Resolve(somSelf,SOMTEnumEntryC,somtGetNextEnumName)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetNextEnumName
				#if defined(_somtGetNextEnumName)
					#undef _somtGetNextEnumName
					#define SOMGD__somtGetNextEnumName
				#else
					#define _somtGetNextEnumName SOMTEnumEntryC_somtGetNextEnumName
				#endif
			#endif /* SOMGD__somtGetNextEnumName */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTEnumEntryC_somtGetNextEnumName */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define SOMTEnumEntryC_somInit SOMObject_somInit
#define SOMTEnumEntryC_somUninit SOMObject_somUninit
#define SOMTEnumEntryC_somFree SOMObject_somFree
#define SOMTEnumEntryC_somGetClass SOMObject_somGetClass
#define SOMTEnumEntryC_somGetClassName SOMObject_somGetClassName
#define SOMTEnumEntryC_somGetSize SOMObject_somGetSize
#define SOMTEnumEntryC_somIsA SOMObject_somIsA
#define SOMTEnumEntryC_somIsInstanceOf SOMObject_somIsInstanceOf
#define SOMTEnumEntryC_somRespondsTo SOMObject_somRespondsTo
#define SOMTEnumEntryC_somDispatch SOMObject_somDispatch
#define SOMTEnumEntryC_somClassDispatch SOMObject_somClassDispatch
#define SOMTEnumEntryC_somCastObj SOMObject_somCastObj
#define SOMTEnumEntryC_somResetObj SOMObject_somResetObj
#define SOMTEnumEntryC_somPrintSelf SOMObject_somPrintSelf
#define SOMTEnumEntryC_somDumpSelf SOMObject_somDumpSelf
#define SOMTEnumEntryC_somDumpSelfInt SOMObject_somDumpSelfInt
#define SOMTEnumEntryC_somDefaultInit SOMObject_somDefaultInit
#define SOMTEnumEntryC_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define SOMTEnumEntryC_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define SOMTEnumEntryC_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define SOMTEnumEntryC_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define SOMTEnumEntryC_somDefaultAssign SOMObject_somDefaultAssign
#define SOMTEnumEntryC_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define SOMTEnumEntryC_somDefaultVAssign SOMObject_somDefaultVAssign
#define SOMTEnumEntryC_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define SOMTEnumEntryC_somDestruct SOMObject_somDestruct
#define SOMTEnumEntryC__get_somtEntryStruct SOMTEntryC__get_somtEntryStruct
#define SOMTEnumEntryC__set_somtEntryName SOMTEntryC__set_somtEntryName
#define SOMTEnumEntryC__get_somtEntryName SOMTEntryC__get_somtEntryName
#define SOMTEnumEntryC__set_somtElementType SOMTEntryC__set_somtElementType
#define SOMTEnumEntryC__get_somtElementType SOMTEntryC__get_somtElementType
#define SOMTEnumEntryC__get_somtElementTypeName SOMTEntryC__get_somtElementTypeName
#define SOMTEnumEntryC__get_somtEntryComment SOMTEntryC__get_somtEntryComment
#define SOMTEnumEntryC__get_somtSourceLineNumber SOMTEntryC__get_somtSourceLineNumber
#define SOMTEnumEntryC__get_somtTypeCode SOMTEntryC__get_somtTypeCode
#define SOMTEnumEntryC__get_somtIsReference SOMTEntryC__get_somtIsReference
#define SOMTEnumEntryC__get_somtIDLScopedName SOMTEntryC__get_somtIDLScopedName
#define SOMTEnumEntryC__get_somtCScopedName SOMTEntryC__get_somtCScopedName
#define SOMTEnumEntryC_somtGetModifierValue SOMTEntryC_somtGetModifierValue
#define SOMTEnumEntryC_somtGetFirstModifier SOMTEntryC_somtGetFirstModifier
#define SOMTEnumEntryC_somtGetNextModifier SOMTEntryC_somtGetNextModifier
#define SOMTEnumEntryC_somtFormatModifier SOMTEntryC_somtFormatModifier
#define SOMTEnumEntryC_somtGetModifierList SOMTEntryC_somtGetModifierList
#define SOMTEnumEntryC_somtSetSymbolsOnEntry SOMTEntryC_somtSetSymbolsOnEntry
#define SOMTEnumEntryC_somtSetEntryStruct SOMTEntryC_somtSetEntryStruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_scenum_Header_h */
