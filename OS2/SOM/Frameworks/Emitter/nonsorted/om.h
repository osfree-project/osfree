/* generated from om.idl */
/* internal conditional is SOM_Module_om_Source */
#ifndef SOM_Module_om_Header_h
	#define SOM_Module_om_Header_h 1
	#include <som.h>
	#include <somobj.h>
	#include <somdtype.h>
	#include <containd.h>
	#include <snglicls.h>
	#include <somcls.h>
	#ifndef _IDL_ObjectMgr_defined
		#define _IDL_ObjectMgr_defined
		typedef SOMObject ObjectMgr;
	#endif /* _IDL_ObjectMgr_defined */
	#ifndef _IDL_SEQUENCE_ObjectMgr_defined
		#define _IDL_SEQUENCE_ObjectMgr_defined
		SOM_SEQUENCE_TYPEDEF_NAME(ObjectMgr SOMSTAR ,sequence(ObjectMgr));
	#endif /* _IDL_SEQUENCE_ObjectMgr_defined */
	#ifdef __IBMC__
		typedef SOMObject SOMSTAR (somTP_ObjectMgr_somdNewObject)(
			ObjectMgr SOMSTAR somSelf,
			Environment *ev,
			/* in */ Identifier objclass,
			/* in */ string hints);
		#pragma linkage(somTP_ObjectMgr_somdNewObject,system)
		typedef somTP_ObjectMgr_somdNewObject *somTD_ObjectMgr_somdNewObject;
	#else /* __IBMC__ */
		typedef SOMObject SOMSTAR (SOMLINK * somTD_ObjectMgr_somdNewObject)(
			ObjectMgr SOMSTAR somSelf,
			Environment *ev,
			/* in */ Identifier objclass,
			/* in */ string hints);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_ObjectMgr_somdGetIdFromObject)(
			ObjectMgr SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMObject SOMSTAR obj);
		#pragma linkage(somTP_ObjectMgr_somdGetIdFromObject,system)
		typedef somTP_ObjectMgr_somdGetIdFromObject *somTD_ObjectMgr_somdGetIdFromObject;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_ObjectMgr_somdGetIdFromObject)(
			ObjectMgr SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMObject SOMSTAR obj);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMObject SOMSTAR (somTP_ObjectMgr_somdGetObjectFromId)(
			ObjectMgr SOMSTAR somSelf,
			Environment *ev,
			/* in */ string id);
		#pragma linkage(somTP_ObjectMgr_somdGetObjectFromId,system)
		typedef somTP_ObjectMgr_somdGetObjectFromId *somTD_ObjectMgr_somdGetObjectFromId;
	#else /* __IBMC__ */
		typedef SOMObject SOMSTAR (SOMLINK * somTD_ObjectMgr_somdGetObjectFromId)(
			ObjectMgr SOMSTAR somSelf,
			Environment *ev,
			/* in */ string id);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_ObjectMgr_somdReleaseObject)(
			ObjectMgr SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMObject SOMSTAR obj);
		#pragma linkage(somTP_ObjectMgr_somdReleaseObject,system)
		typedef somTP_ObjectMgr_somdReleaseObject *somTD_ObjectMgr_somdReleaseObject;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_ObjectMgr_somdReleaseObject)(
			ObjectMgr SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMObject SOMSTAR obj);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_ObjectMgr_somdDestroyObject)(
			ObjectMgr SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMObject SOMSTAR obj);
		#pragma linkage(somTP_ObjectMgr_somdDestroyObject,system)
		typedef somTP_ObjectMgr_somdDestroyObject *somTD_ObjectMgr_somdDestroyObject;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_ObjectMgr_somdDestroyObject)(
			ObjectMgr SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMObject SOMSTAR obj);
	#endif /* __IBMC__ */
	#ifndef ObjectMgr_MajorVersion
		#define ObjectMgr_MajorVersion   2
	#endif /* ObjectMgr_MajorVersion */
	#ifndef ObjectMgr_MinorVersion
		#define ObjectMgr_MinorVersion   2
	#endif /* ObjectMgr_MinorVersion */
	typedef struct ObjectMgrClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken somdNewObject;
		somMToken somdGetIdFromObject;
		somMToken somdGetObjectFromId;
		somMToken somdReleaseObject;
		somMToken somdDestroyObject;
	} ObjectMgrClassDataStructure;
	typedef struct ObjectMgrCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} ObjectMgrCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_om_Source) || defined(ObjectMgr_Class_Source)
			SOMEXTERN struct ObjectMgrClassDataStructure _ObjectMgrClassData;
			#ifndef ObjectMgrClassData
				#define ObjectMgrClassData    _ObjectMgrClassData
			#endif /* ObjectMgrClassData */
		#else
			SOMEXTERN struct ObjectMgrClassDataStructure * SOMLINK resolve_ObjectMgrClassData(void);
			#ifndef ObjectMgrClassData
				#define ObjectMgrClassData    (*(resolve_ObjectMgrClassData()))
			#endif /* ObjectMgrClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_om_Source) || defined(ObjectMgr_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_om_Source || ObjectMgr_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_om_Source || ObjectMgr_Class_Source */
		struct ObjectMgrClassDataStructure SOMDLINK ObjectMgrClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_om_Source) || defined(ObjectMgr_Class_Source)
			SOMEXTERN struct ObjectMgrCClassDataStructure _ObjectMgrCClassData;
			#ifndef ObjectMgrCClassData
				#define ObjectMgrCClassData    _ObjectMgrCClassData
			#endif /* ObjectMgrCClassData */
		#else
			SOMEXTERN struct ObjectMgrCClassDataStructure * SOMLINK resolve_ObjectMgrCClassData(void);
			#ifndef ObjectMgrCClassData
				#define ObjectMgrCClassData    (*(resolve_ObjectMgrCClassData()))
			#endif /* ObjectMgrCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_om_Source) || defined(ObjectMgr_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_om_Source || ObjectMgr_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_om_Source || ObjectMgr_Class_Source */
		struct ObjectMgrCClassDataStructure SOMDLINK ObjectMgrCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_om_Source) || defined(ObjectMgr_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_om_Source || ObjectMgr_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_om_Source || ObjectMgr_Class_Source */
	SOMClass SOMSTAR SOMLINK ObjectMgrNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_ObjectMgr (ObjectMgrClassData.classObject)
	#ifndef SOMGD_ObjectMgr
		#if (defined(_ObjectMgr) || defined(__ObjectMgr))
			#undef _ObjectMgr
			#undef __ObjectMgr
			#define SOMGD_ObjectMgr 1
		#else
			#define _ObjectMgr _SOMCLASS_ObjectMgr
		#endif /* _ObjectMgr */
	#endif /* SOMGD_ObjectMgr */
	#define ObjectMgr_classObj _SOMCLASS_ObjectMgr
	#define _SOMMTOKEN_ObjectMgr(method) ((somMToken)(ObjectMgrClassData.method))
	#ifndef ObjectMgrNew
		#define ObjectMgrNew() ( _ObjectMgr ? \
			(SOMClass_somNew(_ObjectMgr)) : \
			( ObjectMgrNewClass( \
				ObjectMgr_MajorVersion, \
				ObjectMgr_MinorVersion), \
			SOMClass_somNew(_ObjectMgr))) 
	#endif /* NewObjectMgr */
	#ifndef ObjectMgr_somdNewObject
		#define ObjectMgr_somdNewObject(somSelf,ev,objclass,hints) \
			SOM_Resolve(somSelf,ObjectMgr,somdNewObject)  \
				(somSelf,ev,objclass,hints)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somdNewObject
				#if defined(_somdNewObject)
					#undef _somdNewObject
					#define SOMGD__somdNewObject
				#else
					#define _somdNewObject ObjectMgr_somdNewObject
				#endif
			#endif /* SOMGD__somdNewObject */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ObjectMgr_somdNewObject */
	#ifndef ObjectMgr_somdGetIdFromObject
		#define ObjectMgr_somdGetIdFromObject(somSelf,ev,obj) \
			SOM_Resolve(somSelf,ObjectMgr,somdGetIdFromObject)  \
				(somSelf,ev,obj)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somdGetIdFromObject
				#if defined(_somdGetIdFromObject)
					#undef _somdGetIdFromObject
					#define SOMGD__somdGetIdFromObject
				#else
					#define _somdGetIdFromObject ObjectMgr_somdGetIdFromObject
				#endif
			#endif /* SOMGD__somdGetIdFromObject */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ObjectMgr_somdGetIdFromObject */
	#ifndef ObjectMgr_somdGetObjectFromId
		#define ObjectMgr_somdGetObjectFromId(somSelf,ev,id) \
			SOM_Resolve(somSelf,ObjectMgr,somdGetObjectFromId)  \
				(somSelf,ev,id)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somdGetObjectFromId
				#if defined(_somdGetObjectFromId)
					#undef _somdGetObjectFromId
					#define SOMGD__somdGetObjectFromId
				#else
					#define _somdGetObjectFromId ObjectMgr_somdGetObjectFromId
				#endif
			#endif /* SOMGD__somdGetObjectFromId */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ObjectMgr_somdGetObjectFromId */
	#ifndef ObjectMgr_somdReleaseObject
		#define ObjectMgr_somdReleaseObject(somSelf,ev,obj) \
			SOM_Resolve(somSelf,ObjectMgr,somdReleaseObject)  \
				(somSelf,ev,obj)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somdReleaseObject
				#if defined(_somdReleaseObject)
					#undef _somdReleaseObject
					#define SOMGD__somdReleaseObject
				#else
					#define _somdReleaseObject ObjectMgr_somdReleaseObject
				#endif
			#endif /* SOMGD__somdReleaseObject */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ObjectMgr_somdReleaseObject */
	#ifndef ObjectMgr_somdDestroyObject
		#define ObjectMgr_somdDestroyObject(somSelf,ev,obj) \
			SOM_Resolve(somSelf,ObjectMgr,somdDestroyObject)  \
				(somSelf,ev,obj)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somdDestroyObject
				#if defined(_somdDestroyObject)
					#undef _somdDestroyObject
					#define SOMGD__somdDestroyObject
				#else
					#define _somdDestroyObject ObjectMgr_somdDestroyObject
				#endif
			#endif /* SOMGD__somdDestroyObject */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ObjectMgr_somdDestroyObject */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define ObjectMgr_somInit SOMObject_somInit
#define ObjectMgr_somUninit SOMObject_somUninit
#define ObjectMgr_somFree SOMObject_somFree
#define ObjectMgr_somGetClass SOMObject_somGetClass
#define ObjectMgr_somGetClassName SOMObject_somGetClassName
#define ObjectMgr_somGetSize SOMObject_somGetSize
#define ObjectMgr_somIsA SOMObject_somIsA
#define ObjectMgr_somIsInstanceOf SOMObject_somIsInstanceOf
#define ObjectMgr_somRespondsTo SOMObject_somRespondsTo
#define ObjectMgr_somDispatch SOMObject_somDispatch
#define ObjectMgr_somClassDispatch SOMObject_somClassDispatch
#define ObjectMgr_somCastObj SOMObject_somCastObj
#define ObjectMgr_somResetObj SOMObject_somResetObj
#define ObjectMgr_somPrintSelf SOMObject_somPrintSelf
#define ObjectMgr_somDumpSelf SOMObject_somDumpSelf
#define ObjectMgr_somDumpSelfInt SOMObject_somDumpSelfInt
#define ObjectMgr_somDefaultInit SOMObject_somDefaultInit
#define ObjectMgr_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define ObjectMgr_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define ObjectMgr_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define ObjectMgr_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define ObjectMgr_somDefaultAssign SOMObject_somDefaultAssign
#define ObjectMgr_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define ObjectMgr_somDefaultVAssign SOMObject_somDefaultVAssign
#define ObjectMgr_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define ObjectMgr_somDestruct SOMObject_somDestruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_om_Header_h */
