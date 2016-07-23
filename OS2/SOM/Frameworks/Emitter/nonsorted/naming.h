/* generated from naming.idl */
/* internal conditional is SOM_Module_naming_Source */
#ifndef SOM_Module_naming_Header_h
	#define SOM_Module_naming_Header_h 1
	#include <som.h>
	#include <somobj.h>
		#ifndef _IDL_CosNaming_BindingIterator_defined
			#define _IDL_CosNaming_BindingIterator_defined
			typedef SOMObject CosNaming_BindingIterator;
		#endif /* _IDL_CosNaming_BindingIterator_defined */
		#ifndef _IDL_SEQUENCE_CosNaming_BindingIterator_defined
			#define _IDL_SEQUENCE_CosNaming_BindingIterator_defined
			SOM_SEQUENCE_TYPEDEF_NAME(CosNaming_BindingIterator SOMSTAR ,sequence(CosNaming_BindingIterator));
		#endif /* _IDL_SEQUENCE_CosNaming_BindingIterator_defined */
		#ifndef _IDL_CosNaming_NamingContext_defined
			#define _IDL_CosNaming_NamingContext_defined
			typedef SOMObject CosNaming_NamingContext;
		#endif /* _IDL_CosNaming_NamingContext_defined */
		#ifndef _IDL_SEQUENCE_CosNaming_NamingContext_defined
			#define _IDL_SEQUENCE_CosNaming_NamingContext_defined
			SOM_SEQUENCE_TYPEDEF_NAME(CosNaming_NamingContext SOMSTAR ,sequence(CosNaming_NamingContext));
		#endif /* _IDL_SEQUENCE_CosNaming_NamingContext_defined */
		#ifndef _IDL_CosNaming_NamingContextExt_defined
			#define _IDL_CosNaming_NamingContextExt_defined
			typedef SOMObject CosNaming_NamingContextExt;
		#endif /* _IDL_CosNaming_NamingContextExt_defined */
		#ifndef _IDL_SEQUENCE_CosNaming_NamingContextExt_defined
			#define _IDL_SEQUENCE_CosNaming_NamingContextExt_defined
			SOM_SEQUENCE_TYPEDEF_NAME(CosNaming_NamingContextExt SOMSTAR ,sequence(CosNaming_NamingContextExt));
		#endif /* _IDL_SEQUENCE_CosNaming_NamingContextExt_defined */
		typedef string CosNaming_Istring;
		#ifndef _IDL_SEQUENCE_string_defined
			#define _IDL_SEQUENCE_string_defined
			SOM_SEQUENCE_TYPEDEF(string);
		#endif /* _IDL_SEQUENCE_string_defined */
		typedef struct CosNaming_NameComponent
		{
			CosNaming_Istring id;
			CosNaming_Istring kind;
		} CosNaming_NameComponent;
		#ifndef _IDL_SEQUENCE_CosNaming_NameComponent_defined
			#define _IDL_SEQUENCE_CosNaming_NameComponent_defined
			SOM_SEQUENCE_TYPEDEF(CosNaming_NameComponent);
		#endif /* _IDL_SEQUENCE_CosNaming_NameComponent_defined */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD_NameComponent
				#if defined(NameComponent)
					#undef NameComponent
					#define SOMGD_NameComponent
				#else
					#define NameComponent CosNaming_NameComponent
				#endif
			#endif /* SOMGD_NameComponent */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef _IDL_SEQUENCE_NameComponent_defined
				#define _IDL_SEQUENCE_NameComponent_defined
				#define _IDL_SEQUENCE_NameComponent _IDL_SEQUENCE_CosNaming_NameComponent
			#endif /* _IDL_SEQUENCE_NameComponent_defined */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		typedef _IDL_SEQUENCE_CosNaming_NameComponent CosNaming_Name;
		#ifndef _IDL_SEQUENCE_CosNaming_Name_defined
			#define _IDL_SEQUENCE_CosNaming_Name_defined
			SOM_SEQUENCE_TYPEDEF(CosNaming_Name);
		#endif /* _IDL_SEQUENCE_CosNaming_Name_defined */
		#ifndef CosNaming_BindingType_defined
			#define CosNaming_BindingType_defined
			typedef unsigned long CosNaming_BindingType;
			#ifndef CosNaming_nobject
				#define CosNaming_nobject 1UL
			#endif /* CosNaming_nobject */
			#ifndef CosNaming_ncontext
				#define CosNaming_ncontext 2UL
			#endif /* CosNaming_ncontext */
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD_BindingType
					#if defined(BindingType)
						#undef BindingType
						#define SOMGD_BindingType
					#else
						#define BindingType CosNaming_BindingType
					#endif
				#endif /* SOMGD_BindingType */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosNaming_BindingType_defined */
		typedef struct CosNaming_Binding
		{
			CosNaming_Name binding_name;
			CosNaming_BindingType binding_type;
		} CosNaming_Binding;
		#ifndef _IDL_SEQUENCE_CosNaming_Binding_defined
			#define _IDL_SEQUENCE_CosNaming_Binding_defined
			SOM_SEQUENCE_TYPEDEF(CosNaming_Binding);
		#endif /* _IDL_SEQUENCE_CosNaming_Binding_defined */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD_Binding
				#if defined(Binding)
					#undef Binding
					#define SOMGD_Binding
				#else
					#define Binding CosNaming_Binding
				#endif
			#endif /* SOMGD_Binding */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef _IDL_SEQUENCE_Binding_defined
				#define _IDL_SEQUENCE_Binding_defined
				#define _IDL_SEQUENCE_Binding _IDL_SEQUENCE_CosNaming_Binding
			#endif /* _IDL_SEQUENCE_Binding_defined */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		typedef _IDL_SEQUENCE_CosNaming_Binding CosNaming_BindingList;
		#ifndef _IDL_SEQUENCE_CosNaming_BindingList_defined
			#define _IDL_SEQUENCE_CosNaming_BindingList_defined
			SOM_SEQUENCE_TYPEDEF(CosNaming_BindingList);
		#endif /* _IDL_SEQUENCE_CosNaming_BindingList_defined */
		#ifdef __IBMC__
			typedef boolean (somTP_CosNaming_BindingIterator_next_one)(
				CosNaming_BindingIterator SOMSTAR somSelf,
				Environment *ev,
				/* out */ CosNaming_Binding *b);
			#pragma linkage(somTP_CosNaming_BindingIterator_next_one,system)
			typedef somTP_CosNaming_BindingIterator_next_one *somTD_CosNaming_BindingIterator_next_one;
		#else /* __IBMC__ */
			typedef boolean (SOMLINK * somTD_CosNaming_BindingIterator_next_one)(
				CosNaming_BindingIterator SOMSTAR somSelf,
				Environment *ev,
				/* out */ CosNaming_Binding *b);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef boolean (somTP_CosNaming_BindingIterator_next_n)(
				CosNaming_BindingIterator SOMSTAR somSelf,
				Environment *ev,
				/* in */ unsigned long how_many,
				/* out */ CosNaming_BindingList *bl);
			#pragma linkage(somTP_CosNaming_BindingIterator_next_n,system)
			typedef somTP_CosNaming_BindingIterator_next_n *somTD_CosNaming_BindingIterator_next_n;
		#else /* __IBMC__ */
			typedef boolean (SOMLINK * somTD_CosNaming_BindingIterator_next_n)(
				CosNaming_BindingIterator SOMSTAR somSelf,
				Environment *ev,
				/* in */ unsigned long how_many,
				/* out */ CosNaming_BindingList *bl);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_CosNaming_BindingIterator_destroy)(
				CosNaming_BindingIterator SOMSTAR somSelf,
				Environment *ev);
			#pragma linkage(somTP_CosNaming_BindingIterator_destroy,system)
			typedef somTP_CosNaming_BindingIterator_destroy *somTD_CosNaming_BindingIterator_destroy;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_CosNaming_BindingIterator_destroy)(
				CosNaming_BindingIterator SOMSTAR somSelf,
				Environment *ev);
		#endif /* __IBMC__ */
		#ifndef CosNaming_BindingIterator_MajorVersion
			#define CosNaming_BindingIterator_MajorVersion   3
		#endif /* CosNaming_BindingIterator_MajorVersion */
		#ifndef CosNaming_BindingIterator_MinorVersion
			#define CosNaming_BindingIterator_MinorVersion   0
		#endif /* CosNaming_BindingIterator_MinorVersion */
		typedef struct CosNaming_BindingIteratorClassDataStructure
		{
			SOMClass SOMSTAR classObject;
			somMToken next_one;
			somMToken next_n;
			somMToken destroy;
		} CosNaming_BindingIteratorClassDataStructure;
		typedef struct CosNaming_BindingIteratorCClassDataStructure
		{
			somMethodTabs parentMtab;
			somDToken instanceDataToken;
		} CosNaming_BindingIteratorCClassDataStructure;
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_naming_Source) || defined(CosNaming_BindingIterator_Class_Source)
				SOMEXTERN struct CosNaming_BindingIteratorClassDataStructure _CosNaming_BindingIteratorClassData;
				#ifndef CosNaming_BindingIteratorClassData
					#define CosNaming_BindingIteratorClassData    _CosNaming_BindingIteratorClassData
				#endif /* CosNaming_BindingIteratorClassData */
			#else
				SOMEXTERN struct CosNaming_BindingIteratorClassDataStructure * SOMLINK resolve_CosNaming_BindingIteratorClassData(void);
				#ifndef CosNaming_BindingIteratorClassData
					#define CosNaming_BindingIteratorClassData    (*(resolve_CosNaming_BindingIteratorClassData()))
				#endif /* CosNaming_BindingIteratorClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_naming_Source) || defined(CosNaming_BindingIterator_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_naming_Source || CosNaming_BindingIterator_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_naming_Source || CosNaming_BindingIterator_Class_Source */
			struct CosNaming_BindingIteratorClassDataStructure SOMDLINK CosNaming_BindingIteratorClassData;
		#endif /* SOM_RESOLVE_DATA */
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_naming_Source) || defined(CosNaming_BindingIterator_Class_Source)
				SOMEXTERN struct CosNaming_BindingIteratorCClassDataStructure _CosNaming_BindingIteratorCClassData;
				#ifndef CosNaming_BindingIteratorCClassData
					#define CosNaming_BindingIteratorCClassData    _CosNaming_BindingIteratorCClassData
				#endif /* CosNaming_BindingIteratorCClassData */
			#else
				SOMEXTERN struct CosNaming_BindingIteratorCClassDataStructure * SOMLINK resolve_CosNaming_BindingIteratorCClassData(void);
				#ifndef CosNaming_BindingIteratorCClassData
					#define CosNaming_BindingIteratorCClassData    (*(resolve_CosNaming_BindingIteratorCClassData()))
				#endif /* CosNaming_BindingIteratorCClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_naming_Source) || defined(CosNaming_BindingIterator_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_naming_Source || CosNaming_BindingIterator_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_naming_Source || CosNaming_BindingIterator_Class_Source */
			struct CosNaming_BindingIteratorCClassDataStructure SOMDLINK CosNaming_BindingIteratorCClassData;
		#endif /* SOM_RESOLVE_DATA */
		extern 
		#if defined(SOM_Module_naming_Source) || defined(CosNaming_BindingIterator_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_naming_Source || CosNaming_BindingIterator_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_naming_Source || CosNaming_BindingIterator_Class_Source */
		SOMClass SOMSTAR SOMLINK CosNaming_BindingIteratorNewClass
				(integer4 somtmajorVersion,integer4 somtminorVersion);
		#define _SOMCLASS_CosNaming_BindingIterator (CosNaming_BindingIteratorClassData.classObject)
		#ifndef SOMGD_CosNaming_BindingIterator
			#if (defined(_CosNaming_BindingIterator) || defined(__CosNaming_BindingIterator))
				#undef _CosNaming_BindingIterator
				#undef __CosNaming_BindingIterator
				#define SOMGD_CosNaming_BindingIterator 1
			#else
				#define _CosNaming_BindingIterator _SOMCLASS_CosNaming_BindingIterator
			#endif /* _CosNaming_BindingIterator */
		#endif /* SOMGD_CosNaming_BindingIterator */
		#define CosNaming_BindingIterator_classObj _SOMCLASS_CosNaming_BindingIterator
		#define _SOMMTOKEN_CosNaming_BindingIterator(method) ((somMToken)(CosNaming_BindingIteratorClassData.method))
		#ifndef CosNaming_BindingIteratorNew
			#define CosNaming_BindingIteratorNew() ( _CosNaming_BindingIterator ? \
				(SOMClass_somNew(_CosNaming_BindingIterator)) : \
				( CosNaming_BindingIteratorNewClass( \
					CosNaming_BindingIterator_MajorVersion, \
					CosNaming_BindingIterator_MinorVersion), \
				SOMClass_somNew(_CosNaming_BindingIterator))) 
		#endif /* NewCosNaming_BindingIterator */
		#ifndef CosNaming_BindingIterator_next_one
			#define CosNaming_BindingIterator_next_one(somSelf,ev,b) \
				SOM_Resolve(somSelf,CosNaming_BindingIterator,next_one)  \
					(somSelf,ev,b)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__next_one
					#if defined(_next_one)
						#undef _next_one
						#define SOMGD__next_one
					#else
						#define _next_one CosNaming_BindingIterator_next_one
					#endif
				#endif /* SOMGD__next_one */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosNaming_BindingIterator_next_one */
		#ifndef CosNaming_BindingIterator_next_n
			#define CosNaming_BindingIterator_next_n(somSelf,ev,how_many,bl) \
				SOM_Resolve(somSelf,CosNaming_BindingIterator,next_n)  \
					(somSelf,ev,how_many,bl)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__next_n
					#if defined(_next_n)
						#undef _next_n
						#define SOMGD__next_n
					#else
						#define _next_n CosNaming_BindingIterator_next_n
					#endif
				#endif /* SOMGD__next_n */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosNaming_BindingIterator_next_n */
		#ifndef CosNaming_BindingIterator_destroy
			#define CosNaming_BindingIterator_destroy(somSelf,ev) \
				SOM_Resolve(somSelf,CosNaming_BindingIterator,destroy)  \
					(somSelf,ev)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__destroy
					#if defined(_destroy)
						#undef _destroy
						#define SOMGD__destroy
					#else
						#define _destroy CosNaming_BindingIterator_destroy
					#endif
				#endif /* SOMGD__destroy */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosNaming_BindingIterator_destroy */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define CosNaming_BindingIterator_somInit SOMObject_somInit
#define CosNaming_BindingIterator_somUninit SOMObject_somUninit
#define CosNaming_BindingIterator_somFree SOMObject_somFree
#define CosNaming_BindingIterator_somGetClass SOMObject_somGetClass
#define CosNaming_BindingIterator_somGetClassName SOMObject_somGetClassName
#define CosNaming_BindingIterator_somGetSize SOMObject_somGetSize
#define CosNaming_BindingIterator_somIsA SOMObject_somIsA
#define CosNaming_BindingIterator_somIsInstanceOf SOMObject_somIsInstanceOf
#define CosNaming_BindingIterator_somRespondsTo SOMObject_somRespondsTo
#define CosNaming_BindingIterator_somDispatch SOMObject_somDispatch
#define CosNaming_BindingIterator_somClassDispatch SOMObject_somClassDispatch
#define CosNaming_BindingIterator_somCastObj SOMObject_somCastObj
#define CosNaming_BindingIterator_somResetObj SOMObject_somResetObj
#define CosNaming_BindingIterator_somPrintSelf SOMObject_somPrintSelf
#define CosNaming_BindingIterator_somDumpSelf SOMObject_somDumpSelf
#define CosNaming_BindingIterator_somDumpSelfInt SOMObject_somDumpSelfInt
#define CosNaming_BindingIterator_somDefaultInit SOMObject_somDefaultInit
#define CosNaming_BindingIterator_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define CosNaming_BindingIterator_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define CosNaming_BindingIterator_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define CosNaming_BindingIterator_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define CosNaming_BindingIterator_somDefaultAssign SOMObject_somDefaultAssign
#define CosNaming_BindingIterator_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define CosNaming_BindingIterator_somDefaultVAssign SOMObject_somDefaultVAssign
#define CosNaming_BindingIterator_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define CosNaming_BindingIterator_somDestruct SOMObject_somDestruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

		#ifndef CosNaming_NamingContext_NotFoundReason_defined
			#define CosNaming_NamingContext_NotFoundReason_defined
			typedef unsigned long CosNaming_NamingContext_NotFoundReason;
			#ifndef CosNaming_NamingContext_missing_node
				#define CosNaming_NamingContext_missing_node 1UL
			#endif /* CosNaming_NamingContext_missing_node */
			#ifndef CosNaming_NamingContext_not_context
				#define CosNaming_NamingContext_not_context 2UL
			#endif /* CosNaming_NamingContext_not_context */
			#ifndef CosNaming_NamingContext_not_object
				#define CosNaming_NamingContext_not_object 3UL
			#endif /* CosNaming_NamingContext_not_object */
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD_NotFoundReason
					#if defined(NotFoundReason)
						#undef NotFoundReason
						#define SOMGD_NotFoundReason
					#else
						#define NotFoundReason CosNaming_NamingContext_NotFoundReason
					#endif
				#endif /* SOMGD_NotFoundReason */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosNaming_NamingContext_NotFoundReason_defined */
#define ex_CosNaming_NamingContext_NotFound   "::CosNaming::NamingContext::NotFound"
#ifndef SOM_DONT_USE_SHORT_NAMES
#ifndef SOMTGD_ex_NotFound
#ifndef ex_NotFound
#define ex_NotFound  ex_CosNaming_NamingContext_NotFound
#else
#define SOMTGD_ex_NotFound
#undef ex_NotFound
#endif /* ex_NotFound */
#endif /* SOMTGD_ex_NotFound */
#endif /* SOM_DONT_USE_SHORT_NAMES */
typedef struct CosNaming_NamingContext_NotFound
{
	CosNaming_NamingContext_NotFoundReason why;
	CosNaming_Name rest_of_name;
} CosNaming_NamingContext_NotFound;
#ifndef SOM_DONT_USE_SHORT_NAMES
	#ifndef SOMGD_NotFound
		#if defined(NotFound)
			#undef NotFound
			#define SOMGD_NotFound
		#else
			#define NotFound CosNaming_NamingContext_NotFound
		#endif
	#endif /* SOMGD_NotFound */
#endif /* SOM_DONT_USE_SHORT_NAMES */
#define ex_CosNaming_NamingContext_CannotProceed   "::CosNaming::NamingContext::CannotProceed"
#ifndef SOM_DONT_USE_SHORT_NAMES
#ifndef SOMTGD_ex_CannotProceed
#ifndef ex_CannotProceed
#define ex_CannotProceed  ex_CosNaming_NamingContext_CannotProceed
#else
#define SOMTGD_ex_CannotProceed
#undef ex_CannotProceed
#endif /* ex_CannotProceed */
#endif /* SOMTGD_ex_CannotProceed */
#endif /* SOM_DONT_USE_SHORT_NAMES */
typedef struct CosNaming_NamingContext_CannotProceed
{
	CosNaming_NamingContext SOMSTAR cxt;
	CosNaming_Name rest_of_name;
} CosNaming_NamingContext_CannotProceed;
#ifndef SOM_DONT_USE_SHORT_NAMES
	#ifndef SOMGD_CannotProceed
		#if defined(CannotProceed)
			#undef CannotProceed
			#define SOMGD_CannotProceed
		#else
			#define CannotProceed CosNaming_NamingContext_CannotProceed
		#endif
	#endif /* SOMGD_CannotProceed */
#endif /* SOM_DONT_USE_SHORT_NAMES */
#define ex_CosNaming_NamingContext_InvalidName   "::CosNaming::NamingContext::InvalidName"
#ifndef SOM_DONT_USE_SHORT_NAMES
#ifndef SOMTGD_ex_InvalidName
#ifndef ex_InvalidName
#define ex_InvalidName  ex_CosNaming_NamingContext_InvalidName
#else
#define SOMTGD_ex_InvalidName
#undef ex_InvalidName
#endif /* ex_InvalidName */
#endif /* SOMTGD_ex_InvalidName */
#endif /* SOM_DONT_USE_SHORT_NAMES */
typedef struct CosNaming_NamingContext_InvalidName
{
	CosNaming_Name name;
} CosNaming_NamingContext_InvalidName;
#ifndef SOM_DONT_USE_SHORT_NAMES
	#ifndef SOMGD_InvalidName
		#if defined(InvalidName)
			#undef InvalidName
			#define SOMGD_InvalidName
		#else
			#define InvalidName CosNaming_NamingContext_InvalidName
		#endif
	#endif /* SOMGD_InvalidName */
#endif /* SOM_DONT_USE_SHORT_NAMES */
#define ex_CosNaming_NamingContext_AlreadyBound   "::CosNaming::NamingContext::AlreadyBound"
#ifndef SOM_DONT_USE_SHORT_NAMES
#ifndef SOMTGD_ex_AlreadyBound
#ifndef ex_AlreadyBound
#define ex_AlreadyBound  ex_CosNaming_NamingContext_AlreadyBound
#else
#define SOMTGD_ex_AlreadyBound
#undef ex_AlreadyBound
#endif /* ex_AlreadyBound */
#endif /* SOMTGD_ex_AlreadyBound */
#endif /* SOM_DONT_USE_SHORT_NAMES */
typedef struct CosNaming_NamingContext_AlreadyBound CosNaming_NamingContext_AlreadyBound;
#ifndef SOM_DONT_USE_SHORT_NAMES
	#ifndef SOMGD_AlreadyBound
		#if defined(AlreadyBound)
			#undef AlreadyBound
			#define SOMGD_AlreadyBound
		#else
			#define AlreadyBound CosNaming_NamingContext_AlreadyBound
		#endif
	#endif /* SOMGD_AlreadyBound */
#endif /* SOM_DONT_USE_SHORT_NAMES */
#define ex_CosNaming_NamingContext_NotEmpty   "::CosNaming::NamingContext::NotEmpty"
#ifndef SOM_DONT_USE_SHORT_NAMES
#ifndef SOMTGD_ex_NotEmpty
#ifndef ex_NotEmpty
#define ex_NotEmpty  ex_CosNaming_NamingContext_NotEmpty
#else
#define SOMTGD_ex_NotEmpty
#undef ex_NotEmpty
#endif /* ex_NotEmpty */
#endif /* SOMTGD_ex_NotEmpty */
#endif /* SOM_DONT_USE_SHORT_NAMES */
typedef struct CosNaming_NamingContext_NotEmpty CosNaming_NamingContext_NotEmpty;
#ifndef SOM_DONT_USE_SHORT_NAMES
	#ifndef SOMGD_NotEmpty
		#if defined(NotEmpty)
			#undef NotEmpty
			#define SOMGD_NotEmpty
		#else
			#define NotEmpty CosNaming_NamingContext_NotEmpty
		#endif
	#endif /* SOMGD_NotEmpty */
#endif /* SOM_DONT_USE_SHORT_NAMES */
		#ifdef __IBMC__
			typedef void (somTP_CosNaming_NamingContext_bind)(
				CosNaming_NamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ SOMObject SOMSTAR obj);
			#pragma linkage(somTP_CosNaming_NamingContext_bind,system)
			typedef somTP_CosNaming_NamingContext_bind *somTD_CosNaming_NamingContext_bind;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_CosNaming_NamingContext_bind)(
				CosNaming_NamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ SOMObject SOMSTAR obj);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_CosNaming_NamingContext_rebind)(
				CosNaming_NamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ SOMObject SOMSTAR obj);
			#pragma linkage(somTP_CosNaming_NamingContext_rebind,system)
			typedef somTP_CosNaming_NamingContext_rebind *somTD_CosNaming_NamingContext_rebind;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_CosNaming_NamingContext_rebind)(
				CosNaming_NamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ SOMObject SOMSTAR obj);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_CosNaming_NamingContext_bind_context)(
				CosNaming_NamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ CosNaming_NamingContext SOMSTAR nc);
			#pragma linkage(somTP_CosNaming_NamingContext_bind_context,system)
			typedef somTP_CosNaming_NamingContext_bind_context *somTD_CosNaming_NamingContext_bind_context;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_CosNaming_NamingContext_bind_context)(
				CosNaming_NamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ CosNaming_NamingContext SOMSTAR nc);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_CosNaming_NamingContext_rebind_context)(
				CosNaming_NamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ CosNaming_NamingContext SOMSTAR nc);
			#pragma linkage(somTP_CosNaming_NamingContext_rebind_context,system)
			typedef somTP_CosNaming_NamingContext_rebind_context *somTD_CosNaming_NamingContext_rebind_context;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_CosNaming_NamingContext_rebind_context)(
				CosNaming_NamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ CosNaming_NamingContext SOMSTAR nc);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef SOMObject SOMSTAR (somTP_CosNaming_NamingContext_resolve)(
				CosNaming_NamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n);
			#pragma linkage(somTP_CosNaming_NamingContext_resolve,system)
			typedef somTP_CosNaming_NamingContext_resolve *somTD_CosNaming_NamingContext_resolve;
		#else /* __IBMC__ */
			typedef SOMObject SOMSTAR (SOMLINK * somTD_CosNaming_NamingContext_resolve)(
				CosNaming_NamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_CosNaming_NamingContext_unbind)(
				CosNaming_NamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n);
			#pragma linkage(somTP_CosNaming_NamingContext_unbind,system)
			typedef somTP_CosNaming_NamingContext_unbind *somTD_CosNaming_NamingContext_unbind;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_CosNaming_NamingContext_unbind)(
				CosNaming_NamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef CosNaming_NamingContext SOMSTAR (somTP_CosNaming_NamingContext_new_context)(
				CosNaming_NamingContext SOMSTAR somSelf,
				Environment *ev);
			#pragma linkage(somTP_CosNaming_NamingContext_new_context,system)
			typedef somTP_CosNaming_NamingContext_new_context *somTD_CosNaming_NamingContext_new_context;
		#else /* __IBMC__ */
			typedef CosNaming_NamingContext SOMSTAR (SOMLINK * somTD_CosNaming_NamingContext_new_context)(
				CosNaming_NamingContext SOMSTAR somSelf,
				Environment *ev);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef CosNaming_NamingContext SOMSTAR (somTP_CosNaming_NamingContext_bind_new_context)(
				CosNaming_NamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n);
			#pragma linkage(somTP_CosNaming_NamingContext_bind_new_context,system)
			typedef somTP_CosNaming_NamingContext_bind_new_context *somTD_CosNaming_NamingContext_bind_new_context;
		#else /* __IBMC__ */
			typedef CosNaming_NamingContext SOMSTAR (SOMLINK * somTD_CosNaming_NamingContext_bind_new_context)(
				CosNaming_NamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_CosNaming_NamingContext_destroy)(
				CosNaming_NamingContext SOMSTAR somSelf,
				Environment *ev);
			#pragma linkage(somTP_CosNaming_NamingContext_destroy,system)
			typedef somTP_CosNaming_NamingContext_destroy *somTD_CosNaming_NamingContext_destroy;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_CosNaming_NamingContext_destroy)(
				CosNaming_NamingContext SOMSTAR somSelf,
				Environment *ev);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_CosNaming_NamingContext_list)(
				CosNaming_NamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ unsigned long how_many,
				/* out */ CosNaming_BindingList *bl,
				/* out */ CosNaming_BindingIterator SOMSTAR *bi);
			#pragma linkage(somTP_CosNaming_NamingContext_list,system)
			typedef somTP_CosNaming_NamingContext_list *somTD_CosNaming_NamingContext_list;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_CosNaming_NamingContext_list)(
				CosNaming_NamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ unsigned long how_many,
				/* out */ CosNaming_BindingList *bl,
				/* out */ CosNaming_BindingIterator SOMSTAR *bi);
		#endif /* __IBMC__ */
		#ifndef CosNaming_NamingContext_MajorVersion
			#define CosNaming_NamingContext_MajorVersion   3
		#endif /* CosNaming_NamingContext_MajorVersion */
		#ifndef CosNaming_NamingContext_MinorVersion
			#define CosNaming_NamingContext_MinorVersion   0
		#endif /* CosNaming_NamingContext_MinorVersion */
		typedef struct CosNaming_NamingContextClassDataStructure
		{
			SOMClass SOMSTAR classObject;
			somMToken bind;
			somMToken rebind;
			somMToken bind_context;
			somMToken rebind_context;
			somMToken resolve;
			somMToken unbind;
			somMToken new_context;
			somMToken bind_new_context;
			somMToken destroy;
			somMToken list;
			somMToken private10;
		} CosNaming_NamingContextClassDataStructure;
		typedef struct CosNaming_NamingContextCClassDataStructure
		{
			somMethodTabs parentMtab;
			somDToken instanceDataToken;
		} CosNaming_NamingContextCClassDataStructure;
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_naming_Source) || defined(CosNaming_NamingContext_Class_Source)
				SOMEXTERN struct CosNaming_NamingContextClassDataStructure _CosNaming_NamingContextClassData;
				#ifndef CosNaming_NamingContextClassData
					#define CosNaming_NamingContextClassData    _CosNaming_NamingContextClassData
				#endif /* CosNaming_NamingContextClassData */
			#else
				SOMEXTERN struct CosNaming_NamingContextClassDataStructure * SOMLINK resolve_CosNaming_NamingContextClassData(void);
				#ifndef CosNaming_NamingContextClassData
					#define CosNaming_NamingContextClassData    (*(resolve_CosNaming_NamingContextClassData()))
				#endif /* CosNaming_NamingContextClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_naming_Source) || defined(CosNaming_NamingContext_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_naming_Source || CosNaming_NamingContext_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_naming_Source || CosNaming_NamingContext_Class_Source */
			struct CosNaming_NamingContextClassDataStructure SOMDLINK CosNaming_NamingContextClassData;
		#endif /* SOM_RESOLVE_DATA */
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_naming_Source) || defined(CosNaming_NamingContext_Class_Source)
				SOMEXTERN struct CosNaming_NamingContextCClassDataStructure _CosNaming_NamingContextCClassData;
				#ifndef CosNaming_NamingContextCClassData
					#define CosNaming_NamingContextCClassData    _CosNaming_NamingContextCClassData
				#endif /* CosNaming_NamingContextCClassData */
			#else
				SOMEXTERN struct CosNaming_NamingContextCClassDataStructure * SOMLINK resolve_CosNaming_NamingContextCClassData(void);
				#ifndef CosNaming_NamingContextCClassData
					#define CosNaming_NamingContextCClassData    (*(resolve_CosNaming_NamingContextCClassData()))
				#endif /* CosNaming_NamingContextCClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_naming_Source) || defined(CosNaming_NamingContext_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_naming_Source || CosNaming_NamingContext_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_naming_Source || CosNaming_NamingContext_Class_Source */
			struct CosNaming_NamingContextCClassDataStructure SOMDLINK CosNaming_NamingContextCClassData;
		#endif /* SOM_RESOLVE_DATA */
		extern 
		#if defined(SOM_Module_naming_Source) || defined(CosNaming_NamingContext_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_naming_Source || CosNaming_NamingContext_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_naming_Source || CosNaming_NamingContext_Class_Source */
		SOMClass SOMSTAR SOMLINK CosNaming_NamingContextNewClass
				(integer4 somtmajorVersion,integer4 somtminorVersion);
		#define _SOMCLASS_CosNaming_NamingContext (CosNaming_NamingContextClassData.classObject)
		#ifndef SOMGD_CosNaming_NamingContext
			#if (defined(_CosNaming_NamingContext) || defined(__CosNaming_NamingContext))
				#undef _CosNaming_NamingContext
				#undef __CosNaming_NamingContext
				#define SOMGD_CosNaming_NamingContext 1
			#else
				#define _CosNaming_NamingContext _SOMCLASS_CosNaming_NamingContext
			#endif /* _CosNaming_NamingContext */
		#endif /* SOMGD_CosNaming_NamingContext */
		#define CosNaming_NamingContext_classObj _SOMCLASS_CosNaming_NamingContext
		#define _SOMMTOKEN_CosNaming_NamingContext(method) ((somMToken)(CosNaming_NamingContextClassData.method))
		#ifndef CosNaming_NamingContextNew
			#define CosNaming_NamingContextNew() ( _CosNaming_NamingContext ? \
				(SOMClass_somNew(_CosNaming_NamingContext)) : \
				( CosNaming_NamingContextNewClass( \
					CosNaming_NamingContext_MajorVersion, \
					CosNaming_NamingContext_MinorVersion), \
				SOMClass_somNew(_CosNaming_NamingContext))) 
		#endif /* NewCosNaming_NamingContext */
		#ifndef CosNaming_NamingContext_bind
			#define CosNaming_NamingContext_bind(somSelf,ev,n,obj) \
				SOM_Resolve(somSelf,CosNaming_NamingContext,bind)  \
					(somSelf,ev,n,obj)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__bind
					#if defined(_bind)
						#undef _bind
						#define SOMGD__bind
					#else
						#define _bind CosNaming_NamingContext_bind
					#endif
				#endif /* SOMGD__bind */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosNaming_NamingContext_bind */
		#ifndef CosNaming_NamingContext_rebind
			#define CosNaming_NamingContext_rebind(somSelf,ev,n,obj) \
				SOM_Resolve(somSelf,CosNaming_NamingContext,rebind)  \
					(somSelf,ev,n,obj)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__rebind
					#if defined(_rebind)
						#undef _rebind
						#define SOMGD__rebind
					#else
						#define _rebind CosNaming_NamingContext_rebind
					#endif
				#endif /* SOMGD__rebind */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosNaming_NamingContext_rebind */
		#ifndef CosNaming_NamingContext_bind_context
			#define CosNaming_NamingContext_bind_context(somSelf,ev,n,nc) \
				SOM_Resolve(somSelf,CosNaming_NamingContext,bind_context)  \
					(somSelf,ev,n,nc)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__bind_context
					#if defined(_bind_context)
						#undef _bind_context
						#define SOMGD__bind_context
					#else
						#define _bind_context CosNaming_NamingContext_bind_context
					#endif
				#endif /* SOMGD__bind_context */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosNaming_NamingContext_bind_context */
		#ifndef CosNaming_NamingContext_rebind_context
			#define CosNaming_NamingContext_rebind_context(somSelf,ev,n,nc) \
				SOM_Resolve(somSelf,CosNaming_NamingContext,rebind_context)  \
					(somSelf,ev,n,nc)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__rebind_context
					#if defined(_rebind_context)
						#undef _rebind_context
						#define SOMGD__rebind_context
					#else
						#define _rebind_context CosNaming_NamingContext_rebind_context
					#endif
				#endif /* SOMGD__rebind_context */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosNaming_NamingContext_rebind_context */
		#ifndef CosNaming_NamingContext_resolve
			#define CosNaming_NamingContext_resolve(somSelf,ev,n) \
				SOM_Resolve(somSelf,CosNaming_NamingContext,resolve)  \
					(somSelf,ev,n)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__resolve
					#if defined(_resolve)
						#undef _resolve
						#define SOMGD__resolve
					#else
						#define _resolve CosNaming_NamingContext_resolve
					#endif
				#endif /* SOMGD__resolve */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosNaming_NamingContext_resolve */
		#ifndef CosNaming_NamingContext_unbind
			#define CosNaming_NamingContext_unbind(somSelf,ev,n) \
				SOM_Resolve(somSelf,CosNaming_NamingContext,unbind)  \
					(somSelf,ev,n)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__unbind
					#if defined(_unbind)
						#undef _unbind
						#define SOMGD__unbind
					#else
						#define _unbind CosNaming_NamingContext_unbind
					#endif
				#endif /* SOMGD__unbind */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosNaming_NamingContext_unbind */
		#ifndef CosNaming_NamingContext_new_context
			#define CosNaming_NamingContext_new_context(somSelf,ev) \
				SOM_Resolve(somSelf,CosNaming_NamingContext,new_context)  \
					(somSelf,ev)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__new_context
					#if defined(_new_context)
						#undef _new_context
						#define SOMGD__new_context
					#else
						#define _new_context CosNaming_NamingContext_new_context
					#endif
				#endif /* SOMGD__new_context */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosNaming_NamingContext_new_context */
		#ifndef CosNaming_NamingContext_bind_new_context
			#define CosNaming_NamingContext_bind_new_context(somSelf,ev,n) \
				SOM_Resolve(somSelf,CosNaming_NamingContext,bind_new_context)  \
					(somSelf,ev,n)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__bind_new_context
					#if defined(_bind_new_context)
						#undef _bind_new_context
						#define SOMGD__bind_new_context
					#else
						#define _bind_new_context CosNaming_NamingContext_bind_new_context
					#endif
				#endif /* SOMGD__bind_new_context */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosNaming_NamingContext_bind_new_context */
		#ifndef CosNaming_NamingContext_destroy
			#define CosNaming_NamingContext_destroy(somSelf,ev) \
				SOM_Resolve(somSelf,CosNaming_NamingContext,destroy)  \
					(somSelf,ev)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__destroy
					#if defined(_destroy)
						#undef _destroy
						#define SOMGD__destroy
					#else
						#define _destroy CosNaming_NamingContext_destroy
					#endif
				#endif /* SOMGD__destroy */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosNaming_NamingContext_destroy */
		#ifndef CosNaming_NamingContext_list
			#define CosNaming_NamingContext_list(somSelf,ev,how_many,bl,bi) \
				SOM_Resolve(somSelf,CosNaming_NamingContext,list)  \
					(somSelf,ev,how_many,bl,bi)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__list
					#if defined(_list)
						#undef _list
						#define SOMGD__list
					#else
						#define _list CosNaming_NamingContext_list
					#endif
				#endif /* SOMGD__list */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosNaming_NamingContext_list */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define CosNaming_NamingContext_somInit SOMObject_somInit
#define CosNaming_NamingContext_somUninit SOMObject_somUninit
#define CosNaming_NamingContext_somFree SOMObject_somFree
#define CosNaming_NamingContext_somGetClass SOMObject_somGetClass
#define CosNaming_NamingContext_somGetClassName SOMObject_somGetClassName
#define CosNaming_NamingContext_somGetSize SOMObject_somGetSize
#define CosNaming_NamingContext_somIsA SOMObject_somIsA
#define CosNaming_NamingContext_somIsInstanceOf SOMObject_somIsInstanceOf
#define CosNaming_NamingContext_somRespondsTo SOMObject_somRespondsTo
#define CosNaming_NamingContext_somDispatch SOMObject_somDispatch
#define CosNaming_NamingContext_somClassDispatch SOMObject_somClassDispatch
#define CosNaming_NamingContext_somCastObj SOMObject_somCastObj
#define CosNaming_NamingContext_somResetObj SOMObject_somResetObj
#define CosNaming_NamingContext_somPrintSelf SOMObject_somPrintSelf
#define CosNaming_NamingContext_somDumpSelf SOMObject_somDumpSelf
#define CosNaming_NamingContext_somDumpSelfInt SOMObject_somDumpSelfInt
#define CosNaming_NamingContext_somDefaultInit SOMObject_somDefaultInit
#define CosNaming_NamingContext_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define CosNaming_NamingContext_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define CosNaming_NamingContext_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define CosNaming_NamingContext_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define CosNaming_NamingContext_somDefaultAssign SOMObject_somDefaultAssign
#define CosNaming_NamingContext_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define CosNaming_NamingContext_somDefaultVAssign SOMObject_somDefaultVAssign
#define CosNaming_NamingContext_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define CosNaming_NamingContext_somDestruct SOMObject_somDestruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

		typedef string CosNaming_NamingContextExt_StringName;
		typedef string CosNaming_NamingContextExt_Address;
		typedef string CosNaming_NamingContextExt_URLString;
#define ex_CosNaming_NamingContextExt_InvalidAddress   "::CosNaming::NamingContextExt::InvalidAddress"
#ifndef SOM_DONT_USE_SHORT_NAMES
#ifndef SOMTGD_ex_InvalidAddress
#ifndef ex_InvalidAddress
#define ex_InvalidAddress  ex_CosNaming_NamingContextExt_InvalidAddress
#else
#define SOMTGD_ex_InvalidAddress
#undef ex_InvalidAddress
#endif /* ex_InvalidAddress */
#endif /* SOMTGD_ex_InvalidAddress */
#endif /* SOM_DONT_USE_SHORT_NAMES */
typedef struct CosNaming_NamingContextExt_InvalidAddress CosNaming_NamingContextExt_InvalidAddress;
#ifndef SOM_DONT_USE_SHORT_NAMES
	#ifndef SOMGD_InvalidAddress
		#if defined(InvalidAddress)
			#undef InvalidAddress
			#define SOMGD_InvalidAddress
		#else
			#define InvalidAddress CosNaming_NamingContextExt_InvalidAddress
		#endif
	#endif /* SOMGD_InvalidAddress */
#endif /* SOM_DONT_USE_SHORT_NAMES */
		#ifdef __IBMC__
			typedef CosNaming_NamingContextExt_StringName (somTP_CosNaming_NamingContextExt_to_string)(
				CosNaming_NamingContextExt SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n);
			#pragma linkage(somTP_CosNaming_NamingContextExt_to_string,system)
			typedef somTP_CosNaming_NamingContextExt_to_string *somTD_CosNaming_NamingContextExt_to_string;
		#else /* __IBMC__ */
			typedef CosNaming_NamingContextExt_StringName (SOMLINK * somTD_CosNaming_NamingContextExt_to_string)(
				CosNaming_NamingContextExt SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef CosNaming_Name (somTP_CosNaming_NamingContextExt_to_name)(
				CosNaming_NamingContextExt SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_NamingContextExt_StringName sn);
			#pragma linkage(somTP_CosNaming_NamingContextExt_to_name,system)
			typedef somTP_CosNaming_NamingContextExt_to_name *somTD_CosNaming_NamingContextExt_to_name;
		#else /* __IBMC__ */
			typedef CosNaming_Name (SOMLINK * somTD_CosNaming_NamingContextExt_to_name)(
				CosNaming_NamingContextExt SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_NamingContextExt_StringName sn);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef CosNaming_NamingContextExt_URLString (somTP_CosNaming_NamingContextExt_to_url)(
				CosNaming_NamingContextExt SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_NamingContextExt_Address addr,
				/* in */ CosNaming_NamingContextExt_StringName sn);
			#pragma linkage(somTP_CosNaming_NamingContextExt_to_url,system)
			typedef somTP_CosNaming_NamingContextExt_to_url *somTD_CosNaming_NamingContextExt_to_url;
		#else /* __IBMC__ */
			typedef CosNaming_NamingContextExt_URLString (SOMLINK * somTD_CosNaming_NamingContextExt_to_url)(
				CosNaming_NamingContextExt SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_NamingContextExt_Address addr,
				/* in */ CosNaming_NamingContextExt_StringName sn);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef SOMObject SOMSTAR (somTP_CosNaming_NamingContextExt_resolve_str)(
				CosNaming_NamingContextExt SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_NamingContextExt_StringName n);
			#pragma linkage(somTP_CosNaming_NamingContextExt_resolve_str,system)
			typedef somTP_CosNaming_NamingContextExt_resolve_str *somTD_CosNaming_NamingContextExt_resolve_str;
		#else /* __IBMC__ */
			typedef SOMObject SOMSTAR (SOMLINK * somTD_CosNaming_NamingContextExt_resolve_str)(
				CosNaming_NamingContextExt SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_NamingContextExt_StringName n);
		#endif /* __IBMC__ */
		#ifndef CosNaming_NamingContextExt_MajorVersion
			#define CosNaming_NamingContextExt_MajorVersion   0
		#endif /* CosNaming_NamingContextExt_MajorVersion */
		#ifndef CosNaming_NamingContextExt_MinorVersion
			#define CosNaming_NamingContextExt_MinorVersion   0
		#endif /* CosNaming_NamingContextExt_MinorVersion */
		typedef struct CosNaming_NamingContextExtClassDataStructure
		{
			SOMClass SOMSTAR classObject;
			somMToken to_string;
			somMToken to_name;
			somMToken to_url;
			somMToken resolve_str;
		} CosNaming_NamingContextExtClassDataStructure;
		typedef struct CosNaming_NamingContextExtCClassDataStructure
		{
			somMethodTabs parentMtab;
			somDToken instanceDataToken;
		} CosNaming_NamingContextExtCClassDataStructure;
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_naming_Source) || defined(CosNaming_NamingContextExt_Class_Source)
				SOMEXTERN struct CosNaming_NamingContextExtClassDataStructure _CosNaming_NamingContextExtClassData;
				#ifndef CosNaming_NamingContextExtClassData
					#define CosNaming_NamingContextExtClassData    _CosNaming_NamingContextExtClassData
				#endif /* CosNaming_NamingContextExtClassData */
			#else
				SOMEXTERN struct CosNaming_NamingContextExtClassDataStructure * SOMLINK resolve_CosNaming_NamingContextExtClassData(void);
				#ifndef CosNaming_NamingContextExtClassData
					#define CosNaming_NamingContextExtClassData    (*(resolve_CosNaming_NamingContextExtClassData()))
				#endif /* CosNaming_NamingContextExtClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_naming_Source) || defined(CosNaming_NamingContextExt_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_naming_Source || CosNaming_NamingContextExt_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_naming_Source || CosNaming_NamingContextExt_Class_Source */
			struct CosNaming_NamingContextExtClassDataStructure SOMDLINK CosNaming_NamingContextExtClassData;
		#endif /* SOM_RESOLVE_DATA */
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_naming_Source) || defined(CosNaming_NamingContextExt_Class_Source)
				SOMEXTERN struct CosNaming_NamingContextExtCClassDataStructure _CosNaming_NamingContextExtCClassData;
				#ifndef CosNaming_NamingContextExtCClassData
					#define CosNaming_NamingContextExtCClassData    _CosNaming_NamingContextExtCClassData
				#endif /* CosNaming_NamingContextExtCClassData */
			#else
				SOMEXTERN struct CosNaming_NamingContextExtCClassDataStructure * SOMLINK resolve_CosNaming_NamingContextExtCClassData(void);
				#ifndef CosNaming_NamingContextExtCClassData
					#define CosNaming_NamingContextExtCClassData    (*(resolve_CosNaming_NamingContextExtCClassData()))
				#endif /* CosNaming_NamingContextExtCClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_naming_Source) || defined(CosNaming_NamingContextExt_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_naming_Source || CosNaming_NamingContextExt_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_naming_Source || CosNaming_NamingContextExt_Class_Source */
			struct CosNaming_NamingContextExtCClassDataStructure SOMDLINK CosNaming_NamingContextExtCClassData;
		#endif /* SOM_RESOLVE_DATA */
		extern 
		#if defined(SOM_Module_naming_Source) || defined(CosNaming_NamingContextExt_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_naming_Source || CosNaming_NamingContextExt_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_naming_Source || CosNaming_NamingContextExt_Class_Source */
		SOMClass SOMSTAR SOMLINK CosNaming_NamingContextExtNewClass
				(integer4 somtmajorVersion,integer4 somtminorVersion);
		#define _SOMCLASS_CosNaming_NamingContextExt (CosNaming_NamingContextExtClassData.classObject)
		#ifndef SOMGD_CosNaming_NamingContextExt
			#if (defined(_CosNaming_NamingContextExt) || defined(__CosNaming_NamingContextExt))
				#undef _CosNaming_NamingContextExt
				#undef __CosNaming_NamingContextExt
				#define SOMGD_CosNaming_NamingContextExt 1
			#else
				#define _CosNaming_NamingContextExt _SOMCLASS_CosNaming_NamingContextExt
			#endif /* _CosNaming_NamingContextExt */
		#endif /* SOMGD_CosNaming_NamingContextExt */
		#define CosNaming_NamingContextExt_classObj _SOMCLASS_CosNaming_NamingContextExt
		#define _SOMMTOKEN_CosNaming_NamingContextExt(method) ((somMToken)(CosNaming_NamingContextExtClassData.method))
		#ifndef CosNaming_NamingContextExtNew
			#define CosNaming_NamingContextExtNew() ( _CosNaming_NamingContextExt ? \
				(SOMClass_somNew(_CosNaming_NamingContextExt)) : \
				( CosNaming_NamingContextExtNewClass( \
					CosNaming_NamingContextExt_MajorVersion, \
					CosNaming_NamingContextExt_MinorVersion), \
				SOMClass_somNew(_CosNaming_NamingContextExt))) 
		#endif /* NewCosNaming_NamingContextExt */
		#ifndef CosNaming_NamingContextExt_to_string
			#define CosNaming_NamingContextExt_to_string(somSelf,ev,n) \
				SOM_Resolve(somSelf,CosNaming_NamingContextExt,to_string)  \
					(somSelf,ev,n)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__to_string
					#if defined(_to_string)
						#undef _to_string
						#define SOMGD__to_string
					#else
						#define _to_string CosNaming_NamingContextExt_to_string
					#endif
				#endif /* SOMGD__to_string */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosNaming_NamingContextExt_to_string */
		#ifndef CosNaming_NamingContextExt_to_name
			#define CosNaming_NamingContextExt_to_name(somSelf,ev,sn) \
				SOM_Resolve(somSelf,CosNaming_NamingContextExt,to_name)  \
					(somSelf,ev,sn)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__to_name
					#if defined(_to_name)
						#undef _to_name
						#define SOMGD__to_name
					#else
						#define _to_name CosNaming_NamingContextExt_to_name
					#endif
				#endif /* SOMGD__to_name */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosNaming_NamingContextExt_to_name */
		#ifndef CosNaming_NamingContextExt_to_url
			#define CosNaming_NamingContextExt_to_url(somSelf,ev,addr,sn) \
				SOM_Resolve(somSelf,CosNaming_NamingContextExt,to_url)  \
					(somSelf,ev,addr,sn)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__to_url
					#if defined(_to_url)
						#undef _to_url
						#define SOMGD__to_url
					#else
						#define _to_url CosNaming_NamingContextExt_to_url
					#endif
				#endif /* SOMGD__to_url */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosNaming_NamingContextExt_to_url */
		#ifndef CosNaming_NamingContextExt_resolve_str
			#define CosNaming_NamingContextExt_resolve_str(somSelf,ev,n) \
				SOM_Resolve(somSelf,CosNaming_NamingContextExt,resolve_str)  \
					(somSelf,ev,n)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__resolve_str
					#if defined(_resolve_str)
						#undef _resolve_str
						#define SOMGD__resolve_str
					#else
						#define _resolve_str CosNaming_NamingContextExt_resolve_str
					#endif
				#endif /* SOMGD__resolve_str */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosNaming_NamingContextExt_resolve_str */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define CosNaming_NamingContextExt_somInit SOMObject_somInit
#define CosNaming_NamingContextExt_somUninit SOMObject_somUninit
#define CosNaming_NamingContextExt_somFree SOMObject_somFree
#define CosNaming_NamingContextExt_somGetClass SOMObject_somGetClass
#define CosNaming_NamingContextExt_somGetClassName SOMObject_somGetClassName
#define CosNaming_NamingContextExt_somGetSize SOMObject_somGetSize
#define CosNaming_NamingContextExt_somIsA SOMObject_somIsA
#define CosNaming_NamingContextExt_somIsInstanceOf SOMObject_somIsInstanceOf
#define CosNaming_NamingContextExt_somRespondsTo SOMObject_somRespondsTo
#define CosNaming_NamingContextExt_somDispatch SOMObject_somDispatch
#define CosNaming_NamingContextExt_somClassDispatch SOMObject_somClassDispatch
#define CosNaming_NamingContextExt_somCastObj SOMObject_somCastObj
#define CosNaming_NamingContextExt_somResetObj SOMObject_somResetObj
#define CosNaming_NamingContextExt_somPrintSelf SOMObject_somPrintSelf
#define CosNaming_NamingContextExt_somDumpSelf SOMObject_somDumpSelf
#define CosNaming_NamingContextExt_somDumpSelfInt SOMObject_somDumpSelfInt
#define CosNaming_NamingContextExt_somDefaultInit SOMObject_somDefaultInit
#define CosNaming_NamingContextExt_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define CosNaming_NamingContextExt_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define CosNaming_NamingContextExt_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define CosNaming_NamingContextExt_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define CosNaming_NamingContextExt_somDefaultAssign SOMObject_somDefaultAssign
#define CosNaming_NamingContextExt_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define CosNaming_NamingContextExt_somDefaultVAssign SOMObject_somDefaultVAssign
#define CosNaming_NamingContextExt_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define CosNaming_NamingContextExt_somDestruct SOMObject_somDestruct
#define CosNaming_NamingContextExt_bind CosNaming_NamingContext_bind
#define CosNaming_NamingContextExt_rebind CosNaming_NamingContext_rebind
#define CosNaming_NamingContextExt_bind_context CosNaming_NamingContext_bind_context
#define CosNaming_NamingContextExt_rebind_context CosNaming_NamingContext_rebind_context
#define CosNaming_NamingContextExt_resolve CosNaming_NamingContext_resolve
#define CosNaming_NamingContextExt_unbind CosNaming_NamingContext_unbind
#define CosNaming_NamingContextExt_new_context CosNaming_NamingContext_new_context
#define CosNaming_NamingContextExt_bind_new_context CosNaming_NamingContext_bind_new_context
#define CosNaming_NamingContextExt_destroy CosNaming_NamingContext_destroy
#define CosNaming_NamingContextExt_list CosNaming_NamingContext_list
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_naming_Header_h */
