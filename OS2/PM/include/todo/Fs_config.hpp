/*
 $Id: Fs_config.hpp,v 1.1 2003/06/15 17:45:47 evgen2 Exp $
*/
/* FreePM Server config */

#include "F_config.h"

/* Limits (compile-time) */
#define F_MAX_CLIENTS           64   /*!< Maximum simultaneous connections. */
#define F_MAX_HABS              128  /*!< Maximum simultaneous HABs. */
#define F_MAX_THREADS           16   /*!< Maximum simultaneous threads per client. */
#define F_MONITOR_TICK_MS       1000 /*!< Monitor thread wake-up, ms. */
#define F_DEFAULT_INACTIVE      60   /*!< Default inactivity timeout, sec. */
#define F_DEFAULT_GRACE         30   /*!< Default grace timeout, sec. */

/* Desktop defaults */
#define F_DEFAULT_DEVICE         "PM"
#define F_DEFAULT_DESKTOP_NX     640
#define F_DEFAULT_DESKTOP_NY     480
#define F_DEFAULT_BYTES_PER_PEL  3
#define F_DEFAULT_DESKTOP_BGCOLOR 0x606060

class F_ServerConfig : public F_Config {
public:
    /* Desktop */
    char deviceName[80];
    int  DesktopNx;
    int  DesktopNy;
    int  BytesPerPel;
    int  DesktopBackColor;
    char DesktopBackgroundPicture[_MAX_PATH];

    /* Session limits */
    ULONG ulMaxClients;
    ULONG ulMaxHabs;
    ULONG ulInactive;
    ULONG ulGrace;
    ULONG ulMonitorTick;
    BOOL  fAcceptLogs;

    /* HAB strict mode */
    BOOL  fStrictHab;

    F_ServerConfig()
    {
        strcpy(deviceName, F_DEFAULT_DEVICE);
        DesktopNx = F_DEFAULT_DESKTOP_NX;
        DesktopNy = F_DEFAULT_DESKTOP_NY;
        BytesPerPel = F_DEFAULT_BYTES_PER_PEL;
        DesktopBackColor = F_DEFAULT_DESKTOP_BGCOLOR;
        DesktopBackgroundPicture[0] = 0;

        ulMaxClients  = F_MAX_CLIENTS;
        ulMaxHabs     = F_MAX_HABS;
        ulInactive    = F_DEFAULT_INACTIVE;
        ulGrace       = F_DEFAULT_GRACE;
        ulMonitorTick = F_MONITOR_TICK_MS;
        fAcceptLogs   = TRUE;
        fStrictHab    = FALSE;
    }

    int AnalizeRecodrRead(char *name, char *par);
};
