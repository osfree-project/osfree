/* generated from excptdef.idl */
/* internal conditional is SOM_Module_excptdef_Source */
#ifndef SOM_Module_excptdef_Header_h
	#define SOM_Module_excptdef_Header_h 1
	#include <som.h>
	#include <containd.h>
	#include <somobj.h>
	#ifndef _IDL_ExceptionDef_defined
		#define _IDL_ExceptionDef_defined
		typedef SOMObject ExceptionDef;
	#endif /* _IDL_ExceptionDef_defined */
	#ifndef _IDL_SEQUENCE_ExceptionDef_defined
		#define _IDL_SEQUENCE_ExceptionDef_defined
		SOM_SEQUENCE_TYPEDEF_NAME(ExceptionDef SOMSTAR ,sequence(ExceptionDef));
	#endif /* _IDL_SEQUENCE_ExceptionDef_defined */
	typedef struct ExceptionDef_ExceptionDescription
	{
		Identifier name;
		RepositoryId id;
		RepositoryId defined_in;
		TypeCode type;
	} ExceptionDef_ExceptionDescription;
	#ifndef _IDL_SEQUENCE_ExceptionDef_ExceptionDescription_defined
		#define _IDL_SEQUENCE_ExceptionDef_ExceptionDescription_defined
		SOM_SEQUENCE_TYPEDEF(ExceptionDef_ExceptionDescription);
	#endif /* _IDL_SEQUENCE_ExceptionDef_ExceptionDescription_defined */
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_ExceptionDescription
			#if defined(ExceptionDescription)
				#undef ExceptionDescription
				#define SOMGD_ExceptionDescription
			#else
				#define ExceptionDescription ExceptionDef_ExceptionDescription
			#endif
		#endif /* SOMGD_ExceptionDescription */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef _IDL_SEQUENCE_ExceptionDescription_defined
			#define _IDL_SEQUENCE_ExceptionDescription_defined
			#define _IDL_SEQUENCE_ExceptionDescription _IDL_SEQUENCE_ExceptionDef_ExceptionDescription
		#endif /* _IDL_SEQUENCE_ExceptionDescription_defined */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
	#ifdef __IBMC__
		typedef void (somTP_ExceptionDef__set_type)(
			ExceptionDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ TypeCode type);
		#pragma linkage(somTP_ExceptionDef__set_type,system)
		typedef somTP_ExceptionDef__set_type *somTD_ExceptionDef__set_type;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_ExceptionDef__set_type)(
			ExceptionDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ TypeCode type);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef TypeCode (somTP_ExceptionDef__get_type)(
			ExceptionDef SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_ExceptionDef__get_type,system)
		typedef somTP_ExceptionDef__get_type *somTD_ExceptionDef__get_type;
	#else /* __IBMC__ */
		typedef TypeCode (SOMLINK * somTD_ExceptionDef__get_type)(
			ExceptionDef SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifndef ExceptionDef_MajorVersion
		#define ExceptionDef_MajorVersion   2
	#endif /* ExceptionDef_MajorVersion */
	#ifndef ExceptionDef_MinorVersion
		#define ExceptionDef_MinorVersion   3
	#endif /* ExceptionDef_MinorVersion */
	typedef struct ExceptionDefClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken _get_type;
		somMToken _set_type;
	} ExceptionDefClassDataStructure;
	typedef struct ExceptionDefCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} ExceptionDefCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_excptdef_Source) || defined(ExceptionDef_Class_Source)
			SOMEXTERN struct ExceptionDefClassDataStructure _ExceptionDefClassData;
			#ifndef ExceptionDefClassData
				#define ExceptionDefClassData    _ExceptionDefClassData
			#endif /* ExceptionDefClassData */
		#else
			SOMEXTERN struct ExceptionDefClassDataStructure * SOMLINK resolve_ExceptionDefClassData(void);
			#ifndef ExceptionDefClassData
				#define ExceptionDefClassData    (*(resolve_ExceptionDefClassData()))
			#endif /* ExceptionDefClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_excptdef_Source) || defined(ExceptionDef_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_excptdef_Source || ExceptionDef_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_excptdef_Source || ExceptionDef_Class_Source */
		struct ExceptionDefClassDataStructure SOMDLINK ExceptionDefClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_excptdef_Source) || defined(ExceptionDef_Class_Source)
			SOMEXTERN struct ExceptionDefCClassDataStructure _ExceptionDefCClassData;
			#ifndef ExceptionDefCClassData
				#define ExceptionDefCClassData    _ExceptionDefCClassData
			#endif /* ExceptionDefCClassData */
		#else
			SOMEXTERN struct ExceptionDefCClassDataStructure * SOMLINK resolve_ExceptionDefCClassData(void);
			#ifndef ExceptionDefCClassData
				#define ExceptionDefCClassData    (*(resolve_ExceptionDefCClassData()))
			#endif /* ExceptionDefCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_excptdef_Source) || defined(ExceptionDef_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_excptdef_Source || ExceptionDef_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_excptdef_Source || ExceptionDef_Class_Source */
		struct ExceptionDefCClassDataStructure SOMDLINK ExceptionDefCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_excptdef_Source) || defined(ExceptionDef_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_excptdef_Source || ExceptionDef_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_excptdef_Source || ExceptionDef_Class_Source */
	SOMClass SOMSTAR SOMLINK ExceptionDefNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_ExceptionDef (ExceptionDefClassData.classObject)
	#ifndef SOMGD_ExceptionDef
		#if (defined(_ExceptionDef) || defined(__ExceptionDef))
			#undef _ExceptionDef
			#undef __ExceptionDef
			#define SOMGD_ExceptionDef 1
		#else
			#define _ExceptionDef _SOMCLASS_ExceptionDef
		#endif /* _ExceptionDef */
	#endif /* SOMGD_ExceptionDef */
	#define ExceptionDef_classObj _SOMCLASS_ExceptionDef
	#define _SOMMTOKEN_ExceptionDef(method) ((somMToken)(ExceptionDefClassData.method))
	#ifndef ExceptionDefNew
		#define ExceptionDefNew() ( _ExceptionDef ? \
			(SOMClass_somNew(_ExceptionDef)) : \
			( ExceptionDefNewClass( \
				ExceptionDef_MajorVersion, \
				ExceptionDef_MinorVersion), \
			SOMClass_somNew(_ExceptionDef))) 
	#endif /* NewExceptionDef */
	#ifndef ExceptionDef__set_type
		#define ExceptionDef__set_type(somSelf,ev,type) \
			SOM_Resolve(somSelf,ExceptionDef,_set_type)  \
				(somSelf,ev,type)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_type
				#if defined(__set_type)
					#undef __set_type
					#define SOMGD___set_type
				#else
					#define __set_type ExceptionDef__set_type
				#endif
			#endif /* SOMGD___set_type */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ExceptionDef__set_type */
	#ifndef ExceptionDef__get_type
		#define ExceptionDef__get_type(somSelf,ev) \
			SOM_Resolve(somSelf,ExceptionDef,_get_type)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_type
				#if defined(__get_type)
					#undef __get_type
					#define SOMGD___get_type
				#else
					#define __get_type ExceptionDef__get_type
				#endif
			#endif /* SOMGD___get_type */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ExceptionDef__get_type */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define ExceptionDef_somInit SOMObject_somInit
#define ExceptionDef_somUninit SOMObject_somUninit
#define ExceptionDef_somFree SOMObject_somFree
#define ExceptionDef_somGetClass SOMObject_somGetClass
#define ExceptionDef_somGetClassName SOMObject_somGetClassName
#define ExceptionDef_somGetSize SOMObject_somGetSize
#define ExceptionDef_somIsA SOMObject_somIsA
#define ExceptionDef_somIsInstanceOf SOMObject_somIsInstanceOf
#define ExceptionDef_somRespondsTo SOMObject_somRespondsTo
#define ExceptionDef_somDispatch SOMObject_somDispatch
#define ExceptionDef_somClassDispatch SOMObject_somClassDispatch
#define ExceptionDef_somCastObj SOMObject_somCastObj
#define ExceptionDef_somResetObj SOMObject_somResetObj
#define ExceptionDef_somPrintSelf SOMObject_somPrintSelf
#define ExceptionDef_somDumpSelf SOMObject_somDumpSelf
#define ExceptionDef_somDumpSelfInt SOMObject_somDumpSelfInt
#define ExceptionDef_somDefaultInit SOMObject_somDefaultInit
#define ExceptionDef_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define ExceptionDef_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define ExceptionDef_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define ExceptionDef_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define ExceptionDef_somDefaultAssign SOMObject_somDefaultAssign
#define ExceptionDef_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define ExceptionDef_somDefaultVAssign SOMObject_somDefaultVAssign
#define ExceptionDef_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define ExceptionDef_somDestruct SOMObject_somDestruct
#define ExceptionDef__set_defined_in Contained__set_defined_in
#define ExceptionDef__get_defined_in Contained__get_defined_in
#define ExceptionDef__set_name Contained__set_name
#define ExceptionDef__get_name Contained__get_name
#define ExceptionDef__set_id Contained__set_id
#define ExceptionDef__get_id Contained__get_id
#define ExceptionDef__set_somModifiers Contained__set_somModifiers
#define ExceptionDef__get_somModifiers Contained__get_somModifiers
#define ExceptionDef_within Contained_within
#define ExceptionDef_describe Contained_describe
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_excptdef_Header_h */
