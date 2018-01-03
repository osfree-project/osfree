#ifndef __FS_SESSION_CONNECTION_H__
#define __FS_SESSION_CONNECTION_H__

#include <fs_session/client.h>
#include <base/connection.h>

namespace OS2::Fs { struct Connection; }

struct OS2::Fs::Connection : Genode::Connection<Session>, Session_client
{
	Connection(Genode::Env &env)
	:
	/* create session */
	Genode::Connection<OS2::Fs::Session>(env, session(env.parent(),
                                             "ram_quota=6K, cap_quota=4")),
	/* Initialize RPC interface */
	Session_client(cap()) {  }
};

#endif /* __FS_SESSION_CONNECTION_H__ */
