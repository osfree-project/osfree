/* generated from scattrib.idl */
/* internal conditional is SOM_Module_scattrib_Source */
#ifndef SOM_Module_scattrib_Header_h
	#define SOM_Module_scattrib_Header_h 1
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
	#ifndef _IDL_SOMTMethodEntryC_defined
		#define _IDL_SOMTMethodEntryC_defined
		typedef SOMObject SOMTMethodEntryC;
	#endif /* _IDL_SOMTMethodEntryC_defined */
	#ifndef _IDL_SEQUENCE_SOMTMethodEntryC_defined
		#define _IDL_SEQUENCE_SOMTMethodEntryC_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMTMethodEntryC SOMSTAR ,sequence(SOMTMethodEntryC));
	#endif /* _IDL_SEQUENCE_SOMTMethodEntryC_defined */
	#ifndef _IDL_SOMTAttributeEntryC_defined
		#define _IDL_SOMTAttributeEntryC_defined
		typedef SOMObject SOMTAttributeEntryC;
	#endif /* _IDL_SOMTAttributeEntryC_defined */
	#ifndef _IDL_SEQUENCE_SOMTAttributeEntryC_defined
		#define _IDL_SEQUENCE_SOMTAttributeEntryC_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMTAttributeEntryC SOMSTAR ,sequence(SOMTAttributeEntryC));
	#endif /* _IDL_SEQUENCE_SOMTAttributeEntryC_defined */
	#ifdef __IBMC__
		typedef boolean (somTP_SOMTAttributeEntryC__get_somtIsReadonly)(
			SOMTAttributeEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTAttributeEntryC__get_somtIsReadonly,system)
		typedef somTP_SOMTAttributeEntryC__get_somtIsReadonly *somTD_SOMTAttributeEntryC__get_somtIsReadonly;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_SOMTAttributeEntryC__get_somtIsReadonly)(
			SOMTAttributeEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMTEntryC SOMSTAR (somTP_SOMTAttributeEntryC__get_somtAttribType)(
			SOMTAttributeEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTAttributeEntryC__get_somtAttribType,system)
		typedef somTP_SOMTAttributeEntryC__get_somtAttribType *somTD_SOMTAttributeEntryC__get_somtAttribType;
	#else /* __IBMC__ */
		typedef SOMTEntryC SOMSTAR (SOMLINK * somTD_SOMTAttributeEntryC__get_somtAttribType)(
			SOMTAttributeEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMTDataEntryC SOMSTAR (somTP_SOMTAttributeEntryC_somtGetFirstAttributeDeclarator)(
			SOMTAttributeEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTAttributeEntryC_somtGetFirstAttributeDeclarator,system)
		typedef somTP_SOMTAttributeEntryC_somtGetFirstAttributeDeclarator *somTD_SOMTAttributeEntryC_somtGetFirstAttributeDeclarator;
	#else /* __IBMC__ */
		typedef SOMTDataEntryC SOMSTAR (SOMLINK * somTD_SOMTAttributeEntryC_somtGetFirstAttributeDeclarator)(
			SOMTAttributeEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMTDataEntryC SOMSTAR (somTP_SOMTAttributeEntryC_somtGetNextAttributeDeclarator)(
			SOMTAttributeEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTAttributeEntryC_somtGetNextAttributeDeclarator,system)
		typedef somTP_SOMTAttributeEntryC_somtGetNextAttributeDeclarator *somTD_SOMTAttributeEntryC_somtGetNextAttributeDeclarator;
	#else /* __IBMC__ */
		typedef SOMTDataEntryC SOMSTAR (SOMLINK * somTD_SOMTAttributeEntryC_somtGetNextAttributeDeclarator)(
			SOMTAttributeEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMTMethodEntryC SOMSTAR (somTP_SOMTAttributeEntryC_somtGetFirstGetMethod)(
			SOMTAttributeEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTAttributeEntryC_somtGetFirstGetMethod,system)
		typedef somTP_SOMTAttributeEntryC_somtGetFirstGetMethod *somTD_SOMTAttributeEntryC_somtGetFirstGetMethod;
	#else /* __IBMC__ */
		typedef SOMTMethodEntryC SOMSTAR (SOMLINK * somTD_SOMTAttributeEntryC_somtGetFirstGetMethod)(
			SOMTAttributeEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMTMethodEntryC SOMSTAR (somTP_SOMTAttributeEntryC_somtGetNextGetMethod)(
			SOMTAttributeEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTAttributeEntryC_somtGetNextGetMethod,system)
		typedef somTP_SOMTAttributeEntryC_somtGetNextGetMethod *somTD_SOMTAttributeEntryC_somtGetNextGetMethod;
	#else /* __IBMC__ */
		typedef SOMTMethodEntryC SOMSTAR (SOMLINK * somTD_SOMTAttributeEntryC_somtGetNextGetMethod)(
			SOMTAttributeEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMTMethodEntryC SOMSTAR (somTP_SOMTAttributeEntryC_somtGetFirstSetMethod)(
			SOMTAttributeEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTAttributeEntryC_somtGetFirstSetMethod,system)
		typedef somTP_SOMTAttributeEntryC_somtGetFirstSetMethod *somTD_SOMTAttributeEntryC_somtGetFirstSetMethod;
	#else /* __IBMC__ */
		typedef SOMTMethodEntryC SOMSTAR (SOMLINK * somTD_SOMTAttributeEntryC_somtGetFirstSetMethod)(
			SOMTAttributeEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMTMethodEntryC SOMSTAR (somTP_SOMTAttributeEntryC_somtGetNextSetMethod)(
			SOMTAttributeEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTAttributeEntryC_somtGetNextSetMethod,system)
		typedef somTP_SOMTAttributeEntryC_somtGetNextSetMethod *somTD_SOMTAttributeEntryC_somtGetNextSetMethod;
	#else /* __IBMC__ */
		typedef SOMTMethodEntryC SOMSTAR (SOMLINK * somTD_SOMTAttributeEntryC_somtGetNextSetMethod)(
			SOMTAttributeEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifndef SOMTAttributeEntryC_MajorVersion
		#define SOMTAttributeEntryC_MajorVersion   2
	#endif /* SOMTAttributeEntryC_MajorVersion */
	#ifndef SOMTAttributeEntryC_MinorVersion
		#define SOMTAttributeEntryC_MinorVersion   1
	#endif /* SOMTAttributeEntryC_MinorVersion */
	typedef struct SOMTAttributeEntryCClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken _get_somtIsReadonly;
		somMToken _get_somtAttribType;
		somMToken somtGetFirstAttributeDeclarator;
		somMToken somtGetNextAttributeDeclarator;
		somMToken somtGetFirstGetMethod;
		somMToken somtGetNextGetMethod;
		somMToken somtGetFirstSetMethod;
		somMToken somtGetNextSetMethod;
	} SOMTAttributeEntryCClassDataStructure;
	typedef struct SOMTAttributeEntryCCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} SOMTAttributeEntryCCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_scattrib_Source) || defined(SOMTAttributeEntryC_Class_Source)
			SOMEXTERN struct SOMTAttributeEntryCClassDataStructure _SOMTAttributeEntryCClassData;
			#ifndef SOMTAttributeEntryCClassData
				#define SOMTAttributeEntryCClassData    _SOMTAttributeEntryCClassData
			#endif /* SOMTAttributeEntryCClassData */
		#else
			SOMEXTERN struct SOMTAttributeEntryCClassDataStructure * SOMLINK resolve_SOMTAttributeEntryCClassData(void);
			#ifndef SOMTAttributeEntryCClassData
				#define SOMTAttributeEntryCClassData    (*(resolve_SOMTAttributeEntryCClassData()))
			#endif /* SOMTAttributeEntryCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_scattrib_Source) || defined(SOMTAttributeEntryC_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_scattrib_Source || SOMTAttributeEntryC_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_scattrib_Source || SOMTAttributeEntryC_Class_Source */
		struct SOMTAttributeEntryCClassDataStructure SOMDLINK SOMTAttributeEntryCClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_scattrib_Source) || defined(SOMTAttributeEntryC_Class_Source)
			SOMEXTERN struct SOMTAttributeEntryCCClassDataStructure _SOMTAttributeEntryCCClassData;
			#ifndef SOMTAttributeEntryCCClassData
				#define SOMTAttributeEntryCCClassData    _SOMTAttributeEntryCCClassData
			#endif /* SOMTAttributeEntryCCClassData */
		#else
			SOMEXTERN struct SOMTAttributeEntryCCClassDataStructure * SOMLINK resolve_SOMTAttributeEntryCCClassData(void);
			#ifndef SOMTAttributeEntryCCClassData
				#define SOMTAttributeEntryCCClassData    (*(resolve_SOMTAttributeEntryCCClassData()))
			#endif /* SOMTAttributeEntryCCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_scattrib_Source) || defined(SOMTAttributeEntryC_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_scattrib_Source || SOMTAttributeEntryC_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_scattrib_Source || SOMTAttributeEntryC_Class_Source */
		struct SOMTAttributeEntryCCClassDataStructure SOMDLINK SOMTAttributeEntryCCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_scattrib_Source) || defined(SOMTAttributeEntryC_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_scattrib_Source || SOMTAttributeEntryC_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_scattrib_Source || SOMTAttributeEntryC_Class_Source */
	SOMClass SOMSTAR SOMLINK SOMTAttributeEntryCNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_SOMTAttributeEntryC (SOMTAttributeEntryCClassData.classObject)
	#ifndef SOMGD_SOMTAttributeEntryC
		#if (defined(_SOMTAttributeEntryC) || defined(__SOMTAttributeEntryC))
			#undef _SOMTAttributeEntryC
			#undef __SOMTAttributeEntryC
			#define SOMGD_SOMTAttributeEntryC 1
		#else
			#define _SOMTAttributeEntryC _SOMCLASS_SOMTAttributeEntryC
		#endif /* _SOMTAttributeEntryC */
	#endif /* SOMGD_SOMTAttributeEntryC */
	#define SOMTAttributeEntryC_classObj _SOMCLASS_SOMTAttributeEntryC
	#define _SOMMTOKEN_SOMTAttributeEntryC(method) ((somMToken)(SOMTAttributeEntryCClassData.method))
	#ifndef SOMTAttributeEntryCNew
		#define SOMTAttributeEntryCNew() ( _SOMTAttributeEntryC ? \
			(SOMClass_somNew(_SOMTAttributeEntryC)) : \
			( SOMTAttributeEntryCNewClass( \
				SOMTAttributeEntryC_MajorVersion, \
				SOMTAttributeEntryC_MinorVersion), \
			SOMClass_somNew(_SOMTAttributeEntryC))) 
	#endif /* NewSOMTAttributeEntryC */
	#ifndef SOMTAttributeEntryC__get_somtIsReadonly
		#define SOMTAttributeEntryC__get_somtIsReadonly(somSelf) \
			SOM_Resolve(somSelf,SOMTAttributeEntryC,_get_somtIsReadonly)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtIsReadonly
				#if defined(__get_somtIsReadonly)
					#undef __get_somtIsReadonly
					#define SOMGD___get_somtIsReadonly
				#else
					#define __get_somtIsReadonly SOMTAttributeEntryC__get_somtIsReadonly
				#endif
			#endif /* SOMGD___get_somtIsReadonly */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTAttributeEntryC__get_somtIsReadonly */
	#ifndef SOMTAttributeEntryC__get_somtAttribType
		#define SOMTAttributeEntryC__get_somtAttribType(somSelf) \
			SOM_Resolve(somSelf,SOMTAttributeEntryC,_get_somtAttribType)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtAttribType
				#if defined(__get_somtAttribType)
					#undef __get_somtAttribType
					#define SOMGD___get_somtAttribType
				#else
					#define __get_somtAttribType SOMTAttributeEntryC__get_somtAttribType
				#endif
			#endif /* SOMGD___get_somtAttribType */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTAttributeEntryC__get_somtAttribType */
	#ifndef SOMTAttributeEntryC_somtGetFirstAttributeDeclarator
		#define SOMTAttributeEntryC_somtGetFirstAttributeDeclarator(somSelf) \
			SOM_Resolve(somSelf,SOMTAttributeEntryC,somtGetFirstAttributeDeclarator)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetFirstAttributeDeclarator
				#if defined(_somtGetFirstAttributeDeclarator)
					#undef _somtGetFirstAttributeDeclarator
					#define SOMGD__somtGetFirstAttributeDeclarator
				#else
					#define _somtGetFirstAttributeDeclarator SOMTAttributeEntryC_somtGetFirstAttributeDeclarator
				#endif
			#endif /* SOMGD__somtGetFirstAttributeDeclarator */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTAttributeEntryC_somtGetFirstAttributeDeclarator */
	#ifndef SOMTAttributeEntryC_somtGetNextAttributeDeclarator
		#define SOMTAttributeEntryC_somtGetNextAttributeDeclarator(somSelf) \
			SOM_Resolve(somSelf,SOMTAttributeEntryC,somtGetNextAttributeDeclarator)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetNextAttributeDeclarator
				#if defined(_somtGetNextAttributeDeclarator)
					#undef _somtGetNextAttributeDeclarator
					#define SOMGD__somtGetNextAttributeDeclarator
				#else
					#define _somtGetNextAttributeDeclarator SOMTAttributeEntryC_somtGetNextAttributeDeclarator
				#endif
			#endif /* SOMGD__somtGetNextAttributeDeclarator */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTAttributeEntryC_somtGetNextAttributeDeclarator */
	#ifndef SOMTAttributeEntryC_somtGetFirstGetMethod
		#define SOMTAttributeEntryC_somtGetFirstGetMethod(somSelf) \
			SOM_Resolve(somSelf,SOMTAttributeEntryC,somtGetFirstGetMethod)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetFirstGetMethod
				#if defined(_somtGetFirstGetMethod)
					#undef _somtGetFirstGetMethod
					#define SOMGD__somtGetFirstGetMethod
				#else
					#define _somtGetFirstGetMethod SOMTAttributeEntryC_somtGetFirstGetMethod
				#endif
			#endif /* SOMGD__somtGetFirstGetMethod */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTAttributeEntryC_somtGetFirstGetMethod */
	#ifndef SOMTAttributeEntryC_somtGetNextGetMethod
		#define SOMTAttributeEntryC_somtGetNextGetMethod(somSelf) \
			SOM_Resolve(somSelf,SOMTAttributeEntryC,somtGetNextGetMethod)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetNextGetMethod
				#if defined(_somtGetNextGetMethod)
					#undef _somtGetNextGetMethod
					#define SOMGD__somtGetNextGetMethod
				#else
					#define _somtGetNextGetMethod SOMTAttributeEntryC_somtGetNextGetMethod
				#endif
			#endif /* SOMGD__somtGetNextGetMethod */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTAttributeEntryC_somtGetNextGetMethod */
	#ifndef SOMTAttributeEntryC_somtGetFirstSetMethod
		#define SOMTAttributeEntryC_somtGetFirstSetMethod(somSelf) \
			SOM_Resolve(somSelf,SOMTAttributeEntryC,somtGetFirstSetMethod)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetFirstSetMethod
				#if defined(_somtGetFirstSetMethod)
					#undef _somtGetFirstSetMethod
					#define SOMGD__somtGetFirstSetMethod
				#else
					#define _somtGetFirstSetMethod SOMTAttributeEntryC_somtGetFirstSetMethod
				#endif
			#endif /* SOMGD__somtGetFirstSetMethod */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTAttributeEntryC_somtGetFirstSetMethod */
	#ifndef SOMTAttributeEntryC_somtGetNextSetMethod
		#define SOMTAttributeEntryC_somtGetNextSetMethod(somSelf) \
			SOM_Resolve(somSelf,SOMTAttributeEntryC,somtGetNextSetMethod)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetNextSetMethod
				#if defined(_somtGetNextSetMethod)
					#undef _somtGetNextSetMethod
					#define SOMGD__somtGetNextSetMethod
				#else
					#define _somtGetNextSetMethod SOMTAttributeEntryC_somtGetNextSetMethod
				#endif
			#endif /* SOMGD__somtGetNextSetMethod */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTAttributeEntryC_somtGetNextSetMethod */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define SOMTAttributeEntryC_somInit SOMObject_somInit
#define SOMTAttributeEntryC_somUninit SOMObject_somUninit
#define SOMTAttributeEntryC_somFree SOMObject_somFree
#define SOMTAttributeEntryC_somGetClass SOMObject_somGetClass
#define SOMTAttributeEntryC_somGetClassName SOMObject_somGetClassName
#define SOMTAttributeEntryC_somGetSize SOMObject_somGetSize
#define SOMTAttributeEntryC_somIsA SOMObject_somIsA
#define SOMTAttributeEntryC_somIsInstanceOf SOMObject_somIsInstanceOf
#define SOMTAttributeEntryC_somRespondsTo SOMObject_somRespondsTo
#define SOMTAttributeEntryC_somDispatch SOMObject_somDispatch
#define SOMTAttributeEntryC_somClassDispatch SOMObject_somClassDispatch
#define SOMTAttributeEntryC_somCastObj SOMObject_somCastObj
#define SOMTAttributeEntryC_somResetObj SOMObject_somResetObj
#define SOMTAttributeEntryC_somPrintSelf SOMObject_somPrintSelf
#define SOMTAttributeEntryC_somDumpSelf SOMObject_somDumpSelf
#define SOMTAttributeEntryC_somDumpSelfInt SOMObject_somDumpSelfInt
#define SOMTAttributeEntryC_somDefaultInit SOMObject_somDefaultInit
#define SOMTAttributeEntryC_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define SOMTAttributeEntryC_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define SOMTAttributeEntryC_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define SOMTAttributeEntryC_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define SOMTAttributeEntryC_somDefaultAssign SOMObject_somDefaultAssign
#define SOMTAttributeEntryC_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define SOMTAttributeEntryC_somDefaultVAssign SOMObject_somDefaultVAssign
#define SOMTAttributeEntryC_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define SOMTAttributeEntryC_somDestruct SOMObject_somDestruct
#define SOMTAttributeEntryC__get_somtEntryStruct SOMTEntryC__get_somtEntryStruct
#define SOMTAttributeEntryC__set_somtEntryName SOMTEntryC__set_somtEntryName
#define SOMTAttributeEntryC__get_somtEntryName SOMTEntryC__get_somtEntryName
#define SOMTAttributeEntryC__set_somtElementType SOMTEntryC__set_somtElementType
#define SOMTAttributeEntryC__get_somtElementType SOMTEntryC__get_somtElementType
#define SOMTAttributeEntryC__get_somtElementTypeName SOMTEntryC__get_somtElementTypeName
#define SOMTAttributeEntryC__get_somtEntryComment SOMTEntryC__get_somtEntryComment
#define SOMTAttributeEntryC__get_somtSourceLineNumber SOMTEntryC__get_somtSourceLineNumber
#define SOMTAttributeEntryC__get_somtTypeCode SOMTEntryC__get_somtTypeCode
#define SOMTAttributeEntryC__get_somtIsReference SOMTEntryC__get_somtIsReference
#define SOMTAttributeEntryC__get_somtIDLScopedName SOMTEntryC__get_somtIDLScopedName
#define SOMTAttributeEntryC__get_somtCScopedName SOMTEntryC__get_somtCScopedName
#define SOMTAttributeEntryC_somtGetModifierValue SOMTEntryC_somtGetModifierValue
#define SOMTAttributeEntryC_somtGetFirstModifier SOMTEntryC_somtGetFirstModifier
#define SOMTAttributeEntryC_somtGetNextModifier SOMTEntryC_somtGetNextModifier
#define SOMTAttributeEntryC_somtFormatModifier SOMTEntryC_somtFormatModifier
#define SOMTAttributeEntryC_somtGetModifierList SOMTEntryC_somtGetModifierList
#define SOMTAttributeEntryC_somtSetSymbolsOnEntry SOMTEntryC_somtSetSymbolsOnEntry
#define SOMTAttributeEntryC_somtSetEntryStruct SOMTEntryC_somtSetEntryStruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_scattrib_Header_h */
