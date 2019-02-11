#ifndef __CPI_SESSION_CLIENT_H__
#define __CPI_SESSION_CLIENT_H__

#include <cpi_session/cpi_session.h>
#include <base/rpc_client.h>

namespace OS2::Cpi { struct Session_client; }

struct OS2::Cpi::Session_client : Genode::Rpc_client<Session>
{
	Session_client(Genode::Capability<Session> cap)
	: Genode::Rpc_client<Session>(cap) { }

        Genode::Dataspace_capability sysio_dataspace()
        {
            return call<Rpc_sysio_dataspace>();
        }

        Genode::Untyped_capability get_cap(int index)
        {
            return call<Rpc_get_cap>(index);
        }

        void send_cap(Genode::Untyped_capability cap, int index)
        {
            call<Rpc_send_cap>(cap, index);
        }

        bool syscall(Syscall syscall)
        {
            return call<Rpc_syscall>(syscall);
        }
};

#endif
