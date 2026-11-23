/* FreePM Client config */

#include "F_config.h"

#define _MAX_FNAME 256

class F_ClientConfig : public F_Config {
public:
    char ExternMachineName[_MAX_FNAME];
    char BackendName[_MAX_FNAME];

    F_ClientConfig()
    {
        ExternMachineName[0] = 0;
        BackendName[0] = 0;
    }

    int AnalizeRecodrRead(char *name, char *par);
};
