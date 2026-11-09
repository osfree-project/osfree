/*! @file F_wire.hpp
    @brief Wire protocol: constants, callbacks, exports (shared + backend).
*/
#ifndef F_WIRE_HPP
#define F_WIRE_HPP

#define INCL_DOSERRORS
#include "F_platform.h"
#include "F_link.hpp"

/*! @brief Handshake mismatch / transport error code. */
#define HAND_SHAKE_ERROR       101

/*! @brief Stack size for per-connection dispatcher thread. */
#define FREEPM_THREAD_STACK    65536

/*! @brief Wire header size: cmd (4) + par (4). */
#define FREEPM_HEADER_SIZE     8

#if defined(__OS2__) || defined(__OS2V2__)
  /*! @brief Calling convention for handler / disconnect callbacks (OS/2). */
  #define PMC_CB            APIENTRY
  /*! @brief "Cannot open transport" return code (OS/2). */
  #define PMC_ERR_OPEN      ERROR_OPEN_FAILED
  /*! @brief "Transport broken" return code (OS/2). */
  #define PMC_ERR_BROKEN    ERROR_BROKEN_PIPE
#else
  /*! @brief Calling convention for handler / disconnect callbacks (Win32). */
  #define PMC_CB            __stdcall
  /*! @brief "Cannot open transport" return code (Win32). */
  #define PMC_ERR_OPEN      WSAECONNREFUSED
  /*! @brief "Transport broken" return code (Win32). */
  #define PMC_ERR_BROKEN    WSAECONNABORTED
#endif

/*! @brief Command handler, invoked once per received 8-byte header. */
typedef void (PMC_CB *PMC_HANDLER)(void *obj, int ncmd, int data, int threadNum);

/*! @brief Disconnect callback, invoked after the recv loop exits. */
typedef void (PMC_CB *PMC_DISCONNECT_CB)(void *obj);

#ifdef __cplusplus
extern "C" {
#endif

/* ---- 9 shared exports (implemented in shared/) ---- */
APIRET EXPENTRY Fc_TrnDisconnect(void *obj);
APIRET EXPENTRY Fc_TrnPostCommand(void *obj, int cmd, int par);
APIRET EXPENTRY Fc_TrnPostData(void *obj, void *buf, ULONG len);
APIRET EXPENTRY Fc_TrnSendCommand(void *obj, int cmd, int par);
APIRET EXPENTRY Fc_TrnSendCommandData(void *obj, int cmd, int par,
                                      void *buf, ULONG len);
APIRET EXPENTRY Fc_TrnGetData(void *obj, void *buf, ULONG *len, ULONG size);
APIRET EXPENTRY Fs_TrnPostData(void *obj, void *buf, ULONG len);
APIRET EXPENTRY Fs_TrnGetCommand(void *obj, int *cmd, int *par);
APIRET EXPENTRY Fs_TrnGetData(void *obj, void *buf, ULONG *len, ULONG size);

/* ---- 2 backend exports (implemented in pmtcp/ or pmpipe/ or pmshm/) ---- */
APIRET EXPENTRY Fc_TrnConnect(char *address, ULONG *obj);
APIRET EXPENTRY Fs_TrnRunListener(void *handler, void *disc_cb);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

/*! @brief Pack (cmd, par) into an 8-byte header.
    @param [out] buf  Destination buffer (8 bytes).
    @param [in]  cmd  Command code.
    @param [in]  par  32-bit argument. */
void F_WirePackHeader(void *buf, int cmd, int par);

/*! @brief Unpack an 8-byte header into (cmd, par).
    @param [in]  buf  Source buffer (8 bytes).
    @param [out] cmd  Receives command code.
    @param [out] par  Receives 32-bit argument. */
void F_WireUnpackHeader(const void *buf, int *cmd, int *par);

/*! @brief Server accept loop. Never returns on success.
    @param [in] listener  Listening F_Link.
    @param [in] h         Command handler (may be NULL).
    @param [in] disc      Disconnect callback (may be NULL). */
void Fs_RunServer(F_Link *listener, PMC_HANDLER h, PMC_DISCONNECT_CB disc);

#endif

#endif /* F_WIRE_HPP */
