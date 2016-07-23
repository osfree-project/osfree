/* generated from scmethod.idl */
/* internal conditional is SOM_Module_scmethod_Source */
#ifndef SOM_Module_scmethod_Header_h
	#define SOM_Module_scmethod_Header_h 1
	#include <som.h>
	#include <sccommon.h>
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
	#ifndef _IDL_SOMTParameterEntryC_defined
		#define _IDL_SOMTParameterEntryC_defined
		typedef SOMObject SOMTParameterEntryC;
	#endif /* _IDL_SOMTParameterEntryC_defined */
	#ifndef _IDL_SEQUENCE_SOMTParameterEntryC_defined
		#define _IDL_SEQUENCE_SOMTParameterEntryC_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMTParameterEntryC SOMSTAR ,sequence(SOMTParameterEntryC));
	#endif /* _IDL_SEQUENCE_SOMTParameterEntryC_defined */
	#ifndef _IDL_SOMTDataEntryC_defined
		#define _IDL_SOMTDataEntryC_defined
		typedef SOMObject SOMTDataEntryC;
	#endif /* _IDL_SOMTDataEntryC_defined */
	#ifndef _IDL_SEQUENCE_SOMTDataEntryC_defined
		#define _IDL_SEQUENCE_SOMTDataEntryC_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMTDataEntryC SOMSTAR ,sequence(SOMTDataEntryC));
	#endif /* _IDL_SEQUENCE_SOMTDataEntryC_defined */
	#ifndef _IDL_SOMTStructEntryC_defined
		#define _IDL_SOMTStructEntryC_defined
		typedef SOMObject SOMTStructEntryC;
	#endif /* _IDL_SOMTStructEntryC_defined */
	#ifndef _IDL_SEQUENCE_SOMTStructEntryC_defined
		#define _IDL_SEQUENCE_SOMTStructEntryC_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMTStructEntryC SOMSTAR ,sequence(SOMTStructEntryC));
	#endif /* _IDL_SEQUENCE_SOMTStructEntryC_defined */
	#ifndef _IDL_SOMTMethodEntryC_defined
		#define _IDL_SOMTMethodEntryC_defined
		typedef SOMObject SOMTMethodEntryC;
	#endif /* _IDL_SOMTMethodEntryC_defined */
	#ifndef _IDL_SEQUENCE_SOMTMethodEntryC_defined
		#define _IDL_SEQUENCE_SOMTMethodEntryC_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMTMethodEntryC SOMSTAR ,sequence(SOMTMethodEntryC));
	#endif /* _IDL_SEQUENCE_SOMTMethodEntryC_defined */
	#ifdef __IBMC__
		typedef boolean (somTP_SOMTMethodEntryC__get_somtIsVarargs)(
			SOMTMethodEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTMethodEntryC__get_somtIsVarargs,system)
		typedef somTP_SOMTMethodEntryC__get_somtIsVarargs *somTD_SOMTMethodEntryC__get_somtIsVarargs;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_SOMTMethodEntryC__get_somtIsVarargs)(
			SOMTMethodEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMTMethodEntryC SOMSTAR (somTP_SOMTMethodEntryC__get_somtOriginalMethod)(
			SOMTMethodEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTMethodEntryC__get_somtOriginalMethod,system)
		typedef somTP_SOMTMethodEntryC__get_somtOriginalMethod *somTD_SOMTMethodEntryC__get_somtOriginalMethod;
	#else /* __IBMC__ */
		typedef SOMTMethodEntryC SOMSTAR (SOMLINK * somTD_SOMTMethodEntryC__get_somtOriginalMethod)(
			SOMTMethodEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMTClassEntryC SOMSTAR (somTP_SOMTMethodEntryC__get_somtOriginalClass)(
			SOMTMethodEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTMethodEntryC__get_somtOriginalClass,system)
		typedef somTP_SOMTMethodEntryC__get_somtOriginalClass *somTD_SOMTMethodEntryC__get_somtOriginalClass;
	#else /* __IBMC__ */
		typedef SOMTClassEntryC SOMSTAR (SOMLINK * somTD_SOMTMethodEntryC__get_somtOriginalClass)(
			SOMTMethodEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMTEntryC SOMSTAR (somTP_SOMTMethodEntryC__get_somtMethodGroup)(
			SOMTMethodEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTMethodEntryC__get_somtMethodGroup,system)
		typedef somTP_SOMTMethodEntryC__get_somtMethodGroup *somTD_SOMTMethodEntryC__get_somtMethodGroup;
	#else /* __IBMC__ */
		typedef SOMTEntryC SOMSTAR (SOMLINK * somTD_SOMTMethodEntryC__get_somtMethodGroup)(
			SOMTMethodEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef boolean (somTP_SOMTMethodEntryC__get_somtIsPrivateMethod)(
			SOMTMethodEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTMethodEntryC__get_somtIsPrivateMethod,system)
		typedef somTP_SOMTMethodEntryC__get_somtIsPrivateMethod *somTD_SOMTMethodEntryC__get_somtIsPrivateMethod;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_SOMTMethodEntryC__get_somtIsPrivateMethod)(
			SOMTMethodEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef boolean (somTP_SOMTMethodEntryC__get_somtIsOneway)(
			SOMTMethodEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTMethodEntryC__get_somtIsOneway,system)
		typedef somTP_SOMTMethodEntryC__get_somtIsOneway *somTD_SOMTMethodEntryC__get_somtIsOneway;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_SOMTMethodEntryC__get_somtIsOneway)(
			SOMTMethodEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef short (somTP_SOMTMethodEntryC__get_somtArgCount)(
			SOMTMethodEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTMethodEntryC__get_somtArgCount,system)
		typedef somTP_SOMTMethodEntryC__get_somtArgCount *somTD_SOMTMethodEntryC__get_somtArgCount;
	#else /* __IBMC__ */
		typedef short (SOMLINK * somTD_SOMTMethodEntryC__get_somtArgCount)(
			SOMTMethodEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMTParameterEntryC SOMSTAR (somTP_SOMTMethodEntryC_somtGetFirstParameter)(
			SOMTMethodEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTMethodEntryC_somtGetFirstParameter,system)
		typedef somTP_SOMTMethodEntryC_somtGetFirstParameter *somTD_SOMTMethodEntryC_somtGetFirstParameter;
	#else /* __IBMC__ */
		typedef SOMTParameterEntryC SOMSTAR (SOMLINK * somTD_SOMTMethodEntryC_somtGetFirstParameter)(
			SOMTMethodEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMTParameterEntryC SOMSTAR (somTP_SOMTMethodEntryC_somtGetNextParameter)(
			SOMTMethodEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTMethodEntryC_somtGetNextParameter,system)
		typedef somTP_SOMTMethodEntryC_somtGetNextParameter *somTD_SOMTMethodEntryC_somtGetNextParameter;
	#else /* __IBMC__ */
		typedef SOMTParameterEntryC SOMSTAR (SOMLINK * somTD_SOMTMethodEntryC_somtGetNextParameter)(
			SOMTMethodEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_SOMTMethodEntryC_somtGetIDLParamList)(
			SOMTMethodEntryC SOMSTAR somSelf,
			/* in */ string buffer);
		#pragma linkage(somTP_SOMTMethodEntryC_somtGetIDLParamList,system)
		typedef somTP_SOMTMethodEntryC_somtGetIDLParamList *somTD_SOMTMethodEntryC_somtGetIDLParamList;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_SOMTMethodEntryC_somtGetIDLParamList)(
			SOMTMethodEntryC SOMSTAR somSelf,
			/* in */ string buffer);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_SOMTMethodEntryC_somtGetShortCParamList)(
			SOMTMethodEntryC SOMSTAR somSelf,
			/* in */ string buffer,
			/* in */ string selfParm,
			/* in */ string varargsParm);
		#pragma linkage(somTP_SOMTMethodEntryC_somtGetShortCParamList,system)
		typedef somTP_SOMTMethodEntryC_somtGetShortCParamList *somTD_SOMTMethodEntryC_somtGetShortCParamList;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_SOMTMethodEntryC_somtGetShortCParamList)(
			SOMTMethodEntryC SOMSTAR somSelf,
			/* in */ string buffer,
			/* in */ string selfParm,
			/* in */ string varargsParm);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_SOMTMethodEntryC_somtGetFullCParamList)(
			SOMTMethodEntryC SOMSTAR somSelf,
			/* in */ string buffer,
			/* in */ string varargsParm);
		#pragma linkage(somTP_SOMTMethodEntryC_somtGetFullCParamList,system)
		typedef somTP_SOMTMethodEntryC_somtGetFullCParamList *somTD_SOMTMethodEntryC_somtGetFullCParamList;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_SOMTMethodEntryC_somtGetFullCParamList)(
			SOMTMethodEntryC SOMSTAR somSelf,
			/* in */ string buffer,
			/* in */ string varargsParm);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_SOMTMethodEntryC_somtGetShortParamNameList)(
			SOMTMethodEntryC SOMSTAR somSelf,
			/* in */ string buffer,
			/* in */ string selfParm,
			/* in */ string varargsParm);
		#pragma linkage(somTP_SOMTMethodEntryC_somtGetShortParamNameList,system)
		typedef somTP_SOMTMethodEntryC_somtGetShortParamNameList *somTD_SOMTMethodEntryC_somtGetShortParamNameList;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_SOMTMethodEntryC_somtGetShortParamNameList)(
			SOMTMethodEntryC SOMSTAR somSelf,
			/* in */ string buffer,
			/* in */ string selfParm,
			/* in */ string varargsParm);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_SOMTMethodEntryC_somtGetFullParamNameList)(
			SOMTMethodEntryC SOMSTAR somSelf,
			/* in */ string buffer,
			/* in */ string varargsParm);
		#pragma linkage(somTP_SOMTMethodEntryC_somtGetFullParamNameList,system)
		typedef somTP_SOMTMethodEntryC_somtGetFullParamNameList *somTD_SOMTMethodEntryC_somtGetFullParamNameList;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_SOMTMethodEntryC_somtGetFullParamNameList)(
			SOMTMethodEntryC SOMSTAR somSelf,
			/* in */ string buffer,
			/* in */ string varargsParm);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMTParameterEntryC SOMSTAR (somTP_SOMTMethodEntryC_somtGetNthParameter)(
			SOMTMethodEntryC SOMSTAR somSelf,
			/* in */ short n);
		#pragma linkage(somTP_SOMTMethodEntryC_somtGetNthParameter,system)
		typedef somTP_SOMTMethodEntryC_somtGetNthParameter *somTD_SOMTMethodEntryC_somtGetNthParameter;
	#else /* __IBMC__ */
		typedef SOMTParameterEntryC SOMSTAR (SOMLINK * somTD_SOMTMethodEntryC_somtGetNthParameter)(
			SOMTMethodEntryC SOMSTAR somSelf,
			/* in */ short n);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMTStructEntryC SOMSTAR (somTP_SOMTMethodEntryC_somtGetFirstException)(
			SOMTMethodEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTMethodEntryC_somtGetFirstException,system)
		typedef somTP_SOMTMethodEntryC_somtGetFirstException *somTD_SOMTMethodEntryC_somtGetFirstException;
	#else /* __IBMC__ */
		typedef SOMTStructEntryC SOMSTAR (SOMLINK * somTD_SOMTMethodEntryC_somtGetFirstException)(
			SOMTMethodEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMTStructEntryC SOMSTAR (somTP_SOMTMethodEntryC_somtGetNextException)(
			SOMTMethodEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTMethodEntryC_somtGetNextException,system)
		typedef somTP_SOMTMethodEntryC_somtGetNextException *somTD_SOMTMethodEntryC_somtGetNextException;
	#else /* __IBMC__ */
		typedef SOMTStructEntryC SOMSTAR (SOMLINK * somTD_SOMTMethodEntryC_somtGetNextException)(
			SOMTMethodEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string *(somTP_SOMTMethodEntryC__get_somtContextArray)(
			SOMTMethodEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTMethodEntryC__get_somtContextArray,system)
		typedef somTP_SOMTMethodEntryC__get_somtContextArray *somTD_SOMTMethodEntryC__get_somtContextArray;
	#else /* __IBMC__ */
		typedef string *(SOMLINK * somTD_SOMTMethodEntryC__get_somtContextArray)(
			SOMTMethodEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_SOMTMethodEntryC__get_somtCReturnType)(
			SOMTMethodEntryC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMTMethodEntryC__get_somtCReturnType,system)
		typedef somTP_SOMTMethodEntryC__get_somtCReturnType *somTD_SOMTMethodEntryC__get_somtCReturnType;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_SOMTMethodEntryC__get_somtCReturnType)(
			SOMTMethodEntryC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifndef SOMTMethodEntryC_MajorVersion
		#define SOMTMethodEntryC_MajorVersion   2
	#endif /* SOMTMethodEntryC_MajorVersion */
	#ifndef SOMTMethodEntryC_MinorVersion
		#define SOMTMethodEntryC_MinorVersion   1
	#endif /* SOMTMethodEntryC_MinorVersion */
	typedef struct SOMTMethodEntryCClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken _get_somtIsVarargs;
		somMToken _get_somtOriginalMethod;
		somMToken _get_somtOriginalClass;
		somMToken _get_somtMethodGroup;
		somMToken somtGetFirstParameter;
		somMToken somtGetNextParameter;
		somMToken somtGetArgList;
		somMToken somtGetParmList;
		somMToken _get_somtIsPrivateMethod;
		somMToken _get_somtIsOneway;
		somMToken _get_somtArgCount;
		somMToken somtGetFirstException;
		somMToken somtGetNextException;
		somMToken _get_somtContextArray;
		somMToken somtGetShortCPrototype;
		somMToken somtGetFullCPrototype;
		somMToken somtGetShortParmList;
		somMToken somtGetFullParmList;
		somMToken somtGetNthParameter;
		somMToken _get_somtCReturnType;
		somMToken somtGetIDLParamList;
		somMToken somtGetShortCParamList;
		somMToken somtGetFullCParamList;
		somMToken somtGetShortParamNameList;
		somMToken somtGetFullParamNameList;
	} SOMTMethodEntryCClassDataStructure;
	typedef struct SOMTMethodEntryCCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} SOMTMethodEntryCCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_scmethod_Source) || defined(SOMTMethodEntryC_Class_Source)
			SOMEXTERN struct SOMTMethodEntryCClassDataStructure _SOMTMethodEntryCClassData;
			#ifndef SOMTMethodEntryCClassData
				#define SOMTMethodEntryCClassData    _SOMTMethodEntryCClassData
			#endif /* SOMTMethodEntryCClassData */
		#else
			SOMEXTERN struct SOMTMethodEntryCClassDataStructure * SOMLINK resolve_SOMTMethodEntryCClassData(void);
			#ifndef SOMTMethodEntryCClassData
				#define SOMTMethodEntryCClassData    (*(resolve_SOMTMethodEntryCClassData()))
			#endif /* SOMTMethodEntryCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_scmethod_Source) || defined(SOMTMethodEntryC_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_scmethod_Source || SOMTMethodEntryC_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_scmethod_Source || SOMTMethodEntryC_Class_Source */
		struct SOMTMethodEntryCClassDataStructure SOMDLINK SOMTMethodEntryCClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_scmethod_Source) || defined(SOMTMethodEntryC_Class_Source)
			SOMEXTERN struct SOMTMethodEntryCCClassDataStructure _SOMTMethodEntryCCClassData;
			#ifndef SOMTMethodEntryCCClassData
				#define SOMTMethodEntryCCClassData    _SOMTMethodEntryCCClassData
			#endif /* SOMTMethodEntryCCClassData */
		#else
			SOMEXTERN struct SOMTMethodEntryCCClassDataStructure * SOMLINK resolve_SOMTMethodEntryCCClassData(void);
			#ifndef SOMTMethodEntryCCClassData
				#define SOMTMethodEntryCCClassData    (*(resolve_SOMTMethodEntryCCClassData()))
			#endif /* SOMTMethodEntryCCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_scmethod_Source) || defined(SOMTMethodEntryC_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_scmethod_Source || SOMTMethodEntryC_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_scmethod_Source || SOMTMethodEntryC_Class_Source */
		struct SOMTMethodEntryCCClassDataStructure SOMDLINK SOMTMethodEntryCCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_scmethod_Source) || defined(SOMTMethodEntryC_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_scmethod_Source || SOMTMethodEntryC_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_scmethod_Source || SOMTMethodEntryC_Class_Source */
	SOMClass SOMSTAR SOMLINK SOMTMethodEntryCNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_SOMTMethodEntryC (SOMTMethodEntryCClassData.classObject)
	#ifndef SOMGD_SOMTMethodEntryC
		#if (defined(_SOMTMethodEntryC) || defined(__SOMTMethodEntryC))
			#undef _SOMTMethodEntryC
			#undef __SOMTMethodEntryC
			#define SOMGD_SOMTMethodEntryC 1
		#else
			#define _SOMTMethodEntryC _SOMCLASS_SOMTMethodEntryC
		#endif /* _SOMTMethodEntryC */
	#endif /* SOMGD_SOMTMethodEntryC */
	#define SOMTMethodEntryC_classObj _SOMCLASS_SOMTMethodEntryC
	#define _SOMMTOKEN_SOMTMethodEntryC(method) ((somMToken)(SOMTMethodEntryCClassData.method))
	#ifndef SOMTMethodEntryCNew
		#define SOMTMethodEntryCNew() ( _SOMTMethodEntryC ? \
			(SOMClass_somNew(_SOMTMethodEntryC)) : \
			( SOMTMethodEntryCNewClass( \
				SOMTMethodEntryC_MajorVersion, \
				SOMTMethodEntryC_MinorVersion), \
			SOMClass_somNew(_SOMTMethodEntryC))) 
	#endif /* NewSOMTMethodEntryC */
	#ifndef SOMTMethodEntryC__get_somtIsVarargs
		#define SOMTMethodEntryC__get_somtIsVarargs(somSelf) \
			SOM_Resolve(somSelf,SOMTMethodEntryC,_get_somtIsVarargs)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtIsVarargs
				#if defined(__get_somtIsVarargs)
					#undef __get_somtIsVarargs
					#define SOMGD___get_somtIsVarargs
				#else
					#define __get_somtIsVarargs SOMTMethodEntryC__get_somtIsVarargs
				#endif
			#endif /* SOMGD___get_somtIsVarargs */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTMethodEntryC__get_somtIsVarargs */
	#ifndef SOMTMethodEntryC__get_somtOriginalMethod
		#define SOMTMethodEntryC__get_somtOriginalMethod(somSelf) \
			SOM_Resolve(somSelf,SOMTMethodEntryC,_get_somtOriginalMethod)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtOriginalMethod
				#if defined(__get_somtOriginalMethod)
					#undef __get_somtOriginalMethod
					#define SOMGD___get_somtOriginalMethod
				#else
					#define __get_somtOriginalMethod SOMTMethodEntryC__get_somtOriginalMethod
				#endif
			#endif /* SOMGD___get_somtOriginalMethod */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTMethodEntryC__get_somtOriginalMethod */
	#ifndef SOMTMethodEntryC__get_somtOriginalClass
		#define SOMTMethodEntryC__get_somtOriginalClass(somSelf) \
			SOM_Resolve(somSelf,SOMTMethodEntryC,_get_somtOriginalClass)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtOriginalClass
				#if defined(__get_somtOriginalClass)
					#undef __get_somtOriginalClass
					#define SOMGD___get_somtOriginalClass
				#else
					#define __get_somtOriginalClass SOMTMethodEntryC__get_somtOriginalClass
				#endif
			#endif /* SOMGD___get_somtOriginalClass */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTMethodEntryC__get_somtOriginalClass */
	#ifndef SOMTMethodEntryC__get_somtMethodGroup
		#define SOMTMethodEntryC__get_somtMethodGroup(somSelf) \
			SOM_Resolve(somSelf,SOMTMethodEntryC,_get_somtMethodGroup)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtMethodGroup
				#if defined(__get_somtMethodGroup)
					#undef __get_somtMethodGroup
					#define SOMGD___get_somtMethodGroup
				#else
					#define __get_somtMethodGroup SOMTMethodEntryC__get_somtMethodGroup
				#endif
			#endif /* SOMGD___get_somtMethodGroup */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTMethodEntryC__get_somtMethodGroup */
	#ifndef SOMTMethodEntryC__get_somtIsPrivateMethod
		#define SOMTMethodEntryC__get_somtIsPrivateMethod(somSelf) \
			SOM_Resolve(somSelf,SOMTMethodEntryC,_get_somtIsPrivateMethod)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtIsPrivateMethod
				#if defined(__get_somtIsPrivateMethod)
					#undef __get_somtIsPrivateMethod
					#define SOMGD___get_somtIsPrivateMethod
				#else
					#define __get_somtIsPrivateMethod SOMTMethodEntryC__get_somtIsPrivateMethod
				#endif
			#endif /* SOMGD___get_somtIsPrivateMethod */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTMethodEntryC__get_somtIsPrivateMethod */
	#ifndef SOMTMethodEntryC__get_somtIsOneway
		#define SOMTMethodEntryC__get_somtIsOneway(somSelf) \
			SOM_Resolve(somSelf,SOMTMethodEntryC,_get_somtIsOneway)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtIsOneway
				#if defined(__get_somtIsOneway)
					#undef __get_somtIsOneway
					#define SOMGD___get_somtIsOneway
				#else
					#define __get_somtIsOneway SOMTMethodEntryC__get_somtIsOneway
				#endif
			#endif /* SOMGD___get_somtIsOneway */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTMethodEntryC__get_somtIsOneway */
	#ifndef SOMTMethodEntryC__get_somtArgCount
		#define SOMTMethodEntryC__get_somtArgCount(somSelf) \
			SOM_Resolve(somSelf,SOMTMethodEntryC,_get_somtArgCount)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtArgCount
				#if defined(__get_somtArgCount)
					#undef __get_somtArgCount
					#define SOMGD___get_somtArgCount
				#else
					#define __get_somtArgCount SOMTMethodEntryC__get_somtArgCount
				#endif
			#endif /* SOMGD___get_somtArgCount */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTMethodEntryC__get_somtArgCount */
	#ifndef SOMTMethodEntryC_somtGetFirstParameter
		#define SOMTMethodEntryC_somtGetFirstParameter(somSelf) \
			SOM_Resolve(somSelf,SOMTMethodEntryC,somtGetFirstParameter)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetFirstParameter
				#if defined(_somtGetFirstParameter)
					#undef _somtGetFirstParameter
					#define SOMGD__somtGetFirstParameter
				#else
					#define _somtGetFirstParameter SOMTMethodEntryC_somtGetFirstParameter
				#endif
			#endif /* SOMGD__somtGetFirstParameter */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTMethodEntryC_somtGetFirstParameter */
	#ifndef SOMTMethodEntryC_somtGetNextParameter
		#define SOMTMethodEntryC_somtGetNextParameter(somSelf) \
			SOM_Resolve(somSelf,SOMTMethodEntryC,somtGetNextParameter)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetNextParameter
				#if defined(_somtGetNextParameter)
					#undef _somtGetNextParameter
					#define SOMGD__somtGetNextParameter
				#else
					#define _somtGetNextParameter SOMTMethodEntryC_somtGetNextParameter
				#endif
			#endif /* SOMGD__somtGetNextParameter */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTMethodEntryC_somtGetNextParameter */
	#ifndef SOMTMethodEntryC_somtGetIDLParamList
		#define SOMTMethodEntryC_somtGetIDLParamList(somSelf,buffer) \
			SOM_Resolve(somSelf,SOMTMethodEntryC,somtGetIDLParamList)  \
				(somSelf,buffer)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetIDLParamList
				#if defined(_somtGetIDLParamList)
					#undef _somtGetIDLParamList
					#define SOMGD__somtGetIDLParamList
				#else
					#define _somtGetIDLParamList SOMTMethodEntryC_somtGetIDLParamList
				#endif
			#endif /* SOMGD__somtGetIDLParamList */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTMethodEntryC_somtGetIDLParamList */
	#ifndef SOMTMethodEntryC_somtGetShortCParamList
		#define SOMTMethodEntryC_somtGetShortCParamList(somSelf,buffer,selfParm,varargsParm) \
			SOM_Resolve(somSelf,SOMTMethodEntryC,somtGetShortCParamList)  \
				(somSelf,buffer,selfParm,varargsParm)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetShortCParamList
				#if defined(_somtGetShortCParamList)
					#undef _somtGetShortCParamList
					#define SOMGD__somtGetShortCParamList
				#else
					#define _somtGetShortCParamList SOMTMethodEntryC_somtGetShortCParamList
				#endif
			#endif /* SOMGD__somtGetShortCParamList */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTMethodEntryC_somtGetShortCParamList */
	#ifndef SOMTMethodEntryC_somtGetFullCParamList
		#define SOMTMethodEntryC_somtGetFullCParamList(somSelf,buffer,varargsParm) \
			SOM_Resolve(somSelf,SOMTMethodEntryC,somtGetFullCParamList)  \
				(somSelf,buffer,varargsParm)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetFullCParamList
				#if defined(_somtGetFullCParamList)
					#undef _somtGetFullCParamList
					#define SOMGD__somtGetFullCParamList
				#else
					#define _somtGetFullCParamList SOMTMethodEntryC_somtGetFullCParamList
				#endif
			#endif /* SOMGD__somtGetFullCParamList */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTMethodEntryC_somtGetFullCParamList */
	#ifndef SOMTMethodEntryC_somtGetShortParamNameList
		#define SOMTMethodEntryC_somtGetShortParamNameList(somSelf,buffer,selfParm,varargsParm) \
			SOM_Resolve(somSelf,SOMTMethodEntryC,somtGetShortParamNameList)  \
				(somSelf,buffer,selfParm,varargsParm)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetShortParamNameList
				#if defined(_somtGetShortParamNameList)
					#undef _somtGetShortParamNameList
					#define SOMGD__somtGetShortParamNameList
				#else
					#define _somtGetShortParamNameList SOMTMethodEntryC_somtGetShortParamNameList
				#endif
			#endif /* SOMGD__somtGetShortParamNameList */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTMethodEntryC_somtGetShortParamNameList */
	#ifndef SOMTMethodEntryC_somtGetFullParamNameList
		#define SOMTMethodEntryC_somtGetFullParamNameList(somSelf,buffer,varargsParm) \
			SOM_Resolve(somSelf,SOMTMethodEntryC,somtGetFullParamNameList)  \
				(somSelf,buffer,varargsParm)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetFullParamNameList
				#if defined(_somtGetFullParamNameList)
					#undef _somtGetFullParamNameList
					#define SOMGD__somtGetFullParamNameList
				#else
					#define _somtGetFullParamNameList SOMTMethodEntryC_somtGetFullParamNameList
				#endif
			#endif /* SOMGD__somtGetFullParamNameList */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTMethodEntryC_somtGetFullParamNameList */
	#ifndef SOMTMethodEntryC_somtGetNthParameter
		#define SOMTMethodEntryC_somtGetNthParameter(somSelf,n) \
			SOM_Resolve(somSelf,SOMTMethodEntryC,somtGetNthParameter)  \
				(somSelf,n)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetNthParameter
				#if defined(_somtGetNthParameter)
					#undef _somtGetNthParameter
					#define SOMGD__somtGetNthParameter
				#else
					#define _somtGetNthParameter SOMTMethodEntryC_somtGetNthParameter
				#endif
			#endif /* SOMGD__somtGetNthParameter */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTMethodEntryC_somtGetNthParameter */
	#ifndef SOMTMethodEntryC_somtGetFirstException
		#define SOMTMethodEntryC_somtGetFirstException(somSelf) \
			SOM_Resolve(somSelf,SOMTMethodEntryC,somtGetFirstException)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetFirstException
				#if defined(_somtGetFirstException)
					#undef _somtGetFirstException
					#define SOMGD__somtGetFirstException
				#else
					#define _somtGetFirstException SOMTMethodEntryC_somtGetFirstException
				#endif
			#endif /* SOMGD__somtGetFirstException */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTMethodEntryC_somtGetFirstException */
	#ifndef SOMTMethodEntryC_somtGetNextException
		#define SOMTMethodEntryC_somtGetNextException(somSelf) \
			SOM_Resolve(somSelf,SOMTMethodEntryC,somtGetNextException)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somtGetNextException
				#if defined(_somtGetNextException)
					#undef _somtGetNextException
					#define SOMGD__somtGetNextException
				#else
					#define _somtGetNextException SOMTMethodEntryC_somtGetNextException
				#endif
			#endif /* SOMGD__somtGetNextException */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTMethodEntryC_somtGetNextException */
	#ifndef SOMTMethodEntryC__get_somtContextArray
		#define SOMTMethodEntryC__get_somtContextArray(somSelf) \
			SOM_Resolve(somSelf,SOMTMethodEntryC,_get_somtContextArray)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtContextArray
				#if defined(__get_somtContextArray)
					#undef __get_somtContextArray
					#define SOMGD___get_somtContextArray
				#else
					#define __get_somtContextArray SOMTMethodEntryC__get_somtContextArray
				#endif
			#endif /* SOMGD___get_somtContextArray */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTMethodEntryC__get_somtContextArray */
	#ifndef SOMTMethodEntryC__get_somtCReturnType
		#define SOMTMethodEntryC__get_somtCReturnType(somSelf) \
			SOM_Resolve(somSelf,SOMTMethodEntryC,_get_somtCReturnType)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somtCReturnType
				#if defined(__get_somtCReturnType)
					#undef __get_somtCReturnType
					#define SOMGD___get_somtCReturnType
				#else
					#define __get_somtCReturnType SOMTMethodEntryC__get_somtCReturnType
				#endif
			#endif /* SOMGD___get_somtCReturnType */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMTMethodEntryC__get_somtCReturnType */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define SOMTMethodEntryC_somInit SOMObject_somInit
#define SOMTMethodEntryC_somUninit SOMObject_somUninit
#define SOMTMethodEntryC_somFree SOMObject_somFree
#define SOMTMethodEntryC_somGetClass SOMObject_somGetClass
#define SOMTMethodEntryC_somGetClassName SOMObject_somGetClassName
#define SOMTMethodEntryC_somGetSize SOMObject_somGetSize
#define SOMTMethodEntryC_somIsA SOMObject_somIsA
#define SOMTMethodEntryC_somIsInstanceOf SOMObject_somIsInstanceOf
#define SOMTMethodEntryC_somRespondsTo SOMObject_somRespondsTo
#define SOMTMethodEntryC_somDispatch SOMObject_somDispatch
#define SOMTMethodEntryC_somClassDispatch SOMObject_somClassDispatch
#define SOMTMethodEntryC_somCastObj SOMObject_somCastObj
#define SOMTMethodEntryC_somResetObj SOMObject_somResetObj
#define SOMTMethodEntryC_somPrintSelf SOMObject_somPrintSelf
#define SOMTMethodEntryC_somDumpSelf SOMObject_somDumpSelf
#define SOMTMethodEntryC_somDumpSelfInt SOMObject_somDumpSelfInt
#define SOMTMethodEntryC_somDefaultInit SOMObject_somDefaultInit
#define SOMTMethodEntryC_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define SOMTMethodEntryC_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define SOMTMethodEntryC_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define SOMTMethodEntryC_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define SOMTMethodEntryC_somDefaultAssign SOMObject_somDefaultAssign
#define SOMTMethodEntryC_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define SOMTMethodEntryC_somDefaultVAssign SOMObject_somDefaultVAssign
#define SOMTMethodEntryC_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define SOMTMethodEntryC_somDestruct SOMObject_somDestruct
#define SOMTMethodEntryC__get_somtEntryStruct SOMTEntryC__get_somtEntryStruct
#define SOMTMethodEntryC__set_somtEntryName SOMTEntryC__set_somtEntryName
#define SOMTMethodEntryC__get_somtEntryName SOMTEntryC__get_somtEntryName
#define SOMTMethodEntryC__set_somtElementType SOMTEntryC__set_somtElementType
#define SOMTMethodEntryC__get_somtElementType SOMTEntryC__get_somtElementType
#define SOMTMethodEntryC__get_somtElementTypeName SOMTEntryC__get_somtElementTypeName
#define SOMTMethodEntryC__get_somtEntryComment SOMTEntryC__get_somtEntryComment
#define SOMTMethodEntryC__get_somtSourceLineNumber SOMTEntryC__get_somtSourceLineNumber
#define SOMTMethodEntryC__get_somtTypeCode SOMTEntryC__get_somtTypeCode
#define SOMTMethodEntryC__get_somtIsReference SOMTEntryC__get_somtIsReference
#define SOMTMethodEntryC__get_somtIDLScopedName SOMTEntryC__get_somtIDLScopedName
#define SOMTMethodEntryC__get_somtCScopedName SOMTEntryC__get_somtCScopedName
#define SOMTMethodEntryC_somtGetModifierValue SOMTEntryC_somtGetModifierValue
#define SOMTMethodEntryC_somtGetFirstModifier SOMTEntryC_somtGetFirstModifier
#define SOMTMethodEntryC_somtGetNextModifier SOMTEntryC_somtGetNextModifier
#define SOMTMethodEntryC_somtFormatModifier SOMTEntryC_somtFormatModifier
#define SOMTMethodEntryC_somtGetModifierList SOMTEntryC_somtGetModifierList
#define SOMTMethodEntryC_somtSetSymbolsOnEntry SOMTEntryC_somtSetSymbolsOnEntry
#define SOMTMethodEntryC_somtSetEntryStruct SOMTEntryC_somtSetEntryStruct
#define SOMTMethodEntryC__get_somtTypeObj SOMTCommonEntryC__get_somtTypeObj
#define SOMTMethodEntryC__get_somtPtrs SOMTCommonEntryC__get_somtPtrs
#define SOMTMethodEntryC__get_somtArrayDimsString SOMTCommonEntryC__get_somtArrayDimsString
#define SOMTMethodEntryC_somtGetFirstArrayDimension SOMTCommonEntryC_somtGetFirstArrayDimension
#define SOMTMethodEntryC_somtGetNextArrayDimension SOMTCommonEntryC_somtGetNextArrayDimension
#define SOMTMethodEntryC__get_somtSourceText SOMTCommonEntryC__get_somtSourceText
#define SOMTMethodEntryC__get_somtType SOMTCommonEntryC__get_somtType
#define SOMTMethodEntryC__get_somtVisibility SOMTCommonEntryC__get_somtVisibility
#define SOMTMethodEntryC_somtIsArray SOMTCommonEntryC_somtIsArray
#define SOMTMethodEntryC_somtIsPointer SOMTCommonEntryC_somtIsPointer
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_scmethod_Header_h */
