/*!

   @file all_performreqursiveaction.h

   @brief directory-tree recurse helper function header
   shared along all code

   (c) osFree Project 2002, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Bartosz Tomasik <bart2@asua.org.pl>
*/

#ifndef _ALL_PERFORMRECURSIVEACTION_H_
#define _ALL_PERFORMRECURSIVEACTION_H_

/* all_PerformRecursiveAction return codes */
#define all_ERR_RECURSE_ACTIONBREAK 1 //!< execution broke by action callback
#define all_ERR_RECURSE_ERRORBREAK  2 //!< execution broke by error callback
#define all_ERR_RECURSE_BADPARAMS   3 //!< invalid parameters were passed
#define all_ERR_RECURSE_NORECURSION 4 //!< no subdirectories for recursion found

/* all_PerformRecursiveAction action bits (also callback, action codes) */
#define all_RECURSE_DIRS       0x001 /*!< recurse into subdirs  */
#define all_RECURSE_FILEACTION 0x002 /*!< perform action on files */
#define all_RECURSE_DIRACTION  0x004 /*!< perform action on dirs */
#define all_RECURSE_IN         0x008 /*!< recursing into subdirectory (callback action code only)*/
#define all_RECURSE_NOFILES    0x010 /*!< no files to perform action (callback action code only)*/
#define all_RECURSE_NODIRS     0x020 /*!< no directories to perform action (callback action code only) */
#define all_RECURSE_NOSUBDIRS  0x040 /*!< no directories to recurse (callback action code) */

int all_PerformRecursiveAction(char *fileMask,int file_options,int fileAttrs,
            int (*action_callback)(char*,char *,int,void *),void *action_callback_data,
            int (*error_callback)(ULONG,void *),void *error_callback_data);

#endif /* _ALL_PERFORMRECURSIVEACTION_H_ */
