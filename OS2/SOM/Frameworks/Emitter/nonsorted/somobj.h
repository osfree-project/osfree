/* generated from somobj.idl */
/* internal conditional is SOM_Module_somobj_Source */
#ifndef SOM_Module_somobj_Header_h
	#define SOM_Module_somobj_Header_h 1
	#include <som.h>
	#ifndef _IDL_SOMClass_defined
		#define _IDL_SOMClass_defined
		typedef SOMObject SOMClass;
	#endif /* _IDL_SOMClass_defined */
	#ifndef _IDL_SEQUENCE_SOMClass_defined
		#define _IDL_SEQUENCE_SOMClass_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMClass SOMSTAR ,sequence(SOMClass));
	#endif /* _IDL_SEQUENCE_SOMClass_defined */
	#ifndef _IDL_SEQUENCE_SOMObject_defined
		#define _IDL_SEQUENCE_SOMObject_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMObject SOMSTAR ,sequence(SOMObject));
	#endif /* _IDL_SEQUENCE_SOMObject_defined */
	typedef _IDL_SEQUENCE_SOMObject SOMObject_SOMObjectSequence;
	#ifndef _IDL_SEQUENCE_SOMObject_SOMObjectSequence_defined
		#define _IDL_SEQUENCE_SOMObject_SOMObjectSequence_defined
		SOM_SEQUENCE_TYPEDEF(SOMObject_SOMObjectSequence);
	#endif /* _IDL_SEQUENCE_SOMObject_SOMObjectSequence_defined */
	#ifndef _IDL_SEQUENCE_octet_defined
		#define _IDL_SEQUENCE_octet_defined
		SOM_SEQUENCE_TYPEDEF(octet);
	#endif /* _IDL_SEQUENCE_octet_defined */
	typedef _IDL_SEQUENCE_octet SOMObject_BooleanSequence;
	#ifndef _IDL_SEQUENCE_SOMObject_BooleanSequence_defined
		#define _IDL_SEQUENCE_SOMObject_BooleanSequence_defined
		SOM_SEQUENCE_TYPEDEF(SOMObject_BooleanSequence);
	#endif /* _IDL_SEQUENCE_SOMObject_BooleanSequence_defined */
	typedef struct SOMObject_somObjectOffset
	{
		SOMObject SOMSTAR obj;
		long offset;
	} SOMObject_somObjectOffset;
	#ifndef _IDL_SEQUENCE_SOMObject_somObjectOffset_defined
		#define _IDL_SEQUENCE_SOMObject_somObjectOffset_defined
		SOM_SEQUENCE_TYPEDEF(SOMObject_somObjectOffset);
	#endif /* _IDL_SEQUENCE_SOMObject_somObjectOffset_defined */
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_somObjectOffset
			#if defined(somObjectOffset)
				#undef somObjectOffset
				#define SOMGD_somObjectOffset
			#else
				#define somObjectOffset SOMObject_somObjectOffset
			#endif
		#endif /* SOMGD_somObjectOffset */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef _IDL_SEQUENCE_somObjectOffset_defined
			#define _IDL_SEQUENCE_somObjectOffset_defined
			#define _IDL_SEQUENCE_somObjectOffset _IDL_SEQUENCE_SOMObject_somObjectOffset
		#endif /* _IDL_SEQUENCE_somObjectOffset_defined */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
	typedef _IDL_SEQUENCE_SOMObject_somObjectOffset SOMObject_somObjectOffsets;
	#ifndef _IDL_SEQUENCE_SOMObject_somObjectOffsets_defined
		#define _IDL_SEQUENCE_SOMObject_somObjectOffsets_defined
		SOM_SEQUENCE_TYPEDEF(SOMObject_somObjectOffsets);
	#endif /* _IDL_SEQUENCE_SOMObject_somObjectOffsets_defined */
	#ifdef __IBMC__
		typedef void (somTP_SOMObject_somInit)(
			SOMObject SOMSTAR somSelf);
		#pragma linkage(somTP_SOMObject_somInit,system)
		typedef somTP_SOMObject_somInit *somTD_SOMObject_somInit;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMObject_somInit)(
			SOMObject SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMObject_somUninit)(
			SOMObject SOMSTAR somSelf);
		#pragma linkage(somTP_SOMObject_somUninit,system)
		typedef somTP_SOMObject_somUninit *somTD_SOMObject_somUninit;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMObject_somUninit)(
			SOMObject SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMObject_somFree)(
			SOMObject SOMSTAR somSelf);
		#pragma linkage(somTP_SOMObject_somFree,system)
		typedef somTP_SOMObject_somFree *somTD_SOMObject_somFree;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMObject_somFree)(
			SOMObject SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMClass SOMSTAR (somTP_SOMObject_somGetClass)(
			SOMObject SOMSTAR somSelf);
		#pragma linkage(somTP_SOMObject_somGetClass,system)
		typedef somTP_SOMObject_somGetClass *somTD_SOMObject_somGetClass;
	#else /* __IBMC__ */
		typedef SOMClass SOMSTAR (SOMLINK * somTD_SOMObject_somGetClass)(
			SOMObject SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_SOMObject_somGetClassName)(
			SOMObject SOMSTAR somSelf);
		#pragma linkage(somTP_SOMObject_somGetClassName,system)
		typedef somTP_SOMObject_somGetClassName *somTD_SOMObject_somGetClassName;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_SOMObject_somGetClassName)(
			SOMObject SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef long (somTP_SOMObject_somGetSize)(
			SOMObject SOMSTAR somSelf);
		#pragma linkage(somTP_SOMObject_somGetSize,system)
		typedef somTP_SOMObject_somGetSize *somTD_SOMObject_somGetSize;
	#else /* __IBMC__ */
		typedef long (SOMLINK * somTD_SOMObject_somGetSize)(
			SOMObject SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef boolean (somTP_SOMObject_somIsA)(
			SOMObject SOMSTAR somSelf,
			/* in */ SOMClass SOMSTAR aClassObj);
		#pragma linkage(somTP_SOMObject_somIsA,system)
		typedef somTP_SOMObject_somIsA *somTD_SOMObject_somIsA;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_SOMObject_somIsA)(
			SOMObject SOMSTAR somSelf,
			/* in */ SOMClass SOMSTAR aClassObj);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef boolean (somTP_SOMObject_somIsInstanceOf)(
			SOMObject SOMSTAR somSelf,
			/* in */ SOMClass SOMSTAR aClassObj);
		#pragma linkage(somTP_SOMObject_somIsInstanceOf,system)
		typedef somTP_SOMObject_somIsInstanceOf *somTD_SOMObject_somIsInstanceOf;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_SOMObject_somIsInstanceOf)(
			SOMObject SOMSTAR somSelf,
			/* in */ SOMClass SOMSTAR aClassObj);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef boolean (somTP_SOMObject_somRespondsTo)(
			SOMObject SOMSTAR somSelf,
			/* in */ somId mId);
		#pragma linkage(somTP_SOMObject_somRespondsTo,system)
		typedef somTP_SOMObject_somRespondsTo *somTD_SOMObject_somRespondsTo;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_SOMObject_somRespondsTo)(
			SOMObject SOMSTAR somSelf,
			/* in */ somId mId);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef boolean (somTP_SOMObject_somDispatch)(
			SOMObject SOMSTAR somSelf,
			/* out */ somToken *retValue,
			/* in */ somId methodId,
			/* in */ va_list ap);
		#pragma linkage(somTP_SOMObject_somDispatch,system)
		typedef somTP_SOMObject_somDispatch *somTD_SOMObject_somDispatch;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_SOMObject_somDispatch)(
			SOMObject SOMSTAR somSelf,
			/* out */ somToken *retValue,
			/* in */ somId methodId,
			/* in */ va_list ap);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef boolean (somTP_SOMObject_somClassDispatch)(
			SOMObject SOMSTAR somSelf,
			/* in */ SOMClass SOMSTAR clsObj,
			/* out */ somToken *retValue,
			/* in */ somId methodId,
			/* in */ va_list ap);
		#pragma linkage(somTP_SOMObject_somClassDispatch,system)
		typedef somTP_SOMObject_somClassDispatch *somTD_SOMObject_somClassDispatch;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_SOMObject_somClassDispatch)(
			SOMObject SOMSTAR somSelf,
			/* in */ SOMClass SOMSTAR clsObj,
			/* out */ somToken *retValue,
			/* in */ somId methodId,
			/* in */ va_list ap);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef boolean (somTP_SOMObject_somCastObj)(
			SOMObject SOMSTAR somSelf,
			/* in */ SOMClass SOMSTAR castedCls);
		#pragma linkage(somTP_SOMObject_somCastObj,system)
		typedef somTP_SOMObject_somCastObj *somTD_SOMObject_somCastObj;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_SOMObject_somCastObj)(
			SOMObject SOMSTAR somSelf,
			/* in */ SOMClass SOMSTAR castedCls);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef boolean (somTP_SOMObject_somResetObj)(
			SOMObject SOMSTAR somSelf);
		#pragma linkage(somTP_SOMObject_somResetObj,system)
		typedef somTP_SOMObject_somResetObj *somTD_SOMObject_somResetObj;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_SOMObject_somResetObj)(
			SOMObject SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMObject SOMSTAR (somTP_SOMObject_somPrintSelf)(
			SOMObject SOMSTAR somSelf);
		#pragma linkage(somTP_SOMObject_somPrintSelf,system)
		typedef somTP_SOMObject_somPrintSelf *somTD_SOMObject_somPrintSelf;
	#else /* __IBMC__ */
		typedef SOMObject SOMSTAR (SOMLINK * somTD_SOMObject_somPrintSelf)(
			SOMObject SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMObject_somDumpSelf)(
			SOMObject SOMSTAR somSelf,
			/* in */ long level);
		#pragma linkage(somTP_SOMObject_somDumpSelf,system)
		typedef somTP_SOMObject_somDumpSelf *somTD_SOMObject_somDumpSelf;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMObject_somDumpSelf)(
			SOMObject SOMSTAR somSelf,
			/* in */ long level);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMObject_somDumpSelfInt)(
			SOMObject SOMSTAR somSelf,
			/* in */ long level);
		#pragma linkage(somTP_SOMObject_somDumpSelfInt,system)
		typedef somTP_SOMObject_somDumpSelfInt *somTD_SOMObject_somDumpSelfInt;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMObject_somDumpSelfInt)(
			SOMObject SOMSTAR somSelf,
			/* in */ long level);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMObject_somDefaultInit)(
			SOMObject SOMSTAR somSelf,
			/* inout */ somInitCtrl *ctrl);
		#pragma linkage(somTP_SOMObject_somDefaultInit,system)
		typedef somTP_SOMObject_somDefaultInit *somTD_SOMObject_somDefaultInit;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMObject_somDefaultInit)(
			SOMObject SOMSTAR somSelf,
			/* inout */ somInitCtrl *ctrl);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMObject_somDefaultCopyInit)(
			SOMObject SOMSTAR somSelf,
			/* inout */ somInitCtrl *ctrl,
			/* in */ SOMObject SOMSTAR fromObj);
		#pragma linkage(somTP_SOMObject_somDefaultCopyInit,system)
		typedef somTP_SOMObject_somDefaultCopyInit *somTD_SOMObject_somDefaultCopyInit;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMObject_somDefaultCopyInit)(
			SOMObject SOMSTAR somSelf,
			/* inout */ somInitCtrl *ctrl,
			/* in */ SOMObject SOMSTAR fromObj);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMObject_somDefaultConstCopyInit)(
			SOMObject SOMSTAR somSelf,
			/* inout */ somInitCtrl *ctrl,
			/* in */ SOMObject SOMSTAR fromObj);
		#pragma linkage(somTP_SOMObject_somDefaultConstCopyInit,system)
		typedef somTP_SOMObject_somDefaultConstCopyInit *somTD_SOMObject_somDefaultConstCopyInit;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMObject_somDefaultConstCopyInit)(
			SOMObject SOMSTAR somSelf,
			/* inout */ somInitCtrl *ctrl,
			/* in */ SOMObject SOMSTAR fromObj);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMObject_somDefaultVCopyInit)(
			SOMObject SOMSTAR somSelf,
			/* inout */ somInitCtrl *ctrl,
			/* in */ SOMObject SOMSTAR fromObj);
		#pragma linkage(somTP_SOMObject_somDefaultVCopyInit,system)
		typedef somTP_SOMObject_somDefaultVCopyInit *somTD_SOMObject_somDefaultVCopyInit;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMObject_somDefaultVCopyInit)(
			SOMObject SOMSTAR somSelf,
			/* inout */ somInitCtrl *ctrl,
			/* in */ SOMObject SOMSTAR fromObj);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMObject_somDefaultConstVCopyInit)(
			SOMObject SOMSTAR somSelf,
			/* inout */ somInitCtrl *ctrl,
			/* in */ SOMObject SOMSTAR fromObj);
		#pragma linkage(somTP_SOMObject_somDefaultConstVCopyInit,system)
		typedef somTP_SOMObject_somDefaultConstVCopyInit *somTD_SOMObject_somDefaultConstVCopyInit;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMObject_somDefaultConstVCopyInit)(
			SOMObject SOMSTAR somSelf,
			/* inout */ somInitCtrl *ctrl,
			/* in */ SOMObject SOMSTAR fromObj);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMObject SOMSTAR (somTP_SOMObject_somDefaultAssign)(
			SOMObject SOMSTAR somSelf,
			/* inout */ somAssignCtrl *ctrl,
			/* in */ SOMObject SOMSTAR fromObj);
		#pragma linkage(somTP_SOMObject_somDefaultAssign,system)
		typedef somTP_SOMObject_somDefaultAssign *somTD_SOMObject_somDefaultAssign;
	#else /* __IBMC__ */
		typedef SOMObject SOMSTAR (SOMLINK * somTD_SOMObject_somDefaultAssign)(
			SOMObject SOMSTAR somSelf,
			/* inout */ somAssignCtrl *ctrl,
			/* in */ SOMObject SOMSTAR fromObj);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMObject SOMSTAR (somTP_SOMObject_somDefaultConstAssign)(
			SOMObject SOMSTAR somSelf,
			/* inout */ somAssignCtrl *ctrl,
			/* in */ SOMObject SOMSTAR fromObj);
		#pragma linkage(somTP_SOMObject_somDefaultConstAssign,system)
		typedef somTP_SOMObject_somDefaultConstAssign *somTD_SOMObject_somDefaultConstAssign;
	#else /* __IBMC__ */
		typedef SOMObject SOMSTAR (SOMLINK * somTD_SOMObject_somDefaultConstAssign)(
			SOMObject SOMSTAR somSelf,
			/* inout */ somAssignCtrl *ctrl,
			/* in */ SOMObject SOMSTAR fromObj);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMObject SOMSTAR (somTP_SOMObject_somDefaultVAssign)(
			SOMObject SOMSTAR somSelf,
			/* inout */ somAssignCtrl *ctrl,
			/* in */ SOMObject SOMSTAR fromObj);
		#pragma linkage(somTP_SOMObject_somDefaultVAssign,system)
		typedef somTP_SOMObject_somDefaultVAssign *somTD_SOMObject_somDefaultVAssign;
	#else /* __IBMC__ */
		typedef SOMObject SOMSTAR (SOMLINK * somTD_SOMObject_somDefaultVAssign)(
			SOMObject SOMSTAR somSelf,
			/* inout */ somAssignCtrl *ctrl,
			/* in */ SOMObject SOMSTAR fromObj);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMObject SOMSTAR (somTP_SOMObject_somDefaultConstVAssign)(
			SOMObject SOMSTAR somSelf,
			/* inout */ somAssignCtrl *ctrl,
			/* in */ SOMObject SOMSTAR fromObj);
		#pragma linkage(somTP_SOMObject_somDefaultConstVAssign,system)
		typedef somTP_SOMObject_somDefaultConstVAssign *somTD_SOMObject_somDefaultConstVAssign;
	#else /* __IBMC__ */
		typedef SOMObject SOMSTAR (SOMLINK * somTD_SOMObject_somDefaultConstVAssign)(
			SOMObject SOMSTAR somSelf,
			/* inout */ somAssignCtrl *ctrl,
			/* in */ SOMObject SOMSTAR fromObj);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMObject_somDestruct)(
			SOMObject SOMSTAR somSelf,
			/* in */ octet doFree,
			/* inout */ somDestructCtrl *ctrl);
		#pragma linkage(somTP_SOMObject_somDestruct,system)
		typedef somTP_SOMObject_somDestruct *somTD_SOMObject_somDestruct;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMObject_somDestruct)(
			SOMObject SOMSTAR somSelf,
			/* in */ octet doFree,
			/* inout */ somDestructCtrl *ctrl);
	#endif /* __IBMC__ */
	#ifndef SOMObject_MajorVersion
		#define SOMObject_MajorVersion   1
	#endif /* SOMObject_MajorVersion */
	#ifndef SOMObject_MinorVersion
		#define SOMObject_MinorVersion   7
	#endif /* SOMObject_MinorVersion */
	typedef struct SOMObjectClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken somInit;
		somMToken somUninit;
		somMToken somFree;
		somMToken somDefaultVCopyInit;
		somMToken somGetClassName;
		somMToken somGetClass;
		somMToken somIsA;
		somMToken somRespondsTo;
		somMToken somIsInstanceOf;
		somMToken somGetSize;
		somMToken somDumpSelf;
		somMToken somDumpSelfInt;
		somMToken somPrintSelf;
		somMToken somDefaultConstVCopyInit;
		somMToken somDispatchV;
		somMToken somDispatchL;
		somMToken somDispatchA;
		somMToken somDispatchD;
		somMToken somDispatch;
		somMToken somClassDispatch;
		somMToken somCastObj;
		somMToken somResetObj;
		somMToken somDefaultInit;
		somMToken somDestruct;
		somMToken somComputeForwardVisitMask;
		somMToken somComputeReverseVisitMask;
		somMToken somDefaultCopyInit;
		somMToken somDefaultConstCopyInit;
		somMToken somDefaultAssign;
		somMToken somDefaultConstAssign;
		somMToken somDefaultVAssign;
		somMToken somDefaultConstVAssign;
		somMToken release;
		somMToken duplicate;
		somMToken get_interface;
		somMToken get_implementation;
		somMToken is_proxy;
		somMToken create_request;
		somMToken create_request_args;
		somMToken is_nil;
	} SOMObjectClassDataStructure;
	typedef struct SOMObjectCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
		somMethodPtr somDispatch;
		somMethodPtr somClassDispatch;
	} SOMObjectCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_somobj_Source) || defined(SOMObject_Class_Source)
			SOMEXTERN struct SOMObjectClassDataStructure _SOMObjectClassData;
			#ifndef SOMObjectClassData
				#define SOMObjectClassData    _SOMObjectClassData
			#endif /* SOMObjectClassData */
		#else
			SOMEXTERN struct SOMObjectClassDataStructure * SOMLINK resolve_SOMObjectClassData(void);
			#ifndef SOMObjectClassData
				#define SOMObjectClassData    (*(resolve_SOMObjectClassData()))
			#endif /* SOMObjectClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_somobj_Source) || defined(SOMObject_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_somobj_Source || SOMObject_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_somobj_Source || SOMObject_Class_Source */
		struct SOMObjectClassDataStructure SOMDLINK SOMObjectClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_somobj_Source) || defined(SOMObject_Class_Source)
			SOMEXTERN struct SOMObjectCClassDataStructure _SOMObjectCClassData;
			#ifndef SOMObjectCClassData
				#define SOMObjectCClassData    _SOMObjectCClassData
			#endif /* SOMObjectCClassData */
		#else
			SOMEXTERN struct SOMObjectCClassDataStructure * SOMLINK resolve_SOMObjectCClassData(void);
			#ifndef SOMObjectCClassData
				#define SOMObjectCClassData    (*(resolve_SOMObjectCClassData()))
			#endif /* SOMObjectCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_somobj_Source) || defined(SOMObject_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_somobj_Source || SOMObject_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_somobj_Source || SOMObject_Class_Source */
		struct SOMObjectCClassDataStructure SOMDLINK SOMObjectCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_somobj_Source) || defined(SOMObject_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_somobj_Source || SOMObject_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_somobj_Source || SOMObject_Class_Source */
	SOMClass SOMSTAR SOMLINK SOMObjectNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_SOMObject (SOMObjectClassData.classObject)
	#ifndef SOMGD_SOMObject
		#if (defined(_SOMObject) || defined(__SOMObject))
			#undef _SOMObject
			#undef __SOMObject
			#define SOMGD_SOMObject 1
		#else
			#define _SOMObject _SOMCLASS_SOMObject
		#endif /* _SOMObject */
	#endif /* SOMGD_SOMObject */
	#define SOMObject_classObj _SOMCLASS_SOMObject
	#define _SOMMTOKEN_SOMObject(method) ((somMToken)(SOMObjectClassData.method))
	#ifndef SOMObjectNew
		#define SOMObjectNew() ( _SOMObject ? \
			(SOMClass_somNew(_SOMObject)) : \
			( SOMObjectNewClass( \
				SOMObject_MajorVersion, \
				SOMObject_MinorVersion), \
			SOMClass_somNew(_SOMObject))) 
	#endif /* NewSOMObject */
	#ifndef SOMObject_somInit
		#define SOMObject_somInit(somSelf) \
			SOM_Resolve(somSelf,SOMObject,somInit)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somInit
				#if defined(_somInit)
					#undef _somInit
					#define SOMGD__somInit
				#else
					#define _somInit SOMObject_somInit
				#endif
			#endif /* SOMGD__somInit */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMObject_somInit */
	#ifndef SOMObject_somUninit
		#define SOMObject_somUninit(somSelf) \
			SOM_Resolve(somSelf,SOMObject,somUninit)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somUninit
				#if defined(_somUninit)
					#undef _somUninit
					#define SOMGD__somUninit
				#else
					#define _somUninit SOMObject_somUninit
				#endif
			#endif /* SOMGD__somUninit */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMObject_somUninit */
	#ifndef SOMObject_somFree
		#define SOMObject_somFree(somSelf) \
			SOM_Resolve(somSelf,SOMObject,somFree)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somFree
				#if defined(_somFree)
					#undef _somFree
					#define SOMGD__somFree
				#else
					#define _somFree SOMObject_somFree
				#endif
			#endif /* SOMGD__somFree */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMObject_somFree */
	#ifndef SOMObject_somGetClass
		#define SOMObject_somGetClass(somSelf) \
			SOM_Resolve(somSelf,SOMObject,somGetClass)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somGetClass
				#if defined(_somGetClass)
					#undef _somGetClass
					#define SOMGD__somGetClass
				#else
					#define _somGetClass SOMObject_somGetClass
				#endif
			#endif /* SOMGD__somGetClass */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMObject_somGetClass */
	#ifndef SOMObject_somGetClassName
		#define SOMObject_somGetClassName(somSelf) \
			SOM_Resolve(somSelf,SOMObject,somGetClassName)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somGetClassName
				#if defined(_somGetClassName)
					#undef _somGetClassName
					#define SOMGD__somGetClassName
				#else
					#define _somGetClassName SOMObject_somGetClassName
				#endif
			#endif /* SOMGD__somGetClassName */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMObject_somGetClassName */
	#ifndef SOMObject_somGetSize
		#define SOMObject_somGetSize(somSelf) \
			SOM_Resolve(somSelf,SOMObject,somGetSize)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somGetSize
				#if defined(_somGetSize)
					#undef _somGetSize
					#define SOMGD__somGetSize
				#else
					#define _somGetSize SOMObject_somGetSize
				#endif
			#endif /* SOMGD__somGetSize */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMObject_somGetSize */
	#ifndef SOMObject_somIsA
		#define SOMObject_somIsA(somSelf,aClassObj) \
			SOM_Resolve(somSelf,SOMObject,somIsA)  \
				(somSelf,aClassObj)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somIsA
				#if defined(_somIsA)
					#undef _somIsA
					#define SOMGD__somIsA
				#else
					#define _somIsA SOMObject_somIsA
				#endif
			#endif /* SOMGD__somIsA */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMObject_somIsA */
	#ifndef SOMObject_somIsInstanceOf
		#define SOMObject_somIsInstanceOf(somSelf,aClassObj) \
			SOM_Resolve(somSelf,SOMObject,somIsInstanceOf)  \
				(somSelf,aClassObj)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somIsInstanceOf
				#if defined(_somIsInstanceOf)
					#undef _somIsInstanceOf
					#define SOMGD__somIsInstanceOf
				#else
					#define _somIsInstanceOf SOMObject_somIsInstanceOf
				#endif
			#endif /* SOMGD__somIsInstanceOf */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMObject_somIsInstanceOf */
	#ifndef SOMObject_somRespondsTo
		#define SOMObject_somRespondsTo(somSelf,mId) \
			SOM_Resolve(somSelf,SOMObject,somRespondsTo)  \
				(somSelf,mId)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somRespondsTo
				#if defined(_somRespondsTo)
					#undef _somRespondsTo
					#define SOMGD__somRespondsTo
				#else
					#define _somRespondsTo SOMObject_somRespondsTo
				#endif
			#endif /* SOMGD__somRespondsTo */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMObject_somRespondsTo */
	#ifndef SOMObject_somDispatch
		#define SOMObject_somDispatch(somSelf,retValue,methodId,ap) \
			SOM_Resolve(somSelf,SOMObject,somDispatch)  \
				(somSelf,retValue,methodId,ap)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somDispatch
				#if defined(_somDispatch)
					#undef _somDispatch
					#define SOMGD__somDispatch
				#else
					#define _somDispatch SOMObject_somDispatch
				#endif
			#endif /* SOMGD__somDispatch */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMObject_somDispatch */
	#ifndef va_SOMObject_somDispatch
		#define va_SOMObject_somDispatch somva_SOMObject_somDispatch
	#endif /* va_SOMObject_somDispatch */
	#ifdef SOMObject_VA_EXTERN
		SOMEXTERN
		#ifdef SOMObject_VA_STUBS
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else
			#if defined(SOM_Module_somobj_Source) || defined(SOMObject_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_somobj_Source || SOMObject_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_somobj_Source || SOMObject_Class_Source */
		#endif /* SOMObject_VA_STUBS */
		boolean SOMLINK somva_SOMObject_somDispatch(SOMObject SOMSTAR somSelf,
			somToken *retValue,
			somId methodId,
			...);
	#endif
	#if (defined(SOMObject_VA_STUBS)||!defined(SOMObject_VA_EXTERN))
		#ifdef SOMObject_VA_EXTERN
			SOMEXTERN
		#else
			static
		#endif
		boolean SOMLINK somva_SOMObject_somDispatch(SOMObject SOMSTAR somSelf,
				somToken *retValue,
				somId methodId,
				...)
		{
			boolean __result;
			va_list ap;
			va_start(ap,methodId);
			__result=SOM_Resolve(somSelf,SOMObject,somDispatch)
					(somSelf,retValue,methodId,ap);
			va_end(ap);
			return __result;
		}
	#endif
	#ifndef SOMObject_somClassDispatch
		#define SOMObject_somClassDispatch(somSelf,clsObj,retValue,methodId,ap) \
			SOM_Resolve(somSelf,SOMObject,somClassDispatch)  \
				(somSelf,clsObj,retValue,methodId,ap)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somClassDispatch
				#if defined(_somClassDispatch)
					#undef _somClassDispatch
					#define SOMGD__somClassDispatch
				#else
					#define _somClassDispatch SOMObject_somClassDispatch
				#endif
			#endif /* SOMGD__somClassDispatch */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMObject_somClassDispatch */
	#ifndef va_SOMObject_somClassDispatch
		#define va_SOMObject_somClassDispatch somva_SOMObject_somClassDispatch
	#endif /* va_SOMObject_somClassDispatch */
	#ifdef SOMObject_VA_EXTERN
		SOMEXTERN
		#ifdef SOMObject_VA_STUBS
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else
			#if defined(SOM_Module_somobj_Source) || defined(SOMObject_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_somobj_Source || SOMObject_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_somobj_Source || SOMObject_Class_Source */
		#endif /* SOMObject_VA_STUBS */
		boolean SOMLINK somva_SOMObject_somClassDispatch(SOMObject SOMSTAR somSelf,
			SOMClass SOMSTAR clsObj,
			somToken *retValue,
			somId methodId,
			...);
	#endif
	#if (defined(SOMObject_VA_STUBS)||!defined(SOMObject_VA_EXTERN))
		#ifdef SOMObject_VA_EXTERN
			SOMEXTERN
		#else
			static
		#endif
		boolean SOMLINK somva_SOMObject_somClassDispatch(SOMObject SOMSTAR somSelf,
				SOMClass SOMSTAR clsObj,
				somToken *retValue,
				somId methodId,
				...)
		{
			boolean __result;
			va_list ap;
			va_start(ap,methodId);
			__result=SOM_Resolve(somSelf,SOMObject,somClassDispatch)
					(somSelf,clsObj,retValue,methodId,ap);
			va_end(ap);
			return __result;
		}
	#endif
	#ifndef SOMObject_somCastObj
		#define SOMObject_somCastObj(somSelf,castedCls) \
			SOM_Resolve(somSelf,SOMObject,somCastObj)  \
				(somSelf,castedCls)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somCastObj
				#if defined(_somCastObj)
					#undef _somCastObj
					#define SOMGD__somCastObj
				#else
					#define _somCastObj SOMObject_somCastObj
				#endif
			#endif /* SOMGD__somCastObj */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMObject_somCastObj */
	#ifndef SOMObject_somResetObj
		#define SOMObject_somResetObj(somSelf) \
			SOM_Resolve(somSelf,SOMObject,somResetObj)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somResetObj
				#if defined(_somResetObj)
					#undef _somResetObj
					#define SOMGD__somResetObj
				#else
					#define _somResetObj SOMObject_somResetObj
				#endif
			#endif /* SOMGD__somResetObj */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMObject_somResetObj */
	#ifndef SOMObject_somPrintSelf
		#define SOMObject_somPrintSelf(somSelf) \
			SOM_Resolve(somSelf,SOMObject,somPrintSelf)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somPrintSelf
				#if defined(_somPrintSelf)
					#undef _somPrintSelf
					#define SOMGD__somPrintSelf
				#else
					#define _somPrintSelf SOMObject_somPrintSelf
				#endif
			#endif /* SOMGD__somPrintSelf */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMObject_somPrintSelf */
	#ifndef SOMObject_somDumpSelf
		#define SOMObject_somDumpSelf(somSelf,level) \
			SOM_Resolve(somSelf,SOMObject,somDumpSelf)  \
				(somSelf,level)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somDumpSelf
				#if defined(_somDumpSelf)
					#undef _somDumpSelf
					#define SOMGD__somDumpSelf
				#else
					#define _somDumpSelf SOMObject_somDumpSelf
				#endif
			#endif /* SOMGD__somDumpSelf */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMObject_somDumpSelf */
	#ifndef SOMObject_somDumpSelfInt
		#define SOMObject_somDumpSelfInt(somSelf,level) \
			SOM_Resolve(somSelf,SOMObject,somDumpSelfInt)  \
				(somSelf,level)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somDumpSelfInt
				#if defined(_somDumpSelfInt)
					#undef _somDumpSelfInt
					#define SOMGD__somDumpSelfInt
				#else
					#define _somDumpSelfInt SOMObject_somDumpSelfInt
				#endif
			#endif /* SOMGD__somDumpSelfInt */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMObject_somDumpSelfInt */
	#ifndef SOMObject_somDefaultInit
		#define SOMObject_somDefaultInit(somSelf,ctrl) \
			SOM_Resolve(somSelf,SOMObject,somDefaultInit)  \
				(somSelf,ctrl)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somDefaultInit
				#if defined(_somDefaultInit)
					#undef _somDefaultInit
					#define SOMGD__somDefaultInit
				#else
					#define _somDefaultInit SOMObject_somDefaultInit
				#endif
			#endif /* SOMGD__somDefaultInit */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMObject_somDefaultInit */
	#ifndef SOMObject_somDefaultCopyInit
		#define SOMObject_somDefaultCopyInit(somSelf,ctrl,fromObj) \
			SOM_Resolve(somSelf,SOMObject,somDefaultCopyInit)  \
				(somSelf,ctrl,fromObj)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somDefaultCopyInit
				#if defined(_somDefaultCopyInit)
					#undef _somDefaultCopyInit
					#define SOMGD__somDefaultCopyInit
				#else
					#define _somDefaultCopyInit SOMObject_somDefaultCopyInit
				#endif
			#endif /* SOMGD__somDefaultCopyInit */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMObject_somDefaultCopyInit */
	#ifndef SOMObject_somDefaultConstCopyInit
		#define SOMObject_somDefaultConstCopyInit(somSelf,ctrl,fromObj) \
			SOM_Resolve(somSelf,SOMObject,somDefaultConstCopyInit)  \
				(somSelf,ctrl,fromObj)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somDefaultConstCopyInit
				#if defined(_somDefaultConstCopyInit)
					#undef _somDefaultConstCopyInit
					#define SOMGD__somDefaultConstCopyInit
				#else
					#define _somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
				#endif
			#endif /* SOMGD__somDefaultConstCopyInit */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMObject_somDefaultConstCopyInit */
	#ifndef SOMObject_somDefaultVCopyInit
		#define SOMObject_somDefaultVCopyInit(somSelf,ctrl,fromObj) \
			SOM_Resolve(somSelf,SOMObject,somDefaultVCopyInit)  \
				(somSelf,ctrl,fromObj)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somDefaultVCopyInit
				#if defined(_somDefaultVCopyInit)
					#undef _somDefaultVCopyInit
					#define SOMGD__somDefaultVCopyInit
				#else
					#define _somDefaultVCopyInit SOMObject_somDefaultVCopyInit
				#endif
			#endif /* SOMGD__somDefaultVCopyInit */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMObject_somDefaultVCopyInit */
	#ifndef SOMObject_somDefaultConstVCopyInit
		#define SOMObject_somDefaultConstVCopyInit(somSelf,ctrl,fromObj) \
			SOM_Resolve(somSelf,SOMObject,somDefaultConstVCopyInit)  \
				(somSelf,ctrl,fromObj)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somDefaultConstVCopyInit
				#if defined(_somDefaultConstVCopyInit)
					#undef _somDefaultConstVCopyInit
					#define SOMGD__somDefaultConstVCopyInit
				#else
					#define _somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
				#endif
			#endif /* SOMGD__somDefaultConstVCopyInit */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMObject_somDefaultConstVCopyInit */
	#ifndef SOMObject_somDefaultAssign
		#define SOMObject_somDefaultAssign(somSelf,ctrl,fromObj) \
			SOM_Resolve(somSelf,SOMObject,somDefaultAssign)  \
				(somSelf,ctrl,fromObj)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somDefaultAssign
				#if defined(_somDefaultAssign)
					#undef _somDefaultAssign
					#define SOMGD__somDefaultAssign
				#else
					#define _somDefaultAssign SOMObject_somDefaultAssign
				#endif
			#endif /* SOMGD__somDefaultAssign */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMObject_somDefaultAssign */
	#ifndef SOMObject_somDefaultConstAssign
		#define SOMObject_somDefaultConstAssign(somSelf,ctrl,fromObj) \
			SOM_Resolve(somSelf,SOMObject,somDefaultConstAssign)  \
				(somSelf,ctrl,fromObj)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somDefaultConstAssign
				#if defined(_somDefaultConstAssign)
					#undef _somDefaultConstAssign
					#define SOMGD__somDefaultConstAssign
				#else
					#define _somDefaultConstAssign SOMObject_somDefaultConstAssign
				#endif
			#endif /* SOMGD__somDefaultConstAssign */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMObject_somDefaultConstAssign */
	#ifndef SOMObject_somDefaultVAssign
		#define SOMObject_somDefaultVAssign(somSelf,ctrl,fromObj) \
			SOM_Resolve(somSelf,SOMObject,somDefaultVAssign)  \
				(somSelf,ctrl,fromObj)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somDefaultVAssign
				#if defined(_somDefaultVAssign)
					#undef _somDefaultVAssign
					#define SOMGD__somDefaultVAssign
				#else
					#define _somDefaultVAssign SOMObject_somDefaultVAssign
				#endif
			#endif /* SOMGD__somDefaultVAssign */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMObject_somDefaultVAssign */
	#ifndef SOMObject_somDefaultConstVAssign
		#define SOMObject_somDefaultConstVAssign(somSelf,ctrl,fromObj) \
			SOM_Resolve(somSelf,SOMObject,somDefaultConstVAssign)  \
				(somSelf,ctrl,fromObj)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somDefaultConstVAssign
				#if defined(_somDefaultConstVAssign)
					#undef _somDefaultConstVAssign
					#define SOMGD__somDefaultConstVAssign
				#else
					#define _somDefaultConstVAssign SOMObject_somDefaultConstVAssign
				#endif
			#endif /* SOMGD__somDefaultConstVAssign */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMObject_somDefaultConstVAssign */
	#ifndef SOMObject_somDestruct
		#define SOMObject_somDestruct(somSelf,doFree,ctrl) \
			SOM_Resolve(somSelf,SOMObject,somDestruct)  \
				(somSelf,doFree,ctrl)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somDestruct
				#if defined(_somDestruct)
					#undef _somDestruct
					#define SOMGD__somDestruct
				#else
					#define _somDestruct SOMObject_somDestruct
				#endif
			#endif /* SOMGD__somDestruct */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMObject_somDestruct */
#endif /* SOM_Module_somobj_Header_h */
