/* generated from nvlist.idl */
/* internal conditional is SOM_Module_nvlist_Source */
#ifndef SOM_Module_nvlist_Header_h
	#define SOM_Module_nvlist_Header_h 1
	#include <som.h>
	#include <somobj.h>
	#include <somdtype.h>
	#include <containd.h>
	#ifndef _IDL_NVList_defined
		#define _IDL_NVList_defined
		typedef SOMObject NVList;
	#endif /* _IDL_NVList_defined */
	#ifndef _IDL_SEQUENCE_NVList_defined
		#define _IDL_SEQUENCE_NVList_defined
		SOM_SEQUENCE_TYPEDEF_NAME(NVList SOMSTAR ,sequence(NVList));
	#endif /* _IDL_SEQUENCE_NVList_defined */
	#ifdef __IBMC__
		typedef ORBStatus (somTP_NVList_add_item)(
			NVList SOMSTAR somSelf,
			Environment *ev,
			/* in */ Identifier item_name,
			/* in */ TypeCode item_type,
			/* in */ void *value,
			/* in */ long value_len,
			/* in */ Flags item_flags);
		#pragma linkage(somTP_NVList_add_item,system)
		typedef somTP_NVList_add_item *somTD_NVList_add_item;
	#else /* __IBMC__ */
		typedef ORBStatus (SOMLINK * somTD_NVList_add_item)(
			NVList SOMSTAR somSelf,
			Environment *ev,
			/* in */ Identifier item_name,
			/* in */ TypeCode item_type,
			/* in */ void *value,
			/* in */ long value_len,
			/* in */ Flags item_flags);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef ORBStatus (somTP_NVList_free)(
			NVList SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_NVList_free,system)
		typedef somTP_NVList_free *somTD_NVList_free;
	#else /* __IBMC__ */
		typedef ORBStatus (SOMLINK * somTD_NVList_free)(
			NVList SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef ORBStatus (somTP_NVList_free_memory)(
			NVList SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_NVList_free_memory,system)
		typedef somTP_NVList_free_memory *somTD_NVList_free_memory;
	#else /* __IBMC__ */
		typedef ORBStatus (SOMLINK * somTD_NVList_free_memory)(
			NVList SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef ORBStatus (somTP_NVList_get_count)(
			NVList SOMSTAR somSelf,
			Environment *ev,
			/* out */ long *count);
		#pragma linkage(somTP_NVList_get_count,system)
		typedef somTP_NVList_get_count *somTD_NVList_get_count;
	#else /* __IBMC__ */
		typedef ORBStatus (SOMLINK * somTD_NVList_get_count)(
			NVList SOMSTAR somSelf,
			Environment *ev,
			/* out */ long *count);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef ORBStatus (somTP_NVList_set_item)(
			NVList SOMSTAR somSelf,
			Environment *ev,
			/* in */ long item_number,
			/* in */ Identifier item_name,
			/* in */ TypeCode item_type,
			/* in */ void *value,
			/* in */ long value_len,
			/* in */ Flags item_flags);
		#pragma linkage(somTP_NVList_set_item,system)
		typedef somTP_NVList_set_item *somTD_NVList_set_item;
	#else /* __IBMC__ */
		typedef ORBStatus (SOMLINK * somTD_NVList_set_item)(
			NVList SOMSTAR somSelf,
			Environment *ev,
			/* in */ long item_number,
			/* in */ Identifier item_name,
			/* in */ TypeCode item_type,
			/* in */ void *value,
			/* in */ long value_len,
			/* in */ Flags item_flags);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef ORBStatus (somTP_NVList_get_item)(
			NVList SOMSTAR somSelf,
			Environment *ev,
			/* in */ long item_number,
			/* out */ Identifier *item_name,
			/* out */ TypeCode *item_type,
			/* out */ void **value,
			/* out */ long *value_len,
			/* out */ Flags *item_flags);
		#pragma linkage(somTP_NVList_get_item,system)
		typedef somTP_NVList_get_item *somTD_NVList_get_item;
	#else /* __IBMC__ */
		typedef ORBStatus (SOMLINK * somTD_NVList_get_item)(
			NVList SOMSTAR somSelf,
			Environment *ev,
			/* in */ long item_number,
			/* out */ Identifier *item_name,
			/* out */ TypeCode *item_type,
			/* out */ void **value,
			/* out */ long *value_len,
			/* out */ Flags *item_flags);
	#endif /* __IBMC__ */
	#ifndef NVList_MajorVersion
		#define NVList_MajorVersion   2
	#endif /* NVList_MajorVersion */
	#ifndef NVList_MinorVersion
		#define NVList_MinorVersion   1
	#endif /* NVList_MinorVersion */
	typedef struct NVListClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken add_item;
		somMToken remove_item;
		somMToken free;
		somMToken free_memory;
		somMToken get_count;
		somMToken set_item;
		somMToken get_item;
		somMToken get_item_by_name;
	} NVListClassDataStructure;
	typedef struct NVListCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} NVListCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_nvlist_Source) || defined(NVList_Class_Source)
			SOMEXTERN struct NVListClassDataStructure _NVListClassData;
			#ifndef NVListClassData
				#define NVListClassData    _NVListClassData
			#endif /* NVListClassData */
		#else
			SOMEXTERN struct NVListClassDataStructure * SOMLINK resolve_NVListClassData(void);
			#ifndef NVListClassData
				#define NVListClassData    (*(resolve_NVListClassData()))
			#endif /* NVListClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_nvlist_Source) || defined(NVList_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_nvlist_Source || NVList_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_nvlist_Source || NVList_Class_Source */
		struct NVListClassDataStructure SOMDLINK NVListClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_nvlist_Source) || defined(NVList_Class_Source)
			SOMEXTERN struct NVListCClassDataStructure _NVListCClassData;
			#ifndef NVListCClassData
				#define NVListCClassData    _NVListCClassData
			#endif /* NVListCClassData */
		#else
			SOMEXTERN struct NVListCClassDataStructure * SOMLINK resolve_NVListCClassData(void);
			#ifndef NVListCClassData
				#define NVListCClassData    (*(resolve_NVListCClassData()))
			#endif /* NVListCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_nvlist_Source) || defined(NVList_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_nvlist_Source || NVList_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_nvlist_Source || NVList_Class_Source */
		struct NVListCClassDataStructure SOMDLINK NVListCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_nvlist_Source) || defined(NVList_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_nvlist_Source || NVList_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_nvlist_Source || NVList_Class_Source */
	SOMClass SOMSTAR SOMLINK NVListNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_NVList (NVListClassData.classObject)
	#ifndef SOMGD_NVList
		#if (defined(_NVList) || defined(__NVList))
			#undef _NVList
			#undef __NVList
			#define SOMGD_NVList 1
		#else
			#define _NVList _SOMCLASS_NVList
		#endif /* _NVList */
	#endif /* SOMGD_NVList */
	#define NVList_classObj _SOMCLASS_NVList
	#define _SOMMTOKEN_NVList(method) ((somMToken)(NVListClassData.method))
	#ifndef NVListNew
		#define NVListNew() ( _NVList ? \
			(SOMClass_somNew(_NVList)) : \
			( NVListNewClass( \
				NVList_MajorVersion, \
				NVList_MinorVersion), \
			SOMClass_somNew(_NVList))) 
	#endif /* NewNVList */
	#ifndef NVList_add_item
		#define NVList_add_item(somSelf,ev,item_name,item_type,value,value_len,item_flags) \
			SOM_Resolve(somSelf,NVList,add_item)  \
				(somSelf,ev,item_name,item_type,value,value_len,item_flags)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__add_item
				#if defined(_add_item)
					#undef _add_item
					#define SOMGD__add_item
				#else
					#define _add_item NVList_add_item
				#endif
			#endif /* SOMGD__add_item */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* NVList_add_item */
	#ifndef NVList_free
		#define NVList_free(somSelf,ev) \
			SOM_Resolve(somSelf,NVList,free)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__free
				#if defined(_free)
					#undef _free
					#define SOMGD__free
				#else
					#define _free NVList_free
				#endif
			#endif /* SOMGD__free */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* NVList_free */
	#ifndef NVList_free_memory
		#define NVList_free_memory(somSelf,ev) \
			SOM_Resolve(somSelf,NVList,free_memory)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__free_memory
				#if defined(_free_memory)
					#undef _free_memory
					#define SOMGD__free_memory
				#else
					#define _free_memory NVList_free_memory
				#endif
			#endif /* SOMGD__free_memory */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* NVList_free_memory */
	#ifndef NVList_get_count
		#define NVList_get_count(somSelf,ev,count) \
			SOM_Resolve(somSelf,NVList,get_count)  \
				(somSelf,ev,count)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__get_count
				#if defined(_get_count)
					#undef _get_count
					#define SOMGD__get_count
				#else
					#define _get_count NVList_get_count
				#endif
			#endif /* SOMGD__get_count */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* NVList_get_count */
	#ifndef NVList_set_item
		#define NVList_set_item(somSelf,ev,item_number,item_name,item_type,value,value_len,item_flags) \
			SOM_Resolve(somSelf,NVList,set_item)  \
				(somSelf,ev,item_number,item_name,item_type,value,value_len,item_flags)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__set_item
				#if defined(_set_item)
					#undef _set_item
					#define SOMGD__set_item
				#else
					#define _set_item NVList_set_item
				#endif
			#endif /* SOMGD__set_item */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* NVList_set_item */
	#ifndef NVList_get_item
		#define NVList_get_item(somSelf,ev,item_number,item_name,item_type,value,value_len,item_flags) \
			SOM_Resolve(somSelf,NVList,get_item)  \
				(somSelf,ev,item_number,item_name,item_type,value,value_len,item_flags)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__get_item
				#if defined(_get_item)
					#undef _get_item
					#define SOMGD__get_item
				#else
					#define _get_item NVList_get_item
				#endif
			#endif /* SOMGD__get_item */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* NVList_get_item */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define NVList_somInit SOMObject_somInit
#define NVList_somUninit SOMObject_somUninit
#define NVList_somFree SOMObject_somFree
#define NVList_somGetClass SOMObject_somGetClass
#define NVList_somGetClassName SOMObject_somGetClassName
#define NVList_somGetSize SOMObject_somGetSize
#define NVList_somIsA SOMObject_somIsA
#define NVList_somIsInstanceOf SOMObject_somIsInstanceOf
#define NVList_somRespondsTo SOMObject_somRespondsTo
#define NVList_somDispatch SOMObject_somDispatch
#define NVList_somClassDispatch SOMObject_somClassDispatch
#define NVList_somCastObj SOMObject_somCastObj
#define NVList_somResetObj SOMObject_somResetObj
#define NVList_somPrintSelf SOMObject_somPrintSelf
#define NVList_somDumpSelf SOMObject_somDumpSelf
#define NVList_somDumpSelfInt SOMObject_somDumpSelfInt
#define NVList_somDefaultInit SOMObject_somDefaultInit
#define NVList_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define NVList_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define NVList_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define NVList_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define NVList_somDefaultAssign SOMObject_somDefaultAssign
#define NVList_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define NVList_somDefaultVAssign SOMObject_somDefaultVAssign
#define NVList_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define NVList_somDestruct SOMObject_somDestruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_nvlist_Header_h */
