#define INCL_KBD

#include <os2.h>



PVOID     Data;    /*  Pointer to event data. */

PULONG    Kind;    /*  Kind of event returned. */

APIRET    return;  /*  Return code. */



return = KbdGetConsole(Data, Kind, Flag, hkbd);
