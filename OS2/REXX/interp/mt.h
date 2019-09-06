#ifndef MT_H_INCLUDED
#define MT_H_INCLUDED

#include <setjmp.h>

struct _OS_Dep_funcs;

/* tsd_t holds all global vars which may change between threads as expected.*/
typedef struct _tsd_t {
   void *                  mem_tsd ;           /* local variables of memory.c */
   void *                  var_tsd ;           /* local variables of variable.c */
   void *                  stk_tsd ;           /* local variables of stack.c */
   void *                  fil_tsd ;           /* local variables of files.c */
   void *                  itp_tsd ;           /* local variables of interprt.c */
   void *                  tra_tsd ;           /* local variables of tracing.c */
   void *                  err_tsd ;           /* local variables of error.c */
   void *                  vms_tsd ;           /* local variables of vmscmd.c */
   void *                  bui_tsd ;           /* local variables of builtin.c */
   void *                  vmf_tsd ;           /* local variables of vmsfuncs.c */
   void *                  lib_tsd ;           /* local variables of library.c */
   void *                  rex_tsd ;           /* local variables of rexxsaa.c */
   void *                  shl_tsd ;           /* local variables of shell.c */
   void *                  mat_tsd ;           /* local variables of strmath.c */
   void *                  cli_tsd ;           /* local variables of client.c */
   void *                  arx_tsd ;           /* local variables of arxfuncs.c */
   void *                  mt_tsd ;            /* local variables of mt_???.c */

   void *                  CH;                 /* only rexxsaa.c */
   int                     indentsize;         /* only in r2perl.c */
   int                     loopcnt;            /* only in r2perl.c */
   paramboxptr             listleaked_params ; /* only in funcs.c */
   paramboxptr             par_stack ;         /* only in funcs.c */
   int                     traceparse ;        /* only in parsing.c */
   num_descr               rdes;               /* only in expr.c */
   num_descr               ldes;               /* only in expr.c */
   void *                  firstenvir;         /* only in envir.c */
   void *                  last_alloca_header; /* only in alloca.c */
   void *                  stkaddr;            /* only in alloca.c */
   volatile char *         tmp_strs[8];        /* only tmpstr_of() */
   int                     next_tmp_str;       /* only tmpstr_of() */
   paramboxptr             bif_first ;         /* only builtinfunc() */
   void *                  firstmacro ;        /* only in macros.c */

   sysinfo                 systeminfo ;
   proclevel               currlevel ;
   int                     listleakedmemory ;
   int                     var_indicator ;
   int                     isclient ;
   nodeptr                 currentnode ;
   sigtype *               nextsig;
   FILE *                  stddump;
   unsigned long           thread_id;
   int                     instore_is_errorfree;
   char                    trace_stat;
   int                     called_from_saa;
   int                     restricted;
   /* Stuff for a delayed exit()/setjmp(): */
   int                     in_protected;
   jmp_buf                 protect_return;
   jmp_buf                 gci_jump;
   volatile delayed_error_type_t delayed_error_type;
   volatile int            expected_exit_error;
                            /* call exit() with this value if
                             * delayed_error_type is PROTECTED_DelayedScriptExit
                             */
   volatile int            HaltRaised;
   void *                  (*MTMalloc)(const struct _tsd_t *TSD,size_t size);
   void                    (*MTFree)(const struct _tsd_t *TSD,void *chunk);
   void                    (*MTExit)(int code);
   char                    gci_prefix[2];
   const char             *BIFname;
   void                   *BIFfunc;
   struct _OS_Dep_funcs   *OS;
} tsd_t;

#if (defined(POSIX) || defined(_POSIX_SOURCE) || defined(_PTHREAD_SEMANTICS)) && (defined(_REENTRANT) || defined(REENTRANT)) && defined(REGINA_REENTRANT)
#  include "mt_posix.h"
#elif ( defined(_WIN64) || defined(_WIN32) ) && defined(REGINA_SINGLE_THREADED)
#  include "mt_notmt.h"
#  define SINGLE_THREADED
#elif defined(_WIN64) && defined(_MT)
#  include "mt_win64.h"
#elif defined(_WIN32) && defined(_MT)
#  include "mt_win32.h"
#elif defined(OS2) && defined(__EMX__) && defined(__MT__) && defined(REGINA_MULTI)
#  include "mt_os2.h"
#elif defined(OS2) && defined(__WATCOMC__) && defined(REGINA_MULTI)
#  include "mt_os2.h"
#else
#  include "mt_notmt.h"
#  if !defined(SINGLE_THREADED) && !defined(MULTI_THREADED)
#     define SINGLE_THREADED
#   endif
#endif

/* SINGLE_THREAD is defined explicitely, thus: */
#if !defined(SINGLE_THREADED) && !defined(MULTI_THREADED)
#  define MULTI_THREADED
#endif

#endif /* MT_H_INCLUDED */
