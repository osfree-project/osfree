/*! @file F_queue.hpp
    @brief OS/2-queue transport for FreePM: QueueLink class.

    Works on OS/2 (native DosCreateQueue / DosOpenQueue / DosWriteQueue /
    DosReadQueue) and on Win32 (via F_platform.h emulation built on named
    shared memory + named semaphores + named mutex). Only FIFO ordering is
    emulated; LIFO and priority are accepted but ignored on Win32.
*/
#ifndef F_QUEUE_HPP
#define F_QUEUE_HPP

#define INCL_DOSERRORS
#define INCL_DOSQUEUES
#include "F_platform.h"

#include "F_link.hpp"
#include "F_wire.hpp"

/*! @def FREEPM_QUEUE_NAME
    @brief Default listener queue name. */
#define FREEPM_QUEUE_NAME  "\\QUEUES\\FREEPM"

/*! @def QUEUE_SERVER_MODE
    @brief QueueLink acts as a listener. */
#define QUEUE_SERVER_MODE  1
/*! @def QUEUE_CLIENT_MODE
    @brief QueueLink acts as a client. */
#define QUEUE_CLIENT_MODE  2

/*! @brief Queue transport: F_Link implementation. */
class QueueLink : public F_Link {
public:
    HQUEUE hqRecv;      /*!< Queue to read from. */
    HQUEUE hqSend;      /*!< Queue to write to (peer's queue). */
    char   name[256];   /*!< Own queue name. */
    int    mode;        /*!< QUEUE_SERVER_MODE or QUEUE_CLIENT_MODE. */
    int    connected;   /*!< Non-zero if handshake completed. */

    QueueLink();
    QueueLink(const char *pszName, int iMode);
    virtual ~QueueLink();

    virtual int Open(void);
    virtual int Accept(F_Link **ppNew);
    virtual int Handshake(void);
    virtual int Send(const void *pvData, int cbData);
    virtual int Recv(void *pvData, int *pcbReceived, int cbBuffer);
    virtual int Close(void);
};

#endif /* F_QUEUE_HPP */
