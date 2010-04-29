#define INCL_GPIBITMAPS         /* Bit map functions            */

#define INCL_DEV                /* Device Function definitions  */

#define INCL_GPICONTROL         /* GPI control Functions        */

#define INCL_WINWINDOWMGR       /* Window Manager Functions     */

#include <os2.h>



HAB     hab;            /* anchor-block handle                  */

HPS    hpsMemory;       /* presentation-space handle            */

HPS    hpsScreen;       /* presentation-space handle            */

HDC    hdcScreen;       /* Device-context handle                */

HDC    hdcMemory;       /* Device-context handle                */

SIZEL  sizl={0, 0};     /* use same page size as device         */

/* context data structure */

DEVOPENSTRUC dop = {0L, "DISPLAY", NULL, 0L, 0L, 0L, 0L, 0L, 0L};

POINTL aptl[4] = {

    300, 400,           /* lower-left corner of target          */

    350, 450,           /* upper-right corner of target         */

    0, 0,               /* lower-left corner of source          */

    100, 100 };         /* upper-right corner of source         */

HWND   hwnd;



/* create memory device context and presentation space, associating

   DC with the PS */

hdcMemory = DevOpenDC(hab, OD_MEMORY, "*", 5L, (PDEVOPENDATA)&dop,

                      NULLHANDLE);

hpsMemory = GpiCreatePS(hab, hdcMemory, &sizl, GPIA_ASSOC

                                               | PU_PELS);



/* create window device context and presentation space, associating

   DC with the PS */

hdcScreen = WinOpenWindowDC(hwnd); /* Open window device context */

hpsScreen = GpiCreatePS(hab, hdcScreen, &sizl, PU_PELS | GPIF_LONG

                                               | GPIA_ASSOC);





/*

  .

  . get bit map, associate bit map with memory device context,

  .     draw into bit map

  .

  */



/* display the bit map on the screen by copying it from the memory

   device context into the screen device context */

GpiBitBlt(hpsScreen, hpsMemory, 4L, aptl, ROP_SRCCOPY, BBO_IGNORE);
