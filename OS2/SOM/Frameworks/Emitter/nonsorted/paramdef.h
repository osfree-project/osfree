/* generated from paramdef.idl */
/* internal conditional is SOM_Module_paramdef_Source */
#ifndef SOM_Module_paramdef_Header_h
	#define SOM_Module_paramdef_Header_h 1
	#include <som.h>
	#include <containd.h>
	#include <somobj.h>
	#ifndef _IDL_ParameterDef_defined
		#define _IDL_ParameterDef_defined
		typedef SOMObject ParameterDef;
	#endif /* _IDL_ParameterDef_defined */
	#ifndef _IDL_SEQUENCE_ParameterDef_defined
		#define _IDL_SEQUENCE_ParameterDef_defined
		SOM_SEQUENCE_TYPEDEF_NAME(ParameterDef SOMSTAR ,sequence(ParameterDef));
	#endif /* _IDL_SEQUENCE_ParameterDef_defined */
	#ifndef ParameterDef_ParameterMode_defined
		#define ParameterDef_ParameterMode_defined
		typedef unsigned long ParameterDef_ParameterMode;
		#ifndef ParameterDef_IN
			#define ParameterDef_IN 1UL
		#endif /* ParameterDef_IN */
		#ifndef ParameterDef_OUT
			#define ParameterDef_OUT 2UL
		#endif /* ParameterDef_OUT */
		#ifndef ParameterDef_INOUT
			#define ParameterDef_INOUT 3UL
		#endif /* ParameterDef_INOUT */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD_ParameterMode
				#if defined(ParameterMode)
					#undef ParameterMode
					#define SOMGD_ParameterMode
				#else
					#define ParameterMode ParameterDef_ParameterMode
				#endif
			#endif /* SOMGD_ParameterMode */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ParameterDef_ParameterMode_defined */
	typedef struct ParameterDef_ParameterDescription
	{
		Identifier name;
		RepositoryId id;
		RepositoryId defined_in;
		TypeCode type;
		ParameterDef_ParameterMode mode;
	} ParameterDef_ParameterDescription;
	#ifndef _IDL_SEQUENCE_ParameterDef_ParameterDescription_defined
		#define _IDL_SEQUENCE_ParameterDef_ParameterDescription_defined
		SOM_SEQUENCE_TYPEDEF(ParameterDef_ParameterDescription);
	#endif /* _IDL_SEQUENCE_ParameterDef_ParameterDescription_defined */
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_ParameterDescription
			#if defined(ParameterDescription)
				#undef ParameterDescription
				#define SOMGD_ParameterDescription
			#else
				#define ParameterDescription ParameterDef_ParameterDescription
			#endif
		#endif /* SOMGD_ParameterDescription */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef _IDL_SEQUENCE_ParameterDescription_defined
			#define _IDL_SEQUENCE_ParameterDescription_defined
			#define _IDL_SEQUENCE_ParameterDescription _IDL_SEQUENCE_ParameterDef_ParameterDescription
		#endif /* _IDL_SEQUENCE_ParameterDescription_defined */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
	#ifdef __IBMC__
		typedef void (somTP_ParameterDef__set_type)(
			ParameterDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ TypeCode type);
		#pragma linkage(somTP_ParameterDef__set_type,system)
		typedef somTP_ParameterDef__set_type *somTD_ParameterDef__set_type;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_ParameterDef__set_type)(
			ParameterDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ TypeCode type);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef TypeCode (somTP_ParameterDef__get_type)(
			ParameterDef SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_ParameterDef__get_type,system)
		typedef somTP_ParameterDef__get_type *somTD_ParameterDef__get_type;
	#else /* __IBMC__ */
		typedef TypeCode (SOMLINK * somTD_ParameterDef__get_type)(
			ParameterDef SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_ParameterDef__set_mode)(
			ParameterDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ ParameterDef_ParameterMode mode);
		#pragma linkage(somTP_ParameterDef__set_mode,system)
		typedef somTP_ParameterDef__set_mode *somTD_ParameterDef__set_mode;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_ParameterDef__set_mode)(
			ParameterDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ ParameterDef_ParameterMode mode);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef ParameterDef_ParameterMode (somTP_ParameterDef__get_mode)(
			ParameterDef SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_ParameterDef__get_mode,system)
		typedef somTP_ParameterDef__get_mode *somTD_ParameterDef__get_mode;
	#else /* __IBMC__ */
		typedef ParameterDef_ParameterMode (SOMLINK * somTD_ParameterDef__get_mode)(
			ParameterDef SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifndef ParameterDef_MajorVersion
		#define ParameterDef_MajorVersion   2
	#endif /* ParameterDef_MajorVersion */
	#ifndef ParameterDef_MinorVersion
		#define ParameterDef_MinorVersion   1
	#endif /* ParameterDef_MinorVersion */
	typedef struct ParameterDefClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken _get_type;
		somMToken _set_type;
		somMToken _get_mode;
		somMToken _set_mode;
	} ParameterDefClassDataStructure;
	typedef struct ParameterDefCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} ParameterDefCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_paramdef_Source) || defined(ParameterDef_Class_Source)
			SOMEXTERN struct ParameterDefClassDataStructure _ParameterDefClassData;
			#ifndef ParameterDefClassData
				#define ParameterDefClassData    _ParameterDefClassData
			#endif /* ParameterDefClassData */
		#else
			SOMEXTERN struct ParameterDefClassDataStructure * SOMLINK resolve_ParameterDefClassData(void);
			#ifndef ParameterDefClassData
				#define ParameterDefClassData    (*(resolve_ParameterDefClassData()))
			#endif /* ParameterDefClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_paramdef_Source) || defined(ParameterDef_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_paramdef_Source || ParameterDef_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_paramdef_Source || ParameterDef_Class_Source */
		struct ParameterDefClassDataStructure SOMDLINK ParameterDefClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_paramdef_Source) || defined(ParameterDef_Class_Source)
			SOMEXTERN struct ParameterDefCClassDataStructure _ParameterDefCClassData;
			#ifndef ParameterDefCClassData
				#define ParameterDefCClassData    _ParameterDefCClassData
			#endif /* ParameterDefCClassData */
		#else
			SOMEXTERN struct ParameterDefCClassDataStructure * SOMLINK resolve_ParameterDefCClassData(void);
			#ifndef ParameterDefCClassData
				#define ParameterDefCClassData    (*(resolve_ParameterDefCClassData()))
			#endif /* ParameterDefCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_paramdef_Source) || defined(ParameterDef_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_paramdef_Source || ParameterDef_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_paramdef_Source || ParameterDef_Class_Source */
		struct ParameterDefCClassDataStructure SOMDLINK ParameterDefCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_paramdef_Source) || defined(ParameterDef_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_paramdef_Source || ParameterDef_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_paramdef_Source || ParameterDef_Class_Source */
	SOMClass SOMSTAR SOMLINK ParameterDefNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_ParameterDef (ParameterDefClassData.classObject)
	#ifndef SOMGD_ParameterDef
		#if (defined(_ParameterDef) || defined(__ParameterDef))
			#undef _ParameterDef
			#undef __ParameterDef
			#define SOMGD_ParameterDef 1
		#else
			#define _ParameterDef _SOMCLASS_ParameterDef
		#endif /* _ParameterDef */
	#endif /* SOMGD_ParameterDef */
	#define ParameterDef_classObj _SOMCLASS_ParameterDef
	#define _SOMMTOKEN_ParameterDef(method) ((somMToken)(ParameterDefClassData.method))
	#ifndef ParameterDefNew
		#define ParameterDefNew() ( _ParameterDef ? \
			(SOMClass_somNew(_ParameterDef)) : \
			( ParameterDefNewClass( \
				ParameterDef_MajorVersion, \
				ParameterDef_MinorVersion), \
			SOMClass_somNew(_ParameterDef))) 
	#endif /* NewParameterDef */
	#ifndef ParameterDef__set_type
		#define ParameterDef__set_type(somSelf,ev,type) \
			SOM_Resolve(somSelf,ParameterDef,_set_type)  \
				(somSelf,ev,type)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_type
				#if defined(__set_type)
					#undef __set_type
					#define SOMGD___set_type
				#else
					#define __set_type ParameterDef__set_type
				#endif
			#endif /* SOMGD___set_type */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ParameterDef__set_type */
	#ifndef ParameterDef__get_type
		#define ParameterDef__get_type(somSelf,ev) \
			SOM_Resolve(somSelf,ParameterDef,_get_type)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_type
				#if defined(__get_type)
					#undef __get_type
					#define SOMGD___get_type
				#else
					#define __get_type ParameterDef__get_type
				#endif
			#endif /* SOMGD___get_type */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ParameterDef__get_type */
	#ifndef ParameterDef__set_mode
		#define ParameterDef__set_mode(somSelf,ev,mode) \
			SOM_Resolve(somSelf,ParameterDef,_set_mode)  \
				(somSelf,ev,mode)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_mode
				#if defined(__set_mode)
					#undef __set_mode
					#define SOMGD___set_mode
				#else
					#define __set_mode ParameterDef__set_mode
				#endif
			#endif /* SOMGD___set_mode */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ParameterDef__set_mode */
	#ifndef ParameterDef__get_mode
		#define ParameterDef__get_mode(somSelf,ev) \
			SOM_Resolve(somSelf,ParameterDef,_get_mode)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_mode
				#if defined(__get_mode)
					#undef __get_mode
					#define SOMGD___get_mode
				#else
					#define __get_mode ParameterDef__get_mode
				#endif
			#endif /* SOMGD___get_mode */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ParameterDef__get_mode */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define ParameterDef_somInit SOMObject_somInit
#define ParameterDef_somUninit SOMObject_somUninit
#define ParameterDef_somFree SOMObject_somFree
#define ParameterDef_somGetClass SOMObject_somGetClass
#define ParameterDef_somGetClassName SOMObject_somGetClassName
#define ParameterDef_somGetSize SOMObject_somGetSize
#define ParameterDef_somIsA SOMObject_somIsA
#define ParameterDef_somIsInstanceOf SOMObject_somIsInstanceOf
#define ParameterDef_somRespondsTo SOMObject_somRespondsTo
#define ParameterDef_somDispatch SOMObject_somDispatch
#define ParameterDef_somClassDispatch SOMObject_somClassDispatch
#define ParameterDef_somCastObj SOMObject_somCastObj
#define ParameterDef_somResetObj SOMObject_somResetObj
#define ParameterDef_somPrintSelf SOMObject_somPrintSelf
#define ParameterDef_somDumpSelf SOMObject_somDumpSelf
#define ParameterDef_somDumpSelfInt SOMObject_somDumpSelfInt
#define ParameterDef_somDefaultInit SOMObject_somDefaultInit
#define ParameterDef_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define ParameterDef_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define ParameterDef_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define ParameterDef_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define ParameterDef_somDefaultAssign SOMObject_somDefaultAssign
#define ParameterDef_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define ParameterDef_somDefaultVAssign SOMObject_somDefaultVAssign
#define ParameterDef_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define ParameterDef_somDestruct SOMObject_somDestruct
#define ParameterDef__set_defined_in Contained__set_defined_in
#define ParameterDef__get_defined_in Contained__get_defined_in
#define ParameterDef__set_name Contained__set_name
#define ParameterDef__get_name Contained__get_name
#define ParameterDef__set_id Contained__set_id
#define ParameterDef__get_id Contained__get_id
#define ParameterDef__set_somModifiers Contained__set_somModifiers
#define ParameterDef__get_somModifiers Contained__get_somModifiers
#define ParameterDef_within Contained_within
#define ParameterDef_describe Contained_describe
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_paramdef_Header_h */
