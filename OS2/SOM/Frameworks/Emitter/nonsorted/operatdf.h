/* generated from operatdf.idl */
/* internal conditional is SOM_Module_operatdf_Source */
#ifndef SOM_Module_operatdf_Header_h
	#define SOM_Module_operatdf_Header_h 1
	#include <som.h>
	#include <containd.h>
	#include <somobj.h>
	#include <containr.h>
	#include <paramdef.h>
	#include <excptdef.h>
	#ifndef _IDL_OperationDef_defined
		#define _IDL_OperationDef_defined
		typedef SOMObject OperationDef;
	#endif /* _IDL_OperationDef_defined */
	#ifndef _IDL_SEQUENCE_OperationDef_defined
		#define _IDL_SEQUENCE_OperationDef_defined
		SOM_SEQUENCE_TYPEDEF_NAME(OperationDef SOMSTAR ,sequence(OperationDef));
	#endif /* _IDL_SEQUENCE_OperationDef_defined */
	#ifndef OperationDef_OperationMode_defined
		#define OperationDef_OperationMode_defined
		typedef unsigned long OperationDef_OperationMode;
		#ifndef OperationDef_NORMAL
			#define OperationDef_NORMAL 1UL
		#endif /* OperationDef_NORMAL */
		#ifndef OperationDef_ONEWAY
			#define OperationDef_ONEWAY 2UL
		#endif /* OperationDef_ONEWAY */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD_OperationMode
				#if defined(OperationMode)
					#undef OperationMode
					#define SOMGD_OperationMode
				#else
					#define OperationMode OperationDef_OperationMode
				#endif
			#endif /* SOMGD_OperationMode */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* OperationDef_OperationMode_defined */
	typedef struct OperationDef_OperationDescription
	{
		Identifier name;
		RepositoryId id;
		RepositoryId defined_in;
		TypeCode result;
		OperationDef_OperationMode mode;
		_IDL_SEQUENCE_string contexts;
		_IDL_SEQUENCE_ParameterDef_ParameterDescription parameter;
		_IDL_SEQUENCE_ExceptionDef_ExceptionDescription exceptions;
	} OperationDef_OperationDescription;
	#ifndef _IDL_SEQUENCE_OperationDef_OperationDescription_defined
		#define _IDL_SEQUENCE_OperationDef_OperationDescription_defined
		SOM_SEQUENCE_TYPEDEF(OperationDef_OperationDescription);
	#endif /* _IDL_SEQUENCE_OperationDef_OperationDescription_defined */
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_OperationDescription
			#if defined(OperationDescription)
				#undef OperationDescription
				#define SOMGD_OperationDescription
			#else
				#define OperationDescription OperationDef_OperationDescription
			#endif
		#endif /* SOMGD_OperationDescription */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef _IDL_SEQUENCE_OperationDescription_defined
			#define _IDL_SEQUENCE_OperationDescription_defined
			#define _IDL_SEQUENCE_OperationDescription _IDL_SEQUENCE_OperationDef_OperationDescription
		#endif /* _IDL_SEQUENCE_OperationDescription_defined */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
	#ifdef __IBMC__
		typedef void (somTP_OperationDef__set_result)(
			OperationDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ TypeCode result);
		#pragma linkage(somTP_OperationDef__set_result,system)
		typedef somTP_OperationDef__set_result *somTD_OperationDef__set_result;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_OperationDef__set_result)(
			OperationDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ TypeCode result);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef TypeCode (somTP_OperationDef__get_result)(
			OperationDef SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_OperationDef__get_result,system)
		typedef somTP_OperationDef__get_result *somTD_OperationDef__get_result;
	#else /* __IBMC__ */
		typedef TypeCode (SOMLINK * somTD_OperationDef__get_result)(
			OperationDef SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_OperationDef__set_mode)(
			OperationDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ OperationDef_OperationMode mode);
		#pragma linkage(somTP_OperationDef__set_mode,system)
		typedef somTP_OperationDef__set_mode *somTD_OperationDef__set_mode;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_OperationDef__set_mode)(
			OperationDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ OperationDef_OperationMode mode);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef OperationDef_OperationMode (somTP_OperationDef__get_mode)(
			OperationDef SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_OperationDef__get_mode,system)
		typedef somTP_OperationDef__get_mode *somTD_OperationDef__get_mode;
	#else /* __IBMC__ */
		typedef OperationDef_OperationMode (SOMLINK * somTD_OperationDef__get_mode)(
			OperationDef SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_OperationDef__set_contexts)(
			OperationDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ _IDL_SEQUENCE_string *contexts);
		#pragma linkage(somTP_OperationDef__set_contexts,system)
		typedef somTP_OperationDef__set_contexts *somTD_OperationDef__set_contexts;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_OperationDef__set_contexts)(
			OperationDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ _IDL_SEQUENCE_string *contexts);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef _IDL_SEQUENCE_string (somTP_OperationDef__get_contexts)(
			OperationDef SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_OperationDef__get_contexts,system)
		typedef somTP_OperationDef__get_contexts *somTD_OperationDef__get_contexts;
	#else /* __IBMC__ */
		typedef _IDL_SEQUENCE_string (SOMLINK * somTD_OperationDef__get_contexts)(
			OperationDef SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifndef OperationDef_MajorVersion
		#define OperationDef_MajorVersion   2
	#endif /* OperationDef_MajorVersion */
	#ifndef OperationDef_MinorVersion
		#define OperationDef_MinorVersion   1
	#endif /* OperationDef_MinorVersion */
	typedef struct OperationDefClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken _get_result;
		somMToken _set_result;
		somMToken _get_mode;
		somMToken _set_mode;
		somMToken _get_contexts;
		somMToken _set_contexts;
		somMToken _get_raiseSeq;
		somMToken _set_raiseSeq;
	} OperationDefClassDataStructure;
	typedef struct OperationDefCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} OperationDefCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_operatdf_Source) || defined(OperationDef_Class_Source)
			SOMEXTERN struct OperationDefClassDataStructure _OperationDefClassData;
			#ifndef OperationDefClassData
				#define OperationDefClassData    _OperationDefClassData
			#endif /* OperationDefClassData */
		#else
			SOMEXTERN struct OperationDefClassDataStructure * SOMLINK resolve_OperationDefClassData(void);
			#ifndef OperationDefClassData
				#define OperationDefClassData    (*(resolve_OperationDefClassData()))
			#endif /* OperationDefClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_operatdf_Source) || defined(OperationDef_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_operatdf_Source || OperationDef_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_operatdf_Source || OperationDef_Class_Source */
		struct OperationDefClassDataStructure SOMDLINK OperationDefClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_operatdf_Source) || defined(OperationDef_Class_Source)
			SOMEXTERN struct OperationDefCClassDataStructure _OperationDefCClassData;
			#ifndef OperationDefCClassData
				#define OperationDefCClassData    _OperationDefCClassData
			#endif /* OperationDefCClassData */
		#else
			SOMEXTERN struct OperationDefCClassDataStructure * SOMLINK resolve_OperationDefCClassData(void);
			#ifndef OperationDefCClassData
				#define OperationDefCClassData    (*(resolve_OperationDefCClassData()))
			#endif /* OperationDefCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_operatdf_Source) || defined(OperationDef_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_operatdf_Source || OperationDef_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_operatdf_Source || OperationDef_Class_Source */
		struct OperationDefCClassDataStructure SOMDLINK OperationDefCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_operatdf_Source) || defined(OperationDef_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_operatdf_Source || OperationDef_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_operatdf_Source || OperationDef_Class_Source */
	SOMClass SOMSTAR SOMLINK OperationDefNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_OperationDef (OperationDefClassData.classObject)
	#ifndef SOMGD_OperationDef
		#if (defined(_OperationDef) || defined(__OperationDef))
			#undef _OperationDef
			#undef __OperationDef
			#define SOMGD_OperationDef 1
		#else
			#define _OperationDef _SOMCLASS_OperationDef
		#endif /* _OperationDef */
	#endif /* SOMGD_OperationDef */
	#define OperationDef_classObj _SOMCLASS_OperationDef
	#define _SOMMTOKEN_OperationDef(method) ((somMToken)(OperationDefClassData.method))
	#ifndef OperationDefNew
		#define OperationDefNew() ( _OperationDef ? \
			(SOMClass_somNew(_OperationDef)) : \
			( OperationDefNewClass( \
				OperationDef_MajorVersion, \
				OperationDef_MinorVersion), \
			SOMClass_somNew(_OperationDef))) 
	#endif /* NewOperationDef */
	#ifndef OperationDef__set_result
		#define OperationDef__set_result(somSelf,ev,result) \
			SOM_Resolve(somSelf,OperationDef,_set_result)  \
				(somSelf,ev,result)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_result
				#if defined(__set_result)
					#undef __set_result
					#define SOMGD___set_result
				#else
					#define __set_result OperationDef__set_result
				#endif
			#endif /* SOMGD___set_result */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* OperationDef__set_result */
	#ifndef OperationDef__get_result
		#define OperationDef__get_result(somSelf,ev) \
			SOM_Resolve(somSelf,OperationDef,_get_result)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_result
				#if defined(__get_result)
					#undef __get_result
					#define SOMGD___get_result
				#else
					#define __get_result OperationDef__get_result
				#endif
			#endif /* SOMGD___get_result */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* OperationDef__get_result */
	#ifndef OperationDef__set_mode
		#define OperationDef__set_mode(somSelf,ev,mode) \
			SOM_Resolve(somSelf,OperationDef,_set_mode)  \
				(somSelf,ev,mode)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_mode
				#if defined(__set_mode)
					#undef __set_mode
					#define SOMGD___set_mode
				#else
					#define __set_mode OperationDef__set_mode
				#endif
			#endif /* SOMGD___set_mode */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* OperationDef__set_mode */
	#ifndef OperationDef__get_mode
		#define OperationDef__get_mode(somSelf,ev) \
			SOM_Resolve(somSelf,OperationDef,_get_mode)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_mode
				#if defined(__get_mode)
					#undef __get_mode
					#define SOMGD___get_mode
				#else
					#define __get_mode OperationDef__get_mode
				#endif
			#endif /* SOMGD___get_mode */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* OperationDef__get_mode */
	#ifndef OperationDef__set_contexts
		#define OperationDef__set_contexts(somSelf,ev,contexts) \
			SOM_Resolve(somSelf,OperationDef,_set_contexts)  \
				(somSelf,ev,contexts)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_contexts
				#if defined(__set_contexts)
					#undef __set_contexts
					#define SOMGD___set_contexts
				#else
					#define __set_contexts OperationDef__set_contexts
				#endif
			#endif /* SOMGD___set_contexts */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* OperationDef__set_contexts */
	#ifndef OperationDef__get_contexts
		#define OperationDef__get_contexts(somSelf,ev) \
			SOM_Resolve(somSelf,OperationDef,_get_contexts)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_contexts
				#if defined(__get_contexts)
					#undef __get_contexts
					#define SOMGD___get_contexts
				#else
					#define __get_contexts OperationDef__get_contexts
				#endif
			#endif /* SOMGD___get_contexts */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* OperationDef__get_contexts */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define OperationDef_somInit SOMObject_somInit
#define OperationDef_somUninit SOMObject_somUninit
#define OperationDef_somFree SOMObject_somFree
#define OperationDef_somGetClass SOMObject_somGetClass
#define OperationDef_somGetClassName SOMObject_somGetClassName
#define OperationDef_somGetSize SOMObject_somGetSize
#define OperationDef_somIsA SOMObject_somIsA
#define OperationDef_somIsInstanceOf SOMObject_somIsInstanceOf
#define OperationDef_somRespondsTo SOMObject_somRespondsTo
#define OperationDef_somDispatch SOMObject_somDispatch
#define OperationDef_somClassDispatch SOMObject_somClassDispatch
#define OperationDef_somCastObj SOMObject_somCastObj
#define OperationDef_somResetObj SOMObject_somResetObj
#define OperationDef_somPrintSelf SOMObject_somPrintSelf
#define OperationDef_somDumpSelf SOMObject_somDumpSelf
#define OperationDef_somDumpSelfInt SOMObject_somDumpSelfInt
#define OperationDef_somDefaultInit SOMObject_somDefaultInit
#define OperationDef_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define OperationDef_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define OperationDef_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define OperationDef_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define OperationDef_somDefaultAssign SOMObject_somDefaultAssign
#define OperationDef_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define OperationDef_somDefaultVAssign SOMObject_somDefaultVAssign
#define OperationDef_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define OperationDef_somDestruct SOMObject_somDestruct
#define OperationDef__set_defined_in Contained__set_defined_in
#define OperationDef__get_defined_in Contained__get_defined_in
#define OperationDef__set_name Contained__set_name
#define OperationDef__get_name Contained__get_name
#define OperationDef__set_id Contained__set_id
#define OperationDef__get_id Contained__get_id
#define OperationDef__set_somModifiers Contained__set_somModifiers
#define OperationDef__get_somModifiers Contained__get_somModifiers
#define OperationDef_within Contained_within
#define OperationDef_describe Contained_describe
#define OperationDef_contents Container_contents
#define OperationDef_lookup_name Container_lookup_name
#define OperationDef_describe_contents Container_describe_contents
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_operatdf_Header_h */
