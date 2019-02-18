#define  INCL_BASE
#include <os2.h>

#include <base/thread.h>

namespace OS2
{
    struct Thread;
}

struct OS2::Thread : Genode::Thread
{
private:
    ThreadFunc _fn;

    ULONG _flags;

    void *_data;

    Genode::Semaphore _term_wait { 0 };

public:
    enum { DEFAULT_STACKSIZE = 0x6000 };

    Thread(Genode::Env &env, ThreadFunc fn, void *data,
           ULONG flags, Name name = "osFree thread",
           Genode::size_t stacksize = DEFAULT_STACKSIZE)
      : Genode::Thread(env, name, stacksize)
    {
        _fn = fn;
        _flags = flags;
        _data = data;

        start();

        if (_flags & THREAD_SYNC)
        {
            _term_wait.up();
        }
    }

    void entry()
    {
        _fn(_data);

        if (_flags & THREAD_SYNC)
        {
            _term_wait.down();
        }
    }
};
