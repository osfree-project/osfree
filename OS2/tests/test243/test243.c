#define INCL_GPILCIDS           /* Font functions               */

#define INCL_GPIPRIMITIVES      /* GPI primitive functions      */

#include <os2.h>
#include <string.h>

int main(VOID)
{

HPS hps;                /* presentation space handle            */

POINTL ptl = { 100, 100 };

FATTRS fat;



fat.usRecordLength = sizeof(FATTRS); /* sets size of structure   */

fat.fsSelection = 0;         /* uses default selection           */

fat.lMatch = 0L;             /* does not force match             */

fat.idRegistry = 0;          /* uses default registry            */

fat.usCodePage = 850;        /* code-page 850                    */

fat.lMaxBaselineExt = 12L;   /* requested font height is 12 pels */

fat.lAveCharWidth = 12L;     /* requested font width is 12 pels  */

fat.fsType = 0;              /* uses default type                */

fat.fsFontUse = FATTR_FONTUSE_NOMIX;/* doesn't mix with graphics */



/* Copy Courier to szFacename field */



strcpy(fat.szFacename ,"Courier");



GpiCreateLogFont(hps,        /* presentation space               */

                 NULL,       /* does not use logical font name   */

                 1L,         /* local identifier                 */

                 &fat);      /* structure with font attributes   */



GpiSetCharSet(hps, 1L);      /* sets font for presentation space */

GpiCharStringAt(hps, &ptl, 5L, "Hello"); /* displays a string    */

return 0;
}
