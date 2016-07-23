/* generated from intfacdf.idl */
/* internal conditional is SOM_Module_intfacdf_Source */
#ifndef SOM_Module_intfacdf_Header_h
	#define SOM_Module_intfacdf_Header_h 1
	#include <som.h>
	#include <containd.h>
	#include <somobj.h>
	#include <containr.h>
	#include <attribdf.h>
	#include <operatdf.h>
	#include <paramdef.h>
	#include <excptdef.h>
	#ifndef _IDL_InterfaceDef_defined
		#define _IDL_InterfaceDef_defined
		typedef SOMObject InterfaceDef;
	#endif /* _IDL_InterfaceDef_defined */
	#ifndef _IDL_SEQUENCE_InterfaceDef_defined
		#define _IDL_SEQUENCE_InterfaceDef_defined
		SOM_SEQUENCE_TYPEDEF_NAME(InterfaceDef SOMSTAR ,sequence(InterfaceDef));
	#endif /* _IDL_SEQUENCE_InterfaceDef_defined */
	typedef struct InterfaceDef_FullInterfaceDescription
	{
		Identifier name;
		RepositoryId id;
		RepositoryId defined_in;
		_IDL_SEQUENCE_OperationDef_OperationDescription operation;
		_IDL_SEQUENCE_AttributeDef_AttributeDescription attributes;
	} InterfaceDef_FullInterfaceDescription;
	#ifndef _IDL_SEQUENCE_InterfaceDef_FullInterfaceDescription_defined
		#define _IDL_SEQUENCE_InterfaceDef_FullInterfaceDescription_defined
		SOM_SEQUENCE_TYPEDEF(InterfaceDef_FullInterfaceDescription);
	#endif /* _IDL_SEQUENCE_InterfaceDef_FullInterfaceDescription_defined */
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_FullInterfaceDescription
			#if defined(FullInterfaceDescription)
				#undef FullInterfaceDescription
				#define SOMGD_FullInterfaceDescription
			#else
				#define FullInterfaceDescription InterfaceDef_FullInterfaceDescription
			#endif
		#endif /* SOMGD_FullInterfaceDescription */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef _IDL_SEQUENCE_FullInterfaceDescription_defined
			#define _IDL_SEQUENCE_FullInterfaceDescription_defined
			#define _IDL_SEQUENCE_FullInterfaceDescription _IDL_SEQUENCE_InterfaceDef_FullInterfaceDescription
		#endif /* _IDL_SEQUENCE_FullInterfaceDescription_defined */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
	typedef struct InterfaceDef_InterfaceDescription
	{
		Identifier name;
		RepositoryId id;
		RepositoryId defined_in;
	} InterfaceDef_InterfaceDescription;
	#ifndef _IDL_SEQUENCE_InterfaceDef_InterfaceDescription_defined
		#define _IDL_SEQUENCE_InterfaceDef_InterfaceDescription_defined
		SOM_SEQUENCE_TYPEDEF(InterfaceDef_InterfaceDescription);
	#endif /* _IDL_SEQUENCE_InterfaceDef_InterfaceDescription_defined */
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_InterfaceDescription
			#if defined(InterfaceDescription)
				#undef InterfaceDescription
				#define SOMGD_InterfaceDescription
			#else
				#define InterfaceDescription InterfaceDef_InterfaceDescription
			#endif
		#endif /* SOMGD_InterfaceDescription */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef _IDL_SEQUENCE_InterfaceDescription_defined
			#define _IDL_SEQUENCE_InterfaceDescription_defined
			#define _IDL_SEQUENCE_InterfaceDescription _IDL_SEQUENCE_InterfaceDef_InterfaceDescription
		#endif /* _IDL_SEQUENCE_InterfaceDescription_defined */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
	#ifdef __IBMC__
		typedef void (somTP_InterfaceDef__set_base_interfaces)(
			InterfaceDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ _IDL_SEQUENCE_string *base_interfaces);
		#pragma linkage(somTP_InterfaceDef__set_base_interfaces,system)
		typedef somTP_InterfaceDef__set_base_interfaces *somTD_InterfaceDef__set_base_interfaces;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_InterfaceDef__set_base_interfaces)(
			InterfaceDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ _IDL_SEQUENCE_string *base_interfaces);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef _IDL_SEQUENCE_string (somTP_InterfaceDef__get_base_interfaces)(
			InterfaceDef SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_InterfaceDef__get_base_interfaces,system)
		typedef somTP_InterfaceDef__get_base_interfaces *somTD_InterfaceDef__get_base_interfaces;
	#else /* __IBMC__ */
		typedef _IDL_SEQUENCE_string (SOMLINK * somTD_InterfaceDef__get_base_interfaces)(
			InterfaceDef SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef InterfaceDef_FullInterfaceDescription (somTP_InterfaceDef_describe_interface)(
			InterfaceDef SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_InterfaceDef_describe_interface,system)
		typedef somTP_InterfaceDef_describe_interface *somTD_InterfaceDef_describe_interface;
	#else /* __IBMC__ */
		typedef InterfaceDef_FullInterfaceDescription (SOMLINK * somTD_InterfaceDef_describe_interface)(
			InterfaceDef SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_InterfaceDef__set_instanceData)(
			InterfaceDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ TypeCode instanceData);
		#pragma linkage(somTP_InterfaceDef__set_instanceData,system)
		typedef somTP_InterfaceDef__set_instanceData *somTD_InterfaceDef__set_instanceData;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_InterfaceDef__set_instanceData)(
			InterfaceDef SOMSTAR somSelf,
			Environment *ev,
			/* in */ TypeCode instanceData);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef TypeCode (somTP_InterfaceDef__get_instanceData)(
			InterfaceDef SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_InterfaceDef__get_instanceData,system)
		typedef somTP_InterfaceDef__get_instanceData *somTD_InterfaceDef__get_instanceData;
	#else /* __IBMC__ */
		typedef TypeCode (SOMLINK * somTD_InterfaceDef__get_instanceData)(
			InterfaceDef SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifndef InterfaceDef_MajorVersion
		#define InterfaceDef_MajorVersion   2
	#endif /* InterfaceDef_MajorVersion */
	#ifndef InterfaceDef_MinorVersion
		#define InterfaceDef_MinorVersion   1
	#endif /* InterfaceDef_MinorVersion */
	typedef struct InterfaceDefClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken describe_interface;
		somMToken _get_base_interfaces;
		somMToken _set_base_interfaces;
		somMToken _get_instanceData;
		somMToken _set_instanceData;
		somMToken subclasses;
	} InterfaceDefClassDataStructure;
	typedef struct InterfaceDefCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} InterfaceDefCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_intfacdf_Source) || defined(InterfaceDef_Class_Source)
			SOMEXTERN struct InterfaceDefClassDataStructure _InterfaceDefClassData;
			#ifndef InterfaceDefClassData
				#define InterfaceDefClassData    _InterfaceDefClassData
			#endif /* InterfaceDefClassData */
		#else
			SOMEXTERN struct InterfaceDefClassDataStructure * SOMLINK resolve_InterfaceDefClassData(void);
			#ifndef InterfaceDefClassData
				#define InterfaceDefClassData    (*(resolve_InterfaceDefClassData()))
			#endif /* InterfaceDefClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_intfacdf_Source) || defined(InterfaceDef_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_intfacdf_Source || InterfaceDef_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_intfacdf_Source || InterfaceDef_Class_Source */
		struct InterfaceDefClassDataStructure SOMDLINK InterfaceDefClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_intfacdf_Source) || defined(InterfaceDef_Class_Source)
			SOMEXTERN struct InterfaceDefCClassDataStructure _InterfaceDefCClassData;
			#ifndef InterfaceDefCClassData
				#define InterfaceDefCClassData    _InterfaceDefCClassData
			#endif /* InterfaceDefCClassData */
		#else
			SOMEXTERN struct InterfaceDefCClassDataStructure * SOMLINK resolve_InterfaceDefCClassData(void);
			#ifndef InterfaceDefCClassData
				#define InterfaceDefCClassData    (*(resolve_InterfaceDefCClassData()))
			#endif /* InterfaceDefCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_intfacdf_Source) || defined(InterfaceDef_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_intfacdf_Source || InterfaceDef_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_intfacdf_Source || InterfaceDef_Class_Source */
		struct InterfaceDefCClassDataStructure SOMDLINK InterfaceDefCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_intfacdf_Source) || defined(InterfaceDef_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_intfacdf_Source || InterfaceDef_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_intfacdf_Source || InterfaceDef_Class_Source */
	SOMClass SOMSTAR SOMLINK InterfaceDefNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_InterfaceDef (InterfaceDefClassData.classObject)
	#ifndef SOMGD_InterfaceDef
		#if (defined(_InterfaceDef) || defined(__InterfaceDef))
			#undef _InterfaceDef
			#undef __InterfaceDef
			#define SOMGD_InterfaceDef 1
		#else
			#define _InterfaceDef _SOMCLASS_InterfaceDef
		#endif /* _InterfaceDef */
	#endif /* SOMGD_InterfaceDef */
	#define InterfaceDef_classObj _SOMCLASS_InterfaceDef
	#define _SOMMTOKEN_InterfaceDef(method) ((somMToken)(InterfaceDefClassData.method))
	#ifndef InterfaceDefNew
		#define InterfaceDefNew() ( _InterfaceDef ? \
			(SOMClass_somNew(_InterfaceDef)) : \
			( InterfaceDefNewClass( \
				InterfaceDef_MajorVersion, \
				InterfaceDef_MinorVersion), \
			SOMClass_somNew(_InterfaceDef))) 
	#endif /* NewInterfaceDef */
	#ifndef InterfaceDef__set_base_interfaces
		#define InterfaceDef__set_base_interfaces(somSelf,ev,base_interfaces) \
			SOM_Resolve(somSelf,InterfaceDef,_set_base_interfaces)  \
				(somSelf,ev,base_interfaces)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_base_interfaces
				#if defined(__set_base_interfaces)
					#undef __set_base_interfaces
					#define SOMGD___set_base_interfaces
				#else
					#define __set_base_interfaces InterfaceDef__set_base_interfaces
				#endif
			#endif /* SOMGD___set_base_interfaces */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* InterfaceDef__set_base_interfaces */
	#ifndef InterfaceDef__get_base_interfaces
		#define InterfaceDef__get_base_interfaces(somSelf,ev) \
			SOM_Resolve(somSelf,InterfaceDef,_get_base_interfaces)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_base_interfaces
				#if defined(__get_base_interfaces)
					#undef __get_base_interfaces
					#define SOMGD___get_base_interfaces
				#else
					#define __get_base_interfaces InterfaceDef__get_base_interfaces
				#endif
			#endif /* SOMGD___get_base_interfaces */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* InterfaceDef__get_base_interfaces */
	#ifndef InterfaceDef_describe_interface
		#define InterfaceDef_describe_interface(somSelf,ev) \
			SOM_Resolve(somSelf,InterfaceDef,describe_interface)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__describe_interface
				#if defined(_describe_interface)
					#undef _describe_interface
					#define SOMGD__describe_interface
				#else
					#define _describe_interface InterfaceDef_describe_interface
				#endif
			#endif /* SOMGD__describe_interface */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* InterfaceDef_describe_interface */
	#ifndef InterfaceDef__set_instanceData
		#define InterfaceDef__set_instanceData(somSelf,ev,instanceData) \
			SOM_Resolve(somSelf,InterfaceDef,_set_instanceData)  \
				(somSelf,ev,instanceData)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_instanceData
				#if defined(__set_instanceData)
					#undef __set_instanceData
					#define SOMGD___set_instanceData
				#else
					#define __set_instanceData InterfaceDef__set_instanceData
				#endif
			#endif /* SOMGD___set_instanceData */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* InterfaceDef__set_instanceData */
	#ifndef InterfaceDef__get_instanceData
		#define InterfaceDef__get_instanceData(somSelf,ev) \
			SOM_Resolve(somSelf,InterfaceDef,_get_instanceData)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_instanceData
				#if defined(__get_instanceData)
					#undef __get_instanceData
					#define SOMGD___get_instanceData
				#else
					#define __get_instanceData InterfaceDef__get_instanceData
				#endif
			#endif /* SOMGD___get_instanceData */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* InterfaceDef__get_instanceData */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define InterfaceDef_somInit SOMObject_somInit
#define InterfaceDef_somUninit SOMObject_somUninit
#define InterfaceDef_somFree SOMObject_somFree
#define InterfaceDef_somGetClass SOMObject_somGetClass
#define InterfaceDef_somGetClassName SOMObject_somGetClassName
#define InterfaceDef_somGetSize SOMObject_somGetSize
#define InterfaceDef_somIsA SOMObject_somIsA
#define InterfaceDef_somIsInstanceOf SOMObject_somIsInstanceOf
#define InterfaceDef_somRespondsTo SOMObject_somRespondsTo
#define InterfaceDef_somDispatch SOMObject_somDispatch
#define InterfaceDef_somClassDispatch SOMObject_somClassDispatch
#define InterfaceDef_somCastObj SOMObject_somCastObj
#define InterfaceDef_somResetObj SOMObject_somResetObj
#define InterfaceDef_somPrintSelf SOMObject_somPrintSelf
#define InterfaceDef_somDumpSelf SOMObject_somDumpSelf
#define InterfaceDef_somDumpSelfInt SOMObject_somDumpSelfInt
#define InterfaceDef_somDefaultInit SOMObject_somDefaultInit
#define InterfaceDef_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define InterfaceDef_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define InterfaceDef_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define InterfaceDef_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define InterfaceDef_somDefaultAssign SOMObject_somDefaultAssign
#define InterfaceDef_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define InterfaceDef_somDefaultVAssign SOMObject_somDefaultVAssign
#define InterfaceDef_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define InterfaceDef_somDestruct SOMObject_somDestruct
#define InterfaceDef__set_defined_in Contained__set_defined_in
#define InterfaceDef__get_defined_in Contained__get_defined_in
#define InterfaceDef__set_name Contained__set_name
#define InterfaceDef__get_name Contained__get_name
#define InterfaceDef__set_id Contained__set_id
#define InterfaceDef__get_id Contained__get_id
#define InterfaceDef__set_somModifiers Contained__set_somModifiers
#define InterfaceDef__get_somModifiers Contained__get_somModifiers
#define InterfaceDef_within Contained_within
#define InterfaceDef_describe Contained_describe
#define InterfaceDef_contents Container_contents
#define InterfaceDef_lookup_name Container_lookup_name
#define InterfaceDef_describe_contents Container_describe_contents
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_intfacdf_Header_h */
