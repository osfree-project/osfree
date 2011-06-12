/*  string routines
 *
 *
 */
 
char *
strupr (char *str)
{
  char *s = str;
  
  while (*s)
    *s++ = toupper (*s);
  
  return str;
}
