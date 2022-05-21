#define INCL_GPIMETAFILES       /* Metafile functions           */

#include <os2.h>

int main(VOID)
{

HAB hab;                /* anchor block handle                  */

HMF hmf;                /* metafile handle                      */



/* loads metafile from disk */

hmf = GpiLoadMetaFile(hab, "sample.met");

//    .

//    .

//    .

GpiDeleteMetaFile(hmf);             /* deletes metafile         */

return 0;
}
