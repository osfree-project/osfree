#
# A Makefile for Unified API
# (c) osFree project,
# valerius, 2006/10/30
#

!include $(%ROOT)/mk/all.mk

OUT = $(%ROOT)$(SEP)build$(SEP)include
TARGETS = h

subdirs = $(OUT) $(OUT)$(SEP)dos $(OUT)$(SEP)os2

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
     $(OUT)$(SEP)os2$(SEP)bseerr.h &
     $(OUT)$(SEP)os2$(SEP)basemid.h &
     $(OUT)$(SEP)os2$(SEP)bsetib.h &
     $(OUT)$(SEP)os2$(SEP)dosdebug.h &
     $(OUT)$(SEP)os2$(SEP)dosmisc.h &
     $(OUT)$(SEP)os2$(SEP)dosmvdm.h &
     $(OUT)$(SEP)os2$(SEP)dosnls.h &
     $(OUT)$(SEP)os2$(SEP)dosnmpipes.h &
     $(OUT)$(SEP)os2$(SEP)dosmodulemgr.h &
     $(OUT)$(SEP)os2$(SEP)dosfilemgr.h &
     $(OUT)$(SEP)os2$(SEP)dosfilemgrcommon.h &
     $(OUT)$(SEP)os2$(SEP)dosmemmgrcommon.h &
     $(OUT)$(SEP)os2$(SEP)dosprocess.h &
     $(OUT)$(SEP)os2$(SEP)dossemaphores.h &
     $(OUT)$(SEP)os2$(SEP)dossemaphorescommon.h &
     $(OUT)$(SEP)os2$(SEP)dosprocesscommon.h &
     $(OUT)$(SEP)os2$(SEP)dosresources.h &
     $(OUT)$(SEP)os2$(SEP)dosexceptions.h &
     $(OUT)$(SEP)os2$(SEP)dosdatetime.h &
     $(OUT)$(SEP)os2$(SEP)dosdatetimecommon.h &
     $(OUT)$(SEP)os2$(SEP)pm.h &
     $(OUT)$(SEP)os2$(SEP)pmdef.h &
     $(OUT)$(SEP)os2$(SEP)pmshl.h &
     $(OUT)$(SEP)os2$(SEP)pmwp.h &
     $(OUT)$(SEP)os2$(SEP)pmwin.h &
     $(OUT)$(SEP)os2$(SEP)winclipboarddde.h &
     $(OUT)$(SEP)os2$(SEP)winprogramlist.h &
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

$(OUT)$(SEP)os2$(SEP)dosprocesscommon.h: os2$(SEP)dosprocesscommon.uni

$(OUT)$(SEP)os2$(SEP)dosprocess.h: os2$(SEP)dosprocess.uni

$(OUT)$(SEP)os2$(SEP)dosmisc.h: os2$(SEP)dosmisc.uni

$(OUT)$(SEP)os2$(SEP)basemid.h: os2$(SEP)basemid.uni

$(OUT)$(SEP)os2$(SEP)dosnmpipes.h: os2$(SEP)dosnmpipes.uni

$(OUT)$(SEP)os2$(SEP)dosmvdm.h: os2$(SEP)dosmvdm.uni

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

$(OUT)$(SEP)os2$(SEP)dosresources.h: os2$(SEP)dosresources.uni


$(OUT)$(SEP)os2$(SEP)dosdebug.h: os2$(SEP)dosdebug.uni

$(OUT)$(SEP)os2$(SEP)hevdef.h: os2$(SEP)hevdef.uni

$(OUT)$(SEP)os2$(SEP)pm.h: os2$(SEP)pm.uni

$(OUT)$(SEP)os2$(SEP)pmdef.h: os2$(SEP)pmdef.uni

$(OUT)$(SEP)os2$(SEP)pmshl.h: os2$(SEP)pmshl.uni

$(OUT)$(SEP)os2$(SEP)pmwp.h: os2$(SEP)pmwp.uni

$(OUT)$(SEP)os2$(SEP)pmwin.h: os2$(SEP)pmwin.uni

$(OUT)$(SEP)os2$(SEP)winprogramlist.h: os2$(SEP)winprogramlist.uni

$(OUT)$(SEP)os2$(SEP)winclipboarddde.h: os2$(SEP)winclipboarddde.uni

## DOS Personality files
dos: $(OUT)$(SEP)dos$(SEP)dos.h .symbolic # $(OUT)$(SEP)dos$(SEP)os2vdm.h

$(OUT)$(SEP)dos$(SEP)dos.h: dos$(SEP)dos.uni

$(OUT)$(SEP)dos$(SEP)os2vdm.h: dos$(SEP)os2vdm.uni

.SUFFIXES:
.SUFFIXES: .h .uni

.uni: $(MYDIR)os2
.uni: $(MYDIR)dos

.uni.h: .AUTODEPEND
 uni2h.exe $< $^@
 $(REXX) cut.cmd $^@
 $(DC) $^@
 $(CP) tmp $^@ $(BLACKHOLE)
 $(DC) tmp
