#ifndef __OS3_ERR_H__
#define __OS3_ERR_H__

#include <l4/env/errno.h>

L4_CV char const *l4sys_errtostr(long err);

// implementation

L4_CV char const *l4sys_errtostr(long err)
{
  return l4env_errstr(err);
}

#endif
