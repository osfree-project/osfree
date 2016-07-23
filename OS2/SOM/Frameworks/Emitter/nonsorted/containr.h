/* generated from containr.idl */
/* internal conditional is SOM_Module_containr_Source */
#ifndef SOM_Module_containr_Header_h
	#define SOM_Module_containr_Header_h 1
	#include <som.h>
	#include <somobj.h>
	#ifndef _IDL_Contained_defined
		#define _IDL_Contained_defined
		typedef SOMObject Contained;
	#endif /* _IDL_Contained_defined */
	#ifndef _IDL_SEQUENCE_Contained_defined
		#define _IDL_SEQUENCE_Contained_defined
		SOM_SEQUENCE_TYPEDEF_NAME(Contained SOMSTAR ,sequence(Contained));
	#endif /* _IDL_SEQUENCE_Contained_defined */
	#ifndef _IDL_Container_defined
		#define _IDL_Container_defined
		typedef SOMObject Container;
	#endif /* _IDL_Container_defined */
	#ifndef _IDL_SEQUENCE_Container_defined
		#define _IDL_SEQUENCE_Container_defined
		SOM_SEQUENCE_TYPEDEF_NAME(Container SOMSTAR ,sequence(Container));
	#endif /* _IDL_SEQUENCE_Container_defined */
	typedef string Container_InterfaceName;
	#ifndef _IDL_SEQUENCE_string_defined
		#define _IDL_SEQUENCE_string_defined
		SOM_SEQUENCE_TYPEDEF(string);
	#endif /* _IDL_SEQUENCE_string_defined */
	typedef struct Container_ContainerDescription
	{
		Contained SOMSTAR contained_object;
		Identifier name;
		any value;
	} Container_ContainerDescription;
	#ifndef _IDL_SEQUENCE_Container_ContainerDescription_defined
		#define _IDL_SEQUENCE_Container_ContainerDescription_defined
		SOM_SEQUENCE_TYPEDEF(Container_ContainerDescription);
	#endif /* _IDL_SEQUENCE_Container_ContainerDescription_defined */
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_ContainerDescription
			#if defined(ContainerDescription)
				#undef ContainerDescription
				#define SOMGD_ContainerDescription
			#else
				#define ContainerDescription Container_ContainerDescription
			#endif
		#endif /* SOMGD_ContainerDescription */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef _IDL_SEQUENCE_ContainerDescription_defined
			#define _IDL_SEQUENCE_ContainerDescription_defined
			#define _IDL_SEQUENCE_ContainerDescription _IDL_SEQUENCE_Container_ContainerDescription
		#endif /* _IDL_SEQUENCE_ContainerDescription_defined */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
	typedef _IDL_SEQUENCE_Contained Container_ContainedSeq;
	#ifndef _IDL_SEQUENCE_Container_ContainedSeq_defined
		#define _IDL_SEQUENCE_Container_ContainedSeq_defined
		SOM_SEQUENCE_TYPEDEF(Container_ContainedSeq);
	#endif /* _IDL_SEQUENCE_Container_ContainedSeq_defined */
	typedef _IDL_SEQUENCE_Container_ContainerDescription Container_ContainerDescriptionSeq;
	#ifndef _IDL_SEQUENCE_Container_ContainerDescriptionSeq_defined
		#define _IDL_SEQUENCE_Container_ContainerDescriptionSeq_defined
		SOM_SEQUENCE_TYPEDEF(Container_ContainerDescriptionSeq);
	#endif /* _IDL_SEQUENCE_Container_ContainerDescriptionSeq_defined */
	#ifdef __IBMC__
		typedef Container_ContainedSeq (somTP_Container_contents)(
			Container SOMSTAR somSelf,
			Environment *ev,
			/* in */ Container_InterfaceName limit_type,
			/* in */ boolean exclude_inherited);
		#pragma linkage(somTP_Container_contents,system)
		typedef somTP_Container_contents *somTD_Container_contents;
	#else /* __IBMC__ */
		typedef Container_ContainedSeq (SOMLINK * somTD_Container_contents)(
			Container SOMSTAR somSelf,
			Environment *ev,
			/* in */ Container_InterfaceName limit_type,
			/* in */ boolean exclude_inherited);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef Container_ContainedSeq (somTP_Container_lookup_name)(
			Container SOMSTAR somSelf,
			Environment *ev,
			/* in */ Identifier search_name,
			/* in */ long levels_to_search,
			/* in */ Container_InterfaceName limit_type,
			/* in */ boolean exclude_inherited);
		#pragma linkage(somTP_Container_lookup_name,system)
		typedef somTP_Container_lookup_name *somTD_Container_lookup_name;
	#else /* __IBMC__ */
		typedef Container_ContainedSeq (SOMLINK * somTD_Container_lookup_name)(
			Container SOMSTAR somSelf,
			Environment *ev,
			/* in */ Identifier search_name,
			/* in */ long levels_to_search,
			/* in */ Container_InterfaceName limit_type,
			/* in */ boolean exclude_inherited);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef Container_ContainerDescriptionSeq (somTP_Container_describe_contents)(
			Container SOMSTAR somSelf,
			Environment *ev,
			/* in */ Container_InterfaceName limit_type,
			/* in */ boolean exclude_inherited,
			/* in */ long max_returned_objs);
		#pragma linkage(somTP_Container_describe_contents,system)
		typedef somTP_Container_describe_contents *somTD_Container_describe_contents;
	#else /* __IBMC__ */
		typedef Container_ContainerDescriptionSeq (SOMLINK * somTD_Container_describe_contents)(
			Container SOMSTAR somSelf,
			Environment *ev,
			/* in */ Container_InterfaceName limit_type,
			/* in */ boolean exclude_inherited,
			/* in */ long max_returned_objs);
	#endif /* __IBMC__ */
	#ifndef Container_MajorVersion
		#define Container_MajorVersion   2
	#endif /* Container_MajorVersion */
	#ifndef Container_MinorVersion
		#define Container_MinorVersion   1
	#endif /* Container_MinorVersion */
	typedef struct ContainerClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken contents;
		somMToken lookup_name;
		somMToken describe_contents;
		somMToken internalNameLookup;
		somMToken private1;
		somMToken private2;
		somMToken addToContents;
		somMToken removeFromContents;
		somMToken getContainerFixedPersistentSize;
		somMToken putContainerDataInFixedBuf;
		somMToken getContainerDataFromFixedBuf;
		somMToken _set_iidDirectory;
		somMToken _get_iidDirectory;
		somMToken saveContents;
		somMToken restoreContents;
		somMToken getContentsSize;
		somMToken deleteContainerFromIR;
		somMToken _get_dirdirtybit;
		somMToken _set_dirdirtybit;
		somMToken gatherSubclasses;
		somMToken _get_mycontents;
		somMToken _set_mycontents;
	} ContainerClassDataStructure;
	typedef struct ContainerCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} ContainerCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_containr_Source) || defined(Container_Class_Source)
			SOMEXTERN struct ContainerClassDataStructure _ContainerClassData;
			#ifndef ContainerClassData
				#define ContainerClassData    _ContainerClassData
			#endif /* ContainerClassData */
		#else
			SOMEXTERN struct ContainerClassDataStructure * SOMLINK resolve_ContainerClassData(void);
			#ifndef ContainerClassData
				#define ContainerClassData    (*(resolve_ContainerClassData()))
			#endif /* ContainerClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_containr_Source) || defined(Container_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_containr_Source || Container_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_containr_Source || Container_Class_Source */
		struct ContainerClassDataStructure SOMDLINK ContainerClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_containr_Source) || defined(Container_Class_Source)
			SOMEXTERN struct ContainerCClassDataStructure _ContainerCClassData;
			#ifndef ContainerCClassData
				#define ContainerCClassData    _ContainerCClassData
			#endif /* ContainerCClassData */
		#else
			SOMEXTERN struct ContainerCClassDataStructure * SOMLINK resolve_ContainerCClassData(void);
			#ifndef ContainerCClassData
				#define ContainerCClassData    (*(resolve_ContainerCClassData()))
			#endif /* ContainerCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_containr_Source) || defined(Container_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_containr_Source || Container_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_containr_Source || Container_Class_Source */
		struct ContainerCClassDataStructure SOMDLINK ContainerCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_containr_Source) || defined(Container_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_containr_Source || Container_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_containr_Source || Container_Class_Source */
	SOMClass SOMSTAR SOMLINK ContainerNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_Container (ContainerClassData.classObject)
	#ifndef SOMGD_Container
		#if (defined(_Container) || defined(__Container))
			#undef _Container
			#undef __Container
			#define SOMGD_Container 1
		#else
			#define _Container _SOMCLASS_Container
		#endif /* _Container */
	#endif /* SOMGD_Container */
	#define Container_classObj _SOMCLASS_Container
	#define _SOMMTOKEN_Container(method) ((somMToken)(ContainerClassData.method))
	#ifndef ContainerNew
		#define ContainerNew() ( _Container ? \
			(SOMClass_somNew(_Container)) : \
			( ContainerNewClass( \
				Container_MajorVersion, \
				Container_MinorVersion), \
			SOMClass_somNew(_Container))) 
	#endif /* NewContainer */
	#ifndef Container_contents
		#define Container_contents(somSelf,ev,limit_type,exclude_inherited) \
			SOM_Resolve(somSelf,Container,contents)  \
				(somSelf,ev,limit_type,exclude_inherited)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__contents
				#if defined(_contents)
					#undef _contents
					#define SOMGD__contents
				#else
					#define _contents Container_contents
				#endif
			#endif /* SOMGD__contents */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* Container_contents */
	#ifndef Container_lookup_name
		#define Container_lookup_name(somSelf,ev,search_name,levels_to_search,limit_type,exclude_inherited) \
			SOM_Resolve(somSelf,Container,lookup_name)  \
				(somSelf,ev,search_name,levels_to_search,limit_type,exclude_inherited)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__lookup_name
				#if defined(_lookup_name)
					#undef _lookup_name
					#define SOMGD__lookup_name
				#else
					#define _lookup_name Container_lookup_name
				#endif
			#endif /* SOMGD__lookup_name */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* Container_lookup_name */
	#ifndef Container_describe_contents
		#define Container_describe_contents(somSelf,ev,limit_type,exclude_inherited,max_returned_objs) \
			SOM_Resolve(somSelf,Container,describe_contents)  \
				(somSelf,ev,limit_type,exclude_inherited,max_returned_objs)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__describe_contents
				#if defined(_describe_contents)
					#undef _describe_contents
					#define SOMGD__describe_contents
				#else
					#define _describe_contents Container_describe_contents
				#endif
			#endif /* SOMGD__describe_contents */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* Container_describe_contents */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define Container_somInit SOMObject_somInit
#define Container_somUninit SOMObject_somUninit
#define Container_somFree SOMObject_somFree
#define Container_somGetClass SOMObject_somGetClass
#define Container_somGetClassName SOMObject_somGetClassName
#define Container_somGetSize SOMObject_somGetSize
#define Container_somIsA SOMObject_somIsA
#define Container_somIsInstanceOf SOMObject_somIsInstanceOf
#define Container_somRespondsTo SOMObject_somRespondsTo
#define Container_somDispatch SOMObject_somDispatch
#define Container_somClassDispatch SOMObject_somClassDispatch
#define Container_somCastObj SOMObject_somCastObj
#define Container_somResetObj SOMObject_somResetObj
#define Container_somPrintSelf SOMObject_somPrintSelf
#define Container_somDumpSelf SOMObject_somDumpSelf
#define Container_somDumpSelfInt SOMObject_somDumpSelfInt
#define Container_somDefaultInit SOMObject_somDefaultInit
#define Container_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define Container_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define Container_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define Container_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define Container_somDefaultAssign SOMObject_somDefaultAssign
#define Container_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define Container_somDefaultVAssign SOMObject_somDefaultVAssign
#define Container_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define Container_somDestruct SOMObject_somDestruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_containr_Header_h */
