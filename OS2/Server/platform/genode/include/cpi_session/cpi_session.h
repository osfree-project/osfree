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
        static const char *service_name() { return "CPI"; }

        enum { CAP_QUOTA = 2 };

        virtual Genode::Dataspace_capability sysio_dataspace() = 0;

        virtual Genode::Untyped_capability get_cap(int index) = 0;

        virtual void send_cap(Genode::Untyped_capability cap, int index) = 0;

        enum Syscall
        {
            SYSCALL_MAIN_TEST,
            SYSCALL_MAIN_CFGGETENV,
            SYSCALL_MAIN_CFGGETOPT,
            SYSCALL_MAIN_APPNOTIFY1,
            SYSCALL_MAIN_APPNOTIFY2,
            SYSCALL_MAIN_APPSEND,
            SYSCALL_MAIN_APPGET,
            SYSCALL_MAIN_EXIT,
            SYSCALL_MAIN_EXECPGM,
            SYSCALL_MAIN_GETPIB,
            SYSCALL_MAIN_GETTIB,
            SYSCALL_MAIN_ERROR,
            SYSCALL_MAIN_QUERYDBCSENV,
            SYSCALL_MAIN_QUERYCP,
            SYSCALL_MAIN_QUERYCURRENTDISK,
            SYSCALL_MAIN_QUERYCURRENTDIR,
            SYSCALL_MAIN_SETCURRENTDIR,
            SYSCALL_MAIN_SETDEFAULTDISK,
            SYSCALL_MAIN_CREATEEVENTSEM,
            SYSCALL_MAIN_OPENEVENTSEM,
            SYSCALL_MAIN_CLOSEEVENTSEM,
            SYSCALL_MAIN_GETPID,
            SYSCALL_MAIN_GETNATIVEID,
            SYSCALL_MAIN_NEWTIB,
            SYSCALL_MAIN_DESTROYTIB,

            SYSCALL_EXEC_OPEN,
            SYSCALL_EXEC_LOAD,
            SYSCALL_EXEC_FREE,
            SYSCALL_EXEC_SHARE,
            SYSCALL_EXEC_GETIMP,
            SYSCALL_EXEC_GETSECT,
            SYSCALL_EXEC_QUERYPROCADDR,
            SYSCALL_EXEC_QUERYMODULEHANDLE,
            SYSCALL_EXEC_QUERYMODULENAME,
            SYSCALL_EXEC_ALLOCSHAREDMEM,
            SYSCALL_EXEC_MAPDATASPACE,
            SYSCALL_EXEC_UNMAPDATASPACE,
            SYSCALL_EXEC_GETDATASPACE,
            SYSCALL_EXEC_GETSHAREDMEM,
            SYSCALL_EXEC_GETNAMEDSHAREDMEM,
            SYSCALL_EXEC_INCREMENTSHAREDMEMREFCNT,
            SYSCALL_EXEC_RELEASESHAREDMEM,

            SYSCALL_FS_GETDRIVEMAP,
            SYSCALL_FS_READ,
            SYSCALL_FS_WRITE,
            SYSCALL_FS_RESETBUFFER,
            SYSCALL_FS_SETFILEPTRL,
            SYSCALL_FS_CLOSE,
            SYSCALL_FS_QUERYHTYPE,
            SYSCALL_FS_OPENL,
            SYSCALL_FS_DUPHANDLE,
            SYSCALL_FS_DELETE,
            SYSCALL_FS_FORCEDELETE,
            SYSCALL_FS_DELETEDIR,
            SYSCALL_FS_CREATEDIR,
            SYSCALL_FS_FINDFIRST,
            SYSCALL_FS_FINDNEXT,
            SYSCALL_FS_FINDCLOSE,
            SYSCALL_FS_QUERYFHSTATE,
            SYSCALL_FS_SETFHSTATE,
            SYSCALL_FS_QUERYFILEINFO,
            SYSCALL_FS_QUERYPATHINFO,
            SYSCALL_FS_SETFILESIZEL,
            SYSCALL_FS_SETFILEINFO,
            SYSCALL_FS_SETPATHINFO,

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
