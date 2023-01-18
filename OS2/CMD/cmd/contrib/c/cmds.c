// CMDS.C - Table of internal commands for 4os2
//   (c) 1988 - 1997  Rex C. Conn  All rights reserved

#include "product.h"

#include <stdio.h>
#include <string.h>

#include "4all.h"

#include <cmd_shared.h>

// Structure for the internal commands
// The args are:
//  command name
//  pointer to function
//  enabled/disabled flag (see SETDOS /I)
//  parsing flag, where bit flags are:
//      CMD_EXPAND_VARS - expand variables
//      CMD_EXPAND_REDIR - do redirection
//      CMD_STRIP_QUOTES - strip quoting
//          CMD_ADD_NULLS - add terminators to each argument
//          CMD_GROUPS - check for command groups
//      CMD_CLOSE_BATCH - close batch file before executing command
//      CMD_ONLY_BATCH - command only allowed in a batch file
//      CMD_DISABLED - command disabled (SETDOS /I-)
//      CMD_RESET_DISKS - reset disks upon return
//      CMD_SET_ERRORLEVEL - set ERRORLEVEL upon return (OS/2 only)
//      CMD_BACKSLASH_OK - allow trailing backslash
//      CMD_DETACH_LINE - send entire line to DETACH command

BUILTIN commands[] = {
    "?", cmds_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES), 8000,
    "ACTIVATE", activate_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_ADD_NULLS), 8001,
    "ALIAS", set_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_CLOSE_BATCH), 8002,
    "BEEP", beep_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_ADD_NULLS), 8003,
    "CALL", call_cmd, (CMD_CLOSE_BATCH | CMD_BACKSLASH_OK), cmd_MSG_CALL_HELP,
    "CANCEL", quit_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_ONLY_BATCH), 8004,
    "CASE",  case_cmd, CMD_ONLY_BATCH, 8005,
    "CD", cd_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_SET_ERRORLEVEL | CMD_BACKSLASH_OK), cmd_MSG_CHDIR_HELP,
    "CDD", cdd_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_SET_ERRORLEVEL | CMD_BACKSLASH_OK), 8006,
    "CHCP", chcp_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_ADD_NULLS | CMD_SET_ERRORLEVEL), cmd_MSG_CHCP_HELP,
    "CHDIR", cd_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_SET_ERRORLEVEL | CMD_BACKSLASH_OK), cmd_MSG_CHDIR_HELP,
    "COLOR", color_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES), 8007,
    "COPY", copy_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_CLOSE_BATCH | CMD_RESET_DISKS | CMD_SET_ERRORLEVEL | CMD_BACKSLASH_OK), cmd_MSG_COPY_HELP,
    "DATE", setdate_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_SET_ERRORLEVEL), cmd_MSG_DATE_HELP,
    "DEFAULT",  case_cmd, CMD_ONLY_BATCH, 8008,
    "DEL", del_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_CLOSE_BATCH | CMD_RESET_DISKS | CMD_SET_ERRORLEVEL | CMD_BACKSLASH_OK), cmd_MSG_DEL_HELP,
    "DELAY", delay_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_ADD_NULLS), 8009,
    "DESCRIBE", describe_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_CLOSE_BATCH | CMD_RESET_DISKS), 8010,
    "DETACH", detach_cmd, (CMD_STRIP_QUOTES | CMD_SET_ERRORLEVEL | CMD_DETACH_LINE | CMD_CLOSE_BATCH), 8011,
    "DIR", dir_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_SET_ERRORLEVEL | CMD_BACKSLASH_OK), cmd_MSG_DIR_HELP,
    "DIRHISTORY", history_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_CLOSE_BATCH), 8012,
    "DIRS", dirs_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES), 8013,
    "DO", do_cmd, (CMD_GROUPS | CMD_STRIP_QUOTES | CMD_ONLY_BATCH), 8014,
    "DPATH", dpath_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_BACKSLASH_OK), 8015,
    "DRAWBOX", drawbox_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES), 8016,
    "DRAWHLINE", drawline_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES), 8017,
    "DRAWVLINE", drawline_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES), 8018,
    "ECHO", echo_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_BACKSLASH_OK), cmd_MSG_ECHO_HELP,
    "ECHOERR", echo_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_BACKSLASH_OK), 8019,
    "ECHOS", echos_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES), 8020,
    "ECHOSERR", echos_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES), 8021,
    "ENDLOCAL", endlocal_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_ONLY_BATCH), 8022,
    "ENDSWITCH",  remark_cmd, CMD_ONLY_BATCH, 8023,
    "ERASE", del_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_CLOSE_BATCH | CMD_RESET_DISKS | CMD_SET_ERRORLEVEL | CMD_BACKSLASH_OK), cmd_MSG_DEL_HELP,
    "ESET", eset_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES), 8024,
    "EXCEPT", except_cmd, CMD_GROUPS, 8025,
    "EXIT", exit_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES), cmd_MSG_EXIT_HELP,
    "FFIND", ffind_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES), 8026,
    "FOR", for_cmd, (CMD_GROUPS | CMD_CLOSE_BATCH), cmd_MSG_FOR_HELP,
    "FREE", df_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_ADD_NULLS), 8027,
    "GLOBAL", global_cmd,CMD_GROUPS, 8028,
    "GOSUB", gosub_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_ADD_NULLS | CMD_ONLY_BATCH), 8029,
    "GOTO", goto_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_ONLY_BATCH), cmd_MSG_GOTO_HELP,
    "HELP", help_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES), 8030,
    "HISTORY", history_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_CLOSE_BATCH), 8031,
    "IF", if_cmd, CMD_GROUPS, cmd_MSG_IF_HELP,
    "IFF", if_cmd, CMD_GROUPS, 8032,
    "INKEY", inkey_input_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES), 8033,
    "INPUT", inkey_input_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES), 8034,
    "KEYBD", keybd_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_ADD_NULLS), 8035,
    "KEYS", keys_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_ADD_NULLS), 8036,
    "KEYSTACK", keystack_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_CLOSE_BATCH), 8037,
    "LIST", list_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES), 8038,
    "LOADBTM", loadbtm_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_ADD_NULLS | CMD_ONLY_BATCH), 8039,
    "LOG", log_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES), 8040,
    "MEMORY", memory_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES), 8041,
    "MOVE", mv_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_CLOSE_BATCH | CMD_RESET_DISKS | CMD_SET_ERRORLEVEL), cmd_MSG_MOVE_HELP,
    "ON", on_cmd, (CMD_ONLY_BATCH | CMD_GROUPS), 8042,
    "OPTION", option_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES ), 8043,
    "PATH", path_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_BACKSLASH_OK), cmd_MSG_PATH_HELP,
    "PAUSE", pause_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_CLOSE_BATCH), cmd_MSG_PAUSE_HELP,
    "POPD", popd_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES), 8044,
    "PROMPT", prompt_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_BACKSLASH_OK), cmd_MSG_PROMPT_HELP,
    "PUSHD", pushd_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_ADD_NULLS | CMD_BACKSLASH_OK), 8045,
    "QUIT", quit_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES), 8046,
    "RD", rd_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_RESET_DISKS | CMD_SET_ERRORLEVEL | CMD_BACKSLASH_OK), cmd_MSG_RMDIR_HELP,
    "REBOOT", reboot_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES), 8047,
    "REM", remark_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_BACKSLASH_OK), cmd_MSG_REM_HELP,
    "REN", ren_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_CLOSE_BATCH | CMD_RESET_DISKS | CMD_SET_ERRORLEVEL | CMD_BACKSLASH_OK), cmd_MSG_RENAME_HELP,
    "RENAME", ren_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_CLOSE_BATCH | CMD_RESET_DISKS | CMD_SET_ERRORLEVEL | CMD_BACKSLASH_OK), cmd_MSG_RENAME_HELP,
    "RETURN", ret_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_ONLY_BATCH), 8048,
    "RMDIR", rd_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_RESET_DISKS | CMD_SET_ERRORLEVEL | CMD_BACKSLASH_OK), cmd_MSG_RMDIR_HELP,
    "SCREEN", scr_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES), 8049,
    "SCRPUT", scrput_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR), 8050,
    "SELECT", select_cmd,CMD_GROUPS, 8051,
    "SET", set_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_SET_ERRORLEVEL), cmd_MSG_SET_HELP,
    "SETDOS", setdos_cmd, (CMD_EXPAND_VARS | CMD_STRIP_QUOTES | CMD_EXPAND_REDIR), 8052,
    "SETLOCAL", setlocal_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_ONLY_BATCH), 8053,
    "SHIFT", shift_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_ONLY_BATCH), cmd_MSG_SHIFT_HELP,
    "SHRALIAS", shralias_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_SET_ERRORLEVEL | CMD_CLOSE_BATCH), 8054,
    "START", start_cmd, (CMD_EXPAND_VARS | CMD_STRIP_QUOTES | CMD_SET_ERRORLEVEL | CMD_CLOSE_BATCH), 8055,
    "SWITCH",  switch_cmd, (CMD_EXPAND_VARS | CMD_STRIP_QUOTES | CMD_ONLY_BATCH), 8056,
    "TEE", tee_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_CLOSE_BATCH), 8057,
    "TEXT", battext_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_ONLY_BATCH), 8058,
    "TIME", settime_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_SET_ERRORLEVEL), cmd_MSG_TIME_HELP,
    "TIMER", timer_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES), 8059,
    "TITLE", title_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES), 8060,
    "TOUCH", touch_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES), 8061,
    "TYPE", type_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_SET_ERRORLEVEL | CMD_CLOSE_BATCH | CMD_BACKSLASH_OK), cmd_MSG_TYPE_HELP,
    "UNALIAS", unset_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES), 8062,
    "UNSET", unset_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES), 8063,
    "VERIFY", verify_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_ADD_NULLS), cmd_MSG_VERIFY_HELP,
    "VSCRPUT", scrput_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR), 8064,
    "WINDOW", window_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_SET_ERRORLEVEL), 8065,
    "Y", y_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_CLOSE_BATCH), 8066
};

//    "ATTRIB", attrib_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_SET_ERRORLEVEL | CMD_BACKSLASH_OK), 3092,
//    "CLS", cls_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES), 3067,
//    "MD", md_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_SET_ERRORLEVEL | CMD_BACKSLASH_OK), 3053,
//    "MKDIR", md_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_SET_ERRORLEVEL | CMD_BACKSLASH_OK), 3053,
//    "TREE", tree_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES), 3148,
//    "VER", ver_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES), 3050,
//    "VOL", volume_cmd, (CMD_EXPAND_VARS | CMD_EXPAND_REDIR | CMD_STRIP_QUOTES | CMD_ADD_NULLS | CMD_RESET_DISKS | CMD_SET_ERRORLEVEL), 3051,

// # of internal cmds
#define NUMCMDS (sizeof(commands)/sizeof(BUILTIN))


// display the enabled internal commands
int cmds_cmd(int argc, char **argv)
{
    unsigned int i, nColumn;

    argc = GetScrCols();
    for ( i = 0, nColumn = 0; ( i < NUMCMDS ); i++ ) {

        // make sure command hasn't been disabled (SETDOS /I-cmd)
        if ((commands[i].pflag & CMD_DISABLED) == 0) {

            printf( FMT_STR, commands[i].cmdname );

            // get width of display
            if (( ++nColumn % ( argc / 12 )) != 0 )
                printf( FMT_LEFT_STR, 12 - strlen( commands[i].cmdname ), NULLSTR );
            else
                crlf();
        }
    }

    if (( nColumn % ( argc / 12 )) != 0 )
        crlf();

    return 0;
}


// do binary search to find command in internal command table & return index
int findcmd( char *cmd, int eflag )
{
    static char DELIMS[] = "%10[^    \t;,.\"`\\+=<>|]";
    int low, high, mid, cond;
    char szInternalName[12];

    // set the current compound command character & switch character
    DELIMS[5] = gpIniptr->CmdSep;
    DELIMS[6] = gpIniptr->SwChr;

    // extract the command name (first argument)
    //   (including nasty kludge for nasty people who do "echo:"
    //   and a minor kludge for "y:")
    DELIMS[7] = (char)(( cmd[1] == ':' ) ? ' ' : ':' );
    sscanf( cmd, DELIMS, szInternalName );

    // do a binary search for the command name
    for ( low = 0, high = ( NUMCMDS - 1 ); ( low <= high ); ) {

        mid = ( low + high ) / 2;

        if (( cond = _stricmp( szInternalName, commands[mid].cmdname )) < 0)
            high = mid - 1;
        else if ( cond > 0 )
            low = mid + 1;
        else {
            // kludge for trailing '\' (i.e., "TEXT\")
            if ((( commands[mid].pflag & CMD_BACKSLASH_OK ) == 0 ) && ( cmd[ strlen(szInternalName) ] == '\\' ))
                return -1;
            return (((( commands[mid].pflag & CMD_DISABLED) == 0 ) || eflag ) ? mid : -1 );
        }
    }

    return -1;
}


// do binary search to find command help in internal command table & return help index
int findcmdhelp( char *cmd, int eflag )
{
        static char DELIMS[] = "%10[^    \t;,.\"`\\+=<>|]";
        int low, high, mid, cond;
        char szInternalName[12];

        // set the current compound command character & switch character
        DELIMS[5] = gpIniptr->CmdSep;
        DELIMS[6] = gpIniptr->SwChr;

        // extract the command name (first argument)
        //   (including nasty kludge for nasty people who do "echo:"
        //   and a minor kludge for "y:")
        DELIMS[7] = (char)(( cmd[1] == ':' ) ? ' ' : ':' );
        sscanf( cmd, DELIMS, szInternalName );

        // do a binary search for the command name
        for ( low = 0, high = ( NUMCMDS - 1 ); ( low <= high ); ) {

                mid = ( low + high ) / 2;

                if (( cond = _stricmp( szInternalName, commands[mid].cmdname )) < 0)
                        high = mid - 1;
                else if ( cond > 0 )
                        low = mid + 1;
                else {
                        // kludge for trailing '\' (i.e., "TEXT\")
                        if ((( commands[mid].pflag & CMD_BACKSLASH_OK ) == 0 ) && ( cmd[ strlen(szInternalName) ] == '\\' ))
                                return -1;
                        return (((( commands[mid].pflag & CMD_DISABLED) == 0 ) || eflag ) ? commands[mid].index : -1 );
                }
        }

        return -1;
}
