/*! @file F_shm.cpp
    @brief Shared-memory transport for FreePM: ShmLink + 2 backend exports.

    Data travels through shared memory; OS/2 queues carry only 8-byte
    {slot, length} notifications. Send/Recv split/assemble arbitrary
    length into QMS_SLOT_SIZE chunks; from F_Link's perspective they
    are stream operations. Names derived from (clientPid, clientTid) by
    formula; only {version, pid, tid} crosses the wire during handshake
    (9 bytes via main queue, 9 bytes echo via C_).

    Peer-death detection: Recv and Accept poll with a bounded timeout.
    QMS_RECV_TIMEOUT_MS must exceed the PMCLIENT keepalive interval.

    Memory: shared regions are reserved (PAG_READ | PAG_WRITE, no
    PAG_COMMIT); pages commit on first access.
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>

#include "F_shm.hpp"
#include "F_debug.h"

/*! @def QMS_VERSION
    @brief Queue handshake protocol version. */
#define QMS_VERSION 1

/*! @def QMS_HANDSHAKE_TIMEOUT_MS
    @brief Timeout for Accept-side hello read, milliseconds. */
#define QMS_HANDSHAKE_TIMEOUT_MS 30000

/*! @def QMS_RECV_TIMEOUT_MS
    @brief Idle timeout for Recv on an established connection, milliseconds.
    Must exceed PMCLIENT keepalive interval (ulInactive/2). */
#define QMS_RECV_TIMEOUT_MS 60000

/*! @def QMS_POLL_MS
    @brief Polling granularity inside queue read/wait loops, milliseconds. */
#define QMS_POLL_MS 10

/*! @brief Handshake payload (wire 9 bytes). */
#pragma pack(1)
typedef struct _QmsHello {
    BYTE  version;
    ULONG pid;
    ULONG tid;
} QmsHello;
#pragma pack()

/*! @brief Queue element for chunk notification (8 bytes). */
#pragma pack(1)
typedef struct _QmsElem {
    ULONG slot;
    ULONG length;
} QmsElem;
#pragma pack()

/* ======================================================================== */
/*  Names                                                                  */
/* ======================================================================== */

/*! @brief Build queue name from (pid, tid) and direction prefix C or S.
    @param [out] buf     Destination buffer.
    @param [in]  cb      Capacity in bytes.
    @param [in]  prefix  'C' or 'S'.
    @param [in]  pid     Client process id.
    @param [in]  tid     Client thread id. */
static void QmsQueueName(char *buf, int cb, char prefix, ULONG pid, ULONG tid)
{
    _snprintf(buf, cb, "\\QUEUES\\FREEPM_%c_%u_%u", prefix, pid, tid);
    buf[cb - 1] = 0;
}

/*! @brief Build shared-memory name from (pid, tid) and direction.
    @param [out] buf  Destination buffer.
    @param [in]  cb   Capacity in bytes.
    @param [in]  dir  "C2S" or "S2C".
    @param [in]  pid  Client process id.
    @param [in]  tid  Client thread id. */
static void QmsShmName(char *buf, int cb, const char *dir, ULONG pid, ULONG tid)
{
    _snprintf(buf, cb, "\\SHAREMEM\\FREEPM_QMS_%s_%u_%u", dir, pid, tid);
    buf[cb - 1] = 0;
}

/* ======================================================================== */
/*  Self identity                                                          */
/* ======================================================================== */

/*! @brief Fill *pid and *tid for the calling thread.
    @param [out] pid  Receives process id.
    @param [out] tid  Receives thread id. */
static void QmsSelfIds(ULONG *pid, ULONG *tid)
{
    PTIB ptib;
    PPIB ppib;
    DosGetInfoBlocks(&ptib, &ppib);
    *pid = (ULONG)ppib->pib_ulpid;
    *tid = (ULONG)ptib->tib_ptib2->tib2_ultid;
}

/* ======================================================================== */
/*  Shared memory region lifetime                                          */
/* ======================================================================== */

/*! @brief Total data region size in bytes.
    @return Bytes. */
static ULONG QmsRegionSize(void)
{
    return (ULONG)sizeof(QmsHeader);
}

/*! @brief Create a named data region (server side, owner).

    On ERROR_ALREADY_EXISTS (stale region from a crashed predecessor)
    closes the returned handle and retries once. Region is reserved
    (PAG_READ | PAG_WRITE) without PAG_COMMIT: pages commit on first
    access, so idle connections cost only the header.

    @param [in]  name      OS/2 shared-memory name.
    @param [out] ppRegion  Receives mapped base.
    @return 0 on success, -1 on failure.
    @retval 0   Region created and initialised.
    @retval -1  DosAllocSharedMem failed after retry. */
static int QmsRegionCreate(const char *name, QmsHeader **ppRegion)
{
    PVOID  p  = NULL;
    APIRET rc;

    F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_VERBOSE,
               "PMSHM: QmsRegionCreate('%s', cb=%u) enter",
               name, (unsigned)QmsRegionSize());

    rc = DosAllocSharedMem(&p, name, QmsRegionSize(), PAG_READ | PAG_WRITE);
    if (rc == ERROR_ALREADY_EXISTS) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMSHM: QmsRegionCreate('%s'): stale region "
                   "ERROR_ALREADY_EXISTS, retrying", name);
        if (p) DosFreeMem(p);
        rc = DosAllocSharedMem(&p, name, QmsRegionSize(), PAG_READ | PAG_WRITE);
        if (rc == ERROR_ALREADY_EXISTS) {
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMSHM: QmsRegionCreate('%s'): still exists after "
                       "retry (stale region held by another process, "
                       "or previous run not cleaned up)", name);
        }
    }
    if (rc != 0) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMSHM: QmsRegionCreate('%s') FAILED rc=%u",
                   name, (unsigned)rc);
        return -1;
    }

    {
        QmsHeader *h = (QmsHeader *)p;
        h->magic    = QMS_MAGIC;
        h->nSlots   = QMS_SLOTS;
        h->slotSize = QMS_SLOT_SIZE;
        h->writeIdx = 0;
        h->readIdx  = 0;
        h->pad[0] = h->pad[1] = h->pad[2] = 0;
    }
    *ppRegion = (QmsHeader *)p;

    F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_VERBOSE,
               "PMSHM: QmsRegionCreate('%s') OK, base=%p", name, p);
    return 0;
}

/*! @brief Attach to an existing named data region.
    @param [in]  name      OS/2 shared-memory name.
    @param [out] ppRegion  Receives mapped base.
    @return 0 on success, -1 on failure.
    @retval 0   Region mapped and magic verified.
    @retval -1  DosGetNamedSharedMem failed or bad magic. */
static int QmsRegionOpen(const char *name, QmsHeader **ppRegion)
{
    PVOID  p  = NULL;
    APIRET rc;

    F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_VERBOSE,
               "PMSHM: QmsRegionOpen('%s') enter", name);

    rc = DosGetNamedSharedMem(&p, name, PAG_READ | PAG_WRITE);
    if (rc != 0) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMSHM: QmsRegionOpen('%s'): DosGetNamedSharedMem "
                   "FAILED rc=%u", name, (unsigned)rc);
        return -1;
    }
    if (((QmsHeader *)p)->magic != QMS_MAGIC) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMSHM: QmsRegionOpen('%s'): bad magic 0x%lx "
                   "(expected 0x%lx)", name,
                   (unsigned long)((QmsHeader *)p)->magic,
                   (unsigned long)QMS_MAGIC);
        DosFreeMem(p);
        return -1;
    }
    *ppRegion = (QmsHeader *)p;

    F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_VERBOSE,
               "PMSHM: QmsRegionOpen('%s') OK, base=%p", name, p);
    return 0;
}

/* ======================================================================== */
/*  Handshake via OS/2 queue                                               */
/* ======================================================================== */

/*! @brief Write a QmsHello element to a queue.
    @param [in] hq       Queue handle.
    @param [in] version  Protocol version.
    @param [in] pid      Sender process id.
    @param [in] tid      Sender thread id.
    @return 0 on success, -1 on failure.
    @retval 0   Hello written.
    @retval -1  DosWriteQueue failed. */
static int QmsSendHello(HQUEUE hq, BYTE version, ULONG pid, ULONG tid)
{
    QmsHello h;
    APIRET   rc;

    h.version = version;
    h.pid     = pid;
    h.tid     = tid;

    rc = DosWriteQueue(hq, 0, sizeof(h), &h, 0);
    if (rc != 0) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMSHM: QmsSendHello(hq=%p, ver=%u, pid=%u, tid=%u) "
                   "FAILED rc=%u",
                   hq, (unsigned)version, (unsigned)pid, (unsigned)tid,
                   (unsigned)rc);
        return -1;
    }
    F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_VERBOSE,
               "PMSHM: QmsSendHello(hq=%p, ver=%u, pid=%u, tid=%u) OK",
               hq, (unsigned)version, (unsigned)pid, (unsigned)tid);
    return 0;
}

/*! @brief Read a QmsHello from a queue with optional timeout.
    @param [in]  hq         Queue.
    @param [in]  timeoutMs  0 = block forever; >0 = poll for this many ms.
    @param [out] pHello     Receives hello.
    @return 0 on success, -1 on failure.
    @retval 0   Hello read and copied.
    @retval -1  Timeout, queue error, or wrong element size. */
static int QmsRecvHello(HQUEUE hq, int timeoutMs, QmsHello *pHello)
{
    ULONG elapsed = 0;
    ULONG wait    = (timeoutMs > 0) ? DCWW_NOWAIT : DCWW_WAIT;

    F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_VERBOSE,
               "PMSHM: QmsRecvHello(hq=%p, timeout=%d ms) enter",
               hq, timeoutMs);

    for (;;) {
        REQUESTDATA req;
        PVOID       pElem = NULL;
        ULONG       cb    = sizeof(QmsHello);
        APIRET      rc;

        req.ulData = 0;
        req.pid    = 0;
        rc = DosReadQueue(hq, &req, &cb, &pElem, 0, wait, NULL, 0);

        if (rc == 0) {
            if (pElem && cb == sizeof(QmsHello)) {
                memcpy(pHello, pElem, cb);
                DosFreeMem(pElem);
                F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_VERBOSE,
                           "PMSHM: QmsRecvHello: got ver=%u pid=%u tid=%u "
                           "after %u ms",
                           (unsigned)pHello->version,
                           (unsigned)pHello->pid,
                           (unsigned)pHello->tid,
                           (unsigned)elapsed);
                return 0;
            }
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMSHM: QmsRecvHello: bad element "
                       "(pElem=%p, cb=%u, expected %u)",
                       pElem, (unsigned)cb, (unsigned)sizeof(QmsHello));
            if (pElem) DosFreeMem(pElem);
            return -1;
        }
        if (rc != ERROR_QUE_EMPTY) {
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMSHM: QmsRecvHello: DosReadQueue FAILED rc=%u",
                       (unsigned)rc);
            return -1;
        }
        if (wait != DCWW_NOWAIT) {
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMSHM: QmsRecvHello: queue empty on blocking wait "
                       "(unexpected)");
            return -1;
        }

        if (elapsed >= (ULONG)timeoutMs) {
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMSHM: QmsRecvHello: TIMEOUT after %u ms "
                       "(hq=%p) - no hello from peer",
                       (unsigned)elapsed, hq);
            return -1;
        }
        DosSleep(QMS_POLL_MS);
        elapsed += QMS_POLL_MS;
    }
}

/* ======================================================================== */
/*  ShmLink                                                                */
/* ======================================================================== */

/*! @brief Construct client ShmLink with default listener name.
    @return Nothing. */
ShmLink::ShmLink()
    : hqRecv(NULL), hqSend(NULL),
      pWriteRegion(NULL), pReadRegion(NULL),
      mode(SHM_CLIENT_MODE), connected(0), peerPid(0),
      recvBuf(NULL), recvBufCap(0), recvBufLen(0), recvBufPos(0)
{
    strcpy(name, FREEPM_SHM_QUEUE_NAME);
}

/*! @brief Construct ShmLink with explicit name and mode.
    @param [in] pszName  Listener queue name (server) or unused (client).
    @param [in] iMode    SHM_CLIENT_MODE / SHM_SERVER_MODE.
    @return Nothing. */
ShmLink::ShmLink(const char *pszName, int iMode)
    : hqRecv(NULL), hqSend(NULL),
      pWriteRegion(NULL), pReadRegion(NULL),
      mode(iMode), connected(0), peerPid(0),
      recvBuf(NULL), recvBufCap(0), recvBufLen(0), recvBufPos(0)
{
    if (pszName) {
        strncpy(name, pszName, sizeof(name) - 1);
        name[sizeof(name) - 1] = 0;
    } else {
        strcpy(name, FREEPM_SHM_QUEUE_NAME);
    }
}

/*! @brief Destroy ShmLink. Closes queues and regions.
    @return Nothing. */
ShmLink::~ShmLink()
{
    Close();
}

/*! @brief Open: create listener (server) or connect (client).

    Server: create listener queue only.
    Client: create C_ (queue) and C2S (region), open main, send hello,
    wait for echo (QMS_HANDSHAKE_TIMEOUT_MS), open S_ (queue) and S2C
    (region).

    @return 0 on success, -1 on failure.
    @retval 0   Listener created (server) or connected (client).
    @retval -1  Queue/region/handshake failure. */
int ShmLink::Open(void)
{
    F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_VERBOSE,
               "PMSHM: ShmLink::Open enter (mode=%s, name='%s')",
               (mode == SHM_SERVER_MODE) ? "SERVER" : "CLIENT", name);

    if (mode == SHM_SERVER_MODE) {
        APIRET rc = DosCreateQueue(&hqRecv, QUE_FIFO, name);
        if (rc != 0) {
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMSHM: ShmLink::Open[SERVER] step 1: "
                       "DosCreateQueue('%s') FAILED rc=%u "
                       "(stale listener queue from previous run? "
                       "PMSERVER already running?)",
                       name, (unsigned)rc);
            return -1;
        }
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_VERBOSE,
                   "PMSHM: ShmLink::Open[SERVER] OK, listener='%s' hq=%p",
                   name, hqRecv);
        return 0;
    }
    else {
        char     cQn[256], sQn[256], c2sN[256], s2cN[256];
        HQUEUE   hqMain;
        ULONG    myPid, myTid, ownerPid;
        QmsHello echo;
        APIRET   rc;

        QmsSelfIds(&myPid, &myTid);
        QmsQueueName(cQn,  sizeof(cQn),  'C',   myPid, myTid);
        QmsQueueName(sQn,  sizeof(sQn),  'S',   myPid, myTid);
        QmsShmName(c2sN,   sizeof(c2sN), "C2S", myPid, myTid);
        QmsShmName(s2cN,   sizeof(s2cN), "S2C", myPid, myTid);

        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_VERBOSE,
                   "PMSHM: ShmLink::Open[CLIENT] self pid=%u tid=%u; "
                   "cQ='%s' sQ='%s' c2s='%s' s2c='%s'",
                   (unsigned)myPid, (unsigned)myTid, cQn, sQn, c2sN, s2cN);

        /* step 1/N: create our C_ queue */
        rc = DosCreateQueue(&hqRecv, QUE_FIFO, cQn);
        if (rc != 0) {
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMSHM: ShmLink::Open[CLIENT] step 1/6: "
                       "DosCreateQueue('%s') FAILED rc=%u "
                       "(stale C_ queue from previous run?)",
                       cQn, (unsigned)rc);
            return -1;
        }

        /* step 2/N: create our C2S region */
        if (QmsRegionCreate(c2sN, &pWriteRegion) != 0) {
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMSHM: ShmLink::Open[CLIENT] step 2/6: "
                       "QmsRegionCreate('%s') FAILED (stale C2S region?)",
                       c2sN);
            DosCloseQueue(hqRecv); hqRecv = NULL;
            return -1;
        }

        /* step 3/N: open main listener queue */
        ownerPid = 0;
        rc = DosOpenQueue(&ownerPid, &hqMain, name);
        if (rc != 0) {
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMSHM: ShmLink::Open[CLIENT] step 3/6: "
                       "DosOpenQueue('%s') FAILED rc=%u "
                       "(listener not running / listener name wrong)",
                       name, (unsigned)rc);
            DosFreeMem(pWriteRegion); pWriteRegion = NULL;
            DosCloseQueue(hqRecv); hqRecv = NULL;
            return -1;
        }

        /* step 4/N: send hello */
        if (QmsSendHello(hqMain, QMS_VERSION, myPid, myTid) != 0) {
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMSHM: ShmLink::Open[CLIENT] step 4/6: "
                       "QmsSendHello(main) FAILED");
            DosCloseQueue(hqMain);
            DosFreeMem(pWriteRegion); pWriteRegion = NULL;
            DosCloseQueue(hqRecv); hqRecv = NULL;
            return -1;
        }
        DosCloseQueue(hqMain);

        /* step 5/N: wait for echo hello */
        if (QmsRecvHello(hqRecv, QMS_HANDSHAKE_TIMEOUT_MS, &echo) != 0) {
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMSHM: ShmLink::Open[CLIENT] step 5/6: "
                       "QmsRecvHello(echo) FAILED (timeout %d ms, "
                       "server did not reply)",
                       QMS_HANDSHAKE_TIMEOUT_MS);
            DosFreeMem(pWriteRegion); pWriteRegion = NULL;
            DosCloseQueue(hqRecv); hqRecv = NULL;
            return -1;
        }
        if (echo.version != QMS_VERSION ||
            echo.pid != myPid || echo.tid != myTid) {
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMSHM: ShmLink::Open[CLIENT] step 5/6: "
                       "echo mismatch: got ver=%u pid=%u tid=%u, "
                       "expected ver=%u pid=%u tid=%u",
                       (unsigned)echo.version, (unsigned)echo.pid,
                       (unsigned)echo.tid,
                       (unsigned)QMS_VERSION, (unsigned)myPid,
                       (unsigned)myTid);
            DosFreeMem(pWriteRegion); pWriteRegion = NULL;
            DosCloseQueue(hqRecv); hqRecv = NULL;
            return -1;
        }

        /* step 6/N: open S_ queue and S2C region */
        ownerPid = 0;
        rc = DosOpenQueue(&ownerPid, &hqSend, sQn);
        if (rc != 0) {
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMSHM: ShmLink::Open[CLIENT] step 6/6: "
                       "DosOpenQueue('%s') FAILED rc=%u "
                       "(server should have created S_ before echoing)",
                       sQn, (unsigned)rc);
            DosFreeMem(pWriteRegion); pWriteRegion = NULL;
            DosCloseQueue(hqRecv); hqRecv = NULL;
            return -1;
        }
        if (QmsRegionOpen(s2cN, &pReadRegion) != 0) {
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMSHM: ShmLink::Open[CLIENT] step 6/6: "
                       "QmsRegionOpen('%s') FAILED", s2cN);
            DosCloseQueue(hqSend); hqSend = NULL;
            DosFreeMem(pWriteRegion); pWriteRegion = NULL;
            DosCloseQueue(hqRecv); hqRecv = NULL;
            return -1;
        }

        peerPid   = ownerPid;
        connected = 1;

        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_VERBOSE,
                   "PMSHM: ShmLink::Open[CLIENT] OK, peerPid=%u "
                   "hqRecv=%p hqSend=%p pWrite=%p pRead=%p",
                   (unsigned)peerPid, hqRecv, hqSend,
                   pWriteRegion, pReadRegion);
        return 0;
    }
}

/*! @brief Accept one incoming client (server mode).

    Reads HELLO (with QMS_HANDSHAKE_TIMEOUT_MS), verifies version,
    creates S_ + S2C (own), opens C_ + C2S (peer's), echoes HELLO
    via C_ to confirm.

    @param [out] ppNew  Receives new ShmLink.
    @return 0 on success, -1 on failure.
    @retval 0   Connection accepted.
    @retval -1  Handshake timeout, version mismatch, or resource failure. */
int ShmLink::Accept(F_Link **ppNew)
{
    char       cQn[256], sQn[256], c2sN[256], s2cN[256];
    QmsHello   hello;
    ULONG      cpid, ctid, ownerPid;
    HQUEUE     hqS = NULLHANDLE;
    HQUEUE     hqC = NULLHANDLE;
    QmsHeader *pS2C = NULL;
    QmsHeader *pC2S = NULL;
    ShmLink   *pNew;
    APIRET     rc;

    if (hqRecv == NULL || ppNew == NULL) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMSHM: ShmLink::Accept: bad args (hqRecv=%p, ppNew=%p)",
                   hqRecv, ppNew);
        return -1;
    }

    F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_VERBOSE,
               "PMSHM: ShmLink::Accept: waiting for hello (timeout %d ms)",
               QMS_HANDSHAKE_TIMEOUT_MS);

    if (QmsRecvHello(hqRecv, QMS_HANDSHAKE_TIMEOUT_MS, &hello) != 0) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMSHM: ShmLink::Accept: QmsRecvHello FAILED "
                   "(no client hello within %d ms)",
                   QMS_HANDSHAKE_TIMEOUT_MS);
        return -1;
    }

    cpid = hello.pid;
    ctid = hello.tid;
    QmsQueueName(cQn,  sizeof(cQn),  'C',   cpid, ctid);
    QmsQueueName(sQn,  sizeof(sQn),  'S',   cpid, ctid);
    QmsShmName(c2sN,   sizeof(c2sN), "C2S", cpid, ctid);
    QmsShmName(s2cN,   sizeof(s2cN), "S2C", cpid, ctid);

    F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_VERBOSE,
               "PMSHM: ShmLink::Accept: hello ver=%u cpid=%u ctid=%u; "
               "cQ='%s' sQ='%s' c2s='%s' s2c='%s'",
               (unsigned)hello.version, (unsigned)cpid, (unsigned)ctid,
               cQn, sQn, c2sN, s2cN);

    if (hello.version != QMS_VERSION) {
        HQUEUE hqCerr = NULLHANDLE;
        ownerPid = 0;
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMSHM: ShmLink::Accept: version mismatch "
                   "(got %u, expected %u) - rejecting",
                   (unsigned)hello.version, (unsigned)QMS_VERSION);
        if (DosOpenQueue(&ownerPid, &hqCerr, cQn) == 0) {
            QmsSendHello(hqCerr, 0xFF, 0, 0);
            DosCloseQueue(hqCerr);
        }
        return -1;
    }

    /* step 1: create our S_ queue (with stale-retry) */
    rc = DosCreateQueue(&hqS, QUE_FIFO, sQn);
    if (rc != 0) {
        HQUEUE hqStale = NULLHANDLE;
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMSHM: ShmLink::Accept step 1/5: "
                   "DosCreateQueue('%s') FAILED rc=%u (retrying after "
                   "stale close)", sQn, (unsigned)rc);
        ownerPid = 0;
        if (DosOpenQueue(&ownerPid, &hqStale, sQn) == 0) {
            DosCloseQueue(hqStale);
            rc = DosCreateQueue(&hqS, QUE_FIFO, sQn);
            if (rc != 0) {
                F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                           "PMSHM: ShmLink::Accept step 1/5: "
                           "DosCreateQueue('%s') STILL FAILED after retry "
                           "rc=%u", sQn, (unsigned)rc);
                return -1;
            }
        } else {
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMSHM: ShmLink::Accept step 1/5: "
                       "cannot even open stale '%s' to close it", sQn);
            return -1;
        }
    }

    /* step 2: create our S2C region */
    if (QmsRegionCreate(s2cN, &pS2C) != 0) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMSHM: ShmLink::Accept step 2/5: "
                   "QmsRegionCreate('%s') FAILED", s2cN);
        DosCloseQueue(hqS);
        return -1;
    }

    /* step 3: open peer's C_ queue */
    ownerPid = 0;
    rc = DosOpenQueue(&ownerPid, &hqC, cQn);
    if (rc != 0) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMSHM: ShmLink::Accept step 3/5: "
                   "DosOpenQueue('%s') FAILED rc=%u", cQn, (unsigned)rc);
        DosFreeMem(pS2C);
        DosCloseQueue(hqS);
        return -1;
    }

    /* step 4: open peer's C2S region */
    if (QmsRegionOpen(c2sN, &pC2S) != 0) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMSHM: ShmLink::Accept step 4/5: "
                   "QmsRegionOpen('%s') FAILED", c2sN);
        DosCloseQueue(hqC);
        DosFreeMem(pS2C);
        DosCloseQueue(hqS);
        return -1;
    }

    /* step 5: echo hello */
    if (QmsSendHello(hqC, QMS_VERSION, cpid, ctid) != 0) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMSHM: ShmLink::Accept step 5/5: "
                   "QmsSendHello(echo) FAILED");
        DosFreeMem(pC2S);
        DosCloseQueue(hqC);
        DosFreeMem(pS2C);
        DosCloseQueue(hqS);
        return -1;
    }

    pNew = new ShmLink();
    if (!pNew) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMSHM: ShmLink::Accept: new ShmLink FAILED (OOM)");
        DosFreeMem(pC2S);
        DosCloseQueue(hqC);
        DosFreeMem(pS2C);
        DosCloseQueue(hqS);
        return -1;
    }
    pNew->hqRecv       = hqS;
    pNew->hqSend       = hqC;
    pNew->pWriteRegion = pS2C;
    pNew->pReadRegion  = pC2S;
    pNew->peerPid      = cpid;
    pNew->mode         = SHM_SERVER_MODE;
    pNew->connected    = 1;
    *ppNew = pNew;

    F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_VERBOSE,
               "PMSHM: ShmLink::Accept OK, cpid=%u ctid=%u "
               "hqRecv=%p hqSend=%p pWrite=%p pRead=%p",
               (unsigned)cpid, (unsigned)ctid, hqS, hqC, pS2C, pC2S);
    return 0;
}

/*! @brief Handshake is a no-op (already done in Open/Accept).
    @return 0.
    @retval 0  Always. */
int ShmLink::Handshake(void)
{
    F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_VERBOSE,
               "PMSHM: ShmLink::Handshake (no-op, done in Open/Accept)");
    return 0;
}

/*! @brief Send exactly cbData bytes. Splits into QMS_SLOT_SIZE chunks.

    Each chunk: copy to slot, post {slot, length} through hqSend,
    advance writeIdx, wait for reader to consume (readIdx advances).

    @param [in] pvData  Source buffer.
    @param [in] cbData  Bytes to send (0 is a no-op).
    @return 0 on success, -1 on error.
    @retval 0   All bytes sent.
    @retval -1  Not connected, null buffer with cbData > 0, or queue
                write failed. */
int ShmLink::Send(const void *pvData, ULONG cbData)
{
    const BYTE *p = (const BYTE *)pvData;
    ULONG sent = 0;
    ULONG slotSize;

    if (cbData == 0) return 0;
    if (!connected || !pWriteRegion) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMSHM: ShmLink::Send: not connected "
                   "(connected=%d, pWriteRegion=%p, cbData=%u)",
                   connected, pWriteRegion, (unsigned)cbData);
        return -1;
    }
    if (!pvData) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMSHM: ShmLink::Send: null buffer, cbData=%u",
                   (unsigned)cbData);
        return -1;
    }

    slotSize = pWriteRegion->slotSize;

    while (sent < cbData) {
        ULONG    slot;
        ULONG    chunk;
        QmsElem  elem;
        APIRET   rc;

        while (pWriteRegion->writeIdx - pWriteRegion->readIdx
               >= pWriteRegion->nSlots) {
            DosSleep(1);
            if (!connected) {
                F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                           "PMSHM: ShmLink::Send: disconnected while "
                           "waiting for free slot (sent=%u/%u)",
                           (unsigned)sent, (unsigned)cbData);
                return -1;
            }
        }

        chunk = cbData - sent;
        if (chunk > slotSize) chunk = slotSize;

        slot = pWriteRegion->writeIdx % pWriteRegion->nSlots;
        pWriteRegion->slots[slot].length = chunk;
        memcpy(pWriteRegion->slots[slot].data, p + sent, chunk);

        elem.slot   = slot;
        elem.length = chunk;
        rc = DosWriteQueue(hqSend, 0, sizeof(elem), &elem, 0);
        if (rc != 0) {
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMSHM: ShmLink::Send: DosWriteQueue FAILED rc=%u "
                       "(slot=%u len=%u)",
                       (unsigned)rc, (unsigned)slot, (unsigned)chunk);
            return -1;
        }

        pWriteRegion->writeIdx++;
        sent += chunk;
    }
    return 0;
}

/*! @brief Receive exactly cbBuffer bytes. Reassembles from chunks.

    Chunks that overflow the caller's request are buffered in recvBuf
    and delivered on subsequent Recv calls.

    Idle timeout QMS_RECV_TIMEOUT_MS. On timeout returns -1 (peer
    considered dead). Must exceed PMCLIENT keepalive interval.

    @param [out] pvData       Destination.
    @param [out] pcbReceived  Bytes received.
    @param [in]  cbBuffer     Bytes requested.
    @return 0 on success, -1 on error.
    @retval 0   Exactly cbBuffer bytes received.
    @retval -1  Not connected, null buffer with cbBuffer > 0,
                queue error, idle timeout, or short read. */
int ShmLink::Recv(void *pvData, ULONG *pcbReceived, ULONG cbBuffer)
{
    BYTE *pOut = (BYTE *)pvData;
    ULONG total = 0;

    if (pcbReceived) *pcbReceived = 0;
    if (cbBuffer == 0) return 0;
    if (!connected || !pReadRegion) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMSHM: ShmLink::Recv: not connected "
                   "(connected=%d, pReadRegion=%p, cbBuffer=%u)",
                   connected, pReadRegion, (unsigned)cbBuffer);
        return -1;
    }
    if (!pvData) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMSHM: ShmLink::Recv: null buffer, cbBuffer=%u",
                   (unsigned)cbBuffer);
        return -1;
    }

    while (total < cbBuffer) {

        if (recvBufLen > recvBufPos) {
            ULONG avail = recvBufLen - recvBufPos;
            ULONG n = cbBuffer - total;
            if (n > avail) n = avail;
            memcpy(pOut + total, recvBuf + recvBufPos, n);
            recvBufPos += n;
            total += n;
            if (recvBufPos >= recvBufLen) {
                recvBufPos = 0;
                recvBufLen = 0;
            }
            continue;
        }

        {
            QmsElem     elem;
            REQUESTDATA req;
            PVOID       pElem = NULL;
            ULONG       cb;
            APIRET      rc;
            ULONG       elapsed = 0;

            for (;;) {
                cb         = sizeof(QmsElem);
                req.ulData = 0;
                req.pid    = 0;
                pElem      = NULL;
                rc = DosReadQueue(hqRecv, &req, &cb, &pElem,
                                  0, DCWW_NOWAIT, NULL, 0);
                if (rc == 0) break;
                if (pElem) DosFreeMem(pElem);
                if (rc != ERROR_QUE_EMPTY) {
                    F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                               "PMSHM: ShmLink::Recv: DosReadQueue FAILED "
                               "rc=%u (total=%u/%u)",
                               (unsigned)rc, (unsigned)total,
                               (unsigned)cbBuffer);
                    if (pcbReceived) *pcbReceived = total;
                    return -1;
                }
                DosSleep(QMS_POLL_MS);
                elapsed += QMS_POLL_MS;
                if (elapsed >= QMS_RECV_TIMEOUT_MS) {
                    F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                               "PMSHM: ShmLink::Recv: IDLE TIMEOUT after "
                               "%u ms (peer dead? total=%u/%u)",
                               (unsigned)elapsed, (unsigned)total,
                               (unsigned)cbBuffer);
                    if (pcbReceived) *pcbReceived = total;
                    return -1;
                }
            }

            if (!pElem || cb != sizeof(QmsElem)) {
                F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                           "PMSHM: ShmLink::Recv: bad element "
                           "(pElem=%p, cb=%u, expected %u)",
                           pElem, (unsigned)cb, (unsigned)sizeof(QmsElem));
                if (pElem) DosFreeMem(pElem);
                if (pcbReceived) *pcbReceived = total;
                return -1;
            }
            memcpy(&elem, pElem, sizeof(elem));
            DosFreeMem(pElem);

            if (elem.slot >= pReadRegion->nSlots) {
                F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                           "PMSHM: ShmLink::Recv: invalid slot=%u "
                           "(nSlots=%u)",
                           (unsigned)elem.slot,
                           (unsigned)pReadRegion->nSlots);
                if (pcbReceived) *pcbReceived = total;
                return -1;
            }
            if (elem.length > pReadRegion->slotSize) {
                F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                           "PMSHM: ShmLink::Recv: invalid length=%u "
                           "(slotSize=%u)",
                           (unsigned)elem.length,
                           (unsigned)pReadRegion->slotSize);
                if (pcbReceived) *pcbReceived = total;
                return -1;
            }

            if (elem.length <= cbBuffer - total) {
                memcpy(pOut + total, pReadRegion->slots[elem.slot].data,
                       elem.length);
                total += elem.length;
            } else {
                ULONG n    = cbBuffer - total;
                ULONG tail = elem.length - n;

                if (recvBufCap < tail) {
                    if (recvBuf) free(recvBuf);
                    recvBuf = (BYTE *)malloc(tail);
                    if (!recvBuf) {
                        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                                   "PMSHM: ShmLink::Recv: malloc(%u) "
                                   "FAILED for reassembly buffer",
                                   (unsigned)tail);
                        recvBufCap = 0;
                        if (pcbReceived) *pcbReceived = total;
                        return -1;
                    }
                    recvBufCap = tail;
                }
                memcpy(pOut + total, pReadRegion->slots[elem.slot].data, n);
                memcpy(recvBuf, pReadRegion->slots[elem.slot].data + n, tail);
                recvBufPos = 0;
                recvBufLen = tail;
                total += n;
            }

            pReadRegion->readIdx++;
        }
    }

    if (pcbReceived) *pcbReceived = total;
    return 0;
}

/*! @brief Close queues, regions, and free the reassembly buffer.
    @return 0 always.
    @retval 0  Always. */
/*! @brief Close queues, regions, and free the reassembly buffer.

    Client-side: before tearing down handles, send a wire-close (8-byte
    header with cmd=0) so the server exits its recv loop and releases its
    C_/S_ handles immediately. Without this the server holds the queue
    names alive for up to QMS_RECV_TIMEOUT_MS (60 s), and a rapid
    reconnect fails on DosCreateQueue(ERROR_QUE_DUPLICATE).

    Server-side: no wire-close is sent (the server already knows it's
    closing — it just broke out of the recv loop).

    @return 0 always.
    @retval 0  Always. */
int ShmLink::Close(void)
{
    F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_VERBOSE,
               "PMSHM: ShmLink::Close enter "
               "(mode=%s, hqRecv=%p hqSend=%p pWrite=%p pRead=%p connected=%d)",
               (mode == SHM_SERVER_MODE) ? "SERVER" : "CLIENT",
               hqRecv, hqSend, pWriteRegion, pReadRegion, connected);

    /* Wire-close (client only). */
    if (connected && mode == SHM_CLIENT_MODE && hqSend && pWriteRegion) {
        if (pWriteRegion->writeIdx - pWriteRegion->readIdx
                < pWriteRegion->nSlots) {
            int     hdr[2] = { 0, 0 };   /* cmd=0, par=0 */
            ULONG   slot   = pWriteRegion->writeIdx % pWriteRegion->nSlots;
            QmsElem elem;
            APIRET  rc;

            pWriteRegion->slots[slot].length = 8;
            memcpy(pWriteRegion->slots[slot].data, hdr, 8);

            elem.slot   = slot;
            elem.length = 8;
            rc = DosWriteQueue(hqSend, 0, sizeof(elem), &elem, 0);
            if (rc == 0) {
                pWriteRegion->writeIdx++;
                F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_VERBOSE,
                           "PMSHM: ShmLink::Close: wire-close (cmd=0) sent");
            } else {
                F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                           "PMSHM: ShmLink::Close: DosWriteQueue(cmd=0) "
                           "FAILED rc=%u", (unsigned)rc);
            }
        } else {
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_VERBOSE,
                       "PMSHM: ShmLink::Close: no free slot for cmd=0, "
                       "skipping wire-close");
        }
        /* Give the server a moment to receive cmd=0 and release its
           handles. Server polls at QMS_POLL_MS (10 ms), so 100 ms is
           ample on any reasonable machine. */
        DosSleep(100);
    }

    if (recvBuf) { free(recvBuf); recvBuf = NULL; }
    recvBufCap = recvBufLen = recvBufPos = 0;

    if (pReadRegion)  { DosFreeMem(pReadRegion);  pReadRegion  = NULL; }
    if (pWriteRegion) { DosFreeMem(pWriteRegion); pWriteRegion = NULL; }
    if (hqSend) { DosCloseQueue(hqSend); hqSend = NULL; }
    if (hqRecv) { DosCloseQueue(hqRecv); hqRecv = NULL; }
    connected = 0;
    peerPid   = 0;

    F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_VERBOSE,
               "PMSHM: ShmLink::Close done");
    return 0;
}

/* ======================================================================== */
/*  Backend exports                                                        */
/* ======================================================================== */

/*! @brief Open a client connection and perform the handshake.
    @param [in]  address  Listener queue name, or NULL for default.
    @param [out] obj      Receives connection object; 0 on error.
    @return 0 on success, non-zero on failure.
    @retval 0                  Connected.
    @retval PMC_ERR_OPEN       new or Open failed.
    @retval HAND_SHAKE_ERROR   Handshake mismatch. */
extern "C" APIRET EXPENTRY Fc_TrnConnect(char *address, ULONG *obj)
{
    ShmLink *pNew;

    F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_VERBOSE,
               "PMSHM: Fc_TrnConnect(address='%s') enter",
               address ? address : "NULL");

    if (!obj) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMSHM: Fc_TrnConnect: NULL obj");
        return PMC_ERR_OPEN;
    }
    *obj = 0;

    pNew = new ShmLink(address, SHM_CLIENT_MODE);
    if (!pNew) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMSHM: Fc_TrnConnect: new ShmLink FAILED (OOM)");
        return PMC_ERR_OPEN;
    }
    if (pNew->Open() != 0) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMSHM: Fc_TrnConnect: pNew->Open() FAILED");
        delete pNew;
        return PMC_ERR_OPEN;
    }
    if (pNew->Handshake() == HAND_SHAKE_ERROR) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMSHM: Fc_TrnConnect: Handshake FAILED");
        delete pNew;
        return HAND_SHAKE_ERROR;
    }
    *obj = (ULONG)pNew;

    F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_VERBOSE,
               "PMSHM: Fc_TrnConnect OK obj=0x%lx",
               (unsigned long)*obj);
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
    ShmLink          *pServer;

    F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_VERBOSE,
               "PMSHM: Fs_TrnRunListener enter (handler=%p, disc=%p)",
               handler, disc_cb);

    pServer = new ShmLink(FREEPM_SHM_QUEUE_NAME, SHM_SERVER_MODE);
    if (!pServer) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMSHM: Fs_TrnRunListener: new ShmLink FAILED (OOM)");
        return PMC_ERR_OPEN;
    }
    if (pServer->Open() != 0) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMSHM: Fs_TrnRunListener: Open('%s') FAILED",
                   FREEPM_SHM_QUEUE_NAME);
        delete pServer;
        return PMC_ERR_OPEN;
    }

    F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_VERBOSE,
               "PMSHM: Fs_TrnRunListener: listener '%s' up, "
               "entering Fs_RunServer",
               FREEPM_SHM_QUEUE_NAME);

    Fs_RunServer(pServer, h, disc);

    delete pServer;
    return 0;
}
