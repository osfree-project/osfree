/* generated from sctdef.idl */
/* internal conditional is SOM_Module_sctdef_Source */
#ifndef SOM_Module_sctdef_Header_h
	#define SOM_Module_sctdef_Header_h 1
	#include <som.h>
	#include <sccommon.h>
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
	#ifdef __IBMC__
		typedef SOMTEntryC SOMSTAR (somTP_SOMTTypedefEntryC__get_somtTypedefType)(
			SOMTTypedefEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTTypedefEntryC__get_somtTypedefType,system)
		typedef somTP_SOMTTypedefEntryC__get_somtTypedefType *somTD_SOMTTypedefEntryC__get_somtTypedefType;
	#else /* __IBMC__ */
		typedef SOMTEntryC SOMSTAR (SOMLINK * somTD_SOMTTypedefEntryC__get_somtTypedefType)(
			SOMTTypedefEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMTCommonEntryC SOMSTAR (somTP_SOMTTypedefEntryC_somtGetFirstDeclarator)(
			SOMTTypedefEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTTypedefEntryC_somtGetFirstDeclarator,system)
		typedef somTP_SOMTTypedefEntryC_somtGetFirstDeclarator *somTD_SOMTTypedefEntryC_somtGetFirstDeclarator;
	#else /* __IBMC__ */
		typedef SOMTCommonEntryC SOMSTAR (SOMLINK * somTD_SOMTTypedefEntryC_somtGetFirstDeclarator)(
			SOMTTypedefEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMTCommonEntryC SOMSTAR (somTP_SOMTTypedefEntryC_somtGetNextDeclarator)(
			SOMTTypedefEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTTypedefEntryC_somtGetNextDeclarator,system)
		typedef somTP_SOMTTypedefEntryC_somtGetNextDeclarator *somTD_SOMTTypedefEntryC_somtGetNextDeclarator;
	#else /* __IBMC__ */
		typedef SOMTCommonEntryC SOMSTAR (SOMLINK * somTD_SOMTTypedefEntryC_somtGetNextDeclarator)(
			SOMTTypedefEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifndef SOMTTypedefEntryC_MajorVersion
		#define SOMTTypedefEntryC_MajorVersion   2
	#endif /* SOMTTypedefEntryC_MajorVersion */
	#ifndef SOMTTypedefEntryC_MinorVersion
		#define SOMTTypedefEntryC_MinorVersion   2
	#endif /* SOMTTypedefEntryC_MinorVersion */
	typedef struct SOMTTypedefEntryCClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken _get_somtTypedefType;
		somMToken somtGetFirstDeclarator;
		somMToken somtGetNextDeclarator;
	} SOMTTypedefEntryCClassDataStructure;
	typedef struct SOMTTypedefEntryCCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} SOMTTypedefEntryCCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_sctdef_Source) || defined(SOMTTypedefEntryC_Class_Source)
			SOMEXTERN struct SOMTTypedefEntryCClassDataStructure _SOMTTypedefEntryCClassData;
			#ifndef SOMTTypedefEntryCClassData
				#define SOMTTypedefEntryCClassData    _SOMTTypedefEntryCClassData
			#endif /* SOMTTypedefEntryCClassData */
		#else
			SOMEXTERN struct SOMTTypedefEntryCClassDataStructure * SOMLINK resolve_SOMTTypedefEntryCClassData(void);
			#ifndef SOMTTypedefEntryCClassData
				#define SOMTTypedefEntryCClassData    (*(resolve_SOMTTypedefEntryCClassData()))
			#endif /* SOMTTypedefEntryCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_sctdef_Source) || defined(SOMTTypedefEntryC_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_sctdef_Source || SOMTTypedefEntryC_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_sctdef_Source || SOMTTypedefEntryC_Class_Source */
		struct SOMTTypedefEntryCClassDataStructure SOMDLINK SOMTTypedefEntryCClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_sctdef_Source) || defined(SOMTTypedefEntryC_Class_Source)
			SOMEXTERN struct SOMTTypedefEntryCCClassDataStructure _SOMTTypedefEntryCCClassData;
			#ifndef SOMTTypedefEntryCCClassData
				#define SOMTTypedefEntryCCClassData    _SOMTTypedefEntryCCClassData
			#endif /* SOMTTypedefEntryCCClassData */
		#else
			SOMEXTERN struct SOMTTypedefEntryCCClassDataStructure * SOMLINK resolve_SOMTTypedefEntryCCClassData(void);
			#ifndef SOMTTypedefEntryCCClassData
				#define SOMTTypedefEntryCCClassData    (*(resolve_SOMTTypedefEntryCCClassData()))
			#endif /* SOMTTypedefEntryCCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_sctdef_Source) || defined(SOMTTypedefEntryC_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_sctdef_Source || SOMTTypedefEntryC_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_sctdef_Source || SOMTTypedefEntryC_Class_Source */
		struct SOMTTypedefEntryCCClassDataStructure SOMDLINK SOMTTypedefEntryCCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_sctdef_Source) || defined(SOMTTypedefEntryC_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_sctdef_Source || SOMTTypedefEntryC_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_sctdef_Source || SOMTTypedefEntryC_Class_Source */
	SOMClass SOMSTAR SOMLINK SOMTTypedefEntryCNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_SOMTTypedefEntryC (SOMTTypedefEntryCClassData.classObject)
	#ifndef SOMGD_SOMTTypedefEntryC
		#if (defined(_SOMTTypedefEntryC) || defined(__SOMTTypedefEntryC))
			#undef _SOMTTypedefEntryC
			#undef __SOMTTypedefEntryC
			#define SOMGD_SOMTTypedefEntryC 1
		#else
			#define _SOMTTypedefEntryC _SOMCLASS_SOMTTypedefEntryC
		#endif /* _SOMTTypedefEntryC */
	#endif /* SOMGD_SOMTTypedefEntryC */
	#define SOMTTypedefEntryC_classObj _SOMCLASS_SOMTTypedefEntryC
	#define _SOMMTOKEN_SOMTTypedefEntryC(method) ((somMToken)(SOMTTypedefEntryCClassData.method))
	#ifndef SOMTTypedefEntryCNew
		#define SOMTTypedefEntryCNew() ( _SOMTTypedefEntryC ? \
			(SOMClass_somNew(_SOMTTypedefEntryC)) : \
			( SOMTTypedefEntryCNewClass( \
				SOMTTypedefEntryC_MajorVersion, \
				SOMTTypedefEntryC_MinorVersion), \
			SOMClass_somNew(_SOMTTypedefEntryC))) 
	#endif /* NewSOMTTypedefEntryC */
	#ifndef SOMTTypedefEntryC__get_somtTypedefType
		#define SOMTTypedefEntryC__get_somtTypedefType(somSelf) \
			SOM_Resolve(somSelf,SOMTTypedefEntryC,_get_somtTypedefType)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtTypedefType
				#if defined(__get_somtTypedefType)
					#undef __get_somtTypedefType
					#define SOMGD___get_somtTypedefType
				#else
					#define __get_somtTypedefType SOMTTypedefEntryC__get_somtTypedefType
				#endif
			#endif /* SOMGD___get_somtTypedefType */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTTypedefEntryC__get_somtTypedefType */
	#ifndef SOMTTypedefEntryC_somtGetFirstDeclarator
		#define SOMTTypedefEntryC_somtGetFirstDeclarator(somSelf) \
			SOM_Resolve(somSelf,SOMTTypedefEntryC,somtGetFirstDeclarator)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetFirstDeclarator
				#if defined(_somtGetFirstDeclarator)
					#undef _somtGetFirstDeclarator
					#define SOMGD__somtGetFirstDeclarator
				#else
					#define _somtGetFirstDeclarator SOMTTypedefEntryC_somtGetFirstDeclarator
				#endif
			#endif /* SOMGD__somtGetFirstDeclarator */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTTypedefEntryC_somtGetFirstDeclarator */
	#ifndef SOMTTypedefEntryC_somtGetNextDeclarator
		#define SOMTTypedefEntryC_somtGetNextDeclarator(somSelf) \
			SOM_Resolve(somSelf,SOMTTypedefEntryC,somtGetNextDeclarator)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetNextDeclarator
				#if defined(_somtGetNextDeclarator)
					#undef _somtGetNextDeclarator
					#define SOMGD__somtGetNextDeclarator
				#else
					#define _somtGetNextDeclarator SOMTTypedefEntryC_somtGetNextDeclarator
				#endif
			#endif /* SOMGD__somtGetNextDeclarator */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTTypedefEntryC_somtGetNextDeclarator */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define SOMTTypedefEntryC_somInit SOMObject_somInit
#define SOMTTypedefEntryC_somUninit SOMObject_somUninit
#define SOMTTypedefEntryC_somFree SOMObject_somFree
#define SOMTTypedefEntryC_somGetClass SOMObject_somGetClass
#define SOMTTypedefEntryC_somGetClassName SOMObject_somGetClassName
#define SOMTTypedefEntryC_somGetSize SOMObject_somGetSize
#define SOMTTypedefEntryC_somIsA SOMObject_somIsA
#define SOMTTypedefEntryC_somIsInstanceOf SOMObject_somIsInstanceOf
#define SOMTTypedefEntryC_somRespondsTo SOMObject_somRespondsTo
#define SOMTTypedefEntryC_somDispatch SOMObject_somDispatch
#define SOMTTypedefEntryC_somClassDispatch SOMObject_somClassDispatch
#define SOMTTypedefEntryC_somCastObj SOMObject_somCastObj
#define SOMTTypedefEntryC_somResetObj SOMObject_somResetObj
#define SOMTTypedefEntryC_somPrintSelf SOMObject_somPrintSelf
#define SOMTTypedefEntryC_somDumpSelf SOMObject_somDumpSelf
#define SOMTTypedefEntryC_somDumpSelfInt SOMObject_somDumpSelfInt
#define SOMTTypedefEntryC_somDefaultInit SOMObject_somDefaultInit
#define SOMTTypedefEntryC_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define SOMTTypedefEntryC_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define SOMTTypedefEntryC_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define SOMTTypedefEntryC_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define SOMTTypedefEntryC_somDefaultAssign SOMObject_somDefaultAssign
#define SOMTTypedefEntryC_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define SOMTTypedefEntryC_somDefaultVAssign SOMObject_somDefaultVAssign
#define SOMTTypedefEntryC_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define SOMTTypedefEntryC_somDestruct SOMObject_somDestruct
#define SOMTTypedefEntryC__get_somtEntryStruct SOMTEntryC__get_somtEntryStruct
#define SOMTTypedefEntryC__set_somtEntryName SOMTEntryC__set_somtEntryName
#define SOMTTypedefEntryC__get_somtEntryName SOMTEntryC__get_somtEntryName
#define SOMTTypedefEntryC__set_somtElementType SOMTEntryC__set_somtElementType
#define SOMTTypedefEntryC__get_somtElementType SOMTEntryC__get_somtElementType
#define SOMTTypedefEntryC__get_somtElementTypeName SOMTEntryC__get_somtElementTypeName
#define SOMTTypedefEntryC__get_somtEntryComment SOMTEntryC__get_somtEntryComment
#define SOMTTypedefEntryC__get_somtSourceLineNumber SOMTEntryC__get_somtSourceLineNumber
#define SOMTTypedefEntryC__get_somtTypeCode SOMTEntryC__get_somtTypeCode
#define SOMTTypedefEntryC__get_somtIsReference SOMTEntryC__get_somtIsReference
#define SOMTTypedefEntryC__get_somtIDLScopedName SOMTEntryC__get_somtIDLScopedName
#define SOMTTypedefEntryC__get_somtCScopedName SOMTEntryC__get_somtCScopedName
#define SOMTTypedefEntryC_somtGetModifierValue SOMTEntryC_somtGetModifierValue
#define SOMTTypedefEntryC_somtGetFirstModifier SOMTEntryC_somtGetFirstModifier
#define SOMTTypedefEntryC_somtGetNextModifier SOMTEntryC_somtGetNextModifier
#define SOMTTypedefEntryC_somtFormatModifier SOMTEntryC_somtFormatModifier
#define SOMTTypedefEntryC_somtGetModifierList SOMTEntryC_somtGetModifierList
#define SOMTTypedefEntryC_somtSetSymbolsOnEntry SOMTEntryC_somtSetSymbolsOnEntry
#define SOMTTypedefEntryC_somtSetEntryStruct SOMTEntryC_somtSetEntryStruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_sctdef_Header_h */
