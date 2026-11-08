/*! @file F_link.hpp
    @brief Abstract transport interface (L4).
*/
#ifndef F_LINK_HPP
#define F_LINK_HPP

class F_Link {
public:
    virtual ~F_Link() {}

    /* Client: connect to server. Server: create listener. */
    virtual int Open(void) = 0;

    /* Server: block until peer; allocate new F_Link for accepted conn. */
    virtual int Accept(F_Link **ppNew) = 0;

    /* FreePM handshake. */
    virtual int Handshake(void) = 0;

    /* Exactly cbData bytes. Loops on partial send, EINTR-retry. */
    virtual int Send(const void *pvData, int cbData) = 0;

    /* Exactly cbBuffer bytes. Loops on partial recv, EINTR-retry.
       On success sets *pcbReceived = cbBuffer, returns 0.
       On peer-close/error sets *pcbReceived = bytes-read-so-far, returns -1. */
    virtual int Recv(void *pvData, int *pcbReceived, int cbBuffer) = 0;

    virtual int Close(void) = 0;
};

#endif
