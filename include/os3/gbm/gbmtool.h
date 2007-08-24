/**
 * gbmtool.h - Command line parser with support for regular
 *             expressions. Splits filename and options.
 *
 */

#ifndef _GBMTOOL_H_
#define _GBMTOOL_H_

#include <stdlib.h>

#ifdef __cplusplus
  extern "C"
  {
#endif

/**
 * Maximum filename length that will be parsed.
 * This does not yet include the \0 character !
 */
#if (defined(_MAX_DRIVE) && defined(_MAX_DIR) && defined(_MAX_FNAME) && defined(_MAX_EXT))
  #define GBMTOOL_FILENAME_MAX (_MAX_DRIVE+_MAX_DIR+_MAX_FNAME+_MAX_EXT)
#else
  #define GBMTOOL_FILENAME_MAX  800
#endif

/**
 * Maximum options string length that will be parsed.
 * This does not yet include the \0 character !
 */
#define GBMTOOL_OPTIONS_MAX  800

/**
 * Describes a chained list of filenames.
 */
typedef struct GBMTOOL_FILE_
{
  char                 * filename; /* A fully expanded filename. */
  struct GBMTOOL_FILE_ * next;     /* Pointer to the next filename or NULL. */

} GBMTOOL_FILE;


/**
 * Describes a command line argument.
 */
typedef struct
{
  const char   * argin;       /** \0 terminated string containing "filename,options".
                                  Note: The filename should be enclosed by " " ! */

  GBMTOOL_FILE * files;       /** Chained list of expanded filenames or NULL. */
  unsigned int   filecount;   /** Number of expanded filenames that can be addressed with filename[]. */

  char         * options;     /** options part. If no options found this contains "". */

} GBMTOOL_FILEARG;


#if defined(__OS2__) || defined(OS2)
 #ifndef FILENAME_EXPANSION_MODE
  #define FILENAME_EXPANSION_MODE
 #endif

 /* Extension point for platform dependend implementation. */
 /* Just compile and link with the appropriate implementation file gbmtos2.c */
 BOOLEAN gbmtool_findFiles(const char * filename, GBMTOOL_FILE ** files, unsigned int * filecount);

#else

 /* Extension point for platform dependend implementation. */
 /* Add the appropriate platform definition here. */
 #ifdef FILENAME_EXPANSION_MODE
  #undef FILENAME_EXPANSION_MODE
 #endif
#endif


/**
 * Create a new GBMTOOL_FILE node and init with
 * specified values.
 *
 * @param filename  The filename (will be copied) or NULL.
 *
 * @return The new node or NULL in case of memory error.
 */
GBMTOOL_FILE * gbmtool_createFileNode(const char * filename);


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
 * @retval GBM_ERR_BAD_ARG  Argument could not be parsed.
 */
GBM_ERR gbmtool_parse_argument(GBMTOOL_FILEARG * gbmfilearg, const BOOLEAN expandRegEx);


/**
 *  Frees file arguments.
 *
 * @param gbmfilearg  Struct that initially has been allocated by
 *                    the function gbmtool_parse_argument.
 */
void gbmtool_free_argument(GBMTOOL_FILEARG * gbmfilearg);



/** Free specified node only. */
void gbmtool_free_node(GBMTOOL_FILE * node);

/** Free subnodes. Returns the number of removed nodes. */
void gbmtool_free_all_subnodes(GBMTOOL_FILE * firstNode, unsigned int * filecount);


#ifdef __cplusplus
  }  /* extern "C" */
#endif

#endif /* _GBMTOOL_H_ */

