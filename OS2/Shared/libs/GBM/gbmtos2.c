/*

gbmtos2.c - Platform specific functionality for file expansion on OS/2.

Author: Heiko Nitzsche

History
-------
26-Apr-2006: Initial version

*/

#define INCL_DOSFILEMGR   /* File Manager values */
#define INCL_DOSERRORS    /* DOS error values */
#include <os2.h>

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
  char * split_filename  = NULL;

  if ((filename == NULL) || (path == NULL))
  {
     return GBM_FALSE;
  }

  split_filename = strdup(filename);
  _splitpath(split_filename, drive, dir, NULL, NULL);
  free(split_filename); split_filename = NULL;
  
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
  HDIR          hdirFindHandle = HDIR_CREATE;
  FILEFINDBUF3  findBuffer     = {0};      /* Returned from FindFirst/Next */
  ULONG         ulResultBufLen = sizeof(FILEFINDBUF3);
  ULONG         ulFindCount    = 1;        /* Look for 1 file at a time    */
  APIRET        rc             = NO_ERROR; /* Return code                  */

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
  pathlength = strlen(path);
#endif

  rc = DosFindFirst((char *)filename, /* File pattern              */
                    &hdirFindHandle,  /* Directory search handle   */
                    FILE_NORMAL,      /* Search attribute          */
                    &findBuffer,      /* Result buffer             */
                    ulResultBufLen,   /* Result buffer length      */
                    &ulFindCount,     /* Number of entries to find */
                    FIL_STANDARD);    /* Return Level 1 file info  */
  if ((rc != NO_ERROR) || (ulFindCount != 1))
  {
    DosFindClose(hdirFindHandle);
    free(path);
    return GBM_FALSE;
  }

  /* add first found filename */
  assert(pathlength + strlen(findBuffer.achName) + 1 <= sizeof(buffer));
  sprintf(buffer, "%s%s", path, findBuffer.achName);

  *filecount = 0;
  *files = gbmtool_createFileNode(buffer);
  if (*files == NULL)
  {
    DosFindClose(hdirFindHandle);
    free(path);
    return GBM_FALSE;
  }
  *filecount += ulFindCount;

  firstNode   = *files;
  currentNode = firstNode;

  /* Keep finding the next file until there are no more files */
  while (rc != ERROR_NO_MORE_FILES)
  {
    ulFindCount = 1;                  /* Reset find count.         */

    rc = DosFindNext(hdirFindHandle,  /* Directory handle          */
                     &findBuffer,     /* Result buffer             */
                     ulResultBufLen,  /* Result buffer length      */
                     &ulFindCount);   /* Number of entries to find */

     if ((rc != NO_ERROR && rc != ERROR_NO_MORE_FILES) || (ulFindCount > 1))
     {
       unsigned int count;
       gbmtool_free_all_subnodes(firstNode, &count);
       *filecount -= count;
       gbmtool_free_node(firstNode);
       (*filecount)--;
       DosFindClose(hdirFindHandle);
       free(path);
       return GBM_FALSE;
     }

     if ((rc != ERROR_NO_MORE_FILES) && (ulFindCount == 1))
     {
       /* add found filename */
       assert(pathlength + strlen(findBuffer.achName) + 1 <= sizeof(buffer));
       sprintf(buffer, "%s%s", path, findBuffer.achName);

       currentNode->next = gbmtool_createFileNode(buffer);
       if (currentNode->next == NULL)
       {
         unsigned int count;
         gbmtool_free_all_subnodes(firstNode, &count);
         *filecount -= count;
         gbmtool_free_node(firstNode);
         (*filecount)--;
         DosFindClose(hdirFindHandle);
         free(path);
         return GBM_FALSE;
       }
       *filecount += ulFindCount;
       currentNode = currentNode->next;
     }
  } /* endwhile */

  DosFindClose(hdirFindHandle); /* Close our directory handle */
  free(path);

  return GBM_TRUE;
}

#endif /* FILENAME_EXPANSION_MODE */

