/*! @file Ft_main.cpp
    @brief TCP backend for FreePM: exported entry points.

    Same ordinals and same signatures as PMPIPE (Fc_main.cpp).
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
  #define PMC_CB APIENTRY
#else
  #define PMC_CB __stdcall
#endif

/*! @brief Handler callback type, same as in PMPIPE. */
typedef void (PMC_CB *PMC_HANDLER)(void *obj, int ncmd, int data, int threadNum);

/*! @brief Transport-level disconnect callback. Called by PMTCP's
    per-connection thread when the socket is gone (recv failed or
    returned EOF). Not a protocol command; no response is sent. */
typedef void (PMC_CB *PMC_DISCONNECT_CB)(void *obj);

/* Forward declarations, same for all platforms. */
/*! @brief Server-side receive of a message buffer.
    @param [in]  recvobj  Connection object.
    @param [out] sqmsg    Destination buffer.
    @param [out] l        Receives the number of bytes read.
    @param [in]  size     Size of the destination buffer.
    @return 0 on success, -1 on failure. */
extern "C" APIRET EXPENTRY _F_RecvDataFromClient(void *recvobj, void *sqmsg, int *l, int size);

/*! @brief Server-side receive of a (cmd, data) pair.
    @param [in]  recvobj  Connection object.
    @param [out] ncmd     Receives the command code.
    @param [out] data     Receives the 32-bit argument.
    @return 0 on success, -1 on failure. */
extern "C" APIRET EXPENTRY _F_RecvCmdFromClient(void *recvobj, int *ncmd, int *data);

/*! @brief Client-side receive of raw data.
    @param [in]  recvobj  Connection object.
    @param [out] data     Destination buffer.
    @param [out] len      Receives the number of bytes read.
    @param [in]  maxlen   Size of the destination buffer.
    @return 0 on success, -1 on failure. */
extern "C" APIRET EXPENTRY _F_RecvDataFromServer(void *recvobj, void *data, int *len, int maxlen);

/*! @brief Client-side send of raw data.
    @param [in] recvobj  Connection object.
    @param [in] data     Payload.
    @param [in] len      Payload size in bytes.
    @return 0 on success, ERROR_BROKEN_PIPE on failure. */
extern "C" APIRET EXPENTRY _F_SendDataToServer(void *recvobj, void *data, int len);

/*! @brief Client-side send of a command with a payload.
    @param [in] recvobj  Connection object.
    @param [in] cmd      Command code.
    @param [in] par      32-bit parameter.
    @param [in] data     Payload.
    @param [in] datalen  Payload size in dwords.
    @return Server reply code, or the transport error code. */
extern "C" APIRET EXPENTRY _F_SendGenCmdDataToServer(void *recvobj, int cmd, int par, void *data, int datalen);

/*! @brief Client-side send of a generic command.
    @param [in] recvobj  Connection object.
    @param [in] cmd      Command code.
    @param [in] par      32-bit parameter.
    @return Server reply code, or the transport error code. */
extern "C" APIRET EXPENTRY _F_SendGenCmdToServer(void *recvobj, int cmd, int par);

/*! @brief Client-side send of a command.
    @param [in] recvobj  Connection object.
    @param [in] ncmd     Command code.
    @param [in] data     32-bit argument.
    @return 0 on success, ERROR_BROKEN_PIPE on failure. */
extern "C" APIRET EXPENTRY _F_SendCmdToServer(void *recvobj, int ncmd, int data);

/*! @brief Server-side send of raw data.
    @param [in] recvobj  Connection object.
    @param [in] data     Payload.
    @param [in] len      Payload size in bytes.
    @return 0 on success, ERROR_BROKEN_PIPE on failure. */
extern "C" APIRET EXPENTRY _F_SendDataToClient(void *recvobj, void *data, int len);

/*! @brief Client connection object, exported for the debug module. */
class TCPipe *pF_pipe = NULL;
/*! @brief Host of the server we are connected to. */
static char    PipeHost[256] = FREEPM_TCP_DEFAULT_HOST;
/*! @brief Port of the server we are connected to. */
static int     PipePort     = FREEPM_TCP_DEFAULT_PORT;

/*! @brief Function-pointer type for the shutdown callback passed by
    PMCLIENT through _InitServerConnection. */
typedef void (PMC_CB *PMC_SHUTDOWN_CB)(void);

/*! @brief Callback registered by PMCLIENT; invoked from
    DllMain(DETACH) before PMTCP's CRT is torn down, while both DLLs
    are fully alive. */
static PMC_SHUTDOWN_CB g_shutdownCb = NULL;

/*! @brief Open a client TCP connection and perform the handshake.
    @param [in]  ExternMachine  Server host name, or NULL for the default.
    @param [out] obj            Receives the connection object.
    @param [in]  cb             Shutdown callback invoked from
                                DllMain(DETACH) before this DLL's CRT is
                                torn down. May be NULL.
    @return 0 on success, ERROR_OPEN_FAILED on connect failure, HAND_SHAKE_ERROR on mismatch.
    @retval 0                  Connected.
    @retval PMC_ERR_OPEN       Connect failed.
    @retval HAND_SHAKE_ERROR   Handshake mismatch. */
extern "C" APIRET EXPENTRY _InitServerConnection(char *ExternMachine, ULONG *obj,
                                                 void *cb)
{
    int ctx;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_PMTCP, F_DBG_LVL_VERBOSE,
                         "PMTCP: _InitServerConnection(ExternMachine=%s, cb=%p)",
                         ExternMachine ? ExternMachine : "NULL", cb);

    if (cb) g_shutdownCb = (PMC_SHUTDOWN_CB)cb;

    if (PMC_WSA_STARTUP() != 0) {
        F_DbgPrint(F_DBG_SEC_PMTCP, F_DBG_LVL_ERROR,
                   "PMTCP: _InitServerConnection ERROR: WSAStartup failed");
        F_DbgCtxExitf(ctx, "WSAStartup failed");
        return PMC_ERR_OPEN;
    }

#if defined(__WIN32__) || defined(_WIN32)
    /* ws2_32 is loaded by WSAStartup inside this DLL, so on ExitProcess
       its DllMain(DETACH) runs before ours (LIFO) and resets Winsock
       state, making send() in our shutdown callback fail with 10093.
       Pin it via GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_PIN,
       0x00000001) so its DllMain(DETACH) is not called on process exit.
       The function and the flag constant are missing from Watcom's
       windows.h, so resolve by name at runtime. */
    {
        typedef BOOL (__stdcall *PFN_GME)(DWORD, const char *, HMODULE *);
        HMODULE hKernel = GetModuleHandleA("kernel32.dll");
        if (hKernel != NULL) {
            PFN_GME pfnGME = (PFN_GME)GetProcAddress(hKernel,
                                                     "GetModuleHandleExA");
            if (pfnGME != NULL) {
                HMODULE hPin = NULL;
                pfnGME(0x00000001UL, "ws2_32.dll", &hPin);
            }
        }
    }
#endif

    if (ExternMachine != NULL)
    {
        strncpy(PipeHost, ExternMachine, sizeof(PipeHost) - 1);
        PipeHost[sizeof(PipeHost) - 1] = 0;
    }

    /* A previous TCPipe may still be around from an earlier session
       (reconnect retry). Drop it before creating the new one. */
    if (pF_pipe != NULL) {
        delete pF_pipe;
        pF_pipe = NULL;
    }

    pF_pipe = new TCPipe(PipeHost, PipePort, TCP_CLIENT_MODE);
    if (pF_pipe == NULL) {
        F_DbgPrint(F_DBG_SEC_PMTCP, F_DBG_LVL_ERROR,
                   "PMTCP: _InitServerConnection ERROR: new TCPipe failed");
        PMC_WSA_CLEANUP();
        F_DbgCtxExitf(ctx, "new TCPipe failed");
        return PMC_ERR_OPEN;
    }

    if (pF_pipe->Connect() != 0)
    {
        F_DbgPrint(F_DBG_SEC_PMTCP, F_DBG_LVL_ERROR,
                   "PMTCP: _InitServerConnection ERROR: Connect failed");
        delete pF_pipe;
        pF_pipe = NULL;
        PMC_WSA_CLEANUP();
        F_DbgCtxExitf(ctx, "Connect failed");
        return PMC_ERR_OPEN;
    }

    if (pF_pipe->HandShake() == HAND_SHAKE_ERROR)
    {
        F_DbgPrint(F_DBG_SEC_PMTCP, F_DBG_LVL_ERROR,
                   "PMTCP: _InitServerConnection ERROR: HandShake failed");
        pF_pipe->Close();
        delete pF_pipe;
        pF_pipe = NULL;
        PMC_WSA_CLEANUP();
        F_DbgCtxExitf(ctx, "HandShake failed");
        return HAND_SHAKE_ERROR;
    }

    *obj = (ULONG)pF_pipe;
    F_DbgCtxExitf(ctx, "Success (obj=%p)", pF_pipe);
    return 0;
}

/*! @brief Close the current client connection.
    @return 0.
    @retval 0 Always. */
extern "C" APIRET EXPENTRY _CloseServerConnection(void)
{
    int ctx;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_PMTCP, F_DBG_LVL_VERBOSE,
                         "PMTCP: _CloseServerConnection");

    if (pF_pipe != NULL)
    {
        delete pF_pipe;
        pF_pipe = NULL;
    }
    PMC_WSA_CLEANUP();
    F_DbgCtxExitf(ctx, "Success");
    return 0;
}

/*! @brief Send a command with a 32-bit argument to the server.
    @param [in] recvobj  Connection object.
    @param [in] ncmd     Command code.
    @param [in] data     32-bit argument.
    @return 0 on success, ERROR_BROKEN_PIPE on failure. */
extern "C" APIRET EXPENTRY _F_SendCmdToServer(void *recvobj, int ncmd, int data)
{
    int  ctx;
    char buf[8];
    int  rc;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_WIRE, F_DBG_LVL_VERBOSE,
                         "PMTCP: _F_SendCmdToServer(recvobj=%p, ncmd=0x%x, data=0x%x)",
                         recvobj, ncmd, data);
    if (!recvobj) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: _F_SendCmdToServer ERROR: recvobj is NULL!");
        F_DbgCtxExitf(ctx, "recvobj is NULL");
        return PMC_ERR_BROKEN;
    }
    memcpy(buf,     &ncmd, 4);
    memcpy(buf + 4, &data, 4);
    rc = ((TCPipe *)recvobj)->Send(buf, 8);
    if (rc != 0) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: _F_SendCmdToServer ERROR: Send failed");
        F_DbgCtxExitf(ctx, "Send failed");
        return PMC_ERR_BROKEN;
    }
    F_DbgCtxExitf(ctx, "Success");
    return 0;
}

/*! @brief Send raw data to the server.
    @param [in] recvobj  Connection object.
    @param [in] data     Payload.
    @param [in] len      Payload size in bytes.
    @return 0 on success, ERROR_BROKEN_PIPE on failure. */
extern "C" APIRET EXPENTRY _F_SendDataToServer(void *recvobj, void *data, int len)
{
    int ctx;
    int rc;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_WIRE, F_DBG_LVL_VERBOSE,
                         "PMTCP: _F_SendDataToServer(recvobj=%p, data=%p, len=%d)",
                         recvobj, data, len);
    if (!recvobj) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: _F_SendDataToServer ERROR: recvobj is NULL!");
        F_DbgCtxExitf(ctx, "recvobj is NULL");
        return PMC_ERR_BROKEN;
    }
    rc = ((TCPipe *)recvobj)->Send(data, len);
    if (rc != 0) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: _F_SendDataToServer ERROR: Send failed");
        F_DbgCtxExitf(ctx, "Send failed");
        return PMC_ERR_BROKEN;
    }
    F_DbgCtxExitf(ctx, "Success");
    return 0;
}

/*! @brief Receive raw data from the server.
    @param [in]  recvobj  Connection object.
    @param [out] data     Destination buffer.
    @param [out] len      Receives the number of bytes read.
    @param [in]  maxlen   Size of the destination buffer.
    @return 0 on success, -1 on failure. */
extern "C" APIRET EXPENTRY _F_RecvDataFromServer(void *recvobj, void *data, int *len, int maxlen)
{
    int ctx;
    int rc;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_WIRE, F_DBG_LVL_VERBOSE,
                         "PMTCP: _F_RecvDataFromServer(recvobj=%p, maxlen=%d)",
                         recvobj, maxlen);
    if (!recvobj) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: _F_RecvDataFromServer ERROR: recvobj is NULL!");
        F_DbgCtxExitf(ctx, "recvobj is NULL");
        return -1;
    }
    rc = ((TCPipe *)recvobj)->Recv(data, len, maxlen);
    if (rc != 0) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: _F_RecvDataFromServer ERROR: Recv failed");
        F_DbgCtxExitf(ctx, "Recv failed");
        return -1;
    }
    F_DbgCtxExitf(ctx, "len=%d", *len);
    return 0;
}

/*! @brief Send a generic command and wait for the server reply.

    On transport error (send or recv failed) returns (APIRET)-1 so
    callers can distinguish it from a legitimate server reply.
    @param [in] recvobj  Connection object.
    @param [in] cmd      Command code.
    @param [in] par      32-bit parameter.
    @return Server reply code, or (APIRET)-1 on transport error. */
extern "C" APIRET EXPENTRY _F_SendGenCmdToServer(void *recvobj, int cmd, int par)
{
    int ctx, rc, retcode, l;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_PMTCP, F_DBG_LVL_VERBOSE,
                         "PMTCP: _F_SendGenCmdToServer(recvobj=%p, cmd=0x%x, par=0x%x)",
                         recvobj, cmd, par);
    if (!recvobj) {
        F_DbgPrint(F_DBG_SEC_PMTCP, F_DBG_LVL_ERROR,
                   "PMTCP: _F_SendGenCmdToServer ERROR: recvobj is NULL!");
        F_DbgCtxExitf(ctx, "recvobj is NULL");
        return (APIRET)-1;
    }

    rc = _F_SendCmdToServer(recvobj, cmd, par);
    if (rc) {
        F_DbgCtxExitf(ctx, "SendCmd failed");
        return (APIRET)-1;
    }

    rc = _F_RecvDataFromServer(recvobj, &retcode, &l, sizeof(retcode));
    if (rc) {
        F_DbgCtxExitf(ctx, "RecvData failed");
        return (APIRET)-1;
    }

    F_DbgCtxExitf(ctx, "retcode=0x%x", retcode);
    return (APIRET)retcode;
}

/*! @brief Send a command with a payload and wait for the server reply.

    On transport error (send or recv failed) returns (APIRET)-1 so
    callers can distinguish it from a legitimate server reply.
    @param [in] recvobj  Connection object.
    @param [in] cmd      Command code.
    @param [in] par      32-bit parameter.
    @param [in] data     Payload.
    @param [in] datalen  Payload size in dwords.
    @return Server reply code, or (APIRET)-1 on transport error. */
extern "C" APIRET EXPENTRY _F_SendGenCmdDataToServer(void *recvobj, int cmd, int par,
                                                     void *data, int datalen)
{
    int ctx, rc, retcode, l;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_PMTCP, F_DBG_LVL_VERBOSE,
                         "PMTCP: _F_SendGenCmdDataToServer(recvobj=%p, cmd=0x%x, par=0x%x)",
                         recvobj, cmd, par);
    if (!recvobj) {
        F_DbgPrint(F_DBG_SEC_PMTCP, F_DBG_LVL_ERROR,
                   "PMTCP: _F_SendGenCmdDataToServer ERROR: recvobj is NULL!");
        F_DbgCtxExitf(ctx, "recvobj is NULL");
        return (APIRET)-1;
    }

    rc = _F_SendCmdToServer(recvobj, cmd, par);
    if (rc) {
        F_DbgCtxExitf(ctx, "SendCmd failed");
        return (APIRET)-1;
    }

    rc = _F_SendDataToServer(recvobj, data, datalen * sizeof(int));
    if (rc) {
        F_DbgCtxExitf(ctx, "SendData failed");
        return (APIRET)-1;
    }

    rc = _F_RecvDataFromServer(recvobj, &retcode, &l, sizeof(retcode));
    if (rc) {
        F_DbgCtxExitf(ctx, "RecvData failed");
        return (APIRET)-1;
    }

    F_DbgCtxExitf(ctx, "retcode=0x%x", retcode);
    return (APIRET)retcode;
}

/*! @brief Server-side: receive a (command, data) pair.
    @param [in]  recvobj  Connection object.
    @param [out] ncmd     Receives the command code.
    @param [out] data     Receives the 32-bit argument.
    @return 0 on success, -1 on failure or short read.
    @retval 0  Success.
    @retval -1 Failure or short read. */
extern "C" APIRET EXPENTRY _F_RecvCmdFromClient(void *recvobj, int *ncmd, int *data)
{
    int  ctx;
    char buf[8];
    int  len = 0;
    int  rc;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_WIRE, F_DBG_LVL_VERBOSE,
                         "PMTCP: _F_RecvCmdFromClient(recvobj=%p)", recvobj);
    if (!recvobj) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: _F_RecvCmdFromClient ERROR: recvobj is NULL!");
        F_DbgCtxExitf(ctx, "recvobj is NULL");
        return -1;
    }
    rc = ((TCPipe *)recvobj)->Recv(buf, &len, 8);
    if (rc != 0 || len != 8) {
        int err = 0;
#if defined(__WIN32__) || defined(__NT__) || defined(_WIN32)
        err = WSAGetLastError();
#endif
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: _F_RecvCmdFromClient ERROR: Recv failed or short read. "
                   "rc=%d, len=%d, WSAError=%d", rc, len, err);
        F_DbgCtxExitf(ctx, "Recv failed rc=%d, len=%d", rc, len);
        return -1;
    }
    memcpy(ncmd, buf,     4);
    memcpy(data, buf + 4, 4);
    F_DbgCtxExitf(ctx, "ncmd=0x%x, data=0x%x", *ncmd, *data);
    return 0;
}

/*! @brief Server-side: receive a message buffer.
    @param [in]  recvobj  Connection object.
    @param [out] sqmsg    Destination buffer.
    @param [out] l        Receives the number of bytes read.
    @param [in]  size     Number of bytes to read.
    @return 0 on success, -1 on failure or short read.
    @retval 0  Exactly size bytes were read.
    @retval -1 Failure or short read. */
extern "C" APIRET EXPENTRY _F_RecvDataFromClient(void *recvobj, void *sqmsg, int *l, int size)
{
    int ctx;
    int rc;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_WIRE, F_DBG_LVL_VERBOSE,
                         "PMTCP: _F_RecvDataFromClient(recvobj=%p, size=%d)",
                         recvobj, size);
    if (!recvobj) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: _F_RecvDataFromClient ERROR: recvobj is NULL!");
        F_DbgCtxExitf(ctx, "recvobj is NULL");
        return -1;
    }
    rc = ((TCPipe *)recvobj)->Recv(sqmsg, l, size);
    if (rc != 0) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: _F_RecvDataFromClient ERROR: Recv failed");
        F_DbgCtxExitf(ctx, "Recv failed");
        return -1;
    }
    if (*l != size) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: _F_RecvDataFromClient ERROR: short read l=%d, "
                   "expected %d", *l, size);
        F_DbgCtxExitf(ctx, "short read l=%d, expected %d", *l, size);
        return -1;
    }
    F_DbgCtxExitf(ctx, "l=%d", *l);
    return 0;
}

/*! @brief Server-side: send raw data to a client.
    @param [in] recvobj  Connection object.
    @param [in] data     Payload.
    @param [in] len      Payload size in bytes.
    @return 0 on success, ERROR_BROKEN_PIPE on failure. */
extern "C" APIRET EXPENTRY _F_SendDataToClient(void *recvobj, void *data, int len)
{
    int ctx;
    int rc;

    ctx = F_DbgCtxEnterf(F_DBG_SEC_WIRE, F_DBG_LVL_VERBOSE,
                         "PMTCP: _F_SendDataToClient(recvobj=%p, data=%p, len=%d)",
                         recvobj, data, len);
    if (!recvobj) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: _F_SendDataToClient ERROR: recvobj is NULL!");
        F_DbgCtxExitf(ctx, "recvobj is NULL");
        return PMC_ERR_BROKEN; 
    }
    rc = ((TCPipe *)recvobj)->Send(data, len);
    if (rc != 0) {
        int err = 0;
#if defined(__WIN32__) || defined(__NT__) || defined(_WIN32)
        err = WSAGetLastError();
#endif
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMTCP: _F_SendDataToClient ERROR: Send failed. "
                   "rc=%d, WSAError=%d", rc, err);
        F_DbgCtxExitf(ctx, "Send failed rc=%d", rc);
        return PMC_ERR_BROKEN;
    }
    F_DbgCtxExitf(ctx, "Success");
    return 0;
}

/*! @brief Start server-side dispatcher threads.
    @param [in] handl  Command handler callback.
    @return 0 on success, PMC_ERR_OPEN on setup failure.
    @retval 0            Loop exited (normally unreachable).
    @retval PMC_ERR_OPEN Setup failed. */

struct TCP_ClientArg {
    TCPipe           *pClient;
    PMC_HANDLER       h;
    PMC_DISCONNECT_CB disc;
    int               threadNum;
};

static void ClientThreadProc(void *param)
{
    TCP_ClientArg    *pArg    = (TCP_ClientArg *)param;
    TCPipe           *pClient = pArg->pClient;
    PMC_HANDLER       h       = pArg->h;
    PMC_DISCONNECT_CB disc    = pArg->disc;
    int               ncmd, data;

    while (_F_RecvCmdFromClient(pClient, &ncmd, &data) == 0)
    {
        if (ncmd == 0) break;
        if (h) h(pClient, ncmd, data, pArg->threadNum);
    }

    /* Transport-level notification: socket is gone. Fs_Server
       transitions the session to DETACHED so a legitimate RECONNECT
       can resume it without waiting for ulInactive. The recvobj
       (pClient) is still valid here; it is deleted below. */
    if (disc) disc(pClient);

    pClient->Close();
    delete pClient;
    delete pArg;
}

extern "C" APIRET EXPENTRY _startServerThreads(void *handl, void *disc_cb)
{
    PMC_HANDLER       h    = (PMC_HANDLER)handl;
    PMC_DISCONNECT_CB disc = (PMC_DISCONNECT_CB)disc_cb;
    TCPipe           *pServer = new TCPipe();
    int               wsaRc;
    int               threadCounter = 0;

    if (!pServer) return PMC_ERR_OPEN;

    wsaRc = PMC_WSA_STARTUP();
    if (wsaRc != 0)
    {
        F_DbgPrint(F_DBG_SEC_PMTCP, F_DBG_LVL_ERROR,
                   "PMTCP: _startServerThreads: WSAStartup failed (rc=%d)", wsaRc);
        delete pServer;
        return PMC_ERR_OPEN;
    }

    pServer->mode = TCP_SERVER_MODE;
    pServer->port = FREEPM_TCP_DEFAULT_PORT;

    if (pServer->Create() != 0)
    {
        F_DbgPrint(F_DBG_SEC_PMTCP, F_DBG_LVL_ERROR,
                   "PMTCP: _startServerThreads: Create() failed (port %d)",
                   pServer->port);
        delete pServer;
        PMC_WSA_CLEANUP();
        return PMC_ERR_OPEN;
    }

    F_DbgPrint(F_DBG_SEC_PMTCP, F_DBG_LVL_VERBOSE,
               "PMTCP: _startServerThreads: listening on port %d (multi-thread)",
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
                       "PMTCP: _startServerThreads: handshake failed, dropping");
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


/* ======================================================================== */
/*  Platform entry points                                                   */
/* ======================================================================== */

#if defined(__OS2__) || defined(__OS2V2__)

unsigned long _System _DLL_InitTerm(unsigned long hModule, unsigned long flag)
{
    (void)hModule;
    if (flag == 0) return 1;

    if (g_shutdownCb) {
        g_shutdownCb();
        g_shutdownCb = NULL;
    }
    if (pF_pipe != NULL) {
        delete pF_pipe;
        pF_pipe = NULL;
    }
    PMC_WSA_CLEANUP();
    return 1;
}

#else /* Win32 */

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpReserved)
{
    (void)hinstDLL; (void)lpReserved;
    if (fdwReason == DLL_PROCESS_DETACH) {
        /* ws2_32.dll is loaded by WSAStartup inside this DLL, so on
           ExitProcess it is detached first (LIFO) and its DllMain
           resets Winsock state. Re-init WSA here so send(EXIT) inside
           the shutdown callback goes through live code. */
        WSADATA wsa;
        int     wsaOk = (WSAStartup(MAKEWORD(1, 1), &wsa) == 0);

        if (g_shutdownCb) {
            g_shutdownCb();
            g_shutdownCb = NULL;
        }
        if (pF_pipe != NULL) {
            delete pF_pipe;
            pF_pipe = NULL;
        }
        if (wsaOk) WSACleanup();
    }
    return TRUE;
}

#endif
