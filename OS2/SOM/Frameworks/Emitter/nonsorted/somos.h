/* generated from somos.idl */
/* internal conditional is SOM_Module_somos_Source */
#ifndef SOM_Module_somos_Header_h
	#define SOM_Module_somos_Header_h 1
	#include <som.h>
	#include <somobj.h>
	#include <somdserv.h>
	#include <somdtype.h>
	#include <containd.h>
	#include <snglicls.h>
	#include <somcls.h>
	#include <xnaming.h>
	#include <naming.h>
	#include <somdcprx.h>
	#include <somproxy.h>
	#include <somdobj.h>
	#include <unotypes.h>
	#include <xmscssae.h>
	#include <omgidobj.h>
		#ifndef _IDL_somOS_Server_defined
			#define _IDL_somOS_Server_defined
			typedef SOMObject somOS_Server;
		#endif /* _IDL_somOS_Server_defined */
		#ifndef _IDL_SEQUENCE_somOS_Server_defined
			#define _IDL_SEQUENCE_somOS_Server_defined
			SOM_SEQUENCE_TYPEDEF_NAME(somOS_Server SOMSTAR ,sequence(somOS_Server));
		#endif /* _IDL_SEQUENCE_somOS_Server_defined */
		#ifndef _IDL_somOS_ServiceBase_defined
			#define _IDL_somOS_ServiceBase_defined
			typedef SOMObject somOS_ServiceBase;
		#endif /* _IDL_somOS_ServiceBase_defined */
		#ifndef _IDL_SEQUENCE_somOS_ServiceBase_defined
			#define _IDL_SEQUENCE_somOS_ServiceBase_defined
			SOM_SEQUENCE_TYPEDEF_NAME(somOS_ServiceBase SOMSTAR ,sequence(somOS_ServiceBase));
		#endif /* _IDL_SEQUENCE_somOS_ServiceBase_defined */
		#ifndef _IDL_somOS_ServiceBasePRef_defined
			#define _IDL_somOS_ServiceBasePRef_defined
			typedef SOMObject somOS_ServiceBasePRef;
		#endif /* _IDL_somOS_ServiceBasePRef_defined */
		#ifndef _IDL_SEQUENCE_somOS_ServiceBasePRef_defined
			#define _IDL_SEQUENCE_somOS_ServiceBasePRef_defined
			SOM_SEQUENCE_TYPEDEF_NAME(somOS_ServiceBasePRef SOMSTAR ,sequence(somOS_ServiceBasePRef));
		#endif /* _IDL_SEQUENCE_somOS_ServiceBasePRef_defined */
		#ifndef _IDL_somOS_ServiceBaseCORBA_defined
			#define _IDL_somOS_ServiceBaseCORBA_defined
			typedef SOMObject somOS_ServiceBaseCORBA;
		#endif /* _IDL_somOS_ServiceBaseCORBA_defined */
		#ifndef _IDL_SEQUENCE_somOS_ServiceBaseCORBA_defined
			#define _IDL_SEQUENCE_somOS_ServiceBaseCORBA_defined
			SOM_SEQUENCE_TYPEDEF_NAME(somOS_ServiceBaseCORBA SOMSTAR ,sequence(somOS_ServiceBaseCORBA));
		#endif /* _IDL_SEQUENCE_somOS_ServiceBaseCORBA_defined */
		#ifndef _IDL_somOS_ServiceBasePRefCORBA_defined
			#define _IDL_somOS_ServiceBasePRefCORBA_defined
			typedef SOMObject somOS_ServiceBasePRefCORBA;
		#endif /* _IDL_somOS_ServiceBasePRefCORBA_defined */
		#ifndef _IDL_SEQUENCE_somOS_ServiceBasePRefCORBA_defined
			#define _IDL_SEQUENCE_somOS_ServiceBasePRefCORBA_defined
			SOM_SEQUENCE_TYPEDEF_NAME(somOS_ServiceBasePRefCORBA SOMSTAR ,sequence(somOS_ServiceBasePRefCORBA));
		#endif /* _IDL_SEQUENCE_somOS_ServiceBasePRefCORBA_defined */
		#ifndef somOS_service_id_defined
			#define somOS_service_id_defined
			typedef unsigned long somOS_service_id;
			#ifndef somOS_somOSNaming
				#define somOS_somOSNaming 1UL
			#endif /* somOS_somOSNaming */
			#ifndef somOS_somOSEvents
				#define somOS_somOSEvents 2UL
			#endif /* somOS_somOSEvents */
			#ifndef somOS_somOSLifeCycle
				#define somOS_somOSLifeCycle 3UL
			#endif /* somOS_somOSLifeCycle */
			#ifndef somOS_somOSPersistence
				#define somOS_somOSPersistence 4UL
			#endif /* somOS_somOSPersistence */
			#ifndef somOS_somOSSecurity
				#define somOS_somOSSecurity 5UL
			#endif /* somOS_somOSSecurity */
			#ifndef somOS_somOSObjectIdentity
				#define somOS_somOSObjectIdentity 6UL
			#endif /* somOS_somOSObjectIdentity */
			#ifndef somOS_somOSTransactions
				#define somOS_somOSTransactions 7UL
			#endif /* somOS_somOSTransactions */
			#ifndef somOS_somOSConcurrency
				#define somOS_somOSConcurrency 8UL
			#endif /* somOS_somOSConcurrency */
			#ifndef somOS_somOSExternalization
				#define somOS_somOSExternalization 9UL
			#endif /* somOS_somOSExternalization */
			#ifndef somOS_somOSAttributePersistence
				#define somOS_somOSAttributePersistence 10UL
			#endif /* somOS_somOSAttributePersistence */
			#ifndef somOS_somOSLastEnum
				#define somOS_somOSLastEnum 11UL
			#endif /* somOS_somOSLastEnum */
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD_service_id
					#if defined(service_id)
						#undef service_id
						#define SOMGD_service_id
					#else
						#define service_id somOS_service_id
					#endif
				#endif /* SOMGD_service_id */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* somOS_service_id_defined */
		typedef somOS_service_id somOS_service_id_e;
		#ifndef _IDL_SEQUENCE_somOS_service_id_defined
			#define _IDL_SEQUENCE_somOS_service_id_defined
			SOM_SEQUENCE_TYPEDEF(somOS_service_id);
		#endif /* _IDL_SEQUENCE_somOS_service_id_defined */
		typedef struct somOS_metastate_struct
		{
			somOS_service_id_e svc_id;
			unsigned short version_major;
			unsigned short version_minor;
			any service_metastate;
		} somOS_metastate_struct;
		#ifndef _IDL_SEQUENCE_somOS_metastate_struct_defined
			#define _IDL_SEQUENCE_somOS_metastate_struct_defined
			SOM_SEQUENCE_TYPEDEF(somOS_metastate_struct);
		#endif /* _IDL_SEQUENCE_somOS_metastate_struct_defined */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD_metastate_struct
				#if defined(metastate_struct)
					#undef metastate_struct
					#define SOMGD_metastate_struct
				#else
					#define metastate_struct somOS_metastate_struct
				#endif
			#endif /* SOMGD_metastate_struct */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef _IDL_SEQUENCE_metastate_struct_defined
				#define _IDL_SEQUENCE_metastate_struct_defined
				#define _IDL_SEQUENCE_metastate_struct _IDL_SEQUENCE_somOS_metastate_struct
			#endif /* _IDL_SEQUENCE_metastate_struct_defined */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		typedef somOS_metastate_struct somOS_metastate_struct_t;
		#ifdef __IBMC__
			typedef SOMObject SOMSTAR (somTP_somOS_Server_make_persistent_ref)(
				somOS_Server SOMSTAR somSelf,
				Environment *ev,
				/* in */ SOMObject SOMSTAR referenced_object);
			#pragma linkage(somTP_somOS_Server_make_persistent_ref,system)
			typedef somTP_somOS_Server_make_persistent_ref *somTD_somOS_Server_make_persistent_ref;
		#else /* __IBMC__ */
			typedef SOMObject SOMSTAR (SOMLINK * somTD_somOS_Server_make_persistent_ref)(
				somOS_Server SOMSTAR somSelf,
				Environment *ev,
				/* in */ SOMObject SOMSTAR referenced_object);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_somOS_Server_delete_ref)(
				somOS_Server SOMSTAR somSelf,
				Environment *ev,
				/* in */ SOMObject SOMSTAR referenced_object);
			#pragma linkage(somTP_somOS_Server_delete_ref,system)
			typedef somTP_somOS_Server_delete_ref *somTD_somOS_Server_delete_ref;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_somOS_Server_delete_ref)(
				somOS_Server SOMSTAR somSelf,
				Environment *ev,
				/* in */ SOMObject SOMSTAR referenced_object);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef boolean (somTP_somOS_Server_has_persistent_ref)(
				somOS_Server SOMSTAR somSelf,
				Environment *ev,
				/* in */ SOMObject SOMSTAR referenced_object);
			#pragma linkage(somTP_somOS_Server_has_persistent_ref,system)
			typedef somTP_somOS_Server_has_persistent_ref *somTD_somOS_Server_has_persistent_ref;
		#else /* __IBMC__ */
			typedef boolean (SOMLINK * somTD_somOS_Server_has_persistent_ref)(
				somOS_Server SOMSTAR somSelf,
				Environment *ev,
				/* in */ SOMObject SOMSTAR referenced_object);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_somOS_Server_passivate_object)(
				somOS_Server SOMSTAR somSelf,
				Environment *ev,
				/* in */ SOMObject SOMSTAR referenced_object);
			#pragma linkage(somTP_somOS_Server_passivate_object,system)
			typedef somTP_somOS_Server_passivate_object *somTD_somOS_Server_passivate_object;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_somOS_Server_passivate_object)(
				somOS_Server SOMSTAR somSelf,
				Environment *ev,
				/* in */ SOMObject SOMSTAR referenced_object);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_somOS_Server_passivate_all_objects)(
				somOS_Server SOMSTAR somSelf,
				Environment *ev);
			#pragma linkage(somTP_somOS_Server_passivate_all_objects,system)
			typedef somTP_somOS_Server_passivate_all_objects *somTD_somOS_Server_passivate_all_objects;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_somOS_Server_passivate_all_objects)(
				somOS_Server SOMSTAR somSelf,
				Environment *ev);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_somOS_Server_store_service_metastate)(
				somOS_Server SOMSTAR somSelf,
				Environment *ev,
				/* in */ SOMObject SOMSTAR referenced_object,
				/* in */ somOS_service_id_e somos_service_id,
				/* in */ any *service_metadata);
			#pragma linkage(somTP_somOS_Server_store_service_metastate,system)
			typedef somTP_somOS_Server_store_service_metastate *somTD_somOS_Server_store_service_metastate;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_somOS_Server_store_service_metastate)(
				somOS_Server SOMSTAR somSelf,
				Environment *ev,
				/* in */ SOMObject SOMSTAR referenced_object,
				/* in */ somOS_service_id_e somos_service_id,
				/* in */ any *service_metadata);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_somOS_Server_store_metastate)(
				somOS_Server SOMSTAR somSelf,
				Environment *ev,
				/* in */ SOMObject SOMSTAR referenced_object);
			#pragma linkage(somTP_somOS_Server_store_metastate,system)
			typedef somTP_somOS_Server_store_metastate *somTD_somOS_Server_store_metastate;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_somOS_Server_store_metastate)(
				somOS_Server SOMSTAR somSelf,
				Environment *ev,
				/* in */ SOMObject SOMSTAR referenced_object);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_somOS_Server_restore_metastate)(
				somOS_Server SOMSTAR somSelf,
				Environment *ev,
				/* in */ SOMObject SOMSTAR referenced_object);
			#pragma linkage(somTP_somOS_Server_restore_metastate,system)
			typedef somTP_somOS_Server_restore_metastate *somTD_somOS_Server_restore_metastate;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_somOS_Server_restore_metastate)(
				somOS_Server SOMSTAR somSelf,
				Environment *ev,
				/* in */ SOMObject SOMSTAR referenced_object);
		#endif /* __IBMC__ */
		#ifndef somOS_Server_MajorVersion
			#define somOS_Server_MajorVersion   3
		#endif /* somOS_Server_MajorVersion */
		#ifndef somOS_Server_MinorVersion
			#define somOS_Server_MinorVersion   0
		#endif /* somOS_Server_MinorVersion */
		typedef struct somOS_ServerClassDataStructure
		{
			SOMMSingleInstance SOMSTAR classObject;
			somMToken make_persistent_ref;
			somMToken delete_ref;
			somMToken passivate_object;
			somMToken passivate_all_objects;
			somMToken store_metastate;
			somMToken restore_metastate;
			somMToken has_persistent_ref;
			somMToken store_service_metastate;
		} somOS_ServerClassDataStructure;
		typedef struct somOS_ServerCClassDataStructure
		{
			somMethodTabs parentMtab;
			somDToken instanceDataToken;
		} somOS_ServerCClassDataStructure;
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_somos_Source) || defined(somOS_Server_Class_Source)
				SOMEXTERN struct somOS_ServerClassDataStructure _somOS_ServerClassData;
				#ifndef somOS_ServerClassData
					#define somOS_ServerClassData    _somOS_ServerClassData
				#endif /* somOS_ServerClassData */
			#else
				SOMEXTERN struct somOS_ServerClassDataStructure * SOMLINK resolve_somOS_ServerClassData(void);
				#ifndef somOS_ServerClassData
					#define somOS_ServerClassData    (*(resolve_somOS_ServerClassData()))
				#endif /* somOS_ServerClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_somos_Source) || defined(somOS_Server_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_somos_Source || somOS_Server_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_somos_Source || somOS_Server_Class_Source */
			struct somOS_ServerClassDataStructure SOMDLINK somOS_ServerClassData;
		#endif /* SOM_RESOLVE_DATA */
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_somos_Source) || defined(somOS_Server_Class_Source)
				SOMEXTERN struct somOS_ServerCClassDataStructure _somOS_ServerCClassData;
				#ifndef somOS_ServerCClassData
					#define somOS_ServerCClassData    _somOS_ServerCClassData
				#endif /* somOS_ServerCClassData */
			#else
				SOMEXTERN struct somOS_ServerCClassDataStructure * SOMLINK resolve_somOS_ServerCClassData(void);
				#ifndef somOS_ServerCClassData
					#define somOS_ServerCClassData    (*(resolve_somOS_ServerCClassData()))
				#endif /* somOS_ServerCClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_somos_Source) || defined(somOS_Server_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_somos_Source || somOS_Server_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_somos_Source || somOS_Server_Class_Source */
			struct somOS_ServerCClassDataStructure SOMDLINK somOS_ServerCClassData;
		#endif /* SOM_RESOLVE_DATA */
		extern 
		#if defined(SOM_Module_somos_Source) || defined(somOS_Server_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_somos_Source || somOS_Server_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_somos_Source || somOS_Server_Class_Source */
		SOMMSingleInstance SOMSTAR SOMLINK somOS_ServerNewClass
				(integer4 somtmajorVersion,integer4 somtminorVersion);
		#define _SOMCLASS_somOS_Server (somOS_ServerClassData.classObject)
		#ifndef SOMGD_somOS_Server
			#if (defined(_somOS_Server) || defined(__somOS_Server))
				#undef _somOS_Server
				#undef __somOS_Server
				#define SOMGD_somOS_Server 1
			#else
				#define _somOS_Server _SOMCLASS_somOS_Server
			#endif /* _somOS_Server */
		#endif /* SOMGD_somOS_Server */
		#define somOS_Server_classObj _SOMCLASS_somOS_Server
		#define _SOMMTOKEN_somOS_Server(method) ((somMToken)(somOS_ServerClassData.method))
		#ifndef somOS_ServerNew
			#define somOS_ServerNew() ( _somOS_Server ? \
				(SOMClass_somNew(_somOS_Server)) : \
				( somOS_ServerNewClass( \
					somOS_Server_MajorVersion, \
					somOS_Server_MinorVersion), \
				SOMClass_somNew(_somOS_Server))) 
		#endif /* NewsomOS_Server */
		#ifndef somOS_Server_make_persistent_ref
			#define somOS_Server_make_persistent_ref(somSelf,ev,referenced_object) \
				SOM_Resolve(somSelf,somOS_Server,make_persistent_ref)  \
					(somSelf,ev,referenced_object)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__make_persistent_ref
					#if defined(_make_persistent_ref)
						#undef _make_persistent_ref
						#define SOMGD__make_persistent_ref
					#else
						#define _make_persistent_ref somOS_Server_make_persistent_ref
					#endif
				#endif /* SOMGD__make_persistent_ref */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* somOS_Server_make_persistent_ref */
		#ifndef somOS_Server_delete_ref
			#define somOS_Server_delete_ref(somSelf,ev,referenced_object) \
				SOM_Resolve(somSelf,somOS_Server,delete_ref)  \
					(somSelf,ev,referenced_object)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__delete_ref
					#if defined(_delete_ref)
						#undef _delete_ref
						#define SOMGD__delete_ref
					#else
						#define _delete_ref somOS_Server_delete_ref
					#endif
				#endif /* SOMGD__delete_ref */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* somOS_Server_delete_ref */
		#ifndef somOS_Server_has_persistent_ref
			#define somOS_Server_has_persistent_ref(somSelf,ev,referenced_object) \
				SOM_Resolve(somSelf,somOS_Server,has_persistent_ref)  \
					(somSelf,ev,referenced_object)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__has_persistent_ref
					#if defined(_has_persistent_ref)
						#undef _has_persistent_ref
						#define SOMGD__has_persistent_ref
					#else
						#define _has_persistent_ref somOS_Server_has_persistent_ref
					#endif
				#endif /* SOMGD__has_persistent_ref */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* somOS_Server_has_persistent_ref */
		#ifndef somOS_Server_passivate_object
			#define somOS_Server_passivate_object(somSelf,ev,referenced_object) \
				SOM_Resolve(somSelf,somOS_Server,passivate_object)  \
					(somSelf,ev,referenced_object)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__passivate_object
					#if defined(_passivate_object)
						#undef _passivate_object
						#define SOMGD__passivate_object
					#else
						#define _passivate_object somOS_Server_passivate_object
					#endif
				#endif /* SOMGD__passivate_object */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* somOS_Server_passivate_object */
		#ifndef somOS_Server_passivate_all_objects
			#define somOS_Server_passivate_all_objects(somSelf,ev) \
				SOM_Resolve(somSelf,somOS_Server,passivate_all_objects)  \
					(somSelf,ev)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__passivate_all_objects
					#if defined(_passivate_all_objects)
						#undef _passivate_all_objects
						#define SOMGD__passivate_all_objects
					#else
						#define _passivate_all_objects somOS_Server_passivate_all_objects
					#endif
				#endif /* SOMGD__passivate_all_objects */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* somOS_Server_passivate_all_objects */
		#ifndef somOS_Server_store_service_metastate
			#define somOS_Server_store_service_metastate(somSelf,ev,referenced_object,somos_service_id,service_metadata) \
				SOM_Resolve(somSelf,somOS_Server,store_service_metastate)  \
					(somSelf,ev,referenced_object,somos_service_id,service_metadata)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__store_service_metastate
					#if defined(_store_service_metastate)
						#undef _store_service_metastate
						#define SOMGD__store_service_metastate
					#else
						#define _store_service_metastate somOS_Server_store_service_metastate
					#endif
				#endif /* SOMGD__store_service_metastate */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* somOS_Server_store_service_metastate */
		#ifndef somOS_Server_store_metastate
			#define somOS_Server_store_metastate(somSelf,ev,referenced_object) \
				SOM_Resolve(somSelf,somOS_Server,store_metastate)  \
					(somSelf,ev,referenced_object)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__store_metastate
					#if defined(_store_metastate)
						#undef _store_metastate
						#define SOMGD__store_metastate
					#else
						#define _store_metastate somOS_Server_store_metastate
					#endif
				#endif /* SOMGD__store_metastate */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* somOS_Server_store_metastate */
		#ifndef somOS_Server_restore_metastate
			#define somOS_Server_restore_metastate(somSelf,ev,referenced_object) \
				SOM_Resolve(somSelf,somOS_Server,restore_metastate)  \
					(somSelf,ev,referenced_object)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__restore_metastate
					#if defined(_restore_metastate)
						#undef _restore_metastate
						#define SOMGD__restore_metastate
					#else
						#define _restore_metastate somOS_Server_restore_metastate
					#endif
				#endif /* SOMGD__restore_metastate */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* somOS_Server_restore_metastate */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define somOS_Server_somInit SOMObject_somInit
#define somOS_Server_somUninit SOMObject_somUninit
#define somOS_Server_somFree SOMObject_somFree
#define somOS_Server_somGetClass SOMObject_somGetClass
#define somOS_Server_somGetClassName SOMObject_somGetClassName
#define somOS_Server_somGetSize SOMObject_somGetSize
#define somOS_Server_somIsA SOMObject_somIsA
#define somOS_Server_somIsInstanceOf SOMObject_somIsInstanceOf
#define somOS_Server_somRespondsTo SOMObject_somRespondsTo
#define somOS_Server_somDispatch SOMObject_somDispatch
#define somOS_Server_somClassDispatch SOMObject_somClassDispatch
#define somOS_Server_somCastObj SOMObject_somCastObj
#define somOS_Server_somResetObj SOMObject_somResetObj
#define somOS_Server_somPrintSelf SOMObject_somPrintSelf
#define somOS_Server_somDumpSelf SOMObject_somDumpSelf
#define somOS_Server_somDumpSelfInt SOMObject_somDumpSelfInt
#define somOS_Server_somDefaultInit SOMObject_somDefaultInit
#define somOS_Server_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define somOS_Server_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define somOS_Server_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define somOS_Server_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define somOS_Server_somDefaultAssign SOMObject_somDefaultAssign
#define somOS_Server_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define somOS_Server_somDefaultVAssign SOMObject_somDefaultVAssign
#define somOS_Server_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define somOS_Server_somDestruct SOMObject_somDestruct
#define somOS_Server_somdRefFromSOMObj SOMDServer_somdRefFromSOMObj
#define somOS_Server_somdSOMObjFromRef SOMDServer_somdSOMObjFromRef
#define somOS_Server_somdDispatchMethod SOMDServer_somdDispatchMethod
#define somOS_Server_somdCreateObj SOMDServer_somdCreateObj
#define somOS_Server_somdDeleteObj SOMDServer_somdDeleteObj
#define somOS_Server_somdGetClassObj SOMDServer_somdGetClassObj
#define somOS_Server_somdObjReferencesCached SOMDServer_somdObjReferencesCached
#define somOS_Server_somdCreateFactory SOMDServer_somdCreateFactory
#define somOS_Server__get_ImplDefObject SOMDServer__get_ImplDefObject
#endif /* SOM_DONT_USE_INHERITED_MACROS */

		typedef _IDL_SEQUENCE_somOS_metastate_struct somOS_ServiceBase_metastate_t;
		#ifndef _IDL_SEQUENCE_somOS_ServiceBase_metastate_t_defined
			#define _IDL_SEQUENCE_somOS_ServiceBase_metastate_t_defined
			SOM_SEQUENCE_TYPEDEF(somOS_ServiceBase_metastate_t);
		#endif /* _IDL_SEQUENCE_somOS_ServiceBase_metastate_t_defined */
		#ifdef __IBMC__
			typedef void (somTP_somOS_ServiceBase_reinit)(
				somOS_ServiceBase SOMSTAR somSelf,
				Environment *ev,
				/* in */ somOS_ServiceBase_metastate_t *meta_data);
			#pragma linkage(somTP_somOS_ServiceBase_reinit,system)
			typedef somTP_somOS_ServiceBase_reinit *somTD_somOS_ServiceBase_reinit;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_somOS_ServiceBase_reinit)(
				somOS_ServiceBase SOMSTAR somSelf,
				Environment *ev,
				/* in */ somOS_ServiceBase_metastate_t *meta_data);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_somOS_ServiceBase_capture)(
				somOS_ServiceBase SOMSTAR somSelf,
				Environment *ev,
				/* inout */ somOS_ServiceBase_metastate_t *meta_data);
			#pragma linkage(somTP_somOS_ServiceBase_capture,system)
			typedef somTP_somOS_ServiceBase_capture *somTD_somOS_ServiceBase_capture;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_somOS_ServiceBase_capture)(
				somOS_ServiceBase SOMSTAR somSelf,
				Environment *ev,
				/* inout */ somOS_ServiceBase_metastate_t *meta_data);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef SOMObject SOMSTAR (somTP_somOS_ServiceBase_GetInstanceManager)(
				somOS_ServiceBase SOMSTAR somSelf,
				Environment *ev);
			#pragma linkage(somTP_somOS_ServiceBase_GetInstanceManager,system)
			typedef somTP_somOS_ServiceBase_GetInstanceManager *somTD_somOS_ServiceBase_GetInstanceManager;
		#else /* __IBMC__ */
			typedef SOMObject SOMSTAR (SOMLINK * somTD_somOS_ServiceBase_GetInstanceManager)(
				somOS_ServiceBase SOMSTAR somSelf,
				Environment *ev);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef SOMObject SOMSTAR (somTP_somOS_ServiceBase_init_for_object_creation)(
				somOS_ServiceBase SOMSTAR somSelf,
				Environment *ev);
			#pragma linkage(somTP_somOS_ServiceBase_init_for_object_creation,system)
			typedef somTP_somOS_ServiceBase_init_for_object_creation *somTD_somOS_ServiceBase_init_for_object_creation;
		#else /* __IBMC__ */
			typedef SOMObject SOMSTAR (SOMLINK * somTD_somOS_ServiceBase_init_for_object_creation)(
				somOS_ServiceBase SOMSTAR somSelf,
				Environment *ev);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef SOMObject SOMSTAR (somTP_somOS_ServiceBase_init_for_object_reactivation)(
				somOS_ServiceBase SOMSTAR somSelf,
				Environment *ev);
			#pragma linkage(somTP_somOS_ServiceBase_init_for_object_reactivation,system)
			typedef somTP_somOS_ServiceBase_init_for_object_reactivation *somTD_somOS_ServiceBase_init_for_object_reactivation;
		#else /* __IBMC__ */
			typedef SOMObject SOMSTAR (SOMLINK * somTD_somOS_ServiceBase_init_for_object_reactivation)(
				somOS_ServiceBase SOMSTAR somSelf,
				Environment *ev);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef SOMObject SOMSTAR (somTP_somOS_ServiceBase_init_for_object_copy)(
				somOS_ServiceBase SOMSTAR somSelf,
				Environment *ev);
			#pragma linkage(somTP_somOS_ServiceBase_init_for_object_copy,system)
			typedef somTP_somOS_ServiceBase_init_for_object_copy *somTD_somOS_ServiceBase_init_for_object_copy;
		#else /* __IBMC__ */
			typedef SOMObject SOMSTAR (SOMLINK * somTD_somOS_ServiceBase_init_for_object_copy)(
				somOS_ServiceBase SOMSTAR somSelf,
				Environment *ev);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_somOS_ServiceBase_uninit_for_object_move)(
				somOS_ServiceBase SOMSTAR somSelf,
				Environment *ev);
			#pragma linkage(somTP_somOS_ServiceBase_uninit_for_object_move,system)
			typedef somTP_somOS_ServiceBase_uninit_for_object_move *somTD_somOS_ServiceBase_uninit_for_object_move;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_somOS_ServiceBase_uninit_for_object_move)(
				somOS_ServiceBase SOMSTAR somSelf,
				Environment *ev);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_somOS_ServiceBase_uninit_for_object_passivation)(
				somOS_ServiceBase SOMSTAR somSelf,
				Environment *ev);
			#pragma linkage(somTP_somOS_ServiceBase_uninit_for_object_passivation,system)
			typedef somTP_somOS_ServiceBase_uninit_for_object_passivation *somTD_somOS_ServiceBase_uninit_for_object_passivation;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_somOS_ServiceBase_uninit_for_object_passivation)(
				somOS_ServiceBase SOMSTAR somSelf,
				Environment *ev);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_somOS_ServiceBase_uninit_for_object_destruction)(
				somOS_ServiceBase SOMSTAR somSelf,
				Environment *ev);
			#pragma linkage(somTP_somOS_ServiceBase_uninit_for_object_destruction,system)
			typedef somTP_somOS_ServiceBase_uninit_for_object_destruction *somTD_somOS_ServiceBase_uninit_for_object_destruction;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_somOS_ServiceBase_uninit_for_object_destruction)(
				somOS_ServiceBase SOMSTAR somSelf,
				Environment *ev);
		#endif /* __IBMC__ */
		#ifndef somOS_ServiceBase_MajorVersion
			#define somOS_ServiceBase_MajorVersion   3
		#endif /* somOS_ServiceBase_MajorVersion */
		#ifndef somOS_ServiceBase_MinorVersion
			#define somOS_ServiceBase_MinorVersion   0
		#endif /* somOS_ServiceBase_MinorVersion */
		typedef struct somOS_ServiceBaseClassDataStructure
		{
			SOMClass SOMSTAR classObject;
			somMToken reinit;
			somMToken capture;
			somMToken init_for_object_creation;
			somMToken init_for_object_reactivation;
			somMToken init_for_object_copy;
			somMToken uninit_for_object_move;
			somMToken uninit_for_object_passivation;
			somMToken uninit_for_object_destruction;
			somMToken _get_som_uuid;
			somMToken GetInstanceManager;
		} somOS_ServiceBaseClassDataStructure;
		typedef struct somOS_ServiceBaseCClassDataStructure
		{
			somMethodTabs parentMtab;
			somDToken instanceDataToken;
		} somOS_ServiceBaseCClassDataStructure;
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_somos_Source) || defined(somOS_ServiceBase_Class_Source)
				SOMEXTERN struct somOS_ServiceBaseClassDataStructure _somOS_ServiceBaseClassData;
				#ifndef somOS_ServiceBaseClassData
					#define somOS_ServiceBaseClassData    _somOS_ServiceBaseClassData
				#endif /* somOS_ServiceBaseClassData */
			#else
				SOMEXTERN struct somOS_ServiceBaseClassDataStructure * SOMLINK resolve_somOS_ServiceBaseClassData(void);
				#ifndef somOS_ServiceBaseClassData
					#define somOS_ServiceBaseClassData    (*(resolve_somOS_ServiceBaseClassData()))
				#endif /* somOS_ServiceBaseClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_somos_Source) || defined(somOS_ServiceBase_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_somos_Source || somOS_ServiceBase_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_somos_Source || somOS_ServiceBase_Class_Source */
			struct somOS_ServiceBaseClassDataStructure SOMDLINK somOS_ServiceBaseClassData;
		#endif /* SOM_RESOLVE_DATA */
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_somos_Source) || defined(somOS_ServiceBase_Class_Source)
				SOMEXTERN struct somOS_ServiceBaseCClassDataStructure _somOS_ServiceBaseCClassData;
				#ifndef somOS_ServiceBaseCClassData
					#define somOS_ServiceBaseCClassData    _somOS_ServiceBaseCClassData
				#endif /* somOS_ServiceBaseCClassData */
			#else
				SOMEXTERN struct somOS_ServiceBaseCClassDataStructure * SOMLINK resolve_somOS_ServiceBaseCClassData(void);
				#ifndef somOS_ServiceBaseCClassData
					#define somOS_ServiceBaseCClassData    (*(resolve_somOS_ServiceBaseCClassData()))
				#endif /* somOS_ServiceBaseCClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_somos_Source) || defined(somOS_ServiceBase_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_somos_Source || somOS_ServiceBase_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_somos_Source || somOS_ServiceBase_Class_Source */
			struct somOS_ServiceBaseCClassDataStructure SOMDLINK somOS_ServiceBaseCClassData;
		#endif /* SOM_RESOLVE_DATA */
		extern 
		#if defined(SOM_Module_somos_Source) || defined(somOS_ServiceBase_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_somos_Source || somOS_ServiceBase_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_somos_Source || somOS_ServiceBase_Class_Source */
		SOMClass SOMSTAR SOMLINK somOS_ServiceBaseNewClass
				(integer4 somtmajorVersion,integer4 somtminorVersion);
		#define _SOMCLASS_somOS_ServiceBase (somOS_ServiceBaseClassData.classObject)
		#ifndef SOMGD_somOS_ServiceBase
			#if (defined(_somOS_ServiceBase) || defined(__somOS_ServiceBase))
				#undef _somOS_ServiceBase
				#undef __somOS_ServiceBase
				#define SOMGD_somOS_ServiceBase 1
			#else
				#define _somOS_ServiceBase _SOMCLASS_somOS_ServiceBase
			#endif /* _somOS_ServiceBase */
		#endif /* SOMGD_somOS_ServiceBase */
		#define somOS_ServiceBase_classObj _SOMCLASS_somOS_ServiceBase
		#define _SOMMTOKEN_somOS_ServiceBase(method) ((somMToken)(somOS_ServiceBaseClassData.method))
		#ifndef somOS_ServiceBaseNew
			#define somOS_ServiceBaseNew() ( _somOS_ServiceBase ? \
				(SOMClass_somNew(_somOS_ServiceBase)) : \
				( somOS_ServiceBaseNewClass( \
					somOS_ServiceBase_MajorVersion, \
					somOS_ServiceBase_MinorVersion), \
				SOMClass_somNew(_somOS_ServiceBase))) 
		#endif /* NewsomOS_ServiceBase */
		#ifndef somOS_ServiceBase_reinit
			#define somOS_ServiceBase_reinit(somSelf,ev,meta_data) \
				SOM_Resolve(somSelf,somOS_ServiceBase,reinit)  \
					(somSelf,ev,meta_data)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__reinit
					#if defined(_reinit)
						#undef _reinit
						#define SOMGD__reinit
					#else
						#define _reinit somOS_ServiceBase_reinit
					#endif
				#endif /* SOMGD__reinit */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* somOS_ServiceBase_reinit */
		#ifndef somOS_ServiceBase_capture
			#define somOS_ServiceBase_capture(somSelf,ev,meta_data) \
				SOM_Resolve(somSelf,somOS_ServiceBase,capture)  \
					(somSelf,ev,meta_data)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__capture
					#if defined(_capture)
						#undef _capture
						#define SOMGD__capture
					#else
						#define _capture somOS_ServiceBase_capture
					#endif
				#endif /* SOMGD__capture */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* somOS_ServiceBase_capture */
		#ifndef somOS_ServiceBase_GetInstanceManager
			#define somOS_ServiceBase_GetInstanceManager(somSelf,ev) \
				SOM_Resolve(somSelf,somOS_ServiceBase,GetInstanceManager)  \
					(somSelf,ev)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__GetInstanceManager
					#if defined(_GetInstanceManager)
						#undef _GetInstanceManager
						#define SOMGD__GetInstanceManager
					#else
						#define _GetInstanceManager somOS_ServiceBase_GetInstanceManager
					#endif
				#endif /* SOMGD__GetInstanceManager */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* somOS_ServiceBase_GetInstanceManager */
		#ifndef somOS_ServiceBase_init_for_object_creation
			#define somOS_ServiceBase_init_for_object_creation(somSelf,ev) \
				SOM_Resolve(somSelf,somOS_ServiceBase,init_for_object_creation)  \
					(somSelf,ev)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__init_for_object_creation
					#if defined(_init_for_object_creation)
						#undef _init_for_object_creation
						#define SOMGD__init_for_object_creation
					#else
						#define _init_for_object_creation somOS_ServiceBase_init_for_object_creation
					#endif
				#endif /* SOMGD__init_for_object_creation */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* somOS_ServiceBase_init_for_object_creation */
		#ifndef somOS_ServiceBase_init_for_object_reactivation
			#define somOS_ServiceBase_init_for_object_reactivation(somSelf,ev) \
				SOM_Resolve(somSelf,somOS_ServiceBase,init_for_object_reactivation)  \
					(somSelf,ev)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__init_for_object_reactivation
					#if defined(_init_for_object_reactivation)
						#undef _init_for_object_reactivation
						#define SOMGD__init_for_object_reactivation
					#else
						#define _init_for_object_reactivation somOS_ServiceBase_init_for_object_reactivation
					#endif
				#endif /* SOMGD__init_for_object_reactivation */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* somOS_ServiceBase_init_for_object_reactivation */
		#ifndef somOS_ServiceBase_init_for_object_copy
			#define somOS_ServiceBase_init_for_object_copy(somSelf,ev) \
				SOM_Resolve(somSelf,somOS_ServiceBase,init_for_object_copy)  \
					(somSelf,ev)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__init_for_object_copy
					#if defined(_init_for_object_copy)
						#undef _init_for_object_copy
						#define SOMGD__init_for_object_copy
					#else
						#define _init_for_object_copy somOS_ServiceBase_init_for_object_copy
					#endif
				#endif /* SOMGD__init_for_object_copy */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* somOS_ServiceBase_init_for_object_copy */
		#ifndef somOS_ServiceBase_uninit_for_object_move
			#define somOS_ServiceBase_uninit_for_object_move(somSelf,ev) \
				SOM_Resolve(somSelf,somOS_ServiceBase,uninit_for_object_move)  \
					(somSelf,ev)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__uninit_for_object_move
					#if defined(_uninit_for_object_move)
						#undef _uninit_for_object_move
						#define SOMGD__uninit_for_object_move
					#else
						#define _uninit_for_object_move somOS_ServiceBase_uninit_for_object_move
					#endif
				#endif /* SOMGD__uninit_for_object_move */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* somOS_ServiceBase_uninit_for_object_move */
		#ifndef somOS_ServiceBase_uninit_for_object_passivation
			#define somOS_ServiceBase_uninit_for_object_passivation(somSelf,ev) \
				SOM_Resolve(somSelf,somOS_ServiceBase,uninit_for_object_passivation)  \
					(somSelf,ev)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__uninit_for_object_passivation
					#if defined(_uninit_for_object_passivation)
						#undef _uninit_for_object_passivation
						#define SOMGD__uninit_for_object_passivation
					#else
						#define _uninit_for_object_passivation somOS_ServiceBase_uninit_for_object_passivation
					#endif
				#endif /* SOMGD__uninit_for_object_passivation */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* somOS_ServiceBase_uninit_for_object_passivation */
		#ifndef somOS_ServiceBase_uninit_for_object_destruction
			#define somOS_ServiceBase_uninit_for_object_destruction(somSelf,ev) \
				SOM_Resolve(somSelf,somOS_ServiceBase,uninit_for_object_destruction)  \
					(somSelf,ev)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__uninit_for_object_destruction
					#if defined(_uninit_for_object_destruction)
						#undef _uninit_for_object_destruction
						#define SOMGD__uninit_for_object_destruction
					#else
						#define _uninit_for_object_destruction somOS_ServiceBase_uninit_for_object_destruction
					#endif
				#endif /* SOMGD__uninit_for_object_destruction */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* somOS_ServiceBase_uninit_for_object_destruction */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define somOS_ServiceBase_somInit SOMObject_somInit
#define somOS_ServiceBase_somUninit SOMObject_somUninit
#define somOS_ServiceBase_somFree SOMObject_somFree
#define somOS_ServiceBase_somGetClass SOMObject_somGetClass
#define somOS_ServiceBase_somGetClassName SOMObject_somGetClassName
#define somOS_ServiceBase_somGetSize SOMObject_somGetSize
#define somOS_ServiceBase_somIsA SOMObject_somIsA
#define somOS_ServiceBase_somIsInstanceOf SOMObject_somIsInstanceOf
#define somOS_ServiceBase_somRespondsTo SOMObject_somRespondsTo
#define somOS_ServiceBase_somDispatch SOMObject_somDispatch
#define somOS_ServiceBase_somClassDispatch SOMObject_somClassDispatch
#define somOS_ServiceBase_somCastObj SOMObject_somCastObj
#define somOS_ServiceBase_somResetObj SOMObject_somResetObj
#define somOS_ServiceBase_somPrintSelf SOMObject_somPrintSelf
#define somOS_ServiceBase_somDumpSelf SOMObject_somDumpSelf
#define somOS_ServiceBase_somDumpSelfInt SOMObject_somDumpSelfInt
#define somOS_ServiceBase_somDefaultInit SOMObject_somDefaultInit
#define somOS_ServiceBase_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define somOS_ServiceBase_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define somOS_ServiceBase_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define somOS_ServiceBase_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define somOS_ServiceBase_somDefaultAssign SOMObject_somDefaultAssign
#define somOS_ServiceBase_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define somOS_ServiceBase_somDefaultVAssign SOMObject_somDefaultVAssign
#define somOS_ServiceBase_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define somOS_ServiceBase_somDestruct SOMObject_somDestruct
#define somOS_ServiceBase__get_constant_random_id CosObjectIdentity_IdentifiableObject__get_constant_random_id
#define somOS_ServiceBase_is_identical CosObjectIdentity_IdentifiableObject_is_identical
#endif /* SOM_DONT_USE_INHERITED_MACROS */

		#ifndef somOS_ServiceBasePRef_MajorVersion
			#define somOS_ServiceBasePRef_MajorVersion   3
		#endif /* somOS_ServiceBasePRef_MajorVersion */
		#ifndef somOS_ServiceBasePRef_MinorVersion
			#define somOS_ServiceBasePRef_MinorVersion   0
		#endif /* somOS_ServiceBasePRef_MinorVersion */
		typedef struct somOS_ServiceBasePRefClassDataStructure
		{
			SOMClass SOMSTAR classObject;
		} somOS_ServiceBasePRefClassDataStructure;
		typedef struct somOS_ServiceBasePRefCClassDataStructure
		{
			somMethodTabs parentMtab;
			somDToken instanceDataToken;
		} somOS_ServiceBasePRefCClassDataStructure;
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_somos_Source) || defined(somOS_ServiceBasePRef_Class_Source)
				SOMEXTERN struct somOS_ServiceBasePRefClassDataStructure _somOS_ServiceBasePRefClassData;
				#ifndef somOS_ServiceBasePRefClassData
					#define somOS_ServiceBasePRefClassData    _somOS_ServiceBasePRefClassData
				#endif /* somOS_ServiceBasePRefClassData */
			#else
				SOMEXTERN struct somOS_ServiceBasePRefClassDataStructure * SOMLINK resolve_somOS_ServiceBasePRefClassData(void);
				#ifndef somOS_ServiceBasePRefClassData
					#define somOS_ServiceBasePRefClassData    (*(resolve_somOS_ServiceBasePRefClassData()))
				#endif /* somOS_ServiceBasePRefClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_somos_Source) || defined(somOS_ServiceBasePRef_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_somos_Source || somOS_ServiceBasePRef_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_somos_Source || somOS_ServiceBasePRef_Class_Source */
			struct somOS_ServiceBasePRefClassDataStructure SOMDLINK somOS_ServiceBasePRefClassData;
		#endif /* SOM_RESOLVE_DATA */
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_somos_Source) || defined(somOS_ServiceBasePRef_Class_Source)
				SOMEXTERN struct somOS_ServiceBasePRefCClassDataStructure _somOS_ServiceBasePRefCClassData;
				#ifndef somOS_ServiceBasePRefCClassData
					#define somOS_ServiceBasePRefCClassData    _somOS_ServiceBasePRefCClassData
				#endif /* somOS_ServiceBasePRefCClassData */
			#else
				SOMEXTERN struct somOS_ServiceBasePRefCClassDataStructure * SOMLINK resolve_somOS_ServiceBasePRefCClassData(void);
				#ifndef somOS_ServiceBasePRefCClassData
					#define somOS_ServiceBasePRefCClassData    (*(resolve_somOS_ServiceBasePRefCClassData()))
				#endif /* somOS_ServiceBasePRefCClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_somos_Source) || defined(somOS_ServiceBasePRef_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_somos_Source || somOS_ServiceBasePRef_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_somos_Source || somOS_ServiceBasePRef_Class_Source */
			struct somOS_ServiceBasePRefCClassDataStructure SOMDLINK somOS_ServiceBasePRefCClassData;
		#endif /* SOM_RESOLVE_DATA */
		extern 
		#if defined(SOM_Module_somos_Source) || defined(somOS_ServiceBasePRef_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_somos_Source || somOS_ServiceBasePRef_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_somos_Source || somOS_ServiceBasePRef_Class_Source */
		SOMClass SOMSTAR SOMLINK somOS_ServiceBasePRefNewClass
				(integer4 somtmajorVersion,integer4 somtminorVersion);
		#define _SOMCLASS_somOS_ServiceBasePRef (somOS_ServiceBasePRefClassData.classObject)
		#ifndef SOMGD_somOS_ServiceBasePRef
			#if (defined(_somOS_ServiceBasePRef) || defined(__somOS_ServiceBasePRef))
				#undef _somOS_ServiceBasePRef
				#undef __somOS_ServiceBasePRef
				#define SOMGD_somOS_ServiceBasePRef 1
			#else
				#define _somOS_ServiceBasePRef _SOMCLASS_somOS_ServiceBasePRef
			#endif /* _somOS_ServiceBasePRef */
		#endif /* SOMGD_somOS_ServiceBasePRef */
		#define somOS_ServiceBasePRef_classObj _SOMCLASS_somOS_ServiceBasePRef
		#define _SOMMTOKEN_somOS_ServiceBasePRef(method) ((somMToken)(somOS_ServiceBasePRefClassData.method))
		#ifndef somOS_ServiceBasePRefNew
			#define somOS_ServiceBasePRefNew() ( _somOS_ServiceBasePRef ? \
				(SOMClass_somNew(_somOS_ServiceBasePRef)) : \
				( somOS_ServiceBasePRefNewClass( \
					somOS_ServiceBasePRef_MajorVersion, \
					somOS_ServiceBasePRef_MinorVersion), \
				SOMClass_somNew(_somOS_ServiceBasePRef))) 
		#endif /* NewsomOS_ServiceBasePRef */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define somOS_ServiceBasePRef_somInit SOMObject_somInit
#define somOS_ServiceBasePRef_somUninit SOMObject_somUninit
#define somOS_ServiceBasePRef_somFree SOMObject_somFree
#define somOS_ServiceBasePRef_somGetClass SOMObject_somGetClass
#define somOS_ServiceBasePRef_somGetClassName SOMObject_somGetClassName
#define somOS_ServiceBasePRef_somGetSize SOMObject_somGetSize
#define somOS_ServiceBasePRef_somIsA SOMObject_somIsA
#define somOS_ServiceBasePRef_somIsInstanceOf SOMObject_somIsInstanceOf
#define somOS_ServiceBasePRef_somRespondsTo SOMObject_somRespondsTo
#define somOS_ServiceBasePRef_somDispatch SOMObject_somDispatch
#define somOS_ServiceBasePRef_somClassDispatch SOMObject_somClassDispatch
#define somOS_ServiceBasePRef_somCastObj SOMObject_somCastObj
#define somOS_ServiceBasePRef_somResetObj SOMObject_somResetObj
#define somOS_ServiceBasePRef_somPrintSelf SOMObject_somPrintSelf
#define somOS_ServiceBasePRef_somDumpSelf SOMObject_somDumpSelf
#define somOS_ServiceBasePRef_somDumpSelfInt SOMObject_somDumpSelfInt
#define somOS_ServiceBasePRef_somDefaultInit SOMObject_somDefaultInit
#define somOS_ServiceBasePRef_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define somOS_ServiceBasePRef_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define somOS_ServiceBasePRef_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define somOS_ServiceBasePRef_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define somOS_ServiceBasePRef_somDefaultAssign SOMObject_somDefaultAssign
#define somOS_ServiceBasePRef_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define somOS_ServiceBasePRef_somDefaultVAssign SOMObject_somDefaultVAssign
#define somOS_ServiceBasePRef_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define somOS_ServiceBasePRef_somDestruct SOMObject_somDestruct
#define somOS_ServiceBasePRef__get_constant_random_id CosObjectIdentity_IdentifiableObject__get_constant_random_id
#define somOS_ServiceBasePRef_is_identical CosObjectIdentity_IdentifiableObject_is_identical
#define somOS_ServiceBasePRef_reinit somOS_ServiceBase_reinit
#define somOS_ServiceBasePRef_capture somOS_ServiceBase_capture
#define somOS_ServiceBasePRef_GetInstanceManager somOS_ServiceBase_GetInstanceManager
#define somOS_ServiceBasePRef_init_for_object_creation somOS_ServiceBase_init_for_object_creation
#define somOS_ServiceBasePRef_init_for_object_reactivation somOS_ServiceBase_init_for_object_reactivation
#define somOS_ServiceBasePRef_init_for_object_copy somOS_ServiceBase_init_for_object_copy
#define somOS_ServiceBasePRef_uninit_for_object_move somOS_ServiceBase_uninit_for_object_move
#define somOS_ServiceBasePRef_uninit_for_object_passivation somOS_ServiceBase_uninit_for_object_passivation
#define somOS_ServiceBasePRef_uninit_for_object_destruction somOS_ServiceBase_uninit_for_object_destruction
#endif /* SOM_DONT_USE_INHERITED_MACROS */

		#ifndef somOS_ServiceBaseCORBA_MajorVersion
			#define somOS_ServiceBaseCORBA_MajorVersion   3
		#endif /* somOS_ServiceBaseCORBA_MajorVersion */
		#ifndef somOS_ServiceBaseCORBA_MinorVersion
			#define somOS_ServiceBaseCORBA_MinorVersion   0
		#endif /* somOS_ServiceBaseCORBA_MinorVersion */
		typedef struct somOS_ServiceBaseCORBAClassDataStructure
		{
			SOMClass SOMSTAR classObject;
		} somOS_ServiceBaseCORBAClassDataStructure;
		typedef struct somOS_ServiceBaseCORBACClassDataStructure
		{
			somMethodTabs parentMtab;
			somDToken instanceDataToken;
		} somOS_ServiceBaseCORBACClassDataStructure;
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_somos_Source) || defined(somOS_ServiceBaseCORBA_Class_Source)
				SOMEXTERN struct somOS_ServiceBaseCORBAClassDataStructure _somOS_ServiceBaseCORBAClassData;
				#ifndef somOS_ServiceBaseCORBAClassData
					#define somOS_ServiceBaseCORBAClassData    _somOS_ServiceBaseCORBAClassData
				#endif /* somOS_ServiceBaseCORBAClassData */
			#else
				SOMEXTERN struct somOS_ServiceBaseCORBAClassDataStructure * SOMLINK resolve_somOS_ServiceBaseCORBAClassData(void);
				#ifndef somOS_ServiceBaseCORBAClassData
					#define somOS_ServiceBaseCORBAClassData    (*(resolve_somOS_ServiceBaseCORBAClassData()))
				#endif /* somOS_ServiceBaseCORBAClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_somos_Source) || defined(somOS_ServiceBaseCORBA_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_somos_Source || somOS_ServiceBaseCORBA_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_somos_Source || somOS_ServiceBaseCORBA_Class_Source */
			struct somOS_ServiceBaseCORBAClassDataStructure SOMDLINK somOS_ServiceBaseCORBAClassData;
		#endif /* SOM_RESOLVE_DATA */
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_somos_Source) || defined(somOS_ServiceBaseCORBA_Class_Source)
				SOMEXTERN struct somOS_ServiceBaseCORBACClassDataStructure _somOS_ServiceBaseCORBACClassData;
				#ifndef somOS_ServiceBaseCORBACClassData
					#define somOS_ServiceBaseCORBACClassData    _somOS_ServiceBaseCORBACClassData
				#endif /* somOS_ServiceBaseCORBACClassData */
			#else
				SOMEXTERN struct somOS_ServiceBaseCORBACClassDataStructure * SOMLINK resolve_somOS_ServiceBaseCORBACClassData(void);
				#ifndef somOS_ServiceBaseCORBACClassData
					#define somOS_ServiceBaseCORBACClassData    (*(resolve_somOS_ServiceBaseCORBACClassData()))
				#endif /* somOS_ServiceBaseCORBACClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_somos_Source) || defined(somOS_ServiceBaseCORBA_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_somos_Source || somOS_ServiceBaseCORBA_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_somos_Source || somOS_ServiceBaseCORBA_Class_Source */
			struct somOS_ServiceBaseCORBACClassDataStructure SOMDLINK somOS_ServiceBaseCORBACClassData;
		#endif /* SOM_RESOLVE_DATA */
		extern 
		#if defined(SOM_Module_somos_Source) || defined(somOS_ServiceBaseCORBA_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_somos_Source || somOS_ServiceBaseCORBA_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_somos_Source || somOS_ServiceBaseCORBA_Class_Source */
		SOMClass SOMSTAR SOMLINK somOS_ServiceBaseCORBANewClass
				(integer4 somtmajorVersion,integer4 somtminorVersion);
		#define _SOMCLASS_somOS_ServiceBaseCORBA (somOS_ServiceBaseCORBAClassData.classObject)
		#ifndef SOMGD_somOS_ServiceBaseCORBA
			#if (defined(_somOS_ServiceBaseCORBA) || defined(__somOS_ServiceBaseCORBA))
				#undef _somOS_ServiceBaseCORBA
				#undef __somOS_ServiceBaseCORBA
				#define SOMGD_somOS_ServiceBaseCORBA 1
			#else
				#define _somOS_ServiceBaseCORBA _SOMCLASS_somOS_ServiceBaseCORBA
			#endif /* _somOS_ServiceBaseCORBA */
		#endif /* SOMGD_somOS_ServiceBaseCORBA */
		#define somOS_ServiceBaseCORBA_classObj _SOMCLASS_somOS_ServiceBaseCORBA
		#define _SOMMTOKEN_somOS_ServiceBaseCORBA(method) ((somMToken)(somOS_ServiceBaseCORBAClassData.method))
		#ifndef somOS_ServiceBaseCORBANew
			#define somOS_ServiceBaseCORBANew() ( _somOS_ServiceBaseCORBA ? \
				(SOMClass_somNew(_somOS_ServiceBaseCORBA)) : \
				( somOS_ServiceBaseCORBANewClass( \
					somOS_ServiceBaseCORBA_MajorVersion, \
					somOS_ServiceBaseCORBA_MinorVersion), \
				SOMClass_somNew(_somOS_ServiceBaseCORBA))) 
		#endif /* NewsomOS_ServiceBaseCORBA */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define somOS_ServiceBaseCORBA_somInit SOMObject_somInit
#define somOS_ServiceBaseCORBA_somUninit SOMObject_somUninit
#define somOS_ServiceBaseCORBA_somFree SOMObject_somFree
#define somOS_ServiceBaseCORBA_somGetClass SOMObject_somGetClass
#define somOS_ServiceBaseCORBA_somGetClassName SOMObject_somGetClassName
#define somOS_ServiceBaseCORBA_somGetSize SOMObject_somGetSize
#define somOS_ServiceBaseCORBA_somIsA SOMObject_somIsA
#define somOS_ServiceBaseCORBA_somIsInstanceOf SOMObject_somIsInstanceOf
#define somOS_ServiceBaseCORBA_somRespondsTo SOMObject_somRespondsTo
#define somOS_ServiceBaseCORBA_somDispatch SOMObject_somDispatch
#define somOS_ServiceBaseCORBA_somClassDispatch SOMObject_somClassDispatch
#define somOS_ServiceBaseCORBA_somCastObj SOMObject_somCastObj
#define somOS_ServiceBaseCORBA_somResetObj SOMObject_somResetObj
#define somOS_ServiceBaseCORBA_somPrintSelf SOMObject_somPrintSelf
#define somOS_ServiceBaseCORBA_somDumpSelf SOMObject_somDumpSelf
#define somOS_ServiceBaseCORBA_somDumpSelfInt SOMObject_somDumpSelfInt
#define somOS_ServiceBaseCORBA_somDefaultInit SOMObject_somDefaultInit
#define somOS_ServiceBaseCORBA_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define somOS_ServiceBaseCORBA_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define somOS_ServiceBaseCORBA_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define somOS_ServiceBaseCORBA_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define somOS_ServiceBaseCORBA_somDefaultAssign SOMObject_somDefaultAssign
#define somOS_ServiceBaseCORBA_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define somOS_ServiceBaseCORBA_somDefaultVAssign SOMObject_somDefaultVAssign
#define somOS_ServiceBaseCORBA_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define somOS_ServiceBaseCORBA_somDestruct SOMObject_somDestruct
#define somOS_ServiceBaseCORBA__get_constant_random_id CosObjectIdentity_IdentifiableObject__get_constant_random_id
#define somOS_ServiceBaseCORBA_is_identical CosObjectIdentity_IdentifiableObject_is_identical
#define somOS_ServiceBaseCORBA_reinit somOS_ServiceBase_reinit
#define somOS_ServiceBaseCORBA_capture somOS_ServiceBase_capture
#define somOS_ServiceBaseCORBA_GetInstanceManager somOS_ServiceBase_GetInstanceManager
#define somOS_ServiceBaseCORBA_init_for_object_creation somOS_ServiceBase_init_for_object_creation
#define somOS_ServiceBaseCORBA_init_for_object_reactivation somOS_ServiceBase_init_for_object_reactivation
#define somOS_ServiceBaseCORBA_init_for_object_copy somOS_ServiceBase_init_for_object_copy
#define somOS_ServiceBaseCORBA_uninit_for_object_move somOS_ServiceBase_uninit_for_object_move
#define somOS_ServiceBaseCORBA_uninit_for_object_passivation somOS_ServiceBase_uninit_for_object_passivation
#define somOS_ServiceBaseCORBA_uninit_for_object_destruction somOS_ServiceBase_uninit_for_object_destruction
#endif /* SOM_DONT_USE_INHERITED_MACROS */

		#ifndef somOS_ServiceBasePRefCORBA_MajorVersion
			#define somOS_ServiceBasePRefCORBA_MajorVersion   3
		#endif /* somOS_ServiceBasePRefCORBA_MajorVersion */
		#ifndef somOS_ServiceBasePRefCORBA_MinorVersion
			#define somOS_ServiceBasePRefCORBA_MinorVersion   0
		#endif /* somOS_ServiceBasePRefCORBA_MinorVersion */
		typedef struct somOS_ServiceBasePRefCORBAClassDataStructure
		{
			SOMClass SOMSTAR classObject;
		} somOS_ServiceBasePRefCORBAClassDataStructure;
		typedef struct somOS_ServiceBasePRefCORBACClassDataStructure
		{
			somMethodTabs parentMtab;
			somDToken instanceDataToken;
		} somOS_ServiceBasePRefCORBACClassDataStructure;
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_somos_Source) || defined(somOS_ServiceBasePRefCORBA_Class_Source)
				SOMEXTERN struct somOS_ServiceBasePRefCORBAClassDataStructure _somOS_ServiceBasePRefCORBAClassData;
				#ifndef somOS_ServiceBasePRefCORBAClassData
					#define somOS_ServiceBasePRefCORBAClassData    _somOS_ServiceBasePRefCORBAClassData
				#endif /* somOS_ServiceBasePRefCORBAClassData */
			#else
				SOMEXTERN struct somOS_ServiceBasePRefCORBAClassDataStructure * SOMLINK resolve_somOS_ServiceBasePRefCORBAClassData(void);
				#ifndef somOS_ServiceBasePRefCORBAClassData
					#define somOS_ServiceBasePRefCORBAClassData    (*(resolve_somOS_ServiceBasePRefCORBAClassData()))
				#endif /* somOS_ServiceBasePRefCORBAClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_somos_Source) || defined(somOS_ServiceBasePRefCORBA_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_somos_Source || somOS_ServiceBasePRefCORBA_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_somos_Source || somOS_ServiceBasePRefCORBA_Class_Source */
			struct somOS_ServiceBasePRefCORBAClassDataStructure SOMDLINK somOS_ServiceBasePRefCORBAClassData;
		#endif /* SOM_RESOLVE_DATA */
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_somos_Source) || defined(somOS_ServiceBasePRefCORBA_Class_Source)
				SOMEXTERN struct somOS_ServiceBasePRefCORBACClassDataStructure _somOS_ServiceBasePRefCORBACClassData;
				#ifndef somOS_ServiceBasePRefCORBACClassData
					#define somOS_ServiceBasePRefCORBACClassData    _somOS_ServiceBasePRefCORBACClassData
				#endif /* somOS_ServiceBasePRefCORBACClassData */
			#else
				SOMEXTERN struct somOS_ServiceBasePRefCORBACClassDataStructure * SOMLINK resolve_somOS_ServiceBasePRefCORBACClassData(void);
				#ifndef somOS_ServiceBasePRefCORBACClassData
					#define somOS_ServiceBasePRefCORBACClassData    (*(resolve_somOS_ServiceBasePRefCORBACClassData()))
				#endif /* somOS_ServiceBasePRefCORBACClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_somos_Source) || defined(somOS_ServiceBasePRefCORBA_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_somos_Source || somOS_ServiceBasePRefCORBA_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_somos_Source || somOS_ServiceBasePRefCORBA_Class_Source */
			struct somOS_ServiceBasePRefCORBACClassDataStructure SOMDLINK somOS_ServiceBasePRefCORBACClassData;
		#endif /* SOM_RESOLVE_DATA */
		extern 
		#if defined(SOM_Module_somos_Source) || defined(somOS_ServiceBasePRefCORBA_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_somos_Source || somOS_ServiceBasePRefCORBA_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_somos_Source || somOS_ServiceBasePRefCORBA_Class_Source */
		SOMClass SOMSTAR SOMLINK somOS_ServiceBasePRefCORBANewClass
				(integer4 somtmajorVersion,integer4 somtminorVersion);
		#define _SOMCLASS_somOS_ServiceBasePRefCORBA (somOS_ServiceBasePRefCORBAClassData.classObject)
		#ifndef SOMGD_somOS_ServiceBasePRefCORBA
			#if (defined(_somOS_ServiceBasePRefCORBA) || defined(__somOS_ServiceBasePRefCORBA))
				#undef _somOS_ServiceBasePRefCORBA
				#undef __somOS_ServiceBasePRefCORBA
				#define SOMGD_somOS_ServiceBasePRefCORBA 1
			#else
				#define _somOS_ServiceBasePRefCORBA _SOMCLASS_somOS_ServiceBasePRefCORBA
			#endif /* _somOS_ServiceBasePRefCORBA */
		#endif /* SOMGD_somOS_ServiceBasePRefCORBA */
		#define somOS_ServiceBasePRefCORBA_classObj _SOMCLASS_somOS_ServiceBasePRefCORBA
		#define _SOMMTOKEN_somOS_ServiceBasePRefCORBA(method) ((somMToken)(somOS_ServiceBasePRefCORBAClassData.method))
		#ifndef somOS_ServiceBasePRefCORBANew
			#define somOS_ServiceBasePRefCORBANew() ( _somOS_ServiceBasePRefCORBA ? \
				(SOMClass_somNew(_somOS_ServiceBasePRefCORBA)) : \
				( somOS_ServiceBasePRefCORBANewClass( \
					somOS_ServiceBasePRefCORBA_MajorVersion, \
					somOS_ServiceBasePRefCORBA_MinorVersion), \
				SOMClass_somNew(_somOS_ServiceBasePRefCORBA))) 
		#endif /* NewsomOS_ServiceBasePRefCORBA */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define somOS_ServiceBasePRefCORBA_somInit SOMObject_somInit
#define somOS_ServiceBasePRefCORBA_somUninit SOMObject_somUninit
#define somOS_ServiceBasePRefCORBA_somFree SOMObject_somFree
#define somOS_ServiceBasePRefCORBA_somGetClass SOMObject_somGetClass
#define somOS_ServiceBasePRefCORBA_somGetClassName SOMObject_somGetClassName
#define somOS_ServiceBasePRefCORBA_somGetSize SOMObject_somGetSize
#define somOS_ServiceBasePRefCORBA_somIsA SOMObject_somIsA
#define somOS_ServiceBasePRefCORBA_somIsInstanceOf SOMObject_somIsInstanceOf
#define somOS_ServiceBasePRefCORBA_somRespondsTo SOMObject_somRespondsTo
#define somOS_ServiceBasePRefCORBA_somDispatch SOMObject_somDispatch
#define somOS_ServiceBasePRefCORBA_somClassDispatch SOMObject_somClassDispatch
#define somOS_ServiceBasePRefCORBA_somCastObj SOMObject_somCastObj
#define somOS_ServiceBasePRefCORBA_somResetObj SOMObject_somResetObj
#define somOS_ServiceBasePRefCORBA_somPrintSelf SOMObject_somPrintSelf
#define somOS_ServiceBasePRefCORBA_somDumpSelf SOMObject_somDumpSelf
#define somOS_ServiceBasePRefCORBA_somDumpSelfInt SOMObject_somDumpSelfInt
#define somOS_ServiceBasePRefCORBA_somDefaultInit SOMObject_somDefaultInit
#define somOS_ServiceBasePRefCORBA_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define somOS_ServiceBasePRefCORBA_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define somOS_ServiceBasePRefCORBA_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define somOS_ServiceBasePRefCORBA_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define somOS_ServiceBasePRefCORBA_somDefaultAssign SOMObject_somDefaultAssign
#define somOS_ServiceBasePRefCORBA_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define somOS_ServiceBasePRefCORBA_somDefaultVAssign SOMObject_somDefaultVAssign
#define somOS_ServiceBasePRefCORBA_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define somOS_ServiceBasePRefCORBA_somDestruct SOMObject_somDestruct
#define somOS_ServiceBasePRefCORBA__get_constant_random_id CosObjectIdentity_IdentifiableObject__get_constant_random_id
#define somOS_ServiceBasePRefCORBA_is_identical CosObjectIdentity_IdentifiableObject_is_identical
#define somOS_ServiceBasePRefCORBA_reinit somOS_ServiceBase_reinit
#define somOS_ServiceBasePRefCORBA_capture somOS_ServiceBase_capture
#define somOS_ServiceBasePRefCORBA_GetInstanceManager somOS_ServiceBase_GetInstanceManager
#define somOS_ServiceBasePRefCORBA_init_for_object_creation somOS_ServiceBase_init_for_object_creation
#define somOS_ServiceBasePRefCORBA_init_for_object_reactivation somOS_ServiceBase_init_for_object_reactivation
#define somOS_ServiceBasePRefCORBA_init_for_object_copy somOS_ServiceBase_init_for_object_copy
#define somOS_ServiceBasePRefCORBA_uninit_for_object_move somOS_ServiceBase_uninit_for_object_move
#define somOS_ServiceBasePRefCORBA_uninit_for_object_passivation somOS_ServiceBase_uninit_for_object_passivation
#define somOS_ServiceBasePRefCORBA_uninit_for_object_destruction somOS_ServiceBase_uninit_for_object_destruction
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_somos_Header_h */
