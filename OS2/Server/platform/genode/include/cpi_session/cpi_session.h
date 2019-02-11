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
#include <base/capability.h>
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

        virtual Genode::Untyped_capability get_cap(int index) = 0;

        virtual void send_cap(Genode::Untyped_capability cap, int index) = 0;

        enum Syscall
        {
            SYSCALL_TEST,
            SYSCALL_CFGGETENV,
            SYSCALL_CFGGETOPT,
            SYSCALL_APPNOTIFY1,
            SYSCALL_APPNOTIFY2,
            SYSCALL_APPSEND,
            SYSCALL_APPGET,
            SYSCALL_EXIT,
            SYSCALL_EXECPGM,
            SYSCALL_GETPIB,
            SYSCALL_GETTIB,
            SYSCALL_ERROR,
            SYSCALL_QUERYDBCSENV,
            SYSCALL_QUERYCP,
            SYSCALL_QUERYCURRENTDISK,
            SYSCALL_QUERYCURRENTDIR,
            SYSCALL_SETCURRENTDIR,
            SYSCALL_SETDEFAULTDISK,
            SYSCALL_CREATEEVENTSEM,
            SYSCALL_OPENEVENTSEM,
            SYSCALL_CLOSEEVENTSEM,
            SYSCALL_GETPID,
            SYSCALL_GETNATIVEID,
            SYSCALL_NEWTIB,
            SYSCALL_DESTROYTIB,
            SYSCALL_INVALID = -1
        };

        virtual bool syscall(Syscall syscall) = 0;

        GENODE_RPC(Rpc_syscall, bool, syscall, Syscall);
        GENODE_RPC(Rpc_sysio_dataspace, Genode::Dataspace_capability, sysio_dataspace);
        GENODE_RPC(Rpc_get_cap, Genode::Untyped_capability, get_cap, int);
        GENODE_RPC(Rpc_send_cap, void, send_cap, Genode::Untyped_capability, int);

        GENODE_RPC_INTERFACE(Rpc_syscall, Rpc_sysio_dataspace, Rpc_get_cap, Rpc_send_cap);
};

#endif
