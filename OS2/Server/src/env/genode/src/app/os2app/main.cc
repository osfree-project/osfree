/* Genode includes */
#include <base/component.h>
#include <base/log.h>
#include <exec_session/connection.h>


void Component::construct(Genode::Env &env)
{
	OS2::Exec::Connection exec(env);
	OS2::Exec::Session::Buf buf;

	exec.test(buf);
	Genode::log("Return from RPC: ", Genode::Cstring(buf.str));
}
