#ifndef __CPI_SESSION_CLIENT_H__
#define __CPI_SESSION_CLIENT_H__

#include <cpi_session/cpi_session.h>
#include <base/rpc_client.h>

namespace OS2::Cpi { struct Session_client; }

struct OS2::Cpi::Session_client : Genode::Rpc_client<Session>
{
	Session_client(Genode::Capability<Session> cap)
	: Genode::Rpc_client<Session>(cap) { }

        void test(void)
        {
            call<Rpc_test>();
        }
};

#endif
