#
# A Makefile for mkmsgf
# (c) osFree project,
# valerius, 2006/10/30
#
# $Id: makefile,v 1.1 2004/08/16 06:27:30 prokushev Exp $
#

!include $(%ROOT)/build.conf
!include $(%ROOT)/mk/site.mk

32_BITS  = 1

!include $(%ROOT)/mk/all.mk

OUT = $(%ROOT)$(SEP)INCLUDE$(SEP)OS3

all: h .SYMBOLIC

h: workaround $(OUT)$(SEP)osfree.h os2 dos
  @rem

$(OUT)$(SEP)osfree.h: osfree.uni
# Main osFree file
  uni2h osfree.uni $(OUT)$(SEP)osfree.h
  $(REXX) cut.cmd $(OUT)$(SEP)osfree.h
  $(DC) $(OUT)$(SEP)osfree.h
  $(CP) tmp $(OUT)$(SEP)osfree.h $(BLACKHOLE)
  $(DC) tmp

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
     .symbolic

#     $(OUT)$(SEP)OS2$(SEP)bsexcpt.h &

$(OUT)$(SEP)OS2$(SEP)os2.h: os2$(SEP)os2.uni
  uni2h os2$(SEP)os2.uni $(OUT)$(SEP)OS2$(SEP)os2.h
  $(REXX) cut.cmd $(OUT)$(SEP)OS2$(SEP)os2.h
  $(DC) $(OUT)$(SEP)OS2$(SEP)os2.h
  $(CP) tmp $(OUT)$(SEP)OS2$(SEP)os2.h
  $(DC) tmp

### OS/2 Personality base type and macros definitions
$(OUT)$(SEP)OS2$(SEP)os2def.h: os2$(SEP)os2def.uni
  uni2h os2$(SEP)os2def.uni $(OUT)$(SEP)OS2$(SEP)os2def.h

### OS/2 Personality base API
$(OUT)$(SEP)OS2$(SEP)bse.h: os2$(SEP)bse.uni
  uni2h os2$(SEP)bse.uni $(OUT)$(SEP)OS2$(SEP)bse.h

### OS/2 Personality base API ordinals
$(OUT)$(SEP)OS2$(SEP)bseord.h: os2$(SEP)bseord.uni
  uni2h os2$(SEP)bseord.uni $(OUT)$(SEP)OS2$(SEP)bseord.h

### OS/2 Personality memory API flags
$(OUT)$(SEP)OS2$(SEP)bsememf.h: os2$(SEP)bsememf.uni
  uni2h os2$(SEP)bsememf.uni $(OUT)$(SEP)OS2$(SEP)bsememf.h

$(OUT)$(SEP)OS2$(SEP)bsetib.h: os2$(SEP)bsetib.uni
  uni2h os2$(SEP)bsetib.uni $(OUT)$(SEP)OS2$(SEP)bsetib.h

$(OUT)$(SEP)OS2$(SEP)bsexcpt.h: os2$(SEP)bsexcpt.uni
  uni2h os2$(SEP)bsexcpt.uni $(OUT)$(SEP)OS2$(SEP)bsexcpt.h

$(OUT)$(SEP)OS2$(SEP)dosprocesscommon.h: os2$(SEP)dosprocesscommon.uni
  uni2h os2$(SEP)dosprocesscommon.uni $(OUT)$(SEP)OS2$(SEP)dosprocesscommon.h

$(OUT)$(SEP)OS2$(SEP)dosprocess.h: os2$(SEP)dosprocess.uni
  uni2h os2$(SEP)dosprocess.uni $(OUT)$(SEP)OS2$(SEP)dosresources.h

$(OUT)$(SEP)OS2$(SEP)dosmisc.h: os2$(SEP)dosmisc.uni
  uni2h os2$(SEP)dosmisc.uni $(OUT)$(SEP)OS2$(SEP)dosmisc.h

$(OUT)$(SEP)OS2$(SEP)dosmvdm.h: os2$(SEP)dosmvdm.uni
  uni2h os2$(SEP)dosmvdm.uni $(OUT)$(SEP)OS2$(SEP)dosmvdm.h

$(OUT)$(SEP)OS2$(SEP)dosmodulemgr.h: os2$(SEP)dosmodulemgr.uni
  uni2h os2$(SEP)dosmodulemgr.uni $(OUT)$(SEP)OS2$(SEP)dosmodulemgr.h

$(OUT)$(SEP)OS2$(SEP)dosexceptions.h: os2$(SEP)dosexceptions.uni
  uni2h os2$(SEP)dosexceptions.uni $(OUT)$(SEP)OS2$(SEP)dosexceptions.h

$(OUT)$(SEP)OS2$(SEP)dosresources.h: os2$(SEP)dosresources.uni
  uni2h os2$(SEP)dosresources.uni $(OUT)$(SEP)OS2$(SEP)dosresources.h

$(OUT)$(SEP)OS2$(SEP)dosdevices.h: os2$(SEP)dosdevices.uni
  uni2h os2$(SEP)dosdevices.uni $(OUT)$(SEP)OS2$(SEP)dosdevices.h

$(OUT)$(SEP)OS2$(SEP)dosdebug.h: os2$(SEP)dosdebug.uni
  uni2h os2$(SEP)dosdebug.uni $(OUT)$(SEP)OS2$(SEP)dosdebug.h

## DOS Personality files
dos: $(OUT)$(SEP)DOS$(SEP)dos.h $(OUT)$(SEP)DOS$(SEP)os2vdm.h .symbolic

$(OUT)$(SEP)DOS$(SEP)dos.h: dos$(SEP)dos.uni
  uni2h dos$(SEP)dos.uni $(OUT)$(SEP)DOS$(SEP)dos.h

# Clean all

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
