/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree OS/2 personality internal */
#include <os3/io.h>
#include <os3/memmgr.h>
#include <os3/modmgr.h>
#include <os3/thread.h>
#include <os3/cfgparser.h>
#include <os3/cpi.h>
#include <os3/fs.h>

/* Genode includes */
#include <base/log.h>
#include <base/heap.h>
#include <base/capability.h>
#include <base/attached_ram_dataspace.h>
#include <base/attached_rom_dataspace.h>
#include <libc/component.h>
#include <root/component.h>
#include <base/rpc_server.h>
#include <util/string.h>

#include <cpi_session/cpi_session.h>

/* libc includes */
#include <string.h>
#include <stdlib.h>

/* local includes */
#include <genode_env.h>
#include "api.h"

#define OPENFLAG_EXEC 1

Genode::Env *_env_ptr = NULL;
Genode::Allocator *_alloc = NULL;

extern "C" {

struct options
{
    char use_events;
    char *configfile;
    char *kal_map;
};

l4_os3_thread_t os2srv;

extern l4_os3_thread_t fs;

cfg_opts options;

/* shared memory arena settings */
extern void         *shared_memory_base;
extern unsigned long shared_memory_size;
extern unsigned long long shared_memory_area;

int init(struct options *opts);
void done(void);

/* Root mem area for memmgr */
struct t_mem_area root_area;

void reserve_regions(void)
{
    /* Add a real implementation! */
}

}

namespace OS2::Cpi {
	struct Session_component;
	struct Root;
	struct Main;
}

struct OS2::Cpi::Session_component : Genode::Rpc_object<Session>
{
private:
	Genode::Env &_env;

	Genode::Untyped_capability _cap[4];

public:
	Session_component(Genode::Env &env)
	:
	_env(env) {  }

	enum { PAGE_SIZE = 4096, PAGE_MASK = ~(PAGE_SIZE - 1) };
	enum { SYSIO_DS_SIZE = PAGE_MASK & (sizeof(Sysio) + PAGE_SIZE - 1) };

	Genode::Attached_ram_dataspace _sysio_ds { _env.ram(), _env.rm(), SYSIO_DS_SIZE };
	Sysio &_sysio = *_sysio_ds.local_addr<Sysio>();

	Genode::Dataspace_capability sysio_dataspace()
	{
		return _sysio_ds.cap();
	}

	Genode::Untyped_capability get_cap(int index)
	{
		return _cap[index];
	}

	void send_cap(Genode::Untyped_capability cap, int index)
	{
		_cap[index] = cap;
	}

	bool syscall(Syscall sc)
	{
		bool result = false;
		APIRET rc;

		switch (sc)
		{
		    case SYSCALL_EXEC_OPEN:
		    {
		            Genode::Ram_dataspace_capability ds;
		            Genode::Untyped_capability cap = _cap[0];
		            ds = Genode::reinterpret_cap_cast<Genode::Ram_dataspace>(cap);
		            char *addr = _env.rm().attach(ds);
		            rc = ExcOpen(addr,
		                         _sysio.execopen.in.cbLoadError,
                                         _sysio.execopen.in.pszName,
                                         _sysio.execopen.in.flags,
                                         &_sysio.execopen.out.hmod);
		            _sysio.execopen.out.rc = rc;
		            _env.rm().detach(addr);
		            result = true;
		            break;
		    }

		    case SYSCALL_EXEC_LOAD:
		    {
		            Genode::Ram_dataspace_capability err_ds, mod_ds;
		            Genode::Untyped_capability cap = _cap[0];
		            err_ds = Genode::reinterpret_cap_cast<Genode::Ram_dataspace>(cap);
		            cap = _cap[1];
		            mod_ds = Genode::reinterpret_cap_cast<Genode::Ram_dataspace>(cap);
		            char *addr  = _env.rm().attach(err_ds);
		            os2exec_module_t *addr2 = _env.rm().attach(mod_ds);
		            rc = ExcLoad(&_sysio.execload.in.hmod,
		                         addr,
		                         _sysio.execload.in.cbLoadError,
                                         addr2);
		            _sysio.execload.out.rc = rc;
		            _env.rm().detach(addr);
		            _env.rm().detach(addr2);
		            result = true;
		            break;
		    }

		    case SYSCALL_EXEC_FREE:
		            rc = ExcFree(_sysio.execfree.in.hmod);
		            _sysio.execfree.out.rc = rc;
		            result = true;
		            break;

		    case SYSCALL_EXEC_SHARE:
		            rc = ExcShare(_sysio.execshare.in.hmod, this);
		            _sysio.execshare.out.rc = rc;
		            result = true;
		            break;

		    case SYSCALL_EXEC_GETIMP:
		            rc = ExcGetImp(_sysio.execgetimp.in.hmod,
                                           &_sysio.execgetimp.out.index,
                                           &_sysio.execgetimp.out.imp_hmod);
		            _sysio.execgetimp.out.rc = rc;
		            result = true;
		            break;

		    case SYSCALL_EXEC_GETSECT:
		    {
		            Genode::Ram_dataspace_capability ds;
		            Genode::Untyped_capability cap = _cap[0];
		            ds = Genode::reinterpret_cap_cast<Genode::Ram_dataspace>(cap);
		            l4_os3_section_t *sect = _env.rm().attach(ds);
		            rc = ExcGetSect(_sysio.execgetsect.in.hmod,
                                           &_sysio.execgetsect.out.index,
                                           sect);
		            _sysio.execgetsect.out.rc = rc;
		            _env.rm().detach(sect);
		            result = true;
		            break;
		    }

		    case SYSCALL_EXEC_QUERYPROCADDR:
		            rc = ExcQueryProcAddr(_sysio.execqueryprocaddr.in.hmod,
                                                  _sysio.execqueryprocaddr.in.ordinal,
                                                  (char *)_sysio.execqueryprocaddr.in.mName,
                                                  (void **)&_sysio.execqueryprocaddr.out.addr);
		            _sysio.execqueryprocaddr.out.rc = rc;
		            result = true;
		            break;

		    case SYSCALL_EXEC_QUERYMODULEHANDLE:
		            rc = ExcQueryModuleHandle((char *)_sysio.execquerymodulehandle.in.mName,
                                                      &_sysio.execquerymodulehandle.out.hmod);
		            _sysio.execquerymodulehandle.out.rc = rc;
		            result = true;
		            break;

		    case SYSCALL_EXEC_QUERYMODULENAME:
		            rc = ExcQueryModuleName(_sysio.execquerymodulename.in.hmod,
                                                    _sysio.execquerymodulename.in.cbName,
                                                    (char *)_sysio.execquerymodulename.out.pbName);
		            _sysio.execquerymodulename.out.rc = rc;
		            result = true;
		            break;

		    case SYSCALL_EXEC_ALLOCSHAREDMEM:
		            rc = ExcAllocSharedMem(_sysio.execallocsharedmem.in.cbSize,
                                                   _sysio.execallocsharedmem.in.pszName,
                                                   _sysio.execallocsharedmem.in.rights,
                                                   &_sysio.execallocsharedmem.out.addr,
                                                   (ULONGLONG *)&_sysio.execallocsharedmem.out.area);
		            _sysio.execallocsharedmem.out.rc = rc;
		            result = true;
		            break;

		    case SYSCALL_EXEC_MAPDATASPACE:
		    {
		            l4_os3_dataspace_t ds;
		            Genode::Ram_dataspace_capability _ds;
		            Genode::Untyped_capability cap = _cap[0];
		            _ds = Genode::reinterpret_cap_cast<Genode::Ram_dataspace>(cap);
		            ds = (l4_os3_dataspace_t)&_ds;
		            rc = ExcMapDataspace(_sysio.execmapdataspace.in.addr,
                                                 _sysio.execmapdataspace.in.rights,
                                                 ds);
		            _sysio.execmapdataspace.out.rc = rc;
		            result = true;
		            break;
		    }

		    case SYSCALL_EXEC_UNMAPDATASPACE:
		    {
		            l4_os3_dataspace_t ds;
		            Genode::Ram_dataspace_capability _ds;
		            Genode::Untyped_capability cap = _cap[0];
		            _ds = Genode::reinterpret_cap_cast<Genode::Ram_dataspace>(cap);
		            ds = (l4_os3_dataspace_t)&_ds;
		            rc = ExcUnmapDataspace(_sysio.execunmapdataspace.in.addr,
                                                   ds);
		            _sysio.execunmapdataspace.out.rc = rc;
		            result = true;
		            break;
		    }

		    case SYSCALL_EXEC_GETDATASPACE:
		    {
		            l4_os3_dataspace_t ds;
		            Genode::Ram_dataspace_capability _ds;
		            rc = ExcGetDataspace(&_sysio.execgetdataspace.out.addr,
                                                 &_sysio.execgetdataspace.out.size,
                                                 &ds, this);
                            _ds = *(Genode::Ram_dataspace_capability *)ds;
                            _cap[0] = (Genode::Untyped_capability)_ds;
		            _sysio.execgetdataspace.out.rc = rc;
		            result = true;
		            break;
		    }

		    case SYSCALL_EXEC_GETSHAREDMEM:
		            rc = ExcGetSharedMem(_sysio.execgetsharedmem.in.pb,
                                                 &_sysio.execgetsharedmem.out.addr,
                                                 &_sysio.execgetsharedmem.out.size,
                                                 &_sysio.execgetsharedmem.out.owner);
		            _sysio.execgetsharedmem.out.rc = rc;
		            result = true;
		            break;

		    case SYSCALL_EXEC_GETNAMEDSHAREDMEM:
		            rc = ExcGetNamedSharedMem(_sysio.execgetnamedsharedmem.in.pszName,
                                                      &_sysio.execgetnamedsharedmem.out.addr,
                                                      &_sysio.execgetnamedsharedmem.out.size,
                                                      &_sysio.execgetnamedsharedmem.out.owner);
		            _sysio.execgetnamedsharedmem.out.rc = rc;
		            result = true;
		            break;

		    case SYSCALL_EXEC_INCREMENTSHAREDMEMREFCNT:
		            rc = ExcIncrementSharedMemRefcnt(_sysio.execincrementsharedmemrefcnt.in.addr);
		            _sysio.execincrementsharedmemrefcnt.out.rc = rc;
		            result = true;
		            break;

		    case SYSCALL_EXEC_RELEASESHAREDMEM:
		            rc = ExcReleaseSharedMem(_sysio.execreleasesharedmem.in.addr,
                                                     &_sysio.execreleasesharedmem.out.count);
		            _sysio.execreleasesharedmem.out.rc = rc;
		            result = true;
		            break;

		    default:
		        io_log("invalid syscall!\n");
		}

		return result;
	}
};

class OS2::Cpi::Root : public Genode::Root_component<Session_component>
{
private:
	Genode::Env &_env;

protected:
	Session_component *_create_session(const char *args)
	{
		Genode::Session_label const &label = Genode::label_from_args(args);
		Genode::Session_label const &module = label.last_element();
		io_log("CPI connection for %s requested\n", module.string());

		return new (md_alloc()) Session_component(_env);
	}

public:
	Root(Genode::Env &env,
	     Genode::Allocator &alloc)
	:
		Genode::Root_component<Session_component>(env.ep(), alloc),
	 _env(env) { init_genode_env(env, alloc); }
};

struct OS2::Cpi::Main
{
	Genode::Env &_env;

	Genode::Attached_rom_dataspace config { _env, "config" };

	Genode::Sliced_heap sliced_heap { _env.ram(), _env.rm() };

	OS2::Cpi::Root root { _env, sliced_heap };

	void parse_options(Genode::Xml_node node, struct options *opts)
	{
		opts->configfile = (char *)"config.sys";
		opts->kal_map = (char *)"kal.map";

		try
		{
		    Genode::String<64> cfg = node.sub_node("config-file")
			    .attribute_value("value", Genode::String<64>("config.sys"));
		    opts->configfile = (char *)cfg.string();
		}
		catch (Genode::Xml_node::Nonexistent_sub_node) { };
	}

	Main(Genode::Env &env) : _env(env)
	{
		struct options opts = {0};

		parse_options(config.xml(), &opts);

		/* call platform-independent init */
		init(&opts);

		/* announce 'CPI' service */
		_env.parent().announce(_env.ep().manage(root));
	}

	~Main()
	{
		/* destruct */
		done();
	}
};

void Libc::Component::construct(Libc::Env &env)
{
	static OS2::Cpi::Main main(env);
}
