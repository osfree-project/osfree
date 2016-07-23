/* generated from boa.idl */
/* internal conditional is SOM_Module_boa_Source */
#ifndef SOM_Module_boa_Header_h
	#define SOM_Module_boa_Header_h 1
	#include <som.h>
	#include <somobj.h>
	#include <somdtype.h>
	#include <containd.h>
	#include <snglicls.h>
	#include <somcls.h>
	#ifndef _IDL_SOMDObject_defined
		#define _IDL_SOMDObject_defined
		typedef SOMObject SOMDObject;
	#endif /* _IDL_SOMDObject_defined */
	#ifndef _IDL_SEQUENCE_SOMDObject_defined
		#define _IDL_SEQUENCE_SOMDObject_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMDObject SOMSTAR ,sequence(SOMDObject));
	#endif /* _IDL_SEQUENCE_SOMDObject_defined */
	#ifndef _IDL_Principal_defined
		#define _IDL_Principal_defined
		typedef SOMObject Principal;
	#endif /* _IDL_Principal_defined */
	#ifndef _IDL_SEQUENCE_Principal_defined
		#define _IDL_SEQUENCE_Principal_defined
		SOM_SEQUENCE_TYPEDEF_NAME(Principal SOMSTAR ,sequence(Principal));
	#endif /* _IDL_SEQUENCE_Principal_defined */
	#ifndef _IDL_ImplementationDef_defined
		#define _IDL_ImplementationDef_defined
		typedef SOMObject ImplementationDef;
	#endif /* _IDL_ImplementationDef_defined */
	#ifndef _IDL_SEQUENCE_ImplementationDef_defined
		#define _IDL_SEQUENCE_ImplementationDef_defined
		SOM_SEQUENCE_TYPEDEF_NAME(ImplementationDef SOMSTAR ,sequence(ImplementationDef));
	#endif /* _IDL_SEQUENCE_ImplementationDef_defined */
	#ifndef _IDL_InterfaceDef_defined
		#define _IDL_InterfaceDef_defined
		typedef SOMObject InterfaceDef;
	#endif /* _IDL_InterfaceDef_defined */
	#ifndef _IDL_SEQUENCE_InterfaceDef_defined
		#define _IDL_SEQUENCE_InterfaceDef_defined
		SOM_SEQUENCE_TYPEDEF_NAME(InterfaceDef SOMSTAR ,sequence(InterfaceDef));
	#endif /* _IDL_SEQUENCE_InterfaceDef_defined */
	#ifndef _IDL_Context_defined
		#define _IDL_Context_defined
		typedef SOMObject Context;
	#endif /* _IDL_Context_defined */
	#ifndef _IDL_SEQUENCE_Context_defined
		#define _IDL_SEQUENCE_Context_defined
		SOM_SEQUENCE_TYPEDEF_NAME(Context SOMSTAR ,sequence(Context));
	#endif /* _IDL_SEQUENCE_Context_defined */
	#ifndef _IDL_BOA_defined
		#define _IDL_BOA_defined
		typedef SOMObject BOA;
	#endif /* _IDL_BOA_defined */
	#ifndef _IDL_SEQUENCE_BOA_defined
		#define _IDL_SEQUENCE_BOA_defined
		SOM_SEQUENCE_TYPEDEF_NAME(BOA SOMSTAR ,sequence(BOA));
	#endif /* _IDL_SEQUENCE_BOA_defined */
	#ifdef __IBMC__
		typedef SOMDObject SOMSTAR (somTP_BOA_create)(
			BOA SOMSTAR somSelf,
			Environment *ev,
			/* in */ ReferenceData *id,
			/* in */ InterfaceDef SOMSTAR intf,
			/* in */ ImplementationDef SOMSTAR impl);
		#pragma linkage(somTP_BOA_create,system)
		typedef somTP_BOA_create *somTD_BOA_create;
	#else /* __IBMC__ */
		typedef SOMDObject SOMSTAR (SOMLINK * somTD_BOA_create)(
			BOA SOMSTAR somSelf,
			Environment *ev,
			/* in */ ReferenceData *id,
			/* in */ InterfaceDef SOMSTAR intf,
			/* in */ ImplementationDef SOMSTAR impl);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_BOA_dispose)(
			BOA SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMDObject SOMSTAR obj);
		#pragma linkage(somTP_BOA_dispose,system)
		typedef somTP_BOA_dispose *somTD_BOA_dispose;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_BOA_dispose)(
			BOA SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMDObject SOMSTAR obj);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef ReferenceData (somTP_BOA_get_id)(
			BOA SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMDObject SOMSTAR obj);
		#pragma linkage(somTP_BOA_get_id,system)
		typedef somTP_BOA_get_id *somTD_BOA_get_id;
	#else /* __IBMC__ */
		typedef ReferenceData (SOMLINK * somTD_BOA_get_id)(
			BOA SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMDObject SOMSTAR obj);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef Principal SOMSTAR (somTP_BOA_get_principal)(
			BOA SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMDObject SOMSTAR obj,
			/* in */ Environment *req_ev);
		#pragma linkage(somTP_BOA_get_principal,system)
		typedef somTP_BOA_get_principal *somTD_BOA_get_principal;
	#else /* __IBMC__ */
		typedef Principal SOMSTAR (SOMLINK * somTD_BOA_get_principal)(
			BOA SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMDObject SOMSTAR obj,
			/* in */ Environment *req_ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_BOA_set_exception)(
			BOA SOMSTAR somSelf,
			Environment *ev,
			/* in */ exception_type major,
			/* in */ string userid,
			/* in */ void *param);
		#pragma linkage(somTP_BOA_set_exception,system)
		typedef somTP_BOA_set_exception *somTD_BOA_set_exception;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_BOA_set_exception)(
			BOA SOMSTAR somSelf,
			Environment *ev,
			/* in */ exception_type major,
			/* in */ string userid,
			/* in */ void *param);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_BOA_impl_is_ready)(
			BOA SOMSTAR somSelf,
			Environment *ev,
			/* in */ ImplementationDef SOMSTAR impl);
		#pragma linkage(somTP_BOA_impl_is_ready,system)
		typedef somTP_BOA_impl_is_ready *somTD_BOA_impl_is_ready;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_BOA_impl_is_ready)(
			BOA SOMSTAR somSelf,
			Environment *ev,
			/* in */ ImplementationDef SOMSTAR impl);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_BOA_deactivate_impl)(
			BOA SOMSTAR somSelf,
			Environment *ev,
			/* in */ ImplementationDef SOMSTAR impl);
		#pragma linkage(somTP_BOA_deactivate_impl,system)
		typedef somTP_BOA_deactivate_impl *somTD_BOA_deactivate_impl;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_BOA_deactivate_impl)(
			BOA SOMSTAR somSelf,
			Environment *ev,
			/* in */ ImplementationDef SOMSTAR impl);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_BOA_change_implementation)(
			BOA SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMDObject SOMSTAR obj,
			/* in */ ImplementationDef SOMSTAR impl);
		#pragma linkage(somTP_BOA_change_implementation,system)
		typedef somTP_BOA_change_implementation *somTD_BOA_change_implementation;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_BOA_change_implementation)(
			BOA SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMDObject SOMSTAR obj,
			/* in */ ImplementationDef SOMSTAR impl);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_BOA_obj_is_ready)(
			BOA SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMDObject SOMSTAR obj,
			/* in */ ImplementationDef SOMSTAR impl);
		#pragma linkage(somTP_BOA_obj_is_ready,system)
		typedef somTP_BOA_obj_is_ready *somTD_BOA_obj_is_ready;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_BOA_obj_is_ready)(
			BOA SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMDObject SOMSTAR obj,
			/* in */ ImplementationDef SOMSTAR impl);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_BOA_deactivate_obj)(
			BOA SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMDObject SOMSTAR obj);
		#pragma linkage(somTP_BOA_deactivate_obj,system)
		typedef somTP_BOA_deactivate_obj *somTD_BOA_deactivate_obj;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_BOA_deactivate_obj)(
			BOA SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMDObject SOMSTAR obj);
	#endif /* __IBMC__ */
	#ifndef BOA_MajorVersion
		#define BOA_MajorVersion   2
	#endif /* BOA_MajorVersion */
	#ifndef BOA_MinorVersion
		#define BOA_MinorVersion   2
	#endif /* BOA_MinorVersion */
	typedef struct BOAClassDataStructure
	{
		SOMMSingleInstance SOMSTAR classObject;
		somMToken create;
		somMToken dispose;
		somMToken get_id;
		somMToken change_implementation;
		somMToken get_principal;
		somMToken set_exception;
		somMToken impl_is_ready;
		somMToken deactivate_impl;
		somMToken obj_is_ready;
		somMToken deactivate_obj;
		somMToken private10;
		somMToken private11;
	} BOAClassDataStructure;
	typedef struct BOACClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} BOACClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_boa_Source) || defined(BOA_Class_Source)
			SOMEXTERN struct BOAClassDataStructure _BOAClassData;
			#ifndef BOAClassData
				#define BOAClassData    _BOAClassData
			#endif /* BOAClassData */
		#else
			SOMEXTERN struct BOAClassDataStructure * SOMLINK resolve_BOAClassData(void);
			#ifndef BOAClassData
				#define BOAClassData    (*(resolve_BOAClassData()))
			#endif /* BOAClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_boa_Source) || defined(BOA_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_boa_Source || BOA_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_boa_Source || BOA_Class_Source */
		struct BOAClassDataStructure SOMDLINK BOAClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_boa_Source) || defined(BOA_Class_Source)
			SOMEXTERN struct BOACClassDataStructure _BOACClassData;
			#ifndef BOACClassData
				#define BOACClassData    _BOACClassData
			#endif /* BOACClassData */
		#else
			SOMEXTERN struct BOACClassDataStructure * SOMLINK resolve_BOACClassData(void);
			#ifndef BOACClassData
				#define BOACClassData    (*(resolve_BOACClassData()))
			#endif /* BOACClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_boa_Source) || defined(BOA_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_boa_Source || BOA_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_boa_Source || BOA_Class_Source */
		struct BOACClassDataStructure SOMDLINK BOACClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_boa_Source) || defined(BOA_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_boa_Source || BOA_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_boa_Source || BOA_Class_Source */
	SOMMSingleInstance SOMSTAR SOMLINK BOANewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_BOA (BOAClassData.classObject)
	#ifndef SOMGD_BOA
		#if (defined(_BOA) || defined(__BOA))
			#undef _BOA
			#undef __BOA
			#define SOMGD_BOA 1
		#else
			#define _BOA _SOMCLASS_BOA
		#endif /* _BOA */
	#endif /* SOMGD_BOA */
	#define BOA_classObj _SOMCLASS_BOA
	#define _SOMMTOKEN_BOA(method) ((somMToken)(BOAClassData.method))
	#ifndef BOANew
		#define BOANew() ( _BOA ? \
			(SOMClass_somNew(_BOA)) : \
			( BOANewClass( \
				BOA_MajorVersion, \
				BOA_MinorVersion), \
			SOMClass_somNew(_BOA))) 
	#endif /* NewBOA */
	#ifndef BOA_create
		#define BOA_create(somSelf,ev,id,intf,impl) \
			SOM_Resolve(somSelf,BOA,create)  \
				(somSelf,ev,id,intf,impl)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__create
				#if defined(_create)
					#undef _create
					#define SOMGD__create
				#else
					#define _create BOA_create
				#endif
			#endif /* SOMGD__create */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* BOA_create */
	#ifndef BOA_dispose
		#define BOA_dispose(somSelf,ev,obj) \
			SOM_Resolve(somSelf,BOA,dispose)  \
				(somSelf,ev,obj)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__dispose
				#if defined(_dispose)
					#undef _dispose
					#define SOMGD__dispose
				#else
					#define _dispose BOA_dispose
				#endif
			#endif /* SOMGD__dispose */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* BOA_dispose */
	#ifndef BOA_get_id
		#define BOA_get_id(somSelf,ev,obj) \
			SOM_Resolve(somSelf,BOA,get_id)  \
				(somSelf,ev,obj)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__get_id
				#if defined(_get_id)
					#undef _get_id
					#define SOMGD__get_id
				#else
					#define _get_id BOA_get_id
				#endif
			#endif /* SOMGD__get_id */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* BOA_get_id */
	#ifndef BOA_get_principal
		#define BOA_get_principal(somSelf,ev,obj,req_ev) \
			SOM_Resolve(somSelf,BOA,get_principal)  \
				(somSelf,ev,obj,req_ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__get_principal
				#if defined(_get_principal)
					#undef _get_principal
					#define SOMGD__get_principal
				#else
					#define _get_principal BOA_get_principal
				#endif
			#endif /* SOMGD__get_principal */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* BOA_get_principal */
	#ifndef BOA_set_exception
		#define BOA_set_exception(somSelf,ev,major,userid,param) \
			SOM_Resolve(somSelf,BOA,set_exception)  \
				(somSelf,ev,major,userid,param)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__set_exception
				#if defined(_set_exception)
					#undef _set_exception
					#define SOMGD__set_exception
				#else
					#define _set_exception BOA_set_exception
				#endif
			#endif /* SOMGD__set_exception */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* BOA_set_exception */
	#ifndef BOA_impl_is_ready
		#define BOA_impl_is_ready(somSelf,ev,impl) \
			SOM_Resolve(somSelf,BOA,impl_is_ready)  \
				(somSelf,ev,impl)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__impl_is_ready
				#if defined(_impl_is_ready)
					#undef _impl_is_ready
					#define SOMGD__impl_is_ready
				#else
					#define _impl_is_ready BOA_impl_is_ready
				#endif
			#endif /* SOMGD__impl_is_ready */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* BOA_impl_is_ready */
	#ifndef BOA_deactivate_impl
		#define BOA_deactivate_impl(somSelf,ev,impl) \
			SOM_Resolve(somSelf,BOA,deactivate_impl)  \
				(somSelf,ev,impl)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__deactivate_impl
				#if defined(_deactivate_impl)
					#undef _deactivate_impl
					#define SOMGD__deactivate_impl
				#else
					#define _deactivate_impl BOA_deactivate_impl
				#endif
			#endif /* SOMGD__deactivate_impl */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* BOA_deactivate_impl */
	#ifndef BOA_change_implementation
		#define BOA_change_implementation(somSelf,ev,obj,impl) \
			SOM_Resolve(somSelf,BOA,change_implementation)  \
				(somSelf,ev,obj,impl)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__change_implementation
				#if defined(_change_implementation)
					#undef _change_implementation
					#define SOMGD__change_implementation
				#else
					#define _change_implementation BOA_change_implementation
				#endif
			#endif /* SOMGD__change_implementation */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* BOA_change_implementation */
	#ifndef BOA_obj_is_ready
		#define BOA_obj_is_ready(somSelf,ev,obj,impl) \
			SOM_Resolve(somSelf,BOA,obj_is_ready)  \
				(somSelf,ev,obj,impl)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__obj_is_ready
				#if defined(_obj_is_ready)
					#undef _obj_is_ready
					#define SOMGD__obj_is_ready
				#else
					#define _obj_is_ready BOA_obj_is_ready
				#endif
			#endif /* SOMGD__obj_is_ready */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* BOA_obj_is_ready */
	#ifndef BOA_deactivate_obj
		#define BOA_deactivate_obj(somSelf,ev,obj) \
			SOM_Resolve(somSelf,BOA,deactivate_obj)  \
				(somSelf,ev,obj)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__deactivate_obj
				#if defined(_deactivate_obj)
					#undef _deactivate_obj
					#define SOMGD__deactivate_obj
				#else
					#define _deactivate_obj BOA_deactivate_obj
				#endif
			#endif /* SOMGD__deactivate_obj */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* BOA_deactivate_obj */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define BOA_somInit SOMObject_somInit
#define BOA_somUninit SOMObject_somUninit
#define BOA_somFree SOMObject_somFree
#define BOA_somGetClass SOMObject_somGetClass
#define BOA_somGetClassName SOMObject_somGetClassName
#define BOA_somGetSize SOMObject_somGetSize
#define BOA_somIsA SOMObject_somIsA
#define BOA_somIsInstanceOf SOMObject_somIsInstanceOf
#define BOA_somRespondsTo SOMObject_somRespondsTo
#define BOA_somDispatch SOMObject_somDispatch
#define BOA_somClassDispatch SOMObject_somClassDispatch
#define BOA_somCastObj SOMObject_somCastObj
#define BOA_somResetObj SOMObject_somResetObj
#define BOA_somPrintSelf SOMObject_somPrintSelf
#define BOA_somDumpSelf SOMObject_somDumpSelf
#define BOA_somDumpSelfInt SOMObject_somDumpSelfInt
#define BOA_somDefaultInit SOMObject_somDefaultInit
#define BOA_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define BOA_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define BOA_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define BOA_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define BOA_somDefaultAssign SOMObject_somDefaultAssign
#define BOA_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define BOA_somDefaultVAssign SOMObject_somDefaultVAssign
#define BOA_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define BOA_somDestruct SOMObject_somDestruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_boa_Header_h */
