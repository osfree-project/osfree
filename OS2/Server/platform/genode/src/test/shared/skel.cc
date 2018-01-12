/* libc-based Genode application skeleton */

/* Genode includes */
#include <base/heap.h>
#include <libc/component.h>

/* local includes */
#include "genode_env.h"

extern Genode::Allocator *_alloc;
extern Genode::Env *_env_ptr;

extern "C" int main(void);

void Libc::Component::construct(Libc::Env &env)
{
    Genode::Heap heap(env.ram(), env.rm());
    init_genode_env(env, heap);

    main();
}
