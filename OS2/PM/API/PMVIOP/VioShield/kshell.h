#ifndef __KSHELL_H_
#define __KSHELL_H_

#define INCL_VIO
#include <os2.h>

#include "cpdlg.h"

#define WC_KSHELL   "KShellClass"

#define ID_KSHELL   1

#define IDM_CODEPAGE    0x01
#define IDM_FONT        0x02

#define ID_POPUP        2
#define IDM_COPY        0x203
#define IDM_COPYALL     0x204
#define IDM_PASTE       0x205
#define IDM_FT2LIB      0x210

#define VIO_CELLSIZE    2

#define KSHELL_BUFSIZE  32768

#define MEM_KSHELL_VIOBUF_LEN   64
#define MEM_KSHELL_VIOBUF_BASE  "\\SHAREMEM\\KSHELL\\VIOBUF\\"

#define PIPE_KSHELL_VIOSUB_LEN  64
#define PIPE_KSHELL_VIOSUB_BASE "\\PIPE\\KSHELL\\VIOSUB\\"

#define SEM_KSHELL_VIODMN_LEN   64
#define SEM_KSHELL_VIODMN_BASE  "\\SEM32\\KSHELL\\VIODMN\\"

#define PM_SC_LSHIFT    0x2A
#define PM_SC_RSHIFT    0x36
#define PM_SC_LCTRL     0x1D
#define PM_SC_RCTRL     0x5B
#define PM_SC_LALT      0x38
#define PM_SC_RALT      0x5E

#define KSHELLM_INITFRAME   ( WM_USER + 1 )

#endif
