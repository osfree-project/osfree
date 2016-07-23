/* generated from orb.idl */
/* internal conditional is SOM_Module_orb_Source */
#ifndef SOM_Module_orb_Header_h
	#define SOM_Module_orb_Header_h 1
	#include <som.h>
	#include <somobj.h>
	#include <somdtype.h>
	#include <containd.h>
	#include <snglicls.h>
	#include <somcls.h>
	#include <unotypes.h>
	#ifndef _IDL_OperationDef_defined
		#define _IDL_OperationDef_defined
		typedef SOMObject OperationDef;
	#endif /* _IDL_OperationDef_defined */
	#ifndef _IDL_SEQUENCE_OperationDef_defined
		#define _IDL_SEQUENCE_OperationDef_defined
		SOM_SEQUENCE_TYPEDEF_NAME(OperationDef SOMSTAR ,sequence(OperationDef));
	#endif /* _IDL_SEQUENCE_OperationDef_defined */
	#ifndef _IDL_SOMDObject_defined
		#define _IDL_SOMDObject_defined
		typedef SOMObject SOMDObject;
	#endif /* _IDL_SOMDObject_defined */
	#ifndef _IDL_SEQUENCE_SOMDObject_defined
		#define _IDL_SEQUENCE_SOMDObject_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMDObject SOMSTAR ,sequence(SOMDObject));
	#endif /* _IDL_SEQUENCE_SOMDObject_defined */
	#ifndef _IDL_NVList_defined
		#define _IDL_NVList_defined
		typedef SOMObject NVList;
	#endif /* _IDL_NVList_defined */
	#ifndef _IDL_SEQUENCE_NVList_defined
		#define _IDL_SEQUENCE_NVList_defined
		SOM_SEQUENCE_TYPEDEF_NAME(NVList SOMSTAR ,sequence(NVList));
	#endif /* _IDL_SEQUENCE_NVList_defined */
	#ifndef _IDL_Context_defined
		#define _IDL_Context_defined
		typedef SOMObject Context;
	#endif /* _IDL_Context_defined */
	#ifndef _IDL_SEQUENCE_Context_defined
		#define _IDL_SEQUENCE_Context_defined
		SOM_SEQUENCE_TYPEDEF_NAME(Context SOMSTAR ,sequence(Context));
	#endif /* _IDL_SEQUENCE_Context_defined */
	#ifndef _IDL_ORB_defined
		#define _IDL_ORB_defined
		typedef SOMObject ORB;
	#endif /* _IDL_ORB_defined */
	#ifndef _IDL_SEQUENCE_ORB_defined
		#define _IDL_SEQUENCE_ORB_defined
		SOM_SEQUENCE_TYPEDEF_NAME(ORB SOMSTAR ,sequence(ORB));
	#endif /* _IDL_SEQUENCE_ORB_defined */
	typedef string ORB_ObjectId;
	#ifndef _IDL_SEQUENCE_string_defined
		#define _IDL_SEQUENCE_string_defined
		SOM_SEQUENCE_TYPEDEF(string);
	#endif /* _IDL_SEQUENCE_string_defined */
	typedef _IDL_SEQUENCE_string ORB_ObjectIdList;
	#ifndef _IDL_SEQUENCE_ORB_ObjectIdList_defined
		#define _IDL_SEQUENCE_ORB_ObjectIdList_defined
		SOM_SEQUENCE_TYPEDEF(ORB_ObjectIdList);
	#endif /* _IDL_SEQUENCE_ORB_ObjectIdList_defined */
#define ex_ORB_InvalidName   "::ORB::InvalidName"
#ifndef SOM_DONT_USE_SHORT_NAMES
#ifndef SOMTGD_ex_InvalidName
#ifndef ex_InvalidName
#define ex_InvalidName  ex_ORB_InvalidName
#else
#define SOMTGD_ex_InvalidName
#undef ex_InvalidName
#endif /* ex_InvalidName */
#endif /* SOMTGD_ex_InvalidName */
#endif /* SOM_DONT_USE_SHORT_NAMES */
typedef struct ORB_InvalidName ORB_InvalidName;
#ifndef SOM_DONT_USE_SHORT_NAMES
	#ifndef SOMGD_InvalidName
		#if defined(InvalidName)
			#undef InvalidName
			#define SOMGD_InvalidName
		#else
			#define InvalidName ORB_InvalidName
		#endif
	#endif /* SOMGD_InvalidName */
#endif /* SOM_DONT_USE_SHORT_NAMES */
#ifndef ORB_initial_InterfaceRepository
	#define ORB_initial_InterfaceRepository   "InterfaceRepository"
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_initial_InterfaceRepository
			#if defined(initial_InterfaceRepository)
				#undef initial_InterfaceRepository
				#define SOMGD_initial_InterfaceRepository
			#else
				#define initial_InterfaceRepository ORB_initial_InterfaceRepository
			#endif
		#endif /* SOMGD_initial_InterfaceRepository */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* ORB_initial_InterfaceRepository */
#ifndef ORB_initial_NameService
	#define ORB_initial_NameService   "NameService"
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_initial_NameService
			#if defined(initial_NameService)
				#undef initial_NameService
				#define SOMGD_initial_NameService
			#else
				#define initial_NameService ORB_initial_NameService
			#endif
		#endif /* SOMGD_initial_NameService */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* ORB_initial_NameService */
#ifndef ORB_initial_CodecFactory
	#define ORB_initial_CodecFactory   "CodecFactory"
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_initial_CodecFactory
			#if defined(initial_CodecFactory)
				#undef initial_CodecFactory
				#define SOMGD_initial_CodecFactory
			#else
				#define initial_CodecFactory ORB_initial_CodecFactory
			#endif
		#endif /* SOMGD_initial_CodecFactory */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* ORB_initial_CodecFactory */
#ifndef ORB_initial_DynAnyFactory
	#define ORB_initial_DynAnyFactory   "DynAnyFactory"
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_initial_DynAnyFactory
			#if defined(initial_DynAnyFactory)
				#undef initial_DynAnyFactory
				#define SOMGD_initial_DynAnyFactory
			#else
				#define initial_DynAnyFactory ORB_initial_DynAnyFactory
			#endif
		#endif /* SOMGD_initial_DynAnyFactory */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* ORB_initial_DynAnyFactory */
	#ifdef __IBMC__
		typedef string (somTP_ORB_object_to_string)(
			ORB SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMObject SOMSTAR obj);
		#pragma linkage(somTP_ORB_object_to_string,system)
		typedef somTP_ORB_object_to_string *somTD_ORB_object_to_string;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_ORB_object_to_string)(
			ORB SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMObject SOMSTAR obj);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMObject SOMSTAR (somTP_ORB_string_to_object)(
			ORB SOMSTAR somSelf,
			Environment *ev,
			/* in */ string str);
		#pragma linkage(somTP_ORB_string_to_object,system)
		typedef somTP_ORB_string_to_object *somTD_ORB_string_to_object;
	#else /* __IBMC__ */
		typedef SOMObject SOMSTAR (SOMLINK * somTD_ORB_string_to_object)(
			ORB SOMSTAR somSelf,
			Environment *ev,
			/* in */ string str);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef ORBStatus (somTP_ORB_create_list)(
			ORB SOMSTAR somSelf,
			Environment *ev,
			/* in */ long count,
			/* out */ NVList SOMSTAR *new_list);
		#pragma linkage(somTP_ORB_create_list,system)
		typedef somTP_ORB_create_list *somTD_ORB_create_list;
	#else /* __IBMC__ */
		typedef ORBStatus (SOMLINK * somTD_ORB_create_list)(
			ORB SOMSTAR somSelf,
			Environment *ev,
			/* in */ long count,
			/* out */ NVList SOMSTAR *new_list);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef ORBStatus (somTP_ORB_create_operation_list)(
			ORB SOMSTAR somSelf,
			Environment *ev,
			/* in */ OperationDef SOMSTAR oper,
			/* out */ NVList SOMSTAR *new_list);
		#pragma linkage(somTP_ORB_create_operation_list,system)
		typedef somTP_ORB_create_operation_list *somTD_ORB_create_operation_list;
	#else /* __IBMC__ */
		typedef ORBStatus (SOMLINK * somTD_ORB_create_operation_list)(
			ORB SOMSTAR somSelf,
			Environment *ev,
			/* in */ OperationDef SOMSTAR oper,
			/* out */ NVList SOMSTAR *new_list);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef ORBStatus (somTP_ORB_get_default_context)(
			ORB SOMSTAR somSelf,
			Environment *ev,
			/* out */ Context SOMSTAR *ctx);
		#pragma linkage(somTP_ORB_get_default_context,system)
		typedef somTP_ORB_get_default_context *somTD_ORB_get_default_context;
	#else /* __IBMC__ */
		typedef ORBStatus (SOMLINK * somTD_ORB_get_default_context)(
			ORB SOMSTAR somSelf,
			Environment *ev,
			/* out */ Context SOMSTAR *ctx);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef ORB_ObjectIdList (somTP_ORB_list_initial_services)(
			ORB SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_ORB_list_initial_services,system)
		typedef somTP_ORB_list_initial_services *somTD_ORB_list_initial_services;
	#else /* __IBMC__ */
		typedef ORB_ObjectIdList (SOMLINK * somTD_ORB_list_initial_services)(
			ORB SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMObject SOMSTAR (somTP_ORB_resolve_initial_references)(
			ORB SOMSTAR somSelf,
			Environment *ev,
			/* in */ ORB_ObjectId identifier);
		#pragma linkage(somTP_ORB_resolve_initial_references,system)
		typedef somTP_ORB_resolve_initial_references *somTD_ORB_resolve_initial_references;
	#else /* __IBMC__ */
		typedef SOMObject SOMSTAR (SOMLINK * somTD_ORB_resolve_initial_references)(
			ORB SOMSTAR somSelf,
			Environment *ev,
			/* in */ ORB_ObjectId identifier);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_ORB__set_stringToObject30)(
			ORB SOMSTAR somSelf,
			Environment *ev,
			/* in */ boolean stringToObject30);
		#pragma linkage(somTP_ORB__set_stringToObject30,system)
		typedef somTP_ORB__set_stringToObject30 *somTD_ORB__set_stringToObject30;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_ORB__set_stringToObject30)(
			ORB SOMSTAR somSelf,
			Environment *ev,
			/* in */ boolean stringToObject30);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef boolean (somTP_ORB__get_stringToObject30)(
			ORB SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_ORB__get_stringToObject30,system)
		typedef somTP_ORB__get_stringToObject30 *somTD_ORB__get_stringToObject30;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_ORB__get_stringToObject30)(
			ORB SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef boolean (somTP_ORB_work_pending)(
			ORB SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_ORB_work_pending,system)
		typedef somTP_ORB_work_pending *somTD_ORB_work_pending;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_ORB_work_pending)(
			ORB SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_ORB_perform_work)(
			ORB SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_ORB_perform_work,system)
		typedef somTP_ORB_perform_work *somTD_ORB_perform_work;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_ORB_perform_work)(
			ORB SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_ORB_shutdown)(
			ORB SOMSTAR somSelf,
			Environment *ev,
			/* in */ boolean wait_for_completion);
		#pragma linkage(somTP_ORB_shutdown,system)
		typedef somTP_ORB_shutdown *somTD_ORB_shutdown;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_ORB_shutdown)(
			ORB SOMSTAR somSelf,
			Environment *ev,
			/* in */ boolean wait_for_completion);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_ORB_run)(
			ORB SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_ORB_run,system)
		typedef somTP_ORB_run *somTD_ORB_run;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_ORB_run)(
			ORB SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifndef ORB_MajorVersion
		#define ORB_MajorVersion   2
	#endif /* ORB_MajorVersion */
	#ifndef ORB_MinorVersion
		#define ORB_MinorVersion   1
	#endif /* ORB_MinorVersion */
	typedef struct ORBClassDataStructure
	{
		SOMMSingleInstance SOMSTAR classObject;
		somMToken object_to_string;
		somMToken string_to_object;
		somMToken create_list;
		somMToken create_operation_list;
		somMToken get_default_context;
		somMToken private5;
		somMToken private6;
		somMToken private7;
		somMToken private8;
		somMToken list_initial_services;
		somMToken resolve_initial_references;
		somMToken _get_stringToObject30;
		somMToken _set_stringToObject30;
		somMToken private13;
		somMToken private14;
		somMToken private15;
		somMToken private16;
		somMToken run;
		somMToken shutdown;
		somMToken perform_work;
		somMToken work_pending;
	} ORBClassDataStructure;
	typedef struct ORBCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} ORBCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_orb_Source) || defined(ORB_Class_Source)
			SOMEXTERN struct ORBClassDataStructure _ORBClassData;
			#ifndef ORBClassData
				#define ORBClassData    _ORBClassData
			#endif /* ORBClassData */
		#else
			SOMEXTERN struct ORBClassDataStructure * SOMLINK resolve_ORBClassData(void);
			#ifndef ORBClassData
				#define ORBClassData    (*(resolve_ORBClassData()))
			#endif /* ORBClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_orb_Source) || defined(ORB_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_orb_Source || ORB_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_orb_Source || ORB_Class_Source */
		struct ORBClassDataStructure SOMDLINK ORBClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_orb_Source) || defined(ORB_Class_Source)
			SOMEXTERN struct ORBCClassDataStructure _ORBCClassData;
			#ifndef ORBCClassData
				#define ORBCClassData    _ORBCClassData
			#endif /* ORBCClassData */
		#else
			SOMEXTERN struct ORBCClassDataStructure * SOMLINK resolve_ORBCClassData(void);
			#ifndef ORBCClassData
				#define ORBCClassData    (*(resolve_ORBCClassData()))
			#endif /* ORBCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_orb_Source) || defined(ORB_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_orb_Source || ORB_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_orb_Source || ORB_Class_Source */
		struct ORBCClassDataStructure SOMDLINK ORBCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_orb_Source) || defined(ORB_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_orb_Source || ORB_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_orb_Source || ORB_Class_Source */
	SOMMSingleInstance SOMSTAR SOMLINK ORBNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_ORB (ORBClassData.classObject)
	#ifndef SOMGD_ORB
		#if (defined(_ORB) || defined(__ORB))
			#undef _ORB
			#undef __ORB
			#define SOMGD_ORB 1
		#else
			#define _ORB _SOMCLASS_ORB
		#endif /* _ORB */
	#endif /* SOMGD_ORB */
	#define ORB_classObj _SOMCLASS_ORB
	#define _SOMMTOKEN_ORB(method) ((somMToken)(ORBClassData.method))
	#ifndef ORBNew
		#define ORBNew() ( _ORB ? \
			(SOMClass_somNew(_ORB)) : \
			( ORBNewClass( \
				ORB_MajorVersion, \
				ORB_MinorVersion), \
			SOMClass_somNew(_ORB))) 
	#endif /* NewORB */
	#ifndef ORB_object_to_string
		#define ORB_object_to_string(somSelf,ev,obj) \
			SOM_Resolve(somSelf,ORB,object_to_string)  \
				(somSelf,ev,obj)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__object_to_string
				#if defined(_object_to_string)
					#undef _object_to_string
					#define SOMGD__object_to_string
				#else
					#define _object_to_string ORB_object_to_string
				#endif
			#endif /* SOMGD__object_to_string */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ORB_object_to_string */
	#ifndef ORB_string_to_object
		#define ORB_string_to_object(somSelf,ev,str) \
			SOM_Resolve(somSelf,ORB,string_to_object)  \
				(somSelf,ev,str)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__string_to_object
				#if defined(_string_to_object)
					#undef _string_to_object
					#define SOMGD__string_to_object
				#else
					#define _string_to_object ORB_string_to_object
				#endif
			#endif /* SOMGD__string_to_object */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ORB_string_to_object */
	#ifndef ORB_create_list
		#define ORB_create_list(somSelf,ev,count,new_list) \
			SOM_Resolve(somSelf,ORB,create_list)  \
				(somSelf,ev,count,new_list)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__create_list
				#if defined(_create_list)
					#undef _create_list
					#define SOMGD__create_list
				#else
					#define _create_list ORB_create_list
				#endif
			#endif /* SOMGD__create_list */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ORB_create_list */
	#ifndef ORB_create_operation_list
		#define ORB_create_operation_list(somSelf,ev,oper,new_list) \
			SOM_Resolve(somSelf,ORB,create_operation_list)  \
				(somSelf,ev,oper,new_list)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__create_operation_list
				#if defined(_create_operation_list)
					#undef _create_operation_list
					#define SOMGD__create_operation_list
				#else
					#define _create_operation_list ORB_create_operation_list
				#endif
			#endif /* SOMGD__create_operation_list */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ORB_create_operation_list */
	#ifndef ORB_get_default_context
		#define ORB_get_default_context(somSelf,ev,ctx) \
			SOM_Resolve(somSelf,ORB,get_default_context)  \
				(somSelf,ev,ctx)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__get_default_context
				#if defined(_get_default_context)
					#undef _get_default_context
					#define SOMGD__get_default_context
				#else
					#define _get_default_context ORB_get_default_context
				#endif
			#endif /* SOMGD__get_default_context */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ORB_get_default_context */
	#ifndef ORB_list_initial_services
		#define ORB_list_initial_services(somSelf,ev) \
			SOM_Resolve(somSelf,ORB,list_initial_services)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__list_initial_services
				#if defined(_list_initial_services)
					#undef _list_initial_services
					#define SOMGD__list_initial_services
				#else
					#define _list_initial_services ORB_list_initial_services
				#endif
			#endif /* SOMGD__list_initial_services */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ORB_list_initial_services */
	#ifndef ORB_resolve_initial_references
		#define ORB_resolve_initial_references(somSelf,ev,identifier) \
			SOM_Resolve(somSelf,ORB,resolve_initial_references)  \
				(somSelf,ev,identifier)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__resolve_initial_references
				#if defined(_resolve_initial_references)
					#undef _resolve_initial_references
					#define SOMGD__resolve_initial_references
				#else
					#define _resolve_initial_references ORB_resolve_initial_references
				#endif
			#endif /* SOMGD__resolve_initial_references */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ORB_resolve_initial_references */
	#ifndef ORB__set_stringToObject30
		#define ORB__set_stringToObject30(somSelf,ev,stringToObject30) \
			SOM_Resolve(somSelf,ORB,_set_stringToObject30)  \
				(somSelf,ev,stringToObject30)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_stringToObject30
				#if defined(__set_stringToObject30)
					#undef __set_stringToObject30
					#define SOMGD___set_stringToObject30
				#else
					#define __set_stringToObject30 ORB__set_stringToObject30
				#endif
			#endif /* SOMGD___set_stringToObject30 */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ORB__set_stringToObject30 */
	#ifndef ORB__get_stringToObject30
		#define ORB__get_stringToObject30(somSelf,ev) \
			SOM_Resolve(somSelf,ORB,_get_stringToObject30)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_stringToObject30
				#if defined(__get_stringToObject30)
					#undef __get_stringToObject30
					#define SOMGD___get_stringToObject30
				#else
					#define __get_stringToObject30 ORB__get_stringToObject30
				#endif
			#endif /* SOMGD___get_stringToObject30 */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ORB__get_stringToObject30 */
	#ifndef ORB_work_pending
		#define ORB_work_pending(somSelf,ev) \
			SOM_Resolve(somSelf,ORB,work_pending)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__work_pending
				#if defined(_work_pending)
					#undef _work_pending
					#define SOMGD__work_pending
				#else
					#define _work_pending ORB_work_pending
				#endif
			#endif /* SOMGD__work_pending */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ORB_work_pending */
	#ifndef ORB_perform_work
		#define ORB_perform_work(somSelf,ev) \
			SOM_Resolve(somSelf,ORB,perform_work)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__perform_work
				#if defined(_perform_work)
					#undef _perform_work
					#define SOMGD__perform_work
				#else
					#define _perform_work ORB_perform_work
				#endif
			#endif /* SOMGD__perform_work */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ORB_perform_work */
	#ifndef ORB_shutdown
		#define ORB_shutdown(somSelf,ev,wait_for_completion) \
			SOM_Resolve(somSelf,ORB,shutdown)  \
				(somSelf,ev,wait_for_completion)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__shutdown
				#if defined(_shutdown)
					#undef _shutdown
					#define SOMGD__shutdown
				#else
					#define _shutdown ORB_shutdown
				#endif
			#endif /* SOMGD__shutdown */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ORB_shutdown */
	#ifndef ORB_run
		#define ORB_run(somSelf,ev) \
			SOM_Resolve(somSelf,ORB,run)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__run
				#if defined(_run)
					#undef _run
					#define SOMGD__run
				#else
					#define _run ORB_run
				#endif
			#endif /* SOMGD__run */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ORB_run */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define ORB_somInit SOMObject_somInit
#define ORB_somUninit SOMObject_somUninit
#define ORB_somFree SOMObject_somFree
#define ORB_somGetClass SOMObject_somGetClass
#define ORB_somGetClassName SOMObject_somGetClassName
#define ORB_somGetSize SOMObject_somGetSize
#define ORB_somIsA SOMObject_somIsA
#define ORB_somIsInstanceOf SOMObject_somIsInstanceOf
#define ORB_somRespondsTo SOMObject_somRespondsTo
#define ORB_somDispatch SOMObject_somDispatch
#define ORB_somClassDispatch SOMObject_somClassDispatch
#define ORB_somCastObj SOMObject_somCastObj
#define ORB_somResetObj SOMObject_somResetObj
#define ORB_somPrintSelf SOMObject_somPrintSelf
#define ORB_somDumpSelf SOMObject_somDumpSelf
#define ORB_somDumpSelfInt SOMObject_somDumpSelfInt
#define ORB_somDefaultInit SOMObject_somDefaultInit
#define ORB_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define ORB_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define ORB_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define ORB_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define ORB_somDefaultAssign SOMObject_somDefaultAssign
#define ORB_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define ORB_somDefaultVAssign SOMObject_somDefaultVAssign
#define ORB_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define ORB_somDestruct SOMObject_somDestruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_orb_Header_h */
