/*
 $Id: F_WinStartApp.cpp,v 1.2 2002/10/10 13:40:19 evgen2 Exp $
*/


#include "FreePM.hpp"
 #include "F_OS2.hpp"
#include "F_win.hpp"
#include "F_shl.hpp"
/*
 This function starts an application.
*/

// #define F_INCL_WINWINDOWMGR /* Or use INCL_WIN, INCL_PM, */
// #include <os2.h>


#ifdef __cplusplus
      extern "C"
#endif
HAPP APIENTRY WinStartApp(HWND hwndNotify,     /*  Notification-window handle. */
                          PPROGDETAILS pDetails,/*  Input parameters for the application to be started. */
                          PCSZ  pszParams,     /*  Start data.         */
                          PVOID Reserved,      /*  Option indicators.  */
                          ULONG fbOptions)     /*  Application handle. */
{
//todo
return 0;
}



