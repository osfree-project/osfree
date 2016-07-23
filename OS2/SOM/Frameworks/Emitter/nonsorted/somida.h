/* generated from somida.idl */
/* internal conditional is SOM_Module_somida_Source */
#ifndef SOM_Module_somida_Header_h
	#define SOM_Module_somida_Header_h 1
	#include <som.h>
	#include <somobj.h>
	#ifndef _IDL_SOMUTId_defined
		#define _IDL_SOMUTId_defined
		typedef SOMObject SOMUTId;
	#endif /* _IDL_SOMUTId_defined */
	#ifndef _IDL_SEQUENCE_SOMUTId_defined
		#define _IDL_SEQUENCE_SOMUTId_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMUTId SOMSTAR ,sequence(SOMUTId));
	#endif /* _IDL_SEQUENCE_SOMUTId_defined */
	#ifdef __IBMC__
		typedef void (somTP_SOMUTId_somutSetIdId)(
			SOMUTId SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMUTId SOMSTAR otherId);
		#pragma linkage(somTP_SOMUTId_somutSetIdId,system)
		typedef somTP_SOMUTId_somutSetIdId *somTD_SOMUTId_somutSetIdId;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMUTId_somutSetIdId)(
			SOMUTId SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMUTId SOMSTAR otherId);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef boolean (somTP_SOMUTId_somutEqualsId)(
			SOMUTId SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMUTId SOMSTAR otherId);
		#pragma linkage(somTP_SOMUTId_somutEqualsId,system)
		typedef somTP_SOMUTId_somutEqualsId *somTD_SOMUTId_somutEqualsId;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_SOMUTId_somutEqualsId)(
			SOMUTId SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMUTId SOMSTAR otherId);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef short (somTP_SOMUTId_somutCompareId)(
			SOMUTId SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMUTId SOMSTAR otherId);
		#pragma linkage(somTP_SOMUTId_somutCompareId,system)
		typedef somTP_SOMUTId_somutCompareId *somTD_SOMUTId_somutCompareId;
	#else /* __IBMC__ */
		typedef short (SOMLINK * somTD_SOMUTId_somutCompareId)(
			SOMUTId SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMUTId SOMSTAR otherId);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef unsigned long (somTP_SOMUTId_somutHashId)(
			SOMUTId SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_SOMUTId_somutHashId,system)
		typedef somTP_SOMUTId_somutHashId *somTD_SOMUTId_somutHashId;
	#else /* __IBMC__ */
		typedef unsigned long (SOMLINK * somTD_SOMUTId_somutHashId)(
			SOMUTId SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifndef SOMUTId_MajorVersion
		#define SOMUTId_MajorVersion   2
	#endif /* SOMUTId_MajorVersion */
	#ifndef SOMUTId_MinorVersion
		#define SOMUTId_MinorVersion   1
	#endif /* SOMUTId_MinorVersion */
	typedef struct SOMUTIdClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken somutSetIdId;
		somMToken somutEqualsId;
		somMToken somutCompareId;
		somMToken somutHashId;
	} SOMUTIdClassDataStructure;
	typedef struct SOMUTIdCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} SOMUTIdCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_somida_Source) || defined(SOMUTId_Class_Source)
			SOMEXTERN struct SOMUTIdClassDataStructure _SOMUTIdClassData;
			#ifndef SOMUTIdClassData
				#define SOMUTIdClassData    _SOMUTIdClassData
			#endif /* SOMUTIdClassData */
		#else
			SOMEXTERN struct SOMUTIdClassDataStructure * SOMLINK resolve_SOMUTIdClassData(void);
			#ifndef SOMUTIdClassData
				#define SOMUTIdClassData    (*(resolve_SOMUTIdClassData()))
			#endif /* SOMUTIdClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_somida_Source) || defined(SOMUTId_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_somida_Source || SOMUTId_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_somida_Source || SOMUTId_Class_Source */
		struct SOMUTIdClassDataStructure SOMDLINK SOMUTIdClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_somida_Source) || defined(SOMUTId_Class_Source)
			SOMEXTERN struct SOMUTIdCClassDataStructure _SOMUTIdCClassData;
			#ifndef SOMUTIdCClassData
				#define SOMUTIdCClassData    _SOMUTIdCClassData
			#endif /* SOMUTIdCClassData */
		#else
			SOMEXTERN struct SOMUTIdCClassDataStructure * SOMLINK resolve_SOMUTIdCClassData(void);
			#ifndef SOMUTIdCClassData
				#define SOMUTIdCClassData    (*(resolve_SOMUTIdCClassData()))
			#endif /* SOMUTIdCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_somida_Source) || defined(SOMUTId_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_somida_Source || SOMUTId_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_somida_Source || SOMUTId_Class_Source */
		struct SOMUTIdCClassDataStructure SOMDLINK SOMUTIdCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_somida_Source) || defined(SOMUTId_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_somida_Source || SOMUTId_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_somida_Source || SOMUTId_Class_Source */
	SOMClass SOMSTAR SOMLINK SOMUTIdNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_SOMUTId (SOMUTIdClassData.classObject)
	#ifndef SOMGD_SOMUTId
		#if (defined(_SOMUTId) || defined(__SOMUTId))
			#undef _SOMUTId
			#undef __SOMUTId
			#define SOMGD_SOMUTId 1
		#else
			#define _SOMUTId _SOMCLASS_SOMUTId
		#endif /* _SOMUTId */
	#endif /* SOMGD_SOMUTId */
	#define SOMUTId_classObj _SOMCLASS_SOMUTId
	#define _SOMMTOKEN_SOMUTId(method) ((somMToken)(SOMUTIdClassData.method))
	#ifndef SOMUTIdNew
		#define SOMUTIdNew() ( _SOMUTId ? \
			(SOMClass_somNew(_SOMUTId)) : \
			( SOMUTIdNewClass( \
				SOMUTId_MajorVersion, \
				SOMUTId_MinorVersion), \
			SOMClass_somNew(_SOMUTId))) 
	#endif /* NewSOMUTId */
	#ifndef SOMUTId_somutSetIdId
		#define SOMUTId_somutSetIdId(somSelf,ev,otherId) \
			SOM_Resolve(somSelf,SOMUTId,somutSetIdId)  \
				(somSelf,ev,otherId)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somutSetIdId
				#if defined(_somutSetIdId)
					#undef _somutSetIdId
					#define SOMGD__somutSetIdId
				#else
					#define _somutSetIdId SOMUTId_somutSetIdId
				#endif
			#endif /* SOMGD__somutSetIdId */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMUTId_somutSetIdId */
	#ifndef SOMUTId_somutEqualsId
		#define SOMUTId_somutEqualsId(somSelf,ev,otherId) \
			SOM_Resolve(somSelf,SOMUTId,somutEqualsId)  \
				(somSelf,ev,otherId)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somutEqualsId
				#if defined(_somutEqualsId)
					#undef _somutEqualsId
					#define SOMGD__somutEqualsId
				#else
					#define _somutEqualsId SOMUTId_somutEqualsId
				#endif
			#endif /* SOMGD__somutEqualsId */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMUTId_somutEqualsId */
	#ifndef SOMUTId_somutCompareId
		#define SOMUTId_somutCompareId(somSelf,ev,otherId) \
			SOM_Resolve(somSelf,SOMUTId,somutCompareId)  \
				(somSelf,ev,otherId)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somutCompareId
				#if defined(_somutCompareId)
					#undef _somutCompareId
					#define SOMGD__somutCompareId
				#else
					#define _somutCompareId SOMUTId_somutCompareId
				#endif
			#endif /* SOMGD__somutCompareId */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMUTId_somutCompareId */
	#ifndef SOMUTId_somutHashId
		#define SOMUTId_somutHashId(somSelf,ev) \
			SOM_Resolve(somSelf,SOMUTId,somutHashId)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somutHashId
				#if defined(_somutHashId)
					#undef _somutHashId
					#define SOMGD__somutHashId
				#else
					#define _somutHashId SOMUTId_somutHashId
				#endif
			#endif /* SOMGD__somutHashId */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMUTId_somutHashId */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define SOMUTId_somInit SOMObject_somInit
#define SOMUTId_somUninit SOMObject_somUninit
#define SOMUTId_somFree SOMObject_somFree
#define SOMUTId_somGetClass SOMObject_somGetClass
#define SOMUTId_somGetClassName SOMObject_somGetClassName
#define SOMUTId_somGetSize SOMObject_somGetSize
#define SOMUTId_somIsA SOMObject_somIsA
#define SOMUTId_somIsInstanceOf SOMObject_somIsInstanceOf
#define SOMUTId_somRespondsTo SOMObject_somRespondsTo
#define SOMUTId_somDispatch SOMObject_somDispatch
#define SOMUTId_somClassDispatch SOMObject_somClassDispatch
#define SOMUTId_somCastObj SOMObject_somCastObj
#define SOMUTId_somResetObj SOMObject_somResetObj
#define SOMUTId_somPrintSelf SOMObject_somPrintSelf
#define SOMUTId_somDumpSelf SOMObject_somDumpSelf
#define SOMUTId_somDumpSelfInt SOMObject_somDumpSelfInt
#define SOMUTId_somDefaultInit SOMObject_somDefaultInit
#define SOMUTId_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define SOMUTId_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define SOMUTId_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define SOMUTId_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define SOMUTId_somDefaultAssign SOMObject_somDefaultAssign
#define SOMUTId_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define SOMUTId_somDefaultVAssign SOMObject_somDefaultVAssign
#define SOMUTId_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define SOMUTId_somDestruct SOMObject_somDestruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_somida_Header_h */
