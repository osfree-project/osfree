/*
 $Id: Fs_globals.cpp,v 1.7 2002/11/21 13:19:04 evgen2 Exp $
*/
/* Fs_globals.cpp */
/* server-side globals  */
/* ver 0.01 07.09.2002  */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include "F_def.hpp"
#include "F_config.hpp"
#include "F_hab.hpp"

#include "F_globals.hpp"
#include "Fs_globals.hpp"
#include "Fs_driver.h"

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

PFPM_DeviceStart_FN FPM_DeviceStart;

/* debug support */
int _FreePM_db_level    = 7;
int _FreePM_debugLevels[MAX_DEBUG_SECTIONS];
int _FreePM_opt_debug_stderr = 5;
FILE *_FreePM_debug_log=NULL;

const char *const w_space = " \t\n\r";
const int  *const  w_t1 = (const int*)11;

int _FreePM_shutting_down = 0;
int _FreePM_FatalInProgress = 0;

/* time variables */
DATETIME  _FreePM_curtime;
DATETIME _FreePM_current_time;
DATETIME _FreePM_start;
double _FreePM_current_dtime = 0.;

#define LONGfromRGB(R,G,B) (LONG)(((LONG)R<<16)+((LONG)G<<8)+(LONG)B)


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
     0        /*  UseBeepAtFatalError 1 */

   },

   10.,    /* double Timeout_connection;  client connection timeout, sec */
/* GUI */
/* struct GUI_WindowOptions desktop;   default desktop options */
 {  640, 480, /* Window size     */
    3,        /* bytes per pixel */
    0,        /* colors per pixel: */
    {   LONGfromRGB(255,127,255), /* ForegroundColor */
        LONGfromRGB(50,60,255),   /* BackgroungColor */
        LONGfromRGB(127,255,127), /* HiliteForegroundColor */
        LONGfromRGB(64,200,64),   /* HiliteBackgroundColor */
        LONGfromRGB(127,96,127),  /* DisabledForegroundColor */
        LONGfromRGB(20,30,127),   /* DisabledBackgroungColor */
        LONGfromRGB(10,10,10),    /* BorderColor */
        NULL,                     /* pFontNameSize */
        LONGfromRGB(255,200,200), /* ActiveColor */
        LONGfromRGB(64,100,100),  /* InactiveColor */
        LONGfromRGB(255,210,210), /* ActiveTextFGNDColor */
        LONGfromRGB(64,100,100),  /* ActiveTextBGNDColor */
        LONGfromRGB(155,110,110), /* ActiveTextFGNDColor */
        LONGfromRGB(64,64,64),    /* InctiveTextBGNDColor */
        LONGfromRGB(0,255,64),    /* ShadowColor */
        0                         /* user */
    }
 },
/* struct GUI_WindowOptions window;  default window  options */
 {  320, 240, /* Window size     */
    3,        /* bytes per pixel */
    0,        /* colors per pixel: */
    {   LONGfromRGB(255,200,255), /* ForegroundColor */
        LONGfromRGB(150,60,255),  /* BackgroungColor */
        LONGfromRGB(255,255,127), /* HiliteForegroundColor */
        LONGfromRGB(127,200,64),  /* HiliteBackgroundColor */
        LONGfromRGB(127,127,127), /* DisabledForegroundColor */
        LONGfromRGB(120,30,127),  /* DisabledBackgroungColor */
        LONGfromRGB(110,10,10),   /* BorderColor */
        NULL,                     /* pFontNameSize */
        LONGfromRGB(255,255,100), /* ActiveColor */
        LONGfromRGB(64,200,100),  /* InactiveColor */
        LONGfromRGB(255,110,210), /* ActiveTextFGNDColor */
        LONGfromRGB(64,200,100),  /* ActiveTextBGNDColor */
        LONGfromRGB(155,210,110), /* ActiveTextFGNDColor */
        LONGfromRGB(64,164,64),   /* InctiveTextBGNDColor */
        LONGfromRGB(0,255,255),   /* ShadowColor */
        0                         /* user */
    }

 }


};

/* server globals  */

class F_WND_List  _WndList;
class _FreePM_HAB  _shab;

