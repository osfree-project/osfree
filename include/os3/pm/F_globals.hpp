/* F_globals.hpp */

#ifndef FREEPM_GLOBALS
   #define FREEPM_GLOBALS

#include "F_def.hpp"

/*+---------------------------------+*/
/*| Global variables                |*/
/*+---------------------------------+*/
extern int _FreePM_id_index;           /* 0 */
extern int _FreePM_NeedToExit;         /* 0 */
extern int _FreePM_detachedMode;       /* 0 */
extern const char *const _FreePM_Application_Name;
extern const char *const _FreePM_Application_Vers;

/* debug support */
extern int _FreePM_db_level;           /* 7 */
extern int _FreePM_debugLevels[MAX_DEBUG_SECTIONS];
extern int _FreePM_opt_debug_stderr;   /* 5 */
extern FILE *_FreePM_debug_log;        /* NULL */
extern const char *const w_space;      /* " \t\n\r" */
extern int _FreePM_shutting_down;      /* 0 */
extern int _FreePM_FatalInProgress;    /* 0 */

/* time variables */
//extern DATETIME _FreePM_curtime;         /* 0 */
//extern DATETIME _FreePM_current_time;
//extern DATETIME _FreePM_start;

extern time_t _FreePM_curtime;         /* 0 */
extern time_t _FreePM_current_time;
extern time_t _FreePM_start;

extern double _FreePM_current_dtime;  /* 0. */

extern struct FreePM_Config _FreePMconfig;

#ifndef FREPM_SERVER
extern class _FreePM_HAB  _hab;
#endif

#endif
    /* FREEPM_GLOBALS */
