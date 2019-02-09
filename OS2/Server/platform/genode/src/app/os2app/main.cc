/* Genode includes */
#include <base/heap.h>
#include <libc/component.h>
#include <cpi_session/connection.h>
#include <base/attached_rom_dataspace.h>

#include <os3/io.h>
#include <os3/cpi.h>
#include <os3/exec.h>
#include <os3/fs.h>
#include <os3/stacksw.h>

#include <stdlib.h>

#include <genode_env.h>

Genode::Env *_env_ptr = NULL;
Genode::Allocator *_alloc = NULL;

namespace OS2::App {
    struct Main;
}

struct OS2::App::Main
{
    Libc::Env &env;

    Genode::Attached_rom_dataspace config { env, "config" };

    Genode::Heap heap { env.ram(), env.rm() };

    void __exit(ULONG action, ULONG result)
    {
        STKIN
        // send OS/2 server a message that we want to terminate
        io_log("action=%lu\n", action);
        io_log("result=%lu\n", result);
        CPClientExit(action, result);
        // tell L4 task server that we want to terminate
        exit(result);
        STKOUT
    }

    Main(Libc::Env &env) : env(env)
    {
        //char szLoadError[260];
        APIRET rc;

        init_genode_env(env, heap);

        io_log("---os2app started---\n");

        if ( (rc = CPClientInit()) )
        {
            io_log("Can't find os2srv exiting...\n");
            __exit(1, 1);
        }

        if ( (rc = FSClientInit()) )
        {
            io_log("Can't find os2fs, exiting...\n");
            __exit(1, 1);
        }

        if ( (rc = ExcClientInit()) )
        {
            io_log("Can't find os2exec, exiting...\n");
            __exit(1, 1);
        }

        // reserve the lower 64 Mb for OS/2 app
        // ...

        // reserve the upper 1 Gb for shared memory arena
        // ...

        // Parse config
        // ...

        CPClientTest();

        io_log("calling KalStartApp...\n");
        //KalStartApp(argv[argc - 1], pszLoadError, sizeof(pszLoadError));
    }

    ~Main()
    {
        FSClientDone();
        ExcClientDone();
        CPClientDone();
    }
};

void Libc::Component::construct(Libc::Env &env)
{
    static OS2::App::Main main(env);
}
