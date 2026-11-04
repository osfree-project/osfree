/*! @file F_pipe.cpp
    @brief OS/2 named-pipe transport for FreePM.
*/
#if defined(__OS2__) || defined(__OS2V2__)

#include <string.h>
#include <process.h>

#include "F_pipe.hpp"
#include "F_wire.hpp"
#include "F_debug.hpp"

/* ======================================================================== */
/*  PipeLink                                                                */
/* ======================================================================== */

PipeLink::PipeLink()
    : Hpipe(NULLHANDLE), mode(PIPE_CLIENT_MODE), connected(0)
{
    strcpy(name, FREEPM_PIPE_NAME);
}

PipeLink::PipeLink(const char *pszName, int iMode)
    : Hpipe(NULLHANDLE), mode(iMode), connected(0)
{
    strncpy(name, pszName, sizeof(name) - 1);
    name[sizeof(name) - 1] = 0;
}

PipeLink::~PipeLink()
{
    Close();
}

/* Internal helper: create one listener instance. The max-instance
   count lives in the low byte of the pipe mode; each call increments
   the internal instance index automatically. */
static int PipeCreateListener(const char *name, HPIPE *ph)
{
    ULONG   openMode  = NP_ACCESS_DUPLEX | NP_NOINHERIT;
    ULONG   pipeMode  = NP_WMESG | NP_RMESG |
                        (ULONG)(FREEPM_PIPE_MAX_INST & 0xFF);

    return (DosCreateNPipe((PSZ)name, ph, openMode, pipeMode,
                           FREEPM_PIPE_OUTB_SIZE, FREEPM_PIPE_INPB_SIZE,
                           FREEPM_PIPE_TIMEOUT) == 0) ? 0 : -1;
}

int PipeLink::Open(void)
{
    if (mode == PIPE_SERVER_MODE)
    {
        if (PipeCreateListener(name, &Hpipe) != 0) {
            F_Dbg("PIPE: DosCreateNPipe failed\n");
            return -1;
        }
        return 0;
    }
    else
    {
        ULONG   ulAction  = 0;
        ULONG   openMode  = OPEN_ACTION_OPEN_IF_EXISTS;

        if (DosOpen((PSZ)name, &Hpipe, &ulAction, 0, 0,
                    openMode,
                    OPEN_FLAGS_WRITE_THROUGH |
                    OPEN_FLAGS_FAIL_ON_ERROR |
                    OPEN_FLAGS_RANDOM |
                    OPEN_SHARE_DENYNONE |
                    OPEN_ACCESS_READWRITE,
                    FREEPM_PIPE_TIMEOUT) != 0)
        {
            F_Dbg("PIPE: DosOpen failed\n");
            return -1;
        }
        connected = 1;
        return 0;
    }
}

int PipeLink::Accept(F_Link **ppNew)
{
    PipeLink *pClient;
    HPIPE     oldHpipe = Hpipe;

    if (DosConnectNPipe(Hpipe) != 0) return -1;

    pClient = new PipeLink();
    if (!pClient) { DosDisConnectNPipe(Hpipe); return -1; }

    pClient->Hpipe     = oldHpipe;
    pClient->connected = 1;
    pClient->mode      = PIPE_SERVER_MODE;

    Hpipe = NULLHANDLE;
    if (PipeCreateListener(name, &Hpipe) != 0) {
        F_Dbg("PIPE: listener re-create failed, no further accepts\n");
    }

    *ppNew = pClient;
    return 0;
}

int PipeLink::Handshake(void)
{
    char  buf[32];
    int   cb;
    return F_LinkHandshakeCommon(this, buf, &cb);
}

/* Note: F_LinkHandshakeCommon must be provided; if not, use the
   Handshake body below instead. For now, inline. */

#undef HAND_SHAKE_COMMON_PLACEHOLDER
int PipeLink_HandshakeInline(PipeLink *p);

int PipeLink::Handshake(void)
{
    char buf[32];
    int  cb;
    ULONG done = 0;

    if (mode == PIPE_SERVER_MODE) {
        if (Recv(buf, &cb, (int)strlen(HAND_SHAKE_INP) + 1) != 0) return HAND_SHAKE_ERROR;
        if (strcmp(buf, HAND_SHAKE_INP) != 0) return HAND_SHAKE_ERROR;
        if (Send(HAND_SHAKE_OUT, (int)strlen(HAND_SHAKE_OUT) + 1) != 0) return HAND_SHAKE_ERROR;
    } else {
        if (Send(HAND_SHAKE_INP, (int)strlen(HAND_SHAKE_INP) + 1) != 0) return HAND_SHAKE_ERROR;
        if (Recv(buf, &cb, (int)strlen(HAND_SHAKE_OUT) + 1) != 0) return HAND_SHAKE_ERROR;
        if (strcmp(buf, HAND_SHAKE_OUT) != 0) return HAND_SHAKE_ERROR;
    }
    return 0;
}

int PipeLink::Send(const void *pvData, int cbData)
{
    ULONG done = 0;

    if (!connected) return -1;
    if (DosWrite(Hpipe, (PVOID)pvData, (ULONG)cbData, &done) != 0) return -1;
    return (done == (ULONG)cbData) ? 0 : -1;
}

int PipeLink::Recv(void *pvData, int *pcbReceived, int cbBuffer)
{
    ULONG done = 0;
    APIRET rc;

    if (!connected) return -1;
    rc = DosRead(Hpipe, pvData, (ULONG)cbBuffer, &done);
    if (rc != 0 && rc != ERROR_MORE_DATA) return -1;
    *pcbReceived = (int)done;
    return (done == (ULONG)cbBuffer) ? 0 : -1;
}

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
/*  Exports                                                                 */
/* ======================================================================== */

extern "C" APIRET EXPENTRY _InitServerConnection(char *ExternMachine, ULONG *obj,
                                                 void *cb)
{
    PipeLink *pLink;

    F_Dbg("FPMPIPE: _InitServerConnection IN -> ExternMachine=%s, cb=%p\n",
          ExternMachine ? ExternMachine : "NULL", cb);

    if (cb) g_shutdownCb = (PMC_SHUTDOWN_CB)cb;

    if (g_pClientLink != NULL) {
        delete g_pClientLink;
        g_pClientLink = NULL;
    }

    pLink = new PipeLink(FREEPM_PIPE_NAME, PIPE_CLIENT_MODE);
    if (!pLink) return PMC_ERR_OPEN;

    if (pLink->Open() != 0) { delete pLink; return PMC_ERR_OPEN; }
    if (pLink->Handshake() != 0) {
        pLink->Close();
        delete pLink;
        return HAND_SHAKE_ERROR;
    }

    g_pClientLink = pLink;
    F_WireRegisterFatalSink();
    *obj = (ULONG)pLink;

    F_Dbg("FPMPIPE: _InitServerConnection OUT -> Success (obj=%p)\n", pLink);
    return 0;
}

extern "C" APIRET EXPENTRY _CloseServerConnection(void)
{
    F_Dbg("FPMPIPE: _CloseServerConnection IN\n");
    F_DbgSetWireSink(NULL);
    if (g_pClientLink != NULL) {
        delete g_pClientLink;
        g_pClientLink = NULL;
    }
    return 0;
}

extern "C" APIRET EXPENTRY _startServerThreads(void *handl, void *disc_cb)
{
    PMC_HANDLER       h    = (PMC_HANDLER)handl;
    PMC_DISCONNECT_CB disc = (PMC_DISCONNECT_CB)disc_cb;
    PipeLink         *pListener;

    pListener = new PipeLink(FREEPM_PIPE_NAME, PIPE_SERVER_MODE);
    if (!pListener) return PMC_ERR_OPEN;

    if (pListener->Open() != 0) {
        F_Dbg("FPMPIPE: listener Open failed\n");
        delete pListener;
        return PMC_ERR_OPEN;
    }

    F_Dbg("FPMPIPE: listening on %s\n", FREEPM_PIPE_NAME);

    F_RunServer(pListener, h, disc);

    delete pListener;
    return 0;
}

/* ======================================================================== */
/*  DLL entry point                                                         */
/* ======================================================================== */

unsigned long _System _DLL_InitTerm(unsigned long hModule, unsigned long flag)
{
    (void)hModule;
    if (flag == 0) return 1;

    if (g_shutdownCb) {
        g_shutdownCb();
        g_shutdownCb = NULL;
    }
    F_DbgSetWireSink(NULL);
    if (g_pClientLink) {
        delete g_pClientLink;
        g_pClientLink = NULL;
    }
    return 1;
}

#endif /* __OS2__ */
