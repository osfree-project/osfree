#define INCL_GPIPRIMITIVES      /* GPI primitive functions      */

#include <os2.h>

int main(VOID)
{

HPS hps;                /* presentation space handle            */

SIZEL sizl = { 8, 8 };  /* image is 8 pels wide by 8 pels high  */

BYTE abImage[] = { 0x00, 0x18, 0x3c, 0x7e, 0xff,

    0xff, 0x7e, 0x3c, 0x18, 0x00 }; /* image data               */



GpiImage(hps, 0L, &sizl, 8L, abImage);       /* draws the image */

return 0;
}
