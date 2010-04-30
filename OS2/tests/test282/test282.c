#define INCL_GPIMETAFILES       /* Metafile functions           */

#include <os2.h>



HAB hab;                /* anchor block handle                  */

HMF hmf;                /* metafile handle                      */



/* loads metafile from disk */

hmf = GpiLoadMetaFile(hab, "sample.met");

    .

    .

    .

GpiDeleteMetaFile(hmf);             /* deletes metafile         */
