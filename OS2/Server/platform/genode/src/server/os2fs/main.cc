/* OS/2 API includes */
#define  INCL_BASE
#include <os2.h>

/* osFree internal */
#include <os3/io.h>
#include <os3/MountReg.h>
#include <os3/globals.h>
#include <os3/cfgparser.h>
#include <os3/cpi.h>

/* Genode includes */
#include <base/log.h>
#include <base/heap.h>
#include <base/attached_rom_dataspace.h>
#include <base/attached_ram_dataspace.h>
#include <libc/component.h>
#include <root/component.h>
#include <base/rpc_server.h>
#include <dataspace/client.h>
#include <base/signal.h>

#include <cpi_session/cpi_session.h>
#include <rom_session/rom_session.h>

/* local includes */
#include "genode_env.h"
#include "api.h"

Genode::Env *_env_ptr = NULL;
Genode::Allocator *_alloc = NULL;

extern "C" {

int init(struct options *opts);
void done(void);

int FSR_INIT(void);

extern cfg_opts options;

struct options
{
  char use_events;
};

}

namespace OS2::Cpi
{
    struct Cpi_session_component;
    struct Cpi_root;
    struct Rom_session_component;
    struct Rom_root;
    struct Main;
}

struct OS2::Cpi::Rom_session_component : Genode::Rpc_object<Genode::Rom_session>
{
private:
    Genode::Ram_allocator &_ram;
    Genode::Region_map &_rm;

    Genode::Session_label const &_label;
    Genode::Ram_dataspace_capability _file_ds;

    Genode::Ram_dataspace_capability _init_file_ds(Genode::Ram_allocator &ram,
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

    void sigh(Genode::Signal_context_capability sigh) { sigh = sigh; }

    // constructor
    Rom_session_component(Genode::Ram_allocator &ram, Genode::Region_map &rm,
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

struct OS2::Cpi::Cpi_session_component : Genode::Rpc_object<Session>
{
private:
    Genode::Env &_env;

    Genode::Untyped_capability _cap[4];

public:
    Cpi_session_component(Libc::Env &env)
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
            case SYSCALL_FS_GETDRIVEMAP:
                rc = FSGetDriveMap(&_sysio.fsgetdrivemap.out.map);
                _sysio.fsgetdrivemap.out.rc = rc;
                result = true;
                break;

            case SYSCALL_FS_READ:
            {
                Genode::Ram_dataspace_capability ds;
                ds = Genode::reinterpret_cap_cast<Genode::Ram_dataspace>(_cap[0]);
                char *addr = _env.rm().attach(ds);
                _sysio.fsread.out.cbActual = _sysio.fsread.in.cbRead;

                Libc::with_libc([&] () {
                    rc = FSRead(_sysio.fsread.in.hFile,
                                addr,
                                &_sysio.fsread.out.cbActual);
                });

                _sysio.fsread.out.rc = rc;
                _env.rm().detach(addr);
                result = true;
                break;
            }

            case SYSCALL_FS_WRITE:
            {
                Genode::Ram_dataspace_capability ds;
                ds = Genode::reinterpret_cap_cast<Genode::Ram_dataspace>(_cap[0]);
                char *addr = _env.rm().attach(ds);
                _sysio.fswrite.out.cbActual = _sysio.fswrite.in.cbWrite;

                Libc::with_libc([&] () {
                    rc = FSWrite(_sysio.fswrite.in.hFile,
                                 addr,
                                 &_sysio.fswrite.out.cbActual);
                });

                _sysio.fswrite.out.rc = rc;
                _env.rm().detach(addr);
                result = true;
                break;
            }

            case SYSCALL_FS_RESETBUFFER:
                rc = FSResetBuffer(_sysio.fsresetbuffer.in.hFile);
                _sysio.fsresetbuffer.out.rc = rc;
                result = true;
                break;

            case SYSCALL_FS_SETFILEPTRL:
                rc = FSSetFilePtrL(_sysio.fssetfileptrl.in.hFile,
                                   _sysio.fssetfileptrl.in.ib,
                                   _sysio.fssetfileptrl.in.method,
                                   &_sysio.fssetfileptrl.out.ibActual);
                _sysio.fssetfileptrl.out.rc = rc;
                result = true;
                break;

            case SYSCALL_FS_CLOSE:
                rc = FSClose(_sysio.fsclose.in.hFile);
                _sysio.fsclose.out.rc = rc;
                result = true;
                break;

            case SYSCALL_FS_QUERYHTYPE:
                rc = FSQueryHType(_sysio.fsqueryhtype.in.hFile,
                                  &_sysio.fsqueryhtype.out.type,
                                  &_sysio.fsqueryhtype.out.attr);
                _sysio.fsqueryhtype.out.rc = rc;
                result = true;
                break;

            case SYSCALL_FS_OPENL:
                rc = FSOpenL(_sysio.fsopenl.in.pszFilename,
                             &_sysio.fsopenl.out.hFile,
                             &_sysio.fsopenl.out.ulAction,
                             _sysio.fsopenl.in.cbFile,
                             _sysio.fsopenl.in.ulAttribute,
                             _sysio.fsopenl.in.fsOpenFlags,
                             _sysio.fsopenl.in.fsOpenMode,
                             &_sysio.fsopenl.out.eaop2);
                _sysio.fsopenl.out.rc = rc;
                result = true;
                break;

            case SYSCALL_FS_DUPHANDLE:
                rc = FSDupHandle(_sysio.fsduphandle.in.hFile,
                                 &_sysio.fsduphandle.out.hFile2);
                _sysio.fsduphandle.out.rc = rc;
                result = true;
                break;

            case SYSCALL_FS_DELETE:
                rc = FSDelete(_sysio.fsdelete.in.pszFilename);
                _sysio.fsdelete.out.rc = rc;
                result = true;
                break;

            case SYSCALL_FS_FORCEDELETE:
                rc = FSForceDelete(_sysio.fsforcedelete.in.pszFilename);
                _sysio.fsforcedelete.out.rc = rc;
                result = true;
                break;

            case SYSCALL_FS_DELETEDIR:
                rc = FSDeleteDir(_sysio.fsdeletedir.in.pszDirname);
                _sysio.fsdeletedir.out.rc = rc;
                result = true;
                break;

            case SYSCALL_FS_CREATEDIR:
                rc = FSCreateDir(_sysio.fscreatedir.in.pszDirname,
                                 &_sysio.fscreatedir.in.eaop2);
                _sysio.fscreatedir.out.rc = rc;
                result = true;
                break;

            case SYSCALL_FS_FINDFIRST:
            {
                Genode::Ram_dataspace_capability ds;
                ds = Genode::reinterpret_cap_cast<Genode::Ram_dataspace>(_cap[0]);
                char *addr = _env.rm().attach(ds);

                Libc::with_libc([&] () {
                    rc = FSFindFirst(_sysio.fsfindfirst.in.pszFilespec,
                                     &_sysio.fsfindfirst.out.hDir,
                                     _sysio.fsfindfirst.in.ulAttribute,
                                     &addr,
                                     &_sysio.fsfindfirst.out.cbBuf,
                                     &_sysio.fsfindfirst.out.cFileNames,
                                     _sysio.fsfindfirst.in.ulInfoLevel);
                });

                _sysio.fsfindfirst.out.rc = rc;
                _env.rm().detach(addr);
                result = true;
                break;
            }

            case SYSCALL_FS_FINDNEXT:
            {
                Genode::Ram_dataspace_capability ds;
                ds = Genode::reinterpret_cap_cast<Genode::Ram_dataspace>(_cap[0]);
                char *addr = _env.rm().attach(ds);

                Libc::with_libc([&] () {
                    rc = FSFindNext(_sysio.fsfindnext.in.hDir,
                                    &addr,
                                    &_sysio.fsfindnext.out.cbBuf,
                                    &_sysio.fsfindnext.out.cFileNames);
                });

                _sysio.fsfindnext.out.rc = rc;
                _env.rm().detach(addr);
                result = true;
                break;
            }

            case SYSCALL_FS_FINDCLOSE:
            {
                rc = FSFindClose(_sysio.fsfindclose.in.hDir);
                _sysio.fsfindclose.out.rc = rc;
                result = true;
                break;
            }

            case SYSCALL_FS_QUERYFHSTATE:
            {
                rc = FSQueryFHState(_sysio.fsqueryfhstate.in.hFile,
                                    &_sysio.fsqueryfhstate.out.ulMode);
                _sysio.fsqueryfhstate.out.rc = rc;
                result = true;
                break;
            }

            case SYSCALL_FS_SETFHSTATE:
            {
                rc = FSSetFHState(_sysio.fssetfhstate.in.hFile,
                                  _sysio.fssetfhstate.in.ulMode);
                _sysio.fssetfhstate.out.rc = rc;
                result = true;
                break;
            }

            case SYSCALL_FS_QUERYFILEINFO:
            {
                Genode::Ram_dataspace_capability ds;
                ds = Genode::reinterpret_cap_cast<Genode::Ram_dataspace>(_cap[0]);
                char *addr = _env.rm().attach(ds);
                rc = FSQueryFileInfo(_sysio.fsqueryfileinfo.in.hFile,
                                     _sysio.fsqueryfileinfo.in.ulInfoLevel,
                                     &addr,
                                     &_sysio.fsqueryfileinfo.out.cbInfoBuf);
                _sysio.fsqueryfileinfo.out.rc = rc;
                _env.rm().detach(addr);
                result = true;
                break;
            }

            case SYSCALL_FS_QUERYPATHINFO:
            {
                Genode::Ram_dataspace_capability ds;
                ds = Genode::reinterpret_cap_cast<Genode::Ram_dataspace>(_cap[0]);
                char *addr = _env.rm().attach(ds);
                rc = FSQueryPathInfo(_sysio.fsquerypathinfo.in.pszPathName,
                                     _sysio.fsquerypathinfo.in.ulInfoLevel,
                                     &addr,
                                     &_sysio.fsquerypathinfo.out.cbInfoBuf);
                _sysio.fsquerypathinfo.out.rc = rc;
                _env.rm().detach(addr);
                result = true;
                break;
            }

            case SYSCALL_FS_SETFILESIZEL:
            {
                rc = FSSetFileSizeL(_sysio.fssetfilesizel.in.hFile,
                                    _sysio.fssetfilesizel.in.cbSize);
                _sysio.fssetfilesizel.out.rc = rc;
                result = true;
                break;
            }

            case SYSCALL_FS_SETFILEINFO:
            {
                Genode::Ram_dataspace_capability ds;
                ds = Genode::reinterpret_cap_cast<Genode::Ram_dataspace>(_cap[0]);
                char *addr = _env.rm().attach(ds);
                rc = FSSetFileInfo(_sysio.fssetfileinfo.in.hFile,
                                   _sysio.fssetfileinfo.in.ulInfoLevel,
                                   &addr,
                                   &_sysio.fssetfileinfo.out.cbInfoBuf);
                _sysio.fssetfileinfo.out.rc = rc;
                _env.rm().detach(addr);
                result = true;
                break;
            }

            case SYSCALL_FS_SETPATHINFO:
            {
                Genode::Ram_dataspace_capability ds;
                ds = Genode::reinterpret_cap_cast<Genode::Ram_dataspace>(_cap[0]);
                char *addr = _env.rm().attach(ds);
                rc = FSSetPathInfo(_sysio.fssetpathinfo.in.pszPathName,
                                   _sysio.fssetpathinfo.in.ulInfoLevel,
                                   &addr,
                                   &_sysio.fssetpathinfo.out.cbInfoBuf,
                                   _sysio.fssetpathinfo.in.flOptions);
                _sysio.fssetpathinfo.out.rc = rc;
                _env.rm().detach(addr);
                result = true;
                break;
            }

            default:
                io_log("invalid syscall!\n");
        }

        return result;
    }
};

struct OS2::Cpi::Cpi_root : public Genode::Root_component<Cpi_session_component>
{
private:
    Libc::Env &_env;

protected:
    Cpi_session_component *_create_session(const char *args)
    {
        Genode::Session_label const &label = Genode::label_from_args(args);
        Genode::Session_label const &module = label.last_element();
        io_log("CPI connection for %s requested\n", module.string());

        return new (md_alloc()) Cpi_session_component(_env);
    }

public:
    Cpi_root(Libc::Env &env,
             Genode::Allocator &alloc)
    :
    Genode::Root_component<Cpi_session_component>(env.ep(),
                                                  alloc),
    _env(env) { init_genode_env(env, alloc); }
};

struct OS2::Cpi::Rom_root : public Genode::Root_component<Rom_session_component>
{
private:
    Libc::Env &_env;

protected:
    Rom_session_component *_create_session(const char *args)
    {
        Genode::Session_label const &label = Genode::label_from_args(args);
        Genode::Session_label const &module = label.last_element();
        io_log("ROM connection for %s requested\n", module.string());

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

struct OS2::Cpi::Main
{
    Libc::Env &env;

    Genode::Attached_rom_dataspace config { env, "config" };

    Genode::Sliced_heap sliced_heap { env.ram(), env.rm() };

    OS2::Cpi::Rom_root rom_root { env, sliced_heap };

    OS2::Cpi::Cpi_root cpi_root { env, sliced_heap };

    Main(Libc::Env &env) : env(env)
    {
        struct options opts = {0};

        /* call platform independent init */
        init(&opts);

        // announce "ROM" service
        env.parent().announce(env.ep().manage(rom_root));

        // announce "CPI" service
        env.parent().announce(env.ep().manage(cpi_root));
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
