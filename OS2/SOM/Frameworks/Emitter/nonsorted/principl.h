/* generated from principl.idl */
/* internal conditional is SOM_Module_principl_Source */
#ifndef SOM_Module_principl_Header_h
	#define SOM_Module_principl_Header_h 1
	#include <som.h>
	#include <somobj.h>
	#ifndef _IDL_Principal_defined
		#define _IDL_Principal_defined
		typedef SOMObject Principal;
	#endif /* _IDL_Principal_defined */
	#ifndef _IDL_SEQUENCE_Principal_defined
		#define _IDL_SEQUENCE_Principal_defined
		SOM_SEQUENCE_TYPEDEF_NAME(Principal SOMSTAR ,sequence(Principal));
	#endif /* _IDL_SEQUENCE_Principal_defined */
	#ifdef __IBMC__
		typedef void (somTP_Principal__set_userName)(
			Principal SOMSTAR somSelf,
			Environment *ev,
			/* in */ string userName);
		#pragma linkage(somTP_Principal__set_userName,system)
		typedef somTP_Principal__set_userName *somTD_Principal__set_userName;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_Principal__set_userName)(
			Principal SOMSTAR somSelf,
			Environment *ev,
			/* in */ string userName);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_Principal__get_userName)(
			Principal SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_Principal__get_userName,system)
		typedef somTP_Principal__get_userName *somTD_Principal__get_userName;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_Principal__get_userName)(
			Principal SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_Principal__set_hostName)(
			Principal SOMSTAR somSelf,
			Environment *ev,
			/* in */ string hostName);
		#pragma linkage(somTP_Principal__set_hostName,system)
		typedef somTP_Principal__set_hostName *somTD_Principal__set_hostName;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_Principal__set_hostName)(
			Principal SOMSTAR somSelf,
			Environment *ev,
			/* in */ string hostName);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_Principal__get_hostName)(
			Principal SOMSTAR somSelf,
			Environment *ev);
		#pragma linkage(somTP_Principal__get_hostName,system)
		typedef somTP_Principal__get_hostName *somTD_Principal__get_hostName;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_Principal__get_hostName)(
			Principal SOMSTAR somSelf,
			Environment *ev);
	#endif /* __IBMC__ */
	#ifndef Principal_MajorVersion
		#define Principal_MajorVersion   2
	#endif /* Principal_MajorVersion */
	#ifndef Principal_MinorVersion
		#define Principal_MinorVersion   2
	#endif /* Principal_MinorVersion */
	typedef struct PrincipalClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken _set_userName;
		somMToken _get_userName;
		somMToken _set_hostName;
		somMToken _get_hostName;
	} PrincipalClassDataStructure;
	typedef struct PrincipalCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} PrincipalCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_principl_Source) || defined(Principal_Class_Source)
			SOMEXTERN struct PrincipalClassDataStructure _PrincipalClassData;
			#ifndef PrincipalClassData
				#define PrincipalClassData    _PrincipalClassData
			#endif /* PrincipalClassData */
		#else
			SOMEXTERN struct PrincipalClassDataStructure * SOMLINK resolve_PrincipalClassData(void);
			#ifndef PrincipalClassData
				#define PrincipalClassData    (*(resolve_PrincipalClassData()))
			#endif /* PrincipalClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_principl_Source) || defined(Principal_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_principl_Source || Principal_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_principl_Source || Principal_Class_Source */
		struct PrincipalClassDataStructure SOMDLINK PrincipalClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_principl_Source) || defined(Principal_Class_Source)
			SOMEXTERN struct PrincipalCClassDataStructure _PrincipalCClassData;
			#ifndef PrincipalCClassData
				#define PrincipalCClassData    _PrincipalCClassData
			#endif /* PrincipalCClassData */
		#else
			SOMEXTERN struct PrincipalCClassDataStructure * SOMLINK resolve_PrincipalCClassData(void);
			#ifndef PrincipalCClassData
				#define PrincipalCClassData    (*(resolve_PrincipalCClassData()))
			#endif /* PrincipalCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_principl_Source) || defined(Principal_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_principl_Source || Principal_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_principl_Source || Principal_Class_Source */
		struct PrincipalCClassDataStructure SOMDLINK PrincipalCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_principl_Source) || defined(Principal_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_principl_Source || Principal_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_principl_Source || Principal_Class_Source */
	SOMClass SOMSTAR SOMLINK PrincipalNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_Principal (PrincipalClassData.classObject)
	#ifndef SOMGD_Principal
		#if (defined(_Principal) || defined(__Principal))
			#undef _Principal
			#undef __Principal
			#define SOMGD_Principal 1
		#else
			#define _Principal _SOMCLASS_Principal
		#endif /* _Principal */
	#endif /* SOMGD_Principal */
	#define Principal_classObj _SOMCLASS_Principal
	#define _SOMMTOKEN_Principal(method) ((somMToken)(PrincipalClassData.method))
	#ifndef PrincipalNew
		#define PrincipalNew() ( _Principal ? \
			(SOMClass_somNew(_Principal)) : \
			( PrincipalNewClass( \
				Principal_MajorVersion, \
				Principal_MinorVersion), \
			SOMClass_somNew(_Principal))) 
	#endif /* NewPrincipal */
	#ifndef Principal__set_userName
		#define Principal__set_userName(somSelf,ev,userName) \
			SOM_Resolve(somSelf,Principal,_set_userName)  \
				(somSelf,ev,userName)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_userName
				#if defined(__set_userName)
					#undef __set_userName
					#define SOMGD___set_userName
				#else
					#define __set_userName Principal__set_userName
				#endif
			#endif /* SOMGD___set_userName */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* Principal__set_userName */
	#ifndef Principal__get_userName
		#define Principal__get_userName(somSelf,ev) \
			SOM_Resolve(somSelf,Principal,_get_userName)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_userName
				#if defined(__get_userName)
					#undef __get_userName
					#define SOMGD___get_userName
				#else
					#define __get_userName Principal__get_userName
				#endif
			#endif /* SOMGD___get_userName */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* Principal__get_userName */
	#ifndef Principal__set_hostName
		#define Principal__set_hostName(somSelf,ev,hostName) \
			SOM_Resolve(somSelf,Principal,_set_hostName)  \
				(somSelf,ev,hostName)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_hostName
				#if defined(__set_hostName)
					#undef __set_hostName
					#define SOMGD___set_hostName
				#else
					#define __set_hostName Principal__set_hostName
				#endif
			#endif /* SOMGD___set_hostName */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* Principal__set_hostName */
	#ifndef Principal__get_hostName
		#define Principal__get_hostName(somSelf,ev) \
			SOM_Resolve(somSelf,Principal,_get_hostName)  \
				(somSelf,ev)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_hostName
				#if defined(__get_hostName)
					#undef __get_hostName
					#define SOMGD___get_hostName
				#else
					#define __get_hostName Principal__get_hostName
				#endif
			#endif /* SOMGD___get_hostName */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* Principal__get_hostName */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define Principal_somInit SOMObject_somInit
#define Principal_somUninit SOMObject_somUninit
#define Principal_somFree SOMObject_somFree
#define Principal_somGetClass SOMObject_somGetClass
#define Principal_somGetClassName SOMObject_somGetClassName
#define Principal_somGetSize SOMObject_somGetSize
#define Principal_somIsA SOMObject_somIsA
#define Principal_somIsInstanceOf SOMObject_somIsInstanceOf
#define Principal_somRespondsTo SOMObject_somRespondsTo
#define Principal_somDispatch SOMObject_somDispatch
#define Principal_somClassDispatch SOMObject_somClassDispatch
#define Principal_somCastObj SOMObject_somCastObj
#define Principal_somResetObj SOMObject_somResetObj
#define Principal_somPrintSelf SOMObject_somPrintSelf
#define Principal_somDumpSelf SOMObject_somDumpSelf
#define Principal_somDumpSelfInt SOMObject_somDumpSelfInt
#define Principal_somDefaultInit SOMObject_somDefaultInit
#define Principal_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define Principal_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define Principal_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define Principal_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define Principal_somDefaultAssign SOMObject_somDefaultAssign
#define Principal_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define Principal_somDefaultVAssign SOMObject_somDefaultVAssign
#define Principal_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define Principal_somDestruct SOMObject_somDestruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_principl_Header_h */
