#include <string.h>

size_t strnlen(const char * str, size_t n)
{
  const char *start = str;

  while (*str && n-- > 0)
    str++;

  return str - start;
}
