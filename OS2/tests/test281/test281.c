#define INCL_GPILCIDS           /* Font functions               */

#include <os2.h>

int main(VOID)
{

HPS hps;                /* presentation space handle            */

HAB hab;                /* anchor-block handle                  */

LONG cFonts = 0L;       /* font count                           */

LONG remFonts;          /* fonts not returned                   */



GpiLoadFonts(hab, "C:\\HELV.FON");



remFonts = GpiQueryFonts(hps, QF_PRIVATE, NULL, &cFonts, 0L, NULL);

return 0;
}
