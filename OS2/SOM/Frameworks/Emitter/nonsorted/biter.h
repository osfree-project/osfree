/* generated from biter.idl */
/* internal conditional is SOM_Module_biter_Source */
#ifndef SOM_Module_biter_Header_h
	#define SOM_Module_biter_Header_h 1
	#include <som.h>
	#include <naming.h>
	#include <somobj.h>
		#ifndef _IDL_Biter_BINDITER_TWO_defined
			#define _IDL_Biter_BINDITER_TWO_defined
			typedef SOMObject Biter_BINDITER_TWO;
		#endif /* _IDL_Biter_BINDITER_TWO_defined */
		#ifndef _IDL_SEQUENCE_Biter_BINDITER_TWO_defined
			#define _IDL_SEQUENCE_Biter_BINDITER_TWO_defined
			SOM_SEQUENCE_TYPEDEF_NAME(Biter_BINDITER_TWO SOMSTAR ,sequence(Biter_BINDITER_TWO));
		#endif /* _IDL_SEQUENCE_Biter_BINDITER_TWO_defined */
		#ifndef _IDL_Biter_BINDITER_ONE_defined
			#define _IDL_Biter_BINDITER_ONE_defined
			typedef SOMObject Biter_BINDITER_ONE;
		#endif /* _IDL_Biter_BINDITER_ONE_defined */
		#ifndef _IDL_SEQUENCE_Biter_BINDITER_ONE_defined
			#define _IDL_SEQUENCE_Biter_BINDITER_ONE_defined
			SOM_SEQUENCE_TYPEDEF_NAME(Biter_BINDITER_ONE SOMSTAR ,sequence(Biter_BINDITER_ONE));
		#endif /* _IDL_SEQUENCE_Biter_BINDITER_ONE_defined */
		#ifdef __IBMC__
			typedef void (somTP_Biter_BINDITER_TWO_BI_Init2)(
				Biter_BINDITER_TWO SOMSTAR somSelf,
				Environment *ev,
				/* inout */ somToken *ctrl,
				/* in */ CosNaming_BindingList *bl);
			#pragma linkage(somTP_Biter_BINDITER_TWO_BI_Init2,system)
			typedef somTP_Biter_BINDITER_TWO_BI_Init2 *somTD_Biter_BINDITER_TWO_BI_Init2;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_Biter_BINDITER_TWO_BI_Init2)(
				Biter_BINDITER_TWO SOMSTAR somSelf,
				Environment *ev,
				/* inout */ somToken *ctrl,
				/* in */ CosNaming_BindingList *bl);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_Biter_BINDITER_TWO_add_bindings)(
				Biter_BINDITER_TWO SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_BindingList *bl);
			#pragma linkage(somTP_Biter_BINDITER_TWO_add_bindings,system)
			typedef somTP_Biter_BINDITER_TWO_add_bindings *somTD_Biter_BINDITER_TWO_add_bindings;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_Biter_BINDITER_TWO_add_bindings)(
				Biter_BINDITER_TWO SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_BindingList *bl);
		#endif /* __IBMC__ */
		#ifndef Biter_BINDITER_TWO_MajorVersion
			#define Biter_BINDITER_TWO_MajorVersion   3
		#endif /* Biter_BINDITER_TWO_MajorVersion */
		#ifndef Biter_BINDITER_TWO_MinorVersion
			#define Biter_BINDITER_TWO_MinorVersion   0
		#endif /* Biter_BINDITER_TWO_MinorVersion */
		typedef struct Biter_BINDITER_TWOClassDataStructure
		{
			SOMClass SOMSTAR classObject;
			somMToken BI_Init2;
			somMToken add_bindings;
		} Biter_BINDITER_TWOClassDataStructure;
		typedef struct Biter_BINDITER_TWOCClassDataStructure
		{
			somMethodTabs parentMtab;
			somDToken instanceDataToken;
		} Biter_BINDITER_TWOCClassDataStructure;
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_biter_Source) || defined(Biter_BINDITER_TWO_Class_Source)
				SOMEXTERN struct Biter_BINDITER_TWOClassDataStructure _Biter_BINDITER_TWOClassData;
				#ifndef Biter_BINDITER_TWOClassData
					#define Biter_BINDITER_TWOClassData    _Biter_BINDITER_TWOClassData
				#endif /* Biter_BINDITER_TWOClassData */
			#else
				SOMEXTERN struct Biter_BINDITER_TWOClassDataStructure * SOMLINK resolve_Biter_BINDITER_TWOClassData(void);
				#ifndef Biter_BINDITER_TWOClassData
					#define Biter_BINDITER_TWOClassData    (*(resolve_Biter_BINDITER_TWOClassData()))
				#endif /* Biter_BINDITER_TWOClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_biter_Source) || defined(Biter_BINDITER_TWO_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_biter_Source || Biter_BINDITER_TWO_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_biter_Source || Biter_BINDITER_TWO_Class_Source */
			struct Biter_BINDITER_TWOClassDataStructure SOMDLINK Biter_BINDITER_TWOClassData;
		#endif /* SOM_RESOLVE_DATA */
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_biter_Source) || defined(Biter_BINDITER_TWO_Class_Source)
				SOMEXTERN struct Biter_BINDITER_TWOCClassDataStructure _Biter_BINDITER_TWOCClassData;
				#ifndef Biter_BINDITER_TWOCClassData
					#define Biter_BINDITER_TWOCClassData    _Biter_BINDITER_TWOCClassData
				#endif /* Biter_BINDITER_TWOCClassData */
			#else
				SOMEXTERN struct Biter_BINDITER_TWOCClassDataStructure * SOMLINK resolve_Biter_BINDITER_TWOCClassData(void);
				#ifndef Biter_BINDITER_TWOCClassData
					#define Biter_BINDITER_TWOCClassData    (*(resolve_Biter_BINDITER_TWOCClassData()))
				#endif /* Biter_BINDITER_TWOCClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_biter_Source) || defined(Biter_BINDITER_TWO_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_biter_Source || Biter_BINDITER_TWO_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_biter_Source || Biter_BINDITER_TWO_Class_Source */
			struct Biter_BINDITER_TWOCClassDataStructure SOMDLINK Biter_BINDITER_TWOCClassData;
		#endif /* SOM_RESOLVE_DATA */
		extern 
		#if defined(SOM_Module_biter_Source) || defined(Biter_BINDITER_TWO_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_biter_Source || Biter_BINDITER_TWO_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_biter_Source || Biter_BINDITER_TWO_Class_Source */
		SOMClass SOMSTAR SOMLINK Biter_BINDITER_TWONewClass
				(integer4 somtmajorVersion,integer4 somtminorVersion);
		#define _SOMCLASS_Biter_BINDITER_TWO (Biter_BINDITER_TWOClassData.classObject)
		#ifndef SOMGD_Biter_BINDITER_TWO
			#if (defined(_Biter_BINDITER_TWO) || defined(__Biter_BINDITER_TWO))
				#undef _Biter_BINDITER_TWO
				#undef __Biter_BINDITER_TWO
				#define SOMGD_Biter_BINDITER_TWO 1
			#else
				#define _Biter_BINDITER_TWO _SOMCLASS_Biter_BINDITER_TWO
			#endif /* _Biter_BINDITER_TWO */
		#endif /* SOMGD_Biter_BINDITER_TWO */
		#define Biter_BINDITER_TWO_classObj _SOMCLASS_Biter_BINDITER_TWO
		#define _SOMMTOKEN_Biter_BINDITER_TWO(method) ((somMToken)(Biter_BINDITER_TWOClassData.method))
		#ifndef Biter_BINDITER_TWONew
			#define Biter_BINDITER_TWONew() ( _Biter_BINDITER_TWO ? \
				(SOMClass_somNew(_Biter_BINDITER_TWO)) : \
				( Biter_BINDITER_TWONewClass( \
					Biter_BINDITER_TWO_MajorVersion, \
					Biter_BINDITER_TWO_MinorVersion), \
				SOMClass_somNew(_Biter_BINDITER_TWO))) 
		#endif /* NewBiter_BINDITER_TWO */
		#ifndef Biter_BINDITER_TWO_BI_Init2
			#define Biter_BINDITER_TWO_BI_Init2(somSelf,ev,ctrl,bl) \
				SOM_Resolve(somSelf,Biter_BINDITER_TWO,BI_Init2)  \
					(somSelf,ev,ctrl,bl)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__BI_Init2
					#if defined(_BI_Init2)
						#undef _BI_Init2
						#define SOMGD__BI_Init2
					#else
						#define _BI_Init2 Biter_BINDITER_TWO_BI_Init2
					#endif
				#endif /* SOMGD__BI_Init2 */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* Biter_BINDITER_TWO_BI_Init2 */
		#ifndef Biter_BINDITER_TWO_add_bindings
			#define Biter_BINDITER_TWO_add_bindings(somSelf,ev,bl) \
				SOM_Resolve(somSelf,Biter_BINDITER_TWO,add_bindings)  \
					(somSelf,ev,bl)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__add_bindings
					#if defined(_add_bindings)
						#undef _add_bindings
						#define SOMGD__add_bindings
					#else
						#define _add_bindings Biter_BINDITER_TWO_add_bindings
					#endif
				#endif /* SOMGD__add_bindings */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* Biter_BINDITER_TWO_add_bindings */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define Biter_BINDITER_TWO_somInit SOMObject_somInit
#define Biter_BINDITER_TWO_somUninit SOMObject_somUninit
#define Biter_BINDITER_TWO_somFree SOMObject_somFree
#define Biter_BINDITER_TWO_somGetClass SOMObject_somGetClass
#define Biter_BINDITER_TWO_somGetClassName SOMObject_somGetClassName
#define Biter_BINDITER_TWO_somGetSize SOMObject_somGetSize
#define Biter_BINDITER_TWO_somIsA SOMObject_somIsA
#define Biter_BINDITER_TWO_somIsInstanceOf SOMObject_somIsInstanceOf
#define Biter_BINDITER_TWO_somRespondsTo SOMObject_somRespondsTo
#define Biter_BINDITER_TWO_somDispatch SOMObject_somDispatch
#define Biter_BINDITER_TWO_somClassDispatch SOMObject_somClassDispatch
#define Biter_BINDITER_TWO_somCastObj SOMObject_somCastObj
#define Biter_BINDITER_TWO_somResetObj SOMObject_somResetObj
#define Biter_BINDITER_TWO_somPrintSelf SOMObject_somPrintSelf
#define Biter_BINDITER_TWO_somDumpSelf SOMObject_somDumpSelf
#define Biter_BINDITER_TWO_somDumpSelfInt SOMObject_somDumpSelfInt
#define Biter_BINDITER_TWO_somDefaultInit SOMObject_somDefaultInit
#define Biter_BINDITER_TWO_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define Biter_BINDITER_TWO_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define Biter_BINDITER_TWO_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define Biter_BINDITER_TWO_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define Biter_BINDITER_TWO_somDefaultAssign SOMObject_somDefaultAssign
#define Biter_BINDITER_TWO_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define Biter_BINDITER_TWO_somDefaultVAssign SOMObject_somDefaultVAssign
#define Biter_BINDITER_TWO_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define Biter_BINDITER_TWO_somDestruct SOMObject_somDestruct
#define Biter_BINDITER_TWO_next_one CosNaming_BindingIterator_next_one
#define Biter_BINDITER_TWO_next_n CosNaming_BindingIterator_next_n
#define Biter_BINDITER_TWO_destroy CosNaming_BindingIterator_destroy
#endif /* SOM_DONT_USE_INHERITED_MACROS */

		#ifndef _IDL_SEQUENCE_string_defined
			#define _IDL_SEQUENCE_string_defined
			SOM_SEQUENCE_TYPEDEF(string);
		#endif /* _IDL_SEQUENCE_string_defined */
		typedef _IDL_SEQUENCE_string Biter_BINDITER_ONE_stringSeq;
		#ifndef _IDL_SEQUENCE_Biter_BINDITER_ONE_stringSeq_defined
			#define _IDL_SEQUENCE_Biter_BINDITER_ONE_stringSeq_defined
			SOM_SEQUENCE_TYPEDEF(Biter_BINDITER_ONE_stringSeq);
		#endif /* _IDL_SEQUENCE_Biter_BINDITER_ONE_stringSeq_defined */
		#ifdef __IBMC__
			typedef void (somTP_Biter_BINDITER_ONE_BI_Init1)(
				Biter_BINDITER_ONE SOMSTAR somSelf,
				Environment *ev,
				/* inout */ somToken *ctrl,
				/* in */ Biter_BINDITER_ONE_stringSeq *key,
				/* in */ string databaseFile);
			#pragma linkage(somTP_Biter_BINDITER_ONE_BI_Init1,system)
			typedef somTP_Biter_BINDITER_ONE_BI_Init1 *somTD_Biter_BINDITER_ONE_BI_Init1;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_Biter_BINDITER_ONE_BI_Init1)(
				Biter_BINDITER_ONE SOMSTAR somSelf,
				Environment *ev,
				/* inout */ somToken *ctrl,
				/* in */ Biter_BINDITER_ONE_stringSeq *key,
				/* in */ string databaseFile);
		#endif /* __IBMC__ */
		#ifndef Biter_BINDITER_ONE_MajorVersion
			#define Biter_BINDITER_ONE_MajorVersion   0
		#endif /* Biter_BINDITER_ONE_MajorVersion */
		#ifndef Biter_BINDITER_ONE_MinorVersion
			#define Biter_BINDITER_ONE_MinorVersion   0
		#endif /* Biter_BINDITER_ONE_MinorVersion */
		typedef struct Biter_BINDITER_ONEClassDataStructure
		{
			SOMClass SOMSTAR classObject;
			somMToken BI_Init1;
		} Biter_BINDITER_ONEClassDataStructure;
		typedef struct Biter_BINDITER_ONECClassDataStructure
		{
			somMethodTabs parentMtab;
			somDToken instanceDataToken;
		} Biter_BINDITER_ONECClassDataStructure;
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_biter_Source) || defined(Biter_BINDITER_ONE_Class_Source)
				SOMEXTERN struct Biter_BINDITER_ONEClassDataStructure _Biter_BINDITER_ONEClassData;
				#ifndef Biter_BINDITER_ONEClassData
					#define Biter_BINDITER_ONEClassData    _Biter_BINDITER_ONEClassData
				#endif /* Biter_BINDITER_ONEClassData */
			#else
				SOMEXTERN struct Biter_BINDITER_ONEClassDataStructure * SOMLINK resolve_Biter_BINDITER_ONEClassData(void);
				#ifndef Biter_BINDITER_ONEClassData
					#define Biter_BINDITER_ONEClassData    (*(resolve_Biter_BINDITER_ONEClassData()))
				#endif /* Biter_BINDITER_ONEClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_biter_Source) || defined(Biter_BINDITER_ONE_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_biter_Source || Biter_BINDITER_ONE_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_biter_Source || Biter_BINDITER_ONE_Class_Source */
			struct Biter_BINDITER_ONEClassDataStructure SOMDLINK Biter_BINDITER_ONEClassData;
		#endif /* SOM_RESOLVE_DATA */
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_biter_Source) || defined(Biter_BINDITER_ONE_Class_Source)
				SOMEXTERN struct Biter_BINDITER_ONECClassDataStructure _Biter_BINDITER_ONECClassData;
				#ifndef Biter_BINDITER_ONECClassData
					#define Biter_BINDITER_ONECClassData    _Biter_BINDITER_ONECClassData
				#endif /* Biter_BINDITER_ONECClassData */
			#else
				SOMEXTERN struct Biter_BINDITER_ONECClassDataStructure * SOMLINK resolve_Biter_BINDITER_ONECClassData(void);
				#ifndef Biter_BINDITER_ONECClassData
					#define Biter_BINDITER_ONECClassData    (*(resolve_Biter_BINDITER_ONECClassData()))
				#endif /* Biter_BINDITER_ONECClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_biter_Source) || defined(Biter_BINDITER_ONE_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_biter_Source || Biter_BINDITER_ONE_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_biter_Source || Biter_BINDITER_ONE_Class_Source */
			struct Biter_BINDITER_ONECClassDataStructure SOMDLINK Biter_BINDITER_ONECClassData;
		#endif /* SOM_RESOLVE_DATA */
		extern 
		#if defined(SOM_Module_biter_Source) || defined(Biter_BINDITER_ONE_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_biter_Source || Biter_BINDITER_ONE_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_biter_Source || Biter_BINDITER_ONE_Class_Source */
		SOMClass SOMSTAR SOMLINK Biter_BINDITER_ONENewClass
				(integer4 somtmajorVersion,integer4 somtminorVersion);
		#define _SOMCLASS_Biter_BINDITER_ONE (Biter_BINDITER_ONEClassData.classObject)
		#ifndef SOMGD_Biter_BINDITER_ONE
			#if (defined(_Biter_BINDITER_ONE) || defined(__Biter_BINDITER_ONE))
				#undef _Biter_BINDITER_ONE
				#undef __Biter_BINDITER_ONE
				#define SOMGD_Biter_BINDITER_ONE 1
			#else
				#define _Biter_BINDITER_ONE _SOMCLASS_Biter_BINDITER_ONE
			#endif /* _Biter_BINDITER_ONE */
		#endif /* SOMGD_Biter_BINDITER_ONE */
		#define Biter_BINDITER_ONE_classObj _SOMCLASS_Biter_BINDITER_ONE
		#define _SOMMTOKEN_Biter_BINDITER_ONE(method) ((somMToken)(Biter_BINDITER_ONEClassData.method))
		#ifndef Biter_BINDITER_ONENew
			#define Biter_BINDITER_ONENew() ( _Biter_BINDITER_ONE ? \
				(SOMClass_somNew(_Biter_BINDITER_ONE)) : \
				( Biter_BINDITER_ONENewClass( \
					Biter_BINDITER_ONE_MajorVersion, \
					Biter_BINDITER_ONE_MinorVersion), \
				SOMClass_somNew(_Biter_BINDITER_ONE))) 
		#endif /* NewBiter_BINDITER_ONE */
		#ifndef Biter_BINDITER_ONE_BI_Init1
			#define Biter_BINDITER_ONE_BI_Init1(somSelf,ev,ctrl,key,databaseFile) \
				SOM_Resolve(somSelf,Biter_BINDITER_ONE,BI_Init1)  \
					(somSelf,ev,ctrl,key,databaseFile)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__BI_Init1
					#if defined(_BI_Init1)
						#undef _BI_Init1
						#define SOMGD__BI_Init1
					#else
						#define _BI_Init1 Biter_BINDITER_ONE_BI_Init1
					#endif
				#endif /* SOMGD__BI_Init1 */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* Biter_BINDITER_ONE_BI_Init1 */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define Biter_BINDITER_ONE_somInit SOMObject_somInit
#define Biter_BINDITER_ONE_somUninit SOMObject_somUninit
#define Biter_BINDITER_ONE_somFree SOMObject_somFree
#define Biter_BINDITER_ONE_somGetClass SOMObject_somGetClass
#define Biter_BINDITER_ONE_somGetClassName SOMObject_somGetClassName
#define Biter_BINDITER_ONE_somGetSize SOMObject_somGetSize
#define Biter_BINDITER_ONE_somIsA SOMObject_somIsA
#define Biter_BINDITER_ONE_somIsInstanceOf SOMObject_somIsInstanceOf
#define Biter_BINDITER_ONE_somRespondsTo SOMObject_somRespondsTo
#define Biter_BINDITER_ONE_somDispatch SOMObject_somDispatch
#define Biter_BINDITER_ONE_somClassDispatch SOMObject_somClassDispatch
#define Biter_BINDITER_ONE_somCastObj SOMObject_somCastObj
#define Biter_BINDITER_ONE_somResetObj SOMObject_somResetObj
#define Biter_BINDITER_ONE_somPrintSelf SOMObject_somPrintSelf
#define Biter_BINDITER_ONE_somDumpSelf SOMObject_somDumpSelf
#define Biter_BINDITER_ONE_somDumpSelfInt SOMObject_somDumpSelfInt
#define Biter_BINDITER_ONE_somDefaultInit SOMObject_somDefaultInit
#define Biter_BINDITER_ONE_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define Biter_BINDITER_ONE_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define Biter_BINDITER_ONE_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define Biter_BINDITER_ONE_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define Biter_BINDITER_ONE_somDefaultAssign SOMObject_somDefaultAssign
#define Biter_BINDITER_ONE_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define Biter_BINDITER_ONE_somDefaultVAssign SOMObject_somDefaultVAssign
#define Biter_BINDITER_ONE_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define Biter_BINDITER_ONE_somDestruct SOMObject_somDestruct
#define Biter_BINDITER_ONE_next_one CosNaming_BindingIterator_next_one
#define Biter_BINDITER_ONE_next_n CosNaming_BindingIterator_next_n
#define Biter_BINDITER_ONE_destroy CosNaming_BindingIterator_destroy
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_biter_Header_h */
