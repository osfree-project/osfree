/* generated from somcls.idl */
/* internal conditional is SOM_Module_somcls_Source */
#ifndef SOM_Module_somcls_Header_h
	#define SOM_Module_somcls_Header_h 1
	#include <som.h>
	#include <somobj.h>
	#ifndef _IDL_SOMClass_defined
		#define _IDL_SOMClass_defined
		typedef SOMObject SOMClass;
	#endif /* _IDL_SOMClass_defined */
	#ifndef _IDL_SEQUENCE_SOMClass_defined
		#define _IDL_SEQUENCE_SOMClass_defined
		SOM_SEQUENCE_TYPEDEF_NAME(SOMClass SOMSTAR ,sequence(SOMClass));
	#endif /* _IDL_SEQUENCE_SOMClass_defined */
	#ifndef _IDL_SEQUENCE_somToken_defined
		#define _IDL_SEQUENCE_somToken_defined
		SOM_SEQUENCE_TYPEDEF(somToken);
	#endif /* _IDL_SEQUENCE_somToken_defined */
	typedef _IDL_SEQUENCE_somToken SOMClass_somTokenSequence;
	#ifndef _IDL_SEQUENCE_SOMClass_somTokenSequence_defined
		#define _IDL_SEQUENCE_SOMClass_somTokenSequence_defined
		SOM_SEQUENCE_TYPEDEF(SOMClass_somTokenSequence);
	#endif /* _IDL_SEQUENCE_SOMClass_somTokenSequence_defined */
	typedef _IDL_SEQUENCE_SOMClass SOMClass_SOMClassSequence;
	#ifndef _IDL_SEQUENCE_SOMClass_SOMClassSequence_defined
		#define _IDL_SEQUENCE_SOMClass_SOMClassSequence_defined
		SOM_SEQUENCE_TYPEDEF(SOMClass_SOMClassSequence);
	#endif /* _IDL_SEQUENCE_SOMClass_SOMClassSequence_defined */
	typedef struct SOMClass_somOffsetInfo
	{
		SOMClass SOMSTAR cls;
		long offset;
	} SOMClass_somOffsetInfo;
	#ifndef _IDL_SEQUENCE_SOMClass_somOffsetInfo_defined
		#define _IDL_SEQUENCE_SOMClass_somOffsetInfo_defined
		SOM_SEQUENCE_TYPEDEF(SOMClass_somOffsetInfo);
	#endif /* _IDL_SEQUENCE_SOMClass_somOffsetInfo_defined */
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef SOMGD_somOffsetInfo
			#if defined(somOffsetInfo)
				#undef somOffsetInfo
				#define SOMGD_somOffsetInfo
			#else
				#define somOffsetInfo SOMClass_somOffsetInfo
			#endif
		#endif /* SOMGD_somOffsetInfo */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
	#ifndef SOM_DONT_USE_SHORT_NAMES
		#ifndef _IDL_SEQUENCE_somOffsetInfo_defined
			#define _IDL_SEQUENCE_somOffsetInfo_defined
			#define _IDL_SEQUENCE_somOffsetInfo _IDL_SEQUENCE_SOMClass_somOffsetInfo
		#endif /* _IDL_SEQUENCE_somOffsetInfo_defined */
	#endif /* SOM_DONT_USE_SHORT_NAMES */
	typedef _IDL_SEQUENCE_SOMClass_somOffsetInfo SOMClass_somOffsets;
	#ifndef _IDL_SEQUENCE_SOMClass_somOffsets_defined
		#define _IDL_SEQUENCE_SOMClass_somOffsets_defined
		SOM_SEQUENCE_TYPEDEF(SOMClass_somOffsets);
	#endif /* _IDL_SEQUENCE_SOMClass_somOffsets_defined */
	#ifndef _IDL_SEQUENCE_somId_defined
		#define _IDL_SEQUENCE_somId_defined
		SOM_SEQUENCE_TYPEDEF(somId);
	#endif /* _IDL_SEQUENCE_somId_defined */
	typedef _IDL_SEQUENCE_somId SOMClass_somIdSequence;
	#ifndef _IDL_SEQUENCE_SOMClass_somIdSequence_defined
		#define _IDL_SEQUENCE_SOMClass_somIdSequence_defined
		SOM_SEQUENCE_TYPEDEF(SOMClass_somIdSequence);
	#endif /* _IDL_SEQUENCE_SOMClass_somIdSequence_defined */
	#ifdef __IBMC__
		typedef SOMObject SOMSTAR (somTP_SOMClass_somNew)(
			SOMClass SOMSTAR somSelf);
		#pragma linkage(somTP_SOMClass_somNew,system)
		typedef somTP_SOMClass_somNew *somTD_SOMClass_somNew;
	#else /* __IBMC__ */
		typedef SOMObject SOMSTAR (SOMLINK * somTD_SOMClass_somNew)(
			SOMClass SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMObject SOMSTAR (somTP_SOMClass_somNewNoInit)(
			SOMClass SOMSTAR somSelf);
		#pragma linkage(somTP_SOMClass_somNewNoInit,system)
		typedef somTP_SOMClass_somNewNoInit *somTD_SOMClass_somNewNoInit;
	#else /* __IBMC__ */
		typedef SOMObject SOMSTAR (SOMLINK * somTD_SOMClass_somNewNoInit)(
			SOMClass SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMObject SOMSTAR (somTP_SOMClass_somRenew)(
			SOMClass SOMSTAR somSelf,
			/* in */ void *obj);
		#pragma linkage(somTP_SOMClass_somRenew,system)
		typedef somTP_SOMClass_somRenew *somTD_SOMClass_somRenew;
	#else /* __IBMC__ */
		typedef SOMObject SOMSTAR (SOMLINK * somTD_SOMClass_somRenew)(
			SOMClass SOMSTAR somSelf,
			/* in */ void *obj);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMObject SOMSTAR (somTP_SOMClass_somRenewNoInit)(
			SOMClass SOMSTAR somSelf,
			/* in */ void *obj);
		#pragma linkage(somTP_SOMClass_somRenewNoInit,system)
		typedef somTP_SOMClass_somRenewNoInit *somTD_SOMClass_somRenewNoInit;
	#else /* __IBMC__ */
		typedef SOMObject SOMSTAR (SOMLINK * somTD_SOMClass_somRenewNoInit)(
			SOMClass SOMSTAR somSelf,
			/* in */ void *obj);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMObject SOMSTAR (somTP_SOMClass_somRenewNoZero)(
			SOMClass SOMSTAR somSelf,
			/* in */ void *obj);
		#pragma linkage(somTP_SOMClass_somRenewNoZero,system)
		typedef somTP_SOMClass_somRenewNoZero *somTD_SOMClass_somRenewNoZero;
	#else /* __IBMC__ */
		typedef SOMObject SOMSTAR (SOMLINK * somTD_SOMClass_somRenewNoZero)(
			SOMClass SOMSTAR somSelf,
			/* in */ void *obj);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMObject SOMSTAR (somTP_SOMClass_somRenewNoInitNoZero)(
			SOMClass SOMSTAR somSelf,
			/* in */ void *obj);
		#pragma linkage(somTP_SOMClass_somRenewNoInitNoZero,system)
		typedef somTP_SOMClass_somRenewNoInitNoZero *somTD_SOMClass_somRenewNoInitNoZero;
	#else /* __IBMC__ */
		typedef SOMObject SOMSTAR (SOMLINK * somTD_SOMClass_somRenewNoInitNoZero)(
			SOMClass SOMSTAR somSelf,
			/* in */ void *obj);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef somToken (somTP_SOMClass_somAllocate)(
			SOMClass SOMSTAR somSelf,
			/* in */ long size);
		#pragma linkage(somTP_SOMClass_somAllocate,system)
		typedef somTP_SOMClass_somAllocate *somTD_SOMClass_somAllocate;
	#else /* __IBMC__ */
		typedef somToken (SOMLINK * somTD_SOMClass_somAllocate)(
			SOMClass SOMSTAR somSelf,
			/* in */ long size);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMClass_somDeallocate)(
			SOMClass SOMSTAR somSelf,
			/* in */ somToken memptr);
		#pragma linkage(somTP_SOMClass_somDeallocate,system)
		typedef somTP_SOMClass_somDeallocate *somTD_SOMClass_somDeallocate;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMClass_somDeallocate)(
			SOMClass SOMSTAR somSelf,
			/* in */ somToken memptr);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMClass SOMSTAR (somTP_SOMClass_somGetParent)(
			SOMClass SOMSTAR somSelf);
		#pragma linkage(somTP_SOMClass_somGetParent,system)
		typedef somTP_SOMClass_somGetParent *somTD_SOMClass_somGetParent;
	#else /* __IBMC__ */
		typedef SOMClass SOMSTAR (SOMLINK * somTD_SOMClass_somGetParent)(
			SOMClass SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMClass SOMSTAR (somTP_SOMClass_somClassOfNewClassWithParents)(
			/* in */ string newClassName,
			/* in */ SOMClass_SOMClassSequence *parents,
			/* in */ SOMClass SOMSTAR explicitMeta);
		#pragma linkage(somTP_SOMClass_somClassOfNewClassWithParents,system)
		typedef somTP_SOMClass_somClassOfNewClassWithParents *somTD_SOMClass_somClassOfNewClassWithParents;
	#else /* __IBMC__ */
		typedef SOMClass SOMSTAR (SOMLINK * somTD_SOMClass_somClassOfNewClassWithParents)(
			/* in */ string newClassName,
			/* in */ SOMClass_SOMClassSequence *parents,
			/* in */ SOMClass SOMSTAR explicitMeta);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMClass_somInitClass)(
			SOMClass SOMSTAR somSelf,
			/* in */ string className,
			/* in */ SOMClass SOMSTAR parentClass,
			/* in */ long dataSize,
			/* in */ long maxStaticMethods,
			/* in */ long majorVersion,
			/* in */ long minorVersion);
		#pragma linkage(somTP_SOMClass_somInitClass,system)
		typedef somTP_SOMClass_somInitClass *somTD_SOMClass_somInitClass;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMClass_somInitClass)(
			SOMClass SOMSTAR somSelf,
			/* in */ string className,
			/* in */ SOMClass SOMSTAR parentClass,
			/* in */ long dataSize,
			/* in */ long maxStaticMethods,
			/* in */ long majorVersion,
			/* in */ long minorVersion);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMClass_somInitMIClass)(
			SOMClass SOMSTAR somSelf,
			/* in */ unsigned long inherit_vars,
			/* in */ string className,
			/* in */ SOMClass_SOMClassSequence *parentClasses,
			/* in */ long dataSize,
			/* in */ long dataAlignment,
			/* in */ long maxNDMethods,
			/* in */ long majorVersion,
			/* in */ long minorVersion);
		#pragma linkage(somTP_SOMClass_somInitMIClass,system)
		typedef somTP_SOMClass_somInitMIClass *somTD_SOMClass_somInitMIClass;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMClass_somInitMIClass)(
			SOMClass SOMSTAR somSelf,
			/* in */ unsigned long inherit_vars,
			/* in */ string className,
			/* in */ SOMClass_SOMClassSequence *parentClasses,
			/* in */ long dataSize,
			/* in */ long dataAlignment,
			/* in */ long maxNDMethods,
			/* in */ long majorVersion,
			/* in */ long minorVersion);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef somMToken (somTP_SOMClass_somAddStaticMethod)(
			SOMClass SOMSTAR somSelf,
			/* in */ somId methodId,
			/* in */ somId methodDescriptor,
			/* in */ somMethodPtr method,
			/* in */ somMethodPtr redispatchStub,
			/* in */ somMethodPtr applyStub);
		#pragma linkage(somTP_SOMClass_somAddStaticMethod,system)
		typedef somTP_SOMClass_somAddStaticMethod *somTD_SOMClass_somAddStaticMethod;
	#else /* __IBMC__ */
		typedef somMToken (SOMLINK * somTD_SOMClass_somAddStaticMethod)(
			SOMClass SOMSTAR somSelf,
			/* in */ somId methodId,
			/* in */ somId methodDescriptor,
			/* in */ somMethodPtr method,
			/* in */ somMethodPtr redispatchStub,
			/* in */ somMethodPtr applyStub);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMClass_somOverrideSMethod)(
			SOMClass SOMSTAR somSelf,
			/* in */ somId methodId,
			/* in */ somMethodPtr method);
		#pragma linkage(somTP_SOMClass_somOverrideSMethod,system)
		typedef somTP_SOMClass_somOverrideSMethod *somTD_SOMClass_somOverrideSMethod;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMClass_somOverrideSMethod)(
			SOMClass SOMSTAR somSelf,
			/* in */ somId methodId,
			/* in */ somMethodPtr method);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMClass_somClassReady)(
			SOMClass SOMSTAR somSelf);
		#pragma linkage(somTP_SOMClass_somClassReady,system)
		typedef somTP_SOMClass_somClassReady *somTD_SOMClass_somClassReady;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMClass_somClassReady)(
			SOMClass SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMClass_somAddDynamicMethod)(
			SOMClass SOMSTAR somSelf,
			/* in */ somId methodId,
			/* in */ somId methodDescriptor,
			/* in */ somMethodPtr methodImpl,
			/* in */ somMethodPtr applyStub);
		#pragma linkage(somTP_SOMClass_somAddDynamicMethod,system)
		typedef somTP_SOMClass_somAddDynamicMethod *somTD_SOMClass_somAddDynamicMethod;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMClass_somAddDynamicMethod)(
			SOMClass SOMSTAR somSelf,
			/* in */ somId methodId,
			/* in */ somId methodDescriptor,
			/* in */ somMethodPtr methodImpl,
			/* in */ somMethodPtr applyStub);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef string (somTP_SOMClass_somGetName)(
			SOMClass SOMSTAR somSelf);
		#pragma linkage(somTP_SOMClass_somGetName,system)
		typedef somTP_SOMClass_somGetName *somTD_SOMClass_somGetName;
	#else /* __IBMC__ */
		typedef string (SOMLINK * somTD_SOMClass_somGetName)(
			SOMClass SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMClass_somGetVersionNumbers)(
			SOMClass SOMSTAR somSelf,
			/* out */ long *majorVersion,
			/* out */ long *minorVersion);
		#pragma linkage(somTP_SOMClass_somGetVersionNumbers,system)
		typedef somTP_SOMClass_somGetVersionNumbers *somTD_SOMClass_somGetVersionNumbers;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMClass_somGetVersionNumbers)(
			SOMClass SOMSTAR somSelf,
			/* out */ long *majorVersion,
			/* out */ long *minorVersion);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef long (somTP_SOMClass_somGetNumMethods)(
			SOMClass SOMSTAR somSelf);
		#pragma linkage(somTP_SOMClass_somGetNumMethods,system)
		typedef somTP_SOMClass_somGetNumMethods *somTD_SOMClass_somGetNumMethods;
	#else /* __IBMC__ */
		typedef long (SOMLINK * somTD_SOMClass_somGetNumMethods)(
			SOMClass SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef long (somTP_SOMClass_somGetNumStaticMethods)(
			SOMClass SOMSTAR somSelf);
		#pragma linkage(somTP_SOMClass_somGetNumStaticMethods,system)
		typedef somTP_SOMClass_somGetNumStaticMethods *somTD_SOMClass_somGetNumStaticMethods;
	#else /* __IBMC__ */
		typedef long (SOMLINK * somTD_SOMClass_somGetNumStaticMethods)(
			SOMClass SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMClass_SOMClassSequence (somTP_SOMClass_somGetParents)(
			SOMClass SOMSTAR somSelf);
		#pragma linkage(somTP_SOMClass_somGetParents,system)
		typedef somTP_SOMClass_somGetParents *somTD_SOMClass_somGetParents;
	#else /* __IBMC__ */
		typedef SOMClass_SOMClassSequence (SOMLINK * somTD_SOMClass_somGetParents)(
			SOMClass SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef long (somTP_SOMClass_somGetInstanceSize)(
			SOMClass SOMSTAR somSelf);
		#pragma linkage(somTP_SOMClass_somGetInstanceSize,system)
		typedef somTP_SOMClass_somGetInstanceSize *somTD_SOMClass_somGetInstanceSize;
	#else /* __IBMC__ */
		typedef long (SOMLINK * somTD_SOMClass_somGetInstanceSize)(
			SOMClass SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef long (somTP_SOMClass_somGetInstancePartSize)(
			SOMClass SOMSTAR somSelf);
		#pragma linkage(somTP_SOMClass_somGetInstancePartSize,system)
		typedef somTP_SOMClass_somGetInstancePartSize *somTD_SOMClass_somGetInstancePartSize;
	#else /* __IBMC__ */
		typedef long (SOMLINK * somTD_SOMClass_somGetInstancePartSize)(
			SOMClass SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef somDToken (somTP_SOMClass_somGetInstanceToken)(
			SOMClass SOMSTAR somSelf);
		#pragma linkage(somTP_SOMClass_somGetInstanceToken,system)
		typedef somTP_SOMClass_somGetInstanceToken *somTD_SOMClass_somGetInstanceToken;
	#else /* __IBMC__ */
		typedef somDToken (SOMLINK * somTD_SOMClass_somGetInstanceToken)(
			SOMClass SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef somDToken (somTP_SOMClass_somGetMemberToken)(
			SOMClass SOMSTAR somSelf,
			/* in */ long memberOffset,
			/* in */ somDToken instanceToken);
		#pragma linkage(somTP_SOMClass_somGetMemberToken,system)
		typedef somTP_SOMClass_somGetMemberToken *somTD_SOMClass_somGetMemberToken;
	#else /* __IBMC__ */
		typedef somDToken (SOMLINK * somTD_SOMClass_somGetMemberToken)(
			SOMClass SOMSTAR somSelf,
			/* in */ long memberOffset,
			/* in */ somDToken instanceToken);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef somMethodTab *(somTP_SOMClass_somGetClassMtab)(
			SOMClass SOMSTAR somSelf);
		#pragma linkage(somTP_SOMClass_somGetClassMtab,system)
		typedef somTP_SOMClass_somGetClassMtab *somTD_SOMClass_somGetClassMtab;
	#else /* __IBMC__ */
		typedef somMethodTab *(SOMLINK * somTD_SOMClass_somGetClassMtab)(
			SOMClass SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef somClassDataStructure *(somTP_SOMClass_somGetClassData)(
			SOMClass SOMSTAR somSelf);
		#pragma linkage(somTP_SOMClass_somGetClassData,system)
		typedef somTP_SOMClass_somGetClassData *somTD_SOMClass_somGetClassData;
	#else /* __IBMC__ */
		typedef somClassDataStructure *(SOMLINK * somTD_SOMClass_somGetClassData)(
			SOMClass SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMClass_somSetClassData)(
			SOMClass SOMSTAR somSelf,
			/* in */ somClassDataStructure *cds);
		#pragma linkage(somTP_SOMClass_somSetClassData,system)
		typedef somTP_SOMClass_somSetClassData *somTD_SOMClass_somSetClassData;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMClass_somSetClassData)(
			SOMClass SOMSTAR somSelf,
			/* in */ somClassDataStructure *cds);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef boolean (somTP_SOMClass_somSetMethodDescriptor)(
			SOMClass SOMSTAR somSelf,
			/* in */ somId methodId,
			/* in */ somId descriptor);
		#pragma linkage(somTP_SOMClass_somSetMethodDescriptor,system)
		typedef somTP_SOMClass_somSetMethodDescriptor *somTD_SOMClass_somSetMethodDescriptor;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_SOMClass_somSetMethodDescriptor)(
			SOMClass SOMSTAR somSelf,
			/* in */ somId methodId,
			/* in */ somId descriptor);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef long (somTP_SOMClass__get_somDataAlignment)(
			SOMClass SOMSTAR somSelf);
		#pragma linkage(somTP_SOMClass__get_somDataAlignment,system)
		typedef somTP_SOMClass__get_somDataAlignment *somTD_SOMClass__get_somDataAlignment;
	#else /* __IBMC__ */
		typedef long (SOMLINK * somTD_SOMClass__get_somDataAlignment)(
			SOMClass SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMClass_somOffsets (somTP_SOMClass__get_somInstanceDataOffsets)(
			SOMClass SOMSTAR somSelf);
		#pragma linkage(somTP_SOMClass__get_somInstanceDataOffsets,system)
		typedef somTP_SOMClass__get_somInstanceDataOffsets *somTD_SOMClass__get_somInstanceDataOffsets;
	#else /* __IBMC__ */
		typedef SOMClass_somOffsets (SOMLINK * somTD_SOMClass__get_somInstanceDataOffsets)(
			SOMClass SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMClass__set_somDirectInitClasses)(
			SOMClass SOMSTAR somSelf,
			/* in */ SOMClass_SOMClassSequence *somDirectInitClasses);
		#pragma linkage(somTP_SOMClass__set_somDirectInitClasses,system)
		typedef somTP_SOMClass__set_somDirectInitClasses *somTD_SOMClass__set_somDirectInitClasses;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMClass__set_somDirectInitClasses)(
			SOMClass SOMSTAR somSelf,
			/* in */ SOMClass_SOMClassSequence *somDirectInitClasses);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef SOMClass_SOMClassSequence (somTP_SOMClass__get_somDirectInitClasses)(
			SOMClass SOMSTAR somSelf);
		#pragma linkage(somTP_SOMClass__get_somDirectInitClasses,system)
		typedef somTP_SOMClass__get_somDirectInitClasses *somTD_SOMClass__get_somDirectInitClasses;
	#else /* __IBMC__ */
		typedef SOMClass_SOMClassSequence (SOMLINK * somTD_SOMClass__get_somDirectInitClasses)(
			SOMClass SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef somId (somTP_SOMClass_somGetMethodDescriptor)(
			SOMClass SOMSTAR somSelf,
			/* in */ somId methodId);
		#pragma linkage(somTP_SOMClass_somGetMethodDescriptor,system)
		typedef somTP_SOMClass_somGetMethodDescriptor *somTD_SOMClass_somGetMethodDescriptor;
	#else /* __IBMC__ */
		typedef somId (SOMLINK * somTD_SOMClass_somGetMethodDescriptor)(
			SOMClass SOMSTAR somSelf,
			/* in */ somId methodId);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef long (somTP_SOMClass_somGetMethodIndex)(
			SOMClass SOMSTAR somSelf,
			/* in */ somId id);
		#pragma linkage(somTP_SOMClass_somGetMethodIndex,system)
		typedef somTP_SOMClass_somGetMethodIndex *somTD_SOMClass_somGetMethodIndex;
	#else /* __IBMC__ */
		typedef long (SOMLINK * somTD_SOMClass_somGetMethodIndex)(
			SOMClass SOMSTAR somSelf,
			/* in */ somId id);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef somMToken (somTP_SOMClass_somGetMethodToken)(
			SOMClass SOMSTAR somSelf,
			/* in */ somId methodId);
		#pragma linkage(somTP_SOMClass_somGetMethodToken,system)
		typedef somTP_SOMClass_somGetMethodToken *somTD_SOMClass_somGetMethodToken;
	#else /* __IBMC__ */
		typedef somMToken (SOMLINK * somTD_SOMClass_somGetMethodToken)(
			SOMClass SOMSTAR somSelf,
			/* in */ somId methodId);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef somId (somTP_SOMClass_somGetNthMethodInfo)(
			SOMClass SOMSTAR somSelf,
			/* in */ long n,
			/* out */ somId *descriptor);
		#pragma linkage(somTP_SOMClass_somGetNthMethodInfo,system)
		typedef somTP_SOMClass_somGetNthMethodInfo *somTD_SOMClass_somGetNthMethodInfo;
	#else /* __IBMC__ */
		typedef somId (SOMLINK * somTD_SOMClass_somGetNthMethodInfo)(
			SOMClass SOMSTAR somSelf,
			/* in */ long n,
			/* out */ somId *descriptor);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef boolean (somTP_SOMClass_somGetMethodData)(
			SOMClass SOMSTAR somSelf,
			/* in */ somId methodId,
			/* out */ somMethodData *md);
		#pragma linkage(somTP_SOMClass_somGetMethodData,system)
		typedef somTP_SOMClass_somGetMethodData *somTD_SOMClass_somGetMethodData;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_SOMClass_somGetMethodData)(
			SOMClass SOMSTAR somSelf,
			/* in */ somId methodId,
			/* out */ somMethodData *md);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef boolean (somTP_SOMClass_somGetNthMethodData)(
			SOMClass SOMSTAR somSelf,
			/* in */ long n,
			/* out */ somMethodData *md);
		#pragma linkage(somTP_SOMClass_somGetNthMethodData,system)
		typedef somTP_SOMClass_somGetNthMethodData *somTD_SOMClass_somGetNthMethodData;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_SOMClass_somGetNthMethodData)(
			SOMClass SOMSTAR somSelf,
			/* in */ long n,
			/* out */ somMethodData *md);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef boolean (somTP_SOMClass_somFindMethod)(
			SOMClass SOMSTAR somSelf,
			/* in */ somId methodId,
			/* out */ somMethodPtr *m);
		#pragma linkage(somTP_SOMClass_somFindMethod,system)
		typedef somTP_SOMClass_somFindMethod *somTD_SOMClass_somFindMethod;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_SOMClass_somFindMethod)(
			SOMClass SOMSTAR somSelf,
			/* in */ somId methodId,
			/* out */ somMethodPtr *m);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef boolean (somTP_SOMClass_somFindMethodOk)(
			SOMClass SOMSTAR somSelf,
			/* in */ somId methodId,
			/* out */ somMethodPtr *m);
		#pragma linkage(somTP_SOMClass_somFindMethodOk,system)
		typedef somTP_SOMClass_somFindMethodOk *somTD_SOMClass_somFindMethodOk;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_SOMClass_somFindMethodOk)(
			SOMClass SOMSTAR somSelf,
			/* in */ somId methodId,
			/* out */ somMethodPtr *m);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef somMethodPtr (somTP_SOMClass_somFindSMethod)(
			SOMClass SOMSTAR somSelf,
			/* in */ somId methodId);
		#pragma linkage(somTP_SOMClass_somFindSMethod,system)
		typedef somTP_SOMClass_somFindSMethod *somTD_SOMClass_somFindSMethod;
	#else /* __IBMC__ */
		typedef somMethodPtr (SOMLINK * somTD_SOMClass_somFindSMethod)(
			SOMClass SOMSTAR somSelf,
			/* in */ somId methodId);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef somMethodPtr (somTP_SOMClass_somFindSMethodOk)(
			SOMClass SOMSTAR somSelf,
			/* in */ somId methodId);
		#pragma linkage(somTP_SOMClass_somFindSMethodOk,system)
		typedef somTP_SOMClass_somFindSMethodOk *somTD_SOMClass_somFindSMethodOk;
	#else /* __IBMC__ */
		typedef somMethodPtr (SOMLINK * somTD_SOMClass_somFindSMethodOk)(
			SOMClass SOMSTAR somSelf,
			/* in */ somId methodId);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef somMethodPtr (somTP_SOMClass_somLookupMethod)(
			SOMClass SOMSTAR somSelf,
			/* in */ somId methodId);
		#pragma linkage(somTP_SOMClass_somLookupMethod,system)
		typedef somTP_SOMClass_somLookupMethod *somTD_SOMClass_somLookupMethod;
	#else /* __IBMC__ */
		typedef somMethodPtr (SOMLINK * somTD_SOMClass_somLookupMethod)(
			SOMClass SOMSTAR somSelf,
			/* in */ somId methodId);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef somMethodPtr (somTP_SOMClass_somGetApplyStub)(
			SOMClass SOMSTAR somSelf,
			/* in */ somId methodId);
		#pragma linkage(somTP_SOMClass_somGetApplyStub,system)
		typedef somTP_SOMClass_somGetApplyStub *somTD_SOMClass_somGetApplyStub;
	#else /* __IBMC__ */
		typedef somMethodPtr (SOMLINK * somTD_SOMClass_somGetApplyStub)(
			SOMClass SOMSTAR somSelf,
			/* in */ somId methodId);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef somMethodTabs (somTP_SOMClass_somGetPClsMtab)(
			SOMClass SOMSTAR somSelf);
		#pragma linkage(somTP_SOMClass_somGetPClsMtab,system)
		typedef somTP_SOMClass_somGetPClsMtab *somTD_SOMClass_somGetPClsMtab;
	#else /* __IBMC__ */
		typedef somMethodTabs (SOMLINK * somTD_SOMClass_somGetPClsMtab)(
			SOMClass SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef boolean (somTP_SOMClass_somCheckVersion)(
			SOMClass SOMSTAR somSelf,
			/* in */ long majorVersion,
			/* in */ long minorVersion);
		#pragma linkage(somTP_SOMClass_somCheckVersion,system)
		typedef somTP_SOMClass_somCheckVersion *somTD_SOMClass_somCheckVersion;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_SOMClass_somCheckVersion)(
			SOMClass SOMSTAR somSelf,
			/* in */ long majorVersion,
			/* in */ long minorVersion);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef boolean (somTP_SOMClass_somDescendedFrom)(
			SOMClass SOMSTAR somSelf,
			/* in */ SOMClass SOMSTAR aClassObj);
		#pragma linkage(somTP_SOMClass_somDescendedFrom,system)
		typedef somTP_SOMClass_somDescendedFrom *somTD_SOMClass_somDescendedFrom;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_SOMClass_somDescendedFrom)(
			SOMClass SOMSTAR somSelf,
			/* in */ SOMClass SOMSTAR aClassObj);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef boolean (somTP_SOMClass_somSupportsMethod)(
			SOMClass SOMSTAR somSelf,
			/* in */ somId mId);
		#pragma linkage(somTP_SOMClass_somSupportsMethod,system)
		typedef somTP_SOMClass_somSupportsMethod *somTD_SOMClass_somSupportsMethod;
	#else /* __IBMC__ */
		typedef boolean (SOMLINK * somTD_SOMClass_somSupportsMethod)(
			SOMClass SOMSTAR somSelf,
			/* in */ somId mId);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef somMethodPtr (somTP_SOMClass_somDefinedMethod)(
			SOMClass SOMSTAR somSelf,
			/* in */ somMToken method);
		#pragma linkage(somTP_SOMClass_somDefinedMethod,system)
		typedef somTP_SOMClass_somDefinedMethod *somTD_SOMClass_somDefinedMethod;
	#else /* __IBMC__ */
		typedef somMethodPtr (SOMLINK * somTD_SOMClass_somDefinedMethod)(
			SOMClass SOMSTAR somSelf,
			/* in */ somMToken method);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef somMethodProc *(somTP_SOMClass_somGetRdStub)(
			SOMClass SOMSTAR somSelf,
			/* in */ somId methodId);
		#pragma linkage(somTP_SOMClass_somGetRdStub,system)
		typedef somTP_SOMClass_somGetRdStub *somTD_SOMClass_somGetRdStub;
	#else /* __IBMC__ */
		typedef somMethodProc *(SOMLINK * somTD_SOMClass_somGetRdStub)(
			SOMClass SOMSTAR somSelf,
			/* in */ somId methodId);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef void (somTP_SOMClass_somOverrideMtab)(
			SOMClass SOMSTAR somSelf);
		#pragma linkage(somTP_SOMClass_somOverrideMtab,system)
		typedef somTP_SOMClass_somOverrideMtab *somTD_SOMClass_somOverrideMtab;
	#else /* __IBMC__ */
		typedef void (SOMLINK * somTD_SOMClass_somOverrideMtab)(
			SOMClass SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef somMethodPtr (somTP_SOMClass__get_somClassAllocate)(
			SOMClass SOMSTAR somSelf);
		#pragma linkage(somTP_SOMClass__get_somClassAllocate,system)
		typedef somTP_SOMClass__get_somClassAllocate *somTD_SOMClass__get_somClassAllocate;
	#else /* __IBMC__ */
		typedef somMethodPtr (SOMLINK * somTD_SOMClass__get_somClassAllocate)(
			SOMClass SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifdef __IBMC__
		typedef somMethodPtr (somTP_SOMClass__get_somClassDeallocate)(
			SOMClass SOMSTAR somSelf);
		#pragma linkage(somTP_SOMClass__get_somClassDeallocate,system)
		typedef somTP_SOMClass__get_somClassDeallocate *somTD_SOMClass__get_somClassDeallocate;
	#else /* __IBMC__ */
		typedef somMethodPtr (SOMLINK * somTD_SOMClass__get_somClassDeallocate)(
			SOMClass SOMSTAR somSelf);
	#endif /* __IBMC__ */
	#ifndef SOMClass_MajorVersion
		#define SOMClass_MajorVersion   1
	#endif /* SOMClass_MajorVersion */
	#ifndef SOMClass_MinorVersion
		#define SOMClass_MinorVersion   6
	#endif /* SOMClass_MinorVersion */
	typedef struct SOMClassClassDataStructure
	{
		SOMClass SOMSTAR classObject;
		somMToken somNew;
		somMToken somRenew;
		somMToken somInitClass;
		somMToken somClassReady;
		somMToken somGetName;
		somMToken somGetParent;
		somMToken somDescendedFrom;
		somMToken somCheckVersion;
		somMToken somFindMethod;
		somMToken somFindMethodOk;
		somMToken somSupportsMethod;
		somMToken somGetNumMethods;
		somMToken somGetInstanceSize;
		somMToken somGetInstanceOffset;
		somMToken somGetInstancePartSize;
		somMToken somGetMethodIndex;
		somMToken somGetNumStaticMethods;
		somMToken somGetPClsMtab;
		somMToken somGetClassMtab;
		somMToken somAddStaticMethod;
		somMToken somOverrideSMethod;
		somMToken somAddDynamicMethod;
		somMToken somGetMethodOffset;
		somMToken somGetApplyStub;
		somMToken somFindSMethod;
		somMToken somFindSMethodOk;
		somMToken somGetMethodDescriptor;
		somMToken somGetNthMethodInfo;
		somMToken somSetClassData;
		somMToken somGetClassData;
		somMToken somNewNoInit;
		somMToken somRenewNoInit;
		somMToken somGetInstanceToken;
		somMToken somGetMemberToken;
		somMToken somSetMethodDescriptor;
		somMToken somGetMethodData;
		somMToken somOverrideMtab;
		somMToken somGetMethodToken;
		somMToken somGetParents;
		somMToken somGetPClsMtabs;
		somMToken somInitMIClass;
		somMToken somGetVersionNumbers;
		somMToken somLookupMethod;
		somMToken _get_somInstanceDataOffsets;
		somMToken somRenewNoZero;
		somMToken somRenewNoInitNoZero;
		somMToken somAllocate;
		somMToken somDeallocate;
		somMToken somGetRdStub;
		somMToken somGetNthMethodData;
		somMToken somCloneClass;
		somMToken _get_somMethodOffsets;
		somMToken _get_somDirectInitClasses;
		somMToken _set_somDirectInitClasses;
		somMToken somGetInstanceInitMask;
		somMToken somGetInstanceDestructionMask;
		somMToken somCastObjCls;
		somMToken somResetObjCls;
		somMToken _get_somTrueClass;
		somMToken _get_somCastedClass;
		somMToken somRegLPMToken;
		somMToken somDefinedMethod;
		somMToken somAddMethod;
		somMToken _get_somCClassData;
		somMToken _set_somCClassData;
		somTD_SOMClass_somClassOfNewClassWithParents somClassOfNewClassWithParents;
		somMToken _set_somClassDataOrder;
		somMToken _get_somClassDataOrder;
		somMToken somGetClassDataEntry;
		somMToken somSetClassDataEntry;
		somMToken _get_somDataAlignment;
		somMToken somGetInstanceAssignmentMask;
		somMToken _get_somDirectAssignClasses;
		somMToken setUserPCallDispatch;
		somMToken _get_somClassAllocate;
		somMToken _get_somClassDeallocate;
	} SOMClassClassDataStructure;
	typedef struct SOMClassCClassDataStructure
	{
		somMethodTabs parentMtab;
		somDToken instanceDataToken;
	} SOMClassCClassDataStructure;
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_somcls_Source) || defined(SOMClass_Class_Source)
			SOMEXTERN struct SOMClassClassDataStructure _SOMClassClassData;
			#ifndef SOMClassClassData
				#define SOMClassClassData    _SOMClassClassData
			#endif /* SOMClassClassData */
		#else
			SOMEXTERN struct SOMClassClassDataStructure * SOMLINK resolve_SOMClassClassData(void);
			#ifndef SOMClassClassData
				#define SOMClassClassData    (*(resolve_SOMClassClassData()))
			#endif /* SOMClassClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_somcls_Source) || defined(SOMClass_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_somcls_Source || SOMClass_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_somcls_Source || SOMClass_Class_Source */
		struct SOMClassClassDataStructure SOMDLINK SOMClassClassData;
	#endif /* SOM_RESOLVE_DATA */
	#ifdef SOM_RESOLVE_DATA
		#if defined(SOM_Module_somcls_Source) || defined(SOMClass_Class_Source)
			SOMEXTERN struct SOMClassCClassDataStructure _SOMClassCClassData;
			#ifndef SOMClassCClassData
				#define SOMClassCClassData    _SOMClassCClassData
			#endif /* SOMClassCClassData */
		#else
			SOMEXTERN struct SOMClassCClassDataStructure * SOMLINK resolve_SOMClassCClassData(void);
			#ifndef SOMClassCClassData
				#define SOMClassCClassData    (*(resolve_SOMClassCClassData()))
			#endif /* SOMClassCClassData */
		#endif
	#else /* SOM_RESOLVE_DATA */
		SOMEXTERN 
		#if defined(SOM_Module_somcls_Source) || defined(SOMClass_Class_Source)
			#ifdef SOMDLLEXPORT
				SOMDLLEXPORT
			#endif /* SOMDLLEXPORT */
		#else /* SOM_Module_somcls_Source || SOMClass_Class_Source */
			#ifdef SOMDLLIMPORT
				SOMDLLIMPORT
			#endif /* SOMDLLIMPORT */
		#endif /* SOM_Module_somcls_Source || SOMClass_Class_Source */
		struct SOMClassCClassDataStructure SOMDLINK SOMClassCClassData;
	#endif /* SOM_RESOLVE_DATA */
	extern 
	#if defined(SOM_Module_somcls_Source) || defined(SOMClass_Class_Source)
		#ifdef SOMDLLEXPORT
			SOMDLLEXPORT
		#endif /* SOMDLLEXPORT */
	#else /* SOM_Module_somcls_Source || SOMClass_Class_Source */
		#ifdef SOMDLLIMPORT
			SOMDLLIMPORT
		#endif /* SOMDLLIMPORT */
	#endif /* SOM_Module_somcls_Source || SOMClass_Class_Source */
	SOMClass SOMSTAR SOMLINK SOMClassNewClass
			(integer4 somtmajorVersion,integer4 somtminorVersion);
	#define _SOMCLASS_SOMClass (SOMClassClassData.classObject)
	#ifndef SOMGD_SOMClass
		#if (defined(_SOMClass) || defined(__SOMClass))
			#undef _SOMClass
			#undef __SOMClass
			#define SOMGD_SOMClass 1
		#else
			#define _SOMClass _SOMCLASS_SOMClass
		#endif /* _SOMClass */
	#endif /* SOMGD_SOMClass */
	#define SOMClass_classObj _SOMCLASS_SOMClass
	#define _SOMMTOKEN_SOMClass(method) ((somMToken)(SOMClassClassData.method))
	#ifndef SOMClassNew
		#define SOMClassNew() ( _SOMClass ? \
			(SOMClass_somNew(_SOMClass)) : \
			( SOMClassNewClass( \
				SOMClass_MajorVersion, \
				SOMClass_MinorVersion), \
			SOMClass_somNew(_SOMClass))) 
	#endif /* NewSOMClass */
	#ifndef SOMClass_somNew
		#define SOMClass_somNew(somSelf) \
			SOM_Resolve(somSelf,SOMClass,somNew)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somNew
				#if defined(_somNew)
					#undef _somNew
					#define SOMGD__somNew
				#else
					#define _somNew SOMClass_somNew
				#endif
			#endif /* SOMGD__somNew */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somNew */
	#ifndef SOMClass_somNewNoInit
		#define SOMClass_somNewNoInit(somSelf) \
			SOM_Resolve(somSelf,SOMClass,somNewNoInit)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somNewNoInit
				#if defined(_somNewNoInit)
					#undef _somNewNoInit
					#define SOMGD__somNewNoInit
				#else
					#define _somNewNoInit SOMClass_somNewNoInit
				#endif
			#endif /* SOMGD__somNewNoInit */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somNewNoInit */
	#ifndef SOMClass_somRenew
		#define SOMClass_somRenew(somSelf,obj) \
			SOM_Resolve(somSelf,SOMClass,somRenew)  \
				(somSelf,obj)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somRenew
				#if defined(_somRenew)
					#undef _somRenew
					#define SOMGD__somRenew
				#else
					#define _somRenew SOMClass_somRenew
				#endif
			#endif /* SOMGD__somRenew */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somRenew */
	#ifndef SOMClass_somRenewNoInit
		#define SOMClass_somRenewNoInit(somSelf,obj) \
			SOM_Resolve(somSelf,SOMClass,somRenewNoInit)  \
				(somSelf,obj)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somRenewNoInit
				#if defined(_somRenewNoInit)
					#undef _somRenewNoInit
					#define SOMGD__somRenewNoInit
				#else
					#define _somRenewNoInit SOMClass_somRenewNoInit
				#endif
			#endif /* SOMGD__somRenewNoInit */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somRenewNoInit */
	#ifndef SOMClass_somRenewNoZero
		#define SOMClass_somRenewNoZero(somSelf,obj) \
			SOM_Resolve(somSelf,SOMClass,somRenewNoZero)  \
				(somSelf,obj)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somRenewNoZero
				#if defined(_somRenewNoZero)
					#undef _somRenewNoZero
					#define SOMGD__somRenewNoZero
				#else
					#define _somRenewNoZero SOMClass_somRenewNoZero
				#endif
			#endif /* SOMGD__somRenewNoZero */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somRenewNoZero */
	#ifndef SOMClass_somRenewNoInitNoZero
		#define SOMClass_somRenewNoInitNoZero(somSelf,obj) \
			SOM_Resolve(somSelf,SOMClass,somRenewNoInitNoZero)  \
				(somSelf,obj)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somRenewNoInitNoZero
				#if defined(_somRenewNoInitNoZero)
					#undef _somRenewNoInitNoZero
					#define SOMGD__somRenewNoInitNoZero
				#else
					#define _somRenewNoInitNoZero SOMClass_somRenewNoInitNoZero
				#endif
			#endif /* SOMGD__somRenewNoInitNoZero */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somRenewNoInitNoZero */
	#ifndef SOMClass_somAllocate
		#define SOMClass_somAllocate(somSelf,size) \
			SOM_Resolve(somSelf,SOMClass,somAllocate)  \
				(somSelf,size)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somAllocate
				#if defined(_somAllocate)
					#undef _somAllocate
					#define SOMGD__somAllocate
				#else
					#define _somAllocate SOMClass_somAllocate
				#endif
			#endif /* SOMGD__somAllocate */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somAllocate */
	#ifndef SOMClass_somDeallocate
		#define SOMClass_somDeallocate(somSelf,memptr) \
			SOM_Resolve(somSelf,SOMClass,somDeallocate)  \
				(somSelf,memptr)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somDeallocate
				#if defined(_somDeallocate)
					#undef _somDeallocate
					#define SOMGD__somDeallocate
				#else
					#define _somDeallocate SOMClass_somDeallocate
				#endif
			#endif /* SOMGD__somDeallocate */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somDeallocate */
	#ifndef SOMClass_somGetParent
		#define SOMClass_somGetParent(somSelf) \
			SOM_Resolve(somSelf,SOMClass,somGetParent)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somGetParent
				#if defined(_somGetParent)
					#undef _somGetParent
					#define SOMGD__somGetParent
				#else
					#define _somGetParent SOMClass_somGetParent
				#endif
			#endif /* SOMGD__somGetParent */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somGetParent */
	#ifndef SOMClass_somClassOfNewClassWithParents
		#define SOMClass_somClassOfNewClassWithParents SOMClassClassData.somClassOfNewClassWithParents
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somClassOfNewClassWithParents
				#if defined(_somClassOfNewClassWithParents)
					#undef _somClassOfNewClassWithParents
					#define SOMGD__somClassOfNewClassWithParents
				#else
					#define _somClassOfNewClassWithParents SOMClass_somClassOfNewClassWithParents
				#endif
			#endif /* SOMGD__somClassOfNewClassWithParents */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somClassOfNewClassWithParents */
	#ifndef SOMClass_somInitClass
		#define SOMClass_somInitClass(somSelf,className,parentClass,dataSize,maxStaticMethods,majorVersion,minorVersion) \
			SOM_Resolve(somSelf,SOMClass,somInitClass)  \
				(somSelf,className,parentClass,dataSize,maxStaticMethods,majorVersion,minorVersion)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somInitClass
				#if defined(_somInitClass)
					#undef _somInitClass
					#define SOMGD__somInitClass
				#else
					#define _somInitClass SOMClass_somInitClass
				#endif
			#endif /* SOMGD__somInitClass */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somInitClass */
	#ifndef SOMClass_somInitMIClass
		#define SOMClass_somInitMIClass(somSelf,inherit_vars,className,parentClasses,dataSize,dataAlignment,maxNDMethods,majorVersion,minorVersion) \
			SOM_Resolve(somSelf,SOMClass,somInitMIClass)  \
				(somSelf,inherit_vars,className,parentClasses,dataSize,dataAlignment,maxNDMethods,majorVersion,minorVersion)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somInitMIClass
				#if defined(_somInitMIClass)
					#undef _somInitMIClass
					#define SOMGD__somInitMIClass
				#else
					#define _somInitMIClass SOMClass_somInitMIClass
				#endif
			#endif /* SOMGD__somInitMIClass */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somInitMIClass */
	#ifndef SOMClass_somAddStaticMethod
		#define SOMClass_somAddStaticMethod(somSelf,methodId,methodDescriptor,method,redispatchStub,applyStub) \
			SOM_Resolve(somSelf,SOMClass,somAddStaticMethod)  \
				(somSelf,methodId,methodDescriptor,method,redispatchStub,applyStub)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somAddStaticMethod
				#if defined(_somAddStaticMethod)
					#undef _somAddStaticMethod
					#define SOMGD__somAddStaticMethod
				#else
					#define _somAddStaticMethod SOMClass_somAddStaticMethod
				#endif
			#endif /* SOMGD__somAddStaticMethod */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somAddStaticMethod */
	#ifndef SOMClass_somOverrideSMethod
		#define SOMClass_somOverrideSMethod(somSelf,methodId,method) \
			SOM_Resolve(somSelf,SOMClass,somOverrideSMethod)  \
				(somSelf,methodId,method)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somOverrideSMethod
				#if defined(_somOverrideSMethod)
					#undef _somOverrideSMethod
					#define SOMGD__somOverrideSMethod
				#else
					#define _somOverrideSMethod SOMClass_somOverrideSMethod
				#endif
			#endif /* SOMGD__somOverrideSMethod */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somOverrideSMethod */
	#ifndef SOMClass_somClassReady
		#define SOMClass_somClassReady(somSelf) \
			SOM_Resolve(somSelf,SOMClass,somClassReady)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somClassReady
				#if defined(_somClassReady)
					#undef _somClassReady
					#define SOMGD__somClassReady
				#else
					#define _somClassReady SOMClass_somClassReady
				#endif
			#endif /* SOMGD__somClassReady */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somClassReady */
	#ifndef SOMClass_somAddDynamicMethod
		#define SOMClass_somAddDynamicMethod(somSelf,methodId,methodDescriptor,methodImpl,applyStub) \
			SOM_Resolve(somSelf,SOMClass,somAddDynamicMethod)  \
				(somSelf,methodId,methodDescriptor,methodImpl,applyStub)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somAddDynamicMethod
				#if defined(_somAddDynamicMethod)
					#undef _somAddDynamicMethod
					#define SOMGD__somAddDynamicMethod
				#else
					#define _somAddDynamicMethod SOMClass_somAddDynamicMethod
				#endif
			#endif /* SOMGD__somAddDynamicMethod */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somAddDynamicMethod */
	#ifndef SOMClass_somGetName
		#define SOMClass_somGetName(somSelf) \
			SOM_Resolve(somSelf,SOMClass,somGetName)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somGetName
				#if defined(_somGetName)
					#undef _somGetName
					#define SOMGD__somGetName
				#else
					#define _somGetName SOMClass_somGetName
				#endif
			#endif /* SOMGD__somGetName */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somGetName */
	#ifndef SOMClass_somGetVersionNumbers
		#define SOMClass_somGetVersionNumbers(somSelf,majorVersion,minorVersion) \
			SOM_Resolve(somSelf,SOMClass,somGetVersionNumbers)  \
				(somSelf,majorVersion,minorVersion)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somGetVersionNumbers
				#if defined(_somGetVersionNumbers)
					#undef _somGetVersionNumbers
					#define SOMGD__somGetVersionNumbers
				#else
					#define _somGetVersionNumbers SOMClass_somGetVersionNumbers
				#endif
			#endif /* SOMGD__somGetVersionNumbers */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somGetVersionNumbers */
	#ifndef SOMClass_somGetNumMethods
		#define SOMClass_somGetNumMethods(somSelf) \
			SOM_Resolve(somSelf,SOMClass,somGetNumMethods)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somGetNumMethods
				#if defined(_somGetNumMethods)
					#undef _somGetNumMethods
					#define SOMGD__somGetNumMethods
				#else
					#define _somGetNumMethods SOMClass_somGetNumMethods
				#endif
			#endif /* SOMGD__somGetNumMethods */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somGetNumMethods */
	#ifndef SOMClass_somGetNumStaticMethods
		#define SOMClass_somGetNumStaticMethods(somSelf) \
			SOM_Resolve(somSelf,SOMClass,somGetNumStaticMethods)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somGetNumStaticMethods
				#if defined(_somGetNumStaticMethods)
					#undef _somGetNumStaticMethods
					#define SOMGD__somGetNumStaticMethods
				#else
					#define _somGetNumStaticMethods SOMClass_somGetNumStaticMethods
				#endif
			#endif /* SOMGD__somGetNumStaticMethods */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somGetNumStaticMethods */
	#ifndef SOMClass_somGetParents
		#define SOMClass_somGetParents(somSelf) \
			SOM_Resolve(somSelf,SOMClass,somGetParents)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somGetParents
				#if defined(_somGetParents)
					#undef _somGetParents
					#define SOMGD__somGetParents
				#else
					#define _somGetParents SOMClass_somGetParents
				#endif
			#endif /* SOMGD__somGetParents */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somGetParents */
	#ifndef SOMClass_somGetInstanceSize
		#define SOMClass_somGetInstanceSize(somSelf) \
			SOM_Resolve(somSelf,SOMClass,somGetInstanceSize)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somGetInstanceSize
				#if defined(_somGetInstanceSize)
					#undef _somGetInstanceSize
					#define SOMGD__somGetInstanceSize
				#else
					#define _somGetInstanceSize SOMClass_somGetInstanceSize
				#endif
			#endif /* SOMGD__somGetInstanceSize */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somGetInstanceSize */
	#ifndef SOMClass_somGetInstancePartSize
		#define SOMClass_somGetInstancePartSize(somSelf) \
			SOM_Resolve(somSelf,SOMClass,somGetInstancePartSize)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somGetInstancePartSize
				#if defined(_somGetInstancePartSize)
					#undef _somGetInstancePartSize
					#define SOMGD__somGetInstancePartSize
				#else
					#define _somGetInstancePartSize SOMClass_somGetInstancePartSize
				#endif
			#endif /* SOMGD__somGetInstancePartSize */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somGetInstancePartSize */
	#ifndef SOMClass_somGetInstanceToken
		#define SOMClass_somGetInstanceToken(somSelf) \
			SOM_Resolve(somSelf,SOMClass,somGetInstanceToken)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somGetInstanceToken
				#if defined(_somGetInstanceToken)
					#undef _somGetInstanceToken
					#define SOMGD__somGetInstanceToken
				#else
					#define _somGetInstanceToken SOMClass_somGetInstanceToken
				#endif
			#endif /* SOMGD__somGetInstanceToken */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somGetInstanceToken */
	#ifndef SOMClass_somGetMemberToken
		#define SOMClass_somGetMemberToken(somSelf,memberOffset,instanceToken) \
			SOM_Resolve(somSelf,SOMClass,somGetMemberToken)  \
				(somSelf,memberOffset,instanceToken)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somGetMemberToken
				#if defined(_somGetMemberToken)
					#undef _somGetMemberToken
					#define SOMGD__somGetMemberToken
				#else
					#define _somGetMemberToken SOMClass_somGetMemberToken
				#endif
			#endif /* SOMGD__somGetMemberToken */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somGetMemberToken */
	#ifndef SOMClass_somGetClassMtab
		#define SOMClass_somGetClassMtab(somSelf) \
			SOM_Resolve(somSelf,SOMClass,somGetClassMtab)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somGetClassMtab
				#if defined(_somGetClassMtab)
					#undef _somGetClassMtab
					#define SOMGD__somGetClassMtab
				#else
					#define _somGetClassMtab SOMClass_somGetClassMtab
				#endif
			#endif /* SOMGD__somGetClassMtab */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somGetClassMtab */
	#ifndef SOMClass_somGetClassData
		#define SOMClass_somGetClassData(somSelf) \
			SOM_Resolve(somSelf,SOMClass,somGetClassData)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somGetClassData
				#if defined(_somGetClassData)
					#undef _somGetClassData
					#define SOMGD__somGetClassData
				#else
					#define _somGetClassData SOMClass_somGetClassData
				#endif
			#endif /* SOMGD__somGetClassData */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somGetClassData */
	#ifndef SOMClass_somSetClassData
		#define SOMClass_somSetClassData(somSelf,cds) \
			SOM_Resolve(somSelf,SOMClass,somSetClassData)  \
				(somSelf,cds)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somSetClassData
				#if defined(_somSetClassData)
					#undef _somSetClassData
					#define SOMGD__somSetClassData
				#else
					#define _somSetClassData SOMClass_somSetClassData
				#endif
			#endif /* SOMGD__somSetClassData */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somSetClassData */
	#ifndef SOMClass_somSetMethodDescriptor
		#define SOMClass_somSetMethodDescriptor(somSelf,methodId,descriptor) \
			SOM_Resolve(somSelf,SOMClass,somSetMethodDescriptor)  \
				(somSelf,methodId,descriptor)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somSetMethodDescriptor
				#if defined(_somSetMethodDescriptor)
					#undef _somSetMethodDescriptor
					#define SOMGD__somSetMethodDescriptor
				#else
					#define _somSetMethodDescriptor SOMClass_somSetMethodDescriptor
				#endif
			#endif /* SOMGD__somSetMethodDescriptor */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somSetMethodDescriptor */
	#ifndef SOMClass__get_somDataAlignment
		#define SOMClass__get_somDataAlignment(somSelf) \
			SOM_Resolve(somSelf,SOMClass,_get_somDataAlignment)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somDataAlignment
				#if defined(__get_somDataAlignment)
					#undef __get_somDataAlignment
					#define SOMGD___get_somDataAlignment
				#else
					#define __get_somDataAlignment SOMClass__get_somDataAlignment
				#endif
			#endif /* SOMGD___get_somDataAlignment */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass__get_somDataAlignment */
	#ifndef SOMClass__get_somInstanceDataOffsets
		#define SOMClass__get_somInstanceDataOffsets(somSelf) \
			SOM_Resolve(somSelf,SOMClass,_get_somInstanceDataOffsets)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somInstanceDataOffsets
				#if defined(__get_somInstanceDataOffsets)
					#undef __get_somInstanceDataOffsets
					#define SOMGD___get_somInstanceDataOffsets
				#else
					#define __get_somInstanceDataOffsets SOMClass__get_somInstanceDataOffsets
				#endif
			#endif /* SOMGD___get_somInstanceDataOffsets */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass__get_somInstanceDataOffsets */
	#ifndef SOMClass__set_somDirectInitClasses
		#define SOMClass__set_somDirectInitClasses(somSelf,somDirectInitClasses) \
			SOM_Resolve(somSelf,SOMClass,_set_somDirectInitClasses)  \
				(somSelf,somDirectInitClasses)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___set_somDirectInitClasses
				#if defined(__set_somDirectInitClasses)
					#undef __set_somDirectInitClasses
					#define SOMGD___set_somDirectInitClasses
				#else
					#define __set_somDirectInitClasses SOMClass__set_somDirectInitClasses
				#endif
			#endif /* SOMGD___set_somDirectInitClasses */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass__set_somDirectInitClasses */
	#ifndef SOMClass__get_somDirectInitClasses
		#define SOMClass__get_somDirectInitClasses(somSelf) \
			SOM_Resolve(somSelf,SOMClass,_get_somDirectInitClasses)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somDirectInitClasses
				#if defined(__get_somDirectInitClasses)
					#undef __get_somDirectInitClasses
					#define SOMGD___get_somDirectInitClasses
				#else
					#define __get_somDirectInitClasses SOMClass__get_somDirectInitClasses
				#endif
			#endif /* SOMGD___get_somDirectInitClasses */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass__get_somDirectInitClasses */
	#ifndef SOMClass_somGetMethodDescriptor
		#define SOMClass_somGetMethodDescriptor(somSelf,methodId) \
			SOM_Resolve(somSelf,SOMClass,somGetMethodDescriptor)  \
				(somSelf,methodId)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somGetMethodDescriptor
				#if defined(_somGetMethodDescriptor)
					#undef _somGetMethodDescriptor
					#define SOMGD__somGetMethodDescriptor
				#else
					#define _somGetMethodDescriptor SOMClass_somGetMethodDescriptor
				#endif
			#endif /* SOMGD__somGetMethodDescriptor */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somGetMethodDescriptor */
	#ifndef SOMClass_somGetMethodIndex
		#define SOMClass_somGetMethodIndex(somSelf,id) \
			SOM_Resolve(somSelf,SOMClass,somGetMethodIndex)  \
				(somSelf,id)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somGetMethodIndex
				#if defined(_somGetMethodIndex)
					#undef _somGetMethodIndex
					#define SOMGD__somGetMethodIndex
				#else
					#define _somGetMethodIndex SOMClass_somGetMethodIndex
				#endif
			#endif /* SOMGD__somGetMethodIndex */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somGetMethodIndex */
	#ifndef SOMClass_somGetMethodToken
		#define SOMClass_somGetMethodToken(somSelf,methodId) \
			SOM_Resolve(somSelf,SOMClass,somGetMethodToken)  \
				(somSelf,methodId)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somGetMethodToken
				#if defined(_somGetMethodToken)
					#undef _somGetMethodToken
					#define SOMGD__somGetMethodToken
				#else
					#define _somGetMethodToken SOMClass_somGetMethodToken
				#endif
			#endif /* SOMGD__somGetMethodToken */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somGetMethodToken */
	#ifndef SOMClass_somGetNthMethodInfo
		#define SOMClass_somGetNthMethodInfo(somSelf,n,descriptor) \
			SOM_Resolve(somSelf,SOMClass,somGetNthMethodInfo)  \
				(somSelf,n,descriptor)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somGetNthMethodInfo
				#if defined(_somGetNthMethodInfo)
					#undef _somGetNthMethodInfo
					#define SOMGD__somGetNthMethodInfo
				#else
					#define _somGetNthMethodInfo SOMClass_somGetNthMethodInfo
				#endif
			#endif /* SOMGD__somGetNthMethodInfo */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somGetNthMethodInfo */
	#ifndef SOMClass_somGetMethodData
		#define SOMClass_somGetMethodData(somSelf,methodId,md) \
			SOM_Resolve(somSelf,SOMClass,somGetMethodData)  \
				(somSelf,methodId,md)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somGetMethodData
				#if defined(_somGetMethodData)
					#undef _somGetMethodData
					#define SOMGD__somGetMethodData
				#else
					#define _somGetMethodData SOMClass_somGetMethodData
				#endif
			#endif /* SOMGD__somGetMethodData */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somGetMethodData */
	#ifndef SOMClass_somGetNthMethodData
		#define SOMClass_somGetNthMethodData(somSelf,n,md) \
			SOM_Resolve(somSelf,SOMClass,somGetNthMethodData)  \
				(somSelf,n,md)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somGetNthMethodData
				#if defined(_somGetNthMethodData)
					#undef _somGetNthMethodData
					#define SOMGD__somGetNthMethodData
				#else
					#define _somGetNthMethodData SOMClass_somGetNthMethodData
				#endif
			#endif /* SOMGD__somGetNthMethodData */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somGetNthMethodData */
	#ifndef SOMClass_somFindMethod
		#define SOMClass_somFindMethod(somSelf,methodId,m) \
			SOM_Resolve(somSelf,SOMClass,somFindMethod)  \
				(somSelf,methodId,m)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somFindMethod
				#if defined(_somFindMethod)
					#undef _somFindMethod
					#define SOMGD__somFindMethod
				#else
					#define _somFindMethod SOMClass_somFindMethod
				#endif
			#endif /* SOMGD__somFindMethod */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somFindMethod */
	#ifndef SOMClass_somFindMethodOk
		#define SOMClass_somFindMethodOk(somSelf,methodId,m) \
			SOM_Resolve(somSelf,SOMClass,somFindMethodOk)  \
				(somSelf,methodId,m)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somFindMethodOk
				#if defined(_somFindMethodOk)
					#undef _somFindMethodOk
					#define SOMGD__somFindMethodOk
				#else
					#define _somFindMethodOk SOMClass_somFindMethodOk
				#endif
			#endif /* SOMGD__somFindMethodOk */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somFindMethodOk */
	#ifndef SOMClass_somFindSMethod
		#define SOMClass_somFindSMethod(somSelf,methodId) \
			SOM_Resolve(somSelf,SOMClass,somFindSMethod)  \
				(somSelf,methodId)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somFindSMethod
				#if defined(_somFindSMethod)
					#undef _somFindSMethod
					#define SOMGD__somFindSMethod
				#else
					#define _somFindSMethod SOMClass_somFindSMethod
				#endif
			#endif /* SOMGD__somFindSMethod */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somFindSMethod */
	#ifndef SOMClass_somFindSMethodOk
		#define SOMClass_somFindSMethodOk(somSelf,methodId) \
			SOM_Resolve(somSelf,SOMClass,somFindSMethodOk)  \
				(somSelf,methodId)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somFindSMethodOk
				#if defined(_somFindSMethodOk)
					#undef _somFindSMethodOk
					#define SOMGD__somFindSMethodOk
				#else
					#define _somFindSMethodOk SOMClass_somFindSMethodOk
				#endif
			#endif /* SOMGD__somFindSMethodOk */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somFindSMethodOk */
	#ifndef SOMClass_somLookupMethod
		#define SOMClass_somLookupMethod(somSelf,methodId) \
			SOM_Resolve(somSelf,SOMClass,somLookupMethod)  \
				(somSelf,methodId)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somLookupMethod
				#if defined(_somLookupMethod)
					#undef _somLookupMethod
					#define SOMGD__somLookupMethod
				#else
					#define _somLookupMethod SOMClass_somLookupMethod
				#endif
			#endif /* SOMGD__somLookupMethod */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somLookupMethod */
	#ifndef SOMClass_somGetApplyStub
		#define SOMClass_somGetApplyStub(somSelf,methodId) \
			SOM_Resolve(somSelf,SOMClass,somGetApplyStub)  \
				(somSelf,methodId)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somGetApplyStub
				#if defined(_somGetApplyStub)
					#undef _somGetApplyStub
					#define SOMGD__somGetApplyStub
				#else
					#define _somGetApplyStub SOMClass_somGetApplyStub
				#endif
			#endif /* SOMGD__somGetApplyStub */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somGetApplyStub */
	#ifndef SOMClass_somGetPClsMtab
		#define SOMClass_somGetPClsMtab(somSelf) \
			SOM_Resolve(somSelf,SOMClass,somGetPClsMtab)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somGetPClsMtab
				#if defined(_somGetPClsMtab)
					#undef _somGetPClsMtab
					#define SOMGD__somGetPClsMtab
				#else
					#define _somGetPClsMtab SOMClass_somGetPClsMtab
				#endif
			#endif /* SOMGD__somGetPClsMtab */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somGetPClsMtab */
	#ifndef SOMClass_somCheckVersion
		#define SOMClass_somCheckVersion(somSelf,majorVersion,minorVersion) \
			SOM_Resolve(somSelf,SOMClass,somCheckVersion)  \
				(somSelf,majorVersion,minorVersion)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somCheckVersion
				#if defined(_somCheckVersion)
					#undef _somCheckVersion
					#define SOMGD__somCheckVersion
				#else
					#define _somCheckVersion SOMClass_somCheckVersion
				#endif
			#endif /* SOMGD__somCheckVersion */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somCheckVersion */
	#ifndef SOMClass_somDescendedFrom
		#define SOMClass_somDescendedFrom(somSelf,aClassObj) \
			SOM_Resolve(somSelf,SOMClass,somDescendedFrom)  \
				(somSelf,aClassObj)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somDescendedFrom
				#if defined(_somDescendedFrom)
					#undef _somDescendedFrom
					#define SOMGD__somDescendedFrom
				#else
					#define _somDescendedFrom SOMClass_somDescendedFrom
				#endif
			#endif /* SOMGD__somDescendedFrom */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somDescendedFrom */
	#ifndef SOMClass_somSupportsMethod
		#define SOMClass_somSupportsMethod(somSelf,mId) \
			SOM_Resolve(somSelf,SOMClass,somSupportsMethod)  \
				(somSelf,mId)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somSupportsMethod
				#if defined(_somSupportsMethod)
					#undef _somSupportsMethod
					#define SOMGD__somSupportsMethod
				#else
					#define _somSupportsMethod SOMClass_somSupportsMethod
				#endif
			#endif /* SOMGD__somSupportsMethod */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somSupportsMethod */
	#ifndef SOMClass_somDefinedMethod
		#define SOMClass_somDefinedMethod(somSelf,method) \
			SOM_Resolve(somSelf,SOMClass,somDefinedMethod)  \
				(somSelf,method)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somDefinedMethod
				#if defined(_somDefinedMethod)
					#undef _somDefinedMethod
					#define SOMGD__somDefinedMethod
				#else
					#define _somDefinedMethod SOMClass_somDefinedMethod
				#endif
			#endif /* SOMGD__somDefinedMethod */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somDefinedMethod */
	#ifndef SOMClass_somGetRdStub
		#define SOMClass_somGetRdStub(somSelf,methodId) \
			SOM_Resolve(somSelf,SOMClass,somGetRdStub)  \
				(somSelf,methodId)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somGetRdStub
				#if defined(_somGetRdStub)
					#undef _somGetRdStub
					#define SOMGD__somGetRdStub
				#else
					#define _somGetRdStub SOMClass_somGetRdStub
				#endif
			#endif /* SOMGD__somGetRdStub */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somGetRdStub */
	#ifndef SOMClass_somOverrideMtab
		#define SOMClass_somOverrideMtab(somSelf) \
			SOM_Resolve(somSelf,SOMClass,somOverrideMtab)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD__somOverrideMtab
				#if defined(_somOverrideMtab)
					#undef _somOverrideMtab
					#define SOMGD__somOverrideMtab
				#else
					#define _somOverrideMtab SOMClass_somOverrideMtab
				#endif
			#endif /* SOMGD__somOverrideMtab */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass_somOverrideMtab */
	#ifndef SOMClass__get_somClassAllocate
		#define SOMClass__get_somClassAllocate(somSelf) \
			SOM_Resolve(somSelf,SOMClass,_get_somClassAllocate)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somClassAllocate
				#if defined(__get_somClassAllocate)
					#undef __get_somClassAllocate
					#define SOMGD___get_somClassAllocate
				#else
					#define __get_somClassAllocate SOMClass__get_somClassAllocate
				#endif
			#endif /* SOMGD___get_somClassAllocate */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass__get_somClassAllocate */
	#ifndef SOMClass__get_somClassDeallocate
		#define SOMClass__get_somClassDeallocate(somSelf) \
			SOM_Resolve(somSelf,SOMClass,_get_somClassDeallocate)  \
				(somSelf)
		#ifndef SOM_DONT_USE_SHORT_NAMES
			#ifndef SOMGD___get_somClassDeallocate
				#if defined(__get_somClassDeallocate)
					#undef __get_somClassDeallocate
					#define SOMGD___get_somClassDeallocate
				#else
					#define __get_somClassDeallocate SOMClass__get_somClassDeallocate
				#endif
			#endif /* SOMGD___get_somClassDeallocate */
		#endif /* SOM_DONT_USE_SHORT_NAMES */
	#endif /* SOMClass__get_somClassDeallocate */

#ifndef SOM_DONT_USE_INHERITED_MACROS
#define SOMClass_somInit SOMObject_somInit
#define SOMClass_somUninit SOMObject_somUninit
#define SOMClass_somFree SOMObject_somFree
#define SOMClass_somGetClass SOMObject_somGetClass
#define SOMClass_somGetClassName SOMObject_somGetClassName
#define SOMClass_somGetSize SOMObject_somGetSize
#define SOMClass_somIsA SOMObject_somIsA
#define SOMClass_somIsInstanceOf SOMObject_somIsInstanceOf
#define SOMClass_somRespondsTo SOMObject_somRespondsTo
#define SOMClass_somDispatch SOMObject_somDispatch
#define SOMClass_somClassDispatch SOMObject_somClassDispatch
#define SOMClass_somCastObj SOMObject_somCastObj
#define SOMClass_somResetObj SOMObject_somResetObj
#define SOMClass_somPrintSelf SOMObject_somPrintSelf
#define SOMClass_somDumpSelf SOMObject_somDumpSelf
#define SOMClass_somDumpSelfInt SOMObject_somDumpSelfInt
#define SOMClass_somDefaultInit SOMObject_somDefaultInit
#define SOMClass_somDefaultCopyInit SOMObject_somDefaultCopyInit
#define SOMClass_somDefaultConstCopyInit SOMObject_somDefaultConstCopyInit
#define SOMClass_somDefaultVCopyInit SOMObject_somDefaultVCopyInit
#define SOMClass_somDefaultConstVCopyInit SOMObject_somDefaultConstVCopyInit
#define SOMClass_somDefaultAssign SOMObject_somDefaultAssign
#define SOMClass_somDefaultConstAssign SOMObject_somDefaultConstAssign
#define SOMClass_somDefaultVAssign SOMObject_somDefaultVAssign
#define SOMClass_somDefaultConstVAssign SOMObject_somDefaultConstVAssign
#define SOMClass_somDestruct SOMObject_somDestruct
#endif /* SOM_DONT_USE_INHERITED_MACROS */

#endif /* SOM_Module_somcls_Header_h */
