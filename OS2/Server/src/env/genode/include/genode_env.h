#ifndef __GENODE_ENV_H__
#define __GENODE_ENV_H__

/* Genode includes */
#include <base/component.h>

extern Genode::Env *_env_ptr;
extern Genode::Allocator *_alloc;

inline Genode::Env &genode_env()
{
    if (_env_ptr)
        return *_env_ptr;

    throw 1;
}

inline Genode::Allocator &genode_alloc()
{
    if (_alloc)
        return *_alloc;

    throw 1;
}

inline void init_genode_env(Genode::Env &env, Genode::Allocator &alloc)
{
    _env_ptr = &env;
    _alloc = &alloc;
}

#endif /* __GENODE_ENV_H__ */
