/* This is the file to support single-threading.
 * We initialize the global data structure and the global access variable.
 */

#include "regina_c.h"
#include "rexxsaa.h"
#define DONT_TYPEDEF_PFN
#include "rexx.h"

typedef struct _MT_mem {
   struct _MT_mem *prev;
   struct _MT_mem *next;
   /* real data follows here */
} MT_mem;

typedef struct { /* mt_tsd: static variables of this module (thread-safe) */
   MT_mem *mem_base;
} mt_tsd_t; /* thread-specific but only needed by this module. see
             * ReginaInitializeThread
             */

int __regina_tsd_initialized = 0;
tsd_t __regina_tsd = {0,}; /* Be sure the var is initialized here   */

/* Lowest level memory allocation function for normal circumstances. */
static void *MTMalloc(const tsd_t *TSD,size_t size)
{
   mt_tsd_t *mt;
   MT_mem *newptr = (MT_mem *)malloc(size + sizeof(MT_mem));

   if (newptr == NULL) /* may happen. errors are detected in the above layers */
      return(NULL);

   mt = (mt_tsd_t *)TSD->mt_tsd;
   newptr->prev = NULL;
   newptr->next = mt->mem_base;
   if (mt->mem_base)
      mt->mem_base->prev = newptr;
   mt->mem_base = newptr;
   return(newptr + 1); /* jump over the head */
}

/* Lowest level memory deallocation function for normal circumstances. */
static void MTFree(const tsd_t *TSD,void *chunk)
{
   mt_tsd_t *mt = (mt_tsd_t *)TSD->mt_tsd;
   MT_mem *thisptr;

   /*
    * Just in case...
    */
   if ( chunk == NULL)
      return;

   thisptr = (MT_mem *)chunk;
   thisptr--; /* Go to the header of the chunk */

   if (thisptr->prev)
   {
      if (thisptr->prev->next != thisptr)
         return;
   }
   if (thisptr->next)
   {
      if (thisptr->next->prev != thisptr)
         return;
   }

   /* This is a chunk allocated by MTMalloc */
   if (thisptr->prev)
      thisptr->prev->next = thisptr->next;
   if (thisptr->next)
      thisptr->next->prev = thisptr->prev;
   if (thisptr == mt->mem_base)
      mt->mem_base = thisptr->next;

   /* Last not least we set the pointers to NULL. This prevents a double-free*/
   thisptr->next = NULL;
   thisptr->prev = NULL;
   free(thisptr);
}

/* Lowest level exit handler. Use this indirection to prevent errors. */
static void MTExit(int code)
{
   exit(code);
}

int IfcReginaCleanup( VOID )
{
   tsd_t *TSD = &__regina_tsd;
   mt_tsd_t *mt;
   MT_mem *chunk;

   if (!__regina_tsd_initialized)
      return 0;

   __regina_tsd_initialized = 0;

   deinit_rexxsaa(TSD);

   mt = (mt_tsd_t *)TSD->mt_tsd;
   if (mt)
   {
      while ((chunk = mt->mem_base) != NULL)
      {
         TSD->MTFree(TSD,chunk + 1);
         if (mt->mem_base == chunk)
            break; /* something goes wrong. Don't run into an endless loop */
      }
   }
   if (mt)
      free(mt);
   memset( TSD, 0, sizeof(tsd_t) );
   return 1;
}

tsd_t *ReginaInitializeProcess(void)
{
   int OK;

   if (__regina_tsd_initialized)
      return(&__regina_tsd);
   __regina_tsd_initialized = 1;

                                        /* Set up the current (single) tsd_t:*/
                                        /* Default all values to zero        */
   memset(&__regina_tsd,0,sizeof(__regina_tsd));
   __regina_tsd.MTMalloc = MTMalloc;
   __regina_tsd.MTFree = MTFree;
   __regina_tsd.MTExit = MTExit;

   /* Since the local data structure contains a memory chain for the memory
    * management we initialize it first.
    */
   if ((__regina_tsd.mt_tsd = malloc(sizeof(mt_tsd_t))) == NULL)
      return(NULL);                     /* This is a catastrophy             */
   memset(__regina_tsd.mt_tsd,0,sizeof(mt_tsd_t));

   OK = init_memory(&__regina_tsd);     /* Initialize the memory module FIRST*/

   /* Without the initial memory we don't have ANY chance! */
   if (!OK)
      return(NULL);

   /*
    * Some systems with an own MT file don't compile in MT mode. But they
    * still are systems of that kind.
    */
#if defined(WIN32) || defined(__WIN32__)
   {
      extern OS_Dep_funcs __regina_OS_Win;
      __regina_tsd.OS = &__regina_OS_Win;
   }
#elif defined(OS2) && !defined(DOS)
   {
      extern OS_Dep_funcs __regina_OS_Os2;
      __regina_tsd.OS = &__regina_OS_Os2;
   }
#elif defined(GO32)
   {
      extern OS_Dep_funcs __regina_OS_Other;
      __regina_tsd.OS = &__regina_OS_Other;
   }
#elif defined(unix) || defined(__unix__) || defined(__unix) || defined(__QNX__) || defined(__BEOS__) || defined(__HAIKU__) || defined(SKYOS) || ( defined( __APPLE_CC__ ) && defined( __MACH__ ) ) || defined(AIX) || defined(__MINIX__)
   {
      extern OS_Dep_funcs __regina_OS_Unx;
      __regina_tsd.OS = &__regina_OS_Unx;
   }
#else
   {
      extern OS_Dep_funcs __regina_OS_Other;
      __regina_tsd.OS = &__regina_OS_Other;
   }
#endif
   __regina_tsd.OS->init();
   OK |= init_vars(&__regina_tsd);      /* Initialize the variable module    */
   OK |= init_stacks(&__regina_tsd);    /* Initialize the stack module       */
   OK |= init_filetable(&__regina_tsd); /* Initialize the files module       */
   OK |= init_math(&__regina_tsd);      /* Initialize the math module        */
   OK |= init_spec_vars(&__regina_tsd); /* Initialize the interprt module    */
   OK |= init_tracing(&__regina_tsd);   /* Initialize the tracing module     */
   OK |= init_builtin(&__regina_tsd);   /* Initialize the builtin module     */
   OK |= init_client(&__regina_tsd);    /* Initialize the client module      */
   OK |= init_library(&__regina_tsd);   /* Initialize the library module     */
   OK |= init_rexxsaa(&__regina_tsd);   /* Initialize the rexxsaa module     */
   OK |= init_shell(&__regina_tsd);     /* Initialize the shell module       */
   OK |= init_envir(&__regina_tsd);     /* Initialize the envir module       */
   OK |= init_expr(&__regina_tsd);      /* Initialize the expr module        */
   OK |= init_error(&__regina_tsd);     /* Initialize the error module       */
#ifdef VMS
   OK |= init_vms(&__regina_tsd);       /* Initialize the vmscmd module      */
   OK |= init_vmf(&__regina_tsd);       /* Initialize the vmsfuncs module    */
#endif
   OK |= init_arexxf(&__regina_tsd);    /* Initialize the arxfuncs modules */
   __regina_tsd.loopcnt = 1;            /* stupid r2perl-module              */
   __regina_tsd.traceparse = -1;
   __regina_tsd.thread_id = 1;

   if (!OK)
      exiterror( ERR_STORAGE_EXHAUSTED, 0 ) ;

   return(&__regina_tsd);
}

int __regina_get_number_concurrent_regina_threads(void)
{
   return 1;
}

tsd_t *__regina_get_tsd_for_threadid( unsigned long threadid )
{
   return &__regina_tsd;
}

tsd_t *__regina_get_next_tsd( int idx )
{
   return &__regina_tsd;
}


#ifdef TRACK_TSD_USAGE
/* We may want to check the counts of calls to __regina_get_tsd() which may do
 * MUCH work on different platforms. We do some not optimizable work here.
 * If you really wanna track down all calls to figure out WHERE to
 * optimize try under a GNU friendly system:
 * 1) In Makefile: Add "-pg -a" to the variable called "CFLAGS".
 * 2) "make rexx" (Other targets might not work)
 * 3) "./rexx whatyoulike.rexx"
 * 4) "gprof rexx >usage.lst"
 * 5) look at usage.lst for occurances of "WorkHeavy".
 */

volatile int __regina_Calls = 300; /* factor to get a "feel" for multithreading */
volatile int __regina_Point = 1;

void __regina_Nop(void)
{
   __regina_Point = 2;
}

/* WorkHeavy does some work and returns the correct thread-specific data. */
tsd_t *__regina_WorkHeavy(void)
{
   int todo = __regina_Calls;
   while (todo--)
      __regina_Nop();
   return(&__regina_tsd);
}
#endif
