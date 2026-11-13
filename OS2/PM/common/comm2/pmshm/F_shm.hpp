/*! @file F_shm.hpp
    @brief Shared-memory transport for FreePM: ShmLink class.

    Data path is shared memory; notification path is OS/2 queues.
    One region per direction with QMS_SLOTS slots of QMS_SLOT_SIZE bytes.
    Send/Recv split/assemble arbitrary length into QMS_SLOT_SIZE chunks;
    from F_Link's perspective they are stream operations of any length.
*/
#ifndef F_SHM_HPP
#define F_SHM_HPP

#define INCL_BASE
#define INCL_PM
#include "F_platform.h"

#include "F_link.hpp"
#include "F_wire.hpp"

/*! @def FREEPM_SHM_QUEUE_NAME
    @brief Server listener queue (Accept entry point). */
#define FREEPM_SHM_QUEUE_NAME  "\\QUEUES\\FREEPM"

/*! @def SHM_SERVER_MODE
    @brief ShmLink acts as listener. */
#define SHM_SERVER_MODE  1
/*! @def SHM_CLIENT_MODE
    @brief ShmLink acts as client. */
#define SHM_CLIENT_MODE  2

/*! @def QMS_MAGIC
    @brief Data region magic ("QMS1"). */
#define QMS_MAGIC        0x514D5331UL
/*! @def QMS_SLOTS
    @brief Slots per direction. 1 = one chunk in flight per direction. */
#define QMS_SLOTS        1
/*! @def QMS_SLOT_SIZE
    @brief Internal chunk size, bytes. Not a message-size limit. */
#define QMS_SLOT_SIZE    (1024 * 1024)

/*! @brief One data slot. */
typedef struct _QmsSlot {
    ULONG length;
    BYTE  data[QMS_SLOT_SIZE];
} QmsSlot;

/*! @brief Data region header (first object in shared memory). */
typedef struct _QmsHeader {
    ULONG    magic;
    ULONG    nSlots;
    ULONG    slotSize;
    ULONG    writeIdx;
    ULONG    readIdx;
    ULONG    pad[3];
    QmsSlot  slots[QMS_SLOTS];
} QmsHeader;

/*! @brief Shared-memory transport: F_Link implementation. */
class ShmLink : public F_Link {
public:
    HQUEUE     hqRecv;
    HQUEUE     hqSend;
    QmsHeader *pWriteRegion;
    QmsHeader *pReadRegion;
    int        mode;
    int        connected;
    ULONG      peerPid;
    char       name[256];

    /* Stream reassembly buffer for Recv. */
    BYTE  *recvBuf;
    ULONG  recvBufCap;
    ULONG  recvBufLen;
    ULONG  recvBufPos;

    ShmLink();
    ShmLink(const char *pszName, int iMode);
    virtual ~ShmLink();

    virtual int Open(void);
    virtual int Accept(F_Link **ppNew);
    virtual int Handshake(void);
    virtual int Send(const void *pvData, ULONG cbData);
    virtual int Recv(void *pvData, ULONG *pcbReceived, ULONG cbBuffer);
    virtual int Close(void);
};

#endif /* F_SHM_HPP */
