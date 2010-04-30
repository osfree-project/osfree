#define INCL_GPIMETAFILES       /* Metafile functions           */

#include <os2.h>



HAB hab;                /* anchor block handle                  */

HMF hmf, hmf2;          /* metafile handle                      */



/* loads metafile from disk */

hmf = GpiLoadMetaFile(hab, "sample.met");

    .

    .

    .

hmf2 = GpiCopyMetaFile(hmf);          /* copy the metafile       */
