/*! @file F_pipe.hpp
    @brief Named-pipe transport for FreePM: PipeLink class.

    Works on OS/2 (native Dos*NPipe API) and Win32 (via F_platform.h
    emulation). INCL_DOSERRORS and INCL_DOSNMPIPES are defined before
    including F_platform.h so that ERROR_OPEN_FAILED, ERROR_BROKEN_PIPE
    and the NP_* constants are visible on OS/2.
*/
#ifndef F_PIPE_HPP
#define F_PIPE_HPP

#define INCL_DOSERRORS
#define INCL_DOSNMPIPES
#include "F_platform.h"

#include "F_link.hpp"
#include "F_wire.hpp"

/*! @def FREEPM_PIPE_NAME
    @brief Default pipe name (platform-specific namespace). */
#if defined(__OS2__) || defined(__OS2V2__)
  #define FREEPM_PIPE_NAME          "\\PIPE\\FREEPM"
#else
  #define FREEPM_PIPE_NAME          "\\\\.\\pipe\\FREEPM"
#endif

/*! @def FREEPM_PIPE_MAX_INST
    @brief Max listener instances (low byte of pipe mode). */
#define FREEPM_PIPE_MAX_INST      32
/*! @def FREEPM_PIPE_OUTB_SIZE
    @brief Outbound buffer size, bytes. */
#define FREEPM_PIPE_OUTB_SIZE     0x1000
/*! @def FREEPM_PIPE_INPB_SIZE
    @brief Inbound buffer size, bytes. */
#define FREEPM_PIPE_INPB_SIZE     0x1000
/*! @def FREEPM_PIPE_TIMEOUT
    @brief Default pipe I/O timeout, milliseconds. */
#define FREEPM_PIPE_TIMEOUT       20000L

/*! @def PIPE_SERVER_MODE
    @brief PipeLink acts as a listener. */
#define PIPE_SERVER_MODE          1
/*! @def PIPE_CLIENT_MODE
    @brief PipeLink acts as a client. */
#define PIPE_CLIENT_MODE          2

/*! @brief Named-pipe transport: F_Link implementation. */
class PipeLink : public F_Link {
public:
    HPIPE   Hpipe;          /*!< Pipe handle; NULLHANDLE if closed. */
    char    name[256];      /*!< Pipe name. */
    int     mode;           /*!< PIPE_SERVER_MODE or PIPE_CLIENT_MODE. */
    int     connected;      /*!< Non-zero if handshake completed / accepted. */

    PipeLink();
    PipeLink(const char *pszName, int iMode);
    virtual ~PipeLink();

    virtual int Open(void);
    virtual int Accept(F_Link **ppNew);
    virtual int Handshake(void);
    virtual int Send(const void *pvData, int cbData);
    virtual int Recv(void *pvData, int *pcbReceived, int cbBuffer);
    virtual int Close(void);
};

#endif /* F_PIPE_HPP */
