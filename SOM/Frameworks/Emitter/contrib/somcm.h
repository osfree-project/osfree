/* generated from somcm.idl */
/* internal conditional is SOM_Module_somcm_Source */
#ifndef SOM_Module_somcm_Header_h
	#define SOM_Module_somcm_Header_h 1
	#include <som.h>
	#include <somobj.h>
	#ifndef _IDL_Repository_defined
		#define _IDL_Repository_defined
		typedef SOMObject Repository;
	#endif /* _IDL_Repository_defined */
	#ifndef _IDL_SEQUENCE_Repository_defined
		#define _IDL_SEQUENCE_Repository_defined
		SOM_SEQUENCE_TYPEDEF_NAME(Repository SOMSTAR ,sequence(Repository));
	#endif /* _IDL_SEQUENCE_Repository_defined */
	#ifndef _IDL_SOMClassMgr_defined
		#define _IDL_SOMClassMgr_defined
		typedef SOMObject SOMClassMgr;
	#endif /* _IDL_SOMClassMgr_defined */
	#ifndef _IDL_SEQUENCE_SOMClassMgr_defined
		#define _IDL_SEQUENCE_SOMClassMgr_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMClassMgr SOMSTAR ,sequence(SOMClassMgr));
	#endif /* _IDL_SEQUENCE_SOMClassMgr_defined */
	typedef SOMClass SOMSTAR *SOMClassMgr_SOMClassArray;
	#ifndef _IDL_SEQUENCE_SOMClassMgr_SOMClassArray_defined
		#define _IDL_SEQUENCE_SOMClassMgr_SOMClassArray_defined
		SOM_SEQUENCE_TYPEDEF(SOMClassMgr_SOMClassArray);
	#endif /* _IDL_SEQUENCE_SOMClassMgr_SOMClassArray_defined */
	#ifdef __IBMC__
		typedef SOMClass SOMSTAR (somTP_SOMClassMgr_somLoadClassFile)(
			SOMClassMgr SOMSTAR somSelf,
			/* in */ somId classId,
			/* in */ long majorVersion,
			/* in */ long minorVersion,
			/* in */ string file);
		#pragma linkage(somTP_SOMClassMgr_somLoadClassFile,system)
		typedef somTP_SOMClassMgr_somLoadClassFile *somTD_SOMClassMgr_somLoadClassFile;
	#else /* __IBMC__ */
		typedef SOMClass SOMSTAR (SOMLINK * somTD_SOMClassMgr_somLoadClassFile)(
			SOMClassMgr SOMSTAR somSelf,
			/* in */ somId classId,
			/* in */ long majorVersion,
			/* in */ long minorVersion,
			/* in */ string file);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_SOMClassMgr_somLocateClassFile)(
			SOMClassMgr SOMSTAR somSelf,
			/* in */ somId classId,
			/* in */ long majorVersion,
			/* in */ long minorVersion);
		#pragma linkage(somTP_SOMClassMgr_somLocateClassFile,system)
		typedef somTP_SOMClassMgr_somLocateClassFile *somTD_SOMClassMgr_somLocateClassFile;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_SOMClassMgr_somLocateClassFile)(
			SOMClassMgr SOMSTAR somSelf,
			/* in */ somId classId,
			/* in */ long majorVersion,
			/* in */ long minorVersion);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMClassMgr_somRegisterClass)(
			SOMClassMgr SOMSTAR somSelf,
			/* in */ SOMClass SOMSTAR classObj);
		#pragma linkage(somTP_SOMClassMgr_somRegisterClass,system)
		typedef somTP_SOMClassMgr_somRegisterClass *somTD_SOMClassMgr_somRegisterClass;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMClassMgr_somRegisterClass)(
			SOMClassMgr SOMSTAR somSelf,
			/* in */ SOMClass SOMSTAR classObj);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMClassMgr_somRegisterClassLibrary)(
			SOMClassMgr SOMSTAR somSelf,
			/* in */ string libraryName,
			/* in */ somMethodPtr libraryInitRtn);
		#pragma linkage(somTP_SOMClassMgr_somRegisterClassLibrary,system)
		typedef somTP_SOMClassMgr_somRegisterClassLibrary *somTD_SOMClassMgr_somRegisterClassLibrary;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMClassMgr_somRegisterClassLibrary)(
			SOMClassMgr SOMSTAR somSelf,
			/* in */ string libraryName,
			/* in */ somMethodPtr libraryInitRtn);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMClassMgr_somUnregisterClassLibrary)(
			SOMClassMgr SOMSTAR somSelf,
			/* in */ string libraryName);
		#pragma linkage(somTP_SOMClassMgr_somUnregisterClassLibrary,system)
		typedef somTP_SOMClassMgr_somUnregisterClassLibrary *somTD_SOMClassMgr_somUnregisterClassLibrary;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMClassMgr_somUnregisterClassLibrary)(
			SOMClassMgr SOMSTAR somSelf,
			/* in */ string libraryName);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef long (somTP_SOMClassMgr_somUnloadClassFile)(
			SOMClassMgr SOMSTAR somSelf,
			/* in */ SOMClass SOMSTAR classObj);
		#pragma linkage(somTP_SOMClassMgr_somUnloadClassFile,system)
		typedef somTP_SOMClassMgr_somUnloadClassFile *somTD_SOMClassMgr_somUnloadClassFile;
	#else /* __IBMC__ */
		typedef long (SOMLINK * somTD_SOMClassMgr_somUnloadClassFile)(
			SOMClassMgr SOMSTAR somSelf,
			/* in */ SOMClass SOMSTAR classObj);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef long (somTP_SOMClassMgr_somUnregisterClass)(
			SOMClassMgr SOMSTAR somSelf,
			/* in */ SOMClass SOMSTAR classObj);
		#pragma linkage(somTP_SOMClassMgr_somUnregisterClass,system)
		typedef somTP_SOMClassMgr_somUnregisterClass *somTD_SOMClassMgr_somUnregisterClass;
	#else /* __IBMC__ */
		typedef long (SOMLINK * somTD_SOMClassMgr_somUnregisterClass)(
			SOMClassMgr SOMSTAR somSelf,
			/* in */ SOMClass SOMSTAR classObj);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMClassMgr_somBeginPersistentClasses)(
			SOMClassMgr SOMSTAR somSelf);
		#pragma linkage(somTP_SOMClassMgr_somBeginPersistentClasses,system)
		typedef somTP_SOMClassMgr_somBeginPersistentClasses *somTD_SOMClassMgr_somBeginPersistentClasses;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMClassMgr_somBeginPersistentClasses)(
			SOMClassMgr SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMClassMgr_somEndPersistentClasses)(
			SOMClassMgr SOMSTAR somSelf);
		#pragma linkage(somTP_SOMClassMgr_somEndPersistentClasses,system)
		typedef somTP_SOMClassMgr_somEndPersistentClasses *somTD_SOMClassMgr_somEndPersistentClasses;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMClassMgr_somEndPersistentClasses)(
			SOMClassMgr SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef boolean (somTP_SOMClassMgr_somJoinAffinityGroup)(
			SOMClassMgr SOMSTAR somSelf,
			/* in */ SOMClass SOMSTAR newClass,
			/* in */ SOMClass SOMSTAR affClass);
		#pragma linkage(somTP_SOMClassMgr_somJoinAffinityGroup,system)
		typedef somTP_SOMClassMgr_somJoinAffinityGroup *somTD_SOMClassMgr_somJoinAffinityGroup;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_SOMClassMgr_somJoinAffinityGroup)(
			SOMClassMgr SOMSTAR somSelf,
			/* in */ SOMClass SOMSTAR newClass,
			/* in */ SOMClass SOMSTAR affClass);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_SOMClassMgr_somGetInitFunction)(
			SOMClassMgr SOMSTAR somSelf);
		#pragma linkage(somTP_SOMClassMgr_somGetInitFunction,system)
		typedef somTP_SOMClassMgr_somGetInitFunction *somTD_SOMClassMgr_somGetInitFunction;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_SOMClassMgr_somGetInitFunction)(
			SOMClassMgr SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMClassMgr__set_somInterfaceRepository)(
			SOMClassMgr SOMSTAR somSelf,
			/* in */ Repository SOMSTAR somInterfaceRepository);
		#pragma linkage(somTP_SOMClassMgr__set_somInterfaceRepository,system)
		typedef somTP_SOMClassMgr__set_somInterfaceRepository *somTD_SOMClassMgr__set_somInterfaceRepository;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMClassMgr__set_somInterfaceRepository)(
			SOMClassMgr SOMSTAR somSelf,
			/* in */ Repository SOMSTAR somInterfaceRepository);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef Repository SOMSTAR (somTP_SOMClassMgr__get_somInterfaceRepository)(
			SOMClassMgr SOMSTAR somSelf);
		#pragma linkage(somTP_SOMClassMgr__get_somInterfaceRepository,system)
		typedef somTP_SOMClassMgr__get_somInterfaceRepository *somTD_SOMClassMgr__get_somInterfaceRepository;
	#else /* __IBMC__ */
		typedef Repository SOMSTAR (SOMLINK * somTD_SOMClassMgr__get_somInterfaceRepository)(
			SOMClassMgr SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef _IDL_SEQUENCE_SOMClass (somTP_SOMClassMgr__get_somRegisteredClasses)(
			SOMClassMgr SOMSTAR somSelf);
		#pragma linkage(somTP_SOMClassMgr__get_somRegisteredClasses,system)
		typedef somTP_SOMClassMgr__get_somRegisteredClasses *somTD_SOMClassMgr__get_somRegisteredClasses;
	#else /* __IBMC__ */
		typedef _IDL_SEQUENCE_SOMClass (SOMLINK * somTD_SOMClassMgr__get_somRegisteredClasses)(
			SOMClassMgr SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMClassMgr_SOMClassArray (somTP_SOMClassMgr_somGetRelatedClasses)(
			SOMClassMgr SOMSTAR somSelf,
			/* in */ SOMClass SOMSTAR classObj);
		#pragma linkage(somTP_SOMClassMgr_somGetRelatedClasses,system)
		typedef somTP_SOMClassMgr_somGetRelatedClasses *somTD_SOMClassMgr_somGetRelatedClasses;
	#else /* __IBMC__ */
		typedef SOMClassMgr_SOMClassArray (SOMLINK * somTD_SOMClassMgr_somGetRelatedClasses)(
			SOMClassMgr SOMSTAR somSelf,
			/* in */ SOMClass SOMSTAR classObj);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMClass SOMSTAR (somTP_SOMClassMgr_somClassFromId)(
			SOMClassMgr SOMSTAR somSelf,
			/* in */ somId classId);
		#pragma linkage(somTP_SOMClassMgr_somClassFromId,system)
		typedef somTP_SOMClassMgr_somClassFromId *somTD_SOMClassMgr_somClassFromId;
	#else /* __IBMC__ */
		typedef SOMClass SOMSTAR (SOMLINK * somTD_SOMClassMgr_somClassFromId)(
			SOMClassMgr SOMSTAR somSelf,
			/* in */ somId classId);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMClass SOMSTAR (somTP_SOMClassMgr_somFindClass)(
			SOMClassMgr SOMSTAR somSelf,
			/* in */ somId classId,
			/* in */ long majorVersion,
			/* in */ long minorVersion);
		#pragma linkage(somTP_SOMClassMgr_somFindClass,system)
		typedef somTP_SOMClassMgr_somFindClass *somTD_SOMClassMgr_somFindClass;
	#else /* __IBMC__ */
		typedef SOMClass SOMSTAR (SOMLINK * somTD_SOMClassMgr_somFindClass)(
			SOMClassMgr SOMSTAR somSelf,
			/* in */ somId classId,
			/* in */ long majorVersion,
			/* in */ long minorVersion);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMClass SOMSTAR (somTP_SOMClassMgr_somFindClsInFile)(
			SOMClassMgr SOMSTAR somSelf,
			/* in */ somId classId,
			/* in */ long majorVersion,
			/* in */ long minorVersion,
			/* in */ string file);
		#pragma linkage(somTP_SOMClassMgr_somFindClsInFile,system)
		typedef somTP_SOMClassMgr_somFindClsInFile *somTD_SOMClassMgr_somFindClsInFile;
	#else /* __IBMC__ */
		typedef SOMClass SOMSTAR (SOMLINK * somTD_SOMClassMgr_somFindClsInFile)(
			SOMClassMgr SOMSTAR somSelf,
			/* in */ somId classId,
			/* in */ long majorVersion,
			/* in */ long minorVersion,
			/* in */ string file);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMClassMgr_somMergeInto)(
			SOMClassMgr SOMSTAR somSelf,
			/* in */ SOMObject SOMSTAR targetObj);
		#pragma linkage(somTP_SOMClassMgr_somMergeInto,system)
		typedef somTP_SOMClassMgr_somMergeInto *somTD_SOMClassMgr_somMergeInto;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMClassMgr_somMergeInto)(
			SOMClassMgr SOMSTAR somSelf,
			/* in */ SOMObject SOMSTAR targetObj);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef long (somTP_SOMClassMgr_somSubstituteClass)(
			SOMClassMgr SOMSTAR somSelf,
			/* in */ string origClassName,
			/* in */ string newClassName);
		#pragma linkage(somTP_SOMClassMgr_somSubstituteClass,system)
		typedef somTP_SOMClassMgr_somSubstituteClass *somTD_SOMClassMgr_somSubstituteClass;
	#else /* __IBMC__ */
		typedef long (SOMLINK * somTD_SOMClassMgr_somSubstituteClass)(
			SOMClassMgr SOMSTAR somSelf,
			/* in */ string origClassName,
			/* in */ string newClassName);
	#endif /* __IBMC__ */
	#ifndef SOMClassMgr_MajorVersion
		#define SOMClassMgr_MajorVersion   1
	#endif /* SOMClassMgr_MajorVersion */
	#ifndef SOMClassMgr_MinorVersion
		#define SOMClassMgr_MinorVersion   4
	#endif /* SOMClassMgr_MinorVersion */
	typedef struct SOMClassMgrClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken somFindClsInFile;
		somMToken somFindClass;
		somMToken somClassFromId;
		somMToken somRegisterClass;
		somMToken somUnregisterClass;
		somMToken somLocateClassFile;
		somMToken somLoadClassFile;
		somMToken somUnloadClassFile;
		somMToken somGetInitFunction;
		somMToken somMergeInto;
		somMToken somGetRelatedClasses;
		somMToken somSubstituteClass;
		somMToken _get_somInterfaceRepository;
		somMToken _set_somInterfaceRepository;
		somMToken _get_somRegisteredClasses;
		somMToken somBeginPersistentClasses;
		somMToken somEndPersistentClasses;
		somMToken somReleaseClasses;
		somMToken somRegisterThreadUsage;
		somMToken somRegisterClassLibrary;
		somMToken somJoinAffinityGroup;
		somMToken somUnregisterClassLibrary;
		somMToken somImportObject;
		somMToken private23;
		somMToken private24;
	} SOMClassMgrClassDataStructure;
	typedef struct SOMClassMgrCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} SOMClassMgrCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_somcm_Source) || defined(SOMClassMgr_Class_Source)
			SOMEXTERN struct SOMClassMgrClassDataStructure _SOMClassMgrClassData;
			#ifndef SOMClassMgrClassData
				#define SOMClassMgrClassData    _SOMClassMgrClassData
			#endif /* SOMClassMgrClassData */
		#else
			SOMEXTERN struct SOMClassMgrClassDataStructure * SOMLINK resolve_SOMClassMgrClassData(void);
			#ifndef SOMClassMgrClassData
				#define SOMClassMgrClassData    (*(resolve_SOMClassMgrClassData()))
			#endif /* SOMClassMgrClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_somcm_Source) || defined(SOMClassMgr_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_somcm_Source || SOMClassMgr_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_somcm_Source || SOMClassMgr_Class_Source */
		struct SOMClassMgrClassDataStructure SOMDLINK SOMClassMgrClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_somcm_Source) || defined(SOMClassMgr_Class_Source)
			SOMEXTERN struct SOMClassMgrCClassDataStructure _SOMClassMgrCClassData;
			#ifndef SOMClassMgrCClassData
				#define SOMClassMgrCClassData    _SOMClassMgrCClassData
			#endif /* SOMClassMgrCClassData */
		#else
			SOMEXTERN struct SOMClassMgrCClassDataStructure * SOMLINK resolve_SOMClassMgrCClassData(void);
			#ifndef SOMClassMgrCClassData
				#define SOMClassMgrCClassData    (*(resolve_SOMClassMgrCClassData()))
			#endif /* SOMClassMgrCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_somcm_Source) || defined(SOMClassMgr_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_somcm_Source || SOMClassMgr_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_somcm_Source || SOMClassMgr_Class_Source */
		struct SOMClassMgrCClassDataStructure SOMDLINK SOMClassMgrCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_somcm_Source) || defined(SOMClassMgr_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_somcm_Source || SOMClassMgr_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_somcm_Source || SOMClassMgr_Class_Source */
	SOMClass SOMSTAR SOMLINK SOMClassMgrNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_SOMClassMgr (SOMClassMgrClassData.classObject)
	#ifndef SOMGD_SOMClassMgr
		#if (defined(_SOMClassMgr) || defined(__SOMClassMgr))
			#undef _SOMClassMgr
			#undef __SOMClassMgr
			#define SOMGD_SOMClassMgr 1
		#else
			#define _SOMClassMgr _SOMCLASS_SOMClassMgr
		#endif /* _SOMClassMgr */
	#endif /* SOMGD_SOMClassMgr */
	#define SOMClassMgr_classObj _SOMCLASS_SOMClassMgr
	#define _SOMMTOKEN_SOMClassMgr(method) ((somMToken)(SOMClassMgrClassData.method))
	#ifndef SOMClassMgrNew
		#define SOMClassMgrNew() ( _SOMClassMgr ? \
			(SOMClass_somNew(_SOMClassMgr)) : \
			( SOMClassMgrNewClass( \
				SOMClassMgr_MajorVersion, \
				SOMClassMgr_MinorVersion), \
			SOMClass_somNew(_SOMClassMgr))) 
	#endif /* NewSOMClassMgr */
	#ifndef SOMClassMgr_somLoadClassFile
		#define SOMClassMgr_somLoadClassFile(somSelf,classId,majorVersion,minorVersion,file) \
			SOM_Resolve(somSelf,SOMClassMgr,somLoadClassFile)  \
				(somSelf,classId,majorVersion,minorVersion,file)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somLoadClassFile
				#if defined(_somLoadClassFile)
					#undef _somLoadClassFile
					#define SOMGD__somLoadClassFile
				#else
					#define _somLoadClassFile SOMClassMgr_somLoadClassFile
				#endif
			#endif /* SOMGD__somLoadClassFile */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClassMgr_somLoadClassFile */
	#ifndef SOMClassMgr_somLocateClassFile
		#define SOMClassMgr_somLocateClassFile(somSelf,classId,majorVersion,minorVersion) \
			SOM_Resolve(somSelf,SOMClassMgr,somLocateClassFile)  \
				(somSelf,classId,majorVersion,minorVersion)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somLocateClassFile
				#if defined(_somLocateClassFile)
					#undef _somLocateClassFile
					#define SOMGD__somLocateClassFile
				#else
					#define _somLocateClassFile SOMClassMgr_somLocateClassFile
				#endif
			#endif /* SOMGD__somLocateClassFile */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClassMgr_somLocateClassFile */
	#ifndef SOMClassMgr_somRegisterClass
		#define SOMClassMgr_somRegisterClass(somSelf,classObj) \
			SOM_Resolve(somSelf,SOMClassMgr,somRegisterClass)  \
				(somSelf,classObj)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somRegisterClass
				#if defined(_somRegisterClass)
					#undef _somRegisterClass
					#define SOMGD__somRegisterClass
				#else
					#define _somRegisterClass SOMClassMgr_somRegisterClass
				#endif
			#endif /* SOMGD__somRegisterClass */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClassMgr_somRegisterClass */
	#ifndef SOMClassMgr_somRegisterClassLibrary
		#define SOMClassMgr_somRegisterClassLibrary(somSelf,libraryName,libraryInitRtn) \
			SOM_Resolve(somSelf,SOMClassMgr,somRegisterClassLibrary)  \
				(somSelf,libraryName,libraryInitRtn)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somRegisterClassLibrary
				#if defined(_somRegisterClassLibrary)
					#undef _somRegisterClassLibrary
					#define SOMGD__somRegisterClassLibrary
				#else
					#define _somRegisterClassLibrary SOMClassMgr_somRegisterClassLibrary
				#endif
			#endif /* SOMGD__somRegisterClassLibrary */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClassMgr_somRegisterClassLibrary */
	#ifndef SOMClassMgr_somUnregisterClassLibrary
		#define SOMClassMgr_somUnregisterClassLibrary(somSelf,libraryName) \
			SOM_Resolve(somSelf,SOMClassMgr,somUnregisterClassLibrary)  \
				(somSelf,libraryName)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somUnregisterClassLibrary
				#if defined(_somUnregisterClassLibrary)
					#undef _somUnregisterClassLibrary
					#define SOMGD__somUnregisterClassLibrary
				#else
					#define _somUnregisterClassLibrary SOMClassMgr_somUnregisterClassLibrary
				#endif
			#endif /* SOMGD__somUnregisterClassLibrary */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClassMgr_somUnregisterClassLibrary */
	#ifndef SOMClassMgr_somUnloadClassFile
		#define SOMClassMgr_somUnloadClassFile(somSelf,classObj) \
			SOM_Resolve(somSelf,SOMClassMgr,somUnloadClassFile)  \
				(somSelf,classObj)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somUnloadClassFile
				#if defined(_somUnloadClassFile)
					#undef _somUnloadClassFile
					#define SOMGD__somUnloadClassFile
				#else
					#define _somUnloadClassFile SOMClassMgr_somUnloadClassFile
				#endif
			#endif /* SOMGD__somUnloadClassFile */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClassMgr_somUnloadClassFile */
	#ifndef SOMClassMgr_somUnregisterClass
		#define SOMClassMgr_somUnregisterClass(somSelf,classObj) \
			SOM_Resolve(somSelf,SOMClassMgr,somUnregisterClass)  \
				(somSelf,classObj)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somUnregisterClass
				#if defined(_somUnregisterClass)
					#undef _somUnregisterClass
					#define SOMGD__somUnregisterClass
				#else
					#define _somUnregisterClass SOMClassMgr_somUnregisterClass
				#endif
			#endif /* SOMGD__somUnregisterClass */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClassMgr_somUnregisterClass */
	#ifndef SOMClassMgr_somBeginPersistentClasses
		#define SOMClassMgr_somBeginPersistentClasses(somSelf) \
			SOM_Resolve(somSelf,SOMClassMgr,somBeginPersistentClasses)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somBeginPersistentClasses
				#if defined(_somBeginPersistentClasses)
					#undef _somBeginPersistentClasses
					#define SOMGD__somBeginPersistentClasses
				#else
					#define _somBeginPersistentClasses SOMClassMgr_somBeginPersistentClasses
				#endif
			#endif /* SOMGD__somBeginPersistentClasses */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClassMgr_somBeginPersistentClasses */
	#ifndef SOMClassMgr_somEndPersistentClasses
		#define SOMClassMgr_somEndPersistentClasses(somSelf) \
			SOM_Resolve(somSelf,SOMClassMgr,somEndPersistentClasses)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somEndPersistentClasses
				#if defined(_somEndPersistentClasses)
					#undef _somEndPersistentClasses
					#define SOMGD__somEndPersistentClasses
				#else
					#define _somEndPersistentClasses SOMClassMgr_somEndPersistentClasses
				#endif
			#endif /* SOMGD__somEndPersistentClasses */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClassMgr_somEndPersistentClasses */
	#ifndef SOMClassMgr_somJoinAffinityGroup
		#define SOMClassMgr_somJoinAffinityGroup(somSelf,newClass,affClass) \
			SOM_Resolve(somSelf,SOMClassMgr,somJoinAffinityGroup)  \
				(somSelf,newClass,affClass)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somJoinAffinityGroup
				#if defined(_somJoinAffinityGroup)
					#undef _somJoinAffinityGroup
					#define SOMGD__somJoinAffinityGroup
				#else
					#define _somJoinAffinityGroup SOMClassMgr_somJoinAffinityGroup
				#endif
			#endif /* SOMGD__somJoinAffinityGroup */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClassMgr_somJoinAffinityGroup */
	#ifndef SOMClassMgr_somGetInitFunction
		#define SOMClassMgr_somGetInitFunction(somSelf) \
			SOM_Resolve(somSelf,SOMClassMgr,somGetInitFunction)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somGetInitFunction
				#if defined(_somGetInitFunction)
					#undef _somGetInitFunction
					#define SOMGD__somGetInitFunction
				#else
					#define _somGetInitFunction SOMClassMgr_somGetInitFunction
				#endif
			#endif /* SOMGD__somGetInitFunction */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClassMgr_somGetInitFunction */
	#ifndef SOMClassMgr__set_somInterfaceRepository
		#define SOMClassMgr__set_somInterfaceRepository(somSelf,somInterfaceRepository) \
			SOM_Resolve(somSelf,SOMClassMgr,_set_somInterfaceRepository)  \
				(somSelf,somInterfaceRepository)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_somInterfaceRepository
				#if defined(__set_somInterfaceRepository)
					#undef __set_somInterfaceRepository
					#define SOMGD___set_somInterfaceRepository
				#else
					#define __set_somInterfaceRepository SOMClassMgr__set_somInterfaceRepository
				#endif
			#endif /* SOMGD___set_somInterfaceRepository */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClassMgr__set_somInterfaceRepository */
	#ifndef SOMClassMgr__get_somInterfaceRepository
		#define SOMClassMgr__get_somInterfaceRepository(somSelf) \
			SOM_Resolve(somSelf,SOMClassMgr,_get_somInterfaceRepository)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somInterfaceRepository
				#if defined(__get_somInterfaceRepository)
					#undef __get_somInterfaceRepository
					#define SOMGD___get_somInterfaceRepository
				#else
					#define __get_somInterfaceRepository SOMClassMgr__get_somInterfaceRepository
				#endif
			#endif /* SOMGD___get_somInterfaceRepository */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClassMgr__get_somInterfaceRepository */
	#ifndef SOMClassMgr__get_somRegisteredClasses
		#define SOMClassMgr__get_somRegisteredClasses(somSelf) \
			SOM_Resolve(somSelf,SOMClassMgr,_get_somRegisteredClasses)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somRegisteredClasses
				#if defined(__get_somRegisteredClasses)
					#undef __get_somRegisteredClasses
					#define SOMGD___get_somRegisteredClasses
				#else
					#define __get_somRegisteredClasses SOMClassMgr__get_somRegisteredClasses
				#endif
			#endif /* SOMGD___get_somRegisteredClasses */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClassMgr__get_somRegisteredClasses */
	#ifndef SOMClassMgr_somGetRelatedClasses
		#define SOMClassMgr_somGetRelatedClasses(somSelf,classObj) \
			SOM_Resolve(somSelf,SOMClassMgr,somGetRelatedClasses)  \
				(somSelf,classObj)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somGetRelatedClasses
				#if defined(_somGetRelatedClasses)
					#undef _somGetRelatedClasses
					#define SOMGD__somGetRelatedClasses
				#else
					#define _somGetRelatedClasses SOMClassMgr_somGetRelatedClasses
				#endif
			#endif /* SOMGD__somGetRelatedClasses */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClassMgr_somGetRelatedClasses */
	#ifndef SOMClassMgr_somClassFromId
		#define SOMClassMgr_somClassFromId(somSelf,classId) \
			SOM_Resolve(somSelf,SOMClassMgr,somClassFromId)  \
				(somSelf,classId)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somClassFromId
				#if defined(_somClassFromId)
					#undef _somClassFromId
					#define SOMGD__somClassFromId
				#else
					#define _somClassFromId SOMClassMgr_somClassFromId
				#endif
			#endif /* SOMGD__somClassFromId */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClassMgr_somClassFromId */
	#ifndef SOMClassMgr_somFindClass
		#define SOMClassMgr_somFindClass(somSelf,classId,majorVersion,minorVersion) \
			SOM_Resolve(somSelf,SOMClassMgr,somFindClass)  \
				(somSelf,classId,majorVersion,minorVersion)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somFindClass
				#if defined(_somFindClass)
					#undef _somFindClass
					#define SOMGD__somFindClass
				#else
					#define _somFindClass SOMClassMgr_somFindClass
				#endif
			#endif /* SOMGD__somFindClass */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClassMgr_somFindClass */
	#ifndef SOMClassMgr_somFindClsInFile
		#define SOMClassMgr_somFindClsInFile(somSelf,classId,majorVersion,minorVersion,file) \
			SOM_Resolve(somSelf,SOMClassMgr,somFindClsInFile)  \
				(somSelf,classId,majorVersion,minorVersion,file)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somFindClsInFile
				#if defined(_somFindClsInFile)
					#undef _somFindClsInFile
					#define SOMGD__somFindClsInFile
				#else
					#define _somFindClsInFile SOMClassMgr_somFindClsInFile
				#endif
			#endif /* SOMGD__somFindClsInFile */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClassMgr_somFindClsInFile */
	#ifndef SOMClassMgr_somMergeInto
		#define SOMClassMgr_somMergeInto(somSelf,targetObj) \
			SOM_Resolve(somSelf,SOMClassMgr,somMergeInto)  \
				(somSelf,targetObj)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somMergeInto
				#if defined(_somMergeInto)
					#undef _somMergeInto
					#define SOMGD__somMergeInto
				#else
					#define _somMergeInto SOMClassMgr_somMergeInto
				#endif
			#endif /* SOMGD__somMergeInto */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClassMgr_somMergeInto */
	#ifndef SOMClassMgr_somSubstituteClass
		#define SOMClassMgr_somSubstituteClass(somSelf,origClassName,newClassName) \
			SOM_Resolve(somSelf,SOMClassMgr,somSubstituteClass)  \
				(somSelf,origClassName,newClassName)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somSubstituteClass
				#if defined(_somSubstituteClass)
					#undef _somSubstituteClass
					#define SOMGD__somSubstituteClass
				#else
					#define _somSubstituteClass SOMClassMgr_somSubstituteClass
				#endif
			#endif /* SOMGD__somSubstituteClass */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClassMgr_somSubstituteClass */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define SOMClassMgr_somInit SOMObject_somInit
#define SOMClassMgr_somUninit SOMObject_somUninit
#define SOMClassMgr_somFree SOMObject_somFree
#define SOMClassMgr_somGetClass SOMObject_somGetClass
#define SOMClassMgr_somGetClassName SOMObject_somGetClassName
#define SOMClassMgr_somGetSize SOMObject_somGetSize
#define SOMClassMgr_somIsA SOMObject_somIsA
#define SOMClassMgr_somIsInstanceOf SOMObject_somIsInstanceOf
#define SOMClassMgr_somRespondsTo SOMObject_somRespondsTo
#define SOMClassMgr_somDispatch SOMObject_somDispatch
#define SOMClassMgr_somClassDispatch SOMObject_somClassDispatch
#define SOMClassMgr_somCastObj SOMObject_somCastObj
#define SOMClassMgr_somResetObj SOMObject_somResetObj
#define SOMClassMgr_somPrintSelf SOMObject_somPrintSelf
#define SOMClassMgr_somDumpSelf SOMObject_somDumpSelf
#define SOMClassMgr_somDumpSelfInt SOMObject_somDumpSelfInt
#define SOMClassMgr_somDefaultInit SOMObject_somDefaultInit
#define SOMClassMgr_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define SOMClassMgr_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define SOMClassMgr_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define SOMClassMgr_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define SOMClassMgr_somDefaultAssign SOMObject_somDefaultAssign
#define SOMClassMgr_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define SOMClassMgr_somDefaultVAssign SOMObject_somDefaultVAssign
#define SOMClassMgr_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define SOMClassMgr_somDestruct SOMObject_somDestruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_somcm_Header_h */
