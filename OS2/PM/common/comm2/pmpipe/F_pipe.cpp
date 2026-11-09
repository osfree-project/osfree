/*! @file F_pipe.cpp
    @brief Named-pipe transport for FreePM: PipeLink + 2 backend exports.

    Byte-mode pipe. Client retries DosOpen on ERROR_PIPE_BUSY via
    DosWaitNPipe. Listener recovery: replacement created before hand-off.
*/
#include <string.h>
#include <process.h>

#include "F_pipe.hpp"
#include "F_debug.h"

/*! @brief Create one listener pipe instance.
    @param [in]  name  Pipe name.
    @param [out] ph    Receives the listener handle.
    @return 0 on success, -1 on failure. */
static int PipeCreateListener(const char *name, HPIPE *ph)
{
    ULONG openMode = NP_ACCESS_DUPLEX | NP_NOINHERIT;
    ULONG pipeMode = (ULONG)(FREEPM_PIPE_MAX_INST & 0xFF);

    return (DosCreateNPipe((PSZ)name, ph, openMode, pipeMode,
                           FREEPM_PIPE_OUTB_SIZE, FREEPM_PIPE_INPB_SIZE,
                           FREEPM_PIPE_TIMEOUT) == 0) ? 0 : -1;
}

/*! @brief Construct a client PipeLink with default name. */
PipeLink::PipeLink()
    : Hpipe(NULLHANDLE), mode(PIPE_CLIENT_MODE), connected(0)
{
    strcpy(name, FREEPM_PIPE_NAME);
}

/*! @brief Construct a PipeLink with explicit name and mode. */
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

/*! @brief Destroy the PipeLink. */
PipeLink::~PipeLink()
{
    Close();
}

/*! @brief Open: create listener (server) or connect (client).
    Client retries DosOpen on ERROR_PIPE_BUSY via DosWaitNPipe. */
int PipeLink::Open(void)
{
    if (mode == PIPE_SERVER_MODE) {
        if (PipeCreateListener(name, &Hpipe) != 0) return -1;
        return 0;
    }
    else {
        ULONG ulAction = 0;
        APIRET rc;

        for (;;) {
            rc = DosOpen((PSZ)name, &Hpipe, &ulAction, 0, 0,
                         OPEN_ACTION_OPEN_IF_EXISTS,
                         OPEN_FLAGS_WRITE_THROUGH |
                         OPEN_FLAGS_FAIL_ON_ERROR |
                         OPEN_FLAGS_RANDOM |
                         OPEN_SHARE_DENYNONE |
                         OPEN_ACCESS_READWRITE,
                         NULL);
            if (rc == 0) break;
            if (rc != ERROR_PIPE_BUSY) return -1;
            rc = DosWaitNPipe((PSZ)name, FREEPM_PIPE_TIMEOUT);
            if (rc != 0) return -1;
        }
        connected = 1;
        return 0;
    }
}

/*! @brief Accept one incoming client (server mode). */
int PipeLink::Accept(F_Link **ppNew)
{
    PipeLink *pClient;
    HPIPE     oldHpipe = Hpipe;
    HPIPE     newHpipe = NULLHANDLE;
    int       retries;

    if (Hpipe == NULLHANDLE) return -1;
    if (ppNew == NULL) return -1;

    if (DosConnectNPipe(Hpipe) != 0) return -1;

    retries = 0;
    while (PipeCreateListener(name, &newHpipe) != 0) {
        if (++retries >= 10) {
            DosDisConnectNPipe(Hpipe);
            return -1;
        }
        DosSleep(100);
    }

    pClient = new PipeLink();
    if (!pClient) {
        DosClose(newHpipe);
        DosDisConnectNPipe(Hpipe);
        return -1;
    }

    pClient->Hpipe     = oldHpipe;
    pClient->connected = 1;
    pClient->mode      = PIPE_SERVER_MODE;

    Hpipe = newHpipe;
    *ppNew = pClient;
    return 0;
}

/*! @brief Perform the FreePM handshake. */
int PipeLink::Handshake(void)
{
    char  buf[32];
    ULONG cb;
    ULONG inpLen = (ULONG)strlen(HAND_SHAKE_INP) + 1;
    ULONG outLen = (ULONG)strlen(HAND_SHAKE_OUT) + 1;

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

/*! @brief Send exactly cbData bytes. Loops on partial write. */
int PipeLink::Send(const void *pvData, ULONG cbData)
{
    const char *p = (const char *)pvData;
    ULONG total = 0;

    if (cbData == 0) return 0;
    if (!connected) return -1;
    if (!pvData) return -1;

    while (total < cbData) {
        ULONG chunk = cbData - total;
        ULONG done  = 0;
        APIRET rc;

        if (chunk > FREEPM_PIPE_OUTB_SIZE) chunk = FREEPM_PIPE_OUTB_SIZE;

        rc = DosWrite(Hpipe, (PVOID)(p + total), chunk, &done);
        if (rc != 0) return -1;
        if (done == 0) return -1;
        total += done;
    }
    return 0;
}

/*! @brief Receive exactly cbBuffer bytes. Loops on partial read. */
int PipeLink::Recv(void *pvData, ULONG *pcbReceived, ULONG cbBuffer)
{
    char *p = (char *)pvData;
    ULONG total = 0;

    if (pcbReceived) *pcbReceived = 0;
    if (cbBuffer == 0) return 0;
    if (!connected) return -1;
    if (!pvData) return -1;

    while (total < cbBuffer) {
        ULONG want = cbBuffer - total;
        ULONG done = 0;
        APIRET rc;

        if (want > FREEPM_PIPE_INPB_SIZE) want = FREEPM_PIPE_INPB_SIZE;

        rc = DosRead(Hpipe, p + total, want, &done);
        if (rc != 0) {
            if (pcbReceived) *pcbReceived = total;
            return -1;
        }
        if (done == 0) {
            if (pcbReceived) *pcbReceived = total;
            return -1;
        }
        total += done;
    }
    if (pcbReceived) *pcbReceived = total;
    return 0;
}

/*! @brief Close the pipe handle. Idempotent. */
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

/*! @brief Open a client pipe connection and perform the handshake. */
extern "C" APIRET EXPENTRY Fc_TrnConnect(char *address, ULONG *obj)
{
    PipeLink *pNew;

    if (!obj) return PMC_ERR_OPEN;
    *obj = 0;

    pNew = new PipeLink(address, PIPE_CLIENT_MODE);
    if (pNew == NULL) return PMC_ERR_OPEN;

    if (pNew->Open() != 0) { delete pNew; return PMC_ERR_OPEN; }

    if (pNew->Handshake() == HAND_SHAKE_ERROR) {
        delete pNew;
        return HAND_SHAKE_ERROR;
    }

    *obj = (ULONG)pNew;
    return 0;
}

/*! @brief Server-side listener loop. Does not return on success. */
extern "C" APIRET EXPENTRY Fs_TrnRunListener(void *handler, void *disc_cb)
{
    PMC_HANDLER       h    = (PMC_HANDLER)handler;
    PMC_DISCONNECT_CB disc = (PMC_DISCONNECT_CB)disc_cb;
    PipeLink         *pServer;

    pServer = new PipeLink(FREEPM_PIPE_NAME, PIPE_SERVER_MODE);
    if (!pServer) return PMC_ERR_OPEN;

    if (pServer->Open() != 0) {
        delete pServer;
        return PMC_ERR_OPEN;
    }

    Fs_RunServer(pServer, h, disc);

    delete pServer;
    return 0;
}
