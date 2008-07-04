/* kit2msgs - program to convert kitten databases to the msgs.h format

   AUTHOR: Gregory Pietsch
   
   This program is freely distributable; just don't claim you wrote it.
*/

#include <ctype.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int
main (void)
{
  unsigned char buf[32768], *p;
  int n;

  while ((p = fgets (buf, 32768, stdin)) != 0)
    {
      if (*p == '#')
        {
          p[strlen (p) - 1] = '\0';
          printf ("/* %-75s */\n", buf + 2);
        }
      else if (isspace (*p))
        printf ("\n");
      else
        {
          for (n = 0, p = buf + 2; isdigit (*p); n = n * 10 + *p - '0', p++)
            ;
          if (n == 0)
            printf ("#ifndef MSGS_H\n#define MSGS_H\n\n#define YES\t\"");
          else
            printf ("#define G%05d\t\"", n);
          while (*p++ != '\n')
            {
              if (*p > 127)
                printf ("\\%03o", *p);
              else if (isprint (*p))
                putchar (*p);
            }
          printf ("\"\n");
        }
    }
  printf ("\n#endif\n\n/* END OF FILE */\n");
  return 0;
}

/* END OF FILE */
