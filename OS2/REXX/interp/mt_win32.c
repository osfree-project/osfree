/* This is the file to support WIN32 threads.
 * We initialize the global data structure and the global access variable.
 */

#include "regina_c.h"
#include "rexxsaa.h"
#define DONT_TYPEDEF_PFN
#include "rexx.h"
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#define WIN32_LEAN_AND_MEAN
#ifdef _MSC_VER
# if _MSC_VER >= 1100
/* Stupid MSC can't compile own headers without warning at least in VC 5.0 */
#  pragma warning(disable: 4115 4201 4214 4514)
# endif
#endif
#include <windows.h>
#ifdef _MSC_VER
# if _MSC_VER >= 1100
#  pragma warning(default: 4115 4201 4214)
# endif
#endif

typedef struct { /* mt_tsd: static variables of this module (thread-safe) */
   HANDLE Heap;
} mt_tsd_t; /* thread-specific but only needed by this module. see
             * ReginaInitializeThread
             */

static DWORD ThreadIndex = 0xFFFFFFFF; /* index of the TSD, not yet got */

/* We use only one critical section for all purposes. That's enough since
 * we use it very rarely.
 */
static CRITICAL_SECTION cs = {0,};

#if defined(DYNAMIC) || (defined(__MINGW32__) && (__GNUC__ > 3 || (__GNUC__ == 3 && __GNUC_MINOR__ >= 4)))
static void DestroyHeap(tsd_t *TSD)
{
   mt_tsd_t *mt = TSD->mt_tsd;

   if (mt == NULL)
      return;
   if (mt->Heap != (HANDLE) 0)
      HeapDestroy(mt->Heap);
   free(mt);
   free(TSD);
}

int IfcReginaCleanup( VOID )
{
   tsd_t *TSD = __regina_get_tsd();

   if (TSD == NULL)
      return 0;

   deinit_rexxsaa(TSD);
   DestroyHeap(TSD);
   TlsSetValue(ThreadIndex,NULL);

   return 1;
}
#endif

#ifdef DYNAMIC
#define AcquireCriticalSection(cs) EnterCriticalSection(cs)
#define AcquireThreadIndex() ThreadIndex

/* We provide a DLL entry function. Look at the standard documentation */
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD Reason, LPVOID reserved)
{
   tsd_t *TSD;

   switch (Reason) {
      case DLL_PROCESS_ATTACH:
         /* Two things to do */
         InitializeCriticalSection(&cs);
         ThreadIndex = TlsAlloc();
         if (ThreadIndex == 0xFFFFFFFF)
            return FALSE; /* FIXME, FGC: Do we have to call SetLastError()? */
         break;
      case DLL_PROCESS_DETACH:
         break;
      case DLL_THREAD_ATTACH:
         break;
      case DLL_THREAD_DETACH:
         TSD = __regina_get_tsd();
         if (TSD != NULL)
         {
            deinit_rexxsaa(TSD);
            DestroyHeap(TSD);
         }
         break;
   }
   return(TRUE);
}
#else
/* The DLL will do this work in DllMain above. This is for a MT library: */

/* AcquireCriticalSection locks the given critical section and
 * initializes it on the first use.
 */
static void AcquireCriticalSection(CRITICAL_SECTION *cs)
{
   /* double initializing the critical section won't produce an error.
    * We must do this on thread attachment if an error occurs in later
    * implementations of WIN32.
    */
   InitializeCriticalSection(cs);

   EnterCriticalSection(cs);
}

/* AcquireThreadIndex returns a valid ThreadIndex. */
static DWORD AcquireThreadIndex(void)
{
   if (ThreadIndex == 0xFFFFFFFF)
   {  /* get a unique access variable for the whole process */
      AcquireCriticalSection(&cs);
      if (ThreadIndex == 0xFFFFFFFF) /* may've changed just before Acquire */
         ThreadIndex = TlsAlloc();
      LeaveCriticalSection(&cs);
      /* give back a possible error value. nothing will help at this point */
   }
   return(ThreadIndex);
}
#endif

/* This should prevent some error messages and is used as a #define */
static unsigned sizeof_ptr(void)
{
   return(sizeof(void *));
}

/* Lowest level memory allocation function for normal circumstances. */
static void *MTMalloc( const tsd_t *TSD, size_t size )
{
   mt_tsd_t *mt = TSD->mt_tsd;

   if (mt == NULL)
      return(NULL); /* Let it die */

   return(HeapAlloc(mt->Heap,HEAP_NO_SERIALIZE,size));
}

/* Lowest level memory deallocation function for normal circumstances. */
static void MTFree( const tsd_t *TSD, void *chunk )
{
   mt_tsd_t *mt = TSD->mt_tsd;

   /*
    * Just in case...
    */
   if ( chunk == NULL)
      return;

   if (mt == NULL)
      return; /* ??? */

   HeapFree(mt->Heap,HEAP_NO_SERIALIZE,chunk);
}

/* Lowest level exit handler. */
static void MTExit(int code)
{
   ExitThread(code);
}

/* ReginaInitializeThread creates a new thread structure and returns a ptr
 * to the initialized value.
 * The function may be called more than once.
 */
tsd_t *ReginaInitializeThread(void)
{
   int OK;
   DWORD idx;
   tsd_t *retval;
   mt_tsd_t *mt;

   /* If you run into trouble here, you must change the code in
    * ReginsSetMutex/ReginaUnsetMutex. The argument there assumes the
    * following rule. This is an ugly hack.
    */
   assert(sizeof_ptr() >= sizeof(HANDLE));
   if (sizeof_ptr() < sizeof(HANDLE))
      return(NULL); /* Be absolutely sure that we HAVE a problem */

   idx = AcquireThreadIndex();

   /* fetch the value of the access variable */
   retval = TlsGetValue(idx);

   if (retval != NULL) /* already initialized? */
      return(retval);

   /* First call in this thread... */
   retval = malloc(sizeof(tsd_t)); /* no Malloc, etc! */

   if (retval == NULL) /* THIS is really a problem. I don't know what we */
      return(NULL);    /* should do now. Let the caller run into a crash... */

   TlsSetValue(idx,retval);

   memset(retval,0,sizeof(tsd_t));
   retval->MTMalloc = MTMalloc;
   retval->MTFree = MTFree;
   retval->MTExit = MTExit;

   /* Since the local data structure contains a Heap object for the memory
    * management we initialize it first.
    */
   if ((mt = malloc(sizeof(mt_tsd_t))) == NULL)
      return(NULL);                     /* This is a catastrophy             */
   retval->mt_tsd = mt;
   memset(mt,0,sizeof(mt_tsd_t));
   if ((mt->Heap = HeapCreate(HEAP_NO_SERIALIZE,0x10000,0)) == NULL)
      return(NULL);                     /* This is a catastrophy             */

   OK = init_memory(retval);            /* Initialize the memory module FIRST*/

   /* Without the initial memory we don't have ANY chance! */
   if (!OK)
      return(NULL);

   {
      extern OS_Dep_funcs __regina_OS_Win;
      retval->OS = &__regina_OS_Win;
   }
   retval->OS->init();
   OK &= init_vars(retval);             /* Initialize the variable module    */
   OK &= init_stacks(retval);           /* Initialize the stack module       */
   OK &= init_filetable(retval);        /* Initialize the files module       */
   OK &= init_math(retval);             /* Initialize the math module        */
   OK &= init_spec_vars(retval);        /* Initialize the interprt module    */
   OK &= init_tracing(retval);          /* Initialize the tracing module     */
   OK &= init_builtin(retval);          /* Initialize the builtin module     */
   OK &= init_client(retval);           /* Initialize the client module      */
   OK &= init_library(retval);          /* Initialize the library module     */
   OK &= init_rexxsaa(retval);          /* Initialize the rexxsaa module     */
   OK &= init_shell(retval);            /* Initialize the shell module       */
   OK &= init_envir(retval);            /* Initialize the envir module       */
   OK &= init_expr(retval);             /* Initialize the expr module        */
   OK &= init_error(retval);            /* Initialize the error module       */
#ifdef VMS
   OK &= init_vms(retval);              /* Initialize the vmscmd module      */
   OK &= init_vmf(retval);              /* Initialize the vmsfuncs module    */
#endif
   OK &= init_arexxf(retval);           /* Initialize the arxfuncs modules */
   retval->loopcnt = 1;                 /* stupid r2perl-module              */
   retval->traceparse = -1;
   retval->thread_id = (unsigned long)GetCurrentThreadId();

   if (!OK)
      exiterror( ERR_STORAGE_EXHAUSTED, 0 ) ;

   return(retval);
}

/* __regina_get_tsd returns a pointer to the thread specific data. Be sure to
 * calls this after a ReginaInitializeThread only.
 */
tsd_t *__regina_get_tsd(void)
{
   /* See above for comments */
   return(TlsGetValue(ThreadIndex));
}

/* ReginaSetMutex is the opposite of ReginaUnsetMutex and sets a mutex
 * variable. The "true" mutex is "*arg" since we have hidden the type
 * HANDLE which is the correct type. Thus, we have used "HANDLE" and
 * "void *" in the same manner. If we include windows.h for the
 * definition of HANDLE we cant include windows later and may run
 * into trouble. The initialization code will check of errors of
 * this assumption.
 * The argument (*mutex) may be NULL. We initialize the mutex in this
 * case. This prevents the calling functions to initialize the mutex.
 * The is a little speed penalty but the mutexes are not used very
 * often. YOU should change it if it hurts you.
 */
void ReginaSetMutex(void **mutex)
{
   int OK = 1;
   volatile HANDLE *w32_mutex = (volatile HANDLE *) mutex;

   if (*w32_mutex == (HANDLE) 0)
   {
      AcquireCriticalSection(&cs);
      if (*w32_mutex == (HANDLE)0) /* may have changed due MT */
      {
         *w32_mutex = CreateMutex(NULL,FALSE,NULL);
         if (*w32_mutex == NULL)
            OK = 0;
      }
      LeaveCriticalSection(&cs);
      if (!OK)
      { /* We must die now! There is no other chance. */
         *((int *) NULL) = 1;
      }
   }

   WaitForSingleObject(*w32_mutex,INFINITE);
   /* ignore errors, we continue especially if WAIT_ABANDONED occurs */
}

/* see ReginaSetMutex */
void ReginaUnsetMutex(void **mutex)
{
   volatile HANDLE *w32_mutex = (volatile HANDLE *) mutex;

   ReleaseMutex(*w32_mutex);
}
