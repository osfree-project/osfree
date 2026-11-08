/*! @file F_shm.cpp
    @brief Shared-memory transport for FreePM: ShmLink + 2 backend exports.

    Pure OS/2 API (DosAllocSharedMem / DosGetNamedSharedMem / DosFreeMem /
    DosCreateMutexSem / DosOpenMutexSem / DosRequestMutexSem). Win32
    differences are absorbed by F_platform.h emulation. Only names differ
    by platform (strings, not API calls).

    Connection protocol: client attaches, claims a free slot under the
    global mutex, creates the per-slot data mutex, sets state=1, then
    handshakes over the rings. Server Accept polls for state=1, flips it
    to 2, opens the per-slot mutex, returns a ShmLink.
*/
#include <stdio.h>
#include <string.h>
#include <process.h>

#include "F_shm.hpp"
#include "F_debug.h"

/* ======================================================================== */
/*  Name construction (strings only)                                       */
/* ======================================================================== */

/*! @brief Build shared-memory object name in platform-native form.
    @param [out] buf  Destination.
    @param [in]  cb   Capacity.
    @param [in]  nm   Base name (e.g. "\\SHAREMEM\\FREEPM_SHM"). */
static void ShmObjName(char *buf, int cb, const char *nm)
{
    const char *base = nm;
    if (base && strncmp(base, "\\SHAREMEM\\", 10) == 0) base += 10;
    else if (base && strncmp(base, "\\sharemem\\", 10) == 0) base += 10;
    else if (base && base[0] == '\\') base++;
#if defined(__OS2__) || defined(__OS2V2__)
    _snprintf(buf, cb, "\\SHAREMEM\\%s", base);
#else
    _snprintf(buf, cb, "Local\\%s", base);
#endif
    buf[cb - 1] = 0;
}

/*! @brief Build mutex object name in platform-native form.
    @param [out] buf  Destination.
    @param [in]  cb   Capacity.
    @param [in]  slot Slot index, or -1 for the global mutex. */
static void ShmMtxName(char *buf, int cb, int slot)
{
#if defined(__OS2__) || defined(__OS2V2__)
    if (slot < 0) _snprintf(buf, cb, "\\SEM32\\FREEPM_SHM_G");
    else          _snprintf(buf, cb, "\\SEM32\\FREEPM_SHM_D_%d", slot);
#else
    if (slot < 0) _snprintf(buf, cb, "Local\\FREEPM_SHM_G");
    else          _snprintf(buf, cb, "Local\\FREEPM_SHM_D_%d", slot);
#endif
    buf[cb - 1] = 0;
}

/* ======================================================================== */
/*  Region lifetime (OS/2 API only)                                        */
/* ======================================================================== */

/*! @brief Total region size in bytes.
    @return Bytes. */
static ULONG ShmTotalSize(void)
{
    return (ULONG)sizeof(ShmHeader)
         + (ULONG)FREEPM_SHM_MAX_SLOTS * FREEPM_SHM_RING_SIZE * 2;
}

/*! @brief Create a named shared-memory region (server side).
    @param [in]  size     Bytes.
    @param [out] pExists  Non-zero if a region with this name already exists.
    @return Mapped base pointer, or NULL on failure.
    @retval NULL  Region already exists or allocation failed. */
static void *ShmAlloc(ULONG size, int *pExists)
{
    char   objName[300];
    PVOID  p  = NULL;
    APIRET rc;

    *pExists = 0;
    ShmObjName(objName, sizeof(objName), FREEPM_SHM_NAME);

    rc = DosAllocSharedMem(&p, objName, size,
                           PAG_COMMIT | PAG_READ | PAG_WRITE);
    if (rc == ERROR_ALREADY_EXISTS) {
        if (p) DosFreeMem(p);
        *pExists = 1;
        return NULL;
    }
    if (rc != 0) return NULL;
    return p;
}

/*! @brief Attach to an existing shared-memory region.
    @return Mapped base pointer, or NULL on failure.
    @retval NULL  Region not found or attach failed. */
static void *ShmAttach(void)
{
    char   objName[300];
    PVOID  p  = NULL;

    ShmObjName(objName, sizeof(objName), FREEPM_SHM_NAME);
    if (DosGetNamedSharedMem(&p, objName, PAG_READ | PAG_WRITE) != 0)
        return NULL;
    return p;
}

/*! @brief Release a shared-memory region or view.
    @param [in] p  Base pointer (NULL-safe). */
static void ShmFree(void *p)
{
    if (p) DosFreeMem(p);
}

/* ======================================================================== */
/*  Ring buffer                                                            */
/* ======================================================================== */

/*! @brief Write cbData bytes into a byte-ring under hmtx.
    @param [in] pRing     Ring base.
    @param [in] pRingHdr  Ring indices.
    @param [in] ringSize  Capacity.
    @param [in] hmtx      Direction mutex.
    @param [in] pState    Pointer to slot.state.
    @param [in] pData     Source.
    @param [in] cbData    Bytes to write.
    @return 0 on success, -1 if the slot is freed.
    @retval -1  Slot was freed (state==0).
    @retval 0   All bytes written. */
static int ShmRingWrite(BYTE *pRing, ShmRing *pRingHdr, ULONG ringSize,
                        HMTX hmtx, ULONG *pState,
                        const BYTE *pData, int cbData)
{
    int remaining = cbData;

    while (remaining > 0) {
        ULONG s = *pState;
        ULONG used, space, w, toWrite, firstPart;

        if (s == 0) return -1;
        if (DosRequestMutexSem(hmtx, 100) != 0) {
            DosSleep(0);
            continue;
        }
        used  = pRingHdr->writeIdx - pRingHdr->readIdx;
        space = ringSize - used;
        if (space > 0) {
            w         = pRingHdr->writeIdx % ringSize;
            toWrite   = space < (ULONG)remaining ? space : (ULONG)remaining;
            firstPart = ringSize - w;
            if (firstPart > toWrite) firstPart = toWrite;
            memcpy(pRing + w, pData + (cbData - remaining), firstPart);
            if (toWrite > firstPart) {
                memcpy(pRing, pData + (cbData - remaining) + firstPart,
                       toWrite - firstPart);
            }
            pRingHdr->writeIdx += toWrite;
            remaining -= (int)toWrite;
        }
        DosReleaseMutexSem(hmtx);
        if (space == 0) DosSleep(0);
    }
    return 0;
}

/*! @brief Read up to cbBuffer bytes from a byte-ring under hmtx.
    @param [in]  pRing     Ring base.
    @param [in]  pRingHdr  Ring indices.
    @param [in]  ringSize  Capacity.
    @param [in]  hmtx      Direction mutex.
    @param [in]  pState    Pointer to slot.state.
    @param [out] pData     Destination.
    @param [out] pcb       Bytes read.
    @param [in]  cbBuffer  Bytes required.
    @return 0 on success, -1 on peer-close.
    @retval 0   All cbBuffer bytes read.
    @retval -1  Slot was freed (state==0) before all bytes arrived. */
static int ShmRingRead(BYTE *pRing, ShmRing *pRingHdr, ULONG ringSize,
                       HMTX hmtx, ULONG *pState,
                       BYTE *pData, int *pcb, int cbBuffer)
{
    int got = 0;

    if (pcb) *pcb = 0;
    if (cbBuffer <= 0) return 0;

    while (got < cbBuffer) {
        ULONG s = *pState;
        ULONG avail, r, toRead, firstPart;

        if (s == 0) {
            if (pcb) *pcb = got;
            return -1;
        }
        if (DosRequestMutexSem(hmtx, 100) != 0) {
            DosSleep(0);
            continue;
        }
        avail = pRingHdr->writeIdx - pRingHdr->readIdx;
        if (avail > 0) {
            r         = pRingHdr->readIdx % ringSize;
            toRead    = avail < (ULONG)(cbBuffer - got)
                        ? avail : (ULONG)(cbBuffer - got);
            firstPart = ringSize - r;
            if (firstPart > toRead) firstPart = toRead;
            memcpy(pData + got, pRing + r, firstPart);
            if (toRead > firstPart) {
                memcpy(pData + got + firstPart, pRing, toRead - firstPart);
            }
            pRingHdr->readIdx += toRead;
            got += (int)toRead;
        }
        DosReleaseMutexSem(hmtx);
        if (avail == 0) DosSleep(0);
    }
    if (pcb) *pcb = got;
    return 0;
}

/* ======================================================================== */
/*  ShmLink                                                                */
/* ======================================================================== */

/*! @brief Construct a client ShmLink.
    Region not attached.
    @return Nothing. */
ShmLink::ShmLink()
    : pHeader(NULL), pC2S(NULL), pS2C(NULL),
      slot(-1), mode(SHM_CLIENT_MODE), connected(0), isCreator(0),
      hMtxGlobal(NULLHANDLE), hMtxData(NULLHANDLE)
{
}

/*! @brief Construct a ShmLink with explicit mode.
    @param [in] pszName  Unused (region name is fixed).
    @param [in] iMode    SHM_CLIENT_MODE or SHM_SERVER_MODE.
    @return Nothing. */
ShmLink::ShmLink(const char *pszName, int iMode)
    : pHeader(NULL), pC2S(NULL), pS2C(NULL),
      slot(-1), mode(iMode), connected(0), isCreator(0),
      hMtxGlobal(NULLHANDLE), hMtxData(NULLHANDLE)
{
    (void)pszName;
}

/*! @brief Destroy the ShmLink. Closes slot and mutexes.
    @return Nothing. */
ShmLink::~ShmLink()
{
    Close();
}

/*! @brief Open the link.

    Server: create the shared region and global mutex.
    Client: attach, open the global mutex, claim a free slot, create the
    per-slot data mutex, publish state=1.

    @return 0 on success, -1 on failure.
    @retval 0   Listener ready (server) or slot claimed (client).
    @retval -1  Region/mutex/slot failure. */
int ShmLink::Open(void)
{
    char mtxName[64];

    if (mode == SHM_SERVER_MODE) {
        int exists = 0;
        pHeader = (ShmHeader *)ShmAlloc(ShmTotalSize(), &exists);
        if (!pHeader || exists) {
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMSHM: ShmLink::Open ERROR: region exists or alloc failed");
            return -1;
        }
        pHeader->magic    = FREEPM_SHM_MAGIC;
        pHeader->version  = 1;
        pHeader->nSlots   = FREEPM_SHM_MAX_SLOTS;
        pHeader->ringSize = FREEPM_SHM_RING_SIZE;
        memset(pHeader->slots, 0, sizeof(pHeader->slots));

        pC2S = (BYTE *)pHeader + sizeof(ShmHeader);
        pS2C = pC2S + pHeader->nSlots * pHeader->ringSize;

        ShmMtxName(mtxName, sizeof(mtxName), -1);
        if (DosCreateMutexSem(mtxName, &hMtxGlobal, DC_SEM_SHARED, FALSE) != 0) {
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMSHM: ShmLink::Open ERROR: global mutex create");
            ShmFree(pHeader);
            pHeader = NULL;
            return -1;
        }
        isCreator = 1;
        return 0;
    }

    /* Client */
    pHeader = (ShmHeader *)ShmAttach();
    if (!pHeader || pHeader->magic != FREEPM_SHM_MAGIC) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMSHM: ShmLink::Open ERROR: attach failed or bad magic");
        if (pHeader) { ShmFree(pHeader); pHeader = NULL; }
        return -1;
    }
    pC2S = (BYTE *)pHeader + sizeof(ShmHeader);
    pS2C = pC2S + pHeader->nSlots * pHeader->ringSize;

    ShmMtxName(mtxName, sizeof(mtxName), -1);
    if (DosOpenMutexSem(mtxName, &hMtxGlobal) != 0) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMSHM: ShmLink::Open ERROR: global mutex open");
        ShmFree(pHeader); pHeader = NULL;
        return -1;
    }

    if (DosRequestMutexSem(hMtxGlobal, SEM_INDEFINITE_WAIT) != 0) {
        ShmFree(pHeader); pHeader = NULL;
        return -1;
    }
    {
        int i;
        for (i = 0; i < (int)pHeader->nSlots; i++) {
            if (pHeader->slots[i].state == 0) { slot = i; break; }
        }
        if (slot < 0) {
            DosReleaseMutexSem(hMtxGlobal);
            F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                       "PMSHM: ShmLink::Open ERROR: no free slot");
            ShmFree(pHeader); pHeader = NULL;
            return -1;
        }
        ShmMtxName(mtxName, sizeof(mtxName), slot);
        if (DosCreateMutexSem(mtxName, &hMtxData,
                              DC_SEM_SHARED, FALSE) != 0) {
            DosReleaseMutexSem(hMtxGlobal);
            ShmFree(pHeader); pHeader = NULL;
            return -1;
        }
        pHeader->slots[slot].c2s.writeIdx = 0;
        pHeader->slots[slot].c2s.readIdx  = 0;
        pHeader->slots[slot].s2c.writeIdx = 0;
        pHeader->slots[slot].s2c.readIdx  = 0;
        pHeader->slots[slot].state        = 1;
    }
    DosReleaseMutexSem(hMtxGlobal);
    connected = 1;
    return 0;
}

/*! @brief Accept one incoming client (server mode).
    @param [out] ppNew  Receives a new ShmLink.
    @return 0 on success, -1 on failure.
    @retval 0   Connection accepted.
    @retval -1  Not a listener, bad args, or mutex open failed. */
int ShmLink::Accept(F_Link **ppNew)
{
    int       slotIdx = -1;
    char      mtxName[64];
    ShmLink  *pNew;

    if (mode != SHM_SERVER_MODE || !pHeader) return -1;
    if (ppNew == NULL) return -1;

    for (;;) {
        int i;
        if (DosRequestMutexSem(hMtxGlobal, 100) != 0) {
            DosSleep(1);
            continue;
        }
        for (i = 0; i < (int)pHeader->nSlots; i++) {
            if (pHeader->slots[i].state == 1) {
                slotIdx = i;
                pHeader->slots[i].state = 2;
                break;
            }
        }
        DosReleaseMutexSem(hMtxGlobal);
        if (slotIdx >= 0) break;
        DosSleep(1);
    }

    pNew = new ShmLink();
    if (!pNew) {
        if (DosRequestMutexSem(hMtxGlobal, SEM_INDEFINITE_WAIT) == 0) {
            pHeader->slots[slotIdx].state = 0;
            DosReleaseMutexSem(hMtxGlobal);
        }
        return -1;
    }

    pNew->pHeader   = pHeader;
    pNew->pC2S      = pC2S;
    pNew->pS2C      = pS2C;
    pNew->slot      = slotIdx;
    pNew->mode      = SHM_SERVER_MODE;
    pNew->isCreator = 0;

    ShmMtxName(mtxName, sizeof(mtxName), -1);
    if (DosOpenMutexSem(mtxName, &pNew->hMtxGlobal) != 0) goto fail;
    ShmMtxName(mtxName, sizeof(mtxName), slotIdx);
    if (DosOpenMutexSem(mtxName, &pNew->hMtxData) != 0) goto fail;

    pNew->connected = 1;
    *ppNew = pNew;
    return 0;

fail:
    if (pNew->hMtxGlobal) DosCloseMutexSem(pNew->hMtxGlobal);
    if (pNew->hMtxData)   DosCloseMutexSem(pNew->hMtxData);
    if (DosRequestMutexSem(hMtxGlobal, SEM_INDEFINITE_WAIT) == 0) {
        pHeader->slots[slotIdx].state = 0;
        DosReleaseMutexSem(hMtxGlobal);
    }
    delete pNew;
    return -1;
}

/*! @brief Perform the FreePM handshake.
    @return 0 on success, HAND_SHAKE_ERROR on protocol mismatch/transport error.
    @retval 0                 Handshake completed.
    @retval HAND_SHAKE_ERROR  Magic mismatch or transport failure. */
int ShmLink::Handshake(void)
{
    char buf[32];
    int  cb;
    int  inpLen = (int)strlen(HAND_SHAKE_INP) + 1;
    int  outLen = (int)strlen(HAND_SHAKE_OUT) + 1;

    if (mode == SHM_SERVER_MODE) {
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

/*! @brief Send exactly cbData bytes to the peer ring.
    @param [in] pvData  Source.
    @param [in] cbData  Bytes to send (<= 0 is a no-op).
    @return 0 on success, -1 on error.
    @retval 0   All bytes written.
    @retval -1  Not connected or slot freed. */
int ShmLink::Send(const void *pvData, int cbData)
{
    BYTE    *pRing;
    ShmRing *pRingHdr;

    if (cbData <= 0) return 0;
    if (!connected || !pHeader || slot < 0) return -1;

    if (mode == SHM_CLIENT_MODE) {
        pRing    = pC2S + (ULONG)slot * pHeader->ringSize;
        pRingHdr = &pHeader->slots[slot].c2s;
    } else {
        pRing    = pS2C + (ULONG)slot * pHeader->ringSize;
        pRingHdr = &pHeader->slots[slot].s2c;
    }
    return ShmRingWrite(pRing, pRingHdr, pHeader->ringSize, hMtxData,
                        &pHeader->slots[slot].state,
                        (const BYTE *)pvData, cbData);
}

/*! @brief Receive exactly cbBuffer bytes from the peer ring.
    @param [out] pvData       Destination.
    @param [out] pcbReceived  Bytes received.
    @param [in]  cbBuffer     Bytes required.
    @return 0 on success, -1 on peer-close or error.
    @retval 0   Exactly cbBuffer bytes received.
    @retval -1  Not connected, peer gone, or no data. */
int ShmLink::Recv(void *pvData, int *pcbReceived, int cbBuffer)
{
    BYTE    *pRing;
    ShmRing *pRingHdr;

    if (pcbReceived) *pcbReceived = 0;
    if (cbBuffer <= 0) return 0;
    if (!connected || !pHeader || slot < 0) return -1;

    if (mode == SHM_CLIENT_MODE) {
        pRing    = pS2C + (ULONG)slot * pHeader->ringSize;
        pRingHdr = &pHeader->slots[slot].s2c;
    } else {
        pRing    = pC2S + (ULONG)slot * pHeader->ringSize;
        pRingHdr = &pHeader->slots[slot].c2s;
    }
    return ShmRingRead(pRing, pRingHdr, pHeader->ringSize, hMtxData,
                       &pHeader->slots[slot].state,
                       (BYTE *)pvData, pcbReceived, cbBuffer);
}

/*! @brief Close the slot and mutexes. Idempotent.
    @return 0 always.
    @retval 0  Always. */
int ShmLink::Close(void)
{
    if (hMtxData) {
        DosCloseMutexSem(hMtxData);
        hMtxData = NULLHANDLE;
    }

    if (pHeader && slot >= 0 && hMtxGlobal) {
        if (DosRequestMutexSem(hMtxGlobal, 1000) == 0) {
            pHeader->slots[slot].state = 0;
            DosReleaseMutexSem(hMtxGlobal);
        }
    }

    if (hMtxGlobal) {
        DosCloseMutexSem(hMtxGlobal);
        hMtxGlobal = NULLHANDLE;
    }

    if (isCreator || mode == SHM_CLIENT_MODE) {
        ShmFree(pHeader);
    }

    pHeader = NULL; pC2S = NULL; pS2C = NULL;
    slot = -1; connected = 0; isCreator = 0;
    return 0;
}

/* ======================================================================== */
/*  Backend exports                                                        */
/* ======================================================================== */

/*! @brief Open a client shared-memory connection and perform the handshake.
    @param [in]  address  Unused (region name is fixed).
    @param [out] obj      Receives the connection object; set to 0 on error.
    @return 0 on success, non-zero on failure.
    @retval 0                  Connected.
    @retval PMC_ERR_OPEN       new or Open failed.
    @retval HAND_SHAKE_ERROR   Handshake mismatch. */
extern "C" APIRET EXPENTRY Fc_TrnConnect(char *address, ULONG *obj)
{
    ShmLink *pNew;

    (void)address;
    if (!obj) return PMC_ERR_OPEN;
    *obj = 0;

    pNew = new ShmLink(NULL, SHM_CLIENT_MODE);
    if (pNew == NULL) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMSHM: Fc_TrnConnect ERROR: new ShmLink failed");
        return PMC_ERR_OPEN;
    }
    if (pNew->Open() != 0) {
        delete pNew;
        return PMC_ERR_OPEN;
    }
    if (pNew->Handshake() == HAND_SHAKE_ERROR) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMSHM: Fc_TrnConnect ERROR: Handshake failed");
        delete pNew;
        return HAND_SHAKE_ERROR;
    }
    *obj = (ULONG)pNew;
    return 0;
}

/*! @brief Server-side listener loop: create region, accept, spawn thread.
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

    pServer = new ShmLink(NULL, SHM_SERVER_MODE);
    if (!pServer) return PMC_ERR_OPEN;

    if (pServer->Open() != 0) {
        F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_ERROR,
                   "PMSHM: Fs_TrnRunListener: Open failed");
        delete pServer;
        return PMC_ERR_OPEN;
    }

    F_DbgPrint(F_DBG_SEC_WIRE, F_DBG_LVL_VERBOSE,
               "PMSHM: Fs_TrnRunListener: region %s, %d slots",
               FREEPM_SHM_NAME, FREEPM_SHM_MAX_SLOTS);

    Fs_RunServer(pServer, h, disc);

    delete pServer;
    return 0;
}
