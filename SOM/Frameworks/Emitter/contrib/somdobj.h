/* generated from somdobj.idl */
/* internal conditional is SOM_Module_somdobj_Source */
#ifndef SOM_Module_somdobj_Header_h
	#define SOM_Module_somdobj_Header_h 1
	#include <som.h>
	#include <somobj.h>
	#include <unotypes.h>
	#include <containd.h>
	#include <somdtype.h>
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
	#ifndef _IDL_NVList_defined
		#define _IDL_NVList_defined
		typedef SOMObject NVList;
	#endif /* _IDL_NVList_defined */
	#ifndef _IDL_SEQUENCE_NVList_defined
		#define _IDL_SEQUENCE_NVList_defined
		SOM_SEQUENCE_TYPEDEF_NAME(NVList SOMSTAR ,sequence(NVList));
	#endif /* _IDL_SEQUENCE_NVList_defined */
	#ifndef _IDL_Request_defined
		#define _IDL_Request_defined
		typedef SOMObject Request;
	#endif /* _IDL_Request_defined */
	#ifndef _IDL_SEQUENCE_Request_defined
		#define _IDL_SEQUENCE_Request_defined
		SOM_SEQUENCE_TYPEDEF_NAME(Request SOMSTAR ,sequence(Request));
	#endif /* _IDL_SEQUENCE_Request_defined */
	#ifndef _IDL_Context_defined
		#define _IDL_Context_defined
		typedef SOMObject Context;
	#endif /* _IDL_Context_defined */
	#ifndef _IDL_SEQUENCE_Context_defined
		#define _IDL_SEQUENCE_Context_defined
		SOM_SEQUENCE_TYPEDEF_NAME(Context SOMSTAR ,sequence(Context));
	#endif /* _IDL_SEQUENCE_Context_defined */
	#ifndef _IDL_SOMDObject_defined
		#define _IDL_SOMDObject_defined
		typedef SOMObject SOMDObject;
	#endif /* _IDL_SOMDObject_defined */
	#ifndef _IDL_SEQUENCE_SOMDObject_defined
		#define _IDL_SEQUENCE_SOMDObject_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMDObject SOMSTAR ,sequence(SOMDObject));
	#endif /* _IDL_SEQUENCE_SOMDObject_defined */
	#ifdef __IBMC__
		typedef boolean (somTP_SOMDObject_is_nil)(
			SOMDObject SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_SOMDObject_is_nil,system)
		typedef somTP_SOMDObject_is_nil *somTD_SOMDObject_is_nil;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_SOMDObject_is_nil)(
			SOMDObject SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef boolean (somTP_SOMDObject_is_SOM_ref)(
			SOMDObject SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_SOMDObject_is_SOM_ref,system)
		typedef somTP_SOMDObject_is_SOM_ref *somTD_SOMDObject_is_SOM_ref;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_SOMDObject_is_SOM_ref)(
			SOMDObject SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef boolean (somTP_SOMDObject_is_constant)(
			SOMDObject SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_SOMDObject_is_constant,system)
		typedef somTP_SOMDObject_is_constant *somTD_SOMDObject_is_constant;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_SOMDObject_is_constant)(
			SOMDObject SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef ImplementationDef SOMSTAR (somTP_SOMDObject_get_implementation)(
			SOMDObject SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_SOMDObject_get_implementation,system)
		typedef somTP_SOMDObject_get_implementation *somTD_SOMDObject_get_implementation;
	#else /* __IBMC__ */
		typedef ImplementationDef SOMSTAR (SOMLINK * somTD_SOMDObject_get_implementation)(
			SOMDObject SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef InterfaceDef SOMSTAR (somTP_SOMDObject_get_interface)(
			SOMDObject SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_SOMDObject_get_interface,system)
		typedef somTP_SOMDObject_get_interface *somTD_SOMDObject_get_interface;
	#else /* __IBMC__ */
		typedef InterfaceDef SOMSTAR (SOMLINK * somTD_SOMDObject_get_interface)(
			SOMDObject SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef boolean (somTP_SOMDObject_is_proxy)(
			SOMDObject SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_SOMDObject_is_proxy,system)
		typedef somTP_SOMDObject_is_proxy *somTD_SOMDObject_is_proxy;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_SOMDObject_is_proxy)(
			SOMDObject SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMObject SOMSTAR (somTP_SOMDObject_duplicate)(
			SOMDObject SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_SOMDObject_duplicate,system)
		typedef somTP_SOMDObject_duplicate *somTD_SOMDObject_duplicate;
	#else /* __IBMC__ */
		typedef SOMObject SOMSTAR (SOMLINK * somTD_SOMDObject_duplicate)(
			SOMDObject SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMDObject_release)(
			SOMDObject SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_SOMDObject_release,system)
		typedef somTP_SOMDObject_release *somTD_SOMDObject_release;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMDObject_release)(
			SOMDObject SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef ORBStatus (somTP_SOMDObject_create_request)(
			SOMDObject SOMSTAR somSelf,
			Environment *ev,
			/* in */ Context SOMSTAR ctx,
			/* in */ Identifier operation,
			/* in */ NVList SOMSTAR arg_list,
			/* inout */ NamedValue *result,
			/* out */ Request SOMSTAR *request,
			/* in */ Flags req_flags);
		#pragma linkage(somTP_SOMDObject_create_request,system)
		typedef somTP_SOMDObject_create_request *somTD_SOMDObject_create_request;
	#else /* __IBMC__ */
		typedef ORBStatus (SOMLINK * somTD_SOMDObject_create_request)(
			SOMDObject SOMSTAR somSelf,
			Environment *ev,
			/* in */ Context SOMSTAR ctx,
			/* in */ Identifier operation,
			/* in */ NVList SOMSTAR arg_list,
			/* inout */ NamedValue *result,
			/* out */ Request SOMSTAR *request,
			/* in */ Flags req_flags);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef ORBStatus (somTP_SOMDObject_create_request_args)(
			SOMDObject SOMSTAR somSelf,
			Environment *ev,
			/* in */ Identifier operation,
			/* out */ NVList SOMSTAR *arg_list,
			/* out */ NamedValue *result);
		#pragma linkage(somTP_SOMDObject_create_request_args,system)
		typedef somTP_SOMDObject_create_request_args *somTD_SOMDObject_create_request_args;
	#else /* __IBMC__ */
		typedef ORBStatus (SOMLINK * somTD_SOMDObject_create_request_args)(
			SOMDObject SOMSTAR somSelf,
			Environment *ev,
			/* in */ Identifier operation,
			/* out */ NVList SOMSTAR *arg_list,
			/* out */ NamedValue *result);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMDObject__set_type_id)(
			SOMDObject SOMSTAR somSelf,
			Environment *ev,
			/* in */ string type_id);
		#pragma linkage(somTP_SOMDObject__set_type_id,system)
		typedef somTP_SOMDObject__set_type_id *somTD_SOMDObject__set_type_id;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMDObject__set_type_id)(
			SOMDObject SOMSTAR somSelf,
			Environment *ev,
			/* in */ string type_id);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_SOMDObject__get_type_id)(
			SOMDObject SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_SOMDObject__get_type_id,system)
		typedef somTP_SOMDObject__get_type_id *somTD_SOMDObject__get_type_id;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_SOMDObject__get_type_id)(
			SOMDObject SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifndef SOMDObject_MajorVersion
		#define SOMDObject_MajorVersion   2
	#endif /* SOMDObject_MajorVersion */
	#ifndef SOMDObject_MinorVersion
		#define SOMDObject_MinorVersion   1
	#endif /* SOMDObject_MinorVersion */
	typedef struct SOMDObjectClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken get_implementation;
		somMToken get_interface;
		somMToken get_c_proxy;
		somTD_SOMDObject_is_nil is_nil;
		somMToken is_SOM_ref;
		somMToken is_constant;
		somMToken is_proxy;
		somMToken duplicate;
		somMToken release;
		somMToken create_request;
		somMToken create_request_args;
		somMToken _get_somd_tag;
		somMToken _get_somd_flags;
		somMToken _set_somd_flags;
		somMToken _get_somd_impl;
		somMToken _set_somd_impl;
		somMToken _get_somd_rid;
		somMToken _set_somd_rid;
		somMToken _get_somd_id;
		somMToken _set_somd_id;
		somMToken somdMarshal;
		somMToken set_to_nil;
		somMToken _get_type_id;
		somMToken _set_type_id;
	} SOMDObjectClassDataStructure;
	typedef struct SOMDObjectCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} SOMDObjectCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_somdobj_Source) || defined(SOMDObject_Class_Source)
			SOMEXTERN struct SOMDObjectClassDataStructure _SOMDObjectClassData;
			#ifndef SOMDObjectClassData
				#define SOMDObjectClassData    _SOMDObjectClassData
			#endif /* SOMDObjectClassData */
		#else
			SOMEXTERN struct SOMDObjectClassDataStructure * SOMLINK resolve_SOMDObjectClassData(void);
			#ifndef SOMDObjectClassData
				#define SOMDObjectClassData    (*(resolve_SOMDObjectClassData()))
			#endif /* SOMDObjectClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_somdobj_Source) || defined(SOMDObject_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_somdobj_Source || SOMDObject_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_somdobj_Source || SOMDObject_Class_Source */
		struct SOMDObjectClassDataStructure SOMDLINK SOMDObjectClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_somdobj_Source) || defined(SOMDObject_Class_Source)
			SOMEXTERN struct SOMDObjectCClassDataStructure _SOMDObjectCClassData;
			#ifndef SOMDObjectCClassData
				#define SOMDObjectCClassData    _SOMDObjectCClassData
			#endif /* SOMDObjectCClassData */
		#else
			SOMEXTERN struct SOMDObjectCClassDataStructure * SOMLINK resolve_SOMDObjectCClassData(void);
			#ifndef SOMDObjectCClassData
				#define SOMDObjectCClassData    (*(resolve_SOMDObjectCClassData()))
			#endif /* SOMDObjectCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_somdobj_Source) || defined(SOMDObject_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_somdobj_Source || SOMDObject_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_somdobj_Source || SOMDObject_Class_Source */
		struct SOMDObjectCClassDataStructure SOMDLINK SOMDObjectCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_somdobj_Source) || defined(SOMDObject_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_somdobj_Source || SOMDObject_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_somdobj_Source || SOMDObject_Class_Source */
	SOMClass SOMSTAR SOMLINK SOMDObjectNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_SOMDObject (SOMDObjectClassData.classObject)
	#ifndef SOMGD_SOMDObject
		#if (defined(_SOMDObject) || defined(__SOMDObject))
			#undef _SOMDObject
			#undef __SOMDObject
			#define SOMGD_SOMDObject 1
		#else
			#define _SOMDObject _SOMCLASS_SOMDObject
		#endif /* _SOMDObject */
	#endif /* SOMGD_SOMDObject */
	#define SOMDObject_classObj _SOMCLASS_SOMDObject
	#define _SOMMTOKEN_SOMDObject(method) ((somMToken)(SOMDObjectClassData.method))
	#ifndef SOMDObjectNew
		#define SOMDObjectNew() ( _SOMDObject ? \
			(SOMClass_somNew(_SOMDObject)) : \
			( SOMDObjectNewClass( \
				SOMDObject_MajorVersion, \
				SOMDObject_MinorVersion), \
			SOMClass_somNew(_SOMDObject))) 
	#endif /* NewSOMDObject */
	#ifndef SOMDObject_is_nil
		#define SOMDObject_is_nil SOMDObjectClassData.is_nil
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__is_nil
				#if defined(_is_nil)
					#undef _is_nil
					#define SOMGD__is_nil
				#else
					#define _is_nil SOMDObject_is_nil
				#endif
			#endif /* SOMGD__is_nil */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDObject_is_nil */
	#ifndef SOMDObject_is_SOM_ref
		#define SOMDObject_is_SOM_ref(somSelf,ev) \
			SOM_Resolve(somSelf,SOMDObject,is_SOM_ref)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__is_SOM_ref
				#if defined(_is_SOM_ref)
					#undef _is_SOM_ref
					#define SOMGD__is_SOM_ref
				#else
					#define _is_SOM_ref SOMDObject_is_SOM_ref
				#endif
			#endif /* SOMGD__is_SOM_ref */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDObject_is_SOM_ref */
	#ifndef SOMDObject_is_constant
		#define SOMDObject_is_constant(somSelf,ev) \
			SOM_Resolve(somSelf,SOMDObject,is_constant)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__is_constant
				#if defined(_is_constant)
					#undef _is_constant
					#define SOMGD__is_constant
				#else
					#define _is_constant SOMDObject_is_constant
				#endif
			#endif /* SOMGD__is_constant */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDObject_is_constant */
	#ifndef SOMDObject_get_implementation
		#define SOMDObject_get_implementation(somSelf,ev) \
			SOM_Resolve(somSelf,SOMDObject,get_implementation)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__get_implementation
				#if defined(_get_implementation)
					#undef _get_implementation
					#define SOMGD__get_implementation
				#else
					#define _get_implementation SOMDObject_get_implementation
				#endif
			#endif /* SOMGD__get_implementation */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDObject_get_implementation */
	#ifndef SOMDObject_get_interface
		#define SOMDObject_get_interface(somSelf,ev) \
			SOM_Resolve(somSelf,SOMDObject,get_interface)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__get_interface
				#if defined(_get_interface)
					#undef _get_interface
					#define SOMGD__get_interface
				#else
					#define _get_interface SOMDObject_get_interface
				#endif
			#endif /* SOMGD__get_interface */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDObject_get_interface */
	#ifndef SOMDObject_is_proxy
		#define SOMDObject_is_proxy(somSelf,ev) \
			SOM_Resolve(somSelf,SOMDObject,is_proxy)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__is_proxy
				#if defined(_is_proxy)
					#undef _is_proxy
					#define SOMGD__is_proxy
				#else
					#define _is_proxy SOMDObject_is_proxy
				#endif
			#endif /* SOMGD__is_proxy */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDObject_is_proxy */
	#ifndef SOMDObject_duplicate
		#define SOMDObject_duplicate(somSelf,ev) \
			SOM_Resolve(somSelf,SOMDObject,duplicate)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__duplicate
				#if defined(_duplicate)
					#undef _duplicate
					#define SOMGD__duplicate
				#else
					#define _duplicate SOMDObject_duplicate
				#endif
			#endif /* SOMGD__duplicate */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDObject_duplicate */
	#ifndef SOMDObject_release
		#define SOMDObject_release(somSelf,ev) \
			SOM_Resolve(somSelf,SOMDObject,release)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__release
				#if defined(_release)
					#undef _release
					#define SOMGD__release
				#else
					#define _release SOMDObject_release
				#endif
			#endif /* SOMGD__release */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDObject_release */
	#ifndef SOMDObject_create_request
		#define SOMDObject_create_request(somSelf,ev,ctx,operation,arg_list,result,request,req_flags) \
			SOM_Resolve(somSelf,SOMDObject,create_request)  \
				(somSelf,ev,ctx,operation,arg_list,result,request,req_flags)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__create_request
				#if defined(_create_request)
					#undef _create_request
					#define SOMGD__create_request
				#else
					#define _create_request SOMDObject_create_request
				#endif
			#endif /* SOMGD__create_request */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDObject_create_request */
	#ifndef SOMDObject_create_request_args
		#define SOMDObject_create_request_args(somSelf,ev,operation,arg_list,result) \
			SOM_Resolve(somSelf,SOMDObject,create_request_args)  \
				(somSelf,ev,operation,arg_list,result)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__create_request_args
				#if defined(_create_request_args)
					#undef _create_request_args
					#define SOMGD__create_request_args
				#else
					#define _create_request_args SOMDObject_create_request_args
				#endif
			#endif /* SOMGD__create_request_args */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDObject_create_request_args */
	#ifndef SOMDObject__set_type_id
		#define SOMDObject__set_type_id(somSelf,ev,type_id) \
			SOM_Resolve(somSelf,SOMDObject,_set_type_id)  \
				(somSelf,ev,type_id)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_type_id
				#if defined(__set_type_id)
					#undef __set_type_id
					#define SOMGD___set_type_id
				#else
					#define __set_type_id SOMDObject__set_type_id
				#endif
			#endif /* SOMGD___set_type_id */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDObject__set_type_id */
	#ifndef SOMDObject__get_type_id
		#define SOMDObject__get_type_id(somSelf,ev) \
			SOM_Resolve(somSelf,SOMDObject,_get_type_id)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_type_id
				#if defined(__get_type_id)
					#undef __get_type_id
					#define SOMGD___get_type_id
				#else
					#define __get_type_id SOMDObject__get_type_id
				#endif
			#endif /* SOMGD___get_type_id */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDObject__get_type_id */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define SOMDObject_somInit SOMObject_somInit
#define SOMDObject_somUninit SOMObject_somUninit
#define SOMDObject_somFree SOMObject_somFree
#define SOMDObject_somGetClass SOMObject_somGetClass
#define SOMDObject_somGetClassName SOMObject_somGetClassName
#define SOMDObject_somGetSize SOMObject_somGetSize
#define SOMDObject_somIsA SOMObject_somIsA
#define SOMDObject_somIsInstanceOf SOMObject_somIsInstanceOf
#define SOMDObject_somRespondsTo SOMObject_somRespondsTo
#define SOMDObject_somDispatch SOMObject_somDispatch
#define SOMDObject_somClassDispatch SOMObject_somClassDispatch
#define SOMDObject_somCastObj SOMObject_somCastObj
#define SOMDObject_somResetObj SOMObject_somResetObj
#define SOMDObject_somPrintSelf SOMObject_somPrintSelf
#define SOMDObject_somDumpSelf SOMObject_somDumpSelf
#define SOMDObject_somDumpSelfInt SOMObject_somDumpSelfInt
#define SOMDObject_somDefaultInit SOMObject_somDefaultInit
#define SOMDObject_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define SOMDObject_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define SOMDObject_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define SOMDObject_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define SOMDObject_somDefaultAssign SOMObject_somDefaultAssign
#define SOMDObject_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define SOMDObject_somDefaultVAssign SOMObject_somDefaultVAssign
#define SOMDObject_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define SOMDObject_somDestruct SOMObject_somDestruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_somdobj_Header_h */
