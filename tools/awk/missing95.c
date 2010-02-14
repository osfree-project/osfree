/* popen and pclose are not part of win 95 and nt,
   but it appears that _popen and _pclose "work".
   if this won't load, use the return NULL statements. */

#ifndef __LINUX__
#include <stdio.h>
FILE *popen(char *s, char *m) {
	return _popen(s, m);	/* return NULL; */
}

int pclose(FILE *f) {
	return _pclose(f);	/* return NULL; */
}

#else
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Prototypes


#define READ 0
#define WRITE 1

FILE *popen(const char * s, const char * m)
{
  int pfp[2], pid;
  FILE *fdopen(), *fp;
  int parent_end, child_end;
  
  if (*m=='r')
  {
    parent_end=READ;
    child_end=WRITE;
  } else if (*m=='w') {
    parent_end=WRITE;
    child_end=READ;
  } else return NULL;
  
  if (pipe(pfp)==-1)
    return NULL;
  if ((pid=fork())==-1)
  {
     close(pfp[0]);
     close(pfp[1]);
     return NULL;
  }
  
  if (pid >0)
  {
    if (close(pfp[child_end])==-1) return NULL;
    return fdopen (pfp[parent_end], m);
  }
  
  if (close(pfp[parent_end]) ==-1) exit(1);
  
  if (dup2(pfp[child_end], child_end)==-1 ) exit(1);
  
  if (close(pfp[child_end])==-1) exit(1);
  
  execl("/bin/sh", "sh", "-c", s, NULL);
  exit(1);
  return NULL;
}


int pclose(FILE *f)
{
  fclose(f);
  return NULL;
}
#endif
