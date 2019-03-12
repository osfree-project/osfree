/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/io.h>
#include <os3/thread.h>
#include <os3/processmgr.h>
#include <os3/cfgparser.h>

/* Genode includes */
#include <base/heap.h>
#include <base/log.h>
#include <base/thread.h>
#include <base/attached_ram_dataspace.h>
#include <base/attached_rom_dataspace.h>
#include <libc/component.h>
#include <root/component.h>
#include <base/rpc_server.h>

#include <cpi_session/cpi_session.h>

/* local includes */
#include "genode_env.h"
#include "api.h"

Genode::Env *_env_ptr = NULL;
Genode::Allocator *_alloc = NULL;

extern "C" {

struct options
{
  char use_events;
  char *configfile;
  char *bootdrive;
  char fprov[20];
};

//int sysinit (cfg_opts *options);
int init(struct options *opts);
void done(void);

void exit_notify(void)
{
}

l4_os3_thread_t CPNativeID(void)
{
    Genode::Thread *thread = Genode::Thread::myself();
    return thread;
}

}

namespace OS2::Cpi
{
    struct Session_component;
    struct Root;
    struct Main;
}

struct OS2::Cpi::Session_component : Genode::Rpc_object<Session>
{
private:
    Libc::Env &_env;

    Genode::Untyped_capability _cap[4];

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
            case SYSCALL_MAIN_TEST:
                CPTest();
                result = true;
                break;

            case SYSCALL_MAIN_CFGGETENV:
                rc = CPCfgGetenv(_sysio.cfggetenv.in.name,
                                 (char **)&_sysio.cfggetenv.out.value);
                _sysio.cfggetenv.out.rc = rc;
                result = true;
                break;

            case SYSCALL_MAIN_CFGGETOPT:
                rc = CPCfgGetopt(_sysio.cfggetopt.in.name,
                                 &_sysio.cfggetopt.out.is_int,
                                 &_sysio.cfggetopt.out.value_int,
                                 (char **)&_sysio.cfggetopt.out.value_str);
                _sysio.cfggetopt.out.rc = rc;
                result = true;
                break;

            case SYSCALL_MAIN_APPNOTIFY1:
                CPAppNotify1(this);
                result = true;
                break;

            case SYSCALL_MAIN_APPNOTIFY2:
                // note: in l4env we change lthread here
                // (to lthread of os2app service thread)
                CPAppNotify2(this,
                             &_sysio.appnotify2.in.s,
                             (char *)_sysio.appnotify2.in.pszName,
                             (char *)_sysio.appnotify2.in.szLoadError,
                             _sysio.appnotify2.in.cbLoadError,
                             _sysio.appnotify2.in.ret);
                result = true;
                break;

            case SYSCALL_MAIN_APPSEND:
                rc = CPAppAddData(&_sysio.appsend.in.data);
                _sysio.appsend.out.rc = rc;
                result = true;
                break;

            case SYSCALL_MAIN_APPGET:
                {
                    struct t_os2process *proc = PrcGetProcNative(this);
                    PID pid;

                    if (! proc)
                    {
                        return ERROR_PROC_NOT_FOUND;
                    }

                    pid = proc->pid;
                    rc = CPAppGetData(pid, &_sysio.appget.out.data);
                    _sysio.appsend.out.rc = rc;
                    result = true;
                    break;
                }

            case SYSCALL_MAIN_EXIT:
                rc = CPExit(this,
                            _sysio.exit.in.action,
                            _sysio.exit.in.result);
                _sysio.exit.out.rc = rc;
                result = true;
                break;

            case SYSCALL_MAIN_EXECPGM:
                rc = CPExecPgm(this,
                               (char **)&_sysio.execpgm.out.pObjname,
                               &_sysio.execpgm.in.cbObjname,
                               _sysio.execpgm.in.execFlag,
                               _sysio.execpgm.in.pArgs,
                               _sysio.execpgm.in.arglen,
                               _sysio.execpgm.in.pEnv,
                               _sysio.execpgm.in.envlen,
                               &_sysio.execpgm.out.pRes,
                               _sysio.execpgm.in.pName);
                _sysio.execpgm.out.rc = rc;
                if (! rc)
                {
                    result = true;
                }
                break;

            case SYSCALL_MAIN_GETPIB:
                {
                    l4_os3_dataspace_t _ds;
                    Genode::Dataspace_capability *ds;
                    rc = CPGetPIB(_sysio.getpib.in.pid, this, &_ds);
                    _sysio.getpib.out.rc = rc;
                    ds = (Genode::Dataspace_capability *)_ds;
                    _cap[0] = (Genode::Untyped_capability)*ds;
                    result = true;
                    break;
                }

            case SYSCALL_MAIN_GETTIB:
                {
                    l4_os3_dataspace_t _ds;
                    Genode::Dataspace_capability *ds;
                    rc = CPGetTIB(_sysio.gettib.in.pid,
                                  _sysio.gettib.in.tid,
                                  this, &_ds);
                    _sysio.gettib.out.rc = rc;
                    ds = (Genode::Dataspace_capability *)_ds;
                    _cap[0] = (Genode::Untyped_capability)*ds;
                    result = true;
                    break;
                }

            case SYSCALL_MAIN_ERROR:
                rc = CPError(_sysio.error.in.error);
                _sysio.error.out.rc = rc;
                result = true;
                break;

            case SYSCALL_MAIN_QUERYDBCSENV:
                rc = CPQueryDBCSEnv(&_sysio.querydbcsenv.in.cb,
                                    &_sysio.querydbcsenv.in.cc,
                                    (char **)&_sysio.querydbcsenv.out.pBuf);
                _sysio.querydbcsenv.out.cb = _sysio.querydbcsenv.in.cb;
                _sysio.querydbcsenv.out.rc = rc;
                result = true;
                break;

            case SYSCALL_MAIN_QUERYCP:
                rc = CPQueryCp(&_sysio.querycp.in.cb,
                              (char **)&_sysio.querycp.out.arCP);
                _sysio.querycp.out.cb = _sysio.querycp.in.cb;
                _sysio.querycp.out.rc = rc;
                result = true;
                break;

            case SYSCALL_MAIN_QUERYCURRENTDISK:
                rc = CPQueryCurrentDisk(this, &_sysio.querycurrentdisk.out.disknum);
                _sysio.querycurrentdisk.out.rc = rc;
                result = true;
                break;

            case SYSCALL_MAIN_QUERYCURRENTDIR:
                rc = CPQueryCurrentDir(this,
                                       _sysio.querycurrentdir.in.disknum,
                                       _sysio.querycurrentdir.in.logical,
                                       (char **)&_sysio.querycurrentdir.out.pBuf,
                                       &_sysio.querycurrentdir.out.cbBuf);
                _sysio.querycurrentdir.out.rc = rc;
                result = true;
                break;

            case SYSCALL_MAIN_SETCURRENTDIR:
                rc = CPSetCurrentDir(this,
                                     (char *)_sysio.setcurrentdir.in.pszDir);
                _sysio.setcurrentdir.out.rc = rc;
                result = true;
                break;

            case SYSCALL_MAIN_SETDEFAULTDISK:
                rc = CPSetDefaultDisk(this,
                                      _sysio.setdefaultdisk.in.disknum,
                                      _sysio.setdefaultdisk.in.logical);
                _sysio.setdefaultdisk.out.rc = rc;
                result = true;
                break;

            case SYSCALL_MAIN_CREATEEVENTSEM:
                rc = CPCreateEventSem(this,
                                      _sysio.createeventsem.in.pszName,
                                      &_sysio.createeventsem.out.hev,
                                      _sysio.createeventsem.in.flAttr,
                                      _sysio.createeventsem.in.fState);
                _sysio.createeventsem.out.rc = rc;
                result = true;
                break;

            case SYSCALL_MAIN_OPENEVENTSEM:
                rc = CPOpenEventSem(this,
                                    _sysio.openeventsem.in.pszName,
                                    &_sysio.openeventsem.out.hev);
                _sysio.openeventsem.out.rc = rc;
                result = true;
                break;

            case SYSCALL_MAIN_CLOSEEVENTSEM:
                rc = CPCloseEventSem(this,
                                     _sysio.closeeventsem.in.hev);
                _sysio.closeeventsem.out.rc = rc;
                result = true;
                break;

            case SYSCALL_MAIN_GETPID:
                rc = CPGetPID(this,
                              &_sysio.getpid.out.pid);
                _sysio.getpid.out.rc = rc;
                result = true;
                break;

            case SYSCALL_MAIN_GETNATIVEID:
                rc = CPGetNativeID(_sysio.getnativeid.in.pid,
                                   _sysio.getnativeid.in.tid,
                                   &_sysio.getnativeid.out.id);
                _sysio.getnativeid.out.rc = rc;
                result = true;
                break;

            case SYSCALL_MAIN_NEWTIB:
                rc = CPNewTIB(_sysio.newtib.in.pid,
                              _sysio.newtib.in.tid,
                              &_sysio.newtib.in.id);
                _sysio.newtib.out.rc = rc;
                result = true;
                break;

            case SYSCALL_MAIN_DESTROYTIB:
                rc = CPDestroyTIB(_sysio.destroytib.in.pid,
                                  _sysio.destroytib.in.tid);
                _sysio.destroytib.out.rc = rc;
                result = true;
                break;

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
        Genode::Session_label const &label = Genode::label_from_args(args);
        Genode::Session_label const &module = label.last_element();
        io_log("CPI connection for %s requested\n", module.string());

        return new (md_alloc()) Session_component(_env);
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

    void parse_options(Genode::Xml_node node, struct options *opts)
    {
        try
        {
            Genode::String<64> cfg = node.sub_node("config-file")
                .attribute_value("value", Genode::String<64>("config.sys"));

            opts->configfile = (char *)cfg.string();
        }
        catch (Genode::Xml_node::Nonexistent_sub_node) { };
    }

    Main(Libc::Env &env) : env(env)
    {
        struct options opts = {0};

        /* parse options */
        parse_options(config.xml(), &opts);

        /* call platform-independent init */
        init(&opts);

        /* announce "CPI" service */
        env.parent().announce(env.ep().manage(root));
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
