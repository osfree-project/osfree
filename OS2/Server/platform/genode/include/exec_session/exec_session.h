#ifndef __EXEC_SESSION_EXEC_SESSION_H__
#define __EXEC_SESSION_EXEC_SESSION_H__

/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree OS/2 personality internal */
#include <os3/loader.h>
#include <os3/ixfmgr.h>

/* Genode includes */
#include <session/session.h>
#include <base/rpc.h>
#include <base/rpc_args.h>
#include <base/ram_allocator.h>
#include <dataspace/capability.h>

namespace OS2::Exec { struct Session; }

struct OS2::Exec::Session : Genode::Session
{
	static const char *service_name() { return "os2exec"; }

	enum { CAP_QUOTA = 2 };

	typedef Genode::Rpc_in_buffer<CCHMAXPATHCOMP> Pathname;

	typedef struct { char str[CCHMAXPATHCOMP]; } Buf;

	//typedef struct { char buf[260]; } LoadError;

	virtual void test(Buf *str) = 0;

	/* Open a binary */
	virtual long open(Pathname &fName,
	                  unsigned long flags,
	                  Genode::Ram_dataspace_capability err_ds,
	                  unsigned long *pcbLoadError,
	                  unsigned long *hmod) = 0;

	/* Load the opened binary */
	virtual long load(unsigned long hmod,
	                  Genode::Ram_dataspace_capability err_ds,
	                  unsigned long *cbLoadError,
	                  Genode::Ram_dataspace_capability mod_ds) = 0;

	/* Share all section dataspaces for loaded binary
	   with calling process   */
	virtual long share(unsigned long hmod) = 0;

	/* get hmods of loaded module import with specified
	   hmod and index in the import table  */
	virtual long getimp(unsigned long hmod,
	                    unsigned long *index,
	                    unsigned long *imp_hmod) = 0;

	/* get next section for a given module */
	virtual long getsect(unsigned long hmod,
	                     unsigned long *index,
	                     Genode::Ram_dataspace_capability sect_ds) = 0;

	virtual long query_procaddr(unsigned long hmod,
	                            unsigned long ordinal,
	                            Pathname &mName,
	                            ULONGLONG *addr) = 0;
	
	virtual long query_modhandle(Pathname &pszModname,
	                             unsigned long *hmod) = 0;

	virtual long query_modname(unsigned long hmod,
	                           Genode::Ram_dataspace_capability ds) = 0;
	
	virtual long alloc_sharemem(ULONG size,
	                            Pathname &mName,
	                            ULONG rights,
	                            ULONGLONG *addr,
	                            ULONGLONG *area) = 0;

	virtual long map_dataspace(ULONGLONG addr,
                                   ULONG rights,
                                   Genode::Ram_dataspace_capability ds) = 0;

	virtual long unmap_dataspace(ULONGLONG addr,
	                             Genode::Ram_dataspace_capability ds) = 0;

	virtual long get_dataspace(ULONGLONG *addr,
	                           ULONG *size,
	                           Genode::Ram_dataspace_capability *ds) = 0;
	
	virtual long get_sharemem(ULONGLONG pb,
	                          ULONGLONG *addr,
	                          ULONG *size,
	                          ULONGLONG *owner) = 0;

	virtual long get_named_sharemem(Pathname &mName,
	                                ULONGLONG *addr,
	                                ULONG *size,
	                                ULONGLONG *owner) = 0;

	virtual long increment_sharemem_refcnt(ULONGLONG addr) = 0;

	virtual long release_sharemem(ULONGLONG addr,
	                              ULONG *count) = 0;

	/*
	 *  RPC interface
	 */

	GENODE_RPC(Rpc_test, void, test, Buf *);
	GENODE_RPC(Rpc_open, long, open,
	           Pathname &,
	           unsigned long,
	           Genode::Ram_dataspace_capability,
	           unsigned long *,
	           unsigned long *);
	GENODE_RPC(Rpc_load, long, load,
	           unsigned long,
	           Genode::Ram_dataspace_capability,
	           unsigned long *,
	           Genode::Ram_dataspace_capability);
	GENODE_RPC(Rpc_share, long, share,
	           unsigned long);
	GENODE_RPC(Rpc_getimp, long, getimp,
	           unsigned long, unsigned long *, unsigned long *);
	GENODE_RPC(Rpc_getsect, long, getsect,
	           unsigned long, unsigned long *,
	           Genode::Ram_dataspace_capability);
	GENODE_RPC(Rpc_query_procaddr, long, query_procaddr,
	           unsigned long, unsigned long, Pathname &,
	           ULONGLONG *);
	GENODE_RPC(Rpc_query_modhandle, long, query_modhandle,
	           Pathname &, unsigned long *);
	GENODE_RPC(Rpc_query_modname, long, query_modname,
	           unsigned long, Genode::Ram_dataspace_capability);
	GENODE_RPC(Rpc_alloc_sharemem, long, alloc_sharemem, ULONG, Pathname &,
	           ULONG, ULONGLONG *, ULONGLONG *);
	GENODE_RPC(Rpc_map_dataspace, long, map_dataspace,
	           ULONGLONG, ULONG, Genode::Ram_dataspace_capability);
	GENODE_RPC(Rpc_unmap_dataspace, long, unmap_dataspace,
	           ULONGLONG, Genode::Ram_dataspace_capability);
	GENODE_RPC(Rpc_get_dataspace, long, get_dataspace,
	           ULONGLONG *, ULONG *, Genode::Ram_dataspace_capability *);
	GENODE_RPC(Rpc_get_sharemem, long, get_sharemem,
	           ULONGLONG, ULONGLONG *, ULONG *, ULONGLONG *);
	GENODE_RPC(Rpc_get_named_sharemem, long, get_named_sharemem,
	           Pathname &, ULONGLONG *, ULONG *, ULONGLONG *);
	GENODE_RPC(Rpc_increment_sharemem_refcnt, long, increment_sharemem_refcnt,
	           ULONGLONG);
	GENODE_RPC(Rpc_release_sharemem, long, release_sharemem,
	           ULONGLONG, ULONG *);

	GENODE_RPC_INTERFACE(Rpc_test, Rpc_open, Rpc_load,
	   Rpc_share, Rpc_getimp, Rpc_getsect, Rpc_query_procaddr,
	   Rpc_query_modhandle, Rpc_query_modname, Rpc_alloc_sharemem,
	   Rpc_map_dataspace, Rpc_unmap_dataspace, Rpc_get_dataspace,
	   Rpc_get_sharemem, Rpc_get_named_sharemem, Rpc_increment_sharemem_refcnt,
	   Rpc_release_sharemem);
};

#endif
