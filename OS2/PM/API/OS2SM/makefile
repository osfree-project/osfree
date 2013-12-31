#
# A Makefile for os2sm.dll
# (c) osFree project,
# author, date
#

PROJ     = os2sm
DESC     = Session manager API forwarder
srcfiles = $(p)..$(SEP)shared$(SEP)stub$(e)
ADD_COPT = -i=..$(SEP)include
ADD_LINKOPT  = lib os2386.lib lib clib3r.lib segment   type DATA shared
DLL     = 1
DLLOPT  = initglobal
OPTIONS = manyautodata
ADD_COPT = -i=..\include
ADD_LINKOPT  = lib os2386.lib lib clib3r.lib
IMPORTS = &
        WININITSESSIONMGR              PMMERGE.3144, &
        WINSETFGNDWINDOW               PMMERGE.3254, &
        SHLUNLOADPUBLICFONTS           PMMERGE.3009, &
        WINSTPREGISTER                 PMMERGE.3285, &
        WININSTSTARTAPP                PMMERGE.3147, &
        WINTERMINATEAPP                PMMERGE.3295, &
        WINCREATESWITCHENTRY           PMMERGE.3096, &
        WINQUERYSESSIONTITLE           PMMERGE.3202, &
        WINADDSWITCHENTRY              PMMERGE.3080, &
        WINCHANGESWITCHENTRY           PMMERGE.3087, &
        WINQUERYSWITCHENTRY            PMMERGE.3204, &
        WINQUERYSWITCHHANDLE           PMMERGE.3205, &
        WINQUERYTASKTITLE              PMMERGE.3209, &
        WINQUERYTASKSIZEPOS            PMMERGE.3208, &
        WINQUERYSWITCHLIST             PMMERGE.3206, &
        WINREMOVESWITCHENTRY           PMMERGE.3232, &
        WINSWITCHTOPROGRAM             PMMERGE.3291, &
        WINSWITCHPROGRAMREGISTER       PMMERGE.3289, &
#
        WINENDPROGRAM                  PMMERGE.3116, &
        WINSTOPPROGRAM                 PMMERGE.3283, &
        WINENDWINDOWSESSION            PMMERGE.3118, &
        LPFNSHELLWNDPROC               PMMERGE.3004, &
        LPFNICONWNDPROC                PMMERGE.3003, &
        WINSWITCHTOTASKMANAGER         PMMERGE.3292, &
        FNBADAPPDLGPROC                PMMERGE.3001, &
        WINSWITCHTOPROGRAM2            PMMERGE.3290, &
        WINPROCESSHOTKEY               PMMERGE.5421, &
#
        WININITSESSION                 PMMERGE.3143, &
        WINENDSESSION                  PMMERGE.3117, &
        WININITSWENTRY                 PMMERGE.3145, &
        WINSETSWENTRY                  PMMERGE.3267, &
        WINQUERYEXTIDFOCUS             PMMERGE.3188, &
        WINSETEXTIDFOCUS               PMMERGE.3253, &
        SHESYSTEMSHUTDOWN              PMMERGE.3006, &
        STARTSYSTEMEXECUTABLES         PMMERGE.3013, &
        SHLLOADPUBLICFONTS             PMMERGE.3008, &
        WINNOSHUTDOWN                  PMMERGE.3169, &
        SHEINITIALIZEINIFILE           PMMERGE.2031, &
        WINSETTITLE                    PMMERGE.3271, &
        WINCPLREGISTER                 PMMERGE.3090, &
        WINPMFILEREGISTER              PMMERGE.3171, &
        SHESTARTLONGPROGRAM            PMMERGE.3005, &
        STARTSTARTUPDOTCMD             PMMERGE.3012, &
        WINCANTERMINATE                PMMERGE.3085, &
#
        SHELLWINDOW                    PMMERGE.6202
EXPORTS = &
        WININITSESSIONMGR               .0001, &
        WINSETFGNDWINDOW                .0002, &
        SHLUNLOADPUBLICFONTS            .0003, &
        WINSTPREGISTER                  .0004, &
        WININSTSTARTAPP                 .0005, &
        WINTERMINATEAPP                 .0006, &
        WINCREATESWITCHENTRY            .0007, &
        WINQUERYSESSIONTITLE            .0008, &
        WINADDSWITCHENTRY               .0009, &
        WINCHANGESWITCHENTRY            .0010, &
        WINQUERYSWITCHENTRY             .0011, &
        WINQUERYSWITCHHANDLE            .0012, &
        WINQUERYTASKTITLE               .0013, &
        WINQUERYTASKSIZEPOS             .0014, &
        WINQUERYSWITCHLIST              .0015, &
        WINREMOVESWITCHENTRY            .0016, &
        WINSWITCHTOPROGRAM              .0017, &
        WINSWITCHPROGRAMREGISTER        .0018, &
#
        WINENDPROGRAM                   .0021, &
        WINSTOPPROGRAM                  .0022, &
        WINENDWINDOWSESSION             .0023, &
        LPFNSHELLWNDPROC                .0024, &
        LPFNICONWNDPROC                 .0025, &
        WINSWITCHTOTASKMANAGER          .0026, &
        FNBADAPPDLGPROC                 .0027, &
        WINSWITCHTOPROGRAM2             .0028, &
        WINPROCESSHOTKEY                .0029, &
#
        WININITSESSION                  .0031, &
        WINENDSESSION                   .0032, &
        WININITSWENTRY                  .0033, &
        WINSETSWENTRY                   .0034, &
        WINQUERYEXTIDFOCUS              .0035, &
        WINSETEXTIDFOCUS                .0036, &
        SHESYSTEMSHUTDOWN               .0037, &
        STARTSYSTEMEXECUTABLES          .0038, &
        SHLLOADPUBLICFONTS              .0039, &
        WINNOSHUTDOWN                   .0040, &
        SHEINITIALIZEINIFILE            .0041, &
        WINSETTITLE                     .0042, &
        WINCPLREGISTER                  .0043, &
        WINPMFILEREGISTER               .0044, &
        SHESTARTLONGPROGRAM             .0045, &
        STARTSTARTUPDOTCMD              .0046, &
        WINCANTERMINATE                 .0047, &
#
        SHELLWINDOW                     .0103

DEST     = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2_cmd.mk

