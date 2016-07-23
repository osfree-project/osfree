/* generated from constdef.idl */
/* internal conditional is SOM_Module_constdef_Source */
#ifndef SOM_Module_constdef_Header_h
	#define SOM_Module_constdef_Header_h 1
	#include <som.h>
	#include <containd.h>
	#include <somobj.h>
	#ifndef _IDL_ConstantDef_defined
		#define _IDL_ConstantDef_defined
		typedef SOMObject ConstantDef;
	#endif /* _IDL_ConstantDef_defined */
	#ifndef _IDL_SEQUENCE_ConstantDef_defined
		#define _IDL_SEQUENCE_ConstantDef_defined
		SOM_SEQUENCE_TYPEDEF_NAME(ConstantDef SOMSTAR ,sequence(ConstantDef));
	#endif /* _IDL_SEQUENCE_ConstantDef_defined */
	typedef struct ConstantDef_ConstantDescription
	{
		Identifier name;
		RepositoryId id;
		RepositoryId defined_in;
		TypeCode type;
		any value;
	} ConstantDef_ConstantDescription;
	#ifndef _IDL_SEQUENCE_ConstantDef_ConstantDescription_defined
		#define _IDL_SEQUENCE_ConstantDef_ConstantDescription_defined
		SOM_SEQUENCE_TYPEDEF(ConstantDef_ConstantDescription);
	#endif /* _IDL_SEQUENCE_ConstantDef_ConstantDescription_defined */
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_ConstantDescription
			#if defined(ConstantDescription)
				#undef ConstantDescription
				#define SOMGD_ConstantDescription
			#else
				#define ConstantDescription ConstantDef_ConstantDescription
			#endif
		#endif /* SOMGD_ConstantDescription */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef _IDL_SEQUENCE_ConstantDescription_defined
			#define _IDL_SEQUENCE_ConstantDescription_defined
			#define _IDL_SEQUENCE_ConstantDescription _IDL_SEQUENCE_ConstantDef_ConstantDescription
		#endif /* _IDL_SEQUENCE_ConstantDescription_defined */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
	#ifdef __IBMC__
		typedef void (somTP_ConstantDef__set_type)(
			ConstantDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ TypeCode type);
		#pragma linkage(somTP_ConstantDef__set_type,system)
		typedef somTP_ConstantDef__set_type *somTD_ConstantDef__set_type;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_ConstantDef__set_type)(
			ConstantDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ TypeCode type);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef TypeCode (somTP_ConstantDef__get_type)(
			ConstantDef SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_ConstantDef__get_type,system)
		typedef somTP_ConstantDef__get_type *somTD_ConstantDef__get_type;
	#else /* __IBMC__ */
		typedef TypeCode (SOMLINK * somTD_ConstantDef__get_type)(
			ConstantDef SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_ConstantDef__set_value)(
			ConstantDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ any *value);
		#pragma linkage(somTP_ConstantDef__set_value,system)
		typedef somTP_ConstantDef__set_value *somTD_ConstantDef__set_value;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_ConstantDef__set_value)(
			ConstantDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ any *value);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef any (somTP_ConstantDef__get_value)(
			ConstantDef SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_ConstantDef__get_value,system)
		typedef somTP_ConstantDef__get_value *somTD_ConstantDef__get_value;
	#else /* __IBMC__ */
		typedef any (SOMLINK * somTD_ConstantDef__get_value)(
			ConstantDef SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifndef ConstantDef_MajorVersion
		#define ConstantDef_MajorVersion   2
	#endif /* ConstantDef_MajorVersion */
	#ifndef ConstantDef_MinorVersion
		#define ConstantDef_MinorVersion   1
	#endif /* ConstantDef_MinorVersion */
	typedef struct ConstantDefClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken _get_type;
		somMToken _set_type;
		somMToken _get_value;
		somMToken _set_value;
	} ConstantDefClassDataStructure;
	typedef struct ConstantDefCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} ConstantDefCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_constdef_Source) || defined(ConstantDef_Class_Source)
			SOMEXTERN struct ConstantDefClassDataStructure _ConstantDefClassData;
			#ifndef ConstantDefClassData
				#define ConstantDefClassData    _ConstantDefClassData
			#endif /* ConstantDefClassData */
		#else
			SOMEXTERN struct ConstantDefClassDataStructure * SOMLINK resolve_ConstantDefClassData(void);
			#ifndef ConstantDefClassData
				#define ConstantDefClassData    (*(resolve_ConstantDefClassData()))
			#endif /* ConstantDefClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_constdef_Source) || defined(ConstantDef_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_constdef_Source || ConstantDef_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_constdef_Source || ConstantDef_Class_Source */
		struct ConstantDefClassDataStructure SOMDLINK ConstantDefClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_constdef_Source) || defined(ConstantDef_Class_Source)
			SOMEXTERN struct ConstantDefCClassDataStructure _ConstantDefCClassData;
			#ifndef ConstantDefCClassData
				#define ConstantDefCClassData    _ConstantDefCClassData
			#endif /* ConstantDefCClassData */
		#else
			SOMEXTERN struct ConstantDefCClassDataStructure * SOMLINK resolve_ConstantDefCClassData(void);
			#ifndef ConstantDefCClassData
				#define ConstantDefCClassData    (*(resolve_ConstantDefCClassData()))
			#endif /* ConstantDefCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_constdef_Source) || defined(ConstantDef_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_constdef_Source || ConstantDef_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_constdef_Source || ConstantDef_Class_Source */
		struct ConstantDefCClassDataStructure SOMDLINK ConstantDefCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_constdef_Source) || defined(ConstantDef_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_constdef_Source || ConstantDef_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_constdef_Source || ConstantDef_Class_Source */
	SOMClass SOMSTAR SOMLINK ConstantDefNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_ConstantDef (ConstantDefClassData.classObject)
	#ifndef SOMGD_ConstantDef
		#if (defined(_ConstantDef) || defined(__ConstantDef))
			#undef _ConstantDef
			#undef __ConstantDef
			#define SOMGD_ConstantDef 1
		#else
			#define _ConstantDef _SOMCLASS_ConstantDef
		#endif /* _ConstantDef */
	#endif /* SOMGD_ConstantDef */
	#define ConstantDef_classObj _SOMCLASS_ConstantDef
	#define _SOMMTOKEN_ConstantDef(method) ((somMToken)(ConstantDefClassData.method))
	#ifndef ConstantDefNew
		#define ConstantDefNew() ( _ConstantDef ? \
			(SOMClass_somNew(_ConstantDef)) : \
			( ConstantDefNewClass( \
				ConstantDef_MajorVersion, \
				ConstantDef_MinorVersion), \
			SOMClass_somNew(_ConstantDef))) 
	#endif /* NewConstantDef */
	#ifndef ConstantDef__set_type
		#define ConstantDef__set_type(somSelf,ev,type) \
			SOM_Resolve(somSelf,ConstantDef,_set_type)  \
				(somSelf,ev,type)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_type
				#if defined(__set_type)
					#undef __set_type
					#define SOMGD___set_type
				#else
					#define __set_type ConstantDef__set_type
				#endif
			#endif /* SOMGD___set_type */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ConstantDef__set_type */
	#ifndef ConstantDef__get_type
		#define ConstantDef__get_type(somSelf,ev) \
			SOM_Resolve(somSelf,ConstantDef,_get_type)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_type
				#if defined(__get_type)
					#undef __get_type
					#define SOMGD___get_type
				#else
					#define __get_type ConstantDef__get_type
				#endif
			#endif /* SOMGD___get_type */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ConstantDef__get_type */
	#ifndef ConstantDef__set_value
		#define ConstantDef__set_value(somSelf,ev,value) \
			SOM_Resolve(somSelf,ConstantDef,_set_value)  \
				(somSelf,ev,value)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_value
				#if defined(__set_value)
					#undef __set_value
					#define SOMGD___set_value
				#else
					#define __set_value ConstantDef__set_value
				#endif
			#endif /* SOMGD___set_value */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ConstantDef__set_value */
	#ifndef ConstantDef__get_value
		#define ConstantDef__get_value(somSelf,ev) \
			SOM_Resolve(somSelf,ConstantDef,_get_value)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_value
				#if defined(__get_value)
					#undef __get_value
					#define SOMGD___get_value
				#else
					#define __get_value ConstantDef__get_value
				#endif
			#endif /* SOMGD___get_value */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ConstantDef__get_value */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define ConstantDef_somInit SOMObject_somInit
#define ConstantDef_somUninit SOMObject_somUninit
#define ConstantDef_somFree SOMObject_somFree
#define ConstantDef_somGetClass SOMObject_somGetClass
#define ConstantDef_somGetClassName SOMObject_somGetClassName
#define ConstantDef_somGetSize SOMObject_somGetSize
#define ConstantDef_somIsA SOMObject_somIsA
#define ConstantDef_somIsInstanceOf SOMObject_somIsInstanceOf
#define ConstantDef_somRespondsTo SOMObject_somRespondsTo
#define ConstantDef_somDispatch SOMObject_somDispatch
#define ConstantDef_somClassDispatch SOMObject_somClassDispatch
#define ConstantDef_somCastObj SOMObject_somCastObj
#define ConstantDef_somResetObj SOMObject_somResetObj
#define ConstantDef_somPrintSelf SOMObject_somPrintSelf
#define ConstantDef_somDumpSelf SOMObject_somDumpSelf
#define ConstantDef_somDumpSelfInt SOMObject_somDumpSelfInt
#define ConstantDef_somDefaultInit SOMObject_somDefaultInit
#define ConstantDef_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define ConstantDef_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define ConstantDef_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define ConstantDef_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define ConstantDef_somDefaultAssign SOMObject_somDefaultAssign
#define ConstantDef_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define ConstantDef_somDefaultVAssign SOMObject_somDefaultVAssign
#define ConstantDef_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define ConstantDef_somDestruct SOMObject_somDestruct
#define ConstantDef__set_defined_in Contained__set_defined_in
#define ConstantDef__get_defined_in Contained__get_defined_in
#define ConstantDef__set_name Contained__set_name
#define ConstantDef__get_name Contained__get_name
#define ConstantDef__set_id Contained__set_id
#define ConstantDef__get_id Contained__get_id
#define ConstantDef__set_somModifiers Contained__set_somModifiers
#define ConstantDef__get_somModifiers Contained__get_somModifiers
#define ConstantDef_within Contained_within
#define ConstantDef_describe Contained_describe
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_constdef_Header_h */
