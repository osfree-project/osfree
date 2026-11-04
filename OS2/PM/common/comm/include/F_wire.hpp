/*! @file F_wire.hpp
    @brief Wire-protocol layer shared by all transports.

    Implements the _F_* exports (fixed 8-byte header, raw payload) on
    top of F_Link. Also owns the per-connection dispatcher thread and
    the server accept loop.
*/
#ifndef F_WIRE_HPP
#define F_WIRE_HPP

#include "F_platform.h"
#include "F_link.hpp"

#if defined(__OS2__) || defined(__OS2V2__)
  #define PMC_CB APIENTRY
#else
  #define PMC_CB __stdcall
#endif

/*! @brief Command handler, invoked once per received header. */
typedef void (PMC_CB *PMC_HANDLER)(void *obj, int ncmd, int data, int threadNum);

/*! @brief Transport-level disconnect callback. Invoked after the
    receive loop exits, before the F_Link is destroyed. */
typedef void (PMC_CB *PMC_DISCONNECT_CB)(void *obj);

/*! @brief Shutdown callback passed by PMCLIENT. */
typedef void (PMC_CB *PMC_SHUTDOWN_CB)(void);

/*! @brief Process-wide client connection (set by _InitServerConnection). */
extern F_Link *g_pClientLink;

/*! @brief Registered shutdown callback (invoked from DllMain detach). */
extern PMC_SHUTDOWN_CB g_shutdownCb;

/*! @brief Server-side accept loop. Blocks until the listener fails
    permanently. For each accepted connection spawns a dispatcher
    thread that runs the receive loop. */
void F_RunServer(F_Link *pListener, PMC_HANDLER h, PMC_DISCONNECT_CB disc);

#ifdef __cplusplus
extern "C" {
#endif

APIRET EXPENTRY _F_SendCmdToServer(void *recvobj, int ncmd, int data);
APIRET EXPENTRY _F_SendDataToServer(void *recvobj, void *data, int len);
APIRET EXPENTRY _F_RecvDataFromServer(void *recvobj, void *data, int *len, int maxlen);
APIRET EXPENTRY _F_SendGenCmdToServer(void *recvobj, int cmd, int par);
APIRET EXPENTRY _F_SendGenCmdDataToServer(void *recvobj, int cmd, int par, void *data, int datalen);

APIRET EXPENTRY _F_RecvCmdFromClient(void *recvobj, int *ncmd, int *data);
APIRET EXPENTRY _F_RecvDataFromClient(void *recvobj, void *sqmsg, int *l, int size);
APIRET EXPENTRY _F_SendDataToClient(void *recvobj, void *data, int len);

#ifdef __cplusplus
}
#endif

#endif /* F_WIRE_HPP */
