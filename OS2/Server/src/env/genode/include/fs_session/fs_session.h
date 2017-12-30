#ifndef __FS_SESSION_FS_SESSION_H__
#define __FS_SESSION_FS_SESSION_H__

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

namespace OS2::Fs { struct Session; }

/* Genode includes */
#include <session/session.h>
#include <base/rpc.h>
#include <base/rpc_args.h>

struct OS2::Fs::Session : Genode::Session
{
    static const char *service_name() { return "os2fs"; }

    enum { CAP_QUOTA = 2; }

    virtual long get_drivemap(ULONG *map) = 0;

    virtual APIRET dos_Read(HFILE hFile,
                            char **pBuf,
                            ULONG *count) = 0;
}

#endif /* __FS_SESSION_FS_SESSION_H__ */
