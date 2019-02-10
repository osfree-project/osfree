/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree OS/2 personality internal */
#include <os3/io.h>
#include <os3/memmgr.h>
#include <os3/modmgr.h>
#include <os3/cfgparser.h>
#include <os3/cpi.h>
#include <os3/fs.h>

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

cfg_opts options;

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
private:
	Genode::Env &env;

public:
	Session_component(Genode::Env &env)
	:
	env(env) {  }

	void test(OS2::Exec::Session::Buf *buf)
	{
		const char *s = "qwerty";
		Genode::memcpy((char *)buf->str, s, Genode::strlen(s));
	}

	long open(Pathname &fName,
	          unsigned long flags,
	          Genode::Ram_dataspace_capability ds,
	          unsigned long *cbLoadError,
	          unsigned long *hmod)
	{
		char *addr = env.rm().attach(ds);

		return ExcOpen(addr, *cbLoadError,
		               fName.string(), flags, hmod);
	}

	long load(unsigned long hmod,
	          Genode::Ram_dataspace_capability err_ds,
	          unsigned long *cbLoadError,
	          Genode::Ram_dataspace_capability mod_ds)
	{
		char *addr = env.rm().attach(err_ds);
		char *addr2 = env.rm().attach(mod_ds);

		return ExcLoad(&hmod, addr,
		               *cbLoadError, (os2exec_module_t *)addr2);
	}

	long share(unsigned long hmod)
	{
		return ExcShare(hmod, (void *)this);
	}

	long getimp(unsigned long hmod,
	            unsigned long *index,
	            unsigned long *imp_hmod)
	{
		return ExcGetImp(hmod, index, imp_hmod);
	}

	long getsect(unsigned long hmod,
	             unsigned long *index,
	             Genode::Ram_dataspace_capability sect_ds)
	{
		l4_os3_section_t *sect = env.rm().attach(sect_ds);

		return ExcGetSect(hmod, index, sect);
	}

	long query_procaddr(unsigned long hmod,
	                    unsigned long ordinal,
	                    Pathname &mName,
	                    ULONGLONG *addr)
	{
		return ExcQueryProcAddr(hmod, ordinal, (PSZ)mName.string(), (void **)addr);
	}

	long query_modhandle(Pathname &mName,
	                     unsigned long *hmod)
	{
		return ExcQueryModuleHandle(mName.string(), hmod);
	}

	long query_modname(unsigned long hmod,
	                   Genode::Ram_dataspace_capability ds)
	{
		char *addr = env.rm().attach(ds);
		ULONG cbSize = Genode::Dataspace_client(ds).size();

		return ExcQueryModuleName(hmod, cbSize, addr);
	}

	long alloc_sharemem(ULONG size,
	                    Pathname &mName,
	                    ULONG rights,
	                    ULONGLONG *addr,
	                    ULONGLONG *area)
	{
		return ExcAllocSharedMem(size, (PSZ)mName.string(), rights,
		                         (void **)addr, area);
	}

	long map_dataspace(ULONGLONG addr,
                           ULONG rights,
                           Genode::Ram_dataspace_capability ds)
	{
		return ExcMapDataspace((void *)addr, rights,
		                       (l4_os3_dataspace_t)&ds);
	}

	long unmap_dataspace(ULONGLONG addr,
	                     Genode::Ram_dataspace_capability ds)
	{
		return ExcUnmapDataspace((void *)addr,
		                         (l4_os3_dataspace_t)&ds);
	}

	long get_dataspace(ULONGLONG *addr,
	                   ULONG *size,
	                   Genode::Ram_dataspace_capability *ds)
	{
		return ExcGetDataspace((void **)addr, size,
		                       (l4_os3_dataspace_t *)&ds, (void *)this);
	}

	long get_sharemem(ULONGLONG pb,
	                  ULONGLONG *addr,
	                  ULONG *size,
	                  PID *owner)
	{
		return ExcGetSharedMem((void *)pb, (void **)addr,
		                       size, (PID *)owner);
	}

	long get_named_sharemem(Pathname &mName,
	                        ULONGLONG *addr,
	                        ULONG *size,
	                        PID *owner)
	{
		return ExcGetNamedSharedMem(mName.string(), (void **)addr,
		                            size, (PID *)owner);
	}

	long increment_sharemem_refcnt(ULONGLONG addr)
	{
		return ExcIncrementSharedMemRefcnt((void *)addr);
	}

	long release_sharemem(ULONGLONG addr,
	                      ULONG *count)
	{
		return ExcReleaseSharedMem((void *)addr, count);
	}
};

class OS2::Exec::Root : public Genode::Root_component<Session_component>
{
private:
	Genode::Env &_env;

protected:
	Session_component *_create_session(const char *args)
	{
		return new (md_alloc()) Session_component(_env);
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
		os2exec_module_t s = {0, 0, 0, 0, 0, 0};
		char szLoadError[260];
		l4_os3_thread_t thread;
		unsigned long size;
		void *addr;
		APIRET rc;

		io_log("osFree Exec server started\n");

		memset (&options, 0, sizeof(options));

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

		if ( CPClientInit() )
		{
		    io_log("os2srv not found on name server!\n");
		    return;
		}

		if ( FSClientInit() )
		{
		    io_log("os2fs not found on name server!\n");
		    return;
		}

		// KAL virtual library map file
		options.kal_map = (char *)"kal.map";

		init_memmgr(&root_area);

		/* Load IXF's */
		rc = load_ixfs();

		if (rc)
		{
		    io_log("Error loading IXF drivers!\n");
		    return;
		}

		/* reserve the area below 64 Mb for application private code
		   (not for use by libraries, loaded by execsrv) */
		// addr = (void *)0x2f000; size = 0x04000000 - (unsigned long)addr;
		// ...

		// reserve the upper 1 Gb for shared memory arena
		// ...

#if 0
		if (! CfgGetopt("debugmodmgr", &is_int, &value_int, (char **)&p) )
		{
		    if (is_int)
		        options.debugmodmgr = value_int;
		}

		if (! CfgGetopt("debugixfmgr", &is_int, &value_int, (char **)&p) )
		{
		    if (is_int)
		      options.debugixfmgr = value_int;
		}

		if (! CfgGetopt("libpath", &is_int, &value_int, (char **)&p) )
		{
		    if (! is_int)
		    {
		      options.libpath = (char *)malloc(strlen(p) + 1);
		      strcpy(options.libpath, p);
		    }
		}

		io_log("debugmodmgr=%d\n", options.debugmodmgr);
		io_log("debugixfmgr=%d\n", options.debugixfmgr);
		io_log("libpath=%s\n", options.libpath);
#endif
		//options.libpath = (char *)malloc(4);
		//strcpy(options.libpath, "c:\\");

		/* Initializes the module list. Keeps info about which dlls an process have loaded and
		   has linked to it (Only support for LX dlls so far). The head of the linked list is
		   declared as a global inside dynlink.c */
		rc = ModInitialize();

		if (rc)
		{
		    io_log("Can't initialize module manager\n");
		    return;
		}

		// notify os2srv about successful startup
		CPClientAppNotify2(&s, "os2exec", &thread,
		                   szLoadError, sizeof(szLoadError), rc);

		// announce 'os2exec' service
		env.parent().announce(env.ep().manage(root));
	}

	~Main()
	{
		FSClientDone();
		CPClientDone();
	}
};

void Libc::Component::construct(Libc::Env &env)
{
	static OS2::Exec::Main main(env);
}
