#ifndef __CPI_SESSION_CPI_SESSION_H__
#define __CPI_SESSION_CPI_SESSION_H__

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree OS/2 personality internal */
#include <os3/loader.h>
#include <os3/ixfmgr.h>
#include <os3/thread.h>

/* Genode includes */
#include <session/session.h>
#include <base/rpc.h>
#include <base/rpc_args.h>
#include <base/ram_allocator.h>
#include <dataspace/capability.h>
#include <cpi_session/sysio.h>

namespace OS2::Cpi {
    struct Session;
}

struct OS2::Cpi::Session : Genode::Session
{
        static const char *service_name() { return "cpi"; }

        enum { CAP_QUOTA = 2 };

        virtual Genode::Dataspace_capability sysio_dataspace() = 0;

        enum Syscall
        {
            SYSCALL_TEST,
            SYSCALL_CFGGETENV,
            SYSCALL_CFGGETOPT,
            SYSCALL_APPNOTIFY1,
            SYSCALL_APPNOTIFY2,
            SYSCALL_EXIT,
            SYSCALL_EXECPGM,
            SYSCALL_INVALID = -1
        };

        virtual bool syscall(Syscall syscall) = 0;

        GENODE_RPC(Rpc_syscall, bool, syscall, Syscall);
        GENODE_RPC(Rpc_sysio_dataspace, Genode::Dataspace_capability, sysio_dataspace);

        GENODE_RPC_INTERFACE(Rpc_syscall, Rpc_sysio_dataspace);
};

#endif
