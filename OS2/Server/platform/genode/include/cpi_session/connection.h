#ifndef __CPI_SESSION_CONNECTION_H__
#define __CPI_SESSION_CONNECTION_H__

#include <cpi_session/client.h>
#include <base/connection.h>

namespace OS2::Cpi { struct Connection; }

struct OS2::Cpi::Connection : Genode::Connection<Session>, Session_client
{
	Connection(Genode::Env &env, char const *label)
	:
		/* create session */
		Genode::Connection<OS2::Cpi::Session>(env, session(env.parent(),
		                                      "ram_quota=6K, cap_quota=4, label=\"%s\"", label)),
		/* initialize RPC interface */
		Session_client(cap()) { }
};

#endif
