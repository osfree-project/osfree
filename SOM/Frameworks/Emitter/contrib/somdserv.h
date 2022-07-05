/* generated from somdserv.idl */
/* internal conditional is SOM_Module_somdserv_Source */
#ifndef SOM_Module_somdserv_Header_h
	#define SOM_Module_somdserv_Header_h 1
	#include <som.h>
	#include <somobj.h>
	#include <somdtype.h>
	#include <containd.h>
	#include <snglicls.h>
	#include <somcls.h>
	#include <xnaming.h>
	#include <naming.h>
	#ifndef _IDL_SOMDObject_defined
		#define _IDL_SOMDObject_defined
		typedef SOMObject SOMDObject;
	#endif /* _IDL_SOMDObject_defined */
	#ifndef _IDL_SEQUENCE_SOMDObject_defined
		#define _IDL_SEQUENCE_SOMDObject_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMDObject SOMSTAR ,sequence(SOMDObject));
	#endif /* _IDL_SEQUENCE_SOMDObject_defined */
	#ifndef _IDL_SOMOA_defined
		#define _IDL_SOMOA_defined
		typedef SOMObject SOMOA;
	#endif /* _IDL_SOMOA_defined */
	#ifndef _IDL_SEQUENCE_SOMOA_defined
		#define _IDL_SEQUENCE_SOMOA_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMOA SOMSTAR ,sequence(SOMOA));
	#endif /* _IDL_SEQUENCE_SOMOA_defined */
	#ifndef _IDL_ORB_defined
		#define _IDL_ORB_defined
		typedef SOMObject ORB;
	#endif /* _IDL_ORB_defined */
	#ifndef _IDL_SEQUENCE_ORB_defined
		#define _IDL_SEQUENCE_ORB_defined
		SOM_SEQUENCE_TYPEDEF_NAME(ORB SOMSTAR ,sequence(ORB));
	#endif /* _IDL_SEQUENCE_ORB_defined */
	#ifndef _IDL_ImplementationDef_defined
		#define _IDL_ImplementationDef_defined
		typedef SOMObject ImplementationDef;
	#endif /* _IDL_ImplementationDef_defined */
	#ifndef _IDL_SEQUENCE_ImplementationDef_defined
		#define _IDL_SEQUENCE_ImplementationDef_defined
		SOM_SEQUENCE_TYPEDEF_NAME(ImplementationDef SOMSTAR ,sequence(ImplementationDef));
	#endif /* _IDL_SEQUENCE_ImplementationDef_defined */
	#ifndef _IDL_SOMDServer_defined
		#define _IDL_SOMDServer_defined
		typedef SOMObject SOMDServer;
	#endif /* _IDL_SOMDServer_defined */
	#ifndef _IDL_SEQUENCE_SOMDServer_defined
		#define _IDL_SEQUENCE_SOMDServer_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMDServer SOMSTAR ,sequence(SOMDServer));
	#endif /* _IDL_SEQUENCE_SOMDServer_defined */
	#ifdef __IBMC__
		typedef SOMDObject SOMSTAR (somTP_SOMDServer_somdRefFromSOMObj)(
			SOMDServer SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMObject SOMSTAR somobj);
		#pragma linkage(somTP_SOMDServer_somdRefFromSOMObj,system)
		typedef somTP_SOMDServer_somdRefFromSOMObj *somTD_SOMDServer_somdRefFromSOMObj;
	#else /* __IBMC__ */
		typedef SOMDObject SOMSTAR (SOMLINK * somTD_SOMDServer_somdRefFromSOMObj)(
			SOMDServer SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMObject SOMSTAR somobj);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMObject SOMSTAR (somTP_SOMDServer_somdSOMObjFromRef)(
			SOMDServer SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMDObject SOMSTAR objref);
		#pragma linkage(somTP_SOMDServer_somdSOMObjFromRef,system)
		typedef somTP_SOMDServer_somdSOMObjFromRef *somTD_SOMDServer_somdSOMObjFromRef;
	#else /* __IBMC__ */
		typedef SOMObject SOMSTAR (SOMLINK * somTD_SOMDServer_somdSOMObjFromRef)(
			SOMDServer SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMDObject SOMSTAR objref);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMDServer_somdDispatchMethod)(
			SOMDServer SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMObject SOMSTAR somobj,
			/* out */ somToken *retValue,
			/* in */ somId methodId,
			/* in */ va_list ap);
		#pragma linkage(somTP_SOMDServer_somdDispatchMethod,system)
		typedef somTP_SOMDServer_somdDispatchMethod *somTD_SOMDServer_somdDispatchMethod;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMDServer_somdDispatchMethod)(
			SOMDServer SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMObject SOMSTAR somobj,
			/* out */ somToken *retValue,
			/* in */ somId methodId,
			/* in */ va_list ap);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMObject SOMSTAR (somTP_SOMDServer_somdCreateObj)(
			SOMDServer SOMSTAR somSelf,
			Environment *ev,
			/* in */ string objclass,
			/* in */ string hints);
		#pragma linkage(somTP_SOMDServer_somdCreateObj,system)
		typedef somTP_SOMDServer_somdCreateObj *somTD_SOMDServer_somdCreateObj;
	#else /* __IBMC__ */
		typedef SOMObject SOMSTAR (SOMLINK * somTD_SOMDServer_somdCreateObj)(
			SOMDServer SOMSTAR somSelf,
			Environment *ev,
			/* in */ string objclass,
			/* in */ string hints);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMDServer_somdDeleteObj)(
			SOMDServer SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMObject SOMSTAR somobj);
		#pragma linkage(somTP_SOMDServer_somdDeleteObj,system)
		typedef somTP_SOMDServer_somdDeleteObj *somTD_SOMDServer_somdDeleteObj;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMDServer_somdDeleteObj)(
			SOMDServer SOMSTAR somSelf,
			Environment *ev,
			/* in */ SOMObject SOMSTAR somobj);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMClass SOMSTAR (somTP_SOMDServer_somdGetClassObj)(
			SOMDServer SOMSTAR somSelf,
			Environment *ev,
			/* in */ string objclass);
		#pragma linkage(somTP_SOMDServer_somdGetClassObj,system)
		typedef somTP_SOMDServer_somdGetClassObj *somTD_SOMDServer_somdGetClassObj;
	#else /* __IBMC__ */
		typedef SOMClass SOMSTAR (SOMLINK * somTD_SOMDServer_somdGetClassObj)(
			SOMDServer SOMSTAR somSelf,
			Environment *ev,
			/* in */ string objclass);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef boolean (somTP_SOMDServer_somdObjReferencesCached)(
			SOMDServer SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_SOMDServer_somdObjReferencesCached,system)
		typedef somTP_SOMDServer_somdObjReferencesCached *somTD_SOMDServer_somdObjReferencesCached;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_SOMDServer_somdObjReferencesCached)(
			SOMDServer SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMObject SOMSTAR (somTP_SOMDServer_somdCreateFactory)(
			SOMDServer SOMSTAR somSelf,
			Environment *ev,
			/* in */ string className,
			/* in */ ExtendedNaming_PropertyList *props);
		#pragma linkage(somTP_SOMDServer_somdCreateFactory,system)
		typedef somTP_SOMDServer_somdCreateFactory *somTD_SOMDServer_somdCreateFactory;
	#else /* __IBMC__ */
		typedef SOMObject SOMSTAR (SOMLINK * somTD_SOMDServer_somdCreateFactory)(
			SOMDServer SOMSTAR somSelf,
			Environment *ev,
			/* in */ string className,
			/* in */ ExtendedNaming_PropertyList *props);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef ImplementationDef SOMSTAR (somTP_SOMDServer__get_ImplDefObject)(
			SOMDServer SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_SOMDServer__get_ImplDefObject,system)
		typedef somTP_SOMDServer__get_ImplDefObject *somTD_SOMDServer__get_ImplDefObject;
	#else /* __IBMC__ */
		typedef ImplementationDef SOMSTAR (SOMLINK * somTD_SOMDServer__get_ImplDefObject)(
			SOMDServer SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifndef SOMDServer_MajorVersion
		#define SOMDServer_MajorVersion   2
	#endif /* SOMDServer_MajorVersion */
	#ifndef SOMDServer_MinorVersion
		#define SOMDServer_MinorVersion   2
	#endif /* SOMDServer_MinorVersion */
	typedef struct SOMDServerClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken somdSOMObjFromRef;
		somMToken somdRefFromSOMObj;
		somMToken somdCreateObj;
		somMToken somdDeleteObj;
		somMToken somdGetClassObj;
		somMToken somdDispatchMethod;
		somMToken somdObjReferencesCached;
		somMToken private7;
		somMToken private8;
		somMToken somdCreateFactory;
		somMToken _get_ImplDefObject;
		somMToken _set_ImplDefObject;
		somMToken somdDispatchArgs;
	} SOMDServerClassDataStructure;
	typedef struct SOMDServerCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
		somMethodPtr somdDispatchMethod;
	} SOMDServerCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_somdserv_Source) || defined(SOMDServer_Class_Source)
			SOMEXTERN struct SOMDServerClassDataStructure _SOMDServerClassData;
			#ifndef SOMDServerClassData
				#define SOMDServerClassData    _SOMDServerClassData
			#endif /* SOMDServerClassData */
		#else
			SOMEXTERN struct SOMDServerClassDataStructure * SOMLINK resolve_SOMDServerClassData(void);
			#ifndef SOMDServerClassData
				#define SOMDServerClassData    (*(resolve_SOMDServerClassData()))
			#endif /* SOMDServerClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_somdserv_Source) || defined(SOMDServer_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_somdserv_Source || SOMDServer_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_somdserv_Source || SOMDServer_Class_Source */
		struct SOMDServerClassDataStructure SOMDLINK SOMDServerClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_somdserv_Source) || defined(SOMDServer_Class_Source)
			SOMEXTERN struct SOMDServerCClassDataStructure _SOMDServerCClassData;
			#ifndef SOMDServerCClassData
				#define SOMDServerCClassData    _SOMDServerCClassData
			#endif /* SOMDServerCClassData */
		#else
			SOMEXTERN struct SOMDServerCClassDataStructure * SOMLINK resolve_SOMDServerCClassData(void);
			#ifndef SOMDServerCClassData
				#define SOMDServerCClassData    (*(resolve_SOMDServerCClassData()))
			#endif /* SOMDServerCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_somdserv_Source) || defined(SOMDServer_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_somdserv_Source || SOMDServer_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_somdserv_Source || SOMDServer_Class_Source */
		struct SOMDServerCClassDataStructure SOMDLINK SOMDServerCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_somdserv_Source) || defined(SOMDServer_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_somdserv_Source || SOMDServer_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_somdserv_Source || SOMDServer_Class_Source */
	SOMClass SOMSTAR SOMLINK SOMDServerNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_SOMDServer (SOMDServerClassData.classObject)
	#ifndef SOMGD_SOMDServer
		#if (defined(_SOMDServer) || defined(__SOMDServer))
			#undef _SOMDServer
			#undef __SOMDServer
			#define SOMGD_SOMDServer 1
		#else
			#define _SOMDServer _SOMCLASS_SOMDServer
		#endif /* _SOMDServer */
	#endif /* SOMGD_SOMDServer */
	#define SOMDServer_classObj _SOMCLASS_SOMDServer
	#define _SOMMTOKEN_SOMDServer(method) ((somMToken)(SOMDServerClassData.method))
	#ifndef SOMDServerNew
		#define SOMDServerNew() ( _SOMDServer ? \
			(SOMClass_somNew(_SOMDServer)) : \
			( SOMDServerNewClass( \
				SOMDServer_MajorVersion, \
				SOMDServer_MinorVersion), \
			SOMClass_somNew(_SOMDServer))) 
	#endif /* NewSOMDServer */
	#ifndef SOMDServer_somdRefFromSOMObj
		#define SOMDServer_somdRefFromSOMObj(somSelf,ev,somobj) \
			SOM_Resolve(somSelf,SOMDServer,somdRefFromSOMObj)  \
				(somSelf,ev,somobj)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somdRefFromSOMObj
				#if defined(_somdRefFromSOMObj)
					#undef _somdRefFromSOMObj
					#define SOMGD__somdRefFromSOMObj
				#else
					#define _somdRefFromSOMObj SOMDServer_somdRefFromSOMObj
				#endif
			#endif /* SOMGD__somdRefFromSOMObj */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDServer_somdRefFromSOMObj */
	#ifndef SOMDServer_somdSOMObjFromRef
		#define SOMDServer_somdSOMObjFromRef(somSelf,ev,objref) \
			SOM_Resolve(somSelf,SOMDServer,somdSOMObjFromRef)  \
				(somSelf,ev,objref)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somdSOMObjFromRef
				#if defined(_somdSOMObjFromRef)
					#undef _somdSOMObjFromRef
					#define SOMGD__somdSOMObjFromRef
				#else
					#define _somdSOMObjFromRef SOMDServer_somdSOMObjFromRef
				#endif
			#endif /* SOMGD__somdSOMObjFromRef */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDServer_somdSOMObjFromRef */
	#ifndef SOMDServer_somdDispatchMethod
		#define SOMDServer_somdDispatchMethod(somSelf,ev,somobj,retValue,methodId,ap) \
			SOM_Resolve(somSelf,SOMDServer,somdDispatchMethod)  \
				(somSelf,ev,somobj,retValue,methodId,ap)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somdDispatchMethod
				#if defined(_somdDispatchMethod)
					#undef _somdDispatchMethod
					#define SOMGD__somdDispatchMethod
				#else
					#define _somdDispatchMethod SOMDServer_somdDispatchMethod
				#endif
			#endif /* SOMGD__somdDispatchMethod */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDServer_somdDispatchMethod */
	#ifndef va_SOMDServer_somdDispatchMethod
		#define va_SOMDServer_somdDispatchMethod somva_SOMDServer_somdDispatchMethod
	#endif /* va_SOMDServer_somdDispatchMethod */
	#ifdef SOMDServer_VA_EXTERN
		SOMEXTERN
		#ifdef SOMDServer_VA_STUBS
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else
			#if defined(SOM_Module_somdserv_Source) || defined(SOMDServer_Class_Source)
				#ifdef SOMDLLEXPORT
					SOMDLLEXPORT
				#endif /* SOMDLLEXPORT */
			#else /* SOM_Module_somdserv_Source || SOMDServer_Class_Source */
				#ifdef SOMDLLIMPORT
					SOMDLLIMPORT
				#endif /* SOMDLLIMPORT */
			#endif /* SOM_Module_somdserv_Source || SOMDServer_Class_Source */
		#endif /* SOMDServer_VA_STUBS */
		void SOMLINK somva_SOMDServer_somdDispatchMethod(SOMDServer SOMSTAR somSelf,Environment *ev,
			SOMObject SOMSTAR somobj,
			somToken *retValue,
			somId methodId,
			...);
	#endif
	#if (defined(SOMDServer_VA_STUBS)||!defined(SOMDServer_VA_EXTERN))
		#ifdef SOMDServer_VA_EXTERN
			SOMEXTERN
		#else
			static
		#endif
		void SOMLINK somva_SOMDServer_somdDispatchMethod(SOMDServer SOMSTAR somSelf,Environment *ev,
				SOMObject SOMSTAR somobj,
				somToken *retValue,
				somId methodId,
				...)
		{
			va_list ap;
			va_start(ap,methodId);
			SOM_Resolve(somSelf,SOMDServer,somdDispatchMethod)
					(somSelf,ev,somobj,retValue,methodId,ap);
			va_end(ap);
		}
	#endif
	#ifndef SOMDServer_somdCreateObj
		#define SOMDServer_somdCreateObj(somSelf,ev,objclass,hints) \
			SOM_Resolve(somSelf,SOMDServer,somdCreateObj)  \
				(somSelf,ev,objclass,hints)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somdCreateObj
				#if defined(_somdCreateObj)
					#undef _somdCreateObj
					#define SOMGD__somdCreateObj
				#else
					#define _somdCreateObj SOMDServer_somdCreateObj
				#endif
			#endif /* SOMGD__somdCreateObj */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDServer_somdCreateObj */
	#ifndef SOMDServer_somdDeleteObj
		#define SOMDServer_somdDeleteObj(somSelf,ev,somobj) \
			SOM_Resolve(somSelf,SOMDServer,somdDeleteObj)  \
				(somSelf,ev,somobj)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somdDeleteObj
				#if defined(_somdDeleteObj)
					#undef _somdDeleteObj
					#define SOMGD__somdDeleteObj
				#else
					#define _somdDeleteObj SOMDServer_somdDeleteObj
				#endif
			#endif /* SOMGD__somdDeleteObj */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDServer_somdDeleteObj */
	#ifndef SOMDServer_somdGetClassObj
		#define SOMDServer_somdGetClassObj(somSelf,ev,objclass) \
			SOM_Resolve(somSelf,SOMDServer,somdGetClassObj)  \
				(somSelf,ev,objclass)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somdGetClassObj
				#if defined(_somdGetClassObj)
					#undef _somdGetClassObj
					#define SOMGD__somdGetClassObj
				#else
					#define _somdGetClassObj SOMDServer_somdGetClassObj
				#endif
			#endif /* SOMGD__somdGetClassObj */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDServer_somdGetClassObj */
	#ifndef SOMDServer_somdObjReferencesCached
		#define SOMDServer_somdObjReferencesCached(somSelf,ev) \
			SOM_Resolve(somSelf,SOMDServer,somdObjReferencesCached)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somdObjReferencesCached
				#if defined(_somdObjReferencesCached)
					#undef _somdObjReferencesCached
					#define SOMGD__somdObjReferencesCached
				#else
					#define _somdObjReferencesCached SOMDServer_somdObjReferencesCached
				#endif
			#endif /* SOMGD__somdObjReferencesCached */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDServer_somdObjReferencesCached */
	#ifndef SOMDServer_somdCreateFactory
		#define SOMDServer_somdCreateFactory(somSelf,ev,className,props) \
			SOM_Resolve(somSelf,SOMDServer,somdCreateFactory)  \
				(somSelf,ev,className,props)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somdCreateFactory
				#if defined(_somdCreateFactory)
					#undef _somdCreateFactory
					#define SOMGD__somdCreateFactory
				#else
					#define _somdCreateFactory SOMDServer_somdCreateFactory
				#endif
			#endif /* SOMGD__somdCreateFactory */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDServer_somdCreateFactory */
	#ifndef SOMDServer__get_ImplDefObject
		#define SOMDServer__get_ImplDefObject(somSelf,ev) \
			SOM_Resolve(somSelf,SOMDServer,_get_ImplDefObject)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_ImplDefObject
				#if defined(__get_ImplDefObject)
					#undef __get_ImplDefObject
					#define SOMGD___get_ImplDefObject
				#else
					#define __get_ImplDefObject SOMDServer__get_ImplDefObject
				#endif
			#endif /* SOMGD___get_ImplDefObject */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMDServer__get_ImplDefObject */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define SOMDServer_somInit SOMObject_somInit
#define SOMDServer_somUninit SOMObject_somUninit
#define SOMDServer_somFree SOMObject_somFree
#define SOMDServer_somGetClass SOMObject_somGetClass
#define SOMDServer_somGetClassName SOMObject_somGetClassName
#define SOMDServer_somGetSize SOMObject_somGetSize
#define SOMDServer_somIsA SOMObject_somIsA
#define SOMDServer_somIsInstanceOf SOMObject_somIsInstanceOf
#define SOMDServer_somRespondsTo SOMObject_somRespondsTo
#define SOMDServer_somDispatch SOMObject_somDispatch
#define SOMDServer_somClassDispatch SOMObject_somClassDispatch
#define SOMDServer_somCastObj SOMObject_somCastObj
#define SOMDServer_somResetObj SOMObject_somResetObj
#define SOMDServer_somPrintSelf SOMObject_somPrintSelf
#define SOMDServer_somDumpSelf SOMObject_somDumpSelf
#define SOMDServer_somDumpSelfInt SOMObject_somDumpSelfInt
#define SOMDServer_somDefaultInit SOMObject_somDefaultInit
#define SOMDServer_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define SOMDServer_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define SOMDServer_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define SOMDServer_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define SOMDServer_somDefaultAssign SOMObject_somDefaultAssign
#define SOMDServer_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define SOMDServer_somDefaultVAssign SOMObject_somDefaultVAssign
#define SOMDServer_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define SOMDServer_somDestruct SOMObject_somDestruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_somdserv_Header_h */
