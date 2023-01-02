#
# A Makefile for os2char2.dll
# (c) osFree project,
# author, date
#

PROJ        = os2char2
TRGT        = $(PROJ).dll
DESC        = 32TEXT API 
srcfiles    = $(p)os2char$(e)
#ADD_COPT    = -i=..$(SEP)include
ADD_LINKOPT = lib os2386.lib lib clib3r.lib segment type DATA shared
DLL     = 1
DLLOPT  = initglobal
OPTIONS = manyautodata
IMPORTS = &
            EMXKbdSetCustXt                EMXWRAP.201, &
            EMXKbdGetCp                    EMXWRAP.203, &
            EMXKbdCharIn                   EMXWRAP.204, &
            EMXKbdSetCp                    EMXWRAP.205, &
            EMXKbdSynch                    EMXWRAP.207, &
            EMXKbdRegister                 EMXWRAP.208, &
            EMXKbdStringIn                 EMXWRAP.209, &
            EMXKbdGetStatus                EMXWRAP.210, &
            EMXKbdSetStatus                EMXWRAP.211, &
            EMXKbdGetFocus                 EMXWRAP.212, &
            EMXKbdFlushBuffer              EMXWRAP.213, &
            EMXKbdXlate                    EMXWRAP.214, &
            EMXKbdClose                    EMXWRAP.217, &
            EMXKbdFreeFocus                EMXWRAP.218, &
            EMXKbdDeRegister               EMXWRAP.220, &
            EMXKbdSetFgnd                  EMXWRAP.221, &
            EMXKbdPeek                     EMXWRAP.222, &
            EMXKbdOpen                     EMXWRAP.223, &
            EMXKbdGetHWID                  EMXWRAP.224, &
            EMXKbdSetHWID                  EMXWRAP.225, &
            EMXMouGetPtrShape              EMXWRAP.301, &
            EMXMouSetPtrShape              EMXWRAP.302, &
            EMXMouGetNumMickeys            EMXWRAP.303, &
            EMXMouGetScaleFact             EMXWRAP.306, &
            EMXMouFlushQue                 EMXWRAP.307, &
            EMXMouGetNumButtons            EMXWRAP.308, &
            EMXMouClose                    EMXWRAP.309, &
            EMXMouSetScaleFact             EMXWRAP.311, &
            EMXMouGetNumQueEl              EMXWRAP.313, &
            EMXMouDeRegister               EMXWRAP.314, &
            EMXMouGetEventMask             EMXWRAP.315, &
            EMXMouSetEventMask             EMXWRAP.316, &
            EMXMouOpen                     EMXWRAP.317, &
            EMXMouRemovePtr                EMXWRAP.318, &
            EMXMouGetPtrPos                EMXWRAP.319, &
            EMXMouReadEventQue             EMXWRAP.320, &
            EMXMouSetPtrPos                EMXWRAP.321, &
            EMXMouGetDevStatus             EMXWRAP.322, &
            EMXMouSynch                    EMXWRAP.323, &
            EMXMouRegister                 EMXWRAP.324, &
            EMXMouSetDevStatus             EMXWRAP.325, &
            EMXMouDrawPtr                  EMXWRAP.326, &
            EMXMouInitReal                 EMXWRAP.327, &
            EMXMouSetThreshold             EMXWRAP.329, &
            EMXMouGetThreshold             EMXWRAP.330, &
            EMXWinDefAVioWindowProc        EMXWRAP.30,  &
            EMXVioEndPopUp                 EMXWRAP.101, &
            EMXVioGetPhysBuf               EMXWRAP.102, &
            EMXVioGetAnsi                  EMXWRAP.103, &
            EMXVioSetAnsi                  EMXWRAP.105, &
            EMXVioWrtTTY                   EMXWRAP.119, &
            EMXVioDeRegister               EMXWRAP.106, &
            EMXVioScrollUp                 EMXWRAP.107, &
            EMXVioPrtSc                    EMXWRAP.108, &
            EMXVioGetCurPos                EMXWRAP.109, &
            EMXVioWrtCellStr               EMXWRAP.110, &
            EMXVioPopUp                    EMXWRAP.111, &
            EMXVioScrollRt                 EMXWRAP.112, &
            EMXVioWrtCharStr               EMXWRAP.113, &
            EMXVioSetCurPos                EMXWRAP.115, &
            EMXVioScrUnLock                EMXWRAP.118, &
            EMXVioGetMode                  EMXWRAP.121, &
            EMXVioSetMode                  EMXWRAP.122, &
            EMXVioScrLock                  EMXWRAP.123, &
            EMXVioReadCellStr              EMXWRAP.124, &
            EMXVioSavRedrawWait            EMXWRAP.125, &
            EMXVioWrtNAttr                 EMXWRAP.126, &
            EMXVioGetCurType               EMXWRAP.127, &
            EMXVioSavRedrawUndo            EMXWRAP.128, &
            EMXVioGetFont                  EMXWRAP.129, &
            EMXVioReadCharStr              EMXWRAP.130, &
            EMXVioGetBuf                   EMXWRAP.131, &
            EMXVioSetCurType               EMXWRAP.132, &
            EMXVioSetFont                  EMXWRAP.133, &
            EMXVioModeUndo                 EMXWRAP.135, &
            EMXVioModeWait                 EMXWRAP.137, &
            EMXVioGetCp                    EMXWRAP.140, &
            EMXVioSetCp                    EMXWRAP.142, &
            EMXVioShowBuf                  EMXWRAP.143, &
            EMXVioScrollLf                 EMXWRAP.144, &
            EMXVioRegister                 EMXWRAP.145, &
            EMXVioGetConfig                EMXWRAP.146, &
            EMXVioScrollDn                 EMXWRAP.147, &
            EMXVioWrtCharStrAtt            EMXWRAP.148, &
            EMXVioGetState                 EMXWRAP.149, &
            EMXVioPrtScToggle              EMXWRAP.150, &
            EMXVioSetState                 EMXWRAP.151, &
            EMXVioWrtNCell                 EMXWRAP.152, &
            EMXVioWrtNChar                 EMXWRAP.153, &
            EMXVioAssociate                EMXWRAP.155, &
            EMXVioCreatePS                 EMXWRAP.156, &
            EMXVioDeleteSetId              EMXWRAP.157, &
            EMXVioGetDeviceCellSize        EMXWRAP.158, &
            EMXVioGetOrg                   EMXWRAP.159, &
            EMXVioCreateLogFont            EMXWRAP.160, &
            EMXVioDestroyPS                EMXWRAP.161, &
            EMXVioQuerySetIds              EMXWRAP.162, &
            EMXVioSetOrg                   EMXWRAP.163, &
            EMXVioQueryFonts               EMXWRAP.164, &
            EMXVioSetDeviceCellSize        EMXWRAP.165, &
            EMXVioShowPS                   EMXWRAP.166, &
            EMXVioGlobalReg                EMXWRAP.170, &
            EMXVioCheckCharType            EMXWRAP.175


#EXPORTS = 

DEST    = os2$(SEP)dll

!include $(%ROOT)tools/mk/appsos2_cmd.mk
