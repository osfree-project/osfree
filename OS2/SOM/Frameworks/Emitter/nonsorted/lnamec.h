/* generated from lnamec.idl */
/* internal conditional is SOM_Module_lnamec_Source */
#ifndef SOM_Module_lnamec_Header_h
	#define SOM_Module_lnamec_Header_h 1
	#include <som.h>
	#include <xnaming.h>
	#include <naming.h>
	#include <somobj.h>
	#ifndef _IDL_LNameComponent_defined
		#define _IDL_LNameComponent_defined
		typedef SOMObject LNameComponent;
	#endif /* _IDL_LNameComponent_defined */
	#ifndef _IDL_SEQUENCE_LNameComponent_defined
		#define _IDL_SEQUENCE_LNameComponent_defined
		SOM_SEQUENCE_TYPEDEF_NAME(LNameComponent SOMSTAR ,sequence(LNameComponent));
	#endif /* _IDL_SEQUENCE_LNameComponent_defined */
#define ex_LNameComponent_NotSet   "::LNameComponent::NotSet"
#ifndef SOM_DONT_USE_SHORT_NAMES
#ifndef SOMTGD_ex_NotSet
#ifndef ex_NotSet
#define ex_NotSet  ex_LNameComponent_NotSet
#else
#define SOMTGD_ex_NotSet
#undef ex_NotSet
#endif /* ex_NotSet */
#endif /* SOMTGD_ex_NotSet */
#endif /* SOM_DONT_USE_SHORT_NAMES */
typedef struct LNameComponent_NotSet LNameComponent_NotSet;
#ifndef SOM_DONT_USE_SHORT_NAMES
	#ifndef SOMGD_NotSet
		#if defined(NotSet)
			#undef NotSet
			#define SOMGD_NotSet
		#else
			#define NotSet LNameComponent_NotSet
		#endif
	#endif /* SOMGD_NotSet */
#endif /* SOM_DONT_USE_SHORT_NAMES */
	#ifdef __IBMC__
		typedef string (somTP_LNameComponent_get_id)(
			LNameComponent SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_LNameComponent_get_id,system)
		typedef somTP_LNameComponent_get_id *somTD_LNameComponent_get_id;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_LNameComponent_get_id)(
			LNameComponent SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_LNameComponent_set_id)(
			LNameComponent SOMSTAR somSelf,
			Environment *ev,
			/* in */ string i);
		#pragma linkage(somTP_LNameComponent_set_id,system)
		typedef somTP_LNameComponent_set_id *somTD_LNameComponent_set_id;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_LNameComponent_set_id)(
			LNameComponent SOMSTAR somSelf,
			Environment *ev,
			/* in */ string i);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_LNameComponent_get_kind)(
			LNameComponent SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_LNameComponent_get_kind,system)
		typedef somTP_LNameComponent_get_kind *somTD_LNameComponent_get_kind;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_LNameComponent_get_kind)(
			LNameComponent SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_LNameComponent_set_kind)(
			LNameComponent SOMSTAR somSelf,
			Environment *ev,
			/* in */ string k);
		#pragma linkage(somTP_LNameComponent_set_kind,system)
		typedef somTP_LNameComponent_set_kind *somTD_LNameComponent_set_kind;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_LNameComponent_set_kind)(
			LNameComponent SOMSTAR somSelf,
			Environment *ev,
			/* in */ string k);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_LNameComponent_destroy)(
			LNameComponent SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_LNameComponent_destroy,system)
		typedef somTP_LNameComponent_destroy *somTD_LNameComponent_destroy;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_LNameComponent_destroy)(
			LNameComponent SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifndef LNameComponent_MajorVersion
		#define LNameComponent_MajorVersion   3
	#endif /* LNameComponent_MajorVersion */
	#ifndef LNameComponent_MinorVersion
		#define LNameComponent_MinorVersion   0
	#endif /* LNameComponent_MinorVersion */
	typedef struct LNameComponentClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken get_id;
		somMToken set_id;
		somMToken get_kind;
		somMToken set_kind;
		somMToken destroy;
	} LNameComponentClassDataStructure;
	typedef struct LNameComponentCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} LNameComponentCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_lnamec_Source) || defined(LNameComponent_Class_Source)
			SOMEXTERN struct LNameComponentClassDataStructure _LNameComponentClassData;
			#ifndef LNameComponentClassData
				#define LNameComponentClassData    _LNameComponentClassData
			#endif /* LNameComponentClassData */
		#else
			SOMEXTERN struct LNameComponentClassDataStructure * SOMLINK resolve_LNameComponentClassData(void);
			#ifndef LNameComponentClassData
				#define LNameComponentClassData    (*(resolve_LNameComponentClassData()))
			#endif /* LNameComponentClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_lnamec_Source) || defined(LNameComponent_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_lnamec_Source || LNameComponent_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_lnamec_Source || LNameComponent_Class_Source */
		struct LNameComponentClassDataStructure SOMDLINK LNameComponentClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_lnamec_Source) || defined(LNameComponent_Class_Source)
			SOMEXTERN struct LNameComponentCClassDataStructure _LNameComponentCClassData;
			#ifndef LNameComponentCClassData
				#define LNameComponentCClassData    _LNameComponentCClassData
			#endif /* LNameComponentCClassData */
		#else
			SOMEXTERN struct LNameComponentCClassDataStructure * SOMLINK resolve_LNameComponentCClassData(void);
			#ifndef LNameComponentCClassData
				#define LNameComponentCClassData    (*(resolve_LNameComponentCClassData()))
			#endif /* LNameComponentCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_lnamec_Source) || defined(LNameComponent_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_lnamec_Source || LNameComponent_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_lnamec_Source || LNameComponent_Class_Source */
		struct LNameComponentCClassDataStructure SOMDLINK LNameComponentCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_lnamec_Source) || defined(LNameComponent_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_lnamec_Source || LNameComponent_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_lnamec_Source || LNameComponent_Class_Source */
	SOMClass SOMSTAR SOMLINK LNameComponentNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_LNameComponent (LNameComponentClassData.classObject)
	#ifndef SOMGD_LNameComponent
		#if (defined(_LNameComponent) || defined(__LNameComponent))
			#undef _LNameComponent
			#undef __LNameComponent
			#define SOMGD_LNameComponent 1
		#else
			#define _LNameComponent _SOMCLASS_LNameComponent
		#endif /* _LNameComponent */
	#endif /* SOMGD_LNameComponent */
	#define LNameComponent_classObj _SOMCLASS_LNameComponent
	#define _SOMMTOKEN_LNameComponent(method) ((somMToken)(LNameComponentClassData.method))
	#ifndef LNameComponentNew
		#define LNameComponentNew() ( _LNameComponent ? \
			(SOMClass_somNew(_LNameComponent)) : \
			( LNameComponentNewClass( \
				LNameComponent_MajorVersion, \
				LNameComponent_MinorVersion), \
			SOMClass_somNew(_LNameComponent))) 
	#endif /* NewLNameComponent */
	#ifndef LNameComponent_get_id
		#define LNameComponent_get_id(somSelf,ev) \
			SOM_Resolve(somSelf,LNameComponent,get_id)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__get_id
				#if defined(_get_id)
					#undef _get_id
					#define SOMGD__get_id
				#else
					#define _get_id LNameComponent_get_id
				#endif
			#endif /* SOMGD__get_id */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* LNameComponent_get_id */
	#ifndef LNameComponent_set_id
		#define LNameComponent_set_id(somSelf,ev,i) \
			SOM_Resolve(somSelf,LNameComponent,set_id)  \
				(somSelf,ev,i)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__set_id
				#if defined(_set_id)
					#undef _set_id
					#define SOMGD__set_id
				#else
					#define _set_id LNameComponent_set_id
				#endif
			#endif /* SOMGD__set_id */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* LNameComponent_set_id */
	#ifndef LNameComponent_get_kind
		#define LNameComponent_get_kind(somSelf,ev) \
			SOM_Resolve(somSelf,LNameComponent,get_kind)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__get_kind
				#if defined(_get_kind)
					#undef _get_kind
					#define SOMGD__get_kind
				#else
					#define _get_kind LNameComponent_get_kind
				#endif
			#endif /* SOMGD__get_kind */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* LNameComponent_get_kind */
	#ifndef LNameComponent_set_kind
		#define LNameComponent_set_kind(somSelf,ev,k) \
			SOM_Resolve(somSelf,LNameComponent,set_kind)  \
				(somSelf,ev,k)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__set_kind
				#if defined(_set_kind)
					#undef _set_kind
					#define SOMGD__set_kind
				#else
					#define _set_kind LNameComponent_set_kind
				#endif
			#endif /* SOMGD__set_kind */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* LNameComponent_set_kind */
	#ifndef LNameComponent_destroy
		#define LNameComponent_destroy(somSelf,ev) \
			SOM_Resolve(somSelf,LNameComponent,destroy)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__destroy
				#if defined(_destroy)
					#undef _destroy
					#define SOMGD__destroy
				#else
					#define _destroy LNameComponent_destroy
				#endif
			#endif /* SOMGD__destroy */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* LNameComponent_destroy */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define LNameComponent_somInit SOMObject_somInit
#define LNameComponent_somUninit SOMObject_somUninit
#define LNameComponent_somFree SOMObject_somFree
#define LNameComponent_somGetClass SOMObject_somGetClass
#define LNameComponent_somGetClassName SOMObject_somGetClassName
#define LNameComponent_somGetSize SOMObject_somGetSize
#define LNameComponent_somIsA SOMObject_somIsA
#define LNameComponent_somIsInstanceOf SOMObject_somIsInstanceOf
#define LNameComponent_somRespondsTo SOMObject_somRespondsTo
#define LNameComponent_somDispatch SOMObject_somDispatch
#define LNameComponent_somClassDispatch SOMObject_somClassDispatch
#define LNameComponent_somCastObj SOMObject_somCastObj
#define LNameComponent_somResetObj SOMObject_somResetObj
#define LNameComponent_somPrintSelf SOMObject_somPrintSelf
#define LNameComponent_somDumpSelf SOMObject_somDumpSelf
#define LNameComponent_somDumpSelfInt SOMObject_somDumpSelfInt
#define LNameComponent_somDefaultInit SOMObject_somDefaultInit
#define LNameComponent_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define LNameComponent_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define LNameComponent_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define LNameComponent_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define LNameComponent_somDefaultAssign SOMObject_somDefaultAssign
#define LNameComponent_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define LNameComponent_somDefaultVAssign SOMObject_somDefaultVAssign
#define LNameComponent_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define LNameComponent_somDestruct SOMObject_somDestruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_lnamec_Header_h */
