/* generated from servmgr.idl */
/* internal conditional is SOM_Module_servmgr_Source */
#ifndef SOM_Module_servmgr_Header_h
	#define SOM_Module_servmgr_Header_h 1
	#include <som.h>
	#include <somobj.h>
	#include <somdtype.h>
	#include <containd.h>
	#ifndef _IDL_ImplementationDef_defined
		#define _IDL_ImplementationDef_defined
		typedef SOMObject ImplementationDef;
	#endif /* _IDL_ImplementationDef_defined */
	#ifndef _IDL_SEQUENCE_ImplementationDef_defined
		#define _IDL_SEQUENCE_ImplementationDef_defined
		SOM_SEQUENCE_TYPEDEF_NAME(ImplementationDef SOMSTAR ,sequence(ImplementationDef));
	#endif /* _IDL_SEQUENCE_ImplementationDef_defined */
	#ifndef _IDL_SOMDServerMgr_defined
		#define _IDL_SOMDServerMgr_defined
		typedef SOMObject SOMDServerMgr;
	#endif /* _IDL_SOMDServerMgr_defined */
	#ifndef _IDL_SEQUENCE_SOMDServerMgr_defined
		#define _IDL_SEQUENCE_SOMDServerMgr_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMDServerMgr SOMSTAR ,sequence(SOMDServerMgr));
	#endif /* _IDL_SEQUENCE_SOMDServerMgr_defined */
	#ifdef __IBMC__
		typedef ORBStatus (somTP_SOMDServerMgr_somdShutdownServer)(
			SOMDServerMgr SOMSTAR somSelf,
			Environment *ev,
			/* in */ string server_alias);
		#pragma linkage(somTP_SOMDServerMgr_somdShutdownServer,system)
		typedef somTP_SOMDServerMgr_somdShutdownServer *somTD_SOMDServerMgr_somdShutdownServer;
	#else /* __IBMC__ */
		typedef ORBStatus (SOMLINK * somTD_SOMDServerMgr_somdShutdownServer)(
			SOMDServerMgr SOMSTAR somSelf,
			Environment *ev,
			/* in */ string server_alias);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef ORBStatus (somTP_SOMDServerMgr_somdShutdownImpl)(
			SOMDServerMgr SOMSTAR somSelf,
			Environment *ev,
			/* in */ ImplementationDef SOMSTAR impldef);
		#pragma linkage(somTP_SOMDServerMgr_somdShutdownImpl,system)
		typedef somTP_SOMDServerMgr_somdShutdownImpl *somTD_SOMDServerMgr_somdShutdownImpl;
	#else /* __IBMC__ */
		typedef ORBStatus (SOMLINK * somTD_SOMDServerMgr_somdShutdownImpl)(
			SOMDServerMgr SOMSTAR somSelf,
			Environment *ev,
			/* in */ ImplementationDef SOMSTAR impldef);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef ORBStatus (somTP_SOMDServerMgr_somdStartServer)(
			SOMDServerMgr SOMSTAR somSelf,
			Environment *ev,
			/* in */ string server_alias);
		#pragma linkage(somTP_SOMDServerMgr_somdStartServer,system)
		typedef somTP_SOMDServerMgr_somdStartServer *somTD_SOMDServerMgr_somdStartServer;
	#else /* __IBMC__ */
		typedef ORBStatus (SOMLINK * somTD_SOMDServerMgr_somdStartServer)(
			SOMDServerMgr SOMSTAR somSelf,
			Environment *ev,
			/* in */ string server_alias);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef ORBStatus (somTP_SOMDServerMgr_somdRestartServer)(
			SOMDServerMgr SOMSTAR somSelf,
			Environment *ev,
			/* in */ string server_alias);
		#pragma linkage(somTP_SOMDServerMgr_somdRestartServer,system)
		typedef somTP_SOMDServerMgr_somdRestartServer *somTD_SOMDServerMgr_somdRestartServer;
	#else /* __IBMC__ */
		typedef ORBStatus (SOMLINK * somTD_SOMDServerMgr_somdRestartServer)(
			SOMDServerMgr SOMSTAR somSelf,
			Environment *ev,
			/* in */ string server_alias);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef ORBStatus (somTP_SOMDServerMgr_somdListServer)(
			SOMDServerMgr SOMSTAR somSelf,
			Environment *ev,
			/* in */ string server_alias);
		#pragma linkage(somTP_SOMDServerMgr_somdListServer,system)
		typedef somTP_SOMDServerMgr_somdListServer *somTD_SOMDServerMgr_somdListServer;
	#else /* __IBMC__ */
		typedef ORBStatus (SOMLINK * somTD_SOMDServerMgr_somdListServer)(
			SOMDServerMgr SOMSTAR somSelf,
			Environment *ev,
			/* in */ string server_alias);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef ORBStatus (somTP_SOMDServerMgr_somdDisableServer)(
			SOMDServerMgr SOMSTAR somSelf,
			Environment *ev,
			/* in */ string server_alias);
		#pragma linkage(somTP_SOMDServerMgr_somdDisableServer,system)
		typedef somTP_SOMDServerMgr_somdDisableServer *somTD_SOMDServerMgr_somdDisableServer;
	#else /* __IBMC__ */
		typedef ORBStatus (SOMLINK * somTD_SOMDServerMgr_somdDisableServer)(
			SOMDServerMgr SOMSTAR somSelf,
			Environment *ev,
			/* in */ string server_alias);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef ORBStatus (somTP_SOMDServerMgr_somdEnableServer)(
			SOMDServerMgr SOMSTAR somSelf,
			Environment *ev,
			/* in */ string server_alias);
		#pragma linkage(somTP_SOMDServerMgr_somdEnableServer,system)
		typedef somTP_SOMDServerMgr_somdEnableServer *somTD_SOMDServerMgr_somdEnableServer;
	#else /* __IBMC__ */
		typedef ORBStatus (SOMLINK * somTD_SOMDServerMgr_somdEnableServer)(
			SOMDServerMgr SOMSTAR somSelf,
			Environment *ev,
			/* in */ string server_alias);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef boolean (somTP_SOMDServerMgr_somdIsServerEnabled)(
			SOMDServerMgr SOMSTAR somSelf,
			Environment *ev,
			/* in */ ImplementationDef SOMSTAR impldef);
		#pragma linkage(somTP_SOMDServerMgr_somdIsServerEnabled,system)
		typedef somTP_SOMDServerMgr_somdIsServerEnabled *somTD_SOMDServerMgr_somdIsServerEnabled;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_SOMDServerMgr_somdIsServerEnabled)(
			SOMDServerMgr SOMSTAR somSelf,
			Environment *ev,
			/* in */ ImplementationDef SOMSTAR impldef);
	#endif /* __IBMC__ */
	#ifndef SOMDServerMgr_MajorVersion
		#define SOMDServerMgr_MajorVersion   2
	#endif /* SOMDServerMgr_MajorVersion */
	#ifndef SOMDServerMgr_MinorVersion
		#define SOMDServerMgr_MinorVersion   1
	#endif /* SOMDServerMgr_MinorVersion */
	typedef struct SOMDServerMgrClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken somdShutdownServer;
		somMToken somdStartServer;
		somMToken somdRestartServer;
		somMToken somdListServer;
		somMToken somdDisableServer;
		somMToken somdEnableServer;
		somMToken somdIsServerEnabled;
		somMToken somdShutdownImpl;
	} SOMDServerMgrClassDataStructure;
	typedef struct SOMDServerMgrCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} SOMDServerMgrCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_servmgr_Source) || defined(SOMDServerMgr_Class_Source)
			SOMEXTERN struct SOMDServerMgrClassDataStructure _SOMDServerMgrClassData;
			#ifndef SOMDServerMgrClassData
				#define SOMDServerMgrClassData    _SOMDServerMgrClassData
			#endif /* SOMDServerMgrClassData */
		#else
			SOMEXTERN struct SOMDServerMgrClassDataStructure * SOMLINK resolve_SOMDServerMgrClassData(void);
			#ifndef SOMDServerMgrClassData
				#define SOMDServerMgrClassData    (*(resolve_SOMDServerMgrClassData()))
			#endif /* SOMDServerMgrClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_servmgr_Source) || defined(SOMDServerMgr_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_servmgr_Source || SOMDServerMgr_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_servmgr_Source || SOMDServerMgr_Class_Source */
		struct SOMDServerMgrClassDataStructure SOMDLINK SOMDServerMgrClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_servmgr_Source) || defined(SOMDServerMgr_Class_Source)
			SOMEXTERN struct SOMDServerMgrCClassDataStructure _SOMDServerMgrCClassData;
			#ifndef SOMDServerMgrCClassData
				#define SOMDServerMgrCClassData    _SOMDServerMgrCClassData
			#endif /* SOMDServerMgrCClassData */
		#else
			SOMEXTERN struct SOMDServerMgrCClassDataStructure * SOMLINK resolve_SOMDServerMgrCClassData(void);
			#ifndef SOMDServerMgrCClassData
				#define SOMDServerMgrCClassData    (*(resolve_SOMDServerMgrCClassData()))
			#endif /* SOMDServerMgrCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_servmgr_Source) || defined(SOMDServerMgr_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_servmgr_Source || SOMDServerMgr_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_servmgr_Source || SOMDServerMgr_Class_Source */
		struct SOMDServerMgrCClassDataStructure SOMDLINK SOMDServerMgrCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_servmgr_Source) || defined(SOMDServerMgr_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_servmgr_Source || SOMDServerMgr_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_servmgr_Source || SOMDServerMgr_Class_Source */
	SOMClass SOMSTAR SOMLINK SOMDServerMgrNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_SOMDServerMgr (SOMDServerMgrClassData.classObject)
	#ifndef SOMGD_SOMDServerMgr
		#if (defined(_SOMDServerMgr) || defined(__SOMDServerMgr))
			#undef _SOMDServerMgr
			#undef __SOMDServerMgr
			#define SOMGD_SOMDServerMgr 1
		#else
			#define _SOMDServerMgr _SOMCLASS_SOMDServerMgr
		#endif /* _SOMDServerMgr */
	#endif /* SOMGD_SOMDServerMgr */
	#define SOMDServerMgr_classObj _SOMCLASS_SOMDServerMgr
	#define _SOMMTOKEN_SOMDServerMgr(method) ((somMToken)(SOMDServerMgrClassData.method))
	#ifndef SOMDServerMgrNew
		#define SOMDServerMgrNew() ( _SOMDServerMgr ? \
			(SOMClass_somNew(_SOMDServerMgr)) : \
			( SOMDServerMgrNewClass( \
				SOMDServerMgr_MajorVersion, \
				SOMDServerMgr_MinorVersion), \
			SOMClass_somNew(_SOMDServerMgr))) 
	#endif /* NewSOMDServerMgr */
	#ifndef SOMDServerMgr_somdShutdownServer
		#define SOMDServerMgr_somdShutdownServer(somSelf,ev,server_alias) \
			SOM_Resolve(somSelf,SOMDServerMgr,somdShutdownServer)  \
				(somSelf,ev,server_alias)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somdShutdownServer
				#if defined(_somdShutdownServer)
					#undef _somdShutdownServer
					#define SOMGD__somdShutdownServer
				#else
					#define _somdShutdownServer SOMDServerMgr_somdShutdownServer
				#endif
			#endif /* SOMGD__somdShutdownServer */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDServerMgr_somdShutdownServer */
	#ifndef SOMDServerMgr_somdShutdownImpl
		#define SOMDServerMgr_somdShutdownImpl(somSelf,ev,impldef) \
			SOM_Resolve(somSelf,SOMDServerMgr,somdShutdownImpl)  \
				(somSelf,ev,impldef)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somdShutdownImpl
				#if defined(_somdShutdownImpl)
					#undef _somdShutdownImpl
					#define SOMGD__somdShutdownImpl
				#else
					#define _somdShutdownImpl SOMDServerMgr_somdShutdownImpl
				#endif
			#endif /* SOMGD__somdShutdownImpl */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDServerMgr_somdShutdownImpl */
	#ifndef SOMDServerMgr_somdStartServer
		#define SOMDServerMgr_somdStartServer(somSelf,ev,server_alias) \
			SOM_Resolve(somSelf,SOMDServerMgr,somdStartServer)  \
				(somSelf,ev,server_alias)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somdStartServer
				#if defined(_somdStartServer)
					#undef _somdStartServer
					#define SOMGD__somdStartServer
				#else
					#define _somdStartServer SOMDServerMgr_somdStartServer
				#endif
			#endif /* SOMGD__somdStartServer */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDServerMgr_somdStartServer */
	#ifndef SOMDServerMgr_somdRestartServer
		#define SOMDServerMgr_somdRestartServer(somSelf,ev,server_alias) \
			SOM_Resolve(somSelf,SOMDServerMgr,somdRestartServer)  \
				(somSelf,ev,server_alias)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somdRestartServer
				#if defined(_somdRestartServer)
					#undef _somdRestartServer
					#define SOMGD__somdRestartServer
				#else
					#define _somdRestartServer SOMDServerMgr_somdRestartServer
				#endif
			#endif /* SOMGD__somdRestartServer */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDServerMgr_somdRestartServer */
	#ifndef SOMDServerMgr_somdListServer
		#define SOMDServerMgr_somdListServer(somSelf,ev,server_alias) \
			SOM_Resolve(somSelf,SOMDServerMgr,somdListServer)  \
				(somSelf,ev,server_alias)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somdListServer
				#if defined(_somdListServer)
					#undef _somdListServer
					#define SOMGD__somdListServer
				#else
					#define _somdListServer SOMDServerMgr_somdListServer
				#endif
			#endif /* SOMGD__somdListServer */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDServerMgr_somdListServer */
	#ifndef SOMDServerMgr_somdDisableServer
		#define SOMDServerMgr_somdDisableServer(somSelf,ev,server_alias) \
			SOM_Resolve(somSelf,SOMDServerMgr,somdDisableServer)  \
				(somSelf,ev,server_alias)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somdDisableServer
				#if defined(_somdDisableServer)
					#undef _somdDisableServer
					#define SOMGD__somdDisableServer
				#else
					#define _somdDisableServer SOMDServerMgr_somdDisableServer
				#endif
			#endif /* SOMGD__somdDisableServer */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDServerMgr_somdDisableServer */
	#ifndef SOMDServerMgr_somdEnableServer
		#define SOMDServerMgr_somdEnableServer(somSelf,ev,server_alias) \
			SOM_Resolve(somSelf,SOMDServerMgr,somdEnableServer)  \
				(somSelf,ev,server_alias)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somdEnableServer
				#if defined(_somdEnableServer)
					#undef _somdEnableServer
					#define SOMGD__somdEnableServer
				#else
					#define _somdEnableServer SOMDServerMgr_somdEnableServer
				#endif
			#endif /* SOMGD__somdEnableServer */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDServerMgr_somdEnableServer */
	#ifndef SOMDServerMgr_somdIsServerEnabled
		#define SOMDServerMgr_somdIsServerEnabled(somSelf,ev,impldef) \
			SOM_Resolve(somSelf,SOMDServerMgr,somdIsServerEnabled)  \
				(somSelf,ev,impldef)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somdIsServerEnabled
				#if defined(_somdIsServerEnabled)
					#undef _somdIsServerEnabled
					#define SOMGD__somdIsServerEnabled
				#else
					#define _somdIsServerEnabled SOMDServerMgr_somdIsServerEnabled
				#endif
			#endif /* SOMGD__somdIsServerEnabled */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDServerMgr_somdIsServerEnabled */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define SOMDServerMgr_somInit SOMObject_somInit
#define SOMDServerMgr_somUninit SOMObject_somUninit
#define SOMDServerMgr_somFree SOMObject_somFree
#define SOMDServerMgr_somGetClass SOMObject_somGetClass
#define SOMDServerMgr_somGetClassName SOMObject_somGetClassName
#define SOMDServerMgr_somGetSize SOMObject_somGetSize
#define SOMDServerMgr_somIsA SOMObject_somIsA
#define SOMDServerMgr_somIsInstanceOf SOMObject_somIsInstanceOf
#define SOMDServerMgr_somRespondsTo SOMObject_somRespondsTo
#define SOMDServerMgr_somDispatch SOMObject_somDispatch
#define SOMDServerMgr_somClassDispatch SOMObject_somClassDispatch
#define SOMDServerMgr_somCastObj SOMObject_somCastObj
#define SOMDServerMgr_somResetObj SOMObject_somResetObj
#define SOMDServerMgr_somPrintSelf SOMObject_somPrintSelf
#define SOMDServerMgr_somDumpSelf SOMObject_somDumpSelf
#define SOMDServerMgr_somDumpSelfInt SOMObject_somDumpSelfInt
#define SOMDServerMgr_somDefaultInit SOMObject_somDefaultInit
#define SOMDServerMgr_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define SOMDServerMgr_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define SOMDServerMgr_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define SOMDServerMgr_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define SOMDServerMgr_somDefaultAssign SOMObject_somDefaultAssign
#define SOMDServerMgr_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define SOMDServerMgr_somDefaultVAssign SOMObject_somDefaultVAssign
#define SOMDServerMgr_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define SOMDServerMgr_somDestruct SOMObject_somDestruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_servmgr_Header_h */
