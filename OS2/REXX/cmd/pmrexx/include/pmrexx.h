/*static char *SCCSID = "@(#)pmrexx.h	6.2 91/12/13";                       */
/******************************************************************************/
/* Global values for common uses...                                           */
/******************************************************************************/

#define  ID_NULL        -1
#define  ENTER          1
#define  ESCAPE         2
#define  HELP           3
#define  DELIMITER_SIZE 5
#define  TITLE_SIZE     55             /* Maximum title length                */
#define  FNAME_SIZE     260            /* Maximum filename size               */
#define  STACK_SIZE     8196           /* thread stack size                   */
#define  PROC_NOT_FOUND (-3)
#define  TIMER_DELAY    100            /* Length of timer delay               */
#define  MAXWIDTH       200
#define  ONE_K          1024
#define  BUTTON_TEXT_LENGTH 30         /* length of text for buttons          */
#define  MSG_SIZE       300
#define  ON_m(a,b)      ((a & b) == b)

/******************************************************************************/
/* Main Menu defines                                                          */
/******************************************************************************/

#define  PMRXM_FILE          RXIOM_USER+1
#define  PMRXA_OPEN          RXIOM_USER+7
#define  PMRXA_SAVE          RXIOM_USER+2  
#define  PMRXA_SAVEAS        RXIOM_USER+3
#define  PMRXA_EXIT          RXIOM_USER+4
#define  PMRXA_ABOUT         RXIOM_USER+5
#define  PMRXA_RESTART       RXIOM_USER+6
#define  PMRXA_SEARCH        RXIOM_USER+8


/******************************************************************************/
/* SubClass Message Defines                                                   */
/******************************************************************************/

#define  DOS_ERROR       RXIOWM_USER+11
#define  REXX_PROC_NF    RXIOWM_USER+12
#define  REXX_PROC_ENDED RXIOWM_USER+13
#define  RXHB_MSGBOX     RXIOWM_USER+14
#define  REXX_PROC_RERUN RXIOWM_USER+15

/* Start of string space:                                                     */

#define  RXHM_PROC_ENDED  0x01
#define  BAD_NUM_PARMS    0x02
#define  PROGRAM_NAME     0x03
#define  RXHA_MSGBOXHDR   0x05
#define  RXHM_PROC_NOT_FOUND 0x06
#define  RELEASE_INFO     0x75

#define  RETRY            0x07
#define  CANCEL           0x08
#define  ABORT            0x09
#define  IGNORE           0x0a
#define  OK               0x0b
#define  YES              0x0c
#define  NO               0x0d
#define  BENTER           0x0e

/* ClipBoard Errors Headings                                                  */

#define  RXH_APP_ERROR     0x0f
#define  RXH_READ_ERROR    0x10
#define  RXH_WRITE_ERROR   0x11
#define  RXH_OPEN_ERROR    0x12
#define  RXH_BAD_PATH      0x13
#define  RXH_TOO_MANY_OPEN 0x14
#define  RXH_ACCESS_DENIED 0x15
#define  RXH_INV_ACCESS    0x16
#define  RXH_NOT_DISK      0x17
#define  RXH_SHARE_VIOL    0x18
#define  RXH_SHARE_BUFF_EXC 0x19
#define  RXH_CANNOT_MAKE   0x1a
#define  RXH_DRIVE_LOCKED  0x1b
#define  RXH_DISK_FULL     0x1c
#define  RXH_OUTOF_MEM     0x1d
#define  RXH_NOT_READY     0x1e
#define  RXH_WRITE_PROT    0x1f
#define  RXH_WERRL_UNK     0x40
#define  RXH_WERRC_START   0x40
#define  RXH_WERRC_UNK     0x41

#define  RXH_TITLE         0x42
#define  RXH_WARNING       0x43
#define  RXHD_EXISTS       0x44
#define  SAVE_QUIT         0x45
#define  SAVE              0x46
#define  NOCHANGE          0x47
#define  RX_UNTITLED       0x48
#define  MSG_FILE          0x49
#define  SYS_ERROR_TEXT    0x4a
#define  RXH_NAME_LONG     0x4b
#define  RXH_BAD_NAME      0x4c
#define  BACKPATH_DELIMETER 0x4d
#define  DRIVE_DELIMETER   0x4e
#define  OPEN              0x4f
#define  RXH_ERRC_START    0x50
#define  RXH_ERRC_OUTRES   0x51
#define  RXH_ERRC_TEMPSIT  0x52
#define  RXH_ERRC_AUTH     0x53
#define  RXH_ERRC_INTRN    0x54
#define  RXH_ERRC_HRDFAIL  0x55
#define  RXH_ERRC_SYSFAIL  0x56
#define  RXH_ERRC_APPERR   0x57
#define  RXH_ERRC_NOTFND   0x58
#define  RXH_ERRC_BADFMT   0x59
#define  RXH_ERRC_LOCKED   0x5a
#define  RXH_ERRC_MEDIA    0x5b
#define  RXH_ERRC_ALREADY  0x5c
#define  RXH_ERRC_UNK      0x5d
#define  RXH_ERRC_CANT     0x5e
#define  RXH_ERRC_TIME     0x5f
#define  RXH_ERRL_START    0x60
#define  RXH_ERRL_UNK      0x61
#define  RXH_ERRL_DISK     0x62
#define  RXH_ERRL_NET      0x63
#define  RXH_ERRL_SERDEV   0x64
#define  RXH_ERRL_MEM      0x65
#define  RXH_WERRL_START   0x66
#define  RXH_WERRL_SHELL   0x67
#define  RXH_WERRL_GPI     0x68
#define  RXH_WERRL_PM      0x69

/* Help Defines                                                               */

#define  RXHH_HELPHELP     1
#define  FONT_HELP         2
#define  PMREXX_HELPTABLE  3

/******************************************************************************/
/* String Defines                                                             */
/******************************************************************************/

#define  RXH_ERRORTITLE    0x2FC
#define  RXH_NOTIFICATION  0x2FD
#define  RXH_CRITICAL      0x2FF
#define  SELECTED_TEXT     0x8110
#define  NORMAL_TEXT       0x8111
#define  RXHD_WIN_ERROR    0x8112
#define  RXHD_NOT_FOUND    0x8114
#define  RXHD_STRING_NOT_FOUND 0x8118
#define  RXHD_RO_FILE      0x8119
#define  RXHD_CONFUCIOUS   0x811A
#define  RXHM_UNDO         0x811B
#define  RXHD_SEARCH_FAILS 0x811D
#define  RXHD_BAD_SEARCH   0x811E
#define  RHD_ABOUT         4500
#define  RXHD_ERROR        0x5500

/* string identifiers                                                         */

#define  PMRXHB_FILE       0x2F0
#define  PMRXHB_SAVE       0x2F1
#define  PMRXHB_SAVEAS     0x2F2
#define  PMRXHB_EXIT       0x2F3
#define  PMRXHB_ABOUT      0x2F4
#define  PMRXHB_RESTART    0x2F5
#define  PMRXHB_SAVETITLE  0x2F6
#define  PMRXHB_FILEMASK   0x2F7

/* Position Dialog -- rfd */
#define DLG_OFFSET_X           5
#define DLG_OFFSET_Y           5
#define DLG_OFFSET_TOP         45

/* This structure contains information about PMREXX's environment             */

typedef struct _ENVINFO {
    PCHAR  envptr;
    USHORT cursize;
    USHORT maxsize;
} ENVINFO;

typedef ENVINFO FAR *PENVINFO;

/******************************************************************************/
/* Dos Errors                                                                 */
/******************************************************************************/

typedef struct _RXHA_ERR {
    PSZ Title;
    PSZ RetCode;
    PSZ Locus;
    PSZ Class;
    PSZ message;
    PSZ AppTitle;
    int buttonStyle;
    SHORT usIconStyle;
    ULONG  action;
    HAB hab;
    HMODULE hmod;
} RXHA_ERR;

typedef RXHA_ERR *PRXHA_ERR;

/******************************************************************************/
/* structure to hold data for the save/quit dialog box                        */
/******************************************************************************/

typedef struct _RX_SAVQUIT_STRUCT {
    HAB hab;                           /* handle anchor block                 */
    HWND hClientWnd;                   /* clients handle                      */
    PSZ pszSaveQuitMsg;                /* Want to save or quit message        */
    BOOL bNamedFile;                   /* does this file have a name          */
} RX_SAVQUIT_STRUCT;

typedef RX_SAVQUIT_STRUCT *PRX_SAVQUIT_STRUCT;


void    CallRexx(void);
#pragma linkage(CallRexx, system)
PUCHAR  getstring(ULONG );
ULONG   CheckRexxFile(PUCHAR);
MRESULT EXPENTRY ErrorNotifyProc(HWND,
                                ULONG,
                                MPARAM,
                                MPARAM);
MRESULT EXPENTRY PMRexxSubProc(HWND,
                                ULONG,
                                MPARAM,
                                MPARAM);
MRESULT EXPENTRY SaveQuitDlgProc(HWND,
                                ULONG,
                                MPARAM,
                                MPARAM);
MRESULT EXPENTRY CuaLogoDlgProc(HWND,
                                ULONG,
                                MPARAM,
                                MPARAM);
RexxExitHandler init_routine;
void    SetOptions(ULONG, BOOL);
void    SetCheckMark(ULONG, BOOL);
MRESULT PMRXCmds(HWND, MPARAM, MPARAM);
ULONG   SysErrorBoxM(HWND,ULONG ,PSZ,ULONG );
ULONG   SelectFile(PSZ, PSZ, PSZ);
BOOL    WriteFile(HAB, HWND, PSZ, PSZ, BOOL);
ULONG   OpenFile(PSZ, PHFILE, PULONG, ULONG);
ULONG   CheckDosError(HAB, HWND, ULONG, PSZ);
BOOL    PositionDlg(HWND);
void    InitDlgSysMenu(HWND, BOOL);
ULONG   MapButtonText(PSZ);
ULONG   ProcessSaveQuit(HWND);
ULONG   SaveQuit(HAB,HWND,PSZ);
