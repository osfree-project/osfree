#ifndef __VIODMN_H_
#define __VIODMN_H_

#define VIODMN_MAGIC    "VioDmn_should_be_run_by_KShell_!!!"

#define MSG_PIPE_SIZE   4   /* ULONG */

#define MSG_CURINFO 0x0001
#define MSG_CHAR    0x0002
#define MSG_VIOINFO 0x0003
#define MSG_SGID    0x0004

#define MSG_QUIT    0x0013

#define MSG_DONE    0xFFFF

#define PIPE_VIODMN_LEN   64
#define PIPE_VIODMN_BASE  "\\PIPE\\VIODMN\\"

#define SEM_VIODMN_KSHELL_LEN   64
#define SEM_VIODMN_KSHELL_BASE  "\\SEM32\\VIODMN\\KSHELL\\"

#endif

