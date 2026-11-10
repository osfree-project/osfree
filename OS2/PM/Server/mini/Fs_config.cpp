/*
 $Id: Fs_config.cpp,v 1.1 2003/06/15 17:45:47 evgen2 Exp $
*/
/* Fs_config.cpp */
/* Server config: server-specific key handler. Parsing is shared with
   the client through F_Config (see F_config.h). */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Fs_config.hpp"

int F_ServerConfig::AnalizeRecodrRead(char *name, char *par)
{
    if (!strcmp(name, "device"))
        { sscanf(par, "%s",  deviceName);                 return 0; }
    if (!strcmp(name, "DesktopNx"))
        { sscanf(par, "%i",  &DesktopNx);                 return 0; }
    if (!strcmp(name, "DesktopNy"))
        { sscanf(par, "%i",  &DesktopNy);                 return 0; }
    if (!strcmp(name, "BytesPerPel"))
        { sscanf(par, "%i",  &BytesPerPel);               return 0; }
    if (!strcmp(name, "DesktopBackColor"))
        { sscanf(par, "%x",  &DesktopBackColor);          return 0; }
    if (!strcmp(name, "DesktopBackgroundPicture"))
        { sscanf(par, "%s",  DesktopBackgroundPicture);   return 0; }
    if (!strcmp(name, "MaxClients"))
        { sscanf(par, "%lu", &ulMaxClients);              return 0; }
    if (!strcmp(name, "MaxHabs"))
        { sscanf(par, "%lu", &ulMaxHabs);                 return 0; }
    if (!strcmp(name, "Inactive"))
        { sscanf(par, "%lu", &ulInactive);                return 0; }
    if (!strcmp(name, "Grace"))
        { sscanf(par, "%lu", &ulGrace);                   return 0; }
    if (!strcmp(name, "MonitorTick"))
        { sscanf(par, "%lu", &ulMonitorTick);             return 0; }
    if (!strcmp(name, "AcceptLogs"))
        { sscanf(par, "%i",  &fAcceptLogs);               return 0; }
    if (!strcmp(name, "StrictHab"))
        { sscanf(par, "%i",  &fStrictHab);                return 0; }
    return F_Config::AnalizeRecodrRead(name, par);
}
