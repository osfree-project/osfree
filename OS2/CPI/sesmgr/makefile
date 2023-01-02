#
# A Makefile for sesmgr.dll
# (c) osFree project,
# author, date
#

PROJ        = sesmgr
TRGT        = $(PROJ).dll
DESC        = Session Manager API
OBJS        = $(PATH)..$(SEP)shared$(SEP)stub.$(O)
ADD_COPT    = -i=..\include
ADD_LINKOPT = lib os2386.lib lib clib3r.lib segment type DATA shared
DLL     = 1
DLLOPT  = initglobal
OPTIONS = manyautodata
IMPORTS =  DOSSMSGDOPOPUP          DOSCALLS.686, &
           DOSSMSWITCH             DOSCALLS.687, &
           DOSSMSERVEAPPREQ        DOSCALLS.688, &
           DOSGETTIMES             DOSCALLS.689, &
           DOSSMSETTITLE           DOSCALLS.690, &
           DOSSCRUNLOCK            DOSCALLS.691, &
           DOSSMDOAPPREQ           DOSCALLS.692, &
           DOSSTOPSESSION          DOSCALLS.693, &
           DOSSELECTSESSION        DOSCALLS.694, &
           DOSSCRLOCK              DOSCALLS.695, &
           DOSSAVREDRAWWAIT        DOSCALLS.696, &
           DOSSAVREDRAWUNDO        DOSCALLS.697, &
           DOSSMSGENDPOPUP         DOSCALLS.698, &
           DOSSETSESSION           DOSCALLS.699, &
           DOSSETMNLOCKTIME        DOSCALLS.700, &
           DOSMODEUNDO             DOSCALLS.701, &
           DOSSTARTSESSION         DOSCALLS.702, &
           DOSSMGETSTATUS          DOSCALLS.703, &
           DOSMODEWAIT             DOSCALLS.704, &
           DOSSMTERMINATE          DOSCALLS.705, &
           DOSSMGETAPPREQ          DOSCALLS.706, &
           DOSSMINITIALIZE         DOSCALLS.707, &
           DOSSMSTART              DOSCALLS.708, &
           DOSSMPARENTSWITCH       DOSCALLS.709, &
           DOSSMPAUSE              DOSCALLS.710, &
           DOSSMHDEINIT            DOSCALLS.711, &
           DOSSMPMPRESENT          DOSCALLS.712, &
           DOSSMREGISTERDD         DOSCALLS.713, &
           DOSSMSYSINIT            DOSCALLS.855, &
           DOSSMNOTIFYDD           DOSCALLS.714, &
           DOSSMNOTIFYDD2          DOSCALLS.715, &
           DOSSMOPENDD             DOSCALLS.716, &
           QHKEYBDHANDLE           DOSCALLS.856, &
           QHMOUSEHANDLE           DOSCALLS.857, &
           DOSSMSETSESSIONTYPE     DOSCALLS.717, &
           DosStartSession2        DOSCALLS.669, &
           DosSelectSession2       DOSCALLS.670, &
           DosSetSession2          DOSCALLS.671, &
           DosStopSession2         DOSCALLS.672, &
           DOSREGISTERNOTIFICATION DOSCALLS.673
EXPORTS =  DOSSMSGDOPOPUP.1,           &
           DOSSMSWITCH.2,              &
           DOSSMSERVEAPPREQ.3,         &
           DOSGETTIMES.4,              &
           DOSSMSETTITLE.5,            &
           DOSSCRUNLOCK.6,             &
           DOSSMDOAPPREQ.7,            &
           DOSSTOPSESSION.8,           &
           DOSSELECTSESSION.9,         &
           DOSSCRLOCK.10,              &
           DOSSAVREDRAWWAIT.11,        &
           DOSSAVREDRAWUNDO.12,        &
           DOSSMSGENDPOPUP.13,         &
           DOSSETSESSION.14,           &
           DOSSETMNLOCKTIME.15,        &
           DOSMODEUNDO.16,             &
           DOSSTARTSESSION.17,         &
           DOSSMGETSTATUS.18,          &
           DOSMODEWAIT.19,             &
           DOSSMTERMINATE.20,          &
           DOSSMGETAPPREQ.21,          &
           DOSSMINITIALIZE.23,         &
           DOSSMSTART.24,              &
           DOSSMPARENTSWITCH.25,       &
           DOSSMPAUSE.26,              &
           DOSSMHDEINIT.27,            &
           DOSSMPMPRESENT.28,          &
           DOSSMREGISTERDD.29,         &
           DOSSMSYSINIT.30,            &
           DOSSMNOTIFYDD.31,           &
           DOSSMNOTIFYDD2.32,          &
           DOSSMOPENDD.33,             &
           QHKEYBDHANDLE.34,           &
           QHMOUSEHANDLE.35,           &
           DOSSMSETSESSIONTYPE.36,     &
           DosStartSession2.37,        &
           DosSelectSession2.38,       &
           DosSetSession2.39,          &
           DosStopSession2.40,         &
           DOSREGISTERNOTIFICATION.41
DEST     = os2$(SEP)dll

!include $(%ROOT)tools/mk/appsos2_cmd.mk
