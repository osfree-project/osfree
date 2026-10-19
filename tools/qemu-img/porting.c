/*!
 * @file porting.c
 * @brief Portability shims for non-POSIX hosts.
 *
 * Provides replacement implementations for functions that are available on
 * POSIX systems but missing or differently named on the target host.
 */

#include <stdlib.h>
#ifndef LINUX
#include <io.h> 
#else
#include <unistd.h>
#endif


/*!
 * @brief Truncate or extend a file to a specified length.
 * @param[in] handle File descriptor.
 * @param[in] size New file size in bytes.
 * @retval -1 Failure.
 * @retval 0 Success.
 * @return 0 on success, -1 on failure.
 */
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

/*!
 * @brief Resolve a pathname to an absolute path.
 * @param[in] path Input pathname.
 * @param[out] resolved_path Destination buffer for the resolved path.
 * @return Pointer to @p resolved_path.
 */
char *realpath(const char *path, char *resolved_path)
{
    _fullpath(resolved_path, path, _MAX_PATH);
    return resolved_path;
}
