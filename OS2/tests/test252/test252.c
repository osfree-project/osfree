#define INCL_GPILOGCOLORTABLE   /* Color Table functions        */

#include <os2.h>

int main(VOID)
{

BOOL     fSuccess;      /* success indicator                    */

HPAL     hpal;          /* palette handle                       */

HPS      hps;           /* Presentation-space handle            */



/* get handle of currently associated palette */

hpal = GpiQueryPalette(hps);



/* delete palette */

fSuccess = GpiDeletePalette(hpal);

return 0;
}
