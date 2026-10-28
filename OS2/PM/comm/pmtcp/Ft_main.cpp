/*! @file Ft_main.cpp
    @brief TCP backend for FreePM: exported entry points.

    Same ordinals and same signatures as PMPIPE (Fc_main.cpp).
    Backend function pointers are resolved in PMCLIENT via the
    ordinals listed in the Makefile EXPORTS directive.
*/

#define INCL_DOS
#define INCL_DOSERRORS

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <os2.h>

#include "F_tcp.hpp"
#include "F_config.hpp"

/*! @brief Server-side receive of a message buffer (defined below).

    @param [in]  recvobj  Connection object.
    @param [out] sqmsg    Destination buffer.
    @param [out] l        Receives the number of bytes read.
    @param [in]  size     Size of the destination buffer.
    @return 0 on success, -1 on failure. */
extern "C" APIRET APIENTRY _F_RecvDataFromClient(void *recvobj, void *sqmsg, int *l, int size);

/*! @brief Server-side receive of a (cmd, data) pair (defined below).

    @param [in]  recvobj  Connection object.
    @param [out] ncmd     Receives the command code.
    @param [out] data     Receives the 32-bit argument.
    @return 0 on success, -1 on failure or short read. */
extern "C" APIRET APIENTRY _F_RecvCmdFromClient(void *recvobj, int *ncmd, int *data);

/*! @brief Client-side receive of raw data (defined below).

    @param [in]  recvobj  Connection object.
    @param [out] data     Destination buffer.
    @param [out] len      Receives the number of bytes read.
    @param [in]  maxlen   Size of the destination buffer.
    @return 0 on success, -1 on failure. */
extern "C" APIRET APIENTRY _F_RecvDataFromServer(void *recvobj, void *data, int *len, int maxlen);

/*! @brief Client-side send of raw data (defined below).

    @param [in] recvobj  Connection object.
    @param [in] data     Payload.
    @param [in] len      Payload size in bytes.
    @return 0 on success, ERROR_BROKEN_PIPE on failure. */
extern "C" APIRET APIENTRY _F_SendDataToServer(void *recvobj, void *data, int len);

/*! @brief Client-side send of a command with a payload (defined below).

    @param [in] recvobj  Connection object.
    @param [in] cmd      Command code.
    @param [in] par      32-bit parameter.
    @param [in] data     Payload.
    @param [in] datalen  Payload size in dwords.
    @return Server reply code, or the transport error code. */
extern "C" APIRET APIENTRY _F_SendGenCmdDataToServer(void *recvobj, int cmd, int par, void *data, int datalen);

/*! @brief Client-side send of a generic command (defined below).

    @param [in] recvobj  Connection object.
    @param [in] cmd      Command code.
    @param [in] par      32-bit parameter.
    @return Server reply code, or the transport error code. */
extern "C" APIRET APIENTRY _F_SendGenCmdToServer(void *recvobj, int cmd, int par);

/*! @brief Client-side send of a command (defined below).

    @param [in] recvobj  Connection object.
    @param [in] ncmd     Command code.
    @param [in] data     32-bit argument.
    @return 0 on success, ERROR_BROKEN_PIPE on failure. */
extern "C" APIRET APIENTRY _F_SendCmdToServer(void *recvobj, int ncmd, int data);

/*! @brief Server-side send of raw data (defined below).

    @param [in] recvobj  Connection object.
    @param [in] data     Payload.
    @param [in] len      Payload size in bytes.
    @return 0 on success, ERROR_BROKEN_PIPE on failure. */
extern "C" APIRET APIENTRY _F_SendDataToClient(void *recvobj, void *data, int len);

/*! @brief Client connection object, exported for the debug module. */
class TCPipe *pF_pipe = NULL;
/*! @brief Host of the server we are connected to. */
static char    PipeHost[256] = FREEPM_TCP_DEFAULT_HOST;
/*! @brief Port of the server we are connected to. */
static int     PipePort     = FREEPM_TCP_DEFAULT_PORT;

/*! @brief Open a client TCP connection and perform the handshake.

    @param [in]  ExternMachine  Server host name, or NULL for the default.
    @param [out] obj            Receives the connection object.
    @return 0 on success, ERROR_OPEN_FAILED on connect failure,
            HAND_SHAKE_ERROR on handshake mismatch.
    @retval 0                  Connected and handshake completed.
    @retval ERROR_OPEN_FAILED  Connect failed.
    @retval HAND_SHAKE_ERROR   Handshake mismatch. */
extern "C" APIRET APIENTRY _InitServerConnection(char *ExternMachine, ULONG *obj)
{
    if (ExternMachine != NULL)
    {
        strncpy(PipeHost, ExternMachine, sizeof(PipeHost) - 1);
        PipeHost[sizeof(PipeHost) - 1] = 0;
    }

    pF_pipe = new TCPipe(PipeHost, PipePort, TCP_CLIENT_MODE);
    if (pF_pipe->Connect() != 0)
    {
        delete pF_pipe;
        pF_pipe = NULL;
        return ERROR_OPEN_FAILED;
    }

    if (pF_pipe->HandShake() == HAND_SHAKE_ERROR)
    {
        pF_pipe->Close();
        delete pF_pipe;
        pF_pipe = NULL;
        return HAND_SHAKE_ERROR;
    }

    *obj = (ULONG)pF_pipe;
    return 0;
}

/*! @brief Close the current client connection.

    @return 0.
    @retval 0  Always. */
extern "C" APIRET APIENTRY _CloseServerConnection(void)
{
    if (pF_pipe != NULL)
    {
        delete pF_pipe;
        pF_pipe = NULL;
    }
    return 0;
}

/*! @brief Send a command with a 32-bit argument to the server.

    @param [in] recvobj  Connection object.
    @param [in] ncmd     Command code.
    @param [in] data     32-bit argument.
    @return 0 on success, ERROR_BROKEN_PIPE on failure.
    @retval 0                   Sent.
    @retval ERROR_BROKEN_PIPE   Transport failure. */
extern "C" APIRET APIENTRY _F_SendCmdToServer(void *recvobj, int ncmd, int data)
{
    char buf[8];
    memcpy(buf,     &ncmd, 4);
    memcpy(buf + 4, &data, 4);
    return ((TCPipe *)recvobj)->Send(buf, 8) ? ERROR_BROKEN_PIPE : 0;
}

/*! @brief Send raw data to the server.

    @param [in] recvobj  Connection object.
    @param [in] data     Payload.
    @param [in] len      Payload size in bytes.
    @return 0 on success, ERROR_BROKEN_PIPE on failure.
    @retval 0                   Sent.
    @retval ERROR_BROKEN_PIPE   Transport failure. */
extern "C" APIRET APIENTRY _F_SendDataToServer(void *recvobj, void *data, int len)
{
    return ((TCPipe *)recvobj)->Send(data, len) ? ERROR_BROKEN_PIPE : 0;
}

/*! @brief Receive raw data from the server.

    @param [in]  recvobj  Connection object.
    @param [out] data     Destination buffer.
    @param [out] len      Receives the number of bytes read.
    @param [in]  maxlen   Size of the destination buffer.
    @return 0 on success, -1 on failure.
    @retval 0   At least one byte was read.
    @retval -1  Receive failed. */
extern "C" APIRET APIENTRY _F_RecvDataFromServer(void *recvobj, void *data, int *len, int maxlen)
{
    return ((TCPipe *)recvobj)->Recv(data, len, maxlen);
}

/*! @brief Send a generic command and wait for the server reply.

    @param [in] recvobj  Connection object.
    @param [in] cmd      Command code.
    @param [in] par      32-bit parameter.
    @return Server reply code, or the transport error code. */
extern "C" APIRET APIENTRY _F_SendGenCmdToServer(void *recvobj, int cmd, int par)
{
    int rc, retcode, l;

    rc = _F_SendCmdToServer(recvobj, cmd, par);
    if (rc) return rc;

    rc = _F_RecvDataFromServer(recvobj, &retcode, &l, sizeof(retcode));
    return rc ? rc : (APIRET)retcode;
}

/*! @brief Send a command with a payload and wait for the server reply.

    @param [in] recvobj  Connection object.
    @param [in] cmd      Command code.
    @param [in] par      32-bit parameter.
    @param [in] data     Payload.
    @param [in] datalen  Payload size in dwords.
    @return Server reply code, or the transport error code. */
extern "C" APIRET APIENTRY _F_SendGenCmdDataToServer(void *recvobj, int cmd, int par, void *data, int datalen)
{
    int rc, retcode, l;

    rc = _F_SendCmdToServer(recvobj, cmd, par);
    if (rc) return rc;

    rc = _F_SendDataToServer(recvobj, data, datalen * sizeof(int));
    if (rc) return rc;

    rc = _F_RecvDataFromServer(recvobj, &retcode, &l, sizeof(retcode));
    return rc ? rc : (APIRET)retcode;
}

/*! @brief Server-side: receive a (command, data) pair.

    @param [in]  recvobj  Connection object.
    @param [out] ncmd     Receives the command code.
    @param [out] data     Receives the 32-bit argument.
    @return 0 on success, -1 on failure or short read.
    @retval 0   A (command, data) pair was received.
    @retval -1  Receive failed or a short read occurred. */
extern "C" APIRET APIENTRY _F_RecvCmdFromClient(void *recvobj, int *ncmd, int *data)
{
    char buf[8];
    int  len;

    int rc = ((TCPipe *)recvobj)->Recv(buf, &len, 8);
    if (rc || len != 8) return -1;

    memcpy(ncmd, buf,     4);
    memcpy(data, buf + 4, 4);
    return 0;
}

/*! @brief Server-side: receive a message buffer.

    @param [in]  recvobj  Connection object.
    @param [out] sqmsg    Destination buffer.
    @param [out] l        Receives the number of bytes read.
    @param [in]  size     Size of the destination buffer.
    @return 0 on success, -1 on failure.
    @retval 0   At least one byte was read.
    @retval -1  Receive failed. */
extern "C" APIRET APIENTRY _F_RecvDataFromClient(void *recvobj, void *sqmsg, int *l, int size)
{
    return ((TCPipe *)recvobj)->Recv(sqmsg, l, size);
}

/*! @brief Server-side: send raw data to a client.

    @param [in] recvobj  Connection object.
    @param [in] data     Payload.
    @param [in] len      Payload size in bytes.
    @return 0 on success, ERROR_BROKEN_PIPE on failure.
    @retval 0                   Sent.
    @retval ERROR_BROKEN_PIPE   Transport failure. */
extern "C" APIRET APIENTRY _F_SendDataToClient(void *recvobj, void *data, int len)
{
    return ((TCPipe *)recvobj)->Send(data, len) ? ERROR_BROKEN_PIPE : 0;
}

/*! @brief Start server-side dispatcher threads (not implemented yet).

    @param [in] handl  Command handler callback.
    @return 0.
    @retval 0  Always (stub). */
extern "C" APIRET APIENTRY _startServerThreads(void *handl)
{
    /* Server-side multi-thread dispatcher is not implemented yet in PMTCP.
       The same threading model as in Fc_main.cpp will be added when
       PMSERVER.EXE is switched to the TCP backend. */
    return 0;
}
