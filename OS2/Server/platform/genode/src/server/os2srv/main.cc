/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/io.h>
#include <os3/thread.h>
#include <os3/cfgparser.h>

/* Genode includes */
#include <base/heap.h>
#include <base/log.h>
#include <base/attached_ram_dataspace.h>
#include <base/attached_rom_dataspace.h>
#include <libc/component.h>
#include <root/component.h>
#include <cpi_session/cpi_session.h>
#include <base/rpc_server.h>

/* local includes */
#include "genode_env.h"
#include "api.h"

Genode::Env *_env_ptr = NULL;
Genode::Allocator *_alloc = NULL;

l4_os3_thread_t sysinit_id;

cfg_opts options;

extern "C" int sysinit (cfg_opts *options);

namespace OS2::Cpi
{
    struct Session_component;
    struct Root;
    struct Main;
}

extern "C"
void exit_notify(void)
{
}

extern "C" l4_os3_thread_t
CPNativeID(void)
{
    return INVALID_THREAD;
}

struct OS2::Cpi::Session_component : Genode::Rpc_object<Session>
{
private:
    Libc::Env &_env;

public:
    Session_component(Libc::Env &env)
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

    bool syscall(Syscall sc)
    {
        bool result = false;
        APIRET rc;

        switch (sc)
        {
            case SYSCALL_TEST:
                Genode::log("Hello OS/2!");
                result = true;
                break;

            case SYSCALL_CFGGETENV:
                CPCfgGetenv(_sysio.cfggetenv_in.name,
                            (char **)&_sysio.cfggetenv_out.value);
                result = true;
                break;

            case SYSCALL_CFGGETOPT:
                rc = CPCfgGetopt(_sysio.cfggetopt_in.name,
                                 &_sysio.cfggetopt_out.is_int,
                                 &_sysio.cfggetopt_out.value_int,
                                 (char **)&_sysio.cfggetopt_out.value_str);
                 _sysio.cfggetopt_out.rc = rc;
                result = true;
                break;

            case SYSCALL_APPNOTIFY1:
                CPAppNotify1(this);
                result = true;
                break;

            case SYSCALL_APPNOTIFY2:
                // note: in l4env we change lthread here
                // (to lthread of os2app service thread)
                CPAppNotify2(this,
                             &_sysio.appnotify2_in.s,
                             (char *)_sysio.appnotify2_in.pszName,
                             _sysio.appnotify2_in.pid,
                             (char *)_sysio.appnotify2_in.szLoadError,
                             _sysio.appnotify2_in.cbLoadError,
                             _sysio.appnotify2_in.ret);
                result = true;
                break;

            case SYSCALL_EXIT:
                CPExit(this,
                       _sysio.exit_in.action,
                       _sysio.exit_in.result);
                result = true;
                break;

            case SYSCALL_EXECPGM:
                rc = CPExecPgm(this,
                               (char **)&_sysio.execpgm_out.pObjname,
                               &_sysio.execpgm_in.cbObjname,
                               _sysio.execpgm_in.execFlag,
                               _sysio.execpgm_in.pArgs,
                               _sysio.execpgm_in.arglen,
                               _sysio.execpgm_in.pEnv,
                               _sysio.execpgm_in.envlen,
                               &_sysio.execpgm_out.pRes,
                               _sysio.execpgm_in.pName);
                _sysio.execpgm_out.rc = rc;
                if (! rc)
                {
                    result = true;
                }
                break;

            case SYSCALL_INVALID:
            default:
                io_log("invalid syscall!\n");
        }

        return result;
    }
};

struct OS2::Cpi::Root : public Genode::Root_component<Session_component>
{
private:
    Libc::Env &_env;

protected:
    Session_component *_create_session(const char *args)
    {
        return new (md_alloc()) Session_component(_env);
        args = args;
    }

public:
    Root(Libc::Env &env,
         Genode::Allocator &alloc)
    :
    Genode::Root_component<Session_component>(env.ep(),
                                              alloc),
    _env(env) { init_genode_env(env, alloc); }
};

struct OS2::Cpi::Main
{
    Libc::Env &env;

    Genode::Attached_rom_dataspace config { env, "config" };

    Genode::Sliced_heap sliced_heap { env.ram(), env.rm() };

    OS2::Cpi::Root root { env, sliced_heap };

    Main(Libc::Env &env) : env(env)
    {
        unsigned long size;
        void *addr;
        APIRET rc;

        io_log("---os2srv started---\n");

        // Initialize initial values from CONFIG.SYS
        rc = CfgInitOptions();

        if (rc != NO_ERROR)
        {
            io_log("Can't initialize CONFIG.SYS parser\n");
            return;
        }

        // default config.sys path
        options.configfile = (char *)"config.sys";

        try
        {
            Genode::String<64> cfg = config.xml().sub_node("config-file")
                .attribute_value("value", Genode::String<64>("config.sys"));
                options.configfile = (char *)cfg.string();
        }
        catch (Genode::Xml_node::Nonexistent_sub_node) { };

        io_log("options.configfile=%s\n", options.configfile);

        //CPClientInit();

        // Load CONFIG.SYS into memory
        rc = io_load_file(options.configfile, &addr, &size);

        if (rc != NO_ERROR)
        {
            io_log("Can't load CONFIG.SYS\n");
            return;
        }

        //io_log("%s\n", (char *)addr);

        // Parse CONFIG.SYS in memory
        rc = CfgParseConfig((char *)addr, size);

        if (rc != NO_ERROR)
        {
            io_log("Error parse CONFIG.SYS\n");
            return;
        }

        // Remove CONFIG.SYS from memory
        io_close_file(addr);

        // Perform the System initialization
        ThreadCreate((ThreadFunc)sysinit, (void *)&options, THREAD_ASYNC);

        // announce "cpi" service
        env.parent().announce(env.ep().manage(root));
    }

    ~Main()
    {
        CPClientDone();
    }
};

void Libc::Component::construct(Libc::Env &env)
{
    static OS2::Cpi::Main main(env);
}
