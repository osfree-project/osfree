/* generated from somsid.idl */
/* internal conditional is SOM_Module_somsid_Source */
#ifndef SOM_Module_somsid_Header_h
	#define SOM_Module_somsid_Header_h 1
	#include <som.h>
	#include <somida.h>
	#include <somobj.h>
	#ifndef _IDL_SOMUTStringId_defined
		#define _IDL_SOMUTStringId_defined
		typedef SOMObject SOMUTStringId;
	#endif /* _IDL_SOMUTStringId_defined */
	#ifndef _IDL_SEQUENCE_SOMUTStringId_defined
		#define _IDL_SEQUENCE_SOMUTStringId_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMUTStringId SOMSTAR ,sequence(SOMUTStringId));
	#endif /* _IDL_SEQUENCE_SOMUTStringId_defined */
	#ifdef __IBMC__
		typedef long (somTP_SOMUTStringId_somutSetIdString)(
			SOMUTStringId SOMSTAR somSelf,
			Environment *ev,
			/* in */ string IdString);
		#pragma linkage(somTP_SOMUTStringId_somutSetIdString,system)
		typedef somTP_SOMUTStringId_somutSetIdString *somTD_SOMUTStringId_somutSetIdString;
	#else /* __IBMC__ */
		typedef long (SOMLINK * somTD_SOMUTStringId_somutSetIdString)(
			SOMUTStringId SOMSTAR somSelf,
			Environment *ev,
			/* in */ string IdString);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_SOMUTStringId_somutGetIdString)(
			SOMUTStringId SOMSTAR somSelf,
			Environment *ev,
			/* in */ string toBuffer);
		#pragma linkage(somTP_SOMUTStringId_somutGetIdString,system)
		typedef somTP_SOMUTStringId_somutGetIdString *somTD_SOMUTStringId_somutGetIdString;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_SOMUTStringId_somutGetIdString)(
			SOMUTStringId SOMSTAR somSelf,
			Environment *ev,
			/* in */ string toBuffer);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef long (somTP_SOMUTStringId_somutGetIdStringLen)(
			SOMUTStringId SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_SOMUTStringId_somutGetIdStringLen,system)
		typedef somTP_SOMUTStringId_somutGetIdStringLen *somTD_SOMUTStringId_somutGetIdStringLen;
	#else /* __IBMC__ */
		typedef long (SOMLINK * somTD_SOMUTStringId_somutGetIdStringLen)(
			SOMUTStringId SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef boolean (somTP_SOMUTStringId_somutEqualsString)(
			SOMUTStringId SOMSTAR somSelf,
			Environment *ev,
			/* in */ string IdString);
		#pragma linkage(somTP_SOMUTStringId_somutEqualsString,system)
		typedef somTP_SOMUTStringId_somutEqualsString *somTD_SOMUTStringId_somutEqualsString;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_SOMUTStringId_somutEqualsString)(
			SOMUTStringId SOMSTAR somSelf,
			Environment *ev,
			/* in */ string IdString);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef short (somTP_SOMUTStringId_somutCompareString)(
			SOMUTStringId SOMSTAR somSelf,
			Environment *ev,
			/* in */ string IdString);
		#pragma linkage(somTP_SOMUTStringId_somutCompareString,system)
		typedef somTP_SOMUTStringId_somutCompareString *somTD_SOMUTStringId_somutCompareString;
	#else /* __IBMC__ */
		typedef short (SOMLINK * somTD_SOMUTStringId_somutCompareString)(
			SOMUTStringId SOMSTAR somSelf,
			Environment *ev,
			/* in */ string IdString);
	#endif /* __IBMC__ */
	#ifndef SOMUTStringId_MajorVersion
		#define SOMUTStringId_MajorVersion   2
	#endif /* SOMUTStringId_MajorVersion */
	#ifndef SOMUTStringId_MinorVersion
		#define SOMUTStringId_MinorVersion   1
	#endif /* SOMUTStringId_MinorVersion */
	typedef struct SOMUTStringIdClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken somutSetIdString;
		somMToken somutGetIdString;
		somMToken somutGetIdStringLen;
		somMToken somutEqualsString;
		somMToken somutCompareString;
	} SOMUTStringIdClassDataStructure;
	typedef struct SOMUTStringIdCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} SOMUTStringIdCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_somsid_Source) || defined(SOMUTStringId_Class_Source)
			SOMEXTERN struct SOMUTStringIdClassDataStructure _SOMUTStringIdClassData;
			#ifndef SOMUTStringIdClassData
				#define SOMUTStringIdClassData    _SOMUTStringIdClassData
			#endif /* SOMUTStringIdClassData */
		#else
			SOMEXTERN struct SOMUTStringIdClassDataStructure * SOMLINK resolve_SOMUTStringIdClassData(void);
			#ifndef SOMUTStringIdClassData
				#define SOMUTStringIdClassData    (*(resolve_SOMUTStringIdClassData()))
			#endif /* SOMUTStringIdClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_somsid_Source) || defined(SOMUTStringId_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_somsid_Source || SOMUTStringId_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_somsid_Source || SOMUTStringId_Class_Source */
		struct SOMUTStringIdClassDataStructure SOMDLINK SOMUTStringIdClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_somsid_Source) || defined(SOMUTStringId_Class_Source)
			SOMEXTERN struct SOMUTStringIdCClassDataStructure _SOMUTStringIdCClassData;
			#ifndef SOMUTStringIdCClassData
				#define SOMUTStringIdCClassData    _SOMUTStringIdCClassData
			#endif /* SOMUTStringIdCClassData */
		#else
			SOMEXTERN struct SOMUTStringIdCClassDataStructure * SOMLINK resolve_SOMUTStringIdCClassData(void);
			#ifndef SOMUTStringIdCClassData
				#define SOMUTStringIdCClassData    (*(resolve_SOMUTStringIdCClassData()))
			#endif /* SOMUTStringIdCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_somsid_Source) || defined(SOMUTStringId_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_somsid_Source || SOMUTStringId_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_somsid_Source || SOMUTStringId_Class_Source */
		struct SOMUTStringIdCClassDataStructure SOMDLINK SOMUTStringIdCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_somsid_Source) || defined(SOMUTStringId_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_somsid_Source || SOMUTStringId_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_somsid_Source || SOMUTStringId_Class_Source */
	SOMClass SOMSTAR SOMLINK SOMUTStringIdNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_SOMUTStringId (SOMUTStringIdClassData.classObject)
	#ifndef SOMGD_SOMUTStringId
		#if (defined(_SOMUTStringId) || defined(__SOMUTStringId))
			#undef _SOMUTStringId
			#undef __SOMUTStringId
			#define SOMGD_SOMUTStringId 1
		#else
			#define _SOMUTStringId _SOMCLASS_SOMUTStringId
		#endif /* _SOMUTStringId */
	#endif /* SOMGD_SOMUTStringId */
	#define SOMUTStringId_classObj _SOMCLASS_SOMUTStringId
	#define _SOMMTOKEN_SOMUTStringId(method) ((somMToken)(SOMUTStringIdClassData.method))
	#ifndef SOMUTStringIdNew
		#define SOMUTStringIdNew() ( _SOMUTStringId ? \
			(SOMClass_somNew(_SOMUTStringId)) : \
			( SOMUTStringIdNewClass( \
				SOMUTStringId_MajorVersion, \
				SOMUTStringId_MinorVersion), \
			SOMClass_somNew(_SOMUTStringId))) 
	#endif /* NewSOMUTStringId */
	#ifndef SOMUTStringId_somutSetIdString
		#define SOMUTStringId_somutSetIdString(somSelf,ev,IdString) \
			SOM_Resolve(somSelf,SOMUTStringId,somutSetIdString)  \
				(somSelf,ev,IdString)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somutSetIdString
				#if defined(_somutSetIdString)
					#undef _somutSetIdString
					#define SOMGD__somutSetIdString
				#else
					#define _somutSetIdString SOMUTStringId_somutSetIdString
				#endif
			#endif /* SOMGD__somutSetIdString */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMUTStringId_somutSetIdString */
	#ifndef SOMUTStringId_somutGetIdString
		#define SOMUTStringId_somutGetIdString(somSelf,ev,toBuffer) \
			SOM_Resolve(somSelf,SOMUTStringId,somutGetIdString)  \
				(somSelf,ev,toBuffer)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somutGetIdString
				#if defined(_somutGetIdString)
					#undef _somutGetIdString
					#define SOMGD__somutGetIdString
				#else
					#define _somutGetIdString SOMUTStringId_somutGetIdString
				#endif
			#endif /* SOMGD__somutGetIdString */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMUTStringId_somutGetIdString */
	#ifndef SOMUTStringId_somutGetIdStringLen
		#define SOMUTStringId_somutGetIdStringLen(somSelf,ev) \
			SOM_Resolve(somSelf,SOMUTStringId,somutGetIdStringLen)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somutGetIdStringLen
				#if defined(_somutGetIdStringLen)
					#undef _somutGetIdStringLen
					#define SOMGD__somutGetIdStringLen
				#else
					#define _somutGetIdStringLen SOMUTStringId_somutGetIdStringLen
				#endif
			#endif /* SOMGD__somutGetIdStringLen */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMUTStringId_somutGetIdStringLen */
	#ifndef SOMUTStringId_somutEqualsString
		#define SOMUTStringId_somutEqualsString(somSelf,ev,IdString) \
			SOM_Resolve(somSelf,SOMUTStringId,somutEqualsString)  \
				(somSelf,ev,IdString)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somutEqualsString
				#if defined(_somutEqualsString)
					#undef _somutEqualsString
					#define SOMGD__somutEqualsString
				#else
					#define _somutEqualsString SOMUTStringId_somutEqualsString
				#endif
			#endif /* SOMGD__somutEqualsString */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMUTStringId_somutEqualsString */
	#ifndef SOMUTStringId_somutCompareString
		#define SOMUTStringId_somutCompareString(somSelf,ev,IdString) \
			SOM_Resolve(somSelf,SOMUTStringId,somutCompareString)  \
				(somSelf,ev,IdString)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somutCompareString
				#if defined(_somutCompareString)
					#undef _somutCompareString
					#define SOMGD__somutCompareString
				#else
					#define _somutCompareString SOMUTStringId_somutCompareString
				#endif
			#endif /* SOMGD__somutCompareString */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMUTStringId_somutCompareString */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define SOMUTStringId_somInit SOMObject_somInit
#define SOMUTStringId_somUninit SOMObject_somUninit
#define SOMUTStringId_somFree SOMObject_somFree
#define SOMUTStringId_somGetClass SOMObject_somGetClass
#define SOMUTStringId_somGetClassName SOMObject_somGetClassName
#define SOMUTStringId_somGetSize SOMObject_somGetSize
#define SOMUTStringId_somIsA SOMObject_somIsA
#define SOMUTStringId_somIsInstanceOf SOMObject_somIsInstanceOf
#define SOMUTStringId_somRespondsTo SOMObject_somRespondsTo
#define SOMUTStringId_somDispatch SOMObject_somDispatch
#define SOMUTStringId_somClassDispatch SOMObject_somClassDispatch
#define SOMUTStringId_somCastObj SOMObject_somCastObj
#define SOMUTStringId_somResetObj SOMObject_somResetObj
#define SOMUTStringId_somPrintSelf SOMObject_somPrintSelf
#define SOMUTStringId_somDumpSelf SOMObject_somDumpSelf
#define SOMUTStringId_somDumpSelfInt SOMObject_somDumpSelfInt
#define SOMUTStringId_somDefaultInit SOMObject_somDefaultInit
#define SOMUTStringId_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define SOMUTStringId_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define SOMUTStringId_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define SOMUTStringId_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define SOMUTStringId_somDefaultAssign SOMObject_somDefaultAssign
#define SOMUTStringId_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define SOMUTStringId_somDefaultVAssign SOMObject_somDefaultVAssign
#define SOMUTStringId_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define SOMUTStringId_somDestruct SOMObject_somDestruct
#define SOMUTStringId_somutSetIdId SOMUTId_somutSetIdId
#define SOMUTStringId_somutEqualsId SOMUTId_somutEqualsId
#define SOMUTStringId_somutCompareId SOMUTId_somutCompareId
#define SOMUTStringId_somutHashId SOMUTId_somutHashId
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_somsid_Header_h */
