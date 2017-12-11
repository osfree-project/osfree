#ifndef __INCLUDE__EXEC_SESSION__CPI_SESSION_H__
#define __INCLUDE__EXEC_SESSION__CPI_SESSION_H__

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
#include <dataspace/capability.h>

namespace OS2::Exec { struct Session; }

struct OS2::Exec::Session : Genode::Session
{
	static const char *service_name() { return "os2exec"; }

	enum { CAP_QUOTA = 2 };
	
	typedef struct { char str[CCHMAXPATHCOMP]; } Buf;

	typedef struct { char buf[260]; } LoadError;

	virtual void test(Buf &str) = 0;

	/* Open a binary */
	virtual long open(Genode::Rpc_in_buffer<CCHMAXPATHCOMP> &fname,
	                  Genode::Dataspace_capability img_ds,
	                  unsigned long flags,
	                  LoadError &szLoadError,
	                  unsigned long &cbLoadError,
	                  unsigned long &hmod) = 0;

	/* Load the opened binary */
	virtual long load(unsigned long hmod,
	                  LoadError &szLoadError,
	                  unsigned long &cbLoadError,
	                  os2exec_module_t &s) = 0;

	/* Share all section dataspaces for loaded binary
	   with calling process   */
	virtual long share(unsigned long hmod) = 0;

	/* get hmods of loaded module import with specified
	   hmod and index in the import table  */
	virtual long getimp(unsigned long hmod,
	                    unsigned long &index,
	                    unsigned long &imp_hmod) = 0;

	/* get next section for a given module */
	virtual long getsect(unsigned long hmod,
	                     unsigned long &index,
	                     l4exec_section_t *s) = 0;

	virtual long query_procaddr(unsigned long hmod,
	                            unsigned long ordinal,
	                            Genode::Rpc_in_buffer<CCHMAXPATHCOMP> &modname,
	                            void *&addr) = 0;

	virtual long query_modhandle(Genode::Rpc_in_buffer<CCHMAXPATHCOMP> &pszModname,
	                             unsigned long &hmod) = 0;

	virtual long query_modname(unsigned long hmod,
	                           unsigned long cbBuf,
	                           Buf &pszBuf) = 0;
	/*
	 *  RPC interface
	 */

	GENODE_RPC(Rpc_test, void, test, Buf &);
	GENODE_RPC(Rpc_open, long, open,
	           Genode::Rpc_in_buffer<CCHMAXPATHCOMP> &,
	           Genode::Dataspace_capability,
	           unsigned long,
	           LoadError &,
	           unsigned long &,
	           unsigned long &);
	GENODE_RPC(Rpc_load, long, load,
	           unsigned long,
	           LoadError &,
	           unsigned long &,
	           os2exec_module_t &);
	GENODE_RPC(Rpc_share, long, share,
	           unsigned long);
	GENODE_RPC(Rpc_getimp, long, getimp,
	           unsigned long, unsigned long &, unsigned long &);
	GENODE_RPC(Rpc_getsect, long, getsect,
	           unsigned long, unsigned long &, l4exec_section_t *);
	GENODE_RPC(Rpc_query_procaddr, long, query_procaddr,
	           unsigned long, unsigned long, Genode::Rpc_in_buffer<CCHMAXPATHCOMP> &,
	           void *&);
	GENODE_RPC(Rpc_query_modhandle, long, query_modhandle,
	           Genode::Rpc_in_buffer<CCHMAXPATHCOMP> &, unsigned long &);
	GENODE_RPC(Rpc_query_modname, long, query_modname,
	           unsigned long, unsigned long, Buf &);


	GENODE_RPC_INTERFACE(Rpc_test, Rpc_open, Rpc_load,
	   Rpc_share, Rpc_getimp, Rpc_getsect, Rpc_query_procaddr,
	   Rpc_query_modhandle, Rpc_query_modname);
};

#endif
