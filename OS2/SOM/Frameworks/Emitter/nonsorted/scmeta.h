/* generated from scmeta.idl */
/* internal conditional is SOM_Module_scmeta_Source */
#ifndef SOM_Module_scmeta_Header_h
	#define SOM_Module_scmeta_Header_h 1
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
	#ifndef _IDL_SOMTMetaClassEntryC_defined
		#define _IDL_SOMTMetaClassEntryC_defined
		typedef SOMObject SOMTMetaClassEntryC;
	#endif /* _IDL_SOMTMetaClassEntryC_defined */
	#ifndef _IDL_SEQUENCE_SOMTMetaClassEntryC_defined
		#define _IDL_SEQUENCE_SOMTMetaClassEntryC_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMTMetaClassEntryC SOMSTAR ,sequence(SOMTMetaClassEntryC));
	#endif /* _IDL_SEQUENCE_SOMTMetaClassEntryC_defined */
	#ifdef __IBMC__
		typedef string (somTP_SOMTMetaClassEntryC__get_somtMetaFile)(
			SOMTMetaClassEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTMetaClassEntryC__get_somtMetaFile,system)
		typedef somTP_SOMTMetaClassEntryC__get_somtMetaFile *somTD_SOMTMetaClassEntryC__get_somtMetaFile;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_SOMTMetaClassEntryC__get_somtMetaFile)(
			SOMTMetaClassEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMTClassEntryC SOMSTAR (somTP_SOMTMetaClassEntryC__get_somtMetaClassDef)(
			SOMTMetaClassEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTMetaClassEntryC__get_somtMetaClassDef,system)
		typedef somTP_SOMTMetaClassEntryC__get_somtMetaClassDef *somTD_SOMTMetaClassEntryC__get_somtMetaClassDef;
	#else /* __IBMC__ */
		typedef SOMTClassEntryC SOMSTAR (SOMLINK * somTD_SOMTMetaClassEntryC__get_somtMetaClassDef)(
			SOMTMetaClassEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifndef SOMTMetaClassEntryC_MajorVersion
		#define SOMTMetaClassEntryC_MajorVersion   2
	#endif /* SOMTMetaClassEntryC_MajorVersion */
	#ifndef SOMTMetaClassEntryC_MinorVersion
		#define SOMTMetaClassEntryC_MinorVersion   1
	#endif /* SOMTMetaClassEntryC_MinorVersion */
	typedef struct SOMTMetaClassEntryCClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken _get_somtMetaFile;
		somMToken _get_somtMetaClassDef;
	} SOMTMetaClassEntryCClassDataStructure;
	typedef struct SOMTMetaClassEntryCCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} SOMTMetaClassEntryCCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_scmeta_Source) || defined(SOMTMetaClassEntryC_Class_Source)
			SOMEXTERN struct SOMTMetaClassEntryCClassDataStructure _SOMTMetaClassEntryCClassData;
			#ifndef SOMTMetaClassEntryCClassData
				#define SOMTMetaClassEntryCClassData    _SOMTMetaClassEntryCClassData
			#endif /* SOMTMetaClassEntryCClassData */
		#else
			SOMEXTERN struct SOMTMetaClassEntryCClassDataStructure * SOMLINK resolve_SOMTMetaClassEntryCClassData(void);
			#ifndef SOMTMetaClassEntryCClassData
				#define SOMTMetaClassEntryCClassData    (*(resolve_SOMTMetaClassEntryCClassData()))
			#endif /* SOMTMetaClassEntryCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_scmeta_Source) || defined(SOMTMetaClassEntryC_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_scmeta_Source || SOMTMetaClassEntryC_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_scmeta_Source || SOMTMetaClassEntryC_Class_Source */
		struct SOMTMetaClassEntryCClassDataStructure SOMDLINK SOMTMetaClassEntryCClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_scmeta_Source) || defined(SOMTMetaClassEntryC_Class_Source)
			SOMEXTERN struct SOMTMetaClassEntryCCClassDataStructure _SOMTMetaClassEntryCCClassData;
			#ifndef SOMTMetaClassEntryCCClassData
				#define SOMTMetaClassEntryCCClassData    _SOMTMetaClassEntryCCClassData
			#endif /* SOMTMetaClassEntryCCClassData */
		#else
			SOMEXTERN struct SOMTMetaClassEntryCCClassDataStructure * SOMLINK resolve_SOMTMetaClassEntryCCClassData(void);
			#ifndef SOMTMetaClassEntryCCClassData
				#define SOMTMetaClassEntryCCClassData    (*(resolve_SOMTMetaClassEntryCCClassData()))
			#endif /* SOMTMetaClassEntryCCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_scmeta_Source) || defined(SOMTMetaClassEntryC_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_scmeta_Source || SOMTMetaClassEntryC_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_scmeta_Source || SOMTMetaClassEntryC_Class_Source */
		struct SOMTMetaClassEntryCCClassDataStructure SOMDLINK SOMTMetaClassEntryCCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_scmeta_Source) || defined(SOMTMetaClassEntryC_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_scmeta_Source || SOMTMetaClassEntryC_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_scmeta_Source || SOMTMetaClassEntryC_Class_Source */
	SOMClass SOMSTAR SOMLINK SOMTMetaClassEntryCNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_SOMTMetaClassEntryC (SOMTMetaClassEntryCClassData.classObject)
	#ifndef SOMGD_SOMTMetaClassEntryC
		#if (defined(_SOMTMetaClassEntryC) || defined(__SOMTMetaClassEntryC))
			#undef _SOMTMetaClassEntryC
			#undef __SOMTMetaClassEntryC
			#define SOMGD_SOMTMetaClassEntryC 1
		#else
			#define _SOMTMetaClassEntryC _SOMCLASS_SOMTMetaClassEntryC
		#endif /* _SOMTMetaClassEntryC */
	#endif /* SOMGD_SOMTMetaClassEntryC */
	#define SOMTMetaClassEntryC_classObj _SOMCLASS_SOMTMetaClassEntryC
	#define _SOMMTOKEN_SOMTMetaClassEntryC(method) ((somMToken)(SOMTMetaClassEntryCClassData.method))
	#ifndef SOMTMetaClassEntryCNew
		#define SOMTMetaClassEntryCNew() ( _SOMTMetaClassEntryC ? \
			(SOMClass_somNew(_SOMTMetaClassEntryC)) : \
			( SOMTMetaClassEntryCNewClass( \
				SOMTMetaClassEntryC_MajorVersion, \
				SOMTMetaClassEntryC_MinorVersion), \
			SOMClass_somNew(_SOMTMetaClassEntryC))) 
	#endif /* NewSOMTMetaClassEntryC */
	#ifndef SOMTMetaClassEntryC__get_somtMetaFile
		#define SOMTMetaClassEntryC__get_somtMetaFile(somSelf) \
			SOM_Resolve(somSelf,SOMTMetaClassEntryC,_get_somtMetaFile)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtMetaFile
				#if defined(__get_somtMetaFile)
					#undef __get_somtMetaFile
					#define SOMGD___get_somtMetaFile
				#else
					#define __get_somtMetaFile SOMTMetaClassEntryC__get_somtMetaFile
				#endif
			#endif /* SOMGD___get_somtMetaFile */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTMetaClassEntryC__get_somtMetaFile */
	#ifndef SOMTMetaClassEntryC__get_somtMetaClassDef
		#define SOMTMetaClassEntryC__get_somtMetaClassDef(somSelf) \
			SOM_Resolve(somSelf,SOMTMetaClassEntryC,_get_somtMetaClassDef)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtMetaClassDef
				#if defined(__get_somtMetaClassDef)
					#undef __get_somtMetaClassDef
					#define SOMGD___get_somtMetaClassDef
				#else
					#define __get_somtMetaClassDef SOMTMetaClassEntryC__get_somtMetaClassDef
				#endif
			#endif /* SOMGD___get_somtMetaClassDef */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTMetaClassEntryC__get_somtMetaClassDef */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define SOMTMetaClassEntryC_somInit SOMObject_somInit
#define SOMTMetaClassEntryC_somUninit SOMObject_somUninit
#define SOMTMetaClassEntryC_somFree SOMObject_somFree
#define SOMTMetaClassEntryC_somGetClass SOMObject_somGetClass
#define SOMTMetaClassEntryC_somGetClassName SOMObject_somGetClassName
#define SOMTMetaClassEntryC_somGetSize SOMObject_somGetSize
#define SOMTMetaClassEntryC_somIsA SOMObject_somIsA
#define SOMTMetaClassEntryC_somIsInstanceOf SOMObject_somIsInstanceOf
#define SOMTMetaClassEntryC_somRespondsTo SOMObject_somRespondsTo
#define SOMTMetaClassEntryC_somDispatch SOMObject_somDispatch
#define SOMTMetaClassEntryC_somClassDispatch SOMObject_somClassDispatch
#define SOMTMetaClassEntryC_somCastObj SOMObject_somCastObj
#define SOMTMetaClassEntryC_somResetObj SOMObject_somResetObj
#define SOMTMetaClassEntryC_somPrintSelf SOMObject_somPrintSelf
#define SOMTMetaClassEntryC_somDumpSelf SOMObject_somDumpSelf
#define SOMTMetaClassEntryC_somDumpSelfInt SOMObject_somDumpSelfInt
#define SOMTMetaClassEntryC_somDefaultInit SOMObject_somDefaultInit
#define SOMTMetaClassEntryC_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define SOMTMetaClassEntryC_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define SOMTMetaClassEntryC_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define SOMTMetaClassEntryC_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define SOMTMetaClassEntryC_somDefaultAssign SOMObject_somDefaultAssign
#define SOMTMetaClassEntryC_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define SOMTMetaClassEntryC_somDefaultVAssign SOMObject_somDefaultVAssign
#define SOMTMetaClassEntryC_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define SOMTMetaClassEntryC_somDestruct SOMObject_somDestruct
#define SOMTMetaClassEntryC__get_somtEntryStruct SOMTEntryC__get_somtEntryStruct
#define SOMTMetaClassEntryC__set_somtEntryName SOMTEntryC__set_somtEntryName
#define SOMTMetaClassEntryC__get_somtEntryName SOMTEntryC__get_somtEntryName
#define SOMTMetaClassEntryC__set_somtElementType SOMTEntryC__set_somtElementType
#define SOMTMetaClassEntryC__get_somtElementType SOMTEntryC__get_somtElementType
#define SOMTMetaClassEntryC__get_somtElementTypeName SOMTEntryC__get_somtElementTypeName
#define SOMTMetaClassEntryC__get_somtEntryComment SOMTEntryC__get_somtEntryComment
#define SOMTMetaClassEntryC__get_somtSourceLineNumber SOMTEntryC__get_somtSourceLineNumber
#define SOMTMetaClassEntryC__get_somtTypeCode SOMTEntryC__get_somtTypeCode
#define SOMTMetaClassEntryC__get_somtIsReference SOMTEntryC__get_somtIsReference
#define SOMTMetaClassEntryC__get_somtIDLScopedName SOMTEntryC__get_somtIDLScopedName
#define SOMTMetaClassEntryC__get_somtCScopedName SOMTEntryC__get_somtCScopedName
#define SOMTMetaClassEntryC_somtGetModifierValue SOMTEntryC_somtGetModifierValue
#define SOMTMetaClassEntryC_somtGetFirstModifier SOMTEntryC_somtGetFirstModifier
#define SOMTMetaClassEntryC_somtGetNextModifier SOMTEntryC_somtGetNextModifier
#define SOMTMetaClassEntryC_somtFormatModifier SOMTEntryC_somtFormatModifier
#define SOMTMetaClassEntryC_somtGetModifierList SOMTEntryC_somtGetModifierList
#define SOMTMetaClassEntryC_somtSetSymbolsOnEntry SOMTEntryC_somtSetSymbolsOnEntry
#define SOMTMetaClassEntryC_somtSetEntryStruct SOMTEntryC_somtSetEntryStruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_scmeta_Header_h */
