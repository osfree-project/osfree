/* FreePM_ErrCodes.h */

#ifndef FREEPM_ERR_CODES_H
#define FREEPM_ERR_CODES_H

#include <os2.h>

#define PMERR_OK                          0x0000
#define PMERR_INVALID_HWND               0x1001

struct OS2_ErrMsg  FreePM_ErrMessages[]=
{
 PMERR_OK,      "NO_ERROR",
 PMERR_INVALID_HWND, "0x1001 An invalid window handle was specified",
 PMERR_NO_MSG_QUEUE, "0x1036 Message queue is not created (with WinCreateMsgQueue)",
 PMERR_INVALID_HAB, "0x104A  Invalid hab: there was no call to WinInitialize for current thread",
 PMERR_MSG_QUEUE_ALREADY_EXISTS, "0x1052 An attempt to create a message queue for a thread failed because a message queue already exists for the calling thread.",
 PMERR_ALREADY_INITIALIZED, "0x1403 Hab already initialized",
 FPMERR_NULL_POINTER,       "0x5001 NULL pointer in function call",
 FPMERR_NULL_WINDPROC,      "0x5002 window procedure is NULL pointer",
 FPMERR_INITSERVER_CONNECTION, "0x5101 Error init server connection",

0xffffffff, "Last error"
};

#endif
   /* FREEPM_ERR_CODES_H */

