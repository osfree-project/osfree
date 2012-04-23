/* F_globals.cpp */
/* ver 0.01 01.09.2002       */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "FreePM.hpp"
/*+---------------------------------+*/
/*| Global variables                |*/
/*+---------------------------------+*/
int _FreePM_id_index   = 0;
int _FreePM_NeedToExit = 0;
int _FreePM_detachedMode = 0;
/* defined in main():
   const char *const _FreePM_Application_Name;
   const char *const _FreePM_Application_Vers;
*/

/* debug support */
extern "C" APIRET _FreePM_db_level = 7;
extern "C" APIRET _FreePM_debugLevels[MAX_DEBUG_SECTIONS] = {0};
int _FreePM_opt_debug_stderr = 5;
FILE *_FreePM_debug_log=NULL;

const char *const w_space = " \t\n\r";
const int  *const  w_t1 = (const int*)11;

int _FreePM_shutting_down = 0;
int _FreePM_FatalInProgress = 0;

/* time variables */
time_t _FreePM_curtime;
time_t _FreePM_current_time;
time_t _FreePM_start;
double _FreePM_current_dtime = 0.;

struct FreePM_Config _FreePMconfig =
{
/* struct Log */
   {
     "freepm.log",    /*  char *log;         */
     10               /*   int rotateNumber; */
   },

/* struct onoff */
   { 1,       /*  buffered_logs  */
     1,       /*  UseANSI_stdout */
     1        /*  UseBeepAtFatalError */

   },

   10.    /* double Timeout_connection;  client connection timeout, sec */

};



