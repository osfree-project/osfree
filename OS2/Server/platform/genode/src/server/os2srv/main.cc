/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/io.h>
#include <os3/cfgparser.h>

/* Genode includes */
#include <base/heap.h>
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

extern cfg_opts options;

namespace OS2::Cpi
{
    struct Session_component;
    struct Root;
    struct Main;
}

struct OS2::Cpi::Session_component : Genode::Rpc_object<Session>
{
private:
    Libc::Env &env;

public:
    Session_component(Libc::Env &env)
    :
    env(env) {  }

    void test(void)
    {
        io_log("Hello OS/2!\n");
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
        //init_globals();

        // announce "cpi" service
        env.parent().announce(env.ep().manage(root));
    }
};

void Libc::Component::construct(Libc::Env &env)
{
    static OS2::Cpi::Main main(env);
}
