/*!
 *  @file missing95.c
 *  @brief popen/pclose compatibility layer for Windows and POSIX.
 *
 *  popen and pclose are not part of win 95 and nt, but it appears
 *  that _popen and _pclose "work". If this won't load, use the
 *  return NULL statements.
 *
 *  @copyright Copyright (C) Lucent Technologies 1997.
 */

#ifndef __LINUX__
#include <stdio.h>

/*!
 *  @brief Runs a command and opens a pipe (Windows implementation).
 *
 *  @param[in] s Command string.
 *  @param[in] m Mode: "r" for read, "w" for write.
 *
 *  @return Pipe stream, or NULL on failure.
 */
FILE *popen(char *s, char *m) {
	return _popen(s, m);	/* return NULL; */
}

/*!
 *  @brief Closes a pipe opened by popen (Windows implementation).
 *
 *  @param[in] f Pipe stream.
 *
 *  @return Exit status of the command.
 */
int pclose(FILE *f) {
	return _pclose(f);	/* return NULL; */
}

#else
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* Prototypes */

/*!
 *  @brief Read end of a pipe.
 *  @def READ
 */
#define READ 0
/*!
 *  @brief Write end of a pipe.
 *  @def WRITE
 */
#define WRITE 1

/*!
 *  @brief Runs a command and opens a pipe (POSIX implementation).
 *
 *  @param[in] s Command string.
 *  @param[in] m Mode: "r" for read, "w" for write.
 *
 *  @return Pipe stream, or NULL on failure.
 */
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


/*!
 *  @brief Closes a pipe opened by popen (POSIX implementation).
 *
 *  @param[in] f Pipe stream.
 *
 *  @return Always 0.
 */
int pclose(FILE *f)
{
  fclose(f);
  return 0;
}
#endif
