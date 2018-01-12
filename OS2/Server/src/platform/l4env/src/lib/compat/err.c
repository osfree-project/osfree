/* error interface */

/* l4env includes */
#include <l4/env/errno.h>

char const *l4os3_errtostr(long err)
{
  return l4env_errstr(err);
}
