/*! @file Fs_trn.cpp
    @brief Fs_ server wire-ops + accept loop + per-conn thread.
*/
#include <stdio.h>
#include <process.h>

#include "F_wire.hpp"
#include "F_debug.h"

/*! @brief Blocking read of an 8-byte header (cmd + par).
    @param [in]  obj  Connection.
    @param [out] cmd  Receives command code.
    @param [out] par  Receives 32-bit argument.
    @return 0 on success, -1 on error.
    @retval 0   Header read.
    @retval -1  obj is NULL or Recv() failed. */
extern "C" APIRET EXPENTRY Fs_TrnGetCommand(void *obj, int *cmd, int *par)
{
    char buf[8];
    int  len = 0;

    if (!obj) return (APIRET)-1;
    if (((F_Link *)obj)->Recv(buf, &len, 8) != 0) return (APIRET)-1;
    F_WireUnpackHeader(buf, cmd, par);
    return 0;
}

/*! @brief Blocking read of exactly size bytes.
    @param [in]  obj   Connection.
    @param [out] buf   Destination.
    @param [out] len   Receives bytes read.
    @param [in]  size  Bytes to read.
    @return 0 on success, -1 on error.
    @retval 0   Exactly size bytes received.
    @retval -1  obj is NULL, Recv() failed, or short read. */
extern "C" APIRET EXPENTRY Fs_TrnGetData(void *obj, void *buf, int *len, int size)
{
    if (!obj) return (APIRET)-1;
    if (((F_Link *)obj)->Recv(buf, len, size) != 0) return (APIRET)-1;
    if (len && *len != size) return (APIRET)-1;
    return 0;
}

/*! @brief Async write of raw data.
    @param [in] obj  Connection.
    @param [in] buf  Payload.
    @param [in] len  Payload size in bytes.
    @return 0 on success, PMC_ERR_BROKEN on transport error.
    @retval 0              Success.
    @retval PMC_ERR_BROKEN obj is NULL or Send() failed. */
extern "C" APIRET EXPENTRY Fs_TrnPostData(void *obj, void *buf, int len)
{
    if (!obj) return PMC_ERR_BROKEN;
    return (((F_Link *)obj)->Send(buf, len) == 0) ? 0 : PMC_ERR_BROKEN;
}

/* ======================================================================== */
/*  Server accept loop                                                     */
/* ======================================================================== */

/*! @brief Per-connection context passed to Fs_ClientThreadProc. */
struct Fs_ClientArg {
    F_Link           *pClient;   /*!< Accepted connection. */
    PMC_HANDLER       h;         /*!< Command handler (may be NULL). */
    PMC_DISCONNECT_CB disc;      /*!< Disconnect callback (may be NULL). */
    int               threadNum; /*!< Thread index passed to handler. */
};

/*! @brief Per-connection dispatcher thread.

    Reads 8-byte headers in a loop, dispatches each non-zero command
    through the handler, then calls the disconnect callback and cleans up.

    @param [in] param  Pointer to Fs_ClientArg. */
static void Fs_ClientThreadProc(void *param)
{
    Fs_ClientArg     *pArg    = (Fs_ClientArg *)param;
    F_Link           *pClient = pArg->pClient;
    PMC_HANDLER       h       = pArg->h;
    PMC_DISCONNECT_CB disc    = pArg->disc;
    int               cmd, par;

    while (Fs_TrnGetCommand(pClient, &cmd, &par) == 0) {
        if (cmd == 0) break;
        if (h) h(pClient, cmd, par, pArg->threadNum);
    }

    if (disc) disc(pClient);

    delete pClient;
    delete pArg;
}

/*! @brief Server accept loop.

    Blocks forever. For each accepted connection: handshake (bounded by
    the transport), then spawn a dispatcher thread. On _beginthread
    failure drops the connection and continues.

    @param [in] listener  Listening F_Link (e.g. TcpLink in server mode).
    @param [in] h         Command handler (may be NULL).
    @param [in] disc      Disconnect callback (may be NULL). */
void Fs_RunServer(F_Link *listener, PMC_HANDLER h, PMC_DISCONNECT_CB disc)
{
    int threadCounter = 0;

    for (;;) {
        F_Link       *pClient = NULL;
        Fs_ClientArg *pArg;
        int           thrc;

        if (listener->Accept(&pClient) != 0) {
            DosSleep(10);
            continue;
        }
        if (!pClient) { DosSleep(10); continue; }

        if (pClient->Handshake() != 0) {
            pClient->Close();
            delete pClient;
            continue;
        }

        pArg = new Fs_ClientArg;
        if (!pArg) {
            pClient->Close();
            delete pClient;
            continue;
        }
        pArg->pClient   = pClient;
        pArg->h         = h;
        pArg->disc      = disc;
        pArg->threadNum = threadCounter++;

#if defined(__OS2__) || defined(__OS2V2__)
        thrc = _beginthread(Fs_ClientThreadProc, NULL, FREEPM_THREAD_STACK, pArg);
#else
        thrc = _beginthread(Fs_ClientThreadProc, FREEPM_THREAD_STACK, pArg);
#endif
        if (thrc == -1) {
            pClient->Close();
            delete pClient;
            delete pArg;
        }
    }
}
