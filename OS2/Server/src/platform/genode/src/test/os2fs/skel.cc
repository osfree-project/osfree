/* libc-based Genode application skeleton */

/* Genode includes */
#include <libc/component.h>

/* local includes */
#include "genode_env.h"

extern "C" void main(void);

Genode::Allocator *_alloc = NULL;
Genode::Env *_env_ptr = NULL;

void Libc::Component::construct(Libc::Env &env)
{
    _env_ptr = &env;

    main();
}
