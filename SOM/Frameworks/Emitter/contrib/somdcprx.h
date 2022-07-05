/* generated from somdcprx.idl */
/* internal conditional is SOM_Module_somdcprx_Source */
#ifndef SOM_Module_somdcprx_Header_h
	#define SOM_Module_somdcprx_Header_h 1
	#include <som.h>
	#include <somobj.h>
	#include <somproxy.h>
	#include <somcls.h>
	#include <somdobj.h>
	#include <unotypes.h>
	#include <containd.h>
	#include <somdtype.h>
	#ifndef _IDL_SOMDClientProxy_defined
		#define _IDL_SOMDClientProxy_defined
		typedef SOMObject SOMDClientProxy;
	#endif /* _IDL_SOMDClientProxy_defined */
	#ifndef _IDL_SEQUENCE_SOMDClientProxy_defined
		#define _IDL_SEQUENCE_SOMDClientProxy_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMDClientProxy SOMSTAR ,sequence(SOMDClientProxy));
	#endif /* _IDL_SEQUENCE_SOMDClientProxy_defined */
	#ifdef __IBMC__
		typedef void (somTP_SOMDClientProxy_somdTargetFree)(
			SOMDClientProxy SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_SOMDClientProxy_somdTargetFree,system)
		typedef somTP_SOMDClientProxy_somdTargetFree *somTD_SOMDClientProxy_somdTargetFree;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMDClientProxy_somdTargetFree)(
			SOMDClientProxy SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMClass SOMSTAR (somTP_SOMDClientProxy_somdTargetGetClass)(
			SOMDClientProxy SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_SOMDClientProxy_somdTargetGetClass,system)
		typedef somTP_SOMDClientProxy_somdTargetGetClass *somTD_SOMDClientProxy_somdTargetGetClass;
	#else /* __IBMC__ */
		typedef SOMClass SOMSTAR (SOMLINK * somTD_SOMDClientProxy_somdTargetGetClass)(
			SOMDClientProxy SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_SOMDClientProxy_somdTargetGetClassName)(
			SOMDClientProxy SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_SOMDClientProxy_somdTargetGetClassName,system)
		typedef somTP_SOMDClientProxy_somdTargetGetClassName *somTD_SOMDClientProxy_somdTargetGetClassName;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_SOMDClientProxy_somdTargetGetClassName)(
			SOMDClientProxy SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMDClientProxy_somdProxyFree)(
			SOMDClientProxy SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_SOMDClientProxy_somdProxyFree,system)
		typedef somTP_SOMDClientProxy_somdProxyFree *somTD_SOMDClientProxy_somdProxyFree;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMDClientProxy_somdProxyFree)(
			SOMDClientProxy SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMClass SOMSTAR (somTP_SOMDClientProxy_somdProxyGetClass)(
			SOMDClientProxy SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_SOMDClientProxy_somdProxyGetClass,system)
		typedef somTP_SOMDClientProxy_somdProxyGetClass *somTD_SOMDClientProxy_somdProxyGetClass;
	#else /* __IBMC__ */
		typedef SOMClass SOMSTAR (SOMLINK * somTD_SOMDClientProxy_somdProxyGetClass)(
			SOMDClientProxy SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_SOMDClientProxy_somdProxyGetClassName)(
			SOMDClientProxy SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_SOMDClientProxy_somdProxyGetClassName,system)
		typedef somTP_SOMDClientProxy_somdProxyGetClassName *somTD_SOMDClientProxy_somdProxyGetClassName;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_SOMDClientProxy_somdProxyGetClassName)(
			SOMDClientProxy SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMDClientProxy_somdReleaseResources)(
			SOMDClientProxy SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_SOMDClientProxy_somdReleaseResources,system)
		typedef somTP_SOMDClientProxy_somdReleaseResources *somTD_SOMDClientProxy_somdReleaseResources;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMDClientProxy_somdReleaseResources)(
			SOMDClientProxy SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifndef SOMDClientProxy_MajorVersion
		#define SOMDClientProxy_MajorVersion   2
	#endif /* SOMDClientProxy_MajorVersion */
	#ifndef SOMDClientProxy_MinorVersion
		#define SOMDClientProxy_MinorVersion   2
	#endif /* SOMDClientProxy_MinorVersion */
	typedef struct SOMDClientProxyClassDataStructure
	{
		SOMMProxyFor SOMSTAR classObject;
		somMToken somdTargetFree;
		somMToken somdTargetGetClass;
		somMToken somdTargetGetClassName;
		somMToken somdProxyFree;
		somMToken somdProxyGetClass;
		somMToken somdProxyGetClassName;
		somMToken private6;
		somMToken private7;
		somMToken private8;
		somMToken private9;
		somMToken private10;
		somMToken somdReleaseResources;
		somMToken private12;
		somMToken private13;
		somMToken private14;
		somMToken private15;
		somMToken private16;
	} SOMDClientProxyClassDataStructure;
	typedef struct SOMDClientProxyCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} SOMDClientProxyCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_somdcprx_Source) || defined(SOMDClientProxy_Class_Source)
			SOMEXTERN struct SOMDClientProxyClassDataStructure _SOMDClientProxyClassData;
			#ifndef SOMDClientProxyClassData
				#define SOMDClientProxyClassData    _SOMDClientProxyClassData
			#endif /* SOMDClientProxyClassData */
		#else
			SOMEXTERN struct SOMDClientProxyClassDataStructure * SOMLINK resolve_SOMDClientProxyClassData(void);
			#ifndef SOMDClientProxyClassData
				#define SOMDClientProxyClassData    (*(resolve_SOMDClientProxyClassData()))
			#endif /* SOMDClientProxyClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_somdcprx_Source) || defined(SOMDClientProxy_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_somdcprx_Source || SOMDClientProxy_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_somdcprx_Source || SOMDClientProxy_Class_Source */
		struct SOMDClientProxyClassDataStructure SOMDLINK SOMDClientProxyClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_somdcprx_Source) || defined(SOMDClientProxy_Class_Source)
			SOMEXTERN struct SOMDClientProxyCClassDataStructure _SOMDClientProxyCClassData;
			#ifndef SOMDClientProxyCClassData
				#define SOMDClientProxyCClassData    _SOMDClientProxyCClassData
			#endif /* SOMDClientProxyCClassData */
		#else
			SOMEXTERN struct SOMDClientProxyCClassDataStructure * SOMLINK resolve_SOMDClientProxyCClassData(void);
			#ifndef SOMDClientProxyCClassData
				#define SOMDClientProxyCClassData    (*(resolve_SOMDClientProxyCClassData()))
			#endif /* SOMDClientProxyCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_somdcprx_Source) || defined(SOMDClientProxy_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_somdcprx_Source || SOMDClientProxy_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_somdcprx_Source || SOMDClientProxy_Class_Source */
		struct SOMDClientProxyCClassDataStructure SOMDLINK SOMDClientProxyCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_somdcprx_Source) || defined(SOMDClientProxy_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_somdcprx_Source || SOMDClientProxy_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_somdcprx_Source || SOMDClientProxy_Class_Source */
	SOMMProxyFor SOMSTAR SOMLINK SOMDClientProxyNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_SOMDClientProxy (SOMDClientProxyClassData.classObject)
	#ifndef SOMGD_SOMDClientProxy
		#if (defined(_SOMDClientProxy) || defined(__SOMDClientProxy))
			#undef _SOMDClientProxy
			#undef __SOMDClientProxy
			#define SOMGD_SOMDClientProxy 1
		#else
			#define _SOMDClientProxy _SOMCLASS_SOMDClientProxy
		#endif /* _SOMDClientProxy */
	#endif /* SOMGD_SOMDClientProxy */
	#define SOMDClientProxy_classObj _SOMCLASS_SOMDClientProxy
	#define _SOMMTOKEN_SOMDClientProxy(method) ((somMToken)(SOMDClientProxyClassData.method))
	#ifndef SOMDClientProxyNew
		#define SOMDClientProxyNew() ( _SOMDClientProxy ? \
			(SOMClass_somNew(_SOMDClientProxy)) : \
			( SOMDClientProxyNewClass( \
				SOMDClientProxy_MajorVersion, \
				SOMDClientProxy_MinorVersion), \
			SOMClass_somNew(_SOMDClientProxy))) 
	#endif /* NewSOMDClientProxy */
	#ifndef SOMDClientProxy_somdTargetFree
		#define SOMDClientProxy_somdTargetFree(somSelf,ev) \
			SOM_Resolve(somSelf,SOMDClientProxy,somdTargetFree)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somdTargetFree
				#if defined(_somdTargetFree)
					#undef _somdTargetFree
					#define SOMGD__somdTargetFree
				#else
					#define _somdTargetFree SOMDClientProxy_somdTargetFree
				#endif
			#endif /* SOMGD__somdTargetFree */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDClientProxy_somdTargetFree */
	#ifndef SOMDClientProxy_somdTargetGetClass
		#define SOMDClientProxy_somdTargetGetClass(somSelf,ev) \
			SOM_Resolve(somSelf,SOMDClientProxy,somdTargetGetClass)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somdTargetGetClass
				#if defined(_somdTargetGetClass)
					#undef _somdTargetGetClass
					#define SOMGD__somdTargetGetClass
				#else
					#define _somdTargetGetClass SOMDClientProxy_somdTargetGetClass
				#endif
			#endif /* SOMGD__somdTargetGetClass */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDClientProxy_somdTargetGetClass */
	#ifndef SOMDClientProxy_somdTargetGetClassName
		#define SOMDClientProxy_somdTargetGetClassName(somSelf,ev) \
			SOM_Resolve(somSelf,SOMDClientProxy,somdTargetGetClassName)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somdTargetGetClassName
				#if defined(_somdTargetGetClassName)
					#undef _somdTargetGetClassName
					#define SOMGD__somdTargetGetClassName
				#else
					#define _somdTargetGetClassName SOMDClientProxy_somdTargetGetClassName
				#endif
			#endif /* SOMGD__somdTargetGetClassName */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDClientProxy_somdTargetGetClassName */
	#ifndef SOMDClientProxy_somdProxyFree
		#define SOMDClientProxy_somdProxyFree(somSelf,ev) \
			SOM_Resolve(somSelf,SOMDClientProxy,somdProxyFree)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somdProxyFree
				#if defined(_somdProxyFree)
					#undef _somdProxyFree
					#define SOMGD__somdProxyFree
				#else
					#define _somdProxyFree SOMDClientProxy_somdProxyFree
				#endif
			#endif /* SOMGD__somdProxyFree */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDClientProxy_somdProxyFree */
	#ifndef SOMDClientProxy_somdProxyGetClass
		#define SOMDClientProxy_somdProxyGetClass(somSelf,ev) \
			SOM_Resolve(somSelf,SOMDClientProxy,somdProxyGetClass)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somdProxyGetClass
				#if defined(_somdProxyGetClass)
					#undef _somdProxyGetClass
					#define SOMGD__somdProxyGetClass
				#else
					#define _somdProxyGetClass SOMDClientProxy_somdProxyGetClass
				#endif
			#endif /* SOMGD__somdProxyGetClass */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDClientProxy_somdProxyGetClass */
	#ifndef SOMDClientProxy_somdProxyGetClassName
		#define SOMDClientProxy_somdProxyGetClassName(somSelf,ev) \
			SOM_Resolve(somSelf,SOMDClientProxy,somdProxyGetClassName)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somdProxyGetClassName
				#if defined(_somdProxyGetClassName)
					#undef _somdProxyGetClassName
					#define SOMGD__somdProxyGetClassName
				#else
					#define _somdProxyGetClassName SOMDClientProxy_somdProxyGetClassName
				#endif
			#endif /* SOMGD__somdProxyGetClassName */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDClientProxy_somdProxyGetClassName */
	#ifndef SOMDClientProxy_somdReleaseResources
		#define SOMDClientProxy_somdReleaseResources(somSelf,ev) \
			SOM_Resolve(somSelf,SOMDClientProxy,somdReleaseResources)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somdReleaseResources
				#if defined(_somdReleaseResources)
					#undef _somdReleaseResources
					#define SOMGD__somdReleaseResources
				#else
					#define _somdReleaseResources SOMDClientProxy_somdReleaseResources
				#endif
			#endif /* SOMGD__somdReleaseResources */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDClientProxy_somdReleaseResources */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define SOMDClientProxy_somInit SOMObject_somInit
#define SOMDClientProxy_somUninit SOMObject_somUninit
#define SOMDClientProxy_somFree SOMObject_somFree
#define SOMDClientProxy_somGetClass SOMObject_somGetClass
#define SOMDClientProxy_somGetClassName SOMObject_somGetClassName
#define SOMDClientProxy_somGetSize SOMObject_somGetSize
#define SOMDClientProxy_somIsA SOMObject_somIsA
#define SOMDClientProxy_somIsInstanceOf SOMObject_somIsInstanceOf
#define SOMDClientProxy_somRespondsTo SOMObject_somRespondsTo
#define SOMDClientProxy_somDispatch SOMObject_somDispatch
#define SOMDClientProxy_somClassDispatch SOMObject_somClassDispatch
#define SOMDClientProxy_somCastObj SOMObject_somCastObj
#define SOMDClientProxy_somResetObj SOMObject_somResetObj
#define SOMDClientProxy_somPrintSelf SOMObject_somPrintSelf
#define SOMDClientProxy_somDumpSelf SOMObject_somDumpSelf
#define SOMDClientProxy_somDumpSelfInt SOMObject_somDumpSelfInt
#define SOMDClientProxy_somDefaultInit SOMObject_somDefaultInit
#define SOMDClientProxy_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define SOMDClientProxy_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define SOMDClientProxy_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define SOMDClientProxy_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define SOMDClientProxy_somDefaultAssign SOMObject_somDefaultAssign
#define SOMDClientProxy_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define SOMDClientProxy_somDefaultVAssign SOMObject_somDefaultVAssign
#define SOMDClientProxy_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define SOMDClientProxy_somDestruct SOMObject_somDestruct
#define SOMDClientProxy__set_sommProxyTarget SOMMProxyForObject__set_sommProxyTarget
#define SOMDClientProxy__get_sommProxyTarget SOMMProxyForObject__get_sommProxyTarget
#define SOMDClientProxy_sommProxyDispatch SOMMProxyForObject_sommProxyDispatch
#define SOMDClientProxy_is_nil SOMDObject_is_nil
#define SOMDClientProxy_is_SOM_ref SOMDObject_is_SOM_ref
#define SOMDClientProxy_is_constant SOMDObject_is_constant
#define SOMDClientProxy_get_implementation SOMDObject_get_implementation
#define SOMDClientProxy_get_interface SOMDObject_get_interface
#define SOMDClientProxy_is_proxy SOMDObject_is_proxy
#define SOMDClientProxy_duplicate SOMDObject_duplicate
#define SOMDClientProxy_release SOMDObject_release
#define SOMDClientProxy_create_request SOMDObject_create_request
#define SOMDClientProxy_create_request_args SOMDObject_create_request_args
#define SOMDClientProxy__set_type_id SOMDObject__set_type_id
#define SOMDClientProxy__get_type_id SOMDObject__get_type_id
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_somdcprx_Header_h */
