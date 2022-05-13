/* This is the file to support OS/2 threads.
 * This file is under construction. We partially support EMX. The EMX
 * environment must currently be "OS2". Thus, usage is provided for
 * makefile.os2.emx. Others may run.
 * We initialize the global data structure and the global access variable.
 */

#define INCL_NOCOMMON
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_DOSERRORS
#define INCL_DOSMEMMGR

#ifdef __EMX__
# define DONT_TYPEDEF_PFN
# include <io.h>
# include <os2emx.h>
#elif defined (__WATCOMC__)
# define DONT_TYPEDEF_PFN
# include <os2.h>
#endif

#include "rexx.h"
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

static ULONG *TSD_ptrs = NULL;

typedef struct _MT_mem {
   struct _MT_mem *prev;
   struct _MT_mem *next;
   /* real data follows here */
} MT_mem;

typedef struct { /* mt_tsd: static variables of this module (thread-safe) */
   MT_mem *mem_base; /* From DosAllocMem, we use this memory scheme since it is
                      * native OS/2. emx has its own heap allocation functions.
                      */
} mt_tsd_t; /* thread-specific but only needed by this module. see
             * ReginaInitializeThread
             */

/* every time a Regina thread is initialised by ReginaInitializeThread, the TSD
 * for that thread is added to an empty slot in this array. When a thread
 * terminates and calls Deinitialize, the used slot is freed up
 */
tsd_t *tsds[MAX_CONCURRENT_REGINA_THREADS] = {0,};

/* Returns a pointer to the TSD pointer */
static tsd_t **FindThreadDataIdx(void)
{
   tsd_t **retval = NULL;
   ULONG rc;

   DosEnterCritSec();
   if (TSD_ptrs != NULL)
   {
      retval = (tsd_t **) TSD_ptrs;
      goto Leave;
   }
   if ((rc = DosAllocThreadLocalMemory(1,&TSD_ptrs)) != NO_ERROR)
   {
      fprintf(stderr,"rc = %lu from DosAllocThreadLocalMemory, await SIGSEGV!\n",
                     (unsigned long) rc);
      TSD_ptrs = NULL;
   }
   if (TSD_ptrs != NULL)
      retval = (tsd_t **) TSD_ptrs;
Leave:
   DosExitCritSec();
   return(retval);
}

/* Deinitialize is called when the thread terminates.
 * This is a wonderful position to place code which frees all allocated stuff!
 */
static void Deinitialize(void *buf)
{
   tsd_t *TSD = buf;
   mt_tsd_t *mt;
   MT_mem *chunk;

   if (TSD == NULL) /* Should never happen but be sure */
      return;

   deinit_rexxsaa(TSD);

   mt = TSD->mt_tsd;
   if (mt)
      while ((chunk = mt->mem_base) != NULL)
      {
         TSD->MTFree(TSD,chunk + 1);
         if (mt->mem_base == chunk)
            break; /* something goes wrong. Don't run into an endless loop */
      }
   if (mt)
      free(mt);
   free(TSD);
}

int IfcReginaCleanup( VOID )
{
   tsd_t **ptsd = FindThreadDataIdx();

   if (*ptsd == NULL)
      return 0;
   Deinitialize(*ptsd);
   *ptsd = NULL;

   return 1;
}

#ifdef DYNAMIC
# ifdef __EMX__
/* We provide a DLL entry function. Look at the standard documentation for
 * EMX. See emxdev.doc and testdll6.c. We may use the macros CRT_INIT1 and
 * CRT_TERM1 from sys/startup.h instead but that's undocumented stuff.
 */
int _CRT_init (void);
void _CRT_term (void);
void __ctordtorInit (void);
void __ctordtorTerm (void);

unsigned long _DLL_InitTerm (unsigned long mod_handle, unsigned long flag)
{
   switch (flag)
   {
      case 0:
         if (_CRT_init() != 0)
            return 0;
         __ctordtorInit();
         return 1;
      case 1:
         /*
          * This will run ONLY if called on dynamic unload of the complete
          * library. This means, the last thread will receive an unload
          * command. Stupid OS/2.
          */
         IfcReginaCleanup();
         __ctordtorTerm();
         _CRT_term();
         return 1;
      default:
         break;
   }
   return 0;
}
# endif /* EMX */
#endif /* DYNAMIC */

/* This should prevent some error messages and is used as a #define */
static unsigned sizeof_ptr(void)
{
   return(sizeof(void *));
}

/* Lowest level memory allocation function for normal circumstances. */
static void *MTMalloc( const tsd_t *TSD, size_t size )
{
   mt_tsd_t *mt;
   MT_mem *new;

   if (DosAllocMem((PPVOID) &new,
                   size + sizeof(MT_mem),
                   PAG_READ|PAG_WRITE|PAG_COMMIT) != NO_ERROR)
      return(NULL); /* may happen. errors are detected in the above layers */

   mt = TSD->mt_tsd;
   new->prev = NULL;
   new->next = mt->mem_base;
   if (mt->mem_base)
      mt->mem_base->prev = new;
   mt->mem_base = new;
   return(new + 1); /* jump over the head */
}

/* Lowest level memory deallocation function for normal circumstances. */
static void MTFree( const tsd_t *TSD, void *chunk )
{
   mt_tsd_t *mt = TSD->mt_tsd;
   MT_mem *this;

   /*
    * Just in case...
    */
   if (chunk == NULL)
      return;

   this = chunk;
   this--; /* Go to the header of the chunk */

   if (this->prev)
      if (this->prev->next != this)
         return;
   if (this->next)
      if (this->next->prev != this)
         return;

   /* This is a chunk allocated by MTMalloc */
   if (this->prev)
      this->prev->next = this->next;
   if (this->next)
      this->next->prev = this->prev;
   if (this == mt->mem_base)
      mt->mem_base = this->next;

   /* Last not least we set the pointers to NULL. This prevents a double-free*/
   this->next = NULL;
   this->prev = NULL;
   DosFreeMem(this);
}

/* Lowest level exit handler. */
static void MTExit(int code)
{
   DosExit(EXIT_THREAD,code);
}

/* ReginaInitializeThread creates a new thread structure and returns a ptr
 * to the initialized value.
 * The function may be called more than once.
 */
tsd_t *ReginaInitializeThread(void)
{
   int OK;
   tsd_t *retval,**ptsd;
   mt_tsd_t *mt;

   /* If you run into trouble here, you must change the code in
    * ReginsSetMutex/ReginaUnsetMutex. The argument there assumes the
    * following rule. This is an ugly hack.
    */
   assert(sizeof_ptr() >= sizeof(HMTX));
   if (sizeof_ptr() < sizeof(HMTX))
      return(NULL); /* Be absolutely sure that we HAVE a problem */

   ptsd = FindThreadDataIdx();
   if (ptsd == NULL) /* can't initialize? */
      return(NULL);

   if (*ptsd != NULL) /* already initialized? */
      return(*ptsd);

   /* First call in this thread, a atexit() per thread will be great, sigh...*/
   retval = malloc(sizeof(tsd_t)); /* no Malloc, etc! */

   if (retval == NULL) /* THIS is really a problem. I don't know what we */
      return(NULL);    /* should do now. Let the caller run into a crash... */

   *ptsd = retval;

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
   mt->mem_base = NULL;

   OK = init_memory(retval);            /* Initialize the memory module FIRST*/

   /* Without the initial memory we don't have ANY chance! */
   if (!OK)
      return(NULL);

   {
      extern OS_Dep_funcs __regina_OS_Os2;
      retval->OS = &__regina_OS_Os2;
   }

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
#ifdef __EMX__
   retval->thread_id = _gettid();
#elif defined(__WATCOMC__)
   retval->thread_id = *_threadid;
#else
   retval->thread_id = _gettid();
#endif

   if (!OK)
      exiterror( ERR_STORAGE_EXHAUSTED, 0 ) ;
   return(retval);
}

int __regina_get_number_concurrent_regina_threads(void)
{
   return MAX_CONCURRENT_REGINA_THREADS;
}

tsd_t *__regina_get_tsd_for_threadid( unsigned long threadid )
{
   int i;
   for ( i = 0; i < MAX_CONCURRENT_REGINA_THREADS; i++ )
   {
      if ( tsds[i]->thread_id == threadid )
         return tsds[i];
   }
   return NULL;
}

tsd_t *__regina_get_next_tsd( int idx )
{
   return tsds[idx];
}

/* __regina_get_tsd returns a pointer to the thread specific data. Be sure to
 * calls this after a ReginaInitializeThread only.
 */
tsd_t *__regina_get_tsd(void)
{
   /* See above for comments */
   return(*FindThreadDataIdx());
}

/* ReginaSetMutex is the opposite of ReginaUnsetMutex and sets a mutex
 * variable. The "true" mutex is "*arg" since we have hidden the type
 * HMTX which is the correct type. Thus, we have used "HMTX" and
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
   HMTX *os2_mutex = (HMTX *) mutex;

   if (*os2_mutex == (HMTX) 0)
   {
      DosEnterCritSec();
      if (*os2_mutex == (HMTX) 0) /* may have changed due MT */
      {
         if (DosCreateMutexSem(NULL,os2_mutex,0,FALSE) != NO_ERROR)
            OK = 0;
      }
      DosExitCritSec();
      if (!OK)
      { /* We must die now! There is no other chance. */
         *((int *) NULL) = 1;
      }
   }

   DosRequestMutexSem(*os2_mutex,SEM_INDEFINITE_WAIT);
   /* ignore errors, we continue especially if ERROR_INTERRUPTED occurs.
    * FIXME, ignoring ERROR_INTERRUPTED OK?
    */
}

/* see ReginaSetMutex */
void ReginaUnsetMutex(void **mutex)
{
   HMTX *os2_mutex = (HMTX *) mutex;

   DosReleaseMutexSem(*os2_mutex);
}
