/*! @file F_queue.cpp
    @brief OS/2-queue transport for FreePM: QueueLink + 2 backend exports.

    Connection establishment: client creates its reply queue, opens the
    server listener queue, sends HELLO carrying its reply queue name.
    Server Accept reads HELLO from the listener queue, creates a per-client
    queue, opens the client reply queue, sends WELCOME carrying the
    per-client queue name. Handshake follows on the established queues.

    DosReadQueue returns element data via a freshly-allocated buffer
    (ppElemHandle); caller must DosFreeMem it after copying.
*/
#include <stdio.h>
#include <string.h>
#include <process.h>

#include "F_queue.hpp"
#include "F_debug.h"

/*! @def QMSG_HELLO
    @brief Message kind: HELLO (client -> server). */
#define QMSG_HELLO    1
/*! @def QMSG_WELCOME
    @brief Message kind: WELCOME (server -> client). */
#define QMSG_WELCOME  2

/*! @brief Client-to-server HELLO payload. */
typedef struct _QHelloMsg {
    ULONG cmd;                 /*!< QMSG_HELLO. */
    char  clientQueue[256];    /*!< Client reply queue name. */
} QHelloMsg;

/*! @brief Server-to-client WELCOME payload. */
typedef struct _QWelcomeMsg {
    ULONG cmd;                 /*!< QMSG_WELCOME. */
    char  serverQueue[256];    /*!< Per-client server queue name. */
} QWelcomeMsg;

/*! @brief Get current process id (OS/2 and Win32).
    @return Process id.
    @retval 0  GetInfoBlocks failed (OS/2 only; not expected).
    @retval pid  Process id. */
static ULONG QueueGetPid(void)
{
#if defined(__OS2__) || defined(__OS2V2__)
    PTIB ptib;
    PPIB ppib;
    if (DosGetInfoBlocks(&ptib, &ppib) == 0)
        return ppib->pib_ulpid;
    return 0;
#else
    return (ULONG)GetCurrentProcessId();
#endif
}

/*! @brief Read one queue element into a caller buffer.

    Wraps DosReadQueue, copies out of the system-allocated element
    buffer, and frees it with DosFreeMem. Handles truncation.

    @param [in]  hq       Queue handle.
    @param [out] pvOut    Destination (may be NULL if cbMax == 0).
    @param [out] pcbOut   Receives bytes copied.
    @param [in]  cbMax    Capacity of pvOut.
    @return 0 on success, -1 on failure.
    @retval -1  DosReadQueue failed.
    @retval 0   Element read. */
static int QueueReadInto(HQUEUE hq, void *pvOut, int *pcbOut, int cbMax)
{
    REQUESTDATA req;
    ULONG       cbData;
    PVOID       pElem = NULL;
    BYTE        bPrio = 0;

    if (pcbOut) *pcbOut = 0;
    cbData = (ULONG)cbMax;
    req.ulData = 0;
    req.pid    = 0;

    if (DosReadQueue(hq, &req, &cbData, &pElem,
                     SEM_INDEFINITE_WAIT, DCWW_WAIT, &bPrio, 0) != 0)
        return -1;

    if (pElem && cbData > 0 && pvOut) {
        ULONG copyLen = cbData;
        if (copyLen > (ULONG)cbMax) copyLen = (ULONG)cbMax;
        memcpy(pvOut, pElem, copyLen);
        if (pcbOut) *pcbOut = (int)copyLen;
    } else if (pcbOut) {
        *pcbOut = (int)cbData;
    }

    if (pElem) DosFreeMem(pElem);
    return 0;
}

/* ======================================================================== */
/*  QueueLink                                                              */
/* ======================================================================== */

/*! @brief Construct a client QueueLink with default name.
    Queues not opened.
    @return Nothing. */
QueueLink::QueueLink()
    : hqRecv(NULL), hqSend(NULL), mode(QUEUE_CLIENT_MODE), connected(0)
{
    strcpy(name, FREEPM_QUEUE_NAME);
}

/*! @brief Construct a QueueLink with explicit name and mode.
    Queues not opened.
    @param [in] pszName  Queue name (NULL uses FREEPM_QUEUE_NAME).
    @param [in] iMode    QUEUE_CLIENT_MODE or QUEUE_SERVER_MODE.
    @return Nothing. */
QueueLink::QueueLink(const char *pszName, int iMode)
    : hqRecv(NULL), hqSend(NULL), mode(iMode), connected(0)
{
    if (pszName) {
        strncpy(name, pszName, sizeof(name) - 1);
        name[sizeof(name) - 1] = 0;
    } else {
        strcpy(name, FREEPM_QUEUE_NAME);
    }
}

/*! @brief Destroy the QueueLink. Closes both queues.
    @return Nothing. */
QueueLink::~QueueLink()
{
    Close();
}

/*! @brief Open the listener queue (server) or connect to the server (client).

    Server mode: create the listener queue. Client mode: create client
    reply queue, open server listener queue, send HELLO, wait WELCOME on
    own queue, open per-client server queue.

    @return 0 on success, -1 on failure.
    @retval 0   Listener created (server) or connected (client).
    @retval -1  Queue create/open/HELLO/WELCOME failed. */
int QueueLink::Open(void)
{
    if (mode == QUEUE_SERVER_MODE) {
        if (DosCreateQueue(&hqRecv, QUE_FIFO, (PSZ)name) != 0) {
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMQUEUE: QueueLink::Open ERROR: DosCreateQueue failed");
            return -1;
        }
        return 0;
    }
    else {
        char        clientQName[256];
        QHelloMsg   hello;
        QWelcomeMsg welcome;
        ULONG       ownerPid;
        ULONG       cbRead;
        int         cb;
        HQUEUE      hqServer, hqServerPC;
        static int  clientCounter = 0;

        _snprintf(clientQName, sizeof(clientQName),
                  "\\QUEUES\\FREEPM_C_%d_%d",
                  (int)QueueGetPid(), ++clientCounter);
        clientQName[sizeof(clientQName) - 1] = 0;

        if (DosCreateQueue(&hqRecv, QUE_FIFO, (PSZ)clientQName) != 0) {
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMQUEUE: QueueLink::Open ERROR: client queue create");
            return -1;
        }

        ownerPid = 0;
        if (DosOpenQueue(&ownerPid, &hqServer, (PSZ)name) != 0) {
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMQUEUE: QueueLink::Open ERROR: server queue open");
            DosCloseQueue(hqRecv); hqRecv = NULL;
            return -1;
        }

        hello.cmd = QMSG_HELLO;
        strncpy(hello.clientQueue, clientQName, sizeof(hello.clientQueue) - 1);
        hello.clientQueue[sizeof(hello.clientQueue) - 1] = 0;
        if (DosWriteQueue(hqServer, 0, sizeof(hello), &hello, 0) != 0) {
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMQUEUE: QueueLink::Open ERROR: HELLO write");
            DosCloseQueue(hqServer);
            DosCloseQueue(hqRecv); hqRecv = NULL;
            return -1;
        }
        DosCloseQueue(hqServer);

        cbRead = sizeof(welcome);
        if (QueueReadInto(hqRecv, &welcome, &cb, (int)cbRead) != 0 ||
            cb != (int)sizeof(welcome) ||
            welcome.cmd != QMSG_WELCOME) {
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMQUEUE: QueueLink::Open ERROR: bad WELCOME");
            DosCloseQueue(hqRecv); hqRecv = NULL;
            return -1;
        }

        ownerPid = 0;
        if (DosOpenQueue(&ownerPid, &hqServerPC,
                         (PSZ)welcome.serverQueue) != 0) {
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMQUEUE: QueueLink::Open ERROR: per-client queue open");
            DosCloseQueue(hqRecv); hqRecv = NULL;
            return -1;
        }
        hqSend    = hqServerPC;
        connected = 1;
        return 0;
    }
}

/*! @brief Accept one incoming client (server mode).

    Reads HELLO from the listener queue, creates a per-client server
    queue, opens the client reply queue, sends WELCOME, returns a new
    QueueLink.

    @param [out] ppNew  Receives a new QueueLink for the accepted connection.
    @return 0 on success, -1 on failure.
    @retval 0   Connection accepted.
    @retval -1  Not a listener, bad args, or queue op failed. */
int QueueLink::Accept(F_Link **ppNew)
{
    QHelloMsg   hello;
    QWelcomeMsg welcome;
    ULONG       ownerPid;
    ULONG       cbRead;
    int         cb;
    HQUEUE      hqClient, hqServerPC;
    QueueLink  *pNew;
    static int  serverCounter = 0;
    char        serverQCName[256];

    if (hqRecv == NULL) return -1;
    if (ppNew == NULL) return -1;

    cbRead = sizeof(hello);
    if (QueueReadInto(hqRecv, &hello, &cb, (int)cbRead) != 0 ||
        cb != (int)sizeof(hello) ||
        hello.cmd != QMSG_HELLO)
        return -1;

    ownerPid = 0;
    if (DosOpenQueue(&ownerPid, &hqClient, (PSZ)hello.clientQueue) != 0)
        return -1;

    _snprintf(serverQCName, sizeof(serverQCName),
              "\\QUEUES\\FREEPM_S_%d_%d",
              (int)QueueGetPid(), ++serverCounter);
    serverQCName[sizeof(serverQCName) - 1] = 0;
    if (DosCreateQueue(&hqServerPC, QUE_FIFO, (PSZ)serverQCName) != 0) {
        DosCloseQueue(hqClient);
        return -1;
    }

    welcome.cmd = QMSG_WELCOME;
    strncpy(welcome.serverQueue, serverQCName, sizeof(welcome.serverQueue) - 1);
    welcome.serverQueue[sizeof(welcome.serverQueue) - 1] = 0;
    if (DosWriteQueue(hqClient, 0, sizeof(welcome), &welcome, 0) != 0) {
        DosCloseQueue(hqServerPC);
        DosCloseQueue(hqClient);
        return -1;
    }

    pNew = new QueueLink();
    if (!pNew) {
        DosCloseQueue(hqServerPC);
        DosCloseQueue(hqClient);
        return -1;
    }
    pNew->hqRecv    = hqServerPC;
    pNew->hqSend    = hqClient;
    pNew->mode      = QUEUE_SERVER_MODE;
    pNew->connected = 1;
    *ppNew = pNew;
    return 0;
}

/*! @brief Perform the FreePM handshake.
    @return 0 on success, HAND_SHAKE_ERROR on protocol mismatch/transport error.
    @retval 0                 Handshake completed.
    @retval HAND_SHAKE_ERROR  Magic mismatch or transport failure. */
int QueueLink::Handshake(void)
{
    char buf[32];
    int  cb;
    int  inpLen = (int)strlen(HAND_SHAKE_INP) + 1;
    int  outLen = (int)strlen(HAND_SHAKE_OUT) + 1;

    if (mode == QUEUE_SERVER_MODE) {
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

/*! @brief Send exactly cbData bytes as one queue element.
    @param [in] pvData  Source buffer.
    @param [in] cbData  Bytes to send (<= 0 is a no-op).
    @return 0 on success, -1 on error.
    @retval 0   Element written.
    @retval -1  Not connected or DosWriteQueue failed. */
int QueueLink::Send(const void *pvData, int cbData)
{
    if (cbData <= 0) return 0;
    if (!connected) return -1;
    if (DosWriteQueue(hqSend, 0, (ULONG)cbData, (PVOID)pvData, 0) != 0) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMQUEUE: QueueLink::Send ERROR: DosWriteQueue failed");
        return -1;
    }
    return 0;
}

/*! @brief Receive exactly cbBuffer bytes from one queue element.

    On short read sets *pcbReceived = bytes-read-so-far and returns -1.

    @param [out] pvData       Destination.
    @param [out] pcbReceived  Bytes received.
    @param [in]  cbBuffer     Bytes required.
    @return 0 on success, -1 on peer-close or error.
    @retval 0   Exactly cbBuffer bytes received.
    @retval -1  Not connected, short read, or DosReadQueue failed. */
int QueueLink::Recv(void *pvData, int *pcbReceived, int cbBuffer)
{
    int cb = 0;

    if (pcbReceived) *pcbReceived = 0;
    if (cbBuffer <= 0) return 0;
    if (!connected) return -1;

    if (QueueReadInto(hqRecv, pvData, &cb, cbBuffer) != 0) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMQUEUE: QueueLink::Recv ERROR: DosReadQueue failed");
        return -1;
    }
    if (cb < cbBuffer) {
        if (pcbReceived) *pcbReceived = cb;
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMQUEUE: QueueLink::Recv ERROR: short read %d of %d",
                   cb, cbBuffer);
        return -1;
    }
    if (pcbReceived) *pcbReceived = cb;
    return 0;
}

/*! @brief Close both queues. Idempotent.
    @return 0 always.
    @retval 0  Always. */
int QueueLink::Close(void)
{
    if (hqRecv) { DosCloseQueue(hqRecv); hqRecv = NULL; }
    if (hqSend) { DosCloseQueue(hqSend); hqSend = NULL; }
    connected = 0;
    return 0;
}

/* ======================================================================== */
/*  Backend exports                                                        */
/* ======================================================================== */

/*! @brief Open a client queue connection and perform the handshake.
    @param [in]  address  Server queue name, or NULL for FREEPM_QUEUE_NAME.
    @param [out] obj      Receives the connection object; set to 0 on error.
    @return 0 on success, non-zero on failure.
    @retval 0                  Connected.
    @retval PMC_ERR_OPEN       new or Open failed.
    @retval HAND_SHAKE_ERROR   Handshake mismatch. */
extern "C" APIRET EXPENTRY Fc_TrnConnect(char *address, ULONG *obj)
{
    QueueLink *pNew;

    if (!obj) return PMC_ERR_OPEN;
    *obj = 0;

    pNew = new QueueLink(address, QUEUE_CLIENT_MODE);
    if (pNew == NULL) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMQUEUE: Fc_TrnConnect ERROR: new QueueLink failed");
        return PMC_ERR_OPEN;
    }

    if (pNew->Open() != 0) {
        delete pNew;
        return PMC_ERR_OPEN;
    }

    if (pNew->Handshake() == HAND_SHAKE_ERROR) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMQUEUE: Fc_TrnConnect ERROR: Handshake failed");
        delete pNew;
        return HAND_SHAKE_ERROR;
    }

    *obj = (ULONG)pNew;
    return 0;
}

/*! @brief Server-side listener loop: create, accept, spawn thread.

    Does not return on success.

    @param [in] handler  Command handler callback (PMC_HANDLER).
    @param [in] disc_cb  Disconnect callback (PMC_DISCONNECT_CB), may be NULL.
    @return PMC_ERR_OPEN on setup failure; does not return otherwise.
    @retval PMC_ERR_OPEN  new or Open failed.
    @retval 0             Unreachable under normal operation. */
extern "C" APIRET EXPENTRY Fs_TrnRunListener(void *handler, void *disc_cb)
{
    PMC_HANDLER       h    = (PMC_HANDLER)handler;
    PMC_DISCONNECT_CB disc = (PMC_DISCONNECT_CB)disc_cb;
    QueueLink        *pServer;

    pServer = new QueueLink(FREEPM_QUEUE_NAME, QUEUE_SERVER_MODE);
    if (!pServer) return PMC_ERR_OPEN;

    if (pServer->Open() != 0) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMQUEUE: Fs_TrnRunListener: Open failed");
        delete pServer;
        return PMC_ERR_OPEN;
    }

    F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_VERBOSE,
               "PMQUEUE: Fs_TrnRunListener: listening on %s",
               FREEPM_QUEUE_NAME);

    Fs_RunServer(pServer, h, disc);

    delete pServer;
    return 0;
}
