/* generated from somproxy.idl */
/* internal conditional is SOM_Module_somproxy_Source */
#ifndef SOM_Module_somproxy_Header_h
	#define SOM_Module_somproxy_Header_h 1
	#include <som.h>
	#include <somcls.h>
	#include <somobj.h>
	#ifndef _IDL_SOMMProxyFor_defined
		#define _IDL_SOMMProxyFor_defined
		typedef SOMObject SOMMProxyFor;
	#endif /* _IDL_SOMMProxyFor_defined */
	#ifndef _IDL_SEQUENCE_SOMMProxyFor_defined
		#define _IDL_SEQUENCE_SOMMProxyFor_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMMProxyFor SOMSTAR ,sequence(SOMMProxyFor));
	#endif /* _IDL_SEQUENCE_SOMMProxyFor_defined */
	#ifndef _IDL_SOMMProxyForObject_defined
		#define _IDL_SOMMProxyForObject_defined
		typedef SOMObject SOMMProxyForObject;
	#endif /* _IDL_SOMMProxyForObject_defined */
	#ifndef _IDL_SEQUENCE_SOMMProxyForObject_defined
		#define _IDL_SEQUENCE_SOMMProxyForObject_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMMProxyForObject SOMSTAR ,sequence(SOMMProxyForObject));
	#endif /* _IDL_SEQUENCE_SOMMProxyForObject_defined */
	#ifdef __IBMC__
		typedef SOMClass SOMSTAR (somTP_SOMMProxyFor_sommMakeProxyClass)(
			SOMMProxyFor SOMSTAR somSelf,
			/* in */ SOMClass SOMSTAR targetClass,
			/* in */ string className);
		#pragma linkage(somTP_SOMMProxyFor_sommMakeProxyClass,system)
		typedef somTP_SOMMProxyFor_sommMakeProxyClass *somTD_SOMMProxyFor_sommMakeProxyClass;
	#else /* __IBMC__ */
		typedef SOMClass SOMSTAR (SOMLINK * somTD_SOMMProxyFor_sommMakeProxyClass)(
			SOMMProxyFor SOMSTAR somSelf,
			/* in */ SOMClass SOMSTAR targetClass,
			/* in */ string className);
	#endif /* __IBMC__ */
	#ifndef SOMMProxyFor_MajorVersion
		#define SOMMProxyFor_MajorVersion   0
	#endif /* SOMMProxyFor_MajorVersion */
	#ifndef SOMMProxyFor_MinorVersion
		#define SOMMProxyFor_MinorVersion   0
	#endif /* SOMMProxyFor_MinorVersion */
	typedef struct SOMMProxyForClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken sommMakeProxyClass;
	} SOMMProxyForClassDataStructure;
	typedef struct SOMMProxyForCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} SOMMProxyForCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_somproxy_Source) || defined(SOMMProxyFor_Class_Source)
			SOMEXTERN struct SOMMProxyForClassDataStructure _SOMMProxyForClassData;
			#ifndef SOMMProxyForClassData
				#define SOMMProxyForClassData    _SOMMProxyForClassData
			#endif /* SOMMProxyForClassData */
		#else
			SOMEXTERN struct SOMMProxyForClassDataStructure * SOMLINK resolve_SOMMProxyForClassData(void);
			#ifndef SOMMProxyForClassData
				#define SOMMProxyForClassData    (*(resolve_SOMMProxyForClassData()))
			#endif /* SOMMProxyForClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_somproxy_Source) || defined(SOMMProxyFor_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_somproxy_Source || SOMMProxyFor_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_somproxy_Source || SOMMProxyFor_Class_Source */
		struct SOMMProxyForClassDataStructure SOMDLINK SOMMProxyForClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_somproxy_Source) || defined(SOMMProxyFor_Class_Source)
			SOMEXTERN struct SOMMProxyForCClassDataStructure _SOMMProxyForCClassData;
			#ifndef SOMMProxyForCClassData
				#define SOMMProxyForCClassData    _SOMMProxyForCClassData
			#endif /* SOMMProxyForCClassData */
		#else
			SOMEXTERN struct SOMMProxyForCClassDataStructure * SOMLINK resolve_SOMMProxyForCClassData(void);
			#ifndef SOMMProxyForCClassData
				#define SOMMProxyForCClassData    (*(resolve_SOMMProxyForCClassData()))
			#endif /* SOMMProxyForCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_somproxy_Source) || defined(SOMMProxyFor_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_somproxy_Source || SOMMProxyFor_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_somproxy_Source || SOMMProxyFor_Class_Source */
		struct SOMMProxyForCClassDataStructure SOMDLINK SOMMProxyForCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_somproxy_Source) || defined(SOMMProxyFor_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_somproxy_Source || SOMMProxyFor_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_somproxy_Source || SOMMProxyFor_Class_Source */
	SOMClass SOMSTAR SOMLINK SOMMProxyForNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_SOMMProxyFor (SOMMProxyForClassData.classObject)
	#ifndef SOMGD_SOMMProxyFor
		#if (defined(_SOMMProxyFor) || defined(__SOMMProxyFor))
			#undef _SOMMProxyFor
			#undef __SOMMProxyFor
			#define SOMGD_SOMMProxyFor 1
		#else
			#define _SOMMProxyFor _SOMCLASS_SOMMProxyFor
		#endif /* _SOMMProxyFor */
	#endif /* SOMGD_SOMMProxyFor */
	#define SOMMProxyFor_classObj _SOMCLASS_SOMMProxyFor
	#define _SOMMTOKEN_SOMMProxyFor(method) ((somMToken)(SOMMProxyForClassData.method))
	#ifndef SOMMProxyForNew
		#define SOMMProxyForNew() ( _SOMMProxyFor ? \
			(SOMClass_somNew(_SOMMProxyFor)) : \
			( SOMMProxyForNewClass( \
				SOMMProxyFor_MajorVersion, \
				SOMMProxyFor_MinorVersion), \
			SOMClass_somNew(_SOMMProxyFor))) 
	#endif /* NewSOMMProxyFor */
	#ifndef SOMMProxyFor_sommMakeProxyClass
		#define SOMMProxyFor_sommMakeProxyClass(somSelf,targetClass,className) \
			SOM_Resolve(somSelf,SOMMProxyFor,sommMakeProxyClass)  \
				(somSelf,targetClass,className)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__sommMakeProxyClass
				#if defined(_sommMakeProxyClass)
					#undef _sommMakeProxyClass
					#define SOMGD__sommMakeProxyClass
				#else
					#define _sommMakeProxyClass SOMMProxyFor_sommMakeProxyClass
				#endif
			#endif /* SOMGD__sommMakeProxyClass */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMMProxyFor_sommMakeProxyClass */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define SOMMProxyFor_somInit SOMObject_somInit
#define SOMMProxyFor_somUninit SOMObject_somUninit
#define SOMMProxyFor_somFree SOMObject_somFree
#define SOMMProxyFor_somGetClass SOMObject_somGetClass
#define SOMMProxyFor_somGetClassName SOMObject_somGetClassName
#define SOMMProxyFor_somGetSize SOMObject_somGetSize
#define SOMMProxyFor_somIsA SOMObject_somIsA
#define SOMMProxyFor_somIsInstanceOf SOMObject_somIsInstanceOf
#define SOMMProxyFor_somRespondsTo SOMObject_somRespondsTo
#define SOMMProxyFor_somDispatch SOMObject_somDispatch
#define SOMMProxyFor_somClassDispatch SOMObject_somClassDispatch
#define SOMMProxyFor_somCastObj SOMObject_somCastObj
#define SOMMProxyFor_somResetObj SOMObject_somResetObj
#define SOMMProxyFor_somPrintSelf SOMObject_somPrintSelf
#define SOMMProxyFor_somDumpSelf SOMObject_somDumpSelf
#define SOMMProxyFor_somDumpSelfInt SOMObject_somDumpSelfInt
#define SOMMProxyFor_somDefaultInit SOMObject_somDefaultInit
#define SOMMProxyFor_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define SOMMProxyFor_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define SOMMProxyFor_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define SOMMProxyFor_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define SOMMProxyFor_somDefaultAssign SOMObject_somDefaultAssign
#define SOMMProxyFor_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define SOMMProxyFor_somDefaultVAssign SOMObject_somDefaultVAssign
#define SOMMProxyFor_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define SOMMProxyFor_somDestruct SOMObject_somDestruct
#define SOMMProxyFor_somNew SOMClass_somNew
#define SOMMProxyFor_somNewNoInit SOMClass_somNewNoInit
#define SOMMProxyFor_somRenew SOMClass_somRenew
#define SOMMProxyFor_somRenewNoInit SOMClass_somRenewNoInit
#define SOMMProxyFor_somRenewNoZero SOMClass_somRenewNoZero
#define SOMMProxyFor_somRenewNoInitNoZero SOMClass_somRenewNoInitNoZero
#define SOMMProxyFor_somAllocate SOMClass_somAllocate
#define SOMMProxyFor_somDeallocate SOMClass_somDeallocate
#define SOMMProxyFor_somGetParent SOMClass_somGetParent
#define SOMMProxyFor_somClassOfNewClassWithParents SOMClass_somClassOfNewClassWithParents
#define SOMMProxyFor_somInitClass SOMClass_somInitClass
#define SOMMProxyFor_somInitMIClass SOMClass_somInitMIClass
#define SOMMProxyFor_somAddStaticMethod SOMClass_somAddStaticMethod
#define SOMMProxyFor_somOverrideSMethod SOMClass_somOverrideSMethod
#define SOMMProxyFor_somClassReady SOMClass_somClassReady
#define SOMMProxyFor_somAddDynamicMethod SOMClass_somAddDynamicMethod
#define SOMMProxyFor_somGetName SOMClass_somGetName
#define SOMMProxyFor_somGetVersionNumbers SOMClass_somGetVersionNumbers
#define SOMMProxyFor_somGetNumMethods SOMClass_somGetNumMethods
#define SOMMProxyFor_somGetNumStaticMethods SOMClass_somGetNumStaticMethods
#define SOMMProxyFor_somGetParents SOMClass_somGetParents
#define SOMMProxyFor_somGetInstanceSize SOMClass_somGetInstanceSize
#define SOMMProxyFor_somGetInstancePartSize SOMClass_somGetInstancePartSize
#define SOMMProxyFor_somGetInstanceToken SOMClass_somGetInstanceToken
#define SOMMProxyFor_somGetMemberToken SOMClass_somGetMemberToken
#define SOMMProxyFor_somGetClassMtab SOMClass_somGetClassMtab
#define SOMMProxyFor_somGetClassData SOMClass_somGetClassData
#define SOMMProxyFor_somSetClassData SOMClass_somSetClassData
#define SOMMProxyFor_somSetMethodDescriptor SOMClass_somSetMethodDescriptor
#define SOMMProxyFor__get_somDataAlignment SOMClass__get_somDataAlignment
#define SOMMProxyFor__get_somInstanceDataOffsets SOMClass__get_somInstanceDataOffsets
#define SOMMProxyFor__set_somDirectInitClasses SOMClass__set_somDirectInitClasses
#define SOMMProxyFor__get_somDirectInitClasses SOMClass__get_somDirectInitClasses
#define SOMMProxyFor_somGetMethodDescriptor SOMClass_somGetMethodDescriptor
#define SOMMProxyFor_somGetMethodIndex SOMClass_somGetMethodIndex
#define SOMMProxyFor_somGetMethodToken SOMClass_somGetMethodToken
#define SOMMProxyFor_somGetNthMethodInfo SOMClass_somGetNthMethodInfo
#define SOMMProxyFor_somGetMethodData SOMClass_somGetMethodData
#define SOMMProxyFor_somGetNthMethodData SOMClass_somGetNthMethodData
#define SOMMProxyFor_somFindMethod SOMClass_somFindMethod
#define SOMMProxyFor_somFindMethodOk SOMClass_somFindMethodOk
#define SOMMProxyFor_somFindSMethod SOMClass_somFindSMethod
#define SOMMProxyFor_somFindSMethodOk SOMClass_somFindSMethodOk
#define SOMMProxyFor_somLookupMethod SOMClass_somLookupMethod
#define SOMMProxyFor_somGetApplyStub SOMClass_somGetApplyStub
#define SOMMProxyFor_somGetPClsMtab SOMClass_somGetPClsMtab
#define SOMMProxyFor_somCheckVersion SOMClass_somCheckVersion
#define SOMMProxyFor_somDescendedFrom SOMClass_somDescendedFrom
#define SOMMProxyFor_somSupportsMethod SOMClass_somSupportsMethod
#define SOMMProxyFor_somDefinedMethod SOMClass_somDefinedMethod
#define SOMMProxyFor_somGetRdStub SOMClass_somGetRdStub
#define SOMMProxyFor_somOverrideMtab SOMClass_somOverrideMtab
#define SOMMProxyFor__get_somClassAllocate SOMClass__get_somClassAllocate
#define SOMMProxyFor__get_somClassDeallocate SOMClass__get_somClassDeallocate
#endif /* SOM_DONT_USE_INHERITED_MACROS */

	typedef struct SOMMProxyForObject_sommProxyDispatchInfo
	{
		somMethodData md;
		somMethodPtr dispatchFcn;
	} SOMMProxyForObject_sommProxyDispatchInfo;
	#ifndef _IDL_SEQUENCE_SOMMProxyForObject_sommProxyDispatchInfo_defined
		#define _IDL_SEQUENCE_SOMMProxyForObject_sommProxyDispatchInfo_defined
		SOM_SEQUENCE_TYPEDEF(SOMMProxyForObject_sommProxyDispatchInfo);
	#endif /* _IDL_SEQUENCE_SOMMProxyForObject_sommProxyDispatchInfo_defined */
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_sommProxyDispatchInfo
			#if defined(sommProxyDispatchInfo)
				#undef sommProxyDispatchInfo
				#define SOMGD_sommProxyDispatchInfo
			#else
				#define sommProxyDispatchInfo SOMMProxyForObject_sommProxyDispatchInfo
			#endif
		#endif /* SOMGD_sommProxyDispatchInfo */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef _IDL_SEQUENCE_sommProxyDispatchInfo_defined
			#define _IDL_SEQUENCE_sommProxyDispatchInfo_defined
			#define _IDL_SEQUENCE_sommProxyDispatchInfo _IDL_SEQUENCE_SOMMProxyForObject_sommProxyDispatchInfo
		#endif /* _IDL_SEQUENCE_sommProxyDispatchInfo_defined */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
	#ifdef __IBMC__
		typedef void (somTP_SOMMProxyForObject__set_sommProxyTarget)(
			SOMMProxyForObject SOMSTAR somSelf,
			/* in */ SOMObject SOMSTAR sommProxyTarget);
		#pragma linkage(somTP_SOMMProxyForObject__set_sommProxyTarget,system)
		typedef somTP_SOMMProxyForObject__set_sommProxyTarget *somTD_SOMMProxyForObject__set_sommProxyTarget;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMMProxyForObject__set_sommProxyTarget)(
			SOMMProxyForObject SOMSTAR somSelf,
			/* in */ SOMObject SOMSTAR sommProxyTarget);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMObject SOMSTAR (somTP_SOMMProxyForObject__get_sommProxyTarget)(
			SOMMProxyForObject SOMSTAR somSelf);
		#pragma linkage(somTP_SOMMProxyForObject__get_sommProxyTarget,system)
		typedef somTP_SOMMProxyForObject__get_sommProxyTarget *somTD_SOMMProxyForObject__get_sommProxyTarget;
	#else /* __IBMC__ */
		typedef SOMObject SOMSTAR (SOMLINK * somTD_SOMMProxyForObject__get_sommProxyTarget)(
			SOMMProxyForObject SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef boolean (somTP_SOMMProxyForObject_sommProxyDispatch)(
			SOMMProxyForObject SOMSTAR somSelf,
			/* out */ somToken *returnBufferPointer,
			/* in */ SOMMProxyForObject_sommProxyDispatchInfo *dispatchInfo,
			/* in */ va_list ap);
		#pragma linkage(somTP_SOMMProxyForObject_sommProxyDispatch,system)
		typedef somTP_SOMMProxyForObject_sommProxyDispatch *somTD_SOMMProxyForObject_sommProxyDispatch;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_SOMMProxyForObject_sommProxyDispatch)(
			SOMMProxyForObject SOMSTAR somSelf,
			/* out */ somToken *returnBufferPointer,
			/* in */ SOMMProxyForObject_sommProxyDispatchInfo *dispatchInfo,
			/* in */ va_list ap);
	#endif /* __IBMC__ */
	#ifndef SOMMProxyForObject_MajorVersion
		#define SOMMProxyForObject_MajorVersion   0
	#endif /* SOMMProxyForObject_MajorVersion */
	#ifndef SOMMProxyForObject_MinorVersion
		#define SOMMProxyForObject_MinorVersion   0
	#endif /* SOMMProxyForObject_MinorVersion */
	typedef struct SOMMProxyForObjectClassDataStructure
	{
		SOMMProxyFor SOMSTAR classObject;
		somMToken _get_sommProxyTarget;
		somMToken _set_sommProxyTarget;
		somMToken sommProxyDispatch;
	} SOMMProxyForObjectClassDataStructure;
	typedef struct SOMMProxyForObjectCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
		somMethodPtr sommProxyDispatch;
	} SOMMProxyForObjectCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_somproxy_Source) || defined(SOMMProxyForObject_Class_Source)
			SOMEXTERN struct SOMMProxyForObjectClassDataStructure _SOMMProxyForObjectClassData;
			#ifndef SOMMProxyForObjectClassData
				#define SOMMProxyForObjectClassData    _SOMMProxyForObjectClassData
			#endif /* SOMMProxyForObjectClassData */
		#else
			SOMEXTERN struct SOMMProxyForObjectClassDataStructure * SOMLINK resolve_SOMMProxyForObjectClassData(void);
			#ifndef SOMMProxyForObjectClassData
				#define SOMMProxyForObjectClassData    (*(resolve_SOMMProxyForObjectClassData()))
			#endif /* SOMMProxyForObjectClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_somproxy_Source) || defined(SOMMProxyForObject_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_somproxy_Source || SOMMProxyForObject_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_somproxy_Source || SOMMProxyForObject_Class_Source */
		struct SOMMProxyForObjectClassDataStructure SOMDLINK SOMMProxyForObjectClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_somproxy_Source) || defined(SOMMProxyForObject_Class_Source)
			SOMEXTERN struct SOMMProxyForObjectCClassDataStructure _SOMMProxyForObjectCClassData;
			#ifndef SOMMProxyForObjectCClassData
				#define SOMMProxyForObjectCClassData    _SOMMProxyForObjectCClassData
			#endif /* SOMMProxyForObjectCClassData */
		#else
			SOMEXTERN struct SOMMProxyForObjectCClassDataStructure * SOMLINK resolve_SOMMProxyForObjectCClassData(void);
			#ifndef SOMMProxyForObjectCClassData
				#define SOMMProxyForObjectCClassData    (*(resolve_SOMMProxyForObjectCClassData()))
			#endif /* SOMMProxyForObjectCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_somproxy_Source) || defined(SOMMProxyForObject_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_somproxy_Source || SOMMProxyForObject_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_somproxy_Source || SOMMProxyForObject_Class_Source */
		struct SOMMProxyForObjectCClassDataStructure SOMDLINK SOMMProxyForObjectCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_somproxy_Source) || defined(SOMMProxyForObject_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_somproxy_Source || SOMMProxyForObject_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_somproxy_Source || SOMMProxyForObject_Class_Source */
	SOMMProxyFor SOMSTAR SOMLINK SOMMProxyForObjectNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_SOMMProxyForObject (SOMMProxyForObjectClassData.classObject)
	#ifndef SOMGD_SOMMProxyForObject
		#if (defined(_SOMMProxyForObject) || defined(__SOMMProxyForObject))
			#undef _SOMMProxyForObject
			#undef __SOMMProxyForObject
			#define SOMGD_SOMMProxyForObject 1
		#else
			#define _SOMMProxyForObject _SOMCLASS_SOMMProxyForObject
		#endif /* _SOMMProxyForObject */
	#endif /* SOMGD_SOMMProxyForObject */
	#define SOMMProxyForObject_classObj _SOMCLASS_SOMMProxyForObject
	#define _SOMMTOKEN_SOMMProxyForObject(method) ((somMToken)(SOMMProxyForObjectClassData.method))
	#ifndef SOMMProxyForObjectNew
		#define SOMMProxyForObjectNew() ( _SOMMProxyForObject ? \
			(SOMClass_somNew(_SOMMProxyForObject)) : \
			( SOMMProxyForObjectNewClass( \
				SOMMProxyForObject_MajorVersion, \
				SOMMProxyForObject_MinorVersion), \
			SOMClass_somNew(_SOMMProxyForObject))) 
	#endif /* NewSOMMProxyForObject */
	#ifndef SOMMProxyForObject__set_sommProxyTarget
		#define SOMMProxyForObject__set_sommProxyTarget(somSelf,sommProxyTarget) \
			SOM_Resolve(somSelf,SOMMProxyForObject,_set_sommProxyTarget)  \
				(somSelf,sommProxyTarget)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_sommProxyTarget
				#if defined(__set_sommProxyTarget)
					#undef __set_sommProxyTarget
					#define SOMGD___set_sommProxyTarget
				#else
					#define __set_sommProxyTarget SOMMProxyForObject__set_sommProxyTarget
				#endif
			#endif /* SOMGD___set_sommProxyTarget */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMMProxyForObject__set_sommProxyTarget */
	#ifndef SOMMProxyForObject__get_sommProxyTarget
		#define SOMMProxyForObject__get_sommProxyTarget(somSelf) \
			SOM_Resolve(somSelf,SOMMProxyForObject,_get_sommProxyTarget)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_sommProxyTarget
				#if defined(__get_sommProxyTarget)
					#undef __get_sommProxyTarget
					#define SOMGD___get_sommProxyTarget
				#else
					#define __get_sommProxyTarget SOMMProxyForObject__get_sommProxyTarget
				#endif
			#endif /* SOMGD___get_sommProxyTarget */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMMProxyForObject__get_sommProxyTarget */
	#ifndef SOMMProxyForObject_sommProxyDispatch
		#define SOMMProxyForObject_sommProxyDispatch(somSelf,returnBufferPointer,dispatchInfo,ap) \
			SOM_Resolve(somSelf,SOMMProxyForObject,sommProxyDispatch)  \
				(somSelf,returnBufferPointer,dispatchInfo,ap)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__sommProxyDispatch
				#if defined(_sommProxyDispatch)
					#undef _sommProxyDispatch
					#define SOMGD__sommProxyDispatch
				#else
					#define _sommProxyDispatch SOMMProxyForObject_sommProxyDispatch
				#endif
			#endif /* SOMGD__sommProxyDispatch */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMMProxyForObject_sommProxyDispatch */
	#ifndef va_SOMMProxyForObject_sommProxyDispatch
		#define va_SOMMProxyForObject_sommProxyDispatch somva_SOMMProxyForObject_sommProxyDispatch
	#endif /* va_SOMMProxyForObject_sommProxyDispatch */
	#ifdef SOMMProxyForObject_VA_EXTERN
		SOMEXTERN
		#ifdef SOMMProxyForObject_VA_STUBS
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else
			#if defined(SOM_Module_somproxy_Source) || defined(SOMMProxyForObject_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_somproxy_Source || SOMMProxyForObject_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_somproxy_Source || SOMMProxyForObject_Class_Source */
		#endif /* SOMMProxyForObject_VA_STUBS */
		boolean SOMLINK somva_SOMMProxyForObject_sommProxyDispatch(SOMMProxyForObject SOMSTAR somSelf,
			somToken *returnBufferPointer,
			SOMMProxyForObject_sommProxyDispatchInfo *dispatchInfo,
			...);
	#endif
	#if (defined(SOMMProxyForObject_VA_STUBS)||!defined(SOMMProxyForObject_VA_EXTERN))
		#ifdef SOMMProxyForObject_VA_EXTERN
			SOMEXTERN
		#else
			static
		#endif
		boolean SOMLINK somva_SOMMProxyForObject_sommProxyDispatch(SOMMProxyForObject SOMSTAR somSelf,
				somToken *returnBufferPointer,
				SOMMProxyForObject_sommProxyDispatchInfo *dispatchInfo,
				...)
		{
			boolean __result;
			va_list ap;
			va_start(ap,dispatchInfo);
			__result=SOM_Resolve(somSelf,SOMMProxyForObject,sommProxyDispatch)
					(somSelf,returnBufferPointer,dispatchInfo,ap);
			va_end(ap);
			return __result;
		}
	#endif

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define SOMMProxyForObject_somInit SOMObject_somInit
#define SOMMProxyForObject_somUninit SOMObject_somUninit
#define SOMMProxyForObject_somFree SOMObject_somFree
#define SOMMProxyForObject_somGetClass SOMObject_somGetClass
#define SOMMProxyForObject_somGetClassName SOMObject_somGetClassName
#define SOMMProxyForObject_somGetSize SOMObject_somGetSize
#define SOMMProxyForObject_somIsA SOMObject_somIsA
#define SOMMProxyForObject_somIsInstanceOf SOMObject_somIsInstanceOf
#define SOMMProxyForObject_somRespondsTo SOMObject_somRespondsTo
#define SOMMProxyForObject_somDispatch SOMObject_somDispatch
#define SOMMProxyForObject_somClassDispatch SOMObject_somClassDispatch
#define SOMMProxyForObject_somCastObj SOMObject_somCastObj
#define SOMMProxyForObject_somResetObj SOMObject_somResetObj
#define SOMMProxyForObject_somPrintSelf SOMObject_somPrintSelf
#define SOMMProxyForObject_somDumpSelf SOMObject_somDumpSelf
#define SOMMProxyForObject_somDumpSelfInt SOMObject_somDumpSelfInt
#define SOMMProxyForObject_somDefaultInit SOMObject_somDefaultInit
#define SOMMProxyForObject_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define SOMMProxyForObject_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define SOMMProxyForObject_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define SOMMProxyForObject_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define SOMMProxyForObject_somDefaultAssign SOMObject_somDefaultAssign
#define SOMMProxyForObject_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define SOMMProxyForObject_somDefaultVAssign SOMObject_somDefaultVAssign
#define SOMMProxyForObject_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define SOMMProxyForObject_somDestruct SOMObject_somDestruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_somproxy_Header_h */
