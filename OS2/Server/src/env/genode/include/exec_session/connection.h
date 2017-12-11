#ifndef __INCLUDE__EXEC_SESSION_CONNECTION_H__
#define __INCLUDE__EXEC_SESSION_CONNECTION_H__

#include <exec_session/client.h>
#include <base/connection.h>

namespace OS2::Exec { struct Connection; }

struct OS2::Exec::Connection : Genode::Connection<Session>, Session_client
{
	Connection(Genode::Env &env)
	:
		/* create session */
		Genode::Connection<OS2::Exec::Session>(env, session(env.parent(),
		                                                "ram_quota=6K, cap_quota=4")),
		/* initialize RPC interface */
		Session_client(cap()) { }
};

#endif
