/* generated from xnamingf.idl */
/* internal conditional is SOM_Module_xnamingf_Source */
#ifndef SOM_Module_xnamingf_Header_h
	#define SOM_Module_xnamingf_Header_h 1
	#include <som.h>
	#include <xnaming.h>
	#include <naming.h>
	#include <somobj.h>
	#include <somos.h>
	#include <somdserv.h>
	#include <somdtype.h>
	#include <containd.h>
	#include <snglicls.h>
	#include <somcls.h>
	#include <somdcprx.h>
	#include <somproxy.h>
	#include <somdobj.h>
	#include <unotypes.h>
	#include <xmscssae.h>
	#include <omgidobj.h>
		#ifndef _IDL_FileXNaming_FileENC_defined
			#define _IDL_FileXNaming_FileENC_defined
			typedef SOMObject FileXNaming_FileENC;
		#endif /* _IDL_FileXNaming_FileENC_defined */
		#ifndef _IDL_SEQUENCE_FileXNaming_FileENC_defined
			#define _IDL_SEQUENCE_FileXNaming_FileENC_defined
			SOM_SEQUENCE_TYPEDEF_NAME(FileXNaming_FileENC SOMSTAR ,sequence(FileXNaming_FileENC));
		#endif /* _IDL_SEQUENCE_FileXNaming_FileENC_defined */
		#ifndef _IDL_FileXNaming_FileBindingIterator_defined
			#define _IDL_FileXNaming_FileBindingIterator_defined
			typedef SOMObject FileXNaming_FileBindingIterator;
		#endif /* _IDL_FileXNaming_FileBindingIterator_defined */
		#ifndef _IDL_SEQUENCE_FileXNaming_FileBindingIterator_defined
			#define _IDL_SEQUENCE_FileXNaming_FileBindingIterator_defined
			SOM_SEQUENCE_TYPEDEF_NAME(FileXNaming_FileBindingIterator SOMSTAR ,sequence(FileXNaming_FileBindingIterator));
		#endif /* _IDL_SEQUENCE_FileXNaming_FileBindingIterator_defined */
		#ifndef _IDL_FileXNaming_FPropertyBindingIterator_defined
			#define _IDL_FileXNaming_FPropertyBindingIterator_defined
			typedef SOMObject FileXNaming_FPropertyBindingIterator;
		#endif /* _IDL_FileXNaming_FPropertyBindingIterator_defined */
		#ifndef _IDL_SEQUENCE_FileXNaming_FPropertyBindingIterator_defined
			#define _IDL_SEQUENCE_FileXNaming_FPropertyBindingIterator_defined
			SOM_SEQUENCE_TYPEDEF_NAME(FileXNaming_FPropertyBindingIterator SOMSTAR ,sequence(FileXNaming_FPropertyBindingIterator));
		#endif /* _IDL_SEQUENCE_FileXNaming_FPropertyBindingIterator_defined */
		#ifndef _IDL_FileXNaming_FPropertyIterator_defined
			#define _IDL_FileXNaming_FPropertyIterator_defined
			typedef SOMObject FileXNaming_FPropertyIterator;
		#endif /* _IDL_FileXNaming_FPropertyIterator_defined */
		#ifndef _IDL_SEQUENCE_FileXNaming_FPropertyIterator_defined
			#define _IDL_SEQUENCE_FileXNaming_FPropertyIterator_defined
			SOM_SEQUENCE_TYPEDEF_NAME(FileXNaming_FPropertyIterator SOMSTAR ,sequence(FileXNaming_FPropertyIterator));
		#endif /* _IDL_SEQUENCE_FileXNaming_FPropertyIterator_defined */
		#ifndef _IDL_FileXNaming_FileIndexIterator_defined
			#define _IDL_FileXNaming_FileIndexIterator_defined
			typedef SOMObject FileXNaming_FileIndexIterator;
		#endif /* _IDL_FileXNaming_FileIndexIterator_defined */
		#ifndef _IDL_SEQUENCE_FileXNaming_FileIndexIterator_defined
			#define _IDL_SEQUENCE_FileXNaming_FileIndexIterator_defined
			SOM_SEQUENCE_TYPEDEF_NAME(FileXNaming_FileIndexIterator SOMSTAR ,sequence(FileXNaming_FileIndexIterator));
		#endif /* _IDL_SEQUENCE_FileXNaming_FileIndexIterator_defined */
#ifndef FileXNaming_STARTINGLENGTH
	#define FileXNaming_STARTINGLENGTH   16UL
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_STARTINGLENGTH
			#if defined(STARTINGLENGTH)
				#undef STARTINGLENGTH
				#define SOMGD_STARTINGLENGTH
			#else
				#define STARTINGLENGTH FileXNaming_STARTINGLENGTH
			#endif
		#endif /* SOMGD_STARTINGLENGTH */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* FileXNaming_STARTINGLENGTH */
#ifndef FileXNaming_MAXSTACK
	#define FileXNaming_MAXSTACK   100L
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_MAXSTACK
			#if defined(MAXSTACK)
				#undef MAXSTACK
				#define SOMGD_MAXSTACK
			#else
				#define MAXSTACK FileXNaming_MAXSTACK
			#endif
		#endif /* SOMGD_MAXSTACK */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
#endif /* FileXNaming_MAXSTACK */
		typedef string FileXNaming_FileENC_Constraint;
		#ifndef _IDL_SEQUENCE_string_defined
			#define _IDL_SEQUENCE_string_defined
			SOM_SEQUENCE_TYPEDEF(string);
		#endif /* _IDL_SEQUENCE_string_defined */
		typedef char *FileXNaming_FileENC_Strings;
		#ifndef _IDL_SEQUENCE_FileXNaming_FileENC_Strings_defined
			#define _IDL_SEQUENCE_FileXNaming_FileENC_Strings_defined
			SOM_SEQUENCE_TYPEDEF(FileXNaming_FileENC_Strings);
		#endif /* _IDL_SEQUENCE_FileXNaming_FileENC_Strings_defined */
		#ifndef FileXNaming_FileENC_MajorVersion
			#define FileXNaming_FileENC_MajorVersion   0
		#endif /* FileXNaming_FileENC_MajorVersion */
		#ifndef FileXNaming_FileENC_MinorVersion
			#define FileXNaming_FileENC_MinorVersion   0
		#endif /* FileXNaming_FileENC_MinorVersion */
		typedef struct FileXNaming_FileENCClassDataStructure
		{
			SOMClass SOMSTAR classObject;
		} FileXNaming_FileENCClassDataStructure;
		typedef struct FileXNaming_FileENCCClassDataStructure
		{
			somMethodTabs parentMtab;
			somDToken instanceDataToken;
		} FileXNaming_FileENCCClassDataStructure;
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_xnamingf_Source) || defined(FileXNaming_FileENC_Class_Source)
				SOMEXTERN struct FileXNaming_FileENCClassDataStructure _FileXNaming_FileENCClassData;
				#ifndef FileXNaming_FileENCClassData
					#define FileXNaming_FileENCClassData    _FileXNaming_FileENCClassData
				#endif /* FileXNaming_FileENCClassData */
			#else
				SOMEXTERN struct FileXNaming_FileENCClassDataStructure * SOMLINK resolve_FileXNaming_FileENCClassData(void);
				#ifndef FileXNaming_FileENCClassData
					#define FileXNaming_FileENCClassData    (*(resolve_FileXNaming_FileENCClassData()))
				#endif /* FileXNaming_FileENCClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_xnamingf_Source) || defined(FileXNaming_FileENC_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_xnamingf_Source || FileXNaming_FileENC_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_xnamingf_Source || FileXNaming_FileENC_Class_Source */
			struct FileXNaming_FileENCClassDataStructure SOMDLINK FileXNaming_FileENCClassData;
		#endif /* SOM_RESOLVE_DATA */
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_xnamingf_Source) || defined(FileXNaming_FileENC_Class_Source)
				SOMEXTERN struct FileXNaming_FileENCCClassDataStructure _FileXNaming_FileENCCClassData;
				#ifndef FileXNaming_FileENCCClassData
					#define FileXNaming_FileENCCClassData    _FileXNaming_FileENCCClassData
				#endif /* FileXNaming_FileENCCClassData */
			#else
				SOMEXTERN struct FileXNaming_FileENCCClassDataStructure * SOMLINK resolve_FileXNaming_FileENCCClassData(void);
				#ifndef FileXNaming_FileENCCClassData
					#define FileXNaming_FileENCCClassData    (*(resolve_FileXNaming_FileENCCClassData()))
				#endif /* FileXNaming_FileENCCClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_xnamingf_Source) || defined(FileXNaming_FileENC_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_xnamingf_Source || FileXNaming_FileENC_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_xnamingf_Source || FileXNaming_FileENC_Class_Source */
			struct FileXNaming_FileENCCClassDataStructure SOMDLINK FileXNaming_FileENCCClassData;
		#endif /* SOM_RESOLVE_DATA */
		extern 
		#if defined(SOM_Module_xnamingf_Source) || defined(FileXNaming_FileENC_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_xnamingf_Source || FileXNaming_FileENC_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_xnamingf_Source || FileXNaming_FileENC_Class_Source */
		SOMClass SOMSTAR SOMLINK FileXNaming_FileENCNewClass
				(integer4 somtmajorVersion,integer4 somtminorVersion);
		#define _SOMCLASS_FileXNaming_FileENC (FileXNaming_FileENCClassData.classObject)
		#ifndef SOMGD_FileXNaming_FileENC
			#if (defined(_FileXNaming_FileENC) || defined(__FileXNaming_FileENC))
				#undef _FileXNaming_FileENC
				#undef __FileXNaming_FileENC
				#define SOMGD_FileXNaming_FileENC 1
			#else
				#define _FileXNaming_FileENC _SOMCLASS_FileXNaming_FileENC
			#endif /* _FileXNaming_FileENC */
		#endif /* SOMGD_FileXNaming_FileENC */
		#define FileXNaming_FileENC_classObj _SOMCLASS_FileXNaming_FileENC
		#define _SOMMTOKEN_FileXNaming_FileENC(method) ((somMToken)(FileXNaming_FileENCClassData.method))
		#ifndef FileXNaming_FileENCNew
			#define FileXNaming_FileENCNew() ( _FileXNaming_FileENC ? \
				(SOMClass_somNew(_FileXNaming_FileENC)) : \
				( FileXNaming_FileENCNewClass( \
					FileXNaming_FileENC_MajorVersion, \
					FileXNaming_FileENC_MinorVersion), \
				SOMClass_somNew(_FileXNaming_FileENC))) 
		#endif /* NewFileXNaming_FileENC */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define FileXNaming_FileENC_somInit SOMObject_somInit
#define FileXNaming_FileENC_somUninit SOMObject_somUninit
#define FileXNaming_FileENC_somFree SOMObject_somFree
#define FileXNaming_FileENC_somGetClass SOMObject_somGetClass
#define FileXNaming_FileENC_somGetClassName SOMObject_somGetClassName
#define FileXNaming_FileENC_somGetSize SOMObject_somGetSize
#define FileXNaming_FileENC_somIsA SOMObject_somIsA
#define FileXNaming_FileENC_somIsInstanceOf SOMObject_somIsInstanceOf
#define FileXNaming_FileENC_somRespondsTo SOMObject_somRespondsTo
#define FileXNaming_FileENC_somDispatch SOMObject_somDispatch
#define FileXNaming_FileENC_somClassDispatch SOMObject_somClassDispatch
#define FileXNaming_FileENC_somCastObj SOMObject_somCastObj
#define FileXNaming_FileENC_somResetObj SOMObject_somResetObj
#define FileXNaming_FileENC_somPrintSelf SOMObject_somPrintSelf
#define FileXNaming_FileENC_somDumpSelf SOMObject_somDumpSelf
#define FileXNaming_FileENC_somDumpSelfInt SOMObject_somDumpSelfInt
#define FileXNaming_FileENC_somDefaultInit SOMObject_somDefaultInit
#define FileXNaming_FileENC_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define FileXNaming_FileENC_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define FileXNaming_FileENC_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define FileXNaming_FileENC_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define FileXNaming_FileENC_somDefaultAssign SOMObject_somDefaultAssign
#define FileXNaming_FileENC_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define FileXNaming_FileENC_somDefaultVAssign SOMObject_somDefaultVAssign
#define FileXNaming_FileENC_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define FileXNaming_FileENC_somDestruct SOMObject_somDestruct
#define FileXNaming_FileENC_bind CosNaming_NamingContext_bind
#define FileXNaming_FileENC_rebind CosNaming_NamingContext_rebind
#define FileXNaming_FileENC_bind_context CosNaming_NamingContext_bind_context
#define FileXNaming_FileENC_rebind_context CosNaming_NamingContext_rebind_context
#define FileXNaming_FileENC_resolve CosNaming_NamingContext_resolve
#define FileXNaming_FileENC_unbind CosNaming_NamingContext_unbind
#define FileXNaming_FileENC_new_context CosNaming_NamingContext_new_context
#define FileXNaming_FileENC_bind_new_context CosNaming_NamingContext_bind_new_context
#define FileXNaming_FileENC_destroy CosNaming_NamingContext_destroy
#define FileXNaming_FileENC_list CosNaming_NamingContext_list
#define FileXNaming_FileENC__get_allowed_object_types ExtendedNaming_ExtendedNamingContext__get_allowed_object_types
#define FileXNaming_FileENC__get_allowed_property_types ExtendedNaming_ExtendedNamingContext__get_allowed_property_types
#define FileXNaming_FileENC__get_allowed_property_names ExtendedNaming_ExtendedNamingContext__get_allowed_property_names
#define FileXNaming_FileENC__get_shared_property_types ExtendedNaming_ExtendedNamingContext__get_shared_property_types
#define FileXNaming_FileENC__get_shared_property_names ExtendedNaming_ExtendedNamingContext__get_shared_property_names
#define FileXNaming_FileENC_get_features_supported ExtendedNaming_ExtendedNamingContext_get_features_supported
#define FileXNaming_FileENC_add_property ExtendedNaming_ExtendedNamingContext_add_property
#define FileXNaming_FileENC_add_properties ExtendedNaming_ExtendedNamingContext_add_properties
#define FileXNaming_FileENC_bind_with_properties ExtendedNaming_ExtendedNamingContext_bind_with_properties
#define FileXNaming_FileENC_bind_context_with_properties ExtendedNaming_ExtendedNamingContext_bind_context_with_properties
#define FileXNaming_FileENC_rebind_with_properties ExtendedNaming_ExtendedNamingContext_rebind_with_properties
#define FileXNaming_FileENC_rebind_context_with_properties ExtendedNaming_ExtendedNamingContext_rebind_context_with_properties
#define FileXNaming_FileENC_share_property ExtendedNaming_ExtendedNamingContext_share_property
#define FileXNaming_FileENC_share_properties ExtendedNaming_ExtendedNamingContext_share_properties
#define FileXNaming_FileENC_unshare_property ExtendedNaming_ExtendedNamingContext_unshare_property
#define FileXNaming_FileENC_unshare_properties ExtendedNaming_ExtendedNamingContext_unshare_properties
#define FileXNaming_FileENC_list_properties ExtendedNaming_ExtendedNamingContext_list_properties
#define FileXNaming_FileENC_get_property ExtendedNaming_ExtendedNamingContext_get_property
#define FileXNaming_FileENC_get_properties ExtendedNaming_ExtendedNamingContext_get_properties
#define FileXNaming_FileENC_get_all_properties ExtendedNaming_ExtendedNamingContext_get_all_properties
#define FileXNaming_FileENC_resolve_with_property ExtendedNaming_ExtendedNamingContext_resolve_with_property
#define FileXNaming_FileENC_resolve_with_properties ExtendedNaming_ExtendedNamingContext_resolve_with_properties
#define FileXNaming_FileENC_resolve_with_all_properties ExtendedNaming_ExtendedNamingContext_resolve_with_all_properties
#define FileXNaming_FileENC_remove_property ExtendedNaming_ExtendedNamingContext_remove_property
#define FileXNaming_FileENC_remove_properties ExtendedNaming_ExtendedNamingContext_remove_properties
#define FileXNaming_FileENC_remove_all_properties ExtendedNaming_ExtendedNamingContext_remove_all_properties
#define FileXNaming_FileENC_find_any ExtendedNaming_ExtendedNamingContext_find_any
#define FileXNaming_FileENC_find_all ExtendedNaming_ExtendedNamingContext_find_all
#define FileXNaming_FileENC_find_any_name_binding ExtendedNaming_ExtendedNamingContext_find_any_name_binding
#define FileXNaming_FileENC_add_index ExtendedNaming_ExtendedNamingContext_add_index
#define FileXNaming_FileENC_list_indexes ExtendedNaming_ExtendedNamingContext_list_indexes
#define FileXNaming_FileENC_remove_index ExtendedNaming_ExtendedNamingContext_remove_index
#define FileXNaming_FileENC__get_constant_random_id CosObjectIdentity_IdentifiableObject__get_constant_random_id
#define FileXNaming_FileENC_is_identical CosObjectIdentity_IdentifiableObject_is_identical
#define FileXNaming_FileENC_reinit somOS_ServiceBase_reinit
#define FileXNaming_FileENC_capture somOS_ServiceBase_capture
#define FileXNaming_FileENC_GetInstanceManager somOS_ServiceBase_GetInstanceManager
#define FileXNaming_FileENC_init_for_object_creation somOS_ServiceBase_init_for_object_creation
#define FileXNaming_FileENC_init_for_object_reactivation somOS_ServiceBase_init_for_object_reactivation
#define FileXNaming_FileENC_init_for_object_copy somOS_ServiceBase_init_for_object_copy
#define FileXNaming_FileENC_uninit_for_object_move somOS_ServiceBase_uninit_for_object_move
#define FileXNaming_FileENC_uninit_for_object_passivation somOS_ServiceBase_uninit_for_object_passivation
#define FileXNaming_FileENC_uninit_for_object_destruction somOS_ServiceBase_uninit_for_object_destruction
#endif /* SOM_DONT_USE_INHERITED_MACROS */

		typedef _IDL_SEQUENCE_string FileXNaming_FileBindingIterator_stringSeq;
		#ifndef _IDL_SEQUENCE_FileXNaming_FileBindingIterator_stringSeq_defined
			#define _IDL_SEQUENCE_FileXNaming_FileBindingIterator_stringSeq_defined
			SOM_SEQUENCE_TYPEDEF(FileXNaming_FileBindingIterator_stringSeq);
		#endif /* _IDL_SEQUENCE_FileXNaming_FileBindingIterator_stringSeq_defined */
		#ifdef __IBMC__
			typedef void (somTP_FileXNaming_FileBindingIterator_FileBindingIterator_Init1)(
				FileXNaming_FileBindingIterator SOMSTAR somSelf,
				Environment *ev,
				/* inout */ somToken *ctrl,
				/* in */ FileXNaming_FileBindingIterator_stringSeq *key,
				/* in */ string databaseFile);
			#pragma linkage(somTP_FileXNaming_FileBindingIterator_FileBindingIterator_Init1,system)
			typedef somTP_FileXNaming_FileBindingIterator_FileBindingIterator_Init1 *somTD_FileXNaming_FileBindingIterator_FileBindingIterator_Init1;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_FileXNaming_FileBindingIterator_FileBindingIterator_Init1)(
				FileXNaming_FileBindingIterator SOMSTAR somSelf,
				Environment *ev,
				/* inout */ somToken *ctrl,
				/* in */ FileXNaming_FileBindingIterator_stringSeq *key,
				/* in */ string databaseFile);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_FileXNaming_FileBindingIterator_FileBindingIterator_Init2)(
				FileXNaming_FileBindingIterator SOMSTAR somSelf,
				Environment *ev,
				/* inout */ somToken *ctrl,
				/* in */ CosNaming_BindingList *bl);
			#pragma linkage(somTP_FileXNaming_FileBindingIterator_FileBindingIterator_Init2,system)
			typedef somTP_FileXNaming_FileBindingIterator_FileBindingIterator_Init2 *somTD_FileXNaming_FileBindingIterator_FileBindingIterator_Init2;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_FileXNaming_FileBindingIterator_FileBindingIterator_Init2)(
				FileXNaming_FileBindingIterator SOMSTAR somSelf,
				Environment *ev,
				/* inout */ somToken *ctrl,
				/* in */ CosNaming_BindingList *bl);
		#endif /* __IBMC__ */
		#ifndef FileXNaming_FileBindingIterator_MajorVersion
			#define FileXNaming_FileBindingIterator_MajorVersion   0
		#endif /* FileXNaming_FileBindingIterator_MajorVersion */
		#ifndef FileXNaming_FileBindingIterator_MinorVersion
			#define FileXNaming_FileBindingIterator_MinorVersion   0
		#endif /* FileXNaming_FileBindingIterator_MinorVersion */
		typedef struct FileXNaming_FileBindingIteratorClassDataStructure
		{
			SOMClass SOMSTAR classObject;
			somMToken FileBindingIterator_Init1;
			somMToken FileBindingIterator_Init2;
		} FileXNaming_FileBindingIteratorClassDataStructure;
		typedef struct FileXNaming_FileBindingIteratorCClassDataStructure
		{
			somMethodTabs parentMtab;
			somDToken instanceDataToken;
		} FileXNaming_FileBindingIteratorCClassDataStructure;
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_xnamingf_Source) || defined(FileXNaming_FileBindingIterator_Class_Source)
				SOMEXTERN struct FileXNaming_FileBindingIteratorClassDataStructure _FileXNaming_FileBindingIteratorClassData;
				#ifndef FileXNaming_FileBindingIteratorClassData
					#define FileXNaming_FileBindingIteratorClassData    _FileXNaming_FileBindingIteratorClassData
				#endif /* FileXNaming_FileBindingIteratorClassData */
			#else
				SOMEXTERN struct FileXNaming_FileBindingIteratorClassDataStructure * SOMLINK resolve_FileXNaming_FileBindingIteratorClassData(void);
				#ifndef FileXNaming_FileBindingIteratorClassData
					#define FileXNaming_FileBindingIteratorClassData    (*(resolve_FileXNaming_FileBindingIteratorClassData()))
				#endif /* FileXNaming_FileBindingIteratorClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_xnamingf_Source) || defined(FileXNaming_FileBindingIterator_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_xnamingf_Source || FileXNaming_FileBindingIterator_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_xnamingf_Source || FileXNaming_FileBindingIterator_Class_Source */
			struct FileXNaming_FileBindingIteratorClassDataStructure SOMDLINK FileXNaming_FileBindingIteratorClassData;
		#endif /* SOM_RESOLVE_DATA */
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_xnamingf_Source) || defined(FileXNaming_FileBindingIterator_Class_Source)
				SOMEXTERN struct FileXNaming_FileBindingIteratorCClassDataStructure _FileXNaming_FileBindingIteratorCClassData;
				#ifndef FileXNaming_FileBindingIteratorCClassData
					#define FileXNaming_FileBindingIteratorCClassData    _FileXNaming_FileBindingIteratorCClassData
				#endif /* FileXNaming_FileBindingIteratorCClassData */
			#else
				SOMEXTERN struct FileXNaming_FileBindingIteratorCClassDataStructure * SOMLINK resolve_FileXNaming_FileBindingIteratorCClassData(void);
				#ifndef FileXNaming_FileBindingIteratorCClassData
					#define FileXNaming_FileBindingIteratorCClassData    (*(resolve_FileXNaming_FileBindingIteratorCClassData()))
				#endif /* FileXNaming_FileBindingIteratorCClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_xnamingf_Source) || defined(FileXNaming_FileBindingIterator_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_xnamingf_Source || FileXNaming_FileBindingIterator_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_xnamingf_Source || FileXNaming_FileBindingIterator_Class_Source */
			struct FileXNaming_FileBindingIteratorCClassDataStructure SOMDLINK FileXNaming_FileBindingIteratorCClassData;
		#endif /* SOM_RESOLVE_DATA */
		extern 
		#if defined(SOM_Module_xnamingf_Source) || defined(FileXNaming_FileBindingIterator_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_xnamingf_Source || FileXNaming_FileBindingIterator_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_xnamingf_Source || FileXNaming_FileBindingIterator_Class_Source */
		SOMClass SOMSTAR SOMLINK FileXNaming_FileBindingIteratorNewClass
				(integer4 somtmajorVersion,integer4 somtminorVersion);
		#define _SOMCLASS_FileXNaming_FileBindingIterator (FileXNaming_FileBindingIteratorClassData.classObject)
		#ifndef SOMGD_FileXNaming_FileBindingIterator
			#if (defined(_FileXNaming_FileBindingIterator) || defined(__FileXNaming_FileBindingIterator))
				#undef _FileXNaming_FileBindingIterator
				#undef __FileXNaming_FileBindingIterator
				#define SOMGD_FileXNaming_FileBindingIterator 1
			#else
				#define _FileXNaming_FileBindingIterator _SOMCLASS_FileXNaming_FileBindingIterator
			#endif /* _FileXNaming_FileBindingIterator */
		#endif /* SOMGD_FileXNaming_FileBindingIterator */
		#define FileXNaming_FileBindingIterator_classObj _SOMCLASS_FileXNaming_FileBindingIterator
		#define _SOMMTOKEN_FileXNaming_FileBindingIterator(method) ((somMToken)(FileXNaming_FileBindingIteratorClassData.method))
		#ifndef FileXNaming_FileBindingIteratorNew
			#define FileXNaming_FileBindingIteratorNew() ( _FileXNaming_FileBindingIterator ? \
				(SOMClass_somNew(_FileXNaming_FileBindingIterator)) : \
				( FileXNaming_FileBindingIteratorNewClass( \
					FileXNaming_FileBindingIterator_MajorVersion, \
					FileXNaming_FileBindingIterator_MinorVersion), \
				SOMClass_somNew(_FileXNaming_FileBindingIterator))) 
		#endif /* NewFileXNaming_FileBindingIterator */
		#ifndef FileXNaming_FileBindingIterator_FileBindingIterator_Init1
			#define FileXNaming_FileBindingIterator_FileBindingIterator_Init1(somSelf,ev,ctrl,key,databaseFile) \
				SOM_Resolve(somSelf,FileXNaming_FileBindingIterator,FileBindingIterator_Init1)  \
					(somSelf,ev,ctrl,key,databaseFile)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__FileBindingIterator_Init1
					#if defined(_FileBindingIterator_Init1)
						#undef _FileBindingIterator_Init1
						#define SOMGD__FileBindingIterator_Init1
					#else
						#define _FileBindingIterator_Init1 FileXNaming_FileBindingIterator_FileBindingIterator_Init1
					#endif
				#endif /* SOMGD__FileBindingIterator_Init1 */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* FileXNaming_FileBindingIterator_FileBindingIterator_Init1 */
		#ifndef FileXNaming_FileBindingIterator_FileBindingIterator_Init2
			#define FileXNaming_FileBindingIterator_FileBindingIterator_Init2(somSelf,ev,ctrl,bl) \
				SOM_Resolve(somSelf,FileXNaming_FileBindingIterator,FileBindingIterator_Init2)  \
					(somSelf,ev,ctrl,bl)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__FileBindingIterator_Init2
					#if defined(_FileBindingIterator_Init2)
						#undef _FileBindingIterator_Init2
						#define SOMGD__FileBindingIterator_Init2
					#else
						#define _FileBindingIterator_Init2 FileXNaming_FileBindingIterator_FileBindingIterator_Init2
					#endif
				#endif /* SOMGD__FileBindingIterator_Init2 */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* FileXNaming_FileBindingIterator_FileBindingIterator_Init2 */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define FileXNaming_FileBindingIterator_somInit SOMObject_somInit
#define FileXNaming_FileBindingIterator_somUninit SOMObject_somUninit
#define FileXNaming_FileBindingIterator_somFree SOMObject_somFree
#define FileXNaming_FileBindingIterator_somGetClass SOMObject_somGetClass
#define FileXNaming_FileBindingIterator_somGetClassName SOMObject_somGetClassName
#define FileXNaming_FileBindingIterator_somGetSize SOMObject_somGetSize
#define FileXNaming_FileBindingIterator_somIsA SOMObject_somIsA
#define FileXNaming_FileBindingIterator_somIsInstanceOf SOMObject_somIsInstanceOf
#define FileXNaming_FileBindingIterator_somRespondsTo SOMObject_somRespondsTo
#define FileXNaming_FileBindingIterator_somDispatch SOMObject_somDispatch
#define FileXNaming_FileBindingIterator_somClassDispatch SOMObject_somClassDispatch
#define FileXNaming_FileBindingIterator_somCastObj SOMObject_somCastObj
#define FileXNaming_FileBindingIterator_somResetObj SOMObject_somResetObj
#define FileXNaming_FileBindingIterator_somPrintSelf SOMObject_somPrintSelf
#define FileXNaming_FileBindingIterator_somDumpSelf SOMObject_somDumpSelf
#define FileXNaming_FileBindingIterator_somDumpSelfInt SOMObject_somDumpSelfInt
#define FileXNaming_FileBindingIterator_somDefaultInit SOMObject_somDefaultInit
#define FileXNaming_FileBindingIterator_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define FileXNaming_FileBindingIterator_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define FileXNaming_FileBindingIterator_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define FileXNaming_FileBindingIterator_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define FileXNaming_FileBindingIterator_somDefaultAssign SOMObject_somDefaultAssign
#define FileXNaming_FileBindingIterator_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define FileXNaming_FileBindingIterator_somDefaultVAssign SOMObject_somDefaultVAssign
#define FileXNaming_FileBindingIterator_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define FileXNaming_FileBindingIterator_somDestruct SOMObject_somDestruct
#define FileXNaming_FileBindingIterator_next_one CosNaming_BindingIterator_next_one
#define FileXNaming_FileBindingIterator_next_n CosNaming_BindingIterator_next_n
#define FileXNaming_FileBindingIterator_destroy CosNaming_BindingIterator_destroy
#endif /* SOM_DONT_USE_INHERITED_MACROS */

		#ifndef FileXNaming_FPropertyBindingIterator_MajorVersion
			#define FileXNaming_FPropertyBindingIterator_MajorVersion   0
		#endif /* FileXNaming_FPropertyBindingIterator_MajorVersion */
		#ifndef FileXNaming_FPropertyBindingIterator_MinorVersion
			#define FileXNaming_FPropertyBindingIterator_MinorVersion   0
		#endif /* FileXNaming_FPropertyBindingIterator_MinorVersion */
		typedef struct FileXNaming_FPropertyBindingIteratorClassDataStructure
		{
			SOMClass SOMSTAR classObject;
		} FileXNaming_FPropertyBindingIteratorClassDataStructure;
		typedef struct FileXNaming_FPropertyBindingIteratorCClassDataStructure
		{
			somMethodTabs parentMtab;
			somDToken instanceDataToken;
		} FileXNaming_FPropertyBindingIteratorCClassDataStructure;
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_xnamingf_Source) || defined(FileXNaming_FPropertyBindingIterator_Class_Source)
				SOMEXTERN struct FileXNaming_FPropertyBindingIteratorClassDataStructure _FileXNaming_FPropertyBindingIteratorClassData;
				#ifndef FileXNaming_FPropertyBindingIteratorClassData
					#define FileXNaming_FPropertyBindingIteratorClassData    _FileXNaming_FPropertyBindingIteratorClassData
				#endif /* FileXNaming_FPropertyBindingIteratorClassData */
			#else
				SOMEXTERN struct FileXNaming_FPropertyBindingIteratorClassDataStructure * SOMLINK resolve_FileXNaming_FPropertyBindingIteratorClassData(void);
				#ifndef FileXNaming_FPropertyBindingIteratorClassData
					#define FileXNaming_FPropertyBindingIteratorClassData    (*(resolve_FileXNaming_FPropertyBindingIteratorClassData()))
				#endif /* FileXNaming_FPropertyBindingIteratorClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_xnamingf_Source) || defined(FileXNaming_FPropertyBindingIterator_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_xnamingf_Source || FileXNaming_FPropertyBindingIterator_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_xnamingf_Source || FileXNaming_FPropertyBindingIterator_Class_Source */
			struct FileXNaming_FPropertyBindingIteratorClassDataStructure SOMDLINK FileXNaming_FPropertyBindingIteratorClassData;
		#endif /* SOM_RESOLVE_DATA */
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_xnamingf_Source) || defined(FileXNaming_FPropertyBindingIterator_Class_Source)
				SOMEXTERN struct FileXNaming_FPropertyBindingIteratorCClassDataStructure _FileXNaming_FPropertyBindingIteratorCClassData;
				#ifndef FileXNaming_FPropertyBindingIteratorCClassData
					#define FileXNaming_FPropertyBindingIteratorCClassData    _FileXNaming_FPropertyBindingIteratorCClassData
				#endif /* FileXNaming_FPropertyBindingIteratorCClassData */
			#else
				SOMEXTERN struct FileXNaming_FPropertyBindingIteratorCClassDataStructure * SOMLINK resolve_FileXNaming_FPropertyBindingIteratorCClassData(void);
				#ifndef FileXNaming_FPropertyBindingIteratorCClassData
					#define FileXNaming_FPropertyBindingIteratorCClassData    (*(resolve_FileXNaming_FPropertyBindingIteratorCClassData()))
				#endif /* FileXNaming_FPropertyBindingIteratorCClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_xnamingf_Source) || defined(FileXNaming_FPropertyBindingIterator_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_xnamingf_Source || FileXNaming_FPropertyBindingIterator_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_xnamingf_Source || FileXNaming_FPropertyBindingIterator_Class_Source */
			struct FileXNaming_FPropertyBindingIteratorCClassDataStructure SOMDLINK FileXNaming_FPropertyBindingIteratorCClassData;
		#endif /* SOM_RESOLVE_DATA */
		extern 
		#if defined(SOM_Module_xnamingf_Source) || defined(FileXNaming_FPropertyBindingIterator_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_xnamingf_Source || FileXNaming_FPropertyBindingIterator_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_xnamingf_Source || FileXNaming_FPropertyBindingIterator_Class_Source */
		SOMClass SOMSTAR SOMLINK FileXNaming_FPropertyBindingIteratorNewClass
				(integer4 somtmajorVersion,integer4 somtminorVersion);
		#define _SOMCLASS_FileXNaming_FPropertyBindingIterator (FileXNaming_FPropertyBindingIteratorClassData.classObject)
		#ifndef SOMGD_FileXNaming_FPropertyBindingIterator
			#if (defined(_FileXNaming_FPropertyBindingIterator) || defined(__FileXNaming_FPropertyBindingIterator))
				#undef _FileXNaming_FPropertyBindingIterator
				#undef __FileXNaming_FPropertyBindingIterator
				#define SOMGD_FileXNaming_FPropertyBindingIterator 1
			#else
				#define _FileXNaming_FPropertyBindingIterator _SOMCLASS_FileXNaming_FPropertyBindingIterator
			#endif /* _FileXNaming_FPropertyBindingIterator */
		#endif /* SOMGD_FileXNaming_FPropertyBindingIterator */
		#define FileXNaming_FPropertyBindingIterator_classObj _SOMCLASS_FileXNaming_FPropertyBindingIterator
		#define _SOMMTOKEN_FileXNaming_FPropertyBindingIterator(method) ((somMToken)(FileXNaming_FPropertyBindingIteratorClassData.method))
		#ifndef FileXNaming_FPropertyBindingIteratorNew
			#define FileXNaming_FPropertyBindingIteratorNew() ( _FileXNaming_FPropertyBindingIterator ? \
				(SOMClass_somNew(_FileXNaming_FPropertyBindingIterator)) : \
				( FileXNaming_FPropertyBindingIteratorNewClass( \
					FileXNaming_FPropertyBindingIterator_MajorVersion, \
					FileXNaming_FPropertyBindingIterator_MinorVersion), \
				SOMClass_somNew(_FileXNaming_FPropertyBindingIterator))) 
		#endif /* NewFileXNaming_FPropertyBindingIterator */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define FileXNaming_FPropertyBindingIterator_somInit SOMObject_somInit
#define FileXNaming_FPropertyBindingIterator_somUninit SOMObject_somUninit
#define FileXNaming_FPropertyBindingIterator_somFree SOMObject_somFree
#define FileXNaming_FPropertyBindingIterator_somGetClass SOMObject_somGetClass
#define FileXNaming_FPropertyBindingIterator_somGetClassName SOMObject_somGetClassName
#define FileXNaming_FPropertyBindingIterator_somGetSize SOMObject_somGetSize
#define FileXNaming_FPropertyBindingIterator_somIsA SOMObject_somIsA
#define FileXNaming_FPropertyBindingIterator_somIsInstanceOf SOMObject_somIsInstanceOf
#define FileXNaming_FPropertyBindingIterator_somRespondsTo SOMObject_somRespondsTo
#define FileXNaming_FPropertyBindingIterator_somDispatch SOMObject_somDispatch
#define FileXNaming_FPropertyBindingIterator_somClassDispatch SOMObject_somClassDispatch
#define FileXNaming_FPropertyBindingIterator_somCastObj SOMObject_somCastObj
#define FileXNaming_FPropertyBindingIterator_somResetObj SOMObject_somResetObj
#define FileXNaming_FPropertyBindingIterator_somPrintSelf SOMObject_somPrintSelf
#define FileXNaming_FPropertyBindingIterator_somDumpSelf SOMObject_somDumpSelf
#define FileXNaming_FPropertyBindingIterator_somDumpSelfInt SOMObject_somDumpSelfInt
#define FileXNaming_FPropertyBindingIterator_somDefaultInit SOMObject_somDefaultInit
#define FileXNaming_FPropertyBindingIterator_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define FileXNaming_FPropertyBindingIterator_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define FileXNaming_FPropertyBindingIterator_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define FileXNaming_FPropertyBindingIterator_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define FileXNaming_FPropertyBindingIterator_somDefaultAssign SOMObject_somDefaultAssign
#define FileXNaming_FPropertyBindingIterator_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define FileXNaming_FPropertyBindingIterator_somDefaultVAssign SOMObject_somDefaultVAssign
#define FileXNaming_FPropertyBindingIterator_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define FileXNaming_FPropertyBindingIterator_somDestruct SOMObject_somDestruct
#define FileXNaming_FPropertyBindingIterator_next_one ExtendedNaming_PropertyBindingIterator_next_one
#define FileXNaming_FPropertyBindingIterator_next_n ExtendedNaming_PropertyBindingIterator_next_n
#define FileXNaming_FPropertyBindingIterator_destroy ExtendedNaming_PropertyBindingIterator_destroy
#endif /* SOM_DONT_USE_INHERITED_MACROS */

		#ifndef FileXNaming_FPropertyIterator_MajorVersion
			#define FileXNaming_FPropertyIterator_MajorVersion   0
		#endif /* FileXNaming_FPropertyIterator_MajorVersion */
		#ifndef FileXNaming_FPropertyIterator_MinorVersion
			#define FileXNaming_FPropertyIterator_MinorVersion   0
		#endif /* FileXNaming_FPropertyIterator_MinorVersion */
		typedef struct FileXNaming_FPropertyIteratorClassDataStructure
		{
			SOMClass SOMSTAR classObject;
		} FileXNaming_FPropertyIteratorClassDataStructure;
		typedef struct FileXNaming_FPropertyIteratorCClassDataStructure
		{
			somMethodTabs parentMtab;
			somDToken instanceDataToken;
		} FileXNaming_FPropertyIteratorCClassDataStructure;
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_xnamingf_Source) || defined(FileXNaming_FPropertyIterator_Class_Source)
				SOMEXTERN struct FileXNaming_FPropertyIteratorClassDataStructure _FileXNaming_FPropertyIteratorClassData;
				#ifndef FileXNaming_FPropertyIteratorClassData
					#define FileXNaming_FPropertyIteratorClassData    _FileXNaming_FPropertyIteratorClassData
				#endif /* FileXNaming_FPropertyIteratorClassData */
			#else
				SOMEXTERN struct FileXNaming_FPropertyIteratorClassDataStructure * SOMLINK resolve_FileXNaming_FPropertyIteratorClassData(void);
				#ifndef FileXNaming_FPropertyIteratorClassData
					#define FileXNaming_FPropertyIteratorClassData    (*(resolve_FileXNaming_FPropertyIteratorClassData()))
				#endif /* FileXNaming_FPropertyIteratorClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_xnamingf_Source) || defined(FileXNaming_FPropertyIterator_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_xnamingf_Source || FileXNaming_FPropertyIterator_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_xnamingf_Source || FileXNaming_FPropertyIterator_Class_Source */
			struct FileXNaming_FPropertyIteratorClassDataStructure SOMDLINK FileXNaming_FPropertyIteratorClassData;
		#endif /* SOM_RESOLVE_DATA */
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_xnamingf_Source) || defined(FileXNaming_FPropertyIterator_Class_Source)
				SOMEXTERN struct FileXNaming_FPropertyIteratorCClassDataStructure _FileXNaming_FPropertyIteratorCClassData;
				#ifndef FileXNaming_FPropertyIteratorCClassData
					#define FileXNaming_FPropertyIteratorCClassData    _FileXNaming_FPropertyIteratorCClassData
				#endif /* FileXNaming_FPropertyIteratorCClassData */
			#else
				SOMEXTERN struct FileXNaming_FPropertyIteratorCClassDataStructure * SOMLINK resolve_FileXNaming_FPropertyIteratorCClassData(void);
				#ifndef FileXNaming_FPropertyIteratorCClassData
					#define FileXNaming_FPropertyIteratorCClassData    (*(resolve_FileXNaming_FPropertyIteratorCClassData()))
				#endif /* FileXNaming_FPropertyIteratorCClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_xnamingf_Source) || defined(FileXNaming_FPropertyIterator_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_xnamingf_Source || FileXNaming_FPropertyIterator_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_xnamingf_Source || FileXNaming_FPropertyIterator_Class_Source */
			struct FileXNaming_FPropertyIteratorCClassDataStructure SOMDLINK FileXNaming_FPropertyIteratorCClassData;
		#endif /* SOM_RESOLVE_DATA */
		extern 
		#if defined(SOM_Module_xnamingf_Source) || defined(FileXNaming_FPropertyIterator_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_xnamingf_Source || FileXNaming_FPropertyIterator_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_xnamingf_Source || FileXNaming_FPropertyIterator_Class_Source */
		SOMClass SOMSTAR SOMLINK FileXNaming_FPropertyIteratorNewClass
				(integer4 somtmajorVersion,integer4 somtminorVersion);
		#define _SOMCLASS_FileXNaming_FPropertyIterator (FileXNaming_FPropertyIteratorClassData.classObject)
		#ifndef SOMGD_FileXNaming_FPropertyIterator
			#if (defined(_FileXNaming_FPropertyIterator) || defined(__FileXNaming_FPropertyIterator))
				#undef _FileXNaming_FPropertyIterator
				#undef __FileXNaming_FPropertyIterator
				#define SOMGD_FileXNaming_FPropertyIterator 1
			#else
				#define _FileXNaming_FPropertyIterator _SOMCLASS_FileXNaming_FPropertyIterator
			#endif /* _FileXNaming_FPropertyIterator */
		#endif /* SOMGD_FileXNaming_FPropertyIterator */
		#define FileXNaming_FPropertyIterator_classObj _SOMCLASS_FileXNaming_FPropertyIterator
		#define _SOMMTOKEN_FileXNaming_FPropertyIterator(method) ((somMToken)(FileXNaming_FPropertyIteratorClassData.method))
		#ifndef FileXNaming_FPropertyIteratorNew
			#define FileXNaming_FPropertyIteratorNew() ( _FileXNaming_FPropertyIterator ? \
				(SOMClass_somNew(_FileXNaming_FPropertyIterator)) : \
				( FileXNaming_FPropertyIteratorNewClass( \
					FileXNaming_FPropertyIterator_MajorVersion, \
					FileXNaming_FPropertyIterator_MinorVersion), \
				SOMClass_somNew(_FileXNaming_FPropertyIterator))) 
		#endif /* NewFileXNaming_FPropertyIterator */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define FileXNaming_FPropertyIterator_somInit SOMObject_somInit
#define FileXNaming_FPropertyIterator_somUninit SOMObject_somUninit
#define FileXNaming_FPropertyIterator_somFree SOMObject_somFree
#define FileXNaming_FPropertyIterator_somGetClass SOMObject_somGetClass
#define FileXNaming_FPropertyIterator_somGetClassName SOMObject_somGetClassName
#define FileXNaming_FPropertyIterator_somGetSize SOMObject_somGetSize
#define FileXNaming_FPropertyIterator_somIsA SOMObject_somIsA
#define FileXNaming_FPropertyIterator_somIsInstanceOf SOMObject_somIsInstanceOf
#define FileXNaming_FPropertyIterator_somRespondsTo SOMObject_somRespondsTo
#define FileXNaming_FPropertyIterator_somDispatch SOMObject_somDispatch
#define FileXNaming_FPropertyIterator_somClassDispatch SOMObject_somClassDispatch
#define FileXNaming_FPropertyIterator_somCastObj SOMObject_somCastObj
#define FileXNaming_FPropertyIterator_somResetObj SOMObject_somResetObj
#define FileXNaming_FPropertyIterator_somPrintSelf SOMObject_somPrintSelf
#define FileXNaming_FPropertyIterator_somDumpSelf SOMObject_somDumpSelf
#define FileXNaming_FPropertyIterator_somDumpSelfInt SOMObject_somDumpSelfInt
#define FileXNaming_FPropertyIterator_somDefaultInit SOMObject_somDefaultInit
#define FileXNaming_FPropertyIterator_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define FileXNaming_FPropertyIterator_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define FileXNaming_FPropertyIterator_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define FileXNaming_FPropertyIterator_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define FileXNaming_FPropertyIterator_somDefaultAssign SOMObject_somDefaultAssign
#define FileXNaming_FPropertyIterator_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define FileXNaming_FPropertyIterator_somDefaultVAssign SOMObject_somDefaultVAssign
#define FileXNaming_FPropertyIterator_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define FileXNaming_FPropertyIterator_somDestruct SOMObject_somDestruct
#define FileXNaming_FPropertyIterator_next_one ExtendedNaming_PropertyIterator_next_one
#define FileXNaming_FPropertyIterator_next_n ExtendedNaming_PropertyIterator_next_n
#define FileXNaming_FPropertyIterator_destroy ExtendedNaming_PropertyIterator_destroy
#endif /* SOM_DONT_USE_INHERITED_MACROS */

		#ifdef __IBMC__
			typedef void (somTP_FileXNaming_FileIndexIterator_FileIndexIterator_Init)(
				FileXNaming_FileIndexIterator SOMSTAR somSelf,
				Environment *ev,
				/* inout */ somToken *ctrl,
				/* in */ _IDL_SEQUENCE_octet *key,
				/* in */ string databaseFile);
			#pragma linkage(somTP_FileXNaming_FileIndexIterator_FileIndexIterator_Init,system)
			typedef somTP_FileXNaming_FileIndexIterator_FileIndexIterator_Init *somTD_FileXNaming_FileIndexIterator_FileIndexIterator_Init;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_FileXNaming_FileIndexIterator_FileIndexIterator_Init)(
				FileXNaming_FileIndexIterator SOMSTAR somSelf,
				Environment *ev,
				/* inout */ somToken *ctrl,
				/* in */ _IDL_SEQUENCE_octet *key,
				/* in */ string databaseFile);
		#endif /* __IBMC__ */
		#ifndef FileXNaming_FileIndexIterator_MajorVersion
			#define FileXNaming_FileIndexIterator_MajorVersion   0
		#endif /* FileXNaming_FileIndexIterator_MajorVersion */
		#ifndef FileXNaming_FileIndexIterator_MinorVersion
			#define FileXNaming_FileIndexIterator_MinorVersion   0
		#endif /* FileXNaming_FileIndexIterator_MinorVersion */
		typedef struct FileXNaming_FileIndexIteratorClassDataStructure
		{
			SOMClass SOMSTAR classObject;
			somMToken FileIndexIterator_Init;
		} FileXNaming_FileIndexIteratorClassDataStructure;
		typedef struct FileXNaming_FileIndexIteratorCClassDataStructure
		{
			somMethodTabs parentMtab;
			somDToken instanceDataToken;
		} FileXNaming_FileIndexIteratorCClassDataStructure;
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_xnamingf_Source) || defined(FileXNaming_FileIndexIterator_Class_Source)
				SOMEXTERN struct FileXNaming_FileIndexIteratorClassDataStructure _FileXNaming_FileIndexIteratorClassData;
				#ifndef FileXNaming_FileIndexIteratorClassData
					#define FileXNaming_FileIndexIteratorClassData    _FileXNaming_FileIndexIteratorClassData
				#endif /* FileXNaming_FileIndexIteratorClassData */
			#else
				SOMEXTERN struct FileXNaming_FileIndexIteratorClassDataStructure * SOMLINK resolve_FileXNaming_FileIndexIteratorClassData(void);
				#ifndef FileXNaming_FileIndexIteratorClassData
					#define FileXNaming_FileIndexIteratorClassData    (*(resolve_FileXNaming_FileIndexIteratorClassData()))
				#endif /* FileXNaming_FileIndexIteratorClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_xnamingf_Source) || defined(FileXNaming_FileIndexIterator_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_xnamingf_Source || FileXNaming_FileIndexIterator_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_xnamingf_Source || FileXNaming_FileIndexIterator_Class_Source */
			struct FileXNaming_FileIndexIteratorClassDataStructure SOMDLINK FileXNaming_FileIndexIteratorClassData;
		#endif /* SOM_RESOLVE_DATA */
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_xnamingf_Source) || defined(FileXNaming_FileIndexIterator_Class_Source)
				SOMEXTERN struct FileXNaming_FileIndexIteratorCClassDataStructure _FileXNaming_FileIndexIteratorCClassData;
				#ifndef FileXNaming_FileIndexIteratorCClassData
					#define FileXNaming_FileIndexIteratorCClassData    _FileXNaming_FileIndexIteratorCClassData
				#endif /* FileXNaming_FileIndexIteratorCClassData */
			#else
				SOMEXTERN struct FileXNaming_FileIndexIteratorCClassDataStructure * SOMLINK resolve_FileXNaming_FileIndexIteratorCClassData(void);
				#ifndef FileXNaming_FileIndexIteratorCClassData
					#define FileXNaming_FileIndexIteratorCClassData    (*(resolve_FileXNaming_FileIndexIteratorCClassData()))
				#endif /* FileXNaming_FileIndexIteratorCClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_xnamingf_Source) || defined(FileXNaming_FileIndexIterator_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_xnamingf_Source || FileXNaming_FileIndexIterator_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_xnamingf_Source || FileXNaming_FileIndexIterator_Class_Source */
			struct FileXNaming_FileIndexIteratorCClassDataStructure SOMDLINK FileXNaming_FileIndexIteratorCClassData;
		#endif /* SOM_RESOLVE_DATA */
		extern 
		#if defined(SOM_Module_xnamingf_Source) || defined(FileXNaming_FileIndexIterator_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_xnamingf_Source || FileXNaming_FileIndexIterator_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_xnamingf_Source || FileXNaming_FileIndexIterator_Class_Source */
		SOMClass SOMSTAR SOMLINK FileXNaming_FileIndexIteratorNewClass
				(integer4 somtmajorVersion,integer4 somtminorVersion);
		#define _SOMCLASS_FileXNaming_FileIndexIterator (FileXNaming_FileIndexIteratorClassData.classObject)
		#ifndef SOMGD_FileXNaming_FileIndexIterator
			#if (defined(_FileXNaming_FileIndexIterator) || defined(__FileXNaming_FileIndexIterator))
				#undef _FileXNaming_FileIndexIterator
				#undef __FileXNaming_FileIndexIterator
				#define SOMGD_FileXNaming_FileIndexIterator 1
			#else
				#define _FileXNaming_FileIndexIterator _SOMCLASS_FileXNaming_FileIndexIterator
			#endif /* _FileXNaming_FileIndexIterator */
		#endif /* SOMGD_FileXNaming_FileIndexIterator */
		#define FileXNaming_FileIndexIterator_classObj _SOMCLASS_FileXNaming_FileIndexIterator
		#define _SOMMTOKEN_FileXNaming_FileIndexIterator(method) ((somMToken)(FileXNaming_FileIndexIteratorClassData.method))
		#ifndef FileXNaming_FileIndexIteratorNew
			#define FileXNaming_FileIndexIteratorNew() ( _FileXNaming_FileIndexIterator ? \
				(SOMClass_somNew(_FileXNaming_FileIndexIterator)) : \
				( FileXNaming_FileIndexIteratorNewClass( \
					FileXNaming_FileIndexIterator_MajorVersion, \
					FileXNaming_FileIndexIterator_MinorVersion), \
				SOMClass_somNew(_FileXNaming_FileIndexIterator))) 
		#endif /* NewFileXNaming_FileIndexIterator */
		#ifndef FileXNaming_FileIndexIterator_FileIndexIterator_Init
			#define FileXNaming_FileIndexIterator_FileIndexIterator_Init(somSelf,ev,ctrl,key,databaseFile) \
				SOM_Resolve(somSelf,FileXNaming_FileIndexIterator,FileIndexIterator_Init)  \
					(somSelf,ev,ctrl,key,databaseFile)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__FileIndexIterator_Init
					#if defined(_FileIndexIterator_Init)
						#undef _FileIndexIterator_Init
						#define SOMGD__FileIndexIterator_Init
					#else
						#define _FileIndexIterator_Init FileXNaming_FileIndexIterator_FileIndexIterator_Init
					#endif
				#endif /* SOMGD__FileIndexIterator_Init */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* FileXNaming_FileIndexIterator_FileIndexIterator_Init */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define FileXNaming_FileIndexIterator_somInit SOMObject_somInit
#define FileXNaming_FileIndexIterator_somUninit SOMObject_somUninit
#define FileXNaming_FileIndexIterator_somFree SOMObject_somFree
#define FileXNaming_FileIndexIterator_somGetClass SOMObject_somGetClass
#define FileXNaming_FileIndexIterator_somGetClassName SOMObject_somGetClassName
#define FileXNaming_FileIndexIterator_somGetSize SOMObject_somGetSize
#define FileXNaming_FileIndexIterator_somIsA SOMObject_somIsA
#define FileXNaming_FileIndexIterator_somIsInstanceOf SOMObject_somIsInstanceOf
#define FileXNaming_FileIndexIterator_somRespondsTo SOMObject_somRespondsTo
#define FileXNaming_FileIndexIterator_somDispatch SOMObject_somDispatch
#define FileXNaming_FileIndexIterator_somClassDispatch SOMObject_somClassDispatch
#define FileXNaming_FileIndexIterator_somCastObj SOMObject_somCastObj
#define FileXNaming_FileIndexIterator_somResetObj SOMObject_somResetObj
#define FileXNaming_FileIndexIterator_somPrintSelf SOMObject_somPrintSelf
#define FileXNaming_FileIndexIterator_somDumpSelf SOMObject_somDumpSelf
#define FileXNaming_FileIndexIterator_somDumpSelfInt SOMObject_somDumpSelfInt
#define FileXNaming_FileIndexIterator_somDefaultInit SOMObject_somDefaultInit
#define FileXNaming_FileIndexIterator_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define FileXNaming_FileIndexIterator_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define FileXNaming_FileIndexIterator_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define FileXNaming_FileIndexIterator_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define FileXNaming_FileIndexIterator_somDefaultAssign SOMObject_somDefaultAssign
#define FileXNaming_FileIndexIterator_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define FileXNaming_FileIndexIterator_somDefaultVAssign SOMObject_somDefaultVAssign
#define FileXNaming_FileIndexIterator_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define FileXNaming_FileIndexIterator_somDestruct SOMObject_somDestruct
#define FileXNaming_FileIndexIterator_next_one ExtendedNaming_IndexIterator_next_one
#define FileXNaming_FileIndexIterator_next_n ExtendedNaming_IndexIterator_next_n
#define FileXNaming_FileIndexIterator_destroy ExtendedNaming_IndexIterator_destroy
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_xnamingf_Header_h */
