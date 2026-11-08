/*! @file Fc_trn.cpp
    @brief Fc_ client wire-ops.
*/
#include <stdio.h>

#include "F_wire.hpp"
#include "F_debug.h"

/*! @brief Close a client connection. NULL is a no-op.
    @param [in] obj  Connection handle. May be NULL.
    @return 0.
    @retval 0  Always. */
extern "C" APIRET EXPENTRY Fc_TrnDisconnect(void *obj)
{
    if (obj == NULL) return 0;
    delete (F_Link *)obj;
    return 0;
}

/*! @brief Async write of an 8-byte header (cmd + par).
    @param [in] obj  Connection.
    @param [in] cmd  Command code.
    @param [in] par  32-bit argument.
    @return 0 on success, PMC_ERR_BROKEN on transport error.
    @retval 0              Success.
    @retval PMC_ERR_BROKEN obj is NULL or Send() failed. */
extern "C" APIRET EXPENTRY Fc_TrnPostCommand(void *obj, int cmd, int par)
{
    char buf[8];
    if (!obj) return PMC_ERR_BROKEN;
    F_WirePackHeader(buf, cmd, par);
    return (((F_Link *)obj)->Send(buf, 8) == 0) ? 0 : PMC_ERR_BROKEN;
}

/*! @brief Async write of raw data.
    @param [in] obj  Connection.
    @param [in] buf  Payload.
    @param [in] len  Payload size in bytes.
    @return 0 on success, PMC_ERR_BROKEN on transport error.
    @retval 0              Success.
    @retval PMC_ERR_BROKEN obj is NULL or Send() failed. */
extern "C" APIRET EXPENTRY Fc_TrnPostData(void *obj, void *buf, int len)
{
    if (!obj) return PMC_ERR_BROKEN;
    return (((F_Link *)obj)->Send(buf, len) == 0) ? 0 : PMC_ERR_BROKEN;
}

/*! @brief Blocking read of exactly size bytes.
    @param [in]  obj   Connection.
    @param [out] buf   Destination.
    @param [out] len   Receives bytes read.
    @param [in]  size  Bytes to read.
    @return 0 on success, -1 on error.
    @retval 0   Exactly size bytes received.
    @retval -1  obj is NULL or Recv() failed. */
extern "C" APIRET EXPENTRY Fc_TrnGetData(void *obj, void *buf, int *len, int size)
{
    if (!obj) return (APIRET)-1;
    if (((F_Link *)obj)->Recv(buf, len, size) != 0) return (APIRET)-1;
    return 0;
}

/*! @brief PostCommand + GetData(4) — send command, wait reply.
    On any transport error the connection is closed.
    @param [in] obj  Connection.
    @param [in] cmd  Command code.
    @param [in] par  32-bit argument.
    @return Server reply code, or -1 on transport error.
    @retval -1       obj is NULL, Post failed, or Recv failed.
    @retval retcode  Server reply code. */
extern "C" APIRET EXPENTRY Fc_TrnSendCommand(void *obj, int cmd, int par)
{
    int retcode, l;

    if (!obj) return (APIRET)-1;
    if (Fc_TrnPostCommand(obj, cmd, par) != 0) {
        ((F_Link *)obj)->Close();
        return (APIRET)-1;
    }
    if (Fc_TrnGetData(obj, &retcode, &l, sizeof(retcode)) != 0) {
        ((F_Link *)obj)->Close();
        return (APIRET)-1;
    }
    return (APIRET)retcode;
}

/*! @brief PostCommand + PostData + GetData(4) — send command+payload, wait reply.
    On any transport error the connection is closed.
    @param [in] obj  Connection.
    @param [in] cmd  Command code.
    @param [in] par  32-bit argument.
    @param [in] buf  Payload.
    @param [in] len  Payload size in bytes.
    @return Server reply code, or -1 on transport error.
    @retval -1       obj is NULL, PostCommand/PostData/Recv failed.
    @retval retcode  Server reply code. */
extern "C" APIRET EXPENTRY Fc_TrnSendCommandData(void *obj, int cmd, int par,
                                                 void *buf, int len)
{
    int retcode, l;

    if (!obj) return (APIRET)-1;
    if (Fc_TrnPostCommand(obj, cmd, par) != 0) {
        ((F_Link *)obj)->Close();
        return (APIRET)-1;
    }
    if (Fc_TrnPostData(obj, buf, len) != 0) {
        ((F_Link *)obj)->Close();
        return (APIRET)-1;
    }
    if (Fc_TrnGetData(obj, &retcode, &l, sizeof(retcode)) != 0) {
        ((F_Link *)obj)->Close();
        return (APIRET)-1;
    }
    return (APIRET)retcode;
}
