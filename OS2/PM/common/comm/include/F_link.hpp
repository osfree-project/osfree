/*! @file F_link.hpp
    @brief Abstract transport interface.

    One F_Link instance per connection: either the client-side
    connection or an accepted server-side client. Concrete subclasses
    (TcpLink, PipeLink) implement the transport primitives.
*/
#ifndef F_LINK_HPP
#define F_LINK_HPP

#include "F_platform.h"

/* Handshake magic strings (same on both transports). */
#define HAND_SHAKE_INP     "pIpEtEsT"
#define HAND_SHAKE_OUT     "PiPeTeSt"
#define HAND_SHAKE_ERROR   101

/* Per-connection thread stack size. */
#define FREEPM_THREAD_STACK  65536

class F_Link {
public:
    virtual ~F_Link() {}

    /*! @brief Open the link.
        Client mode: connect to server.
        Server mode: create the listener.
        @return 0 on success, non-zero on failure. */
    virtual int Open(void) = 0;

    /*! @brief Accept one incoming connection (server mode only).

        Blocks until a peer connects. On success, allocates a fresh
        F_Link representing the accepted connection, and returns it
        through *ppNew. The listener stays ready for the next Accept.

        @param [out] ppNew  Receives the new F_Link (caller owns it).
        @return 0 on success, non-zero on failure. */
    virtual int Accept(F_Link **ppNew) = 0;

    /*! @brief Perform the FreePM handshake over the link. */
    virtual int Handshake(void) = 0;

    /*! @brief Send exactly cb bytes. */
    virtual int Send(const void *pv, int cb) = 0;

    /*! @brief Receive exactly cbMax bytes (blocks until full).
        @param [out] pcb  Receives the number of bytes read (== cbMax). */
    virtual int Recv(void *pv, int *pcb, int cbMax) = 0;

    /*! @brief Close the underlying transport handle. */
    virtual int Close(void) = 0;
};

#endif /* F_LINK_HPP */
