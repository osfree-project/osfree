/* generated from implrep.idl */
/* internal conditional is SOM_Module_implrep_Source */
#ifndef SOM_Module_implrep_Header_h
	#define SOM_Module_implrep_Header_h 1
	#include <som.h>
	#include <somdtype.h>
	#include <somobj.h>
	#include <containd.h>
	#include <impldef.h>
	#include <omgestio.h>
	#include <naming.h>
	#include <omgidobj.h>
	#ifndef _IDL_ImplRepository_defined
		#define _IDL_ImplRepository_defined
		typedef SOMObject ImplRepository;
	#endif /* _IDL_ImplRepository_defined */
	#ifndef _IDL_SEQUENCE_ImplRepository_defined
		#define _IDL_SEQUENCE_ImplRepository_defined
		SOM_SEQUENCE_TYPEDEF_NAME(ImplRepository SOMSTAR ,sequence(ImplRepository));
	#endif /* _IDL_SEQUENCE_ImplRepository_defined */
	typedef struct ImplRepository_PV
	{
		string name;
		string value;
	} ImplRepository_PV;
	#ifndef _IDL_SEQUENCE_ImplRepository_PV_defined
		#define _IDL_SEQUENCE_ImplRepository_PV_defined
		SOM_SEQUENCE_TYPEDEF(ImplRepository_PV);
	#endif /* _IDL_SEQUENCE_ImplRepository_PV_defined */
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_PV
			#if defined(PV)
				#undef PV
				#define SOMGD_PV
			#else
				#define PV ImplRepository_PV
			#endif
		#endif /* SOMGD_PV */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef _IDL_SEQUENCE_PV_defined
			#define _IDL_SEQUENCE_PV_defined
			#define _IDL_SEQUENCE_PV _IDL_SEQUENCE_ImplRepository_PV
		#endif /* _IDL_SEQUENCE_PV_defined */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
	typedef ImplRepository_PV ImplRepository_PropertyValue;
	typedef _IDL_SEQUENCE_ImplRepository_PV ImplRepository_PVList;
	#ifndef _IDL_SEQUENCE_ImplRepository_PVList_defined
		#define _IDL_SEQUENCE_ImplRepository_PVList_defined
		SOM_SEQUENCE_TYPEDEF(ImplRepository_PVList);
	#endif /* _IDL_SEQUENCE_ImplRepository_PVList_defined */
	#ifdef __IBMC__
		typedef ImplementationDef SOMSTAR (somTP_ImplRepository_find_impldef)(
			ImplRepository SOMSTAR somSelf,
			Environment *ev,
			/* in */ ImplId implid);
		#pragma linkage(somTP_ImplRepository_find_impldef,system)
		typedef somTP_ImplRepository_find_impldef *somTD_ImplRepository_find_impldef;
	#else /* __IBMC__ */
		typedef ImplementationDef SOMSTAR (SOMLINK * somTD_ImplRepository_find_impldef)(
			ImplRepository SOMSTAR somSelf,
			Environment *ev,
			/* in */ ImplId implid);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef ImplementationDef SOMSTAR (somTP_ImplRepository_find_impldef_by_alias)(
			ImplRepository SOMSTAR somSelf,
			Environment *ev,
			/* in */ string alias_name);
		#pragma linkage(somTP_ImplRepository_find_impldef_by_alias,system)
		typedef somTP_ImplRepository_find_impldef_by_alias *somTD_ImplRepository_find_impldef_by_alias;
	#else /* __IBMC__ */
		typedef ImplementationDef SOMSTAR (SOMLINK * somTD_ImplRepository_find_impldef_by_alias)(
			ImplRepository SOMSTAR somSelf,
			Environment *ev,
			/* in */ string alias_name);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef ORBStatus (somTP_ImplRepository_find_all_aliases)(
			ImplRepository SOMSTAR somSelf,
			Environment *ev,
			/* out */ _IDL_SEQUENCE_string *impl_aliases);
		#pragma linkage(somTP_ImplRepository_find_all_aliases,system)
		typedef somTP_ImplRepository_find_all_aliases *somTD_ImplRepository_find_all_aliases;
	#else /* __IBMC__ */
		typedef ORBStatus (SOMLINK * somTD_ImplRepository_find_all_aliases)(
			ImplRepository SOMSTAR somSelf,
			Environment *ev,
			/* out */ _IDL_SEQUENCE_string *impl_aliases);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef ORBStatus (somTP_ImplRepository_find_all_impldefs)(
			ImplRepository SOMSTAR somSelf,
			Environment *ev,
			/* out */ _IDL_SEQUENCE_ImplementationDef *outimpldefs);
		#pragma linkage(somTP_ImplRepository_find_all_impldefs,system)
		typedef somTP_ImplRepository_find_all_impldefs *somTD_ImplRepository_find_all_impldefs;
	#else /* __IBMC__ */
		typedef ORBStatus (SOMLINK * somTD_ImplRepository_find_all_impldefs)(
			ImplRepository SOMSTAR somSelf,
			Environment *ev,
			/* out */ _IDL_SEQUENCE_ImplementationDef *outimpldefs);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef ORBStatus (somTP_ImplRepository_add_impldef)(
			ImplRepository SOMSTAR somSelf,
			Environment *ev,
			/* in */ ImplementationDef SOMSTAR impldef);
		#pragma linkage(somTP_ImplRepository_add_impldef,system)
		typedef somTP_ImplRepository_add_impldef *somTD_ImplRepository_add_impldef;
	#else /* __IBMC__ */
		typedef ORBStatus (SOMLINK * somTD_ImplRepository_add_impldef)(
			ImplRepository SOMSTAR somSelf,
			Environment *ev,
			/* in */ ImplementationDef SOMSTAR impldef);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef ORBStatus (somTP_ImplRepository_update_impldef)(
			ImplRepository SOMSTAR somSelf,
			Environment *ev,
			/* in */ ImplementationDef SOMSTAR impldef);
		#pragma linkage(somTP_ImplRepository_update_impldef,system)
		typedef somTP_ImplRepository_update_impldef *somTD_ImplRepository_update_impldef;
	#else /* __IBMC__ */
		typedef ORBStatus (SOMLINK * somTD_ImplRepository_update_impldef)(
			ImplRepository SOMSTAR somSelf,
			Environment *ev,
			/* in */ ImplementationDef SOMSTAR impldef);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef ORBStatus (somTP_ImplRepository_delete_impldef)(
			ImplRepository SOMSTAR somSelf,
			Environment *ev,
			/* in */ ImplId implid);
		#pragma linkage(somTP_ImplRepository_delete_impldef,system)
		typedef somTP_ImplRepository_delete_impldef *somTD_ImplRepository_delete_impldef;
	#else /* __IBMC__ */
		typedef ORBStatus (SOMLINK * somTD_ImplRepository_delete_impldef)(
			ImplRepository SOMSTAR somSelf,
			Environment *ev,
			/* in */ ImplId implid);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef _IDL_SEQUENCE_ImplementationDef (somTP_ImplRepository_find_impldef_by_class)(
			ImplRepository SOMSTAR somSelf,
			Environment *ev,
			/* in */ string classname);
		#pragma linkage(somTP_ImplRepository_find_impldef_by_class,system)
		typedef somTP_ImplRepository_find_impldef_by_class *somTD_ImplRepository_find_impldef_by_class;
	#else /* __IBMC__ */
		typedef _IDL_SEQUENCE_ImplementationDef (SOMLINK * somTD_ImplRepository_find_impldef_by_class)(
			ImplRepository SOMSTAR somSelf,
			Environment *ev,
			/* in */ string classname);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_ImplRepository_add_class_to_impldef)(
			ImplRepository SOMSTAR somSelf,
			Environment *ev,
			/* in */ ImplId implid,
			/* in */ string classname);
		#pragma linkage(somTP_ImplRepository_add_class_to_impldef,system)
		typedef somTP_ImplRepository_add_class_to_impldef *somTD_ImplRepository_add_class_to_impldef;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_ImplRepository_add_class_to_impldef)(
			ImplRepository SOMSTAR somSelf,
			Environment *ev,
			/* in */ ImplId implid,
			/* in */ string classname);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_ImplRepository_remove_class_from_impldef)(
			ImplRepository SOMSTAR somSelf,
			Environment *ev,
			/* in */ ImplId implid,
			/* in */ string classname);
		#pragma linkage(somTP_ImplRepository_remove_class_from_impldef,system)
		typedef somTP_ImplRepository_remove_class_from_impldef *somTD_ImplRepository_remove_class_from_impldef;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_ImplRepository_remove_class_from_impldef)(
			ImplRepository SOMSTAR somSelf,
			Environment *ev,
			/* in */ ImplId implid,
			/* in */ string classname);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_ImplRepository_remove_class_from_all)(
			ImplRepository SOMSTAR somSelf,
			Environment *ev,
			/* in */ string classname);
		#pragma linkage(somTP_ImplRepository_remove_class_from_all,system)
		typedef somTP_ImplRepository_remove_class_from_all *somTD_ImplRepository_remove_class_from_all;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_ImplRepository_remove_class_from_all)(
			ImplRepository SOMSTAR somSelf,
			Environment *ev,
			/* in */ string classname);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef _IDL_SEQUENCE_string (somTP_ImplRepository_find_classes_by_impldef)(
			ImplRepository SOMSTAR somSelf,
			Environment *ev,
			/* in */ ImplId implid);
		#pragma linkage(somTP_ImplRepository_find_classes_by_impldef,system)
		typedef somTP_ImplRepository_find_classes_by_impldef *somTD_ImplRepository_find_classes_by_impldef;
	#else /* __IBMC__ */
		typedef _IDL_SEQUENCE_string (SOMLINK * somTD_ImplRepository_find_classes_by_impldef)(
			ImplRepository SOMSTAR somSelf,
			Environment *ev,
			/* in */ ImplId implid);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef ORBStatus (somTP_ImplRepository_add_class_to_all)(
			ImplRepository SOMSTAR somSelf,
			Environment *ev,
			/* in */ string classname);
		#pragma linkage(somTP_ImplRepository_add_class_to_all,system)
		typedef somTP_ImplRepository_add_class_to_all *somTD_ImplRepository_add_class_to_all;
	#else /* __IBMC__ */
		typedef ORBStatus (SOMLINK * somTD_ImplRepository_add_class_to_all)(
			ImplRepository SOMSTAR somSelf,
			Environment *ev,
			/* in */ string classname);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef ORBStatus (somTP_ImplRepository_add_class_with_properties)(
			ImplRepository SOMSTAR somSelf,
			Environment *ev,
			/* in */ ImplId implid,
			/* in */ string classname,
			/* in */ ImplRepository_PVList *pvl);
		#pragma linkage(somTP_ImplRepository_add_class_with_properties,system)
		typedef somTP_ImplRepository_add_class_with_properties *somTD_ImplRepository_add_class_with_properties;
	#else /* __IBMC__ */
		typedef ORBStatus (SOMLINK * somTD_ImplRepository_add_class_with_properties)(
			ImplRepository SOMSTAR somSelf,
			Environment *ev,
			/* in */ ImplId implid,
			/* in */ string classname,
			/* in */ ImplRepository_PVList *pvl);
	#endif /* __IBMC__ */
	#ifndef ImplRepository_MajorVersion
		#define ImplRepository_MajorVersion   2
	#endif /* ImplRepository_MajorVersion */
	#ifndef ImplRepository_MinorVersion
		#define ImplRepository_MinorVersion   2
	#endif /* ImplRepository_MinorVersion */
	typedef struct ImplRepositoryClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken find_impldef;
		somMToken find_impldef_by_alias;
		somMToken private2;
		somMToken private3;
		somMToken find_impldef_by_class;
		somMToken add_impldef;
		somMToken update_impldef;
		somMToken delete_impldef;
		somMToken add_class_to_impldef;
		somMToken remove_class_from_impldef;
		somMToken remove_class_from_all;
		somMToken find_classes_by_impldef;
		somMToken private12;
		somMToken private13;
		somMToken private14;
		somMToken private15;
		somMToken find_all_impldefs;
		somMToken find_all_aliases;
		somMToken add_class_to_all;
		somMToken add_class_with_properties;
	} ImplRepositoryClassDataStructure;
	typedef struct ImplRepositoryCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} ImplRepositoryCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_implrep_Source) || defined(ImplRepository_Class_Source)
			SOMEXTERN struct ImplRepositoryClassDataStructure _ImplRepositoryClassData;
			#ifndef ImplRepositoryClassData
				#define ImplRepositoryClassData    _ImplRepositoryClassData
			#endif /* ImplRepositoryClassData */
		#else
			SOMEXTERN struct ImplRepositoryClassDataStructure * SOMLINK resolve_ImplRepositoryClassData(void);
			#ifndef ImplRepositoryClassData
				#define ImplRepositoryClassData    (*(resolve_ImplRepositoryClassData()))
			#endif /* ImplRepositoryClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_implrep_Source) || defined(ImplRepository_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_implrep_Source || ImplRepository_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_implrep_Source || ImplRepository_Class_Source */
		struct ImplRepositoryClassDataStructure SOMDLINK ImplRepositoryClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_implrep_Source) || defined(ImplRepository_Class_Source)
			SOMEXTERN struct ImplRepositoryCClassDataStructure _ImplRepositoryCClassData;
			#ifndef ImplRepositoryCClassData
				#define ImplRepositoryCClassData    _ImplRepositoryCClassData
			#endif /* ImplRepositoryCClassData */
		#else
			SOMEXTERN struct ImplRepositoryCClassDataStructure * SOMLINK resolve_ImplRepositoryCClassData(void);
			#ifndef ImplRepositoryCClassData
				#define ImplRepositoryCClassData    (*(resolve_ImplRepositoryCClassData()))
			#endif /* ImplRepositoryCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_implrep_Source) || defined(ImplRepository_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_implrep_Source || ImplRepository_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_implrep_Source || ImplRepository_Class_Source */
		struct ImplRepositoryCClassDataStructure SOMDLINK ImplRepositoryCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_implrep_Source) || defined(ImplRepository_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_implrep_Source || ImplRepository_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_implrep_Source || ImplRepository_Class_Source */
	SOMClass SOMSTAR SOMLINK ImplRepositoryNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_ImplRepository (ImplRepositoryClassData.classObject)
	#ifndef SOMGD_ImplRepository
		#if (defined(_ImplRepository) || defined(__ImplRepository))
			#undef _ImplRepository
			#undef __ImplRepository
			#define SOMGD_ImplRepository 1
		#else
			#define _ImplRepository _SOMCLASS_ImplRepository
		#endif /* _ImplRepository */
	#endif /* SOMGD_ImplRepository */
	#define ImplRepository_classObj _SOMCLASS_ImplRepository
	#define _SOMMTOKEN_ImplRepository(method) ((somMToken)(ImplRepositoryClassData.method))
	#ifndef ImplRepositoryNew
		#define ImplRepositoryNew() ( _ImplRepository ? \
			(SOMClass_somNew(_ImplRepository)) : \
			( ImplRepositoryNewClass( \
				ImplRepository_MajorVersion, \
				ImplRepository_MinorVersion), \
			SOMClass_somNew(_ImplRepository))) 
	#endif /* NewImplRepository */
	#ifndef ImplRepository_find_impldef
		#define ImplRepository_find_impldef(somSelf,ev,implid) \
			SOM_Resolve(somSelf,ImplRepository,find_impldef)  \
				(somSelf,ev,implid)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__find_impldef
				#if defined(_find_impldef)
					#undef _find_impldef
					#define SOMGD__find_impldef
				#else
					#define _find_impldef ImplRepository_find_impldef
				#endif
			#endif /* SOMGD__find_impldef */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplRepository_find_impldef */
	#ifndef ImplRepository_find_impldef_by_alias
		#define ImplRepository_find_impldef_by_alias(somSelf,ev,alias_name) \
			SOM_Resolve(somSelf,ImplRepository,find_impldef_by_alias)  \
				(somSelf,ev,alias_name)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__find_impldef_by_alias
				#if defined(_find_impldef_by_alias)
					#undef _find_impldef_by_alias
					#define SOMGD__find_impldef_by_alias
				#else
					#define _find_impldef_by_alias ImplRepository_find_impldef_by_alias
				#endif
			#endif /* SOMGD__find_impldef_by_alias */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplRepository_find_impldef_by_alias */
	#ifndef ImplRepository_find_all_aliases
		#define ImplRepository_find_all_aliases(somSelf,ev,impl_aliases) \
			SOM_Resolve(somSelf,ImplRepository,find_all_aliases)  \
				(somSelf,ev,impl_aliases)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__find_all_aliases
				#if defined(_find_all_aliases)
					#undef _find_all_aliases
					#define SOMGD__find_all_aliases
				#else
					#define _find_all_aliases ImplRepository_find_all_aliases
				#endif
			#endif /* SOMGD__find_all_aliases */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplRepository_find_all_aliases */
	#ifndef ImplRepository_find_all_impldefs
		#define ImplRepository_find_all_impldefs(somSelf,ev,outimpldefs) \
			SOM_Resolve(somSelf,ImplRepository,find_all_impldefs)  \
				(somSelf,ev,outimpldefs)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__find_all_impldefs
				#if defined(_find_all_impldefs)
					#undef _find_all_impldefs
					#define SOMGD__find_all_impldefs
				#else
					#define _find_all_impldefs ImplRepository_find_all_impldefs
				#endif
			#endif /* SOMGD__find_all_impldefs */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplRepository_find_all_impldefs */
	#ifndef ImplRepository_add_impldef
		#define ImplRepository_add_impldef(somSelf,ev,impldef) \
			SOM_Resolve(somSelf,ImplRepository,add_impldef)  \
				(somSelf,ev,impldef)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__add_impldef
				#if defined(_add_impldef)
					#undef _add_impldef
					#define SOMGD__add_impldef
				#else
					#define _add_impldef ImplRepository_add_impldef
				#endif
			#endif /* SOMGD__add_impldef */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplRepository_add_impldef */
	#ifndef ImplRepository_update_impldef
		#define ImplRepository_update_impldef(somSelf,ev,impldef) \
			SOM_Resolve(somSelf,ImplRepository,update_impldef)  \
				(somSelf,ev,impldef)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__update_impldef
				#if defined(_update_impldef)
					#undef _update_impldef
					#define SOMGD__update_impldef
				#else
					#define _update_impldef ImplRepository_update_impldef
				#endif
			#endif /* SOMGD__update_impldef */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplRepository_update_impldef */
	#ifndef ImplRepository_delete_impldef
		#define ImplRepository_delete_impldef(somSelf,ev,implid) \
			SOM_Resolve(somSelf,ImplRepository,delete_impldef)  \
				(somSelf,ev,implid)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__delete_impldef
				#if defined(_delete_impldef)
					#undef _delete_impldef
					#define SOMGD__delete_impldef
				#else
					#define _delete_impldef ImplRepository_delete_impldef
				#endif
			#endif /* SOMGD__delete_impldef */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplRepository_delete_impldef */
	#ifndef ImplRepository_find_impldef_by_class
		#define ImplRepository_find_impldef_by_class(somSelf,ev,classname) \
			SOM_Resolve(somSelf,ImplRepository,find_impldef_by_class)  \
				(somSelf,ev,classname)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__find_impldef_by_class
				#if defined(_find_impldef_by_class)
					#undef _find_impldef_by_class
					#define SOMGD__find_impldef_by_class
				#else
					#define _find_impldef_by_class ImplRepository_find_impldef_by_class
				#endif
			#endif /* SOMGD__find_impldef_by_class */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplRepository_find_impldef_by_class */
	#ifndef ImplRepository_add_class_to_impldef
		#define ImplRepository_add_class_to_impldef(somSelf,ev,implid,classname) \
			SOM_Resolve(somSelf,ImplRepository,add_class_to_impldef)  \
				(somSelf,ev,implid,classname)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__add_class_to_impldef
				#if defined(_add_class_to_impldef)
					#undef _add_class_to_impldef
					#define SOMGD__add_class_to_impldef
				#else
					#define _add_class_to_impldef ImplRepository_add_class_to_impldef
				#endif
			#endif /* SOMGD__add_class_to_impldef */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplRepository_add_class_to_impldef */
	#ifndef ImplRepository_remove_class_from_impldef
		#define ImplRepository_remove_class_from_impldef(somSelf,ev,implid,classname) \
			SOM_Resolve(somSelf,ImplRepository,remove_class_from_impldef)  \
				(somSelf,ev,implid,classname)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__remove_class_from_impldef
				#if defined(_remove_class_from_impldef)
					#undef _remove_class_from_impldef
					#define SOMGD__remove_class_from_impldef
				#else
					#define _remove_class_from_impldef ImplRepository_remove_class_from_impldef
				#endif
			#endif /* SOMGD__remove_class_from_impldef */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplRepository_remove_class_from_impldef */
	#ifndef ImplRepository_remove_class_from_all
		#define ImplRepository_remove_class_from_all(somSelf,ev,classname) \
			SOM_Resolve(somSelf,ImplRepository,remove_class_from_all)  \
				(somSelf,ev,classname)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__remove_class_from_all
				#if defined(_remove_class_from_all)
					#undef _remove_class_from_all
					#define SOMGD__remove_class_from_all
				#else
					#define _remove_class_from_all ImplRepository_remove_class_from_all
				#endif
			#endif /* SOMGD__remove_class_from_all */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplRepository_remove_class_from_all */
	#ifndef ImplRepository_find_classes_by_impldef
		#define ImplRepository_find_classes_by_impldef(somSelf,ev,implid) \
			SOM_Resolve(somSelf,ImplRepository,find_classes_by_impldef)  \
				(somSelf,ev,implid)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__find_classes_by_impldef
				#if defined(_find_classes_by_impldef)
					#undef _find_classes_by_impldef
					#define SOMGD__find_classes_by_impldef
				#else
					#define _find_classes_by_impldef ImplRepository_find_classes_by_impldef
				#endif
			#endif /* SOMGD__find_classes_by_impldef */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplRepository_find_classes_by_impldef */
	#ifndef ImplRepository_add_class_to_all
		#define ImplRepository_add_class_to_all(somSelf,ev,classname) \
			SOM_Resolve(somSelf,ImplRepository,add_class_to_all)  \
				(somSelf,ev,classname)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__add_class_to_all
				#if defined(_add_class_to_all)
					#undef _add_class_to_all
					#define SOMGD__add_class_to_all
				#else
					#define _add_class_to_all ImplRepository_add_class_to_all
				#endif
			#endif /* SOMGD__add_class_to_all */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplRepository_add_class_to_all */
	#ifndef ImplRepository_add_class_with_properties
		#define ImplRepository_add_class_with_properties(somSelf,ev,implid,classname,pvl) \
			SOM_Resolve(somSelf,ImplRepository,add_class_with_properties)  \
				(somSelf,ev,implid,classname,pvl)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__add_class_with_properties
				#if defined(_add_class_with_properties)
					#undef _add_class_with_properties
					#define SOMGD__add_class_with_properties
				#else
					#define _add_class_with_properties ImplRepository_add_class_with_properties
				#endif
			#endif /* SOMGD__add_class_with_properties */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* ImplRepository_add_class_with_properties */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define ImplRepository_somInit SOMObject_somInit
#define ImplRepository_somUninit SOMObject_somUninit
#define ImplRepository_somFree SOMObject_somFree
#define ImplRepository_somGetClass SOMObject_somGetClass
#define ImplRepository_somGetClassName SOMObject_somGetClassName
#define ImplRepository_somGetSize SOMObject_somGetSize
#define ImplRepository_somIsA SOMObject_somIsA
#define ImplRepository_somIsInstanceOf SOMObject_somIsInstanceOf
#define ImplRepository_somRespondsTo SOMObject_somRespondsTo
#define ImplRepository_somDispatch SOMObject_somDispatch
#define ImplRepository_somClassDispatch SOMObject_somClassDispatch
#define ImplRepository_somCastObj SOMObject_somCastObj
#define ImplRepository_somResetObj SOMObject_somResetObj
#define ImplRepository_somPrintSelf SOMObject_somPrintSelf
#define ImplRepository_somDumpSelf SOMObject_somDumpSelf
#define ImplRepository_somDumpSelfInt SOMObject_somDumpSelfInt
#define ImplRepository_somDefaultInit SOMObject_somDefaultInit
#define ImplRepository_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define ImplRepository_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define ImplRepository_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define ImplRepository_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define ImplRepository_somDefaultAssign SOMObject_somDefaultAssign
#define ImplRepository_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define ImplRepository_somDefaultVAssign SOMObject_somDefaultVAssign
#define ImplRepository_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define ImplRepository_somDestruct SOMObject_somDestruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_implrep_Header_h */
