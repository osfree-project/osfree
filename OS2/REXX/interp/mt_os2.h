/* This is the supporting header file for WIN32 threads.
 * Checked with MSVC 6.0.
 * We need the following globals:
 *
 * 1) THREAD_PROTECT(varname)
 *    = a pseudo code NOT TERMINATED BY A SEMICOLON. After this point all
 *      code until a THREAD_UNPROTECT is executed by one thread at once.
 *      This may be done by a call to a semaphore action or something else.
 *      THREAD_PROTECT and THREAD_UNPROTECT define a C block logically.
 *      varname ist a variable created by PROTECTION_VAR()
 * 2) THREAD_UNPROTECT(varname)
 *    = see above
 * 3) PROTECTION_VAR(varname)
 *    = a pseudo code NOT TERMINATED BY A SEMICOLON. This define will
 *      create and initialize a local variable which may be used by
 *      THREAD_PROTECT and THREAD_UNPROTECT.
 *      Typical examples are the protection of static local variables.
 * 4) GLOBAL_PROTECTION_VAR(varname)
 *    = a pseudo code NOT TERMINATED BY A SEMICOLON. This define will
 *      create and initialize a global variable which may be used by
 *      THREAD_PROTECT and THREAD_UNPROTECT.
 *      Typical examples are the usage of the parser or global variables
 *      like macro_serialno.
 * 5) EXPORT_GLOBAL_PROTECTION_VAR(varname)
 *    = a pseudo code NOT TERMINATED BY A SEMICOLON. This define will
 *      export the varname in header files. There must exist one declaration
 *      of the variable done by GLOBAL_PROTECTION_VAR.
 * 6) GLOBAL_ENTRY_POINT()
 *    = initializes the process specific data and the thread specific data.
 *      This pseudo function is only called by those functions which are
 *      external (rexxsaa.h).
 *      It should return (tsd_t *) of the current thread.
 * 7) __regina_get_tsd()
 *    = pointer to a variable of type tsd_t.
 *      This may only exist after a GLOBAL_ENTRY_POINT()-call and must then
 *      exist.
 *
 */

#ifndef MT_H_INCLUDED
#  error This file should included by mt.h, only.
#endif

/* We hide the mutex management to fasten up the compilation step. I
 * don't want to include every special stuff here.
 */
void ReginaSetMutex(void **mutex);
void ReginaUnsetMutex(void **mutex);
#define THREAD_PROTECT(varname) ReginaSetMutex(&varname); {
#define THREAD_UNPROTECT(varname) } ReginaUnsetMutex(&varname);

#define PROTECTION_VAR(varname) static void * varname = NULL;

#define EXPORT_GLOBAL_PROTECTION_VAR(varname) extern void *varname;
#define GLOBAL_PROTECTION_VAR(varname) void *varname;

tsd_t *ReginaInitializeThread(void);
#define GLOBAL_ENTRY_POINT() ReginaInitializeThread()

/* we need a pointer to tsd_t sometimes: */
tsd_t *__regina_get_tsd(void);

/* NEVER USE __regina_get_tsd() IF YOU CAN GET THE VALUE FROM SOMEWHERE ELSE.
 * IT REDUCES THE EXECUTION SPEED SIGNIFICANTLY. TAKE THE VALUE FROM THE CALLER
 * WHERE IT IS POSSIBLE.
 */
