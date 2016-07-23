/* generated from somap.idl */
/* internal conditional is SOM_Module_somap_Source */
#ifndef SOM_Module_somap_Header_h
	#define SOM_Module_somap_Header_h 1
	#include <som.h>
	#include <somos.h>
	#include <somobj.h>
	#include <somdserv.h>
	#include <somdtype.h>
	#include <containd.h>
	#include <snglicls.h>
	#include <somcls.h>
	#include <xnaming.h>
	#include <naming.h>
	#include <somdcprx.h>
	#include <somproxy.h>
	#include <somdobj.h>
	#include <unotypes.h>
	#include <xmscssae.h>
	#include <omgidobj.h>
	#include <somtdm.h>
	#include <principl.h>
		#ifndef _IDL_somAttributePersistence_Persist_defined
			#define _IDL_somAttributePersistence_Persist_defined
			typedef SOMObject somAttributePersistence_Persist;
		#endif /* _IDL_somAttributePersistence_Persist_defined */
		#ifndef _IDL_SEQUENCE_somAttributePersistence_Persist_defined
			#define _IDL_SEQUENCE_somAttributePersistence_Persist_defined
			SOM_SEQUENCE_TYPEDEF_NAME(somAttributePersistence_Persist SOMSTAR ,sequence(somAttributePersistence_Persist));
		#endif /* _IDL_SEQUENCE_somAttributePersistence_Persist_defined */
		#ifdef __IBMC__
			typedef void (somTP_somAttributePersistence_Persist_load_attribute)(
				somAttributePersistence_Persist SOMSTAR somSelf,
				Environment *ev,
				/* in */ string attribute_name,
				/* out */ void **buffer,
				/* in */ TypeCode attribute_tc);
			#pragma linkage(somTP_somAttributePersistence_Persist_load_attribute,system)
			typedef somTP_somAttributePersistence_Persist_load_attribute *somTD_somAttributePersistence_Persist_load_attribute;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_somAttributePersistence_Persist_load_attribute)(
				somAttributePersistence_Persist SOMSTAR somSelf,
				Environment *ev,
				/* in */ string attribute_name,
				/* out */ void **buffer,
				/* in */ TypeCode attribute_tc);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_somAttributePersistence_Persist_store_attribute)(
				somAttributePersistence_Persist SOMSTAR somSelf,
				Environment *ev,
				/* in */ string attribute_name,
				/* in */ void *buffer,
				/* in */ TypeCode attribute_tc);
			#pragma linkage(somTP_somAttributePersistence_Persist_store_attribute,system)
			typedef somTP_somAttributePersistence_Persist_store_attribute *somTD_somAttributePersistence_Persist_store_attribute;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_somAttributePersistence_Persist_store_attribute)(
				somAttributePersistence_Persist SOMSTAR somSelf,
				Environment *ev,
				/* in */ string attribute_name,
				/* in */ void *buffer,
				/* in */ TypeCode attribute_tc);
		#endif /* __IBMC__ */
		#ifndef somAttributePersistence_Persist_MajorVersion
			#define somAttributePersistence_Persist_MajorVersion   3
		#endif /* somAttributePersistence_Persist_MajorVersion */
		#ifndef somAttributePersistence_Persist_MinorVersion
			#define somAttributePersistence_Persist_MinorVersion   0
		#endif /* somAttributePersistence_Persist_MinorVersion */
		typedef struct somAttributePersistence_PersistClassDataStructure
		{
			SOMClass SOMSTAR classObject;
			somMToken load_attribute;
			somMToken store_attribute;
		} somAttributePersistence_PersistClassDataStructure;
		typedef struct somAttributePersistence_PersistCClassDataStructure
		{
			somMethodTabs parentMtab;
			somDToken instanceDataToken;
		} somAttributePersistence_PersistCClassDataStructure;
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_somap_Source) || defined(somAttributePersistence_Persist_Class_Source)
				SOMEXTERN struct somAttributePersistence_PersistClassDataStructure _somAttributePersistence_PersistClassData;
				#ifndef somAttributePersistence_PersistClassData
					#define somAttributePersistence_PersistClassData    _somAttributePersistence_PersistClassData
				#endif /* somAttributePersistence_PersistClassData */
			#else
				SOMEXTERN struct somAttributePersistence_PersistClassDataStructure * SOMLINK resolve_somAttributePersistence_PersistClassData(void);
				#ifndef somAttributePersistence_PersistClassData
					#define somAttributePersistence_PersistClassData    (*(resolve_somAttributePersistence_PersistClassData()))
				#endif /* somAttributePersistence_PersistClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_somap_Source) || defined(somAttributePersistence_Persist_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_somap_Source || somAttributePersistence_Persist_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_somap_Source || somAttributePersistence_Persist_Class_Source */
			struct somAttributePersistence_PersistClassDataStructure SOMDLINK somAttributePersistence_PersistClassData;
		#endif /* SOM_RESOLVE_DATA */
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_somap_Source) || defined(somAttributePersistence_Persist_Class_Source)
				SOMEXTERN struct somAttributePersistence_PersistCClassDataStructure _somAttributePersistence_PersistCClassData;
				#ifndef somAttributePersistence_PersistCClassData
					#define somAttributePersistence_PersistCClassData    _somAttributePersistence_PersistCClassData
				#endif /* somAttributePersistence_PersistCClassData */
			#else
				SOMEXTERN struct somAttributePersistence_PersistCClassDataStructure * SOMLINK resolve_somAttributePersistence_PersistCClassData(void);
				#ifndef somAttributePersistence_PersistCClassData
					#define somAttributePersistence_PersistCClassData    (*(resolve_somAttributePersistence_PersistCClassData()))
				#endif /* somAttributePersistence_PersistCClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_somap_Source) || defined(somAttributePersistence_Persist_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_somap_Source || somAttributePersistence_Persist_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_somap_Source || somAttributePersistence_Persist_Class_Source */
			struct somAttributePersistence_PersistCClassDataStructure SOMDLINK somAttributePersistence_PersistCClassData;
		#endif /* SOM_RESOLVE_DATA */
		extern 
		#if defined(SOM_Module_somap_Source) || defined(somAttributePersistence_Persist_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_somap_Source || somAttributePersistence_Persist_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_somap_Source || somAttributePersistence_Persist_Class_Source */
		SOMClass SOMSTAR SOMLINK somAttributePersistence_PersistNewClass
				(integer4 somtmajorVersion,integer4 somtminorVersion);
		#define _SOMCLASS_somAttributePersistence_Persist (somAttributePersistence_PersistClassData.classObject)
		#ifndef SOMGD_somAttributePersistence_Persist
			#if (defined(_somAttributePersistence_Persist) || defined(__somAttributePersistence_Persist))
				#undef _somAttributePersistence_Persist
				#undef __somAttributePersistence_Persist
				#define SOMGD_somAttributePersistence_Persist 1
			#else
				#define _somAttributePersistence_Persist _SOMCLASS_somAttributePersistence_Persist
			#endif /* _somAttributePersistence_Persist */
		#endif /* SOMGD_somAttributePersistence_Persist */
		#define somAttributePersistence_Persist_classObj _SOMCLASS_somAttributePersistence_Persist
		#define _SOMMTOKEN_somAttributePersistence_Persist(method) ((somMToken)(somAttributePersistence_PersistClassData.method))
		#ifndef somAttributePersistence_PersistNew
			#define somAttributePersistence_PersistNew() ( _somAttributePersistence_Persist ? \
				(SOMClass_somNew(_somAttributePersistence_Persist)) : \
				( somAttributePersistence_PersistNewClass( \
					somAttributePersistence_Persist_MajorVersion, \
					somAttributePersistence_Persist_MinorVersion), \
				SOMClass_somNew(_somAttributePersistence_Persist))) 
		#endif /* NewsomAttributePersistence_Persist */
		#ifndef somAttributePersistence_Persist_load_attribute
			#define somAttributePersistence_Persist_load_attribute(somSelf,ev,attribute_name,buffer,attribute_tc) \
				SOM_Resolve(somSelf,somAttributePersistence_Persist,load_attribute)  \
					(somSelf,ev,attribute_name,buffer,attribute_tc)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__load_attribute
					#if defined(_load_attribute)
						#undef _load_attribute
						#define SOMGD__load_attribute
					#else
						#define _load_attribute somAttributePersistence_Persist_load_attribute
					#endif
				#endif /* SOMGD__load_attribute */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* somAttributePersistence_Persist_load_attribute */
		#ifndef somAttributePersistence_Persist_store_attribute
			#define somAttributePersistence_Persist_store_attribute(somSelf,ev,attribute_name,buffer,attribute_tc) \
				SOM_Resolve(somSelf,somAttributePersistence_Persist,store_attribute)  \
					(somSelf,ev,attribute_name,buffer,attribute_tc)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__store_attribute
					#if defined(_store_attribute)
						#undef _store_attribute
						#define SOMGD__store_attribute
					#else
						#define _store_attribute somAttributePersistence_Persist_store_attribute
					#endif
				#endif /* SOMGD__store_attribute */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* somAttributePersistence_Persist_store_attribute */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define somAttributePersistence_Persist_somInit SOMObject_somInit
#define somAttributePersistence_Persist_somUninit SOMObject_somUninit
#define somAttributePersistence_Persist_somFree SOMObject_somFree
#define somAttributePersistence_Persist_somGetClass SOMObject_somGetClass
#define somAttributePersistence_Persist_somGetClassName SOMObject_somGetClassName
#define somAttributePersistence_Persist_somGetSize SOMObject_somGetSize
#define somAttributePersistence_Persist_somIsA SOMObject_somIsA
#define somAttributePersistence_Persist_somIsInstanceOf SOMObject_somIsInstanceOf
#define somAttributePersistence_Persist_somRespondsTo SOMObject_somRespondsTo
#define somAttributePersistence_Persist_somDispatch SOMObject_somDispatch
#define somAttributePersistence_Persist_somClassDispatch SOMObject_somClassDispatch
#define somAttributePersistence_Persist_somCastObj SOMObject_somCastObj
#define somAttributePersistence_Persist_somResetObj SOMObject_somResetObj
#define somAttributePersistence_Persist_somPrintSelf SOMObject_somPrintSelf
#define somAttributePersistence_Persist_somDumpSelf SOMObject_somDumpSelf
#define somAttributePersistence_Persist_somDumpSelfInt SOMObject_somDumpSelfInt
#define somAttributePersistence_Persist_somDefaultInit SOMObject_somDefaultInit
#define somAttributePersistence_Persist_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define somAttributePersistence_Persist_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define somAttributePersistence_Persist_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define somAttributePersistence_Persist_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define somAttributePersistence_Persist_somDefaultAssign SOMObject_somDefaultAssign
#define somAttributePersistence_Persist_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define somAttributePersistence_Persist_somDefaultVAssign SOMObject_somDefaultVAssign
#define somAttributePersistence_Persist_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define somAttributePersistence_Persist_somDestruct SOMObject_somDestruct
#define somAttributePersistence_Persist__get_constant_random_id CosObjectIdentity_IdentifiableObject__get_constant_random_id
#define somAttributePersistence_Persist_is_identical CosObjectIdentity_IdentifiableObject_is_identical
#define somAttributePersistence_Persist_reinit somOS_ServiceBase_reinit
#define somAttributePersistence_Persist_capture somOS_ServiceBase_capture
#define somAttributePersistence_Persist_GetInstanceManager somOS_ServiceBase_GetInstanceManager
#define somAttributePersistence_Persist_init_for_object_creation somOS_ServiceBase_init_for_object_creation
#define somAttributePersistence_Persist_init_for_object_reactivation somOS_ServiceBase_init_for_object_reactivation
#define somAttributePersistence_Persist_init_for_object_copy somOS_ServiceBase_init_for_object_copy
#define somAttributePersistence_Persist_uninit_for_object_move somOS_ServiceBase_uninit_for_object_move
#define somAttributePersistence_Persist_uninit_for_object_passivation somOS_ServiceBase_uninit_for_object_passivation
#define somAttributePersistence_Persist_uninit_for_object_destruction somOS_ServiceBase_uninit_for_object_destruction
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_somap_Header_h */
