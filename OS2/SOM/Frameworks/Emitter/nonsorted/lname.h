/* generated from lname.idl */
/* internal conditional is SOM_Module_lname_Source */
#ifndef SOM_Module_lname_Header_h
	#define SOM_Module_lname_Header_h 1
	#include <som.h>
	#include <lnamec.h>
	#include <xnaming.h>
	#include <naming.h>
	#include <somobj.h>
	#ifndef _IDL_LName_defined
		#define _IDL_LName_defined
		typedef SOMObject LName;
	#endif /* _IDL_LName_defined */
	#ifndef _IDL_SEQUENCE_LName_defined
		#define _IDL_SEQUENCE_LName_defined
		SOM_SEQUENCE_TYPEDEF_NAME(LName SOMSTAR ,sequence(LName));
	#endif /* _IDL_SEQUENCE_LName_defined */
#define ex_LName_NoComponent   "::LName::NoComponent"
#ifndef SOM_DONT_USE_SHORT_NAMES
#ifndef SOMTGD_ex_NoComponent
#ifndef ex_NoComponent
#define ex_NoComponent  ex_LName_NoComponent
#else
#define SOMTGD_ex_NoComponent
#undef ex_NoComponent
#endif /* ex_NoComponent */
#endif /* SOMTGD_ex_NoComponent */
#endif /* SOM_DONT_USE_SHORT_NAMES */
typedef struct LName_NoComponent LName_NoComponent;
#ifndef SOM_DONT_USE_SHORT_NAMES
	#ifndef SOMGD_NoComponent
		#if defined(NoComponent)
			#undef NoComponent
			#define SOMGD_NoComponent
		#else
			#define NoComponent LName_NoComponent
		#endif
	#endif /* SOMGD_NoComponent */
#endif /* SOM_DONT_USE_SHORT_NAMES */
#define ex_LName_OverFlow   "::LName::OverFlow"
#ifndef SOM_DONT_USE_SHORT_NAMES
#ifndef SOMTGD_ex_OverFlow
#ifndef ex_OverFlow
#define ex_OverFlow  ex_LName_OverFlow
#else
#define SOMTGD_ex_OverFlow
#undef ex_OverFlow
#endif /* ex_OverFlow */
#endif /* SOMTGD_ex_OverFlow */
#endif /* SOM_DONT_USE_SHORT_NAMES */
typedef struct LName_OverFlow LName_OverFlow;
#ifndef SOM_DONT_USE_SHORT_NAMES
	#ifndef SOMGD_OverFlow
		#if defined(OverFlow)
			#undef OverFlow
			#define SOMGD_OverFlow
		#else
			#define OverFlow LName_OverFlow
		#endif
	#endif /* SOMGD_OverFlow */
#endif /* SOM_DONT_USE_SHORT_NAMES */
#define ex_LName_InvalidName   "::LName::InvalidName"
#ifndef SOM_DONT_USE_SHORT_NAMES
#ifndef SOMTGD_ex_InvalidName
#ifndef ex_InvalidName
#define ex_InvalidName  ex_LName_InvalidName
#else
#define SOMTGD_ex_InvalidName
#undef ex_InvalidName
#endif /* ex_InvalidName */
#endif /* SOMTGD_ex_InvalidName */
#endif /* SOM_DONT_USE_SHORT_NAMES */
typedef struct LName_InvalidName LName_InvalidName;
#ifndef SOM_DONT_USE_SHORT_NAMES
	#ifndef SOMGD_InvalidName
		#if defined(InvalidName)
			#undef InvalidName
			#define SOMGD_InvalidName
		#else
			#define InvalidName LName_InvalidName
		#endif
	#endif /* SOMGD_InvalidName */
#endif /* SOM_DONT_USE_SHORT_NAMES */
	#ifdef __IBMC__
		typedef LName SOMSTAR (somTP_LName_insert_component)(
			LName SOMSTAR somSelf,
			Environment *ev,
			/* in */ unsigned long i,
			/* in */ LNameComponent SOMSTAR n);
		#pragma linkage(somTP_LName_insert_component,system)
		typedef somTP_LName_insert_component *somTD_LName_insert_component;
	#else /* __IBMC__ */
		typedef LName SOMSTAR (SOMLINK * somTD_LName_insert_component)(
			LName SOMSTAR somSelf,
			Environment *ev,
			/* in */ unsigned long i,
			/* in */ LNameComponent SOMSTAR n);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef LNameComponent SOMSTAR (somTP_LName_get_component)(
			LName SOMSTAR somSelf,
			Environment *ev,
			/* in */ unsigned long i);
		#pragma linkage(somTP_LName_get_component,system)
		typedef somTP_LName_get_component *somTD_LName_get_component;
	#else /* __IBMC__ */
		typedef LNameComponent SOMSTAR (SOMLINK * somTD_LName_get_component)(
			LName SOMSTAR somSelf,
			Environment *ev,
			/* in */ unsigned long i);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef LNameComponent SOMSTAR (somTP_LName_delete_component)(
			LName SOMSTAR somSelf,
			Environment *ev,
			/* in */ unsigned long i);
		#pragma linkage(somTP_LName_delete_component,system)
		typedef somTP_LName_delete_component *somTD_LName_delete_component;
	#else /* __IBMC__ */
		typedef LNameComponent SOMSTAR (SOMLINK * somTD_LName_delete_component)(
			LName SOMSTAR somSelf,
			Environment *ev,
			/* in */ unsigned long i);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef unsigned long (somTP_LName_num_components)(
			LName SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_LName_num_components,system)
		typedef somTP_LName_num_components *somTD_LName_num_components;
	#else /* __IBMC__ */
		typedef unsigned long (SOMLINK * somTD_LName_num_components)(
			LName SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef boolean (somTP_LName_equal)(
			LName SOMSTAR somSelf,
			Environment *ev,
			/* in */ LName SOMSTAR ln);
		#pragma linkage(somTP_LName_equal,system)
		typedef somTP_LName_equal *somTD_LName_equal;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_LName_equal)(
			LName SOMSTAR somSelf,
			Environment *ev,
			/* in */ LName SOMSTAR ln);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef boolean (somTP_LName_less_than)(
			LName SOMSTAR somSelf,
			Environment *ev,
			/* in */ LName SOMSTAR ln);
		#pragma linkage(somTP_LName_less_than,system)
		typedef somTP_LName_less_than *somTD_LName_less_than;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_LName_less_than)(
			LName SOMSTAR somSelf,
			Environment *ev,
			/* in */ LName SOMSTAR ln);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef CosNaming_Name (somTP_LName_to_idl_form)(
			LName SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_LName_to_idl_form,system)
		typedef somTP_LName_to_idl_form *somTD_LName_to_idl_form;
	#else /* __IBMC__ */
		typedef CosNaming_Name (SOMLINK * somTD_LName_to_idl_form)(
			LName SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_LName_from_idl_form)(
			LName SOMSTAR somSelf,
			Environment *ev,
			/* in */ CosNaming_Name *n);
		#pragma linkage(somTP_LName_from_idl_form,system)
		typedef somTP_LName_from_idl_form *somTD_LName_from_idl_form;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_LName_from_idl_form)(
			LName SOMSTAR somSelf,
			Environment *ev,
			/* in */ CosNaming_Name *n);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_LName_destroy)(
			LName SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_LName_destroy,system)
		typedef somTP_LName_destroy *somTD_LName_destroy;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_LName_destroy)(
			LName SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifndef LName_MajorVersion
		#define LName_MajorVersion   3
	#endif /* LName_MajorVersion */
	#ifndef LName_MinorVersion
		#define LName_MinorVersion   0
	#endif /* LName_MinorVersion */
	typedef struct LNameClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken to_idl_form;
		somMToken insert_component;
		somMToken get_component;
		somMToken delete_component;
		somMToken num_components;
		somMToken equal;
		somMToken less_than;
		somMToken from_idl_form;
		somMToken destroy;
	} LNameClassDataStructure;
	typedef struct LNameCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} LNameCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_lname_Source) || defined(LName_Class_Source)
			SOMEXTERN struct LNameClassDataStructure _LNameClassData;
			#ifndef LNameClassData
				#define LNameClassData    _LNameClassData
			#endif /* LNameClassData */
		#else
			SOMEXTERN struct LNameClassDataStructure * SOMLINK resolve_LNameClassData(void);
			#ifndef LNameClassData
				#define LNameClassData    (*(resolve_LNameClassData()))
			#endif /* LNameClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_lname_Source) || defined(LName_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_lname_Source || LName_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_lname_Source || LName_Class_Source */
		struct LNameClassDataStructure SOMDLINK LNameClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_lname_Source) || defined(LName_Class_Source)
			SOMEXTERN struct LNameCClassDataStructure _LNameCClassData;
			#ifndef LNameCClassData
				#define LNameCClassData    _LNameCClassData
			#endif /* LNameCClassData */
		#else
			SOMEXTERN struct LNameCClassDataStructure * SOMLINK resolve_LNameCClassData(void);
			#ifndef LNameCClassData
				#define LNameCClassData    (*(resolve_LNameCClassData()))
			#endif /* LNameCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_lname_Source) || defined(LName_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_lname_Source || LName_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_lname_Source || LName_Class_Source */
		struct LNameCClassDataStructure SOMDLINK LNameCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_lname_Source) || defined(LName_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_lname_Source || LName_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_lname_Source || LName_Class_Source */
	SOMClass SOMSTAR SOMLINK LNameNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_LName (LNameClassData.classObject)
	#ifndef SOMGD_LName
		#if (defined(_LName) || defined(__LName))
			#undef _LName
			#undef __LName
			#define SOMGD_LName 1
		#else
			#define _LName _SOMCLASS_LName
		#endif /* _LName */
	#endif /* SOMGD_LName */
	#define LName_classObj _SOMCLASS_LName
	#define _SOMMTOKEN_LName(method) ((somMToken)(LNameClassData.method))
	#ifndef LNameNew
		#define LNameNew() ( _LName ? \
			(SOMClass_somNew(_LName)) : \
			( LNameNewClass( \
				LName_MajorVersion, \
				LName_MinorVersion), \
			SOMClass_somNew(_LName))) 
	#endif /* NewLName */
	#ifndef LName_insert_component
		#define LName_insert_component(somSelf,ev,i,n) \
			SOM_Resolve(somSelf,LName,insert_component)  \
				(somSelf,ev,i,n)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__insert_component
				#if defined(_insert_component)
					#undef _insert_component
					#define SOMGD__insert_component
				#else
					#define _insert_component LName_insert_component
				#endif
			#endif /* SOMGD__insert_component */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* LName_insert_component */
	#ifndef LName_get_component
		#define LName_get_component(somSelf,ev,i) \
			SOM_Resolve(somSelf,LName,get_component)  \
				(somSelf,ev,i)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__get_component
				#if defined(_get_component)
					#undef _get_component
					#define SOMGD__get_component
				#else
					#define _get_component LName_get_component
				#endif
			#endif /* SOMGD__get_component */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* LName_get_component */
	#ifndef LName_delete_component
		#define LName_delete_component(somSelf,ev,i) \
			SOM_Resolve(somSelf,LName,delete_component)  \
				(somSelf,ev,i)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__delete_component
				#if defined(_delete_component)
					#undef _delete_component
					#define SOMGD__delete_component
				#else
					#define _delete_component LName_delete_component
				#endif
			#endif /* SOMGD__delete_component */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* LName_delete_component */
	#ifndef LName_num_components
		#define LName_num_components(somSelf,ev) \
			SOM_Resolve(somSelf,LName,num_components)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__num_components
				#if defined(_num_components)
					#undef _num_components
					#define SOMGD__num_components
				#else
					#define _num_components LName_num_components
				#endif
			#endif /* SOMGD__num_components */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* LName_num_components */
	#ifndef LName_equal
		#define LName_equal(somSelf,ev,ln) \
			SOM_Resolve(somSelf,LName,equal)  \
				(somSelf,ev,ln)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__equal
				#if defined(_equal)
					#undef _equal
					#define SOMGD__equal
				#else
					#define _equal LName_equal
				#endif
			#endif /* SOMGD__equal */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* LName_equal */
	#ifndef LName_less_than
		#define LName_less_than(somSelf,ev,ln) \
			SOM_Resolve(somSelf,LName,less_than)  \
				(somSelf,ev,ln)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__less_than
				#if defined(_less_than)
					#undef _less_than
					#define SOMGD__less_than
				#else
					#define _less_than LName_less_than
				#endif
			#endif /* SOMGD__less_than */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* LName_less_than */
	#ifndef LName_to_idl_form
		#define LName_to_idl_form(somSelf,ev) \
			SOM_Resolve(somSelf,LName,to_idl_form)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__to_idl_form
				#if defined(_to_idl_form)
					#undef _to_idl_form
					#define SOMGD__to_idl_form
				#else
					#define _to_idl_form LName_to_idl_form
				#endif
			#endif /* SOMGD__to_idl_form */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* LName_to_idl_form */
	#ifndef LName_from_idl_form
		#define LName_from_idl_form(somSelf,ev,n) \
			SOM_Resolve(somSelf,LName,from_idl_form)  \
				(somSelf,ev,n)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__from_idl_form
				#if defined(_from_idl_form)
					#undef _from_idl_form
					#define SOMGD__from_idl_form
				#else
					#define _from_idl_form LName_from_idl_form
				#endif
			#endif /* SOMGD__from_idl_form */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* LName_from_idl_form */
	#ifndef LName_destroy
		#define LName_destroy(somSelf,ev) \
			SOM_Resolve(somSelf,LName,destroy)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__destroy
				#if defined(_destroy)
					#undef _destroy
					#define SOMGD__destroy
				#else
					#define _destroy LName_destroy
				#endif
			#endif /* SOMGD__destroy */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* LName_destroy */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define LName_somInit SOMObject_somInit
#define LName_somUninit SOMObject_somUninit
#define LName_somFree SOMObject_somFree
#define LName_somGetClass SOMObject_somGetClass
#define LName_somGetClassName SOMObject_somGetClassName
#define LName_somGetSize SOMObject_somGetSize
#define LName_somIsA SOMObject_somIsA
#define LName_somIsInstanceOf SOMObject_somIsInstanceOf
#define LName_somRespondsTo SOMObject_somRespondsTo
#define LName_somDispatch SOMObject_somDispatch
#define LName_somClassDispatch SOMObject_somClassDispatch
#define LName_somCastObj SOMObject_somCastObj
#define LName_somResetObj SOMObject_somResetObj
#define LName_somPrintSelf SOMObject_somPrintSelf
#define LName_somDumpSelf SOMObject_somDumpSelf
#define LName_somDumpSelfInt SOMObject_somDumpSelfInt
#define LName_somDefaultInit SOMObject_somDefaultInit
#define LName_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define LName_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define LName_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define LName_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define LName_somDefaultAssign SOMObject_somDefaultAssign
#define LName_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define LName_somDefaultVAssign SOMObject_somDefaultVAssign
#define LName_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define LName_somDestruct SOMObject_somDestruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_lname_Header_h */
