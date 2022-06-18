/**************************************************************************
 *
 *  Copyright 2008, Roger Brown
 *
 *  This file is part of Roger Brown's Toolkit.
 *
 *  This program is free software: you can redistribute it and/or modify it
 *  under the terms of the GNU Lesser General Public License as published by the
 *  Free Software Foundation, either version 3 of the License, or (at your
 *  option) any later version.
 * 
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 */

/*
 * $Id$
 */

 /*********************************
 * api for som library
 */

#ifndef somapi_h
#define somapi_h

typedef octet *somBooleanVector;

#ifdef __cplusplus
extern "C" {
#endif

#ifndef SOM_CLASSINFO_DEFINED
	typedef somToken somClassInfo;
#endif /* SOM_CLASSINFO_DEFINED */

#ifndef SOM_MTAB_DEFINED
	typedef struct 
	{
		SOMClass     SOMSTAR *copp; 
		long         cnt;    
		long         offset; 
	} somEmbeddedObjStruct;

#ifndef SOM_MDT_DEFINED
	typedef somToken somMToken;
	typedef somToken somDToken;
#endif /* SOM_MDT_DEFINED */

	typedef struct somMethodTabStruct 
	{
		SOMClass        SOMSTAR  classObject;
		somClassInfo			 *classInfo;
		char					 *className;
		long					 instanceSize;
		long					 dataAlignment;
		long					 mtabSize;
		long                     protectedDataOffset;
		somDToken                protectedDataToken;
		somEmbeddedObjStruct     *embeddedObjs;

		somMethodPtr entries[1];
	} somMethodTab, *somMethodTabPtr;
#endif /*SOM_MTAB_DEFINED */

	typedef struct {
		SOMClass SOMSTAR classObject; 
		somToken tokens[1];    
	} somClassDataStructure, *somClassDataStructurePtr;

	#ifndef SOM_SMD_DEFINED
		typedef somToken somSharedMethodData;
	#endif /* SOM_SMD_DEFINED */

	typedef struct
	{
		SOMClass SOMSTAR cls;
		somMethodPtr defaultAssign;
		somMethodPtr defaultConstAssign; /* used by somDefaultConstAssign */
		somMethodPtr defaultNCArgAssign; /* used by somDefaultAssign */
		somMethodPtr udaAssign;
		somMethodPtr udaConstAssign;
		long dataOffset;
		/* somDefaultVAssign uses somClassResolve(parent) */
		/* somDefaultConstVAssign uses somClassResolve(parent) */
	} somAssignInfo;

	typedef struct
	{
		SOMClass SOMSTAR cls;
		somMethodPtr defaultInit;		/* used by somDefaultInit */
		somMethodPtr defaultCopyInit;
		somMethodPtr defaultConstCopyInit; /* used by somDefaultConstCopyInit */
		somMethodPtr defaultNCArgCopyInit; /* used by somDefaultCopyInit */
		long dataOffset;
		somMethodPtr legacyInit;		/* somInit */
		/* somDefaultVCopyInit uses somClassResolve(parent,SOMObjectClassData.somDefaultVCopyInit) .... */
		/* somDefaultConstVCopyInit uses somClassResolve(parent,SOMObjectClassData.somDefaultConstVCopyInit) .... */
	} somInitInfo;

	typedef struct
	{
		SOMClass SOMSTAR cls;
		somMethodPtr defaultDestruct;
		long dataOffset;
		somMethodPtr legacyUninit;
	} somDestructInfo;

	#ifndef SOM_CTI_DEFINED
	typedef somToken somCtrlInfo;
	#endif

	typedef struct
	{
		somBooleanVector mask;
		somInitInfo      *info;
		int				 infoSize;
		somCtrlInfo      ctrlInfo;
	} somInitCtrl;

	typedef struct
	{
		somBooleanVector mask;
		somDestructInfo *info;
		int				infoSize;
		somCtrlInfo		ctrlInfo;
	} somDestructCtrl;

	typedef struct
	{
		somBooleanVector mask;
		somAssignInfo   *info;
		int             infoSize;
		somCtrlInfo		ctrlInfo;
	} somAssignCtrl;
		
	typedef struct somMethodDataStruct 
	{
		somId id;
		long type;          
		somId descriptor;   
		somMToken mToken;   
		somMethodPtr method;
		somSharedMethodData *shared;
	} somMethodData, *somMethodDataPtr;

	typedef unsigned long somRdAppType;
	typedef unsigned long somFloatMap[13];
	typedef struct somMethodInfoStruct
	{
		somRdAppType callType;
		long va_listSize;
		somFloatMap *float_map;
	} somMethodInfo;

	#define SOMRdRetsimple			0
	#define SOMRdRetfloat			2
	#define SOMRdRetdouble			4
	#define SOMRdRetlongdouble		6
	#define SOMRdRetaggregate		8
	#define SOMRdRetbyte			10
	#define SOMRdRethalf			12
	#if defined(_WIN32)
	#define SOMRdRetsmallaggregate	14
	#endif
	#define SOMRdNoFloatArgs		0
	#define SOMRdFloatArgs			1
	#define SOMFMSingle				1
	#define SOMFMLast				2

	typedef struct
	{
		somMethodPtr rdStub;
		somMethodPtr apStub;
		somMethodInfo *stubInfo;
	} somApRdInfo;

	typedef struct somMethodStruct {
		unsigned long type;
		somMToken *classData;
		somId *methodId;
		somId *methodDescriptor;
		somMethodProc *method;
		somMethodProc *redispatchStub;
		somMethodProc *applyStub;
	} somMethods_t;

	typedef const char *constPtr;
	typedef const constPtr * somConstId;

	typedef struct somStaticMethodStruct {
		somMToken *classData;
		const somConstId *methodId; 
		const somConstId *methodDescriptor;
		somMethodProc *method;
		somMethodProc *redispatchStub;
		somMethodProc *applyStub;
	} somStaticMethod_t;

	typedef struct somOverideMethodStruct {
		const somConstId *methodId; 
		somMethodProc *method;
	} somOverrideMethod_t;

	typedef struct somInheritedMethodStruct {
		const somId     *methodId;  
		long parentNum;       
		somMToken *mToken;    
	} somInheritedMethod_t;

	typedef struct somMigratedMethodStruct
	{
		somMToken *clsMToken;
		somMToken *destMToken;
	} somMigratedMethod_t;

	typedef struct somNonInternalDataStruct {
		somDToken *classData;
		char *basisForDataOffset;
	} somNonInternalData_t;

	typedef struct somProcMethodsStruct {
		somMethodProc **classData, *pEntry;
	} somProcMethods_t;

	typedef struct somMethodTabList 
	{
		somMethodTab             *mtab;
		struct somMethodTabList  *next;
	} somMethodTabList, *somMethodTabs;

	/* note how this struct effectively starts as a somMethodTabList */
	typedef struct {
		somMethodTab    *mtab;
		somMethodTabs   next;
		SOMClass        SOMSTAR classObject; /* changes to struct SOM_CIB *cib in 3.0 */
		void (SOMLINK * somRenewNoInitNoZeroThunk)(void *);
		long            instanceSize; 
		somMethodPtr    *initializers;		/* aka resolvedInits, in releaseorder */
		somMethodPtr    *resolvedMToken;	/* actually this points to the ClassData structure */
		somInitCtrl     initCtrl;
		somDestructCtrl destructCtrl;
		somAssignCtrl   assignCtrl;
		/* beyond here, SOM 3.0 various significantly */
		long            embeddedTotalCount;
		long			hierarchyTotalCount;
		long			unused;
	} somParentMtabStruct, *somParentMtabStructPtr;

	typedef struct {
		somParentMtabStructPtr parentMtab;
		somDToken              instanceDataToken;
		somMethodProc          *wrappers[1]; 
	} somCClassDataStructure, *somCClassDataStructurePtr;

	typedef struct somVarargsFuncsStruct {
		somMethodProc **classData, *vEntry;
	} somVarargsFuncs_t;

	typedef struct
	{
		int version;
		long instanceDataSize;
		long dataAlignment;
		somEmbeddedObjStruct *embeddedObjs;
	} somDynamicSCI;


	typedef struct somStaticClassInfoStruct {
		unsigned long layoutVersion;  /* this struct defines layout version SOM_SCILEVEL */
		unsigned long numStaticMethods;   /* count of smt entries */
		unsigned long numStaticOverrides; /* count of omt entries */
		unsigned long numNonInternalData; /* count of nit entries */
		unsigned long numProcMethods;     /* count of pmt entries */
		unsigned long numVarargsFuncs;    /* count of vft entries */
		unsigned long majorVersion;
		unsigned long minorVersion;
		unsigned long instanceDataSize;   /* instance data introduced by this class */
		unsigned long maxMethods;         /* count numStaticMethods and numMethods */
		unsigned long numParents;
		somConstId    classId;
		somConstId    explicitMetaId;
		long implicitParentMeta;
		const somConstId		*parents;
		somClassDataStructure	*cds;
		somCClassDataStructure	*ccds;
		somStaticMethod_t		*smt;
		somOverrideMethod_t		*omt;
		char					*nitReferenceBase;
		somNonInternalData_t	*nit;
		somProcMethods_t		*pmt;
		somVarargsFuncs_t		*vft;
		somTP_somClassInitFunc  *cif;
		/* -- VERSION 2 -- */
		long dataAlignment;
		/* none of the following are supported, 11-June-2005 .... */
		/* -- VERSION 3 -- */
	#define SOMSCIVERSION       1
		long numDirectInitClasses;
		somId *directInitClasses;
		unsigned long numMethods;
		somMethods_t *mt;
		unsigned long protectedDataOffset;
		unsigned long somSCIVersion;
		unsigned long numInheritedMethods;
		somInheritedMethod_t *imt;
		/* but I don't know what a 
			classDataEntryNames are for,
			guess is the names used in the XXXClassDataStructure */
		unsigned long numClassDataEntries; /* should always be filled in?? */
		somId *classDataEntryNames;
		unsigned long numMigratedMethods;
		somMigratedMethod_t *mmt;
		unsigned long numInitializers;
		somId *initializers;
		unsigned long somDTSClass;
		somDynamicSCI *dcsi;

	} somStaticClassInfo, *somStaticClassInfoPtr;

	#define SOM_GetClass(obj) (**(SOMClass SOMSTAR **)obj)

	typedef void    SOMLINK somTD_classInitRoutine(SOMClass SOMSTAR, SOMClass SOMSTAR);
	typedef int     SOMLINK somTD_SOMOutCharRoutine(char);
	typedef int     SOMLINK somTD_SOMLoadModule(char *className,char *fileName,char *functionName,long majorVersion,long minorVersion,somToken * modHandle);
	typedef int     SOMLINK somTD_SOMDeleteModule(somToken modHandle);
	typedef char *  SOMLINK somTD_SOMClassInitFuncName(void);
	typedef unsigned long SOMLINK somTD_SOMCreateMutexSem(somToken *sem);
	typedef unsigned long SOMLINK somTD_SOMRequestMutexSem(somToken sem);
	typedef unsigned long SOMLINK somTD_SOMReleaseMutexSem(somToken sem);
	typedef unsigned long SOMLINK somTD_SOMDestroyMutexSem(somToken sem);
	typedef unsigned long SOMLINK somTD_SOMGetThreadId (void);
	typedef void		SOMLINK somTD_SOMError(int code,const char * fileName,int lineNum);
	typedef somToken    SOMLINK somTD_SOMMalloc(size_t nbytes);
	typedef somToken    SOMLINK somTD_SOMCalloc(size_t element_count,size_t element_size);
	typedef somToken    SOMLINK somTD_SOMRealloc(somToken memory,size_t nbytes);
	typedef void		SOMLINK somTD_SOMFree(somToken memory);



#ifdef __cplusplus
}
#endif /* __cplusplus */


#ifndef SOM_IMPORTEXPORT_som
	#ifdef SOMDLLIMPORT
		#define SOM_IMPORTEXPORT_som SOMDLLIMPORT
	#else
		#define SOM_IMPORTEXPORT_som 
	#endif
#endif /* SOM_IMPORTEXPORT_som */


#ifdef SOM_RESOLVE_DATA
	#define EXPORT_GLOBAL_DATA(x,y)   SOMEXTERN SOM_IMPORTEXPORT_som x * SOMLINK resolve_##y(void);
#else
	#define EXPORT_GLOBAL_DATA(x,y)   SOMEXTERN SOM_IMPORTEXPORT_som x SOMDLINK y;
#endif

	EXPORT_GLOBAL_DATA(somTD_SOMMalloc *,SOMMalloc)
	EXPORT_GLOBAL_DATA(somTD_SOMCalloc *,SOMCalloc)
	EXPORT_GLOBAL_DATA(somTD_SOMFree *,SOMFree)
	EXPORT_GLOBAL_DATA(somTD_SOMRealloc *,SOMRealloc)
	EXPORT_GLOBAL_DATA(somTD_SOMCreateMutexSem *,SOMCreateMutexSem)
	EXPORT_GLOBAL_DATA(somTD_SOMRequestMutexSem *,SOMRequestMutexSem)
	EXPORT_GLOBAL_DATA(somTD_SOMReleaseMutexSem *,SOMReleaseMutexSem)
	EXPORT_GLOBAL_DATA(somTD_SOMDestroyMutexSem *,SOMDestroyMutexSem)
 	EXPORT_GLOBAL_DATA(SOMClassMgr SOMSTAR,SOMClassMgrObject)
	EXPORT_GLOBAL_DATA(somTD_SOMGetThreadId *,SOMGetThreadId)
	EXPORT_GLOBAL_DATA(somTD_SOMLoadModule *,SOMLoadModule)
	EXPORT_GLOBAL_DATA(somTD_SOMDeleteModule *,SOMDeleteModule)
	EXPORT_GLOBAL_DATA(somTD_SOMClassInitFuncName *,SOMClassInitFuncName)
	EXPORT_GLOBAL_DATA(int,SOM_TraceLevel)
	EXPORT_GLOBAL_DATA(int,SOM_AssertLevel)
	EXPORT_GLOBAL_DATA(int,SOM_WarnLevel)
	EXPORT_GLOBAL_DATA(long,SOM_MajorVersion)
	EXPORT_GLOBAL_DATA(long,SOM_MinorVersion)
	EXPORT_GLOBAL_DATA(long,SOM_MaxThreads)
	EXPORT_GLOBAL_DATA(long,SOM_IdTableSize)
	EXPORT_GLOBAL_DATA(somToken,SOM_IdTable)
	EXPORT_GLOBAL_DATA(somTD_SOMOutCharRoutine *,SOMOutCharRoutine)
	EXPORT_GLOBAL_DATA(somTD_SOMError *,SOMError)

#undef EXPORT_GLOBAL_DATA

#if defined(SOM_RESOLVE_DATA) && !defined(BUILD_SOM)
	#define SOMMalloc			(*(resolve_SOMMalloc()))
	#define SOMCalloc			(*(resolve_SOMCalloc()))
	#define SOMFree				(*(resolve_SOMFree()))
	#define SOMError			(*(resolve_SOMError()))
	#define SOMRealloc			(*(resolve_SOMRealloc()))
	#define SOMClassMgrObject	(*(resolve_SOMClassMgrObject()))
	#define SOMOutCharRoutine	(*(resolve_SOMOutCharRoutine()))
#endif

#if defined(BUILD_SOM) 
SOMEXTERN SOM_IMPORTEXPORT_som void SOMLINK somStartCriticalSection(void);
SOMEXTERN SOM_IMPORTEXPORT_som void SOMLINK somEndCriticalSection(void);
#endif

SOMEXTERN SOM_IMPORTEXPORT_som int SOMLINK somPrintf(const char * fmt, ...);
SOMEXTERN SOM_IMPORTEXPORT_som int SOMLINK somVprintf(const char * fmt, va_list ap);
SOMEXTERN SOM_IMPORTEXPORT_som void SOMLINK somPrefixLevel(long level);
SOMEXTERN SOM_IMPORTEXPORT_som int SOMLINK somLPrintf(int level, char * fmt, ...);
SOMEXTERN SOM_IMPORTEXPORT_som somMethodProc * SOMLINK somClassResolve(SOMClass SOMSTAR obj, somMToken mdata);
SOMEXTERN SOM_IMPORTEXPORT_som somMethodProc * SOMLINK somResolve(SOMObject SOMSTAR obj,somMToken mdata);
SOMEXTERN SOM_IMPORTEXPORT_som somMethodProc * SOMLINK somResolveByName(SOMObject SOMSTAR obj,char *methodName);
SOMEXTERN SOM_IMPORTEXPORT_som somMethodProc * SOMLINK somParentNumResolve(somMethodTabs parentMtabs,int parentNum,somMToken mToken);
SOMEXTERN SOM_IMPORTEXPORT_som somToken SOMLINK somDataResolve(SOMObject SOMSTAR obj, somDToken dataId);
SOMEXTERN SOM_IMPORTEXPORT_som boolean SOMLINK somApply(SOMObject SOMSTAR somSelf,somToken *retVal,somMethodDataPtr mdPtr,va_list ap);
SOMEXTERN SOM_IMPORTEXPORT_som int SOMLINK somCompareIds(somId id1, somId id2);
SOMEXTERN SOM_IMPORTEXPORT_som char * SOMLINK somStringFromId (somId id);
SOMEXTERN SOM_IMPORTEXPORT_som somId SOMLINK somIdFromString(char *);
SOMEXTERN SOM_IMPORTEXPORT_som void SOMLINK somCheckArgs(int argc, char **argv);
SOMEXTERN SOM_IMPORTEXPORT_som SOMClass SOMSTAR SOMLINK somBuildClass(long inherit_vars,somStaticClassInfo *sci,long majorVersion,long minorVersion);
SOMEXTERN SOM_IMPORTEXPORT_som SOMObject SOMSTAR SOMLINK somTestCls(SOMObject SOMSTAR obj,SOMClass SOMSTAR classObj,const char * fileName,int lineNumber);
SOMEXTERN SOM_IMPORTEXPORT_som void SOMLINK somAssert(int condition, int ecode,const char * fileName, int lineNum,const char * msg);
SOMEXTERN SOM_IMPORTEXPORT_som void SOMLINK somTest(int condition, int severity,const char * fileName,int lineNum, char * msg);
SOMEXTERN SOM_IMPORTEXPORT_som void SOMLINK somExceptionFree(Environment *ev);
SOMEXTERN SOM_IMPORTEXPORT_som void SOMLINK somSetException(Environment *ev,exception_type major,const char *exception_name,void *params);
SOMEXTERN SOM_IMPORTEXPORT_som char * SOMLINK somExceptionId(Environment *ev);
SOMEXTERN SOM_IMPORTEXPORT_som void * SOMLINK somExceptionValue(Environment *ev);
SOMEXTERN SOM_IMPORTEXPORT_som void SOMLINK somSetOutChar(somTD_SOMOutCharRoutine *outch);
SOMEXTERN SOM_IMPORTEXPORT_som SOMClassMgr SOMSTAR SOMLINK somMainProgram(void);
SOMEXTERN SOM_IMPORTEXPORT_som SOMClassMgr SOMSTAR SOMLINK somEnvironmentNew(void);
SOMEXTERN SOM_IMPORTEXPORT_som Environment * SOMLINK somGetGlobalEnvironment(void);
SOMEXTERN SOM_IMPORTEXPORT_som void SOMLINK somEnvironmentEnd(void);
SOMEXTERN SOM_IMPORTEXPORT_som boolean SOMLINK somIsObj(somToken obj);
SOMEXTERN SOM_IMPORTEXPORT_som SOMClass SOMSTAR SOMLINK somGetClassFromMToken(somMToken mToken);

#endif /* somapi_h */
