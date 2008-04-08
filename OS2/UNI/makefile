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

all: .SYMBOLIC
# Main osFree file
  uni2h osfree.uni $(OUT)$(SEP)osfree.h
# Workaround of uniplemented features of uni2h tool
  $(CP) cdeftypes.h $(OUT)$(SEP)cdeftypes.h
  $(CP) cdeftypes2.h $(OUT)$(SEP)cdeftypes2.h
!ifeq OS_REXX Classic
  cmd /c cut $(OUT)$(SEP)osfree.h
!else
  $(OS_REXX) cut.cmd $(OUT)$(SEP)osfree.h
!endif
  $(DC) $(OUT)$(SEP)osfree.h
  $(CP) tmp $(OUT)$(SEP)osfree.h
  $(DC) tmp
## OS/2 Personality files
  uni2h os2$(SEP)os2.uni $(OUT)$(SEP)OS2$(SEP)os2.h
!ifeq OS_REXX Classic
  cmd /c cut $(OUT)$(SEP)OS2$(SEP)os2.h
!else
  $(OS_REXX) cut.cmd $(OUT)$(SEP)OS2$(SEP)os2.h
!endif
  $(DC) $(OUT)$(SEP)OS2$(SEP)os2.h
  $(CP) tmp $(OUT)$(SEP)OS2$(SEP)os2.h
  $(DC) tmp
### OS/2 Personality base type and macros definitions
  uni2h os2$(SEP)os2def.uni $(OUT)$(SEP)OS2$(SEP)os2def.h
### OS/2 Personality base API
  uni2h os2$(SEP)bse.uni $(OUT)$(SEP)OS2$(SEP)bse.h

# Clean all

.IGNORE
clean: .SYMBOLIC
 $(SAY) Making clean... $(LOG)
 $(CLEAN_CMD)
