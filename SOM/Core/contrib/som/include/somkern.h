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

#ifndef SOMKERN_H
#define SOMKERN_H
/* internals for the data structures used by the APIs */

#include <rhbopt.h>

#define SOM_DLL_som
#define SOM_Module_somcm_Source
#define SOM_Module_somcls_Source
#define SOM_Module_somobj_Source

#ifndef SOM_MDT_DEFINED
	#define SOM_MDT_DEFINED
	typedef struct somMTokenData * somMToken;
	typedef struct somDTokenData * somDToken;
#endif

#ifndef SOM_CLASSINFO_DEFINED
	#define SOM_CLASSINFO_DEFINED
	typedef struct somClassInfoData * somClassInfo;
#endif

#ifndef SOM_MTAB_DEFINED
	#define SOM_MTAB_DEFINED
	typedef struct somMethodTabStruct somMethodTab,*somMethodTabPtr;
	typedef struct {
		void /* SOMClass */     **copp; /* address of class object ptr */
		long         cnt;    /* object count */
		long         offset; /* offset to pointer */
	} somEmbeddedObjStruct, *somEmbeddedObjStructPtr;
#endif

#ifndef SOM_Scope
	#define SOM_Scope    static
#endif

#include <somplatf.h>

#ifdef SOMDLLEXPORT
	#define SOM_IMPORTEXPORT_som SOMDLLEXPORT
#else
	#define SOM_IMPORTEXPORT_som
#endif

#include <rhbsomex.h>

#ifdef _DEBUG
	#define SOM_DEBUG_MEMORY  
/*	#define SOM_DEBUG_MEMORY_SPRINTF*/
/*	#define USE_IS_OVERRIDDEN */
#endif

#include <rhbsomkd.h>

#ifdef _PLATFORM_MACINTOSH_
	#include <Processes.h>
	#ifdef USE_ASLM
		extern void somaslm_unloadall(void);
	#endif
#endif

typedef unsigned short SOMKERN_mtoken_index_t;
typedef unsigned long SOMKERN_data_index_t;

#define SOMKERN_offsetof(x,y)	((long)((char *)&(((x *)0)->y)))
#define SOMKERN_alignof(x)		(short)(long)(&((struct { octet _x; x _d; } *)NULL)->_d)

/* need to pack this as this is used to generate a assembler jump of the form:

		jmp dword ptr [address]

	in i386 code this would be 0xff,0x15,a,b,c,d but as it's a little-endian
	cpu and we are stuffing the first two bytes into a short we are
	using 0x15FF followed by address of a pointer to the resolve stub
	a different stub is used for aggregates where the size is > 8
	the goal is to get the C compiler to make all size/alignment decisions
	rather than the IDL compiler, eg, the compiler knows nothing about
	the size of elements in struct, union, sequence or any
	*/

#ifdef SOM_DATA_THUNKS
#pragma pack(push,1)
typedef struct 
{
	unsigned short jump_dr;
	somMethodPtr *address_dr;
} somkern_long_jump_indirect_data;
#pragma pack(pop)

	extern somMethodPtr somkern_jump_data;
	#if defined(_PLATFORM_LITTLE_ENDIAN_) && defined(_WIN32)
		/* intel i386 far indirect jump */
		#define somDToken_call_dword_ptr			0x15FF
	#else
		#error not built for thunking
	#endif
	#define somDToken_jump	{somDToken_call_dword_ptr,&somkern_jump_data},
	#define somthunk_jump_data_offset   6
#else
	#define somDToken_jump
#endif

#ifdef SOM_METHOD_THUNKS

#pragma pack(push,1)
typedef struct
{
	unsigned short jump_instr;
	somMethodPtr *jump_address;
} somkern_general_thunk;
typedef struct
{
	somkern_general_thunk jump_thunk;
	struct somMTokenData *mToken;
} somkern_resolve_thunk;
typedef struct 
{
	somkern_resolve_thunk resolve,direct;
#ifdef SOM_METHOD_STUBS
	somkern_general_thunk apply,redispatch;
#endif
} somkern_long_jump_indirect;
#pragma pack(pop)

#define somthunk_jump_ro_offset   6			/* jmp+address */

#ifdef SOM_METHOD_STUBS
	#define somthunk_jump_ap_offset   20+6 /* (jmp+address+ptr)*2+jmp+address */
	#define somthunk_jump_rd_offset   somthunk_jump_ap_offset+6	 /* (jmp+address+ptr)*2+jmp+address+jmp+address */
#endif

#if defined(_PLATFORM_LITTLE_ENDIAN_) && defined(_WIN32)
	/* intel i386 far indirect jump */
	#define somMToken_call_dword_ptr			0x15FF

	#define somMToken_jmp_dword_ptr				0x25FF
	#define somMToken_setjmp(x,y)			\
				((unsigned char *)x)[0]=0xE9; \
				((long *)(1+(char *)x))[0]=(((long)y)-(5+(long)x))
#else
	#error not built for thunking
#endif

#ifdef SOM_METHOD_STUBS
	#define somMToken_direct_jump(t,i)	{{somMToken_jmp_dword_ptr,&(t[i].defined.method)},t+i}
	#define somMToken_jump_root(t,i)	{{{somMToken_call_dword_ptr,&somkern_jump_root},t+i},\
										 somMToken_direct_jump(t,i),\
										{somMToken_call_dword_ptr,&somkern_jump_apply}, \
										{somMToken_call_dword_ptr,&somkern_jump_redispatch}},
	#define somMToken_jump(t,i)			{{{somMToken_call_dword_ptr,&somkern_jump_standard},t+i}, \
										 somMToken_direct_jump(t,i),\
										{somMToken_call_dword_ptr,&somkern_jump_apply}, \
										{somMToken_call_dword_ptr,&somkern_jump_redispatch}},
	#define somMToken_jump_aggregate(t,i,x)	{{{somMToken_call_dword_ptr, \
										(sizeof(x) > 8) ? &somkern_jump_aggregate : & somkern_jump_standard}, \
											t+i}, \
										 somMToken_direct_jump(t,i),\
										{somMToken_call_dword_ptr,&somkern_jump_apply}, \
										{somMToken_call_dword_ptr,&somkern_jump_redispatch}},
#else
	#define somMToken_jump_root			{somMToken_call_dword_ptr,&somkern_jump_root},
	#define somMToken_jump				{somMToken_call_dword_ptr,&somkern_jump_standard},
	#define somMToken_jump_aggregate(x)	{somMToken_call_dword_ptr, \
										(sizeof(x) > 8) ? &somkern_jump_aggregate : & somkern_jump_standard},
#endif

	#define somMToken_direct(t)			(somMToken)&((t)->jumper.direct)
	#define somMToken_override(t)		(somMToken)&((t)->jumper.resolve)

extern somMethodPtr somkern_jump_root,somkern_jump_standard,somkern_jump_aggregate,
				somkern_jump_apply,
				somkern_jump_redispatch;

	#define SOMMTOKEN_DATA(x)		((x)->jumper.resolve.mToken)
#else
	#define somMToken_jump(t,i)
	#define somMToken_jump_root(t,i)
	#define somMToken_jump_aggregate(t,i,x)
	#define somMToken_jump_raw(t,i)	
	#define SOMMTOKEN_DATA(x)		(x)
	#define somMToken_direct(t)		t
	#define somMToken_override(t)	t
#endif

struct somDTokenData
{
#ifdef SOM_DATA_THUNKS
	somkern_long_jump_indirect_data jump_data;
#endif
	struct somMethodTabStruct *cls;
	SOMKERN_data_index_t data_offset;
};

typedef struct somMTokenData
{
#ifdef SOM_METHOD_THUNKS
	somkern_long_jump_indirect jumper;
#endif
	/* somStaticMethod_t *defined;	*//* what it originally was */
	struct somMethodTabStruct *classInfoOwner;		/* who introduced it */
	struct
	{
		unsigned char type;
		somMToken *classData;
		somConstId somId_methodId;
		somConstId somId_methodDescriptor;
		somMethodPtr method;
		somMethodPtr redispatchStub;
		somMethodPtr applyStub;
	} defined;					/* copy of the method data in case of added methods */
	SOMKERN_mtoken_index_t index;			/* index into the class who introduced it */
	SOMKERN_mtoken_index_t index_total;		/* index from the start,
											works as long as not multiply
											inherited */
} somMTokenData;

struct somParentClassInfo
{
	struct somMethodTabStruct *cls;
	SOMKERN_data_index_t data_offset;
	unsigned long jump_table_offset;
	somMethodTabList list_entry;
	boolean inherit_var;            /* indicates if implementation is included */
	short destructor_usage;			/* used in a calculation for supporting
											somDestruct() */
};

struct somClassInfoData
{
	/* these are bits needed that don't fit into the method tab struct,
		and is normally appended to the method tab struct
		so it's lifetime is the same and no additional allocations 
		needed and has the mtabs classInfo pointer reference
		this so it can be found immediately following
		the jump table */

	/* this is at offset zero since it's commonly referenced */

	boolean multiple_inheritance;

	long major_version,minor_version;
	long instancePartSize,dataAlignment;
	unsigned long numParents;

	somStaticClassInfo *sci;
	struct somDTokenData data_token;
#if 0
	struct somMethodTabStruct *substituted_mtab;
#else
	void *unused_pointer;
	#define substituted_mtab	parents.mtab
#endif
	unsigned short max_mask_length;
	somTD_SOMObject_somUninit somUninit;

	/* the XXXXCClassData parentsMtab points here,
		the mtab gets swizzled during substitution
	*/
	somParentMtabStruct parents;

	struct
	{
		unsigned long _maximum;
		unsigned long _length;
/*		struct somJumpTableEntry *_buffer;*/
		/* this is always pointing to 'mtab->jump_table_buffer */
	} jump_table;

	struct
	{
		unsigned long _maximum;
		unsigned long _length;
		struct somMTokenData *_buffer;
	} added_methods;

	struct
	{
		unsigned long _maximum;
		unsigned long _length;
		struct somMethodTabStruct **_buffer;
	} parent_jump_table;

	struct 
	{
		unsigned long _maximum;
		unsigned long _length;
		struct somParentClassInfo *_buffer;
	} classes;

	RHBSOMUT_KeyData keyed_data;
};

struct somMethodTabStruct
{
    SOMClass        SOMSTAR classObject;
    somClassInfo    classInfo;
    char            *className;
    long            instanceSize;
    long            dataAlignment;
    long            mtabSize;
    long            protectedDataOffset; /* from class's introduced data */
    somDToken       protectedDataToken;
    somEmbeddedObjStruct *embeddedObjs;
	somMethodPtr entries[1];
};

/* there is more info required than that defined in the somMethodTabStruct
	so this is hidden in the classInfo struct which always precedes the mtab
    in this implementation */

struct somClassInfoMethodTab
{
	struct somClassInfoData classInfo;
	struct somMethodTabStruct mtab;
};

typedef struct somClassInfoMethodTab somClassInfoMethodTab;

#ifdef _DEBUG
	/* these are the first field in each struct confirming it has
		those fields */

	#define MUST_BE_CLASSINFO(x)        (&(x)->multiple_inheritance)
	#define MUST_BE_PARENTMTABS(x)		(&(x)->mtab)
	#define MUST_BE_MTAB(x)				(&(x)->classObject)
#else
	#define MUST_BE_CLASSINFO(x)        (x)
	#define MUST_BE_PARENTMTABS(x)		(x)
	#define MUST_BE_MTAB(x)				(x)
#endif

#define somClassInfoFromMtab(x)			((somClassInfo)(((char *)(MUST_BE_MTAB(x)))-(long)(&((struct somClassInfoMethodTab *)0)->mtab)))
#define somMtabFromClassInfo(x)			((somMethodTabPtr)(&(((struct somClassInfoMethodTab *)(MUST_BE_CLASSINFO(x)))->mtab)))
#define somClassInfoFromParentMtabs(x)	((somClassInfo)(((char *)(MUST_BE_PARENTMTABS(x)))-(long)(&((struct somClassInfoMethodTab *)0)->classInfo.parents)))

#define somMtabFromParentMtabs(x)		(somMtabFromClassInfo(somClassInfoFromParentMtabs(x)))

#define somMethodTabFromObject(o)		((o)->mtab)

#define SOMKERN_resolve_index_si(mtab,token)   (token->index_total)

#define SOMKERN_resolve_index(mtab,token) \
	( somClassInfoFromMtab(mtab)->multiple_inheritance ? \
		SOMKERN_resolve_index_mi(mtab,token) : \
		SOMKERN_resolve_index_si(mtab,token) )

#define SOMKERN_resolve(mtab,token)  \
	( (mtab)->entries[SOMKERN_resolve_index((mtab),token)] )

typedef struct som_globals_t
{
#ifdef USE_THREADS
	#ifdef USE_PTHREADS
		pthread_key_t tls_key;
	#else
		DWORD tls_key;
	#endif
#endif
	boolean dll_alive;
#ifdef RHBOPT_SHARED_DATA
	struct som_thread_globals_t *apps;
#else
	SOMObject SOMSTAR repository;
#endif
} som_globals_t;

typedef struct som_thread_globals_t
{
	Environment ev;
#ifdef RHBOPT_SHARED_DATA
	#ifdef _WIN32
		unsigned long tid;
	#else
		ProcessSerialNumber psn;
	#endif
	struct som_thread_globals_t *next;
	SOMObject SOMSTAR repository;
	somTD_SOMError *somError;
#endif
#if defined(RHBOPT_SHARED_DATA) || defined(USE_THREADS)
	somTD_SOMOutCharRoutine *somOutCharRoutine;
#endif
	unsigned short persistent_ids;
	char somLocateClassFile[260];
} som_thread_globals_t;

#ifdef USE_THREADS
	#define SOMKERN_guard        somStartCriticalSection();
	#define SOMKERN_unguard      somEndCriticalSection();
#else
	#define SOMKERN_guard
	#define SOMKERN_unguard
#endif

#define SOMKERN_guard_memory		SOMKERN_guard
#define SOMKERN_unguard_memory		SOMKERN_unguard

#ifdef _PLATFORM_MACINTOSH_
	#ifdef USE_ASLM
		struct somshlb_t *dlopen(char *,int);
		void *dlsym(struct somshlb_t *,const char *);
		void dlclose(struct somshlb_t *);
	#endif
#endif

#ifdef SOM_RESOLVE_DATA
	extern somTD_SOMCalloc				*SOMCalloc;
	extern somTD_SOMMalloc				*SOMMalloc;
	extern somTD_SOMRealloc				*SOMRealloc;
	extern somTD_SOMFree				*SOMFree;
	extern somTD_SOMCreateMutexSem		*SOMCreateMutexSem;
	extern somTD_SOMRequestMutexSem		*SOMRequestMutexSem;
	extern somTD_SOMReleaseMutexSem		*SOMReleaseMutexSem;
	extern somTD_SOMDestroyMutexSem		*SOMDestroyMutexSem;
	extern somTD_SOMGetThreadId			*SOMGetThreadId;
	extern somTD_SOMLoadModule			*SOMLoadModule;
	extern somTD_SOMDeleteModule		*SOMDeleteModule;
	extern somTD_SOMClassInitFuncName   *SOMClassInitFuncName;

	extern SOMClassMgr SOMSTAR SOMClassMgrObject;

	#ifdef RHBOPT_SHARED_DATA
		#define SOMError	 (SOMKERN_get_thread_globals(0)->somError)
	#else
		extern somTD_SOMError	  *SOMError;
	#endif
#endif

#include <somderr.h>
#include <stexcep.h>

extern SOMClassMgr SOMSTAR SOMClassMgrObject;

#ifdef SOM_DEBUG_MEMORY
extern void som_dump_mem(void);
#endif

#ifdef _WIN32
	#undef sprintf
	#undef vsprintf
	#define sprintf   do not use
	#define vsprintf   do not use
#endif

#ifdef NEED_SOMKERN_RESOLVE_INDEX_MI
static unsigned int SOMKERN_resolve_index_mi(somMethodTabPtr mtab,somMToken token)
{
	struct somMethodTabStruct *cls=token->classInfoOwner;
	struct somParentClassInfo *buf=somClassInfoFromMtab(mtab)->classes._buffer;

	while (cls != buf->cls)
	{
		/* find this class in the class list */

		buf++;
	}

	return token->index+buf->jump_table_offset;
}
#endif

SOMEXTERN void SOMLINK somFreeThreadData(void);

#endif /* SOMKERN_H */

