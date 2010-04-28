#define INCL_KBD

#include <os2.h>



PKBDHWID    pkbdhwid;  /*  Pointer to hardware ID. */

APIRET      return;    /*  Return code. */



return = KbdGetHWID(pkbdhwid, hkbd);
