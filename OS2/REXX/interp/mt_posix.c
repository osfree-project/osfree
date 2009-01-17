/* This is the file to support POSIX threads.
 * We initialize the global data structure and the global access variable.
 */

#include "regina_c.h"
#include "rexxsaa.h"
#define DONT_TYPEDEF_PFN
#include "rexx.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <pwd.h>
#ifdef HAVE_NETDB_H
# include <netdb.h>
#endif
#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
#endif
#ifdef HAVE_ARPA_INET_H
# include <arpa/inet.h>
#endif
#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif
#ifdef HAVE_GRP_H
# include <grp.h>
#endif
#include <time.h>
#include <errno.h>

typedef struct _MT_mem {
   struct _MT_mem *prev;
   struct _MT_mem *next;
   /* real data follows here */
} MT_mem;

typedef struct { /* mt_tsd: static variables of this module (thread-safe) */
   struct group       getgrgid_retval;
   char               getgrgid_buf[2048]; /* AT LEAST 1K, better 2K or 4K */
   struct passwd      getpwuid_retval;
   char               getpwuid_buf[2048]; /* AT LEAST 1K, better 2K or 4K */
   struct hostent     gethbyn_retval;
   char               gethbyn_buf[8192];  /* sign, size copied from glibc */
   char               inetntoa_buf[16];
   struct tm          gmtime_retval;
   struct tm          localtime_retval;
#ifdef HAVE_RANDOM_DATA
   struct random_data random;
   char               random_buf[64]; /* at least sizeof(clock_t) and time_t */
   int                random_init;
#endif
   char               strerror_buf[1024];
   MT_mem *           mem_base;
} mt_tsd_t; /* thread-specific but only needed by this module. see
             * ReginaInitializeThread
             */

static pthread_key_t ThreadIndex; /* index of the TSD, no initial value */
static pthread_once_t ThreadOnce = PTHREAD_ONCE_INIT; /* for pthread_once */

/* Deinitialize is called when the thread terminates.
 * This is a wonderful position to place code which frees all allocated stuff!
 */
static void Deinitialize(void *buf)
{
   tsd_t *TSD = (tsd_t *)buf;
   mt_tsd_t *mt;
   MT_mem *chunk;

   if (TSD == NULL) /* Should never happen but be sure */
      return;

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
   free(TSD);
}

int IfcReginaCleanup( VOID )
{
   tsd_t *TSD = __regina_get_tsd();

   if (TSD == NULL)
      return 0;
   Deinitialize(TSD);
   pthread_setspecific(ThreadIndex,NULL);

   return 1;
}

/* ThreadGetKey creates a new index key into the TSD table. This function
 * must only be used by pthread_once.
 */
static void ThreadGetKey(void)
{
   pthread_key_create(&ThreadIndex,Deinitialize);
}

/* Lowest level memory allocation function for normal circumstances. */
static void *MTMalloc( const tsd_t *TSD, size_t size )
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
static void MTFree( const tsd_t *TSD, void *chunk )
{
   mt_tsd_t *mt = (mt_tsd_t *)TSD->mt_tsd;
   MT_mem *thisptr;

   /*
    * Just in case...
    */
   if (chunk == NULL)
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

/* Lowest level exit handler. */
static void MTExit(int code)
{
   /* pthread_exit's argument has a different semantic as exit has. */
   pthread_exit(NULL);
}

/* ReginaInitializeThread creates a new thread structure and returns a ptr
 * to the initialized value.
 * The function may be called more than once.
 */
tsd_t *ReginaInitializeThread(void)
{
   int OK;
   tsd_t *retval;

   /* get a unique access variable for the whole process */
   pthread_once(&ThreadOnce,ThreadGetKey);

   /* fetch the value of the access variable */
   retval = (tsd_t *)pthread_getspecific(ThreadIndex);

   if (retval != NULL)
      return(retval);

   /* First call in this thread... */
   retval = (tsd_t *)malloc(sizeof(tsd_t)); /* no Malloc, etc! */

   if (retval == NULL) /* THIS is really a problem. I don't know what we */
      return(NULL);    /* should do now. Let the caller run into a crash... */

   pthread_setspecific(ThreadIndex,retval);

   memset(retval,0,sizeof(tsd_t));
   retval->MTMalloc = MTMalloc;
   retval->MTFree = MTFree;
   retval->MTExit = MTExit;

   /* Since the local data structure contains a memory chain for the memory
    * management we initialize it first.
    */
   if ( ( retval->mt_tsd = (mt_tsd_t *)malloc( sizeof(mt_tsd_t) ) ) == NULL )
      return(NULL);                     /* This is a catastrophy             */
   memset( retval->mt_tsd, 0, sizeof(mt_tsd_t) );

   OK = init_memory(retval);            /* Initialize the memory module FIRST*/

   /* Without the initial memory we don't have ANY chance! */
   if (!OK)
      return(NULL);

   {
      extern OS_Dep_funcs __regina_OS_Unx;
      retval->OS = &__regina_OS_Unx;
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
   retval->thread_id = (unsigned long)pthread_self();

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
   return( (tsd_t *)pthread_getspecific( ThreadIndex ) );
}

/******************************************************************************
 ******************************************************************************
 * Emulation functions to support multi-threading *****************************
 * The functions will be opaque to the caller *********************************
 * The "original" functions at this place are not multi-threading safe ********
 ******************************************************************************
 *****************************************************************************/
#if defined(HAVE_GETGRGID_R_RETURNS_INT_5_PARAMS) || defined(HAVE_GETGRGID_R_RETURNS_INT_4_PARAMS) ||defined(HAVE_GETGRGID_R_RETURNS_STRUCT)
/* see documentation of getgrgid and getgrgid_r */
struct group *getgrgid(gid_t gid)
{
   mt_tsd_t *mt = (mt_tsd_t *)__regina_get_tsd()->mt_tsd;
   struct group *ptr=NULL;
   int rc=0;
# ifdef HAVE_GETGRGID_R_RETURNS_INT_5_PARAMS
   rc = getgrgid_r(gid,
                   &mt->getgrgid_retval,
                   mt->getgrgid_buf,
                   sizeof(mt->getgrgid_buf),
                   &ptr);
# endif
# ifdef HAVE_GETGRGID_R_RETURNS_INT_4_PARAMS
   rc = getgrgid_r(gid,
                   &mt->getgrgid_retval,
                   mt->getgrgid_buf,
                   sizeof(mt->getgrgid_buf));
   ptr = &mt->getgrgid_retval;
# endif
# ifdef HAVE_GETGRGID_R_RETURNS_STRUCT
   ptr = getgrgid_r(gid,
                   &mt->getgrgid_retval,
                   mt->getgrgid_buf,
                   sizeof(mt->getgrgid_buf) );
# endif
   if ((rc != 0) || (ptr == NULL))
   {
      errno = rc;
      return(NULL);
   }
   return(ptr);
}
#endif

#if defined(HAVE_GETPWUID_R_RETURNS_INT) || defined(HAVE_GETPWUID_R_RETURNS_STRUCT)
/* see documentation of getpwuid and getpwuid_r */
struct passwd *getpwuid(uid_t uid)
{
   mt_tsd_t *mt = (mt_tsd_t *)__regina_get_tsd()->mt_tsd;
   struct passwd *ptr=NULL;
   int rc=0;

# ifdef HAVE_GETPWUID_R_RETURNS_INT
   rc = getpwuid_r(uid,
                   &mt->getpwuid_retval,
                   mt->getpwuid_buf,
                   sizeof(mt->getpwuid_buf),
                   &ptr);
# else
   ptr = getpwuid_r(uid,
                   &mt->getpwuid_retval,
                   mt->getpwuid_buf,
                   sizeof(mt->getpwuid_buf) );
# endif
   if ((rc != 0) || (ptr == NULL))
   {
      errno = rc;
      return(NULL);
   }
   return(ptr);
}
#endif

/* see documentation of gmtime and gmtime_r */
struct tm *gmtime(const time_t *time)
{
   mt_tsd_t *mt = (mt_tsd_t *)__regina_get_tsd()->mt_tsd;
#ifdef HAVE_GMTIME_R
   return(gmtime_r(time,&mt->gmtime_retval));
#else
   return NULL;
#endif
}

/* see documentation of localtime and localtime_r */
struct tm *localtime(const time_t *time)
{
   mt_tsd_t *mt = (mt_tsd_t *)__regina_get_tsd()->mt_tsd;

#ifdef HAVE_LOCALTIME_R
   return(localtime_r(time,&mt->localtime_retval));
#else
   return NULL;
#endif
}

/* see documentation of gethostbyname and gethostbyname_r */
struct hostent *gethostbyname(const char *name)
{
   int herr;
   struct hostent *he=NULL;
   mt_tsd_t *mt = (mt_tsd_t *)__regina_get_tsd()->mt_tsd;

#ifdef HAVE_GETHOSTBYNAME_R_RETURNS_INT_6_PARAMS
   if (gethostbyname_r(name,
                       &mt->gethbyn_retval,
                       mt->gethbyn_buf,
                       sizeof(mt->gethbyn_buf),
                       &he,
                       &herr) != 0)
      return(NULL);
#endif
#ifdef HAVE_GETHOSTBYNAME_R_RETURNS_STRUCT_5_PARAMS
   he = gethostbyname_r(name,
                        &mt->gethbyn_retval,
                        mt->gethbyn_buf,
                        sizeof(mt->gethbyn_buf),
                        &herr);
#endif
   return(he);
}

/* see documentation of inet_ntoa and inet_ntop */
char *inet_ntoa(struct in_addr in)
{
#ifdef HAVE_INET_NTOP
   mt_tsd_t *mt = (mt_tsd_t *)__regina_get_tsd()->mt_tsd;

   return((char *) inet_ntop(AF_INET,
                             &in,
                             mt->inetntoa_buf,
                             sizeof(mt->inetntoa_buf)));
#else
   return NULL;
#endif
}

#if defined( HAVE_RANDOM_DATA ) && !defined( HAVE_RANDOM )
/* see documentation of random and random_r. If you get compile time errors
 * exclude this function.
 */
int32_t random(void)
{
   mt_tsd_t *mt = __regina_get_tsd()->mt_tsd;
   int32_t retval;

   if (!mt->random_init) /* according to the ANSI this is equivalent to */
      srandom(1);        /* an initial value of 1 for the seed!         */
   random_r(&mt->random,&retval); /* ignore errors, we MUST return anything */
   return(retval);
}

/* see documentation of srandom and srandom_r. If you get compile time errors
 * exclude this function.
 */
void srandom(unsigned seed)
{
   mt_tsd_t *mt = __regina_get_tsd()->mt_tsd;
   /* The stack is normally not cleared: wonderful random stuff. */
   char this_is_random_stuff[sizeof(mt->random_buf)];

   if (!mt->random_init)
   {
      mt->random_init = 1;
      /* In case of a cleared new stack seqment we initialize our buffer
       * with at least one random stuff:
       */
      time((time_t *) this_is_random_stuff);
      /* If the buffer is large enough, use a second one */
      if (sizeof(time_t) + sizeof(clock_t) < sizeof(this_is_random_stuff))
         *((clock_t *) (this_is_random_stuff + sizeof(time_t))) = clock();

      memcpy(mt->random_buf,this_is_random_stuff,sizeof(mt->random_buf));

      initstate_r(seed,
                  mt->random_buf,
                  sizeof(mt->random_buf),
                  &mt->random);
      /* initstate calls srandom. We can return savely. */
      return;
   }
   srandom_r(seed,&mt->random);
}
#endif

#ifdef STRERROR_R
/* see documentation of strerror and strerror_r. */
char *strerror(int errnum)
{
   mt_tsd_t *mt = __regina_get_tsd()->mt_tsd;

   return(strerror_r(errnum,mt->strerror_buf,sizeof(mt->strerror_buf)));
}
#endif

