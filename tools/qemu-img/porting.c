#include <stdlib.h>
#ifndef LINUX
#include <io.h> 
#else
#include <unistd.h>
#endif


int ftruncate( int handle, long size )
{  
   int rc = 0;
   if (size < filelength(handle)) {
      rc = chsize(handle, size);
   }
   
   if (rc) { 
     rc = -1; 
   }

   return rc;
}

char *realpath(const char *path, char *resolved_path)
{
    _fullpath(resolved_path, path, _MAX_PATH);
    return resolved_path;
}

