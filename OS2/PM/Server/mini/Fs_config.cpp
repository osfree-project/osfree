/*!
 * @file Fs_config.cpp
 * @brief Server-specific key handler for F_ServerConfig.
 *
 * Parsing itself lives in F_Config (F_config.h); only the
 * server-specific keys are handled here. The shared "debugOptions"
 * key is processed by F_Config::AnalizeRecodrRead.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Fs_config.hpp"

/*!
 * @brief Apply one parsed server-specific key to this configuration.
 *
 * Recognised keys: device, DesktopNx, DesktopNy, BytesPerPel,
 * DesktopBackColor, DesktopBackgroundPicture, MaxClients, MaxHabs,
 * Inactive, Grace, MonitorTick, AcceptLogs, StrictHab. Unknown keys
 * are forwarded to F_Config::AnalizeRecodrRead (handles the shared
 * "debugOptions" key).
 *
 * @param [in] name  Key name.
 * @param [in] par   Value string.
 * @return 0 if the key was recognised and applied, 1 otherwise.
 * @retval 0  A recognised key was applied.
 * @retval 1  The key is not recognised.
 */
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
