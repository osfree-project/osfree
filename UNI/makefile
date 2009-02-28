#
# A Makefile for Unified API
# (c) osFree project,
# valerius, 2006/10/30
#

!include $(%ROOT)/mk/all.mk

OUT = $(%ROOT)$(SEP)build$(SEP)include
TARGETS = h

subdirs = $(OUT) $(OUT)$(SEP)dos $(OUT)$(SEP)os2 $(OUT)$(SEP)shared

h: pre workaround $(OUT)$(SEP)osfree.h os2 dos
  @%null

pre: .SYMBOLIC
 $(SAY) Creating directories...
 @for %i in ($(subdirs)) do @$(MDHIER) %i

$(OUT)$(SEP)osfree.h: osfree.uni
# Main osFree file

workaround: cdeftypes.h cdeftypes2.h
# Workaround of uniplemented features of uni2h tool
  $(CP) cdeftypes.h $(OUT)$(SEP)cdeftypes.h $(BLACKHOLE)
  $(CP) cdeftypes2.h $(OUT)$(SEP)cdeftypes2.h $(BLACKHOLE)

## OS/2 Personality files
os2: $(OUT)$(SEP)os2$(SEP)os2.h $(OUT)$(SEP)os2$(SEP)os2def.h &
     $(OUT)$(SEP)os2$(SEP)bse.h $(OUT)$(SEP)os2$(SEP)bseord.h &
     $(OUT)$(SEP)os2$(SEP)bsememf.h &
     $(OUT)$(SEP)os2$(SEP)bsedos.h &
     $(OUT)$(SEP)os2$(SEP)bsedev.h &
     $(OUT)$(SEP)os2$(SEP)hevdef.h &
     $(OUT)$(SEP)os2$(SEP)dosdevices.h &
     $(OUT)$(SEP)os2$(SEP)bsesub.h &
     $(OUT)$(SEP)os2$(SEP)sub.h &
     $(OUT)$(SEP)os2$(SEP)vio.h &
     $(OUT)$(SEP)os2$(SEP)kbd.h &
     $(OUT)$(SEP)os2$(SEP)mou.h &
     $(OUT)$(SEP)os2$(SEP)bseerr.h &
     $(OUT)$(SEP)os2$(SEP)bsexcpt.h &
     $(OUT)$(SEP)os2$(SEP)dosexcptfunc.h &
     $(OUT)$(SEP)os2$(SEP)basemid.h &
     $(OUT)$(SEP)os2$(SEP)bsetib.h &
     $(OUT)$(SEP)os2$(SEP)dosdebug.h &
     $(OUT)$(SEP)os2$(SEP)dosmisc.h &
     $(OUT)$(SEP)os2$(SEP)dosmvdm.h &
     $(OUT)$(SEP)os2$(SEP)dosnls.h &
     $(OUT)$(SEP)os2$(SEP)dosnmpipes.h &
     $(OUT)$(SEP)os2$(SEP)dosmodulemgr.h &
     $(OUT)$(SEP)os2$(SEP)dossesmgr.h &
     $(OUT)$(SEP)os2$(SEP)dosfilemgr.h &
     $(OUT)$(SEP)os2$(SEP)dosfilemgrcommon.h &
     $(OUT)$(SEP)os2$(SEP)dosmemmgrcommon.h &
     $(OUT)$(SEP)os2$(SEP)dosprocess.h &
     $(OUT)$(SEP)os2$(SEP)dossemaphores.h &
     $(OUT)$(SEP)os2$(SEP)dossemaphorescommon.h &
     $(OUT)$(SEP)os2$(SEP)dosprocesscommon.h &
     $(OUT)$(SEP)os2$(SEP)dosresources.h &
     $(OUT)$(SEP)os2$(SEP)dosresourcescommon.h &
     $(OUT)$(SEP)os2$(SEP)dosexceptions.h &
     $(OUT)$(SEP)os2$(SEP)dosdatetime.h &
     $(OUT)$(SEP)os2$(SEP)dosdatetimecommon.h &
     $(OUT)$(SEP)os2$(SEP)pm.h &
     $(OUT)$(SEP)os2$(SEP)pmdef.h &
     $(OUT)$(SEP)os2$(SEP)pmshl.h &
     $(OUT)$(SEP)os2$(SEP)winswitchlistcommon.h &
     $(OUT)$(SEP)os2$(SEP)winswitchlist.h &
     $(OUT)$(SEP)os2$(SEP)pmwp.h &
     $(OUT)$(SEP)os2$(SEP)pmwin.h &
     $(OUT)$(SEP)os2$(SEP)pmstddlg.h &
     $(OUT)$(SEP)os2$(SEP)winstdbook.h &
     $(OUT)$(SEP)os2$(SEP)winsys.h &
     $(OUT)$(SEP)os2$(SEP)wininput.h &
     $(OUT)$(SEP)os2$(SEP)wininputcommon.h &
     $(OUT)$(SEP)shared$(SEP)rexxsaa.h &
     $(OUT)$(SEP)os2$(SEP)winclipboard.h &
     $(OUT)$(SEP)os2$(SEP)winmessagemgr.h &
     $(OUT)$(SEP)os2$(SEP)winmessagemgrcommon.h &
     $(OUT)$(SEP)os2$(SEP)winwindowmgrcommon.h &
     $(OUT)$(SEP)os2$(SEP)winwindowmgr.h &
     $(OUT)$(SEP)os2$(SEP)winclipboarddde.h &
     $(OUT)$(SEP)os2$(SEP)winprogramlist.h &
     $(OUT)$(SEP)os2$(SEP)winframemgr.h &
     $(OUT)$(SEP)os2$(SEP)winframemgrcommon.h &
     $(OUT)$(SEP)os2$(SEP)winshelldata.h &
     $(OUT)$(SEP)os2$(SEP)winerrors.h &
     $(OUT)$(SEP)os2$(SEP)shlerrors.h &
     $(OUT)$(SEP)os2$(SEP)pmerr.h &
     $(OUT)$(SEP)os2$(SEP)wperrors.h &
     $(OUT)$(SEP)os2$(SEP)gpierrors.h &
     $(OUT)$(SEP)os2$(SEP)splerrors.h &
     $(OUT)$(SEP)os2$(SEP)picerrors.h &
     $(OUT)$(SEP)os2$(SEP)winhooks.h &
     $(OUT)$(SEP)os2$(SEP)wintimer.h &
     $(OUT)$(SEP)os2$(SEP)wincountry.h &
     $(OUT)$(SEP)os2$(SEP)winentryfields.h &
     $(OUT)$(SEP)os2$(SEP)windialogs.h &
     $(OUT)$(SEP)os2$(SEP)windialogscommon.h &
     $(OUT)$(SEP)os2$(SEP)winbuttons.h &
     $(OUT)$(SEP)os2$(SEP)winlistboxes.h &
     $(OUT)$(SEP)os2$(SEP)pmgpi.h &
     $(OUT)$(SEP)os2$(SEP)gpiprimitives.h &
     $(OUT)$(SEP)os2$(SEP)gpilcids.h &
     $(OUT)$(SEP)os2$(SEP)gpicontrolcommon.h &
     $(OUT)$(SEP)os2$(SEP)gpibitmapscommon.h &
     .symbolic

#     $(OUT)$(SEP)os2$(SEP)bsexcpt.h &

$(OUT)$(SEP)os2$(SEP)os2.h: os2$(SEP)os2.uni

### OS/2 Personality base type and macros definitions
$(OUT)$(SEP)os2$(SEP)os2def.h: os2$(SEP)os2def.uni

### OS/2 Personality base API
$(OUT)$(SEP)os2$(SEP)bse.h: os2$(SEP)bse.uni

### OS/2 Personality base DOS API ordinals
$(OUT)$(SEP)os2$(SEP)bsedos.h: os2$(SEP)bsedos.uni

### OS/2 Personality base error codes and messages
$(OUT)$(SEP)os2$(SEP)bseerr.h: os2$(SEP)bseerr.uni

### OS/2 Personality base API ordinals
$(OUT)$(SEP)os2$(SEP)bseord.h: os2$(SEP)bseord.uni

### OS/2 Personality memory API flags
$(OUT)$(SEP)os2$(SEP)bsememf.h: os2$(SEP)bsememf.uni

$(OUT)$(SEP)os2$(SEP)bsedev.h: os2$(SEP)bsedev.uni

$(OUT)$(SEP)os2$(SEP)bsetib.h: os2$(SEP)bsetib.uni

$(OUT)$(SEP)os2$(SEP)bsexcpt.h: os2$(SEP)bsexcpt.uni

$(OUT)$(SEP)os2$(SEP)bsesub.h: os2$(SEP)bsesub.uni

$(OUT)$(SEP)os2$(SEP)sub.h: os2$(SEP)sub.uni

$(OUT)$(SEP)os2$(SEP)vio.h: os2$(SEP)vio.uni

$(OUT)$(SEP)os2$(SEP)kbd.h: os2$(SEP)kbd.uni

$(OUT)$(SEP)os2$(SEP)mou.h: os2$(SEP)mou.uni

$(OUT)$(SEP)os2$(SEP)dosprocesscommon.h: os2$(SEP)dosprocesscommon.uni

$(OUT)$(SEP)os2$(SEP)dosprocess.h: os2$(SEP)dosprocess.uni

$(OUT)$(SEP)os2$(SEP)dosmisc.h: os2$(SEP)dosmisc.uni

$(OUT)$(SEP)os2$(SEP)basemid.h: os2$(SEP)basemid.uni

$(OUT)$(SEP)os2$(SEP)dosnmpipes.h: os2$(SEP)dosnmpipes.uni

$(OUT)$(SEP)os2$(SEP)dosmvdm.h: os2$(SEP)dosmvdm.uni

$(OUT)$(SEP)os2$(SEP)dossesmgr.h: os2$(SEP)dossesmgr.uni

$(OUT)$(SEP)os2$(SEP)dosnls.h: os2$(SEP)dosnls.uni

$(OUT)$(SEP)os2$(SEP)dosmodulemgr.h: os2$(SEP)dosmodulemgr.uni

$(OUT)$(SEP)os2$(SEP)dosfilemgr.h: os2$(SEP)dosfilemgr.uni

$(OUT)$(SEP)os2$(SEP)dossemaphores.h: os2$(SEP)dossemaphores.uni

$(OUT)$(SEP)os2$(SEP)dossemaphorescommon.h: os2$(SEP)dossemaphorescommon.uni

$(OUT)$(SEP)os2$(SEP)dosdatetime.h: os2$(SEP)dosdatetime.uni

$(OUT)$(SEP)os2$(SEP)dosdevices.h: os2$(SEP)dosdevices.uni

$(OUT)$(SEP)os2$(SEP)dosdatetimecommon.h: os2$(SEP)dosdatetimecommon.uni

$(OUT)$(SEP)os2$(SEP)dosfilemgrcommon.h: os2$(SEP)dosfilemgrcommon.uni

$(OUT)$(SEP)os2$(SEP)dosmemmgrcommon.h: os2$(SEP)dosmemmgrcommon.uni

$(OUT)$(SEP)os2$(SEP)dosexceptions.h: os2$(SEP)dosexceptions.uni

$(OUT)$(SEP)os2$(SEP)dosexcptfunc.h: os2$(SEP)dosexcptfunc.uni

$(OUT)$(SEP)os2$(SEP)dosresources.h: os2$(SEP)dosresources.uni

$(OUT)$(SEP)os2$(SEP)dosresourcescommon.h: os2$(SEP)dosresourcescommon.uni

$(OUT)$(SEP)os2$(SEP)dosdebug.h: os2$(SEP)dosdebug.uni

$(OUT)$(SEP)os2$(SEP)hevdef.h: os2$(SEP)hevdef.uni

$(OUT)$(SEP)os2$(SEP)pm.h: os2$(SEP)pm.uni

$(OUT)$(SEP)os2$(SEP)pmdef.h: os2$(SEP)pmdef.uni

$(OUT)$(SEP)os2$(SEP)pmshl.h: os2$(SEP)pmshl.uni

$(OUT)$(SEP)os2$(SEP)pmstddlg.h: os2$(SEP)pmstddlg.uni

$(OUT)$(SEP)os2$(SEP)winstdbook.h: os2$(SEP)winstdbook.uni

$(OUT)$(SEP)os2$(SEP)winswitchlist.h: os2$(SEP)winswitchlist.uni

$(OUT)$(SEP)os2$(SEP)winswitchlistcommon.h: os2$(SEP)winswitchlistcommon.uni

$(OUT)$(SEP)os2$(SEP)winframemgr.h: os2$(SEP)winframemgr.uni

$(OUT)$(SEP)os2$(SEP)winframemgrcommon.h: os2$(SEP)winframemgrcommon.uni

$(OUT)$(SEP)os2$(SEP)pmwp.h: os2$(SEP)pmwp.uni

$(OUT)$(SEP)os2$(SEP)pmwin.h: os2$(SEP)pmwin.uni

$(OUT)$(SEP)shared$(SEP)rexxsaa.h: shared$(SEP)rexxsaa.uni

$(OUT)$(SEP)os2$(SEP)winprogramlist.h: os2$(SEP)winprogramlist.uni

$(OUT)$(SEP)os2$(SEP)winsys.h: os2$(SEP)winsys.uni

$(OUT)$(SEP)os2$(SEP)winwindowmgrcommon.h: os2$(SEP)winwindowmgrcommon.uni

$(OUT)$(SEP)os2$(SEP)winwindowmgr.h: os2$(SEP)winwindowmgr.uni

$(OUT)$(SEP)os2$(SEP)winmessagemgrcommon.h: os2$(SEP)winmessagemgrcommon.uni

$(OUT)$(SEP)os2$(SEP)winmessagemgr.h: os2$(SEP)winmessagemgr.uni

$(OUT)$(SEP)os2$(SEP)winclipboard.h: os2$(SEP)winclipboard.uni

$(OUT)$(SEP)os2$(SEP)winclipboarddde.h: os2$(SEP)winclipboarddde.uni

$(OUT)$(SEP)os2$(SEP)wininput.h: os2$(SEP)wininput.uni

$(OUT)$(SEP)os2$(SEP)wininputcommon.h: os2$(SEP)wininputcommon.uni

$(OUT)$(SEP)os2$(SEP)winshelldata.h: os2$(SEP)winshelldata.uni

$(OUT)$(SEP)os2$(SEP)winerrors.h: os2$(SEP)winerrors.uni

$(OUT)$(SEP)os2$(SEP)shlerrors.h: os2$(SEP)shlerrors.uni

$(OUT)$(SEP)os2$(SEP)pmerr.h: os2$(SEP)pmerr.uni

$(OUT)$(SEP)os2$(SEP)wperrors.h: os2$(SEP)wperrors.uni

$(OUT)$(SEP)os2$(SEP)gpierrors.h: os2$(SEP)gpierrors.uni

$(OUT)$(SEP)os2$(SEP)splerrors.h: os2$(SEP)splerrors.uni

$(OUT)$(SEP)os2$(SEP)picerrors.h: os2$(SEP)picerrors.uni

$(OUT)$(SEP)os2$(SEP)winhooks.h: os2$(SEP)winhooks.uni

$(OUT)$(SEP)os2$(SEP)wintimer.h: os2$(SEP)wintimer.uni

$(OUT)$(SEP)os2$(SEP)wincountry.h: os2$(SEP)wincountry.uni

$(OUT)$(SEP)os2$(SEP)windialogscommon.h: os2$(SEP)windialogscommon.uni

$(OUT)$(SEP)os2$(SEP)windialogs.h: os2$(SEP)windialogs.uni

$(OUT)$(SEP)os2$(SEP)winentryfields.h: os2$(SEP)winentryfields.uni

$(OUT)$(SEP)os2$(SEP)winbuttons.h: os2$(SEP)winbuttons.uni

$(OUT)$(SEP)os2$(SEP)winlistboxes.h: os2$(SEP)winlistboxes.uni

$(OUT)$(SEP)os2$(SEP)pmgpi.h: os2$(SEP)pmgpi.uni

$(OUT)$(SEP)os2$(SEP)gpiprimitives.h: os2$(SEP)gpiprimitives.uni

$(OUT)$(SEP)os2$(SEP)gpilcids.h: os2$(SEP)gpilcids.uni

$(OUT)$(SEP)os2$(SEP)gpibitmapscommon.h: os2$(SEP)gpibitmapscommon.uni

$(OUT)$(SEP)os2$(SEP)gpicontrolcommon.h: os2$(SEP)gpicontrolcommon.uni

## DOS Personality files
dos: $(OUT)$(SEP)dos$(SEP)dos.h .symbolic # $(OUT)$(SEP)dos$(SEP)os2vdm.h

$(OUT)$(SEP)dos$(SEP)dos.h: dos$(SEP)dos.uni

$(OUT)$(SEP)dos$(SEP)os2vdm.h: dos$(SEP)os2vdm.uni

.SUFFIXES:
.SUFFIXES: .h .uni

.uni: $(MYDIR)shared
.uni: $(MYDIR)os2
.uni: $(MYDIR)dos

.uni.h: .AUTODEPEND
 uni2h.exe $< $^@
 $(REXX) cut.cmd $^@
 $(DC) $^@
 $(CP) tmp $^@ $(BLACKHOLE)
 $(DC) tmp
