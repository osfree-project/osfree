/* generated from omgestio.idl */
/* internal conditional is SOM_Module_omgestio_Source */
#ifndef SOM_Module_omgestio_Header_h
	#define SOM_Module_omgestio_Header_h 1
	#include <som.h>
	#include <somobj.h>
	#include <naming.h>
	#include <omgidobj.h>
		#ifndef _IDL_CosLifeCycle_FactoryFinder_defined
			#define _IDL_CosLifeCycle_FactoryFinder_defined
			typedef SOMObject CosLifeCycle_FactoryFinder;
		#endif /* _IDL_CosLifeCycle_FactoryFinder_defined */
		#ifndef _IDL_SEQUENCE_CosLifeCycle_FactoryFinder_defined
			#define _IDL_SEQUENCE_CosLifeCycle_FactoryFinder_defined
			SOM_SEQUENCE_TYPEDEF_NAME(CosLifeCycle_FactoryFinder SOMSTAR ,sequence(CosLifeCycle_FactoryFinder));
		#endif /* _IDL_SEQUENCE_CosLifeCycle_FactoryFinder_defined */
		typedef CosNaming_Name CosLifeCycle_Key;
		#ifndef _IDL_SEQUENCE_CosNaming_Name_defined
			#define _IDL_SEQUENCE_CosNaming_Name_defined
			SOM_SEQUENCE_TYPEDEF(CosNaming_Name);
		#endif /* _IDL_SEQUENCE_CosNaming_Name_defined */
		#ifndef CosLifeCycle_FactoryFinder_MajorVersion
			#define CosLifeCycle_FactoryFinder_MajorVersion   0
		#endif /* CosLifeCycle_FactoryFinder_MajorVersion */
		#ifndef CosLifeCycle_FactoryFinder_MinorVersion
			#define CosLifeCycle_FactoryFinder_MinorVersion   0
		#endif /* CosLifeCycle_FactoryFinder_MinorVersion */
		typedef struct CosLifeCycle_FactoryFinderClassDataStructure
		{
			SOMClass SOMSTAR classObject;
		} CosLifeCycle_FactoryFinderClassDataStructure;
		typedef struct CosLifeCycle_FactoryFinderCClassDataStructure
		{
			somMethodTabs parentMtab;
			somDToken instanceDataToken;
		} CosLifeCycle_FactoryFinderCClassDataStructure;
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_omgestio_Source) || defined(CosLifeCycle_FactoryFinder_Class_Source)
				SOMEXTERN struct CosLifeCycle_FactoryFinderClassDataStructure _CosLifeCycle_FactoryFinderClassData;
				#ifndef CosLifeCycle_FactoryFinderClassData
					#define CosLifeCycle_FactoryFinderClassData    _CosLifeCycle_FactoryFinderClassData
				#endif /* CosLifeCycle_FactoryFinderClassData */
			#else
				SOMEXTERN struct CosLifeCycle_FactoryFinderClassDataStructure * SOMLINK resolve_CosLifeCycle_FactoryFinderClassData(void);
				#ifndef CosLifeCycle_FactoryFinderClassData
					#define CosLifeCycle_FactoryFinderClassData    (*(resolve_CosLifeCycle_FactoryFinderClassData()))
				#endif /* CosLifeCycle_FactoryFinderClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_omgestio_Source) || defined(CosLifeCycle_FactoryFinder_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_omgestio_Source || CosLifeCycle_FactoryFinder_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_omgestio_Source || CosLifeCycle_FactoryFinder_Class_Source */
			struct CosLifeCycle_FactoryFinderClassDataStructure SOMDLINK CosLifeCycle_FactoryFinderClassData;
		#endif /* SOM_RESOLVE_DATA */
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_omgestio_Source) || defined(CosLifeCycle_FactoryFinder_Class_Source)
				SOMEXTERN struct CosLifeCycle_FactoryFinderCClassDataStructure _CosLifeCycle_FactoryFinderCClassData;
				#ifndef CosLifeCycle_FactoryFinderCClassData
					#define CosLifeCycle_FactoryFinderCClassData    _CosLifeCycle_FactoryFinderCClassData
				#endif /* CosLifeCycle_FactoryFinderCClassData */
			#else
				SOMEXTERN struct CosLifeCycle_FactoryFinderCClassDataStructure * SOMLINK resolve_CosLifeCycle_FactoryFinderCClassData(void);
				#ifndef CosLifeCycle_FactoryFinderCClassData
					#define CosLifeCycle_FactoryFinderCClassData    (*(resolve_CosLifeCycle_FactoryFinderCClassData()))
				#endif /* CosLifeCycle_FactoryFinderCClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_omgestio_Source) || defined(CosLifeCycle_FactoryFinder_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_omgestio_Source || CosLifeCycle_FactoryFinder_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_omgestio_Source || CosLifeCycle_FactoryFinder_Class_Source */
			struct CosLifeCycle_FactoryFinderCClassDataStructure SOMDLINK CosLifeCycle_FactoryFinderCClassData;
		#endif /* SOM_RESOLVE_DATA */
		extern 
		#if defined(SOM_Module_omgestio_Source) || defined(CosLifeCycle_FactoryFinder_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_omgestio_Source || CosLifeCycle_FactoryFinder_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_omgestio_Source || CosLifeCycle_FactoryFinder_Class_Source */
		SOMClass SOMSTAR SOMLINK CosLifeCycle_FactoryFinderNewClass
				(integer4 somtmajorVersion,integer4 somtminorVersion);
		#define _SOMCLASS_CosLifeCycle_FactoryFinder (CosLifeCycle_FactoryFinderClassData.classObject)
		#ifndef SOMGD_CosLifeCycle_FactoryFinder
			#if (defined(_CosLifeCycle_FactoryFinder) || defined(__CosLifeCycle_FactoryFinder))
				#undef _CosLifeCycle_FactoryFinder
				#undef __CosLifeCycle_FactoryFinder
				#define SOMGD_CosLifeCycle_FactoryFinder 1
			#else
				#define _CosLifeCycle_FactoryFinder _SOMCLASS_CosLifeCycle_FactoryFinder
			#endif /* _CosLifeCycle_FactoryFinder */
		#endif /* SOMGD_CosLifeCycle_FactoryFinder */
		#define CosLifeCycle_FactoryFinder_classObj _SOMCLASS_CosLifeCycle_FactoryFinder
		#define _SOMMTOKEN_CosLifeCycle_FactoryFinder(method) ((somMToken)(CosLifeCycle_FactoryFinderClassData.method))
		#ifndef CosLifeCycle_FactoryFinderNew
			#define CosLifeCycle_FactoryFinderNew() ( _CosLifeCycle_FactoryFinder ? \
				(SOMClass_somNew(_CosLifeCycle_FactoryFinder)) : \
				( CosLifeCycle_FactoryFinderNewClass( \
					CosLifeCycle_FactoryFinder_MajorVersion, \
					CosLifeCycle_FactoryFinder_MinorVersion), \
				SOMClass_somNew(_CosLifeCycle_FactoryFinder))) 
		#endif /* NewCosLifeCycle_FactoryFinder */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define CosLifeCycle_FactoryFinder_somInit SOMObject_somInit
#define CosLifeCycle_FactoryFinder_somUninit SOMObject_somUninit
#define CosLifeCycle_FactoryFinder_somFree SOMObject_somFree
#define CosLifeCycle_FactoryFinder_somGetClass SOMObject_somGetClass
#define CosLifeCycle_FactoryFinder_somGetClassName SOMObject_somGetClassName
#define CosLifeCycle_FactoryFinder_somGetSize SOMObject_somGetSize
#define CosLifeCycle_FactoryFinder_somIsA SOMObject_somIsA
#define CosLifeCycle_FactoryFinder_somIsInstanceOf SOMObject_somIsInstanceOf
#define CosLifeCycle_FactoryFinder_somRespondsTo SOMObject_somRespondsTo
#define CosLifeCycle_FactoryFinder_somDispatch SOMObject_somDispatch
#define CosLifeCycle_FactoryFinder_somClassDispatch SOMObject_somClassDispatch
#define CosLifeCycle_FactoryFinder_somCastObj SOMObject_somCastObj
#define CosLifeCycle_FactoryFinder_somResetObj SOMObject_somResetObj
#define CosLifeCycle_FactoryFinder_somPrintSelf SOMObject_somPrintSelf
#define CosLifeCycle_FactoryFinder_somDumpSelf SOMObject_somDumpSelf
#define CosLifeCycle_FactoryFinder_somDumpSelfInt SOMObject_somDumpSelfInt
#define CosLifeCycle_FactoryFinder_somDefaultInit SOMObject_somDefaultInit
#define CosLifeCycle_FactoryFinder_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define CosLifeCycle_FactoryFinder_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define CosLifeCycle_FactoryFinder_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define CosLifeCycle_FactoryFinder_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define CosLifeCycle_FactoryFinder_somDefaultAssign SOMObject_somDefaultAssign
#define CosLifeCycle_FactoryFinder_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define CosLifeCycle_FactoryFinder_somDefaultVAssign SOMObject_somDefaultVAssign
#define CosLifeCycle_FactoryFinder_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define CosLifeCycle_FactoryFinder_somDestruct SOMObject_somDestruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

		#ifndef _IDL_CosStream_StreamIO_defined
			#define _IDL_CosStream_StreamIO_defined
			typedef SOMObject CosStream_StreamIO;
		#endif /* _IDL_CosStream_StreamIO_defined */
		#ifndef _IDL_SEQUENCE_CosStream_StreamIO_defined
			#define _IDL_SEQUENCE_CosStream_StreamIO_defined
			SOM_SEQUENCE_TYPEDEF_NAME(CosStream_StreamIO SOMSTAR ,sequence(CosStream_StreamIO));
		#endif /* _IDL_SEQUENCE_CosStream_StreamIO_defined */
		#ifndef _IDL_CosStream_Streamable_defined
			#define _IDL_CosStream_Streamable_defined
			typedef SOMObject CosStream_Streamable;
		#endif /* _IDL_CosStream_Streamable_defined */
		#ifndef _IDL_SEQUENCE_CosStream_Streamable_defined
			#define _IDL_SEQUENCE_CosStream_Streamable_defined
			SOM_SEQUENCE_TYPEDEF_NAME(CosStream_Streamable SOMSTAR ,sequence(CosStream_Streamable));
		#endif /* _IDL_SEQUENCE_CosStream_Streamable_defined */
		#ifndef _IDL_CosStream_StreamableFactory_defined
			#define _IDL_CosStream_StreamableFactory_defined
			typedef SOMObject CosStream_StreamableFactory;
		#endif /* _IDL_CosStream_StreamableFactory_defined */
		#ifndef _IDL_SEQUENCE_CosStream_StreamableFactory_defined
			#define _IDL_SEQUENCE_CosStream_StreamableFactory_defined
			SOM_SEQUENCE_TYPEDEF_NAME(CosStream_StreamableFactory SOMSTAR ,sequence(CosStream_StreamableFactory));
		#endif /* _IDL_SEQUENCE_CosStream_StreamableFactory_defined */
#define ex_CosStream_ObjectCreationError   "::CosStream::ObjectCreationError"
#ifndef SOM_DONT_USE_SHORT_NAMES
#ifndef SOMTGD_ex_ObjectCreationError
#ifndef ex_ObjectCreationError
#define ex_ObjectCreationError  ex_CosStream_ObjectCreationError
#else
#define SOMTGD_ex_ObjectCreationError
#undef ex_ObjectCreationError
#endif /* ex_ObjectCreationError */
#endif /* SOMTGD_ex_ObjectCreationError */
#endif /* SOM_DONT_USE_SHORT_NAMES */
typedef struct CosStream_ObjectCreationError CosStream_ObjectCreationError;
#ifndef SOM_DONT_USE_SHORT_NAMES
	#ifndef SOMGD_ObjectCreationError
		#if defined(ObjectCreationError)
			#undef ObjectCreationError
			#define SOMGD_ObjectCreationError
		#else
			#define ObjectCreationError CosStream_ObjectCreationError
		#endif
	#endif /* SOMGD_ObjectCreationError */
#endif /* SOM_DONT_USE_SHORT_NAMES */
#define ex_CosStream_StreamDataFormatError   "::CosStream::StreamDataFormatError"
#ifndef SOM_DONT_USE_SHORT_NAMES
#ifndef SOMTGD_ex_StreamDataFormatError
#ifndef ex_StreamDataFormatError
#define ex_StreamDataFormatError  ex_CosStream_StreamDataFormatError
#else
#define SOMTGD_ex_StreamDataFormatError
#undef ex_StreamDataFormatError
#endif /* ex_StreamDataFormatError */
#endif /* SOMTGD_ex_StreamDataFormatError */
#endif /* SOM_DONT_USE_SHORT_NAMES */
typedef struct CosStream_StreamDataFormatError CosStream_StreamDataFormatError;
#ifndef SOM_DONT_USE_SHORT_NAMES
	#ifndef SOMGD_StreamDataFormatError
		#if defined(StreamDataFormatError)
			#undef StreamDataFormatError
			#define SOMGD_StreamDataFormatError
		#else
			#define StreamDataFormatError CosStream_StreamDataFormatError
		#endif
	#endif /* SOMGD_StreamDataFormatError */
#endif /* SOM_DONT_USE_SHORT_NAMES */
		#ifdef __IBMC__
			typedef void (somTP_CosStream_StreamIO_write_string)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev,
				/* in */ string item);
			#pragma linkage(somTP_CosStream_StreamIO_write_string,system)
			typedef somTP_CosStream_StreamIO_write_string *somTD_CosStream_StreamIO_write_string;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_CosStream_StreamIO_write_string)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev,
				/* in */ string item);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_CosStream_StreamIO_write_char)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev,
				/* in */ char item);
			#pragma linkage(somTP_CosStream_StreamIO_write_char,system)
			typedef somTP_CosStream_StreamIO_write_char *somTD_CosStream_StreamIO_write_char;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_CosStream_StreamIO_write_char)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev,
				/* in */ char item);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_CosStream_StreamIO_write_octet)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev,
				/* in */ octet item);
			#pragma linkage(somTP_CosStream_StreamIO_write_octet,system)
			typedef somTP_CosStream_StreamIO_write_octet *somTD_CosStream_StreamIO_write_octet;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_CosStream_StreamIO_write_octet)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev,
				/* in */ octet item);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_CosStream_StreamIO_write_unsigned_long)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev,
				/* in */ unsigned long item);
			#pragma linkage(somTP_CosStream_StreamIO_write_unsigned_long,system)
			typedef somTP_CosStream_StreamIO_write_unsigned_long *somTD_CosStream_StreamIO_write_unsigned_long;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_CosStream_StreamIO_write_unsigned_long)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev,
				/* in */ unsigned long item);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_CosStream_StreamIO_write_unsigned_short)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev,
				/* in */ unsigned short item);
			#pragma linkage(somTP_CosStream_StreamIO_write_unsigned_short,system)
			typedef somTP_CosStream_StreamIO_write_unsigned_short *somTD_CosStream_StreamIO_write_unsigned_short;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_CosStream_StreamIO_write_unsigned_short)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev,
				/* in */ unsigned short item);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_CosStream_StreamIO_write_long)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev,
				/* in */ long item);
			#pragma linkage(somTP_CosStream_StreamIO_write_long,system)
			typedef somTP_CosStream_StreamIO_write_long *somTD_CosStream_StreamIO_write_long;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_CosStream_StreamIO_write_long)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev,
				/* in */ long item);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_CosStream_StreamIO_write_short)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev,
				/* in */ short item);
			#pragma linkage(somTP_CosStream_StreamIO_write_short,system)
			typedef somTP_CosStream_StreamIO_write_short *somTD_CosStream_StreamIO_write_short;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_CosStream_StreamIO_write_short)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev,
				/* in */ short item);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_CosStream_StreamIO_write_float)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev,
				/* in */ float item);
			#pragma linkage(somTP_CosStream_StreamIO_write_float,system)
			typedef somTP_CosStream_StreamIO_write_float *somTD_CosStream_StreamIO_write_float;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_CosStream_StreamIO_write_float)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev,
				/* in */ float item);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_CosStream_StreamIO_write_double)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev,
				/* in */ double item);
			#pragma linkage(somTP_CosStream_StreamIO_write_double,system)
			typedef somTP_CosStream_StreamIO_write_double *somTD_CosStream_StreamIO_write_double;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_CosStream_StreamIO_write_double)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev,
				/* in */ double item);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_CosStream_StreamIO_write_boolean)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev,
				/* in */ boolean item);
			#pragma linkage(somTP_CosStream_StreamIO_write_boolean,system)
			typedef somTP_CosStream_StreamIO_write_boolean *somTD_CosStream_StreamIO_write_boolean;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_CosStream_StreamIO_write_boolean)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev,
				/* in */ boolean item);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_CosStream_StreamIO_write_object)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosStream_Streamable SOMSTAR item);
			#pragma linkage(somTP_CosStream_StreamIO_write_object,system)
			typedef somTP_CosStream_StreamIO_write_object *somTD_CosStream_StreamIO_write_object;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_CosStream_StreamIO_write_object)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosStream_Streamable SOMSTAR item);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef string (somTP_CosStream_StreamIO_read_string)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev);
			#pragma linkage(somTP_CosStream_StreamIO_read_string,system)
			typedef somTP_CosStream_StreamIO_read_string *somTD_CosStream_StreamIO_read_string;
		#else /* __IBMC__ */
			typedef string (SOMLINK * somTD_CosStream_StreamIO_read_string)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef char (somTP_CosStream_StreamIO_read_char)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev);
			#pragma linkage(somTP_CosStream_StreamIO_read_char,system)
			typedef somTP_CosStream_StreamIO_read_char *somTD_CosStream_StreamIO_read_char;
		#else /* __IBMC__ */
			typedef char (SOMLINK * somTD_CosStream_StreamIO_read_char)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef octet (somTP_CosStream_StreamIO_read_octet)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev);
			#pragma linkage(somTP_CosStream_StreamIO_read_octet,system)
			typedef somTP_CosStream_StreamIO_read_octet *somTD_CosStream_StreamIO_read_octet;
		#else /* __IBMC__ */
			typedef octet (SOMLINK * somTD_CosStream_StreamIO_read_octet)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef unsigned long (somTP_CosStream_StreamIO_read_unsigned_long)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev);
			#pragma linkage(somTP_CosStream_StreamIO_read_unsigned_long,system)
			typedef somTP_CosStream_StreamIO_read_unsigned_long *somTD_CosStream_StreamIO_read_unsigned_long;
		#else /* __IBMC__ */
			typedef unsigned long (SOMLINK * somTD_CosStream_StreamIO_read_unsigned_long)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef unsigned short (somTP_CosStream_StreamIO_read_unsigned_short)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev);
			#pragma linkage(somTP_CosStream_StreamIO_read_unsigned_short,system)
			typedef somTP_CosStream_StreamIO_read_unsigned_short *somTD_CosStream_StreamIO_read_unsigned_short;
		#else /* __IBMC__ */
			typedef unsigned short (SOMLINK * somTD_CosStream_StreamIO_read_unsigned_short)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef long (somTP_CosStream_StreamIO_read_long)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev);
			#pragma linkage(somTP_CosStream_StreamIO_read_long,system)
			typedef somTP_CosStream_StreamIO_read_long *somTD_CosStream_StreamIO_read_long;
		#else /* __IBMC__ */
			typedef long (SOMLINK * somTD_CosStream_StreamIO_read_long)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef short (somTP_CosStream_StreamIO_read_short)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev);
			#pragma linkage(somTP_CosStream_StreamIO_read_short,system)
			typedef somTP_CosStream_StreamIO_read_short *somTD_CosStream_StreamIO_read_short;
		#else /* __IBMC__ */
			typedef short (SOMLINK * somTD_CosStream_StreamIO_read_short)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef float (somTP_CosStream_StreamIO_read_float)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev);
			#pragma linkage(somTP_CosStream_StreamIO_read_float,system)
			typedef somTP_CosStream_StreamIO_read_float *somTD_CosStream_StreamIO_read_float;
		#else /* __IBMC__ */
			typedef float (SOMLINK * somTD_CosStream_StreamIO_read_float)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef double (somTP_CosStream_StreamIO_read_double)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev);
			#pragma linkage(somTP_CosStream_StreamIO_read_double,system)
			typedef somTP_CosStream_StreamIO_read_double *somTD_CosStream_StreamIO_read_double;
		#else /* __IBMC__ */
			typedef double (SOMLINK * somTD_CosStream_StreamIO_read_double)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef boolean (somTP_CosStream_StreamIO_read_boolean)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev);
			#pragma linkage(somTP_CosStream_StreamIO_read_boolean,system)
			typedef somTP_CosStream_StreamIO_read_boolean *somTD_CosStream_StreamIO_read_boolean;
		#else /* __IBMC__ */
			typedef boolean (SOMLINK * somTD_CosStream_StreamIO_read_boolean)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef CosStream_Streamable SOMSTAR (somTP_CosStream_StreamIO_read_object)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosLifeCycle_FactoryFinder SOMSTAR ff,
				/* in */ CosStream_Streamable SOMSTAR obj);
			#pragma linkage(somTP_CosStream_StreamIO_read_object,system)
			typedef somTP_CosStream_StreamIO_read_object *somTD_CosStream_StreamIO_read_object;
		#else /* __IBMC__ */
			typedef CosStream_Streamable SOMSTAR (SOMLINK * somTD_CosStream_StreamIO_read_object)(
				CosStream_StreamIO SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosLifeCycle_FactoryFinder SOMSTAR ff,
				/* in */ CosStream_Streamable SOMSTAR obj);
		#endif /* __IBMC__ */
		#ifndef CosStream_StreamIO_MajorVersion
			#define CosStream_StreamIO_MajorVersion   3
		#endif /* CosStream_StreamIO_MajorVersion */
		#ifndef CosStream_StreamIO_MinorVersion
			#define CosStream_StreamIO_MinorVersion   0
		#endif /* CosStream_StreamIO_MinorVersion */
		typedef struct CosStream_StreamIOClassDataStructure
		{
			SOMClass SOMSTAR classObject;
			somMToken read_short;
			somMToken write_short;
			somMToken read_long;
			somMToken write_long;
			somMToken read_unsigned_short;
			somMToken write_unsigned_short;
			somMToken read_unsigned_long;
			somMToken write_unsigned_long;
			somMToken read_float;
			somMToken write_float;
			somMToken read_double;
			somMToken write_double;
			somMToken read_boolean;
			somMToken write_boolean;
			somMToken read_char;
			somMToken write_char;
			somMToken read_octet;
			somMToken write_octet;
			somMToken read_string;
			somMToken write_string;
			somMToken read_object;
			somMToken write_object;
		} CosStream_StreamIOClassDataStructure;
		typedef struct CosStream_StreamIOCClassDataStructure
		{
			somMethodTabs parentMtab;
			somDToken instanceDataToken;
		} CosStream_StreamIOCClassDataStructure;
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_omgestio_Source) || defined(CosStream_StreamIO_Class_Source)
				SOMEXTERN struct CosStream_StreamIOClassDataStructure _CosStream_StreamIOClassData;
				#ifndef CosStream_StreamIOClassData
					#define CosStream_StreamIOClassData    _CosStream_StreamIOClassData
				#endif /* CosStream_StreamIOClassData */
			#else
				SOMEXTERN struct CosStream_StreamIOClassDataStructure * SOMLINK resolve_CosStream_StreamIOClassData(void);
				#ifndef CosStream_StreamIOClassData
					#define CosStream_StreamIOClassData    (*(resolve_CosStream_StreamIOClassData()))
				#endif /* CosStream_StreamIOClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_omgestio_Source) || defined(CosStream_StreamIO_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_omgestio_Source || CosStream_StreamIO_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_omgestio_Source || CosStream_StreamIO_Class_Source */
			struct CosStream_StreamIOClassDataStructure SOMDLINK CosStream_StreamIOClassData;
		#endif /* SOM_RESOLVE_DATA */
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_omgestio_Source) || defined(CosStream_StreamIO_Class_Source)
				SOMEXTERN struct CosStream_StreamIOCClassDataStructure _CosStream_StreamIOCClassData;
				#ifndef CosStream_StreamIOCClassData
					#define CosStream_StreamIOCClassData    _CosStream_StreamIOCClassData
				#endif /* CosStream_StreamIOCClassData */
			#else
				SOMEXTERN struct CosStream_StreamIOCClassDataStructure * SOMLINK resolve_CosStream_StreamIOCClassData(void);
				#ifndef CosStream_StreamIOCClassData
					#define CosStream_StreamIOCClassData    (*(resolve_CosStream_StreamIOCClassData()))
				#endif /* CosStream_StreamIOCClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_omgestio_Source) || defined(CosStream_StreamIO_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_omgestio_Source || CosStream_StreamIO_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_omgestio_Source || CosStream_StreamIO_Class_Source */
			struct CosStream_StreamIOCClassDataStructure SOMDLINK CosStream_StreamIOCClassData;
		#endif /* SOM_RESOLVE_DATA */
		extern 
		#if defined(SOM_Module_omgestio_Source) || defined(CosStream_StreamIO_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_omgestio_Source || CosStream_StreamIO_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_omgestio_Source || CosStream_StreamIO_Class_Source */
		SOMClass SOMSTAR SOMLINK CosStream_StreamIONewClass
				(integer4 somtmajorVersion,integer4 somtminorVersion);
		#define _SOMCLASS_CosStream_StreamIO (CosStream_StreamIOClassData.classObject)
		#ifndef SOMGD_CosStream_StreamIO
			#if (defined(_CosStream_StreamIO) || defined(__CosStream_StreamIO))
				#undef _CosStream_StreamIO
				#undef __CosStream_StreamIO
				#define SOMGD_CosStream_StreamIO 1
			#else
				#define _CosStream_StreamIO _SOMCLASS_CosStream_StreamIO
			#endif /* _CosStream_StreamIO */
		#endif /* SOMGD_CosStream_StreamIO */
		#define CosStream_StreamIO_classObj _SOMCLASS_CosStream_StreamIO
		#define _SOMMTOKEN_CosStream_StreamIO(method) ((somMToken)(CosStream_StreamIOClassData.method))
		#ifndef CosStream_StreamIONew
			#define CosStream_StreamIONew() ( _CosStream_StreamIO ? \
				(SOMClass_somNew(_CosStream_StreamIO)) : \
				( CosStream_StreamIONewClass( \
					CosStream_StreamIO_MajorVersion, \
					CosStream_StreamIO_MinorVersion), \
				SOMClass_somNew(_CosStream_StreamIO))) 
		#endif /* NewCosStream_StreamIO */
		#ifndef CosStream_StreamIO_write_string
			#define CosStream_StreamIO_write_string(somSelf,ev,item) \
				SOM_Resolve(somSelf,CosStream_StreamIO,write_string)  \
					(somSelf,ev,item)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__write_string
					#if defined(_write_string)
						#undef _write_string
						#define SOMGD__write_string
					#else
						#define _write_string CosStream_StreamIO_write_string
					#endif
				#endif /* SOMGD__write_string */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosStream_StreamIO_write_string */
		#ifndef CosStream_StreamIO_write_char
			#define CosStream_StreamIO_write_char(somSelf,ev,item) \
				SOM_Resolve(somSelf,CosStream_StreamIO,write_char)  \
					(somSelf,ev,item)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__write_char
					#if defined(_write_char)
						#undef _write_char
						#define SOMGD__write_char
					#else
						#define _write_char CosStream_StreamIO_write_char
					#endif
				#endif /* SOMGD__write_char */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosStream_StreamIO_write_char */
		#ifndef CosStream_StreamIO_write_octet
			#define CosStream_StreamIO_write_octet(somSelf,ev,item) \
				SOM_Resolve(somSelf,CosStream_StreamIO,write_octet)  \
					(somSelf,ev,item)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__write_octet
					#if defined(_write_octet)
						#undef _write_octet
						#define SOMGD__write_octet
					#else
						#define _write_octet CosStream_StreamIO_write_octet
					#endif
				#endif /* SOMGD__write_octet */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosStream_StreamIO_write_octet */
		#ifndef CosStream_StreamIO_write_unsigned_long
			#define CosStream_StreamIO_write_unsigned_long(somSelf,ev,item) \
				SOM_Resolve(somSelf,CosStream_StreamIO,write_unsigned_long)  \
					(somSelf,ev,item)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__write_unsigned_long
					#if defined(_write_unsigned_long)
						#undef _write_unsigned_long
						#define SOMGD__write_unsigned_long
					#else
						#define _write_unsigned_long CosStream_StreamIO_write_unsigned_long
					#endif
				#endif /* SOMGD__write_unsigned_long */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosStream_StreamIO_write_unsigned_long */
		#ifndef CosStream_StreamIO_write_unsigned_short
			#define CosStream_StreamIO_write_unsigned_short(somSelf,ev,item) \
				SOM_Resolve(somSelf,CosStream_StreamIO,write_unsigned_short)  \
					(somSelf,ev,item)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__write_unsigned_short
					#if defined(_write_unsigned_short)
						#undef _write_unsigned_short
						#define SOMGD__write_unsigned_short
					#else
						#define _write_unsigned_short CosStream_StreamIO_write_unsigned_short
					#endif
				#endif /* SOMGD__write_unsigned_short */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosStream_StreamIO_write_unsigned_short */
		#ifndef CosStream_StreamIO_write_long
			#define CosStream_StreamIO_write_long(somSelf,ev,item) \
				SOM_Resolve(somSelf,CosStream_StreamIO,write_long)  \
					(somSelf,ev,item)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__write_long
					#if defined(_write_long)
						#undef _write_long
						#define SOMGD__write_long
					#else
						#define _write_long CosStream_StreamIO_write_long
					#endif
				#endif /* SOMGD__write_long */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosStream_StreamIO_write_long */
		#ifndef CosStream_StreamIO_write_short
			#define CosStream_StreamIO_write_short(somSelf,ev,item) \
				SOM_Resolve(somSelf,CosStream_StreamIO,write_short)  \
					(somSelf,ev,item)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__write_short
					#if defined(_write_short)
						#undef _write_short
						#define SOMGD__write_short
					#else
						#define _write_short CosStream_StreamIO_write_short
					#endif
				#endif /* SOMGD__write_short */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosStream_StreamIO_write_short */
		#ifndef CosStream_StreamIO_write_float
			#define CosStream_StreamIO_write_float(somSelf,ev,item) \
				SOM_Resolve(somSelf,CosStream_StreamIO,write_float)  \
					(somSelf,ev,item)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__write_float
					#if defined(_write_float)
						#undef _write_float
						#define SOMGD__write_float
					#else
						#define _write_float CosStream_StreamIO_write_float
					#endif
				#endif /* SOMGD__write_float */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosStream_StreamIO_write_float */
		#ifndef CosStream_StreamIO_write_double
			#define CosStream_StreamIO_write_double(somSelf,ev,item) \
				SOM_Resolve(somSelf,CosStream_StreamIO,write_double)  \
					(somSelf,ev,item)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__write_double
					#if defined(_write_double)
						#undef _write_double
						#define SOMGD__write_double
					#else
						#define _write_double CosStream_StreamIO_write_double
					#endif
				#endif /* SOMGD__write_double */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosStream_StreamIO_write_double */
		#ifndef CosStream_StreamIO_write_boolean
			#define CosStream_StreamIO_write_boolean(somSelf,ev,item) \
				SOM_Resolve(somSelf,CosStream_StreamIO,write_boolean)  \
					(somSelf,ev,item)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__write_boolean
					#if defined(_write_boolean)
						#undef _write_boolean
						#define SOMGD__write_boolean
					#else
						#define _write_boolean CosStream_StreamIO_write_boolean
					#endif
				#endif /* SOMGD__write_boolean */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosStream_StreamIO_write_boolean */
		#ifndef CosStream_StreamIO_write_object
			#define CosStream_StreamIO_write_object(somSelf,ev,item) \
				SOM_Resolve(somSelf,CosStream_StreamIO,write_object)  \
					(somSelf,ev,item)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__write_object
					#if defined(_write_object)
						#undef _write_object
						#define SOMGD__write_object
					#else
						#define _write_object CosStream_StreamIO_write_object
					#endif
				#endif /* SOMGD__write_object */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosStream_StreamIO_write_object */
		#ifndef CosStream_StreamIO_read_string
			#define CosStream_StreamIO_read_string(somSelf,ev) \
				SOM_Resolve(somSelf,CosStream_StreamIO,read_string)  \
					(somSelf,ev)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__read_string
					#if defined(_read_string)
						#undef _read_string
						#define SOMGD__read_string
					#else
						#define _read_string CosStream_StreamIO_read_string
					#endif
				#endif /* SOMGD__read_string */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosStream_StreamIO_read_string */
		#ifndef CosStream_StreamIO_read_char
			#define CosStream_StreamIO_read_char(somSelf,ev) \
				SOM_Resolve(somSelf,CosStream_StreamIO,read_char)  \
					(somSelf,ev)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__read_char
					#if defined(_read_char)
						#undef _read_char
						#define SOMGD__read_char
					#else
						#define _read_char CosStream_StreamIO_read_char
					#endif
				#endif /* SOMGD__read_char */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosStream_StreamIO_read_char */
		#ifndef CosStream_StreamIO_read_octet
			#define CosStream_StreamIO_read_octet(somSelf,ev) \
				SOM_Resolve(somSelf,CosStream_StreamIO,read_octet)  \
					(somSelf,ev)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__read_octet
					#if defined(_read_octet)
						#undef _read_octet
						#define SOMGD__read_octet
					#else
						#define _read_octet CosStream_StreamIO_read_octet
					#endif
				#endif /* SOMGD__read_octet */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosStream_StreamIO_read_octet */
		#ifndef CosStream_StreamIO_read_unsigned_long
			#define CosStream_StreamIO_read_unsigned_long(somSelf,ev) \
				SOM_Resolve(somSelf,CosStream_StreamIO,read_unsigned_long)  \
					(somSelf,ev)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__read_unsigned_long
					#if defined(_read_unsigned_long)
						#undef _read_unsigned_long
						#define SOMGD__read_unsigned_long
					#else
						#define _read_unsigned_long CosStream_StreamIO_read_unsigned_long
					#endif
				#endif /* SOMGD__read_unsigned_long */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosStream_StreamIO_read_unsigned_long */
		#ifndef CosStream_StreamIO_read_unsigned_short
			#define CosStream_StreamIO_read_unsigned_short(somSelf,ev) \
				SOM_Resolve(somSelf,CosStream_StreamIO,read_unsigned_short)  \
					(somSelf,ev)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__read_unsigned_short
					#if defined(_read_unsigned_short)
						#undef _read_unsigned_short
						#define SOMGD__read_unsigned_short
					#else
						#define _read_unsigned_short CosStream_StreamIO_read_unsigned_short
					#endif
				#endif /* SOMGD__read_unsigned_short */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosStream_StreamIO_read_unsigned_short */
		#ifndef CosStream_StreamIO_read_long
			#define CosStream_StreamIO_read_long(somSelf,ev) \
				SOM_Resolve(somSelf,CosStream_StreamIO,read_long)  \
					(somSelf,ev)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__read_long
					#if defined(_read_long)
						#undef _read_long
						#define SOMGD__read_long
					#else
						#define _read_long CosStream_StreamIO_read_long
					#endif
				#endif /* SOMGD__read_long */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosStream_StreamIO_read_long */
		#ifndef CosStream_StreamIO_read_short
			#define CosStream_StreamIO_read_short(somSelf,ev) \
				SOM_Resolve(somSelf,CosStream_StreamIO,read_short)  \
					(somSelf,ev)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__read_short
					#if defined(_read_short)
						#undef _read_short
						#define SOMGD__read_short
					#else
						#define _read_short CosStream_StreamIO_read_short
					#endif
				#endif /* SOMGD__read_short */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosStream_StreamIO_read_short */
		#ifndef CosStream_StreamIO_read_float
			#define CosStream_StreamIO_read_float(somSelf,ev) \
				SOM_Resolve(somSelf,CosStream_StreamIO,read_float)  \
					(somSelf,ev)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__read_float
					#if defined(_read_float)
						#undef _read_float
						#define SOMGD__read_float
					#else
						#define _read_float CosStream_StreamIO_read_float
					#endif
				#endif /* SOMGD__read_float */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosStream_StreamIO_read_float */
		#ifndef CosStream_StreamIO_read_double
			#define CosStream_StreamIO_read_double(somSelf,ev) \
				SOM_Resolve(somSelf,CosStream_StreamIO,read_double)  \
					(somSelf,ev)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__read_double
					#if defined(_read_double)
						#undef _read_double
						#define SOMGD__read_double
					#else
						#define _read_double CosStream_StreamIO_read_double
					#endif
				#endif /* SOMGD__read_double */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosStream_StreamIO_read_double */
		#ifndef CosStream_StreamIO_read_boolean
			#define CosStream_StreamIO_read_boolean(somSelf,ev) \
				SOM_Resolve(somSelf,CosStream_StreamIO,read_boolean)  \
					(somSelf,ev)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__read_boolean
					#if defined(_read_boolean)
						#undef _read_boolean
						#define SOMGD__read_boolean
					#else
						#define _read_boolean CosStream_StreamIO_read_boolean
					#endif
				#endif /* SOMGD__read_boolean */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosStream_StreamIO_read_boolean */
		#ifndef CosStream_StreamIO_read_object
			#define CosStream_StreamIO_read_object(somSelf,ev,ff,obj) \
				SOM_Resolve(somSelf,CosStream_StreamIO,read_object)  \
					(somSelf,ev,ff,obj)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__read_object
					#if defined(_read_object)
						#undef _read_object
						#define SOMGD__read_object
					#else
						#define _read_object CosStream_StreamIO_read_object
					#endif
				#endif /* SOMGD__read_object */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosStream_StreamIO_read_object */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define CosStream_StreamIO_somInit SOMObject_somInit
#define CosStream_StreamIO_somUninit SOMObject_somUninit
#define CosStream_StreamIO_somFree SOMObject_somFree
#define CosStream_StreamIO_somGetClass SOMObject_somGetClass
#define CosStream_StreamIO_somGetClassName SOMObject_somGetClassName
#define CosStream_StreamIO_somGetSize SOMObject_somGetSize
#define CosStream_StreamIO_somIsA SOMObject_somIsA
#define CosStream_StreamIO_somIsInstanceOf SOMObject_somIsInstanceOf
#define CosStream_StreamIO_somRespondsTo SOMObject_somRespondsTo
#define CosStream_StreamIO_somDispatch SOMObject_somDispatch
#define CosStream_StreamIO_somClassDispatch SOMObject_somClassDispatch
#define CosStream_StreamIO_somCastObj SOMObject_somCastObj
#define CosStream_StreamIO_somResetObj SOMObject_somResetObj
#define CosStream_StreamIO_somPrintSelf SOMObject_somPrintSelf
#define CosStream_StreamIO_somDumpSelf SOMObject_somDumpSelf
#define CosStream_StreamIO_somDumpSelfInt SOMObject_somDumpSelfInt
#define CosStream_StreamIO_somDefaultInit SOMObject_somDefaultInit
#define CosStream_StreamIO_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define CosStream_StreamIO_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define CosStream_StreamIO_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define CosStream_StreamIO_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define CosStream_StreamIO_somDefaultAssign SOMObject_somDefaultAssign
#define CosStream_StreamIO_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define CosStream_StreamIO_somDefaultVAssign SOMObject_somDefaultVAssign
#define CosStream_StreamIO_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define CosStream_StreamIO_somDestruct SOMObject_somDestruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

		#ifdef __IBMC__
			typedef void (somTP_CosStream_Streamable_externalize_to_stream)(
				CosStream_Streamable SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosStream_StreamIO SOMSTAR stream);
			#pragma linkage(somTP_CosStream_Streamable_externalize_to_stream,system)
			typedef somTP_CosStream_Streamable_externalize_to_stream *somTD_CosStream_Streamable_externalize_to_stream;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_CosStream_Streamable_externalize_to_stream)(
				CosStream_Streamable SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosStream_StreamIO SOMSTAR stream);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef void (somTP_CosStream_Streamable_internalize_from_stream)(
				CosStream_Streamable SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosStream_StreamIO SOMSTAR stream,
				/* in */ CosLifeCycle_FactoryFinder SOMSTAR ff);
			#pragma linkage(somTP_CosStream_Streamable_internalize_from_stream,system)
			typedef somTP_CosStream_Streamable_internalize_from_stream *somTD_CosStream_Streamable_internalize_from_stream;
		#else /* __IBMC__ */
			typedef void (SOMLINK * somTD_CosStream_Streamable_internalize_from_stream)(
				CosStream_Streamable SOMSTAR somSelf,
				Environment *ev,
				/* in */ CosStream_StreamIO SOMSTAR stream,
				/* in */ CosLifeCycle_FactoryFinder SOMSTAR ff);
		#endif /* __IBMC__ */
		#ifdef __IBMC__
			typedef CosLifeCycle_Key (somTP_CosStream_Streamable__get_external_form_id)(
				CosStream_Streamable SOMSTAR somSelf,
				Environment *ev);
			#pragma linkage(somTP_CosStream_Streamable__get_external_form_id,system)
			typedef somTP_CosStream_Streamable__get_external_form_id *somTD_CosStream_Streamable__get_external_form_id;
		#else /* __IBMC__ */
			typedef CosLifeCycle_Key (SOMLINK * somTD_CosStream_Streamable__get_external_form_id)(
				CosStream_Streamable SOMSTAR somSelf,
				Environment *ev);
		#endif /* __IBMC__ */
		#ifndef CosStream_Streamable_MajorVersion
			#define CosStream_Streamable_MajorVersion   3
		#endif /* CosStream_Streamable_MajorVersion */
		#ifndef CosStream_Streamable_MinorVersion
			#define CosStream_Streamable_MinorVersion   0
		#endif /* CosStream_Streamable_MinorVersion */
		typedef struct CosStream_StreamableClassDataStructure
		{
			SOMClass SOMSTAR classObject;
			somMToken externalize_to_stream;
			somMToken internalize_from_stream;
			somMToken _get_external_form_id;
		} CosStream_StreamableClassDataStructure;
		typedef struct CosStream_StreamableCClassDataStructure
		{
			somMethodTabs parentMtab;
			somDToken instanceDataToken;
		} CosStream_StreamableCClassDataStructure;
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_omgestio_Source) || defined(CosStream_Streamable_Class_Source)
				SOMEXTERN struct CosStream_StreamableClassDataStructure _CosStream_StreamableClassData;
				#ifndef CosStream_StreamableClassData
					#define CosStream_StreamableClassData    _CosStream_StreamableClassData
				#endif /* CosStream_StreamableClassData */
			#else
				SOMEXTERN struct CosStream_StreamableClassDataStructure * SOMLINK resolve_CosStream_StreamableClassData(void);
				#ifndef CosStream_StreamableClassData
					#define CosStream_StreamableClassData    (*(resolve_CosStream_StreamableClassData()))
				#endif /* CosStream_StreamableClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_omgestio_Source) || defined(CosStream_Streamable_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_omgestio_Source || CosStream_Streamable_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_omgestio_Source || CosStream_Streamable_Class_Source */
			struct CosStream_StreamableClassDataStructure SOMDLINK CosStream_StreamableClassData;
		#endif /* SOM_RESOLVE_DATA */
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_omgestio_Source) || defined(CosStream_Streamable_Class_Source)
				SOMEXTERN struct CosStream_StreamableCClassDataStructure _CosStream_StreamableCClassData;
				#ifndef CosStream_StreamableCClassData
					#define CosStream_StreamableCClassData    _CosStream_StreamableCClassData
				#endif /* CosStream_StreamableCClassData */
			#else
				SOMEXTERN struct CosStream_StreamableCClassDataStructure * SOMLINK resolve_CosStream_StreamableCClassData(void);
				#ifndef CosStream_StreamableCClassData
					#define CosStream_StreamableCClassData    (*(resolve_CosStream_StreamableCClassData()))
				#endif /* CosStream_StreamableCClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_omgestio_Source) || defined(CosStream_Streamable_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_omgestio_Source || CosStream_Streamable_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_omgestio_Source || CosStream_Streamable_Class_Source */
			struct CosStream_StreamableCClassDataStructure SOMDLINK CosStream_StreamableCClassData;
		#endif /* SOM_RESOLVE_DATA */
		extern 
		#if defined(SOM_Module_omgestio_Source) || defined(CosStream_Streamable_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_omgestio_Source || CosStream_Streamable_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_omgestio_Source || CosStream_Streamable_Class_Source */
		SOMClass SOMSTAR SOMLINK CosStream_StreamableNewClass
				(integer4 somtmajorVersion,integer4 somtminorVersion);
		#define _SOMCLASS_CosStream_Streamable (CosStream_StreamableClassData.classObject)
		#ifndef SOMGD_CosStream_Streamable
			#if (defined(_CosStream_Streamable) || defined(__CosStream_Streamable))
				#undef _CosStream_Streamable
				#undef __CosStream_Streamable
				#define SOMGD_CosStream_Streamable 1
			#else
				#define _CosStream_Streamable _SOMCLASS_CosStream_Streamable
			#endif /* _CosStream_Streamable */
		#endif /* SOMGD_CosStream_Streamable */
		#define CosStream_Streamable_classObj _SOMCLASS_CosStream_Streamable
		#define _SOMMTOKEN_CosStream_Streamable(method) ((somMToken)(CosStream_StreamableClassData.method))
		#ifndef CosStream_StreamableNew
			#define CosStream_StreamableNew() ( _CosStream_Streamable ? \
				(SOMClass_somNew(_CosStream_Streamable)) : \
				( CosStream_StreamableNewClass( \
					CosStream_Streamable_MajorVersion, \
					CosStream_Streamable_MinorVersion), \
				SOMClass_somNew(_CosStream_Streamable))) 
		#endif /* NewCosStream_Streamable */
		#ifndef CosStream_Streamable_externalize_to_stream
			#define CosStream_Streamable_externalize_to_stream(somSelf,ev,stream) \
				SOM_Resolve(somSelf,CosStream_Streamable,externalize_to_stream)  \
					(somSelf,ev,stream)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__externalize_to_stream
					#if defined(_externalize_to_stream)
						#undef _externalize_to_stream
						#define SOMGD__externalize_to_stream
					#else
						#define _externalize_to_stream CosStream_Streamable_externalize_to_stream
					#endif
				#endif /* SOMGD__externalize_to_stream */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosStream_Streamable_externalize_to_stream */
		#ifndef CosStream_Streamable_internalize_from_stream
			#define CosStream_Streamable_internalize_from_stream(somSelf,ev,stream,ff) \
				SOM_Resolve(somSelf,CosStream_Streamable,internalize_from_stream)  \
					(somSelf,ev,stream,ff)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__internalize_from_stream
					#if defined(_internalize_from_stream)
						#undef _internalize_from_stream
						#define SOMGD__internalize_from_stream
					#else
						#define _internalize_from_stream CosStream_Streamable_internalize_from_stream
					#endif
				#endif /* SOMGD__internalize_from_stream */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosStream_Streamable_internalize_from_stream */
		#ifndef CosStream_Streamable__get_external_form_id
			#define CosStream_Streamable__get_external_form_id(somSelf,ev) \
				SOM_Resolve(somSelf,CosStream_Streamable,_get_external_form_id)  \
					(somSelf,ev)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD___get_external_form_id
					#if defined(__get_external_form_id)
						#undef __get_external_form_id
						#define SOMGD___get_external_form_id
					#else
						#define __get_external_form_id CosStream_Streamable__get_external_form_id
					#endif
				#endif /* SOMGD___get_external_form_id */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosStream_Streamable__get_external_form_id */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define CosStream_Streamable_somInit SOMObject_somInit
#define CosStream_Streamable_somUninit SOMObject_somUninit
#define CosStream_Streamable_somFree SOMObject_somFree
#define CosStream_Streamable_somGetClass SOMObject_somGetClass
#define CosStream_Streamable_somGetClassName SOMObject_somGetClassName
#define CosStream_Streamable_somGetSize SOMObject_somGetSize
#define CosStream_Streamable_somIsA SOMObject_somIsA
#define CosStream_Streamable_somIsInstanceOf SOMObject_somIsInstanceOf
#define CosStream_Streamable_somRespondsTo SOMObject_somRespondsTo
#define CosStream_Streamable_somDispatch SOMObject_somDispatch
#define CosStream_Streamable_somClassDispatch SOMObject_somClassDispatch
#define CosStream_Streamable_somCastObj SOMObject_somCastObj
#define CosStream_Streamable_somResetObj SOMObject_somResetObj
#define CosStream_Streamable_somPrintSelf SOMObject_somPrintSelf
#define CosStream_Streamable_somDumpSelf SOMObject_somDumpSelf
#define CosStream_Streamable_somDumpSelfInt SOMObject_somDumpSelfInt
#define CosStream_Streamable_somDefaultInit SOMObject_somDefaultInit
#define CosStream_Streamable_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define CosStream_Streamable_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define CosStream_Streamable_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define CosStream_Streamable_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define CosStream_Streamable_somDefaultAssign SOMObject_somDefaultAssign
#define CosStream_Streamable_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define CosStream_Streamable_somDefaultVAssign SOMObject_somDefaultVAssign
#define CosStream_Streamable_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define CosStream_Streamable_somDestruct SOMObject_somDestruct
#define CosStream_Streamable__get_constant_random_id CosObjectIdentity_IdentifiableObject__get_constant_random_id
#define CosStream_Streamable_is_identical CosObjectIdentity_IdentifiableObject_is_identical
#endif /* SOM_DONT_USE_INHERITED_MACROS */

		#ifdef __IBMC__
			typedef CosStream_Streamable SOMSTAR (somTP_CosStream_StreamableFactory_create_uninitialized)(
				CosStream_StreamableFactory SOMSTAR somSelf,
				Environment *ev);
			#pragma linkage(somTP_CosStream_StreamableFactory_create_uninitialized,system)
			typedef somTP_CosStream_StreamableFactory_create_uninitialized *somTD_CosStream_StreamableFactory_create_uninitialized;
		#else /* __IBMC__ */
			typedef CosStream_Streamable SOMSTAR (SOMLINK * somTD_CosStream_StreamableFactory_create_uninitialized)(
				CosStream_StreamableFactory SOMSTAR somSelf,
				Environment *ev);
		#endif /* __IBMC__ */
		#ifndef CosStream_StreamableFactory_MajorVersion
			#define CosStream_StreamableFactory_MajorVersion   3
		#endif /* CosStream_StreamableFactory_MajorVersion */
		#ifndef CosStream_StreamableFactory_MinorVersion
			#define CosStream_StreamableFactory_MinorVersion   0
		#endif /* CosStream_StreamableFactory_MinorVersion */
		typedef struct CosStream_StreamableFactoryClassDataStructure
		{
			SOMClass SOMSTAR classObject;
			somMToken create_uninitialized;
		} CosStream_StreamableFactoryClassDataStructure;
		typedef struct CosStream_StreamableFactoryCClassDataStructure
		{
			somMethodTabs parentMtab;
			somDToken instanceDataToken;
		} CosStream_StreamableFactoryCClassDataStructure;
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_omgestio_Source) || defined(CosStream_StreamableFactory_Class_Source)
				SOMEXTERN struct CosStream_StreamableFactoryClassDataStructure _CosStream_StreamableFactoryClassData;
				#ifndef CosStream_StreamableFactoryClassData
					#define CosStream_StreamableFactoryClassData    _CosStream_StreamableFactoryClassData
				#endif /* CosStream_StreamableFactoryClassData */
			#else
				SOMEXTERN struct CosStream_StreamableFactoryClassDataStructure * SOMLINK resolve_CosStream_StreamableFactoryClassData(void);
				#ifndef CosStream_StreamableFactoryClassData
					#define CosStream_StreamableFactoryClassData    (*(resolve_CosStream_StreamableFactoryClassData()))
				#endif /* CosStream_StreamableFactoryClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_omgestio_Source) || defined(CosStream_StreamableFactory_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_omgestio_Source || CosStream_StreamableFactory_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_omgestio_Source || CosStream_StreamableFactory_Class_Source */
			struct CosStream_StreamableFactoryClassDataStructure SOMDLINK CosStream_StreamableFactoryClassData;
		#endif /* SOM_RESOLVE_DATA */
		#ifdef SOM_RESOLVE_DATA
			#if defined(SOM_Module_omgestio_Source) || defined(CosStream_StreamableFactory_Class_Source)
				SOMEXTERN struct CosStream_StreamableFactoryCClassDataStructure _CosStream_StreamableFactoryCClassData;
				#ifndef CosStream_StreamableFactoryCClassData
					#define CosStream_StreamableFactoryCClassData    _CosStream_StreamableFactoryCClassData
				#endif /* CosStream_StreamableFactoryCClassData */
			#else
				SOMEXTERN struct CosStream_StreamableFactoryCClassDataStructure * SOMLINK resolve_CosStream_StreamableFactoryCClassData(void);
				#ifndef CosStream_StreamableFactoryCClassData
					#define CosStream_StreamableFactoryCClassData    (*(resolve_CosStream_StreamableFactoryCClassData()))
				#endif /* CosStream_StreamableFactoryCClassData */
			#endif
		#else /* SOM_RESOLVE_DATA */
			SOMEXTERN 
			#if defined(SOM_Module_omgestio_Source) || defined(CosStream_StreamableFactory_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_omgestio_Source || CosStream_StreamableFactory_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_omgestio_Source || CosStream_StreamableFactory_Class_Source */
			struct CosStream_StreamableFactoryCClassDataStructure SOMDLINK CosStream_StreamableFactoryCClassData;
		#endif /* SOM_RESOLVE_DATA */
		extern 
		#if defined(SOM_Module_omgestio_Source) || defined(CosStream_StreamableFactory_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_omgestio_Source || CosStream_StreamableFactory_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_omgestio_Source || CosStream_StreamableFactory_Class_Source */
		SOMClass SOMSTAR SOMLINK CosStream_StreamableFactoryNewClass
				(integer4 somtmajorVersion,integer4 somtminorVersion);
		#define _SOMCLASS_CosStream_StreamableFactory (CosStream_StreamableFactoryClassData.classObject)
		#ifndef SOMGD_CosStream_StreamableFactory
			#if (defined(_CosStream_StreamableFactory) || defined(__CosStream_StreamableFactory))
				#undef _CosStream_StreamableFactory
				#undef __CosStream_StreamableFactory
				#define SOMGD_CosStream_StreamableFactory 1
			#else
				#define _CosStream_StreamableFactory _SOMCLASS_CosStream_StreamableFactory
			#endif /* _CosStream_StreamableFactory */
		#endif /* SOMGD_CosStream_StreamableFactory */
		#define CosStream_StreamableFactory_classObj _SOMCLASS_CosStream_StreamableFactory
		#define _SOMMTOKEN_CosStream_StreamableFactory(method) ((somMToken)(CosStream_StreamableFactoryClassData.method))
		#ifndef CosStream_StreamableFactoryNew
			#define CosStream_StreamableFactoryNew() ( _CosStream_StreamableFactory ? \
				(SOMClass_somNew(_CosStream_StreamableFactory)) : \
				( CosStream_StreamableFactoryNewClass( \
					CosStream_StreamableFactory_MajorVersion, \
					CosStream_StreamableFactory_MinorVersion), \
				SOMClass_somNew(_CosStream_StreamableFactory))) 
		#endif /* NewCosStream_StreamableFactory */
		#ifndef CosStream_StreamableFactory_create_uninitialized
			#define CosStream_StreamableFactory_create_uninitialized(somSelf,ev) \
				SOM_Resolve(somSelf,CosStream_StreamableFactory,create_uninitialized)  \
					(somSelf,ev)
			#ifndef SOM_DONT_USE_SHORT_NAMES
				#ifndef SOMGD__create_uninitialized
					#if defined(_create_uninitialized)
						#undef _create_uninitialized
						#define SOMGD__create_uninitialized
					#else
						#define _create_uninitialized CosStream_StreamableFactory_create_uninitialized
					#endif
				#endif /* SOMGD__create_uninitialized */
			#endif /* SOM_DONT_USE_SHORT_NAMES */
		#endif /* CosStream_StreamableFactory_create_uninitialized */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define CosStream_StreamableFactory_somInit SOMObject_somInit
#define CosStream_StreamableFactory_somUninit SOMObject_somUninit
#define CosStream_StreamableFactory_somFree SOMObject_somFree
#define CosStream_StreamableFactory_somGetClass SOMObject_somGetClass
#define CosStream_StreamableFactory_somGetClassName SOMObject_somGetClassName
#define CosStream_StreamableFactory_somGetSize SOMObject_somGetSize
#define CosStream_StreamableFactory_somIsA SOMObject_somIsA
#define CosStream_StreamableFactory_somIsInstanceOf SOMObject_somIsInstanceOf
#define CosStream_StreamableFactory_somRespondsTo SOMObject_somRespondsTo
#define CosStream_StreamableFactory_somDispatch SOMObject_somDispatch
#define CosStream_StreamableFactory_somClassDispatch SOMObject_somClassDispatch
#define CosStream_StreamableFactory_somCastObj SOMObject_somCastObj
#define CosStream_StreamableFactory_somResetObj SOMObject_somResetObj
#define CosStream_StreamableFactory_somPrintSelf SOMObject_somPrintSelf
#define CosStream_StreamableFactory_somDumpSelf SOMObject_somDumpSelf
#define CosStream_StreamableFactory_somDumpSelfInt SOMObject_somDumpSelfInt
#define CosStream_StreamableFactory_somDefaultInit SOMObject_somDefaultInit
#define CosStream_StreamableFactory_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define CosStream_StreamableFactory_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define CosStream_StreamableFactory_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define CosStream_StreamableFactory_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define CosStream_StreamableFactory_somDefaultAssign SOMObject_somDefaultAssign
#define CosStream_StreamableFactory_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define CosStream_StreamableFactory_somDefaultVAssign SOMObject_somDefaultVAssign
#define CosStream_StreamableFactory_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define CosStream_StreamableFactory_somDestruct SOMObject_somDestruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_omgestio_Header_h */
