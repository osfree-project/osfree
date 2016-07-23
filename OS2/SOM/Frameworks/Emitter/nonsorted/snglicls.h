/* generated from snglicls.idl */
/* internal conditional is SOM_Module_snglicls_Source */
#ifndef SOM_Module_snglicls_Header_h
	#define SOM_Module_snglicls_Header_h 1
	#include <som.h>
	#include <somobj.h>
	#include <somcls.h>
	#ifndef _IDL_SOMMSingleInstance_defined
		#define _IDL_SOMMSingleInstance_defined
		typedef SOMObject SOMMSingleInstance;
	#endif /* _IDL_SOMMSingleInstance_defined */
	#ifndef _IDL_SEQUENCE_SOMMSingleInstance_defined
		#define _IDL_SEQUENCE_SOMMSingleInstance_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMMSingleInstance SOMSTAR ,sequence(SOMMSingleInstance));
	#endif /* _IDL_SEQUENCE_SOMMSingleInstance_defined */
	#ifdef __IBMC__
		typedef SOMObject SOMSTAR (somTP_SOMMSingleInstance_sommGetSingleInstance)(
			SOMMSingleInstance SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_SOMMSingleInstance_sommGetSingleInstance,system)
		typedef somTP_SOMMSingleInstance_sommGetSingleInstance *somTD_SOMMSingleInstance_sommGetSingleInstance;
	#else /* __IBMC__ */
		typedef SOMObject SOMSTAR (SOMLINK * somTD_SOMMSingleInstance_sommGetSingleInstance)(
			SOMMSingleInstance SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMMSingleInstance_sommFreeSingleInstance)(
			SOMMSingleInstance SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_SOMMSingleInstance_sommFreeSingleInstance,system)
		typedef somTP_SOMMSingleInstance_sommFreeSingleInstance *somTD_SOMMSingleInstance_sommFreeSingleInstance;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMMSingleInstance_sommFreeSingleInstance)(
			SOMMSingleInstance SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMMSingleInstance_sommSingleInstanceFreed)(
			SOMMSingleInstance SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMObject SOMSTAR obj);
		#pragma linkage(somTP_SOMMSingleInstance_sommSingleInstanceFreed,system)
		typedef somTP_SOMMSingleInstance_sommSingleInstanceFreed *somTD_SOMMSingleInstance_sommSingleInstanceFreed;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMMSingleInstance_sommSingleInstanceFreed)(
			SOMMSingleInstance SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMObject SOMSTAR obj);
	#endif /* __IBMC__ */
	#ifndef SOMMSingleInstance_MajorVersion
		#define SOMMSingleInstance_MajorVersion   2
	#endif /* SOMMSingleInstance_MajorVersion */
	#ifndef SOMMSingleInstance_MinorVersion
		#define SOMMSingleInstance_MinorVersion   2
	#endif /* SOMMSingleInstance_MinorVersion */
	typedef struct SOMMSingleInstanceClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken sommGetSingleInstance;
		somMToken sommFreeSingleInstance;
		somTD_SOMMSingleInstance_sommSingleInstanceFreed sommSingleInstanceFreed;
	} SOMMSingleInstanceClassDataStructure;
	typedef struct SOMMSingleInstanceCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} SOMMSingleInstanceCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_snglicls_Source) || defined(SOMMSingleInstance_Class_Source)
			SOMEXTERN struct SOMMSingleInstanceClassDataStructure _SOMMSingleInstanceClassData;
			#ifndef SOMMSingleInstanceClassData
				#define SOMMSingleInstanceClassData    _SOMMSingleInstanceClassData
			#endif /* SOMMSingleInstanceClassData */
		#else
			SOMEXTERN struct SOMMSingleInstanceClassDataStructure * SOMLINK resolve_SOMMSingleInstanceClassData(void);
			#ifndef SOMMSingleInstanceClassData
				#define SOMMSingleInstanceClassData    (*(resolve_SOMMSingleInstanceClassData()))
			#endif /* SOMMSingleInstanceClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_snglicls_Source) || defined(SOMMSingleInstance_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_snglicls_Source || SOMMSingleInstance_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_snglicls_Source || SOMMSingleInstance_Class_Source */
		struct SOMMSingleInstanceClassDataStructure SOMDLINK SOMMSingleInstanceClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_snglicls_Source) || defined(SOMMSingleInstance_Class_Source)
			SOMEXTERN struct SOMMSingleInstanceCClassDataStructure _SOMMSingleInstanceCClassData;
			#ifndef SOMMSingleInstanceCClassData
				#define SOMMSingleInstanceCClassData    _SOMMSingleInstanceCClassData
			#endif /* SOMMSingleInstanceCClassData */
		#else
			SOMEXTERN struct SOMMSingleInstanceCClassDataStructure * SOMLINK resolve_SOMMSingleInstanceCClassData(void);
			#ifndef SOMMSingleInstanceCClassData
				#define SOMMSingleInstanceCClassData    (*(resolve_SOMMSingleInstanceCClassData()))
			#endif /* SOMMSingleInstanceCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_snglicls_Source) || defined(SOMMSingleInstance_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_snglicls_Source || SOMMSingleInstance_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_snglicls_Source || SOMMSingleInstance_Class_Source */
		struct SOMMSingleInstanceCClassDataStructure SOMDLINK SOMMSingleInstanceCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_snglicls_Source) || defined(SOMMSingleInstance_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_snglicls_Source || SOMMSingleInstance_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_snglicls_Source || SOMMSingleInstance_Class_Source */
	SOMClass SOMSTAR SOMLINK SOMMSingleInstanceNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_SOMMSingleInstance (SOMMSingleInstanceClassData.classObject)
	#ifndef SOMGD_SOMMSingleInstance
		#if (defined(_SOMMSingleInstance) || defined(__SOMMSingleInstance))
			#undef _SOMMSingleInstance
			#undef __SOMMSingleInstance
			#define SOMGD_SOMMSingleInstance 1
		#else
			#define _SOMMSingleInstance _SOMCLASS_SOMMSingleInstance
		#endif /* _SOMMSingleInstance */
	#endif /* SOMGD_SOMMSingleInstance */
	#define SOMMSingleInstance_classObj _SOMCLASS_SOMMSingleInstance
	#define _SOMMTOKEN_SOMMSingleInstance(method) ((somMToken)(SOMMSingleInstanceClassData.method))
	#ifndef SOMMSingleInstanceNew
		#define SOMMSingleInstanceNew() ( _SOMMSingleInstance ? \
			(SOMClass_somNew(_SOMMSingleInstance)) : \
			( SOMMSingleInstanceNewClass( \
				SOMMSingleInstance_MajorVersion, \
				SOMMSingleInstance_MinorVersion), \
			SOMClass_somNew(_SOMMSingleInstance))) 
	#endif /* NewSOMMSingleInstance */
	#ifndef SOMMSingleInstance_sommGetSingleInstance
		#define SOMMSingleInstance_sommGetSingleInstance(somSelf,ev) \
			SOM_Resolve(somSelf,SOMMSingleInstance,sommGetSingleInstance)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__sommGetSingleInstance
				#if defined(_sommGetSingleInstance)
					#undef _sommGetSingleInstance
					#define SOMGD__sommGetSingleInstance
				#else
					#define _sommGetSingleInstance SOMMSingleInstance_sommGetSingleInstance
				#endif
			#endif /* SOMGD__sommGetSingleInstance */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMMSingleInstance_sommGetSingleInstance */
	#ifndef SOMMSingleInstance_sommFreeSingleInstance
		#define SOMMSingleInstance_sommFreeSingleInstance(somSelf,ev) \
			SOM_Resolve(somSelf,SOMMSingleInstance,sommFreeSingleInstance)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__sommFreeSingleInstance
				#if defined(_sommFreeSingleInstance)
					#undef _sommFreeSingleInstance
					#define SOMGD__sommFreeSingleInstance
				#else
					#define _sommFreeSingleInstance SOMMSingleInstance_sommFreeSingleInstance
				#endif
			#endif /* SOMGD__sommFreeSingleInstance */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMMSingleInstance_sommFreeSingleInstance */
	#ifndef SOMMSingleInstance_sommSingleInstanceFreed
		#define SOMMSingleInstance_sommSingleInstanceFreed SOMMSingleInstanceClassData.sommSingleInstanceFreed
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__sommSingleInstanceFreed
				#if defined(_sommSingleInstanceFreed)
					#undef _sommSingleInstanceFreed
					#define SOMGD__sommSingleInstanceFreed
				#else
					#define _sommSingleInstanceFreed SOMMSingleInstance_sommSingleInstanceFreed
				#endif
			#endif /* SOMGD__sommSingleInstanceFreed */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMMSingleInstance_sommSingleInstanceFreed */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define SOMMSingleInstance_somInit SOMObject_somInit
#define SOMMSingleInstance_somUninit SOMObject_somUninit
#define SOMMSingleInstance_somFree SOMObject_somFree
#define SOMMSingleInstance_somGetClass SOMObject_somGetClass
#define SOMMSingleInstance_somGetClassName SOMObject_somGetClassName
#define SOMMSingleInstance_somGetSize SOMObject_somGetSize
#define SOMMSingleInstance_somIsA SOMObject_somIsA
#define SOMMSingleInstance_somIsInstanceOf SOMObject_somIsInstanceOf
#define SOMMSingleInstance_somRespondsTo SOMObject_somRespondsTo
#define SOMMSingleInstance_somDispatch SOMObject_somDispatch
#define SOMMSingleInstance_somClassDispatch SOMObject_somClassDispatch
#define SOMMSingleInstance_somCastObj SOMObject_somCastObj
#define SOMMSingleInstance_somResetObj SOMObject_somResetObj
#define SOMMSingleInstance_somPrintSelf SOMObject_somPrintSelf
#define SOMMSingleInstance_somDumpSelf SOMObject_somDumpSelf
#define SOMMSingleInstance_somDumpSelfInt SOMObject_somDumpSelfInt
#define SOMMSingleInstance_somDefaultInit SOMObject_somDefaultInit
#define SOMMSingleInstance_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define SOMMSingleInstance_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define SOMMSingleInstance_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define SOMMSingleInstance_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define SOMMSingleInstance_somDefaultAssign SOMObject_somDefaultAssign
#define SOMMSingleInstance_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define SOMMSingleInstance_somDefaultVAssign SOMObject_somDefaultVAssign
#define SOMMSingleInstance_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define SOMMSingleInstance_somDestruct SOMObject_somDestruct
#define SOMMSingleInstance_somNew SOMClass_somNew
#define SOMMSingleInstance_somNewNoInit SOMClass_somNewNoInit
#define SOMMSingleInstance_somRenew SOMClass_somRenew
#define SOMMSingleInstance_somRenewNoInit SOMClass_somRenewNoInit
#define SOMMSingleInstance_somRenewNoZero SOMClass_somRenewNoZero
#define SOMMSingleInstance_somRenewNoInitNoZero SOMClass_somRenewNoInitNoZero
#define SOMMSingleInstance_somAllocate SOMClass_somAllocate
#define SOMMSingleInstance_somDeallocate SOMClass_somDeallocate
#define SOMMSingleInstance_somGetParent SOMClass_somGetParent
#define SOMMSingleInstance_somClassOfNewClassWithParents SOMClass_somClassOfNewClassWithParents
#define SOMMSingleInstance_somInitClass SOMClass_somInitClass
#define SOMMSingleInstance_somInitMIClass SOMClass_somInitMIClass
#define SOMMSingleInstance_somAddStaticMethod SOMClass_somAddStaticMethod
#define SOMMSingleInstance_somOverrideSMethod SOMClass_somOverrideSMethod
#define SOMMSingleInstance_somClassReady SOMClass_somClassReady
#define SOMMSingleInstance_somAddDynamicMethod SOMClass_somAddDynamicMethod
#define SOMMSingleInstance_somGetName SOMClass_somGetName
#define SOMMSingleInstance_somGetVersionNumbers SOMClass_somGetVersionNumbers
#define SOMMSingleInstance_somGetNumMethods SOMClass_somGetNumMethods
#define SOMMSingleInstance_somGetNumStaticMethods SOMClass_somGetNumStaticMethods
#define SOMMSingleInstance_somGetParents SOMClass_somGetParents
#define SOMMSingleInstance_somGetInstanceSize SOMClass_somGetInstanceSize
#define SOMMSingleInstance_somGetInstancePartSize SOMClass_somGetInstancePartSize
#define SOMMSingleInstance_somGetInstanceToken SOMClass_somGetInstanceToken
#define SOMMSingleInstance_somGetMemberToken SOMClass_somGetMemberToken
#define SOMMSingleInstance_somGetClassMtab SOMClass_somGetClassMtab
#define SOMMSingleInstance_somGetClassData SOMClass_somGetClassData
#define SOMMSingleInstance_somSetClassData SOMClass_somSetClassData
#define SOMMSingleInstance_somSetMethodDescriptor SOMClass_somSetMethodDescriptor
#define SOMMSingleInstance__get_somDataAlignment SOMClass__get_somDataAlignment
#define SOMMSingleInstance__get_somInstanceDataOffsets SOMClass__get_somInstanceDataOffsets
#define SOMMSingleInstance__set_somDirectInitClasses SOMClass__set_somDirectInitClasses
#define SOMMSingleInstance__get_somDirectInitClasses SOMClass__get_somDirectInitClasses
#define SOMMSingleInstance_somGetMethodDescriptor SOMClass_somGetMethodDescriptor
#define SOMMSingleInstance_somGetMethodIndex SOMClass_somGetMethodIndex
#define SOMMSingleInstance_somGetMethodToken SOMClass_somGetMethodToken
#define SOMMSingleInstance_somGetNthMethodInfo SOMClass_somGetNthMethodInfo
#define SOMMSingleInstance_somGetMethodData SOMClass_somGetMethodData
#define SOMMSingleInstance_somGetNthMethodData SOMClass_somGetNthMethodData
#define SOMMSingleInstance_somFindMethod SOMClass_somFindMethod
#define SOMMSingleInstance_somFindMethodOk SOMClass_somFindMethodOk
#define SOMMSingleInstance_somFindSMethod SOMClass_somFindSMethod
#define SOMMSingleInstance_somFindSMethodOk SOMClass_somFindSMethodOk
#define SOMMSingleInstance_somLookupMethod SOMClass_somLookupMethod
#define SOMMSingleInstance_somGetApplyStub SOMClass_somGetApplyStub
#define SOMMSingleInstance_somGetPClsMtab SOMClass_somGetPClsMtab
#define SOMMSingleInstance_somCheckVersion SOMClass_somCheckVersion
#define SOMMSingleInstance_somDescendedFrom SOMClass_somDescendedFrom
#define SOMMSingleInstance_somSupportsMethod SOMClass_somSupportsMethod
#define SOMMSingleInstance_somDefinedMethod SOMClass_somDefinedMethod
#define SOMMSingleInstance_somGetRdStub SOMClass_somGetRdStub
#define SOMMSingleInstance_somOverrideMtab SOMClass_somOverrideMtab
#define SOMMSingleInstance__get_somClassAllocate SOMClass__get_somClassAllocate
#define SOMMSingleInstance__get_somClassDeallocate SOMClass__get_somClassDeallocate
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_snglicls_Header_h */
