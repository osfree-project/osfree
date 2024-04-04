/*!
   $Id: cmd_shared.h,v 1.1.1.1 2003/10/04 08:24:12 prokushev Exp $ 
  
   @file cmd_shared.h 
   
   @brief utility functions header, shared along all command line tools

   (c) osFree Project 2002, <http://www.osFree.org>
   for licence see licence.txt in root directory, or project website

   @author Bartosz Tomasik <bart2@asua.org.pl>
*/

#ifndef _CMD_SHARED_H_
#define _CMD_SHARED_H_

#include "all_shared.h" /* shared functions and defines */

#include "cmd_MessageIDs.h" /* suplementary message id's */
#include "cmd_Messages.h" /* mesage retrieval and displaying */
#include "cmd_QueryCurrentDisk.h"
#include "cmd_QueryFSName.h" 
#include "cmd_ShowVolumeInfo.h"
#include "cmd_ExecFSEntry.h"
#include "cmd_setcurrentdisk.h"

/* return code definitions for command line tools*/
#define cmd_NO_ERROR       0 /*< No error */
#define cmd_FILE_NOT_FOUND 1 /*< File not found */
#define cmd_SOME_LEFT      2 /*< Some files left, due file errors..*/
#define cmd_END_BY_USER    3 /*< Ctrl+C/Break pressed */
#define cmd_ERROR_EXIT     4 /*< Error happened */
#define cmd_OTHER          5 /*< Other - command specific */
#define cmd_EXEC_FS_FAIL   6 /*< Could not execute fs entry */
#define cmd_NOT_SUPPORTED  7 /*< FS entry does not support this function */

#endif /* _CMD_SHARED_H_ */
