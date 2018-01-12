/*  string routines
 *
 *
 */

#include <ctype.h>

char *strupr (char *str);
 
char *
strupr (char *str)
{
  char *s = str;

  for (;*s ; s++)
    *s = toupper (*s);

  return str;
}
