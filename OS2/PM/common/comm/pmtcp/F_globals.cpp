/*! @file F_globals.cpp
    @brief Global variables for the TCP transport backend.

    Cross-platform: uses F_tcp.hpp to pull in the correct system headers
    and OS/2-compatible type definitions for both OS/2 and Win32.
*/

#include <stdio.h>
#include <string.h>
#include <time.h>

#include "F_tcp.hpp"   /* Provides APIRET, ULONG, LONG, etc. */

/*! @brief Monotonic index for unique object identifiers. */
int _FreePM_id_index   = 0;

/*! @brief Set when the module is about to exit. */
int _FreePM_NeedToExit = 0;

/*! @brief Set when running as a detached process. */
int _FreePM_detachedMode = 0;

/*! @brief Current debug verbosity level. */
extern "C" APIRET _FreePM_db_level = 7;

/*! @brief Per-section debug levels. */
extern "C" APIRET _FreePM_debugLevels[64] = {0};

/*! @brief Debug verbosity for stderr output. */
int _FreePM_opt_debug_stderr = 5;

/*! @brief Debug log file handle (unused by PMTCP). */
FILE *_FreePM_debug_log = NULL;

/*! @brief Whitespace characters used by the tokenizer. */
const char *const w_space = " \t\n\r";

/*! @brief Placeholder for a deprecated constant. */
const int  *const  w_t1 = (const int*)11;

/*! @brief Set when the module is shutting down. */
int _FreePM_shutting_down = 0;

/*! @brief Set when a fatal error is in progress. */
int _FreePM_FatalInProgress = 0;

/*! @brief Last recorded time. */
time_t _FreePM_curtime;

/*! @brief Current time. */
time_t _FreePM_current_time;

/*! @brief Module start time. */
time_t _FreePM_start;

/*! @brief Current time as a double. */
double _FreePM_current_dtime = 0.;
