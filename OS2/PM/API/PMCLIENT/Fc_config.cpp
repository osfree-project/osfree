/*! @file Fc_config.cpp
    @brief Client-side configuration reader for fpm.ini.

    Only the client-specific key handler lives here; parsing is shared
    with the server through F_Config (see F_config.h). Recognised keys:
    remote, backend. The shared key debugOptions is handled by the
    F_Config base class.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "Fc_config.hpp"

/*! @brief Apply one parsed key to this configuration object.

    Recognised client-specific keys: "remote", "backend". Unknown keys
    are forwarded to F_Config::AnalizeRecodrRead (handles the shared
    "debugOptions" key).

    @param [in] name  Key name.
    @param [in] par   Value string.
    @return 0 if the key was recognised and applied, 1 otherwise.
    @retval 0  A recognised key was applied.
    @retval 1  The key is not recognised. */
int F_ClientConfig::AnalizeRecodrRead(char *name, char *par)
{
    if (!strcmp(name, "remote")) {
        sscanf(par, "%s", ExternMachineName);
        return 0;
    }
    if (!strcmp(name, "backend")) {
        sscanf(par, "%s", BackendName);
        return 0;
    }
    return F_Config::AnalizeRecodrRead(name, par);
}
