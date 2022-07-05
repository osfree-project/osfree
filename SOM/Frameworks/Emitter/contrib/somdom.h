/* generated from somdom.idl */
/* internal conditional is SOM_Module_somdom_Source */
#ifndef SOM_Module_somdom_Header_h
	#define SOM_Module_somdom_Header_h 1
	#include <som.h>
	#include <om.h>
	#include <somobj.h>
	#include <somdtype.h>
	#include <containd.h>
	#include <snglicls.h>
	#include <somcls.h>
	#ifndef _IDL_SOMDServer_defined
		#define _IDL_SOMDServer_defined
		typedef SOMObject SOMDServer;
	#endif /* _IDL_SOMDServer_defined */
	#ifndef _IDL_SEQUENCE_SOMDServer_defined
		#define _IDL_SEQUENCE_SOMDServer_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMDServer SOMSTAR ,sequence(SOMDServer));
	#endif /* _IDL_SEQUENCE_SOMDServer_defined */
	#ifndef _IDL_SOMDObjectMgr_defined
		#define _IDL_SOMDObjectMgr_defined
		typedef SOMObject SOMDObjectMgr;
	#endif /* _IDL_SOMDObjectMgr_defined */
	#ifndef _IDL_SEQUENCE_SOMDObjectMgr_defined
		#define _IDL_SEQUENCE_SOMDObjectMgr_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMDObjectMgr SOMSTAR ,sequence(SOMDObjectMgr));
	#endif /* _IDL_SEQUENCE_SOMDObjectMgr_defined */
	#ifdef __IBMC__
		typedef SOMDServer SOMSTAR (somTP_SOMDObjectMgr_somdFindServer)(
			SOMDObjectMgr SOMSTAR somSelf,
			Environment *ev,
			/* in */ ImplId serverid);
		#pragma linkage(somTP_SOMDObjectMgr_somdFindServer,system)
		typedef somTP_SOMDObjectMgr_somdFindServer *somTD_SOMDObjectMgr_somdFindServer;
	#else /* __IBMC__ */
		typedef SOMDServer SOMSTAR (SOMLINK * somTD_SOMDObjectMgr_somdFindServer)(
			SOMDObjectMgr SOMSTAR somSelf,
			Environment *ev,
			/* in */ ImplId serverid);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMDServer SOMSTAR (somTP_SOMDObjectMgr_somdFindServerByName)(
			SOMDObjectMgr SOMSTAR somSelf,
			Environment *ev,
			/* in */ string servername);
		#pragma linkage(somTP_SOMDObjectMgr_somdFindServerByName,system)
		typedef somTP_SOMDObjectMgr_somdFindServerByName *somTD_SOMDObjectMgr_somdFindServerByName;
	#else /* __IBMC__ */
		typedef SOMDServer SOMSTAR (SOMLINK * somTD_SOMDObjectMgr_somdFindServerByName)(
			SOMDObjectMgr SOMSTAR somSelf,
			Environment *ev,
			/* in */ string servername);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef _IDL_SEQUENCE_SOMDServer (somTP_SOMDObjectMgr_somdFindServersByClass)(
			SOMDObjectMgr SOMSTAR somSelf,
			Environment *ev,
			/* in */ Identifier objclass);
		#pragma linkage(somTP_SOMDObjectMgr_somdFindServersByClass,system)
		typedef somTP_SOMDObjectMgr_somdFindServersByClass *somTD_SOMDObjectMgr_somdFindServersByClass;
	#else /* __IBMC__ */
		typedef _IDL_SEQUENCE_SOMDServer (SOMLINK * somTD_SOMDObjectMgr_somdFindServersByClass)(
			SOMDObjectMgr SOMSTAR somSelf,
			Environment *ev,
			/* in */ Identifier objclass);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMDServer SOMSTAR (somTP_SOMDObjectMgr_somdFindAnyServerByClass)(
			SOMDObjectMgr SOMSTAR somSelf,
			Environment *ev,
			/* in */ Identifier objclass);
		#pragma linkage(somTP_SOMDObjectMgr_somdFindAnyServerByClass,system)
		typedef somTP_SOMDObjectMgr_somdFindAnyServerByClass *somTD_SOMDObjectMgr_somdFindAnyServerByClass;
	#else /* __IBMC__ */
		typedef SOMDServer SOMSTAR (SOMLINK * somTD_SOMDObjectMgr_somdFindAnyServerByClass)(
			SOMDObjectMgr SOMSTAR somSelf,
			Environment *ev,
			/* in */ Identifier objclass);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMDObjectMgr__set_somd21somFree)(
			SOMDObjectMgr SOMSTAR somSelf,
			Environment *ev,
			/* in */ boolean somd21somFree);
		#pragma linkage(somTP_SOMDObjectMgr__set_somd21somFree,system)
		typedef somTP_SOMDObjectMgr__set_somd21somFree *somTD_SOMDObjectMgr__set_somd21somFree;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMDObjectMgr__set_somd21somFree)(
			SOMDObjectMgr SOMSTAR somSelf,
			Environment *ev,
			/* in */ boolean somd21somFree);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef boolean (somTP_SOMDObjectMgr__get_somd21somFree)(
			SOMDObjectMgr SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_SOMDObjectMgr__get_somd21somFree,system)
		typedef somTP_SOMDObjectMgr__get_somd21somFree *somTD_SOMDObjectMgr__get_somd21somFree;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_SOMDObjectMgr__get_somd21somFree)(
			SOMDObjectMgr SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifndef SOMDObjectMgr_MajorVersion
		#define SOMDObjectMgr_MajorVersion   2
	#endif /* SOMDObjectMgr_MajorVersion */
	#ifndef SOMDObjectMgr_MinorVersion
		#define SOMDObjectMgr_MinorVersion   2
	#endif /* SOMDObjectMgr_MinorVersion */
	typedef struct SOMDObjectMgrClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken somdFindServer;
		somMToken somdFindServerByName;
		somMToken somdFindServersByClass;
		somMToken somdFindAnyServerByClass;
		somMToken _get_somd21somFree;
		somMToken _set_somd21somFree;
	} SOMDObjectMgrClassDataStructure;
	typedef struct SOMDObjectMgrCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} SOMDObjectMgrCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_somdom_Source) || defined(SOMDObjectMgr_Class_Source)
			SOMEXTERN struct SOMDObjectMgrClassDataStructure _SOMDObjectMgrClassData;
			#ifndef SOMDObjectMgrClassData
				#define SOMDObjectMgrClassData    _SOMDObjectMgrClassData
			#endif /* SOMDObjectMgrClassData */
		#else
			SOMEXTERN struct SOMDObjectMgrClassDataStructure * SOMLINK resolve_SOMDObjectMgrClassData(void);
			#ifndef SOMDObjectMgrClassData
				#define SOMDObjectMgrClassData    (*(resolve_SOMDObjectMgrClassData()))
			#endif /* SOMDObjectMgrClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_somdom_Source) || defined(SOMDObjectMgr_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_somdom_Source || SOMDObjectMgr_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_somdom_Source || SOMDObjectMgr_Class_Source */
		struct SOMDObjectMgrClassDataStructure SOMDLINK SOMDObjectMgrClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_somdom_Source) || defined(SOMDObjectMgr_Class_Source)
			SOMEXTERN struct SOMDObjectMgrCClassDataStructure _SOMDObjectMgrCClassData;
			#ifndef SOMDObjectMgrCClassData
				#define SOMDObjectMgrCClassData    _SOMDObjectMgrCClassData
			#endif /* SOMDObjectMgrCClassData */
		#else
			SOMEXTERN struct SOMDObjectMgrCClassDataStructure * SOMLINK resolve_SOMDObjectMgrCClassData(void);
			#ifndef SOMDObjectMgrCClassData
				#define SOMDObjectMgrCClassData    (*(resolve_SOMDObjectMgrCClassData()))
			#endif /* SOMDObjectMgrCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_somdom_Source) || defined(SOMDObjectMgr_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_somdom_Source || SOMDObjectMgr_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_somdom_Source || SOMDObjectMgr_Class_Source */
		struct SOMDObjectMgrCClassDataStructure SOMDLINK SOMDObjectMgrCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_somdom_Source) || defined(SOMDObjectMgr_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_somdom_Source || SOMDObjectMgr_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_somdom_Source || SOMDObjectMgr_Class_Source */
	SOMClass SOMSTAR SOMLINK SOMDObjectMgrNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_SOMDObjectMgr (SOMDObjectMgrClassData.classObject)
	#ifndef SOMGD_SOMDObjectMgr
		#if (defined(_SOMDObjectMgr) || defined(__SOMDObjectMgr))
			#undef _SOMDObjectMgr
			#undef __SOMDObjectMgr
			#define SOMGD_SOMDObjectMgr 1
		#else
			#define _SOMDObjectMgr _SOMCLASS_SOMDObjectMgr
		#endif /* _SOMDObjectMgr */
	#endif /* SOMGD_SOMDObjectMgr */
	#define SOMDObjectMgr_classObj _SOMCLASS_SOMDObjectMgr
	#define _SOMMTOKEN_SOMDObjectMgr(method) ((somMToken)(SOMDObjectMgrClassData.method))
	#ifndef SOMDObjectMgrNew
		#define SOMDObjectMgrNew() ( _SOMDObjectMgr ? \
			(SOMClass_somNew(_SOMDObjectMgr)) : \
			( SOMDObjectMgrNewClass( \
				SOMDObjectMgr_MajorVersion, \
				SOMDObjectMgr_MinorVersion), \
			SOMClass_somNew(_SOMDObjectMgr))) 
	#endif /* NewSOMDObjectMgr */
	#ifndef SOMDObjectMgr_somdFindServer
		#define SOMDObjectMgr_somdFindServer(somSelf,ev,serverid) \
			SOM_Resolve(somSelf,SOMDObjectMgr,somdFindServer)  \
				(somSelf,ev,serverid)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somdFindServer
				#if defined(_somdFindServer)
					#undef _somdFindServer
					#define SOMGD__somdFindServer
				#else
					#define _somdFindServer SOMDObjectMgr_somdFindServer
				#endif
			#endif /* SOMGD__somdFindServer */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDObjectMgr_somdFindServer */
	#ifndef SOMDObjectMgr_somdFindServerByName
		#define SOMDObjectMgr_somdFindServerByName(somSelf,ev,servername) \
			SOM_Resolve(somSelf,SOMDObjectMgr,somdFindServerByName)  \
				(somSelf,ev,servername)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somdFindServerByName
				#if defined(_somdFindServerByName)
					#undef _somdFindServerByName
					#define SOMGD__somdFindServerByName
				#else
					#define _somdFindServerByName SOMDObjectMgr_somdFindServerByName
				#endif
			#endif /* SOMGD__somdFindServerByName */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDObjectMgr_somdFindServerByName */
	#ifndef SOMDObjectMgr_somdFindServersByClass
		#define SOMDObjectMgr_somdFindServersByClass(somSelf,ev,objclass) \
			SOM_Resolve(somSelf,SOMDObjectMgr,somdFindServersByClass)  \
				(somSelf,ev,objclass)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somdFindServersByClass
				#if defined(_somdFindServersByClass)
					#undef _somdFindServersByClass
					#define SOMGD__somdFindServersByClass
				#else
					#define _somdFindServersByClass SOMDObjectMgr_somdFindServersByClass
				#endif
			#endif /* SOMGD__somdFindServersByClass */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDObjectMgr_somdFindServersByClass */
	#ifndef SOMDObjectMgr_somdFindAnyServerByClass
		#define SOMDObjectMgr_somdFindAnyServerByClass(somSelf,ev,objclass) \
			SOM_Resolve(somSelf,SOMDObjectMgr,somdFindAnyServerByClass)  \
				(somSelf,ev,objclass)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somdFindAnyServerByClass
				#if defined(_somdFindAnyServerByClass)
					#undef _somdFindAnyServerByClass
					#define SOMGD__somdFindAnyServerByClass
				#else
					#define _somdFindAnyServerByClass SOMDObjectMgr_somdFindAnyServerByClass
				#endif
			#endif /* SOMGD__somdFindAnyServerByClass */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDObjectMgr_somdFindAnyServerByClass */
	#ifndef SOMDObjectMgr__set_somd21somFree
		#define SOMDObjectMgr__set_somd21somFree(somSelf,ev,somd21somFree) \
			SOM_Resolve(somSelf,SOMDObjectMgr,_set_somd21somFree)  \
				(somSelf,ev,somd21somFree)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_somd21somFree
				#if defined(__set_somd21somFree)
					#undef __set_somd21somFree
					#define SOMGD___set_somd21somFree
				#else
					#define __set_somd21somFree SOMDObjectMgr__set_somd21somFree
				#endif
			#endif /* SOMGD___set_somd21somFree */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDObjectMgr__set_somd21somFree */
	#ifndef SOMDObjectMgr__get_somd21somFree
		#define SOMDObjectMgr__get_somd21somFree(somSelf,ev) \
			SOM_Resolve(somSelf,SOMDObjectMgr,_get_somd21somFree)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somd21somFree
				#if defined(__get_somd21somFree)
					#undef __get_somd21somFree
					#define SOMGD___get_somd21somFree
				#else
					#define __get_somd21somFree SOMDObjectMgr__get_somd21somFree
				#endif
			#endif /* SOMGD___get_somd21somFree */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDObjectMgr__get_somd21somFree */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define SOMDObjectMgr_somInit SOMObject_somInit
#define SOMDObjectMgr_somUninit SOMObject_somUninit
#define SOMDObjectMgr_somFree SOMObject_somFree
#define SOMDObjectMgr_somGetClass SOMObject_somGetClass
#define SOMDObjectMgr_somGetClassName SOMObject_somGetClassName
#define SOMDObjectMgr_somGetSize SOMObject_somGetSize
#define SOMDObjectMgr_somIsA SOMObject_somIsA
#define SOMDObjectMgr_somIsInstanceOf SOMObject_somIsInstanceOf
#define SOMDObjectMgr_somRespondsTo SOMObject_somRespondsTo
#define SOMDObjectMgr_somDispatch SOMObject_somDispatch
#define SOMDObjectMgr_somClassDispatch SOMObject_somClassDispatch
#define SOMDObjectMgr_somCastObj SOMObject_somCastObj
#define SOMDObjectMgr_somResetObj SOMObject_somResetObj
#define SOMDObjectMgr_somPrintSelf SOMObject_somPrintSelf
#define SOMDObjectMgr_somDumpSelf SOMObject_somDumpSelf
#define SOMDObjectMgr_somDumpSelfInt SOMObject_somDumpSelfInt
#define SOMDObjectMgr_somDefaultInit SOMObject_somDefaultInit
#define SOMDObjectMgr_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define SOMDObjectMgr_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define SOMDObjectMgr_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define SOMDObjectMgr_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define SOMDObjectMgr_somDefaultAssign SOMObject_somDefaultAssign
#define SOMDObjectMgr_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define SOMDObjectMgr_somDefaultVAssign SOMObject_somDefaultVAssign
#define SOMDObjectMgr_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define SOMDObjectMgr_somDestruct SOMObject_somDestruct
#define SOMDObjectMgr_somdNewObject ObjectMgr_somdNewObject
#define SOMDObjectMgr_somdGetIdFromObject ObjectMgr_somdGetIdFromObject
#define SOMDObjectMgr_somdGetObjectFromId ObjectMgr_somdGetObjectFromId
#define SOMDObjectMgr_somdReleaseObject ObjectMgr_somdReleaseObject
#define SOMDObjectMgr_somdDestroyObject ObjectMgr_somdDestroyObject
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_somdom_Header_h */
