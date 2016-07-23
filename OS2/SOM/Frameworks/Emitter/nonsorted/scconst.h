/* generated from scconst.idl */
/* internal conditional is SOM_Module_scconst_Source */
#ifndef SOM_Module_scconst_Header_h
	#define SOM_Module_scconst_Header_h 1
	#include <som.h>
	#include <scentry.h>
	#include <somobj.h>
	#include <sctypes.h>
	#include <sm.h>
	#include <somstrt.h>
	#ifndef _IDL_SOMTConstEntryC_defined
		#define _IDL_SOMTConstEntryC_defined
		typedef SOMObject SOMTConstEntryC;
	#endif /* _IDL_SOMTConstEntryC_defined */
	#ifndef _IDL_SEQUENCE_SOMTConstEntryC_defined
		#define _IDL_SEQUENCE_SOMTConstEntryC_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMTConstEntryC SOMSTAR ,sequence(SOMTConstEntryC));
	#endif /* _IDL_SEQUENCE_SOMTConstEntryC_defined */
	#ifdef __IBMC__
		typedef SOMTEntryC SOMSTAR (somTP_SOMTConstEntryC__get_somtConstTypeObj)(
			SOMTConstEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTConstEntryC__get_somtConstTypeObj,system)
		typedef somTP_SOMTConstEntryC__get_somtConstTypeObj *somTD_SOMTConstEntryC__get_somtConstTypeObj;
	#else /* __IBMC__ */
		typedef SOMTEntryC SOMSTAR (SOMLINK * somTD_SOMTConstEntryC__get_somtConstTypeObj)(
			SOMTConstEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_SOMTConstEntryC__get_somtConstType)(
			SOMTConstEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTConstEntryC__get_somtConstType,system)
		typedef somTP_SOMTConstEntryC__get_somtConstType *somTD_SOMTConstEntryC__get_somtConstType;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_SOMTConstEntryC__get_somtConstType)(
			SOMTConstEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_SOMTConstEntryC__get_somtConstStringVal)(
			SOMTConstEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTConstEntryC__get_somtConstStringVal,system)
		typedef somTP_SOMTConstEntryC__get_somtConstStringVal *somTD_SOMTConstEntryC__get_somtConstStringVal;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_SOMTConstEntryC__get_somtConstStringVal)(
			SOMTConstEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef unsigned long (somTP_SOMTConstEntryC__get_somtConstNumVal)(
			SOMTConstEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTConstEntryC__get_somtConstNumVal,system)
		typedef somTP_SOMTConstEntryC__get_somtConstNumVal *somTD_SOMTConstEntryC__get_somtConstNumVal;
	#else /* __IBMC__ */
		typedef unsigned long (SOMLINK * somTD_SOMTConstEntryC__get_somtConstNumVal)(
			SOMTConstEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef long (somTP_SOMTConstEntryC__get_somtConstNumNegVal)(
			SOMTConstEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTConstEntryC__get_somtConstNumNegVal,system)
		typedef somTP_SOMTConstEntryC__get_somtConstNumNegVal *somTD_SOMTConstEntryC__get_somtConstNumNegVal;
	#else /* __IBMC__ */
		typedef long (SOMLINK * somTD_SOMTConstEntryC__get_somtConstNumNegVal)(
			SOMTConstEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef boolean (somTP_SOMTConstEntryC__get_somtConstIsNegative)(
			SOMTConstEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTConstEntryC__get_somtConstIsNegative,system)
		typedef somTP_SOMTConstEntryC__get_somtConstIsNegative *somTD_SOMTConstEntryC__get_somtConstIsNegative;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_SOMTConstEntryC__get_somtConstIsNegative)(
			SOMTConstEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_SOMTConstEntryC__get_somtConstVal)(
			SOMTConstEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTConstEntryC__get_somtConstVal,system)
		typedef somTP_SOMTConstEntryC__get_somtConstVal *somTD_SOMTConstEntryC__get_somtConstVal;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_SOMTConstEntryC__get_somtConstVal)(
			SOMTConstEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifndef SOMTConstEntryC_MajorVersion
		#define SOMTConstEntryC_MajorVersion   2
	#endif /* SOMTConstEntryC_MajorVersion */
	#ifndef SOMTConstEntryC_MinorVersion
		#define SOMTConstEntryC_MinorVersion   1
	#endif /* SOMTConstEntryC_MinorVersion */
	typedef struct SOMTConstEntryCClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken _get_somtConstStringVal;
		somMToken _get_somtConstNumVal;
		somMToken _get_somtConstType;
		somMToken _get_somtConstTypeObj;
		somMToken _get_somtConstVal;
		somMToken _get_somtConstNumNegVal;
		somMToken _get_somtConstIsNegative;
	} SOMTConstEntryCClassDataStructure;
	typedef struct SOMTConstEntryCCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} SOMTConstEntryCCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_scconst_Source) || defined(SOMTConstEntryC_Class_Source)
			SOMEXTERN struct SOMTConstEntryCClassDataStructure _SOMTConstEntryCClassData;
			#ifndef SOMTConstEntryCClassData
				#define SOMTConstEntryCClassData    _SOMTConstEntryCClassData
			#endif /* SOMTConstEntryCClassData */
		#else
			SOMEXTERN struct SOMTConstEntryCClassDataStructure * SOMLINK resolve_SOMTConstEntryCClassData(void);
			#ifndef SOMTConstEntryCClassData
				#define SOMTConstEntryCClassData    (*(resolve_SOMTConstEntryCClassData()))
			#endif /* SOMTConstEntryCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_scconst_Source) || defined(SOMTConstEntryC_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_scconst_Source || SOMTConstEntryC_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_scconst_Source || SOMTConstEntryC_Class_Source */
		struct SOMTConstEntryCClassDataStructure SOMDLINK SOMTConstEntryCClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_scconst_Source) || defined(SOMTConstEntryC_Class_Source)
			SOMEXTERN struct SOMTConstEntryCCClassDataStructure _SOMTConstEntryCCClassData;
			#ifndef SOMTConstEntryCCClassData
				#define SOMTConstEntryCCClassData    _SOMTConstEntryCCClassData
			#endif /* SOMTConstEntryCCClassData */
		#else
			SOMEXTERN struct SOMTConstEntryCCClassDataStructure * SOMLINK resolve_SOMTConstEntryCCClassData(void);
			#ifndef SOMTConstEntryCCClassData
				#define SOMTConstEntryCCClassData    (*(resolve_SOMTConstEntryCCClassData()))
			#endif /* SOMTConstEntryCCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_scconst_Source) || defined(SOMTConstEntryC_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_scconst_Source || SOMTConstEntryC_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_scconst_Source || SOMTConstEntryC_Class_Source */
		struct SOMTConstEntryCCClassDataStructure SOMDLINK SOMTConstEntryCCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_scconst_Source) || defined(SOMTConstEntryC_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_scconst_Source || SOMTConstEntryC_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_scconst_Source || SOMTConstEntryC_Class_Source */
	SOMClass SOMSTAR SOMLINK SOMTConstEntryCNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_SOMTConstEntryC (SOMTConstEntryCClassData.classObject)
	#ifndef SOMGD_SOMTConstEntryC
		#if (defined(_SOMTConstEntryC) || defined(__SOMTConstEntryC))
			#undef _SOMTConstEntryC
			#undef __SOMTConstEntryC
			#define SOMGD_SOMTConstEntryC 1
		#else
			#define _SOMTConstEntryC _SOMCLASS_SOMTConstEntryC
		#endif /* _SOMTConstEntryC */
	#endif /* SOMGD_SOMTConstEntryC */
	#define SOMTConstEntryC_classObj _SOMCLASS_SOMTConstEntryC
	#define _SOMMTOKEN_SOMTConstEntryC(method) ((somMToken)(SOMTConstEntryCClassData.method))
	#ifndef SOMTConstEntryCNew
		#define SOMTConstEntryCNew() ( _SOMTConstEntryC ? \
			(SOMClass_somNew(_SOMTConstEntryC)) : \
			( SOMTConstEntryCNewClass( \
				SOMTConstEntryC_MajorVersion, \
				SOMTConstEntryC_MinorVersion), \
			SOMClass_somNew(_SOMTConstEntryC))) 
	#endif /* NewSOMTConstEntryC */
	#ifndef SOMTConstEntryC__get_somtConstTypeObj
		#define SOMTConstEntryC__get_somtConstTypeObj(somSelf) \
			SOM_Resolve(somSelf,SOMTConstEntryC,_get_somtConstTypeObj)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtConstTypeObj
				#if defined(__get_somtConstTypeObj)
					#undef __get_somtConstTypeObj
					#define SOMGD___get_somtConstTypeObj
				#else
					#define __get_somtConstTypeObj SOMTConstEntryC__get_somtConstTypeObj
				#endif
			#endif /* SOMGD___get_somtConstTypeObj */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTConstEntryC__get_somtConstTypeObj */
	#ifndef SOMTConstEntryC__get_somtConstType
		#define SOMTConstEntryC__get_somtConstType(somSelf) \
			SOM_Resolve(somSelf,SOMTConstEntryC,_get_somtConstType)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtConstType
				#if defined(__get_somtConstType)
					#undef __get_somtConstType
					#define SOMGD___get_somtConstType
				#else
					#define __get_somtConstType SOMTConstEntryC__get_somtConstType
				#endif
			#endif /* SOMGD___get_somtConstType */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTConstEntryC__get_somtConstType */
	#ifndef SOMTConstEntryC__get_somtConstStringVal
		#define SOMTConstEntryC__get_somtConstStringVal(somSelf) \
			SOM_Resolve(somSelf,SOMTConstEntryC,_get_somtConstStringVal)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtConstStringVal
				#if defined(__get_somtConstStringVal)
					#undef __get_somtConstStringVal
					#define SOMGD___get_somtConstStringVal
				#else
					#define __get_somtConstStringVal SOMTConstEntryC__get_somtConstStringVal
				#endif
			#endif /* SOMGD___get_somtConstStringVal */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTConstEntryC__get_somtConstStringVal */
	#ifndef SOMTConstEntryC__get_somtConstNumVal
		#define SOMTConstEntryC__get_somtConstNumVal(somSelf) \
			SOM_Resolve(somSelf,SOMTConstEntryC,_get_somtConstNumVal)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtConstNumVal
				#if defined(__get_somtConstNumVal)
					#undef __get_somtConstNumVal
					#define SOMGD___get_somtConstNumVal
				#else
					#define __get_somtConstNumVal SOMTConstEntryC__get_somtConstNumVal
				#endif
			#endif /* SOMGD___get_somtConstNumVal */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTConstEntryC__get_somtConstNumVal */
	#ifndef SOMTConstEntryC__get_somtConstNumNegVal
		#define SOMTConstEntryC__get_somtConstNumNegVal(somSelf) \
			SOM_Resolve(somSelf,SOMTConstEntryC,_get_somtConstNumNegVal)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtConstNumNegVal
				#if defined(__get_somtConstNumNegVal)
					#undef __get_somtConstNumNegVal
					#define SOMGD___get_somtConstNumNegVal
				#else
					#define __get_somtConstNumNegVal SOMTConstEntryC__get_somtConstNumNegVal
				#endif
			#endif /* SOMGD___get_somtConstNumNegVal */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTConstEntryC__get_somtConstNumNegVal */
	#ifndef SOMTConstEntryC__get_somtConstIsNegative
		#define SOMTConstEntryC__get_somtConstIsNegative(somSelf) \
			SOM_Resolve(somSelf,SOMTConstEntryC,_get_somtConstIsNegative)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtConstIsNegative
				#if defined(__get_somtConstIsNegative)
					#undef __get_somtConstIsNegative
					#define SOMGD___get_somtConstIsNegative
				#else
					#define __get_somtConstIsNegative SOMTConstEntryC__get_somtConstIsNegative
				#endif
			#endif /* SOMGD___get_somtConstIsNegative */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTConstEntryC__get_somtConstIsNegative */
	#ifndef SOMTConstEntryC__get_somtConstVal
		#define SOMTConstEntryC__get_somtConstVal(somSelf) \
			SOM_Resolve(somSelf,SOMTConstEntryC,_get_somtConstVal)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtConstVal
				#if defined(__get_somtConstVal)
					#undef __get_somtConstVal
					#define SOMGD___get_somtConstVal
				#else
					#define __get_somtConstVal SOMTConstEntryC__get_somtConstVal
				#endif
			#endif /* SOMGD___get_somtConstVal */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTConstEntryC__get_somtConstVal */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define SOMTConstEntryC_somInit SOMObject_somInit
#define SOMTConstEntryC_somUninit SOMObject_somUninit
#define SOMTConstEntryC_somFree SOMObject_somFree
#define SOMTConstEntryC_somGetClass SOMObject_somGetClass
#define SOMTConstEntryC_somGetClassName SOMObject_somGetClassName
#define SOMTConstEntryC_somGetSize SOMObject_somGetSize
#define SOMTConstEntryC_somIsA SOMObject_somIsA
#define SOMTConstEntryC_somIsInstanceOf SOMObject_somIsInstanceOf
#define SOMTConstEntryC_somRespondsTo SOMObject_somRespondsTo
#define SOMTConstEntryC_somDispatch SOMObject_somDispatch
#define SOMTConstEntryC_somClassDispatch SOMObject_somClassDispatch
#define SOMTConstEntryC_somCastObj SOMObject_somCastObj
#define SOMTConstEntryC_somResetObj SOMObject_somResetObj
#define SOMTConstEntryC_somPrintSelf SOMObject_somPrintSelf
#define SOMTConstEntryC_somDumpSelf SOMObject_somDumpSelf
#define SOMTConstEntryC_somDumpSelfInt SOMObject_somDumpSelfInt
#define SOMTConstEntryC_somDefaultInit SOMObject_somDefaultInit
#define SOMTConstEntryC_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define SOMTConstEntryC_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define SOMTConstEntryC_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define SOMTConstEntryC_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define SOMTConstEntryC_somDefaultAssign SOMObject_somDefaultAssign
#define SOMTConstEntryC_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define SOMTConstEntryC_somDefaultVAssign SOMObject_somDefaultVAssign
#define SOMTConstEntryC_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define SOMTConstEntryC_somDestruct SOMObject_somDestruct
#define SOMTConstEntryC__get_somtEntryStruct SOMTEntryC__get_somtEntryStruct
#define SOMTConstEntryC__set_somtEntryName SOMTEntryC__set_somtEntryName
#define SOMTConstEntryC__get_somtEntryName SOMTEntryC__get_somtEntryName
#define SOMTConstEntryC__set_somtElementType SOMTEntryC__set_somtElementType
#define SOMTConstEntryC__get_somtElementType SOMTEntryC__get_somtElementType
#define SOMTConstEntryC__get_somtElementTypeName SOMTEntryC__get_somtElementTypeName
#define SOMTConstEntryC__get_somtEntryComment SOMTEntryC__get_somtEntryComment
#define SOMTConstEntryC__get_somtSourceLineNumber SOMTEntryC__get_somtSourceLineNumber
#define SOMTConstEntryC__get_somtTypeCode SOMTEntryC__get_somtTypeCode
#define SOMTConstEntryC__get_somtIsReference SOMTEntryC__get_somtIsReference
#define SOMTConstEntryC__get_somtIDLScopedName SOMTEntryC__get_somtIDLScopedName
#define SOMTConstEntryC__get_somtCScopedName SOMTEntryC__get_somtCScopedName
#define SOMTConstEntryC_somtGetModifierValue SOMTEntryC_somtGetModifierValue
#define SOMTConstEntryC_somtGetFirstModifier SOMTEntryC_somtGetFirstModifier
#define SOMTConstEntryC_somtGetNextModifier SOMTEntryC_somtGetNextModifier
#define SOMTConstEntryC_somtFormatModifier SOMTEntryC_somtFormatModifier
#define SOMTConstEntryC_somtGetModifierList SOMTEntryC_somtGetModifierList
#define SOMTConstEntryC_somtSetSymbolsOnEntry SOMTEntryC_somtSetSymbolsOnEntry
#define SOMTConstEntryC_somtSetEntryStruct SOMTEntryC_somtSetEntryStruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_scconst_Header_h */
