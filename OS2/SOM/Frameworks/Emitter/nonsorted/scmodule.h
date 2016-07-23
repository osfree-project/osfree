/* generated from scmodule.idl */
/* internal conditional is SOM_Module_scmodule_Source */
#ifndef SOM_Module_scmodule_Header_h
	#define SOM_Module_scmodule_Header_h 1
	#include <som.h>
	#include <scentry.h>
	#include <somobj.h>
	#include <sctypes.h>
	#include <sm.h>
	#include <somstrt.h>
	#ifndef _IDL_SOMTStructEntryC_defined
		#define _IDL_SOMTStructEntryC_defined
		typedef SOMObject SOMTStructEntryC;
	#endif /* _IDL_SOMTStructEntryC_defined */
	#ifndef _IDL_SEQUENCE_SOMTStructEntryC_defined
		#define _IDL_SEQUENCE_SOMTStructEntryC_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMTStructEntryC SOMSTAR ,sequence(SOMTStructEntryC));
	#endif /* _IDL_SEQUENCE_SOMTStructEntryC_defined */
	#ifndef _IDL_SOMTTypedefEntryC_defined
		#define _IDL_SOMTTypedefEntryC_defined
		typedef SOMObject SOMTTypedefEntryC;
	#endif /* _IDL_SOMTTypedefEntryC_defined */
	#ifndef _IDL_SEQUENCE_SOMTTypedefEntryC_defined
		#define _IDL_SEQUENCE_SOMTTypedefEntryC_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMTTypedefEntryC SOMSTAR ,sequence(SOMTTypedefEntryC));
	#endif /* _IDL_SEQUENCE_SOMTTypedefEntryC_defined */
	#ifndef _IDL_SOMTUnionEntryC_defined
		#define _IDL_SOMTUnionEntryC_defined
		typedef SOMObject SOMTUnionEntryC;
	#endif /* _IDL_SOMTUnionEntryC_defined */
	#ifndef _IDL_SEQUENCE_SOMTUnionEntryC_defined
		#define _IDL_SEQUENCE_SOMTUnionEntryC_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMTUnionEntryC SOMSTAR ,sequence(SOMTUnionEntryC));
	#endif /* _IDL_SEQUENCE_SOMTUnionEntryC_defined */
	#ifndef _IDL_SOMTEnumEntryC_defined
		#define _IDL_SOMTEnumEntryC_defined
		typedef SOMObject SOMTEnumEntryC;
	#endif /* _IDL_SOMTEnumEntryC_defined */
	#ifndef _IDL_SEQUENCE_SOMTEnumEntryC_defined
		#define _IDL_SEQUENCE_SOMTEnumEntryC_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMTEnumEntryC SOMSTAR ,sequence(SOMTEnumEntryC));
	#endif /* _IDL_SEQUENCE_SOMTEnumEntryC_defined */
	#ifndef _IDL_SOMTConstEntryC_defined
		#define _IDL_SOMTConstEntryC_defined
		typedef SOMObject SOMTConstEntryC;
	#endif /* _IDL_SOMTConstEntryC_defined */
	#ifndef _IDL_SEQUENCE_SOMTConstEntryC_defined
		#define _IDL_SEQUENCE_SOMTConstEntryC_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMTConstEntryC SOMSTAR ,sequence(SOMTConstEntryC));
	#endif /* _IDL_SEQUENCE_SOMTConstEntryC_defined */
	#ifndef _IDL_SOMTSequenceEntryC_defined
		#define _IDL_SOMTSequenceEntryC_defined
		typedef SOMObject SOMTSequenceEntryC;
	#endif /* _IDL_SOMTSequenceEntryC_defined */
	#ifndef _IDL_SEQUENCE_SOMTSequenceEntryC_defined
		#define _IDL_SEQUENCE_SOMTSequenceEntryC_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMTSequenceEntryC SOMSTAR ,sequence(SOMTSequenceEntryC));
	#endif /* _IDL_SEQUENCE_SOMTSequenceEntryC_defined */
	#ifndef _IDL_SOMTClassEntryC_defined
		#define _IDL_SOMTClassEntryC_defined
		typedef SOMObject SOMTClassEntryC;
	#endif /* _IDL_SOMTClassEntryC_defined */
	#ifndef _IDL_SEQUENCE_SOMTClassEntryC_defined
		#define _IDL_SEQUENCE_SOMTClassEntryC_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMTClassEntryC SOMSTAR ,sequence(SOMTClassEntryC));
	#endif /* _IDL_SEQUENCE_SOMTClassEntryC_defined */
	#ifndef _IDL_SOMTModuleEntryC_defined
		#define _IDL_SOMTModuleEntryC_defined
		typedef SOMObject SOMTModuleEntryC;
	#endif /* _IDL_SOMTModuleEntryC_defined */
	#ifndef _IDL_SEQUENCE_SOMTModuleEntryC_defined
		#define _IDL_SEQUENCE_SOMTModuleEntryC_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMTModuleEntryC SOMSTAR ,sequence(SOMTModuleEntryC));
	#endif /* _IDL_SEQUENCE_SOMTModuleEntryC_defined */
	#ifdef __IBMC__
		typedef SOMTModuleEntryC SOMSTAR (somTP_SOMTModuleEntryC__get_somtOuterModule)(
			SOMTModuleEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTModuleEntryC__get_somtOuterModule,system)
		typedef somTP_SOMTModuleEntryC__get_somtOuterModule *somTD_SOMTModuleEntryC__get_somtOuterModule;
	#else /* __IBMC__ */
		typedef SOMTModuleEntryC SOMSTAR (SOMLINK * somTD_SOMTModuleEntryC__get_somtOuterModule)(
			SOMTModuleEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_SOMTModuleEntryC__get_somtModuleFile)(
			SOMTModuleEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTModuleEntryC__get_somtModuleFile,system)
		typedef somTP_SOMTModuleEntryC__get_somtModuleFile *somTD_SOMTModuleEntryC__get_somtModuleFile;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_SOMTModuleEntryC__get_somtModuleFile)(
			SOMTModuleEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMTStructEntryC SOMSTAR (somTP_SOMTModuleEntryC_somtGetFirstModuleStruct)(
			SOMTModuleEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTModuleEntryC_somtGetFirstModuleStruct,system)
		typedef somTP_SOMTModuleEntryC_somtGetFirstModuleStruct *somTD_SOMTModuleEntryC_somtGetFirstModuleStruct;
	#else /* __IBMC__ */
		typedef SOMTStructEntryC SOMSTAR (SOMLINK * somTD_SOMTModuleEntryC_somtGetFirstModuleStruct)(
			SOMTModuleEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMTStructEntryC SOMSTAR (somTP_SOMTModuleEntryC_somtGetNextModuleStruct)(
			SOMTModuleEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTModuleEntryC_somtGetNextModuleStruct,system)
		typedef somTP_SOMTModuleEntryC_somtGetNextModuleStruct *somTD_SOMTModuleEntryC_somtGetNextModuleStruct;
	#else /* __IBMC__ */
		typedef SOMTStructEntryC SOMSTAR (SOMLINK * somTD_SOMTModuleEntryC_somtGetNextModuleStruct)(
			SOMTModuleEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMTTypedefEntryC SOMSTAR (somTP_SOMTModuleEntryC_somtGetFirstModuleTypedef)(
			SOMTModuleEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTModuleEntryC_somtGetFirstModuleTypedef,system)
		typedef somTP_SOMTModuleEntryC_somtGetFirstModuleTypedef *somTD_SOMTModuleEntryC_somtGetFirstModuleTypedef;
	#else /* __IBMC__ */
		typedef SOMTTypedefEntryC SOMSTAR (SOMLINK * somTD_SOMTModuleEntryC_somtGetFirstModuleTypedef)(
			SOMTModuleEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMTTypedefEntryC SOMSTAR (somTP_SOMTModuleEntryC_somtGetNextModuleTypedef)(
			SOMTModuleEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTModuleEntryC_somtGetNextModuleTypedef,system)
		typedef somTP_SOMTModuleEntryC_somtGetNextModuleTypedef *somTD_SOMTModuleEntryC_somtGetNextModuleTypedef;
	#else /* __IBMC__ */
		typedef SOMTTypedefEntryC SOMSTAR (SOMLINK * somTD_SOMTModuleEntryC_somtGetNextModuleTypedef)(
			SOMTModuleEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMTUnionEntryC SOMSTAR (somTP_SOMTModuleEntryC_somtGetFirstModuleUnion)(
			SOMTModuleEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTModuleEntryC_somtGetFirstModuleUnion,system)
		typedef somTP_SOMTModuleEntryC_somtGetFirstModuleUnion *somTD_SOMTModuleEntryC_somtGetFirstModuleUnion;
	#else /* __IBMC__ */
		typedef SOMTUnionEntryC SOMSTAR (SOMLINK * somTD_SOMTModuleEntryC_somtGetFirstModuleUnion)(
			SOMTModuleEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMTUnionEntryC SOMSTAR (somTP_SOMTModuleEntryC_somtGetNextModuleUnion)(
			SOMTModuleEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTModuleEntryC_somtGetNextModuleUnion,system)
		typedef somTP_SOMTModuleEntryC_somtGetNextModuleUnion *somTD_SOMTModuleEntryC_somtGetNextModuleUnion;
	#else /* __IBMC__ */
		typedef SOMTUnionEntryC SOMSTAR (SOMLINK * somTD_SOMTModuleEntryC_somtGetNextModuleUnion)(
			SOMTModuleEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMTEnumEntryC SOMSTAR (somTP_SOMTModuleEntryC_somtGetFirstModuleEnum)(
			SOMTModuleEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTModuleEntryC_somtGetFirstModuleEnum,system)
		typedef somTP_SOMTModuleEntryC_somtGetFirstModuleEnum *somTD_SOMTModuleEntryC_somtGetFirstModuleEnum;
	#else /* __IBMC__ */
		typedef SOMTEnumEntryC SOMSTAR (SOMLINK * somTD_SOMTModuleEntryC_somtGetFirstModuleEnum)(
			SOMTModuleEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMTEnumEntryC SOMSTAR (somTP_SOMTModuleEntryC_somtGetNextModuleEnum)(
			SOMTModuleEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTModuleEntryC_somtGetNextModuleEnum,system)
		typedef somTP_SOMTModuleEntryC_somtGetNextModuleEnum *somTD_SOMTModuleEntryC_somtGetNextModuleEnum;
	#else /* __IBMC__ */
		typedef SOMTEnumEntryC SOMSTAR (SOMLINK * somTD_SOMTModuleEntryC_somtGetNextModuleEnum)(
			SOMTModuleEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMTConstEntryC SOMSTAR (somTP_SOMTModuleEntryC_somtGetFirstModuleConstant)(
			SOMTModuleEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTModuleEntryC_somtGetFirstModuleConstant,system)
		typedef somTP_SOMTModuleEntryC_somtGetFirstModuleConstant *somTD_SOMTModuleEntryC_somtGetFirstModuleConstant;
	#else /* __IBMC__ */
		typedef SOMTConstEntryC SOMSTAR (SOMLINK * somTD_SOMTModuleEntryC_somtGetFirstModuleConstant)(
			SOMTModuleEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMTConstEntryC SOMSTAR (somTP_SOMTModuleEntryC_somtGetNextModuleConstant)(
			SOMTModuleEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTModuleEntryC_somtGetNextModuleConstant,system)
		typedef somTP_SOMTModuleEntryC_somtGetNextModuleConstant *somTD_SOMTModuleEntryC_somtGetNextModuleConstant;
	#else /* __IBMC__ */
		typedef SOMTConstEntryC SOMSTAR (SOMLINK * somTD_SOMTModuleEntryC_somtGetNextModuleConstant)(
			SOMTModuleEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMTSequenceEntryC SOMSTAR (somTP_SOMTModuleEntryC_somtGetFirstModuleSequence)(
			SOMTModuleEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTModuleEntryC_somtGetFirstModuleSequence,system)
		typedef somTP_SOMTModuleEntryC_somtGetFirstModuleSequence *somTD_SOMTModuleEntryC_somtGetFirstModuleSequence;
	#else /* __IBMC__ */
		typedef SOMTSequenceEntryC SOMSTAR (SOMLINK * somTD_SOMTModuleEntryC_somtGetFirstModuleSequence)(
			SOMTModuleEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMTSequenceEntryC SOMSTAR (somTP_SOMTModuleEntryC_somtGetNextModuleSequence)(
			SOMTModuleEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTModuleEntryC_somtGetNextModuleSequence,system)
		typedef somTP_SOMTModuleEntryC_somtGetNextModuleSequence *somTD_SOMTModuleEntryC_somtGetNextModuleSequence;
	#else /* __IBMC__ */
		typedef SOMTSequenceEntryC SOMSTAR (SOMLINK * somTD_SOMTModuleEntryC_somtGetNextModuleSequence)(
			SOMTModuleEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMTClassEntryC SOMSTAR (somTP_SOMTModuleEntryC_somtGetFirstInterface)(
			SOMTModuleEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTModuleEntryC_somtGetFirstInterface,system)
		typedef somTP_SOMTModuleEntryC_somtGetFirstInterface *somTD_SOMTModuleEntryC_somtGetFirstInterface;
	#else /* __IBMC__ */
		typedef SOMTClassEntryC SOMSTAR (SOMLINK * somTD_SOMTModuleEntryC_somtGetFirstInterface)(
			SOMTModuleEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMTClassEntryC SOMSTAR (somTP_SOMTModuleEntryC_somtGetNextInterface)(
			SOMTModuleEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTModuleEntryC_somtGetNextInterface,system)
		typedef somTP_SOMTModuleEntryC_somtGetNextInterface *somTD_SOMTModuleEntryC_somtGetNextInterface;
	#else /* __IBMC__ */
		typedef SOMTClassEntryC SOMSTAR (SOMLINK * somTD_SOMTModuleEntryC_somtGetNextInterface)(
			SOMTModuleEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMTModuleEntryC SOMSTAR (somTP_SOMTModuleEntryC_somtGetFirstModule)(
			SOMTModuleEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTModuleEntryC_somtGetFirstModule,system)
		typedef somTP_SOMTModuleEntryC_somtGetFirstModule *somTD_SOMTModuleEntryC_somtGetFirstModule;
	#else /* __IBMC__ */
		typedef SOMTModuleEntryC SOMSTAR (SOMLINK * somTD_SOMTModuleEntryC_somtGetFirstModule)(
			SOMTModuleEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMTModuleEntryC SOMSTAR (somTP_SOMTModuleEntryC_somtGetNextModule)(
			SOMTModuleEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTModuleEntryC_somtGetNextModule,system)
		typedef somTP_SOMTModuleEntryC_somtGetNextModule *somTD_SOMTModuleEntryC_somtGetNextModule;
	#else /* __IBMC__ */
		typedef SOMTModuleEntryC SOMSTAR (SOMLINK * somTD_SOMTModuleEntryC_somtGetNextModule)(
			SOMTModuleEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMTEntryC SOMSTAR (somTP_SOMTModuleEntryC_somtGetFirstModuleDef)(
			SOMTModuleEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTModuleEntryC_somtGetFirstModuleDef,system)
		typedef somTP_SOMTModuleEntryC_somtGetFirstModuleDef *somTD_SOMTModuleEntryC_somtGetFirstModuleDef;
	#else /* __IBMC__ */
		typedef SOMTEntryC SOMSTAR (SOMLINK * somTD_SOMTModuleEntryC_somtGetFirstModuleDef)(
			SOMTModuleEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMTEntryC SOMSTAR (somTP_SOMTModuleEntryC_somtGetNextModuleDef)(
			SOMTModuleEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTModuleEntryC_somtGetNextModuleDef,system)
		typedef somTP_SOMTModuleEntryC_somtGetNextModuleDef *somTD_SOMTModuleEntryC_somtGetNextModuleDef;
	#else /* __IBMC__ */
		typedef SOMTEntryC SOMSTAR (SOMLINK * somTD_SOMTModuleEntryC_somtGetNextModuleDef)(
			SOMTModuleEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifndef SOMTModuleEntryC_MajorVersion
		#define SOMTModuleEntryC_MajorVersion   2
	#endif /* SOMTModuleEntryC_MajorVersion */
	#ifndef SOMTModuleEntryC_MinorVersion
		#define SOMTModuleEntryC_MinorVersion   1
	#endif /* SOMTModuleEntryC_MinorVersion */
	typedef struct SOMTModuleEntryCClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken _get_somtOuterModule;
		somMToken somtGetFirstModuleStruct;
		somMToken somtGetNextModuleStruct;
		somMToken somtGetFirstModuleTypedef;
		somMToken somtGetNextModuleTypedef;
		somMToken somtGetFirstModuleUnion;
		somMToken somtGetNextModuleUnion;
		somMToken somtGetFirstModuleEnum;
		somMToken somtGetNextModuleEnum;
		somMToken somtGetFirstModuleConstant;
		somMToken somtGetNextModuleConstant;
		somMToken somtGetFirstModuleSequence;
		somMToken somtGetNextModuleSequence;
		somMToken somtGetFirstInterface;
		somMToken somtGetNextInterface;
		somMToken somtGetFirstModule;
		somMToken somtGetNextModule;
		somMToken somtGetFirstModuleDef;
		somMToken somtGetNextModuleDef;
		somMToken _get_somtModuleFile;
	} SOMTModuleEntryCClassDataStructure;
	typedef struct SOMTModuleEntryCCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} SOMTModuleEntryCCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_scmodule_Source) || defined(SOMTModuleEntryC_Class_Source)
			SOMEXTERN struct SOMTModuleEntryCClassDataStructure _SOMTModuleEntryCClassData;
			#ifndef SOMTModuleEntryCClassData
				#define SOMTModuleEntryCClassData    _SOMTModuleEntryCClassData
			#endif /* SOMTModuleEntryCClassData */
		#else
			SOMEXTERN struct SOMTModuleEntryCClassDataStructure * SOMLINK resolve_SOMTModuleEntryCClassData(void);
			#ifndef SOMTModuleEntryCClassData
				#define SOMTModuleEntryCClassData    (*(resolve_SOMTModuleEntryCClassData()))
			#endif /* SOMTModuleEntryCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_scmodule_Source) || defined(SOMTModuleEntryC_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_scmodule_Source || SOMTModuleEntryC_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_scmodule_Source || SOMTModuleEntryC_Class_Source */
		struct SOMTModuleEntryCClassDataStructure SOMDLINK SOMTModuleEntryCClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_scmodule_Source) || defined(SOMTModuleEntryC_Class_Source)
			SOMEXTERN struct SOMTModuleEntryCCClassDataStructure _SOMTModuleEntryCCClassData;
			#ifndef SOMTModuleEntryCCClassData
				#define SOMTModuleEntryCCClassData    _SOMTModuleEntryCCClassData
			#endif /* SOMTModuleEntryCCClassData */
		#else
			SOMEXTERN struct SOMTModuleEntryCCClassDataStructure * SOMLINK resolve_SOMTModuleEntryCCClassData(void);
			#ifndef SOMTModuleEntryCCClassData
				#define SOMTModuleEntryCCClassData    (*(resolve_SOMTModuleEntryCCClassData()))
			#endif /* SOMTModuleEntryCCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_scmodule_Source) || defined(SOMTModuleEntryC_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_scmodule_Source || SOMTModuleEntryC_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_scmodule_Source || SOMTModuleEntryC_Class_Source */
		struct SOMTModuleEntryCCClassDataStructure SOMDLINK SOMTModuleEntryCCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_scmodule_Source) || defined(SOMTModuleEntryC_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_scmodule_Source || SOMTModuleEntryC_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_scmodule_Source || SOMTModuleEntryC_Class_Source */
	SOMClass SOMSTAR SOMLINK SOMTModuleEntryCNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_SOMTModuleEntryC (SOMTModuleEntryCClassData.classObject)
	#ifndef SOMGD_SOMTModuleEntryC
		#if (defined(_SOMTModuleEntryC) || defined(__SOMTModuleEntryC))
			#undef _SOMTModuleEntryC
			#undef __SOMTModuleEntryC
			#define SOMGD_SOMTModuleEntryC 1
		#else
			#define _SOMTModuleEntryC _SOMCLASS_SOMTModuleEntryC
		#endif /* _SOMTModuleEntryC */
	#endif /* SOMGD_SOMTModuleEntryC */
	#define SOMTModuleEntryC_classObj _SOMCLASS_SOMTModuleEntryC
	#define _SOMMTOKEN_SOMTModuleEntryC(method) ((somMToken)(SOMTModuleEntryCClassData.method))
	#ifndef SOMTModuleEntryCNew
		#define SOMTModuleEntryCNew() ( _SOMTModuleEntryC ? \
			(SOMClass_somNew(_SOMTModuleEntryC)) : \
			( SOMTModuleEntryCNewClass( \
				SOMTModuleEntryC_MajorVersion, \
				SOMTModuleEntryC_MinorVersion), \
			SOMClass_somNew(_SOMTModuleEntryC))) 
	#endif /* NewSOMTModuleEntryC */
	#ifndef SOMTModuleEntryC__get_somtOuterModule
		#define SOMTModuleEntryC__get_somtOuterModule(somSelf) \
			SOM_Resolve(somSelf,SOMTModuleEntryC,_get_somtOuterModule)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtOuterModule
				#if defined(__get_somtOuterModule)
					#undef __get_somtOuterModule
					#define SOMGD___get_somtOuterModule
				#else
					#define __get_somtOuterModule SOMTModuleEntryC__get_somtOuterModule
				#endif
			#endif /* SOMGD___get_somtOuterModule */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTModuleEntryC__get_somtOuterModule */
	#ifndef SOMTModuleEntryC__get_somtModuleFile
		#define SOMTModuleEntryC__get_somtModuleFile(somSelf) \
			SOM_Resolve(somSelf,SOMTModuleEntryC,_get_somtModuleFile)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtModuleFile
				#if defined(__get_somtModuleFile)
					#undef __get_somtModuleFile
					#define SOMGD___get_somtModuleFile
				#else
					#define __get_somtModuleFile SOMTModuleEntryC__get_somtModuleFile
				#endif
			#endif /* SOMGD___get_somtModuleFile */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTModuleEntryC__get_somtModuleFile */
	#ifndef SOMTModuleEntryC_somtGetFirstModuleStruct
		#define SOMTModuleEntryC_somtGetFirstModuleStruct(somSelf) \
			SOM_Resolve(somSelf,SOMTModuleEntryC,somtGetFirstModuleStruct)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetFirstModuleStruct
				#if defined(_somtGetFirstModuleStruct)
					#undef _somtGetFirstModuleStruct
					#define SOMGD__somtGetFirstModuleStruct
				#else
					#define _somtGetFirstModuleStruct SOMTModuleEntryC_somtGetFirstModuleStruct
				#endif
			#endif /* SOMGD__somtGetFirstModuleStruct */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTModuleEntryC_somtGetFirstModuleStruct */
	#ifndef SOMTModuleEntryC_somtGetNextModuleStruct
		#define SOMTModuleEntryC_somtGetNextModuleStruct(somSelf) \
			SOM_Resolve(somSelf,SOMTModuleEntryC,somtGetNextModuleStruct)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetNextModuleStruct
				#if defined(_somtGetNextModuleStruct)
					#undef _somtGetNextModuleStruct
					#define SOMGD__somtGetNextModuleStruct
				#else
					#define _somtGetNextModuleStruct SOMTModuleEntryC_somtGetNextModuleStruct
				#endif
			#endif /* SOMGD__somtGetNextModuleStruct */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTModuleEntryC_somtGetNextModuleStruct */
	#ifndef SOMTModuleEntryC_somtGetFirstModuleTypedef
		#define SOMTModuleEntryC_somtGetFirstModuleTypedef(somSelf) \
			SOM_Resolve(somSelf,SOMTModuleEntryC,somtGetFirstModuleTypedef)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetFirstModuleTypedef
				#if defined(_somtGetFirstModuleTypedef)
					#undef _somtGetFirstModuleTypedef
					#define SOMGD__somtGetFirstModuleTypedef
				#else
					#define _somtGetFirstModuleTypedef SOMTModuleEntryC_somtGetFirstModuleTypedef
				#endif
			#endif /* SOMGD__somtGetFirstModuleTypedef */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTModuleEntryC_somtGetFirstModuleTypedef */
	#ifndef SOMTModuleEntryC_somtGetNextModuleTypedef
		#define SOMTModuleEntryC_somtGetNextModuleTypedef(somSelf) \
			SOM_Resolve(somSelf,SOMTModuleEntryC,somtGetNextModuleTypedef)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetNextModuleTypedef
				#if defined(_somtGetNextModuleTypedef)
					#undef _somtGetNextModuleTypedef
					#define SOMGD__somtGetNextModuleTypedef
				#else
					#define _somtGetNextModuleTypedef SOMTModuleEntryC_somtGetNextModuleTypedef
				#endif
			#endif /* SOMGD__somtGetNextModuleTypedef */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTModuleEntryC_somtGetNextModuleTypedef */
	#ifndef SOMTModuleEntryC_somtGetFirstModuleUnion
		#define SOMTModuleEntryC_somtGetFirstModuleUnion(somSelf) \
			SOM_Resolve(somSelf,SOMTModuleEntryC,somtGetFirstModuleUnion)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetFirstModuleUnion
				#if defined(_somtGetFirstModuleUnion)
					#undef _somtGetFirstModuleUnion
					#define SOMGD__somtGetFirstModuleUnion
				#else
					#define _somtGetFirstModuleUnion SOMTModuleEntryC_somtGetFirstModuleUnion
				#endif
			#endif /* SOMGD__somtGetFirstModuleUnion */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTModuleEntryC_somtGetFirstModuleUnion */
	#ifndef SOMTModuleEntryC_somtGetNextModuleUnion
		#define SOMTModuleEntryC_somtGetNextModuleUnion(somSelf) \
			SOM_Resolve(somSelf,SOMTModuleEntryC,somtGetNextModuleUnion)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetNextModuleUnion
				#if defined(_somtGetNextModuleUnion)
					#undef _somtGetNextModuleUnion
					#define SOMGD__somtGetNextModuleUnion
				#else
					#define _somtGetNextModuleUnion SOMTModuleEntryC_somtGetNextModuleUnion
				#endif
			#endif /* SOMGD__somtGetNextModuleUnion */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTModuleEntryC_somtGetNextModuleUnion */
	#ifndef SOMTModuleEntryC_somtGetFirstModuleEnum
		#define SOMTModuleEntryC_somtGetFirstModuleEnum(somSelf) \
			SOM_Resolve(somSelf,SOMTModuleEntryC,somtGetFirstModuleEnum)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetFirstModuleEnum
				#if defined(_somtGetFirstModuleEnum)
					#undef _somtGetFirstModuleEnum
					#define SOMGD__somtGetFirstModuleEnum
				#else
					#define _somtGetFirstModuleEnum SOMTModuleEntryC_somtGetFirstModuleEnum
				#endif
			#endif /* SOMGD__somtGetFirstModuleEnum */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTModuleEntryC_somtGetFirstModuleEnum */
	#ifndef SOMTModuleEntryC_somtGetNextModuleEnum
		#define SOMTModuleEntryC_somtGetNextModuleEnum(somSelf) \
			SOM_Resolve(somSelf,SOMTModuleEntryC,somtGetNextModuleEnum)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetNextModuleEnum
				#if defined(_somtGetNextModuleEnum)
					#undef _somtGetNextModuleEnum
					#define SOMGD__somtGetNextModuleEnum
				#else
					#define _somtGetNextModuleEnum SOMTModuleEntryC_somtGetNextModuleEnum
				#endif
			#endif /* SOMGD__somtGetNextModuleEnum */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTModuleEntryC_somtGetNextModuleEnum */
	#ifndef SOMTModuleEntryC_somtGetFirstModuleConstant
		#define SOMTModuleEntryC_somtGetFirstModuleConstant(somSelf) \
			SOM_Resolve(somSelf,SOMTModuleEntryC,somtGetFirstModuleConstant)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetFirstModuleConstant
				#if defined(_somtGetFirstModuleConstant)
					#undef _somtGetFirstModuleConstant
					#define SOMGD__somtGetFirstModuleConstant
				#else
					#define _somtGetFirstModuleConstant SOMTModuleEntryC_somtGetFirstModuleConstant
				#endif
			#endif /* SOMGD__somtGetFirstModuleConstant */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTModuleEntryC_somtGetFirstModuleConstant */
	#ifndef SOMTModuleEntryC_somtGetNextModuleConstant
		#define SOMTModuleEntryC_somtGetNextModuleConstant(somSelf) \
			SOM_Resolve(somSelf,SOMTModuleEntryC,somtGetNextModuleConstant)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetNextModuleConstant
				#if defined(_somtGetNextModuleConstant)
					#undef _somtGetNextModuleConstant
					#define SOMGD__somtGetNextModuleConstant
				#else
					#define _somtGetNextModuleConstant SOMTModuleEntryC_somtGetNextModuleConstant
				#endif
			#endif /* SOMGD__somtGetNextModuleConstant */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTModuleEntryC_somtGetNextModuleConstant */
	#ifndef SOMTModuleEntryC_somtGetFirstModuleSequence
		#define SOMTModuleEntryC_somtGetFirstModuleSequence(somSelf) \
			SOM_Resolve(somSelf,SOMTModuleEntryC,somtGetFirstModuleSequence)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetFirstModuleSequence
				#if defined(_somtGetFirstModuleSequence)
					#undef _somtGetFirstModuleSequence
					#define SOMGD__somtGetFirstModuleSequence
				#else
					#define _somtGetFirstModuleSequence SOMTModuleEntryC_somtGetFirstModuleSequence
				#endif
			#endif /* SOMGD__somtGetFirstModuleSequence */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTModuleEntryC_somtGetFirstModuleSequence */
	#ifndef SOMTModuleEntryC_somtGetNextModuleSequence
		#define SOMTModuleEntryC_somtGetNextModuleSequence(somSelf) \
			SOM_Resolve(somSelf,SOMTModuleEntryC,somtGetNextModuleSequence)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetNextModuleSequence
				#if defined(_somtGetNextModuleSequence)
					#undef _somtGetNextModuleSequence
					#define SOMGD__somtGetNextModuleSequence
				#else
					#define _somtGetNextModuleSequence SOMTModuleEntryC_somtGetNextModuleSequence
				#endif
			#endif /* SOMGD__somtGetNextModuleSequence */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTModuleEntryC_somtGetNextModuleSequence */
	#ifndef SOMTModuleEntryC_somtGetFirstInterface
		#define SOMTModuleEntryC_somtGetFirstInterface(somSelf) \
			SOM_Resolve(somSelf,SOMTModuleEntryC,somtGetFirstInterface)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetFirstInterface
				#if defined(_somtGetFirstInterface)
					#undef _somtGetFirstInterface
					#define SOMGD__somtGetFirstInterface
				#else
					#define _somtGetFirstInterface SOMTModuleEntryC_somtGetFirstInterface
				#endif
			#endif /* SOMGD__somtGetFirstInterface */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTModuleEntryC_somtGetFirstInterface */
	#ifndef SOMTModuleEntryC_somtGetNextInterface
		#define SOMTModuleEntryC_somtGetNextInterface(somSelf) \
			SOM_Resolve(somSelf,SOMTModuleEntryC,somtGetNextInterface)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetNextInterface
				#if defined(_somtGetNextInterface)
					#undef _somtGetNextInterface
					#define SOMGD__somtGetNextInterface
				#else
					#define _somtGetNextInterface SOMTModuleEntryC_somtGetNextInterface
				#endif
			#endif /* SOMGD__somtGetNextInterface */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTModuleEntryC_somtGetNextInterface */
	#ifndef SOMTModuleEntryC_somtGetFirstModule
		#define SOMTModuleEntryC_somtGetFirstModule(somSelf) \
			SOM_Resolve(somSelf,SOMTModuleEntryC,somtGetFirstModule)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetFirstModule
				#if defined(_somtGetFirstModule)
					#undef _somtGetFirstModule
					#define SOMGD__somtGetFirstModule
				#else
					#define _somtGetFirstModule SOMTModuleEntryC_somtGetFirstModule
				#endif
			#endif /* SOMGD__somtGetFirstModule */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTModuleEntryC_somtGetFirstModule */
	#ifndef SOMTModuleEntryC_somtGetNextModule
		#define SOMTModuleEntryC_somtGetNextModule(somSelf) \
			SOM_Resolve(somSelf,SOMTModuleEntryC,somtGetNextModule)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetNextModule
				#if defined(_somtGetNextModule)
					#undef _somtGetNextModule
					#define SOMGD__somtGetNextModule
				#else
					#define _somtGetNextModule SOMTModuleEntryC_somtGetNextModule
				#endif
			#endif /* SOMGD__somtGetNextModule */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTModuleEntryC_somtGetNextModule */
	#ifndef SOMTModuleEntryC_somtGetFirstModuleDef
		#define SOMTModuleEntryC_somtGetFirstModuleDef(somSelf) \
			SOM_Resolve(somSelf,SOMTModuleEntryC,somtGetFirstModuleDef)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetFirstModuleDef
				#if defined(_somtGetFirstModuleDef)
					#undef _somtGetFirstModuleDef
					#define SOMGD__somtGetFirstModuleDef
				#else
					#define _somtGetFirstModuleDef SOMTModuleEntryC_somtGetFirstModuleDef
				#endif
			#endif /* SOMGD__somtGetFirstModuleDef */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTModuleEntryC_somtGetFirstModuleDef */
	#ifndef SOMTModuleEntryC_somtGetNextModuleDef
		#define SOMTModuleEntryC_somtGetNextModuleDef(somSelf) \
			SOM_Resolve(somSelf,SOMTModuleEntryC,somtGetNextModuleDef)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetNextModuleDef
				#if defined(_somtGetNextModuleDef)
					#undef _somtGetNextModuleDef
					#define SOMGD__somtGetNextModuleDef
				#else
					#define _somtGetNextModuleDef SOMTModuleEntryC_somtGetNextModuleDef
				#endif
			#endif /* SOMGD__somtGetNextModuleDef */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTModuleEntryC_somtGetNextModuleDef */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define SOMTModuleEntryC_somInit SOMObject_somInit
#define SOMTModuleEntryC_somUninit SOMObject_somUninit
#define SOMTModuleEntryC_somFree SOMObject_somFree
#define SOMTModuleEntryC_somGetClass SOMObject_somGetClass
#define SOMTModuleEntryC_somGetClassName SOMObject_somGetClassName
#define SOMTModuleEntryC_somGetSize SOMObject_somGetSize
#define SOMTModuleEntryC_somIsA SOMObject_somIsA
#define SOMTModuleEntryC_somIsInstanceOf SOMObject_somIsInstanceOf
#define SOMTModuleEntryC_somRespondsTo SOMObject_somRespondsTo
#define SOMTModuleEntryC_somDispatch SOMObject_somDispatch
#define SOMTModuleEntryC_somClassDispatch SOMObject_somClassDispatch
#define SOMTModuleEntryC_somCastObj SOMObject_somCastObj
#define SOMTModuleEntryC_somResetObj SOMObject_somResetObj
#define SOMTModuleEntryC_somPrintSelf SOMObject_somPrintSelf
#define SOMTModuleEntryC_somDumpSelf SOMObject_somDumpSelf
#define SOMTModuleEntryC_somDumpSelfInt SOMObject_somDumpSelfInt
#define SOMTModuleEntryC_somDefaultInit SOMObject_somDefaultInit
#define SOMTModuleEntryC_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define SOMTModuleEntryC_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define SOMTModuleEntryC_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define SOMTModuleEntryC_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define SOMTModuleEntryC_somDefaultAssign SOMObject_somDefaultAssign
#define SOMTModuleEntryC_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define SOMTModuleEntryC_somDefaultVAssign SOMObject_somDefaultVAssign
#define SOMTModuleEntryC_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define SOMTModuleEntryC_somDestruct SOMObject_somDestruct
#define SOMTModuleEntryC__get_somtEntryStruct SOMTEntryC__get_somtEntryStruct
#define SOMTModuleEntryC__set_somtEntryName SOMTEntryC__set_somtEntryName
#define SOMTModuleEntryC__get_somtEntryName SOMTEntryC__get_somtEntryName
#define SOMTModuleEntryC__set_somtElementType SOMTEntryC__set_somtElementType
#define SOMTModuleEntryC__get_somtElementType SOMTEntryC__get_somtElementType
#define SOMTModuleEntryC__get_somtElementTypeName SOMTEntryC__get_somtElementTypeName
#define SOMTModuleEntryC__get_somtEntryComment SOMTEntryC__get_somtEntryComment
#define SOMTModuleEntryC__get_somtSourceLineNumber SOMTEntryC__get_somtSourceLineNumber
#define SOMTModuleEntryC__get_somtTypeCode SOMTEntryC__get_somtTypeCode
#define SOMTModuleEntryC__get_somtIsReference SOMTEntryC__get_somtIsReference
#define SOMTModuleEntryC__get_somtIDLScopedName SOMTEntryC__get_somtIDLScopedName
#define SOMTModuleEntryC__get_somtCScopedName SOMTEntryC__get_somtCScopedName
#define SOMTModuleEntryC_somtGetModifierValue SOMTEntryC_somtGetModifierValue
#define SOMTModuleEntryC_somtGetFirstModifier SOMTEntryC_somtGetFirstModifier
#define SOMTModuleEntryC_somtGetNextModifier SOMTEntryC_somtGetNextModifier
#define SOMTModuleEntryC_somtFormatModifier SOMTEntryC_somtFormatModifier
#define SOMTModuleEntryC_somtGetModifierList SOMTEntryC_somtGetModifierList
#define SOMTModuleEntryC_somtSetSymbolsOnEntry SOMTEntryC_somtSetSymbolsOnEntry
#define SOMTModuleEntryC_somtSetEntryStruct SOMTEntryC_somtSetEntryStruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_scmodule_Header_h */
