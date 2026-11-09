/*! @file F_link.hpp
    @brief Abstract transport interface (L4).
*/
#ifndef F_LINK_HPP
#define F_LINK_HPP

/*! @brief Abstract transport interface.

    Send/Recv are stream operations of arbitrary length in bytes.
    Length parameters are ULONG (32-bit unsigned, up to 4 GB - 1).
    Per-call kernel syscalls are internally bounded; the interface
    does not expose per-call chunking. */
class F_Link {
public:
    virtual ~F_Link() {}

    /*! @brief Open the link: connect (client) or create listener (server).
        @return 0 on success, non-zero on failure. */
    virtual int Open(void) = 0;

    /*! @brief Accept one incoming connection (server mode only).
        @param [out] ppNew  Receives a new F_Link for the accepted connection.
        @return 0 on success, non-zero on failure. */
    virtual int Accept(F_Link **ppNew) = 0;

    /*! @brief Perform the FreePM handshake.
        @return 0 on success, HAND_SHAKE_ERROR on protocol mismatch. */
    virtual int Handshake(void) = 0;

    /*! @brief Send exactly cbData bytes. Loops on partial send.
        @param [in] pvData  Source buffer.
        @param [in] cbData  Bytes to send (0 is a no-op).
        @return 0 on success, -1 on error. */
    virtual int Send(const void *pvData, ULONG cbData) = 0;

    /*! @brief Receive exactly cbBuffer bytes. Loops on partial recv.
        On peer-close sets *pcbReceived = bytes-read-so-far, returns -1.
        @param [out] pvData       Destination.
        @param [out] pcbReceived  Bytes received (ULONG).
        @param [in]  cbBuffer     Bytes required.
        @return 0 on success, -1 on error. */
    virtual int Recv(void *pvData, ULONG *pcbReceived, ULONG cbBuffer) = 0;

    /*! @brief Close the transport. Idempotent.
        @return 0 always. */
    virtual int Close(void) = 0;
};

#endif /* F_LINK_HPP */
