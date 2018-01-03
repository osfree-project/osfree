/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree OS/2 personality internal */
#include <os3/io.h>
#include <os3/memmgr.h>
#include <os3/modmgr.h>
#include <os3/cfgparser.h>

/* Genode includes */
#include <base/log.h>
#include <base/heap.h>
#include <base/attached_rom_dataspace.h>
#include <libc/component.h>
#include <root/component.h>
#include <exec_session/exec_session.h>
#include <base/rpc_server.h>
#include <util/string.h>

/* libc includes */
#include <string.h>
#include <stdlib.h>

/* local includes */
#include <genode_env.h>
#include "api.h"

#define OPENFLAG_EXEC 1

Genode::Env *_env_ptr = NULL;
Genode::Allocator *_alloc = NULL;

extern cfg_opts options;

/* shared memory arena settings */
extern void         *shared_memory_base;
extern unsigned long shared_memory_size;
extern unsigned long long shared_memory_area;

/* Root mem area for memmgr */
struct t_mem_area root_area;

namespace OS2::Exec {
	struct Session_component;
	struct Root;
	struct Main;
}


struct OS2::Exec::Session_component : Genode::Rpc_object<Session>
{
	void test(OS2::Exec::Session::Buf *buf)
	{
		const char *s = "qwerty";
		Genode::memcpy((char *)buf->str, s, Genode::strlen(s));
	}

	long open(Genode::Rpc_in_buffer<CCHMAXPATHCOMP> *fname,
	          unsigned long flags,
	          LoadError *szLoadError,
	          unsigned long *cbLoadError,
	          unsigned long *hmod)
	{
		return ExcOpen(szLoadError->buf, *cbLoadError,
		               fname->string(), flags, hmod);
	}

	long load(unsigned long hmod,
	          LoadError *szLoadError,
	          unsigned long *cbLoadError,
	          os2exec_module_t *s)
	{
		return ExcLoad(&hmod, szLoadError->buf,
		               *cbLoadError, s);
	}

	long share(unsigned long hmod, Genode::Capability<void> *client_id)
	{
		return ExcShare(hmod, client_id);
	}

	long getimp(unsigned long hmod,
	            unsigned long *index,
	            unsigned long *imp_hmod)
	{
		return ExcGetImp(hmod, index, imp_hmod);
	}

	long getsect(unsigned long hmod,
	             unsigned long *index,
	             l4exec_section_t *sect)
	{
		return ExcGetSect(hmod, index, sect);
	}

	long query_procaddr(unsigned long hmod,
	                    unsigned long ordinal,
	                    Genode::Rpc_in_buffer<CCHMAXPATHCOMP> *modname,
	                    ULONGLONG *addr)
	{
		return ExcQueryProcAddr(hmod, ordinal, modname->string(), (void **)addr);
	}

	long query_modhandle(Genode::Rpc_in_buffer<CCHMAXPATHCOMP> *pszModname,
	                     unsigned long *hmod)
	{
		return ExcQueryModuleHandle(pszModname->string(), hmod);
	}

	long query_modname(unsigned long hmod,
	                   unsigned long cbBuf,
	                   Buf *pszBuf)
	{
		return ExcQueryModuleName(hmod, cbBuf, pszBuf->str);
	}

	long alloc_sharemem(ULONG size,
	                    char *name,
	                    ULONG rights,
	                    ULONGLONG *addr,
	                    ULONGLONG *area)
	{
		return ExcAllocSharedMem(size, name, rights,
		                         (void **)addr, area);
	}
};

class OS2::Exec::Root : public Genode::Root_component<Session_component>
{
private:
	Genode::Env &_env;

protected:
	Session_component *_create_session(const char *args)
	{
		return new (md_alloc()) Session_component();
	}

public:
	Root(Genode::Env &env,
	     Genode::Allocator &alloc)
	:
		Genode::Root_component<Session_component>(env.ep(), alloc),
	 _env(env) { init_genode_env(env, alloc); }
};

struct OS2::Exec::Main
{
	Genode::Env &env;

	Genode::Attached_rom_dataspace config { env, "config" };

	Genode::Sliced_heap sliced_heap { env.ram(), env.rm() };

	OS2::Exec::Root root { env, sliced_heap };

	Main(Genode::Env &env) : env(env)
	{
		unsigned long size;
		void *addr;
		APIRET rc;

		// Initialize initial values from CONFIG.SYS
		rc = CfgInitOptions();

		if (rc != NO_ERROR)
		{
		    io_log("Can't initialize CONFIG.SYS parser\n");
		    return;
		}

		options.configfile = (char *)"config.sys";

		try
		{
		    Genode::String<64> cfg = config.xml().sub_node("config-file")
			    .attribute_value("value", Genode::String<64>("config.sys"));
		    options.configfile = (char *)cfg.string();
		}
		catch (Genode::Xml_node::Nonexistent_sub_node) { };

		io_log("options.configfile=%s\n", options.configfile);

		// Load CONFIG.SYS into memory
		rc = io_load_file(options.configfile, &addr, &size);

		if (rc != NO_ERROR)
		{
		    io_log("Can't load CONFIG.SYS\n");
		    return;
		}

		// Parse CONFIG.SYS in memory
		rc = CfgParseConfig((char *)addr, size);

		if (rc != NO_ERROR)
		{
		    io_log("Error parsing CONFIG.SYS!\n");
		    return;
		}

		// Release all memory allocated by parser
		CfgCleanup();

		// Remove CONFIG.SYS from memory
		io_close_file(addr);

		init_memmgr(&root_area);

		/* Load IXF's */
		rc = load_ixfs();

		if (rc)
		{
		    io_log("Error loading IXF drivers!\n");
		    return;
		}

		/* Initializes the module list. Keeps info about which dlls an process have loaded and
		   has linked to it (Only support for LX dlls so far). The head of the linked list is
		   declared as a global inside dynlink.c */
		rc = ModInitialize();

		if (rc)
		{
		    io_log("Can't initialize module manager\n");
		    return;
		}

		// announce 'os2exec' service
		env.parent().announce(env.ep().manage(root));
	}
};

void Libc::Component::construct(Libc::Env &env)
{
	static OS2::Exec::Main main(env);
}
