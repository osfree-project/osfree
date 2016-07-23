/* generated from moduledf.idl */
/* internal conditional is SOM_Module_moduledf_Source */
#ifndef SOM_Module_moduledf_Header_h
	#define SOM_Module_moduledf_Header_h 1
	#include <som.h>
	#include <containd.h>
	#include <somobj.h>
	#include <containr.h>
	#ifndef _IDL_ModuleDef_defined
		#define _IDL_ModuleDef_defined
		typedef SOMObject ModuleDef;
	#endif /* _IDL_ModuleDef_defined */
	#ifndef _IDL_SEQUENCE_ModuleDef_defined
		#define _IDL_SEQUENCE_ModuleDef_defined
		SOM_SEQUENCE_TYPEDEF_NAME(ModuleDef SOMSTAR ,sequence(ModuleDef));
	#endif /* _IDL_SEQUENCE_ModuleDef_defined */
	typedef struct ModuleDef_ModuleDescription
	{
		Identifier name;
		RepositoryId id;
		RepositoryId defined_in;
	} ModuleDef_ModuleDescription;
	#ifndef _IDL_SEQUENCE_ModuleDef_ModuleDescription_defined
		#define _IDL_SEQUENCE_ModuleDef_ModuleDescription_defined
		SOM_SEQUENCE_TYPEDEF(ModuleDef_ModuleDescription);
	#endif /* _IDL_SEQUENCE_ModuleDef_ModuleDescription_defined */
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_ModuleDescription
			#if defined(ModuleDescription)
				#undef ModuleDescription
				#define SOMGD_ModuleDescription
			#else
				#define ModuleDescription ModuleDef_ModuleDescription
			#endif
		#endif /* SOMGD_ModuleDescription */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef _IDL_SEQUENCE_ModuleDescription_defined
			#define _IDL_SEQUENCE_ModuleDescription_defined
			#define _IDL_SEQUENCE_ModuleDescription _IDL_SEQUENCE_ModuleDef_ModuleDescription
		#endif /* _IDL_SEQUENCE_ModuleDescription_defined */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
	#ifndef ModuleDef_MajorVersion
		#define ModuleDef_MajorVersion   2
	#endif /* ModuleDef_MajorVersion */
	#ifndef ModuleDef_MinorVersion
		#define ModuleDef_MinorVersion   1
	#endif /* ModuleDef_MinorVersion */
	typedef struct ModuleDefClassDataStructure
	{
		SOMClass SOMSTAR classObject;
	} ModuleDefClassDataStructure;
	typedef struct ModuleDefCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} ModuleDefCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_moduledf_Source) || defined(ModuleDef_Class_Source)
			SOMEXTERN struct ModuleDefClassDataStructure _ModuleDefClassData;
			#ifndef ModuleDefClassData
				#define ModuleDefClassData    _ModuleDefClassData
			#endif /* ModuleDefClassData */
		#else
			SOMEXTERN struct ModuleDefClassDataStructure * SOMLINK resolve_ModuleDefClassData(void);
			#ifndef ModuleDefClassData
				#define ModuleDefClassData    (*(resolve_ModuleDefClassData()))
			#endif /* ModuleDefClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_moduledf_Source) || defined(ModuleDef_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_moduledf_Source || ModuleDef_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_moduledf_Source || ModuleDef_Class_Source */
		struct ModuleDefClassDataStructure SOMDLINK ModuleDefClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_moduledf_Source) || defined(ModuleDef_Class_Source)
			SOMEXTERN struct ModuleDefCClassDataStructure _ModuleDefCClassData;
			#ifndef ModuleDefCClassData
				#define ModuleDefCClassData    _ModuleDefCClassData
			#endif /* ModuleDefCClassData */
		#else
			SOMEXTERN struct ModuleDefCClassDataStructure * SOMLINK resolve_ModuleDefCClassData(void);
			#ifndef ModuleDefCClassData
				#define ModuleDefCClassData    (*(resolve_ModuleDefCClassData()))
			#endif /* ModuleDefCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_moduledf_Source) || defined(ModuleDef_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_moduledf_Source || ModuleDef_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_moduledf_Source || ModuleDef_Class_Source */
		struct ModuleDefCClassDataStructure SOMDLINK ModuleDefCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_moduledf_Source) || defined(ModuleDef_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_moduledf_Source || ModuleDef_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_moduledf_Source || ModuleDef_Class_Source */
	SOMClass SOMSTAR SOMLINK ModuleDefNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_ModuleDef (ModuleDefClassData.classObject)
	#ifndef SOMGD_ModuleDef
		#if (defined(_ModuleDef) || defined(__ModuleDef))
			#undef _ModuleDef
			#undef __ModuleDef
			#define SOMGD_ModuleDef 1
		#else
			#define _ModuleDef _SOMCLASS_ModuleDef
		#endif /* _ModuleDef */
	#endif /* SOMGD_ModuleDef */
	#define ModuleDef_classObj _SOMCLASS_ModuleDef
	#define _SOMMTOKEN_ModuleDef(method) ((somMToken)(ModuleDefClassData.method))
	#ifndef ModuleDefNew
		#define ModuleDefNew() ( _ModuleDef ? \
			(SOMClass_somNew(_ModuleDef)) : \
			( ModuleDefNewClass( \
				ModuleDef_MajorVersion, \
				ModuleDef_MinorVersion), \
			SOMClass_somNew(_ModuleDef))) 
	#endif /* NewModuleDef */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define ModuleDef_somInit SOMObject_somInit
#define ModuleDef_somUninit SOMObject_somUninit
#define ModuleDef_somFree SOMObject_somFree
#define ModuleDef_somGetClass SOMObject_somGetClass
#define ModuleDef_somGetClassName SOMObject_somGetClassName
#define ModuleDef_somGetSize SOMObject_somGetSize
#define ModuleDef_somIsA SOMObject_somIsA
#define ModuleDef_somIsInstanceOf SOMObject_somIsInstanceOf
#define ModuleDef_somRespondsTo SOMObject_somRespondsTo
#define ModuleDef_somDispatch SOMObject_somDispatch
#define ModuleDef_somClassDispatch SOMObject_somClassDispatch
#define ModuleDef_somCastObj SOMObject_somCastObj
#define ModuleDef_somResetObj SOMObject_somResetObj
#define ModuleDef_somPrintSelf SOMObject_somPrintSelf
#define ModuleDef_somDumpSelf SOMObject_somDumpSelf
#define ModuleDef_somDumpSelfInt SOMObject_somDumpSelfInt
#define ModuleDef_somDefaultInit SOMObject_somDefaultInit
#define ModuleDef_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define ModuleDef_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define ModuleDef_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define ModuleDef_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define ModuleDef_somDefaultAssign SOMObject_somDefaultAssign
#define ModuleDef_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define ModuleDef_somDefaultVAssign SOMObject_somDefaultVAssign
#define ModuleDef_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define ModuleDef_somDestruct SOMObject_somDestruct
#define ModuleDef__set_defined_in Contained__set_defined_in
#define ModuleDef__get_defined_in Contained__get_defined_in
#define ModuleDef__set_name Contained__set_name
#define ModuleDef__get_name Contained__get_name
#define ModuleDef__set_id Contained__set_id
#define ModuleDef__get_id Contained__get_id
#define ModuleDef__set_somModifiers Contained__set_somModifiers
#define ModuleDef__get_somModifiers Contained__get_somModifiers
#define ModuleDef_within Contained_within
#define ModuleDef_describe Contained_describe
#define ModuleDef_contents Container_contents
#define ModuleDef_lookup_name Container_lookup_name
#define ModuleDef_describe_contents Container_describe_contents
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_moduledf_Header_h */
