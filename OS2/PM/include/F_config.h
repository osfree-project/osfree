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

/*! @brief Client -> server handshake magic (9 bytes, incl. NUL). */
#define HAND_SHAKE_INP         "pIpEtEsT"
/*! @brief Server -> client handshake magic (9 bytes, incl. NUL). */
#define HAND_SHAKE_OUT         "PiPeTeSt"

/*! @def FREEPM_TCP_DEFAULT_HOST
    @brief Default peer host (loopback). */
#define FREEPM_TCP_DEFAULT_HOST  "127.0.0.1"
/*! @def FREEPM_TCP_DEFAULT_PORT
    @brief Default TCP port (must match PMCLIENT expectations). */
#define FREEPM_TCP_DEFAULT_PORT  9876


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


/* ======================================================================== */
/*  F_Config — shared parser base for client and server configuration.      */
/*  Defined here only for C++; the C-side of this header stays pure         */
/*  #define.                                                                 */
/* ======================================================================== */
#ifdef __cplusplus
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

class F_Config {
public:
    char debugOptions[256];

    F_Config() { strcpy(debugOptions, F_DBG_DEFAULT_LEVELS); }

    int Read(char *fname);
    int ReadStr(FILE *fp, char *str,
                char *nameClass, char *name, char *par);
    virtual int Write(char *fname);
    virtual int AnalizeRecodrRead(char *name, char *par);
};

inline int F_Config::Read(char *fname)
{
    int   rc;
    FILE *fp;
    char  str[256], nameClass[128], name[128], par[128];

    fp = fopen(fname, "r");
    if (fp == NULL) return 1;

    do {
        rc = ReadStr(fp, str, nameClass, name, par);
        if (!rc) AnalizeRecodrRead(name, par);
    } while (!rc);

    fclose(fp);
    return 0;
}

inline int F_Config::ReadStr(FILE *fp, char *str,
                             char *nameClass, char *name, char *par)
{
    int   i, iscomment, l;
    char *pstr;

M:  pstr = fgets(str, 128, fp);
    if (pstr == NULL) return 1;          /* EOF */

    l = (int)strlen(str);
    if (l > 0 && str[l - 1] == '\n') { str[--l] = 0; }
    if (l == 0) goto M;

    iscomment = 0;
    for (i = 0; i < l; i++) {
        if (str[i] > 32) {
            if (str[i] == ';') iscomment = 1;
            break;
        }
    }
    if (iscomment) goto M;

    pstr = strstr(str, "=");
    if (pstr == NULL) goto M;

    *pstr = 0;
    strcpy(par, pstr + 1);

    pstr = strstr(str, ".");
    if (pstr == NULL) {
        nameClass[0] = 0;
        sscanf(str, "%s", name);
    } else {
        *pstr = 0;
        sscanf(str, "%s", nameClass);
        sscanf(pstr + 1, "%s", name);
    }
    return 0;
}

inline int F_Config::Write(char *fname)
{
    (void)fname;
    return 1;
}

inline int F_Config::AnalizeRecodrRead(char *name, char *par)
{
    if (!strcmp(name, "debugOptions")) {
        sscanf(par, "%s", debugOptions);
        return 0;
    }
    return 1;
}

#endif /* __cplusplus */

#endif
/* FREEPMCONFIG */
