#
# A Makefile for sesmgr.dll
# (c) osFree project,
# author, date
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1
DIR      = $(ROOT)\bin\os2\dll
PROJ = sesmgr
DESC = Session Manager API
OBJS     = ..\shared\stub.obj
ADD_COPT = -i=..\include
ADD_LINKOPT  = lib os2386.lib lib clib3r.lib

!include $(%ROOT)/mk/all.mk

all: $(PROJ).dll .SYMBOLIC

$(PROJ).dll: $(OBJS)
 @%create $^&.lnk
 @%append $^&.lnk name      $(PROJ).dll
 @%append $^&.lnk format    os2 lx dll initglobal
 @%append $^&.lnk option    manyautodata
 @%append $^&.lnk segment   type DATA shared
 @%append $^&.lnk OPTION DESCRIPTION '$(FILEVER)  $(DESC)'
 @%append $^&.lnk OPTION MAP=$^&.wmp
 @%append $^&.lnk import
 @%append $^&.lnk   DOSSMSGDOPOPUP          DOSCALLS.686,
 @%append $^&.lnk   DOSSMSWITCH             DOSCALLS.687,
 @%append $^&.lnk   DOSSMSERVEAPPREQ        DOSCALLS.688,
 @%append $^&.lnk   DOSGETTIMES             DOSCALLS.689,
 @%append $^&.lnk   DOSSMSETTITLE           DOSCALLS.690,
 @%append $^&.lnk   DOSSCRUNLOCK            DOSCALLS.691,
 @%append $^&.lnk   DOSSMDOAPPREQ           DOSCALLS.692,
 @%append $^&.lnk   DOSSTOPSESSION          DOSCALLS.693,
 @%append $^&.lnk   DOSSELECTSESSION        DOSCALLS.694,
 @%append $^&.lnk   DOSSCRLOCK              DOSCALLS.695,
 @%append $^&.lnk   DOSSAVREDRAWWAIT        DOSCALLS.696,
 @%append $^&.lnk   DOSSAVREDRAWUNDO        DOSCALLS.697,
 @%append $^&.lnk   DOSSMSGENDPOPUP         DOSCALLS.698,
 @%append $^&.lnk   DOSSETSESSION           DOSCALLS.699,
 @%append $^&.lnk   DOSSETMNLOCKTIME        DOSCALLS.700,
 @%append $^&.lnk   DOSMODEUNDO             DOSCALLS.701,
 @%append $^&.lnk   DOSSTARTSESSION         DOSCALLS.702,
 @%append $^&.lnk   DOSSMGETSTATUS          DOSCALLS.703,
 @%append $^&.lnk   DOSMODEWAIT             DOSCALLS.704,
 @%append $^&.lnk   DOSSMTERMINATE          DOSCALLS.705,
 @%append $^&.lnk   DOSSMGETAPPREQ          DOSCALLS.706,
 @%append $^&.lnk   DOSSMINITIALIZE         DOSCALLS.707,
 @%append $^&.lnk   DOSSMSTART              DOSCALLS.708,
 @%append $^&.lnk   DOSSMPARENTSWITCH       DOSCALLS.709,
 @%append $^&.lnk   DOSSMPAUSE              DOSCALLS.710,
 @%append $^&.lnk   DOSSMHDEINIT            DOSCALLS.711,
 @%append $^&.lnk   DOSSMPMPRESENT          DOSCALLS.712,
 @%append $^&.lnk   DOSSMREGISTERDD         DOSCALLS.713,
 @%append $^&.lnk   DOSSMSYSINIT            DOSCALLS.855,
 @%append $^&.lnk   DOSSMNOTIFYDD           DOSCALLS.714,
 @%append $^&.lnk   DOSSMNOTIFYDD2          DOSCALLS.715,
 @%append $^&.lnk   DOSSMOPENDD             DOSCALLS.716,
 @%append $^&.lnk   QHKEYBDHANDLE           DOSCALLS.856,
 @%append $^&.lnk   QHMOUSEHANDLE           DOSCALLS.857,
 @%append $^&.lnk   DOSSMSETSESSIONTYPE     DOSCALLS.717,
 @%append $^&.lnk   DosStartSession2        DOSCALLS.669,
 @%append $^&.lnk   DosSelectSession2       DOSCALLS.670,
 @%append $^&.lnk   DosSetSession2          DOSCALLS.671,
 @%append $^&.lnk   DosStopSession2         DOSCALLS.672,
 @%append $^&.lnk   DOSREGISTERNOTIFICATION DOSCALLS.673
 @%append $^&.lnk export
 @%append $^&.lnk   DOSSMSGDOPOPUP.1,
 @%append $^&.lnk   DOSSMSWITCH.2,
 @%append $^&.lnk   DOSSMSERVEAPPREQ.3,
 @%append $^&.lnk   DOSGETTIMES.4,
 @%append $^&.lnk   DOSSMSETTITLE.5,
 @%append $^&.lnk   DOSSCRUNLOCK.6,
 @%append $^&.lnk   DOSSMDOAPPREQ.7,
 @%append $^&.lnk   DOSSTOPSESSION.8,
 @%append $^&.lnk   DOSSELECTSESSION.9,
 @%append $^&.lnk   DOSSCRLOCK.10,
 @%append $^&.lnk   DOSSAVREDRAWWAIT.11,
 @%append $^&.lnk   DOSSAVREDRAWUNDO.12,
 @%append $^&.lnk   DOSSMSGENDPOPUP.13,
 @%append $^&.lnk   DOSSETSESSION.14,
 @%append $^&.lnk   DOSSETMNLOCKTIME.15,
 @%append $^&.lnk   DOSMODEUNDO.16,
 @%append $^&.lnk   DOSSTARTSESSION.17,
 @%append $^&.lnk   DOSSMGETSTATUS.18,
 @%append $^&.lnk   DOSMODEWAIT.19,
 @%append $^&.lnk   DOSSMTERMINATE.20,
 @%append $^&.lnk   DOSSMGETAPPREQ.21,
 @%append $^&.lnk   DOSSMINITIALIZE.23,
 @%append $^&.lnk   DOSSMSTART.24,
 @%append $^&.lnk   DOSSMPARENTSWITCH.25,
 @%append $^&.lnk   DOSSMPAUSE.26,
 @%append $^&.lnk   DOSSMHDEINIT.27,
 @%append $^&.lnk   DOSSMPMPRESENT.28,
 @%append $^&.lnk   DOSSMREGISTERDD.29,
 @%append $^&.lnk   DOSSMSYSINIT.30,
 @%append $^&.lnk   DOSSMNOTIFYDD.31,
 @%append $^&.lnk   DOSSMNOTIFYDD2.32,
 @%append $^&.lnk   DOSSMOPENDD.33,
 @%append $^&.lnk   QHKEYBDHANDLE.34,
 @%append $^&.lnk   QHMOUSEHANDLE.35,
 @%append $^&.lnk   DOSSMSETSESSIONTYPE.36,
 @%append $^&.lnk   DosStartSession2.37,
 @%append $^&.lnk   DosSelectSession2.38,
 @%append $^&.lnk   DosSetSession2.39,
 @%append $^&.lnk   DosStopSession2.40,
 @%append $^&.lnk   DOSREGISTERNOTIFICATION.41
 $(ADDFILES_CMD)
 $(SAY) Linking $^@ $(LOG)
 $(LINKER) $(LINKOPT) @$^&.lnk $(LOG)

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)

install: .SYMBOLIC
 $(SAY) Making install... $(LOG)
 $(CP) $(PROJ).dll $(FILESDIR)$(SEP)os2$(SEP)dll
