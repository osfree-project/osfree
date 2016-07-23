/* generated from somstrt.idl */
/* internal conditional is SOM_Module_somstrt_Source */
#ifndef SOM_Module_somstrt_Header_h
	#define SOM_Module_somstrt_Header_h 1
	#include <som.h>
	#include <somobj.h>
	#ifndef _IDL_SOMStringTableC_defined
		#define _IDL_SOMStringTableC_defined
		typedef SOMObject SOMStringTableC;
	#endif /* _IDL_SOMStringTableC_defined */
	#ifndef _IDL_SEQUENCE_SOMStringTableC_defined
		#define _IDL_SEQUENCE_SOMStringTableC_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMStringTableC SOMSTAR ,sequence(SOMStringTableC));
	#endif /* _IDL_SEQUENCE_SOMStringTableC_defined */
	typedef struct SOMStringTableC_entryT SOMStringTableC_entryT;
	#ifndef _IDL_SEQUENCE_SOMStringTableC_entryT_defined
		#define _IDL_SEQUENCE_SOMStringTableC_entryT_defined
		SOM_SEQUENCE_TYPEDEF(SOMStringTableC_entryT);
	#endif /* _IDL_SEQUENCE_SOMStringTableC_entryT_defined */
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_entryT
			#if defined(entryT)
				#undef entryT
				#define SOMGD_entryT
			#else
				#define entryT SOMStringTableC_entryT
			#endif
		#endif /* SOMGD_entryT */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef _IDL_SEQUENCE_entryT_defined
			#define _IDL_SEQUENCE_entryT_defined
			#define _IDL_SEQUENCE_entryT _IDL_SEQUENCE_SOMStringTableC_entryT
		#endif /* _IDL_SEQUENCE_entryT_defined */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
	#ifdef __IBMC__
		typedef void (somTP_SOMStringTableC__set_somstTargetCapacity)(
			SOMStringTableC SOMSTAR somSelf,
			/* in */ unsigned long somstTargetCapacity);
		#pragma linkage(somTP_SOMStringTableC__set_somstTargetCapacity,system)
		typedef somTP_SOMStringTableC__set_somstTargetCapacity *somTD_SOMStringTableC__set_somstTargetCapacity;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMStringTableC__set_somstTargetCapacity)(
			SOMStringTableC SOMSTAR somSelf,
			/* in */ unsigned long somstTargetCapacity);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef unsigned long (somTP_SOMStringTableC__get_somstTargetCapacity)(
			SOMStringTableC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMStringTableC__get_somstTargetCapacity,system)
		typedef somTP_SOMStringTableC__get_somstTargetCapacity *somTD_SOMStringTableC__get_somstTargetCapacity;
	#else /* __IBMC__ */
		typedef unsigned long (SOMLINK * somTD_SOMStringTableC__get_somstTargetCapacity)(
			SOMStringTableC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef unsigned long (somTP_SOMStringTableC__get_somstAssociationsCount)(
			SOMStringTableC SOMSTAR somSelf);
		#pragma linkage(somTP_SOMStringTableC__get_somstAssociationsCount,system)
		typedef somTP_SOMStringTableC__get_somstAssociationsCount *somTD_SOMStringTableC__get_somstAssociationsCount;
	#else /* __IBMC__ */
		typedef unsigned long (SOMLINK * somTD_SOMStringTableC__get_somstAssociationsCount)(
			SOMStringTableC SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef short (somTP_SOMStringTableC_somstAssociate)(
			SOMStringTableC SOMSTAR somSelf,
			/* in */ string key,
			/* in */ string value);
		#pragma linkage(somTP_SOMStringTableC_somstAssociate,system)
		typedef somTP_SOMStringTableC_somstAssociate *somTD_SOMStringTableC_somstAssociate;
	#else /* __IBMC__ */
		typedef short (SOMLINK * somTD_SOMStringTableC_somstAssociate)(
			SOMStringTableC SOMSTAR somSelf,
			/* in */ string key,
			/* in */ string value);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef short (somTP_SOMStringTableC_somstAssociateCopyKey)(
			SOMStringTableC SOMSTAR somSelf,
			/* in */ string key,
			/* in */ string value);
		#pragma linkage(somTP_SOMStringTableC_somstAssociateCopyKey,system)
		typedef somTP_SOMStringTableC_somstAssociateCopyKey *somTD_SOMStringTableC_somstAssociateCopyKey;
	#else /* __IBMC__ */
		typedef short (SOMLINK * somTD_SOMStringTableC_somstAssociateCopyKey)(
			SOMStringTableC SOMSTAR somSelf,
			/* in */ string key,
			/* in */ string value);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef short (somTP_SOMStringTableC_somstAssociateCopyValue)(
			SOMStringTableC SOMSTAR somSelf,
			/* in */ string key,
			/* in */ string value);
		#pragma linkage(somTP_SOMStringTableC_somstAssociateCopyValue,system)
		typedef somTP_SOMStringTableC_somstAssociateCopyValue *somTD_SOMStringTableC_somstAssociateCopyValue;
	#else /* __IBMC__ */
		typedef short (SOMLINK * somTD_SOMStringTableC_somstAssociateCopyValue)(
			SOMStringTableC SOMSTAR somSelf,
			/* in */ string key,
			/* in */ string value);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef short (somTP_SOMStringTableC_somstAssociateCopyBoth)(
			SOMStringTableC SOMSTAR somSelf,
			/* in */ string key,
			/* in */ string value);
		#pragma linkage(somTP_SOMStringTableC_somstAssociateCopyBoth,system)
		typedef somTP_SOMStringTableC_somstAssociateCopyBoth *somTD_SOMStringTableC_somstAssociateCopyBoth;
	#else /* __IBMC__ */
		typedef short (SOMLINK * somTD_SOMStringTableC_somstAssociateCopyBoth)(
			SOMStringTableC SOMSTAR somSelf,
			/* in */ string key,
			/* in */ string value);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_SOMStringTableC_somstGetAssociation)(
			SOMStringTableC SOMSTAR somSelf,
			/* in */ string key);
		#pragma linkage(somTP_SOMStringTableC_somstGetAssociation,system)
		typedef somTP_SOMStringTableC_somstGetAssociation *somTD_SOMStringTableC_somstGetAssociation;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_SOMStringTableC_somstGetAssociation)(
			SOMStringTableC SOMSTAR somSelf,
			/* in */ string key);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef boolean (somTP_SOMStringTableC_somstClearAssociation)(
			SOMStringTableC SOMSTAR somSelf,
			/* in */ string key);
		#pragma linkage(somTP_SOMStringTableC_somstClearAssociation,system)
		typedef somTP_SOMStringTableC_somstClearAssociation *somTD_SOMStringTableC_somstClearAssociation;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_SOMStringTableC_somstClearAssociation)(
			SOMStringTableC SOMSTAR somSelf,
			/* in */ string key);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_SOMStringTableC_somstGetIthKey)(
			SOMStringTableC SOMSTAR somSelf,
			/* in */ unsigned long i);
		#pragma linkage(somTP_SOMStringTableC_somstGetIthKey,system)
		typedef somTP_SOMStringTableC_somstGetIthKey *somTD_SOMStringTableC_somstGetIthKey;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_SOMStringTableC_somstGetIthKey)(
			SOMStringTableC SOMSTAR somSelf,
			/* in */ unsigned long i);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_SOMStringTableC_somstGetIthValue)(
			SOMStringTableC SOMSTAR somSelf,
			/* in */ unsigned long i);
		#pragma linkage(somTP_SOMStringTableC_somstGetIthValue,system)
		typedef somTP_SOMStringTableC_somstGetIthValue *somTD_SOMStringTableC_somstGetIthValue;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_SOMStringTableC_somstGetIthValue)(
			SOMStringTableC SOMSTAR somSelf,
			/* in */ unsigned long i);
	#endif /* __IBMC__ */
	#ifndef SOMStringTableC_MajorVersion
		#define SOMStringTableC_MajorVersion   2
	#endif /* SOMStringTableC_MajorVersion */
	#ifndef SOMStringTableC_MinorVersion
		#define SOMStringTableC_MinorVersion   1
	#endif /* SOMStringTableC_MinorVersion */
	typedef struct SOMStringTableCClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken _get_somstTargetCapacity;
		somMToken _set_somstTargetCapacity;
		somMToken _get_somstAssociationsCount;
		somMToken somstAssociate;
		somMToken somstAssociateCopyKey;
		somMToken somstAssociateCopyValue;
		somMToken somstAssociateCopyBoth;
		somMToken somstGetAssociation;
		somMToken somstClearAssociation;
		somMToken somstGetIthKey;
		somMToken somstGetIthValue;
	} SOMStringTableCClassDataStructure;
	typedef struct SOMStringTableCCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} SOMStringTableCCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_somstrt_Source) || defined(SOMStringTableC_Class_Source)
			SOMEXTERN struct SOMStringTableCClassDataStructure _SOMStringTableCClassData;
			#ifndef SOMStringTableCClassData
				#define SOMStringTableCClassData    _SOMStringTableCClassData
			#endif /* SOMStringTableCClassData */
		#else
			SOMEXTERN struct SOMStringTableCClassDataStructure * SOMLINK resolve_SOMStringTableCClassData(void);
			#ifndef SOMStringTableCClassData
				#define SOMStringTableCClassData    (*(resolve_SOMStringTableCClassData()))
			#endif /* SOMStringTableCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_somstrt_Source) || defined(SOMStringTableC_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_somstrt_Source || SOMStringTableC_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_somstrt_Source || SOMStringTableC_Class_Source */
		struct SOMStringTableCClassDataStructure SOMDLINK SOMStringTableCClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_somstrt_Source) || defined(SOMStringTableC_Class_Source)
			SOMEXTERN struct SOMStringTableCCClassDataStructure _SOMStringTableCCClassData;
			#ifndef SOMStringTableCCClassData
				#define SOMStringTableCCClassData    _SOMStringTableCCClassData
			#endif /* SOMStringTableCCClassData */
		#else
			SOMEXTERN struct SOMStringTableCCClassDataStructure * SOMLINK resolve_SOMStringTableCCClassData(void);
			#ifndef SOMStringTableCCClassData
				#define SOMStringTableCCClassData    (*(resolve_SOMStringTableCCClassData()))
			#endif /* SOMStringTableCCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_somstrt_Source) || defined(SOMStringTableC_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_somstrt_Source || SOMStringTableC_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_somstrt_Source || SOMStringTableC_Class_Source */
		struct SOMStringTableCCClassDataStructure SOMDLINK SOMStringTableCCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_somstrt_Source) || defined(SOMStringTableC_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_somstrt_Source || SOMStringTableC_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_somstrt_Source || SOMStringTableC_Class_Source */
	SOMClass SOMSTAR SOMLINK SOMStringTableCNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_SOMStringTableC (SOMStringTableCClassData.classObject)
	#ifndef SOMGD_SOMStringTableC
		#if (defined(_SOMStringTableC) || defined(__SOMStringTableC))
			#undef _SOMStringTableC
			#undef __SOMStringTableC
			#define SOMGD_SOMStringTableC 1
		#else
			#define _SOMStringTableC _SOMCLASS_SOMStringTableC
		#endif /* _SOMStringTableC */
	#endif /* SOMGD_SOMStringTableC */
	#define SOMStringTableC_classObj _SOMCLASS_SOMStringTableC
	#define _SOMMTOKEN_SOMStringTableC(method) ((somMToken)(SOMStringTableCClassData.method))
	#ifndef SOMStringTableCNew
		#define SOMStringTableCNew() ( _SOMStringTableC ? \
			(SOMClass_somNew(_SOMStringTableC)) : \
			( SOMStringTableCNewClass( \
				SOMStringTableC_MajorVersion, \
				SOMStringTableC_MinorVersion), \
			SOMClass_somNew(_SOMStringTableC))) 
	#endif /* NewSOMStringTableC */
	#ifndef SOMStringTableC__set_somstTargetCapacity
		#define SOMStringTableC__set_somstTargetCapacity(somSelf,somstTargetCapacity) \
			SOM_Resolve(somSelf,SOMStringTableC,_set_somstTargetCapacity)  \
				(somSelf,somstTargetCapacity)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_somstTargetCapacity
				#if defined(__set_somstTargetCapacity)
					#undef __set_somstTargetCapacity
					#define SOMGD___set_somstTargetCapacity
				#else
					#define __set_somstTargetCapacity SOMStringTableC__set_somstTargetCapacity
				#endif
			#endif /* SOMGD___set_somstTargetCapacity */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMStringTableC__set_somstTargetCapacity */
	#ifndef SOMStringTableC__get_somstTargetCapacity
		#define SOMStringTableC__get_somstTargetCapacity(somSelf) \
			SOM_Resolve(somSelf,SOMStringTableC,_get_somstTargetCapacity)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somstTargetCapacity
				#if defined(__get_somstTargetCapacity)
					#undef __get_somstTargetCapacity
					#define SOMGD___get_somstTargetCapacity
				#else
					#define __get_somstTargetCapacity SOMStringTableC__get_somstTargetCapacity
				#endif
			#endif /* SOMGD___get_somstTargetCapacity */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMStringTableC__get_somstTargetCapacity */
	#ifndef SOMStringTableC__get_somstAssociationsCount
		#define SOMStringTableC__get_somstAssociationsCount(somSelf) \
			SOM_Resolve(somSelf,SOMStringTableC,_get_somstAssociationsCount)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somstAssociationsCount
				#if defined(__get_somstAssociationsCount)
					#undef __get_somstAssociationsCount
					#define SOMGD___get_somstAssociationsCount
				#else
					#define __get_somstAssociationsCount SOMStringTableC__get_somstAssociationsCount
				#endif
			#endif /* SOMGD___get_somstAssociationsCount */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMStringTableC__get_somstAssociationsCount */
	#ifndef SOMStringTableC_somstAssociate
		#define SOMStringTableC_somstAssociate(somSelf,key,value) \
			SOM_Resolve(somSelf,SOMStringTableC,somstAssociate)  \
				(somSelf,key,value)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somstAssociate
				#if defined(_somstAssociate)
					#undef _somstAssociate
					#define SOMGD__somstAssociate
				#else
					#define _somstAssociate SOMStringTableC_somstAssociate
				#endif
			#endif /* SOMGD__somstAssociate */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMStringTableC_somstAssociate */
	#ifndef SOMStringTableC_somstAssociateCopyKey
		#define SOMStringTableC_somstAssociateCopyKey(somSelf,key,value) \
			SOM_Resolve(somSelf,SOMStringTableC,somstAssociateCopyKey)  \
				(somSelf,key,value)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somstAssociateCopyKey
				#if defined(_somstAssociateCopyKey)
					#undef _somstAssociateCopyKey
					#define SOMGD__somstAssociateCopyKey
				#else
					#define _somstAssociateCopyKey SOMStringTableC_somstAssociateCopyKey
				#endif
			#endif /* SOMGD__somstAssociateCopyKey */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMStringTableC_somstAssociateCopyKey */
	#ifndef SOMStringTableC_somstAssociateCopyValue
		#define SOMStringTableC_somstAssociateCopyValue(somSelf,key,value) \
			SOM_Resolve(somSelf,SOMStringTableC,somstAssociateCopyValue)  \
				(somSelf,key,value)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somstAssociateCopyValue
				#if defined(_somstAssociateCopyValue)
					#undef _somstAssociateCopyValue
					#define SOMGD__somstAssociateCopyValue
				#else
					#define _somstAssociateCopyValue SOMStringTableC_somstAssociateCopyValue
				#endif
			#endif /* SOMGD__somstAssociateCopyValue */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMStringTableC_somstAssociateCopyValue */
	#ifndef SOMStringTableC_somstAssociateCopyBoth
		#define SOMStringTableC_somstAssociateCopyBoth(somSelf,key,value) \
			SOM_Resolve(somSelf,SOMStringTableC,somstAssociateCopyBoth)  \
				(somSelf,key,value)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somstAssociateCopyBoth
				#if defined(_somstAssociateCopyBoth)
					#undef _somstAssociateCopyBoth
					#define SOMGD__somstAssociateCopyBoth
				#else
					#define _somstAssociateCopyBoth SOMStringTableC_somstAssociateCopyBoth
				#endif
			#endif /* SOMGD__somstAssociateCopyBoth */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMStringTableC_somstAssociateCopyBoth */
	#ifndef SOMStringTableC_somstGetAssociation
		#define SOMStringTableC_somstGetAssociation(somSelf,key) \
			SOM_Resolve(somSelf,SOMStringTableC,somstGetAssociation)  \
				(somSelf,key)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somstGetAssociation
				#if defined(_somstGetAssociation)
					#undef _somstGetAssociation
					#define SOMGD__somstGetAssociation
				#else
					#define _somstGetAssociation SOMStringTableC_somstGetAssociation
				#endif
			#endif /* SOMGD__somstGetAssociation */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMStringTableC_somstGetAssociation */
	#ifndef SOMStringTableC_somstClearAssociation
		#define SOMStringTableC_somstClearAssociation(somSelf,key) \
			SOM_Resolve(somSelf,SOMStringTableC,somstClearAssociation)  \
				(somSelf,key)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somstClearAssociation
				#if defined(_somstClearAssociation)
					#undef _somstClearAssociation
					#define SOMGD__somstClearAssociation
				#else
					#define _somstClearAssociation SOMStringTableC_somstClearAssociation
				#endif
			#endif /* SOMGD__somstClearAssociation */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMStringTableC_somstClearAssociation */
	#ifndef SOMStringTableC_somstGetIthKey
		#define SOMStringTableC_somstGetIthKey(somSelf,i) \
			SOM_Resolve(somSelf,SOMStringTableC,somstGetIthKey)  \
				(somSelf,i)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somstGetIthKey
				#if defined(_somstGetIthKey)
					#undef _somstGetIthKey
					#define SOMGD__somstGetIthKey
				#else
					#define _somstGetIthKey SOMStringTableC_somstGetIthKey
				#endif
			#endif /* SOMGD__somstGetIthKey */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMStringTableC_somstGetIthKey */
	#ifndef SOMStringTableC_somstGetIthValue
		#define SOMStringTableC_somstGetIthValue(somSelf,i) \
			SOM_Resolve(somSelf,SOMStringTableC,somstGetIthValue)  \
				(somSelf,i)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somstGetIthValue
				#if defined(_somstGetIthValue)
					#undef _somstGetIthValue
					#define SOMGD__somstGetIthValue
				#else
					#define _somstGetIthValue SOMStringTableC_somstGetIthValue
				#endif
			#endif /* SOMGD__somstGetIthValue */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMStringTableC_somstGetIthValue */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define SOMStringTableC_somInit SOMObject_somInit
#define SOMStringTableC_somUninit SOMObject_somUninit
#define SOMStringTableC_somFree SOMObject_somFree
#define SOMStringTableC_somGetClass SOMObject_somGetClass
#define SOMStringTableC_somGetClassName SOMObject_somGetClassName
#define SOMStringTableC_somGetSize SOMObject_somGetSize
#define SOMStringTableC_somIsA SOMObject_somIsA
#define SOMStringTableC_somIsInstanceOf SOMObject_somIsInstanceOf
#define SOMStringTableC_somRespondsTo SOMObject_somRespondsTo
#define SOMStringTableC_somDispatch SOMObject_somDispatch
#define SOMStringTableC_somClassDispatch SOMObject_somClassDispatch
#define SOMStringTableC_somCastObj SOMObject_somCastObj
#define SOMStringTableC_somResetObj SOMObject_somResetObj
#define SOMStringTableC_somPrintSelf SOMObject_somPrintSelf
#define SOMStringTableC_somDumpSelf SOMObject_somDumpSelf
#define SOMStringTableC_somDumpSelfInt SOMObject_somDumpSelfInt
#define SOMStringTableC_somDefaultInit SOMObject_somDefaultInit
#define SOMStringTableC_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define SOMStringTableC_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define SOMStringTableC_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define SOMStringTableC_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define SOMStringTableC_somDefaultAssign SOMObject_somDefaultAssign
#define SOMStringTableC_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define SOMStringTableC_somDefaultVAssign SOMObject_somDefaultVAssign
#define SOMStringTableC_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define SOMStringTableC_somDestruct SOMObject_somDestruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_somstrt_Header_h */
