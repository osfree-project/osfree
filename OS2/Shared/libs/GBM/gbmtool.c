/*

gbmtool.c - Command line parser with support for regular
            expressions. Splits filename and options.
            Need platform specific plugin.

Author: Heiko Nitzsche

History
-------
26-Apr-2006: Initial version

06-May-2006: * Check additionally fÅr " in the parsed filename
               which might illegally appear if an incorrept bracketed
               argin is specified (e.g. user provided "\"file.ext\")
             * Fix a possible memory leak in error case of parsing

15-Aug-2008: Integrate new GBM types
*/

#include <assert.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gbm.h"
#include "gbmtool.h"


/**
 * Create a new GBMTOOL_FILE node and init with
 * specified values.
 *
 * @param filename  The filename (will be copied) or NULL.
 *
 * @return The new node or NULL in case of memory error.
 */
GBMTOOL_FILE * gbmtool_createFileNode(const char * filename)
{
  GBMTOOL_FILE * file = (GBMTOOL_FILE *) malloc(sizeof(GBMTOOL_FILE));

  if (file == NULL)
  {
    return NULL;
  }

  file->filename = NULL;
  file->next     = NULL;

  if (filename != NULL)
  {
    file->filename = (char *) malloc(strlen(filename) + 1);
    if (file->filename == NULL)
    {
      free(file);
      return NULL;
    }
    strcpy(file->filename, filename);
  }

  return file;
}

/********************************************************/

/**
 * Fill parsed parameters into fileargs struct and expand regular expression
 * based filename if requested.
 *
 * @param gbmfilearg  Struct that initially has the argin parameter set and
 *                    will be completed by the function.
 *
 * @param filename    Filename array (can contain regular expressions)
 *
 * @param options     Options (optional if NULL or "" is provided)
 *
 * @param expandRegEx The filename can contain regular expression that are
 *                    supported by the platform file system, for instance * or ?.
 *
 * @retval GBM_ERR_OK       Argument successfully parsed.
 * @retval GBM_ERR_MEM      Out of memory.
 * @retval GBM_ERR_BAD_ARG  Argument could not be parsed.
 */
static GBM_ERR fill_arguments(GBMTOOL_FILEARG   * gbmfilearg,
                              const char        * filename,
                              const char        * options,
                              const gbm_boolean   expandRegEx)
{
  gbmfilearg->files     = NULL;
  gbmfilearg->filecount = 0;
  gbmfilearg->options   = NULL;

  if (filename == NULL)
  {
    return GBM_ERR_BAD_ARG;
  }

  /* check for illegal " in the filename */
  if (strchr(filename, '"') != NULL)
  {
    return GBM_ERR_BAD_ARG;
  }

  /* allocate options */
  if (options != NULL)
  {
    gbmfilearg->options = (char *) malloc(strlen(options) + 1);
  }
  else
  {
    gbmfilearg->options = (char *) malloc(1);
  }
  if (gbmfilearg->options == NULL)
  {
    return GBM_ERR_MEM;
  }

  /* fill in options */
  strcpy(gbmfilearg->options, (options != NULL) ? options : "");

  /* check for illegal " in the options */
  if (strchr(gbmfilearg->options, '"') != NULL)
  {
    free(gbmfilearg->options);
    gbmfilearg->options = NULL;
    return GBM_ERR_BAD_ARG;
  }

  /* expand filename template */
  if (expandRegEx)
  {
   #ifndef FILENAME_EXPANSION_MODE
     free(gbmfilearg->options);
     gbmfilearg->options = NULL;
     return GBM_ERR_BAD_ARG;
   #else

     /* dispatch to platform specific handlers (must use C library memory management) */
     if (! gbmtool_findFiles(filename, &gbmfilearg->files, &gbmfilearg->filecount))
     {
       gbmfilearg->filecount = 0;
       free(gbmfilearg->options);
       gbmfilearg->options = NULL;
       return GBM_ERR_BAD_ARG;
     }

   #endif /* FILENAME_EXPANSION_MODE */
  }
  else
  {
    /* no filename template expansion */
    gbmfilearg->files = gbmtool_createFileNode(filename);
    if (gbmfilearg->files == NULL)
    {
      free(gbmfilearg->options);
      gbmfilearg->options = NULL;
      return GBM_ERR_MEM;
    }
    gbmfilearg->filecount = 1;
  }

  return GBM_ERR_OK;
}

/**
 * Parse an argument consisting of filename template and assigned options.
 *
 * The filename should be enclosed by "" and the options are attached by comma.
 * Argument syntax: "filename template",options
 *
 * Filename templates without quotes are supported as well, but if
 * the filename contains a comma, parsing the options part is not
 * correctly possible as options are separated by comma as well.
 *
 * @param gbmfilearg  Struct that initially has the argin parameter set and
 *                    will be completed by the function.
 *
 * @param expandRegEx The filename can contain regular expression that are
 *                    supported by the platform file system, for instance * or ?.
 *
 * @retval GBM_ERR_OK       Argument successfully parsed.
 * @retval GBM_ERR_MEM      Out of memory.
 * @retval GBM_ERR_BAD_ARG  Argument could not be parsed.
 */
GBM_ERR gbmtool_parse_argument(GBMTOOL_FILEARG * gbmfilearg, const gbm_boolean expandRegEx)
{
  GBM_ERR rc;
  int     num;
  char    filename[GBMTOOL_FILENAME_MAX+1] = { 0 };
  char    options [GBMTOOL_OPTIONS_MAX+1]  = { 0 };
  char    buffer[100];

  gbmfilearg->files     = NULL;
  gbmfilearg->filecount = 0;
  gbmfilearg->options   = NULL;

  if (gbmfilearg->argin == NULL)
  {
    return GBM_ERR_BAD_ARG;
  }

  sprintf(buffer, "%%*[\"]%%%d[^\"]\"%%%d[^\n]", GBMTOOL_FILENAME_MAX, GBMTOOL_OPTIONS_MAX);
  num = sscanf(gbmfilearg->argin, buffer, filename, options);
  if (num == 2)
  {
    if (strchr(options, '"') != NULL)
    {
      return GBM_ERR_BAD_ARG;
    }
  }
  strcpy(filename, "");
  strcpy(options , "");

  sprintf(buffer, "%%*[\"]%%%d[^\"]%%*[^,],%%%d[^\n]", GBMTOOL_FILENAME_MAX, GBMTOOL_OPTIONS_MAX);
  num = sscanf(gbmfilearg->argin, buffer, filename, options);

  /* check for error */
  if ((num <= 0) || (num > 2))
  {
    strcpy(filename, "");
    strcpy(options , "");

    /* rescan for: filename,options */
    sprintf(buffer, "%%%d[^,],%%%d[^\n]", GBMTOOL_FILENAME_MAX, GBMTOOL_OPTIONS_MAX);
    num = sscanf(gbmfilearg->argin, buffer, filename, options);

    if ((num < 0) || (num > 2))
    {
      return GBM_ERR_BAD_ARG;
    }
  }

  if ((num > 0) && (num <=2))
  {
    /* potentially expand and fill arguments */
    if (num == 1)
    {
      rc = fill_arguments(gbmfilearg, filename, "", expandRegEx);
    }
    else
    {
      rc = fill_arguments(gbmfilearg, filename, options, expandRegEx);
    }
    if (rc != GBM_ERR_OK)
    {
      return rc;
    }
    return GBM_ERR_OK;
  }

  return GBM_ERR_BAD_ARG;
}

/********************************************************/

/** Free specified node only. */
void gbmtool_free_node(GBMTOOL_FILE * node)
{
  if (node->filename != NULL)
  {
    free(node->filename);
    node->filename = NULL;
  }
  free(node);
}

/** Free subnodes. */
void gbmtool_free_all_subnodes(GBMTOOL_FILE * firstNode, unsigned int * filecount)
{
  GBMTOOL_FILE * currentNode = firstNode->next;
  GBMTOOL_FILE * nextNode    = NULL;

  *filecount = 0;
  while(currentNode != NULL)
  {
    nextNode = currentNode->next;
    gbmtool_free_node(currentNode);
    *filecount += 1;
    currentNode = nextNode;
  }
}

/********************************************************/

/**
 *  Frees file arguments.
 *
 * @param gbmfilearg  Struct that initially has been allocated by
 *                    the function gbmtool_parse_argument.
 */
void gbmtool_free_argument(GBMTOOL_FILEARG * gbmfilearg)
{
  /* Don't free argin as this is a const pointer only ! */

  /* free dynamically allocated elements */
  if (gbmfilearg->files != NULL)
  {
    unsigned int count;
    gbmtool_free_all_subnodes(gbmfilearg->files, &count);
    gbmfilearg->filecount -= count;
    gbmtool_free_node(gbmfilearg->files);
    gbmfilearg->filecount -= 1;
    gbmfilearg->files = NULL;
  }
  assert(gbmfilearg->filecount == 0);

  if (gbmfilearg->options != NULL)
  {
    free(gbmfilearg->options);
    gbmfilearg->options = NULL;
  }
}

