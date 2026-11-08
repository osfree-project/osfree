/*! @file F_shm.hpp
    @brief Shared-memory transport for FreePM: ShmLink class.

    One named region holds a fixed array of slots; each slot has two
    byte-ring buffers (client->server, server->client) and a state word.
    Clients claim a free slot at Open, exchange handshake over the rings,
    then run full duplex. Only per-thread model (one slot per connection);
    a client that dies after claiming a slot leaks it (known limit).

    Pure OS/2 API on both platforms; Win32 differences are absorbed by
    F_platform.h emulation (DosAllocSharedMem, DosGetNamedSharedMem,
    DosFreeMem, DosCreateMutexSem, DosOpenMutexSem).
*/
#ifndef F_SHM_HPP
#define F_SHM_HPP

#if defined(__OS2__) || defined(__OS2V2__)
  #define INCL_DOSERRORS
  #define INCL_DOSMEMMGR
  #define INCL_DOSSEMAPHORES
#endif
#include "F_platform.h"

#include "F_link.hpp"
#include "F_wire.hpp"

/*! @def FREEPM_SHM_NAME
    @brief OS/2 shared-memory name (Win32 translated to Local\\...). */
#define FREEPM_SHM_NAME       "\\SHAREMEM\\FREEPM_SHM"
/*! @def FREEPM_SHM_MAX_SLOTS
    @brief Maximum concurrent connections. */
#define FREEPM_SHM_MAX_SLOTS  32
/*! @def FREEPM_SHM_RING_SIZE
    @brief Bytes per direction per slot (ring capacity). */
#define FREEPM_SHM_RING_SIZE  0x2000
/*! @def FREEPM_SHM_MAGIC
    @brief Header magic ("SHM1"). */
#define FREEPM_SHM_MAGIC      0x53484D31UL

/*! @def SHM_SERVER_MODE
    @brief ShmLink acts as a listener. */
#define SHM_SERVER_MODE  1
/*! @def SHM_CLIENT_MODE
    @brief ShmLink acts as a client. */
#define SHM_CLIENT_MODE  2

/*! @brief Per-direction ring indices (shared). */
typedef struct _ShmRing {
    ULONG writeIdx;   /*!< Bytes written (mod ringSize). */
    ULONG readIdx;    /*!< Bytes read (mod ringSize). */
} ShmRing;

/*! @brief Per-connection slot (shared). */
typedef struct _ShmSlot {
    ULONG   state;    /*!< 0=free, 1=claimed, 2=active. */
    ULONG   pad;      /*!< Alignment. */
    ShmRing c2s;      /*!< client -> server indices. */
    ShmRing s2c;      /*!< server -> client indices. */
} ShmSlot;

/*! @brief Shared header (first object in the region). */
typedef struct _ShmHeader {
    ULONG   magic;    /*!< FREEPM_SHM_MAGIC. */
    ULONG   version;  /*!< 1. */
    ULONG   nSlots;   /*!< FREEPM_SHM_MAX_SLOTS. */
    ULONG   ringSize; /*!< FREEPM_SHM_RING_SIZE. */
    ShmSlot slots[FREEPM_SHM_MAX_SLOTS];
} ShmHeader;

/*! @brief Shared-memory transport: F_Link implementation. */
class ShmLink : public F_Link {
public:
    ShmHeader *pHeader;     /*!< Mapped header. */
    BYTE      *pC2S;        /*!< Base of client->server rings. */
    BYTE      *pS2C;        /*!< Base of server->client rings. */
    int        slot;        /*!< Slot index, -1 if not bound. */
    int        mode;        /*!< SHM_CLIENT_MODE or SHM_SERVER_MODE. */
    int        connected;   /*!< Non-zero if ready for data. */
    int        isCreator;   /*!< Non-zero if this side created the region. */
    HMTX       hMtxGlobal;  /*!< Protects slots[].state. */
    HMTX       hMtxData;    /*!< Protects slots[slot] ring operations. */

    ShmLink();
    ShmLink(const char *pszName, int iMode);
    virtual ~ShmLink();

    virtual int Open(void);
    virtual int Accept(F_Link **ppNew);
    virtual int Handshake(void);
    virtual int Send(const void *pvData, int cbData);
    virtual int Recv(void *pvData, int *pcbReceived, int cbBuffer);
    virtual int Close(void);
};

#endif /* F_SHM_HPP */
