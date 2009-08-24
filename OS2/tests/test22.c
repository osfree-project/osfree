#define INCL_WINLOAD            /* Window Load Functions        */
#include <os2.h>

void main(void)
{
PFNWP   pWndproc;       /* procedure pointer                    */
HAB     hab;            /* anchor-block handle                  */
HLIB    hlib;           /* library handle                       */
char    pszLibname[10]="RES.DLL"; /* library name string        */
char    pszProcname[10]="WndProc"; /* procedure name string     */

/* load RES.DLL */
hlib = WinLoadLibrary(hab, pszLibname);

/* load WndProc */
pWndproc = WinLoadProcedure(hab, hlib, pszProcname);
};
