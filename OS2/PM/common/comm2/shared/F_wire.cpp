/*! @file F_wire.cpp
    @brief F_ common: 8-byte wire header pack/unpack.
*/
#include <string.h>

#include "F_wire.hpp"

/*! @brief Pack (cmd, par) into an 8-byte header.
    @param [out] buf  Destination buffer (8 bytes).
    @param [in]  cmd  Command code.
    @param [in]  par  32-bit argument. */
void F_WirePackHeader(void *buf, int cmd, int par)
{
    memcpy(buf,              &cmd, 4);
    memcpy((char *)buf + 4,  &par, 4);
}

/*! @brief Unpack an 8-byte header into (cmd, par).
    @param [in]  buf  Source buffer (8 bytes).
    @param [out] cmd  Receives command code.
    @param [out] par  Receives 32-bit argument. */
void F_WireUnpackHeader(const void *buf, int *cmd, int *par)
{
    memcpy(cmd, buf,                  4);
    memcpy(par, (const char *)buf + 4, 4);
}
