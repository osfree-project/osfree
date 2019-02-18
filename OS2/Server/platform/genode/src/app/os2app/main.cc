/* Genode includes */
#include <base/heap.h>
#include <util/xml_node.h>
#include <libc/component.h>
#include <cpi_session/connection.h>
#include <base/attached_rom_dataspace.h>

/* osFree internal */
#include <os3/io.h>
#include <os3/kal.h>
#include <os3/cpi.h>
#include <os3/exec.h>
#include <os3/fs.h>
#include <os3/stacksw.h>

/* libc includes */
#include <stdlib.h>

#include <genode_env.h>

Genode::Env *_env_ptr = NULL;
Genode::Allocator *_alloc = NULL;

extern "C" {

/* OS/2 server id        */
l4_os3_thread_t os2srv;
l4_os3_thread_t fs;

int init(struct options *opts);
void done(void);

struct options
{
  char  use_events;
  const char  *progname;
};

void reserve_regions(void)
{
    /* Need a real implementation! */
}

}

namespace OS2::App {
    struct Main;
}

struct OS2::App::Main
{
    Libc::Env &env;

    Genode::Attached_rom_dataspace config { env, "config" };

    Genode::Heap heap { env.ram(), env.rm() };

    void parse_options (Genode::Xml_node node, struct options *opts)
    {
        try
        {
            Genode::String<64> cfg = node.sub_node("arg")
                .attribute_value("value", Genode::String<64>("c:\\mini33.exe"));

            opts->use_events = 0;
            opts->progname = cfg.string();
            io_log("progname=%s\n", opts->progname);
        }
        catch (Genode::Xml_node::Nonexistent_sub_node) { };
    }

    Main(Libc::Env &env) : env(env)
    {
        struct options opts = {0};

        /* init environment and allocator variables */
        init_genode_env(env, heap);

        /* parse options */
        parse_options(config.xml(), &opts);

        /* call platform-independent init */
        init(&opts);
    }

    ~Main()
    {
        /* destruct */
        done();
    }
};

void Libc::Component::construct(Libc::Env &env)
{
    static OS2::App::Main main(env);
}
