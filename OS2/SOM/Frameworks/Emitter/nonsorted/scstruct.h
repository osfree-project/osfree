/* generated from scstruct.idl */
/* internal conditional is SOM_Module_scstruct_Source */
#ifndef SOM_Module_scstruct_Header_h
	#define SOM_Module_scstruct_Header_h 1
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
	#ifndef _IDL_SOMTClassEntryC_defined
		#define _IDL_SOMTClassEntryC_defined
		typedef SOMObject SOMTClassEntryC;
	#endif /* _IDL_SOMTClassEntryC_defined */
	#ifndef _IDL_SEQUENCE_SOMTClassEntryC_defined
		#define _IDL_SEQUENCE_SOMTClassEntryC_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMTClassEntryC SOMSTAR ,sequence(SOMTClassEntryC));
	#endif /* _IDL_SEQUENCE_SOMTClassEntryC_defined */
	#ifndef _IDL_SOMTStructEntryC_defined
		#define _IDL_SOMTStructEntryC_defined
		typedef SOMObject SOMTStructEntryC;
	#endif /* _IDL_SOMTStructEntryC_defined */
	#ifndef _IDL_SEQUENCE_SOMTStructEntryC_defined
		#define _IDL_SEQUENCE_SOMTStructEntryC_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMTStructEntryC SOMSTAR ,sequence(SOMTStructEntryC));
	#endif /* _IDL_SEQUENCE_SOMTStructEntryC_defined */
	#ifdef __IBMC__
		typedef SOMTTypedefEntryC SOMSTAR (somTP_SOMTStructEntryC_somtGetFirstMember)(
			SOMTStructEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTStructEntryC_somtGetFirstMember,system)
		typedef somTP_SOMTStructEntryC_somtGetFirstMember *somTD_SOMTStructEntryC_somtGetFirstMember;
	#else /* __IBMC__ */
		typedef SOMTTypedefEntryC SOMSTAR (SOMLINK * somTD_SOMTStructEntryC_somtGetFirstMember)(
			SOMTStructEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMTTypedefEntryC SOMSTAR (somTP_SOMTStructEntryC_somtGetNextMember)(
			SOMTStructEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTStructEntryC_somtGetNextMember,system)
		typedef somTP_SOMTStructEntryC_somtGetNextMember *somTD_SOMTStructEntryC_somtGetNextMember;
	#else /* __IBMC__ */
		typedef SOMTTypedefEntryC SOMSTAR (SOMLINK * somTD_SOMTStructEntryC_somtGetNextMember)(
			SOMTStructEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMTClassEntryC SOMSTAR (somTP_SOMTStructEntryC__get_somtStructClass)(
			SOMTStructEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTStructEntryC__get_somtStructClass,system)
		typedef somTP_SOMTStructEntryC__get_somtStructClass *somTD_SOMTStructEntryC__get_somtStructClass;
	#else /* __IBMC__ */
		typedef SOMTClassEntryC SOMSTAR (SOMLINK * somTD_SOMTStructEntryC__get_somtStructClass)(
			SOMTStructEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef boolean (somTP_SOMTStructEntryC__get_somtIsException)(
			SOMTStructEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTStructEntryC__get_somtIsException,system)
		typedef somTP_SOMTStructEntryC__get_somtIsException *somTD_SOMTStructEntryC__get_somtIsException;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_SOMTStructEntryC__get_somtIsException)(
			SOMTStructEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifndef SOMTStructEntryC_MajorVersion
		#define SOMTStructEntryC_MajorVersion   2
	#endif /* SOMTStructEntryC_MajorVersion */
	#ifndef SOMTStructEntryC_MinorVersion
		#define SOMTStructEntryC_MinorVersion   1
	#endif /* SOMTStructEntryC_MinorVersion */
	typedef struct SOMTStructEntryCClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken _get_somtStructClass;
		somMToken _get_somtIsException;
		somMToken somtGetFirstMember;
		somMToken somtGetNextMember;
	} SOMTStructEntryCClassDataStructure;
	typedef struct SOMTStructEntryCCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} SOMTStructEntryCCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_scstruct_Source) || defined(SOMTStructEntryC_Class_Source)
			SOMEXTERN struct SOMTStructEntryCClassDataStructure _SOMTStructEntryCClassData;
			#ifndef SOMTStructEntryCClassData
				#define SOMTStructEntryCClassData    _SOMTStructEntryCClassData
			#endif /* SOMTStructEntryCClassData */
		#else
			SOMEXTERN struct SOMTStructEntryCClassDataStructure * SOMLINK resolve_SOMTStructEntryCClassData(void);
			#ifndef SOMTStructEntryCClassData
				#define SOMTStructEntryCClassData    (*(resolve_SOMTStructEntryCClassData()))
			#endif /* SOMTStructEntryCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_scstruct_Source) || defined(SOMTStructEntryC_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_scstruct_Source || SOMTStructEntryC_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_scstruct_Source || SOMTStructEntryC_Class_Source */
		struct SOMTStructEntryCClassDataStructure SOMDLINK SOMTStructEntryCClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_scstruct_Source) || defined(SOMTStructEntryC_Class_Source)
			SOMEXTERN struct SOMTStructEntryCCClassDataStructure _SOMTStructEntryCCClassData;
			#ifndef SOMTStructEntryCCClassData
				#define SOMTStructEntryCCClassData    _SOMTStructEntryCCClassData
			#endif /* SOMTStructEntryCCClassData */
		#else
			SOMEXTERN struct SOMTStructEntryCCClassDataStructure * SOMLINK resolve_SOMTStructEntryCCClassData(void);
			#ifndef SOMTStructEntryCCClassData
				#define SOMTStructEntryCCClassData    (*(resolve_SOMTStructEntryCCClassData()))
			#endif /* SOMTStructEntryCCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_scstruct_Source) || defined(SOMTStructEntryC_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_scstruct_Source || SOMTStructEntryC_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_scstruct_Source || SOMTStructEntryC_Class_Source */
		struct SOMTStructEntryCCClassDataStructure SOMDLINK SOMTStructEntryCCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_scstruct_Source) || defined(SOMTStructEntryC_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_scstruct_Source || SOMTStructEntryC_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_scstruct_Source || SOMTStructEntryC_Class_Source */
	SOMClass SOMSTAR SOMLINK SOMTStructEntryCNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_SOMTStructEntryC (SOMTStructEntryCClassData.classObject)
	#ifndef SOMGD_SOMTStructEntryC
		#if (defined(_SOMTStructEntryC) || defined(__SOMTStructEntryC))
			#undef _SOMTStructEntryC
			#undef __SOMTStructEntryC
			#define SOMGD_SOMTStructEntryC 1
		#else
			#define _SOMTStructEntryC _SOMCLASS_SOMTStructEntryC
		#endif /* _SOMTStructEntryC */
	#endif /* SOMGD_SOMTStructEntryC */
	#define SOMTStructEntryC_classObj _SOMCLASS_SOMTStructEntryC
	#define _SOMMTOKEN_SOMTStructEntryC(method) ((somMToken)(SOMTStructEntryCClassData.method))
	#ifndef SOMTStructEntryCNew
		#define SOMTStructEntryCNew() ( _SOMTStructEntryC ? \
			(SOMClass_somNew(_SOMTStructEntryC)) : \
			( SOMTStructEntryCNewClass( \
				SOMTStructEntryC_MajorVersion, \
				SOMTStructEntryC_MinorVersion), \
			SOMClass_somNew(_SOMTStructEntryC))) 
	#endif /* NewSOMTStructEntryC */
	#ifndef SOMTStructEntryC_somtGetFirstMember
		#define SOMTStructEntryC_somtGetFirstMember(somSelf) \
			SOM_Resolve(somSelf,SOMTStructEntryC,somtGetFirstMember)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetFirstMember
				#if defined(_somtGetFirstMember)
					#undef _somtGetFirstMember
					#define SOMGD__somtGetFirstMember
				#else
					#define _somtGetFirstMember SOMTStructEntryC_somtGetFirstMember
				#endif
			#endif /* SOMGD__somtGetFirstMember */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTStructEntryC_somtGetFirstMember */
	#ifndef SOMTStructEntryC_somtGetNextMember
		#define SOMTStructEntryC_somtGetNextMember(somSelf) \
			SOM_Resolve(somSelf,SOMTStructEntryC,somtGetNextMember)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetNextMember
				#if defined(_somtGetNextMember)
					#undef _somtGetNextMember
					#define SOMGD__somtGetNextMember
				#else
					#define _somtGetNextMember SOMTStructEntryC_somtGetNextMember
				#endif
			#endif /* SOMGD__somtGetNextMember */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTStructEntryC_somtGetNextMember */
	#ifndef SOMTStructEntryC__get_somtStructClass
		#define SOMTStructEntryC__get_somtStructClass(somSelf) \
			SOM_Resolve(somSelf,SOMTStructEntryC,_get_somtStructClass)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtStructClass
				#if defined(__get_somtStructClass)
					#undef __get_somtStructClass
					#define SOMGD___get_somtStructClass
				#else
					#define __get_somtStructClass SOMTStructEntryC__get_somtStructClass
				#endif
			#endif /* SOMGD___get_somtStructClass */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTStructEntryC__get_somtStructClass */
	#ifndef SOMTStructEntryC__get_somtIsException
		#define SOMTStructEntryC__get_somtIsException(somSelf) \
			SOM_Resolve(somSelf,SOMTStructEntryC,_get_somtIsException)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtIsException
				#if defined(__get_somtIsException)
					#undef __get_somtIsException
					#define SOMGD___get_somtIsException
				#else
					#define __get_somtIsException SOMTStructEntryC__get_somtIsException
				#endif
			#endif /* SOMGD___get_somtIsException */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTStructEntryC__get_somtIsException */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define SOMTStructEntryC_somInit SOMObject_somInit
#define SOMTStructEntryC_somUninit SOMObject_somUninit
#define SOMTStructEntryC_somFree SOMObject_somFree
#define SOMTStructEntryC_somGetClass SOMObject_somGetClass
#define SOMTStructEntryC_somGetClassName SOMObject_somGetClassName
#define SOMTStructEntryC_somGetSize SOMObject_somGetSize
#define SOMTStructEntryC_somIsA SOMObject_somIsA
#define SOMTStructEntryC_somIsInstanceOf SOMObject_somIsInstanceOf
#define SOMTStructEntryC_somRespondsTo SOMObject_somRespondsTo
#define SOMTStructEntryC_somDispatch SOMObject_somDispatch
#define SOMTStructEntryC_somClassDispatch SOMObject_somClassDispatch
#define SOMTStructEntryC_somCastObj SOMObject_somCastObj
#define SOMTStructEntryC_somResetObj SOMObject_somResetObj
#define SOMTStructEntryC_somPrintSelf SOMObject_somPrintSelf
#define SOMTStructEntryC_somDumpSelf SOMObject_somDumpSelf
#define SOMTStructEntryC_somDumpSelfInt SOMObject_somDumpSelfInt
#define SOMTStructEntryC_somDefaultInit SOMObject_somDefaultInit
#define SOMTStructEntryC_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define SOMTStructEntryC_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define SOMTStructEntryC_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define SOMTStructEntryC_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define SOMTStructEntryC_somDefaultAssign SOMObject_somDefaultAssign
#define SOMTStructEntryC_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define SOMTStructEntryC_somDefaultVAssign SOMObject_somDefaultVAssign
#define SOMTStructEntryC_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define SOMTStructEntryC_somDestruct SOMObject_somDestruct
#define SOMTStructEntryC__get_somtEntryStruct SOMTEntryC__get_somtEntryStruct
#define SOMTStructEntryC__set_somtEntryName SOMTEntryC__set_somtEntryName
#define SOMTStructEntryC__get_somtEntryName SOMTEntryC__get_somtEntryName
#define SOMTStructEntryC__set_somtElementType SOMTEntryC__set_somtElementType
#define SOMTStructEntryC__get_somtElementType SOMTEntryC__get_somtElementType
#define SOMTStructEntryC__get_somtElementTypeName SOMTEntryC__get_somtElementTypeName
#define SOMTStructEntryC__get_somtEntryComment SOMTEntryC__get_somtEntryComment
#define SOMTStructEntryC__get_somtSourceLineNumber SOMTEntryC__get_somtSourceLineNumber
#define SOMTStructEntryC__get_somtTypeCode SOMTEntryC__get_somtTypeCode
#define SOMTStructEntryC__get_somtIsReference SOMTEntryC__get_somtIsReference
#define SOMTStructEntryC__get_somtIDLScopedName SOMTEntryC__get_somtIDLScopedName
#define SOMTStructEntryC__get_somtCScopedName SOMTEntryC__get_somtCScopedName
#define SOMTStructEntryC_somtGetModifierValue SOMTEntryC_somtGetModifierValue
#define SOMTStructEntryC_somtGetFirstModifier SOMTEntryC_somtGetFirstModifier
#define SOMTStructEntryC_somtGetNextModifier SOMTEntryC_somtGetNextModifier
#define SOMTStructEntryC_somtFormatModifier SOMTEntryC_somtFormatModifier
#define SOMTStructEntryC_somtGetModifierList SOMTEntryC_somtGetModifierList
#define SOMTStructEntryC_somtSetSymbolsOnEntry SOMTEntryC_somtSetSymbolsOnEntry
#define SOMTStructEntryC_somtSetEntryStruct SOMTEntryC_somtSetEntryStruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_scstruct_Header_h */
