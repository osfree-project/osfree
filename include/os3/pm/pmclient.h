/*+---------------------------------+*/
/*| Interface with PM client        |*/
/*+---------------------------------+*/

#include <FreePM_cmd.hpp>
#include <f_err.h>

#ifndef FREEPMDEF
#include <F_def.hpp>
#endif

#include <os2def.h>

#ifdef __cplusplus
      extern "C" {
#endif

//ULONG _client_obj;
//ULONG _server_obj;
//ULONG _num_threads;
extern "C" APIRET client_obj;
extern "C" APIRET server_obj;
extern "C" APIRET num_threads;
extern "C" APIRET APIENTRY (*InitServerConnection)(char *remotemachineName, ULONG *obj);
extern "C" APIRET APIENTRY (*CloseServerConnection)(void);
extern "C" APIRET APIENTRY (*startServerThreads)(void *handl);
extern "C" APIRET APIENTRY (*F_SendCmdToServer)(ULONG obj, int cmd, int data);
extern "C" APIRET APIENTRY (*F_SendDataToServer)(ULONG obj, void *data, int len);
extern "C" APIRET APIENTRY (*F_RecvDataFromServer)(ULONG obj, void *data, int *len, int maxlen);
extern "C" APIRET APIENTRY (*F_SendGenCmdToServer)(ULONG obj, int cmd, int par);
extern "C" APIRET APIENTRY (*F_SendGenCmdDataToServer)(ULONG obj, int cmd, int par, void *data, int datalen);
extern "C" APIRET APIENTRY (*F_RecvCmdFromClient)(ULONG obj, int *ncmd, int *data);
extern "C" APIRET APIENTRY (*F_RecvDataFromClient)(ULONG obj, void *sqmsg, int *l, int size);
#ifndef FPM_SERVER
extern "C" void APIENTRY   (*fatal)(const char *message);
extern "C" void APIENTRY   (*db_print)(const char *format,...);
extern "C" APIRET FreePM_db_level;
extern "C" APIRET FreePM_debugLevels[MAX_DEBUG_SECTIONS];
#endif
#ifdef __cplusplus
  }
#else

//ULONG  client_obj;
//ULONG  server_obj;
//ULONG  num_threads;
#endif
//extern APIRET APIENTRY (*InitServerConnection)(char *remotemachineName, ULONG *obj);
//extern APIRET APIENTRY (*CloseServerConnection)(void);
//extern APIRET APIENTRY (*startServerThreads)(void *handl);
//extern APIRET APIENTRY (*F_SendCmdToServer)(ULONG obj, int cmd, int data);
//extern APIRET APIENTRY (*F_SendDataToServer)(ULONG obj, void *data, int len);
//extern APIRET APIENTRY (*F_RecvDataFromServer)(ULONG obj, void *data, int *len, int maxlen);
//extern APIRET APIENTRY (*F_SendGenCmdToServer)(ULONG obj, int cmd, int par);
//extern APIRET APIENTRY (*F_SendGenCmdDataToServer)(ULONG obj, int cmd, int par, void *data, int datalen);
//extern APIRET APIENTRY (*F_RecvCmdFromClient)(ULONG obj, int *ncmd, int *data);
//extern APIRET APIENTRY (*F_RecvDataFromClient)(ULONG obj, void *sqmsg, int *l, int size);
//extern void APIENTRY   (*fatal)(const char *message);
//extern void APIENTRY   (*db_print)(const char *format,...);
//extern APIRET FreePM_db_level;
//extern APIRET FreePM_debugLevels[MAX_DEBUG_SECTIONS];
