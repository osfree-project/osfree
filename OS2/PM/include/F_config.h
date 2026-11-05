/*
 $Id: F_config.h,v 1.5 2003/06/18 22:51:49 evgen2 Exp $
*/
/* F_config.h
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

/*+------------------------------------+*/
/*| Debug levels and sections          |*/
/*+------------------------------------+*/

/*! @brief Debug levels (0 = critical .. 10 = verbose). */
#define F_DBG_LVL_FATAL     0
#define F_DBG_LVL_ERROR     1
#define F_DBG_LVL_WARNING   2
#define F_DBG_LVL_INFO      3
#define F_DBG_LVL_VERBOSE   5

/*! @brief Maximum context nesting depth. */
#define F_DBG_CTX_MAX       32

/*! @brief Maximum context description length (bytes, incl. NUL). */
#define F_DBG_CTX_MAX_LEN   256

/*! @brief Debug section identifiers. More are added as components
    migrate to the unified debug system. */
#define F_DBG_SEC_DEBUG         0
/*! @brief Debug section: PMSERVER main (pmserver.cpp). */
#define F_DBG_SEC_PMSERVER      1
/*! @brief Debug section: Fs_Server (sessions, monitor, wire). */
#define F_DBG_SEC_FS_SERVER     2
/*! @brief Debug section: Fs_ClientList (per-client slots). */
#define F_DBG_SEC_FS_CLIENT     3
/*! @brief Debug section: server GPI dispatcher (dispatch.cpp). */
#define F_DBG_SEC_DISPATCH      5
/*! @brief Debug section: GOCA order emitters (goca_emit*.h). */
#define F_DBG_SEC_GOCA_EMIT     6
/*! @brief Debug section: GOCA parser (goca_parser.cpp). */
#define F_DBG_SEC_GOCA_PARSE    7
/*! @brief Debug section: GRE (gre.cpp). */
#define F_DBG_SEC_GRE           8
/*! @brief Debug section: GPI dispatcher (gpi_entry.cpp). */
#define F_DBG_SEC_GPI_DISPATCH  20
/*! @brief Debug section: GPI proxy functions (gpi_manual.cpp, gpi_proxies.cpp). */
#define F_DBG_SEC_GPI_PROXIES   21
/*! @brief Debug section: GPI unimplemented stubs (unimpl.cpp). */
#define F_DBG_SEC_GPI_UNIMPL    24
/*! @brief Debug section: PMCLIENT.DLL (init, connect, reconnect, keepalive). */
#define F_DBG_SEC_PMCLIENT      30
/*! @brief Debug section: PMTCP transport. */
#define F_DBG_SEC_PMTCP     40
/*! @brief Debug section: PMTCP socket I/O. */
#define F_DBG_SEC_WIRE      41

/*! @brief Default debug subsystem configuration, used by
    F_DBG_DEFAULT_CFG in F_debug.cpp. Applied in DllMain(ATTACH) so
    that every module loaded afterwards already has a working
    F_DbgPrint / F_DbgCtxEnterf; overridable at runtime through
    F_DbgSetConfig and F_DbgSetLevels. */
#define F_DBG_DEFAULT_LOGFILE        "freepm.log"
#define F_DBG_DEFAULT_ROTATENUMBER   10
#define F_DBG_DEFAULT_ROTATESIZE     0
#define F_DBG_DEFAULT_BUFFERED       0
#define F_DBG_DEFAULT_ANSICOLOR      1
#define F_DBG_DEFAULT_BEEPONFATAL    0
#define F_DBG_DEFAULT_USESYSLOG      1
#define F_DBG_DEFAULT_THRESHOLDLEVEL 10
#define F_DBG_DEFAULT_LEVELS         "ALL,10"

#endif
/* FREEPMCONFIG */
