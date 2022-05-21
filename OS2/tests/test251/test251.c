#define INCL_GPIMETAFILES       /* Metafile functions           */

#include <os2.h>

int main(VOID)
{

BOOL     fSuccess;      /* success indicator                    */

HMF      hmf;           /* metafile handle                      */

HAB hab;                /* anchor block handle                  */



/* loads metafile from disk */

hmf = GpiLoadMetaFile(hab, "sample.met");

//    .

//    .

//    .



fSuccess = GpiDeleteMetaFile(hmf);

return 0;
}
