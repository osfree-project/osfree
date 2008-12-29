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
  @rem

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
os2: $(OUT)$(SEP)OS2$(SEP)os2.h $(OUT)$(SEP)OS2$(SEP)os2def.h &
     $(OUT)$(SEP)OS2$(SEP)bse.h $(OUT)$(SEP)OS2$(SEP)bseord.h &
     $(OUT)$(SEP)OS2$(SEP)bsememf.h &
     $(OUT)$(SEP)OS2$(SEP)bsetib.h &
     $(OUT)$(SEP)OS2$(SEP)dosdebug.h &
     $(OUT)$(SEP)OS2$(SEP)dosmisc.h &
     $(OUT)$(SEP)OS2$(SEP)dosmvdm.h &
     $(OUT)$(SEP)OS2$(SEP)dosmodulemgr.h &
     $(OUT)$(SEP)OS2$(SEP)dosprocess.h &
     $(OUT)$(SEP)OS2$(SEP)dosresources.h &
     $(OUT)$(SEP)OS2$(SEP)dosexceptions.h &
     $(OUT)$(SEP)OS2$(SEP)dosdevices.h &
     $(OUT)$(SEP)OS2$(SEP)dosprocesscommon.h &
     $(OUT)$(SEP)OS2$(SEP)pmdef.h &
     .symbolic

#     $(OUT)$(SEP)OS2$(SEP)bsexcpt.h &

$(OUT)$(SEP)OS2$(SEP)os2.h: os2$(SEP)os2.uni

### OS/2 Personality base type and macros definitions
$(OUT)$(SEP)OS2$(SEP)os2def.h: os2$(SEP)os2def.uni

### OS/2 Personality base API
$(OUT)$(SEP)OS2$(SEP)bse.h: os2$(SEP)bse.uni

### OS/2 Personality base API ordinals
$(OUT)$(SEP)OS2$(SEP)bseord.h: os2$(SEP)bseord.uni

### OS/2 Personality memory API flags
$(OUT)$(SEP)OS2$(SEP)bsememf.h: os2$(SEP)bsememf.uni

$(OUT)$(SEP)OS2$(SEP)bsetib.h: os2$(SEP)bsetib.uni

$(OUT)$(SEP)OS2$(SEP)bsexcpt.h: os2$(SEP)bsexcpt.uni

$(OUT)$(SEP)OS2$(SEP)dosprocesscommon.h: os2$(SEP)dosprocesscommon.uni

$(OUT)$(SEP)OS2$(SEP)dosprocess.h: os2$(SEP)dosprocess.uni

$(OUT)$(SEP)OS2$(SEP)dosmisc.h: os2$(SEP)dosmisc.uni

$(OUT)$(SEP)OS2$(SEP)dosmvdm.h: os2$(SEP)dosmvdm.uni

$(OUT)$(SEP)OS2$(SEP)dosmodulemgr.h: os2$(SEP)dosmodulemgr.uni

$(OUT)$(SEP)OS2$(SEP)dosexceptions.h: os2$(SEP)dosexceptions.uni

$(OUT)$(SEP)OS2$(SEP)dosresources.h: os2$(SEP)dosresources.uni

$(OUT)$(SEP)OS2$(SEP)dosdevices.h: os2$(SEP)dosdevices.uni

$(OUT)$(SEP)OS2$(SEP)dosdebug.h: os2$(SEP)dosdebug.uni

$(OUT)$(SEP)OS2$(SEP)pmdef.h: os2$(SEP)pmdef.uni

## DOS Personality files
dos: $(OUT)$(SEP)DOS$(SEP)dos.h .symbolic # $(OUT)$(SEP)DOS$(SEP)os2vdm.h

$(OUT)$(SEP)DOS$(SEP)dos.h: dos$(SEP)dos.uni

$(OUT)$(SEP)DOS$(SEP)os2vdm.h: dos$(SEP)os2vdm.uni

.SUFFIXES:
.SUFFIXES: .h .uni

.uni: $(MYDIR)os2
.uni: $(MYDIR)dos

.uni.h: .AUTODEPEND
 uni2h $< $^@
 $(REXX) cut.cmd $^@
 $(DC) $^@
 $(CP) tmp $^@ $(BLACKHOLE)
 $(DC) tmp
