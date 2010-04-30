#define INCL_GPIMETAFILES       /* Metafile functions           */

#include <os2.h>



BOOL     fSuccess;      /* success indicator                    */

HMF      hmf;           /* metafile handle                      */

HAB hab;                /* anchor block handle                  */



/* loads metafile from disk */

hmf = GpiLoadMetaFile(hab, "sample.met");

    .

    .

    .



fSuccess = GpiDeleteMetaFile(hmf);
