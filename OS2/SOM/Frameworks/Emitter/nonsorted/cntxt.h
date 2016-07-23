/* generated from cntxt.idl */
/* internal conditional is SOM_Module_cntxt_Source */
#ifndef SOM_Module_cntxt_Header_h
	#define SOM_Module_cntxt_Header_h 1
	#include <som.h>
	#include <somobj.h>
	#include <somdtype.h>
	#include <containd.h>
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
	#ifdef __IBMC__
		typedef ORBStatus (somTP_Context_set_one_value)(
			Context SOMSTAR somSelf,
			Environment *ev,
			/* in */ Identifier prop_name,
			/* in */ string value);
		#pragma linkage(somTP_Context_set_one_value,system)
		typedef somTP_Context_set_one_value *somTD_Context_set_one_value;
	#else /* __IBMC__ */
		typedef ORBStatus (SOMLINK * somTD_Context_set_one_value)(
			Context SOMSTAR somSelf,
			Environment *ev,
			/* in */ Identifier prop_name,
			/* in */ string value);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef ORBStatus (somTP_Context_set_values)(
			Context SOMSTAR somSelf,
			Environment *ev,
			/* in */ NVList SOMSTAR values);
		#pragma linkage(somTP_Context_set_values,system)
		typedef somTP_Context_set_values *somTD_Context_set_values;
	#else /* __IBMC__ */
		typedef ORBStatus (SOMLINK * somTD_Context_set_values)(
			Context SOMSTAR somSelf,
			Environment *ev,
			/* in */ NVList SOMSTAR values);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef ORBStatus (somTP_Context_get_values)(
			Context SOMSTAR somSelf,
			Environment *ev,
			/* in */ Identifier start_scope,
			/* in */ Flags op_flags,
			/* in */ Identifier prop_name,
			/* out */ NVList SOMSTAR *values);
		#pragma linkage(somTP_Context_get_values,system)
		typedef somTP_Context_get_values *somTD_Context_get_values;
	#else /* __IBMC__ */
		typedef ORBStatus (SOMLINK * somTD_Context_get_values)(
			Context SOMSTAR somSelf,
			Environment *ev,
			/* in */ Identifier start_scope,
			/* in */ Flags op_flags,
			/* in */ Identifier prop_name,
			/* out */ NVList SOMSTAR *values);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef ORBStatus (somTP_Context_delete_values)(
			Context SOMSTAR somSelf,
			Environment *ev,
			/* in */ Identifier prop_name);
		#pragma linkage(somTP_Context_delete_values,system)
		typedef somTP_Context_delete_values *somTD_Context_delete_values;
	#else /* __IBMC__ */
		typedef ORBStatus (SOMLINK * somTD_Context_delete_values)(
			Context SOMSTAR somSelf,
			Environment *ev,
			/* in */ Identifier prop_name);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef ORBStatus (somTP_Context_create_child)(
			Context SOMSTAR somSelf,
			Environment *ev,
			/* in */ Identifier ctx_name,
			/* out */ Context SOMSTAR *child_ctx);
		#pragma linkage(somTP_Context_create_child,system)
		typedef somTP_Context_create_child *somTD_Context_create_child;
	#else /* __IBMC__ */
		typedef ORBStatus (SOMLINK * somTD_Context_create_child)(
			Context SOMSTAR somSelf,
			Environment *ev,
			/* in */ Identifier ctx_name,
			/* out */ Context SOMSTAR *child_ctx);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef ORBStatus (somTP_Context_destroy)(
			Context SOMSTAR somSelf,
			Environment *ev,
			/* in */ Flags flags);
		#pragma linkage(somTP_Context_destroy,system)
		typedef somTP_Context_destroy *somTD_Context_destroy;
	#else /* __IBMC__ */
		typedef ORBStatus (SOMLINK * somTD_Context_destroy)(
			Context SOMSTAR somSelf,
			Environment *ev,
			/* in */ Flags flags);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_Context__set_context_name)(
			Context SOMSTAR somSelf,
			Environment *ev,
			/* in */ string context_name);
		#pragma linkage(somTP_Context__set_context_name,system)
		typedef somTP_Context__set_context_name *somTD_Context__set_context_name;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_Context__set_context_name)(
			Context SOMSTAR somSelf,
			Environment *ev,
			/* in */ string context_name);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_Context__get_context_name)(
			Context SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_Context__get_context_name,system)
		typedef somTP_Context__get_context_name *somTD_Context__get_context_name;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_Context__get_context_name)(
			Context SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_Context__set_context_parent)(
			Context SOMSTAR somSelf,
			Environment *ev,
			/* in */ Context SOMSTAR context_parent);
		#pragma linkage(somTP_Context__set_context_parent,system)
		typedef somTP_Context__set_context_parent *somTD_Context__set_context_parent;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_Context__set_context_parent)(
			Context SOMSTAR somSelf,
			Environment *ev,
			/* in */ Context SOMSTAR context_parent);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef Context SOMSTAR (somTP_Context__get_context_parent)(
			Context SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_Context__get_context_parent,system)
		typedef somTP_Context__get_context_parent *somTD_Context__get_context_parent;
	#else /* __IBMC__ */
		typedef Context SOMSTAR (SOMLINK * somTD_Context__get_context_parent)(
			Context SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifndef Context_MajorVersion
		#define Context_MajorVersion   2
	#endif /* Context_MajorVersion */
	#ifndef Context_MinorVersion
		#define Context_MinorVersion   2
	#endif /* Context_MinorVersion */
	typedef struct ContextClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken set_one_value;
		somMToken set_values;
		somMToken get_values;
		somMToken delete_values;
		somMToken create_child;
		somMToken destroy;
		somMToken _get_context_name;
		somMToken _set_context_name;
		somMToken _get_context_parent;
		somMToken _set_context_parent;
		somMToken private10;
		somMToken private11;
	} ContextClassDataStructure;
	typedef struct ContextCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} ContextCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_cntxt_Source) || defined(Context_Class_Source)
			SOMEXTERN struct ContextClassDataStructure _ContextClassData;
			#ifndef ContextClassData
				#define ContextClassData    _ContextClassData
			#endif /* ContextClassData */
		#else
			SOMEXTERN struct ContextClassDataStructure * SOMLINK resolve_ContextClassData(void);
			#ifndef ContextClassData
				#define ContextClassData    (*(resolve_ContextClassData()))
			#endif /* ContextClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_cntxt_Source) || defined(Context_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_cntxt_Source || Context_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_cntxt_Source || Context_Class_Source */
		struct ContextClassDataStructure SOMDLINK ContextClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_cntxt_Source) || defined(Context_Class_Source)
			SOMEXTERN struct ContextCClassDataStructure _ContextCClassData;
			#ifndef ContextCClassData
				#define ContextCClassData    _ContextCClassData
			#endif /* ContextCClassData */
		#else
			SOMEXTERN struct ContextCClassDataStructure * SOMLINK resolve_ContextCClassData(void);
			#ifndef ContextCClassData
				#define ContextCClassData    (*(resolve_ContextCClassData()))
			#endif /* ContextCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_cntxt_Source) || defined(Context_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_cntxt_Source || Context_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_cntxt_Source || Context_Class_Source */
		struct ContextCClassDataStructure SOMDLINK ContextCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_cntxt_Source) || defined(Context_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_cntxt_Source || Context_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_cntxt_Source || Context_Class_Source */
	SOMClass SOMSTAR SOMLINK ContextNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_Context (ContextClassData.classObject)
	#ifndef SOMGD_Context
		#if (defined(_Context) || defined(__Context))
			#undef _Context
			#undef __Context
			#define SOMGD_Context 1
		#else
			#define _Context _SOMCLASS_Context
		#endif /* _Context */
	#endif /* SOMGD_Context */
	#define Context_classObj _SOMCLASS_Context
	#define _SOMMTOKEN_Context(method) ((somMToken)(ContextClassData.method))
	#ifndef ContextNew
		#define ContextNew() ( _Context ? \
			(SOMClass_somNew(_Context)) : \
			( ContextNewClass( \
				Context_MajorVersion, \
				Context_MinorVersion), \
			SOMClass_somNew(_Context))) 
	#endif /* NewContext */
	#ifndef Context_set_one_value
		#define Context_set_one_value(somSelf,ev,prop_name,value) \
			SOM_Resolve(somSelf,Context,set_one_value)  \
				(somSelf,ev,prop_name,value)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__set_one_value
				#if defined(_set_one_value)
					#undef _set_one_value
					#define SOMGD__set_one_value
				#else
					#define _set_one_value Context_set_one_value
				#endif
			#endif /* SOMGD__set_one_value */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* Context_set_one_value */
	#ifndef Context_set_values
		#define Context_set_values(somSelf,ev,values) \
			SOM_Resolve(somSelf,Context,set_values)  \
				(somSelf,ev,values)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__set_values
				#if defined(_set_values)
					#undef _set_values
					#define SOMGD__set_values
				#else
					#define _set_values Context_set_values
				#endif
			#endif /* SOMGD__set_values */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* Context_set_values */
	#ifndef Context_get_values
		#define Context_get_values(somSelf,ev,start_scope,op_flags,prop_name,values) \
			SOM_Resolve(somSelf,Context,get_values)  \
				(somSelf,ev,start_scope,op_flags,prop_name,values)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__get_values
				#if defined(_get_values)
					#undef _get_values
					#define SOMGD__get_values
				#else
					#define _get_values Context_get_values
				#endif
			#endif /* SOMGD__get_values */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* Context_get_values */
	#ifndef Context_delete_values
		#define Context_delete_values(somSelf,ev,prop_name) \
			SOM_Resolve(somSelf,Context,delete_values)  \
				(somSelf,ev,prop_name)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__delete_values
				#if defined(_delete_values)
					#undef _delete_values
					#define SOMGD__delete_values
				#else
					#define _delete_values Context_delete_values
				#endif
			#endif /* SOMGD__delete_values */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* Context_delete_values */
	#ifndef Context_create_child
		#define Context_create_child(somSelf,ev,ctx_name,child_ctx) \
			SOM_Resolve(somSelf,Context,create_child)  \
				(somSelf,ev,ctx_name,child_ctx)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__create_child
				#if defined(_create_child)
					#undef _create_child
					#define SOMGD__create_child
				#else
					#define _create_child Context_create_child
				#endif
			#endif /* SOMGD__create_child */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* Context_create_child */
	#ifndef Context_destroy
		#define Context_destroy(somSelf,ev,flags) \
			SOM_Resolve(somSelf,Context,destroy)  \
				(somSelf,ev,flags)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__destroy
				#if defined(_destroy)
					#undef _destroy
					#define SOMGD__destroy
				#else
					#define _destroy Context_destroy
				#endif
			#endif /* SOMGD__destroy */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* Context_destroy */
	#ifndef Context__set_context_name
		#define Context__set_context_name(somSelf,ev,context_name) \
			SOM_Resolve(somSelf,Context,_set_context_name)  \
				(somSelf,ev,context_name)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_context_name
				#if defined(__set_context_name)
					#undef __set_context_name
					#define SOMGD___set_context_name
				#else
					#define __set_context_name Context__set_context_name
				#endif
			#endif /* SOMGD___set_context_name */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* Context__set_context_name */
	#ifndef Context__get_context_name
		#define Context__get_context_name(somSelf,ev) \
			SOM_Resolve(somSelf,Context,_get_context_name)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_context_name
				#if defined(__get_context_name)
					#undef __get_context_name
					#define SOMGD___get_context_name
				#else
					#define __get_context_name Context__get_context_name
				#endif
			#endif /* SOMGD___get_context_name */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* Context__get_context_name */
	#ifndef Context__set_context_parent
		#define Context__set_context_parent(somSelf,ev,context_parent) \
			SOM_Resolve(somSelf,Context,_set_context_parent)  \
				(somSelf,ev,context_parent)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_context_parent
				#if defined(__set_context_parent)
					#undef __set_context_parent
					#define SOMGD___set_context_parent
				#else
					#define __set_context_parent Context__set_context_parent
				#endif
			#endif /* SOMGD___set_context_parent */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* Context__set_context_parent */
	#ifndef Context__get_context_parent
		#define Context__get_context_parent(somSelf,ev) \
			SOM_Resolve(somSelf,Context,_get_context_parent)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_context_parent
				#if defined(__get_context_parent)
					#undef __get_context_parent
					#define SOMGD___get_context_parent
				#else
					#define __get_context_parent Context__get_context_parent
				#endif
			#endif /* SOMGD___get_context_parent */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* Context__get_context_parent */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define Context_somInit SOMObject_somInit
#define Context_somUninit SOMObject_somUninit
#define Context_somFree SOMObject_somFree
#define Context_somGetClass SOMObject_somGetClass
#define Context_somGetClassName SOMObject_somGetClassName
#define Context_somGetSize SOMObject_somGetSize
#define Context_somIsA SOMObject_somIsA
#define Context_somIsInstanceOf SOMObject_somIsInstanceOf
#define Context_somRespondsTo SOMObject_somRespondsTo
#define Context_somDispatch SOMObject_somDispatch
#define Context_somClassDispatch SOMObject_somClassDispatch
#define Context_somCastObj SOMObject_somCastObj
#define Context_somResetObj SOMObject_somResetObj
#define Context_somPrintSelf SOMObject_somPrintSelf
#define Context_somDumpSelf SOMObject_somDumpSelf
#define Context_somDumpSelfInt SOMObject_somDumpSelfInt
#define Context_somDefaultInit SOMObject_somDefaultInit
#define Context_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define Context_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define Context_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define Context_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define Context_somDefaultAssign SOMObject_somDefaultAssign
#define Context_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define Context_somDefaultVAssign SOMObject_somDefaultVAssign
#define Context_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define Context_somDestruct SOMObject_somDestruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#define CTX_DELETE_DESCENDENTS      0x00000200
#define CTX_RESTRICT_SCOPE          0x00000400
#ifndef Context_delete
#define Context_delete Context_destroy
#endif

#endif /* SOM_Module_cntxt_Header_h */
