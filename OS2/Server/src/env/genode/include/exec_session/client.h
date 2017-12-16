#ifndef __INCLUDE__EXEC_SESSION_CLIENT_H__
#define __INCLUDE__EXEC_SESSION_CLIENT_H__

#include <exec_session/exec_session.h>
#include <base/rpc_client.h>

namespace OS2::Exec { struct Session_client; }

struct OS2::Exec::Session_client : Genode::Rpc_client<Session>
{
	Session_client(Genode::Capability<Session> cap)
	: Genode::Rpc_client<Session>(cap) { }

	void test(Buf &str)
	{
		call<Rpc_test>(str);
	}

	long open(Genode::Rpc_in_buffer<CCHMAXPATHCOMP> &fname,
	          Genode::Dataspace_capability img_ds,
	          unsigned long flags,
	          LoadError &szLoadError,
	          unsigned long &cbLoadError,
	          unsigned long &hmod)
	{
		return call<Rpc_open>(fname, img_ds, flags,
		                      szLoadError, cbLoadError, hmod);
	}

	long load(unsigned long hmod,
	          LoadError &szLoadError,
	          unsigned long &cbLoadError,
	          os2exec_module_t &s)
	{
		return call<Rpc_load>(hmod, szLoadError, cbLoadError, s);
	}

	long share(unsigned long hmod, Genode::Capability<void> *client_id)
	{
		return call<Rpc_share>(hmod, client_id);
	}

	long getimp(unsigned long hmod,
	            unsigned long &index,
	            unsigned long &imp_hmod)
	{
		return call<Rpc_getimp>(hmod, index, imp_hmod);
	}

	long getsect(unsigned long hmod,
	             unsigned long &index,
	             l4exec_section_t *s)
	{
		return call<Rpc_getsect>(hmod, index, s);
	}

	long query_procaddr(unsigned long hmod,
	                    unsigned long ordinal,
	                    Genode::Rpc_in_buffer<CCHMAXPATHCOMP> &modname,
	                    void *&addr)
	{
		return call<Rpc_query_procaddr>(hmod, ordinal, modname, addr);
	}

	long query_modhandle(Genode::Rpc_in_buffer<CCHMAXPATHCOMP> &pszModname,
	                     unsigned long &hmod)
	{
		return call<Rpc_query_modhandle>(pszModname, hmod);
	}

	long query_modname(unsigned long hmod,
	                   unsigned long cbBuf, Buf &pszBuf)
	{
		return call<Rpc_query_modname>(hmod, cbBuf, pszBuf);
	}
};

#endif
