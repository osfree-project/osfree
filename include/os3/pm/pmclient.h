/*+---------------------------------+*/
/*| Interface with pmclient.dll     |*/
/*+---------------------------------+*/

#include <FreePM_cmd.hpp>
#include <f_Err.h>

#ifdef __cplusplus
      extern "C" {
#endif

//void *_client_obj;
//void *_server_obj;
//ULONG _num_threads;
APIRET APIENTRY _InitServerConnection(char *remotemachineName);
APIRET APIENTRY _CloseServerConnection(void);
APIRET APIENTRY _startServerThreads(void *handl);
APIRET APIENTRY _F_SendCmdToServer(void *obj, int cmd, int data);
APIRET APIENTRY _F_SendDataToServer(void *obj, void *data, int len);
APIRET APIENTRY _F_RecvDataFromServer(void *obj, void *data, int *len, int maxlen);
APIRET APIENTRY _F_SendGenCmdToServer(void *obj, int cmd, int par);
APIRET APIENTRY _F_SendGenCmdDataToServer(void *obj, int cmd, int par, void *data, int datalen);
APIRET APIENTRY _F_RecvCmdFromClient(void *obj, int *ncmd, int *data);
APIRET APIENTRY _F_RecvDataFromClient(void *obj, void *sqmsg, int *l, int size);
void APIENTRY   _fatal(const char *message);

#ifdef __cplusplus
  }
#endif

extern "C" APIRET _client_obj;
extern "C" APIRET _server_obj;
extern "C" ULONG  _num_threads;
extern APIRET APIENTRY (*InitServerConnection)(char *remotemachineName);
extern APIRET APIENTRY (*CloseServerConnection)(void);
extern APIRET APIENTRY (*startServerThreads)(void *handl);
extern APIRET APIENTRY (*F_SendCmdToServer)(void *obj, int cmd, int data);
extern APIRET APIENTRY (*F_SendDataToServer)(void *obj, void *data, int len);
extern APIRET APIENTRY (*F_RecvDataFromServer)(void *obj, void *data, int *len, int maxlen);
extern APIRET APIENTRY (*F_SendGenCmdToServer)(void *obj, int cmd, int par);
extern APIRET APIENTRY (*F_SendGenCmdDataToServer)(void *obj, int cmd, int par, void *data, int datalen);
extern APIRET APIENTRY (*F_RecvCmdFromClient)(void *obj, int *ncmd, int *data);
extern APIRET APIENTRY (*F_RecvDataFromClient)(void *obj, void *sqmsg, int *l, int size);
extern void APIENTRY   (*fatal)(const char *message);
