/*!
    @file cmd_MessageIDs.h

    @brief Supplementary defines for messages identification,
    shared along all command line tools

    (c) osFree Project 2002-2008, <http://www.osFree.org>
    for licence see licence.txt in root directory, or project website

    @author Bartosz Tomasik <bart2@asua.org.pl>
    @author Cristiano Guadagnino <criguada@tin.it>

    @todo remove part or whole of this file because duplicates OS/2 include file bseerr and bsemid
*/

#ifndef _CMD_MESSAGEIDS_H_
#define _CMD_MESSAGEIDS_H_

#define cmd_MSG_DEVICE_CMD_UNKNOWN        22  /*! The device does not recognize the command.*/
#define cmd_MSG_PAUSE_WAITING           1032  /*!< Press any key to continue... */
#define cmd_MSG_VERIFY_OFF              1076  /*!< VERIFY is off.  */
#define cmd_MSG_VERIFY_ON               1077  /*!< VERIFY is on. */

#define cmd_MSG_OSVERSION               1090  /*!< The Operating System/2 Version is %1.%2 %0 */
#define cmd_MSG_OSREVISION              1119  /*!< Revision %1.%2 %0 */

#define cmd_MSG_VOLUME_SERIAL_IS        1243  /*!< The Volume Serial Number is %1. */
#define cmd_MSG_ALREADYEXIST            1248  /*!< a subdirectory or file %0 already exists */
#define cmd_MSG_ERROR_WHILE_PROCESSING  1249  /*!< Error occured while processing %1 */
#define cmd_MSG_ENTER_NEW_LABEL         1515  /*!< Enter up to 11 characters for the volume label or press Enter for no volume label update. %0 */

#define cmd_MSG_FSUTIL_HAS_STARTED      1506  /*!< The %1 file system program has been started. */
#define cmd_MSG_TYPE_OF_FS_IS           1507  /*!< The type of file system for the disk is %1. */
#define cmd_MSG_VOLUME_HAS_NO_LABEL     1514  /*!< Volume in drive %1 has no label. */
#define cmd_MSG_VOLUME_LABEL_IS         1516  /*!< The volume label in drive %1 is %2.*/
#define cmd_MSG_CURRENT_HARD_DISK_IS    1568  /*!< The current hard disk drive is: %1 */

#define cmd_MSG_ANSI_EXT_SCR_KEY_ON     1705  /*!< ANSI is on */
#define cmd_MSG_ANSI_EXT_SCR_KEY_OFF    1706  /*!< ANSI is off */

/* help screens */
#define cmd_MSG_LABEL_HELP              3037  /*!< Help screen for label */
#define cmd_MSG_DIR_HELP                3039  /*!< Help screen for dir */
#define cmd_MSG_CALL_HELP               3040  /*!< Help screen for call */
#define cmd_MSG_CHCP_HELP               3041  /*!< Help screen for chcp */
#define cmd_MSG_RENAME_HELP             3042  /*!< Help screen for rename */
#define cmd_MSG_DEL_HELP                3043  /*!< Help screen for del */
#define cmd_MSG_TYPE_HELP               3044  /*!< Help screen for type */
#define cmd_MSG_REM_HELP                3045  /*!< Help screen for rem */
#define cmd_MSG_COPY_HELP               3046  /*!< Help screen for copy */
#define cmd_MSG_PAUSE_HELP              3047  /*!< Help screen for pause */
#define cmd_MSG_DATE_HELP               3048  /*!< Help screen for date */
#define cmd_MSG_TIME_HELP               3049  /*!< Help screen for time */
#define cmd_MSG_VER_HELP                3050  /*!< Help screen for ver */
#define cmd_MSG_VOL_HELP                3051  /*!< Help screen for vol */
#define cmd_MSG_CHDIR_HELP              3052  /*!< Help screen for chdir/cd */
#define cmd_MSG_MKDIR_HELP              3053  /*!< Help screen for mkdir/md */
#define cmd_MSG_RMDIR_HELP              3054  /*!< Help screen for rmdir/rd */
#define cmd_MSG_VERIFY_HELP             3056  /*!< Help screen for verify */
#define cmd_MSG_SET_HELP                3057  /*!< Help screen for set */
#define cmd_MSG_PROMPT_HELP             3058  /*!< Help screen for prompt */
#define cmd_MSG_PATH_HELP               3059  /*!< Help screen for path */
#define cmd_MSG_EXIT_HELP               3060  /*!< Help screen for exit */
#define cmd_MSG_ECHO_HELP               3062  /*!< Help screen for echo */
#define cmd_MSG_GOTO_HELP               3063  /*!< Help screen for goto */
#define cmd_MSG_SHIFT_HELP              3064  /*!< Help screen for shift */
#define cmd_MSG_IF_HELP                 3065  /*!< Help screen for if */
#define cmd_MSG_FOR_HELP                3066  /*!< Help screen for for */
#define cmd_MSG_CLS_HELP                3067  /*!< Help screen for cls */
#define cmd_MSG_ATTRIB_HELP             3092  /*!< Help screen for attrib */
#define cmd_MSG_CHKDSK_HELP             3093  /*!< Help screen for chkdsk */
#define cmd_MSG_COMP_HELP               3094  /*!< Help screen for comp */
#define cmd_MSG_DISKCOPY_HELP           3096L /*!< Help screen for diskcopy */
#define cmd_MSG_MOVE_HELP               3098  /*!< Help screen for move */
#define cmd_MSG_FIND_HELP               3099L /*!< Help screen for find */
#define cmd_MSG_FORMAT_HELP             3120  /*!< Help screen for format */
#define cmd_MSG_MODE_HELP               3133L /*!< Help screen for mode */
#define cmd_MSG_PRINT_HELP              3136L /*!< Help screen for print */
#define cmd_MSG_RECOVER_HELP            3137L /*!< Help screen for recover */
#define cmd_MSG_SORT_HELP               3139L /*!< Help screen for recover */
#define cmd_MSG_TREE_HELP               3148L /*!< Help screen for tree */
#define cmd_MSG_UNDELETE_HELP           3149L /*!< Help screen for undelete */
#define cmd_MSG_KEYB_HELP               8080L /*!< Help screen for keyb */
#define cmd_MSG_ANSI_HELP               8081L /*!< Help screen for ansi */

#define cmd_MSG_TREE_START              1479L /*!< Directory path listing */
#define cmd_MSG_TREE_PATH               1480L /*!< Path:           %1 */
#define cmd_MSG_TREE_SUBDIRS            1481L /*!< Subdirectories: %1 */
#define cmd_MSG_TREE_FILES              1482L /*!< Files:          %1 */
#define cmd_MSG_TREE_NOFILES            1483L /*!< Files:          None */
#define cmd_MSG_TREE_NOSUBDIRS          1484L /*!< Subdirectories: None */
#define cmd_MSG_TREE_SPACER             1485L /*!<                  %1 */
#define cmd_MSG_TREE_NODIRS             1486L /*!< No subdirectories exist. */

#endif /* _CMD_MESSAGEIDS_H_ */

