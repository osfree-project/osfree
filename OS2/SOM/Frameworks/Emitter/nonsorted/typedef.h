/* generated from typedef.idl */
/* internal conditional is SOM_Module_typedef_Source */
#ifndef SOM_Module_typedef_Header_h
	#define SOM_Module_typedef_Header_h 1
	#include <som.h>
	#include <containd.h>
	#include <somobj.h>
	#ifndef _IDL_TypeDef_defined
		#define _IDL_TypeDef_defined
		typedef SOMObject TypeDef;
	#endif /* _IDL_TypeDef_defined */
	#ifndef _IDL_SEQUENCE_TypeDef_defined
		#define _IDL_SEQUENCE_TypeDef_defined
		SOM_SEQUENCE_TYPEDEF_NAME(TypeDef SOMSTAR ,sequence(TypeDef));
	#endif /* _IDL_SEQUENCE_TypeDef_defined */
	typedef struct TypeDef_TypeDescription
	{
		Identifier name;
		RepositoryId id;
		RepositoryId defined_in;
		TypeCode type;
	} TypeDef_TypeDescription;
	#ifndef _IDL_SEQUENCE_TypeDef_TypeDescription_defined
		#define _IDL_SEQUENCE_TypeDef_TypeDescription_defined
		SOM_SEQUENCE_TYPEDEF(TypeDef_TypeDescription);
	#endif /* _IDL_SEQUENCE_TypeDef_TypeDescription_defined */
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_TypeDescription
			#if defined(TypeDescription)
				#undef TypeDescription
				#define SOMGD_TypeDescription
			#else
				#define TypeDescription TypeDef_TypeDescription
			#endif
		#endif /* SOMGD_TypeDescription */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef _IDL_SEQUENCE_TypeDescription_defined
			#define _IDL_SEQUENCE_TypeDescription_defined
			#define _IDL_SEQUENCE_TypeDescription _IDL_SEQUENCE_TypeDef_TypeDescription
		#endif /* _IDL_SEQUENCE_TypeDescription_defined */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
	#ifdef __IBMC__
		typedef void (somTP_TypeDef__set_type)(
			TypeDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ TypeCode type);
		#pragma linkage(somTP_TypeDef__set_type,system)
		typedef somTP_TypeDef__set_type *somTD_TypeDef__set_type;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_TypeDef__set_type)(
			TypeDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ TypeCode type);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef TypeCode (somTP_TypeDef__get_type)(
			TypeDef SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_TypeDef__get_type,system)
		typedef somTP_TypeDef__get_type *somTD_TypeDef__get_type;
	#else /* __IBMC__ */
		typedef TypeCode (SOMLINK * somTD_TypeDef__get_type)(
			TypeDef SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifndef TypeDef_MajorVersion
		#define TypeDef_MajorVersion   2
	#endif /* TypeDef_MajorVersion */
	#ifndef TypeDef_MinorVersion
		#define TypeDef_MinorVersion   1
	#endif /* TypeDef_MinorVersion */
	typedef struct TypeDefClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken _get_type;
		somMToken _set_type;
	} TypeDefClassDataStructure;
	typedef struct TypeDefCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} TypeDefCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_typedef_Source) || defined(TypeDef_Class_Source)
			SOMEXTERN struct TypeDefClassDataStructure _TypeDefClassData;
			#ifndef TypeDefClassData
				#define TypeDefClassData    _TypeDefClassData
			#endif /* TypeDefClassData */
		#else
			SOMEXTERN struct TypeDefClassDataStructure * SOMLINK resolve_TypeDefClassData(void);
			#ifndef TypeDefClassData
				#define TypeDefClassData    (*(resolve_TypeDefClassData()))
			#endif /* TypeDefClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_typedef_Source) || defined(TypeDef_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_typedef_Source || TypeDef_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_typedef_Source || TypeDef_Class_Source */
		struct TypeDefClassDataStructure SOMDLINK TypeDefClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_typedef_Source) || defined(TypeDef_Class_Source)
			SOMEXTERN struct TypeDefCClassDataStructure _TypeDefCClassData;
			#ifndef TypeDefCClassData
				#define TypeDefCClassData    _TypeDefCClassData
			#endif /* TypeDefCClassData */
		#else
			SOMEXTERN struct TypeDefCClassDataStructure * SOMLINK resolve_TypeDefCClassData(void);
			#ifndef TypeDefCClassData
				#define TypeDefCClassData    (*(resolve_TypeDefCClassData()))
			#endif /* TypeDefCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_typedef_Source) || defined(TypeDef_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_typedef_Source || TypeDef_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_typedef_Source || TypeDef_Class_Source */
		struct TypeDefCClassDataStructure SOMDLINK TypeDefCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_typedef_Source) || defined(TypeDef_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_typedef_Source || TypeDef_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_typedef_Source || TypeDef_Class_Source */
	SOMClass SOMSTAR SOMLINK TypeDefNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_TypeDef (TypeDefClassData.classObject)
	#ifndef SOMGD_TypeDef
		#if (defined(_TypeDef) || defined(__TypeDef))
			#undef _TypeDef
			#undef __TypeDef
			#define SOMGD_TypeDef 1
		#else
			#define _TypeDef _SOMCLASS_TypeDef
		#endif /* _TypeDef */
	#endif /* SOMGD_TypeDef */
	#define TypeDef_classObj _SOMCLASS_TypeDef
	#define _SOMMTOKEN_TypeDef(method) ((somMToken)(TypeDefClassData.method))
	#ifndef TypeDefNew
		#define TypeDefNew() ( _TypeDef ? \
			(SOMClass_somNew(_TypeDef)) : \
			( TypeDefNewClass( \
				TypeDef_MajorVersion, \
				TypeDef_MinorVersion), \
			SOMClass_somNew(_TypeDef))) 
	#endif /* NewTypeDef */
	#ifndef TypeDef__set_type
		#define TypeDef__set_type(somSelf,ev,type) \
			SOM_Resolve(somSelf,TypeDef,_set_type)  \
				(somSelf,ev,type)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_type
				#if defined(__set_type)
					#undef __set_type
					#define SOMGD___set_type
				#else
					#define __set_type TypeDef__set_type
				#endif
			#endif /* SOMGD___set_type */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* TypeDef__set_type */
	#ifndef TypeDef__get_type
		#define TypeDef__get_type(somSelf,ev) \
			SOM_Resolve(somSelf,TypeDef,_get_type)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_type
				#if defined(__get_type)
					#undef __get_type
					#define SOMGD___get_type
				#else
					#define __get_type TypeDef__get_type
				#endif
			#endif /* SOMGD___get_type */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* TypeDef__get_type */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define TypeDef_somInit SOMObject_somInit
#define TypeDef_somUninit SOMObject_somUninit
#define TypeDef_somFree SOMObject_somFree
#define TypeDef_somGetClass SOMObject_somGetClass
#define TypeDef_somGetClassName SOMObject_somGetClassName
#define TypeDef_somGetSize SOMObject_somGetSize
#define TypeDef_somIsA SOMObject_somIsA
#define TypeDef_somIsInstanceOf SOMObject_somIsInstanceOf
#define TypeDef_somRespondsTo SOMObject_somRespondsTo
#define TypeDef_somDispatch SOMObject_somDispatch
#define TypeDef_somClassDispatch SOMObject_somClassDispatch
#define TypeDef_somCastObj SOMObject_somCastObj
#define TypeDef_somResetObj SOMObject_somResetObj
#define TypeDef_somPrintSelf SOMObject_somPrintSelf
#define TypeDef_somDumpSelf SOMObject_somDumpSelf
#define TypeDef_somDumpSelfInt SOMObject_somDumpSelfInt
#define TypeDef_somDefaultInit SOMObject_somDefaultInit
#define TypeDef_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define TypeDef_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define TypeDef_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define TypeDef_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define TypeDef_somDefaultAssign SOMObject_somDefaultAssign
#define TypeDef_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define TypeDef_somDefaultVAssign SOMObject_somDefaultVAssign
#define TypeDef_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define TypeDef_somDestruct SOMObject_somDestruct
#define TypeDef__set_defined_in Contained__set_defined_in
#define TypeDef__get_defined_in Contained__get_defined_in
#define TypeDef__set_name Contained__set_name
#define TypeDef__get_name Contained__get_name
#define TypeDef__set_id Contained__set_id
#define TypeDef__get_id Contained__get_id
#define TypeDef__set_somModifiers Contained__set_somModifiers
#define TypeDef__get_somModifiers Contained__get_somModifiers
#define TypeDef_within Contained_within
#define TypeDef_describe Contained_describe
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_typedef_Header_h */
