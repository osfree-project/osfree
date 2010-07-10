#
# A Makefile for bvscalls.dll
# (c) osFree project,
# author, date
#

#TARGETS   = $(MYDIR)sub32.lib

PROJ        = sub32
DESC        = Base Subsystems API
srcfiles    = $(p)emx_revision$(e) $(p)viowrttty$(e) $(p)dllstart$(e) $(p)kbd$(e)
ADD_COPT    = -i=..\include -s
ADD_LINKOPT = segment type DATA shared op nod
DLL     = 1
DLLOPT  = initglobal
OPTIONS = manyautodata #This is required because each application has
                       #its own AnsiState
#IMPORTS =
#
#          MOUGETPTRSHAPE       DOSCALLS.719, &
#          MOUSETPTRSHAPE       DOSCALLS.720, &
#          MOUGETNUMMICKEYS     DOSCALLS.721, &
#          MOUGETHOTKEY         DOSCALLS.853, &
#          MOUSHELLINIT         DOSCALLS.723, &
#          MOUGETSCALEFACT      DOSCALLS.724, &
#          MOUFLUSHQUE          DOSCALLS.725, &
#          MOUGETNUMBUTTONS     DOSCALLS.726, &
#          MOUCLOSE             DOSCALLS.727, &
#          MOUSETHOTKEY         DOSCALLS.854, &
#          MOUSETSCALEFACT      DOSCALLS.729, &
#          MOUGETNUMQUEEL       DOSCALLS.730, &
#          MOUDEREGISTER        DOSCALLS.731, &
#          MOUGETEVENTMASK      DOSCALLS.732, &
#          MOUSETEVENTMASK      DOSCALLS.733, &
#          MOUOPEN              DOSCALLS.734, &
#          MOUREMOVEPTR         DOSCALLS.735, &
#          MOUGETPTRPOS         DOSCALLS.736, &
#          MOUREADEVENTQUE      DOSCALLS.737, &
#          MOUSETPTRPOS         DOSCALLS.738, &
#          MOUGETDEVSTATUS      DOSCALLS.739, &
#          MOUSYNCH             DOSCALLS.740, &
#          MOUREGISTER          DOSCALLS.741, &
#          MOUSETDEVSTATUS      DOSCALLS.742, &
#          MOUDRAWPTR           DOSCALLS.743, &
#          MOUINITREAL          DOSCALLS.744, &
#          MOUGETTHRESHOLD      DOSCALLS.722, &
#          MOUSETTHRESHOLD      DOSCALLS.728

EXPORTS = &
#VioGetAnsi                  .103, &
#            VioSetAnsi                  .105, &
            VioWrtTTY                    .119, &
            KbdStringIn                  .209

#  EMX_REVISION.1 RESIDENT, &
#  WinDefAVioWindowProc        .30, &
#  VioEndPopUp                 .101, &
#  VioGetPhysBuf               .102, &
#  VioDeRegister               .106, &
#  VioScrollUp                 .107, &
#  VioPrtSc                    .108, &
#  VioGetCurPos                .109, &
#  VioWrtCellStr               .110, &
#  VioPopUp                    .111, &
#  VioScrollRt                 .112, &
#  VioWrtCharStr               .113, &
#  VioSetCurPos                .115, &
#  VioScrUnLock                .118, &
#  VioGetMode                  .121, &
#  VioSetMode                  .122, &
#  VioScrLock                  .123, &
#  VioReadCellStr              .124, &
#  VioSavRedrawWait            .125, &
#  VioWrtNAttr                 .126, &
#  VioGetCurType               .127, &
#  VioSavRedrawUndo            .128, &
#  VioGetFont                  .129, &
#  VioReadCharStr              .130, &
#  VioGetBuf                   .131, &
#  VioSetCurType               .132, &
#  VioSetFont                  .133, &
#  VioModeUndo                 .135, &
#  VioModeWait                 .137, &
#  VioGetCp                    .140, &
#  VioSetCp                    .142, &
#  VioShowBuf                  .143, &
#  VioScrollLf                 .144, &
#  VioRegister                 .145, &
#  VioGetConfig                .146, &
#  VioScrollDn                 .147, &
#  VioWrtCharStrAtt            .148, &
#  VioGetState                 .149, &
#  VioPrtScToggle              .150, &
#  VioSetState                 .151, &
#  VioWrtNCell                 .152, &
#  VioWrtNChar                 .153, &
#  VioAssociate                .155, &
#  VioCreatePS                 .156, &
#  VioDeleteSetId              .157, &
#  VioGetDeviceCellSize        .158, &
#  VioGetOrg                   .159, &
#  VioCreateLogFont            .160, &
#  VioDestroyPS                .161, &
#  VioQuerySetIds              .162, &
#  VioSetOrg                   .163, &
#  VioQueryFonts               .164, &
#  VioSetDeviceCellSize        .165, &
#  VioShowPS                   .166, &
#  VioGlobalReg                .170, &
#  VioCheckCharType            .175, &
#  KbdSetCustXt                .201, &
#  KbdGetCp                    .203, &
#  KbdCharIn                   .204, &
#  KbdSetCp                    .205, &
#  KbdSynch                    .207, &
#  KbdRegister                 .208, &
#  KbdGetStatus                .210, &
#  KbdSetStatus                .211, &
#  KbdGetFocus                 .212, &
#  KbdFlushBuffer              .213, &
#  KbdXlate                    .214, &
#  KbdClose                    .217, &
#  KbdFreeFocus                .218, &
#  KbdDeRegister               .220, &
#  KbdSetFgnd                  .221, &
#  KbdPeek                     .222, &
#  KbdOpen                     .223, &
#  KbdGetHWID                  .224, &
#  KbdSetHWID                  .225, &
#  MouGetPtrShape              .301, &
#  MouSetPtrShape              .302, &
#  MouGetNumMickeys            .303, &
#  MouGetScaleFact             .306, &
#  MouFlushQue                 .307, &
#  MouGetNumButtons            .308, &
#  MouClose                    .309, &
#  MouSetScaleFact             .311, &
#  MouGetNumQueEl              .313, &
#  MouDeRegister               .314, &
#  MouGetEventMask             .315, &
#  MouSetEventMask             .316, &
#  MouOpen                     .317, &
#  MouRemovePtr                .318, &
#  MouGetPtrPos                .319, &
#  MouReadEventQue             .320, &
#  MouSetPtrPos                .321, &
#  MouGetDevStatus             .322, &
#  MouSynch                    .323, &
#  MouRegister                 .324, &
#  MouSetDevStatus             .325, &
#  MouDrawPtr                  .326, &
#  MouInitReal                 .327, &
#  MouSetThreshold             .329, &
#  MouGetThreshold             .330, &
#  DosMonWrite                 .401, &
#  DosMonRead                  .402, &
#  DosMonClose                 .403, &
#  DosMonOpen                  .404, &
#  DosMonReg                   .405

DEST    = os2$(SEP)dll

!include $(%ROOT)/mk/appsos2.mk
