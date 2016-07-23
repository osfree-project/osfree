/* generated from containd.idl */
/* internal conditional is SOM_Module_containd_Source */
#ifndef SOM_Module_containd_Header_h
	#define SOM_Module_containd_Header_h 1
#include <somir.h>
#define RepositoryId string
#define _IDL_SEQUENCE_RepositoryId _IDL_SEQUENCE_string
#define TC_Contained ((TypeCode) &TC__Contained)
typedef struct {TCKind tag; short align; unsigned char version; unsigned char flags; string interfaceId;} TC_Contained_td;
SOMEXTERN TC_Contained_td SOMDLINK TC__Contained;

	#include <som.h>
	#include <somobj.h>
	#ifndef _IDL_Container_defined
		#define _IDL_Container_defined
		typedef SOMObject Container;
	#endif /* _IDL_Container_defined */
	#ifndef _IDL_SEQUENCE_Container_defined
		#define _IDL_SEQUENCE_Container_defined
		SOM_SEQUENCE_TYPEDEF_NAME(Container SOMSTAR ,sequence(Container));
	#endif /* _IDL_SEQUENCE_Container_defined */
	#ifndef _IDL_Contained_defined
		#define _IDL_Contained_defined
		typedef SOMObject Contained;
	#endif /* _IDL_Contained_defined */
	#ifndef _IDL_SEQUENCE_Contained_defined
		#define _IDL_SEQUENCE_Contained_defined
		SOM_SEQUENCE_TYPEDEF_NAME(Contained SOMSTAR ,sequence(Contained));
	#endif /* _IDL_SEQUENCE_Contained_defined */
	typedef struct Contained_Description
	{
		Identifier name;
		any value;
	} Contained_Description;
	#ifndef _IDL_SEQUENCE_Contained_Description_defined
		#define _IDL_SEQUENCE_Contained_Description_defined
		SOM_SEQUENCE_TYPEDEF(Contained_Description);
	#endif /* _IDL_SEQUENCE_Contained_Description_defined */
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_Description
			#if defined(Description)
				#undef Description
				#define SOMGD_Description
			#else
				#define Description Contained_Description
			#endif
		#endif /* SOMGD_Description */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef _IDL_SEQUENCE_Description_defined
			#define _IDL_SEQUENCE_Description_defined
			#define _IDL_SEQUENCE_Description _IDL_SEQUENCE_Contained_Description
		#endif /* _IDL_SEQUENCE_Description_defined */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
	#ifndef _IDL_SEQUENCE_somModifier_defined
		#define _IDL_SEQUENCE_somModifier_defined
		SOM_SEQUENCE_TYPEDEF(somModifier);
	#endif /* _IDL_SEQUENCE_somModifier_defined */
	typedef _IDL_SEQUENCE_somModifier Contained_somModifierSeq;
	#ifndef _IDL_SEQUENCE_Contained_somModifierSeq_defined
		#define _IDL_SEQUENCE_Contained_somModifierSeq_defined
		SOM_SEQUENCE_TYPEDEF(Contained_somModifierSeq);
	#endif /* _IDL_SEQUENCE_Contained_somModifierSeq_defined */
	typedef _IDL_SEQUENCE_Container Contained_ContainerSeq;
	#ifndef _IDL_SEQUENCE_Contained_ContainerSeq_defined
		#define _IDL_SEQUENCE_Contained_ContainerSeq_defined
		SOM_SEQUENCE_TYPEDEF(Contained_ContainerSeq);
	#endif /* _IDL_SEQUENCE_Contained_ContainerSeq_defined */
	#ifdef __IBMC__
		typedef void (somTP_Contained__set_defined_in)(
			Contained SOMSTAR somSelf,
			Environment *ev,
			/* in */ RepositoryId defined_in);
		#pragma linkage(somTP_Contained__set_defined_in,system)
		typedef somTP_Contained__set_defined_in *somTD_Contained__set_defined_in;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_Contained__set_defined_in)(
			Contained SOMSTAR somSelf,
			Environment *ev,
			/* in */ RepositoryId defined_in);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef RepositoryId (somTP_Contained__get_defined_in)(
			Contained SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_Contained__get_defined_in,system)
		typedef somTP_Contained__get_defined_in *somTD_Contained__get_defined_in;
	#else /* __IBMC__ */
		typedef RepositoryId (SOMLINK * somTD_Contained__get_defined_in)(
			Contained SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_Contained__set_name)(
			Contained SOMSTAR somSelf,
			Environment *ev,
			/* in */ Identifier name);
		#pragma linkage(somTP_Contained__set_name,system)
		typedef somTP_Contained__set_name *somTD_Contained__set_name;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_Contained__set_name)(
			Contained SOMSTAR somSelf,
			Environment *ev,
			/* in */ Identifier name);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef Identifier (somTP_Contained__get_name)(
			Contained SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_Contained__get_name,system)
		typedef somTP_Contained__get_name *somTD_Contained__get_name;
	#else /* __IBMC__ */
		typedef Identifier (SOMLINK * somTD_Contained__get_name)(
			Contained SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_Contained__set_id)(
			Contained SOMSTAR somSelf,
			Environment *ev,
			/* in */ RepositoryId id);
		#pragma linkage(somTP_Contained__set_id,system)
		typedef somTP_Contained__set_id *somTD_Contained__set_id;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_Contained__set_id)(
			Contained SOMSTAR somSelf,
			Environment *ev,
			/* in */ RepositoryId id);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef RepositoryId (somTP_Contained__get_id)(
			Contained SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_Contained__get_id,system)
		typedef somTP_Contained__get_id *somTD_Contained__get_id;
	#else /* __IBMC__ */
		typedef RepositoryId (SOMLINK * somTD_Contained__get_id)(
			Contained SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_Contained__set_somModifiers)(
			Contained SOMSTAR somSelf,
			Environment *ev,
			/* in */ Contained_somModifierSeq *somModifiers);
		#pragma linkage(somTP_Contained__set_somModifiers,system)
		typedef somTP_Contained__set_somModifiers *somTD_Contained__set_somModifiers;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_Contained__set_somModifiers)(
			Contained SOMSTAR somSelf,
			Environment *ev,
			/* in */ Contained_somModifierSeq *somModifiers);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef Contained_somModifierSeq (somTP_Contained__get_somModifiers)(
			Contained SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_Contained__get_somModifiers,system)
		typedef somTP_Contained__get_somModifiers *somTD_Contained__get_somModifiers;
	#else /* __IBMC__ */
		typedef Contained_somModifierSeq (SOMLINK * somTD_Contained__get_somModifiers)(
			Contained SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef Contained_ContainerSeq (somTP_Contained_within)(
			Contained SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_Contained_within,system)
		typedef somTP_Contained_within *somTD_Contained_within;
	#else /* __IBMC__ */
		typedef Contained_ContainerSeq (SOMLINK * somTD_Contained_within)(
			Contained SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef Contained_Description (somTP_Contained_describe)(
			Contained SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_Contained_describe,system)
		typedef somTP_Contained_describe *somTD_Contained_describe;
	#else /* __IBMC__ */
		typedef Contained_Description (SOMLINK * somTD_Contained_describe)(
			Contained SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifndef Contained_MajorVersion
		#define Contained_MajorVersion   2
	#endif /* Contained_MajorVersion */
	#ifndef Contained_MinorVersion
		#define Contained_MinorVersion   1
	#endif /* Contained_MinorVersion */
	typedef struct ContainedClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken within;
		somMToken describe;
		somMToken _get_name;
		somMToken _set_name;
		somMToken _get_id;
		somMToken _set_id;
		somMToken _get_defined_in;
		somMToken _set_defined_in;
		somMToken _get_somModifiers;
		somMToken _set_somModifiers;
		somMToken _get_repository;
		somMToken _set_repository;
		somMToken _get_reference_list;
		somMToken _set_reference_list;
		somMToken _get_iid;
		somMToken _get_defined_in_obj;
		somMToken getFixedPersistentSize;
		somMToken getVariablePersistentSize;
		somMToken putDataInFixedBuf;
		somMToken putDataInVariableBuf;
		somMToken getDataFromFixedBuf;
		somMToken getDataFromVariableBuf;
		somMToken saveToIR;
		somMToken fixedDirty;
		somMToken variableDirty;
		somMToken addToReferenceList;
		somMToken allocDef;
		somMToken removeFromReferenceList;
		somMToken incrementUseCount;
		somMToken deleteFromIR;
		somMToken forceFree;
		somMToken freeUnreferenced;
		somMToken decrementUseCount;
	} ContainedClassDataStructure;
	typedef struct ContainedCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} ContainedCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_containd_Source) || defined(Contained_Class_Source)
			SOMEXTERN struct ContainedClassDataStructure _ContainedClassData;
			#ifndef ContainedClassData
				#define ContainedClassData    _ContainedClassData
			#endif /* ContainedClassData */
		#else
			SOMEXTERN struct ContainedClassDataStructure * SOMLINK resolve_ContainedClassData(void);
			#ifndef ContainedClassData
				#define ContainedClassData    (*(resolve_ContainedClassData()))
			#endif /* ContainedClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_containd_Source) || defined(Contained_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_containd_Source || Contained_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_containd_Source || Contained_Class_Source */
		struct ContainedClassDataStructure SOMDLINK ContainedClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_containd_Source) || defined(Contained_Class_Source)
			SOMEXTERN struct ContainedCClassDataStructure _ContainedCClassData;
			#ifndef ContainedCClassData
				#define ContainedCClassData    _ContainedCClassData
			#endif /* ContainedCClassData */
		#else
			SOMEXTERN struct ContainedCClassDataStructure * SOMLINK resolve_ContainedCClassData(void);
			#ifndef ContainedCClassData
				#define ContainedCClassData    (*(resolve_ContainedCClassData()))
			#endif /* ContainedCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_containd_Source) || defined(Contained_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_containd_Source || Contained_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_containd_Source || Contained_Class_Source */
		struct ContainedCClassDataStructure SOMDLINK ContainedCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_containd_Source) || defined(Contained_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_containd_Source || Contained_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_containd_Source || Contained_Class_Source */
	SOMClass SOMSTAR SOMLINK ContainedNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_Contained (ContainedClassData.classObject)
	#ifndef SOMGD_Contained
		#if (defined(_Contained) || defined(__Contained))
			#undef _Contained
			#undef __Contained
			#define SOMGD_Contained 1
		#else
			#define _Contained _SOMCLASS_Contained
		#endif /* _Contained */
	#endif /* SOMGD_Contained */
	#define Contained_classObj _SOMCLASS_Contained
	#define _SOMMTOKEN_Contained(method) ((somMToken)(ContainedClassData.method))
	#ifndef ContainedNew
		#define ContainedNew() ( _Contained ? \
			(SOMClass_somNew(_Contained)) : \
			( ContainedNewClass( \
				Contained_MajorVersion, \
				Contained_MinorVersion), \
			SOMClass_somNew(_Contained))) 
	#endif /* NewContained */
	#ifndef Contained__set_defined_in
		#define Contained__set_defined_in(somSelf,ev,defined_in) \
			SOM_Resolve(somSelf,Contained,_set_defined_in)  \
				(somSelf,ev,defined_in)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_defined_in
				#if defined(__set_defined_in)
					#undef __set_defined_in
					#define SOMGD___set_defined_in
				#else
					#define __set_defined_in Contained__set_defined_in
				#endif
			#endif /* SOMGD___set_defined_in */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* Contained__set_defined_in */
	#ifndef Contained__get_defined_in
		#define Contained__get_defined_in(somSelf,ev) \
			SOM_Resolve(somSelf,Contained,_get_defined_in)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_defined_in
				#if defined(__get_defined_in)
					#undef __get_defined_in
					#define SOMGD___get_defined_in
				#else
					#define __get_defined_in Contained__get_defined_in
				#endif
			#endif /* SOMGD___get_defined_in */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* Contained__get_defined_in */
	#ifndef Contained__set_name
		#define Contained__set_name(somSelf,ev,name) \
			SOM_Resolve(somSelf,Contained,_set_name)  \
				(somSelf,ev,name)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_name
				#if defined(__set_name)
					#undef __set_name
					#define SOMGD___set_name
				#else
					#define __set_name Contained__set_name
				#endif
			#endif /* SOMGD___set_name */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* Contained__set_name */
	#ifndef Contained__get_name
		#define Contained__get_name(somSelf,ev) \
			SOM_Resolve(somSelf,Contained,_get_name)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_name
				#if defined(__get_name)
					#undef __get_name
					#define SOMGD___get_name
				#else
					#define __get_name Contained__get_name
				#endif
			#endif /* SOMGD___get_name */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* Contained__get_name */
	#ifndef Contained__set_id
		#define Contained__set_id(somSelf,ev,id) \
			SOM_Resolve(somSelf,Contained,_set_id)  \
				(somSelf,ev,id)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_id
				#if defined(__set_id)
					#undef __set_id
					#define SOMGD___set_id
				#else
					#define __set_id Contained__set_id
				#endif
			#endif /* SOMGD___set_id */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* Contained__set_id */
	#ifndef Contained__get_id
		#define Contained__get_id(somSelf,ev) \
			SOM_Resolve(somSelf,Contained,_get_id)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_id
				#if defined(__get_id)
					#undef __get_id
					#define SOMGD___get_id
				#else
					#define __get_id Contained__get_id
				#endif
			#endif /* SOMGD___get_id */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* Contained__get_id */
	#ifndef Contained__set_somModifiers
		#define Contained__set_somModifiers(somSelf,ev,somModifiers) \
			SOM_Resolve(somSelf,Contained,_set_somModifiers)  \
				(somSelf,ev,somModifiers)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_somModifiers
				#if defined(__set_somModifiers)
					#undef __set_somModifiers
					#define SOMGD___set_somModifiers
				#else
					#define __set_somModifiers Contained__set_somModifiers
				#endif
			#endif /* SOMGD___set_somModifiers */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* Contained__set_somModifiers */
	#ifndef Contained__get_somModifiers
		#define Contained__get_somModifiers(somSelf,ev) \
			SOM_Resolve(somSelf,Contained,_get_somModifiers)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somModifiers
				#if defined(__get_somModifiers)
					#undef __get_somModifiers
					#define SOMGD___get_somModifiers
				#else
					#define __get_somModifiers Contained__get_somModifiers
				#endif
			#endif /* SOMGD___get_somModifiers */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* Contained__get_somModifiers */
	#ifndef Contained_within
		#define Contained_within(somSelf,ev) \
			SOM_Resolve(somSelf,Contained,within)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__within
				#if defined(_within)
					#undef _within
					#define SOMGD__within
				#else
					#define _within Contained_within
				#endif
			#endif /* SOMGD__within */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* Contained_within */
	#ifndef Contained_describe
		#define Contained_describe(somSelf,ev) \
			SOM_Resolve(somSelf,Contained,describe)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__describe
				#if defined(_describe)
					#undef _describe
					#define SOMGD__describe
				#else
					#define _describe Contained_describe
				#endif
			#endif /* SOMGD__describe */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* Contained_describe */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define Contained_somInit SOMObject_somInit
#define Contained_somUninit SOMObject_somUninit
#define Contained_somFree SOMObject_somFree
#define Contained_somGetClass SOMObject_somGetClass
#define Contained_somGetClassName SOMObject_somGetClassName
#define Contained_somGetSize SOMObject_somGetSize
#define Contained_somIsA SOMObject_somIsA
#define Contained_somIsInstanceOf SOMObject_somIsInstanceOf
#define Contained_somRespondsTo SOMObject_somRespondsTo
#define Contained_somDispatch SOMObject_somDispatch
#define Contained_somClassDispatch SOMObject_somClassDispatch
#define Contained_somCastObj SOMObject_somCastObj
#define Contained_somResetObj SOMObject_somResetObj
#define Contained_somPrintSelf SOMObject_somPrintSelf
#define Contained_somDumpSelf SOMObject_somDumpSelf
#define Contained_somDumpSelfInt SOMObject_somDumpSelfInt
#define Contained_somDefaultInit SOMObject_somDefaultInit
#define Contained_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define Contained_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define Contained_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define Contained_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define Contained_somDefaultAssign SOMObject_somDefaultAssign
#define Contained_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define Contained_somDefaultVAssign SOMObject_somDefaultVAssign
#define Contained_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define Contained_somDestruct SOMObject_somDestruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_containd_Header_h */
