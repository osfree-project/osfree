/*+---------------------------------+*/
/*| Interface with pmclient.dll     |*/
/*+---------------------------------+*/

#include <FreePM_cmd.hpp>
#include <f_Err.h>

#ifdef __cplusplus
      extern "C" {
#endif

APIRET APIENTRY InitServerConnection(char *remotemachineName);
APIRET APIENTRY CloseServerConnection(void);
APIRET APIENTRY _F_SendCmdToServer(int cmd, int data);
APIRET APIENTRY _F_SendDataToServer(void *data, int len);
APIRET APIENTRY _F_RecvDataFromServer(void *data, int *len, int maxlen);
APIRET APIENTRY _F_SendGenCmdToServer(int cmd, int par);
APIRET APIENTRY _F_SendGenCmdDataToServer(int cmd, int par, void *data, int datalen);
void APIENTRY fatal(const char *message);

#ifdef __cplusplus
  }
#endif
