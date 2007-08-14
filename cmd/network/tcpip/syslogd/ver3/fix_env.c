#include <string.h>
#include <malloc.h>
#include <stdlib.h>

char *fix_env(char *s1, char *s2)
{
  char *etc, *help1, *help2, *help3;

  help3 = NULL;

  help1=malloc(strlen(s1)+1);
  strcpy(help1,s1);
  help2=help1;
  while (*help2)
  {
    if((*help2=='/')||(*help2=='\\'))
      help3=++help2;
    else help2++;
  }
  if ((etc = getenv(s2)) != NULL)
  {
    strcpy(s1,etc);
    if ((s1[strlen(s1)-1]!='/') &&(s1[strlen(s1)-1]!='\\'))
      strcat(s1,"\\");
    strcat(s1,help3);
  }
  free(help1);
  return(s1);
}
