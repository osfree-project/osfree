/*
 $Id: F_config.hpp,v 1.5 2003/06/18 22:51:49 evgen2 Exp $
*/
/* F_config.hpp
 *
 */

#ifndef FREEPMCONFIG
#define FREEPMCONFIG

/*+---------------------------------+*/
/*| defines for config FreePM       |*/
/*+---------------------------------+*/
#define FREEPM_VERSION         "0.0.0.1"
#define FREEPM_SERVER_APPLICATION_NAME  "FreePM Server"
#define FREEPM_CLIENT_APPLICATION_NAME  "FreePM Client"

/* Ahtung!  the name of mutex smaphore shuld not be the same as VAC3
   project name !!!
*/
#if defined(__OS2__) || defined(__OS2V2__)
#define FREEPM_MUTEX_NAME            "\\SEM32\\_FREE_PM"
#else
#define FREEPM_MUTEX_NAME            "Global\\_FREE_PM"
#endif


/*+------------------------------------+*/
/*| defines for FreePM Debug subsystem |*/
/*+------------------------------------+*/

/*! @brief Number of debug sections (matching F_CMD_* classes, plus
    a few internal ones). */
#define F_DBG_MAX_SECTIONS  64

/*! @brief ANSI color escapes. Set s_cfg.ansiColor = 1 to enable. */
#define F_DBG_COLOR_FATAL     "\033[1;31;40m"
#define F_DBG_COLOR_WARNING   "\033[1;33;40m"
#define F_DBG_COLOR_IGNORING  "\033[1;35;40m"
#define F_DBG_COLOR_NORMAL    "\033[0;37;40m"
#define F_DBG_COLOR_DISABLE   "\033[2;37;40m"


#endif
/* FREEPMCONFIG */
