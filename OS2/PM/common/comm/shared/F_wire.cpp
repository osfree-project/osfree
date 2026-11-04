/*! @file F_wire.cpp
    @brief Wire-protocol layer shared by all transports.

    All _F_* exports go through F_Link virtual methods. The choice of
    transport (TCP, named pipe, ...) is made in _InitServerConnection /
    _startServerThreads, which live in the backend-specific files.
*/
#include <stdio.h>
#include <string.h>
#include <process.h>

#include "F_wire.hpp"
#include "F_debug.hpp"
#include "F_cmd.h"

F_Link         *g_pClientLink = NULL;
PMC_SHUTDOWN_CB g_shutdownCb  = NULL;

/* ======================================================================== */
/*  Client-side exports                                                     */
/* ======================================================================== */

extern "C" APIRET EXPENTRY _F_SendCmdToServer(void *recvobj, int ncmd, int data)
{
    ULONG hdr[2];

    F_Dbg("WIRE: _F_SendCmdToServer IN -> recvobj=%p, ncmd=0x%x, data=0x%x\n",
          recvobj, ncmd, data);
    if (!recvobj) return PMC_ERR_BROKEN;

    hdr[0] = (ULONG)ncmd;
    hdr[1] = (ULONG)data;
    if (((F_Link *)recvobj)->Send(hdr, 8) != 0) return PMC_ERR_BROKEN;
    return 0;
}

extern "C" APIRET EXPENTRY _F_SendDataToServer(void *recvobj, void *data, int len)
{
    F_Dbg("WIRE: _F_SendDataToServer IN -> recvobj=%p, data=%p, len=%d\n",
          recvobj, data, len);
    if (!recvobj) return PMC_ERR_BROKEN;
    if (((F_Link *)recvobj)->Send(data, len) != 0) return PMC_ERR_BROKEN;
    return 0;
}

extern "C" APIRET EXPENTRY _F_RecvDataFromServer(void *recvobj, void *data,
                                                 int *len, int maxlen)
{
    F_Dbg("WIRE: _F_RecvDataFromServer IN -> recvobj=%p, maxlen=%d\n",
          recvobj, maxlen);
    if (!recvobj) return (APIRET)-1;
    if (((F_Link *)recvobj)->Recv(data, len, maxlen) != 0) return (APIRET)-1;
    F_Dbg("WIRE: _F_RecvDataFromServer OUT -> len=%d\n", *len);
    return 0;
}

extern "C" APIRET EXPENTRY _F_SendGenCmdToServer(void *recvobj, int cmd, int par)
{
    int rc, retcode, l;

    F_Dbg("WIRE: _F_SendGenCmdToServer IN -> recvobj=%p, cmd=0x%x, par=0x%x\n",
          recvobj, cmd, par);
    if (!recvobj) return (APIRET)-1;

    rc = _F_SendCmdToServer(recvobj, cmd, par);
    if (rc) return (APIRET)-1;

    rc = _F_RecvDataFromServer(recvobj, &retcode, &l, sizeof(retcode));
    if (rc) return (APIRET)-1;

    F_Dbg("WIRE: _F_SendGenCmdToServer OUT -> retcode=0x%x\n", retcode);
    return (APIRET)retcode;
}

extern "C" APIRET EXPENTRY _F_SendGenCmdDataToServer(void *recvobj, int cmd,
                                                     int par, void *data, int datalen)
{
    int rc, retcode, l;

    F_Dbg("WIRE: _F_SendGenCmdDataToServer IN -> recvobj=%p, cmd=0x%x, par=0x%x\n",
          recvobj, cmd, par);
    if (!recvobj) return (APIRET)-1;

    rc = _F_SendCmdToServer(recvobj, cmd, par);
    if (rc) return (APIRET)-1;

    rc = _F_SendDataToServer(recvobj, data, datalen * sizeof(int));
    if (rc) return (APIRET)-1;

    rc = _F_RecvDataFromServer(recvobj, &retcode, &l, sizeof(retcode));
    if (rc) return (APIRET)-1;

    F_Dbg("WIRE: _F_SendGenCmdDataToServer OUT -> retcode=0x%x\n", retcode);
    return (APIRET)retcode;
}

/* ======================================================================== */
/*  Server-side exports                                                     */
/* ======================================================================== */

extern "C" APIRET EXPENTRY _F_RecvCmdFromClient(void *recvobj, int *ncmd, int *data)
{
    char buf[8];
    int  len = 0;

    if (!recvobj) return (APIRET)-1;
    if (((F_Link *)recvobj)->Recv(buf, &len, 8) != 0) return (APIRET)-1;

    memcpy(ncmd, buf,     4);
    memcpy(data, buf + 4, 4);
    return 0;
}

extern "C" APIRET EXPENTRY _F_RecvDataFromClient(void *recvobj, void *sqmsg,
                                                 int *l, int size)
{
    if (!recvobj) return (APIRET)-1;
    return (((F_Link *)recvobj)->Recv(sqmsg, l, size) == 0) ? 0 : (APIRET)-1;
}

extern "C" APIRET EXPENTRY _F_SendDataToClient(void *recvobj, void *data, int len)
{
    if (!recvobj) return PMC_ERR_BROKEN;
    return (((F_Link *)recvobj)->Send(data, len) == 0) ? 0 : PMC_ERR_BROKEN;
}

/* ======================================================================== */
/*  Fatal sink (registered by _InitServerConnection on the client side)     */
/* ======================================================================== */

static void WireFatalSink(F_DBG_KIND kind, const char *msg)
{
    ULONG hdr[2];

    if (!g_pClientLink) return;

    hdr[0] = (kind == F_DBG_PRINT) ? (ULONG)F_CMD_DB_PRINT
                                   : (ULONG)F_CMD_FATAL_COMMON;
    hdr[1] = 0;

    g_pClientLink->Send(hdr, 8);
    g_pClientLink->Send(msg, (int)strlen(msg));
}

void F_WireRegisterFatalSink(void)
{
    F_DbgSetWireSink(WireFatalSink);
}

/* ======================================================================== */
/*  Per-connection dispatcher thread                                        */
/* ======================================================================== */

struct F_ClientArg {
    F_Link          *pClient;
    PMC_HANDLER      h;
    PMC_DISCONNECT_CB disc;
    int              threadNum;
};

static void ClientThreadProc(void *param)
{
    F_ClientArg      *pArg    = (F_ClientArg *)param;
    F_Link           *pClient = pArg->pClient;
    PMC_HANDLER       h       = pArg->h;
    PMC_DISCONNECT_CB disc    = pArg->disc;
    int               ncmd, data;

    while (_F_RecvCmdFromClient(pClient, &ncmd, &data) == 0)
    {
        if (ncmd == 0) break;
        if (h) h(pClient, ncmd, data, pArg->threadNum);
    }

    if (disc) disc(pClient);

    pClient->Close();
    delete pClient;
    delete pArg;
}

void F_RunServer(F_Link *pListener, PMC_HANDLER h, PMC_DISCONNECT_CB disc)
{
    int threadCounter = 0;

    for (;;)
    {
        F_Link      *pClient = NULL;
        F_ClientArg *pArg;
        int          thrc;

        if (pListener->Accept(&pClient) != 0)
        {
            DosSleep(10);
            continue;
        }
        if (!pClient) { DosSleep(10); continue; }

        if (pClient->Handshake() != 0)
        {
            F_Dbg("WIRE: handshake failed, dropping client %p\n", pClient);
            pClient->Close();
            delete pClient;
            continue;
        }

        pArg = new F_ClientArg;
        if (!pArg) { pClient->Close(); delete pClient; continue; }

        pArg->pClient   = pClient;
        pArg->h         = h;
        pArg->disc      = disc;
        pArg->threadNum = threadCounter++;

#if defined(__OS2__) || defined(__OS2V2__)
        thrc = _beginthread(ClientThreadProc, NULL, FREEPM_THREAD_STACK, pArg);
#else
        thrc = _beginthread(ClientThreadProc, FREEPM_THREAD_STACK, pArg);
#endif
        if (thrc == -1)
        {
            F_Dbg("WIRE: _beginthread failed, dropping client\n");
            pClient->Close();
            delete pClient;
            delete pArg;
        }
    }
}
