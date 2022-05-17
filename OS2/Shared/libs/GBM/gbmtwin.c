/*

gbmtwin.c - Platform specific functionality for file expansion on Windows 32/64bit.

Author: Heiko Nitzsche

History
-------
26-Aug-2008: Initial version

*/

#include <windows.h>

#include <assert.h>
#include <limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gbm.h"
#include "gbmtool.h"


#ifdef FILENAME_EXPANSION_MODE

/************************************************************/

/**
 * Extracts the path (including drive) from the specified file name.
 *
 * @param filename    Filename (can contain regular expressions)
 *
 * @param path        Pointer to character array that will on
 *                    successful return contain the full path.
 *                    The caller is responsible to free the
 *                    allocated buffer.
 *
 * @retval GBM_TRUE   Success.
 * @retval GBM_FALSE  An error occured.
 */
static gbm_boolean getPathFromFullFilename(const char * filename, char ** path)
{
  char drive[_MAX_DRIVE] = { 0 };
  char dir[_MAX_DIR]     = { 0 };
  char split_filename[_MAX_PATH+1] = { 0 };

  if ((filename == NULL) || (path == NULL))
  {
     return GBM_FALSE;
  }

  strcpy(split_filename, filename);
  _splitpath(split_filename, drive, dir, NULL, NULL);
  
  *path = (char *) malloc(strlen(drive) + strlen(dir) + 1);
  strcpy(*path, drive);
  strcat(*path, dir);

  return GBM_TRUE;
}

/************************************************************/

/**
 * Implements extension for resolving filename with regular expressions.
 *
 * @param filename    Filename (can contain regular expressions)
 *
 * @param filearray   Resolved filename array, will be allocated.
 *                    For the number of entries see filearray_length.
 *                    The client has to take care of freeing it with
 *                    the C library free() function.
 *
 * @param filearray_length  Number of filenames contained in filearray.
 *
 * @retval GBM_TRUE   Success.
 * @retval GBM_FALSE  An error occured.
 */
gbm_boolean gbmtool_findFiles(const char * filename, GBMTOOL_FILE ** files, unsigned int * filecount)
{
  HANDLE          hFindHandle = INVALID_HANDLE_VALUE;
  WIN32_FIND_DATA findFileData; /* Returned from FindFirst/Next */

  char   buffer[1025] = { 0 };
  char * path = NULL;
#ifndef NDEBUG
  int    pathlength = 0;
#endif

  GBMTOOL_FILE * firstNode   = NULL;
  GBMTOOL_FILE * currentNode = NULL;

  *filecount = 0;
  *files     = NULL;

  if (! getPathFromFullFilename(filename, &path))
  {
    return GBM_FALSE;
  }
#ifndef NDEBUG
  pathlength = (int)strlen(path);
#endif

  hFindHandle = FindFirstFile(filename, &findFileData);
  if (hFindHandle == INVALID_HANDLE_VALUE) 
  {
    free(path);
    return GBM_FALSE;
  }
  do
  {
    if ((findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
    {
       assert(pathlength + strlen(findFileData.cFileName) + 1 <= sizeof(buffer));
       sprintf(buffer, "%s%s", path, findFileData.cFileName);
     
       if (firstNode == NULL)
       {
         /* add first found filename */
         *filecount = 0;
         *files = gbmtool_createFileNode(buffer);
         if (*files == NULL)
         {
           FindClose(hFindHandle);
           free(path);
           return GBM_FALSE;
         }
         (*filecount)++;
         firstNode   = *files;
         currentNode = firstNode;
       }
       else
       {
         /* add found filename */
         assert(currentNode != NULL);
         currentNode->next = gbmtool_createFileNode(buffer);
         if (currentNode->next == NULL)
         {
           unsigned int count;
           gbmtool_free_all_subnodes(firstNode, &count);
           *filecount -= count;
           gbmtool_free_node(firstNode);
           (*filecount)--;
           FindClose(hFindHandle);
           free(path);
           return GBM_FALSE;
         }
         (*filecount)++;
         currentNode = currentNode->next;
       }
    }
  }
  while (FindNextFile(hFindHandle, &findFileData));
  
  FindClose(hFindHandle);
  free(path);

  return GBM_TRUE;
}

#endif /* FILENAME_EXPANSION_MODE */

