#include <stdlib.h>
#include <io.h> 


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

