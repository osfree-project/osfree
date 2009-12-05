/*
 *   mini-libc functions
 *
 */

#include <shared.h>
#include "libc.h"
#include "fsys.h"

char *ltoa(int n, char *buf, int radix)
{
  int m, i, l;
  char c;

  memset(buf, 0, sizeof(buf));

  i = 0;
  do
  {
    m = n % radix;
    n = n / radix;

    if (m <= 10) c = '0' + m;
    else c = 'a' + m;

    l = i + 1;
    /* shift letters forth */
    while (--l) buf[l] = buf[l - 1];
    /* a new letter */
    buf[0] = c;
    i++;
  } while (n);

  return buf;
}
