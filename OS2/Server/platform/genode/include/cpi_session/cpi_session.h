#ifndef __CPI_SESSION_CPI_SESSION_H__
#define __CPI_SESSION_CPI_SESSION_H__

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

namespace OS2::Cpi { struct Session; }

struct OS2::Cpi::Session : Genode::Session
{
	static const char *service_name() { return "cpi"; }

	enum { CAP_QUOTA = 2 };

	typedef Genode::Rpc_in_buffer<CCHMAXPATHCOMP> Pathname;

        virtual void test(void) = 0;

        GENODE_RPC(Rpc_test, void, test);

        GENODE_RPC_INTERFACE(Rpc_test);
};

#endif
