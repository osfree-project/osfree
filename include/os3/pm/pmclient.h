/*+---------------------------------+*/
/*| Interface with pmclient.dll     |*/
/*+---------------------------------+*/

#include <FreePM_cmd.hpp>
#include <f_Err.h>

#ifdef __cplusplus
      extern "C" {
#endif

APIRET APIENTRY _InitServerConnection(char *remotemachineName);
APIRET APIENTRY _CloseServerConnection(void);
APIRET APIENTRY _F_SendCmdToServer(int cmd, int data);
APIRET APIENTRY _F_SendDataToServer(void *data, int len);
APIRET APIENTRY _F_RecvDataFromServer(void *data, int *len, int maxlen);
APIRET APIENTRY _F_SendGenCmdToServer(int cmd, int par);
APIRET APIENTRY _F_SendGenCmdDataToServer(int cmd, int par, void *data, int datalen);
APIRET APIENTRY _F_RecvCmdFromClient(void *recvobj, int *ncmd, int *data);
void APIENTRY   _fatal(const char *message);

#ifdef __cplusplus
  }
#endif

extern APIRET APIENTRY (*InitServerConnection)(char *remotemachineName);
extern APIRET APIENTRY (*CloseServerConnection)(void);
extern APIRET APIENTRY (*F_SendCmdToServer)(int cmd, int data);
extern APIRET APIENTRY (*F_SendDataToServer)(void *data, int len);
extern APIRET APIENTRY (*F_RecvDataFromServer)(void *data, int *len, int maxlen);
extern APIRET APIENTRY (*F_SendGenCmdToServer)(int cmd, int par);
extern APIRET APIENTRY (*F_SendGenCmdDataToServer)(int cmd, int par, void *data, int datalen);
extern APIRET APIENTRY (*F_RecvCmdFromClient)(void *recvobj, int *ncmd, int *data);
extern void APIENTRY   (*fatal)(const char *message);
