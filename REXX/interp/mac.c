#if defined(MAC)

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "mac.h"

int stat( char *fn, struct stat *buf )
{
   buf = buf; /* keep compiler happy */
   fprintf(stderr,"trying to stat(): %s\n", fn );
   return 0;
}

int fstat( int fd, struct stat *buf )
{
   buf = buf; /* keep compiler happy */
   fprintf(stderr,"trying to stat(): %d\n", fd );
   return 0;
}

int sleep( int sec )
{
   fprintf(stderr,"trying to sleep(): %d\n", sec );
   return 0;
}

char * getcwd( char *buf, size_t size )
{
   size = size; /* keep compiler happy */
   fprintf(stderr,"trying to getcwd()\n" );
   strcpy( buf, "junk" );
   return buf;
}

int chdir( char *buf )
{
   fprintf(stderr,"trying to chdir(): %buf\n", buf );
   return 0;
}

int isatty( int fd )
{
   fd = fd; /* keep compiler happy */
   return 0;
}

#endif
