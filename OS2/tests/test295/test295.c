#define INCL_GPIMETAFILES       /* Metafile functions           */

#define INCL_GPICONTROL         /* GPI control Functions        */

#include <os2.h>



HAB    hab;             /* anchor-block handle                  */

HPS    hps;             /* presentation space handle            */

HMF    hmf;             /* metafile handle                      */

HDC    hdc;             /* Device-context handle                */

HWND   hwnd;            /* window handle                        */

SIZEL  sizl={0,0};      /* use same page size as device         */

CHAR   szBuffer[80];    /* descriptive record buffer            */

LONG     lHits;         /* correlation/error indicator          */

/* play metafile options array */

LONG optArray[PMF_DEFAULTS+1] =

                          {0,LT_DEFAULT,0,LC_DEFAULT,RES_RESET,

                          SUP_SUPPRESS,CTAB_DEFAULT,CREA_DEFAULT,

                          DDEF_DEFAULT};



hmf = GpiLoadMetaFile(hab, "sample.met");



/* create window device context and presentation space,

   associating DC with the PS */

hdc = WinOpenWindowDC(hwnd);

hps = GpiCreatePS(hab, hdc, &sizl, PU_PELS | GPIA_ASSOC);



/* reset presentation space */

lHits = GpiPlayMetaFile(hps, hmf, 9L, optArray, (LONG *)0, 80L,

                        szBuffer);



/* display metafile in window (reset and

   suppress flags must be changed) */

optArray[PMF_SUPPRESS]=SUP_DEFAULT;

optArray[PMF_RESET]=RES_DEFAULT;

lHits = GpiPlayMetaFile(hps, hmf, 9L, optArray, (LONG *)0, 80L,

                        szBuffer);
