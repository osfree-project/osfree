/* generated from attribdf.idl */
/* internal conditional is SOM_Module_attribdf_Source */
#ifndef SOM_Module_attribdf_Header_h
	#define SOM_Module_attribdf_Header_h 1
	#include <som.h>
	#include <containd.h>
	#include <somobj.h>
	#ifndef _IDL_AttributeDef_defined
		#define _IDL_AttributeDef_defined
		typedef SOMObject AttributeDef;
	#endif /* _IDL_AttributeDef_defined */
	#ifndef _IDL_SEQUENCE_AttributeDef_defined
		#define _IDL_SEQUENCE_AttributeDef_defined
		SOM_SEQUENCE_TYPEDEF_NAME(AttributeDef SOMSTAR ,sequence(AttributeDef));
	#endif /* _IDL_SEQUENCE_AttributeDef_defined */
	#ifndef AttributeDef_AttributeMode_defined
		#define AttributeDef_AttributeMode_defined
		typedef unsigned long AttributeDef_AttributeMode;
		#ifndef AttributeDef_NORMAL
			#define AttributeDef_NORMAL 1UL
		#endif /* AttributeDef_NORMAL */
		#ifndef AttributeDef_READONLY
			#define AttributeDef_READONLY 2UL
		#endif /* AttributeDef_READONLY */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD_AttributeMode
				#if defined(AttributeMode)
					#undef AttributeMode
					#define SOMGD_AttributeMode
				#else
					#define AttributeMode AttributeDef_AttributeMode
				#endif
			#endif /* SOMGD_AttributeMode */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* AttributeDef_AttributeMode_defined */
	typedef struct AttributeDef_AttributeDescription
	{
		Identifier name;
		RepositoryId id;
		RepositoryId defined_in;
		TypeCode type;
		AttributeDef_AttributeMode mode;
	} AttributeDef_AttributeDescription;
	#ifndef _IDL_SEQUENCE_AttributeDef_AttributeDescription_defined
		#define _IDL_SEQUENCE_AttributeDef_AttributeDescription_defined
		SOM_SEQUENCE_TYPEDEF(AttributeDef_AttributeDescription);
	#endif /* _IDL_SEQUENCE_AttributeDef_AttributeDescription_defined */
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_AttributeDescription
			#if defined(AttributeDescription)
				#undef AttributeDescription
				#define SOMGD_AttributeDescription
			#else
				#define AttributeDescription AttributeDef_AttributeDescription
			#endif
		#endif /* SOMGD_AttributeDescription */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef _IDL_SEQUENCE_AttributeDescription_defined
			#define _IDL_SEQUENCE_AttributeDescription_defined
			#define _IDL_SEQUENCE_AttributeDescription _IDL_SEQUENCE_AttributeDef_AttributeDescription
		#endif /* _IDL_SEQUENCE_AttributeDescription_defined */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
	#ifdef __IBMC__
		typedef void (somTP_AttributeDef__set_type)(
			AttributeDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ TypeCode type);
		#pragma linkage(somTP_AttributeDef__set_type,system)
		typedef somTP_AttributeDef__set_type *somTD_AttributeDef__set_type;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_AttributeDef__set_type)(
			AttributeDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ TypeCode type);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef TypeCode (somTP_AttributeDef__get_type)(
			AttributeDef SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_AttributeDef__get_type,system)
		typedef somTP_AttributeDef__get_type *somTD_AttributeDef__get_type;
	#else /* __IBMC__ */
		typedef TypeCode (SOMLINK * somTD_AttributeDef__get_type)(
			AttributeDef SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_AttributeDef__set_mode)(
			AttributeDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ AttributeDef_AttributeMode mode);
		#pragma linkage(somTP_AttributeDef__set_mode,system)
		typedef somTP_AttributeDef__set_mode *somTD_AttributeDef__set_mode;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_AttributeDef__set_mode)(
			AttributeDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ AttributeDef_AttributeMode mode);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef AttributeDef_AttributeMode (somTP_AttributeDef__get_mode)(
			AttributeDef SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_AttributeDef__get_mode,system)
		typedef somTP_AttributeDef__get_mode *somTD_AttributeDef__get_mode;
	#else /* __IBMC__ */
		typedef AttributeDef_AttributeMode (SOMLINK * somTD_AttributeDef__get_mode)(
			AttributeDef SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifndef AttributeDef_MajorVersion
		#define AttributeDef_MajorVersion   2
	#endif /* AttributeDef_MajorVersion */
	#ifndef AttributeDef_MinorVersion
		#define AttributeDef_MinorVersion   1
	#endif /* AttributeDef_MinorVersion */
	typedef struct AttributeDefClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken _get_type;
		somMToken _set_type;
		somMToken _get_mode;
		somMToken _set_mode;
	} AttributeDefClassDataStructure;
	typedef struct AttributeDefCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} AttributeDefCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_attribdf_Source) || defined(AttributeDef_Class_Source)
			SOMEXTERN struct AttributeDefClassDataStructure _AttributeDefClassData;
			#ifndef AttributeDefClassData
				#define AttributeDefClassData    _AttributeDefClassData
			#endif /* AttributeDefClassData */
		#else
			SOMEXTERN struct AttributeDefClassDataStructure * SOMLINK resolve_AttributeDefClassData(void);
			#ifndef AttributeDefClassData
				#define AttributeDefClassData    (*(resolve_AttributeDefClassData()))
			#endif /* AttributeDefClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_attribdf_Source) || defined(AttributeDef_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_attribdf_Source || AttributeDef_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_attribdf_Source || AttributeDef_Class_Source */
		struct AttributeDefClassDataStructure SOMDLINK AttributeDefClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_attribdf_Source) || defined(AttributeDef_Class_Source)
			SOMEXTERN struct AttributeDefCClassDataStructure _AttributeDefCClassData;
			#ifndef AttributeDefCClassData
				#define AttributeDefCClassData    _AttributeDefCClassData
			#endif /* AttributeDefCClassData */
		#else
			SOMEXTERN struct AttributeDefCClassDataStructure * SOMLINK resolve_AttributeDefCClassData(void);
			#ifndef AttributeDefCClassData
				#define AttributeDefCClassData    (*(resolve_AttributeDefCClassData()))
			#endif /* AttributeDefCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_attribdf_Source) || defined(AttributeDef_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_attribdf_Source || AttributeDef_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_attribdf_Source || AttributeDef_Class_Source */
		struct AttributeDefCClassDataStructure SOMDLINK AttributeDefCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_attribdf_Source) || defined(AttributeDef_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_attribdf_Source || AttributeDef_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_attribdf_Source || AttributeDef_Class_Source */
	SOMClass SOMSTAR SOMLINK AttributeDefNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_AttributeDef (AttributeDefClassData.classObject)
	#ifndef SOMGD_AttributeDef
		#if (defined(_AttributeDef) || defined(__AttributeDef))
			#undef _AttributeDef
			#undef __AttributeDef
			#define SOMGD_AttributeDef 1
		#else
			#define _AttributeDef _SOMCLASS_AttributeDef
		#endif /* _AttributeDef */
	#endif /* SOMGD_AttributeDef */
	#define AttributeDef_classObj _SOMCLASS_AttributeDef
	#define _SOMMTOKEN_AttributeDef(method) ((somMToken)(AttributeDefClassData.method))
	#ifndef AttributeDefNew
		#define AttributeDefNew() ( _AttributeDef ? \
			(SOMClass_somNew(_AttributeDef)) : \
			( AttributeDefNewClass( \
				AttributeDef_MajorVersion, \
				AttributeDef_MinorVersion), \
			SOMClass_somNew(_AttributeDef))) 
	#endif /* NewAttributeDef */
	#ifndef AttributeDef__set_type
		#define AttributeDef__set_type(somSelf,ev,type) \
			SOM_Resolve(somSelf,AttributeDef,_set_type)  \
				(somSelf,ev,type)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_type
				#if defined(__set_type)
					#undef __set_type
					#define SOMGD___set_type
				#else
					#define __set_type AttributeDef__set_type
				#endif
			#endif /* SOMGD___set_type */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* AttributeDef__set_type */
	#ifndef AttributeDef__get_type
		#define AttributeDef__get_type(somSelf,ev) \
			SOM_Resolve(somSelf,AttributeDef,_get_type)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_type
				#if defined(__get_type)
					#undef __get_type
					#define SOMGD___get_type
				#else
					#define __get_type AttributeDef__get_type
				#endif
			#endif /* SOMGD___get_type */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* AttributeDef__get_type */
	#ifndef AttributeDef__set_mode
		#define AttributeDef__set_mode(somSelf,ev,mode) \
			SOM_Resolve(somSelf,AttributeDef,_set_mode)  \
				(somSelf,ev,mode)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_mode
				#if defined(__set_mode)
					#undef __set_mode
					#define SOMGD___set_mode
				#else
					#define __set_mode AttributeDef__set_mode
				#endif
			#endif /* SOMGD___set_mode */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* AttributeDef__set_mode */
	#ifndef AttributeDef__get_mode
		#define AttributeDef__get_mode(somSelf,ev) \
			SOM_Resolve(somSelf,AttributeDef,_get_mode)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_mode
				#if defined(__get_mode)
					#undef __get_mode
					#define SOMGD___get_mode
				#else
					#define __get_mode AttributeDef__get_mode
				#endif
			#endif /* SOMGD___get_mode */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* AttributeDef__get_mode */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define AttributeDef_somInit SOMObject_somInit
#define AttributeDef_somUninit SOMObject_somUninit
#define AttributeDef_somFree SOMObject_somFree
#define AttributeDef_somGetClass SOMObject_somGetClass
#define AttributeDef_somGetClassName SOMObject_somGetClassName
#define AttributeDef_somGetSize SOMObject_somGetSize
#define AttributeDef_somIsA SOMObject_somIsA
#define AttributeDef_somIsInstanceOf SOMObject_somIsInstanceOf
#define AttributeDef_somRespondsTo SOMObject_somRespondsTo
#define AttributeDef_somDispatch SOMObject_somDispatch
#define AttributeDef_somClassDispatch SOMObject_somClassDispatch
#define AttributeDef_somCastObj SOMObject_somCastObj
#define AttributeDef_somResetObj SOMObject_somResetObj
#define AttributeDef_somPrintSelf SOMObject_somPrintSelf
#define AttributeDef_somDumpSelf SOMObject_somDumpSelf
#define AttributeDef_somDumpSelfInt SOMObject_somDumpSelfInt
#define AttributeDef_somDefaultInit SOMObject_somDefaultInit
#define AttributeDef_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define AttributeDef_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define AttributeDef_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define AttributeDef_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define AttributeDef_somDefaultAssign SOMObject_somDefaultAssign
#define AttributeDef_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define AttributeDef_somDefaultVAssign SOMObject_somDefaultVAssign
#define AttributeDef_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define AttributeDef_somDestruct SOMObject_somDestruct
#define AttributeDef__set_defined_in Contained__set_defined_in
#define AttributeDef__get_defined_in Contained__get_defined_in
#define AttributeDef__set_name Contained__set_name
#define AttributeDef__get_name Contained__get_name
#define AttributeDef__set_id Contained__set_id
#define AttributeDef__get_id Contained__get_id
#define AttributeDef__set_somModifiers Contained__set_somModifiers
#define AttributeDef__get_somModifiers Contained__get_somModifiers
#define AttributeDef_within Contained_within
#define AttributeDef_describe Contained_describe
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_attribdf_Header_h */
