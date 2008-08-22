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
   MT_mem *new = malloc(size + sizeof(MT_mem));

   if (new == NULL) /* may happen. errors are detected in the above layers */
      return(NULL);

   mt = TSD->mt_tsd;
   new->prev = NULL;
   new->next = mt->mem_base;
   if (mt->mem_base)
      mt->mem_base->prev = new;
   mt->mem_base = new;
   return(new + 1); /* jump over the head */
}

/* Lowest level memory deallocation function for normal circumstances. */
static void MTFree(const tsd_t *TSD,void *chunk)
{
   mt_tsd_t *mt = TSD->mt_tsd;
   MT_mem *this;

   /*
    * Just in case...
    */
   if ( chunk == NULL)
      return;

   this = chunk;
   this--; /* Go to the header of the chunk */

   if (this->prev)
   {
      if (this->prev->next != this)
         return;
   }
   if (this->next)
   {
      if (this->next->prev != this)
         return;
   }

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
   free(this);
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

   mt = TSD->mt_tsd;
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
