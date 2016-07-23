/* generated from repostry.idl */
/* internal conditional is SOM_Module_repostry_Source */
#ifndef SOM_Module_repostry_Header_h
	#define SOM_Module_repostry_Header_h 1
	#include <som.h>
	#include <somcls.h>
	#include <somobj.h>
	#include <containr.h>
	#ifndef _IDL_M_Repository_defined
		#define _IDL_M_Repository_defined
		typedef SOMObject M_Repository;
	#endif /* _IDL_M_Repository_defined */
	#ifndef _IDL_SEQUENCE_M_Repository_defined
		#define _IDL_SEQUENCE_M_Repository_defined
		SOM_SEQUENCE_TYPEDEF_NAME(M_Repository SOMSTAR ,sequence(M_Repository));
	#endif /* _IDL_SEQUENCE_M_Repository_defined */
	#ifndef _IDL_Repository_defined
		#define _IDL_Repository_defined
		typedef SOMObject Repository;
	#endif /* _IDL_Repository_defined */
	#ifndef _IDL_SEQUENCE_Repository_defined
		#define _IDL_SEQUENCE_Repository_defined
		SOM_SEQUENCE_TYPEDEF_NAME(Repository SOMSTAR ,sequence(Repository));
	#endif /* _IDL_SEQUENCE_Repository_defined */
	#ifndef M_Repository_MajorVersion
		#define M_Repository_MajorVersion   2
	#endif /* M_Repository_MajorVersion */
	#ifndef M_Repository_MinorVersion
		#define M_Repository_MinorVersion   1
	#endif /* M_Repository_MinorVersion */
	typedef struct M_RepositoryClassDataStructure
	{
		SOMClass SOMSTAR classObject;
	} M_RepositoryClassDataStructure;
	typedef struct M_RepositoryCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} M_RepositoryCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_repostry_Source) || defined(M_Repository_Class_Source)
			SOMEXTERN struct M_RepositoryClassDataStructure _M_RepositoryClassData;
			#ifndef M_RepositoryClassData
				#define M_RepositoryClassData    _M_RepositoryClassData
			#endif /* M_RepositoryClassData */
		#else
			SOMEXTERN struct M_RepositoryClassDataStructure * SOMLINK resolve_M_RepositoryClassData(void);
			#ifndef M_RepositoryClassData
				#define M_RepositoryClassData    (*(resolve_M_RepositoryClassData()))
			#endif /* M_RepositoryClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_repostry_Source) || defined(M_Repository_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_repostry_Source || M_Repository_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_repostry_Source || M_Repository_Class_Source */
		struct M_RepositoryClassDataStructure SOMDLINK M_RepositoryClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_repostry_Source) || defined(M_Repository_Class_Source)
			SOMEXTERN struct M_RepositoryCClassDataStructure _M_RepositoryCClassData;
			#ifndef M_RepositoryCClassData
				#define M_RepositoryCClassData    _M_RepositoryCClassData
			#endif /* M_RepositoryCClassData */
		#else
			SOMEXTERN struct M_RepositoryCClassDataStructure * SOMLINK resolve_M_RepositoryCClassData(void);
			#ifndef M_RepositoryCClassData
				#define M_RepositoryCClassData    (*(resolve_M_RepositoryCClassData()))
			#endif /* M_RepositoryCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_repostry_Source) || defined(M_Repository_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_repostry_Source || M_Repository_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_repostry_Source || M_Repository_Class_Source */
		struct M_RepositoryCClassDataStructure SOMDLINK M_RepositoryCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_repostry_Source) || defined(M_Repository_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_repostry_Source || M_Repository_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_repostry_Source || M_Repository_Class_Source */
	SOMClass SOMSTAR SOMLINK M_RepositoryNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_M_Repository (M_RepositoryClassData.classObject)
	#ifndef SOMGD_M_Repository
		#if (defined(_M_Repository) || defined(__M_Repository))
			#undef _M_Repository
			#undef __M_Repository
			#define SOMGD_M_Repository 1
		#else
			#define _M_Repository _SOMCLASS_M_Repository
		#endif /* _M_Repository */
	#endif /* SOMGD_M_Repository */
	#define M_Repository_classObj _SOMCLASS_M_Repository
	#define _SOMMTOKEN_M_Repository(method) ((somMToken)(M_RepositoryClassData.method))
	#ifndef M_RepositoryNew
		#define M_RepositoryNew() ( _M_Repository ? \
			(SOMClass_somNew(_M_Repository)) : \
			( M_RepositoryNewClass( \
				M_Repository_MajorVersion, \
				M_Repository_MinorVersion), \
			SOMClass_somNew(_M_Repository))) 
	#endif /* NewM_Repository */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define M_Repository_somInit SOMObject_somInit
#define M_Repository_somUninit SOMObject_somUninit
#define M_Repository_somFree SOMObject_somFree
#define M_Repository_somGetClass SOMObject_somGetClass
#define M_Repository_somGetClassName SOMObject_somGetClassName
#define M_Repository_somGetSize SOMObject_somGetSize
#define M_Repository_somIsA SOMObject_somIsA
#define M_Repository_somIsInstanceOf SOMObject_somIsInstanceOf
#define M_Repository_somRespondsTo SOMObject_somRespondsTo
#define M_Repository_somDispatch SOMObject_somDispatch
#define M_Repository_somClassDispatch SOMObject_somClassDispatch
#define M_Repository_somCastObj SOMObject_somCastObj
#define M_Repository_somResetObj SOMObject_somResetObj
#define M_Repository_somPrintSelf SOMObject_somPrintSelf
#define M_Repository_somDumpSelf SOMObject_somDumpSelf
#define M_Repository_somDumpSelfInt SOMObject_somDumpSelfInt
#define M_Repository_somDefaultInit SOMObject_somDefaultInit
#define M_Repository_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define M_Repository_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define M_Repository_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define M_Repository_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define M_Repository_somDefaultAssign SOMObject_somDefaultAssign
#define M_Repository_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define M_Repository_somDefaultVAssign SOMObject_somDefaultVAssign
#define M_Repository_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define M_Repository_somDestruct SOMObject_somDestruct
#define M_Repository_somNew SOMClass_somNew
#define M_Repository_somNewNoInit SOMClass_somNewNoInit
#define M_Repository_somRenew SOMClass_somRenew
#define M_Repository_somRenewNoInit SOMClass_somRenewNoInit
#define M_Repository_somRenewNoZero SOMClass_somRenewNoZero
#define M_Repository_somRenewNoInitNoZero SOMClass_somRenewNoInitNoZero
#define M_Repository_somAllocate SOMClass_somAllocate
#define M_Repository_somDeallocate SOMClass_somDeallocate
#define M_Repository_somGetParent SOMClass_somGetParent
#define M_Repository_somClassOfNewClassWithParents SOMClass_somClassOfNewClassWithParents
#define M_Repository_somInitClass SOMClass_somInitClass
#define M_Repository_somInitMIClass SOMClass_somInitMIClass
#define M_Repository_somAddStaticMethod SOMClass_somAddStaticMethod
#define M_Repository_somOverrideSMethod SOMClass_somOverrideSMethod
#define M_Repository_somClassReady SOMClass_somClassReady
#define M_Repository_somAddDynamicMethod SOMClass_somAddDynamicMethod
#define M_Repository_somGetName SOMClass_somGetName
#define M_Repository_somGetVersionNumbers SOMClass_somGetVersionNumbers
#define M_Repository_somGetNumMethods SOMClass_somGetNumMethods
#define M_Repository_somGetNumStaticMethods SOMClass_somGetNumStaticMethods
#define M_Repository_somGetParents SOMClass_somGetParents
#define M_Repository_somGetInstanceSize SOMClass_somGetInstanceSize
#define M_Repository_somGetInstancePartSize SOMClass_somGetInstancePartSize
#define M_Repository_somGetInstanceToken SOMClass_somGetInstanceToken
#define M_Repository_somGetMemberToken SOMClass_somGetMemberToken
#define M_Repository_somGetClassMtab SOMClass_somGetClassMtab
#define M_Repository_somGetClassData SOMClass_somGetClassData
#define M_Repository_somSetClassData SOMClass_somSetClassData
#define M_Repository_somSetMethodDescriptor SOMClass_somSetMethodDescriptor
#define M_Repository__get_somDataAlignment SOMClass__get_somDataAlignment
#define M_Repository__get_somInstanceDataOffsets SOMClass__get_somInstanceDataOffsets
#define M_Repository__set_somDirectInitClasses SOMClass__set_somDirectInitClasses
#define M_Repository__get_somDirectInitClasses SOMClass__get_somDirectInitClasses
#define M_Repository_somGetMethodDescriptor SOMClass_somGetMethodDescriptor
#define M_Repository_somGetMethodIndex SOMClass_somGetMethodIndex
#define M_Repository_somGetMethodToken SOMClass_somGetMethodToken
#define M_Repository_somGetNthMethodInfo SOMClass_somGetNthMethodInfo
#define M_Repository_somGetMethodData SOMClass_somGetMethodData
#define M_Repository_somGetNthMethodData SOMClass_somGetNthMethodData
#define M_Repository_somFindMethod SOMClass_somFindMethod
#define M_Repository_somFindMethodOk SOMClass_somFindMethodOk
#define M_Repository_somFindSMethod SOMClass_somFindSMethod
#define M_Repository_somFindSMethodOk SOMClass_somFindSMethodOk
#define M_Repository_somLookupMethod SOMClass_somLookupMethod
#define M_Repository_somGetApplyStub SOMClass_somGetApplyStub
#define M_Repository_somGetPClsMtab SOMClass_somGetPClsMtab
#define M_Repository_somCheckVersion SOMClass_somCheckVersion
#define M_Repository_somDescendedFrom SOMClass_somDescendedFrom
#define M_Repository_somSupportsMethod SOMClass_somSupportsMethod
#define M_Repository_somDefinedMethod SOMClass_somDefinedMethod
#define M_Repository_somGetRdStub SOMClass_somGetRdStub
#define M_Repository_somOverrideMtab SOMClass_somOverrideMtab
#define M_Repository__get_somClassAllocate SOMClass__get_somClassAllocate
#define M_Repository__get_somClassDeallocate SOMClass__get_somClassDeallocate
#endif /* SOM_DONT_USE_INHERITED_MACROS */

	typedef struct Repository_RepositoryDescription
	{
		Identifier name;
		string id;
		string defined_in;
	} Repository_RepositoryDescription;
	#ifndef _IDL_SEQUENCE_Repository_RepositoryDescription_defined
		#define _IDL_SEQUENCE_Repository_RepositoryDescription_defined
		SOM_SEQUENCE_TYPEDEF(Repository_RepositoryDescription);
	#endif /* _IDL_SEQUENCE_Repository_RepositoryDescription_defined */
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_RepositoryDescription
			#if defined(RepositoryDescription)
				#undef RepositoryDescription
				#define SOMGD_RepositoryDescription
			#else
				#define RepositoryDescription Repository_RepositoryDescription
			#endif
		#endif /* SOMGD_RepositoryDescription */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef _IDL_SEQUENCE_RepositoryDescription_defined
			#define _IDL_SEQUENCE_RepositoryDescription_defined
			#define _IDL_SEQUENCE_RepositoryDescription _IDL_SEQUENCE_Repository_RepositoryDescription
		#endif /* _IDL_SEQUENCE_RepositoryDescription_defined */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
	#ifndef Repository_irOpenErrorCodes_defined
		#define Repository_irOpenErrorCodes_defined
		typedef unsigned long Repository_irOpenErrorCodes;
		#ifndef Repository_NOACCESS
			#define Repository_NOACCESS 1UL
		#endif /* Repository_NOACCESS */
		#ifndef Repository_BADMAGICNUMBER
			#define Repository_BADMAGICNUMBER 2UL
		#endif /* Repository_BADMAGICNUMBER */
		#ifndef Repository_MISSINGVERSIONINFO
			#define Repository_MISSINGVERSIONINFO 3UL
		#endif /* Repository_MISSINGVERSIONINFO */
		#ifndef Repository_IOERROR
			#define Repository_IOERROR 4UL
		#endif /* Repository_IOERROR */
		#ifndef Repository_VERSIONMISMATCH
			#define Repository_VERSIONMISMATCH 5UL
		#endif /* Repository_VERSIONMISMATCH */
		#ifndef Repository_NOWRITEACCESS
			#define Repository_NOWRITEACCESS 6UL
		#endif /* Repository_NOWRITEACCESS */
		#ifndef Repository_INDEXINCONSISTENT
			#define Repository_INDEXINCONSISTENT 7UL
		#endif /* Repository_INDEXINCONSISTENT */
		#ifndef Repository_INDEXNOTPRESENT
			#define Repository_INDEXNOTPRESENT 8UL
		#endif /* Repository_INDEXNOTPRESENT */
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD_irOpenErrorCodes
				#if defined(irOpenErrorCodes)
					#undef irOpenErrorCodes
					#define SOMGD_irOpenErrorCodes
				#else
					#define irOpenErrorCodes Repository_irOpenErrorCodes
				#endif
			#endif /* SOMGD_irOpenErrorCodes */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* Repository_irOpenErrorCodes_defined */
#define ex_Repository_irOpenError   "::Repository::irOpenError"
#ifndef SOM_DONT_USE_SHORT_NAMES
#ifndef SOMTGD_ex_irOpenError
#ifndef ex_irOpenError
#define ex_irOpenError  ex_Repository_irOpenError
#else
#define SOMTGD_ex_irOpenError
#undef ex_irOpenError
#endif /* ex_irOpenError */
#endif /* SOMTGD_ex_irOpenError */
#endif /* SOM_DONT_USE_SHORT_NAMES */
typedef struct Repository_irOpenError
{
	Repository_irOpenErrorCodes errorCode;
	string fileName;
} Repository_irOpenError;
#ifndef SOM_DONT_USE_SHORT_NAMES
	#ifndef SOMGD_irOpenError
		#if defined(irOpenError)
			#undef irOpenError
			#define SOMGD_irOpenError
		#else
			#define irOpenError Repository_irOpenError
		#endif
	#endif /* SOMGD_irOpenError */
#endif /* SOM_DONT_USE_SHORT_NAMES */
	#ifdef __IBMC__
		typedef Contained SOMSTAR (somTP_Repository_lookup_id)(
			Repository SOMSTAR somSelf,
			Environment *ev,
			/* in */ string search_id);
		#pragma linkage(somTP_Repository_lookup_id,system)
		typedef somTP_Repository_lookup_id *somTD_Repository_lookup_id;
	#else /* __IBMC__ */
		typedef Contained SOMSTAR (SOMLINK * somTD_Repository_lookup_id)(
			Repository SOMSTAR somSelf,
			Environment *ev,
			/* in */ string search_id);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_Repository_lookup_modifier)(
			Repository SOMSTAR somSelf,
			Environment *ev,
			/* in */ string name,
			/* in */ string modifier);
		#pragma linkage(somTP_Repository_lookup_modifier,system)
		typedef somTP_Repository_lookup_modifier *somTD_Repository_lookup_modifier;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_Repository_lookup_modifier)(
			Repository SOMSTAR somSelf,
			Environment *ev,
			/* in */ string name,
			/* in */ string modifier);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_Repository_release_cache)(
			Repository SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_Repository_release_cache,system)
		typedef somTP_Repository_release_cache *somTD_Repository_release_cache;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_Repository_release_cache)(
			Repository SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef boolean (somTP_Repository_queryException)(
			Repository SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_Repository_queryException,system)
		typedef somTP_Repository_queryException *somTD_Repository_queryException;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_Repository_queryException)(
			Repository SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifndef Repository_MajorVersion
		#define Repository_MajorVersion   2
	#endif /* Repository_MajorVersion */
	#ifndef Repository_MinorVersion
		#define Repository_MinorVersion   1
	#endif /* Repository_MinorVersion */
	typedef struct RepositoryClassDataStructure
	{
		M_Repository SOMSTAR classObject;
		somMToken lookup_id;
		somMToken lookup_modifier;
		somMToken release_cache;
		somMToken saveIR;
		somMToken _get_subrepositorylist;
		somMToken _set_subrepositorylist;
		somMToken queryException;
	} RepositoryClassDataStructure;
	typedef struct RepositoryCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} RepositoryCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_repostry_Source) || defined(Repository_Class_Source)
			SOMEXTERN struct RepositoryClassDataStructure _RepositoryClassData;
			#ifndef RepositoryClassData
				#define RepositoryClassData    _RepositoryClassData
			#endif /* RepositoryClassData */
		#else
			SOMEXTERN struct RepositoryClassDataStructure * SOMLINK resolve_RepositoryClassData(void);
			#ifndef RepositoryClassData
				#define RepositoryClassData    (*(resolve_RepositoryClassData()))
			#endif /* RepositoryClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_repostry_Source) || defined(Repository_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_repostry_Source || Repository_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_repostry_Source || Repository_Class_Source */
		struct RepositoryClassDataStructure SOMDLINK RepositoryClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_repostry_Source) || defined(Repository_Class_Source)
			SOMEXTERN struct RepositoryCClassDataStructure _RepositoryCClassData;
			#ifndef RepositoryCClassData
				#define RepositoryCClassData    _RepositoryCClassData
			#endif /* RepositoryCClassData */
		#else
			SOMEXTERN struct RepositoryCClassDataStructure * SOMLINK resolve_RepositoryCClassData(void);
			#ifndef RepositoryCClassData
				#define RepositoryCClassData    (*(resolve_RepositoryCClassData()))
			#endif /* RepositoryCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_repostry_Source) || defined(Repository_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_repostry_Source || Repository_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_repostry_Source || Repository_Class_Source */
		struct RepositoryCClassDataStructure SOMDLINK RepositoryCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_repostry_Source) || defined(Repository_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_repostry_Source || Repository_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_repostry_Source || Repository_Class_Source */
	M_Repository SOMSTAR SOMLINK RepositoryNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_Repository (RepositoryClassData.classObject)
	#ifndef SOMGD_Repository
		#if (defined(_Repository) || defined(__Repository))
			#undef _Repository
			#undef __Repository
			#define SOMGD_Repository 1
		#else
			#define _Repository _SOMCLASS_Repository
		#endif /* _Repository */
	#endif /* SOMGD_Repository */
	#define Repository_classObj _SOMCLASS_Repository
	#define _SOMMTOKEN_Repository(method) ((somMToken)(RepositoryClassData.method))
	#ifndef RepositoryNew
		#define RepositoryNew() ( _Repository ? \
			(SOMClass_somNew(_Repository)) : \
			( RepositoryNewClass( \
				Repository_MajorVersion, \
				Repository_MinorVersion), \
			SOMClass_somNew(_Repository))) 
	#endif /* NewRepository */
	#ifndef Repository_lookup_id
		#define Repository_lookup_id(somSelf,ev,search_id) \
			SOM_Resolve(somSelf,Repository,lookup_id)  \
				(somSelf,ev,search_id)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__lookup_id
				#if defined(_lookup_id)
					#undef _lookup_id
					#define SOMGD__lookup_id
				#else
					#define _lookup_id Repository_lookup_id
				#endif
			#endif /* SOMGD__lookup_id */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* Repository_lookup_id */
	#ifndef Repository_lookup_modifier
		#define Repository_lookup_modifier(somSelf,ev,name,modifier) \
			SOM_Resolve(somSelf,Repository,lookup_modifier)  \
				(somSelf,ev,name,modifier)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__lookup_modifier
				#if defined(_lookup_modifier)
					#undef _lookup_modifier
					#define SOMGD__lookup_modifier
				#else
					#define _lookup_modifier Repository_lookup_modifier
				#endif
			#endif /* SOMGD__lookup_modifier */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* Repository_lookup_modifier */
	#ifndef Repository_release_cache
		#define Repository_release_cache(somSelf,ev) \
			SOM_Resolve(somSelf,Repository,release_cache)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__release_cache
				#if defined(_release_cache)
					#undef _release_cache
					#define SOMGD__release_cache
				#else
					#define _release_cache Repository_release_cache
				#endif
			#endif /* SOMGD__release_cache */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* Repository_release_cache */
	#ifndef Repository_queryException
		#define Repository_queryException(somSelf,ev) \
			SOM_Resolve(somSelf,Repository,queryException)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__queryException
				#if defined(_queryException)
					#undef _queryException
					#define SOMGD__queryException
				#else
					#define _queryException Repository_queryException
				#endif
			#endif /* SOMGD__queryException */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* Repository_queryException */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define Repository_somInit SOMObject_somInit
#define Repository_somUninit SOMObject_somUninit
#define Repository_somFree SOMObject_somFree
#define Repository_somGetClass SOMObject_somGetClass
#define Repository_somGetClassName SOMObject_somGetClassName
#define Repository_somGetSize SOMObject_somGetSize
#define Repository_somIsA SOMObject_somIsA
#define Repository_somIsInstanceOf SOMObject_somIsInstanceOf
#define Repository_somRespondsTo SOMObject_somRespondsTo
#define Repository_somDispatch SOMObject_somDispatch
#define Repository_somClassDispatch SOMObject_somClassDispatch
#define Repository_somCastObj SOMObject_somCastObj
#define Repository_somResetObj SOMObject_somResetObj
#define Repository_somPrintSelf SOMObject_somPrintSelf
#define Repository_somDumpSelf SOMObject_somDumpSelf
#define Repository_somDumpSelfInt SOMObject_somDumpSelfInt
#define Repository_somDefaultInit SOMObject_somDefaultInit
#define Repository_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define Repository_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define Repository_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define Repository_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define Repository_somDefaultAssign SOMObject_somDefaultAssign
#define Repository_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define Repository_somDefaultVAssign SOMObject_somDefaultVAssign
#define Repository_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define Repository_somDestruct SOMObject_somDestruct
#define Repository_contents Container_contents
#define Repository_lookup_name Container_lookup_name
#define Repository_describe_contents Container_describe_contents
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_repostry_Header_h */
