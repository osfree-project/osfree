/*! @file Ft_main.cpp
    @brief TCP backend for FreePM: exported entry points.

    Per-connection model: Fc_TrnConnect / Fc_TrnDisconnect(obj) are
    paired on a per-TCP basis. No global connection state.

    Naming: Fc_Trn* = client transport, Fs_Trn* = server transport.
    Post* = async write (no wait), Send* = write + wait reply,
    Get* = blocking read.

    Cross-platform: WSA init/cleanup and error codes are hidden
    behind PMC_* macros in F_tcp.hpp.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <process.h>

#include "F_tcp.hpp"

#if defined(__WIN32__) || defined(__NT__) || defined(_WIN32)

/*! @brief Initialize Winsock for Win32.
    @return 0 on success, non-zero on error. */
extern "C" int PMC_WsaStartup(void)
{
    WSADATA wsa;
    return WSAStartup(MAKEWORD(1, 1), &wsa);
}

/*! @brief Clean up Winsock for Win32. */
extern "C" void PMC_WsaCleanup(void)
{
    WSACleanup();
}

#endif /* Win32 */

/* APIENTRY on Win32 expands to __declspec(__pragma(...)), which Watcom
   rejects inside pointer declarators (E671). Use the plain calling
   convention keyword there; on OS/2 APIENTRY (_System) is a real type
   modifier and works as-is. */
#if defined(__OS2__) || defined(__OS2V2__)
  /*! @def PMC_CB
      @brief Calling convention keyword for PMC callbacks (OS/2). */
  #define PMC_CB APIENTRY
#else
  /*! @def PMC_CB
      @brief Calling convention keyword for PMC callbacks (Win32). */
  #define PMC_CB __stdcall
#endif

/*! @brief Handler callback type, same as in PMPIPE. */
typedef void (PMC_CB *PMC_HANDLER)(void *obj, int cmd, int par, int threadNum);

/*! @brief Transport-level disconnect callback. Called by PMTCP's
    per-connection thread when the socket is gone (recv failed or
    returned EOF). Not a protocol command; no response is sent. */
typedef void (PMC_CB *PMC_DISCONNECT_CB)(void *obj);

/* Forward declarations, same for all platforms. */

/*! @brief Server-side blocking read of raw data.
    @param [in]  obj   Connection object.
    @param [out] buf   Destination buffer.
    @param [out] len   Receives the number of bytes read.
    @param [in]  size  Number of bytes to read (== buffer size).
    @return 0 on success, -1 on failure.
    @retval 0  Success.
    @retval -1 Failure or short read. */
extern "C" APIRET EXPENTRY Fs_TrnGetData(void *obj, void *buf, int *len, int size);

/*! @brief Server-side blocking read of a (command, par) pair.
    @param [in]  obj  Connection object.
    @param [out] cmd  Receives the command code.
    @param [out] par  Receives the 32-bit argument.
    @return 0 on success, -1 on failure.
    @retval 0  Success.
    @retval -1 Failure or short read. */
extern "C" APIRET EXPENTRY Fs_TrnGetCommand(void *obj, int *cmd, int *par);

/*! @brief Client-side blocking read of raw data.
    @param [in]  obj   Connection object.
    @param [out] buf   Destination buffer.
    @param [out] len   Receives the number of bytes read.
    @param [in]  size  Number of bytes to read (== buffer size).
    @return 0 on success, -1 on failure.
    @retval 0   Exactly size bytes were read.
    @retval -1  Receive failed, peer closed, or partial read. */
extern "C" APIRET EXPENTRY Fc_TrnGetData(void *obj, void *buf, int *len, int size);

/*! @brief Client-side async write of raw data (no wait for reply).
    @param [in] obj  Connection object.
    @param [in] buf  Payload.
    @param [in] len  Payload size in bytes.
    @return 0 on success, non-zero on transport error.
    @retval 0              Success.
    @retval PMC_ERR_BROKEN Send failed or connection is closed. */
extern "C" APIRET EXPENTRY Fc_TrnPostData(void *obj, void *buf, int len);

/*! @brief Client-side send of a command with a payload; wait for reply.
    @param [in] obj  Connection object.
    @param [in] cmd  Command code.
    @param [in] par  32-bit parameter.
    @param [in] buf  Payload.
    @param [in] len  Payload size in bytes.
    @return Server reply code, or (APIRET)-1 on transport error. */
extern "C" APIRET EXPENTRY Fc_TrnSendCommandData(void *obj, int cmd, int par,
                                                 void *buf, int len);

/*! @brief Client-side send of a command; wait for reply.
    @param [in] obj  Connection object.
    @param [in] cmd  Command code.
    @param [in] par  32-bit parameter.
    @return Server reply code, or (APIRET)-1 on transport error. */
extern "C" APIRET EXPENTRY Fc_TrnSendCommand(void *obj, int cmd, int par);

/*! @brief Client-side async write of a command (no wait for reply).
    @param [in] obj  Connection object.
    @param [in] cmd  Command code.
    @param [in] par  32-bit argument.
    @return 0 on success, non-zero on transport error.
    @retval 0              Success.
    @retval PMC_ERR_BROKEN Send failed or connection is closed. */
extern "C" APIRET EXPENTRY Fc_TrnPostCommand(void *obj, int cmd, int par);

/*! @brief Server-side async write of raw data (no wait for reply).
    @param [in] obj  Connection object.
    @param [in] buf  Payload.
    @param [in] len  Payload size in bytes.
    @return 0 on success, non-zero on transport error.
    @retval 0              Success.
    @retval PMC_ERR_BROKEN Send failed or connection is closed. */
extern "C" APIRET EXPENTRY Fs_TrnPostData(void *obj, void *buf, int len);

/*! @brief Default server host (overridable by Fc_TrnConnect). */
static char    PipeHost[256] = FREEPM_TCP_DEFAULT_HOST;
/*! @brief Server TCP port. */
static int     PipePort     = FREEPM_TCP_DEFAULT_PORT;

/*! @brief Open a client TCP connection and perform the handshake.

    *obj is always written: 0 on any error, non-zero on success.

    @param [in]  host  Server host name, or NULL for the default.
    @param [out] obj   Receives the connection object; set to 0 on
                       any error.
    @return 0 on success, non-zero on failure.
    @retval 0                  Connected.
    @retval PMC_ERR_OPEN       Connect failed.
    @retval HAND_SHAKE_ERROR   Handshake mismatch. */
extern "C" APIRET EXPENTRY Fc_TrnConnect(char *host, ULONG *obj)
{
    TCPipe *pNew;
    int     ctx;

    if (obj) *obj = 0;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_PMTCP, F_DBG_LVL_VERBOSE,
                         "PMTCP: Fc_TrnConnect(host=%s)",
                         host ? host : "NULL");

    if (PMC_WSA_STARTUP() != 0) {
        F_DbgPrint(F_DBG_SEC_PMTCP, F_DBG_LVL_ERROR,
                   "PMTCP: Fc_TrnConnect ERROR: WSAStartup failed");
        F_DbgCtxExitf(ctx, "WSAStartup failed");
        return PMC_ERR_OPEN;
    }

    if (host != NULL) {
        strncpy(PipeHost, host, sizeof(PipeHost) - 1);
        PipeHost[sizeof(PipeHost) - 1] = 0;
    }

    pNew = new TCPipe(PipeHost, PipePort, TCP_CLIENT_MODE);
    if (pNew == NULL) {
        F_DbgPrint(F_DBG_SEC_PMTCP, F_DBG_LVL_ERROR,
                   "PMTCP: Fc_TrnConnect ERROR: new TCPipe failed");
        PMC_WSA_CLEANUP();
        F_DbgCtxExitf(ctx, "new TCPipe failed");
        return PMC_ERR_OPEN;
    }

    if (pNew->Connect() != 0) {
        F_DbgPrint(F_DBG_SEC_PMTCP, F_DBG_LVL_ERROR,
                   "PMTCP: Fc_TrnConnect ERROR: Connect failed");
        delete pNew;                 /* ~TCPipe -> Close() -> closesocket */
        PMC_WSA_CLEANUP();
        F_DbgCtxExitf(ctx, "Connect failed");
        return PMC_ERR_OPEN;
    }

    if (pNew->HandShake() == HAND_SHAKE_ERROR) {
        F_DbgPrint(F_DBG_SEC_PMTCP, F_DBG_LVL_ERROR,
                   "PMTCP: Fc_TrnConnect ERROR: HandShake failed");
        pNew->Close();
        delete pNew;
        PMC_WSA_CLEANUP();
        F_DbgCtxExitf(ctx, "HandShake failed");
        return HAND_SHAKE_ERROR;
    }

    *obj = (ULONG)pNew;
    F_DbgCtxExitf(ctx, "Success (obj=%p)", pNew);
    return 0;
}

/*! @brief Close one specific client connection.

    NULL is a no-op (safe for callers that hold an already-reset handle).

    @param [in] obj  Connection handle from Fc_TrnConnect.
    @return 0.
    @retval 0  Always. */
extern "C" APIRET EXPENTRY Fc_TrnDisconnect(void *obj)
{
    int ctx;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_PMTCP, F_DBG_LVL_VERBOSE,
                         "PMTCP: Fc_TrnDisconnect(obj=%p)", obj);
    if (obj == NULL) {
        F_DbgCtxExitf(ctx, "NULL obj (no-op)");
        return 0;
    }
    delete (TCPipe *)obj;            /* ~TCPipe -> Close() -> closesocket */
    PMC_WSA_CLEANUP();
    F_DbgCtxExitf(ctx, "Success");
    return 0;
}

/*! @brief Client-side async write of a command (no wait for reply).
    @param [in] obj  Connection object.
    @param [in] cmd  Command code.
    @param [in] par  32-bit argument.
    @return 0 on success, non-zero on transport error.
    @retval 0              Success.
    @retval PMC_ERR_BROKEN Send failed or connection is closed. */
extern "C" APIRET EXPENTRY Fc_TrnPostCommand(void *obj, int cmd, int par)
{
    int  ctx;
    char buf[8];
    int  rc;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_WIRE, F_DBG_LVL_VERBOSE,
                         "PMTCP: Fc_TrnPostCommand(obj=%p, cmd=0x%x, par=0x%x)",
                         obj, cmd, par);
    if (!obj) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: Fc_TrnPostCommand ERROR: obj is NULL!");
        F_DbgCtxExitf(ctx, "obj is NULL");
        return PMC_ERR_BROKEN;
    }
    memcpy(buf,     &cmd, 4);
    memcpy(buf + 4, &par, 4);
    rc = ((TCPipe *)obj)->Send(buf, 8);
    if (rc != 0) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: Fc_TrnPostCommand ERROR: Send failed");
        F_DbgCtxExitf(ctx, "Send failed");
        return PMC_ERR_BROKEN;
    }
    F_DbgCtxExitf(ctx, "Success");
    return 0;
}

/*! @brief Client-side async write of raw data (no wait for reply).
    @param [in] obj  Connection object.
    @param [in] buf  Payload.
    @param [in] len  Payload size in bytes.
    @return 0 on success, non-zero on transport error.
    @retval 0              Success.
    @retval PMC_ERR_BROKEN Send failed or connection is closed. */
extern "C" APIRET EXPENTRY Fc_TrnPostData(void *obj, void *buf, int len)
{
    int ctx;
    int rc;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_WIRE, F_DBG_LVL_VERBOSE,
                         "PMTCP: Fc_TrnPostData(obj=%p, buf=%p, len=%d)",
                         obj, buf, len);
    if (!obj) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: Fc_TrnPostData ERROR: obj is NULL!");
        F_DbgCtxExitf(ctx, "obj is NULL");
        return PMC_ERR_BROKEN;
    }
    rc = ((TCPipe *)obj)->Send(buf, len);
    if (rc != 0) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: Fc_TrnPostData ERROR: Send failed");
        F_DbgCtxExitf(ctx, "Send failed");
        return PMC_ERR_BROKEN;
    }
    F_DbgCtxExitf(ctx, "Success");
    return 0;
}

/*! @brief Client-side blocking read of raw data.
    @param [in]  obj   Connection object.
    @param [out] buf   Destination buffer.
    @param [out] len   Receives the number of bytes read.
    @param [in]  size  Number of bytes to read (== buffer size).
    @return 0 on success, -1 on failure.
    @retval 0   Exactly size bytes were read.
    @retval -1  Receive failed, peer closed, or partial read. */
extern "C" APIRET EXPENTRY Fc_TrnGetData(void *obj, void *buf, int *len, int size)
{
    int ctx;
    int rc;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_WIRE, F_DBG_LVL_VERBOSE,
                         "PMTCP: Fc_TrnGetData(obj=%p, size=%d)", obj, size);
    if (!obj) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: Fc_TrnGetData ERROR: obj is NULL!");
        F_DbgCtxExitf(ctx, "obj is NULL");
        return -1;
    }
    rc = ((TCPipe *)obj)->Recv(buf, len, size);
    if (rc != 0) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: Fc_TrnGetData ERROR: Recv failed");
        F_DbgCtxExitf(ctx, "Recv failed");
        return -1;
    }
    F_DbgCtxExitf(ctx, "len=%d", *len);
    return 0;
}

/*! @brief Client-side send of a command; wait for reply.

    On transport error (send or recv failed) returns (APIRET)-1 so
    callers can distinguish it from a legitimate server reply.

    @param [in] obj  Connection object.
    @param [in] cmd  Command code.
    @param [in] par  32-bit parameter.
    @return Server reply code, or (APIRET)-1 on transport error. */
extern "C" APIRET EXPENTRY Fc_TrnSendCommand(void *obj, int cmd, int par)
{
    int ctx, rc, retcode, l;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_PMTCP, F_DBG_LVL_VERBOSE,
                         "PMTCP: Fc_TrnSendCommand(obj=%p, cmd=0x%x, par=0x%x)",
                         obj, cmd, par);
    if (!obj) {
        F_DbgPrint(F_DBG_SEC_PMTCP, F_DBG_LVL_ERROR,
                   "PMTCP: Fc_TrnSendCommand ERROR: obj is NULL!");
        F_DbgCtxExitf(ctx, "obj is NULL");
        return (APIRET)-1;
    }

    rc = Fc_TrnPostCommand(obj, cmd, par);
    if (rc) {
        F_DbgCtxExitf(ctx, "PostCommand failed");
        return (APIRET)-1;
    }

    rc = Fc_TrnGetData(obj, &retcode, &l, sizeof(retcode));
    if (rc) {
        F_DbgCtxExitf(ctx, "GetData failed");
        return (APIRET)-1;
    }

    F_DbgCtxExitf(ctx, "retcode=0x%x", retcode);
    return (APIRET)retcode;
}

/*! @brief Client-side send of a command with a payload; wait for reply.

    On transport error (send or recv failed) returns (APIRET)-1 so
    callers can distinguish it from a legitimate server reply.

    @param [in] obj  Connection object.
    @param [in] cmd  Command code.
    @param [in] par  32-bit parameter.
    @param [in] buf  Payload.
    @param [in] len  Payload size in bytes.
    @return Server reply code, or (APIRET)-1 on transport error. */
extern "C" APIRET EXPENTRY Fc_TrnSendCommandData(void *obj, int cmd, int par,
                                                 void *buf, int len)
{
    int ctx, rc, retcode, l;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_PMTCP, F_DBG_LVL_VERBOSE,
                         "PMTCP: Fc_TrnSendCommandData(obj=%p, cmd=0x%x, par=0x%x, len=%d)",
                         obj, cmd, par, len);
    if (!obj) {
        F_DbgPrint(F_DBG_SEC_PMTCP, F_DBG_LVL_ERROR,
                   "PMTCP: Fc_TrnSendCommandData ERROR: obj is NULL!");
        F_DbgCtxExitf(ctx, "obj is NULL");
        return (APIRET)-1;
    }

    rc = Fc_TrnPostCommand(obj, cmd, par);
    if (rc) {
        F_DbgCtxExitf(ctx, "PostCommand failed");
        return (APIRET)-1;
    }

    rc = Fc_TrnPostData(obj, buf, len);
    if (rc) {
        F_DbgCtxExitf(ctx, "PostData failed");
        return (APIRET)-1;
    }

    rc = Fc_TrnGetData(obj, &retcode, &l, sizeof(retcode));
    if (rc) {
        F_DbgCtxExitf(ctx, "GetData failed");
        return (APIRET)-1;
    }

    F_DbgCtxExitf(ctx, "retcode=0x%x", retcode);
    return (APIRET)retcode;
}

/*! @brief Server-side blocking read of a (command, par) pair.
    @param [in]  obj  Connection object.
    @param [out] cmd  Receives the command code.
    @param [out] par  Receives the 32-bit argument.
    @return 0 on success, -1 on failure.
    @retval 0  Success.
    @retval -1 Failure or short read. */
extern "C" APIRET EXPENTRY Fs_TrnGetCommand(void *obj, int *cmd, int *par)
{
    int  ctx;
    char buf[8];
    int  len = 0;
    int  rc;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_WIRE, F_DBG_LVL_VERBOSE,
                         "PMTCP: Fs_TrnGetCommand(obj=%p)", obj);
    if (!obj) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: Fs_TrnGetCommand ERROR: obj is NULL!");
        F_DbgCtxExitf(ctx, "obj is NULL");
        return -1;
    }
    rc = ((TCPipe *)obj)->Recv(buf, &len, 8);
    if (rc != 0 || len != 8) {
        int err = 0;
#if defined(__WIN32__) || defined(__NT__) || defined(_WIN32)
        err = WSAGetLastError();
#endif
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: Fs_TrnGetCommand ERROR: Recv failed or short read. "
                   "rc=%d, len=%d, WSAError=%d", rc, len, err);
        F_DbgCtxExitf(ctx, "Recv failed rc=%d, len=%d", rc, len);
        return -1;
    }
    memcpy(cmd, buf,     4);
    memcpy(par, buf + 4, 4);
    F_DbgCtxExitf(ctx, "cmd=0x%x, par=0x%x", *cmd, *par);
    return 0;
}

/*! @brief Server-side blocking read of raw data.
    @param [in]  obj   Connection object.
    @param [out] buf   Destination buffer.
    @param [out] len   Receives the number of bytes read.
    @param [in]  size  Number of bytes to read (== buffer size).
    @return 0 on success, -1 on failure.
    @retval 0  Success.
    @retval -1 Failure or short read. */
extern "C" APIRET EXPENTRY Fs_TrnGetData(void *obj, void *buf, int *len, int size)
{
    int ctx;
    int rc;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_WIRE, F_DBG_LVL_VERBOSE,
                         "PMTCP: Fs_TrnGetData(obj=%p, size=%d)", obj, size);
    if (!obj) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: Fs_TrnGetData ERROR: obj is NULL!");
        F_DbgCtxExitf(ctx, "obj is NULL");
        return -1;
    }
    rc = ((TCPipe *)obj)->Recv(buf, len, size);
    if (rc != 0) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: Fs_TrnGetData ERROR: Recv failed");
        F_DbgCtxExitf(ctx, "Recv failed");
        return -1;
    }
    if (*len != size) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: Fs_TrnGetData ERROR: short read len=%d, "
                   "expected %d", *len, size);
        F_DbgCtxExitf(ctx, "short read len=%d, expected %d", *len, size);
        return -1;
    }
    F_DbgCtxExitf(ctx, "len=%d", *len);
    return 0;
}

/*! @brief Server-side async write of raw data (no wait for reply).
    @param [in] obj  Connection object.
    @param [in] buf  Payload.
    @param [in] len  Payload size in bytes.
    @return 0 on success, non-zero on transport error.
    @retval 0              Success.
    @retval PMC_ERR_BROKEN Send failed or connection is closed. */
extern "C" APIRET EXPENTRY Fs_TrnPostData(void *obj, void *buf, int len)
{
    int ctx;
    int rc;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_WIRE, F_DBG_LVL_VERBOSE,
                         "PMTCP: Fs_TrnPostData(obj=%p, buf=%p, len=%d)",
                         obj, buf, len);
    if (!obj) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: Fs_TrnPostData ERROR: obj is NULL!");
        F_DbgCtxExitf(ctx, "obj is NULL");
        return PMC_ERR_BROKEN;
    }
    rc = ((TCPipe *)obj)->Send(buf, len);
    if (rc != 0) {
        int err = 0;
#if defined(__WIN32__) || defined(__NT__) || defined(_WIN32)
        err = WSAGetLastError();
#endif
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: Fs_TrnPostData ERROR: Send failed. "
                   "rc=%d, WSAError=%d", rc, err);
        F_DbgCtxExitf(ctx, "Send failed rc=%d", rc);
        return PMC_ERR_BROKEN;
    }
    F_DbgCtxExitf(ctx, "Success");
    return 0;
}

/*! @brief Per-connection context passed to ClientThreadProc. */
struct TCP_ClientArg {
    TCPipe           *pClient;
    PMC_HANDLER       h;
    PMC_DISCONNECT_CB disc;
    int               threadNum;
};

/*! @brief Server-side per-connection thread.

    Reads commands from one client connection until the socket closes
    or a zero command is received, dispatches each through the handler,
    then notifies the disconnect callback and cleans up.

    @param [in] param  Pointer to TCP_ClientArg (connection context). */
static void ClientThreadProc(void *param)
{
    TCP_ClientArg    *pArg    = (TCP_ClientArg *)param;
    TCPipe           *pClient = pArg->pClient;
    PMC_HANDLER       h       = pArg->h;
    PMC_DISCONNECT_CB disc    = pArg->disc;
    int               cmd, par;

    while (Fs_TrnGetCommand(pClient, &cmd, &par) == 0)
    {
        if (cmd == 0) break;
        if (h) h(pClient, cmd, par, pArg->threadNum);
    }

    /* Transport-level notification: socket is gone. Fs_Server
       transitions the session to DETACHED so a legitimate RECONNECT
       can resume it without waiting for ulInactive. The obj
       (pClient) is still valid here; it is deleted below. */
    if (disc) disc(pClient);

    pClient->Close();
    delete pClient;
    delete pArg;
}

/*! @brief Run the server-side listener: accept-loop + per-conn threads.

    Creates a listening socket, accepts one client at a time, performs
    the handshake, and spawns one ClientThreadProc per connection.
    Does not return until the process exits.

    @param [in] handler  Command handler callback.
    @param [in] disc_cb  Disconnect callback.
    @return 0 on success, non-zero on setup failure.
    @retval 0            Loop exited (normally unreachable).
    @retval PMC_ERR_OPEN Setup failed. */
extern "C" APIRET EXPENTRY Fs_TrnRunListener(void *handler, void *disc_cb)
{
    PMC_HANDLER       h    = (PMC_HANDLER)handler;
    PMC_DISCONNECT_CB disc = (PMC_DISCONNECT_CB)disc_cb;
    TCPipe           *pServer = new TCPipe();
    int               wsaRc;
    int               threadCounter = 0;

    if (!pServer) return PMC_ERR_OPEN;

    wsaRc = PMC_WSA_STARTUP();
    if (wsaRc != 0)
    {
        F_DbgPrint(F_DBG_SEC_PMTCP, F_DBG_LVL_ERROR,
                   "PMTCP: Fs_TrnRunListener: WSAStartup failed (rc=%d)", wsaRc);
        delete pServer;
        return PMC_ERR_OPEN;
    }

    pServer->mode = TCP_SERVER_MODE;
    pServer->port = FREEPM_TCP_DEFAULT_PORT;

    if (pServer->Create() != 0)
    {
        F_DbgPrint(F_DBG_SEC_PMTCP, F_DBG_LVL_ERROR,
                   "PMTCP: Fs_TrnRunListener: Create() failed (port %d)",
                   pServer->port);
        delete pServer;
        PMC_WSA_CLEANUP();
        return PMC_ERR_OPEN;
    }

    F_DbgPrint(F_DBG_SEC_PMTCP, F_DBG_LVL_VERBOSE,
               "PMTCP: Fs_TrnRunListener: listening on port %d (multi-thread)",
               pServer->port);

    for (;;)
    {
        TCPipe *pClient = new TCPipe();
        if (!pClient) continue;

        if (pServer->Accept(pClient) != 0)
        {
            delete pClient;
            continue;
        }

        F_DbgPrint(F_DBG_SEC_PMTCP, F_DBG_LVL_VERBOSE,
                   "PMTCP: accepted connection, pClient=%p", pClient);

        if (pClient->HandShake() == HAND_SHAKE_ERROR)
        {
            F_DbgPrint(F_DBG_SEC_PMTCP, F_DBG_LVL_ERROR,
                       "PMTCP: Fs_TrnRunListener: handshake failed, dropping");
            pClient->Close();
            delete pClient;
            continue;
        }

        TCP_ClientArg *pArg = new TCP_ClientArg;
        if (!pArg) {
            pClient->Close();
            delete pClient;
            continue;
        }
        pArg->pClient   = pClient;
        pArg->h         = h;
        pArg->disc      = disc;
        pArg->threadNum = threadCounter++;

#if defined(__OS2__) || defined(__OS2V2__)
    /* OS/2: 4 arguments — (start, stack, stack_size, arglist) */
    _beginthread(ClientThreadProc, NULL, FREEPM_TCP_THREAD_STACK, pArg);
#else
    /* Win32: 3 arguments — (start, stack_size, arglist) */
    _beginthread(ClientThreadProc, FREEPM_TCP_THREAD_STACK, pArg);
#endif
    }

    return 0; /* unreachable */
}
