#
# A main Makefile for OS/3 boot sequence project
# (c) osFree project,
# valerius, 2006/10/30
#

DIRS         = shared filesys term preldr loader kernels
DEST         = boot$(SEP)loader
INSTALL_ADD  = 1

!include $(%ROOT)/mk/all.mk

install_add: $(DEST)$(SEP)boot.cfg $(DEST)$(SEP)preldr.ini .symbolic

$(DEST)$(SEP)boot.cfg: $(MYDIR)boot.cfg
 @$(SAY) INST     $^. $(LOG)
 @$(CP) $< $^@ $(BLACKHOLE)

$(DEST)$(SEP)preldr.ini: $(MYDIR)preldr.ini
 @$(SAY) INST     $^. $(LOG)
 @$(CP) $< $^@ $(BLACKHOLE)
