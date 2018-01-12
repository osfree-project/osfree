#ifndef __EXEC_SESSION_CLIENT_H__
#define __EXEC_SESSION_CLIENT_H__

#include <exec_session/exec_session.h>
#include <base/rpc_client.h>

namespace OS2::Exec { struct Session_client; }

struct OS2::Exec::Session_client : Genode::Rpc_client<Session>
{
	Session_client(Genode::Capability<Session> cap)
	: Genode::Rpc_client<Session>(cap) { }

	void test(OS2::Exec::Session::Buf *str)
	{
		call<Rpc_test>(str);
	}

	long open(Pathname &fName,
	          unsigned long flags,
	          Genode::Ram_dataspace_capability err_ds,
	          unsigned long *pcbError,
	          unsigned long *hmod)
	{
		return call<Rpc_open>(fName, flags,
		                      err_ds, pcbError, hmod);
	}

	long load(unsigned long hmod,
	          Genode::Ram_dataspace_capability err_ds,
	          unsigned long *pcbLoadError,
	          Genode::Ram_dataspace_capability mod_ds)
	{
		return call<Rpc_load>(hmod, err_ds, pcbLoadError, mod_ds);
	}

	long share(unsigned long hmod)
	{
		return call<Rpc_share>(hmod);
	}

	long getimp(unsigned long hmod,
	            unsigned long *index,
	            unsigned long *imp_hmod)
	{
		return call<Rpc_getimp>(hmod, index, imp_hmod);
	}

	long getsect(unsigned long hmod,
	             unsigned long *index,
	             Genode::Ram_dataspace_capability sect_ds)
	{
		return call<Rpc_getsect>(hmod, index, sect_ds);
	}

	long query_procaddr(unsigned long hmod,
	                    unsigned long ordinal,
	                    Pathname &mName,
	                    ULONGLONG *addr)
	{
		return call<Rpc_query_procaddr>(hmod, ordinal, mName, addr);
	}

	long query_modhandle(Pathname &mName,
	                     unsigned long *hmod)
	{
		return call<Rpc_query_modhandle>(mName, hmod);
	}

	long query_modname(unsigned long hmod,
	                   Genode::Ram_dataspace_capability ds)
	{
		return call<Rpc_query_modname>(hmod, ds);
	}

	long alloc_sharemem(ULONG size,
	                    Pathname &mName,
	                    ULONG rights,
	                    ULONGLONG *addr,
	                    ULONGLONG *area)
	{
		return call<Rpc_alloc_sharemem>(size, mName, rights,
		                                addr, area);
	}

	long map_dataspace(ULONGLONG addr,
	                   ULONG rights,
	                   Genode::Ram_dataspace_capability ds)
	{
		return call<Rpc_map_dataspace>(addr, rights, ds);
	}

	long unmap_dataspace(ULONGLONG addr,
	                     Genode::Ram_dataspace_capability ds)
	{
		return call<Rpc_unmap_dataspace>(addr, ds);
	}

	long get_dataspace(ULONGLONG *addr,
	                   ULONG *size,
	                   Genode::Ram_dataspace_capability *ds)
	{
		return call<Rpc_get_dataspace>(addr, size, ds);
	}

	long get_sharemem(ULONGLONG pb,
	                  ULONGLONG *addr,
	                  ULONG *size,
	                  ULONGLONG *owner)
	{
		return call<Rpc_get_sharemem>(pb, addr, size, owner);
	}

	long get_named_sharemem(Pathname &mName,
	                        ULONGLONG *addr,
	                        ULONG *size,
	                        ULONGLONG *owner)
	{
		return call<Rpc_get_named_sharemem>(mName, addr, size, owner);
	}

	long increment_sharemem_refcnt(ULONGLONG addr)
	{
		return call<Rpc_increment_sharemem_refcnt>(addr);
	}

	long release_sharemem(ULONGLONG addr,
	                      ULONG *count)
	{
		return call<Rpc_release_sharemem>(addr, count);
	}
};

#endif
