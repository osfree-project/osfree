/* generated from omgidobj.idl */
/* internal conditional is SOM_Module_omgidobj_Source */
#ifndef SOM_Module_omgidobj_Header_h
	#define SOM_Module_omgidobj_Header_h 1
	#include <som.h>
	#include <somobj.h>
		#ifndef _IDL_CosObjectIdentity_IdentifiableObject_defined
			#define _IDL_CosObjectIdentity_IdentifiableObject_defined
			typedef SOMObject CosObjectIdentity_IdentifiableObject;
		#endif /* _IDL_CosObjectIdentity_IdentifiableObject_defined */
		#ifndef _IDL_SEQUENCE_CosObjectIdentity_IdentifiableObject_defined
			#define _IDL_SEQUENCE_CosObjectIdentity_IdentifiableObject_defined
			SOM_SEQUENCE_TYPEDEF_NAME(CosObjectIdentity_IdentifiableObject SOMSTAR ,sequence(CosObjectIdentity_IdentifiableObject));
		#endif /* _IDL_SEQUENCE_CosObjectIdentity_IdentifiableObject_defined */
		typedef unsigned long CosObjectIdentity_ObjectIdentifier;
		#ifndef _IDL_SEQUENCE_ulong_defined
			#define _IDL_SEQUENCE_ulong_defined
			SOM_SEQUENCE_TYPEDEF_NAME(unsigned long ,sequence(ulong));
		#endif /* _IDL_SEQUENCE_ulong_defined */
		#ifdef __IBMC__
			typedef CosObjectIdentity_ObjectIdentifier (somTP_CosObjectIdentity_IdentifiableObject__get_constant_random_id)(
				CosObjectIdentity_IdentifiableObject SOMSTAR somSelf,
				Environment *ev);
			#pragma linkage(somTP_CosObjectIdentity_IdentifiableObject__get_constant_random_id,system)
			typedef somTP_CosObjectIdentity_IdentifiableObject__get_constant_random_id *somTD_CosObjectIdentity_IdentifiableObject__get_constant_random_id;
		#else /* __IBMC__ */
			typedef CosObjectIdentity_ObjectIdentifier (SOMLINK * somTD_CosObjectIdentity_IdentifiableObject__get_constant_random_id)(
				CosObjectIdentity_IdentifiableObject SOMSTAR somSelf,
				Environment *ev);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef boolean (somTP_CosObjectIdentity_IdentifiableObject_is_identical)(
				CosObjectIdentity_IdentifiableObject SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosObjectIdentity_IdentifiableObject SOMSTAR other_object);
			#pragma linkage(somTP_CosObjectIdentity_IdentifiableObject_is_identical,system)
			typedef somTP_CosObjectIdentity_IdentifiableObject_is_identical *somTD_CosObjectIdentity_IdentifiableObject_is_identical;
		#else /* __IBMC__ */
			typedef boolean (SOMLINK * somTD_CosObjectIdentity_IdentifiableObject_is_identical)(
				CosObjectIdentity_IdentifiableObject SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosObjectIdentity_IdentifiableObject SOMSTAR other_object);
		#endif /* __IBMC__ */
		#ifndef CosObjectIdentity_IdentifiableObject_MajorVersion
			#define CosObjectIdentity_IdentifiableObject_MajorVersion   3
		#endif /* CosObjectIdentity_IdentifiableObject_MajorVersion */
		#ifndef CosObjectIdentity_IdentifiableObject_MinorVersion
			#define CosObjectIdentity_IdentifiableObject_MinorVersion   0
		#endif /* CosObjectIdentity_IdentifiableObject_MinorVersion */
		typedef struct CosObjectIdentity_IdentifiableObjectClassDataStructure
		{
			SOMClass SOMSTAR classObject;
			somMToken _get_constant_random_id;
			somMToken private1;
			somMToken is_identical;
		} CosObjectIdentity_IdentifiableObjectClassDataStructure;
		typedef struct CosObjectIdentity_IdentifiableObjectCClassDataStructure
		{
			somMethodTabs parentMtab;
			somDToken instanceDataToken;
		} CosObjectIdentity_IdentifiableObjectCClassDataStructure;
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_omgidobj_Source) || defined(CosObjectIdentity_IdentifiableObject_Class_Source)
				SOMEXTERN struct CosObjectIdentity_IdentifiableObjectClassDataStructure _CosObjectIdentity_IdentifiableObjectClassData;
				#ifndef CosObjectIdentity_IdentifiableObjectClassData
					#define CosObjectIdentity_IdentifiableObjectClassData    _CosObjectIdentity_IdentifiableObjectClassData
				#endif /* CosObjectIdentity_IdentifiableObjectClassData */
			#else
				SOMEXTERN struct CosObjectIdentity_IdentifiableObjectClassDataStructure * SOMLINK resolve_CosObjectIdentity_IdentifiableObjectClassData(void);
				#ifndef CosObjectIdentity_IdentifiableObjectClassData
					#define CosObjectIdentity_IdentifiableObjectClassData    (*(resolve_CosObjectIdentity_IdentifiableObjectClassData()))
				#endif /* CosObjectIdentity_IdentifiableObjectClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_omgidobj_Source) || defined(CosObjectIdentity_IdentifiableObject_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_omgidobj_Source || CosObjectIdentity_IdentifiableObject_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_omgidobj_Source || CosObjectIdentity_IdentifiableObject_Class_Source */
			struct CosObjectIdentity_IdentifiableObjectClassDataStructure SOMDLINK CosObjectIdentity_IdentifiableObjectClassData;
		#endif /* SOM_RESOLVE_DATA */
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_omgidobj_Source) || defined(CosObjectIdentity_IdentifiableObject_Class_Source)
				SOMEXTERN struct CosObjectIdentity_IdentifiableObjectCClassDataStructure _CosObjectIdentity_IdentifiableObjectCClassData;
				#ifndef CosObjectIdentity_IdentifiableObjectCClassData
					#define CosObjectIdentity_IdentifiableObjectCClassData    _CosObjectIdentity_IdentifiableObjectCClassData
				#endif /* CosObjectIdentity_IdentifiableObjectCClassData */
			#else
				SOMEXTERN struct CosObjectIdentity_IdentifiableObjectCClassDataStructure * SOMLINK resolve_CosObjectIdentity_IdentifiableObjectCClassData(void);
				#ifndef CosObjectIdentity_IdentifiableObjectCClassData
					#define CosObjectIdentity_IdentifiableObjectCClassData    (*(resolve_CosObjectIdentity_IdentifiableObjectCClassData()))
				#endif /* CosObjectIdentity_IdentifiableObjectCClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_omgidobj_Source) || defined(CosObjectIdentity_IdentifiableObject_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_omgidobj_Source || CosObjectIdentity_IdentifiableObject_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_omgidobj_Source || CosObjectIdentity_IdentifiableObject_Class_Source */
			struct CosObjectIdentity_IdentifiableObjectCClassDataStructure SOMDLINK CosObjectIdentity_IdentifiableObjectCClassData;
		#endif /* SOM_RESOLVE_DATA */
		extern 
		#if defined(SOM_Module_omgidobj_Source) || defined(CosObjectIdentity_IdentifiableObject_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_omgidobj_Source || CosObjectIdentity_IdentifiableObject_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_omgidobj_Source || CosObjectIdentity_IdentifiableObject_Class_Source */
		SOMClass SOMSTAR SOMLINK CosObjectIdentity_IdentifiableObjectNewClass
				(integer4 somtmajorVersion,integer4 somtminorVersion);
		#define _SOMCLASS_CosObjectIdentity_IdentifiableObject (CosObjectIdentity_IdentifiableObjectClassData.classObject)
		#ifndef SOMGD_CosObjectIdentity_IdentifiableObject
			#if (defined(_CosObjectIdentity_IdentifiableObject) || defined(__CosObjectIdentity_IdentifiableObject))
				#undef _CosObjectIdentity_IdentifiableObject
				#undef __CosObjectIdentity_IdentifiableObject
				#define SOMGD_CosObjectIdentity_IdentifiableObject 1
			#else
				#define _CosObjectIdentity_IdentifiableObject _SOMCLASS_CosObjectIdentity_IdentifiableObject
			#endif /* _CosObjectIdentity_IdentifiableObject */
		#endif /* SOMGD_CosObjectIdentity_IdentifiableObject */
		#define CosObjectIdentity_IdentifiableObject_classObj _SOMCLASS_CosObjectIdentity_IdentifiableObject
		#define _SOMMTOKEN_CosObjectIdentity_IdentifiableObject(method) ((somMToken)(CosObjectIdentity_IdentifiableObjectClassData.method))
		#ifndef CosObjectIdentity_IdentifiableObjectNew
			#define CosObjectIdentity_IdentifiableObjectNew() ( _CosObjectIdentity_IdentifiableObject ? \
				(SOMClass_somNew(_CosObjectIdentity_IdentifiableObject)) : \
				( CosObjectIdentity_IdentifiableObjectNewClass( \
					CosObjectIdentity_IdentifiableObject_MajorVersion, \
					CosObjectIdentity_IdentifiableObject_MinorVersion), \
				SOMClass_somNew(_CosObjectIdentity_IdentifiableObject))) 
		#endif /* NewCosObjectIdentity_IdentifiableObject */
		#ifndef CosObjectIdentity_IdentifiableObject__get_constant_random_id
			#define CosObjectIdentity_IdentifiableObject__get_constant_random_id(somSelf,ev) \
				SOM_Resolve(somSelf,CosObjectIdentity_IdentifiableObject,_get_constant_random_id)  \
					(somSelf,ev)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD___get_constant_random_id
					#if defined(__get_constant_random_id)
						#undef __get_constant_random_id
						#define SOMGD___get_constant_random_id
					#else
						#define __get_constant_random_id CosObjectIdentity_IdentifiableObject__get_constant_random_id
					#endif
				#endif /* SOMGD___get_constant_random_id */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosObjectIdentity_IdentifiableObject__get_constant_random_id */
		#ifndef CosObjectIdentity_IdentifiableObject_is_identical
			#define CosObjectIdentity_IdentifiableObject_is_identical(somSelf,ev,other_object) \
				SOM_Resolve(somSelf,CosObjectIdentity_IdentifiableObject,is_identical)  \
					(somSelf,ev,other_object)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__is_identical
					#if defined(_is_identical)
						#undef _is_identical
						#define SOMGD__is_identical
					#else
						#define _is_identical CosObjectIdentity_IdentifiableObject_is_identical
					#endif
				#endif /* SOMGD__is_identical */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosObjectIdentity_IdentifiableObject_is_identical */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define CosObjectIdentity_IdentifiableObject_somInit SOMObject_somInit
#define CosObjectIdentity_IdentifiableObject_somUninit SOMObject_somUninit
#define CosObjectIdentity_IdentifiableObject_somFree SOMObject_somFree
#define CosObjectIdentity_IdentifiableObject_somGetClass SOMObject_somGetClass
#define CosObjectIdentity_IdentifiableObject_somGetClassName SOMObject_somGetClassName
#define CosObjectIdentity_IdentifiableObject_somGetSize SOMObject_somGetSize
#define CosObjectIdentity_IdentifiableObject_somIsA SOMObject_somIsA
#define CosObjectIdentity_IdentifiableObject_somIsInstanceOf SOMObject_somIsInstanceOf
#define CosObjectIdentity_IdentifiableObject_somRespondsTo SOMObject_somRespondsTo
#define CosObjectIdentity_IdentifiableObject_somDispatch SOMObject_somDispatch
#define CosObjectIdentity_IdentifiableObject_somClassDispatch SOMObject_somClassDispatch
#define CosObjectIdentity_IdentifiableObject_somCastObj SOMObject_somCastObj
#define CosObjectIdentity_IdentifiableObject_somResetObj SOMObject_somResetObj
#define CosObjectIdentity_IdentifiableObject_somPrintSelf SOMObject_somPrintSelf
#define CosObjectIdentity_IdentifiableObject_somDumpSelf SOMObject_somDumpSelf
#define CosObjectIdentity_IdentifiableObject_somDumpSelfInt SOMObject_somDumpSelfInt
#define CosObjectIdentity_IdentifiableObject_somDefaultInit SOMObject_somDefaultInit
#define CosObjectIdentity_IdentifiableObject_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define CosObjectIdentity_IdentifiableObject_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define CosObjectIdentity_IdentifiableObject_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define CosObjectIdentity_IdentifiableObject_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define CosObjectIdentity_IdentifiableObject_somDefaultAssign SOMObject_somDefaultAssign
#define CosObjectIdentity_IdentifiableObject_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define CosObjectIdentity_IdentifiableObject_somDefaultVAssign SOMObject_somDefaultVAssign
#define CosObjectIdentity_IdentifiableObject_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define CosObjectIdentity_IdentifiableObject_somDestruct SOMObject_somDestruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_omgidobj_Header_h */
