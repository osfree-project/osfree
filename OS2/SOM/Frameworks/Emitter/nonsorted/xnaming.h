/* generated from xnaming.idl */
/* internal conditional is SOM_Module_xnaming_Source */
#ifndef SOM_Module_xnaming_Header_h
	#define SOM_Module_xnaming_Header_h 1
	#include <som.h>
	#include <naming.h>
	#include <somobj.h>
		#ifndef _IDL_ExtendedNaming_PropertyBindingIterator_defined
			#define _IDL_ExtendedNaming_PropertyBindingIterator_defined
			typedef SOMObject ExtendedNaming_PropertyBindingIterator;
		#endif /* _IDL_ExtendedNaming_PropertyBindingIterator_defined */
		#ifndef _IDL_SEQUENCE_ExtendedNaming_PropertyBindingIterator_defined
			#define _IDL_SEQUENCE_ExtendedNaming_PropertyBindingIterator_defined
			SOM_SEQUENCE_TYPEDEF_NAME(ExtendedNaming_PropertyBindingIterator SOMSTAR ,sequence(ExtendedNaming_PropertyBindingIterator));
		#endif /* _IDL_SEQUENCE_ExtendedNaming_PropertyBindingIterator_defined */
		#ifndef _IDL_ExtendedNaming_PropertyIterator_defined
			#define _IDL_ExtendedNaming_PropertyIterator_defined
			typedef SOMObject ExtendedNaming_PropertyIterator;
		#endif /* _IDL_ExtendedNaming_PropertyIterator_defined */
		#ifndef _IDL_SEQUENCE_ExtendedNaming_PropertyIterator_defined
			#define _IDL_SEQUENCE_ExtendedNaming_PropertyIterator_defined
			SOM_SEQUENCE_TYPEDEF_NAME(ExtendedNaming_PropertyIterator SOMSTAR ,sequence(ExtendedNaming_PropertyIterator));
		#endif /* _IDL_SEQUENCE_ExtendedNaming_PropertyIterator_defined */
		#ifndef _IDL_ExtendedNaming_IndexIterator_defined
			#define _IDL_ExtendedNaming_IndexIterator_defined
			typedef SOMObject ExtendedNaming_IndexIterator;
		#endif /* _IDL_ExtendedNaming_IndexIterator_defined */
		#ifndef _IDL_SEQUENCE_ExtendedNaming_IndexIterator_defined
			#define _IDL_SEQUENCE_ExtendedNaming_IndexIterator_defined
			SOM_SEQUENCE_TYPEDEF_NAME(ExtendedNaming_IndexIterator SOMSTAR ,sequence(ExtendedNaming_IndexIterator));
		#endif /* _IDL_SEQUENCE_ExtendedNaming_IndexIterator_defined */
		#ifndef _IDL_ExtendedNaming_ExtendedNamingContext_defined
			#define _IDL_ExtendedNaming_ExtendedNamingContext_defined
			typedef SOMObject ExtendedNaming_ExtendedNamingContext;
		#endif /* _IDL_ExtendedNaming_ExtendedNamingContext_defined */
		#ifndef _IDL_SEQUENCE_ExtendedNaming_ExtendedNamingContext_defined
			#define _IDL_SEQUENCE_ExtendedNaming_ExtendedNamingContext_defined
			SOM_SEQUENCE_TYPEDEF_NAME(ExtendedNaming_ExtendedNamingContext SOMSTAR ,sequence(ExtendedNaming_ExtendedNamingContext));
		#endif /* _IDL_SEQUENCE_ExtendedNaming_ExtendedNamingContext_defined */
		#ifndef _IDL_SEQUENCE_TypeCode_defined
			#define _IDL_SEQUENCE_TypeCode_defined
			SOM_SEQUENCE_TYPEDEF(TypeCode);
		#endif /* _IDL_SEQUENCE_TypeCode_defined */
		typedef _IDL_SEQUENCE_TypeCode ExtendedNaming_kludge;
		#ifndef _IDL_SEQUENCE_ExtendedNaming_kludge_defined
			#define _IDL_SEQUENCE_ExtendedNaming_kludge_defined
			SOM_SEQUENCE_TYPEDEF(ExtendedNaming_kludge);
		#endif /* _IDL_SEQUENCE_ExtendedNaming_kludge_defined */
		typedef _IDL_SEQUENCE_TypeCode ExtendedNaming_TypeCodeSeq;
		#ifndef _IDL_SEQUENCE_ExtendedNaming_TypeCodeSeq_defined
			#define _IDL_SEQUENCE_ExtendedNaming_TypeCodeSeq_defined
			SOM_SEQUENCE_TYPEDEF(ExtendedNaming_TypeCodeSeq);
		#endif /* _IDL_SEQUENCE_ExtendedNaming_TypeCodeSeq_defined */
		#ifndef _IDL_SEQUENCE_string_defined
			#define _IDL_SEQUENCE_string_defined
			SOM_SEQUENCE_TYPEDEF(string);
		#endif /* _IDL_SEQUENCE_string_defined */
		typedef _IDL_SEQUENCE_string ExtendedNaming_IList;
		#ifndef _IDL_SEQUENCE_ExtendedNaming_IList_defined
			#define _IDL_SEQUENCE_ExtendedNaming_IList_defined
			SOM_SEQUENCE_TYPEDEF(ExtendedNaming_IList);
		#endif /* _IDL_SEQUENCE_ExtendedNaming_IList_defined */
		typedef struct ExtendedNaming_PropertyBinding_struct
		{
			CosNaming_Istring property_name;
			boolean sharable;
		} ExtendedNaming_PropertyBinding_struct;
		#ifndef _IDL_SEQUENCE_ExtendedNaming_PropertyBinding_struct_defined
			#define _IDL_SEQUENCE_ExtendedNaming_PropertyBinding_struct_defined
			SOM_SEQUENCE_TYPEDEF(ExtendedNaming_PropertyBinding_struct);
		#endif /* _IDL_SEQUENCE_ExtendedNaming_PropertyBinding_struct_defined */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD_PropertyBinding_struct
				#if defined(PropertyBinding_struct)
					#undef PropertyBinding_struct
					#define SOMGD_PropertyBinding_struct
				#else
					#define PropertyBinding_struct ExtendedNaming_PropertyBinding_struct
				#endif
			#endif /* SOMGD_PropertyBinding_struct */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef _IDL_SEQUENCE_PropertyBinding_struct_defined
				#define _IDL_SEQUENCE_PropertyBinding_struct_defined
				#define _IDL_SEQUENCE_PropertyBinding_struct _IDL_SEQUENCE_ExtendedNaming_PropertyBinding_struct
			#endif /* _IDL_SEQUENCE_PropertyBinding_struct_defined */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		typedef ExtendedNaming_PropertyBinding_struct ExtendedNaming_PropertyBinding;
		typedef _IDL_SEQUENCE_ExtendedNaming_PropertyBinding_struct ExtendedNaming_PropertyBindingList;
		#ifndef _IDL_SEQUENCE_ExtendedNaming_PropertyBindingList_defined
			#define _IDL_SEQUENCE_ExtendedNaming_PropertyBindingList_defined
			SOM_SEQUENCE_TYPEDEF(ExtendedNaming_PropertyBindingList);
		#endif /* _IDL_SEQUENCE_ExtendedNaming_PropertyBindingList_defined */
		typedef struct ExtendedNaming_Property_struct
		{
			ExtendedNaming_PropertyBinding binding;
			any value;
		} ExtendedNaming_Property_struct;
		#ifndef _IDL_SEQUENCE_ExtendedNaming_Property_struct_defined
			#define _IDL_SEQUENCE_ExtendedNaming_Property_struct_defined
			SOM_SEQUENCE_TYPEDEF(ExtendedNaming_Property_struct);
		#endif /* _IDL_SEQUENCE_ExtendedNaming_Property_struct_defined */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD_Property_struct
				#if defined(Property_struct)
					#undef Property_struct
					#define SOMGD_Property_struct
				#else
					#define Property_struct ExtendedNaming_Property_struct
				#endif
			#endif /* SOMGD_Property_struct */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef _IDL_SEQUENCE_Property_struct_defined
				#define _IDL_SEQUENCE_Property_struct_defined
				#define _IDL_SEQUENCE_Property_struct _IDL_SEQUENCE_ExtendedNaming_Property_struct
			#endif /* _IDL_SEQUENCE_Property_struct_defined */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		typedef ExtendedNaming_Property_struct ExtendedNaming_Property;
		typedef _IDL_SEQUENCE_ExtendedNaming_Property_struct ExtendedNaming_PropertyList;
		#ifndef _IDL_SEQUENCE_ExtendedNaming_PropertyList_defined
			#define _IDL_SEQUENCE_ExtendedNaming_PropertyList_defined
			SOM_SEQUENCE_TYPEDEF(ExtendedNaming_PropertyList);
		#endif /* _IDL_SEQUENCE_ExtendedNaming_PropertyList_defined */
		typedef struct ExtendedNaming_IndexDescriptor_struct
		{
			CosNaming_Istring property_name;
			TypeCode property_type;
			unsigned long distance;
		} ExtendedNaming_IndexDescriptor_struct;
		#ifndef _IDL_SEQUENCE_ExtendedNaming_IndexDescriptor_struct_defined
			#define _IDL_SEQUENCE_ExtendedNaming_IndexDescriptor_struct_defined
			SOM_SEQUENCE_TYPEDEF(ExtendedNaming_IndexDescriptor_struct);
		#endif /* _IDL_SEQUENCE_ExtendedNaming_IndexDescriptor_struct_defined */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD_IndexDescriptor_struct
				#if defined(IndexDescriptor_struct)
					#undef IndexDescriptor_struct
					#define SOMGD_IndexDescriptor_struct
				#else
					#define IndexDescriptor_struct ExtendedNaming_IndexDescriptor_struct
				#endif
			#endif /* SOMGD_IndexDescriptor_struct */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef _IDL_SEQUENCE_IndexDescriptor_struct_defined
				#define _IDL_SEQUENCE_IndexDescriptor_struct_defined
				#define _IDL_SEQUENCE_IndexDescriptor_struct _IDL_SEQUENCE_ExtendedNaming_IndexDescriptor_struct
			#endif /* _IDL_SEQUENCE_IndexDescriptor_struct_defined */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
		typedef ExtendedNaming_IndexDescriptor_struct ExtendedNaming_IndexDescriptor;
		typedef _IDL_SEQUENCE_ExtendedNaming_IndexDescriptor_struct ExtendedNaming_IndexDescriptorList;
		#ifndef _IDL_SEQUENCE_ExtendedNaming_IndexDescriptorList_defined
			#define _IDL_SEQUENCE_ExtendedNaming_IndexDescriptorList_defined
			SOM_SEQUENCE_TYPEDEF(ExtendedNaming_IndexDescriptorList);
		#endif /* _IDL_SEQUENCE_ExtendedNaming_IndexDescriptorList_defined */
		#ifdef __IBMC__
			typedef boolean (somTP_ExtendedNaming_PropertyBindingIterator_next_one)(
				ExtendedNaming_PropertyBindingIterator SOMSTAR somSelf,
				Environment *ev,
				/* out */ ExtendedNaming_PropertyBinding *pb);
			#pragma linkage(somTP_ExtendedNaming_PropertyBindingIterator_next_one,system)
			typedef somTP_ExtendedNaming_PropertyBindingIterator_next_one *somTD_ExtendedNaming_PropertyBindingIterator_next_one;
		#else /* __IBMC__ */
			typedef boolean (SOMLINK * somTD_ExtendedNaming_PropertyBindingIterator_next_one)(
				ExtendedNaming_PropertyBindingIterator SOMSTAR somSelf,
				Environment *ev,
				/* out */ ExtendedNaming_PropertyBinding *pb);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef boolean (somTP_ExtendedNaming_PropertyBindingIterator_next_n)(
				ExtendedNaming_PropertyBindingIterator SOMSTAR somSelf,
				Environment *ev,
				/* in */ unsigned long howMany,
				/* out */ ExtendedNaming_PropertyBindingList *il);
			#pragma linkage(somTP_ExtendedNaming_PropertyBindingIterator_next_n,system)
			typedef somTP_ExtendedNaming_PropertyBindingIterator_next_n *somTD_ExtendedNaming_PropertyBindingIterator_next_n;
		#else /* __IBMC__ */
			typedef boolean (SOMLINK * somTD_ExtendedNaming_PropertyBindingIterator_next_n)(
				ExtendedNaming_PropertyBindingIterator SOMSTAR somSelf,
				Environment *ev,
				/* in */ unsigned long howMany,
				/* out */ ExtendedNaming_PropertyBindingList *il);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_ExtendedNaming_PropertyBindingIterator_destroy)(
				ExtendedNaming_PropertyBindingIterator SOMSTAR somSelf,
				Environment *ev);
			#pragma linkage(somTP_ExtendedNaming_PropertyBindingIterator_destroy,system)
			typedef somTP_ExtendedNaming_PropertyBindingIterator_destroy *somTD_ExtendedNaming_PropertyBindingIterator_destroy;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_ExtendedNaming_PropertyBindingIterator_destroy)(
				ExtendedNaming_PropertyBindingIterator SOMSTAR somSelf,
				Environment *ev);
		#endif /* __IBMC__ */
		#ifndef ExtendedNaming_PropertyBindingIterator_MajorVersion
			#define ExtendedNaming_PropertyBindingIterator_MajorVersion   3
		#endif /* ExtendedNaming_PropertyBindingIterator_MajorVersion */
		#ifndef ExtendedNaming_PropertyBindingIterator_MinorVersion
			#define ExtendedNaming_PropertyBindingIterator_MinorVersion   0
		#endif /* ExtendedNaming_PropertyBindingIterator_MinorVersion */
		typedef struct ExtendedNaming_PropertyBindingIteratorClassDataStructure
		{
			SOMClass SOMSTAR classObject;
			somMToken next_one;
			somMToken next_n;
			somMToken destroy;
		} ExtendedNaming_PropertyBindingIteratorClassDataStructure;
		typedef struct ExtendedNaming_PropertyBindingIteratorCClassDataStructure
		{
			somMethodTabs parentMtab;
			somDToken instanceDataToken;
		} ExtendedNaming_PropertyBindingIteratorCClassDataStructure;
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_xnaming_Source) || defined(ExtendedNaming_PropertyBindingIterator_Class_Source)
				SOMEXTERN struct ExtendedNaming_PropertyBindingIteratorClassDataStructure _ExtendedNaming_PropertyBindingIteratorClassData;
				#ifndef ExtendedNaming_PropertyBindingIteratorClassData
					#define ExtendedNaming_PropertyBindingIteratorClassData    _ExtendedNaming_PropertyBindingIteratorClassData
				#endif /* ExtendedNaming_PropertyBindingIteratorClassData */
			#else
				SOMEXTERN struct ExtendedNaming_PropertyBindingIteratorClassDataStructure * SOMLINK resolve_ExtendedNaming_PropertyBindingIteratorClassData(void);
				#ifndef ExtendedNaming_PropertyBindingIteratorClassData
					#define ExtendedNaming_PropertyBindingIteratorClassData    (*(resolve_ExtendedNaming_PropertyBindingIteratorClassData()))
				#endif /* ExtendedNaming_PropertyBindingIteratorClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_xnaming_Source) || defined(ExtendedNaming_PropertyBindingIterator_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_xnaming_Source || ExtendedNaming_PropertyBindingIterator_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_xnaming_Source || ExtendedNaming_PropertyBindingIterator_Class_Source */
			struct ExtendedNaming_PropertyBindingIteratorClassDataStructure SOMDLINK ExtendedNaming_PropertyBindingIteratorClassData;
		#endif /* SOM_RESOLVE_DATA */
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_xnaming_Source) || defined(ExtendedNaming_PropertyBindingIterator_Class_Source)
				SOMEXTERN struct ExtendedNaming_PropertyBindingIteratorCClassDataStructure _ExtendedNaming_PropertyBindingIteratorCClassData;
				#ifndef ExtendedNaming_PropertyBindingIteratorCClassData
					#define ExtendedNaming_PropertyBindingIteratorCClassData    _ExtendedNaming_PropertyBindingIteratorCClassData
				#endif /* ExtendedNaming_PropertyBindingIteratorCClassData */
			#else
				SOMEXTERN struct ExtendedNaming_PropertyBindingIteratorCClassDataStructure * SOMLINK resolve_ExtendedNaming_PropertyBindingIteratorCClassData(void);
				#ifndef ExtendedNaming_PropertyBindingIteratorCClassData
					#define ExtendedNaming_PropertyBindingIteratorCClassData    (*(resolve_ExtendedNaming_PropertyBindingIteratorCClassData()))
				#endif /* ExtendedNaming_PropertyBindingIteratorCClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_xnaming_Source) || defined(ExtendedNaming_PropertyBindingIterator_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_xnaming_Source || ExtendedNaming_PropertyBindingIterator_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_xnaming_Source || ExtendedNaming_PropertyBindingIterator_Class_Source */
			struct ExtendedNaming_PropertyBindingIteratorCClassDataStructure SOMDLINK ExtendedNaming_PropertyBindingIteratorCClassData;
		#endif /* SOM_RESOLVE_DATA */
		extern 
		#if defined(SOM_Module_xnaming_Source) || defined(ExtendedNaming_PropertyBindingIterator_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_xnaming_Source || ExtendedNaming_PropertyBindingIterator_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_xnaming_Source || ExtendedNaming_PropertyBindingIterator_Class_Source */
		SOMClass SOMSTAR SOMLINK ExtendedNaming_PropertyBindingIteratorNewClass
				(integer4 somtmajorVersion,integer4 somtminorVersion);
		#define _SOMCLASS_ExtendedNaming_PropertyBindingIterator (ExtendedNaming_PropertyBindingIteratorClassData.classObject)
		#ifndef SOMGD_ExtendedNaming_PropertyBindingIterator
			#if (defined(_ExtendedNaming_PropertyBindingIterator) || defined(__ExtendedNaming_PropertyBindingIterator))
				#undef _ExtendedNaming_PropertyBindingIterator
				#undef __ExtendedNaming_PropertyBindingIterator
				#define SOMGD_ExtendedNaming_PropertyBindingIterator 1
			#else
				#define _ExtendedNaming_PropertyBindingIterator _SOMCLASS_ExtendedNaming_PropertyBindingIterator
			#endif /* _ExtendedNaming_PropertyBindingIterator */
		#endif /* SOMGD_ExtendedNaming_PropertyBindingIterator */
		#define ExtendedNaming_PropertyBindingIterator_classObj _SOMCLASS_ExtendedNaming_PropertyBindingIterator
		#define _SOMMTOKEN_ExtendedNaming_PropertyBindingIterator(method) ((somMToken)(ExtendedNaming_PropertyBindingIteratorClassData.method))
		#ifndef ExtendedNaming_PropertyBindingIteratorNew
			#define ExtendedNaming_PropertyBindingIteratorNew() ( _ExtendedNaming_PropertyBindingIterator ? \
				(SOMClass_somNew(_ExtendedNaming_PropertyBindingIterator)) : \
				( ExtendedNaming_PropertyBindingIteratorNewClass( \
					ExtendedNaming_PropertyBindingIterator_MajorVersion, \
					ExtendedNaming_PropertyBindingIterator_MinorVersion), \
				SOMClass_somNew(_ExtendedNaming_PropertyBindingIterator))) 
		#endif /* NewExtendedNaming_PropertyBindingIterator */
		#ifndef ExtendedNaming_PropertyBindingIterator_next_one
			#define ExtendedNaming_PropertyBindingIterator_next_one(somSelf,ev,pb) \
				SOM_Resolve(somSelf,ExtendedNaming_PropertyBindingIterator,next_one)  \
					(somSelf,ev,pb)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__next_one
					#if defined(_next_one)
						#undef _next_one
						#define SOMGD__next_one
					#else
						#define _next_one ExtendedNaming_PropertyBindingIterator_next_one
					#endif
				#endif /* SOMGD__next_one */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* ExtendedNaming_PropertyBindingIterator_next_one */
		#ifndef ExtendedNaming_PropertyBindingIterator_next_n
			#define ExtendedNaming_PropertyBindingIterator_next_n(somSelf,ev,howMany,il) \
				SOM_Resolve(somSelf,ExtendedNaming_PropertyBindingIterator,next_n)  \
					(somSelf,ev,howMany,il)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__next_n
					#if defined(_next_n)
						#undef _next_n
						#define SOMGD__next_n
					#else
						#define _next_n ExtendedNaming_PropertyBindingIterator_next_n
					#endif
				#endif /* SOMGD__next_n */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* ExtendedNaming_PropertyBindingIterator_next_n */
		#ifndef ExtendedNaming_PropertyBindingIterator_destroy
			#define ExtendedNaming_PropertyBindingIterator_destroy(somSelf,ev) \
				SOM_Resolve(somSelf,ExtendedNaming_PropertyBindingIterator,destroy)  \
					(somSelf,ev)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__destroy
					#if defined(_destroy)
						#undef _destroy
						#define SOMGD__destroy
					#else
						#define _destroy ExtendedNaming_PropertyBindingIterator_destroy
					#endif
				#endif /* SOMGD__destroy */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* ExtendedNaming_PropertyBindingIterator_destroy */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define ExtendedNaming_PropertyBindingIterator_somInit SOMObject_somInit
#define ExtendedNaming_PropertyBindingIterator_somUninit SOMObject_somUninit
#define ExtendedNaming_PropertyBindingIterator_somFree SOMObject_somFree
#define ExtendedNaming_PropertyBindingIterator_somGetClass SOMObject_somGetClass
#define ExtendedNaming_PropertyBindingIterator_somGetClassName SOMObject_somGetClassName
#define ExtendedNaming_PropertyBindingIterator_somGetSize SOMObject_somGetSize
#define ExtendedNaming_PropertyBindingIterator_somIsA SOMObject_somIsA
#define ExtendedNaming_PropertyBindingIterator_somIsInstanceOf SOMObject_somIsInstanceOf
#define ExtendedNaming_PropertyBindingIterator_somRespondsTo SOMObject_somRespondsTo
#define ExtendedNaming_PropertyBindingIterator_somDispatch SOMObject_somDispatch
#define ExtendedNaming_PropertyBindingIterator_somClassDispatch SOMObject_somClassDispatch
#define ExtendedNaming_PropertyBindingIterator_somCastObj SOMObject_somCastObj
#define ExtendedNaming_PropertyBindingIterator_somResetObj SOMObject_somResetObj
#define ExtendedNaming_PropertyBindingIterator_somPrintSelf SOMObject_somPrintSelf
#define ExtendedNaming_PropertyBindingIterator_somDumpSelf SOMObject_somDumpSelf
#define ExtendedNaming_PropertyBindingIterator_somDumpSelfInt SOMObject_somDumpSelfInt
#define ExtendedNaming_PropertyBindingIterator_somDefaultInit SOMObject_somDefaultInit
#define ExtendedNaming_PropertyBindingIterator_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define ExtendedNaming_PropertyBindingIterator_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define ExtendedNaming_PropertyBindingIterator_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define ExtendedNaming_PropertyBindingIterator_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define ExtendedNaming_PropertyBindingIterator_somDefaultAssign SOMObject_somDefaultAssign
#define ExtendedNaming_PropertyBindingIterator_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define ExtendedNaming_PropertyBindingIterator_somDefaultVAssign SOMObject_somDefaultVAssign
#define ExtendedNaming_PropertyBindingIterator_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define ExtendedNaming_PropertyBindingIterator_somDestruct SOMObject_somDestruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

		#ifdef __IBMC__
			typedef boolean (somTP_ExtendedNaming_PropertyIterator_next_one)(
				ExtendedNaming_PropertyIterator SOMSTAR somSelf,
				Environment *ev,
				/* out */ ExtendedNaming_Property *p);
			#pragma linkage(somTP_ExtendedNaming_PropertyIterator_next_one,system)
			typedef somTP_ExtendedNaming_PropertyIterator_next_one *somTD_ExtendedNaming_PropertyIterator_next_one;
		#else /* __IBMC__ */
			typedef boolean (SOMLINK * somTD_ExtendedNaming_PropertyIterator_next_one)(
				ExtendedNaming_PropertyIterator SOMSTAR somSelf,
				Environment *ev,
				/* out */ ExtendedNaming_Property *p);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef boolean (somTP_ExtendedNaming_PropertyIterator_next_n)(
				ExtendedNaming_PropertyIterator SOMSTAR somSelf,
				Environment *ev,
				/* in */ unsigned long howMany,
				/* out */ ExtendedNaming_PropertyList *pl);
			#pragma linkage(somTP_ExtendedNaming_PropertyIterator_next_n,system)
			typedef somTP_ExtendedNaming_PropertyIterator_next_n *somTD_ExtendedNaming_PropertyIterator_next_n;
		#else /* __IBMC__ */
			typedef boolean (SOMLINK * somTD_ExtendedNaming_PropertyIterator_next_n)(
				ExtendedNaming_PropertyIterator SOMSTAR somSelf,
				Environment *ev,
				/* in */ unsigned long howMany,
				/* out */ ExtendedNaming_PropertyList *pl);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_ExtendedNaming_PropertyIterator_destroy)(
				ExtendedNaming_PropertyIterator SOMSTAR somSelf,
				Environment *ev);
			#pragma linkage(somTP_ExtendedNaming_PropertyIterator_destroy,system)
			typedef somTP_ExtendedNaming_PropertyIterator_destroy *somTD_ExtendedNaming_PropertyIterator_destroy;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_ExtendedNaming_PropertyIterator_destroy)(
				ExtendedNaming_PropertyIterator SOMSTAR somSelf,
				Environment *ev);
		#endif /* __IBMC__ */
		#ifndef ExtendedNaming_PropertyIterator_MajorVersion
			#define ExtendedNaming_PropertyIterator_MajorVersion   3
		#endif /* ExtendedNaming_PropertyIterator_MajorVersion */
		#ifndef ExtendedNaming_PropertyIterator_MinorVersion
			#define ExtendedNaming_PropertyIterator_MinorVersion   0
		#endif /* ExtendedNaming_PropertyIterator_MinorVersion */
		typedef struct ExtendedNaming_PropertyIteratorClassDataStructure
		{
			SOMClass SOMSTAR classObject;
			somMToken next_one;
			somMToken next_n;
			somMToken destroy;
		} ExtendedNaming_PropertyIteratorClassDataStructure;
		typedef struct ExtendedNaming_PropertyIteratorCClassDataStructure
		{
			somMethodTabs parentMtab;
			somDToken instanceDataToken;
		} ExtendedNaming_PropertyIteratorCClassDataStructure;
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_xnaming_Source) || defined(ExtendedNaming_PropertyIterator_Class_Source)
				SOMEXTERN struct ExtendedNaming_PropertyIteratorClassDataStructure _ExtendedNaming_PropertyIteratorClassData;
				#ifndef ExtendedNaming_PropertyIteratorClassData
					#define ExtendedNaming_PropertyIteratorClassData    _ExtendedNaming_PropertyIteratorClassData
				#endif /* ExtendedNaming_PropertyIteratorClassData */
			#else
				SOMEXTERN struct ExtendedNaming_PropertyIteratorClassDataStructure * SOMLINK resolve_ExtendedNaming_PropertyIteratorClassData(void);
				#ifndef ExtendedNaming_PropertyIteratorClassData
					#define ExtendedNaming_PropertyIteratorClassData    (*(resolve_ExtendedNaming_PropertyIteratorClassData()))
				#endif /* ExtendedNaming_PropertyIteratorClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_xnaming_Source) || defined(ExtendedNaming_PropertyIterator_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_xnaming_Source || ExtendedNaming_PropertyIterator_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_xnaming_Source || ExtendedNaming_PropertyIterator_Class_Source */
			struct ExtendedNaming_PropertyIteratorClassDataStructure SOMDLINK ExtendedNaming_PropertyIteratorClassData;
		#endif /* SOM_RESOLVE_DATA */
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_xnaming_Source) || defined(ExtendedNaming_PropertyIterator_Class_Source)
				SOMEXTERN struct ExtendedNaming_PropertyIteratorCClassDataStructure _ExtendedNaming_PropertyIteratorCClassData;
				#ifndef ExtendedNaming_PropertyIteratorCClassData
					#define ExtendedNaming_PropertyIteratorCClassData    _ExtendedNaming_PropertyIteratorCClassData
				#endif /* ExtendedNaming_PropertyIteratorCClassData */
			#else
				SOMEXTERN struct ExtendedNaming_PropertyIteratorCClassDataStructure * SOMLINK resolve_ExtendedNaming_PropertyIteratorCClassData(void);
				#ifndef ExtendedNaming_PropertyIteratorCClassData
					#define ExtendedNaming_PropertyIteratorCClassData    (*(resolve_ExtendedNaming_PropertyIteratorCClassData()))
				#endif /* ExtendedNaming_PropertyIteratorCClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_xnaming_Source) || defined(ExtendedNaming_PropertyIterator_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_xnaming_Source || ExtendedNaming_PropertyIterator_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_xnaming_Source || ExtendedNaming_PropertyIterator_Class_Source */
			struct ExtendedNaming_PropertyIteratorCClassDataStructure SOMDLINK ExtendedNaming_PropertyIteratorCClassData;
		#endif /* SOM_RESOLVE_DATA */
		extern 
		#if defined(SOM_Module_xnaming_Source) || defined(ExtendedNaming_PropertyIterator_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_xnaming_Source || ExtendedNaming_PropertyIterator_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_xnaming_Source || ExtendedNaming_PropertyIterator_Class_Source */
		SOMClass SOMSTAR SOMLINK ExtendedNaming_PropertyIteratorNewClass
				(integer4 somtmajorVersion,integer4 somtminorVersion);
		#define _SOMCLASS_ExtendedNaming_PropertyIterator (ExtendedNaming_PropertyIteratorClassData.classObject)
		#ifndef SOMGD_ExtendedNaming_PropertyIterator
			#if (defined(_ExtendedNaming_PropertyIterator) || defined(__ExtendedNaming_PropertyIterator))
				#undef _ExtendedNaming_PropertyIterator
				#undef __ExtendedNaming_PropertyIterator
				#define SOMGD_ExtendedNaming_PropertyIterator 1
			#else
				#define _ExtendedNaming_PropertyIterator _SOMCLASS_ExtendedNaming_PropertyIterator
			#endif /* _ExtendedNaming_PropertyIterator */
		#endif /* SOMGD_ExtendedNaming_PropertyIterator */
		#define ExtendedNaming_PropertyIterator_classObj _SOMCLASS_ExtendedNaming_PropertyIterator
		#define _SOMMTOKEN_ExtendedNaming_PropertyIterator(method) ((somMToken)(ExtendedNaming_PropertyIteratorClassData.method))
		#ifndef ExtendedNaming_PropertyIteratorNew
			#define ExtendedNaming_PropertyIteratorNew() ( _ExtendedNaming_PropertyIterator ? \
				(SOMClass_somNew(_ExtendedNaming_PropertyIterator)) : \
				( ExtendedNaming_PropertyIteratorNewClass( \
					ExtendedNaming_PropertyIterator_MajorVersion, \
					ExtendedNaming_PropertyIterator_MinorVersion), \
				SOMClass_somNew(_ExtendedNaming_PropertyIterator))) 
		#endif /* NewExtendedNaming_PropertyIterator */
		#ifndef ExtendedNaming_PropertyIterator_next_one
			#define ExtendedNaming_PropertyIterator_next_one(somSelf,ev,p) \
				SOM_Resolve(somSelf,ExtendedNaming_PropertyIterator,next_one)  \
					(somSelf,ev,p)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__next_one
					#if defined(_next_one)
						#undef _next_one
						#define SOMGD__next_one
					#else
						#define _next_one ExtendedNaming_PropertyIterator_next_one
					#endif
				#endif /* SOMGD__next_one */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* ExtendedNaming_PropertyIterator_next_one */
		#ifndef ExtendedNaming_PropertyIterator_next_n
			#define ExtendedNaming_PropertyIterator_next_n(somSelf,ev,howMany,pl) \
				SOM_Resolve(somSelf,ExtendedNaming_PropertyIterator,next_n)  \
					(somSelf,ev,howMany,pl)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__next_n
					#if defined(_next_n)
						#undef _next_n
						#define SOMGD__next_n
					#else
						#define _next_n ExtendedNaming_PropertyIterator_next_n
					#endif
				#endif /* SOMGD__next_n */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* ExtendedNaming_PropertyIterator_next_n */
		#ifndef ExtendedNaming_PropertyIterator_destroy
			#define ExtendedNaming_PropertyIterator_destroy(somSelf,ev) \
				SOM_Resolve(somSelf,ExtendedNaming_PropertyIterator,destroy)  \
					(somSelf,ev)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__destroy
					#if defined(_destroy)
						#undef _destroy
						#define SOMGD__destroy
					#else
						#define _destroy ExtendedNaming_PropertyIterator_destroy
					#endif
				#endif /* SOMGD__destroy */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* ExtendedNaming_PropertyIterator_destroy */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define ExtendedNaming_PropertyIterator_somInit SOMObject_somInit
#define ExtendedNaming_PropertyIterator_somUninit SOMObject_somUninit
#define ExtendedNaming_PropertyIterator_somFree SOMObject_somFree
#define ExtendedNaming_PropertyIterator_somGetClass SOMObject_somGetClass
#define ExtendedNaming_PropertyIterator_somGetClassName SOMObject_somGetClassName
#define ExtendedNaming_PropertyIterator_somGetSize SOMObject_somGetSize
#define ExtendedNaming_PropertyIterator_somIsA SOMObject_somIsA
#define ExtendedNaming_PropertyIterator_somIsInstanceOf SOMObject_somIsInstanceOf
#define ExtendedNaming_PropertyIterator_somRespondsTo SOMObject_somRespondsTo
#define ExtendedNaming_PropertyIterator_somDispatch SOMObject_somDispatch
#define ExtendedNaming_PropertyIterator_somClassDispatch SOMObject_somClassDispatch
#define ExtendedNaming_PropertyIterator_somCastObj SOMObject_somCastObj
#define ExtendedNaming_PropertyIterator_somResetObj SOMObject_somResetObj
#define ExtendedNaming_PropertyIterator_somPrintSelf SOMObject_somPrintSelf
#define ExtendedNaming_PropertyIterator_somDumpSelf SOMObject_somDumpSelf
#define ExtendedNaming_PropertyIterator_somDumpSelfInt SOMObject_somDumpSelfInt
#define ExtendedNaming_PropertyIterator_somDefaultInit SOMObject_somDefaultInit
#define ExtendedNaming_PropertyIterator_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define ExtendedNaming_PropertyIterator_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define ExtendedNaming_PropertyIterator_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define ExtendedNaming_PropertyIterator_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define ExtendedNaming_PropertyIterator_somDefaultAssign SOMObject_somDefaultAssign
#define ExtendedNaming_PropertyIterator_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define ExtendedNaming_PropertyIterator_somDefaultVAssign SOMObject_somDefaultVAssign
#define ExtendedNaming_PropertyIterator_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define ExtendedNaming_PropertyIterator_somDestruct SOMObject_somDestruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

		#ifdef __IBMC__
			typedef boolean (somTP_ExtendedNaming_IndexIterator_next_one)(
				ExtendedNaming_IndexIterator SOMSTAR somSelf,
				Environment *ev,
				/* out */ ExtendedNaming_IndexDescriptor *p);
			#pragma linkage(somTP_ExtendedNaming_IndexIterator_next_one,system)
			typedef somTP_ExtendedNaming_IndexIterator_next_one *somTD_ExtendedNaming_IndexIterator_next_one;
		#else /* __IBMC__ */
			typedef boolean (SOMLINK * somTD_ExtendedNaming_IndexIterator_next_one)(
				ExtendedNaming_IndexIterator SOMSTAR somSelf,
				Environment *ev,
				/* out */ ExtendedNaming_IndexDescriptor *p);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef boolean (somTP_ExtendedNaming_IndexIterator_next_n)(
				ExtendedNaming_IndexIterator SOMSTAR somSelf,
				Environment *ev,
				/* in */ unsigned long howMany,
				/* out */ ExtendedNaming_IndexDescriptorList *il);
			#pragma linkage(somTP_ExtendedNaming_IndexIterator_next_n,system)
			typedef somTP_ExtendedNaming_IndexIterator_next_n *somTD_ExtendedNaming_IndexIterator_next_n;
		#else /* __IBMC__ */
			typedef boolean (SOMLINK * somTD_ExtendedNaming_IndexIterator_next_n)(
				ExtendedNaming_IndexIterator SOMSTAR somSelf,
				Environment *ev,
				/* in */ unsigned long howMany,
				/* out */ ExtendedNaming_IndexDescriptorList *il);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_ExtendedNaming_IndexIterator_destroy)(
				ExtendedNaming_IndexIterator SOMSTAR somSelf,
				Environment *ev);
			#pragma linkage(somTP_ExtendedNaming_IndexIterator_destroy,system)
			typedef somTP_ExtendedNaming_IndexIterator_destroy *somTD_ExtendedNaming_IndexIterator_destroy;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_ExtendedNaming_IndexIterator_destroy)(
				ExtendedNaming_IndexIterator SOMSTAR somSelf,
				Environment *ev);
		#endif /* __IBMC__ */
		#ifndef ExtendedNaming_IndexIterator_MajorVersion
			#define ExtendedNaming_IndexIterator_MajorVersion   3
		#endif /* ExtendedNaming_IndexIterator_MajorVersion */
		#ifndef ExtendedNaming_IndexIterator_MinorVersion
			#define ExtendedNaming_IndexIterator_MinorVersion   0
		#endif /* ExtendedNaming_IndexIterator_MinorVersion */
		typedef struct ExtendedNaming_IndexIteratorClassDataStructure
		{
			SOMClass SOMSTAR classObject;
			somMToken next_one;
			somMToken next_n;
			somMToken destroy;
		} ExtendedNaming_IndexIteratorClassDataStructure;
		typedef struct ExtendedNaming_IndexIteratorCClassDataStructure
		{
			somMethodTabs parentMtab;
			somDToken instanceDataToken;
		} ExtendedNaming_IndexIteratorCClassDataStructure;
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_xnaming_Source) || defined(ExtendedNaming_IndexIterator_Class_Source)
				SOMEXTERN struct ExtendedNaming_IndexIteratorClassDataStructure _ExtendedNaming_IndexIteratorClassData;
				#ifndef ExtendedNaming_IndexIteratorClassData
					#define ExtendedNaming_IndexIteratorClassData    _ExtendedNaming_IndexIteratorClassData
				#endif /* ExtendedNaming_IndexIteratorClassData */
			#else
				SOMEXTERN struct ExtendedNaming_IndexIteratorClassDataStructure * SOMLINK resolve_ExtendedNaming_IndexIteratorClassData(void);
				#ifndef ExtendedNaming_IndexIteratorClassData
					#define ExtendedNaming_IndexIteratorClassData    (*(resolve_ExtendedNaming_IndexIteratorClassData()))
				#endif /* ExtendedNaming_IndexIteratorClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_xnaming_Source) || defined(ExtendedNaming_IndexIterator_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_xnaming_Source || ExtendedNaming_IndexIterator_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_xnaming_Source || ExtendedNaming_IndexIterator_Class_Source */
			struct ExtendedNaming_IndexIteratorClassDataStructure SOMDLINK ExtendedNaming_IndexIteratorClassData;
		#endif /* SOM_RESOLVE_DATA */
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_xnaming_Source) || defined(ExtendedNaming_IndexIterator_Class_Source)
				SOMEXTERN struct ExtendedNaming_IndexIteratorCClassDataStructure _ExtendedNaming_IndexIteratorCClassData;
				#ifndef ExtendedNaming_IndexIteratorCClassData
					#define ExtendedNaming_IndexIteratorCClassData    _ExtendedNaming_IndexIteratorCClassData
				#endif /* ExtendedNaming_IndexIteratorCClassData */
			#else
				SOMEXTERN struct ExtendedNaming_IndexIteratorCClassDataStructure * SOMLINK resolve_ExtendedNaming_IndexIteratorCClassData(void);
				#ifndef ExtendedNaming_IndexIteratorCClassData
					#define ExtendedNaming_IndexIteratorCClassData    (*(resolve_ExtendedNaming_IndexIteratorCClassData()))
				#endif /* ExtendedNaming_IndexIteratorCClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_xnaming_Source) || defined(ExtendedNaming_IndexIterator_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_xnaming_Source || ExtendedNaming_IndexIterator_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_xnaming_Source || ExtendedNaming_IndexIterator_Class_Source */
			struct ExtendedNaming_IndexIteratorCClassDataStructure SOMDLINK ExtendedNaming_IndexIteratorCClassData;
		#endif /* SOM_RESOLVE_DATA */
		extern 
		#if defined(SOM_Module_xnaming_Source) || defined(ExtendedNaming_IndexIterator_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_xnaming_Source || ExtendedNaming_IndexIterator_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_xnaming_Source || ExtendedNaming_IndexIterator_Class_Source */
		SOMClass SOMSTAR SOMLINK ExtendedNaming_IndexIteratorNewClass
				(integer4 somtmajorVersion,integer4 somtminorVersion);
		#define _SOMCLASS_ExtendedNaming_IndexIterator (ExtendedNaming_IndexIteratorClassData.classObject)
		#ifndef SOMGD_ExtendedNaming_IndexIterator
			#if (defined(_ExtendedNaming_IndexIterator) || defined(__ExtendedNaming_IndexIterator))
				#undef _ExtendedNaming_IndexIterator
				#undef __ExtendedNaming_IndexIterator
				#define SOMGD_ExtendedNaming_IndexIterator 1
			#else
				#define _ExtendedNaming_IndexIterator _SOMCLASS_ExtendedNaming_IndexIterator
			#endif /* _ExtendedNaming_IndexIterator */
		#endif /* SOMGD_ExtendedNaming_IndexIterator */
		#define ExtendedNaming_IndexIterator_classObj _SOMCLASS_ExtendedNaming_IndexIterator
		#define _SOMMTOKEN_ExtendedNaming_IndexIterator(method) ((somMToken)(ExtendedNaming_IndexIteratorClassData.method))
		#ifndef ExtendedNaming_IndexIteratorNew
			#define ExtendedNaming_IndexIteratorNew() ( _ExtendedNaming_IndexIterator ? \
				(SOMClass_somNew(_ExtendedNaming_IndexIterator)) : \
				( ExtendedNaming_IndexIteratorNewClass( \
					ExtendedNaming_IndexIterator_MajorVersion, \
					ExtendedNaming_IndexIterator_MinorVersion), \
				SOMClass_somNew(_ExtendedNaming_IndexIterator))) 
		#endif /* NewExtendedNaming_IndexIterator */
		#ifndef ExtendedNaming_IndexIterator_next_one
			#define ExtendedNaming_IndexIterator_next_one(somSelf,ev,p) \
				SOM_Resolve(somSelf,ExtendedNaming_IndexIterator,next_one)  \
					(somSelf,ev,p)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__next_one
					#if defined(_next_one)
						#undef _next_one
						#define SOMGD__next_one
					#else
						#define _next_one ExtendedNaming_IndexIterator_next_one
					#endif
				#endif /* SOMGD__next_one */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* ExtendedNaming_IndexIterator_next_one */
		#ifndef ExtendedNaming_IndexIterator_next_n
			#define ExtendedNaming_IndexIterator_next_n(somSelf,ev,howMany,il) \
				SOM_Resolve(somSelf,ExtendedNaming_IndexIterator,next_n)  \
					(somSelf,ev,howMany,il)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__next_n
					#if defined(_next_n)
						#undef _next_n
						#define SOMGD__next_n
					#else
						#define _next_n ExtendedNaming_IndexIterator_next_n
					#endif
				#endif /* SOMGD__next_n */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* ExtendedNaming_IndexIterator_next_n */
		#ifndef ExtendedNaming_IndexIterator_destroy
			#define ExtendedNaming_IndexIterator_destroy(somSelf,ev) \
				SOM_Resolve(somSelf,ExtendedNaming_IndexIterator,destroy)  \
					(somSelf,ev)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__destroy
					#if defined(_destroy)
						#undef _destroy
						#define SOMGD__destroy
					#else
						#define _destroy ExtendedNaming_IndexIterator_destroy
					#endif
				#endif /* SOMGD__destroy */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* ExtendedNaming_IndexIterator_destroy */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define ExtendedNaming_IndexIterator_somInit SOMObject_somInit
#define ExtendedNaming_IndexIterator_somUninit SOMObject_somUninit
#define ExtendedNaming_IndexIterator_somFree SOMObject_somFree
#define ExtendedNaming_IndexIterator_somGetClass SOMObject_somGetClass
#define ExtendedNaming_IndexIterator_somGetClassName SOMObject_somGetClassName
#define ExtendedNaming_IndexIterator_somGetSize SOMObject_somGetSize
#define ExtendedNaming_IndexIterator_somIsA SOMObject_somIsA
#define ExtendedNaming_IndexIterator_somIsInstanceOf SOMObject_somIsInstanceOf
#define ExtendedNaming_IndexIterator_somRespondsTo SOMObject_somRespondsTo
#define ExtendedNaming_IndexIterator_somDispatch SOMObject_somDispatch
#define ExtendedNaming_IndexIterator_somClassDispatch SOMObject_somClassDispatch
#define ExtendedNaming_IndexIterator_somCastObj SOMObject_somCastObj
#define ExtendedNaming_IndexIterator_somResetObj SOMObject_somResetObj
#define ExtendedNaming_IndexIterator_somPrintSelf SOMObject_somPrintSelf
#define ExtendedNaming_IndexIterator_somDumpSelf SOMObject_somDumpSelf
#define ExtendedNaming_IndexIterator_somDumpSelfInt SOMObject_somDumpSelfInt
#define ExtendedNaming_IndexIterator_somDefaultInit SOMObject_somDefaultInit
#define ExtendedNaming_IndexIterator_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define ExtendedNaming_IndexIterator_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define ExtendedNaming_IndexIterator_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define ExtendedNaming_IndexIterator_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define ExtendedNaming_IndexIterator_somDefaultAssign SOMObject_somDefaultAssign
#define ExtendedNaming_IndexIterator_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define ExtendedNaming_IndexIterator_somDefaultVAssign SOMObject_somDefaultVAssign
#define ExtendedNaming_IndexIterator_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define ExtendedNaming_IndexIterator_somDestruct SOMObject_somDestruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

		typedef string ExtendedNaming_ExtendedNamingContext_Constraint;
		typedef char *ExtendedNaming_ExtendedNamingContext_Strings;
		#ifndef _IDL_SEQUENCE_ExtendedNaming_ExtendedNamingContext_Strings_defined
			#define _IDL_SEQUENCE_ExtendedNaming_ExtendedNamingContext_Strings_defined
			SOM_SEQUENCE_TYPEDEF(ExtendedNaming_ExtendedNamingContext_Strings);
		#endif /* _IDL_SEQUENCE_ExtendedNaming_ExtendedNamingContext_Strings_defined */
#define ex_ExtendedNaming_ExtendedNamingContext_InvalidPropertyName   "::ExtendedNaming::ExtendedNamingContext::InvalidPropertyName"
#ifndef SOM_DONT_USE_SHORT_NAMES
#ifndef SOMTGD_ex_InvalidPropertyName
#ifndef ex_InvalidPropertyName
#define ex_InvalidPropertyName  ex_ExtendedNaming_ExtendedNamingContext_InvalidPropertyName
#else
#define SOMTGD_ex_InvalidPropertyName
#undef ex_InvalidPropertyName
#endif /* ex_InvalidPropertyName */
#endif /* SOMTGD_ex_InvalidPropertyName */
#endif /* SOM_DONT_USE_SHORT_NAMES */
typedef struct ExtendedNaming_ExtendedNamingContext_InvalidPropertyName ExtendedNaming_ExtendedNamingContext_InvalidPropertyName;
#ifndef SOM_DONT_USE_SHORT_NAMES
	#ifndef SOMGD_InvalidPropertyName
		#if defined(InvalidPropertyName)
			#undef InvalidPropertyName
			#define SOMGD_InvalidPropertyName
		#else
			#define InvalidPropertyName ExtendedNaming_ExtendedNamingContext_InvalidPropertyName
		#endif
	#endif /* SOMGD_InvalidPropertyName */
#endif /* SOM_DONT_USE_SHORT_NAMES */
#define ex_ExtendedNaming_ExtendedNamingContext_NotSupported   "::ExtendedNaming::ExtendedNamingContext::NotSupported"
#ifndef SOM_DONT_USE_SHORT_NAMES
#ifndef SOMTGD_ex_NotSupported
#ifndef ex_NotSupported
#define ex_NotSupported  ex_ExtendedNaming_ExtendedNamingContext_NotSupported
#else
#define SOMTGD_ex_NotSupported
#undef ex_NotSupported
#endif /* ex_NotSupported */
#endif /* SOMTGD_ex_NotSupported */
#endif /* SOM_DONT_USE_SHORT_NAMES */
typedef struct ExtendedNaming_ExtendedNamingContext_NotSupported ExtendedNaming_ExtendedNamingContext_NotSupported;
#ifndef SOM_DONT_USE_SHORT_NAMES
	#ifndef SOMGD_NotSupported
		#if defined(NotSupported)
			#undef NotSupported
			#define SOMGD_NotSupported
		#else
			#define NotSupported ExtendedNaming_ExtendedNamingContext_NotSupported
		#endif
	#endif /* SOMGD_NotSupported */
#endif /* SOM_DONT_USE_SHORT_NAMES */
#define ex_ExtendedNaming_ExtendedNamingContext_ConflictingPropertyName   "::ExtendedNaming::ExtendedNamingContext::ConflictingPropertyName"
#ifndef SOM_DONT_USE_SHORT_NAMES
#ifndef SOMTGD_ex_ConflictingPropertyName
#ifndef ex_ConflictingPropertyName
#define ex_ConflictingPropertyName  ex_ExtendedNaming_ExtendedNamingContext_ConflictingPropertyName
#else
#define SOMTGD_ex_ConflictingPropertyName
#undef ex_ConflictingPropertyName
#endif /* ex_ConflictingPropertyName */
#endif /* SOMTGD_ex_ConflictingPropertyName */
#endif /* SOM_DONT_USE_SHORT_NAMES */
typedef struct ExtendedNaming_ExtendedNamingContext_ConflictingPropertyName ExtendedNaming_ExtendedNamingContext_ConflictingPropertyName;
#ifndef SOM_DONT_USE_SHORT_NAMES
	#ifndef SOMGD_ConflictingPropertyName
		#if defined(ConflictingPropertyName)
			#undef ConflictingPropertyName
			#define SOMGD_ConflictingPropertyName
		#else
			#define ConflictingPropertyName ExtendedNaming_ExtendedNamingContext_ConflictingPropertyName
		#endif
	#endif /* SOMGD_ConflictingPropertyName */
#endif /* SOM_DONT_USE_SHORT_NAMES */
#define ex_ExtendedNaming_ExtendedNamingContext_PropertyNotFound   "::ExtendedNaming::ExtendedNamingContext::PropertyNotFound"
#ifndef SOM_DONT_USE_SHORT_NAMES
#ifndef SOMTGD_ex_PropertyNotFound
#ifndef ex_PropertyNotFound
#define ex_PropertyNotFound  ex_ExtendedNaming_ExtendedNamingContext_PropertyNotFound
#else
#define SOMTGD_ex_PropertyNotFound
#undef ex_PropertyNotFound
#endif /* ex_PropertyNotFound */
#endif /* SOMTGD_ex_PropertyNotFound */
#endif /* SOM_DONT_USE_SHORT_NAMES */
typedef struct ExtendedNaming_ExtendedNamingContext_PropertyNotFound
{
	_IDL_SEQUENCE_string property_name;
} ExtendedNaming_ExtendedNamingContext_PropertyNotFound;
#ifndef SOM_DONT_USE_SHORT_NAMES
	#ifndef SOMGD_PropertyNotFound
		#if defined(PropertyNotFound)
			#undef PropertyNotFound
			#define SOMGD_PropertyNotFound
		#else
			#define PropertyNotFound ExtendedNaming_ExtendedNamingContext_PropertyNotFound
		#endif
	#endif /* SOMGD_PropertyNotFound */
#endif /* SOM_DONT_USE_SHORT_NAMES */
#define ex_ExtendedNaming_ExtendedNamingContext_NonSharableProperties   "::ExtendedNaming::ExtendedNamingContext::NonSharableProperties"
#ifndef SOM_DONT_USE_SHORT_NAMES
#ifndef SOMTGD_ex_NonSharableProperties
#ifndef ex_NonSharableProperties
#define ex_NonSharableProperties  ex_ExtendedNaming_ExtendedNamingContext_NonSharableProperties
#else
#define SOMTGD_ex_NonSharableProperties
#undef ex_NonSharableProperties
#endif /* ex_NonSharableProperties */
#endif /* SOMTGD_ex_NonSharableProperties */
#endif /* SOM_DONT_USE_SHORT_NAMES */
typedef struct ExtendedNaming_ExtendedNamingContext_NonSharableProperties ExtendedNaming_ExtendedNamingContext_NonSharableProperties;
#ifndef SOM_DONT_USE_SHORT_NAMES
	#ifndef SOMGD_NonSharableProperties
		#if defined(NonSharableProperties)
			#undef NonSharableProperties
			#define SOMGD_NonSharableProperties
		#else
			#define NonSharableProperties ExtendedNaming_ExtendedNamingContext_NonSharableProperties
		#endif
	#endif /* SOMGD_NonSharableProperties */
#endif /* SOM_DONT_USE_SHORT_NAMES */
#define ex_ExtendedNaming_ExtendedNamingContext_PropertiesNotShared   "::ExtendedNaming::ExtendedNamingContext::PropertiesNotShared"
#ifndef SOM_DONT_USE_SHORT_NAMES
#ifndef SOMTGD_ex_PropertiesNotShared
#ifndef ex_PropertiesNotShared
#define ex_PropertiesNotShared  ex_ExtendedNaming_ExtendedNamingContext_PropertiesNotShared
#else
#define SOMTGD_ex_PropertiesNotShared
#undef ex_PropertiesNotShared
#endif /* ex_PropertiesNotShared */
#endif /* SOMTGD_ex_PropertiesNotShared */
#endif /* SOM_DONT_USE_SHORT_NAMES */
typedef struct ExtendedNaming_ExtendedNamingContext_PropertiesNotShared ExtendedNaming_ExtendedNamingContext_PropertiesNotShared;
#ifndef SOM_DONT_USE_SHORT_NAMES
	#ifndef SOMGD_PropertiesNotShared
		#if defined(PropertiesNotShared)
			#undef PropertiesNotShared
			#define SOMGD_PropertiesNotShared
		#else
			#define PropertiesNotShared ExtendedNaming_ExtendedNamingContext_PropertiesNotShared
		#endif
	#endif /* SOMGD_PropertiesNotShared */
#endif /* SOM_DONT_USE_SHORT_NAMES */
#define ex_ExtendedNaming_ExtendedNamingContext_IllegalConstraintExpression   "::ExtendedNaming::ExtendedNamingContext::IllegalConstraintExpression"
#ifndef SOM_DONT_USE_SHORT_NAMES
#ifndef SOMTGD_ex_IllegalConstraintExpression
#ifndef ex_IllegalConstraintExpression
#define ex_IllegalConstraintExpression  ex_ExtendedNaming_ExtendedNamingContext_IllegalConstraintExpression
#else
#define SOMTGD_ex_IllegalConstraintExpression
#undef ex_IllegalConstraintExpression
#endif /* ex_IllegalConstraintExpression */
#endif /* SOMTGD_ex_IllegalConstraintExpression */
#endif /* SOM_DONT_USE_SHORT_NAMES */
typedef struct ExtendedNaming_ExtendedNamingContext_IllegalConstraintExpression ExtendedNaming_ExtendedNamingContext_IllegalConstraintExpression;
#ifndef SOM_DONT_USE_SHORT_NAMES
	#ifndef SOMGD_IllegalConstraintExpression
		#if defined(IllegalConstraintExpression)
			#undef IllegalConstraintExpression
			#define SOMGD_IllegalConstraintExpression
		#else
			#define IllegalConstraintExpression ExtendedNaming_ExtendedNamingContext_IllegalConstraintExpression
		#endif
	#endif /* SOMGD_IllegalConstraintExpression */
#endif /* SOM_DONT_USE_SHORT_NAMES */
#define ex_ExtendedNaming_ExtendedNamingContext_BindingNotFound   "::ExtendedNaming::ExtendedNamingContext::BindingNotFound"
#ifndef SOM_DONT_USE_SHORT_NAMES
#ifndef SOMTGD_ex_BindingNotFound
#ifndef ex_BindingNotFound
#define ex_BindingNotFound  ex_ExtendedNaming_ExtendedNamingContext_BindingNotFound
#else
#define SOMTGD_ex_BindingNotFound
#undef ex_BindingNotFound
#endif /* ex_BindingNotFound */
#endif /* SOMTGD_ex_BindingNotFound */
#endif /* SOM_DONT_USE_SHORT_NAMES */
typedef struct ExtendedNaming_ExtendedNamingContext_BindingNotFound ExtendedNaming_ExtendedNamingContext_BindingNotFound;
#ifndef SOM_DONT_USE_SHORT_NAMES
	#ifndef SOMGD_BindingNotFound
		#if defined(BindingNotFound)
			#undef BindingNotFound
			#define SOMGD_BindingNotFound
		#else
			#define BindingNotFound ExtendedNaming_ExtendedNamingContext_BindingNotFound
		#endif
	#endif /* SOMGD_BindingNotFound */
#endif /* SOM_DONT_USE_SHORT_NAMES */
		typedef _IDL_SEQUENCE_string ExtendedNaming_ExtendedNamingContext_CosNaming_IstringSeq;
		#ifndef _IDL_SEQUENCE_ExtendedNaming_ExtendedNamingContext_CosNaming_IstringSeq_defined
			#define _IDL_SEQUENCE_ExtendedNaming_ExtendedNamingContext_CosNaming_IstringSeq_defined
			SOM_SEQUENCE_TYPEDEF(ExtendedNaming_ExtendedNamingContext_CosNaming_IstringSeq);
		#endif /* _IDL_SEQUENCE_ExtendedNaming_ExtendedNamingContext_CosNaming_IstringSeq_defined */
		#ifdef __IBMC__
			typedef ExtendedNaming_TypeCodeSeq (somTP_ExtendedNaming_ExtendedNamingContext__get_allowed_object_types)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev);
			#pragma linkage(somTP_ExtendedNaming_ExtendedNamingContext__get_allowed_object_types,system)
			typedef somTP_ExtendedNaming_ExtendedNamingContext__get_allowed_object_types *somTD_ExtendedNaming_ExtendedNamingContext__get_allowed_object_types;
		#else /* __IBMC__ */
			typedef ExtendedNaming_TypeCodeSeq (SOMLINK * somTD_ExtendedNaming_ExtendedNamingContext__get_allowed_object_types)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef ExtendedNaming_TypeCodeSeq (somTP_ExtendedNaming_ExtendedNamingContext__get_allowed_property_types)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev);
			#pragma linkage(somTP_ExtendedNaming_ExtendedNamingContext__get_allowed_property_types,system)
			typedef somTP_ExtendedNaming_ExtendedNamingContext__get_allowed_property_types *somTD_ExtendedNaming_ExtendedNamingContext__get_allowed_property_types;
		#else /* __IBMC__ */
			typedef ExtendedNaming_TypeCodeSeq (SOMLINK * somTD_ExtendedNaming_ExtendedNamingContext__get_allowed_property_types)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef ExtendedNaming_ExtendedNamingContext_CosNaming_IstringSeq (somTP_ExtendedNaming_ExtendedNamingContext__get_allowed_property_names)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev);
			#pragma linkage(somTP_ExtendedNaming_ExtendedNamingContext__get_allowed_property_names,system)
			typedef somTP_ExtendedNaming_ExtendedNamingContext__get_allowed_property_names *somTD_ExtendedNaming_ExtendedNamingContext__get_allowed_property_names;
		#else /* __IBMC__ */
			typedef ExtendedNaming_ExtendedNamingContext_CosNaming_IstringSeq (SOMLINK * somTD_ExtendedNaming_ExtendedNamingContext__get_allowed_property_names)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef ExtendedNaming_TypeCodeSeq (somTP_ExtendedNaming_ExtendedNamingContext__get_shared_property_types)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev);
			#pragma linkage(somTP_ExtendedNaming_ExtendedNamingContext__get_shared_property_types,system)
			typedef somTP_ExtendedNaming_ExtendedNamingContext__get_shared_property_types *somTD_ExtendedNaming_ExtendedNamingContext__get_shared_property_types;
		#else /* __IBMC__ */
			typedef ExtendedNaming_TypeCodeSeq (SOMLINK * somTD_ExtendedNaming_ExtendedNamingContext__get_shared_property_types)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef ExtendedNaming_ExtendedNamingContext_CosNaming_IstringSeq (somTP_ExtendedNaming_ExtendedNamingContext__get_shared_property_names)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev);
			#pragma linkage(somTP_ExtendedNaming_ExtendedNamingContext__get_shared_property_names,system)
			typedef somTP_ExtendedNaming_ExtendedNamingContext__get_shared_property_names *somTD_ExtendedNaming_ExtendedNamingContext__get_shared_property_names;
		#else /* __IBMC__ */
			typedef ExtendedNaming_ExtendedNamingContext_CosNaming_IstringSeq (SOMLINK * somTD_ExtendedNaming_ExtendedNamingContext__get_shared_property_names)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef unsigned short (somTP_ExtendedNaming_ExtendedNamingContext_get_features_supported)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev);
			#pragma linkage(somTP_ExtendedNaming_ExtendedNamingContext_get_features_supported,system)
			typedef somTP_ExtendedNaming_ExtendedNamingContext_get_features_supported *somTD_ExtendedNaming_ExtendedNamingContext_get_features_supported;
		#else /* __IBMC__ */
			typedef unsigned short (SOMLINK * somTD_ExtendedNaming_ExtendedNamingContext_get_features_supported)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_ExtendedNaming_ExtendedNamingContext_add_property)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ ExtendedNaming_Property *prop);
			#pragma linkage(somTP_ExtendedNaming_ExtendedNamingContext_add_property,system)
			typedef somTP_ExtendedNaming_ExtendedNamingContext_add_property *somTD_ExtendedNaming_ExtendedNamingContext_add_property;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_ExtendedNaming_ExtendedNamingContext_add_property)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ ExtendedNaming_Property *prop);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_ExtendedNaming_ExtendedNamingContext_add_properties)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ ExtendedNaming_PropertyList *props);
			#pragma linkage(somTP_ExtendedNaming_ExtendedNamingContext_add_properties,system)
			typedef somTP_ExtendedNaming_ExtendedNamingContext_add_properties *somTD_ExtendedNaming_ExtendedNamingContext_add_properties;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_ExtendedNaming_ExtendedNamingContext_add_properties)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ ExtendedNaming_PropertyList *props);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_ExtendedNaming_ExtendedNamingContext_bind_with_properties)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ SOMObject SOMSTAR obj,
				/* in */ ExtendedNaming_PropertyList *prop);
			#pragma linkage(somTP_ExtendedNaming_ExtendedNamingContext_bind_with_properties,system)
			typedef somTP_ExtendedNaming_ExtendedNamingContext_bind_with_properties *somTD_ExtendedNaming_ExtendedNamingContext_bind_with_properties;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_ExtendedNaming_ExtendedNamingContext_bind_with_properties)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ SOMObject SOMSTAR obj,
				/* in */ ExtendedNaming_PropertyList *prop);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_ExtendedNaming_ExtendedNamingContext_bind_context_with_properties)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ ExtendedNaming_ExtendedNamingContext SOMSTAR obj,
				/* in */ ExtendedNaming_PropertyList *props);
			#pragma linkage(somTP_ExtendedNaming_ExtendedNamingContext_bind_context_with_properties,system)
			typedef somTP_ExtendedNaming_ExtendedNamingContext_bind_context_with_properties *somTD_ExtendedNaming_ExtendedNamingContext_bind_context_with_properties;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_ExtendedNaming_ExtendedNamingContext_bind_context_with_properties)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ ExtendedNaming_ExtendedNamingContext SOMSTAR obj,
				/* in */ ExtendedNaming_PropertyList *props);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_ExtendedNaming_ExtendedNamingContext_rebind_with_properties)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ SOMObject SOMSTAR obj,
				/* in */ ExtendedNaming_PropertyList *prop);
			#pragma linkage(somTP_ExtendedNaming_ExtendedNamingContext_rebind_with_properties,system)
			typedef somTP_ExtendedNaming_ExtendedNamingContext_rebind_with_properties *somTD_ExtendedNaming_ExtendedNamingContext_rebind_with_properties;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_ExtendedNaming_ExtendedNamingContext_rebind_with_properties)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ SOMObject SOMSTAR obj,
				/* in */ ExtendedNaming_PropertyList *prop);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_ExtendedNaming_ExtendedNamingContext_rebind_context_with_properties)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ ExtendedNaming_ExtendedNamingContext SOMSTAR obj,
				/* in */ ExtendedNaming_PropertyList *props);
			#pragma linkage(somTP_ExtendedNaming_ExtendedNamingContext_rebind_context_with_properties,system)
			typedef somTP_ExtendedNaming_ExtendedNamingContext_rebind_context_with_properties *somTD_ExtendedNaming_ExtendedNamingContext_rebind_context_with_properties;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_ExtendedNaming_ExtendedNamingContext_rebind_context_with_properties)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ ExtendedNaming_ExtendedNamingContext SOMSTAR obj,
				/* in */ ExtendedNaming_PropertyList *props);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_ExtendedNaming_ExtendedNamingContext_share_property)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *target_n,
				/* in */ ExtendedNaming_ExtendedNamingContext SOMSTAR source_enc,
				/* in */ CosNaming_Name *source_n,
				/* in */ CosNaming_Istring pname);
			#pragma linkage(somTP_ExtendedNaming_ExtendedNamingContext_share_property,system)
			typedef somTP_ExtendedNaming_ExtendedNamingContext_share_property *somTD_ExtendedNaming_ExtendedNamingContext_share_property;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_ExtendedNaming_ExtendedNamingContext_share_property)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *target_n,
				/* in */ ExtendedNaming_ExtendedNamingContext SOMSTAR source_enc,
				/* in */ CosNaming_Name *source_n,
				/* in */ CosNaming_Istring pname);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_ExtendedNaming_ExtendedNamingContext_share_properties)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *target_n,
				/* in */ ExtendedNaming_ExtendedNamingContext SOMSTAR source_enc,
				/* in */ CosNaming_Name *source_n,
				/* in */ ExtendedNaming_IList *pnames);
			#pragma linkage(somTP_ExtendedNaming_ExtendedNamingContext_share_properties,system)
			typedef somTP_ExtendedNaming_ExtendedNamingContext_share_properties *somTD_ExtendedNaming_ExtendedNamingContext_share_properties;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_ExtendedNaming_ExtendedNamingContext_share_properties)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *target_n,
				/* in */ ExtendedNaming_ExtendedNamingContext SOMSTAR source_enc,
				/* in */ CosNaming_Name *source_n,
				/* in */ ExtendedNaming_IList *pnames);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_ExtendedNaming_ExtendedNamingContext_unshare_property)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ CosNaming_Istring property_name);
			#pragma linkage(somTP_ExtendedNaming_ExtendedNamingContext_unshare_property,system)
			typedef somTP_ExtendedNaming_ExtendedNamingContext_unshare_property *somTD_ExtendedNaming_ExtendedNamingContext_unshare_property;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_ExtendedNaming_ExtendedNamingContext_unshare_property)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ CosNaming_Istring property_name);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_ExtendedNaming_ExtendedNamingContext_unshare_properties)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ ExtendedNaming_IList *pnames);
			#pragma linkage(somTP_ExtendedNaming_ExtendedNamingContext_unshare_properties,system)
			typedef somTP_ExtendedNaming_ExtendedNamingContext_unshare_properties *somTD_ExtendedNaming_ExtendedNamingContext_unshare_properties;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_ExtendedNaming_ExtendedNamingContext_unshare_properties)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ ExtendedNaming_IList *pnames);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_ExtendedNaming_ExtendedNamingContext_list_properties)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ unsigned long howMany,
				/* out */ ExtendedNaming_PropertyBindingList *pbl,
				/* out */ ExtendedNaming_PropertyBindingIterator SOMSTAR *rest);
			#pragma linkage(somTP_ExtendedNaming_ExtendedNamingContext_list_properties,system)
			typedef somTP_ExtendedNaming_ExtendedNamingContext_list_properties *somTD_ExtendedNaming_ExtendedNamingContext_list_properties;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_ExtendedNaming_ExtendedNamingContext_list_properties)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ unsigned long howMany,
				/* out */ ExtendedNaming_PropertyBindingList *pbl,
				/* out */ ExtendedNaming_PropertyBindingIterator SOMSTAR *rest);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_ExtendedNaming_ExtendedNamingContext_get_property)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ CosNaming_Istring pn,
				/* out */ ExtendedNaming_Property *prop);
			#pragma linkage(somTP_ExtendedNaming_ExtendedNamingContext_get_property,system)
			typedef somTP_ExtendedNaming_ExtendedNamingContext_get_property *somTD_ExtendedNaming_ExtendedNamingContext_get_property;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_ExtendedNaming_ExtendedNamingContext_get_property)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ CosNaming_Istring pn,
				/* out */ ExtendedNaming_Property *prop);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_ExtendedNaming_ExtendedNamingContext_get_properties)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ unsigned long howMany,
				/* in */ ExtendedNaming_IList *inames,
				/* out */ ExtendedNaming_PropertyList *props,
				/* out */ ExtendedNaming_PropertyIterator SOMSTAR *rest);
			#pragma linkage(somTP_ExtendedNaming_ExtendedNamingContext_get_properties,system)
			typedef somTP_ExtendedNaming_ExtendedNamingContext_get_properties *somTD_ExtendedNaming_ExtendedNamingContext_get_properties;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_ExtendedNaming_ExtendedNamingContext_get_properties)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ unsigned long howMany,
				/* in */ ExtendedNaming_IList *inames,
				/* out */ ExtendedNaming_PropertyList *props,
				/* out */ ExtendedNaming_PropertyIterator SOMSTAR *rest);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_ExtendedNaming_ExtendedNamingContext_get_all_properties)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ unsigned long howMany,
				/* out */ ExtendedNaming_PropertyList *props,
				/* out */ ExtendedNaming_PropertyIterator SOMSTAR *rest);
			#pragma linkage(somTP_ExtendedNaming_ExtendedNamingContext_get_all_properties,system)
			typedef somTP_ExtendedNaming_ExtendedNamingContext_get_all_properties *somTD_ExtendedNaming_ExtendedNamingContext_get_all_properties;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_ExtendedNaming_ExtendedNamingContext_get_all_properties)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ unsigned long howMany,
				/* out */ ExtendedNaming_PropertyList *props,
				/* out */ ExtendedNaming_PropertyIterator SOMSTAR *rest);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef SOMObject SOMSTAR (somTP_ExtendedNaming_ExtendedNamingContext_resolve_with_property)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ CosNaming_Istring prop,
				/* out */ any *v);
			#pragma linkage(somTP_ExtendedNaming_ExtendedNamingContext_resolve_with_property,system)
			typedef somTP_ExtendedNaming_ExtendedNamingContext_resolve_with_property *somTD_ExtendedNaming_ExtendedNamingContext_resolve_with_property;
		#else /* __IBMC__ */
			typedef SOMObject SOMSTAR (SOMLINK * somTD_ExtendedNaming_ExtendedNamingContext_resolve_with_property)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ CosNaming_Istring prop,
				/* out */ any *v);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef SOMObject SOMSTAR (somTP_ExtendedNaming_ExtendedNamingContext_resolve_with_properties)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ unsigned long howMany,
				/* in */ ExtendedNaming_IList *inames,
				/* out */ ExtendedNaming_PropertyList *props,
				/* out */ ExtendedNaming_PropertyIterator SOMSTAR *rest);
			#pragma linkage(somTP_ExtendedNaming_ExtendedNamingContext_resolve_with_properties,system)
			typedef somTP_ExtendedNaming_ExtendedNamingContext_resolve_with_properties *somTD_ExtendedNaming_ExtendedNamingContext_resolve_with_properties;
		#else /* __IBMC__ */
			typedef SOMObject SOMSTAR (SOMLINK * somTD_ExtendedNaming_ExtendedNamingContext_resolve_with_properties)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ unsigned long howMany,
				/* in */ ExtendedNaming_IList *inames,
				/* out */ ExtendedNaming_PropertyList *props,
				/* out */ ExtendedNaming_PropertyIterator SOMSTAR *rest);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef SOMObject SOMSTAR (somTP_ExtendedNaming_ExtendedNamingContext_resolve_with_all_properties)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ unsigned long howMany,
				/* out */ ExtendedNaming_PropertyList *props,
				/* out */ ExtendedNaming_PropertyIterator SOMSTAR *rest);
			#pragma linkage(somTP_ExtendedNaming_ExtendedNamingContext_resolve_with_all_properties,system)
			typedef somTP_ExtendedNaming_ExtendedNamingContext_resolve_with_all_properties *somTD_ExtendedNaming_ExtendedNamingContext_resolve_with_all_properties;
		#else /* __IBMC__ */
			typedef SOMObject SOMSTAR (SOMLINK * somTD_ExtendedNaming_ExtendedNamingContext_resolve_with_all_properties)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ unsigned long howMany,
				/* out */ ExtendedNaming_PropertyList *props,
				/* out */ ExtendedNaming_PropertyIterator SOMSTAR *rest);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_ExtendedNaming_ExtendedNamingContext_remove_property)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ CosNaming_Istring prop);
			#pragma linkage(somTP_ExtendedNaming_ExtendedNamingContext_remove_property,system)
			typedef somTP_ExtendedNaming_ExtendedNamingContext_remove_property *somTD_ExtendedNaming_ExtendedNamingContext_remove_property;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_ExtendedNaming_ExtendedNamingContext_remove_property)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ CosNaming_Istring prop);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_ExtendedNaming_ExtendedNamingContext_remove_properties)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ ExtendedNaming_IList *plist);
			#pragma linkage(somTP_ExtendedNaming_ExtendedNamingContext_remove_properties,system)
			typedef somTP_ExtendedNaming_ExtendedNamingContext_remove_properties *somTD_ExtendedNaming_ExtendedNamingContext_remove_properties;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_ExtendedNaming_ExtendedNamingContext_remove_properties)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n,
				/* in */ ExtendedNaming_IList *plist);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_ExtendedNaming_ExtendedNamingContext_remove_all_properties)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n);
			#pragma linkage(somTP_ExtendedNaming_ExtendedNamingContext_remove_all_properties,system)
			typedef somTP_ExtendedNaming_ExtendedNamingContext_remove_all_properties *somTD_ExtendedNaming_ExtendedNamingContext_remove_all_properties;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_ExtendedNaming_ExtendedNamingContext_remove_all_properties)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosNaming_Name *n);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef SOMObject SOMSTAR (somTP_ExtendedNaming_ExtendedNamingContext_find_any)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ ExtendedNaming_ExtendedNamingContext_Constraint c,
				/* in */ unsigned long distance);
			#pragma linkage(somTP_ExtendedNaming_ExtendedNamingContext_find_any,system)
			typedef somTP_ExtendedNaming_ExtendedNamingContext_find_any *somTD_ExtendedNaming_ExtendedNamingContext_find_any;
		#else /* __IBMC__ */
			typedef SOMObject SOMSTAR (SOMLINK * somTD_ExtendedNaming_ExtendedNamingContext_find_any)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ ExtendedNaming_ExtendedNamingContext_Constraint c,
				/* in */ unsigned long distance);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_ExtendedNaming_ExtendedNamingContext_find_all)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ ExtendedNaming_ExtendedNamingContext_Constraint c,
				/* in */ unsigned long distance,
				/* in */ unsigned long howMany,
				/* out */ CosNaming_BindingList *bl,
				/* out */ CosNaming_BindingIterator SOMSTAR *bi);
			#pragma linkage(somTP_ExtendedNaming_ExtendedNamingContext_find_all,system)
			typedef somTP_ExtendedNaming_ExtendedNamingContext_find_all *somTD_ExtendedNaming_ExtendedNamingContext_find_all;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_ExtendedNaming_ExtendedNamingContext_find_all)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ ExtendedNaming_ExtendedNamingContext_Constraint c,
				/* in */ unsigned long distance,
				/* in */ unsigned long howMany,
				/* out */ CosNaming_BindingList *bl,
				/* out */ CosNaming_BindingIterator SOMSTAR *bi);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_ExtendedNaming_ExtendedNamingContext_find_any_name_binding)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ ExtendedNaming_ExtendedNamingContext_Constraint c,
				/* in */ unsigned long distance,
				/* out */ CosNaming_Binding *bi);
			#pragma linkage(somTP_ExtendedNaming_ExtendedNamingContext_find_any_name_binding,system)
			typedef somTP_ExtendedNaming_ExtendedNamingContext_find_any_name_binding *somTD_ExtendedNaming_ExtendedNamingContext_find_any_name_binding;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_ExtendedNaming_ExtendedNamingContext_find_any_name_binding)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ ExtendedNaming_ExtendedNamingContext_Constraint c,
				/* in */ unsigned long distance,
				/* out */ CosNaming_Binding *bi);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_ExtendedNaming_ExtendedNamingContext_add_index)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ ExtendedNaming_IndexDescriptor *i);
			#pragma linkage(somTP_ExtendedNaming_ExtendedNamingContext_add_index,system)
			typedef somTP_ExtendedNaming_ExtendedNamingContext_add_index *somTD_ExtendedNaming_ExtendedNamingContext_add_index;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_ExtendedNaming_ExtendedNamingContext_add_index)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ ExtendedNaming_IndexDescriptor *i);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_ExtendedNaming_ExtendedNamingContext_list_indexes)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ unsigned long howMany,
				/* out */ ExtendedNaming_IndexDescriptorList *il,
				/* out */ ExtendedNaming_IndexIterator SOMSTAR *rest);
			#pragma linkage(somTP_ExtendedNaming_ExtendedNamingContext_list_indexes,system)
			typedef somTP_ExtendedNaming_ExtendedNamingContext_list_indexes *somTD_ExtendedNaming_ExtendedNamingContext_list_indexes;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_ExtendedNaming_ExtendedNamingContext_list_indexes)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ unsigned long howMany,
				/* out */ ExtendedNaming_IndexDescriptorList *il,
				/* out */ ExtendedNaming_IndexIterator SOMSTAR *rest);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_ExtendedNaming_ExtendedNamingContext_remove_index)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ ExtendedNaming_IndexDescriptor *i);
			#pragma linkage(somTP_ExtendedNaming_ExtendedNamingContext_remove_index,system)
			typedef somTP_ExtendedNaming_ExtendedNamingContext_remove_index *somTD_ExtendedNaming_ExtendedNamingContext_remove_index;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_ExtendedNaming_ExtendedNamingContext_remove_index)(
				ExtendedNaming_ExtendedNamingContext SOMSTAR somSelf,
				Environment *ev,
				/* in */ ExtendedNaming_IndexDescriptor *i);
		#endif /* __IBMC__ */
		#ifndef ExtendedNaming_ExtendedNamingContext_MajorVersion
			#define ExtendedNaming_ExtendedNamingContext_MajorVersion   3
		#endif /* ExtendedNaming_ExtendedNamingContext_MajorVersion */
		#ifndef ExtendedNaming_ExtendedNamingContext_MinorVersion
			#define ExtendedNaming_ExtendedNamingContext_MinorVersion   0
		#endif /* ExtendedNaming_ExtendedNamingContext_MinorVersion */
		typedef struct ExtendedNaming_ExtendedNamingContextClassDataStructure
		{
			SOMClass SOMSTAR classObject;
			somMToken get_features_supported;
			somMToken add_property;
			somMToken add_properties;
			somMToken bind_with_properties;
			somMToken bind_context_with_properties;
			somMToken rebind_with_properties;
			somMToken rebind_context_with_properties;
			somMToken share_property;
			somMToken share_properties;
			somMToken unshare_property;
			somMToken unshare_properties;
			somMToken list_properties;
			somMToken get_property;
			somMToken get_properties;
			somMToken get_all_properties;
			somMToken resolve_with_property;
			somMToken resolve_with_properties;
			somMToken resolve_with_all_properties;
			somMToken remove_property;
			somMToken remove_properties;
			somMToken remove_all_properties;
			somMToken find_any;
			somMToken find_all;
			somMToken add_index;
			somMToken list_indexes;
			somMToken remove_index;
			somMToken _get_allowed_object_types;
			somMToken find_any_name_binding;
			somMToken private28;
			somMToken _get_allowed_property_types;
			somMToken private30;
			somMToken _get_allowed_property_names;
			somMToken private32;
			somMToken _get_shared_property_names;
			somMToken private34;
			somMToken _get_shared_property_types;
			somMToken private36;
		} ExtendedNaming_ExtendedNamingContextClassDataStructure;
		typedef struct ExtendedNaming_ExtendedNamingContextCClassDataStructure
		{
			somMethodTabs parentMtab;
			somDToken instanceDataToken;
		} ExtendedNaming_ExtendedNamingContextCClassDataStructure;
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_xnaming_Source) || defined(ExtendedNaming_ExtendedNamingContext_Class_Source)
				SOMEXTERN struct ExtendedNaming_ExtendedNamingContextClassDataStructure _ExtendedNaming_ExtendedNamingContextClassData;
				#ifndef ExtendedNaming_ExtendedNamingContextClassData
					#define ExtendedNaming_ExtendedNamingContextClassData    _ExtendedNaming_ExtendedNamingContextClassData
				#endif /* ExtendedNaming_ExtendedNamingContextClassData */
			#else
				SOMEXTERN struct ExtendedNaming_ExtendedNamingContextClassDataStructure * SOMLINK resolve_ExtendedNaming_ExtendedNamingContextClassData(void);
				#ifndef ExtendedNaming_ExtendedNamingContextClassData
					#define ExtendedNaming_ExtendedNamingContextClassData    (*(resolve_ExtendedNaming_ExtendedNamingContextClassData()))
				#endif /* ExtendedNaming_ExtendedNamingContextClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_xnaming_Source) || defined(ExtendedNaming_ExtendedNamingContext_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_xnaming_Source || ExtendedNaming_ExtendedNamingContext_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_xnaming_Source || ExtendedNaming_ExtendedNamingContext_Class_Source */
			struct ExtendedNaming_ExtendedNamingContextClassDataStructure SOMDLINK ExtendedNaming_ExtendedNamingContextClassData;
		#endif /* SOM_RESOLVE_DATA */
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_xnaming_Source) || defined(ExtendedNaming_ExtendedNamingContext_Class_Source)
				SOMEXTERN struct ExtendedNaming_ExtendedNamingContextCClassDataStructure _ExtendedNaming_ExtendedNamingContextCClassData;
				#ifndef ExtendedNaming_ExtendedNamingContextCClassData
					#define ExtendedNaming_ExtendedNamingContextCClassData    _ExtendedNaming_ExtendedNamingContextCClassData
				#endif /* ExtendedNaming_ExtendedNamingContextCClassData */
			#else
				SOMEXTERN struct ExtendedNaming_ExtendedNamingContextCClassDataStructure * SOMLINK resolve_ExtendedNaming_ExtendedNamingContextCClassData(void);
				#ifndef ExtendedNaming_ExtendedNamingContextCClassData
					#define ExtendedNaming_ExtendedNamingContextCClassData    (*(resolve_ExtendedNaming_ExtendedNamingContextCClassData()))
				#endif /* ExtendedNaming_ExtendedNamingContextCClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_xnaming_Source) || defined(ExtendedNaming_ExtendedNamingContext_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_xnaming_Source || ExtendedNaming_ExtendedNamingContext_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_xnaming_Source || ExtendedNaming_ExtendedNamingContext_Class_Source */
			struct ExtendedNaming_ExtendedNamingContextCClassDataStructure SOMDLINK ExtendedNaming_ExtendedNamingContextCClassData;
		#endif /* SOM_RESOLVE_DATA */
		extern 
		#if defined(SOM_Module_xnaming_Source) || defined(ExtendedNaming_ExtendedNamingContext_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_xnaming_Source || ExtendedNaming_ExtendedNamingContext_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_xnaming_Source || ExtendedNaming_ExtendedNamingContext_Class_Source */
		SOMClass SOMSTAR SOMLINK ExtendedNaming_ExtendedNamingContextNewClass
				(integer4 somtmajorVersion,integer4 somtminorVersion);
		#define _SOMCLASS_ExtendedNaming_ExtendedNamingContext (ExtendedNaming_ExtendedNamingContextClassData.classObject)
		#ifndef SOMGD_ExtendedNaming_ExtendedNamingContext
			#if (defined(_ExtendedNaming_ExtendedNamingContext) || defined(__ExtendedNaming_ExtendedNamingContext))
				#undef _ExtendedNaming_ExtendedNamingContext
				#undef __ExtendedNaming_ExtendedNamingContext
				#define SOMGD_ExtendedNaming_ExtendedNamingContext 1
			#else
				#define _ExtendedNaming_ExtendedNamingContext _SOMCLASS_ExtendedNaming_ExtendedNamingContext
			#endif /* _ExtendedNaming_ExtendedNamingContext */
		#endif /* SOMGD_ExtendedNaming_ExtendedNamingContext */
		#define ExtendedNaming_ExtendedNamingContext_classObj _SOMCLASS_ExtendedNaming_ExtendedNamingContext
		#define _SOMMTOKEN_ExtendedNaming_ExtendedNamingContext(method) ((somMToken)(ExtendedNaming_ExtendedNamingContextClassData.method))
		#ifndef ExtendedNaming_ExtendedNamingContextNew
			#define ExtendedNaming_ExtendedNamingContextNew() ( _ExtendedNaming_ExtendedNamingContext ? \
				(SOMClass_somNew(_ExtendedNaming_ExtendedNamingContext)) : \
				( ExtendedNaming_ExtendedNamingContextNewClass( \
					ExtendedNaming_ExtendedNamingContext_MajorVersion, \
					ExtendedNaming_ExtendedNamingContext_MinorVersion), \
				SOMClass_somNew(_ExtendedNaming_ExtendedNamingContext))) 
		#endif /* NewExtendedNaming_ExtendedNamingContext */
		#ifndef ExtendedNaming_ExtendedNamingContext__get_allowed_object_types
			#define ExtendedNaming_ExtendedNamingContext__get_allowed_object_types(somSelf,ev) \
				SOM_Resolve(somSelf,ExtendedNaming_ExtendedNamingContext,_get_allowed_object_types)  \
					(somSelf,ev)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD___get_allowed_object_types
					#if defined(__get_allowed_object_types)
						#undef __get_allowed_object_types
						#define SOMGD___get_allowed_object_types
					#else
						#define __get_allowed_object_types ExtendedNaming_ExtendedNamingContext__get_allowed_object_types
					#endif
				#endif /* SOMGD___get_allowed_object_types */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* ExtendedNaming_ExtendedNamingContext__get_allowed_object_types */
		#ifndef ExtendedNaming_ExtendedNamingContext__get_allowed_property_types
			#define ExtendedNaming_ExtendedNamingContext__get_allowed_property_types(somSelf,ev) \
				SOM_Resolve(somSelf,ExtendedNaming_ExtendedNamingContext,_get_allowed_property_types)  \
					(somSelf,ev)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD___get_allowed_property_types
					#if defined(__get_allowed_property_types)
						#undef __get_allowed_property_types
						#define SOMGD___get_allowed_property_types
					#else
						#define __get_allowed_property_types ExtendedNaming_ExtendedNamingContext__get_allowed_property_types
					#endif
				#endif /* SOMGD___get_allowed_property_types */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* ExtendedNaming_ExtendedNamingContext__get_allowed_property_types */
		#ifndef ExtendedNaming_ExtendedNamingContext__get_allowed_property_names
			#define ExtendedNaming_ExtendedNamingContext__get_allowed_property_names(somSelf,ev) \
				SOM_Resolve(somSelf,ExtendedNaming_ExtendedNamingContext,_get_allowed_property_names)  \
					(somSelf,ev)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD___get_allowed_property_names
					#if defined(__get_allowed_property_names)
						#undef __get_allowed_property_names
						#define SOMGD___get_allowed_property_names
					#else
						#define __get_allowed_property_names ExtendedNaming_ExtendedNamingContext__get_allowed_property_names
					#endif
				#endif /* SOMGD___get_allowed_property_names */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* ExtendedNaming_ExtendedNamingContext__get_allowed_property_names */
		#ifndef ExtendedNaming_ExtendedNamingContext__get_shared_property_types
			#define ExtendedNaming_ExtendedNamingContext__get_shared_property_types(somSelf,ev) \
				SOM_Resolve(somSelf,ExtendedNaming_ExtendedNamingContext,_get_shared_property_types)  \
					(somSelf,ev)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD___get_shared_property_types
					#if defined(__get_shared_property_types)
						#undef __get_shared_property_types
						#define SOMGD___get_shared_property_types
					#else
						#define __get_shared_property_types ExtendedNaming_ExtendedNamingContext__get_shared_property_types
					#endif
				#endif /* SOMGD___get_shared_property_types */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* ExtendedNaming_ExtendedNamingContext__get_shared_property_types */
		#ifndef ExtendedNaming_ExtendedNamingContext__get_shared_property_names
			#define ExtendedNaming_ExtendedNamingContext__get_shared_property_names(somSelf,ev) \
				SOM_Resolve(somSelf,ExtendedNaming_ExtendedNamingContext,_get_shared_property_names)  \
					(somSelf,ev)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD___get_shared_property_names
					#if defined(__get_shared_property_names)
						#undef __get_shared_property_names
						#define SOMGD___get_shared_property_names
					#else
						#define __get_shared_property_names ExtendedNaming_ExtendedNamingContext__get_shared_property_names
					#endif
				#endif /* SOMGD___get_shared_property_names */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* ExtendedNaming_ExtendedNamingContext__get_shared_property_names */
		#ifndef ExtendedNaming_ExtendedNamingContext_get_features_supported
			#define ExtendedNaming_ExtendedNamingContext_get_features_supported(somSelf,ev) \
				SOM_Resolve(somSelf,ExtendedNaming_ExtendedNamingContext,get_features_supported)  \
					(somSelf,ev)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__get_features_supported
					#if defined(_get_features_supported)
						#undef _get_features_supported
						#define SOMGD__get_features_supported
					#else
						#define _get_features_supported ExtendedNaming_ExtendedNamingContext_get_features_supported
					#endif
				#endif /* SOMGD__get_features_supported */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* ExtendedNaming_ExtendedNamingContext_get_features_supported */
		#ifndef ExtendedNaming_ExtendedNamingContext_add_property
			#define ExtendedNaming_ExtendedNamingContext_add_property(somSelf,ev,n,prop) \
				SOM_Resolve(somSelf,ExtendedNaming_ExtendedNamingContext,add_property)  \
					(somSelf,ev,n,prop)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__add_property
					#if defined(_add_property)
						#undef _add_property
						#define SOMGD__add_property
					#else
						#define _add_property ExtendedNaming_ExtendedNamingContext_add_property
					#endif
				#endif /* SOMGD__add_property */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* ExtendedNaming_ExtendedNamingContext_add_property */
		#ifndef ExtendedNaming_ExtendedNamingContext_add_properties
			#define ExtendedNaming_ExtendedNamingContext_add_properties(somSelf,ev,n,props) \
				SOM_Resolve(somSelf,ExtendedNaming_ExtendedNamingContext,add_properties)  \
					(somSelf,ev,n,props)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__add_properties
					#if defined(_add_properties)
						#undef _add_properties
						#define SOMGD__add_properties
					#else
						#define _add_properties ExtendedNaming_ExtendedNamingContext_add_properties
					#endif
				#endif /* SOMGD__add_properties */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* ExtendedNaming_ExtendedNamingContext_add_properties */
		#ifndef ExtendedNaming_ExtendedNamingContext_bind_with_properties
			#define ExtendedNaming_ExtendedNamingContext_bind_with_properties(somSelf,ev,n,obj,prop) \
				SOM_Resolve(somSelf,ExtendedNaming_ExtendedNamingContext,bind_with_properties)  \
					(somSelf,ev,n,obj,prop)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__bind_with_properties
					#if defined(_bind_with_properties)
						#undef _bind_with_properties
						#define SOMGD__bind_with_properties
					#else
						#define _bind_with_properties ExtendedNaming_ExtendedNamingContext_bind_with_properties
					#endif
				#endif /* SOMGD__bind_with_properties */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* ExtendedNaming_ExtendedNamingContext_bind_with_properties */
		#ifndef ExtendedNaming_ExtendedNamingContext_bind_context_with_properties
			#define ExtendedNaming_ExtendedNamingContext_bind_context_with_properties(somSelf,ev,n,obj,props) \
				SOM_Resolve(somSelf,ExtendedNaming_ExtendedNamingContext,bind_context_with_properties)  \
					(somSelf,ev,n,obj,props)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__bind_context_with_properties
					#if defined(_bind_context_with_properties)
						#undef _bind_context_with_properties
						#define SOMGD__bind_context_with_properties
					#else
						#define _bind_context_with_properties ExtendedNaming_ExtendedNamingContext_bind_context_with_properties
					#endif
				#endif /* SOMGD__bind_context_with_properties */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* ExtendedNaming_ExtendedNamingContext_bind_context_with_properties */
		#ifndef ExtendedNaming_ExtendedNamingContext_rebind_with_properties
			#define ExtendedNaming_ExtendedNamingContext_rebind_with_properties(somSelf,ev,n,obj,prop) \
				SOM_Resolve(somSelf,ExtendedNaming_ExtendedNamingContext,rebind_with_properties)  \
					(somSelf,ev,n,obj,prop)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__rebind_with_properties
					#if defined(_rebind_with_properties)
						#undef _rebind_with_properties
						#define SOMGD__rebind_with_properties
					#else
						#define _rebind_with_properties ExtendedNaming_ExtendedNamingContext_rebind_with_properties
					#endif
				#endif /* SOMGD__rebind_with_properties */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* ExtendedNaming_ExtendedNamingContext_rebind_with_properties */
		#ifndef ExtendedNaming_ExtendedNamingContext_rebind_context_with_properties
			#define ExtendedNaming_ExtendedNamingContext_rebind_context_with_properties(somSelf,ev,n,obj,props) \
				SOM_Resolve(somSelf,ExtendedNaming_ExtendedNamingContext,rebind_context_with_properties)  \
					(somSelf,ev,n,obj,props)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__rebind_context_with_properties
					#if defined(_rebind_context_with_properties)
						#undef _rebind_context_with_properties
						#define SOMGD__rebind_context_with_properties
					#else
						#define _rebind_context_with_properties ExtendedNaming_ExtendedNamingContext_rebind_context_with_properties
					#endif
				#endif /* SOMGD__rebind_context_with_properties */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* ExtendedNaming_ExtendedNamingContext_rebind_context_with_properties */
		#ifndef ExtendedNaming_ExtendedNamingContext_share_property
			#define ExtendedNaming_ExtendedNamingContext_share_property(somSelf,ev,target_n,source_enc,source_n,pname) \
				SOM_Resolve(somSelf,ExtendedNaming_ExtendedNamingContext,share_property)  \
					(somSelf,ev,target_n,source_enc,source_n,pname)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__share_property
					#if defined(_share_property)
						#undef _share_property
						#define SOMGD__share_property
					#else
						#define _share_property ExtendedNaming_ExtendedNamingContext_share_property
					#endif
				#endif /* SOMGD__share_property */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* ExtendedNaming_ExtendedNamingContext_share_property */
		#ifndef ExtendedNaming_ExtendedNamingContext_share_properties
			#define ExtendedNaming_ExtendedNamingContext_share_properties(somSelf,ev,target_n,source_enc,source_n,pnames) \
				SOM_Resolve(somSelf,ExtendedNaming_ExtendedNamingContext,share_properties)  \
					(somSelf,ev,target_n,source_enc,source_n,pnames)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__share_properties
					#if defined(_share_properties)
						#undef _share_properties
						#define SOMGD__share_properties
					#else
						#define _share_properties ExtendedNaming_ExtendedNamingContext_share_properties
					#endif
				#endif /* SOMGD__share_properties */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* ExtendedNaming_ExtendedNamingContext_share_properties */
		#ifndef ExtendedNaming_ExtendedNamingContext_unshare_property
			#define ExtendedNaming_ExtendedNamingContext_unshare_property(somSelf,ev,n,property_name) \
				SOM_Resolve(somSelf,ExtendedNaming_ExtendedNamingContext,unshare_property)  \
					(somSelf,ev,n,property_name)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__unshare_property
					#if defined(_unshare_property)
						#undef _unshare_property
						#define SOMGD__unshare_property
					#else
						#define _unshare_property ExtendedNaming_ExtendedNamingContext_unshare_property
					#endif
				#endif /* SOMGD__unshare_property */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* ExtendedNaming_ExtendedNamingContext_unshare_property */
		#ifndef ExtendedNaming_ExtendedNamingContext_unshare_properties
			#define ExtendedNaming_ExtendedNamingContext_unshare_properties(somSelf,ev,n,pnames) \
				SOM_Resolve(somSelf,ExtendedNaming_ExtendedNamingContext,unshare_properties)  \
					(somSelf,ev,n,pnames)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__unshare_properties
					#if defined(_unshare_properties)
						#undef _unshare_properties
						#define SOMGD__unshare_properties
					#else
						#define _unshare_properties ExtendedNaming_ExtendedNamingContext_unshare_properties
					#endif
				#endif /* SOMGD__unshare_properties */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* ExtendedNaming_ExtendedNamingContext_unshare_properties */
		#ifndef ExtendedNaming_ExtendedNamingContext_list_properties
			#define ExtendedNaming_ExtendedNamingContext_list_properties(somSelf,ev,n,howMany,pbl,rest) \
				SOM_Resolve(somSelf,ExtendedNaming_ExtendedNamingContext,list_properties)  \
					(somSelf,ev,n,howMany,pbl,rest)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__list_properties
					#if defined(_list_properties)
						#undef _list_properties
						#define SOMGD__list_properties
					#else
						#define _list_properties ExtendedNaming_ExtendedNamingContext_list_properties
					#endif
				#endif /* SOMGD__list_properties */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* ExtendedNaming_ExtendedNamingContext_list_properties */
		#ifndef ExtendedNaming_ExtendedNamingContext_get_property
			#define ExtendedNaming_ExtendedNamingContext_get_property(somSelf,ev,n,pn,prop) \
				SOM_Resolve(somSelf,ExtendedNaming_ExtendedNamingContext,get_property)  \
					(somSelf,ev,n,pn,prop)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__get_property
					#if defined(_get_property)
						#undef _get_property
						#define SOMGD__get_property
					#else
						#define _get_property ExtendedNaming_ExtendedNamingContext_get_property
					#endif
				#endif /* SOMGD__get_property */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* ExtendedNaming_ExtendedNamingContext_get_property */
		#ifndef ExtendedNaming_ExtendedNamingContext_get_properties
			#define ExtendedNaming_ExtendedNamingContext_get_properties(somSelf,ev,n,howMany,inames,props,rest) \
				SOM_Resolve(somSelf,ExtendedNaming_ExtendedNamingContext,get_properties)  \
					(somSelf,ev,n,howMany,inames,props,rest)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__get_properties
					#if defined(_get_properties)
						#undef _get_properties
						#define SOMGD__get_properties
					#else
						#define _get_properties ExtendedNaming_ExtendedNamingContext_get_properties
					#endif
				#endif /* SOMGD__get_properties */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* ExtendedNaming_ExtendedNamingContext_get_properties */
		#ifndef ExtendedNaming_ExtendedNamingContext_get_all_properties
			#define ExtendedNaming_ExtendedNamingContext_get_all_properties(somSelf,ev,n,howMany,props,rest) \
				SOM_Resolve(somSelf,ExtendedNaming_ExtendedNamingContext,get_all_properties)  \
					(somSelf,ev,n,howMany,props,rest)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__get_all_properties
					#if defined(_get_all_properties)
						#undef _get_all_properties
						#define SOMGD__get_all_properties
					#else
						#define _get_all_properties ExtendedNaming_ExtendedNamingContext_get_all_properties
					#endif
				#endif /* SOMGD__get_all_properties */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* ExtendedNaming_ExtendedNamingContext_get_all_properties */
		#ifndef ExtendedNaming_ExtendedNamingContext_resolve_with_property
			#define ExtendedNaming_ExtendedNamingContext_resolve_with_property(somSelf,ev,n,prop,v) \
				SOM_Resolve(somSelf,ExtendedNaming_ExtendedNamingContext,resolve_with_property)  \
					(somSelf,ev,n,prop,v)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__resolve_with_property
					#if defined(_resolve_with_property)
						#undef _resolve_with_property
						#define SOMGD__resolve_with_property
					#else
						#define _resolve_with_property ExtendedNaming_ExtendedNamingContext_resolve_with_property
					#endif
				#endif /* SOMGD__resolve_with_property */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* ExtendedNaming_ExtendedNamingContext_resolve_with_property */
		#ifndef ExtendedNaming_ExtendedNamingContext_resolve_with_properties
			#define ExtendedNaming_ExtendedNamingContext_resolve_with_properties(somSelf,ev,n,howMany,inames,props,rest) \
				SOM_Resolve(somSelf,ExtendedNaming_ExtendedNamingContext,resolve_with_properties)  \
					(somSelf,ev,n,howMany,inames,props,rest)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__resolve_with_properties
					#if defined(_resolve_with_properties)
						#undef _resolve_with_properties
						#define SOMGD__resolve_with_properties
					#else
						#define _resolve_with_properties ExtendedNaming_ExtendedNamingContext_resolve_with_properties
					#endif
				#endif /* SOMGD__resolve_with_properties */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* ExtendedNaming_ExtendedNamingContext_resolve_with_properties */
		#ifndef ExtendedNaming_ExtendedNamingContext_resolve_with_all_properties
			#define ExtendedNaming_ExtendedNamingContext_resolve_with_all_properties(somSelf,ev,n,howMany,props,rest) \
				SOM_Resolve(somSelf,ExtendedNaming_ExtendedNamingContext,resolve_with_all_properties)  \
					(somSelf,ev,n,howMany,props,rest)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__resolve_with_all_properties
					#if defined(_resolve_with_all_properties)
						#undef _resolve_with_all_properties
						#define SOMGD__resolve_with_all_properties
					#else
						#define _resolve_with_all_properties ExtendedNaming_ExtendedNamingContext_resolve_with_all_properties
					#endif
				#endif /* SOMGD__resolve_with_all_properties */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* ExtendedNaming_ExtendedNamingContext_resolve_with_all_properties */
		#ifndef ExtendedNaming_ExtendedNamingContext_remove_property
			#define ExtendedNaming_ExtendedNamingContext_remove_property(somSelf,ev,n,prop) \
				SOM_Resolve(somSelf,ExtendedNaming_ExtendedNamingContext,remove_property)  \
					(somSelf,ev,n,prop)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__remove_property
					#if defined(_remove_property)
						#undef _remove_property
						#define SOMGD__remove_property
					#else
						#define _remove_property ExtendedNaming_ExtendedNamingContext_remove_property
					#endif
				#endif /* SOMGD__remove_property */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* ExtendedNaming_ExtendedNamingContext_remove_property */
		#ifndef ExtendedNaming_ExtendedNamingContext_remove_properties
			#define ExtendedNaming_ExtendedNamingContext_remove_properties(somSelf,ev,n,plist) \
				SOM_Resolve(somSelf,ExtendedNaming_ExtendedNamingContext,remove_properties)  \
					(somSelf,ev,n,plist)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__remove_properties
					#if defined(_remove_properties)
						#undef _remove_properties
						#define SOMGD__remove_properties
					#else
						#define _remove_properties ExtendedNaming_ExtendedNamingContext_remove_properties
					#endif
				#endif /* SOMGD__remove_properties */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* ExtendedNaming_ExtendedNamingContext_remove_properties */
		#ifndef ExtendedNaming_ExtendedNamingContext_remove_all_properties
			#define ExtendedNaming_ExtendedNamingContext_remove_all_properties(somSelf,ev,n) \
				SOM_Resolve(somSelf,ExtendedNaming_ExtendedNamingContext,remove_all_properties)  \
					(somSelf,ev,n)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__remove_all_properties
					#if defined(_remove_all_properties)
						#undef _remove_all_properties
						#define SOMGD__remove_all_properties
					#else
						#define _remove_all_properties ExtendedNaming_ExtendedNamingContext_remove_all_properties
					#endif
				#endif /* SOMGD__remove_all_properties */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* ExtendedNaming_ExtendedNamingContext_remove_all_properties */
		#ifndef ExtendedNaming_ExtendedNamingContext_find_any
			#define ExtendedNaming_ExtendedNamingContext_find_any(somSelf,ev,c,distance) \
				SOM_Resolve(somSelf,ExtendedNaming_ExtendedNamingContext,find_any)  \
					(somSelf,ev,c,distance)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__find_any
					#if defined(_find_any)
						#undef _find_any
						#define SOMGD__find_any
					#else
						#define _find_any ExtendedNaming_ExtendedNamingContext_find_any
					#endif
				#endif /* SOMGD__find_any */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* ExtendedNaming_ExtendedNamingContext_find_any */
		#ifndef ExtendedNaming_ExtendedNamingContext_find_all
			#define ExtendedNaming_ExtendedNamingContext_find_all(somSelf,ev,c,distance,howMany,bl,bi) \
				SOM_Resolve(somSelf,ExtendedNaming_ExtendedNamingContext,find_all)  \
					(somSelf,ev,c,distance,howMany,bl,bi)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__find_all
					#if defined(_find_all)
						#undef _find_all
						#define SOMGD__find_all
					#else
						#define _find_all ExtendedNaming_ExtendedNamingContext_find_all
					#endif
				#endif /* SOMGD__find_all */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* ExtendedNaming_ExtendedNamingContext_find_all */
		#ifndef ExtendedNaming_ExtendedNamingContext_find_any_name_binding
			#define ExtendedNaming_ExtendedNamingContext_find_any_name_binding(somSelf,ev,c,distance,bi) \
				SOM_Resolve(somSelf,ExtendedNaming_ExtendedNamingContext,find_any_name_binding)  \
					(somSelf,ev,c,distance,bi)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__find_any_name_binding
					#if defined(_find_any_name_binding)
						#undef _find_any_name_binding
						#define SOMGD__find_any_name_binding
					#else
						#define _find_any_name_binding ExtendedNaming_ExtendedNamingContext_find_any_name_binding
					#endif
				#endif /* SOMGD__find_any_name_binding */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* ExtendedNaming_ExtendedNamingContext_find_any_name_binding */
		#ifndef ExtendedNaming_ExtendedNamingContext_add_index
			#define ExtendedNaming_ExtendedNamingContext_add_index(somSelf,ev,i) \
				SOM_Resolve(somSelf,ExtendedNaming_ExtendedNamingContext,add_index)  \
					(somSelf,ev,i)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__add_index
					#if defined(_add_index)
						#undef _add_index
						#define SOMGD__add_index
					#else
						#define _add_index ExtendedNaming_ExtendedNamingContext_add_index
					#endif
				#endif /* SOMGD__add_index */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* ExtendedNaming_ExtendedNamingContext_add_index */
		#ifndef ExtendedNaming_ExtendedNamingContext_list_indexes
			#define ExtendedNaming_ExtendedNamingContext_list_indexes(somSelf,ev,howMany,il,rest) \
				SOM_Resolve(somSelf,ExtendedNaming_ExtendedNamingContext,list_indexes)  \
					(somSelf,ev,howMany,il,rest)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__list_indexes
					#if defined(_list_indexes)
						#undef _list_indexes
						#define SOMGD__list_indexes
					#else
						#define _list_indexes ExtendedNaming_ExtendedNamingContext_list_indexes
					#endif
				#endif /* SOMGD__list_indexes */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* ExtendedNaming_ExtendedNamingContext_list_indexes */
		#ifndef ExtendedNaming_ExtendedNamingContext_remove_index
			#define ExtendedNaming_ExtendedNamingContext_remove_index(somSelf,ev,i) \
				SOM_Resolve(somSelf,ExtendedNaming_ExtendedNamingContext,remove_index)  \
					(somSelf,ev,i)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__remove_index
					#if defined(_remove_index)
						#undef _remove_index
						#define SOMGD__remove_index
					#else
						#define _remove_index ExtendedNaming_ExtendedNamingContext_remove_index
					#endif
				#endif /* SOMGD__remove_index */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* ExtendedNaming_ExtendedNamingContext_remove_index */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define ExtendedNaming_ExtendedNamingContext_somInit SOMObject_somInit
#define ExtendedNaming_ExtendedNamingContext_somUninit SOMObject_somUninit
#define ExtendedNaming_ExtendedNamingContext_somFree SOMObject_somFree
#define ExtendedNaming_ExtendedNamingContext_somGetClass SOMObject_somGetClass
#define ExtendedNaming_ExtendedNamingContext_somGetClassName SOMObject_somGetClassName
#define ExtendedNaming_ExtendedNamingContext_somGetSize SOMObject_somGetSize
#define ExtendedNaming_ExtendedNamingContext_somIsA SOMObject_somIsA
#define ExtendedNaming_ExtendedNamingContext_somIsInstanceOf SOMObject_somIsInstanceOf
#define ExtendedNaming_ExtendedNamingContext_somRespondsTo SOMObject_somRespondsTo
#define ExtendedNaming_ExtendedNamingContext_somDispatch SOMObject_somDispatch
#define ExtendedNaming_ExtendedNamingContext_somClassDispatch SOMObject_somClassDispatch
#define ExtendedNaming_ExtendedNamingContext_somCastObj SOMObject_somCastObj
#define ExtendedNaming_ExtendedNamingContext_somResetObj SOMObject_somResetObj
#define ExtendedNaming_ExtendedNamingContext_somPrintSelf SOMObject_somPrintSelf
#define ExtendedNaming_ExtendedNamingContext_somDumpSelf SOMObject_somDumpSelf
#define ExtendedNaming_ExtendedNamingContext_somDumpSelfInt SOMObject_somDumpSelfInt
#define ExtendedNaming_ExtendedNamingContext_somDefaultInit SOMObject_somDefaultInit
#define ExtendedNaming_ExtendedNamingContext_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define ExtendedNaming_ExtendedNamingContext_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define ExtendedNaming_ExtendedNamingContext_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define ExtendedNaming_ExtendedNamingContext_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define ExtendedNaming_ExtendedNamingContext_somDefaultAssign SOMObject_somDefaultAssign
#define ExtendedNaming_ExtendedNamingContext_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define ExtendedNaming_ExtendedNamingContext_somDefaultVAssign SOMObject_somDefaultVAssign
#define ExtendedNaming_ExtendedNamingContext_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define ExtendedNaming_ExtendedNamingContext_somDestruct SOMObject_somDestruct
#define ExtendedNaming_ExtendedNamingContext_bind CosNaming_NamingContext_bind
#define ExtendedNaming_ExtendedNamingContext_rebind CosNaming_NamingContext_rebind
#define ExtendedNaming_ExtendedNamingContext_bind_context CosNaming_NamingContext_bind_context
#define ExtendedNaming_ExtendedNamingContext_rebind_context CosNaming_NamingContext_rebind_context
#define ExtendedNaming_ExtendedNamingContext_resolve CosNaming_NamingContext_resolve
#define ExtendedNaming_ExtendedNamingContext_unbind CosNaming_NamingContext_unbind
#define ExtendedNaming_ExtendedNamingContext_new_context CosNaming_NamingContext_new_context
#define ExtendedNaming_ExtendedNamingContext_bind_new_context CosNaming_NamingContext_bind_new_context
#define ExtendedNaming_ExtendedNamingContext_destroy CosNaming_NamingContext_destroy
#define ExtendedNaming_ExtendedNamingContext_list CosNaming_NamingContext_list
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_xnaming_Header_h */
