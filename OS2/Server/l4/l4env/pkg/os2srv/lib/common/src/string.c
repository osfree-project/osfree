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
  
  while (*s)
    *s++ = toupper (*s);
  
  return str;
}
