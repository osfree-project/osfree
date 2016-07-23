/* generated from somoa.idl */
/* internal conditional is SOM_Module_somoa_Source */
#ifndef SOM_Module_somoa_Header_h
	#define SOM_Module_somoa_Header_h 1
	#include <som.h>
	#include <somdtype.h>
	#include <somobj.h>
	#include <containd.h>
	#include <unotypes.h>
	#include <boa.h>
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
	#ifndef _IDL_SOMOA_defined
		#define _IDL_SOMOA_defined
		typedef SOMObject SOMOA;
	#endif /* _IDL_SOMOA_defined */
	#ifndef _IDL_SEQUENCE_SOMOA_defined
		#define _IDL_SEQUENCE_SOMOA_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMOA SOMSTAR ,sequence(SOMOA));
	#endif /* _IDL_SEQUENCE_SOMOA_defined */
	#ifdef __IBMC__
		typedef ORBStatus (somTP_SOMOA_execute_next_request)(
			SOMOA SOMSTAR somSelf,
			Environment *ev,
			/* in */ Flags waitFlag);
		#pragma linkage(somTP_SOMOA_execute_next_request,system)
		typedef somTP_SOMOA_execute_next_request *somTD_SOMOA_execute_next_request;
	#else /* __IBMC__ */
		typedef ORBStatus (SOMLINK * somTD_SOMOA_execute_next_request)(
			SOMOA SOMSTAR somSelf,
			Environment *ev,
			/* in */ Flags waitFlag);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef ORBStatus (somTP_SOMOA_execute_request_loop)(
			SOMOA SOMSTAR somSelf,
			Environment *ev,
			/* in */ Flags waitFlag);
		#pragma linkage(somTP_SOMOA_execute_request_loop,system)
		typedef somTP_SOMOA_execute_request_loop *somTD_SOMOA_execute_request_loop;
	#else /* __IBMC__ */
		typedef ORBStatus (SOMLINK * somTD_SOMOA_execute_request_loop)(
			SOMOA SOMSTAR somSelf,
			Environment *ev,
			/* in */ Flags waitFlag);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMOA_change_id)(
			SOMOA SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMDObject SOMSTAR objref,
			/* in */ ReferenceData *id);
		#pragma linkage(somTP_SOMOA_change_id,system)
		typedef somTP_SOMOA_change_id *somTD_SOMOA_change_id;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMOA_change_id)(
			SOMOA SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMDObject SOMSTAR objref,
			/* in */ ReferenceData *id);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMDObject SOMSTAR (somTP_SOMOA_create_constant)(
			SOMOA SOMSTAR somSelf,
			Environment *ev,
			/* in */ ReferenceData *id,
			/* in */ InterfaceDef SOMSTAR intf,
			/* in */ ImplementationDef SOMSTAR impl);
		#pragma linkage(somTP_SOMOA_create_constant,system)
		typedef somTP_SOMOA_create_constant *somTD_SOMOA_create_constant;
	#else /* __IBMC__ */
		typedef SOMDObject SOMSTAR (SOMLINK * somTD_SOMOA_create_constant)(
			SOMOA SOMSTAR somSelf,
			Environment *ev,
			/* in */ ReferenceData *id,
			/* in */ InterfaceDef SOMSTAR intf,
			/* in */ ImplementationDef SOMSTAR impl);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMDObject SOMSTAR (somTP_SOMOA_create_SOM_ref)(
			SOMOA SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMObject SOMSTAR somobj,
			/* in */ ImplementationDef SOMSTAR impl);
		#pragma linkage(somTP_SOMOA_create_SOM_ref,system)
		typedef somTP_SOMOA_create_SOM_ref *somTD_SOMOA_create_SOM_ref;
	#else /* __IBMC__ */
		typedef SOMDObject SOMSTAR (SOMLINK * somTD_SOMOA_create_SOM_ref)(
			SOMOA SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMObject SOMSTAR somobj,
			/* in */ ImplementationDef SOMSTAR impl);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMObject SOMSTAR (somTP_SOMOA_get_SOM_object)(
			SOMOA SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMDObject SOMSTAR somref);
		#pragma linkage(somTP_SOMOA_get_SOM_object,system)
		typedef somTP_SOMOA_get_SOM_object *somTD_SOMOA_get_SOM_object;
	#else /* __IBMC__ */
		typedef SOMObject SOMSTAR (SOMLINK * somTD_SOMOA_get_SOM_object)(
			SOMOA SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMDObject SOMSTAR somref);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMOA_activate_impl_failed)(
			SOMOA SOMSTAR somSelf,
			Environment *ev,
			/* in */ ImplementationDef SOMSTAR impl,
			/* in */ ORBStatus rc);
		#pragma linkage(somTP_SOMOA_activate_impl_failed,system)
		typedef somTP_SOMOA_activate_impl_failed *somTD_SOMOA_activate_impl_failed;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMOA_activate_impl_failed)(
			SOMOA SOMSTAR somSelf,
			Environment *ev,
			/* in */ ImplementationDef SOMSTAR impl,
			/* in */ ORBStatus rc);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMOA_interrupt_server)(
			SOMOA SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_SOMOA_interrupt_server,system)
		typedef somTP_SOMOA_interrupt_server *somTD_SOMOA_interrupt_server;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMOA_interrupt_server)(
			SOMOA SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef IIOP_SOMDServiceContextList (somTP_SOMOA_get_service_contexts)(
			SOMOA SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMDObject SOMSTAR obj);
		#pragma linkage(somTP_SOMOA_get_service_contexts,system)
		typedef somTP_SOMOA_get_service_contexts *somTD_SOMOA_get_service_contexts;
	#else /* __IBMC__ */
		typedef IIOP_SOMDServiceContextList (SOMLINK * somTD_SOMOA_get_service_contexts)(
			SOMOA SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMDObject SOMSTAR obj);
	#endif /* __IBMC__ */
	#ifndef SOMOA_MajorVersion
		#define SOMOA_MajorVersion   2
	#endif /* SOMOA_MajorVersion */
	#ifndef SOMOA_MinorVersion
		#define SOMOA_MinorVersion   2
	#endif /* SOMOA_MinorVersion */
	typedef struct SOMOAClassDataStructure
	{
		SOMMSingleInstance SOMSTAR classObject;
		somMToken execute_next_request;
		somMToken execute_request_loop;
		somMToken create_constant;
		somMToken change_id;
		somMToken create_SOM_ref;
		somMToken get_SOM_object;
		somMToken activate_impl_failed;
		somMToken interrupt_server;
		somMToken private8;
		somMToken get_service_contexts;
		somMToken private10;
		somMToken private11;
		somMToken _get_default_SOMDServer;
		somMToken _set_default_SOMDServer;
		somMToken interrupt_server_status;
		somMToken notify_somdd;
	} SOMOAClassDataStructure;
	typedef struct SOMOACClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} SOMOACClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_somoa_Source) || defined(SOMOA_Class_Source)
			SOMEXTERN struct SOMOAClassDataStructure _SOMOAClassData;
			#ifndef SOMOAClassData
				#define SOMOAClassData    _SOMOAClassData
			#endif /* SOMOAClassData */
		#else
			SOMEXTERN struct SOMOAClassDataStructure * SOMLINK resolve_SOMOAClassData(void);
			#ifndef SOMOAClassData
				#define SOMOAClassData    (*(resolve_SOMOAClassData()))
			#endif /* SOMOAClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_somoa_Source) || defined(SOMOA_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_somoa_Source || SOMOA_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_somoa_Source || SOMOA_Class_Source */
		struct SOMOAClassDataStructure SOMDLINK SOMOAClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_somoa_Source) || defined(SOMOA_Class_Source)
			SOMEXTERN struct SOMOACClassDataStructure _SOMOACClassData;
			#ifndef SOMOACClassData
				#define SOMOACClassData    _SOMOACClassData
			#endif /* SOMOACClassData */
		#else
			SOMEXTERN struct SOMOACClassDataStructure * SOMLINK resolve_SOMOACClassData(void);
			#ifndef SOMOACClassData
				#define SOMOACClassData    (*(resolve_SOMOACClassData()))
			#endif /* SOMOACClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_somoa_Source) || defined(SOMOA_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_somoa_Source || SOMOA_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_somoa_Source || SOMOA_Class_Source */
		struct SOMOACClassDataStructure SOMDLINK SOMOACClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_somoa_Source) || defined(SOMOA_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_somoa_Source || SOMOA_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_somoa_Source || SOMOA_Class_Source */
	SOMMSingleInstance SOMSTAR SOMLINK SOMOANewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_SOMOA (SOMOAClassData.classObject)
	#ifndef SOMGD_SOMOA
		#if (defined(_SOMOA) || defined(__SOMOA))
			#undef _SOMOA
			#undef __SOMOA
			#define SOMGD_SOMOA 1
		#else
			#define _SOMOA _SOMCLASS_SOMOA
		#endif /* _SOMOA */
	#endif /* SOMGD_SOMOA */
	#define SOMOA_classObj _SOMCLASS_SOMOA
	#define _SOMMTOKEN_SOMOA(method) ((somMToken)(SOMOAClassData.method))
	#ifndef SOMOANew
		#define SOMOANew() ( _SOMOA ? \
			(SOMClass_somNew(_SOMOA)) : \
			( SOMOANewClass( \
				SOMOA_MajorVersion, \
				SOMOA_MinorVersion), \
			SOMClass_somNew(_SOMOA))) 
	#endif /* NewSOMOA */
	#ifndef SOMOA_execute_next_request
		#define SOMOA_execute_next_request(somSelf,ev,waitFlag) \
			SOM_Resolve(somSelf,SOMOA,execute_next_request)  \
				(somSelf,ev,waitFlag)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__execute_next_request
				#if defined(_execute_next_request)
					#undef _execute_next_request
					#define SOMGD__execute_next_request
				#else
					#define _execute_next_request SOMOA_execute_next_request
				#endif
			#endif /* SOMGD__execute_next_request */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMOA_execute_next_request */
	#ifndef SOMOA_execute_request_loop
		#define SOMOA_execute_request_loop(somSelf,ev,waitFlag) \
			SOM_Resolve(somSelf,SOMOA,execute_request_loop)  \
				(somSelf,ev,waitFlag)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__execute_request_loop
				#if defined(_execute_request_loop)
					#undef _execute_request_loop
					#define SOMGD__execute_request_loop
				#else
					#define _execute_request_loop SOMOA_execute_request_loop
				#endif
			#endif /* SOMGD__execute_request_loop */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMOA_execute_request_loop */
	#ifndef SOMOA_change_id
		#define SOMOA_change_id(somSelf,ev,objref,id) \
			SOM_Resolve(somSelf,SOMOA,change_id)  \
				(somSelf,ev,objref,id)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__change_id
				#if defined(_change_id)
					#undef _change_id
					#define SOMGD__change_id
				#else
					#define _change_id SOMOA_change_id
				#endif
			#endif /* SOMGD__change_id */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMOA_change_id */
	#ifndef SOMOA_create_constant
		#define SOMOA_create_constant(somSelf,ev,id,intf,impl) \
			SOM_Resolve(somSelf,SOMOA,create_constant)  \
				(somSelf,ev,id,intf,impl)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__create_constant
				#if defined(_create_constant)
					#undef _create_constant
					#define SOMGD__create_constant
				#else
					#define _create_constant SOMOA_create_constant
				#endif
			#endif /* SOMGD__create_constant */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMOA_create_constant */
	#ifndef SOMOA_create_SOM_ref
		#define SOMOA_create_SOM_ref(somSelf,ev,somobj,impl) \
			SOM_Resolve(somSelf,SOMOA,create_SOM_ref)  \
				(somSelf,ev,somobj,impl)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__create_SOM_ref
				#if defined(_create_SOM_ref)
					#undef _create_SOM_ref
					#define SOMGD__create_SOM_ref
				#else
					#define _create_SOM_ref SOMOA_create_SOM_ref
				#endif
			#endif /* SOMGD__create_SOM_ref */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMOA_create_SOM_ref */
	#ifndef SOMOA_get_SOM_object
		#define SOMOA_get_SOM_object(somSelf,ev,somref) \
			SOM_Resolve(somSelf,SOMOA,get_SOM_object)  \
				(somSelf,ev,somref)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__get_SOM_object
				#if defined(_get_SOM_object)
					#undef _get_SOM_object
					#define SOMGD__get_SOM_object
				#else
					#define _get_SOM_object SOMOA_get_SOM_object
				#endif
			#endif /* SOMGD__get_SOM_object */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMOA_get_SOM_object */
	#ifndef SOMOA_activate_impl_failed
		#define SOMOA_activate_impl_failed(somSelf,ev,impl,rc) \
			SOM_Resolve(somSelf,SOMOA,activate_impl_failed)  \
				(somSelf,ev,impl,rc)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__activate_impl_failed
				#if defined(_activate_impl_failed)
					#undef _activate_impl_failed
					#define SOMGD__activate_impl_failed
				#else
					#define _activate_impl_failed SOMOA_activate_impl_failed
				#endif
			#endif /* SOMGD__activate_impl_failed */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMOA_activate_impl_failed */
	#ifndef SOMOA_interrupt_server
		#define SOMOA_interrupt_server(somSelf,ev) \
			SOM_Resolve(somSelf,SOMOA,interrupt_server)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__interrupt_server
				#if defined(_interrupt_server)
					#undef _interrupt_server
					#define SOMGD__interrupt_server
				#else
					#define _interrupt_server SOMOA_interrupt_server
				#endif
			#endif /* SOMGD__interrupt_server */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMOA_interrupt_server */
	#ifndef SOMOA_get_service_contexts
		#define SOMOA_get_service_contexts(somSelf,ev,obj) \
			SOM_Resolve(somSelf,SOMOA,get_service_contexts)  \
				(somSelf,ev,obj)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__get_service_contexts
				#if defined(_get_service_contexts)
					#undef _get_service_contexts
					#define SOMGD__get_service_contexts
				#else
					#define _get_service_contexts SOMOA_get_service_contexts
				#endif
			#endif /* SOMGD__get_service_contexts */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMOA_get_service_contexts */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define SOMOA_somInit SOMObject_somInit
#define SOMOA_somUninit SOMObject_somUninit
#define SOMOA_somFree SOMObject_somFree
#define SOMOA_somGetClass SOMObject_somGetClass
#define SOMOA_somGetClassName SOMObject_somGetClassName
#define SOMOA_somGetSize SOMObject_somGetSize
#define SOMOA_somIsA SOMObject_somIsA
#define SOMOA_somIsInstanceOf SOMObject_somIsInstanceOf
#define SOMOA_somRespondsTo SOMObject_somRespondsTo
#define SOMOA_somDispatch SOMObject_somDispatch
#define SOMOA_somClassDispatch SOMObject_somClassDispatch
#define SOMOA_somCastObj SOMObject_somCastObj
#define SOMOA_somResetObj SOMObject_somResetObj
#define SOMOA_somPrintSelf SOMObject_somPrintSelf
#define SOMOA_somDumpSelf SOMObject_somDumpSelf
#define SOMOA_somDumpSelfInt SOMObject_somDumpSelfInt
#define SOMOA_somDefaultInit SOMObject_somDefaultInit
#define SOMOA_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define SOMOA_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define SOMOA_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define SOMOA_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define SOMOA_somDefaultAssign SOMObject_somDefaultAssign
#define SOMOA_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define SOMOA_somDefaultVAssign SOMObject_somDefaultVAssign
#define SOMOA_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define SOMOA_somDestruct SOMObject_somDestruct
#define SOMOA_create BOA_create
#define SOMOA_dispose BOA_dispose
#define SOMOA_get_id BOA_get_id
#define SOMOA_get_principal BOA_get_principal
#define SOMOA_set_exception BOA_set_exception
#define SOMOA_impl_is_ready BOA_impl_is_ready
#define SOMOA_deactivate_impl BOA_deactivate_impl
#define SOMOA_change_implementation BOA_change_implementation
#define SOMOA_obj_is_ready BOA_obj_is_ready
#define SOMOA_deactivate_obj BOA_deactivate_obj
#endif /* SOM_DONT_USE_INHERITED_MACROS */


#define SOMD_WAIT					1
#define SOMD_NO_WAIT				0
#define SOMD_IMPL_NOT_INITIALIZED	0
#define SOMD_IMPL_NOT_ACTIVE		1
#define SOMD_IMPL_ACTIVE			2


#endif /* SOM_Module_somoa_Header_h */
