/*! @file F_pipe.cpp
    @brief Named-pipe transport for FreePM: PipeLink + 2 backend exports.

    Byte-mode pipe. Recv() loops until exactly cbBuffer bytes are read,
    matching F_Link's stream contract (same as TcpLink). Exports
    Fc_TrnConnect and Fs_TrnRunListener; other 9 exports live in shared/.
    Uses OS/2 Dos*NPipe API on both platforms (Win32 via F_platform.h
    emulation).
*/
#include <string.h>
#include <process.h>

#include "F_pipe.hpp"
#include "../include/F_debug.h"

/* ======================================================================== */
/*  PipeLink                                                               */
/* ======================================================================== */

/*! @brief Create one listener pipe instance.

    Max-instances lives in the low byte of the pipe mode. Byte mode
    (no NP_WMESG / NP_RMESG) is used so that DosRead / DosWrite behave
    like recv / send and Recv() can loop until exactly cbBuffer bytes
    are read. Freed by DosClose (via PipeLink::Close).

    @param [in]  name  Pipe name.
    @param [out] ph    Receives the listener handle.
    @return 0 on success, -1 on failure.
    @retval 0   Listener created.
    @retval -1  DosCreateNPipe failed. */
static int PipeCreateListener(const char *name, HPIPE *ph)
{
    ULONG openMode = NP_ACCESS_DUPLEX | NP_NOINHERIT;
    ULONG pipeMode = (ULONG)(FREEPM_PIPE_MAX_INST & 0xFF);

    return (DosCreateNPipe((PSZ)name, ph, openMode, pipeMode,
                           FREEPM_PIPE_OUTB_SIZE, FREEPM_PIPE_INPB_SIZE,
                           FREEPM_PIPE_TIMEOUT) == 0) ? 0 : -1;
}

/*! @brief Construct a client PipeLink with default name.
    Pipe handle is not opened.
    @return Nothing. */
PipeLink::PipeLink()
    : Hpipe(NULLHANDLE), mode(PIPE_CLIENT_MODE), connected(0)
{
    strcpy(name, FREEPM_PIPE_NAME);
}

/*! @brief Construct a PipeLink with explicit name and mode.
    Pipe handle is not opened.
    @param [in] pszName  Pipe name (NULL uses FREEPM_PIPE_NAME).
    @param [in] iMode    PIPE_CLIENT_MODE or PIPE_SERVER_MODE.
    @return Nothing. */
PipeLink::PipeLink(const char *pszName, int iMode)
    : Hpipe(NULLHANDLE), mode(iMode), connected(0)
{
    if (pszName) {
        strncpy(name, pszName, sizeof(name) - 1);
        name[sizeof(name) - 1] = 0;
    } else {
        strcpy(name, FREEPM_PIPE_NAME);
    }
}

/*! @brief Destroy the PipeLink. Closes pipe handle.
    @return Nothing. */
PipeLink::~PipeLink()
{
    Close();
}

/*! @brief Open the link: create listener (server) or connect (client).
    @return 0 on success, -1 on failure.
    @retval 0   Listening pipe ready (server) or peer connected (client).
    @retval -1  DosCreateNPipe or DosOpen failed. */
int PipeLink::Open(void)
{
    if (mode == PIPE_SERVER_MODE) {
        if (PipeCreateListener(name, &Hpipe) != 0) {
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMPIPE: PipeLink::Open ERROR: DosCreateNPipe failed");
            return -1;
        }
        return 0;
    }
    else {
        ULONG ulAction = 0;

        if (DosOpen((PSZ)name, &Hpipe, &ulAction, 0, 0,
                    OPEN_ACTION_OPEN_IF_EXISTS,
                    OPEN_FLAGS_WRITE_THROUGH |
                    OPEN_FLAGS_FAIL_ON_ERROR |
                    OPEN_FLAGS_RANDOM |
                    OPEN_SHARE_DENYNONE |
                    OPEN_ACCESS_READWRITE,
                    NULL) != 0)
        {
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMPIPE: PipeLink::Open ERROR: DosOpen failed");
            return -1;
        }
        connected = 1;
        return 0;
    }
}

/*! @brief Accept one incoming client (server mode).

    DosConnectNPipe blocks until a client opens the pipe. The accepted
    handle is moved to a fresh PipeLink; the listener is immediately
    re-created so the next Accept can proceed.

    @param [out] ppNew  Receives a new PipeLink for the accepted connection.
    @return 0 on success, -1 on failure.
    @retval 0   Connection accepted.
    @retval -1  Not a listener, bad args, or DosConnectNPipe failed. */
int PipeLink::Accept(F_Link **ppNew)
{
    PipeLink *pClient;
    HPIPE     oldHpipe = Hpipe;

    if (Hpipe == NULLHANDLE) return -1;
    if (ppNew == NULL) return -1;

    if (DosConnectNPipe(Hpipe) != 0) return -1;

    pClient = new PipeLink();
    if (!pClient) {
        DosDisConnectNPipe(Hpipe);
        return -1;
    }

    pClient->Hpipe     = oldHpipe;
    pClient->connected = 1;
    pClient->mode      = PIPE_SERVER_MODE;

    Hpipe = NULLHANDLE;
    if (PipeCreateListener(name, &Hpipe) != 0) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMPIPE: PipeLink::Accept: listener re-create failed");
    }

    *ppNew = pClient;
    return 0;
}

/*! @brief Perform the FreePM handshake.
    @return 0 on success, HAND_SHAKE_ERROR on protocol mismatch/transport error.
    @retval 0                 Handshake completed.
    @retval HAND_SHAKE_ERROR  Magic mismatch or transport failure. */
int PipeLink::Handshake(void)
{
    char buf[32];
    int  cb;
    int  inpLen = (int)strlen(HAND_SHAKE_INP) + 1;
    int  outLen = (int)strlen(HAND_SHAKE_OUT) + 1;

    if (mode == PIPE_SERVER_MODE) {
        if (Recv(buf, &cb, inpLen) != 0) return HAND_SHAKE_ERROR;
        if (memcmp(buf, HAND_SHAKE_INP, inpLen) != 0) return HAND_SHAKE_ERROR;
        if (Send(HAND_SHAKE_OUT, outLen) != 0) return HAND_SHAKE_ERROR;
    } else {
        if (Send(HAND_SHAKE_INP, inpLen) != 0) return HAND_SHAKE_ERROR;
        if (Recv(buf, &cb, outLen) != 0) return HAND_SHAKE_ERROR;
        if (memcmp(buf, HAND_SHAKE_OUT, outLen) != 0) return HAND_SHAKE_ERROR;
    }
    return 0;
}

/*! @brief Send exactly cbData bytes. Loops on partial write.
    @param [in] pvData  Source buffer.
    @param [in] cbData  Bytes to send (<= 0 is a no-op).
    @return 0 on success, -1 on error.
    @retval 0   All bytes sent (or cbData <= 0).
    @retval -1  Not connected or DosWrite failed. */
int PipeLink::Send(const void *pvData, int cbData)
{
    const char *p = (const char *)pvData;
    int total = 0;

    if (cbData <= 0) return 0;
    if (!connected) return -1;

    while (total < cbData) {
        ULONG chunk = (ULONG)(cbData - total);
        ULONG done  = 0;
        APIRET rc;

        if (chunk > 0x8000) chunk = 0x8000;

        rc = DosWrite(Hpipe, (PVOID)(p + total), chunk, &done);
        if (rc != 0) {
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMPIPE: PipeLink::Send ERROR: DosWrite rc=%d", rc);
            return -1;
        }
        if (done == 0) {
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMPIPE: PipeLink::Send ERROR: DosWrite returned 0");
            return -1;
        }
        total += (int)done;
    }
    return 0;
}

/*! @brief Receive exactly cbBuffer bytes. Loops on partial read.
    On peer-close sets *pcbReceived = bytes-read-so-far and returns -1.
    @param [out] pvData       Destination.
    @param [out] pcbReceived  Bytes received.
    @param [in]  cbBuffer     Bytes required.
    @return 0 on success, -1 on peer-close or error.
    @retval 0   Exactly cbBuffer bytes received.
    @retval -1  Not connected, peer close, or DosRead error. */
int PipeLink::Recv(void *pvData, int *pcbReceived, int cbBuffer)
{
    char *p = (char *)pvData;
    int total = 0;

    if (pcbReceived) *pcbReceived = 0;
    if (cbBuffer <= 0) return 0;
    if (!connected) return -1;

    while (total < cbBuffer) {
        ULONG want = (ULONG)(cbBuffer - total);
        ULONG done = 0;
        APIRET rc;

        if (want > 0x8000) want = 0x8000;

        rc = DosRead(Hpipe, p + total, want, &done);
        if (rc != 0) {
            if (pcbReceived) *pcbReceived = total;
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMPIPE: PipeLink::Recv ERROR: DosRead rc=%d", rc);
            return -1;
        }
        if (done == 0) {
            if (pcbReceived) *pcbReceived = total;
            return -1;
        }
        total += (int)done;
    }
    if (pcbReceived) *pcbReceived = total;
    return 0;
}

/*! @brief Close the pipe handle. Idempotent.
    @return 0 always.
    @retval 0  Always. */
int PipeLink::Close(void)
{
    if (Hpipe != NULLHANDLE) {
        DosClose(Hpipe);
        Hpipe = NULLHANDLE;
    }
    connected = 0;
    return 0;
}

/* ======================================================================== */
/*  Backend exports                                                        */
/* ======================================================================== */

/*! @brief Open a client pipe connection and perform the handshake.
    @param [in]  address  Pipe name, or NULL for FREEPM_PIPE_NAME.
    @param [out] obj      Receives the connection object; set to 0 on error.
    @return 0 on success, non-zero on failure.
    @retval 0                  Connected.
    @retval PMC_ERR_OPEN       new or Open failed.
    @retval HAND_SHAKE_ERROR   Handshake mismatch. */
extern "C" APIRET EXPENTRY Fc_TrnConnect(char *address, ULONG *obj)
{
    PipeLink *pNew;

    if (!obj) return PMC_ERR_OPEN;
    *obj = 0;

    pNew = new PipeLink(address, PIPE_CLIENT_MODE);
    if (pNew == NULL) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMPIPE: Fc_TrnConnect ERROR: new PipeLink failed");
        return PMC_ERR_OPEN;
    }

    if (pNew->Open() != 0) {
        delete pNew;
        return PMC_ERR_OPEN;
    }

    if (pNew->Handshake() == HAND_SHAKE_ERROR) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMPIPE: Fc_TrnConnect ERROR: Handshake failed");
        delete pNew;
        return HAND_SHAKE_ERROR;
    }

    *obj = (ULONG)pNew;
    return 0;
}

/*! @brief Server-side listener loop: create, connect, accept, spawn thread.

    Does not return on success. For each accepted connection performs
    the handshake and spawns one thread running Fs_ClientThreadProc.

    @param [in] handler  Command handler callback (PMC_HANDLER).
    @param [in] disc_cb  Disconnect callback (PMC_DISCONNECT_CB), may be NULL.
    @return PMC_ERR_OPEN on setup failure; does not return otherwise.
    @retval PMC_ERR_OPEN  new or Open failed.
    @retval 0             Unreachable under normal operation (Fs_RunServer
                          loops forever). */
extern "C" APIRET EXPENTRY Fs_TrnRunListener(void *handler, void *disc_cb)
{
    PMC_HANDLER       h    = (PMC_HANDLER)handler;
    PMC_DISCONNECT_CB disc = (PMC_DISCONNECT_CB)disc_cb;
    PipeLink         *pServer;

    pServer = new PipeLink(FREEPM_PIPE_NAME, PIPE_SERVER_MODE);
    if (!pServer) return PMC_ERR_OPEN;

    if (pServer->Open() != 0) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMPIPE: Fs_TrnRunListener: Open failed");
        delete pServer;
        return PMC_ERR_OPEN;
    }

    F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_VERBOSE,
               "PMPIPE: Fs_TrnRunListener: listening on %s",
               FREEPM_PIPE_NAME);

    Fs_RunServer(pServer, h, disc);

    delete pServer;
    return 0;
}
