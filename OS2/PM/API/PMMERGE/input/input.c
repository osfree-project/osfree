#define INCL_WIN
#include <os2.h>

#define debug(...)
/*** Keyboard and mouse input COMMON subsection *************************/

BOOL  APIENTRY WinSetFocus(HWND hwndDesktop,
                            HWND hwndSetFocus)
{
 debug(6, 2)( __FUNCTION__ "is not yet implemented\n");
 return 0;
}


BOOL  APIENTRY WinFocusChange(HWND hwndDesktop,
                              HWND hwndSetFocus,
                              ULONG flFocusChange)
{
 debug(6, 2)( __FUNCTION__ "is not yet implemented\n");
 return 0;
}

