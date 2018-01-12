/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/io.h>
#include <os3/MountReg.h>
#include <os3/globals.h>
#include <os3/cfgparser.h>

/* Genode includes */
#include <base/log.h>
#include <base/heap.h>
#include <base/attached_rom_dataspace.h>
#include <base/attached_ram_dataspace.h>
#include <libc/component.h>
#include <root/component.h>
#include <fs_session/fs_session.h>
#include <base/rpc_server.h>
#include <dataspace/client.h>
#include <base/signal.h>
#include <rom_session/rom_session.h>

/* local includes */
#include "genode_env.h"
#include "api.h"

extern "C" int FSR_INIT(void);

Genode::Env *_env_ptr = NULL;
Genode::Allocator *_alloc = NULL;

extern cfg_opts options;

namespace OS2::Fs
{
    struct Fs_session_component;
    struct Fs_root;
    struct Rom_session_component;
    struct Rom_root;
    struct Main;
}

struct OS2::Fs::Rom_session_component : Genode::Rpc_object<Genode::Rom_session>
{
private:
    Genode::Ram_session &_ram;
    Genode::Region_map &_rm;

    Genode::Session_label const &_label;
    Genode::Ram_dataspace_capability _file_ds;

    Genode::Ram_dataspace_capability _init_file_ds(Genode::Ram_session &ram,
                                                   Genode::Region_map &rm,
                                                   Genode::Session_label const &label)
    {
        Genode::Ram_dataspace_capability file_ds;
        Genode::Env &env = genode_env();
        ULONG fSize = 0;
        FILESTATUS3L Info;
        PFILESTATUS3L pInfo = &Info;
        ULONG cbSize = sizeof(Info);
        ULONG ulAction;
        HFILE hf;
        APIRET rc;

        // first try opening file at parent's ROM service
        try
        {
            Genode::Rom_connection rom(env, label.string());
            Genode::Attached_dataspace src(rm, rom.dataspace());
            fSize = src.size();

            file_ds = ram.alloc(fSize);
            Genode::Attached_dataspace dst(rm, file_ds);

            Genode::memcpy(dst.local_addr<char>(),
                           src.local_addr<char>(),
                           src.size());
            return file_ds;
        }
        catch (...)
        {
            if (file_ds.valid())
                ram.free(file_ds);

            io_log("rom connection for %s failed!\n", label.string());
        }

        // otherwise try opening it as a file
        io_log("now opening it as a file: %s\n", label.string());

        Libc::with_libc([&] () {

            rc = FSOpenL((PSZ)label.string(),
                         &hf,
                         &ulAction,
                         0,
                         0,
                         OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS,
                         OPEN_FLAGS_FAIL_ON_ERROR | OPEN_SHARE_DENYNONE |
                         OPEN_ACCESS_READONLY,
                         NULL);

            io_log("FSOpen rc=%u\n", rc);

            if (rc)
            {
                return file_ds;
            }

            rc = FSQueryFileInfo(hf,
                                 FIL_STANDARDL,
                                 (char **)&pInfo,
                                 &cbSize);

            io_log("FSQueryFileInfo rc=%u\n", rc);

            if (rc)
            {
                return file_ds;
            }

            fSize = Info.cbFile;

            try
            {
                file_ds = ram.alloc(fSize);
            }
            catch (...)
            {
                io_log("Could not allocate memory for a dataspace, size %u\n", fSize);
                return file_ds;
            }

            Genode::Attached_dataspace ds(rm, file_ds);

            io_log("fSize=%u\n", fSize);

            rc = FSRead(hf, ds.local_addr<char>(), &fSize);

            io_log("FSRead rc=%u\n", rc);

            FSClose(hf);
            return file_ds;

        });

        return file_ds;
    }

public:
    Genode::Rom_dataspace_capability dataspace()
    {
        Genode::Dataspace_capability ds = _file_ds;
        return Genode::static_cap_cast<Genode::Rom_dataspace>(ds);
    }

    void sigh(Genode::Signal_context_capability sigh) { }

    // constructor
    Rom_session_component(Genode::Ram_session &ram, Genode::Region_map &rm,
                              Genode::Session_label const &label)
    :
    _ram(ram), _rm(rm),
    _label(label), _file_ds(_init_file_ds(ram, rm, label))
    {
        if (! _file_ds.valid())
            throw Genode::Service_denied();
    }

    // destructor
    ~Rom_session_component()
    {
        _ram.free(_file_ds);
    }
};

struct OS2::Fs::Fs_session_component : Genode::Rpc_object<Session>
{
private:
    Libc::Env &env;

public:
    Fs_session_component(Libc::Env &env)
    :
    env(env) {  }

    long get_drivemap(ULONG *map)
    {
        return FSGetDriveMap(map);
    }

    APIRET dos_Read(HFILE hFile,
                    Genode::Ram_dataspace_capability &ds,
                    ULONG *count)
    {
        APIRET rc;
        char *addr = env.rm().attach(ds);

        Libc::with_libc([&] () {
            rc = FSRead(hFile, addr, count);
        });

        return rc;
    }

    APIRET dos_Write(HFILE hFile,
                     Genode::Ram_dataspace_capability &ds,
                     ULONG *count)
    {
        APIRET rc;
        char *addr = env.rm().attach(ds);

        Libc::with_libc([&] () {
            rc =  FSWrite(hFile, addr, count);
        });

        return rc;
    }

    APIRET dos_ResetBuffer(HFILE hFile)
    {
        return FSResetBuffer(hFile);
    }

    APIRET dos_SetFilePtrL(HFILE hFile,
                           LONGLONG ib,
                           ULONG method,
                           ULONGLONG *ibActual)
    {
        return FSSetFilePtrL(hFile, ib, method, ibActual);
    }

    APIRET dos_Close(HFILE hFile)
    {
        return FSClose(hFile);
    }

    APIRET dos_QueryHType(HFILE hFile,
                          ULONG *pType,
                          ULONG *pAttr)
    {
        return FSQueryHType(hFile, pType, pAttr);
    }

    APIRET dos_OpenL(Pathname &fName,
                     HFILE *phFile,
                     ULONG *pulAction,
                     LONGLONG cbSize,
                     ULONG ulAttribute,
                     ULONG fsOpenFlags,
                     ULONG fsOpenMode)
                     // EAOP2 *peaop2
    {
        APIRET rc;

        Libc::with_libc([&] () {
            rc = FSOpenL((PSZ)fName.string(), phFile, pulAction,
                         cbSize, ulAttribute, fsOpenFlags,
                         fsOpenMode, NULL); //, (EAOP2 *)peaop2);
        });

        return rc;
    }

    APIRET dos_DupHandle(HFILE hFile,
                         HFILE *phFile2)
    {
        return FSDupHandle(hFile, phFile2);
    }

    APIRET dos_Delete(Pathname &fName)
    {
        return FSDelete((PSZ)fName.string());
    }

    APIRET dos_ForceDelete(Pathname &fName)
    {
        return FSForceDelete((PSZ)fName.string());
    }

    APIRET dos_DeleteDir(Pathname &dName)
    {
        return FSDeleteDir((PSZ)dName.string());
    }

    APIRET dos_CreateDir(Pathname &dName,
                         Genode::Ram_dataspace_capability &ds)
    {
        EAOP2 *peaop2 = (EAOP2 *)env.rm().attach(ds);
        return FSCreateDir((PSZ)dName.string(), peaop2);
    }

    APIRET dos_FindFirst(Pathname &pName,
                         HDIR *phDir,
                         ULONG flAttribute,
                         Genode::Ram_dataspace_capability &ds,
                         ULONG *pcFileNames,
                         ULONG ulInfoLevel)
    {
        char *addr = env.rm().attach(ds);
        ULONG cbSize = Genode::Dataspace_client(ds).size();

        return FSFindFirst((PSZ)pName.string(), phDir, flAttribute,
                           &addr, &cbSize, pcFileNames, ulInfoLevel);
    }

    APIRET dos_FindNext(HDIR hDir,
                        Genode::Ram_dataspace_capability &ds,
                        ULONG *pcFileNames)
    {
        char *addr = env.rm().attach(ds);
        ULONG cbSize = Genode::Dataspace_client(ds).size();

        return FSFindNext(hDir, &addr, &cbSize, pcFileNames);
    }

    APIRET dos_FindClose(HDIR hDir)
    {
        return FSFindClose(hDir);
    }

    APIRET dos_QueryFHState(HFILE hFile,
                            ULONG *pulMode)
    {
        return FSQueryFHState(hFile, pulMode);
    }

    APIRET dos_SetFHState(HFILE hFile,
                          ULONG ulMode)
    {
        return FSSetFHState(hFile, ulMode);
    }

    APIRET dos_QueryFileInfo(HFILE hFile,
                             ULONG ulInfoLevel,
                             Genode::Ram_dataspace_capability &ds)
    {
        char *addr = env.rm().attach(ds);
        ULONG cbSize = Genode::Dataspace_client(ds).size();

        return FSQueryFileInfo(hFile, ulInfoLevel,
                               &addr, &cbSize);
    }

    APIRET dos_QueryPathInfo(Pathname &pName,
                             ULONG ulInfoLevel,
                             Genode::Ram_dataspace_capability &ds)
    {
        char *addr = env.rm().attach(ds);
        ULONG cbSize = Genode::Dataspace_client(ds).size();

        return FSQueryPathInfo((PSZ)pName.string(), ulInfoLevel,
                               &addr, &cbSize);
    }

    APIRET dos_SetFileSizeL(HFILE hFile,
                            LONGLONG cbSize)
    {
        return FSSetFileSizeL(hFile, cbSize);
    }

    APIRET dos_SetFileInfo(HFILE hFile,
                           ULONG ulInfoLevel,
                           Genode::Ram_dataspace_capability &ds)
    {
        char *addr = env.rm().attach(ds);
        ULONG cbSize = Genode::Dataspace_client(ds).size();

        return FSSetFileInfo(hFile, ulInfoLevel,
                             &addr, &cbSize);
    }

    APIRET dos_SetPathInfo(Pathname &pName,
                           ULONG ulInfoLevel,
                           Genode::Ram_dataspace_capability &ds,
                           ULONG flOptions)
    {
        char *addr = env.rm().attach(ds);
        ULONG cbSize = Genode::Dataspace_client(ds).size();

        return FSSetPathInfo((PSZ)pName.string(), ulInfoLevel,
                             &addr, &cbSize, flOptions);
    }
};

struct OS2::Fs::Fs_root : public Genode::Root_component<Fs_session_component>
{
private:
    Libc::Env &_env;

protected:
    Fs_session_component *_create_session(const char *args)
    {
        return new (md_alloc()) Fs_session_component(_env);
    }

public:
    Fs_root(Libc::Env &env,
            Genode::Allocator &alloc)
    :
    Genode::Root_component<Fs_session_component>(env.ep(),
                                                 alloc),
    _env(env) { init_genode_env(env, alloc); }
};

struct OS2::Fs::Rom_root : public Genode::Root_component<Rom_session_component>
{
private:
    Libc::Env &_env;

protected:
    Rom_session_component *_create_session(const char *args)
    {
        Genode::Session_label const &label = Genode::label_from_args(args);
        Genode::Session_label const &module = label.last_element();
        io_log("rom connection for %s requested\n", module.string());

        return new (md_alloc()) Rom_session_component(_env.ram(),
            _env.rm(), module);
    }

public:
    Rom_root(Libc::Env &env,
             Genode::Allocator &alloc)
    :
    Genode::Root_component<Rom_session_component>(env.ep(),
                                                  alloc),
    _env(env) {  }
};

struct OS2::Fs::Main
{
    Libc::Env &env;

    Genode::Attached_rom_dataspace config { env, "config" };

    Genode::Sliced_heap sliced_heap { env.ram(), env.rm() };

    OS2::Fs::Rom_root rom_root { env, sliced_heap };

    OS2::Fs::Fs_root fs_root { env, sliced_heap };

    Main(Libc::Env &env) : env(env)
    {
        init_globals();
        //FSR_INIT();

        // announce "ROM" service
        env.parent().announce(env.ep().manage(rom_root));

        // announce "os2fs" service
        env.parent().announce(env.ep().manage(fs_root));
    }
};

void Libc::Component::construct(Libc::Env &env)
{
    static OS2::Fs::Main main(env);
}
