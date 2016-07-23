/* generated from def.idl */
/* internal conditional is SOM_Module_def_Source */
#ifndef SOM_Module_def_Header_h
	#define SOM_Module_def_Header_h 1
	#include <som.h>
	#include <scemit.h>
	#include <somobj.h>
	#include <sctmplt.h>
	#include <sctypes.h>
	#include <sm.h>
	#ifndef _IDL_DEFEmitter_defined
		#define _IDL_DEFEmitter_defined
		typedef SOMObject DEFEmitter;
	#endif /* _IDL_DEFEmitter_defined */
	#ifndef _IDL_SEQUENCE_DEFEmitter_defined
		#define _IDL_SEQUENCE_DEFEmitter_defined
		SOM_SEQUENCE_TYPEDEF_NAME(DEFEmitter SOMSTAR ,sequence(DEFEmitter));
	#endif /* _IDL_SEQUENCE_DEFEmitter_defined */
	#ifndef DEFEmitter_MajorVersion
		#define DEFEmitter_MajorVersion   0
	#endif /* DEFEmitter_MajorVersion */
	#ifndef DEFEmitter_MinorVersion
		#define DEFEmitter_MinorVersion   0
	#endif /* DEFEmitter_MinorVersion */
	typedef struct DEFEmitterClassDataStructure
	{
		SOMClass SOMSTAR classObject;
	} DEFEmitterClassDataStructure;
	typedef struct DEFEmitterCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} DEFEmitterCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_def_Source) || defined(DEFEmitter_Class_Source)
			SOMEXTERN struct DEFEmitterClassDataStructure _DEFEmitterClassData;
			#ifndef DEFEmitterClassData
				#define DEFEmitterClassData    _DEFEmitterClassData
			#endif /* DEFEmitterClassData */
		#else
			SOMEXTERN struct DEFEmitterClassDataStructure * SOMLINK resolve_DEFEmitterClassData(void);
			#ifndef DEFEmitterClassData
				#define DEFEmitterClassData    (*(resolve_DEFEmitterClassData()))
			#endif /* DEFEmitterClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		struct DEFEmitterClassDataStructure SOMDLINK DEFEmitterClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_def_Source) || defined(DEFEmitter_Class_Source)
			SOMEXTERN struct DEFEmitterCClassDataStructure _DEFEmitterCClassData;
			#ifndef DEFEmitterCClassData
				#define DEFEmitterCClassData    _DEFEmitterCClassData
			#endif /* DEFEmitterCClassData */
		#else
			SOMEXTERN struct DEFEmitterCClassDataStructure * SOMLINK resolve_DEFEmitterCClassData(void);
			#ifndef DEFEmitterCClassData
				#define DEFEmitterCClassData    (*(resolve_DEFEmitterCClassData()))
			#endif /* DEFEmitterCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		struct DEFEmitterCClassDataStructure SOMDLINK DEFEmitterCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	SOMClass SOMSTAR SOMLINK DEFEmitterNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_DEFEmitter (DEFEmitterClassData.classObject)
	#ifndef SOMGD_DEFEmitter
		#if (defined(_DEFEmitter) || defined(__DEFEmitter))
			#undef _DEFEmitter
			#undef __DEFEmitter
			#define SOMGD_DEFEmitter 1
		#else
			#define _DEFEmitter _SOMCLASS_DEFEmitter
		#endif /* _DEFEmitter */
	#endif /* SOMGD_DEFEmitter */
	#define DEFEmitter_classObj _SOMCLASS_DEFEmitter
	#define _SOMMTOKEN_DEFEmitter(method) ((somMToken)(DEFEmitterClassData.method))
	#ifndef DEFEmitterNew
		#define DEFEmitterNew() ( _DEFEmitter ? \
			(SOMClass_somNew(_DEFEmitter)) : \
			( DEFEmitterNewClass( \
				DEFEmitter_MajorVersion, \
				DEFEmitter_MinorVersion), \
			SOMClass_somNew(_DEFEmitter))) 
	#endif /* NewDEFEmitter */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define DEFEmitter_somInit SOMObject_somInit
#define DEFEmitter_somUninit SOMObject_somUninit
#define DEFEmitter_somFree SOMObject_somFree
#define DEFEmitter_somGetClass SOMObject_somGetClass
#define DEFEmitter_somGetClassName SOMObject_somGetClassName
#define DEFEmitter_somGetSize SOMObject_somGetSize
#define DEFEmitter_somIsA SOMObject_somIsA
#define DEFEmitter_somIsInstanceOf SOMObject_somIsInstanceOf
#define DEFEmitter_somRespondsTo SOMObject_somRespondsTo
#define DEFEmitter_somDispatch SOMObject_somDispatch
#define DEFEmitter_somClassDispatch SOMObject_somClassDispatch
#define DEFEmitter_somCastObj SOMObject_somCastObj
#define DEFEmitter_somResetObj SOMObject_somResetObj
#define DEFEmitter_somPrintSelf SOMObject_somPrintSelf
#define DEFEmitter_somDumpSelf SOMObject_somDumpSelf
#define DEFEmitter_somDumpSelfInt SOMObject_somDumpSelfInt
#define DEFEmitter_somDefaultInit SOMObject_somDefaultInit
#define DEFEmitter_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define DEFEmitter_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define DEFEmitter_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define DEFEmitter_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define DEFEmitter_somDefaultAssign SOMObject_somDefaultAssign
#define DEFEmitter_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define DEFEmitter_somDefaultVAssign SOMObject_somDefaultVAssign
#define DEFEmitter_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define DEFEmitter_somDestruct SOMObject_somDestruct
#define DEFEmitter__set_somtTemplate SOMTEmitC__set_somtTemplate
#define DEFEmitter__get_somtTemplate SOMTEmitC__get_somtTemplate
#define DEFEmitter__set_somtTargetFile SOMTEmitC__set_somtTargetFile
#define DEFEmitter__get_somtTargetFile SOMTEmitC__get_somtTargetFile
#define DEFEmitter__set_somtTargetClass SOMTEmitC__set_somtTargetClass
#define DEFEmitter__get_somtTargetClass SOMTEmitC__get_somtTargetClass
#define DEFEmitter__set_somtTargetModule SOMTEmitC__set_somtTargetModule
#define DEFEmitter__get_somtTargetModule SOMTEmitC__get_somtTargetModule
#define DEFEmitter__set_somtTargetType SOMTEmitC__set_somtTargetType
#define DEFEmitter__get_somtTargetType SOMTEmitC__get_somtTargetType
#define DEFEmitter__set_somtEmitterName SOMTEmitC__set_somtEmitterName
#define DEFEmitter__get_somtEmitterName SOMTEmitC__get_somtEmitterName
#define DEFEmitter_somtGenerateSections SOMTEmitC_somtGenerateSections
#define DEFEmitter_somtOpenSymbolsFile SOMTEmitC_somtOpenSymbolsFile
#define DEFEmitter_somtSetPredefinedSymbols SOMTEmitC_somtSetPredefinedSymbols
#define DEFEmitter_somtFileSymbols SOMTEmitC_somtFileSymbols
#define DEFEmitter_somtEmitProlog SOMTEmitC_somtEmitProlog
#define DEFEmitter_somtEmitBaseIncludesProlog SOMTEmitC_somtEmitBaseIncludesProlog
#define DEFEmitter_somtEmitBaseIncludes SOMTEmitC_somtEmitBaseIncludes
#define DEFEmitter_somtEmitBaseIncludesEpilog SOMTEmitC_somtEmitBaseIncludesEpilog
#define DEFEmitter_somtEmitMetaInclude SOMTEmitC_somtEmitMetaInclude
#define DEFEmitter_somtEmitClass SOMTEmitC_somtEmitClass
#define DEFEmitter_somtEmitMeta SOMTEmitC_somtEmitMeta
#define DEFEmitter_somtEmitBaseProlog SOMTEmitC_somtEmitBaseProlog
#define DEFEmitter_somtEmitBase SOMTEmitC_somtEmitBase
#define DEFEmitter_somtEmitBaseEpilog SOMTEmitC_somtEmitBaseEpilog
#define DEFEmitter_somtEmitPassthruProlog SOMTEmitC_somtEmitPassthruProlog
#define DEFEmitter_somtEmitPassthru SOMTEmitC_somtEmitPassthru
#define DEFEmitter_somtEmitPassthruEpilog SOMTEmitC_somtEmitPassthruEpilog
#define DEFEmitter_somtEmitRelease SOMTEmitC_somtEmitRelease
#define DEFEmitter_somtEmitDataProlog SOMTEmitC_somtEmitDataProlog
#define DEFEmitter_somtEmitData SOMTEmitC_somtEmitData
#define DEFEmitter_somtEmitDataEpilog SOMTEmitC_somtEmitDataEpilog
#define DEFEmitter_somtEmitAttributeProlog SOMTEmitC_somtEmitAttributeProlog
#define DEFEmitter_somtEmitAttribute SOMTEmitC_somtEmitAttribute
#define DEFEmitter_somtEmitAttributeEpilog SOMTEmitC_somtEmitAttributeEpilog
#define DEFEmitter_somtEmitConstantProlog SOMTEmitC_somtEmitConstantProlog
#define DEFEmitter_somtEmitConstant SOMTEmitC_somtEmitConstant
#define DEFEmitter_somtEmitConstantEpilog SOMTEmitC_somtEmitConstantEpilog
#define DEFEmitter_somtEmitTypedefProlog SOMTEmitC_somtEmitTypedefProlog
#define DEFEmitter_somtEmitTypedef SOMTEmitC_somtEmitTypedef
#define DEFEmitter_somtEmitTypedefEpilog SOMTEmitC_somtEmitTypedefEpilog
#define DEFEmitter_somtEmitStructProlog SOMTEmitC_somtEmitStructProlog
#define DEFEmitter_somtEmitStruct SOMTEmitC_somtEmitStruct
#define DEFEmitter_somtEmitStructEpilog SOMTEmitC_somtEmitStructEpilog
#define DEFEmitter_somtEmitUnionProlog SOMTEmitC_somtEmitUnionProlog
#define DEFEmitter_somtEmitUnion SOMTEmitC_somtEmitUnion
#define DEFEmitter_somtEmitUnionEpilog SOMTEmitC_somtEmitUnionEpilog
#define DEFEmitter_somtEmitEnumProlog SOMTEmitC_somtEmitEnumProlog
#define DEFEmitter_somtEmitEnum SOMTEmitC_somtEmitEnum
#define DEFEmitter_somtEmitEnumEpilog SOMTEmitC_somtEmitEnumEpilog
#define DEFEmitter_somtEmitInterfaceProlog SOMTEmitC_somtEmitInterfaceProlog
#define DEFEmitter_somtEmitInterface SOMTEmitC_somtEmitInterface
#define DEFEmitter_somtEmitInterfaceEpilog SOMTEmitC_somtEmitInterfaceEpilog
#define DEFEmitter_somtEmitModuleProlog SOMTEmitC_somtEmitModuleProlog
#define DEFEmitter_somtEmitModule SOMTEmitC_somtEmitModule
#define DEFEmitter_somtEmitModuleEpilog SOMTEmitC_somtEmitModuleEpilog
#define DEFEmitter_somtEmitMethodsProlog SOMTEmitC_somtEmitMethodsProlog
#define DEFEmitter_somtEmitMethods SOMTEmitC_somtEmitMethods
#define DEFEmitter_somtEmitMethodsEpilog SOMTEmitC_somtEmitMethodsEpilog
#define DEFEmitter_somtEmitMethod SOMTEmitC_somtEmitMethod
#define DEFEmitter_somtEmitEpilog SOMTEmitC_somtEmitEpilog
#define DEFEmitter_somtScanBases SOMTEmitC_somtScanBases
#define DEFEmitter_somtScanBaseIncludes SOMTEmitC_somtScanBaseIncludes
#define DEFEmitter_somtCheckVisibility SOMTEmitC_somtCheckVisibility
#define DEFEmitter_somtNew SOMTEmitC_somtNew
#define DEFEmitter_somtImplemented SOMTEmitC_somtImplemented
#define DEFEmitter_somtOverridden SOMTEmitC_somtOverridden
#define DEFEmitter_somtInherited SOMTEmitC_somtInherited
#define DEFEmitter_somtAllVisible SOMTEmitC_somtAllVisible
#define DEFEmitter_somtAll SOMTEmitC_somtAll
#define DEFEmitter_somtNewNoProc SOMTEmitC_somtNewNoProc
#define DEFEmitter_somtPrivOrPub SOMTEmitC_somtPrivOrPub
#define DEFEmitter_somtNewProc SOMTEmitC_somtNewProc
#define DEFEmitter_somtLink SOMTEmitC_somtLink
#define DEFEmitter_somtVA SOMTEmitC_somtVA
#define DEFEmitter_somtScanMethods SOMTEmitC_somtScanMethods
#define DEFEmitter_somtScanConstants SOMTEmitC_somtScanConstants
#define DEFEmitter_somtScanTypedefs SOMTEmitC_somtScanTypedefs
#define DEFEmitter_somtScanStructs SOMTEmitC_somtScanStructs
#define DEFEmitter_somtScanUnions SOMTEmitC_somtScanUnions
#define DEFEmitter_somtScanEnums SOMTEmitC_somtScanEnums
#define DEFEmitter_somtScanData SOMTEmitC_somtScanData
#define DEFEmitter_somtScanAttributes SOMTEmitC_somtScanAttributes
#define DEFEmitter_somtScanInterfaces SOMTEmitC_somtScanInterfaces
#define DEFEmitter_somtScanModules SOMTEmitC_somtScanModules
#define DEFEmitter_somtScanPassthru SOMTEmitC_somtScanPassthru
#define DEFEmitter_somtEmitFullPassthru SOMTEmitC_somtEmitFullPassthru
#define DEFEmitter_somtScanDataF SOMTEmitC_somtScanDataF
#define DEFEmitter_somtScanBasesF SOMTEmitC_somtScanBasesF
#define DEFEmitter_somtGetGlobalModifierValue SOMTEmitC_somtGetGlobalModifierValue
#define DEFEmitter_somtGetFirstGlobalDefinition SOMTEmitC_somtGetFirstGlobalDefinition
#define DEFEmitter_somtGetNextGlobalDefinition SOMTEmitC_somtGetNextGlobalDefinition
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_def_Header_h */
