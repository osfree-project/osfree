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

#include "F_tcp.hpp"

/* --- ÍÀ×ÀËÎ ÏÐÀÂÎÊ: Ëîêàëüíàÿ ôóíêöèÿ îòëàäêè --- */
/*! @brief Print a debug message to stderr and OutputDebugString.
    @param [in] fmt  printf-style format string.
    @param [in] ...  Arguments for the format string. */
static void PMTCP_Dbg(const char *fmt, ...)
{
    char buf[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
#if defined(__WIN32__) || defined(_WIN32)
    OutputDebugStringA(buf);
#endif
    fprintf(stderr, "%s", buf);
    fflush(stderr);
}
/* --- ÊÎÍÅÖ ÏÐÀÂÎÊ --- */

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

/*! @brief Handler callback type, same as in PMPIPE. */
typedef void (APIENTRY *PMC_HANDLER)(void *obj, int ncmd, int data, int threadNum);

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

/*! @brief Open a client TCP connection and perform the handshake.
    @param [in]  ExternMachine  Server host name, or NULL for the default.
    @param [out] obj            Receives the connection object.
    @return 0 on success, ERROR_OPEN_FAILED on connect failure, HAND_SHAKE_ERROR on mismatch.
    @retval 0                  Connected.
    @retval PMC_ERR_OPEN       Connect failed.
    @retval HAND_SHAKE_ERROR   Handshake mismatch. */
extern "C" APIRET EXPENTRY _InitServerConnection(char *ExternMachine, ULONG *obj)
{
    PMTCP_Dbg("PMTCP: _InitServerConnection IN -> ExternMachine=%s\n", ExternMachine ? ExternMachine : "NULL");
    if (PMC_WSA_STARTUP() != 0) {
        PMTCP_Dbg("PMTCP: _InitServerConnection ERROR: WSAStartup failed\n");
        return PMC_ERR_OPEN;
    }

    if (ExternMachine != NULL)
    {
        strncpy(PipeHost, ExternMachine, sizeof(PipeHost) - 1);
        PipeHost[sizeof(PipeHost) - 1] = 0;
    }

    pF_pipe = new TCPipe(PipeHost, PipePort, TCP_CLIENT_MODE);
    if (pF_pipe == NULL) {
        PMTCP_Dbg("PMTCP: _InitServerConnection ERROR: new TCPipe failed\n");
        PMC_WSA_CLEANUP();
        return PMC_ERR_OPEN;
    }

    if (pF_pipe->Connect() != 0)
    {
        PMTCP_Dbg("PMTCP: _InitServerConnection ERROR: Connect failed\n");
        delete pF_pipe;
        pF_pipe = NULL;
        PMC_WSA_CLEANUP();
        return PMC_ERR_OPEN;
    }

    if (pF_pipe->HandShake() == HAND_SHAKE_ERROR)
    {
        PMTCP_Dbg("PMTCP: _InitServerConnection ERROR: HandShake failed\n");
        pF_pipe->Close();
        delete pF_pipe;
        pF_pipe = NULL;
        PMC_WSA_CLEANUP();
        return HAND_SHAKE_ERROR;
    }

    *obj = (ULONG)pF_pipe;
    PMTCP_Dbg("PMTCP: _InitServerConnection OUT -> Success (obj=%p)\n", pF_pipe);
    return 0;
}

/*! @brief Close the current client connection.
    @return 0.
    @retval 0 Always. */
extern "C" APIRET EXPENTRY _CloseServerConnection(void)
{
    PMTCP_Dbg("PMTCP: _CloseServerConnection IN\n");
    if (pF_pipe != NULL)
    {
        delete pF_pipe;
        pF_pipe = NULL;
    }
    PMC_WSA_CLEANUP();
    PMTCP_Dbg("PMTCP: _CloseServerConnection OUT\n");
    return 0;
}

/*! @brief Send a command with a 32-bit argument to the server.
    @param [in] recvobj  Connection object.
    @param [in] ncmd     Command code.
    @param [in] data     32-bit argument.
    @return 0 on success, ERROR_BROKEN_PIPE on failure. */
extern "C" APIRET EXPENTRY _F_SendCmdToServer(void *recvobj, int ncmd, int data)
{
    PMTCP_Dbg("PMTCP: _F_SendCmdToServer IN -> recvobj=%p, ncmd=0x%x, data=0x%x\n", recvobj, ncmd, data);
    if (!recvobj) {
        PMTCP_Dbg("PMTCP: _F_SendCmdToServer ERROR: recvobj is NULL!\n");
        return PMC_ERR_BROKEN;
    }
    char buf[8];
    memcpy(buf,     &ncmd, 4);
    memcpy(buf + 4, &data, 4);
    int rc = ((TCPipe *)recvobj)->Send(buf, 8);
    if (rc != 0) {
        PMTCP_Dbg("PMTCP: _F_SendCmdToServer ERROR: Send failed\n");
        return PMC_ERR_BROKEN;
    }
    return 0;
}

/*! @brief Send raw data to the server.
    @param [in] recvobj  Connection object.
    @param [in] data     Payload.
    @param [in] len      Payload size in bytes.
    @return 0 on success, ERROR_BROKEN_PIPE on failure. */
extern "C" APIRET EXPENTRY _F_SendDataToServer(void *recvobj, void *data, int len)
{
    PMTCP_Dbg("PMTCP: _F_SendDataToServer IN -> recvobj=%p, data=%p, len=%d\n", recvobj, data, len);
    if (!recvobj) {
        PMTCP_Dbg("PMTCP: _F_SendDataToServer ERROR: recvobj is NULL!\n");
        return PMC_ERR_BROKEN;
    }
    int rc = ((TCPipe *)recvobj)->Send(data, len);
    if (rc != 0) {
        PMTCP_Dbg("PMTCP: _F_SendDataToServer ERROR: Send failed\n");
        return PMC_ERR_BROKEN;
    }
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
    PMTCP_Dbg("PMTCP: _F_RecvDataFromServer IN -> recvobj=%p, maxlen=%d\n", recvobj, maxlen);
    if (!recvobj) {
        PMTCP_Dbg("PMTCP: _F_RecvDataFromServer ERROR: recvobj is NULL!\n");
        return -1;
    }
    int rc = ((TCPipe *)recvobj)->Recv(data, len, maxlen);
    if (rc != 0) {
        PMTCP_Dbg("PMTCP: _F_RecvDataFromServer ERROR: Recv failed\n");
        return -1;
    }
    PMTCP_Dbg("PMTCP: _F_RecvDataFromServer OUT -> len=%d\n", *len);
    return 0;
}

/*! @brief Send a generic command and wait for the server reply.
    @param [in] recvobj  Connection object.
    @param [in] cmd      Command code.
    @param [in] par      32-bit parameter.
    @return Server reply code, or the transport error code. */
extern "C" APIRET EXPENTRY _F_SendGenCmdToServer(void *recvobj, int cmd, int par)
{
    PMTCP_Dbg("PMTCP: _F_SendGenCmdToServer IN -> recvobj=%p, cmd=0x%x, par=0x%x\n", recvobj, cmd, par);
    if (!recvobj) {
        PMTCP_Dbg("PMTCP: _F_SendGenCmdToServer ERROR: recvobj is NULL!\n");
        return PMC_ERR_BROKEN;
    }
    int rc, retcode, l;
    rc = _F_SendCmdToServer(recvobj, cmd, par);
    if (rc) return rc;
    rc = _F_RecvDataFromServer(recvobj, &retcode, &l, sizeof(retcode));
    if (rc) return rc;
    PMTCP_Dbg("PMTCP: _F_SendGenCmdToServer OUT -> retcode=0x%x\n", retcode);
    return (APIRET)retcode;
}

/*! @brief Send a command with a payload and wait for the server reply.
    @param [in] recvobj  Connection object.
    @param [in] cmd      Command code.
    @param [in] par      32-bit parameter.
    @param [in] data     Payload.
    @param [in] datalen  Payload size in dwords.
    @return Server reply code, or the transport error code. */
extern "C" APIRET EXPENTRY _F_SendGenCmdDataToServer(void *recvobj, int cmd, int par, void *data, int datalen)
{
    PMTCP_Dbg("PMTCP: _F_SendGenCmdDataToServer IN -> recvobj=%p, cmd=0x%x, par=0x%x\n", recvobj, cmd, par);
    if (!recvobj) {
        PMTCP_Dbg("PMTCP: _F_SendGenCmdDataToServer ERROR: recvobj is NULL!\n");
        return PMC_ERR_BROKEN;
    }
    int rc, retcode, l;
    rc = _F_SendCmdToServer(recvobj, cmd, par);
    if (rc) return rc;
    rc = _F_SendDataToServer(recvobj, data, datalen * sizeof(int));
    if (rc) return rc;
    rc = _F_RecvDataFromServer(recvobj, &retcode, &l, sizeof(retcode));
    if (rc) return rc;
    PMTCP_Dbg("PMTCP: _F_SendGenCmdDataToServer OUT -> retcode=0x%x\n", retcode);
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
    PMTCP_Dbg("PMTCP: _F_RecvCmdFromClient IN -> recvobj=%p\n", recvobj);
    if (!recvobj) {
        PMTCP_Dbg("PMTCP: _F_RecvCmdFromClient ERROR: recvobj is NULL!\n");
        return -1;
    }
    char buf[8];
    int  len = 0;
    int rc = ((TCPipe *)recvobj)->Recv(buf, &len, 8);
    if (rc != 0 || len != 8) {
        int err = 0;
#if defined(__WIN32__) || defined(__NT__) || defined(_WIN32)
        err = WSAGetLastError();
#endif
        PMTCP_Dbg("PMTCP: _F_RecvCmdFromClient ERROR: Recv failed or short read. rc=%d, len=%d, WSAError=%d\n", rc, len, err);
        return -1;
    }
    memcpy(ncmd, buf,     4);
    memcpy(data, buf + 4, 4);
    PMTCP_Dbg("PMTCP: _F_RecvCmdFromClient OUT -> ncmd=0x%x, data=0x%x\n", *ncmd, *data);
    return 0;
}

/*! @brief Server-side: receive a message buffer.
    @param [in]  recvobj  Connection object.
    @param [out] sqmsg    Destination buffer.
    @param [out] l        Receives the number of bytes read.
    @param [in]  size     Size of the destination buffer.
    @return 0 on success, -1 on failure. */
extern "C" APIRET EXPENTRY _F_RecvDataFromClient(void *recvobj, void *sqmsg, int *l, int size)
{
    PMTCP_Dbg("PMTCP: _F_RecvDataFromClient IN -> recvobj=%p, size=%d\n", recvobj, size);
    if (!recvobj) {
        PMTCP_Dbg("PMTCP: _F_RecvDataFromClient ERROR: recvobj is NULL!\n");
        return -1;
    }
    int rc = ((TCPipe *)recvobj)->Recv(sqmsg, l, size);
    if (rc != 0) {
        PMTCP_Dbg("PMTCP: _F_RecvDataFromClient ERROR: Recv failed\n");
        return -1;
    }
    PMTCP_Dbg("PMTCP: _F_RecvDataFromClient OUT -> l=%d\n", *l);
    return 0;
}

/*! @brief Server-side: send raw data to a client.
    @param [in] recvobj  Connection object.
    @param [in] data     Payload.
    @param [in] len      Payload size in bytes.
    @return 0 on success, ERROR_BROKEN_PIPE on failure. */
extern "C" APIRET EXPENTRY _F_SendDataToClient(void *recvobj, void *data, int len)
{
    PMTCP_Dbg("PMTCP: _F_SendDataToClient IN -> recvobj=%p, data=%p, len=%d\n", recvobj, data, len);
    if (!recvobj) {
        PMTCP_Dbg("PMTCP: _F_SendDataToClient ERROR: recvobj is NULL!\n");
        return PMC_ERR_BROKEN; 
    }
    int rc = ((TCPipe *)recvobj)->Send(data, len);
    if (rc != 0) {
        int err = 0;
#if defined(__WIN32__) || defined(__NT__) || defined(_WIN32)
        err = WSAGetLastError();
#endif
        PMTCP_Dbg("PMTCP: _F_SendDataToClient ERROR: Send failed. rc=%d, WSAError=%d\n", rc, err);
        return PMC_ERR_BROKEN;
    }
    PMTCP_Dbg("PMTCP: _F_SendDataToClient OUT -> Success (0)\n");
    return 0;
}

/*! @brief Start server-side dispatcher threads.
    @param [in] handl  Command handler callback.
    @return 0 on success, PMC_ERR_OPEN on setup failure.
    @retval 0            Loop exited (normally unreachable).
    @retval PMC_ERR_OPEN Setup failed. */
extern "C" APIRET EXPENTRY _startServerThreads(void *handl)
{
    PMC_HANDLER h = (PMC_HANDLER)handl;
    TCPipe      server;
    int         threadNum = 0;
    int         wsaRc;

    wsaRc = PMC_WSA_STARTUP();
    if (wsaRc != 0)
    {
        PMTCP_Dbg("PMTCP: _startServerThreads: WSAStartup failed (rc=%d)\n", wsaRc);
        return PMC_ERR_OPEN;
    }

    server.mode = TCP_SERVER_MODE;
    server.port = FREEPM_TCP_DEFAULT_PORT;
    
    if (server.Create() != 0)
    {
        PMTCP_Dbg("PMTCP: _startServerThreads: Create() failed (port %d might be in use)\n", server.port);
        PMC_WSA_CLEANUP();
        return PMC_ERR_OPEN;
    }

    PMTCP_Dbg("PMTCP: _startServerThreads: listening on port %d\n", server.port);

    for (;;)
    {
        TCPipe *pClient = new TCPipe();
        if (server.Accept(pClient) != 0)
        {
            delete pClient;
            continue;
        }

        PMTCP_Dbg("PMTCP: Accepted connection, pClient=%p\n", pClient);

        if (pClient->HandShake() == HAND_SHAKE_ERROR)
        {
            PMTCP_Dbg("PMTCP: _startServerThreads: handshake failed\n");
            pClient->Close();
            delete pClient;
            continue;
        }

        int ncmd, data;
        while (_F_RecvCmdFromClient(pClient, &ncmd, &data) == 0)
        {
            if (ncmd == 0)
                break;
            PMTCP_Dbg("PMTCP: Dispatching to handler: recvobj=%p, ncmd=0x%x\n", pClient, ncmd);
            if (h) h(pClient, ncmd, data, threadNum);
        }

        pClient->Close();
        delete pClient;
        threadNum++;
    }

    return 0;
}